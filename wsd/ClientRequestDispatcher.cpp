/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Implementation of client request routing and handling.
 * Classes: ClientRequestDispatcher
 */

#include <config.h>

#include <wsd/ClientRequestDispatcher.hpp>
#include <wsd/ContentType.hpp>

#if ENABLE_FEATURE_LOCK
#include <common/CommandControl.hpp>
#endif

#include <common/Anonymizer.hpp>
#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/NumUtil.hpp>
#include <common/StateEnum.hpp>
#include <common/Util.hpp>
#include <net/AsyncDNS.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpRequest.hpp>
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>
#include <net/Uri.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Exceptions.hpp>
#include <wsd/FileServer.hpp>
#include <wsd/ProofKey.hpp>
#include <wsd/ProxyRequestHandler.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/RequestVettingStation.hpp>
#include <wsd/UserMessages.hpp>

#if !MOBILEAPP
#include <common/JailUtil.hpp>
#include <wsd/Admin.hpp>
#include <wsd/HostUtil.hpp>
#include <wsd/SpecialBrokers.hpp>
#endif // !MOBILEAPP

#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/File.h>
#include <Poco/MemoryStream.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/StreamCopier.h>

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

std::map<std::string, std::string> ClientRequestDispatcher::StaticFileContentCache;

std::size_t ClientRequestDispatcher::NextRvsCleanupSize = RvsLowWatermark;
std::unordered_map<std::string, std::shared_ptr<RequestVettingStation>>
    ClientRequestDispatcher::RequestVettingStations;

extern std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
extern std::mutex DocBrokersMutex;

#if !MOBILEAPP
static constexpr std::string_view MEDIA_STR = "str";
static constexpr std::string_view MEDIA_MP4 = "url";
#endif

namespace
{

#if ENABLE_SUPPORT_KEY
/// Used in support key enabled builds
inline void shutdownLimitReached(const std::shared_ptr<ProtocolHandlerInterface>& proto)
{
    if (!proto)
        return;

    std::ostringstream oss;
    oss << "error: cmd=socket kind=hardlimitreached params=" << COOLWSD::MaxDocuments << ","
        << COOLWSD::MaxConnections;
    const std::string error = oss.str();
    LOG_INF("Sending client 'hardlimitreached' message: " << error);

    try
    {
        // Let the client know we are shutting down.
        proto->sendTextMessage(error);

        // Shutdown.
        proto->shutdown(true, error);
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Error while shutting down socket on reaching limit: " << ex.what());
    }
}
#endif

} // end anonymous namespace

/// Find the DocumentBroker for the given docKey, if one exists.
/// Otherwise, creates and adds a new one to DocBrokers.
/// May return null if terminating or MaxDocuments limit is reached.
/// Returns the error message, if any, when no DocBroker is created/found.
extern std::pair<std::shared_ptr<DocumentBroker>, std::string>
findOrCreateDocBroker(DocumentBroker::ChildType type, const std::string& uri,
                      const std::string& docKey, const std::string& configId, const std::string& id,
                      const Poco::URI& uriPublic, unsigned mobileAppDocId)
{
    LOG_INF("Find or create DocBroker for docKey ["
            << docKey << "] for session [" << id << "] on url ["
            << COOLWSD::anonymizeUrl(uriPublic.toString()) << ']'
            << " with configid " << configId);

    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);

    COOLWSD::cleanupDocBrokers();

    if (SigUtil::getShutdownRequestFlag())
    {
        // TerminationFlag implies ShutdownRequested.
        LOG_WRN((SigUtil::getTerminationFlag() ? "TerminationFlag" : "ShudownRequestedFlag")
                << " set. Not loading new session [" << id << "] for docKey [" << docKey << ']');

        return std::make_pair(nullptr, "error: cmd=load kind=recycling");
    }

    std::shared_ptr<DocumentBroker> docBroker;

    // Lookup this document.
    const auto it = DocBrokers.find(docKey);
    if (it != DocBrokers.end() && it->second)
    {
        // Get the DocumentBroker from the Cache.
        LOG_DBG("Found DocumentBroker with docKey [" << docKey << ']');
        docBroker = it->second;

        // Destroying the document? Let the client reconnect.
        if (docBroker->isUnloadingUnrecoverably())
        {
            LOG_WRN("DocBroker [" << docKey
                                  << "] is unloading. Rejecting client request to load session ["
                                  << id << ']');

            return std::make_pair(nullptr, "error: cmd=load kind=docunloading");
        }
    }
    else
    {
        LOG_DBG("No DocumentBroker with docKey [" << docKey
                                                  << "] found. Creating new Child and Document");
    }

    if (SigUtil::getShutdownRequestFlag())
    {
        // TerminationFlag implies ShutdownRequested.
        LOG_ERR((SigUtil::getTerminationFlag() ? "TerminationFlag" : "ShudownRequestedFlag")
                << " set. Not loading new session [" << id << "] for docKey [" << docKey << ']');

        return std::make_pair(nullptr, "error: cmd=load kind=recycling");
    }

    if (!docBroker)
    {
        Util::assertIsLocked(DocBrokersMutex);
        if (DocBrokers.size() + 1 > COOLWSD::MaxDocuments)
        {
            LOG_WRN("Maximum number of open documents of "
                    << COOLWSD::MaxDocuments << " reached while loading new session [" << id
                    << "] for docKey [" << docKey << ']');
            if constexpr (ConfigUtil::isSupportKeyEnabled())
            {
                std::ostringstream oss;
                oss << "error: cmd=socket kind=hardlimitreached params=" << COOLWSD::MaxDocuments
                    << "," << COOLWSD::MaxConnections;
                return std::make_pair(nullptr, oss.str());
            }
        }

        // Set the one we just created.
        LOG_DBG("New DocumentBroker for docKey [" << docKey << ']');
        docBroker = std::make_shared<DocumentBroker>(type, uri, uriPublic, docKey,
                                                     configId, mobileAppDocId);
        docBroker->loadTimings().record("docBrokerCreated");
        DocBrokers.emplace(docKey, docBroker);
        LOG_TRC("Have " << DocBrokers.size() << " DocBrokers after inserting [" << docKey << ']');
    }

    return std::make_pair(docBroker, std::string());
}

#if !MOBILEAPP

/// For clipboard setting
class ClipboardPartHandler : public Poco::Net::PartHandler
{
    std::string _filename;

public:
    /// Afterwards someone else is responsible for cleaning that up.
    void takeFile() { _filename.clear(); }

    ClipboardPartHandler(std::string filename)
        : _filename(std::move(filename))
    {
    }

    virtual ~ClipboardPartHandler()
    {
        if (!_filename.empty())
        {
            LOG_TRC("Remove temporary clipboard file '" << _filename << '\'');
            StatelessBatchBroker::removeFile(_filename);
        }
    }

    virtual void handlePart(const Poco::Net::MessageHeader& /* header */,
                            std::istream& stream) override
    {
        LOG_DBG("Storing incoming clipboard to: " << _filename);

        // Copy the stream to _filename.
        std::ofstream fileStream;
        fileStream.open(_filename);

        Poco::StreamCopier::copyStream(stream, fileStream);
        fileStream.close();
    }
};

/// Handles the filename part of the convert-to POST request payload,
/// Also owns the file - cleaning it up when destroyed.
class ConvertToPartHandler : public Poco::Net::PartHandler
{
    /// Parameter name -> filename map.
    AdditionalFilePaths _filenames;

public:
    const AdditionalFilePaths& getFilenames() const { return _filenames; }

    /// Afterwards someone else is responsible for cleaning that up.
    void takeFiles() { _filenames.clear(); }

    ConvertToPartHandler() = default;

    ~ConvertToPartHandler() override
    {
        for (const auto& it : _filenames)
        {
            const std::string& filename = it.second;
            LOG_TRC("Remove un-handled temporary file '" << filename << '\'');
            StatelessBatchBroker::removeFile(filename);
        }
    }

    virtual void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
    {
        // Extract filename and put it to a temporary directory.
        std::string disp;
        Poco::Net::NameValueCollection params;
        if (header.has("Content-Disposition"))
        {
            std::string cd = header.get("Content-Disposition");
            Poco::Net::MessageHeader::splitParameters(cd, disp, params);
        }

        if (!params.has("filename"))
            return;

        // The temporary directory is child-root/<CHILDROOT_TMP_INCOMING_PATH>.
        // Always create a random sub-directory to avoid file-name collision.
        Poco::Path tempPath = Poco::Path::forDirectory(
            FileUtil::createRandomTmpDir(COOLWSD::ChildRoot +
                                         JailUtil::CHILDROOT_TMP_INCOMING_PATH) +
            '/');
        LOG_TRC("Created temporary convert-to/insert path: " << tempPath.toString());

        // Prevent user inputting anything funny here.
        std::string fileParam = params.get("filename");
        std::string cleanFilename = Util::cleanupFilename(fileParam);
        if (fileParam != cleanFilename)
            LOG_DBG("Unexpected characters in conversion filename '"
                    << fileParam << "' cleaned to '" << cleanFilename << "'");

        // A "filename" should always be a filename, not a path
        const Poco::Path filenameParam(cleanFilename);
        if (filenameParam.getFileName() == "callback:")
            tempPath.setFileName("incoming_file"); // A sensible name.
        else
            tempPath.setFileName(filenameParam.getFileName()); //TODO: Sanitize.
        std::string paramName = "data";
        if (params.has("name"))
        {
            paramName = params.get("name");
        }
        _filenames[paramName] = tempPath.toString();
        LOG_DBG("Storing incoming file to: " << _filenames[paramName]);

        // Copy the stream to the temp path.
        std::ofstream fileStream;
        fileStream.open(tempPath.toString());
        Poco::StreamCopier::copyStream(stream, fileStream);
        fileStream.close();
    }
};

class RenderSearchResultPartHandler : public Poco::Net::PartHandler
{
private:
    std::string _filename;
    std::shared_ptr<std::vector<char>> _pSearchResultContent;

public:
    std::string getFilename() const { return _filename; }

    /// Afterwards someone else is responsible for cleaning that up.
    void takeFile() { _filename.clear(); }

    const std::shared_ptr<std::vector<char>>& getSearchResultContent() const
    {
        return _pSearchResultContent;
    }

    RenderSearchResultPartHandler() = default;

    virtual ~RenderSearchResultPartHandler()
    {
        if (!_filename.empty())
        {
            LOG_TRC("Remove un-handled temporary file '" << _filename << '\'');
            StatelessBatchBroker::removeFile(_filename);
        }
    }

    virtual void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
    {
        // Extract filename and put it to a temporary directory.
        std::string label;
        Poco::Net::NameValueCollection content;
        if (header.has("Content-Disposition"))
        {
            Poco::Net::MessageHeader::splitParameters(header.get("Content-Disposition"), label,
                                                      content);
        }

        std::string name = content.get("name", "");
        if (name == "document")
        {
            std::string filename = content.get("filename", "");

            const Poco::Path filenameParam(filename);

            // The temporary directory is child-root/<JAIL_TMP_INCOMING_PATH>.
            // Always create a random sub-directory to avoid file-name collision.
            Poco::Path tempPath = Poco::Path::forDirectory(
                FileUtil::createRandomTmpDir(COOLWSD::ChildRoot +
                                             JailUtil::CHILDROOT_TMP_INCOMING_PATH) +
                '/');

            LOG_TRC("Created temporary render-search-result file path: " << tempPath.toString());

            // Prevent user inputting anything funny here.
            // A "filename" should always be a filename, not a path

            if (filenameParam.getFileName() == "callback:")
                tempPath.setFileName("incoming_file"); // A sensible name.
            else
                tempPath.setFileName(filenameParam.getFileName()); //TODO: Sanitize.
            _filename = tempPath.toString();

            // Copy the stream to _filename.
            std::ofstream fileStream;
            fileStream.open(_filename);
            Poco::StreamCopier::copyStream(stream, fileStream);
            fileStream.close();
        }
        else if (name == "result")
        {
            // Copy content from the stream into a std::vector<char>
            _pSearchResultContent = std::make_shared<std::vector<char>>(
                std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
        }
    }
};

/// Constructs ConvertToBroker implementation based on request type
static std::shared_ptr<ConvertToBroker>
getConvertToBrokerImplementation(const std::string& requestType, const std::string& fromPath,
                                 const Poco::URI& uriPublic, const std::string& docKey,
                                 const std::string& format, const std::string& options,
                                 const std::string& lang, const std::string& target,
                                 const std::string& filter, const std::string& transformJSON)
{
    if (requestType == "convert-to")
        return std::make_shared<ConvertToBroker>(fromPath, uriPublic, docKey, format, options,
                                                 lang);

    if (requestType == "extract-link-targets")
        return std::make_shared<ExtractLinkTargetsBroker>(fromPath, uriPublic, docKey, lang);

    if (requestType == "extract-document-structure")
        return std::make_shared<ExtractDocumentStructureBroker>(fromPath, uriPublic, docKey, lang,
                                                                filter);
    if (requestType == "transform-document-structure")
    {
        if (format.empty())
            return std::make_shared<TransformDocumentStructureBroker>(
                fromPath, uriPublic, docKey, Poco::Path(fromPath).getExtension(), lang,
                transformJSON);
        return std::make_shared<TransformDocumentStructureBroker>(fromPath, uriPublic, docKey,
                                                                  format, lang, transformJSON);
    }

    if (requestType == "get-thumbnail")
        return std::make_shared<GetThumbnailBroker>(fromPath, uriPublic, docKey, lang, target);

    return nullptr;
}

class ConvertToAddressResolver : public std::enable_shared_from_this<ConvertToAddressResolver>
{
    std::shared_ptr<ConvertToAddressResolver> _selfLifecycle;
    std::vector<std::string> _addressesToResolve;
    ClientRequestDispatcher::AsyncFn _asyncCb;
    bool _allow;
    bool _capabilityQuery;

