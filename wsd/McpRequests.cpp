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

#include <config.h>

#include <wsd/McpRequests.hpp>

#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ContentType.hpp>
#include <wsd/McpServer.hpp>
#include <wsd/McpTools.hpp>
#include <wsd/McpUploads.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/ServerURL.hpp>
#include <wsd/SpecialBrokers.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/StringTokenizer.h>
#include <Poco/URI.h>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <istream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
/// Compare two strings without letting the time taken reveal how much of the key matched or how
/// long the key is. The comparison always runs over the length of the expected key and folds any
/// length difference into the result.
bool constantTimeEquals(const std::string& expected, const std::string& given)
{
    // The difference is accumulated at the width of a size, so a difference in length is carried
    // whole.
    std::size_t difference = expected.size() ^ given.size();
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        const unsigned char givenChar =
            i < given.size() ? static_cast<unsigned char>(given[i]) : 0;
        difference |= static_cast<unsigned char>(expected[i]) ^ givenChar;
    }
    return difference == 0;
}

/// What the convert-to broker is asked to do with the document. A tool fills in only the fields its
/// own request type reads and leaves the rest empty.
struct McpBrokerRequest
{
    std::string requestType;
    std::string format;
    std::string options;
    std::string lang;
    std::string target;
    std::string filter;
    std::string transformJSON;
};

/// Puts the responder that carries one request's outcome on the broker.
using McpResponderInstaller = std::function<void(const std::shared_ptr<ConvertToBroker>&)>;

/// Load the document named by input and run one request over it, driving the same stateless broker
/// that the convert-to endpoints use. The client socket moves to the broker through disposition, in
/// the same way a convert-to request does. The outcome is delivered through complete, by way of the
/// responder that installResponder puts on the broker. Returns true when the request was started
/// and will complete later, false when it failed right away and complete has already been called.
bool startMcpDocumentRequest(const McpBrokerRequest& brokerRequest,
                             const McpTools::ResolvedInput& input,
                             const McpResponderInstaller& installResponder,
                             SocketDisposition& disposition, const std::string& id,
                             const McpServer::McpToolCompletion& complete)
{
    Poco::URI uriPublic = RequestDetails::sanitizeLocalPath(input.fromPath);
    const std::string docKey = RequestDetails::getDocKey(uriPublic);

    std::shared_ptr<ConvertToBroker> docBroker = getConvertToBrokerImplementation(
        brokerRequest.requestType, input.fromPath, uriPublic, docKey, brokerRequest.format,
        brokerRequest.options, brokerRequest.lang, brokerRequest.target, brokerRequest.filter,
        brokerRequest.transformJSON);
    if (!docBroker)
    {
        StatelessBatchBroker::removeFile(input.fromPath);
        complete({ /*isError=*/true, "The document could not be processed." });
        return false;
    }

    installResponder(docBroker);

    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    COOLWSD::cleanupDocBrokers();
    DocBrokers.emplace(docKey, docBroker);

    docBroker->startConversion(disposition, id, AdditionalFilePocoUris());
    return true;
}

/// Turns the input part of a tool's arguments into a document and starts the work that tool does
/// with it. Returns whether the work was started and will report its own outcome later.
using McpDocumentWork = std::function<bool(const McpTools::ResolvedInput&)>;

/// Read the input part of a tool's arguments and hand the document to startWork. An input that is
/// refused becomes the tool's failure. Returns whether the outcome reached complete before this
/// call returned.
bool runWithDocument(const Poco::JSON::Object::Ptr& arguments,
                     const McpServer::McpToolCompletion& complete, const McpDocumentWork& startWork)
{
    const McpTools::ResolvedInput input = McpTools::resolveInput(arguments);
    if (!input.ok)
    {
        complete({ /*isError=*/true, input.errorText });
        return true;
    }

    return !startWork(input);
}

/// Run one extract request over the document named by input. The extracted payload comes back
/// through complete.
bool startMcpExtraction(const std::string& requestType, const std::string& filter,
                        const McpTools::ResolvedInput& input, SocketDisposition& disposition,
                        const std::string& id, const McpServer::McpToolCompletion& complete)
{
    McpBrokerRequest brokerRequest;
    brokerRequest.requestType = requestType;
    brokerRequest.filter = filter;

    // A successful extraction carries its payload straight back to the model; a failure becomes a
    // readable message the model can react to.
    const McpResponderInstaller installResponder =
        [complete](const std::shared_ptr<ConvertToBroker>& docBroker)
    {
        docBroker->setExtractResponder(
            [complete](bool success, const std::string& payload)
            {
                if (success)
                    complete({ /*isError=*/false, payload });
                else
                    complete({ /*isError=*/true, McpTools::describeFailure(payload, "processed") });
            });
    };

    return startMcpDocumentRequest(brokerRequest, input, installResponder, disposition, id,
                                   complete);
}

