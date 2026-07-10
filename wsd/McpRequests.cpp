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
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/McpServer.hpp>
#include <wsd/McpTools.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/SpecialBrokers.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <functional>
#include <istream>
#include <memory>
#include <mutex>
#include <optional>
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

/// Load the document named by input and run one extract request over it, driving the same stateless
/// broker that the convert-to extract endpoints use. The outcome is delivered through complete.
/// Returns true when the request was started and will complete later, false when it failed right
/// away and complete has already been called.
bool startMcpExtraction(const std::string& requestType, const std::string& filter,
                        const McpTools::ResolvedInput& input, SocketDisposition& disposition,
                        const std::string& id, const McpServer::McpToolCompletion& complete)
{
    Poco::URI uriPublic = RequestDetails::sanitizeLocalPath(input.fromPath);
    const std::string docKey = RequestDetails::getDocKey(uriPublic);

    std::shared_ptr<ConvertToBroker> docBroker = getConvertToBrokerImplementation(
        requestType, input.fromPath, uriPublic, docKey, /*format=*/std::string(),
        /*options=*/std::string(), /*lang=*/std::string(), /*target=*/std::string(), filter,
        /*transformJSON=*/std::string());
    if (!docBroker)
    {
        StatelessBatchBroker::removeFile(input.fromPath);
        complete({ /*isError=*/true, "The document could not be processed." });
        return false;
    }

    // A successful extraction carries its payload straight back to the model; a failure becomes a
    // readable message the model can react to.
    docBroker->setExtractResponder(
        [complete](bool success, const std::string& payload)
        {
            if (success)
                complete({ /*isError=*/false, payload });
            else
                complete({ /*isError=*/true, McpTools::describeFailure(payload, "processed") });
        });

    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    COOLWSD::cleanupDocBrokers();
    DocBrokers.emplace(docKey, docBroker);

    docBroker->startConversion(disposition, id, AdditionalFilePocoUris());
    return true;
}
}

namespace
{
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
    if (request.getChunkedTransferEncoding() || request.getContentLength() < 0)
    {
        LOG_WRN_S("MCP: refusing a request that does not state its length");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::LengthRequired, socket,
                                         "Send the request with a Content-Length header.\n");
        return true;
    }

    // A larger message would be parsed on the web-server thread at length. A document that large
    // travels as an upload instead.
    const std::streamsize declaredLength = request.getContentLength();
    if (declaredLength != Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH &&
        (declaredLength < 0 || NumUtil::makeUnsigned(declaredLength) > McpTools::MaxMessageBytes))
    {
        LOG_WRN_S("MCP: refusing a message larger than the accepted size");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::PayloadTooLarge, socket,
                                         "The message is too large. Upload a large document "
                                         "and pass its file_id.\n");
        return true;
    }

    return false;
}

bool McpRequests::refuseMcpRequestEarly(const Poco::Net::HTTPRequest& request,
                                        const std::shared_ptr<StreamSocket>& socket)
{
    // Only an address that can name the endpoint, plainly or in the hex form, is parsed here.
    const std::string& uri = request.getURI();
    if (uri.find("/mcp") == std::string::npos && uri.find("cool/0x") == std::string::npos)
        return false;

    // Reading the details takes the service root off the address of the request they are read from,
    // so they are read from a copy, and the request itself reaches the dispatcher as it came.
    Poco::Net::HTTPRequest stripped(request);
    try
    {
        if (!isMcpRequest(RequestDetails(stripped, COOLWSD::ServiceRoot)))
            return false;
    }
    catch (const std::exception& exc)
    {
        // An address that cannot be read is answered by the dispatcher with the other requests.
        LOG_DBG_S("MCP: leaving an unreadable address to the dispatcher: " << exc.what());
        return false;
    }

    return refuseMcpRequest(stripped, socket);
}

bool McpRequests::handleMcpRequest(const Poco::Net::HTTPRequest& request,
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

    std::string body(NumUtil::makeUnsigned(bodyLength), '\0');
    message.read(body.data(), bodyLength);
    const std::string protocolVersionHeader = request.get("MCP-Protocol-Version", std::string());

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

                McpTools::ResolvedInput input = McpTools::resolveInput(arguments);
                if (!input.ok)
                {
                    complete({ /*isError=*/true, input.errorText });
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

                const bool started = startMcpExtraction("extract-document-structure", filter, input,
                                                        disposition, sessionId, complete);
                return !started;
            };
        }
        else if (tool.name == McpServer::ToolName::ExtractLinkTargets)
        {
            tool.executor = [&disposition, sessionId](const Poco::JSON::Object::Ptr& arguments,
                                                      const McpServer::McpToolCompletion& complete)
            {
                McpTools::ResolvedInput input = McpTools::resolveInput(arguments);
                if (!input.ok)
                {
                    complete({ /*isError=*/true, input.errorText });
                    return true;
                }

                const bool started = startMcpExtraction("extract-link-targets", std::string(),
                                                        input, disposition, sessionId, complete);
                return !started;
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