    void logAddressIsDenied(const std::string& addressToCheck) const
    {
        // capability queries if convert-to is available in order to put that
        // info in its results. If disallowed this isn't an attempt by an
        // unauthorized host to use convert-to, only a query to report if it is
        // possible to use convert-to.
        if (_capabilityQuery)
            LOG_DBG("convert-to: Requesting address is denied: " << addressToCheck);
        else
            LOG_WRN("convert-to: Requesting address is denied: " << addressToCheck);
    }

public:

    ConvertToAddressResolver(std::vector<std::string> addressesToResolve, bool capabilityQuery,
                             ClientRequestDispatcher::AsyncFn asyncCb)
        : _addressesToResolve(std::move(addressesToResolve))
        , _asyncCb(std::move(asyncCb))
        , _allow(true)
        , _capabilityQuery(capabilityQuery)
    {
    }

    void testHostName(const std::string& hostToCheck)
    {
        _allow &= HostUtil::allowedWopiHost(hostToCheck);
    }

    // synchronous case
    bool syncProcess()
    {
        assert(!_asyncCb);
        while (!_addressesToResolve.empty())
        {
            const std::string& addressToCheck = _addressesToResolve.front();

            try
            {
                std::string resolvedHostName = net::canonicalHostName(addressToCheck);
                testHostName(resolvedHostName);
            }
            catch (const Poco::Exception& exc)
            {
                LOG_ERR_S("net::canonicalHostName(\"" << addressToCheck
                                                      << "\") failed: " << exc.displayText());
                // We can't find out the hostname, and it already failed the IP check
                _allow = false;
            }

            if (_allow)
            {
                LOG_INF_S("convert-to: Requesting address is allowed: " << addressToCheck);
            }
            else
            {
                logAddressIsDenied(addressToCheck);
                break;
            }

            _addressesToResolve.pop_back();
        }
        return _allow;
    }

    // asynchronous case
    void startAsyncProcessing()
    {
        assert(_asyncCb);
        _selfLifecycle = shared_from_this();
        dispatchNextLookup();
    }

    std::string toState() const
    {
        std::string state = "ConvertToAddressResolver: ";
        for (const auto& address : _addressesToResolve)
            state += address + ", ";
        state += "\n";
        return state;
    }

    void dispatchNextLookup()
    {
        net::AsyncDNS::DNSThreadFn pushHostnameResolvedToPoll = [this](const net::HostEntry& hostEntry) {
            COOLWSD::getWebServerPoll()->addCallback([this, hostEntry]() {
                hostnameResolved(hostEntry);
            });
        };

        net::AsyncDNS::DNSThreadDumpStateFn dumpState = [this]() -> std::string {
            return toState();
        };

        net::AsyncDNS::lookup(_addressesToResolve.front(), std::move(pushHostnameResolvedToPoll),
                              std::move(dumpState));
    }