/// Build a tool result that carries a produced document. The document comes back to the model as an
/// embedded resource, with its media type derived from the result file name.
McpServer::McpToolResult makeDocumentResult(std::string bytes, const std::string& filename)
{
    McpServer::McpToolResult result;
    result.isDocument = true;
    result.documentBytes = std::move(bytes);
    result.documentFilename = filename.empty() ? "document" : filename;
    result.documentMimeType = std::string(ContentType::fromFileName(result.documentFilename));
    return result;
}

/// Run one conversion or transform over the document named by input. The produced document comes
/// back through complete as an embedded resource.
bool startMcpConversion(const std::string& requestType, const std::string& format,
                        const std::string& options, const std::string& lang,
                        const std::string& transformJSON, const McpTools::ResolvedInput& input,
                        SocketDisposition& disposition, const std::string& id,
                        const McpServer::McpToolCompletion& complete)
{
    McpBrokerRequest brokerRequest;
    brokerRequest.requestType = requestType;
    brokerRequest.format = format;
    brokerRequest.options = options;
    brokerRequest.lang = lang;
    brokerRequest.transformJSON = transformJSON;

    // A successful conversion carries the produced document straight back to the model; a failure
    // becomes a readable message the model can react to.
    const McpResponderInstaller installResponder =
        [complete](const std::shared_ptr<ConvertToBroker>& docBroker)
    {
        docBroker->setSaveAsResponder(
            [complete](bool success, std::string payloadOrError, const std::string& filename,
                       const std::string& note)
            {
                if (!success)
                {
                    complete({ /*isError=*/true,
                               McpTools::describeFailure(payloadOrError, "produced") });
                    return;
                }
                McpServer::McpToolResult result =
                    makeDocumentResult(std::move(payloadOrError), filename);
                result.note = note;
                complete(std::move(result));
            });
    };

    return startMcpDocumentRequest(brokerRequest, input, installResponder, disposition, id,
                                   complete);
}
}

namespace
{
/// The part of the Model Context Protocol endpoint a request path names.
enum class McpPath : std::uint8_t
{
    /// The JSON-RPC message path, /cool/mcp itself.
    Message,
    /// The upload path, /cool/mcp/upload.
    Upload
};

/// The part of the endpoint that path names.
McpPath classifyMcpPath(const std::string& path)
{
    Poco::StringTokenizer segments(path, "/",
                                   Poco::StringTokenizer::TOK_IGNORE_EMPTY |
                                       Poco::StringTokenizer::TOK_TRIM);
    for (std::size_t i = 0; i + 1 < segments.count(); ++i)
    {
        if (segments[i] == "mcp" && segments[i + 1] == "upload")
            return McpPath::Upload;
    }
    return McpPath::Message;
}

/// The name an upload goes by: its filename query parameter, or "document" when it has none.
std::string uploadFilename(const Poco::URI& uri)
{
    for (const auto& parameter : uri.getQueryParameters())
    {
        if (parameter.first == "filename" && !parameter.second.empty())
            return parameter.second;
    }
    return "document";
}

/// Answer an upload with the identifier its document is stored under, and how long it stays.
void sendUploadAnswer(const std::shared_ptr<StreamSocket>& socket, const std::string& fileId)
{
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("file_id", fileId);
    result->set("expires_in", static_cast<int>(McpUploads::TimeToLive.count()));
    std::ostringstream resultStream;
    result->stringify(resultStream);

    http::Response httpResponse(http::StatusCode::OK);
    httpResponse.setBody(resultStream.str(), "application/json");
    httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
    socket->sendAndShutdown(httpResponse);
    socket->ignoreInput();
}

/// A copy of request with the service root taken off its address, when the address is for the Model
/// Context Protocol endpoint, plainly or in the hex form, and nothing otherwise. Reading the
/// details takes the service root off the address of the request they are read from, so they are
/// read from the copy and request keeps the address it came with.
std::optional<Poco::Net::HTTPRequest> strippedMcpRequest(const Poco::Net::HTTPRequest& request)
{
    const std::string& uri = request.getURI();
    if (uri.find("/mcp") == std::string::npos && uri.find("cool/0x") == std::string::npos)
        return std::nullopt;

    Poco::Net::HTTPRequest stripped(request);
    try
    {
        if (!McpRequests::isMcpRequest(RequestDetails(stripped, COOLWSD::ServiceRoot)))
            return std::nullopt;
    }
    catch (const std::exception& exc)
    {
        // An address that cannot be read counts as an address for another endpoint.
        LOG_DBG_S("MCP: leaving an unreadable address to the dispatcher: " << exc.what());
        return std::nullopt;
    }
    return stripped;
}

/// The number of bytes in message from its read position to its end, or -1 when the stream cannot
/// tell.
std::streamsize remainingBytes(std::istream& message)
{
    const std::streampos position = message.tellg();
    if (position == std::streampos(-1))
        return -1;
    message.seekg(0, std::ios::end);
    const std::streampos end = message.tellg();
    message.seekg(position);
    return end == std::streampos(-1) ? -1 : static_cast<std::streamsize>(end - position);
}

/// Counts the Model Context Protocol tool calls that are running. Each running call holds one
/// instance, and the count goes down when the last copy of it goes away.
class RunningMcpTool final
{
public:
    RunningMcpTool() { ++Count; }
    ~RunningMcpTool() { --Count; }
    RunningMcpTool(const RunningMcpTool&) = delete;
    RunningMcpTool& operator=(const RunningMcpTool&) = delete;

