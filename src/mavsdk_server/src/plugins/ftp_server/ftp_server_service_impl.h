// WARNING: THIS FILE IS AUTOGENERATED! As such, it should not be edited.
// Edits need to be made to the proto files
// (see https://github.com/mavlink/MAVSDK-Proto/blob/master/protos/ftp_server/ftp_server.proto)

#include "ftp_server/ftp_server.grpc.pb.h"
#include "plugins/ftp_server/ftp_server.h"

#include "mavsdk.h"

#include "lazy_server_plugin.h"

#include "log.h"
#include <atomic>
#include <cmath>
#include <future>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

namespace mavsdk {
namespace mavsdk_server {

template<typename FtpServer = FtpServer, typename LazyServerPlugin = LazyServerPlugin<FtpServer>>

class FtpServerServiceImpl final : public rpc::ftp_server::FtpServerService::Service {
public:
    FtpServerServiceImpl(LazyServerPlugin& lazy_plugin) : _lazy_plugin(lazy_plugin) {}

    template<typename ResponseType>
    void fillResponseWithResult(ResponseType* response, mavsdk::FtpServer::Result& result) const
    {
        auto rpc_result = translateToRpcResult(result);

        auto* rpc_ftp_server_result = new rpc::ftp_server::FtpServerResult();
        rpc_ftp_server_result->set_result(rpc_result);
        std::stringstream ss;
        ss << result;
        rpc_ftp_server_result->set_result_str(ss.str());

        response->set_allocated_ftp_server_result(rpc_ftp_server_result);
    }

    static rpc::ftp_server::FtpServerResult::Result
    translateToRpcResult(const mavsdk::FtpServer::Result& result)
    {
        switch (result) {
            default:
                LogErr() << "Unknown result enum value: " << static_cast<int>(result);
            // FALLTHROUGH
            case mavsdk::FtpServer::Result::Unknown:
                return rpc::ftp_server::FtpServerResult_Result_RESULT_UNKNOWN;
            case mavsdk::FtpServer::Result::Success:
                return rpc::ftp_server::FtpServerResult_Result_RESULT_SUCCESS;
            case mavsdk::FtpServer::Result::DoesNotExist:
                return rpc::ftp_server::FtpServerResult_Result_RESULT_DOES_NOT_EXIST;
            case mavsdk::FtpServer::Result::Busy:
                return rpc::ftp_server::FtpServerResult_Result_RESULT_BUSY;
        }
    }

    static mavsdk::FtpServer::Result
    translateFromRpcResult(const rpc::ftp_server::FtpServerResult::Result result)
    {
        switch (result) {
            default:
                LogErr() << "Unknown result enum value: " << static_cast<int>(result);
            // FALLTHROUGH
            case rpc::ftp_server::FtpServerResult_Result_RESULT_UNKNOWN:
                return mavsdk::FtpServer::Result::Unknown;
            case rpc::ftp_server::FtpServerResult_Result_RESULT_SUCCESS:
                return mavsdk::FtpServer::Result::Success;
            case rpc::ftp_server::FtpServerResult_Result_RESULT_DOES_NOT_EXIST:
                return mavsdk::FtpServer::Result::DoesNotExist;
            case rpc::ftp_server::FtpServerResult_Result_RESULT_BUSY:
                return mavsdk::FtpServer::Result::Busy;
        }
    }

    grpc::Status SetRootDir(
        grpc::ServerContext* /* context */,
        const rpc::ftp_server::SetRootDirRequest* request,
        rpc::ftp_server::SetRootDirResponse* response) override
    {
        if (_lazy_plugin.maybe_plugin() == nullptr) {
            if (response != nullptr) {
                // For server plugins, this should never happen, they should always be
                // constructible.
                auto result = mavsdk::FtpServer::Result::Unknown;
                fillResponseWithResult(response, result);
            }

            return grpc::Status::OK;
        }

        if (request == nullptr) {
            LogWarn() << "SetRootDir sent with a null request! Ignoring...";
            return grpc::Status::OK;
        }

        auto result = _lazy_plugin.maybe_plugin()->set_root_dir(request->path());

        if (response != nullptr) {
            fillResponseWithResult(response, result);
        }

        return grpc::Status::OK;
    }

    void stop()
    {
        _stopped.store(true);
        for (auto& prom : _stream_stop_promises) {
            if (auto handle = prom.lock()) {
                handle->set_value();
            }
        }
    }

private:
    void register_stream_stop_promise(std::weak_ptr<std::promise<void>> prom)
    {
        // If we have already stopped, set promise immediately and don't add it to list.
        if (_stopped.load()) {
            if (auto handle = prom.lock()) {
                handle->set_value();
            }
        } else {
            _stream_stop_promises.push_back(prom);
        }
    }

    void unregister_stream_stop_promise(std::shared_ptr<std::promise<void>> prom)
    {
        for (auto it = _stream_stop_promises.begin(); it != _stream_stop_promises.end();
             /* ++it */) {
            if (it->lock() == prom) {
                it = _stream_stop_promises.erase(it);
            } else {
                ++it;
            }
        }
    }

    LazyServerPlugin& _lazy_plugin;

    std::atomic<bool> _stopped{false};
    std::vector<std::weak_ptr<std::promise<void>>> _stream_stop_promises{};
};

} // namespace mavsdk_server
} // namespace mavsdk