    void hostnameResolved(const net::HostEntry& hostEntry)
    {
        if (hostEntry.good())
            testHostName(hostEntry.getCanonicalName());
        else
        {
            LOG_ERR_S("canonicalHostName failed: " << hostEntry.errorMessage());
            // We can't find out the hostname, and it already failed the IP check
            _allow = false;
        }

        const std::string& addressToCheck = _addressesToResolve.front();
        if (_allow)
            LOG_INF_S("convert-to: Requesting address is allowed: " << addressToCheck);
        else
            logAddressIsDenied(addressToCheck);
        _addressesToResolve.pop_back();

        // If hostToCheck is not allowed, or there are no addresses
        // left to check, then do callback and end
        if (!_allow || _addressesToResolve.empty())
        {
            _asyncCb(_allow);
            _selfLifecycle.reset();
            return;
        }
        dispatchNextLookup();
    }
};

bool ClientRequestDispatcher::allowPostFrom(const std::string& address)
{
    static bool init = false;
    static RegexUtil::RegexListMatcher hosts;
    if (!init)
    {
        const auto& app = Poco::Util::Application::instance();
        // Parse the host allow settings.
        for (size_t i = 0;; ++i)
        {
            const std::string path = "net.post_allow.host[" + std::to_string(i) + ']';
            const auto host = app.config().getString(path, "");
            if (!host.empty())
            {
                LOG_INF_S("Adding trusted POST_ALLOW host: [" << host << ']');
                hosts.allow(host);
            }
            else if (!app.config().has(path))
            {
                break;
            }
        }

        init = true;
    }

    return hosts.match(address);
}

bool ClientRequestDispatcher::allowConvertTo(const std::string& address,
                                             const Poco::Net::HTTPRequest& request,
                                             bool capabilityQuery, const AsyncFn& asyncCb)
{
    const bool allow = allowPostFrom(address) || HostUtil::allowedWopiHost(request.getHost());
    if (!allow)
    {
        LOG_WRN_S("convert-to: Requesting address is denied: " << address);
        if (asyncCb)
            asyncCb(false);
        return false;
    }

    LOG_TRC_S("convert-to: Requesting address is allowed: " << address);

    std::vector<std::string> addressesToResolve;

    // Handle forwarded header and make sure all participating IPs are allowed
    if (request.has("X-Forwarded-For"))
    {
        std::string forwardedData = request.get("X-Forwarded-For");
        LOG_INF_S("convert-to: X-Forwarded-For is: " << forwardedData);
        StringVector tokens = StringVector::tokenize(std::move(forwardedData), ',');
        for (const auto& token : tokens)
        {
            std::string param = tokens.getParam(token);
            std::string addressToCheck = Util::trim(param);
            if (!allowPostFrom(addressToCheck))
            {
                // postpone resolving addresses until later
                addressesToResolve.push_back(std::move(addressToCheck));
                continue;
            }

            LOG_INF_S("convert-to: Requesting address is allowed: " << addressToCheck);
        }
    }

    if (addressesToResolve.empty())
    {
        if (asyncCb)
            asyncCb(true);
        return true;
    }

    auto resolver = std::make_shared<ConvertToAddressResolver>(std::move(addressesToResolve),
                                                               capabilityQuery, asyncCb);
    if (asyncCb)
    {
        resolver->startAsyncProcessing();
        return false;
    }
    return resolver->syncProcess();
}

#endif // !MOBILEAPP

std::atomic<uint64_t> ClientRequestDispatcher::NextConnectionId(1);

void ClientRequestDispatcher::onConnect(const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Expected a valid socket in ClientRequestDispatcher::onConnect()");
    _id = Util::encodeId(NextConnectionId++, 3);
    _socket = socket;
    _lastSeenHTTPHeader = socket->getLastSeenTime();
    setLogContext(socket->getFD());
    LOG_TRC("Connected #" << socket->getFD() << " (connection " << _id
                          << ") to ClientRequestDispatcher " << this);
}

namespace
{
#if !MOBILEAPP
/// Starts an asynchronous CheckFileInfo request in parallel to serving
/// static files. At this point, we don't have the client's WebSocket
/// yet, and we're proactively trying to authenticate the client.
void launchAsyncCheckFileInfo(
    const std::string& id, const FileServerRequestHandler::ResourceAccessDetails& accessDetails,
    std::unordered_map<std::string, std::shared_ptr<RequestVettingStation>>& requestVettingStations,
    const std::size_t highWatermark)
{
    const std::string requestKey = RequestDetails::getRequestKey(
        accessDetails.wopiSrc(), accessDetails.accessToken());
    LOG_DBG("RequestKey: [" << requestKey << "], wopiSrc: [" << accessDetails.wopiSrc()
            << "], accessToken: [" << accessDetails.accessToken() << "], noAuthHeader: ["
            << accessDetails.noAuthHeader() << ']');

    std::vector<std::string> options = {
        "access_token=" + accessDetails.accessToken(), "access_token_ttl=0"
    };

    if (!accessDetails.noAuthHeader().empty())
        options.push_back("no_auth_header=" + accessDetails.noAuthHeader());

    if (!accessDetails.permission().empty())
        options.push_back("permission=" + accessDetails.permission());

#if ENABLE_DEBUG
    if (!accessDetails.wopiConfigId().empty())
        options.push_back("configid=" + accessDetails.wopiConfigId());
#endif

    const RequestDetails fullRequestDetails =
        RequestDetails(accessDetails.wopiSrc(), options, /*compat=*/std::string());

    if (requestVettingStations.find(requestKey) != requestVettingStations.end())
    {
        LOG_TRC("Found RVS under key: " << requestKey << ", nothing to do");
    }
    else if (requestVettingStations.size() >= highWatermark)
    {
        LOG_WRN("RequestVettingStations in flight ("
                << requestVettingStations.size() << ") hit the high-watermark (" << highWatermark
                << "); suppressing ahead-of-time CheckFileInfo");
    }
    else
    {
        LOG_TRC("Creating RVS with key: " << requestKey << ", for DocumentLoadURI: "
                                          << fullRequestDetails.getDocumentURI());
        auto it = requestVettingStations.emplace(
            requestKey, std::make_shared<RequestVettingStation>(
                            COOLWSD::getWebServerPoll(), fullRequestDetails));

        // Start async CheckFileInfo, ahead of getting the client WS.
        it.first->second->handleRequest(id);
    }
}

void socketEraseConsumedBytes(const std::shared_ptr<StreamSocket>& socket, size_t headerSize,
                              size_t contentSize, bool servedSync)
{
    if( socket->getInBuffer().size() > 0 ) // erase request from inBuffer if not cleared by ignoreInput
    {
        // Remove the request header from our input buffer
        socket->eraseFirstInputBytes(headerSize);
        if (servedSync)
        {
            // Remove the request body from our input buffer, as it has been served (synchronously)
            // See cool#9621, commit 895c224efae9c21f0481e2fbf024a015656a5a97 and cool#10042
            socket->eraseFirstInputBytes(contentSize);
        }
    }
}
#endif // !MOBILEAPP
} // namespace

void ClientRequestDispatcher::handleIncomingMessage(SocketDisposition& disposition)
{
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (!socket)
    {
        LOG_ERR("Invalid socket while handling incoming client request");
        return;
    }

#if !MOBILEAPP
    if (!ConfigUtil::isSslEnabled() && socket->sniffSSL())
    {
        LOG_ERR("Looks like SSL/TLS traffic on plain http port");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return;
    }

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> delayMs = now - _lastSeenHTTPHeader;

    Poco::Net::HTTPRequest request;
    ssize_t headerSize;
    if (_postContentPending)
    {
        std::streamsize available = std::min<std::streamsize>(_postContentPending,
                                                              socket->getInBuffer().size());
        _postStream.write(socket->getInBuffer().data(), available);
        socket->eraseFirstInputBytes(available);
        _postContentPending -= available;
        if (_postContentPending)
        {
            // not complete, accumulate more
            return;
        }

        ssize_t messageSize = _postStream.tellp();
        _postStream.seekg(0);

        headerSize = socket->readHeader("Client", _postStream, messageSize, request, delayMs);
        if (headerSize < 0)
        {
            // something rotten happened
            socket->asyncShutdown();
            socket->ignoreInput();
        }
        else
        {
            socket->handleExpect(request.get("Expect", std::string()));

            _postStream.seekg(0);
            handleFullMessage(request, _postStream, disposition, socket, headerSize, messageSize - headerSize, false, now);
        }

        _postStream.close();
        _postFileDir.reset();

        return;
    }

    size_t inBufferSize = socket->getInBuffer().size();

#if 0 // debug a specific command's payload
        if (Util::findInVector(socket->getInBuffer(), "insertfile") != std::string::npos)
        {
            std::ostringstream oss(Util::makeDumpStateStream());
            oss << "Debug - specific command:\n";
            socket->dumpState(oss);
            LOG_INF(oss.str());
        }
#endif

    headerSize = readHeader(socket, request, delayMs);
    if (headerSize < 0)
        return;

    assert(!_postStream.is_open() && !_postFileDir && !_postContentPending);

    // start streaming condition
    const bool canStreamToFile =
        request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST &&
        request.getContentLength() > MaxInMemoryHttpRequestSize && // avoid disk I/O for small data
        !request.getChunkedTransferEncoding() && // ignore chunked transfer for now
        !request.has("ProxyPrefix"); // proxy mode assumes nothing consumed

    if (canStreamToFile)
    {
        assert(request.getContentLength() != Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH &&
               "Expected a known Content-Length");
        _postFileDir = std::make_unique<FileUtil::OwnedFile>(FileUtil::createRandomTmpDir(
                    COOLWSD::ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH) + '/', true);
        std::string postFilename = _postFileDir->_file + "poststream";
        _postStream.open(postFilename.c_str(), std::fstream::in | std::fstream::out | std::fstream::trunc);
        if (!_postStream)
        {
            LOG_ERR("Unable to open [" << postFilename << "] for POST streaming");
            _postFileDir.reset();
        }
        else
        {
            _postStream.write(socket->getInBuffer().data(), headerSize);
            socket->eraseFirstInputBytes(headerSize);
            _postContentPending = request.getContentLength();
            return;
        }
    }

    StreamSocket::MessageMap map;
    if (!socket->parseHeader("Client", headerSize, inBufferSize, request, delayMs, map))
        return;

    socket->handleExpect(request.get("Expect", std::string()));

    if (!socket->checkChunks(request, headerSize, map, delayMs))
        return;

    // We may need to re-write the chunks moving the inBuffer.
    socket->compactChunks(map);

    Poco::MemoryInputStream message(socket->getInBuffer().data(), socket->getInBuffer().size());
    handleFullMessage(request, message, disposition, socket, map._headerSize, map._messageSize - map._headerSize, true, now);

#else // !MOBILEAPP
    Poco::Net::HTTPRequest request;

    // In the iOS app, the URL of the document is sent over the FakeSocket by the code in
    // -[DocumentViewController userContentController:didReceiveScriptMessage:] when it gets the
    // HULLO message from the JavaScript in global.js.

    // In other mobile apps and in CODA, it is done in some similar place.

    // It's currently relevant only for iOS, macOS, and Windows, so fallback if it is not found
    // (Android?).

    // Unwrap what StreamSocket::readIncomingData() did
    ssize_t len;
    memcpy(&len, socket->getInBuffer().data(), sizeof(ssize_t));
    const char* payload = socket->getInBuffer().data() + sizeof(ssize_t);
    auto const space = std::string_view(payload, len).find(' ');
    if (space != std::string_view::npos)
    {
        // The socket buffer is not nul-terminated so we can't just call strtoull() on the number at
        // its end, it might be followed in memory by more digits. Is there really no better way to
        // parse the number at the end of the buffer than to copy the bytes into a nul-terminated
        // buffer?
        const size_t appDocIdLen = len - (space + 1);
        char* appDocIdBuffer = (char*)malloc(appDocIdLen + 1);
        memcpy(appDocIdBuffer, payload + space + 1, appDocIdLen);
        appDocIdBuffer[appDocIdLen] = '\0';
        auto [mobileAppDocId, docIdOk] = NumUtil::u64FromString(appDocIdBuffer);
        if (!docIdOk)
        {
            mobileAppDocId = 0;
            LOG_ERR("Bad document ID \"" << appDocIdBuffer << "\" in \""
                                         << std::string_view(payload, len) << "\"");
        }
        free(appDocIdBuffer);

        handleClientWsUpgrade(request,
                              RequestDetails(std::string(payload, space)),
                              disposition, socket, mobileAppDocId);
    }
    else
    {
        // no appDocId provided
        handleClientWsUpgrade(
            request,
            RequestDetails(std::string(payload, len)),
            disposition, socket);
    }
    socket->getInBuffer().clear();
#endif // MOBILEAPP
}

ssize_t ClientRequestDispatcher::readHeader(const std::shared_ptr<StreamSocket>& socket,
                                            Poco::Net::HTTPRequest& request,
                                            std::chrono::duration<float, std::milli> delayMs)
{
    const std::string_view buffer = socket->getInBuffer().view();
    assert(_headerPos <= buffer.size() &&
           "Unexpected buffer shrunk under us — _headerPos is stale");
    _headerPos = _headerPos > buffer.size() ? 0 : _headerPos; // Recover in release builds.

    // Find the end of the header, if any.
    constexpr std::string_view marker("\r\n\r\n");
    const auto headerSize = buffer.find(marker, _headerPos);
    if (headerSize == std::string_view::npos)
    {
        // A partial marker could span the boundary, so back up by marker.size()-1.
        _headerPos = buffer.size() >= marker.size() ? buffer.size() - marker.size() + 1 : 0;
        LOG_TRC("parseHeader: doesn't have enough data for the header yet. delay "
                << delayMs.count() << "ms");
        return -1;
    }

    // We found the end-of-header marker; Clear to allow for scanning again.
    _headerPos = 0;

    constexpr std::chrono::duration<float, std::milli> DelayMax =
        std::chrono::duration_cast<std::chrono::milliseconds>(SocketPoll::DefaultPollTimeoutMicroS);
    const size_t messagesize = buffer.size();
    try
    {
        // Include the marker.
        Poco::MemoryInputStream startmessage(socket->getInBuffer().data(),
                                             headerSize + marker.size());
        request.read(startmessage);
    }
    catch (const Poco::Net::NotAuthenticatedException& exc)
    {
        LOG_DBG("parseHeader: Exception caught with "
                << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                << delayMs.count() << "ms");
        socket->asyncShutdown();
        return 0; //FIXME: Why not -1 as we've closed the socket already?
    }
    catch (const Poco::Net::UnsupportedRedirectException& exc)
    {
        LOG_DBG("parseHeader: Exception caught with "
                << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                << delayMs.count() << "ms");
        socket->asyncShutdown();
        return -1;
    }
    catch (const Poco::Net::HTTPException& exc)
    {
        LOG_DBG("parseHeader: Exception caught with "
                << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                << delayMs.count() << "ms");
        socket->asyncShutdown();
        return -1;
    }
    catch (const Poco::Exception& exc)
    {
        if (delayMs > DelayMax)
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                    << delayMs.count() << "ms");
            socket->asyncShutdown();
        }
        else
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, continue: " << exc.displayText() << ", delay "
                    << delayMs.count() << "ms");
        }
        return -1;
    }
    catch (const std::exception& exc)
    {
        if (delayMs > DelayMax)
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, shutdown: " << exc.what() << ", delay "
                    << delayMs.count() << "ms");
            socket->asyncShutdown();
        }
        else
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, continue: " << exc.what() << ", delay "
                    << delayMs.count() << "ms");
        }
        return -1;
    }

    return headerSize + marker.size();
}

namespace
{

#if !MOBILEAPP
bool allowedOriginByHost(const std::string& host, const std::string& actualOrigin)
{
    // always allow https host to match origin
    if (net::sameOrigin("https://" + host, actualOrigin))
        return true;
    // allow http too if not ssl enabled
    if (!ConfigUtil::isSslEnabled() && net::sameOrigin("http://" + host, actualOrigin))
        return true;
    return false;
}

template <typename T> bool allowedOrigin(const T& request, const RequestDetails& requestDetails)
{
    auto const it = request.find("Origin");
    if (it == request.end())
    {
        LOG_ERR("Rejecting message with no Origin header");
        return false;
    }

    const std::string actualOrigin = it->second;
    const ServerURL cnxDetails(requestDetails);

    if (net::sameOrigin(cnxDetails.getWebServerUrl(), actualOrigin))
    {
        LOG_TRC("Allowed Origin: " << actualOrigin << " to match " << cnxDetails.getWebServerUrl());
        return true;
    }

    if (COOLWSD::IndirectionServerEnabled && COOLWSD::GeolocationSetup)
    {
        if (HostUtil::allowedWSOrigin(actualOrigin))
        {
            LOG_TRC("Allowed Origin: " << actualOrigin << " to match AllowedWSOriginList");
            return true;
        }
    }

    const std::string host = request.get("Host");
    if (allowedOriginByHost(host, actualOrigin))
    {
        LOG_DBG("Allowed Origin: " << actualOrigin << " to match against host: " << host);
        return true;
    }
    if (host != COOLWSD::ServerName && !COOLWSD::ServerName.empty() &&
        allowedOriginByHost(COOLWSD::ServerName, actualOrigin))
    {
        LOG_DBG("Allowed Origin: " << actualOrigin << " to match ServerName: " << COOLWSD::ServerName);
        return true;
    }

    LOG_ERR("Rejecting origin [" << actualOrigin << "] expected [" << cnxDetails.getWebServerUrl() << "] instead");

    return false;
}
#else
template <typename T>
bool allowedOrigin([[maybe_unused]] const T& request,
                   [[maybe_unused]] const RequestDetails& requestDetails)
{
    return true;
}
#endif
}

#if !MOBILEAPP
void ClientRequestDispatcher::handleFullMessage(Poco::Net::HTTPRequest& request,
                                                std::istream& message,
                                                SocketDisposition& disposition,
                                                const std::shared_ptr<StreamSocket>& socket,
                                                size_t headerSize,
                                                size_t contentSize,
                                                bool eraseMessageFromSocket,
                                                std::chrono::steady_clock::time_point now)
{
    const size_t preInBufferSz = socket->getInBuffer().size();

    _lastSeenHTTPHeader = now;

    const bool closeConnection = !request.getKeepAlive(); // HTTP/1.1: closeConnection true w/ "Connection: close" only!
    LOG_DBG("Handling request: " << request.getURI() << ", closeConnection: " << closeConnection);

    ClientRequestDispatcher::MessageResult result = handleMessage(request, message, disposition, socket, headerSize);
    if (result == MessageResult::Ignore)
        return;

    assert(result == MessageResult::ServedSync || result == MessageResult::ServedAsync);
    bool servedSync = result == MessageResult::ServedSync;

    if (eraseMessageFromSocket)
        socketEraseConsumedBytes(socket, headerSize, contentSize, servedSync);

    finishedMessage(request, socket, servedSync, preInBufferSz);
}