    /// The number of tool calls running now.
    static std::size_t count() { return Count; }

private:
    static std::atomic<std::size_t> Count;
};

std::atomic<std::size_t> RunningMcpTool::Count(0);
}

bool McpRequests::isMcpRequest(const RequestDetails& requestDetails)
{
    return !requestDetails.isWebSocket() &&
           (requestDetails.equals(RequestDetails::Field::Type, "cool") ||
            requestDetails.equals(RequestDetails::Field::Type, "lool")) &&
           requestDetails.equals(1, "mcp");
}

bool McpRequests::refuseMcpRequest(const Poco::Net::HTTPRequest& request,
                                   const std::shared_ptr<StreamSocket>& socket)
{
    const bool enabled = ConfigUtil::getBool("ai.mcp.enable", false);
    const std::string apiKey = ConfigUtil::getString("ai.mcp.api_key", std::string());
    if (!enabled || apiKey.empty())
    {
        HttpHelper::sendErrorAndShutdown(http::StatusCode::NotFound, socket);
        return true;
    }

    // Guard against DNS-rebinding. A protocol client sends no Origin header; a browser page always
    // does, so the presence of one means the request came from a web page and is refused.
    if (request.has("Origin"))
    {
        LOG_WRN_S("MCP: rejecting request that carries an Origin header");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
        return true;
    }

    // The caller must present the configured key as a bearer token. The header holds the scheme, a
    // space and the credential. RFC 7235 makes the scheme case-insensitive, so it is matched letter
    // by letter without regard to case, and the credential alone is compared in constant time.
    const std::string authorization = request.get("Authorization", std::string());
    const std::size_t schemeEnd = authorization.find(' ');
    const std::string scheme = authorization.substr(0, schemeEnd);
    const std::string credential =
        schemeEnd == std::string::npos ? std::string() : authorization.substr(schemeEnd + 1);
    if (!Util::iequal(scheme, "Bearer") || !constantTimeEquals(apiKey, credential))
    {
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket,
                                         std::string_view(), "WWW-Authenticate: Bearer\r\n");
        return true;
    }

    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_POST)
        return false;

    // A body states its length, so it is weighed against the accepted size before it arrives. A
    // chunked body, or a POST with no Content-Length, states none and is refused.
    const McpPath path = classifyMcpPath(Poco::URI(request.getURI()).getPath());
    const std::streamsize declaredLength = request.getContentLength();
    if (request.getChunkedTransferEncoding() || declaredLength < 0)
    {
        LOG_WRN_S("MCP: refusing a request that does not state its length");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::LengthRequired, socket,
                                         "Send the request with a Content-Length header.\n");
        return true;
    }

    // A limit of zero means no limit is configured for an upload. A message has a limit of its own,
    // since a larger one would be parsed on the web-server thread at length. A document that large
    // travels as an upload instead.
    const std::size_t maxBytes =
        path == McpPath::Upload ? McpTools::maxDocumentBytes() : McpTools::MaxMessageBytes;
    if (maxBytes != 0 && NumUtil::makeUnsigned(declaredLength) > maxBytes)
    {
        LOG_WRN_S("MCP: refusing a request body larger than the accepted size");
        HttpHelper::sendErrorAndShutdown(
            http::StatusCode::PayloadTooLarge, socket,
            path == McpPath::Upload ? "The document is larger than the server accepts.\n"
                                    : "The message is too large. Upload a large document and pass "
                                      "its file_id.\n");
        return true;
    }

    // An upload is written to disk as it arrives, so the store is asked for room before then.
    if (path == McpPath::Upload &&
        !McpUploads::instance().hasRoomFor(NumUtil::makeUnsigned(declaredLength)))
    {
        LOG_WRN_S("MCP: refusing an upload the store has no room for");
        HttpHelper::sendErrorAndShutdown(
            http::StatusCode::InsufficientStorage, socket,
            "The server holds too many uploaded documents. Try again later.\n");
        return true;
    }

    return false;
}