ClientRequestDispatcher::MessageResult ClientRequestDispatcher::handleMessage(Poco::Net::HTTPRequest& request,
                                                                              std::istream& message,
                                                                              SocketDisposition& disposition,
                                                                              const std::shared_ptr<StreamSocket>& socket,
                                                                              size_t headerSize)
{
    const bool closeConnection = !request.getKeepAlive(); // HTTP/1.1: closeConnection true w/ "Connection: close" only!
    LOG_DBG("Handling request: " << request << ", closeConnection: " << closeConnection);

    // denotes whether the request has been served synchronously
    bool servedSync = false;

    try
    {
        // update the read cursor - headers are not altered by chunks.
        message.seekg(headerSize, std::ios::beg);

        // re-write ServiceRoot and cache.
        RequestDetails requestDetails(request, COOLWSD::ServiceRoot);
        // LOG_TRC("Request details " << requestDetails.toString());

        // Config & security ...
        if (requestDetails.isProxy())
        {
            if (!COOLWSD::IsProxyPrefixEnabled)
            {
                LOG_ERR("ProxyPrefix but not enabled");
                throw BadRequestException(
                    "ProxyPrefix present but net.proxy_prefix is not enabled");
            }
#if ENABLE_DEBUG
            bool isLocal = true;
#else
            bool isLocal = socket->isLocal();
#endif
            if (!isLocal)
            {
                LOG_ERR("ProxyPrefix request from non-local socket");
                throw BadRequestException("ProxyPrefix request from non-local socket");
            }
        }

        CleanupRequestVettingStations();

        // Routing
        const bool isUnitTesting = UnitWSD::isUnitTesting();
        bool handledByUnitTesting = false;
        if (isUnitTesting)
        {
            LOG_DBG("Unit-Test: handleHttpRequest: " << request.getURI());
            handledByUnitTesting = UnitWSD::get().handleHttpRequest(request, message, socket);
            if (!handledByUnitTesting)
            {
                LOG_DBG("Unit-Test: parallelizeCheckInfo: " << request.getURI());
                auto mapAccessDetails = UnitWSD::get().parallelizeCheckInfo(request, message, socket);
                if (!mapAccessDetails.empty())
                {
                    LOG_DBG("Unit-Test: launchAsyncCheckFileInfo: " << request.getURI());
                    auto accessDetails = FileServerRequestHandler::ResourceAccessDetails(
                        mapAccessDetails.at("wopiSrc"),
                        mapAccessDetails.at("accessToken"),
                        mapAccessDetails.at("noAuthHeader"),
                        mapAccessDetails.at("permission"),
                        mapAccessDetails.at("configid"));
                    launchAsyncCheckFileInfo(_id, accessDetails, RequestVettingStations,
                                             RvsHighWatermark);
                }
            }
        }

        if (handledByUnitTesting)
        {
            // Unit testing, nothing to do here
        }
        else if (requestDetails.equals(RequestDetails::Field::Type, "browser") ||
                 requestDetails.equals(RequestDetails::Field::Type, "wopi"))
        {
            // File server
            assert(socket && "Must have a valid socket");
            constexpr std::string_view ProxyRemoteStatic = "/remote/static/";
            const auto& uri = requestDetails.getURI();
            const auto pos = uri.find(ProxyRemoteStatic);
            if (pos != std::string::npos)
            {
#if ENABLE_FEATURE_LOCK
                const Poco::URI unlockImageUri =
                    CommandControl::LockManager::getUnlockImageUri();
                if (!unlockImageUri.empty())
                {
                    const std::string& serverUri =
                        unlockImageUri.getScheme() + "://" + unlockImageUri.getAuthority();
                    ProxyRequestHandler::handleRequest(
                        uri.substr(pos + sizeof("/remote/static") - 1), socket, serverUri);
                    servedSync = true;
                }
#endif
                if (!servedSync)
                    HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
            }
            else
            {
                FileServerRequestHandler::ResourceAccessDetails accessDetails;
                servedSync = COOLWSD::FileRequestHandler->handleRequest(
                    request, requestDetails, message, socket, accessDetails);
                if (accessDetails.isValid())
                {
                    LOG_ASSERT_MSG(
                        Uri::decode(requestDetails.getField(RequestDetails::Field::WOPISrc)) ==
                            Uri::decode(accessDetails.wopiSrc()),
                        "Expected identical WOPISrc in the request as in cool.html");

                    launchAsyncCheckFileInfo(_id, accessDetails, RequestVettingStations,
                                             RvsHighWatermark);
                }
            }
        }
        else if (requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 requestDetails.equals(1, "adminws"))
        {
            // Admin connections
            LOG_INF("Admin request: " << request.getURI());
            const bool allowed = allowedOrigin(request, requestDetails);
            if (allowed && AdminSocketHandler::handleInitialRequest(_socket, request, allowed))
            {
                // Hand the socket over to the Admin poll.
                disposition.setTransfer(Admin::instance(),
                                        [](const std::shared_ptr<Socket>& /*moveSocket*/) {});
            }
            else
            {
                if (!allowed)
                {
                    LOG_ERR(
                        "Rejecting admin WebSocket upgrade due to disallowed origin for request: "
                        << request);
                    HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
                }
                else
                {
                    LOG_ERR("Rejecting admin WebSocket upgrade due to bad/invalid request: "
                            << request);
                    HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
                }
            }
        }
        else if (requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 requestDetails.equals(1, "getMetrics"))
        {
            if (!COOLWSD::AdminEnabled)
                throw Poco::FileAccessDeniedException("Admin console disabled");

            // See metrics.txt
            std::shared_ptr<http::Response> response =
                std::make_shared<http::Response>(http::StatusCode::OK);

            try
            {
                /* WARNING: security point, we may skip authentication */
                bool skipAuthentication = ConfigUtil::getConfigValue<bool>(
                    "security.enable_metrics_unauthenticated", false);
                if (!skipAuthentication)
                    if (!FileServerRequestHandler::isAdminLoggedIn(request, *response))
                        throw Poco::Net::NotAuthenticatedException("Invalid admin login");
            }
            catch (const Poco::Net::NotAuthenticatedException& exc)
            {
                //LOG_ERR("FileServerRequestHandler::NotAuthenticated: " << exc.displayText());
                http::Response httpResponse(http::StatusCode::Unauthorized);
                httpResponse.set("Content-Type", "text/html charset=UTF-8");
                httpResponse.set("WWW-authenticate", "Basic realm=\"online\"");
                socket->sendAndShutdown(httpResponse);
                socket->ignoreInput();
                return MessageResult::Ignore;
            }

            FileServerRequestHandler::hstsHeaders(*response);
            response->add("Last-Modified", Util::getHttpTimeNow());
            // Ask UAs to block if they detect any XSS attempt
            response->add("X-XSS-Protection", "1; mode=block");
            // No referrer-policy
            response->add("Referrer-Policy", "no-referrer");
            response->add("X-Content-Type-Options", "nosniff");

            disposition.setTransfer(Admin::instance(),
                                    [response=std::move(response)](const std::shared_ptr<Socket>& moveSocket)
                                    {
                                        const std::shared_ptr<StreamSocket> streamSocket =
                                            std::static_pointer_cast<StreamSocket>(moveSocket);
                                        Admin::instance().sendMetrics(streamSocket, response);
                                    });
        }
        else if (requestDetails.isGetOrHead("/"))
            servedSync = handleRootRequest(requestDetails, socket);

        else if (requestDetails.isGet("/favicon.ico"))
            servedSync = handleFaviconRequest(requestDetails, socket);

        else if (requestDetails.equals(0, "hosting"))
        {
            if (requestDetails.equals(1, "discovery"))
                servedSync = handleWopiDiscoveryRequest(requestDetails, socket);
            else if (requestDetails.equals(1, "capabilities"))
                servedSync = handleCapabilitiesRequest(request, socket);
            else if (requestDetails.equals(1, "wopiAccessCheck"))
            {
                const std::string text(std::istreambuf_iterator<char>(message), {});
                handleWopiAccessCheckRequest(request, text, socket);
            }
            else
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        }
        else if (requestDetails.isGet("/robots.txt"))
            servedSync = handleRobotsTxtRequest(request, socket);

        else if (requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 requestDetails.equals(1, "media"))
            servedSync = handleMediaRequest(request, disposition, socket, false);

        else if (requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 requestDetails.equals(1, "mediaVTT"))
            servedSync = handleMediaRequest(request, disposition, socket, true);

        else if (requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 requestDetails.equals(1, "clipboard"))
        {
            servedSync = handleClipboardRequest(request, message, disposition, socket);
        }
        else if (requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 requestDetails.equals(1, "signature"))
        {
            servedSync = handleSignatureRequest(request, socket);
        }

        else if (requestDetails.isProxy() && requestDetails.equals(2, "ws"))
            servedSync = handleClientProxyRequest(request, requestDetails, message, disposition);
        else if (requestDetails.isWebSocket() &&
                 requestDetails.equals(RequestDetails::Field::Type, "cool") &&
                 (requestDetails.equals(1, "ws") || requestDetails.equals(2, "ws")))
        {
            // The new WebSocket URL has 'ws' as the second segment; support both old and new.
            servedSync = handleClientWsUpgrade(request, requestDetails, disposition, socket);
        }

        else if (!requestDetails.isWebSocket() &&
                 (requestDetails.equals(RequestDetails::Field::Type, "cool") ||
                  requestDetails.equals(RequestDetails::Field::Type, "lool")))
        {
            // All post requests have url prefix 'cool', except when the prefix
            // is 'lool' e.g. when integrations use the old /lool/convert-to endpoint
            servedSync = handlePostRequest(requestDetails, request, message, disposition, socket);
        }
        else if (requestDetails.equals(RequestDetails::Field::Type, "wasm"))
        {
            if (COOLWSD::WASMState == COOLWSD::WASMActivationState::Disabled)
            {
                LOG_ERR(
                    "WASM document request while WASM is disabled: " << requestDetails.toString());

                // Bad request.
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
                return MessageResult::Ignore;
            }

            // Tunnel to WASM.
            _wopiProxy = std::make_unique<WopiProxy>(_id, requestDetails, socket);
            _wopiProxy->handleRequest(message, COOLWSD::getWebServerPoll(), disposition);
        }
        else
        {
            LOG_WRN("Unknown resource: " << requestDetails.toString());

            // Bad request.
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
            return MessageResult::Ignore;
        }
    }
    catch (const BadRequestException& exc)
    {
        LOG_ERR("Bad request: " << request << ", closeConnection: " << closeConnection
                                << ", socket-data: ["
                                << COOLProtocol::getAbbreviatedMessage(socket->getInBuffer())
                                << "]: " << exc.what());

        // Bad request.
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return MessageResult::Ignore;
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception while processing incoming request: "
                << request << ", closeConnection: " << closeConnection << ", socket-data: ["
                << COOLProtocol::getAbbreviatedMessage(socket->getInBuffer())
                << "]: " << exc.what());

        // Bad request.
        // NOTE: Check _wsState to choose between HTTP response or WebSocket (app-level) error.
        http::Response httpResponse(http::StatusCode::BadRequest);
        httpResponse.setContentLength(0);
        socket->sendAndShutdown(httpResponse);
        socket->ignoreInput();
        return MessageResult::Ignore;
    }

    return servedSync ? MessageResult::ServedSync : MessageResult::ServedAsync;
}

void ClientRequestDispatcher::finishedMessage(const Poco::Net::HTTPRequest& request,
                                              const std::shared_ptr<StreamSocket>& socket,
                                              bool servedSync, size_t preInBufferSz)
{
    const bool closeConnection = !request.getKeepAlive();

    if (servedSync && closeConnection && !socket->isShutdown())
    {
        LOG_DBG("Handled request: " << request.getURI()
                << ", inBuf[sz " << preInBufferSz << " -> " << socket->getInBuffer().size()
                << ", rm " <<  (preInBufferSz-socket->getInBuffer().size())
                << "], served and closing connection.");
        socket->asyncShutdown();
        socket->ignoreInput();
    }
    else
        LOG_DBG("Handled request: " << request.getURI()
                << ", inBuf[sz " << preInBufferSz << " -> " << socket->getInBuffer().size()
                << ", rm " <<  (preInBufferSz-socket->getInBuffer().size())
                << "], connection open " << !socket->isShutdown());
}