bool McpRequests::refuseMcpRequestEarly(const Poco::Net::HTTPRequest& request,
                                        const std::shared_ptr<StreamSocket>& socket)
{
    const std::optional<Poco::Net::HTTPRequest> stripped = strippedMcpRequest(request);
    return stripped && refuseMcpRequest(*stripped, socket);
}

bool McpRequests::isMcpUpload(const Poco::Net::HTTPRequest& request)
{
    const std::optional<Poco::Net::HTTPRequest> stripped = strippedMcpRequest(request);
    return stripped && stripped->getMethod() == Poco::Net::HTTPRequest::HTTP_POST &&
           classifyMcpPath(Poco::URI(stripped->getURI()).getPath()) == McpPath::Upload;
}

std::shared_ptr<McpUploadSink> McpUploadSink::start(const Poco::Net::HTTPRequest& request,
                                                    bool& storeFull,
                                                    std::chrono::steady_clock::time_point now)
{
    storeFull = false;
    const std::streamsize declaredLength = request.getContentLength();
    if (declaredLength <= 0)
        return nullptr;

    const std::size_t size = NumUtil::makeUnsigned(declaredLength);
    if (!McpUploads::instance().reserve(size, now))
    {
        storeFull = true;
        return nullptr;
    }

    // The sink gives the room back if it fails from here on.
    std::shared_ptr<McpUploadSink> sink(
        new McpUploadSink(size, now + McpUploads::uploadTimeLimit(size)));
    sink->_path = McpTools::makeIncomingPath(uploadFilename(Poco::URI(request.getURI())));
    if (sink->_path.empty())
        return nullptr;

    sink->_stream.open(sink->_path, std::ios::binary);
    if (!sink->_stream.good())
    {
        LOG_ERR_S("MCP: failed to open the upload file [" << sink->_path << ']');
        return nullptr;
    }

    LOG_TRC_S("MCP: writing an upload of " << size << " bytes to [" << sink->_path << ']');
    return sink;
}

McpUploadSink::~McpUploadSink()
{
    if (_stored)
        return;

    if (_stream.is_open())
        _stream.close();
    if (!_path.empty())
        FileUtil::removeFileAndEmptyParent(_path);
    McpUploads::instance().release(_size);
}

bool McpUploadSink::write(const char* data, std::size_t size)
{
    if (_written + size > _size)
        return false;
    _stream.write(data, static_cast<std::streamsize>(size));
    _written += size;
    return _stream.good();
}

void McpUploadSink::finish(const std::shared_ptr<StreamSocket>& socket)
{
    _stream.close();
    if (_written != _size || !_stream.good())
    {
        LOG_ERR_S("MCP: the upload to [" << _path << "] could not be written whole");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::InternalServerError, socket);
        socket->ignoreInput();
        return;
    }

    const std::string fileId = McpUploads::instance().adopt(_path, _size);
    _stored = true;
    sendUploadAnswer(socket, fileId);
}