bool ClientRequestDispatcher::handleRootRequest(const RequestDetails& requestDetails,
                                                const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG("HTTP request: " << requestDetails.getURI());
    const std::string responseString = "OK";

    http::Response httpResponse(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(httpResponse);
    httpResponse.setContentLength(responseString.size());
    httpResponse.set("Content-Type", "text/plain");
    httpResponse.set("Last-Modified", Util::getHttpTimeNow());
    if( requestDetails.closeConnection() )
        httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
    httpResponse.writeData(socket->getOutBuffer());
    if (requestDetails.isGet())
        socket->send(responseString);
    if (socket->attemptWrites())
        LOG_INF("Sent / response successfully");
    else
        LOG_INF("Sent / response partially");
    return true;
}

bool ClientRequestDispatcher::handleFaviconRequest(const RequestDetails& requestDetails,
                                                   const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_TRC_S("Favicon request: " << requestDetails.getURI());
    http::Response response(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(response);
    response.setContentType("image/vnd.microsoft.icon");
    if( requestDetails.closeConnection() )
        response.setConnectionToken(http::Header::ConnectionToken::Close);
    std::string faviconPath =
        Poco::Path(Poco::Util::Application::instance().commandPath()).parent().toString() +
        "favicon.ico";
    if (!Poco::File(faviconPath).exists())
        faviconPath = COOLWSD::FileServerRoot + "/favicon.ico";

    HttpHelper::sendFile(socket, faviconPath, response);
    return true;
}

bool ClientRequestDispatcher::handleWopiDiscoveryRequest(
    const RequestDetails& requestDetails, const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG("Wopi discovery request: " << requestDetails.getURI());

    std::string xml = getFileContent("discovery.xml");
    bool isSsl =
#if ENABLE_SSL
        (ConfigUtil::isSslEnabled() || ConfigUtil::isSSLTermination());
#else
        false;
#endif
    std::string srvUrl = (isSsl ? "https://" : "http://")
        + (COOLWSD::ServerName.empty() ? requestDetails.getHostUntrusted() : COOLWSD::ServerName) +
        COOLWSD::ServiceRoot;
    if (requestDetails.isProxy())
        srvUrl = requestDetails.getProxyPrefix();
    Poco::replaceInPlace(xml, std::string("%SRV_URI%"), srvUrl);
    Poco::replaceInPlace(xml, std::string("%SRV_PROTO%"), std::string(isSsl ? "https" : "http"));

    http::Response httpResponse(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(httpResponse);
    httpResponse.setBody(xml, "text/xml");
    httpResponse.set("Last-Modified", Util::getHttpTimeNow());
    httpResponse.set("X-Content-Type-Options", "nosniff");
    if( requestDetails.closeConnection() )
        httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
    LOG_TRC("Sending back discovery.xml: " << xml);
    socket->send(httpResponse);
    LOG_INF("Sent discovery.xml successfully.");
    return true;
}

//static
void ClientRequestDispatcher::sendResult(const std::shared_ptr<StreamSocket>& socket, CheckStatus result)
{
    std::string output = R"({"status": ")" + JsonUtil::escapeJSONValue(nameShort(result)) + "\"}\n";

    http::Response jsonResponse(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(jsonResponse);
    jsonResponse.set("Last-Modified", Util::getHttpTimeNow());
    jsonResponse.setBody(std::move(output), "application/json");
    jsonResponse.set("X-Content-Type-Options", "nosniff");

    socket->sendAndShutdown(jsonResponse);
    LOG_INF_S("Wopi Access Check request, result: " << nameShort(result));
}

bool ClientRequestDispatcher::handleWopiAccessCheckRequest(
    const Poco::Net::HTTPRequest& request, const std::string& text,
    const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG("Wopi Access Check request: " << request.getURI());

    LOG_TRC("Wopi Access Check request text: " << text);

    std::string callbackUrlStr;

    Poco::JSON::Object::Ptr jsonObject;
    if (!JsonUtil::parseJSON(text, jsonObject))
    {
        LOG_WRN("Wopi Access Check request error, json object expected got ["
                << text << "] on request to URL: " << request.getURI());

        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return false;
    }

    if (!JsonUtil::findJSONValue(jsonObject, "callbackUrl", callbackUrlStr))
    {
        LOG_WRN("Wopi Access Check request error, missing key callbackUrl expected got ["
                << text << "] on request to URL: " << request.getURI());

        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return false;
    }

    LOG_TRC("Wopi Access Check request callbackUrlStr: " << callbackUrlStr);

    std::string scheme, host, portStr, pathAndQuery;
    if (!net::parseUri(callbackUrlStr, scheme, host, portStr, pathAndQuery)) {
        LOG_WRN("Wopi Access Check request error, invalid url ["
                << callbackUrlStr << "] on request to URL: " << request.getURI() << scheme);

        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return false;
    }

    http::Session::Protocol protocol = http::Session::Protocol::HttpSsl;
    unsigned long port = 443;
    if (scheme == "https://" || scheme.empty()) {
        // empty scheme assumes https
    } else if (scheme == "http://") {
        protocol = http::Session::Protocol::HttpUnencrypted;
        port = 80;
    } else {
        LOG_WRN("Wopi Access Check request error, bad request protocol ["
                << text << "] on request to URL: " << request.getURI() << scheme);

        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return false;
    }

    if (!portStr.empty()) {
        try {
            port = std::stoul(portStr);

        } catch(std::invalid_argument &exception) {
            LOG_WRN("Wopi Access Check error parsing invalid_argument portStr:" << portStr);
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
            return false;
        } catch(std::exception &exception) {
            LOG_WRN("Wopi Access Check request error, bad request invalid porl ["
                    << text << "] on request to URL: " << request.getURI());

            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
            return false;
        }
    }

    LOG_TRC("Wopi Access Check request scheme: " << scheme << " " << port);

    if (scheme.empty())
    {
        sendResult(socket, CheckStatus::NoScheme);
        return true;
    }
    // if the wopi hosts uses https, so must cool or it will have Mixed Content errors
    if (protocol == http::Session::Protocol::HttpSsl &&
#if ENABLE_SSL
        !(ConfigUtil::isSslEnabled() || ConfigUtil::isSSLTermination())
#else
        false
#endif
    )
    {
        sendResult(socket, CheckStatus::NotHttps);
        return true;
    }

    if (HostUtil::isWopiHostsEmpty())
        // make sure the wopi hosts settings are loaded
        StorageBase::initialize();

    bool wopiHostAllowed = false;
    if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"), "first"))
        // if first mode was selected and wopi Hosts are empty
        // the domain is allowed, as it will be the effective "first" host
        wopiHostAllowed = HostUtil::isWopiHostsEmpty();

    if (!wopiHostAllowed) {
        // port and scheme from wopi host config are currently ignored by HostUtil
        LOG_TRC("Wopi Access Check, matching allowed wopi host for host " << host);
        wopiHostAllowed = HostUtil::allowedWopiHost(host);
    }
    if (!wopiHostAllowed)
    {
        LOG_TRC("Wopi Access Check, wopi host not allowed " << host);
        sendResult(socket, CheckStatus::WopiHostNotAllowed);
        return true;
    }

    http::Request httpRequest(pathAndQuery.empty() ? "/" : pathAndQuery);
    auto httpProbeSession = http::Session::create(std::move(host), protocol, port);
    httpProbeSession->setTimeout(std::chrono::seconds(2));

    std::weak_ptr<StreamSocket> socketWeak(socket);
    const std::string logPfx = getLogPrefix();

    httpProbeSession->setConnectFailHandler(
        [socketWeak, callbackUrlStr, logPfx](const std::shared_ptr<http::Session>& probeSession)
        {
            CheckStatus status = CheckStatus::UnspecifiedError;

            const auto result = probeSession->connectionResult();

            if (result == net::AsyncConnectResult::UnknownHostError || result == net::AsyncConnectResult::HostNameError)
            {
                status = CheckStatus::HostNotFound;
            }

#if ENABLE_SSL
            if (result == net::AsyncConnectResult::SSLHandShakeFailure) {
                status = CheckStatus::SslHandshakeFail;
            }

            auto sslResult = probeSession->getSslVerifyResult();
            if (sslResult != X509_V_OK)
            {
                if (sslResult == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
                    status = CheckStatus::SelfSignedCertificate;
                } else if (sslResult == X509_V_ERR_CERT_HAS_EXPIRED) {
                    status = CheckStatus::ExpiredCertificate;
                } else {
                    status = CheckStatus::CertificateValidation;
                    LOG_DBG_S(logPfx << "Result ssl: " << probeSession->getSslVerifyMessage());
                }
            }
#endif

            std::shared_ptr<StreamSocket> destSocket = socketWeak.lock();
            if (!destSocket)
            {
                LOG_ERR_S(logPfx << "Invalid socket while sending wopi access check result for: "
                        << callbackUrlStr);
                return;
            }
            sendResult(destSocket, status);
    });

    auto finishHandler = [socketWeak, callbackUrlStr = std::move(callbackUrlStr), logPfx]
                          (const std::shared_ptr<http::Session>& probeSession)
    {
        LOG_TRC_S(logPfx << "finishHandler ");

        const auto lastErrno = errno;

        const std::shared_ptr<http::Response> httpResponse = probeSession->response();
        const http::Response::State responseState = httpResponse->state();
        const http::StatusCode statusCode = httpResponse->statusCode();
        LOG_DBG_S(logPfx << "Wopi Access Check: got response state: " << responseState << " "
                                            << ", response status code: " << statusCode << " "
                                            << ", last errno: " << lastErrno);

        CheckStatus status = statusCode == http::StatusCode::OK ? CheckStatus::Ok: CheckStatus::NotHttpSuccess;

        if (responseState != http::Response::State::Complete)
        {
            // are TLS errors here ?
            status = CheckStatus::UnspecifiedError;
        }

        if (responseState == http::Response::State::Timeout)
            status = CheckStatus::Timeout;


        const auto result = probeSession->connectionResult();

        if (result == net::AsyncConnectResult::UnknownHostError)
            status = CheckStatus::HostNotFound;

        if (result == net::AsyncConnectResult::ConnectionError)
            status = CheckStatus::ConnectionAborted;

#if ENABLE_SSL
        auto sslResult = probeSession->getSslVerifyResult();
        if (sslResult != X509_V_OK)
        {
            if (sslResult == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
                // means we aren't checking certificate or we'd have a connectionFail
                status = CheckStatus::Ok;
            } else {
                status = CheckStatus::CertificateValidation;
                LOG_WRN_S(logPfx << "Unexpected failed Result ssl in a connection success: " << probeSession->getSslVerifyMessage());
            }
        }
#endif

        std::shared_ptr<StreamSocket> destSocket = socketWeak.lock();
        if (!destSocket)
        {
            LOG_ERR_S(logPfx
                << "Invalid socket while sending wopi access check result for: " << callbackUrlStr);
            return;
        }
        sendResult(destSocket, status);
    };

    httpProbeSession->setFinishedHandler(std::move(finishHandler));
    httpProbeSession->asyncRequest(httpRequest, COOLWSD::getWebServerPoll());

    return true;
}

bool ClientRequestDispatcher::handleClipboardRequest(const Poco::Net::HTTPRequest& request,
                                                     std::istream& message,
                                                     SocketDisposition& disposition,
                                                     const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG_S(
        "Clipboard " << ((request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) ? "GET" : "POST")
                     << " request: " << request.getURI());

    Poco::URI requestUri(request.getURI());
    Poco::URI::QueryParameters params = requestUri.getQueryParameters();
    std::string WOPISrc, serverId, viewId, tag, mime, charset;

    for (const auto& it : params)
    {
        if (it.first == "WOPISrc")
            WOPISrc = it.second;
        else if (it.first == "ServerId")
            serverId = it.second;
        else if (it.first == "ViewId")
            viewId = it.second;
        else if (it.first == "Tag")
            tag = it.second;
        else if (it.first == "MimeType")
            mime = it.second;
        else if (it.first == "charset")
            charset = it.second;
    }

    if (!charset.empty())
        mime += ";charset=" + charset;

    if (serverId != Util::getProcessIdentifier())
    {
        LOG_ERR_S("Cluster configuration error: mis-matching serverid ["
                  << serverId << "] vs. [" << Util::getProcessIdentifier() << "] with tag [" << tag
                  << "] on request to URL: " << request.getURI());

        // we got the wrong request.
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket, "wrong server, this is likely an issue with your reverse proxy configuration, See https://sdk.collaboraonline.com/docs/installation/Proxy_settings.html");
        return true;
    }

    // Verify that the WOPISrc is properly encoded.
    if (!HttpHelper::verifyWOPISrc(request.getURI(), WOPISrc, socket))
    {
        return false;
    }

    const auto docKey = RequestDetails::getDocKey(WOPISrc);
    LOG_TRC_S("Clipboard request for us: [" << serverId << "] with tag [" << tag << "] on docKey ["
                                            << docKey << ']');

    std::shared_ptr<DocumentBroker> docBroker;
    {
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        auto it = DocBrokers.find(docKey);
        if (it != DocBrokers.end())
            docBroker = it->second;
    }

    DocumentBroker::ClipboardRequest type;
    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_GET)
        type = DocumentBroker::CLIP_REQUEST_SET;
    else
    {
        if (mime == "text/html")
            type = DocumentBroker::CLIP_REQUEST_GET_RICH_HTML_ONLY;
        else if (mime == "text/html,text/plain;charset=utf-8")
            type = DocumentBroker::CLIP_REQUEST_GET_HTML_PLAIN_ONLY;
        else
            type = DocumentBroker::CLIP_REQUEST_GET;
    }

    // If we have a valid docBroker, use it.
    // Note: there is a race here as DocBroker may
    // have already exited its SocketPoll, but we
    // haven't cleaned up the DocBrokers container.
    // Since we don't care about creating a new one,
    // we simply go to the fallback below.
    if (docBroker && docBroker->isAlive())
    {
        std::string jailClipFile, clipFile;
        if (type == DocumentBroker::CLIP_REQUEST_SET)
        {
            if (!docBroker->getSessionFromClipboardTag(viewId, tag))
            {
                LOG_ERR_S("Unknown tag [" << tag << "] for view [" << viewId << "] on request to URL: " << request.getURI());
                // we got the wrong tag.
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket, "wrong tag");
                return true;
            }

            std::string clipName = "setclipboard." + tag;

            std::string jailId = docBroker->getJailId();

            auto [clipDir, jailDir] = FileUtil::buildPathsToJail(COOLWSD::EnableMountNamespaces, COOLWSD::NoCapsForKit,
                                                                 COOLWSD::ChildRoot + jailId,
                                                                 JAILED_DOCUMENT_ROOT + std::string("clipboards"));

            clipFile = clipDir + '/' + clipName;
            jailClipFile = jailDir + '/' + clipName;

            ClipboardPartHandler handler(clipFile);
            Poco::Net::HTMLForm form(request, message, handler);
            if (FileUtil::Stat(clipFile).size())
                handler.takeFile();
            else
            {
                LOG_ERR_S("Invalid zero size set clipboard content with tag ["
                          << tag << "] on docKey [" << docKey << ']');
                clipFile.clear();
                jailClipFile.clear();
            }
        }

        // Do things in the right thread.
        LOG_TRC_S("Move clipboard request tag [" << tag << "] to docbroker thread with "
                                                 << (!clipFile.empty() ? FileUtil::Stat(clipFile).size() : 0)
                                                 << " bytes of data");
        docBroker->setupTransfer(
            disposition,
            [docBroker, type, viewId=std::move(viewId),
             tag=std::move(tag), jailClipFile=std::move(jailClipFile)](const std::shared_ptr<Socket>& moveSocket)
            {
                auto streamSocket = std::static_pointer_cast<StreamSocket>(moveSocket);
                docBroker->handleClipboardRequest(type, streamSocket, viewId, tag, jailClipFile);
            });
        LOG_TRC_S("queued clipboard command " << type << " on docBroker fetch");
    }
    // fallback to persistent clipboards if we can
    else if (!DocumentBroker::handlePersistentClipboardRequest(type, socket, tag, false))
    {
        LOG_ERR_S("Invalid clipboard request to server ["
                  << serverId << "] with tag [" << tag << "] and broker [" << docKey
                  << "]: " << (docBroker ? "" : "not ") << "found");

        std::string errMsg = "Empty clipboard item / session tag " + tag;

        // Bad request.
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket, errMsg);
        return true;
    }
    return false;
}