bool McpRequests::handleMcpRequest(const Poco::Net::HTTPRequest& request,
                                   const RequestDetails& requestDetails,
                                   std::istream& message,
                                   SocketDisposition& disposition,
                                   const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    if (refuseMcpRequest(request, socket))
        return true;

    // The endpoint only takes POST.
    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_POST)
    {
        HttpHelper::sendErrorAndShutdown(http::StatusCode::MethodNotAllowed, socket,
                                         std::string_view(), "Allow: POST\r\n");
        return true;
    }

    // The body is the number of bytes the request states, and anything after it in the buffer
    // belongs to another request.
    const std::streamsize bodyLength = request.getContentLength();
    if (remainingBytes(message) < bodyLength)
    {
        LOG_WRN_S("MCP: refusing a request whose body is shorter than its stated length");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return true;
    }

    const Poco::URI parsedUri(request.getURI());
    if (classifyMcpPath(parsedUri.getPath()) == McpPath::Upload)
    {
        // The whole request body is the raw document, with no form or multipart wrapping. A
        // filename query parameter names it.
        if (bodyLength == 0)
        {
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
            return true;
        }

        const std::string fileId = McpUploads::instance().store(
            uploadFilename(parsedUri), message, NumUtil::makeUnsigned(bodyLength));
        if (fileId.empty())
        {
            HttpHelper::sendErrorAndShutdown(http::StatusCode::InternalServerError, socket);
            return true;
        }

        sendUploadAnswer(socket, fileId);
        return true;
    }

    std::string body(NumUtil::makeUnsigned(bodyLength), '\0');
    message.read(body.data(), bodyLength);
    const std::string protocolVersionHeader = request.get("MCP-Protocol-Version", std::string());

    // The address of this endpoint as the client reaches it, with the service root. The upload
    // address the descriptions give is made from it.
    const std::string endpointUrl = ServerURL(requestDetails).getSubURLForEndpoint("/cool/mcp");

    // The response is always sent with sendAndShutdown, so the connection is closed after this one
    // message either way.
    std::weak_ptr<StreamSocket> socketWeak = socket;
    std::function<void(McpServer::McpResponse)> sendResponse =
        [socketWeak](McpServer::McpResponse mcpResponse)
    {
        std::shared_ptr<StreamSocket> streamSocket = socketWeak.lock();
        if (!streamSocket)
        {
            LOG_WRN_S("MCP: socket gone before the response could be sent");
            return;
        }

        http::Response httpResponse(static_cast<http::StatusCode>(mcpResponse.statusCode));
        if (mcpResponse.body.empty())
            httpResponse.setContentLength(0);
        else
            httpResponse.setBody(std::move(mcpResponse.body), "application/json");
        httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
        streamSocket->sendAndShutdown(httpResponse);
        streamSocket->ignoreInput();
    };

    // A fresh identifier for the throwaway session a tool call may create.
    const std::string sessionId = "mcp_" + Util::rng::getHexString(8);

    // The tools and their executors live only as long as this call, and so does disposition, which
    // the executors hold by reference.
    std::vector<McpServer::McpTool> tools = McpServer::documentToolDefinitions();
    for (McpServer::McpTool& tool : tools)
    {
        tool.description = McpServer::withEndpointAddress(tool.description, endpointUrl);

        if (tool.name == McpServer::ToolName::ExtractDocumentStructure)
        {
            tool.executor = [&disposition, sessionId](const Poco::JSON::Object::Ptr& arguments,
                                                      const McpServer::McpToolCompletion& complete)
            {
                std::optional<McpServer::McpToolResult> invalid =
                    McpServer::validateExtractArguments(arguments);
                if (invalid)
                {
                    complete(*invalid);
                    return true;
                }

                // A spreadsheet range or sheet, or a text-document target, is folded into the text
                // filter as a ",name:value" part, and any of them alone asks for the text view. The
                // target and the sheet may hold spaces and the separator character, so they are
                // percent-encoded because the command is split on spaces and then read as a query
                // on the engine side. Each of them runs to the end of the filter, so it comes last.
                std::string filter = arguments->optValue<std::string>("filter", std::string());
                const std::string range = arguments->optValue<std::string>("range", std::string());
                const std::string sheet = arguments->optValue<std::string>("sheet", std::string());
                const std::string target =
                    arguments->optValue<std::string>("target", std::string());
                if (filter.empty() && (!range.empty() || !sheet.empty() || !target.empty()))
                    filter = "text";
                const bool textFilter = filter.rfind("text", 0) == 0;
                if (textFilter && !range.empty())
                    filter += ",range:" + range;
                if (textFilter && !sheet.empty())
                    filter += ",sheet:" + Uri::encode(sheet, "%|,/?:@&=+$#");
                if (textFilter && !target.empty())
                    filter += ",target:" + Uri::encode(target, "%|,/?:@&=+$#");

                return runWithDocument(arguments, complete,
                                       [&disposition, sessionId, filter,
                                        complete](const McpTools::ResolvedInput& input)
                                       {
                                           return startMcpExtraction("extract-document-structure",
                                                                     filter, input, disposition,
                                                                     sessionId, complete);
                                       });
            };
        }
        else if (tool.name == McpServer::ToolName::ExtractLinkTargets)
        {
            tool.executor = [&disposition, sessionId](const Poco::JSON::Object::Ptr& arguments,
                                                      const McpServer::McpToolCompletion& complete)
            {
                return runWithDocument(
                    arguments, complete,
                    [&disposition, sessionId, complete](const McpTools::ResolvedInput& input)
                    {
                        return startMcpExtraction("extract-link-targets", std::string(), input,
                                                  disposition, sessionId, complete);
                    });
            };
        }
        else if (tool.name == McpServer::ToolName::ConvertDocument)
        {
            tool.executor = [&disposition, sessionId](const Poco::JSON::Object::Ptr& arguments,
                                                      const McpServer::McpToolCompletion& complete)
            {
                std::optional<McpServer::McpToolResult> invalid =
                    McpServer::validateConvertArguments(arguments);
                if (invalid)
                {
                    complete(*invalid);
                    return true;
                }

                const std::string format =
                    arguments->optValue<std::string>("format", std::string());
                const std::string options =
                    arguments->optValue<std::string>("options", std::string());
                const std::string lang = arguments->optValue<std::string>("lang", std::string());

                return runWithDocument(arguments, complete,
                                       [&disposition, sessionId, format, options, lang,
                                        complete](const McpTools::ResolvedInput& input)
                                       {
                                           return startMcpConversion(
                                               "convert-to", format, options, lang,
                                               /*transformJSON=*/std::string(), input, disposition,
                                               sessionId, complete);
                                       });
            };
        }
        else if (tool.name == McpServer::ToolName::TransformDocumentStructure)
        {
            tool.executor = [&disposition, sessionId](const Poco::JSON::Object::Ptr& arguments,
                                                      const McpServer::McpToolCompletion& complete)
            {
                std::optional<McpServer::McpToolResult> invalid =
                    McpServer::validateTransformArguments(arguments);
                if (invalid)
                {
                    complete(*invalid);
                    return true;
                }

                // The transform is a JSON string that may hold spaces and other characters that the
                // engine-side command is split on, so it is percent-encoded before it reaches the
                // kit.
                const std::string transform =
                    arguments->optValue<std::string>("transform", std::string());
                std::string encodedTransform;
                Poco::URI::encode(transform, "", encodedTransform);

                const std::string format =
                    arguments->optValue<std::string>("format", std::string());

                return runWithDocument(arguments, complete,
                                       [&disposition, sessionId, format, encodedTransform,
                                        complete](const McpTools::ResolvedInput& input)
                                       {
                                           return startMcpConversion(
                                               "transform-document-structure", format,
                                               /*options=*/std::string(),
                                               /*lang=*/std::string(), encodedTransform, input,
                                               disposition, sessionId, complete);
                                       });
            };
        }

        if (!tool.executor)
        {
            LOG_ERR_S("MCP: tool [" << tool.name << "] is offered with nothing to run it");
            continue;
        }

        // A call holds its place among the running calls until the last copy of its completion is
        // gone, which is once it has answered and its document work has been let go.
        tool.executor = [executor = std::move(tool.executor)](
                            const Poco::JSON::Object::Ptr& arguments,
                            const McpServer::McpToolCompletion& complete)
        {
            const std::size_t maxRunning =
                ConfigUtil::getConfigValue<std::size_t>("ai.mcp.max_running_tools", 0);
            if (maxRunning != 0 && RunningMcpTool::count() >= maxRunning)
            {
                complete({ /*isError=*/true, "The server is busy with other tool calls. Try "
                                             "again in a little while." });
                return true;
            }

            const auto running = std::make_shared<RunningMcpTool>();
            return executor(arguments, [complete, running](McpServer::McpToolResult result)
                            { complete(std::move(result)); });
        };
    }

    return McpServer::handleMessage(body, protocolVersionHeader, tools, sendResponse);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