namespace
{
bool handleStaticRequest(const Poco::Net::HTTPRequest& request,
                         const std::shared_ptr<StreamSocket>& socket,
                         const std::string& responseString,
                         const std::string& contentType)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG_S("HTTP request: " << request.getURI());

    http::Response httpResponse(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(httpResponse);
    httpResponse.set("Last-Modified", Util::getHttpTimeNow());
    httpResponse.setContentLength(responseString.size());
    httpResponse.set("Content-Type", contentType);
    if( !request.getKeepAlive() )
        httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
    httpResponse.writeData(socket->getOutBuffer());

    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
    {
        socket->send(responseString);
    }

    if (socket->attemptWrites())
        LOG_INF("Sent static response successfully");
    else
        LOG_INF("Sent static response partially");
    return true;
}
}

bool ClientRequestDispatcher::handleSignatureRequest(const Poco::Net::HTTPRequest& request,
                                                     const std::shared_ptr<StreamSocket>& socket)
{
    const std::string responseString = R"html(
<!doctype html>
<html>
    <head>
        <script type="text/javascript">
            document.addEventListener("DOMContentLoaded", function() {
                window.opener.postMessage({
                    sender: 'EIDEASY_SINGLE_METHOD_SIGNATURE',
                    type: 'SUCCESS',
                });
            });
        </script>
    </head>
    <body>
    </body>
</html>
)html";
    const std::string contentType = "text/html";
    return handleStaticRequest(request, socket, responseString, contentType);
}

bool ClientRequestDispatcher::handleRobotsTxtRequest(const Poco::Net::HTTPRequest& request,
                                                     const std::shared_ptr<StreamSocket>& socket)
{
    const std::string responseString = "User-agent: *\nDisallow: /\n";
    const std::string contentType = "text/plain";
    return handleStaticRequest(request, socket, responseString, contentType);
}

bool ClientRequestDispatcher::handleMediaRequest(const Poco::Net::HTTPRequest& request,
                                                 SocketDisposition& /*disposition*/,
                                                 const std::shared_ptr<StreamSocket>& socket,
                                                 bool bVTT)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG_S("Media request: " << request.getURI());

    const std::string decoded = Uri::decode(request.getURI());
    Poco::URI requestUri(decoded);
    Poco::URI::QueryParameters params = requestUri.getQueryParameters();
    std::string WOPISrc, serverId, viewId, tag, mime;
    for (const auto& it : params)
    {
        if (it.first == "WOPISrc")
            WOPISrc = it.second;
        else if (it.first == "ServerId")
            serverId = it.second;
        else if (it.first == "ViewId")
            viewId = it.second;
        else if (it.first == "Tag")
            tag = it.second;
        else if (it.first == "MimeType")
            mime = it.second;
    }

    LOG_TRC_S("Media request for us: [" << serverId << "] with tag [" << tag << "] and viewId ["
                                        << viewId << ']');

    if (serverId != Util::getProcessIdentifier())
    {
        LOG_ERR_S("Cluster configuration error: mis-matching serverid ["
                  << serverId << "] vs. [" << Util::getProcessIdentifier()
                  << "] on request to URL: " << request.getURI());

        // we got the wrong request.
        http::Response httpResponse(http::StatusCode::BadRequest);
        httpResponse.setContentLength(0);
        socket->sendAndShutdown(httpResponse);
        socket->ignoreInput();
        return true;
    }

    // Verify that the WOPISrc is properly encoded.
    if (!HttpHelper::verifyWOPISrc(request.getURI(), WOPISrc, socket))
    {
        return false;
    }

    const auto docKey = RequestDetails::getDocKey(WOPISrc);
    LOG_TRC_S("Looking up DocBroker with docKey [" << docKey << "] referenced in WOPISrc ["
                                                   << WOPISrc
                                                   << "] in media URL: " + request.getURI());

    std::shared_ptr<DocumentBroker> docBroker;
    {
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        auto it = DocBrokers.find(docKey);
        if (it == DocBrokers.end())
        {
            LOG_ERR_S("Unknown DocBroker with docKey [" << docKey << "] referenced in WOPISrc ["
                                                        << WOPISrc
                                                        << "] in media URL: " + request.getURI());

            http::Response httpResponse(http::StatusCode::BadRequest);
            httpResponse.setContentLength(0);
            socket->sendAndShutdown(httpResponse);
            socket->ignoreInput();
            return true;
        }

        docBroker = it->second;
    }

    // If we have a valid docBroker, use it.
    // Note: there is a race here as DocBroker may
    // have already exited its SocketPoll, but we
    // haven't cleaned up the DocBrokers container.
    // Since we don't care about creating a new one,
    // we simply go to the fallback below.
    if (docBroker && docBroker->isAlive())
    {
        // Do things in the right thread.
        LOG_TRC_S("Move media request " << tag << " to docbroker thread");

        std::string range = request.get("Range", "none");
        docBroker->handleMediaRequest(std::move(range), socket, tag, (bVTT ? std::string(MEDIA_STR)
                                                                      : std::string(MEDIA_MP4)));
    }
    return false; // async
}

bool ClientRequestDispatcher::handlePostRequest(const RequestDetails& requestDetails,
                                                const Poco::Net::HTTPRequest& request,
                                                std::istream& message,
                                                SocketDisposition& disposition,
                                                const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_INF("Post request: [" << COOLWSD::anonymizeUrl(requestDetails.getURI()) << ']');

    if (requestDetails.equals(1, "convert-to") ||
        requestDetails.equals(1, "extract-link-targets") ||
        requestDetails.equals(1, "extract-document-structure") ||
        requestDetails.equals(1, "transform-document-structure") ||
        requestDetails.equals(1, "get-thumbnail"))
    {
        // Validate sender - FIXME: should do this even earlier.
        if (!allowConvertTo(socket->clientAddress(), request, false, nullptr))
        {
            LOG_WRN(
                "Conversion requests not allowed from this address: " << socket->clientAddress());
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
            return true;
        }

        ConvertToPartHandler handler;
        Poco::Net::HTMLForm form(request, message, handler);

        std::string format = (form.has("format") ? form.get("format") : "");
        // prefer what is in the URI
        if (requestDetails.size() > 2)
            format = requestDetails[2];

        bool hasRequiredParameters = true;
        if (requestDetails.equals(1, "convert-to") && format.empty())
            hasRequiredParameters = false;

        const AdditionalFilePaths& fromPaths = handler.getFilenames();
        std::string fromPath;
        auto it = fromPaths.find("data");
        if (it != fromPaths.end())
        {
            fromPath = it->second;
        }
        if (fromPath.empty() && fromPaths.size() == 1)
        {
            // Compatibility: if there is a single stream, then allow any name and assume 'data'.
            it = fromPaths.begin();
            fromPath = it->second;
        }
        LOG_INF("Conversion request for URI [" << fromPath << "] format [" << format << "].");
        if (!fromPath.empty() && hasRequiredParameters)
        {
            Poco::URI uriPublic = RequestDetails::sanitizeLocalPath(fromPath);
            AdditionalFilePocoUris additionalFileUrisPublic;
            for (const auto& key : {"template", "compare"})
            {
                it = fromPaths.find(key);
                if (it == fromPaths.end())
                {
                    continue;
                }

                additionalFileUrisPublic[key] = RequestDetails::sanitizeLocalPath(it->second);
            }
            const std::string docKey = RequestDetails::getDocKey(uriPublic);

            std::string options;
            if (form.has("options"))
            {
                // Allow specifying options as-is, in case only data + format are used.
                options = form.get("options");
            }

            const bool fullSheetPreview =
                (form.has("FullSheetPreview") && form.get("FullSheetPreview") == "true");
            if (fullSheetPreview && format == "pdf" && ContentType::isSpreadsheet(fromPath))
            {
                //FIXME: We shouldn't have "true" as having the option already implies that
                // we want it enabled (i.e. we shouldn't set the option if we don't want it).
                options = ",FullSheetPreview=trueFULLSHEETPREVEND";
            }

            const std::string pdfVer = (form.has("PDFVer") ? form.get("PDFVer") : std::string());
            if (!pdfVer.empty())
            {
                if (strcasecmp(pdfVer.c_str(), "PDF/A-1b") &&
                    strcasecmp(pdfVer.c_str(), "PDF/A-2b") &&
                    strcasecmp(pdfVer.c_str(), "PDF/A-3b") &&
                    strcasecmp(pdfVer.c_str(), "PDF/A-4") &&
                    strcasecmp(pdfVer.c_str(), "PDF-1.5") &&
                    strcasecmp(pdfVer.c_str(), "PDF-1.6") &&
                    strcasecmp(pdfVer.c_str(), "PDF-1.7") &&
                    strcasecmp(pdfVer.c_str(), "PDF-2.0"))
                {
                    LOG_ERR("Wrong PDF type: " << pdfVer << ". Conversion aborted.");
                    http::Response httpResponse(http::StatusCode::BadRequest);
                    httpResponse.setContentLength(0);
                    socket->sendAndShutdown(httpResponse);
                    socket->ignoreInput();
                    return true;
                }
                options += ",PDFVer=" + pdfVer + "PDFVEREND";
            }

            if (form.has("infilterOptions"))
            {
                options += ",infilterOptions=" + form.get("infilterOptions");
            }

            const std::string lang = (form.has("lang") ? form.get("lang") : std::string());
            const std::string target = (form.has("target") ? form.get("target") : std::string());
            const std::string filter = (form.has("filter") ? form.get("filter") : std::string());

            std::string encodedTransformJSON;
            if (form.has("transform"))
            {
                std::string transformJSON = form.get("transform");
                Poco::URI::encode(transformJSON, "", encodedTransformJSON);
            }

            // This lock could become a bottleneck.
            // In that case, we can use a pool and index by publicPath.
            std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);

            LOG_DBG("New DocumentBroker for docKey [" << docKey << "].");
            auto docBroker = getConvertToBrokerImplementation(
                requestDetails[1], fromPath, uriPublic, docKey, format, options, lang, target,
                filter, encodedTransformJSON);
            handler.takeFiles();

            COOLWSD::cleanupDocBrokers();

            DocBrokers.emplace(docKey, docBroker);
            LOG_TRC("Have " << DocBrokers.size() << " DocBrokers after inserting [" << docKey
                            << "].");

            if (!docBroker->startConversion(disposition, _id, additionalFileUrisPublic))
            {
                LOG_WRN("Failed to create Client Session with id [" << _id << "] on docKey ["
                                                                    << docKey << "].");
                COOLWSD::cleanupDocBrokers();
            }
        }
        else
        {
            LOG_INF("Missing parameters for conversion request.");
            http::Response httpResponse(http::StatusCode::BadRequest);
            httpResponse.setContentLength(0);
            socket->sendAndShutdown(httpResponse);
            socket->ignoreInput();
            return true;
        }
        return false;
    }

    if (requestDetails.equals(2, "insertfile"))
    {
        LOG_INF("Insert file request.");

        ConvertToPartHandler handler;
        Poco::Net::HTMLForm form(request, message, handler);

        if (form.has("childid") && form.has("name"))
        {
            const std::string formChildid(form.get("childid"));
            const std::string formName(form.get("name"));

            // Validate the docKey
            const std::string decodedUri = requestDetails.getDocumentURI();
            const std::string docKey = RequestDetails::getDocKey(decodedUri);

            std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
            auto docBrokerIt = DocBrokers.find(docKey);

            // Maybe just free the client from sending childid in form ?
            if (docBrokerIt == DocBrokers.end() || docBrokerIt->second->getJailId() != formChildid)
            {
                throw BadRequestException("DocKey [" + docKey + "] or childid [" + formChildid +
                                          "] is invalid.");
            }
            docBrokersLock.unlock();

            // protect against attempts to inject something funny here
            if (formChildid.find('/') == std::string::npos &&
                formName.find('/') == std::string::npos)
            {
                const std::string dirPath = FileUtil::buildLocalPathToJail(
                    COOLWSD::EnableMountNamespaces, COOLWSD::ChildRoot + formChildid,
                    JAILED_DOCUMENT_ROOT + std::string("insertfile"));
                const std::string fileName = dirPath + '/' + form.get("name");
                LOG_INF("Perform insertfile: " << formChildid << ", " << formName
                                               << ", filename: " << fileName);
                Poco::File(dirPath).createDirectories();
                std::string filename;
                const std::map<std::string, std::string>& filenames = handler.getFilenames();
                if (!filenames.empty())
                {
                    // Expect a single parameter, don't care about the name.
                    auto it = filenames.begin();
                    filename = it->second;
                }
                Poco::File(filename).moveTo(fileName);

                // Cleanup the directory after moving.
                const std::string dir = Poco::Path(filename).parent().toString();
                if (FileUtil::isEmptyDirectory(dir))
                    FileUtil::removeFile(dir);

                handler.takeFiles();

                http::Response httpResponse(http::StatusCode::OK);
                FileServerRequestHandler::hstsHeaders(httpResponse);
                httpResponse.setContentLength(0);
                socket->sendAndShutdown(httpResponse);
                socket->ignoreInput();
                return true;
            }
        }
    }
    else if (requestDetails.equals(2, "download"))
    {
        LOG_INF("File download request.");
        // TODO: Check that the user in question has access to this file!

        // 1. Validate the dockey
        const std::string decodedUri = requestDetails.getDocumentURI();
        const std::string docKey = RequestDetails::getDocKey(decodedUri);

        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        auto docBrokerIt = DocBrokers.find(docKey);
        if (docBrokerIt == DocBrokers.end())
        {
            throw BadRequestException("DocKey [" + docKey + "] is invalid.");
        }

        std::string downloadId = requestDetails[3];
        std::string url = docBrokerIt->second->getDownloadURL(downloadId);
        docBrokerIt->second->unregisterDownloadId(downloadId);
        std::string jailId = docBrokerIt->second->getJailId();

        docBrokersLock.unlock();

        bool foundDownloadId = !url.empty();

        const std::string decoded = Uri::decode(url);

        const Poco::Path filePath(FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces,
                                                                 COOLWSD::ChildRoot + jailId,
                                                                 JAILED_DOCUMENT_ROOT + decoded));
        const std::string filePathAnonym = COOLWSD::anonymizeUrl(filePath.toString());

        if (foundDownloadId && filePath.isAbsolute() && Poco::File(filePath).exists())
        {
            LOG_INF("HTTP request for: " << filePathAnonym);

            const std::string& fileName = filePath.getFileName();
            const Poco::URI postRequestUri(request.getURI());
            const Poco::URI::QueryParameters postRequestQueryParams =
                postRequestUri.getQueryParameters();

            bool serveAsAttachment = true;
            const auto attachmentIt =
                std::find_if(postRequestQueryParams.begin(), postRequestQueryParams.end(),
                             [](const std::pair<std::string, std::string>& element)
                             { return element.first == "attachment"; });
            if (attachmentIt != postRequestQueryParams.end())
                serveAsAttachment = attachmentIt->second != "0";

            http::Response response(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(response);

            // Instruct browsers to download the file, not display it
            // with the exception of SVG where we need the browser to
            // actually show it.
            response.setContentType(std::string(ContentType::fromFileName(fileName)));
            if (serveAsAttachment)
                response.set("Content-Disposition", "attachment; filename=\"" + fileName + '"');

            if (COOLWSD::WASMState != COOLWSD::WASMActivationState::Disabled)
            {
                response.add("Cross-Origin-Opener-Policy", "same-origin");
                response.add("Cross-Origin-Embedder-Policy", "require-corp");
                response.add("Cross-Origin-Resource-Policy", "cross-origin");
            }

            try
            {
                HttpHelper::sendFile(socket, filePath.toString(), response);
            }
            catch (const Poco::Exception& exc)
            {
                LOG_ERR("Error sending file to client: "
                        << exc.displayText()
                        << (exc.nested() ? " (" + exc.nested()->displayText() + ")" : ""));
            }

            FileUtil::removeFile(filePath.toString());
        }
        else
        {
            if (foundDownloadId)
                LOG_ERR("Download file [" << filePathAnonym << "] not found.");
            else
                LOG_ERR("Download with id [" << downloadId << "] not found.");

            http::Response httpResponse(http::StatusCode::NotFound);
            httpResponse.setContentLength(0);
            socket->sendAndShutdown(httpResponse);
            return true;
        }
        return false;
    }
    else if (requestDetails.equals(1, "render-search-result"))
    {
        RenderSearchResultPartHandler handler;
        Poco::Net::HTMLForm form(request, message, handler);

        const std::string fromPath = handler.getFilename();

        LOG_INF("Create render-search-result POST command handler");

        if (fromPath.empty())
            return false;

        Poco::URI uriPublic = RequestDetails::sanitizeLocalPath(fromPath);
        const std::string docKey = RequestDetails::getDocKey(uriPublic);

        // This lock could become a bottleneck.
        // In that case, we can use a pool and index by publicPath.
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);

        LOG_DBG("New DocumentBroker for docKey [" << docKey << "].");
        auto docBroker = std::make_shared<RenderSearchResultBroker>(
            fromPath, uriPublic, docKey, handler.getSearchResultContent());
        handler.takeFile();

        COOLWSD::cleanupDocBrokers();

        DocBrokers.emplace(docKey, docBroker);
        LOG_TRC("Have " << DocBrokers.size() << " DocBrokers after inserting [" << docKey << "].");

        if (!docBroker->executeCommand(disposition, _id))
        {
            LOG_WRN("Failed to create Client Session with id [" << _id << "] on docKey [" << docKey
                                                                << "].");
            COOLWSD::cleanupDocBrokers();
        }

        return false;
    }

    throw BadRequestException("Invalid or unknown request.");
}

bool ClientRequestDispatcher::handleClientProxyRequest(const Poco::Net::HTTPRequest& request,
                                                       const RequestDetails& requestDetails,
                                                       std::istream& message,
                                                       SocketDisposition& disposition)
{
    // cf. RequestVettingStation::handleRequest ...
    const std::string url = requestDetails.getDocumentURI();

    LOG_INF("URL [" << url << "] for Proxy request.");
    auto uriPublic = RequestDetails::sanitizeURI(url);
    const auto docKey = RequestDetails::getDocKey(uriPublic);
    const std::string fileId = Uri::getFilenameFromURL(Uri::decode(docKey));
    Anonymizer::mapAnonymized(fileId,
                              fileId); // Identity mapping, since fileId is already obfuscated

    LOG_INF("Starting Proxy request handler for session [" << _id << "] on url ["
                                                           << COOLWSD::anonymizeUrl(url) << "].");

    // Check if readonly session is required.
    const bool isReadOnly = Uri::hasReadonlyPermission(uriPublic.toString());

    LOG_INF("URL [" << COOLWSD::anonymizeUrl(url) << "] is "
                    << (isReadOnly ? "readonly" : "writable") << '.');
    (void)request;
    (void)message;
    (void)disposition;

    // Request a kit process for this doc.
    std::pair<std::shared_ptr<DocumentBroker>, std::string> pair
        = findOrCreateDocBroker(DocumentBroker::ChildType::Interactive, url, docKey, /*TODO*/ "",
                              _id, uriPublic, /*mobileAppDocId=*/0);
    auto docBroker = pair.first;

    if (!docBroker)
    {
        const auto& errorMsg = pair.second;
        LOG_ERR("Failed to find document [" << docKey << "]: " << errorMsg);
        // badness occurred:
        auto streamSocket = std::static_pointer_cast<StreamSocket>(disposition.getSocket());
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, streamSocket);
        // FIXME: send docunloading & re-try on client ?
        return true;
    }

    // need to move into the DocumentBroker context before doing session lookup / creation etc.
    docBroker->setupTransfer(
        disposition,
        [docBroker, id = _id, uriPublic = std::move(uriPublic), isReadOnly,
         requestDetails](const std::shared_ptr<Socket>& moveSocket)
        {
            // Now inside the document broker thread ...
            LOG_TRC_S("In the docbroker thread for " << docBroker->getDocKey());

            const int fd = moveSocket->getFD();
            auto streamSocket = std::static_pointer_cast<StreamSocket>(moveSocket);
            try
            {
                docBroker->handleProxyRequest(id, uriPublic, isReadOnly, requestDetails,
                                              streamSocket);
                return;
            }
            catch (const UnauthorizedRequestException& exc)
            {
                LOG_ERR_S("Unauthorized Request while starting session on "
                          << docBroker->getDocKey() << " for socket #" << fd
                          << ". Terminating connection. Error: " << exc.what());
            }
            catch (const StorageConnectionException& exc)
            {
                LOG_ERR_S("Storage error while starting session on "
                          << docBroker->getDocKey() << " for socket #" << fd
                          << ". Terminating connection. Error: " << exc.what());
            }
            catch (const std::exception& exc)
            {
                LOG_ERR_S("Error while starting session on "
                          << docBroker->getDocKey() << " for socket #" << fd
                          << ". Terminating connection. Error: " << exc.what());
            }
            // badness occurred:
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, streamSocket);
        });
    return false; // async
}
#endif

bool ClientRequestDispatcher::handleClientWsUpgrade(const Poco::Net::HTTPRequest& request,
                                                    const RequestDetails& requestDetails,
                                                    SocketDisposition& disposition,
                                                    const std::shared_ptr<StreamSocket>& socket,
                                                    unsigned mobileAppDocId)
{
    const std::string url = requestDetails.getDocumentURI();
    assert(socket && "Must have a valid socket");

    // must be trace for anonymization
    LOG_TRC("Client WS request: " << requestDetails.getURI() << ", url: " << url << ", socket #"
                                  << socket->getFD());

    // First Upgrade.
    const bool allowed = allowedOrigin(request, requestDetails);
    if (!allowed)
    {
        LOG_ERR("Rejecting WebSocket upgrade due to disallowed origin for request: " << request);
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
        return true; // Handled.
    }

    auto ws = std::make_shared<WebSocketHandler>(socket, request, allowed);

    // Response to clients beyond this point is done via WebSocket.
    try
    {
        if (COOLWSD::NumConnections >= COOLWSD::MaxConnections)
        {
            LOG_INF("Limit on maximum number of connections of " << COOLWSD::MaxConnections
                                                                 << " reached.");
            if constexpr (ConfigUtil::isSupportKeyEnabled())
            {
#if ENABLE_SUPPORT_KEY
                shutdownLimitReached(ws);
#endif
                return true;
            }
        }

        const std::string requestKey = requestDetails.getRequestKey();
        if (!requestKey.empty())
        {
            auto it = RequestVettingStations.find(requestKey);
            if (it != RequestVettingStations.end())
            {
                LOG_TRC("Found RVS under key: " << requestKey);
                _rvs = it->second;
                RequestVettingStations.erase(it);
            }
        }

        if (!_rvs)
        {
            LOG_TRC("Creating RVS for key: " << requestKey);
            _rvs = std::make_shared<RequestVettingStation>(COOLWSD::getWebServerPoll(),
                                                           requestDetails);
        }

        // Indicate to the client that document broker is searching.
        static constexpr std::string_view status = R"(progress: { "id":"find" })";
        LOG_TRC("Sending to Client [" << status << ']');
        ws->sendTextMessage(status);

        // We have the client's WS and we either got the proactive CheckFileInfo
        // results, which we can use, or we need to issue a new async CheckFileInfo.
        _rvs->handleRequest(_id, requestDetails, ws, socket, mobileAppDocId, disposition);
        return false; // async keep alive
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Error while handling Client WS Request: " << exc.what());
        constexpr std::string_view msg = "error: cmd=internal kind=load";
        ws->sendTextMessage(msg);
        ws->shutdown(WebSocketHandler::StatusCodes::ENDPOINT_GOING_AWAY, msg);
        socket->ignoreInput();
        return true;
    }
}

/// Lookup cached file content.
const std::string& ClientRequestDispatcher::getFileContent(const std::string& filename)
{
    const auto it = StaticFileContentCache.find(filename);
    if (it == StaticFileContentCache.end())
    {
        throw Poco::FileAccessDeniedException("Invalid or forbidden file path: [" + filename + ']');
    }

    return it->second;
}

/// Process the discovery.xml file and return as string.
std::string ClientRequestDispatcher::getDiscoveryXML()
{
#if MOBILEAPP
    // not needed for mobile
    return std::string();
#else
    std::string discoveryPath =
        Poco::Path(Poco::Util::Application::instance().commandPath()).parent().toString() +
        "discovery.xml";
    if (!Poco::File(discoveryPath).exists())
    {
        // http://server/hosting/discovery.xml
        discoveryPath = COOLWSD::FileServerRoot + "/discovery.xml";
    }

    const std::string action = "action";
    const std::string favIconUrl = "favIconUrl";
    const std::string urlsrc = "urlsrc";

    const std::string rootUriValue = "%SRV_URI%";
    const std::string uriBaseValue = rootUriValue + "/browser/" + Util::getCoolVersionHash() + '/';
    const std::string uriValue = uriBaseValue + "cool.html?";

    LOG_DBG_S("Processing discovery.xml from " << discoveryPath);
    Poco::XML::InputSource inputSrc(discoveryPath);
    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> docXML = parser.parse(&inputSrc);
    Poco::AutoPtr<Poco::XML::NodeList> listNodes = docXML->getElementsByTagName(action);

    for (unsigned long it = 0; it < listNodes->length(); ++it)
    {
        Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(listNodes->item(it));
        Poco::XML::Element* parent =
            elem->parentNode() ? static_cast<Poco::XML::Element*>(elem->parentNode()) : nullptr;
        if (parent && parent->getAttribute("name") == "Capabilities")
        {
            elem->setAttribute(urlsrc, rootUriValue + CAPABILITIES_END_POINT);
        }
        else
        {
            elem->setAttribute(urlsrc, uriValue);
        }

        if (parent && parent->getAttribute("name") == "Settings")
        {
            elem->setAttribute(urlsrc, uriBaseValue + SETTING_IFRAME_END_POINT);
        }

        // Set the View extensions cache as well.
        if (elem->getAttribute("name") == "edit")
        {
            const std::string ext = elem->getAttribute("ext");
            if (COOLWSD::EditFileExtensions.insert(ext).second) // Skip duplicates.
                LOG_DBG_S("Enabling editing of [" << ext << "] extension files");
        }
        else if (elem->getAttribute("name") == "view_comment")
        {
            // We don't seem to treat this list differently.
            // The assumption seems to be that if a file is not editable,
            // then it's view-only. And if it's view-only, it supports comments.
        }
    }

    // turn "images/img.svg" into "http://server.tld/browser/12345abcd/images/img.svg"
    listNodes = docXML->getElementsByTagName("app");
    for (unsigned long it = 0; it < listNodes->length(); ++it)
    {
        Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(listNodes->item(it));

        if (elem->hasAttribute(favIconUrl))
        {
            elem->setAttribute(favIconUrl, uriBaseValue + elem->getAttribute(favIconUrl));
        }
    }

    const auto& proofAttribs = GetProofKeyAttributes();
    if (!proofAttribs.empty())
    {
        // Add proof-key element to wopi-discovery root
        Poco::AutoPtr<Poco::XML::Element> keyElem = docXML->createElement("proof-key");
        for (const auto& attrib : proofAttribs)
            keyElem->setAttribute(attrib.first, attrib.second);
        docXML->documentElement()->appendChild(keyElem);
    }

    std::ostringstream ostrXML;
    Poco::XML::DOMWriter writer;
    writer.writeNode(ostrXML, docXML);
    return ostrXML.str();
#endif
}

void ClientRequestDispatcher::CleanupRequestVettingStations()
{
    if (RequestVettingStations.size() >= NextRvsCleanupSize)
    {
        LOG_DBG("Cleaning up RequestVettingStations ("
                << RequestVettingStations.size()
                << ") with NextRvsCleanupSize: " << NextRvsCleanupSize);

        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::erase_if(RequestVettingStations,
                      [now](const auto& pair) { return pair.second->aged(RvsMaxAge, now); });

        // Clean up next when we grow by 10%.
        NextRvsCleanupSize =
            std::min(RvsHighWatermark - 1,
                     std::max(RvsLowWatermark + 1,
                              static_cast<std::size_t>(RequestVettingStations.size() * 1.1)));

        LOG_DBG("Cleaned up RequestVettingStations ("
                << RequestVettingStations.size()
                << ") with NextRvsCleanupSize: " << NextRvsCleanupSize);
    }
}

#if !MOBILEAPP

namespace
{

/// Create the /hosting/capabilities JSON and return as string.
std::string getCapabilitiesJson(bool convertToAvailable)
{
    // Can the convert-to be used?
    Poco::JSON::Object::Ptr convert_to = new Poco::JSON::Object;
    Poco::Dynamic::Var available = convertToAvailable;
    convert_to->set("available", available);
    if (available)
        convert_to->set("endpoint", "/cool/convert-to");

    Poco::JSON::Object::Ptr capabilities = new Poco::JSON::Object;
    capabilities->set("convert-to", convert_to);

    // Supports the TemplateSaveAs in CheckFileInfo?
    // TemplateSaveAs is broken by design, disable it everywhere (and
    // remove at some stage too)
    capabilities->set("hasTemplateSaveAs", false);

    // Supports the TemplateSource in CheckFileInfo?
    capabilities->set("hasTemplateSource", true);

    // Hint to encourage use on mobile devices
    capabilities->set("hasMobileSupport", true);

    // Set the product name
    capabilities->set("productName", ConfigUtil::getString("product_name", APP_NAME));

    // Set the Server ID
    capabilities->set("serverId", Util::getProcessIdentifier());

    // Set the product version
    capabilities->set("productVersion", Util::getCoolVersion());

    // Set the product version hash
    capabilities->set("productVersionHash", Util::getCoolVersionHash());

    // Set the kit version
    capabilities->set("productKitVersion", COOLWSD::LOKitVersionNumber);

    // Set the kit version hash
    capabilities->set("productKitVersionHash", COOLWSD::LOKitVersionHash);

    // Set that this is a proxy.php-enabled instance
    capabilities->set("hasProxyPrefix", COOLWSD::IsProxyPrefixEnabled);

    // Set if this instance supports Setting Iframe
    capabilities->set("hasSettingIframeSupport", true);

    // Set if this instance supports Zotero
    capabilities->set("hasZoteroSupport", ConfigUtil::getBool("zotero.enable", true));

    // Set if this instance supports WASM.
    capabilities->set("hasWASMSupport",
                      COOLWSD::WASMState != COOLWSD::WASMActivationState::Disabled);

    // Set if this instance supports document signing.
    capabilities->set("hasDocumentSigningSupport",
                      ConfigUtil::getBool("document_signing.enable", true));

    // Advertise wopiAccessCheck endpoint availability
    capabilities->set("hasWopiAccessCheck", true);

    const std::string serverName = ConfigUtil::getString("indirection_endpoint.server_name", "");
    if (const char* podName = std::getenv("POD_NAME"))
        capabilities->set("podName", podName);
    else if (!serverName.empty())
        capabilities->set("podName", serverName);

    if (COOLWSD::IndirectionServerEnabled && COOLWSD::GeolocationSetup)
    {
        std::string timezoneName =
            ConfigUtil::getString("indirection_endpoint.geolocation_setup.timezone", "");
        if (!timezoneName.empty())
            capabilities->set("timezone", timezoneName);
    }

    std::ostringstream ostrJSON;
    capabilities->stringify(ostrJSON);
    return ostrJSON.str();
}

/// Send the /hosting/capabilities JSON to socket
void sendCapabilities(bool convertToAvailable, bool closeConnection,
                      const std::weak_ptr<StreamSocket>& socketWeak)
{
    std::shared_ptr<StreamSocket> socket = socketWeak.lock();
    if (!socket)
    {
        LOG_ERR("Invalid socket while sending capabilities");
        return;
    }

    http::Response httpResponse(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(httpResponse);
    httpResponse.set("Last-Modified", Util::getHttpTimeNow());
    httpResponse.setBody(getCapabilitiesJson(convertToAvailable), "application/json");
    httpResponse.set("X-Content-Type-Options", "nosniff");
    if( closeConnection )
        socket->sendAndShutdown(httpResponse);
    else
        socket->send(httpResponse);
    LOG_INF("Sent capabilities.json successfully.");
}

} // namespace

bool ClientRequestDispatcher::handleCapabilitiesRequest(const Poco::Net::HTTPRequest& request,
                                                        const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG("Wopi capabilities request: " << request.getURI());
    const bool closeConnection = !request.getKeepAlive();
    std::weak_ptr<StreamSocket> socketWeak(socket);

    AsyncFn convertToAllowedCb = [socketWeak, closeConnection](bool allowedConvert)
    {
        COOLWSD::getWebServerPoll()->addCallback(
            [socketWeak, allowedConvert, closeConnection]()
            { sendCapabilities(allowedConvert, closeConnection, socketWeak); });
    };

    allowConvertTo(socket->clientAddress(), request, true, std::move(convertToAllowedCb));
    return false;
}

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
