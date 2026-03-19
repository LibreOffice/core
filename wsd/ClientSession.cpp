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
 * Implementation of client session handling and message processing.
 * Classes: ClientSession
 */

#include <config.h>

#include "ClientSession.hpp"

#include <common/Clipboard.hpp>
#include <common/CommandControl.hpp>
#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/HexUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/NumUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Session.hpp>
#include <common/TraceEvent.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpServer.hpp>
#include <wopi/StorageConnectionManager.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/FileServer.hpp>
#if !MOBILEAPP
#include <wsd/DocumentToolDescriptions.hpp>
#endif
#include <wsd/TileDesc.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/MemoryStream.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>

#include <cctype>
#include <ios>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace COOLProtocol;

static constexpr float TILES_ON_FLY_MIN_UPPER_LIMIT = 10.0;
static constexpr int SYNTHETIC_COOL_PID_OFFSET = 10000000;

using Poco::Path;

// rotates regularly
const int ClipboardTokenLengthBytes = 16;
// home-use, disabled by default.
const int ProxyAccessTokenLengthBytes = 32;

namespace
{
std::mutex GlobalSessionMapMutex;
std::unordered_map<std::string, std::weak_ptr<ClientSession>> GlobalSessionMap;

} // namespace

namespace
{
void logSyntaxErrorDetails(const StringVector& tokens, const std::string& firstLine)
{
    LOG_WRN("Invalid syntax for '" << tokens[0] << "' message: [" << firstLine << ']');
}
}

ClientSession::ClientSession(const std::shared_ptr<ProtocolHandlerInterface>& ws,
                             const std::string& id,
                             const std::shared_ptr<DocumentBroker>& docBroker,
                             const Poco::URI& uriPublic, const bool readOnly,
                             const RequestDetails& requestDetails,
                             const AdditionalFilePocoUris& additionalFileUrisPublic)
    : Session(ws, "ToClient-" + id, id, readOnly)
    , _uriPublic(uriPublic)
    , _additionalFileUrisPublic(additionalFileUrisPublic)
    , _serverURL(requestDetails)
    , _auth(Authorization::create(uriPublic))
    , _docBroker(docBroker)
    , _lastStateTime(std::chrono::steady_clock::now())
    , _clientVisibleArea(0, 0, 0, 0)
    , _keyEvents(1)
    , _performanceCounterEpoch(0)
    , _splitX(0)
    , _splitY(0)
    , _clientSelectedPart(-1)
    , _clientSelectedMode(0)
    , _tileWidthPixel(0)
    , _tileHeightPixel(0)
    , _tileWidthTwips(0)
    , _tileHeightTwips(0)
    , _kitViewId(-1)
    , _canonicalViewId(CanonicalViewId::None)
    , _state(SessionState::DETACHED)
    , _isDocumentOwner(false)
    , _isTextDocument(false)
    , _thumbnailSession(false)
    , _sentAudit(false)
    , _sentBrowserSetting(false)
    , _isConvertTo(false)
{
    const std::size_t curConnections = ++COOLWSD::NumConnections;
    LOG_INF("ClientSession ctor [" << getName() << "] for URI: [" << _uriPublic.toString()
                                   << "], current number of connections: " << curConnections);

    // populate with random values.
    for (size_t i = 0; i < N_ELEMENTS(_clipboardKeys); ++i)
        rotateClipboardKey(false);

    // Emit metadata Trace Events for the synthetic pid used for the Trace Events coming in from the
    // client's cool, and for its dummy thread.
    TraceEvent::emitOneRecordingIfEnabled(
        R"({"name":"process_name","ph":"M","args":{"name":"cool-)" + id + R"("},"pid":)" +
        std::to_string(ProcUtil::getProcessId() + SYNTHETIC_COOL_PID_OFFSET) + ",\"tid\":1},\n");
    TraceEvent::emitOneRecordingIfEnabled(
        R"({"name":"thread_name","ph":"M","args":{"name":"JS"},"pid":)" +
        std::to_string(ProcUtil::getProcessId() + SYNTHETIC_COOL_PID_OFFSET) + ",\"tid\":1},\n");

    _browserSettingsJSON = new Poco::JSON::Object();
}

// Can't take a reference in the constructor.
void ClientSession::construct()
{
    std::unique_lock<std::mutex> lock(GlobalSessionMapMutex);
    MessageHandlerInterface::initialize();
    GlobalSessionMap[getId()] = client_from_this();
}

ClientSession::~ClientSession()
{
    const std::size_t curConnections = --COOLWSD::NumConnections;
    LOG_INF("~ClientSession dtor [" << getName() << "], current number of connections: " << curConnections);

    std::unique_lock<std::mutex> lock(GlobalSessionMapMutex);
    GlobalSessionMap.erase(getId());
}

void ClientSession::setState(SessionState newState)
{
    LOG_TRC("transition from " << name(_state) << " to " << name(newState));

    // we can get incoming messages while our disconnection is in transit.
    if (_state == SessionState::WAIT_DISCONNECT)
    {
        if (newState != SessionState::WAIT_DISCONNECT)
            LOG_WRN("Unusual race - attempts to transition from " << name(_state) << " to "
                                                                  << name(newState));
        return;
    }

    switch (newState)
    {
    case SessionState::DETACHED:
        assert(_state == SessionState::DETACHED);
        break;
    case SessionState::LOADING:
        assert(_state == SessionState::DETACHED);
        break;
    case SessionState::LIVE:
        assert(_state == SessionState::LIVE ||
               _state == SessionState::LOADING);
        break;
    case SessionState::WAIT_DISCONNECT:
        assert(_state == SessionState::LOADING ||
               _state == SessionState::LIVE);
        break;
    }
    _state = newState;
    _lastStateTime = std::chrono::steady_clock::now();
}

bool ClientSession::disconnectFromKit()
{
    assert(_state != SessionState::WAIT_DISCONNECT);
    auto docBroker = getDocumentBroker();
    if (docBroker && (_state == SessionState::LIVE || _state == SessionState::LOADING))
    {
        setState(SessionState::WAIT_DISCONNECT);

        // handshake nicely; so wait for 'disconnected'
        LOG_TRC("Sending 'disconnect' command to session " << getId());
        docBroker->forwardToChild(client_from_this(), "disconnect");

        return false;
    }

    return true; // just get on with it
}

// Allow 20secs for the clipboard and disconnection to come.
bool ClientSession::staleWaitDisconnect(const std::chrono::steady_clock::time_point now)
{
    return _state == SessionState::WAIT_DISCONNECT &&
           (now - _lastStateTime) >= std::chrono::seconds(20);
}

void ClientSession::rotateClipboardKey(bool notifyClient)
{
    if (_state == SessionState::WAIT_DISCONNECT)
        return;

    _clipboardKeys[1] = _clipboardKeys[0];
    _clipboardKeys[0] = Util::rng::getHexString(
        ClipboardTokenLengthBytes);
    LOG_TRC("Clipboard key on [" << getId() << "] set to " << _clipboardKeys[0] <<
            " last was " << _clipboardKeys[1]);
    if (notifyClient)
        sendTextFrame("clipboardkey: " + _clipboardKeys[0]);
}

std::string ClientSession::getClipboardURI(bool encode)
{
    if (_wopiFileInfo && _wopiFileInfo->getDisableCopy())
        return std::string();

    return createPublicURI("clipboard", _clipboardKeys[0], encode);
}

std::string ClientSession::createPublicURI(const std::string& subPath, const std::string& tag, bool encode)
{
    Poco::URI wopiSrc = getDocumentBroker()->getPublicUri();
    wopiSrc.setQueryParameters(Poco::URI::QueryParameters());

    const std::string encodedFrom = Uri::encode(wopiSrc.toString());

    std::string meta = _serverURL.getSubURLForEndpoint(
        "/cool/" + subPath + "?WOPISrc=" + encodedFrom +
        "&ServerId=" + Util::getProcessIdentifier() +
        "&ViewId=" + std::to_string(getKitViewId()) +
        "&Tag=" + tag);

#if !MOBILEAPP
    if (!COOLWSD::RouteToken.empty())
        meta += "&RouteToken=" + COOLWSD::RouteToken;
#endif // !MOBILEAPP

    if (!encode)
        return meta;

    return Uri::encode(meta);
}

bool ClientSession::matchesClipboardKeys(const std::string &/*viewId*/, const std::string &tag)
{
    if (tag.empty())
    {
        LOG_ERR("Invalid, empty clipboard tag");
        return false;
    }

    // FIXME: check viewId for paranoia if we can.
    return std::any_of(std::begin(_clipboardKeys), std::end(_clipboardKeys),
                       [&tag](const std::string& it) { return it == tag; });
}

// Rewrite path to be visible to the outside world
static std::string getLocalPathToJail(std::string filePath, const DocumentBroker& docBroker)
{
#if !MOBILEAPP
    // Prepend the jail path in the normal (non-nocaps) case
    if (!COOLWSD::NoCapsForKit)
    {
        if (filePath.size() > 0 && filePath[0] == '/')
            filePath = filePath.substr(1);

        const Path path(FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces,
                                                       docBroker.getJailRoot(),
                                                       filePath));
        if (Poco::File(path).exists())
            filePath = path.toString();
        else
        {
            // Blank for failure.
            filePath.clear();
        }
    }
#else
    (void)docBroker;
#endif
    return filePath;
}

void ClientSession::handleClipboardRequest(DocumentBroker::ClipboardRequest     type,
                                           const std::shared_ptr<StreamSocket> &socket,
                                           const std::string                   &tag,
                                           const std::string                   &clipFile)
{
    // Move the socket into our DocBroker.
    auto docBroker = getDocumentBroker();
    docBroker->addSocketToPoll(socket);

    if (_state == SessionState::WAIT_DISCONNECT)
    {
        LOG_TRC("Clipboard request " << tag << " for disconnecting session");
        if (DocumentBroker::handlePersistentClipboardRequest(type, socket, tag, false))
            return; // the getclipboard already completed.
        if (type == DocumentBroker::CLIP_REQUEST_SET)
        {
            if constexpr (!Util::isMobileApp())
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        }
        else // will be handled during shutdown
        {
            LOG_TRC("Clipboard request " << tag << " queued for shutdown");
            _clipSockets.push_back(socket);
        }
    }

    std::string specific;
    if (type == DocumentBroker::CLIP_REQUEST_GET_RICH_HTML_ONLY)
        specific = " mimetype=text/html";
    else if (type == DocumentBroker::CLIP_REQUEST_GET_HTML_PLAIN_ONLY)
    {
        specific = " mimetype=text/html,text/plain;charset=utf-8";
    }

    if (type != DocumentBroker::CLIP_REQUEST_SET)
    {
        if (_wopiFileInfo && _wopiFileInfo->getDisableCopy())
        {
            // Unsupported clipboard request.
            LOG_ERR("Unsupported Clipboard Request from socket #" << socket->getFD()
                                                                  << ". Terminating connection.");

            HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
            return;
        }

        LOG_TRC("Session [" << getId() << "] sending getclipboard name=" << tag << specific);
        docBroker->forwardToChild(client_from_this(), "getclipboard name=" + tag + specific);
        _clipSockets.push_back(socket);
    }
    else // REQUEST_SET
    {
        LOG_TRC("Session [" << getId() << "] sending setclipboard");
        std::string jailClipFile = getLocalPathToJail(clipFile, *docBroker);
        if (!jailClipFile.empty())
        {
            std::string preProcessedClipFile = jailClipFile + ".preproc";
            std::ofstream ofs(preProcessedClipFile, std::ofstream::out);
            std::ifstream ifs(jailClipFile, std::ifstream::in);
            bool preProcesed = preProcessSetClipboardPayload(ifs, ofs);
            ifs.close();
            ofs.close();

            if (!preProcesed)
                FileUtil::removeFile(preProcessedClipFile);
            else
            {
                if (::rename(preProcessedClipFile.c_str(), jailClipFile.c_str()) < 0)
                {
                    LOG_SYS("Failed to rename [" << preProcessedClipFile << "] to [" << jailClipFile << ']');
                }
                else
                {
                    LOG_TRC("Renamed [" << preProcessedClipFile << "] to [" << jailClipFile << ']');
                }
            }

#if !MOBILEAPP
            ifs.open(jailClipFile, std::ifstream::in);
            if (ifs.get() == '{')
            {
                ifs.seekg(0, std::ios_base::beg);

                // We got JSON, extract the URL and the UNO command name.
                Poco::JSON::Object::Ptr json;

                Poco::JSON::Parser parser;
                try
                {
                    const Poco::Dynamic::Var result = parser.parse(ifs);
                    json = result.extract<Poco::JSON::Object::Ptr>();
                }
                catch (const Poco::JSON::JSONException& exception)
                {
                    LOG_WRN("parseJSON: failed to parse '" << jailClipFile << "': '" << exception.what() << "'");
                }

                ifs.close();

                if (json)
                {
                    std::string url;
                    JsonUtil::findJSONValue(json, "url", url);
                    std::string commandName;
                    JsonUtil::findJSONValue(json, "commandName", commandName);
                    http::Session::FinishedCallback finishedCallback =
                        [selfWeak = weak_from_this(), this, commandName=std::move(commandName),
                         docBroker, jailClipFile, clipFile](const std::shared_ptr<http::Session>& session)
                    {
                        std::shared_ptr<MessageHandlerInterface> selfLifecycle = selfWeak.lock();
                        if (!selfLifecycle)
                        {
                            LOG_ERR_S("Session that requested: " << clipFile << " has already ended.");
                            if (UnitWSD::isUnitTesting())
                                UnitWSD::get().onClipboardDownloadSessionGone();
                            return;
                        }

                        const std::shared_ptr<const http::Response> httpResponse =
                            session->response();
                        if (httpResponse->statusLine().statusCode() != http::StatusCode::OK)
                        {
                            LOG_ERR("Clipboard download request failed");
                            return;
                        }

                        // Check if this is likely produced by us.
                        std::string clipboardHeader = httpResponse->get("X-COOL-Clipboard");
                        if (clipboardHeader != "true")
                        {
                            LOG_ERR("Clipboard response is missing the required 'X-COOL-Clipboard: true' header");
                            return;
                        }

                        std::ifstream stream(jailClipFile, std::ifstream::in);
                        const bool ownFormat = ClipboardData::isOwnFormat(stream);
                        stream.close();
                        if (ownFormat)
                        {
                            docBroker->forwardToChild(client_from_this(), "setclipboard name=" + clipFile, true);
                            docBroker->forwardToChild(client_from_this(), "uno " + commandName);
                        }
                        else
                        {
                            LOG_ERR("Clipboard download: unexpected data format");
                            return;
                        }
                    };

                    if (UnitWSD::isUnitTesting())
                        UnitWSD::get().filterClipboardDownloadURL(url);

                    const std::string pathAndQuery = Poco::URI(url).getPathAndQuery();
                    if (pathAndQuery.find("/cool/clipboard") != std::string::npos)
                    {
                        std::shared_ptr<http::Session> httpSession = http::Session::create(url);
                        if (httpSession)
                        {
                            httpSession->setFinishedHandler(std::move(finishedCallback));

                            http::Session::ConnectFailCallback connectFailCallback = [this, url](const std::shared_ptr<http::Session>& /* session */)
                            {
                                LOG_ERR(
                                    "Failed to start an async clipboard download request with URL ["
                                    << url << ']');
                            };
                            httpSession->setConnectFailHandler(std::move(connectFailCallback));
                            http::Request httpRequest(Poco::URI(url).getPathAndQuery());
                            httpSession->asyncRequest(httpRequest, docBroker->getPoll());

                            if (UnitWSD::isUnitTesting())
                                UnitWSD::get().onClipboardDownloadRequest(httpSession);

                            const std::shared_ptr<http::Response> httpResponse = httpSession->response();
                            httpResponse->saveBodyToFile(jailClipFile);
                        }
                        else
                        {
                            LOG_ERR("Failed to create clipboard request with URL [" << url << ']');
                        }
                    }
                    else
                    {
                        LOG_ERR("Clipboard download URL ["
                                << url << "] does not look like a clipboard one");
                    }
                }
            }
            else
#endif
            {
                // List of mimetype-size-data tuples, pass that over as-is.
                docBroker->forwardToChild(client_from_this(), "setclipboard name=" + clipFile, true);
            }

            // FIXME: work harder for error detection ?
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setContentLength(0);
            httpResponse.set("Connection", "close");
            socket->send(httpResponse);
            socket->asyncShutdown();
        }
        else
        {
            LOG_DBG("clipboardcontent produced no output in '" << clipFile << "'");
            if constexpr (!Util::isMobileApp())
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        }
    }
}

void ClientSession::onTileProcessed(TileWireId wireId)
{
    auto iter = std::find_if(_tilesOnFly.begin(), _tilesOnFly.end(),
    [wireId](const std::pair<TileWireId, std::chrono::steady_clock::time_point>& curTile)
    {
        return curTile.first == wireId;
    });

    if(iter != _tilesOnFly.end())
        _tilesOnFly.erase(iter);
    else
        LOG_INF("Tileprocessed message with an unknown wire-id '" << wireId << "' from session " << getId());
}

#if !MOBILEAPP
namespace
{
std::shared_ptr<http::Session>
makeSignatureActionSession(std::shared_ptr<ClientSession> clientSession,
                           std::string commandName, const std::string& requestUrl)
{
    // Create the session and set a finished callback
    std::shared_ptr<http::Session> httpSession = http::Session::create(requestUrl);
    if (!httpSession)
    {
        LOG_WRN("SignatureAction: failed to create HTTP session");
        return nullptr;
    }

    http::Session::FinishedCallback finishedCallback =
        [clientSession = std::move(clientSession),
         commandName = std::move(commandName)](const std::shared_ptr<http::Session>& session)
    {
        const std::shared_ptr<const http::Response> httpResponse = session->response();
        Poco::JSON::Object::Ptr resultArguments = new Poco::JSON::Object();
        resultArguments->set("commandName", commandName);

        bool ok = httpResponse->statusLine().statusCode() == http::StatusCode::OK;
        resultArguments->set("success", ok);

        const std::string& responseBody = httpResponse->getBody();
        Poco::JSON::Object::Ptr responseBodyObject = new Poco::JSON::Object();
        if (!JsonUtil::parseJSON(responseBody, responseBodyObject))
        {
            LOG_WRN("SignatureAction: failed to parse response body as JSON");
            return;
        }
        resultArguments->set("result", responseBodyObject);

        std::ostringstream oss;
        resultArguments->stringify(oss);
        std::string result = "unocommandresult: " + oss.str();
        clientSession->sendTextFrame(result);
    };
    httpSession->setFinishedHandler(std::move(finishedCallback));
    return httpSession;
}
} // namespace

void ClientSession::sendAIChatResult(bool success, const std::string& text,
                                     const std::string& requestId)
{
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("success", success);
    if (success)
        result->set("content", text);
    else
        result->set("error", text);
    result->set("requestId", requestId);

    std::ostringstream oss;
    result->stringify(oss);
    sendTextFrame("aichatresult: " + oss.str());
}

std::string ClientSession::mapAIHttpStatusToError(
    http::StatusCode statusCode, const std::string& reasonPhrase,
    const std::string& context)
{
    switch (statusCode)
    {
        case http::StatusCode::BadRequest:
            return context.empty() ? "Invalid request"
                                   : "Invalid " + context + " request";
        case http::StatusCode::Unauthorized:     return "Invalid API key";
        case http::StatusCode::Forbidden:        return "API key lacks permissions";
        case http::StatusCode::TooManyRequests:  return "Rate limited - please wait a moment and retry";
        case http::StatusCode::InternalServerError: return "API server error - try again later";
        case http::StatusCode::ServiceUnavailable:  return "Service temporarily unavailable";
        default:
        {
            std::string err = "API error (";
            err.append(std::to_string(static_cast<int>(statusCode)));
            err.append("): ");
            err.append(reasonPhrase);
            return err;
        }
    }
}

static const std::string AI_SYSTEM_PROMPT =
    "You are a helpful assistant for Collabora Online. "
    "Help users with their documents — answering questions, suggesting edits, "
    "rewriting text, and more. When the user shares selected text from their document, "
    "provide relevant help with that text. When no selected text is provided, answer "
    "general questions about documents, formatting, writing, and the application. "
    "When providing rewritten or edited text, return it in markdown format preserving "
    "the original formatting structure. IMPORTANT: Return the markdown text directly "
    "without wrapping it in code fences (do NOT use ```markdown or ``` blocks). "
    "Just return the raw markdown content. Be concise and helpful.";

namespace
{

/// Helper to create an OpenAI function-calling tool object.
Poco::JSON::Object::Ptr makeAITool(const std::string& name,
                                    const std::string& description,
                                    Poco::JSON::Object::Ptr parameters)
{
    Poco::JSON::Object::Ptr fn = new Poco::JSON::Object();
    fn->set("name", name);
    fn->set("description", description);
    fn->set("parameters", parameters);

    Poco::JSON::Object::Ptr tool = new Poco::JSON::Object();
    tool->set("type", "function");
    tool->set("function", fn);
    return tool;
}

/// Helper to create an OpenAI function parameter schema object.
Poco::JSON::Object::Ptr makeParamSchema(
    std::initializer_list<std::pair<std::string, std::pair<std::string, std::string>>> props,
    std::initializer_list<std::string> required)
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    for (const auto& p : props)
    {
        Poco::JSON::Object::Ptr prop = new Poco::JSON::Object();
        prop->set("type", p.second.first);
        prop->set("description", p.second.second);
        properties->set(p.first, prop);
    }

    Poco::JSON::Array::Ptr reqArr = new Poco::JSON::Array();
    for (const auto& r : required)
        reqArr->add(r);

    Poco::JSON::Object::Ptr schema = new Poco::JSON::Object();
    schema->set("type", "object");
    schema->set("properties", properties);
    schema->set("required", reqArr);
    return schema;
}

} // anonymous namespace

Poco::JSON::Array::Ptr ClientSession::buildAIToolDefinitions() const
{
    Poco::JSON::Array::Ptr tools = new Poco::JSON::Array();

    // generate_image - existing tool
    tools->add(makeAITool(
        "generate_image",
        "Generate an image based on the user's description. Call this when the "
        "user asks to create, draw, generate, sketch, or make an image or picture.",
        makeParamSchema(
            {{"prompt", {"string", "A detailed description of the image to generate"}}},
            {"prompt"})));

    // extract_document_structure - inspect the open document
    tools->add(makeAITool(
        "extract_document_structure",
        DocumentToolDescriptions::EXTRACT_DOC_STRUCTURE_DESCRIPTION,
        makeParamSchema(
            {{"filter", {"string",
                "Filter results to a specific structure type. "
                "For Impress: 'slides'. For Writer: 'contentcontrol'. "
                "Omit to get the full structure."}}},
            {})));

    // transform_document_structure - modify the open document
    tools->add(makeAITool(
        "transform_document_structure",
        std::string(
            "Transform the currently-open document's structure using a JSON command "
            "sequence. Supports Impress slide operations, Writer/Calc content control "
            "updates, and arbitrary UNO commands.\n\n")
            + DocumentToolDescriptions::TRANSFORM_PARAM_DESCRIPTION,
        makeParamSchema(
            {{"transform", {"string", "JSON transformation commands"}},
             {"summary", {"string",
                "Markdown summary of the changes for the user to review before "
                "approving. List each slide with its title and "
                "key content points."}}},
            {"transform"})));

    // extract_link_targets - get link targets from the open document
    tools->add(makeAITool(
        "extract_link_targets",
        DocumentToolDescriptions::EXTRACT_LINK_TARGETS_DESCRIPTION,
        makeParamSchema({}, {})));

    return tools;
}

bool ClientSession::handleAIChatAction(const std::string& firstLine)
{
    static constexpr size_t MAX_AI_PAYLOAD_SIZE = 5 * 1024 * 1024; // 5MB
    static constexpr size_t MAX_AI_MESSAGE_LENGTH = 100 * 1024; // 100KB per message
    static constexpr unsigned MAX_AI_MESSAGES = 50;

    // Extract JSON payload after "aichat: "
    const std::string jsonPayload = firstLine.substr(strlen("aichat: "));

    if (jsonPayload.size() > MAX_AI_PAYLOAD_SIZE)
    {
        sendAIChatResult(false, "Request too large", "");
        return true;
    }

    Poco::JSON::Object::Ptr requestObj = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(jsonPayload, requestObj))
    {
        sendAIChatResult(false, "Invalid request format", "");
        return true;
    }

    std::string requestId;
    JsonUtil::findJSONValue(requestObj, "requestId", requestId);

    std::string docType;
    JsonUtil::findJSONValue(requestObj, "docType", docType);

    if (docType != "text" && docType != "spreadsheet" && docType != "presentation" && docType != "drawing")
        docType.clear();

    Poco::JSON::Array::Ptr messages = requestObj->getArray("messages");
    if (!messages || messages->size() == 0)
    {
        sendAIChatResult(false, "No messages provided", requestId);
        return true;
    }

    // Build system prompt with document-type context
    std::string systemPrompt = AI_SYSTEM_PROMPT;
    if (!docType.empty())
        systemPrompt += " You are currently working with a " + docType + " document.";

    if (docType == "spreadsheet")
        systemPrompt +=
            " When referencing specific spreadsheet cells in your responses, "
            "format them as clickable links using this pattern: [B2](cell://B2), "
            "where the column letters come from the header row and the row number from the Row column.";

    if (docType == "presentation")
        systemPrompt +=
            " When creating or modifying slides, you MUST format every slide:"
            " bold every title using EditTextObject with .uno:Bold,"
            " apply .uno:DefaultBullet to content placeholders that list multiple items,"
            " and choose the most appropriate layout for each slide's content from the"
            " Available layouts list in the tool description."
            " Do not use the same layout for every slide."
            " Do not prefix lines with '- ' when using DefaultBullet (the bullet marker"
            " is added automatically)."
            " In content placeholders, put only the items to be bulleted. Do not add"
            " sub-headings or blank lines before the bullet items."
            " When calling transform_document_structure, include a 'summary' parameter"
            " with a markdown preview of ONLY the slides being created or modified in"
            " this transform call, not pre-existing slides.";

    Poco::JSON::Array::Ptr sanitizedMessages = new Poco::JSON::Array();

    for (unsigned i = 0; i < messages->size(); ++i)
    {
        auto msg = messages->getObject(i);
        if (!msg)
            continue;

        std::string role;
        JsonUtil::findJSONValue(msg, "role", role);

        // Only allow user and assistant roles
        if (role != "user" && role != "assistant")
            continue;

        std::string content;
        JsonUtil::findJSONValue(msg, "content", content);
        if (content.size() > MAX_AI_MESSAGE_LENGTH)
        {
            sendAIChatResult(false, "Message too long", requestId);
            return true;
        }

        sanitizedMessages->add(msg);
    }

    // Check whether the last user message includes selected text from the document.
    bool hasSelectedText = false;
    for (int i = static_cast<int>(messages->size()) - 1; i >= 0; --i)
    {
        auto msg = messages->getObject(i);
        if (!msg)
            continue;
        std::string role;
        JsonUtil::findJSONValue(msg, "role", role);
        if (role == "user")
        {
            std::string content;
            JsonUtil::findJSONValue(msg, "content", content);
            if (content.find("[Selected text from document:") != std::string::npos)
                hasSelectedText = true;
            break;
        }
    }

    systemPrompt +=
        " You have tools to inspect and modify the document."
        " Use transform_document_structure to make changes.";

    if (hasSelectedText)
        systemPrompt +=
            " The user has shared selected text from the document as context."
            " Use that context directly to answer their question or make changes."
            " Only call extract_document_structure if you need information about"
            " parts of the document beyond the selection.";
    else
        systemPrompt +=
            " Use extract_document_structure when you need to understand the"
            " existing document layout before making changes."
            " When the user asks you to create new content from scratch (like a table"
            " or text), just generate it directly without extracting first.";

    // Prepend system message: build a new array with system first, then the rest.
    Poco::JSON::Array::Ptr finalMessages = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr systemMsg = new Poco::JSON::Object();
    systemMsg->set("role", "system");
    systemMsg->set("content", systemPrompt);
    finalMessages->add(systemMsg);
    for (unsigned i = 0; i < sanitizedMessages->size(); ++i)
        finalMessages->add(sanitizedMessages->get(i));
    sanitizedMessages = finalMessages;

    // Trim to most recent messages if over limit (keep system prompt at index 0)
    while (sanitizedMessages->size() > MAX_AI_MESSAGES + 1)
        sanitizedMessages->remove(1);

    // Get AI provider settings
    const std::string apiKey = getAIProviderAPIKey();
    const std::string model = getAIProviderModel();
    std::string baseUrl = getAIProviderURL();

    if (apiKey.empty() || model.empty() || baseUrl.empty())
    {
        sendAIChatResult(false, "AI settings not configured", requestId);
        return true;
    }

    if (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();

    LOG_DBG("AIChatAction: request [" << requestId << "] with "
            << sanitizedMessages->size() << " messages, model: " << model);

    std::string requestUrl = std::move(baseUrl);
    requestUrl.append("/v1/chat/completions");

    // Initialize the tool loop state
    _aiToolLoop = std::make_unique<AIToolLoopState>();
    _aiToolLoop->requestId = requestId;
    _aiToolLoop->messages = sanitizedMessages;
    _aiToolLoop->model = model;
    _aiToolLoop->requestUrl = requestUrl;
    _aiToolLoop->apiKey = apiKey;

    callLLMAPI();
    return true;
}

void ClientSession::callLLMAPI()
{
    if (!_aiToolLoop)
        return;

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("model", _aiToolLoop->model);
    payload->set("messages", _aiToolLoop->messages);
    payload->set("tools", buildAIToolDefinitions());

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);
    std::string payloadStr = payloadStream.str();

    std::shared_ptr<http::Session> httpSession =
        http::Session::create(_aiToolLoop->requestUrl);
    if (!httpSession)
    {
        LOG_WRN("AIToolLoop: failed to create HTTP session");
        sendAIChatResult(false, "Failed to create HTTP session", _aiToolLoop->requestId);
        _aiToolLoop.reset();
        return;
    }

    httpSession->setTimeout(std::chrono::seconds(60));

    auto clientSessionPtr = client_from_this();

    http::Session::FinishedCallback finishedCallback =
        [clientSessionPtr](const std::shared_ptr<http::Session>& session)
    {
        clientSessionPtr->_activeAIChatSession.reset();

        if (!clientSessionPtr->_aiToolLoop)
            return;

        const std::string& requestId = clientSessionPtr->_aiToolLoop->requestId;
        const std::shared_ptr<const http::Response> httpResponse = session->response();
        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

        if (statusCode == http::StatusCode::None)
        {
            clientSessionPtr->sendAIChatResult(false, "Request timeout", requestId);
            clientSessionPtr->_aiToolLoop.reset();
            return;
        }

        if (statusCode != http::StatusCode::OK)
        {
            const std::string& body = httpResponse->getBody();
            std::cerr << "AIToolLoop: API returned "
                << static_cast<int>(statusCode) << ' '
                << httpResponse->statusLine().reasonPhrase()
                << " body: " << body.substr(0, 500) << std::endl;
            const std::string errorMessage = mapAIHttpStatusToError(
                statusCode, httpResponse->statusLine().reasonPhrase());
            clientSessionPtr->sendAIChatResult(false, errorMessage, requestId);
            clientSessionPtr->_aiToolLoop.reset();
            return;
        }

        clientSessionPtr->handleLLMResponse(httpResponse->getBody());
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));

    http::Session::ConnectFailCallback connectFailCallback =
        [clientSessionPtr](const std::shared_ptr<http::Session>& /*session*/)
    {
        clientSessionPtr->_activeAIChatSession.reset();
        if (clientSessionPtr->_aiToolLoop)
        {
            clientSessionPtr->sendAIChatResult(
                false, "Network error - please check your connection",
                clientSessionPtr->_aiToolLoop->requestId);
            clientSessionPtr->_aiToolLoop.reset();
        }
    };
    httpSession->setConnectFailHandler(std::move(connectFailCallback));

    http::Request httpRequest(Poco::URI(_aiToolLoop->requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    std::string authHeader = "Bearer ";
    authHeader.append(_aiToolLoop->apiKey);
    httpRequest.set("Authorization", std::move(authHeader));
    httpRequest.setBody(std::move(payloadStr), "application/json");

    LOG_DBG("AIToolLoop: sending request [" << _aiToolLoop->requestId
            << "] round " << (6 - _aiToolLoop->toolRoundsRemaining)
            << " to " << _aiToolLoop->requestUrl);

    _activeAIChatSession = httpSession;
    std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
    httpSession->asyncRequest(httpRequest, docBroker->getPoll());
}

void ClientSession::handleLLMResponse(const std::string& responseBody)
{
    if (!_aiToolLoop)
        return;

    const std::string& requestId = _aiToolLoop->requestId;

    Poco::JSON::Object::Ptr responseObject = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(responseBody, responseObject))
    {
        sendAIChatResult(false, "No response from AI", requestId);
        _aiToolLoop.reset();
        return;
    }

    Poco::JSON::Array::Ptr choices = responseObject->getArray("choices");
    if (!choices || choices->size() == 0)
    {
        sendAIChatResult(false, "No response from AI", requestId);
        _aiToolLoop.reset();
        return;
    }

    Poco::JSON::Object::Ptr choice = choices->getObject(0);
    if (!choice)
    {
        sendAIChatResult(false, "No response from AI", requestId);
        _aiToolLoop.reset();
        return;
    }

    std::string finishReason;
    JsonUtil::findJSONValue(choice, "finish_reason", finishReason);

    Poco::JSON::Object::Ptr message = choice->getObject("message");
    if (!message)
    {
        sendAIChatResult(false, "No response from AI", requestId);
        _aiToolLoop.reset();
        return;
    }

    // Check for tool calls
    Poco::JSON::Array::Ptr toolCalls = message->getArray("tool_calls");
    if (toolCalls && toolCalls->size() > 0)
    {
        if (_aiToolLoop->toolRoundsRemaining <= 0)
        {
            sendAIChatResult(false, "AI used too many tool steps", requestId);
            _aiToolLoop.reset();
            return;
        }
        --_aiToolLoop->toolRoundsRemaining;

        // Append the assistant message (with tool_calls) to the conversation
        _aiToolLoop->messages->add(message);

        // Capture the AI's text content so it can be shown in approval dialogs.
        std::string assistantContent;
        JsonUtil::findJSONValue(message, "content", assistantContent);
        if (!assistantContent.empty())
            _aiToolLoop->pendingSummary = assistantContent;

        // Queue all tool calls for sequential processing
        _aiToolLoop->pendingToolCalls.clear();
        for (unsigned i = 0; i < toolCalls->size(); ++i)
        {
            Poco::JSON::Object::Ptr call = toolCalls->getObject(i);
            if (!call)
                continue;

            PendingToolCall pending;
            JsonUtil::findJSONValue(call, "id", pending.toolCallId);

            Poco::JSON::Object::Ptr fn = call->getObject("function");
            if (!fn)
                continue;

            JsonUtil::findJSONValue(fn, "name", pending.functionName);
            JsonUtil::findJSONValue(fn, "arguments", pending.arguments);
            _aiToolLoop->pendingToolCalls.push_back(std::move(pending));
        }

        // Start processing the first queued tool call
        processNextPendingToolCall();
        return;
    }

    // No tool calls - this is the final text response
    std::string result;
    std::string reasoning;
    JsonUtil::findJSONValue(message, "content", result);
    JsonUtil::findJSONValue(message, "reasoning", reasoning);

    if (result.empty())
    {
        if (!reasoning.empty())
        {
            sendAIChatResult(false,
                "This model returned only internal reasoning and no output. Try a "
                "different model or shorter input.", requestId);
        }
        else if (finishReason == "length")
        {
            sendAIChatResult(false,
                "The model ran out of tokens before producing output. Try a "
                "shorter input or a model with a larger output budget.", requestId);
        }
        else
        {
            sendAIChatResult(false, "No response from AI", requestId);
        }
        _aiToolLoop.reset();
        return;
    }

    sendAIChatResult(true, result, requestId);
    _aiToolLoop.reset();
}

bool ClientSession::executeAIToolCall(const std::string& toolCallId,
                                       const std::string& fnName,
                                       const std::string& argsJson)
{
    if (!_aiToolLoop)
        return false;

    const std::string requestId = _aiToolLoop->requestId;

    // generate_image - delegate to existing handler (terminates tool loop)
    if (fnName == "generate_image")
    {
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        std::string imagePrompt;
        if (JsonUtil::parseJSON(argsJson, argsObj))
            JsonUtil::findJSONValue(argsObj, "prompt", imagePrompt);

        if (imagePrompt.empty())
        {
            sendAIChatResult(false, "Image generation failed: no prompt from model", requestId);
            _aiToolLoop.reset();
            return true;
        }

        _aiToolLoop.reset(); // image generation is terminal
        handleAIImageGeneration(imagePrompt, requestId);
        return true;
    }

    std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
    if (!docBroker)
    {
        sendAIChatResult(false, "Document not available", requestId);
        _aiToolLoop.reset();
        return true;
    }

    // extract_document_structure - requires user approval
    if (fnName == "extract_document_structure")
    {
        std::string filter;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(argsJson, argsObj))
            JsonUtil::findJSONValue(argsObj, "filter", filter);

        std::string command = "extractdocumentstructure url=interactive";
        if (!filter.empty())
            command += " filter=" + filter;

        _aiToolLoop->awaitingApproval = true;
        _aiToolLoop->pendingToolCallId = toolCallId;
        _aiToolLoop->pendingToolName = fnName;
        _aiToolLoop->pendingForwardCommand = command;

        sendAIToolApproval(fnName, "");
        return true;
    }

    // extract_link_targets - read-only, send to kit
    if (fnName == "extract_link_targets")
    {
        _aiToolLoop->awaitingKitResponse = true;
        _aiToolLoop->pendingToolCallId = toolCallId;
        _aiToolLoop->pendingToolName = fnName;

        sendAIToolProgress(fnName, "Extracting link targets...");
        forwardToChild("extractlinktargets url=interactive", docBroker);
        return true;
    }

    // transform_document_structure - requires user approval
    if (fnName == "transform_document_structure")
    {
        std::string transform;
        std::string summary;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(argsJson, argsObj))
        {
            JsonUtil::findJSONValue(argsObj, "transform", transform);
            JsonUtil::findJSONValue(argsObj, "summary", summary);
        }

        if (transform.empty())
        {
            continueAIToolLoop(toolCallId,
                "{\"error\":\"No transform parameter provided\"}");
            return true;
        }

        // findJSONValue resolves JSON escapes, so literal control characters
        // (newlines, tabs) may appear inside string values. Escape them so
        // the inner JSON is valid, then re-serialize through Poco for a
        // clean string that core can parse.
        {
            std::string sanitized;
            sanitized.reserve(transform.size());
            bool inStr = false;
            for (std::size_t i = 0; i < transform.size(); ++i)
            {
                char c = transform[i];
                if (c == '"' && (i == 0 || transform[i - 1] != '\\'))
                    inStr = !inStr;
                if (inStr)
                {
                    if (c == '\n') { sanitized += "\\n"; continue; }
                    if (c == '\r') { sanitized += "\\r"; continue; }
                    if (c == '\t') { sanitized += "\\t"; continue; }
                }
                sanitized += c;
            }
            transform = sanitized;
        }

        Poco::JSON::Object::Ptr transformObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(transform, transformObj))
        {
            std::ostringstream oss;
            transformObj->stringify(oss);
            transform = oss.str();
        }
        else
        {
            continueAIToolLoop(toolCallId,
                "{\"error\":\"Invalid JSON in transform parameter. "
                "All slides must be in a single SlideCommands array within one "
                "Transforms object. Use InsertMasterSlide to add slides within "
                "the same array.\"}");
            return true;
        }

        // Navigation-only transforms (JumpToSlide) do not modify the document
        // and can be executed without user approval.
        bool navigationOnly = false;
        {
            Poco::JSON::Object::Ptr transforms = transformObj->getObject("Transforms");
            Poco::JSON::Array::Ptr cmds =
                transforms ? transforms->getArray("SlideCommands") : nullptr;
            if (cmds && cmds->size() > 0)
            {
                navigationOnly = true;
                for (unsigned i = 0; i < cmds->size(); ++i)
                {
                    Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
                    if (!cmd ||
                        !(cmd->has("JumpToSlide") || cmd->has("JumpToSlideByName")))
                    {
                        navigationOnly = false;
                        break;
                    }
                }
            }
        }

        if (navigationOnly)
        {
            _aiToolLoop->pendingToolCallId = toolCallId;
            _aiToolLoop->pendingToolName = fnName;
            _aiToolLoop->awaitingKitResponse = true;

            std::string encodedTransform;
            Poco::URI::encode(transform, "", encodedTransform);
            forwardToChild(
                "transformdocumentstructure url=interactive transform=" + encodedTransform,
                docBroker);
            return true;
        }

        _aiToolLoop->awaitingApproval = true;
        _aiToolLoop->pendingToolCallId = toolCallId;
        _aiToolLoop->pendingToolName = fnName;
        _aiToolLoop->pendingTransformArgs = transform;
        _aiToolLoop->pendingSummary = summary;

        sendAIToolApproval(fnName, transform);
        return true;
    }

    // Unknown tool - feed error back to LLM
    continueAIToolLoop(toolCallId, "{\"error\":\"Unknown tool: " + fnName + "\"}");
    return true;
}

void ClientSession::processNextPendingToolCall()
{
    if (!_aiToolLoop)
        return;

    if (_aiToolLoop->pendingToolCalls.empty())
    {
        // All tool calls processed, call LLM with all results
        sendAIToolProgress(_aiToolLoop->pendingToolName, "Thinking...");
        callLLMAPI();
        return;
    }

    PendingToolCall next = std::move(_aiToolLoop->pendingToolCalls.front());
    _aiToolLoop->pendingToolCalls.erase(_aiToolLoop->pendingToolCalls.begin());

    LOG_DBG("AIToolLoop: tool call [" << next.functionName << "] id=" << next.toolCallId
            << " for request [" << _aiToolLoop->requestId << ']');

    executeAIToolCall(next.toolCallId, next.functionName, next.arguments);
}

void ClientSession::continueAIToolLoop(const std::string& toolCallId,
                                        const std::string& result)
{
    if (!_aiToolLoop)
        return;

    // Append tool result message to the conversation
    Poco::JSON::Object::Ptr toolResult = new Poco::JSON::Object();
    toolResult->set("role", "tool");
    toolResult->set("tool_call_id", toolCallId);
    toolResult->set("content", result);
    _aiToolLoop->messages->add(toolResult);

    _aiToolLoop->awaitingKitResponse = false;
    _aiToolLoop->awaitingApproval = false;

    // Process the next queued tool call, or call LLM if all done
    processNextPendingToolCall();
}

void ClientSession::sendAIToolProgress(const std::string& toolName,
                                        const std::string& status)
{
    if (!_aiToolLoop)
        return;

    Poco::JSON::Object::Ptr progress = new Poco::JSON::Object();
    progress->set("requestId", _aiToolLoop->requestId);
    progress->set("toolName", toolName);
    progress->set("status", status);

    std::ostringstream oss;
    progress->stringify(oss);
    sendTextFrame("aichatprogress: " + oss.str());
}

void ClientSession::sendAIToolApproval(const std::string& toolName,
                                        const std::string& transformJson)
{
    if (!_aiToolLoop)
        return;

    Poco::JSON::Object::Ptr approval = new Poco::JSON::Object();
    approval->set("requestId", _aiToolLoop->requestId);
    approval->set("toolName", toolName);
    approval->set("transformJson", transformJson);
    if (!_aiToolLoop->pendingSummary.empty())
        approval->set("summary", _aiToolLoop->pendingSummary);

    std::ostringstream oss;
    approval->stringify(oss);
    sendTextFrame("aichatapproval: " + oss.str());
}

bool ClientSession::handleAIChatApprove(const std::string& firstLine)
{
    const std::string jsonPayload = firstLine.substr(strlen("aichatapprove: "));

    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(jsonPayload, obj))
    {
        LOG_WRN("AIChatApprove: invalid JSON");
        return true;
    }

    std::string action;
    JsonUtil::findJSONValue(obj, "action", action);

    if (!_aiToolLoop || !_aiToolLoop->awaitingApproval)
    {
        LOG_WRN("AIChatApprove: no pending approval");
        return true;
    }

    const std::string toolCallId = _aiToolLoop->pendingToolCallId;

    if (action == "approve")
    {
        std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
        if (!docBroker)
        {
            sendAIChatResult(false, "Document not available", _aiToolLoop->requestId);
            _aiToolLoop.reset();
            return true;
        }

        std::string command;
        if (!_aiToolLoop->pendingForwardCommand.empty())
        {
            // Generic forwarding (extract_document_structure, etc.)
            command = _aiToolLoop->pendingForwardCommand;
            _aiToolLoop->pendingForwardCommand.clear();
        }
        else
        {
            // transform_document_structure - URI-encode the transform JSON
            std::string encodedTransform;
            Poco::URI::encode(_aiToolLoop->pendingTransformArgs, "", encodedTransform);
            command = "transformdocumentstructure url=interactive transform=" + encodedTransform;
        }

        _aiToolLoop->awaitingApproval = false;
        _aiToolLoop->awaitingKitResponse = true;

        sendAIToolProgress(_aiToolLoop->pendingToolName, "Working...");
        forwardToChild(command, docBroker);
    }
    else
    {
        // User rejected - feed rejection back to LLM with tool-specific message
        _aiToolLoop->awaitingApproval = false;
        std::string rejectionMsg;
        if (_aiToolLoop->pendingToolName == "extract_document_structure")
            rejectionMsg =
                "{\"error\":\"User declined document inspection. "
                "Answer their request directly without inspecting the document. "
                "If the request is to create new content, just generate it.\"}";
        else
            rejectionMsg =
                "{\"error\":\"User rejected the document modification. "
                "Explain what you wanted to do and ask if they would like a different approach.\"}";
        continueAIToolLoop(toolCallId, rejectionMsg);
    }

    return true;
}

bool ClientSession::handleAIChatCancel(const std::string& firstLine)
{
    const std::string cancelRequestId = firstLine.substr(strlen("aichatcancel: "));
    LOG_DBG("AIChatCancel: cancelling request [" << cancelRequestId << ']');

    if (_activeAIChatSession)
    {
        _activeAIChatSession->asyncShutdown();
        _activeAIChatSession.reset();
    }

    _aiToolLoop.reset();

    return true;
}

bool ClientSession::handleAIImageGeneration(const std::string& prompt,
                                             const std::string& requestId)
{
    LOG_DBG("AIImageGeneration: request [" << requestId
            << "], prompt: " << prompt);

    // Get AI image provider settings (fall back to chat provider)
    std::string apiKey = getAIImageProviderAPIKey();
    if (apiKey.empty())
        apiKey = getAIProviderAPIKey();
    std::string baseUrl = getAIImageProviderURL();
    if (baseUrl.empty())
        baseUrl = getAIProviderURL();

    if (apiKey.empty() || baseUrl.empty())
    {
        sendAIChatResult(false, "AI settings not configured", requestId);
        return true;
    }

    if (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();

    std::string requestUrl = std::move(baseUrl);
    requestUrl.append("/v1/images/generations");

    // Build HTTP payload
    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("prompt", prompt);
    payload->set("size", "1024x1024");
    payload->set("n", 1);
    payload->set("response_format", "b64_json");

    const std::string imageModel = getAIImageModel();
    if (imageModel.empty())
    {
        sendAIChatResult(false, "Image model not configured", requestId);
        return true;
    }
    payload->set("model", imageModel);

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);
    std::string payloadStr = payloadStream.str();

    std::shared_ptr<http::Session> httpSession = http::Session::create(requestUrl);
    if (!httpSession)
    {
        LOG_WRN("AIImageGeneration: failed to create HTTP session");
        sendAIChatResult(false, "Failed to create HTTP session", requestId);
        return true;
    }

    httpSession->setTimeout(std::chrono::seconds(60));

    // Send image result via aichatresult with imageData field
    auto clientSessionPtr = client_from_this();
    auto sendImageResult = [clientSession = clientSessionPtr, requestId](
                               bool success, const std::string& imageData,
                               const std::string& error)
    {
        Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
        result->set("success", success);
        if (success)
            result->set("imageData", imageData);
        else
            result->set("error", error);
        result->set("requestId", requestId);

        std::ostringstream oss;
        result->stringify(oss);
        clientSession->sendTextFrame("aichatresult: " + oss.str());
    };

    http::Session::FinishedCallback finishedCallback =
        [clientSessionPtr, sendImageResult](const std::shared_ptr<http::Session>& session)
    {
        clientSessionPtr->_activeAIChatSession.reset();

        const std::shared_ptr<const http::Response> httpResponse = session->response();
        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

        if (statusCode == http::StatusCode::None)
        {
            sendImageResult(false, "", "Request timeout");
            return;
        }

        if (statusCode != http::StatusCode::OK)
        {
            const std::string errorMessage = mapAIHttpStatusToError(
                statusCode, httpResponse->statusLine().reasonPhrase(), "image");
            sendImageResult(false, "", errorMessage);
            return;
        }

        const std::string& responseBody = httpResponse->getBody();
        Poco::JSON::Object::Ptr responseObject = new Poco::JSON::Object();
        if (!JsonUtil::parseJSON(responseBody, responseObject))
        {
            sendImageResult(false, "", "Failed to parse image generation response");
            return;
        }

        Poco::JSON::Array::Ptr dataArray = responseObject->getArray("data");
        if (!dataArray || dataArray->size() == 0)
        {
            sendImageResult(false, "", "No image generated");
            return;
        }

        Poco::JSON::Object::Ptr firstItem = dataArray->getObject(0);
        if (!firstItem)
        {
            sendImageResult(false, "", "No image generated");
            return;
        }

        std::string b64Json;
        JsonUtil::findJSONValue(firstItem, "b64_json", b64Json);

        if (b64Json.empty())
        {
            sendImageResult(false, "", "No image data in response");
            return;
        }

        sendImageResult(true, b64Json, "");
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));

    http::Session::ConnectFailCallback connectFailCallback =
        [clientSessionPtr = std::move(clientSessionPtr),
         sendImageResult = std::move(sendImageResult)](
            const std::shared_ptr<http::Session>& /*session*/)
    {
        clientSessionPtr->_activeAIChatSession.reset();
        sendImageResult(false, "", "Network error - please check your connection");
    };
    httpSession->setConnectFailHandler(std::move(connectFailCallback));

    http::Request httpRequest(Poco::URI(requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    std::string authHeader = "Bearer ";
    authHeader.append(apiKey);
    httpRequest.set("Authorization", std::move(authHeader));
    httpRequest.setBody(std::move(payloadStr), "application/json");

    LOG_DBG("AIImageGeneration: sending request [" << requestId << "] to "
            << requestUrl << ", model: " << imageModel);

    _activeAIChatSession = httpSession;
    std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
    httpSession->asyncRequest(httpRequest, docBroker->getPoll());
    return true;
}

bool ClientSession::handleSignatureAction(const StringVector& tokens)
{
    // Make the HTTP session: this requires an URL
    Poco::JSON::Object::Ptr serverPrivateInfoObject = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(getServerPrivateInfo(), serverPrivateInfoObject))
    {
        LOG_WRN("SignatureAction: failed to parse server private info as JSON");
        return false;
    }
    std::string requestUrl;
    JsonUtil::findJSONValue(serverPrivateInfoObject, "ESignatureBaseUrl", requestUrl);
    std::string commandName = tokens[1];
    if (commandName == ".uno:PrepareSignature")
    {
        requestUrl += "/api/signatures/prepare-files-for-signing";
    }
    else if (commandName == ".uno:DownloadSignature")
    {
        requestUrl += "/api/signatures/download-signed-file";
    }
    std::shared_ptr<http::Session> httpSession =
        makeSignatureActionSession(client_from_this(), std::move(commandName), requestUrl);
    if (!httpSession)
    {
        return false;
    }

    // Make the request: this requires a JSON body, where we set the secret
    std::string commandArguments = tokens.cat(' ', 2);
    Poco::JSON::Object::Ptr commandArgumentsObject;
    if (!JsonUtil::parseJSON(commandArguments, commandArgumentsObject))
    {
        LOG_WRN("SignatureAction: failed to parse arguments as JSON");
        return false;
    }
    Poco::JSON::Object::Ptr requestBodyObject = commandArgumentsObject->getObject("body");
    if (!requestBodyObject)
    {
        LOG_WRN("SignatureAction: no body in arguments");
        return false;
    }
    std::string secret;
    JsonUtil::findJSONValue(serverPrivateInfoObject, "ESignatureSecret", secret);
    requestBodyObject->set("secret", secret);
    std::stringstream oss;
    requestBodyObject->stringify(oss);
    http::Request httpRequest(Poco::URI(requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.setBody(oss.str(), "application/json");
    std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
    httpSession->asyncRequest(httpRequest, docBroker->getPoll());
    return true;
}
#endif

bool ClientSession::_handleInput(const char *buffer, int length)
{
    LOG_TRC("handling incoming [" << getAbbreviatedMessage(buffer, length) << ']');
    const std::string firstLine = getFirstLine(buffer, length);
    const StringVector tokens = StringVector::tokenize(firstLine.data(), firstLine.size());

    std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
    if (!docBroker || docBroker->isMarkedToDestroy())
    {
        LOG_ERR("No DocBroker found, or DocBroker marked to be destroyed. Terminating session " << getName());
        return false;
    }

    if (tokens.size() < 1)
    {
        sendTextFrameAndLogError("error: cmd=empty kind=unknown");
        return false;
    }

    if (tokens.equals(0, "DEBUG"))
    {
        LOG_DBG("From client: " << std::string(buffer, length).substr(strlen("DEBUG") + 1));
        return false;
    }
    else if (tokens.equals(0, "ERROR"))
    {
        LOG_ERR("From client: " << std::string(buffer, length).substr(strlen("ERROR") + 1));
        return false;
    }
    else if (tokens.equals(0, "TRACEEVENT"))
    {
        if (COOLWSD::EnableTraceEventLogging)
        {
            if (_performanceCounterEpoch == 0)
            {
                static bool warnedOnce = false;
                if (!warnedOnce)
                {
                    LOG_WRN("For some reason the _performanceCounterEpoch is still zero, ignoring TRACEEVENT from cool as the timestamp would be garbage");
                    warnedOnce = true;
                }

                return false;
            }

            if (_performanceCounterEpoch < 1620000000000000ull ||
                _performanceCounterEpoch > 2000000000000000ull)
            {
                static bool warnedOnce = false;
                if (!warnedOnce)
                {
                    LOG_WRN("For some reason the _performanceCounterEpoch is bogus, ignoring TRACEEVENT from cool as the timestamp would be garbage");
                    warnedOnce = true;
                }

                return false;
            }

            if (tokens.size() >= 4)
            {
                // The intent is that when doing Trace Event generation, the web browser client and
                // the server run on the same machine, so there is no clock skew problem.
                std::string name;
                std::string ph;
                uint64_t ts;
                if (getTokenString(tokens[1], "name", name) &&
                    getTokenString(tokens[2], "ph", ph) &&
                    getTokenUInt64(tokens[3], "ts", ts))
                {
                    std::string args;
                    if (tokens.size() >= 5 && getTokenString(tokens, "args", args))
                        args = ",\"args\":" + args;

                    uint64_t id, tid;
                    uint64_t dur;
                    if (ph == "i")
                    {
                        COOLWSD::writeTraceEventRecording(
                            "{\"name\":" + name + R"(,"ph":"i")" + args + ",\"ts\":" +
                            std::to_string(ts + _performanceCounterEpoch) + ",\"pid\":" +
                            std::to_string(ProcUtil::getProcessId() + SYNTHETIC_COOL_PID_OFFSET) +
                            ",\"tid\":1},\n");
                    }
                    // Should the first getTokenUInt64()'s return value really
                    // be ignored?
                    else if ((ph == "S" || ph == "F") &&
                             (static_cast<void>(getTokenUInt64(tokens[4], "id", id)),
                             getTokenUInt64(tokens[5], "tid", tid)))
                    {
                        COOLWSD::writeTraceEventRecording(
                            "{\"name\":" + name + R"(,"ph":")" + ph + "\"" + args + ",\"ts\":" +
                            std::to_string(ts + _performanceCounterEpoch) + ",\"pid\":" +
                            std::to_string(ProcUtil::getProcessId() + SYNTHETIC_COOL_PID_OFFSET) +
                            ",\"tid\":" + std::to_string(tid) + ",\"id\":" + std::to_string(id) +
                            "},\n");
                    }
                    else if (ph == "X" &&
                             getTokenUInt64(tokens[4], "dur", dur))
                    {
                        COOLWSD::writeTraceEventRecording(
                            "{\"name\":" + name + R"(,"ph":"X")" + args + ",\"ts\":" +
                            std::to_string(ts + _performanceCounterEpoch) + ",\"pid\":" +
                            std::to_string(ProcUtil::getProcessId() + SYNTHETIC_COOL_PID_OFFSET) +
                            ",\"tid\":1"
                            ",\"dur\":" +
                            std::to_string(dur) + "},\n");
                    }
                    else
                    {
                        LOG_WRN("Unrecognized TRACEEVENT message");
                    }
                }
            }
            else
                LOG_WRN("Unrecognized TRACEEVENT message");
        }
        return false;
    }

    COOLWSD::dumpIncomingTrace(docBroker->getJailId(), getId(), firstLine);

    if (COOLProtocol::tokenIndicatesUserInteraction(tokens[0]))
    {
        // Keep track of timestamps of incoming client messages that indicate user activity.
        updateLastActivityTime();
        docBroker->updateLastActivityTime();

        if (isEditable() && isViewLoaded())
        {
            assert(!inWaitDisconnected() && "A writable view can't be waiting disconnection.");
            docBroker->updateEditingSessionId(getId());
        }
    }

    if (tokens.equals(0, "urp"))
    {
        // This can't be pushed down into the long list of tokens that are
        // forwarded to the child later as we need it to be able to run before
        // documents are loaded
        LOG_TRC("UNO remote protocol message (from client): " << firstLine);
        return forwardToChild(std::string(buffer, length), docBroker);
    }

    if (tokens.equals(0, "coolclient"))
    {
        if (tokens.size() < 2)
        {
            sendTextFrameAndLogError("error: cmd=coolclient kind=badprotocolversion");
            return false;
        }

        const std::tuple<int, int, std::string> versionTuple = ParseVersion(tokens[1]);
        if (std::get<0>(versionTuple) != ProtocolMajorVersionNumber ||
            std::get<1>(versionTuple) != ProtocolMinorVersionNumber)
        {
            sendTextFrameAndLogError("error: cmd=coolclient kind=badprotocolversion");
            return false;
        }

        _performanceCounterEpoch = 0;
        if (tokens.size() >= 4)
        {
            const std::string timestamp = tokens[2];
            const char* str = timestamp.c_str();
            char* endptr = nullptr;
            uint64_t ts = strtoull(str, &endptr, 10);
            if (*endptr == '\0')
            {
                const std::string perfcounter = tokens[3];
                str = perfcounter.data();
                endptr = nullptr;
                double counter = strtod(str, &endptr);
                if (*endptr == '\0' && counter > 0 &&
                    (counter < (double)(uint64_t)(std::numeric_limits<uint64_t>::max() / 1000)))
                {
                    // Now we know how to translate from the client's performance.now() values to
                    // microseconds since the epoch.
                    _performanceCounterEpoch = ts * 1000 - (uint64_t)(counter * 1000);
                    LOG_INF("Client timestamps: Date.now():" << ts <<
                            ", performance.now():" << counter
                            << " => " << _performanceCounterEpoch);
                }
            }
        }

        std::string timezoneName;
        if (COOLWSD::IndirectionServerEnabled && COOLWSD::GeolocationSetup)
            timezoneName =
                ConfigUtil::getString("indirection_endpoint.geolocation_setup.timezone", "");

        // Send COOL version information
        sendTextFrame("coolserver " + Util::getVersionJSON(EnableExperimental, timezoneName));
        // Send LOKit version information
        sendTextFrame("lokitversion " + COOLWSD::LOKitVersion);

        // If Trace Event generation and logging is enabled (whether it can be turned on), tell it
        // to cool
        if (COOLWSD::EnableTraceEventLogging)
            sendTextFrame("enabletraceeventlogging yes");

        if constexpr (!Util::isMobileApp())
        {
            // If it is not mobile, it must be Linux (for now).
            std::string osVersionInfo(
                ConfigUtil::getConfigValue<std::string>("per_view.custom_os_info", ""));
            if (osVersionInfo.empty())
            {
                CONFIG_STATIC const bool sig = ConfigUtil::getBool("security.server_signature", false);
                // Honour security.server_signature for reporting OS details too
                if (sig)
                    osVersionInfo = Util::getLinuxVersion();
                else
                    osVersionInfo = "unknown";
            }

            sendTextFrame(std::string("osinfo ") + osVersionInfo);
        }

        // Send clipboard key
        rotateClipboardKey(true);

        return true;
    }

    if (tokens.equals(0, "versionbar"))
    {
#if !MOBILEAPP
        std::string versionBar;
        {
            std::lock_guard<std::mutex> lock(COOLWSD::FetchUpdateMutex);
            versionBar = COOLWSD::LatestVersion;
        }

        if (!versionBar.empty())
            sendTextFrame("versionbar: " + versionBar);
#endif
    }
    else if (tokens.equals(0, "jserror") || tokens.equals(0, "jsexception"))
    {
        LOG_ERR(std::string(buffer, length));
        return true;
    }
    else if (tokens.equals(0, "load"))
    {
        if (!getDocURL().empty())
        {
            sendTextFrameAndLogError("error: cmd=load kind=docalreadyloaded");
            return false;
        }

        return loadDocument(buffer, length, tokens, docBroker);
    }
    else if (tokens.equals(0, "loadwithpassword"))
    {
        std::string docPassword;
        if (tokens.size() > 1 && getTokenString(tokens[1], "password", docPassword))
        {
            if (!docPassword.empty())
            {
                setHaveDocPassword(true);
                setDocPassword(docPassword);
            }
        }
        return loadDocument(buffer, length, tokens, docBroker);
    }
    else if (getDocURL().empty())
    {
        sendTextFrameAndLogError("error: cmd=" + tokens[0] + " kind=nodocloaded");
        return false;
    }
    else if (tokens.equals(0, "commandvalues"))
    {
        return getCommandValues(buffer, length, tokens, docBroker);
    }
    else if (tokens.equals(0, "closedocument"))
    {
        // If this session is the owner of the file & 'EnableOwnerTermination' feature
        // is turned on by WOPI, let it close all sessions
        if (isDocumentOwner() && _wopiFileInfo && _wopiFileInfo->getEnableOwnerTermination())
        {
            LOG_DBG("Session [" << getId() << "] requested owner termination");
            docBroker->closeDocument("ownertermination");
        }
        else if (docBroker->isDocumentChangedInStorage())
        {
            LOG_DBG("Document marked as changed in storage and user ["
                    << getUserId() << ", " << getUserName()
                    << "] wants to refresh the document for all.");
            docBroker->stop("documentconflict " + getUserName());
        }

        return true;
    }
    else if (tokens.equals(0, "versionrestore"))
    {
        if (tokens.size() > 1 && tokens.equals(1, "prerestore"))
        {
            // green signal to WOPI host to restore the version *after* saving
            // any unsaved changes, if any, to the storage
            docBroker->closeDocument("versionrestore: prerestore_ack");
        }
    }
    else if (tokens.equals(0, "partpagerectangles"))
    {
        // We don't support partpagerectangles any more, will be removed in the
        // next version
        sendTextFrame("partpagerectangles: ");
        return true;
    }
    else if (tokens.equals(0, "ping"))
    {
        std::string count = std::to_string(docBroker->getRenderedTileCount());
        sendTextFrame("pong rendercount=" + count);
        return true;
    }
    else if (tokens.equals(0, "status") || tokens.equals(0, "statusupdate"))
    {
        assert(firstLine.size() == static_cast<std::size_t>(length));
        return forwardToChild(firstLine, docBroker);
    }
    else if (tokens.equals(0, "tile"))
    {
        const int canonicalViewId = to_underlying(getCanonicalViewId());
        if (!UnitWSD::isUnitTesting() && canonicalViewId < 1000)
        {
            LOG_WRN("Got tile request for session ["
                    << getId() << "] on document [" << docBroker->getDocKey()
                    << "] with invalid view ID [" << canonicalViewId << ']');
        }
        return sendTile(buffer, length, tokens, docBroker);
    }
    else if (tokens.equals(0, "tilecombine"))
    {
        const int canonicalViewId = to_underlying(getCanonicalViewId());
        if (!UnitWSD::isUnitTesting() && canonicalViewId < 1000)
        {
            LOG_WRN("Got tilecombine request for session ["
                    << getId() << "] on document [" << docBroker->getDocKey()
                    << "] with invalid view ID [" << canonicalViewId << ']');
        }
        return sendCombinedTiles(buffer, length, tokens, docBroker);
    }
    else if (tokens.equals(0, "save"))
    {
        // If we can't write to Storage, there is no point in saving.
        if (!isWritable())
        {
            LOG_WRN("Session [" << getId() << "] on document [" << docBroker->getDocKey()
                                << "] has no write permissions in Storage and cannot save.");
            sendTextFrameAndLogError("error: cmd=save kind=savefailed");
        }
        else
        {
            // Don't save unmodified docs by default.
            int dontSaveIfUnmodified = 1;
            int dontTerminateEdit = 1;
            std::string extendedData;

            // We expect at most 3 arguments.
            for (int i = 0; i < 3; ++i)
            {
                // +1 to skip the command token.
                const StringVector attr = StringVector::tokenize(tokens[i + 1], '=');
                if (attr.size() == 2)
                {
                    if (attr[0] == "dontTerminateEdit")
                        COOLProtocol::stringToInteger(attr[1], dontTerminateEdit);
                    else if (attr[0] == "dontSaveIfUnmodified")
                        COOLProtocol::stringToInteger(attr[1], dontSaveIfUnmodified);
                    else if (attr[0] == "extendedData")
                    {
                        extendedData = Uri::decode(attr[1]);
                    }
                }
            }

            docBroker->manualSave(client_from_this(), dontTerminateEdit != 0,
                                  dontSaveIfUnmodified != 0, extendedData);
        }
    }
    else if (tokens.equals(0, "savetostorage"))
    {
        // By default savetostorage implies forcing.
        int force = 1;
        if (tokens.size() > 1)
            (void)getTokenInteger(tokens[1], "force", force);

        // The savetostorage command is really only used to resolve save conflicts
        // and it seems to always have force=1. However, we should still honor the
        // contract and do as told, not as we expect the API to be used. Use force if provided.
        docBroker->uploadToStorage(client_from_this(), force);
    }
    else if (tokens.equals(0, "clientvisiblearea"))
    {
        int x;
        int y;
        int width;
        int height;
        if ((tokens.size() != 5 && tokens.size() != 7) ||
            !getTokenInteger(tokens[1], "x", x) ||
            !getTokenInteger(tokens[2], "y", y) ||
            !getTokenInteger(tokens[3], "width", width) ||
            !getTokenInteger(tokens[4], "height", height))
        {
            // Be forgiving and log instead of disconnecting.
            // sendTextFrameAndLogError("error: cmd=clientvisiblearea kind=syntax");
            logSyntaxErrorDetails(tokens, firstLine);
            return true;
        }

        if (tokens.size() == 7)
        {
            int splitX;
            int splitY;
            if (!getTokenInteger(tokens[5], "splitx", splitX) ||
                !getTokenInteger(tokens[6], "splity", splitY))
            {
                logSyntaxErrorDetails(tokens, firstLine);
                return true;
            }

            _splitX = splitX;
            _splitY = splitY;
        }

        // Untrusted user input, make sure these are not negative.
        if (width < 0)
        {
            width = 0;
        }

        if (height < 0)
        {
            height = 0;
        }

        _clientVisibleArea = Util::Rectangle(x, y, width, height);
        return forwardToChild(std::string(buffer, length), docBroker);
    }
    else if (tokens.equals(0, "setclientpart"))
    {
        if(!_isTextDocument)
        {
            int temp;
            if (tokens.size() != 2 ||
                !getTokenInteger(tokens[1], "part", temp))
            {
                logSyntaxErrorDetails(tokens, firstLine);
                return false;
            }

            _clientSelectedPart = temp;
            return forwardToChild(std::string(buffer, length), docBroker);
        }
    }
    else if (tokens.equals(0, "selectclientpart"))
    {
        if(!_isTextDocument)
        {
            int part;
            int how;
            if (tokens.size() != 3 ||
                !getTokenInteger(tokens[1], "part", part) ||
                !getTokenInteger(tokens[2], "how", how))
            {
                sendTextFrameAndLogError("error: cmd=selectclientpart kind=syntax");
                return false;
            }

            return forwardToChild(std::string(buffer, length), docBroker);
        }
    }
    else if (tokens.equals(0, "moveselectedclientparts"))
    {
        if (!_isTextDocument)
        {
            int position;
            if (tokens.size() != 2 ||
                !getTokenInteger(tokens[1], "position", position))
            {
                sendTextFrameAndLogError("error: cmd=moveselectedclientparts kind=syntax");
                return false;
            }

            if (isEditable())
                docBroker->updateLastModifyingActivityTime();
            return forwardToChild(std::string(buffer, length), docBroker);
        }
    }
    else if (tokens.equals(0, "clientzoom"))
    {
        int tilePixelWidth;
        int tilePixelHeight;
        int tileTwipWidth;
        int tileTwipHeight;
        if (tokens.size() < 5 ||
            !getTokenInteger(tokens[1], "tilepixelwidth", tilePixelWidth) ||
            !getTokenInteger(tokens[2], "tilepixelheight", tilePixelHeight) ||
            !getTokenInteger(tokens[3], "tiletwipwidth", tileTwipWidth) ||
            !getTokenInteger(tokens[4], "tiletwipheight", tileTwipHeight))
        {
            // Be forgiving and log instead of disconnecting.
            // sendTextFrameAndLogError("error: cmd=clientzoom kind=syntax");
            logSyntaxErrorDetails(tokens, firstLine);
            return true;
        }

        _tileWidthPixel = tilePixelWidth;
        _tileHeightPixel = tilePixelHeight;
        _tileWidthTwips = tileTwipWidth;
        _tileHeightTwips = tileTwipHeight;
        return forwardToChild(std::string(buffer, length), docBroker);
    }
    else if (tokens.equals(0, "tileprocessed"))
    {
        std::string wids;
        if (tokens.size() != 2 ||
            !getTokenString(tokens[1], "wids", wids))
        {
            // Be forgiving and log instead of disconnecting.
            // sendTextFrameAndLogError("error: cmd=tileprocessed kind=syntax");
            logSyntaxErrorDetails(tokens, firstLine);
            assert(!"Invalid syntax for tileprocessed");
            return true;
        }

        // call onTileProcessed on each tileID of tileid1, tileid2, ...
        auto lambda = [this](size_t /*nIndex*/, const std::string_view token)
        {
            const auto [wireId, res] = NumUtil::i32FromString(token);
            if (!res)
                LOG_WRN("Invalid syntax for tileprocessed wireid '" << token << "'");
            onTileProcessed(wireId);
            return false;
        };
        StringVector::tokenize_foreach(lambda, wids.data(), wids.size(), ',');

        docBroker->sendRequestedTiles(client_from_this());
        return true;
    }
    else if (tokens.equals(0, "removesession"))
    {
        if (tokens.size() > 1 && (isDocumentOwner() || !isReadOnly()))
        {
            docBroker->broadcastMessage(firstLine);
            docBroker->removeSession(client_from_this());
        }
        else
            LOG_WRN("Readonly session '" << getId() << "' trying to kill another view");
    }
    else if (tokens.equals(0, "renamefile"))
    {
        std::string encodedWopiFilename;
        if (tokens.size() < 2 || !getTokenString(tokens[1], "filename", encodedWopiFilename))
        {
            LOG_ERR("Bad syntax for: " << firstLine);
            sendTextFrameAndLogError("error: cmd=renamefile kind=syntax");
            return false;
        }

        std::string wopiFilename = Uri::decode(encodedWopiFilename);
        const std::string error =
            docBroker->handleRenameFileCommand(getId(), std::move(wopiFilename));
        if (!error.empty())
        {
            sendTextFrameAndLogError(error);
            return false;
        }

        return true;
    }
    else if (tokens.equals(0, "dialogevent"))
    {
        if (tokens.size() > 2)
        {
            std::string jsonString = tokens.cat("", 2);
            try
            {
                Poco::JSON::Parser parser;
                const Poco::Dynamic::Var result = parser.parse(jsonString);
                const auto& object = result.extract<Poco::JSON::Object::Ptr>();
                const std::string id = object->has("id") ? object->get("id").toString() : "";
                if (id == "changepass" && _wopiFileInfo && !isDocumentOwner())
                {
                    sendTextFrameAndLogError("error: cmd=dialogevent kind=cantchangepass");
                    return false;
                }
            }
            catch (const std::exception& exception)
            {
                // Child will handle this case
            }
        }

        return forwardToChild(firstLine, docBroker);
    }
    else if (tokens.equals(0, "formfieldevent") ||
             tokens.equals(0, "sallogoverride") ||
             tokens.equals(0, "contentcontrolevent"))
    {
        return forwardToChild(firstLine, docBroker);
    }
    else if (tokens.equals(0, "loggingleveloverride"))
    {
        if (tokens.size() > 0)
        {
            // Note that these LOG_INF() messages won't necessarily show up if the current logging
            // level is higher, of course.
            if (tokens.equals(1, "default"))
            {
                LOG_INF("Thread-local logging level being set to default ["
                        << Log::getLevel()
                        << "]");
                Log::setThreadLocalLogLevel(Log::getLevelName());
            }
            else
            {
                try
                {
                    auto leastVerboseAllowed = Poco::Logger::parseLevel(COOLWSD::LeastVerboseLogLevelSettableFromClient);
                    auto mostVerboseAllowed = Poco::Logger::parseLevel(COOLWSD::MostVerboseLogLevelSettableFromClient);

                    if (tokens.equals(1, "verbose"))
                    {
                        LOG_INF("Client sets thread-local logging level to the most verbose allowed ["
                                << COOLWSD::MostVerboseLogLevelSettableFromClient
                                << "]");
                        Log::setThreadLocalLogLevel(COOLWSD::MostVerboseLogLevelSettableFromClient);
                        LOG_INF("Thread-local logging level was set to ["
                                << COOLWSD::MostVerboseLogLevelSettableFromClient
                                << "]");
                    }
                    else if (tokens.equals(1, "terse"))
                    {
                        LOG_INF("Client sets thread-local logging level to the least verbose allowed ["
                                << COOLWSD::LeastVerboseLogLevelSettableFromClient
                                << "]");
                        Log::setThreadLocalLogLevel(COOLWSD::LeastVerboseLogLevelSettableFromClient);
                        LOG_INF("Thread-local logging level was set to ["
                                << COOLWSD::LeastVerboseLogLevelSettableFromClient
                                << "]");
                    }
                    else
                    {
                        auto level = Poco::Logger::parseLevel(tokens[1]);
                        // Note that numerically the higher priority levels are lower in value.
                        if (level >= leastVerboseAllowed && level <= mostVerboseAllowed)
                        {
                            LOG_INF("Thread-local logging level being set to ["
                                    << tokens[1]
                                    << "]");
                            Log::setThreadLocalLogLevel(tokens[1]);
                        }
                        else
                        {
                            LOG_WRN("Client tries to set logging level to ["
                                    << tokens[1]
                                    << "] which is outside of bounds ["
                                    << COOLWSD::LeastVerboseLogLevelSettableFromClient << ","
                                    << COOLWSD::MostVerboseLogLevelSettableFromClient << "]");
                        }
                    }
                }
                catch (const Poco::Exception &e)
                {
                    LOG_WRN("Exception while handling loggingleveloverride message: " << e.message());
                }
            }
        }
    }
    else if (tokens.equals(0, "traceeventrecording"))
    {
        if (ConfigUtil::getConfigValue<bool>("trace_event[@enable]", false))
        {
            if (tokens.size() > 0)
            {
                if (tokens.equals(1, "start"))
                {
                    TraceEvent::startRecording();
                    LOG_INF("Trace Event recording in this WSD process turned on (might have been on already)");
                }
                else if (tokens.equals(1, "stop"))
                {
                    TraceEvent::stopRecording();
                    LOG_INF("Trace Event recording in this WSD process turned off (might have been off already)");
                }
            }
            forwardToChild(firstLine, docBroker);
        }
        return true;
    }
    else if (tokens.equals(0, "a11ystate"))
    {
        if (ConfigUtil::getConfigValue<bool>("accessibility.enable", false))
        {
            return forwardToChild(std::string(buffer, length), docBroker);
        }
    }
    else if (tokens.equals(0, "completefunction"))
    {
        return forwardToChild(std::string(buffer, length), docBroker);
    }
#if !MOBILEAPP
    else if (tokens.equals(0, "aichat:"))
    {
        return handleAIChatAction(firstLine);
    }
    else if (tokens.equals(0, "aichatcancel:"))
    {
        return handleAIChatCancel(firstLine);
    }
    else if (tokens.equals(0, "aichatapprove:"))
    {
        return handleAIChatApprove(firstLine);
    }
#endif
    else if (tokens.equals(0, "resetaccesstoken"))
    {
        if (tokens.size() != 2)
        {
            LOG_ERR("Bad syntax for: " << tokens[0]);
            sendTextFrameAndLogError("error: cmd=resetaccesstoken kind=syntax");
            return false;
        }

        _auth.resetAccessToken(tokens[1]);
        return true;
    }
#if !MOBILEAPP && !WASMAPP
    else if (tokens.equals(0, "switch_request"))
    {
        if (tokens.size() != 2)
        {
            LOG_ERR("Bad syntax for: " << tokens[0]);
            sendTextFrameAndLogError("error: cmd=switch_request kind=syntax");
            return false;
        }

        docBroker->switchMode(client_from_this(), tokens[1]);
        return true;
    }
#endif // !MOBILEAPP && !WASMAPP
    else if (tokens.equals(0, "outlinestate") ||
             tokens.equals(0, "downloadas") ||
             tokens.equals(0, "getchildid") ||
             tokens.equals(0, "gettextselection") ||
             tokens.equals(0, "paste") ||
             tokens.equals(0, "insertfile") ||
             tokens.equals(0, "key") ||
             tokens.equals(0, "textinput") ||
             tokens.equals(0, "windowkey") ||
             tokens.equals(0, "mouse") ||
             tokens.equals(0, "windowmouse") ||
             tokens.equals(0, "windowgesture") ||
             tokens.equals(0, "resetselection") ||
             tokens.equals(0, "saveas") ||
             tokens.equals(0, "exportas") ||
             tokens.equals(0, "selectgraphic") ||
             tokens.equals(0, "selecttext") ||
             tokens.equals(0, "windowselecttext") ||
             tokens.equals(0, "setpage") ||
             tokens.equals(0, "uno") ||
             tokens.equals(0, "urp") ||
             tokens.equals(0, "useractive") ||
             tokens.equals(0, "userinactive") ||
             tokens.equals(0, "paintwindow") ||
             tokens.equals(0, "windowcommand") ||
             tokens.equals(0, "asksignaturestatus") ||
             tokens.equals(0, "rendershapeselection") ||
             tokens.equals(0, "resizewindow") ||
             tokens.equals(0, "removetextcontext") ||
             tokens.equals(0, "rendersearchresult") ||
             tokens.equals(0, "geta11yfocusedparagraph") ||
             tokens.equals(0, "geta11ycaretposition") ||
             tokens.equals(0, "getpresentationinfo") ||
             tokens.equals(0, "slideshowfollow"))
    {
#if !MOBILEAPP
        if (tokens.equals(0, "uno"))
        {
            if (tokens.equals(1, ".uno:PrepareSignature") || tokens.equals(1, ".uno:DownloadSignature"))
            {
                return handleSignatureAction(tokens);
            }
        }
#endif

        if (tokens.equals(0, "key"))
        {
            _keyEvents++;

            // Suppress Ctrl+q, which exits Core immediately.
            // key type=input char=0 key=8720
            if (tokens.size() == 4 && tokens.equals(2, "char=0") && tokens.equals(3, "key=8720"))
            {
                LOG_DBG("Suppressing Ctrl+q");
                return true;
            }
        }

        if (isEditable() && COOLProtocol::tokenIndicatesDocumentModification(tokens))
        {
            docBroker->updateLastModifyingActivityTime();
        }

        if (!filterMessage(firstLine))
        {
            const std::string dummyFrame = "dummymsg";
            return forwardToChild(dummyFrame, docBroker);
        }

        if (tokens.equals(0, "slideshowfollow"))
        {
            if(tokens.equals(1, "newfollowmepresentation"))
                docBroker->setIsFollowmeSlideShowOn(true);
            else if(tokens.equals(1, "endpresentation"))
                docBroker->setIsFollowmeSlideShowOn(false);
            else if(tokens.equals(1, "effect")){
                Poco::JSON::Parser parser;
                auto result = parser.parse(tokens[2]);
                int effectNumber = JsonUtil::getJSONValue<int>(result.extract<Poco::JSON::Object::Ptr>(), "currentEffect");
                docBroker->setLeaderEffect(effectNumber);
            }
            else if(tokens.equals(1, "displayslide")) {
                Poco::JSON::Parser parser;
                auto result = parser.parse(tokens[2]);
                int slideNumber = JsonUtil::getJSONValue<int>(result.extract<Poco::JSON::Object::Ptr>(), "currentSlide");
                docBroker->setLeaderSlide(slideNumber);
                docBroker->setLeaderEffect(0);
            }
            docBroker->broadcastMessageToOthers(tokens.substrFromToken(0), client_from_this());
            return true;
        }

        return forwardToChild(std::string(buffer, length), docBroker);
    }
    else if (tokens.equals(0, "attemptlock"))
    {
        return attemptLock(docBroker);
    }
    else if (tokens.equals(0, "blockingcommandstatus"))
    {
        return forwardToChild(std::string(buffer, length), docBroker);
    }
    else if (tokens.equals(0, "toggletiledumping"))
    {
        return forwardToChild(std::string(buffer, length), docBroker);
    }
    else if (tokens.equals(0, "getslide"))
    {
        return handleGetSlideRequest(tokens, docBroker);
    }
#if !MOBILEAPP
    else if (tokens.equals(0, "routetokensanitycheck"))
    {
        Admin::instance().routeTokenSanityCheck();
    }
    else if (tokens.equals(0, "updateviewsettings") && tokens.size() >= 2)
    {
        return handleUpdateViewSettings(firstLine);
    }
    else if (tokens.equals(0, "browsersetting") && tokens.size() >= 3)
    {
        std::string action;
        getTokenString(tokens[1], "action", action);
        if (action == "update")
        {
            std::string json;
            getTokenString(tokens[2], "json", json);
            try
            {
                updateBrowserSettingsJSON(json);
                COOLWSD::syncUsersBrowserSettings(getUserId(), docBroker->getPid(), json);
                uploadBrowserSettingsToWopiHost();
            }
            catch (const std::exception& exc)
            {
                LOG_WRN("Failed to upload browsersetting json for session ["
                        << getId() << "] with error[" << exc.what() << ']');
            }
        }
    }
#endif
    else
    {
        LOG_ERR("Session [" << getId() << "] got unknown command [" << tokens[0] << ']');
        sendTextFrameAndLogError("error: cmd=" + tokens[0] + " kind=unknown");
    }

    return false;
}

#if !MOBILEAPP
void ClientSession::uploadBrowserSettingsToWopiHost()
{
    const Authorization& auth = getAuthorization();
    Poco::URI uriObject = DocumentBroker::getPresetUploadBaseUrl(_uriPublic);

    const std::string& filePath = "/settings/userconfig/browsersetting/browsersetting.json";
    uriObject.addQueryParameter("fileId", filePath);
    auth.authorizeURI(uriObject);

    const std::string& uriAnonym = COOLWSD::anonymizeUrl(uriObject.toString());

    auto httpRequest = StorageConnectionManager::createHttpRequest(uriObject, auth);
    httpRequest.setVerb(http::Request::VERB_POST);
    auto httpSession = StorageConnectionManager::getHttpSession(uriObject);

    std::ostringstream jsonStream;
    _browserSettingsJSON->stringify(jsonStream, 2);
    httpRequest.setBody(jsonStream.str(), "application/json; charset=utf-8");

    const std::string logPfx = getLogPrefix();
    http::Session::FinishedCallback finishedCallback =
        [uriAnonym, logPfx](const std::shared_ptr<http::Session>& wopiSession)
    {
        const std::shared_ptr<const http::Response> httpResponse = wopiSession->response();
        const http::StatusLine statusLine = httpResponse->statusLine();
        if (statusLine.statusCode() != http::StatusCode::OK)
        {
            LOG_ERR_S(logPfx << "Failed to upload updated browsersetting to wopiHost["
                    << uriAnonym << "] with status[" << statusLine.reasonPhrase() << ']');
            return;
        }
        LOG_TRC_S(logPfx << "Successfully uploaded browsersetting to wopiHost");
    };

    LOG_DBG("Uploading browsersetting json [" << jsonStream.str() << "] to wopiHost[" << uriAnonym
                                              << ']');
    httpSession->setFinishedHandler(std::move(finishedCallback));
    httpSession->asyncRequest(httpRequest, COOLWSD::getWebServerPoll());
}

void ClientSession::uploadViewSettingsToWopiHost()
{
    try
    {
        const Authorization& auth = getAuthorization();
        Poco::URI uriObject = DocumentBroker::getPresetUploadBaseUrl(_uriPublic);

        const std::string filePath = "/settings/userconfig/viewsetting/viewsetting.json";
        uriObject.addQueryParameter("fileId", filePath);
        auth.authorizeURI(uriObject);

        const std::string uriAnonym = COOLWSD::anonymizeUrl(uriObject.toString());

        auto httpRequest = StorageConnectionManager::createHttpRequest(uriObject, auth);
        httpRequest.setVerb(http::Request::VERB_POST);
        auto httpSession = StorageConnectionManager::getHttpSession(uriObject);

        std::ostringstream jsonStream;
        _viewSettingsJSON->stringify(jsonStream, 2);
        httpRequest.setBody(jsonStream.str(), "application/json; charset=utf-8");

        const std::string logPfx = getLogPrefix();
        http::Session::FinishedCallback finishedCallback =
            [uriAnonym, logPfx](const std::shared_ptr<http::Session>& wopiSession)
        {
            wopiSession->asyncShutdown();

            const std::shared_ptr<const http::Response> httpResponse = wopiSession->response();
            const http::StatusLine statusLine = httpResponse->statusLine();
            if (statusLine.statusCode() != http::StatusCode::OK)
            {
                LOG_ERR_S(logPfx << "Failed to upload updated viewsetting to wopiHost["
                        << uriAnonym << "] with status[" << statusLine.reasonPhrase() << ']');
                return;
            }
            LOG_TRC_S(logPfx << "Successfully uploaded viewsetting to wopiHost");
        };

        LOG_DBG("Uploading viewsetting json [" << jsonStream.str() << "] to wopiHost[" << uriAnonym
                                               << ']');
        httpSession->setFinishedHandler(std::move(finishedCallback));
        httpSession->asyncRequest(httpRequest, COOLWSD::getWebServerPoll());
    }
    catch (const std::exception& e)
    {
        LOG_ERR("Failed to upload viewsetting to WOPI host: " << e.what());
    }
}

bool ClientSession::handleUpdateViewSettings(const std::string& firstLine)
{
    const std::string jsonPayload = firstLine.substr(strlen("updateviewsettings "));

    Poco::JSON::Object::Ptr viewSettings;
    if (!JsonUtil::parseJSON(jsonPayload, viewSettings))
    {
        LOG_WRN("Failed to parse updateviewsettings JSON");
        return true;
    }

    std::string aiProviderAPIKey, aiProviderModel, aiProviderURL;
    std::string aiImageProviderAPIKey, aiImageProviderURL, aiImageModel;

    JsonUtil::findJSONValue(viewSettings, "aiProviderAPIKey", aiProviderAPIKey);
    JsonUtil::findJSONValue(viewSettings, "aiProviderModel", aiProviderModel);
    JsonUtil::findJSONValue(viewSettings, "aiProviderURL", aiProviderURL);
    JsonUtil::findJSONValue(viewSettings, "aiImageProviderAPIKey", aiImageProviderAPIKey);
    JsonUtil::findJSONValue(viewSettings, "aiImageProviderURL", aiImageProviderURL);
    JsonUtil::findJSONValue(viewSettings, "aiImageModel", aiImageModel);

    setAIProviderAPIKey(aiProviderAPIKey);
    setAIProviderModel(aiProviderModel);
    setAIProviderURL(aiProviderURL);
    setAIImageProviderAPIKey(aiImageProviderAPIKey);
    setAIImageProviderURL(aiImageProviderURL);
    setAIImageModel(aiImageModel);

    std::string zoteroAPIKey, signatureCert, signatureKey, signatureCa;
    JsonUtil::findJSONValue(viewSettings, "zoteroAPIKey", zoteroAPIKey);
    JsonUtil::findJSONValue(viewSettings, "signatureCert", signatureCert);
    JsonUtil::findJSONValue(viewSettings, "signatureKey", signatureKey);
    JsonUtil::findJSONValue(viewSettings, "signatureCa", signatureCa);
    setZoteroAPIKey(zoteroAPIKey);
    setSignatureCertificate(signatureCert);
    setSignatureKey(signatureKey);
    setSignatureCa(signatureCa);

    // Strip sensitive fields before sending sanitized version to client
    viewSettings->remove("aiProviderAPIKey");
    viewSettings->remove("aiProviderModel");
    viewSettings->remove("aiProviderURL");
    viewSettings->remove("aiImageProviderAPIKey");
    viewSettings->remove("aiImageProviderURL");
    viewSettings->remove("aiImageModel");

    const bool aiConfigured = !aiProviderAPIKey.empty() &&
                              !aiProviderModel.empty() &&
                              !aiProviderURL.empty();
    viewSettings->set("aiConfigured", aiConfigured);

    sendTextFrame("viewsetting: " + JsonUtil::jsonToString(viewSettings));

    LOG_DBG("Updated view settings for session [" << getId()
            << "], aiConfigured=" << aiConfigured);
    return true;
}

void ClientSession::updateBrowserSettingsJSON(const std::string& json)
{
    Poco::JSON::Parser parser;
    auto result = parser.parse(json);
    const auto& extractedObject = result.extract<Poco::JSON::Object::Ptr>();
    for (const auto& key : extractedObject->getNames())
    {
        const std::string& value = extractedObject->get(key);
        std::vector<std::string> vec = Util::splitStringToVector(key, '.');
        if (vec.size() == 2)
        {
            const std::string& parentKey = vec[0];
            const std::string& childKey = vec[1];
            if (!childKey.empty() && !parentKey.empty())
            {
                Poco::JSON::Object::Ptr jsonObject;
                if (_browserSettingsJSON->has(parentKey))
                    jsonObject = _browserSettingsJSON->getObject(parentKey);
                else
                    jsonObject = new Poco::JSON::Object();

                jsonObject->set(childKey, value);
                _browserSettingsJSON->set(parentKey, jsonObject);
            }
        }
        else
        {
            _browserSettingsJSON->set(key, value);
        }
    }
}
#endif

void ClientSession::overrideDocOption()
{
    if (!_sentBrowserSetting)
    {
        LOG_DBG("Browser settings not fetched, skipping DocOption override.");
        return;
    }

    std::string spellOnline, darkTheme, darkBackgroundForTheme, accessibilityState;
    JsonUtil::findJSONValue(_browserSettingsJSON, "spellOnline", spellOnline);
    JsonUtil::findJSONValue(_browserSettingsJSON, "darkTheme", darkTheme);
    JsonUtil::findJSONValue(_browserSettingsJSON, "accessibilityState", accessibilityState);
    Poco::JSON::Object::Ptr darkBackgroundObj =
        _browserSettingsJSON->getObject("darkBackgroundForTheme");

    // follow darkTheme preference if darkBackgroundForTheme is not set
    if (darkBackgroundObj.isNull())
    {
        if (!darkTheme.empty())
            setDarkBackground(darkTheme);
    }
    else
    {
        JsonUtil::findJSONValue(darkBackgroundObj, darkTheme == "true" ? "dark" : "light",
                                darkBackgroundForTheme);
    }

    if (!darkTheme.empty())
    {
        setDarkTheme(darkTheme);
        LOG_DBG("Overriding parsed docOption darkTheme[" << darkTheme << ']');
    }

    if (!darkBackgroundForTheme.empty())
    {
        setDarkBackground(darkBackgroundForTheme);
        LOG_DBG("Overriding parsed docOption darkBackgroundForTheme[" << darkBackgroundForTheme
                                                                      << ']');
    }

    if (!spellOnline.empty())
    {
        setSpellOnline(spellOnline);
        LOG_DBG("Overriding parsed docOption spellOnline[" << spellOnline << ']');
    }

    if (!accessibilityState.empty())
    {
        setAccessibilityState(accessibilityState == "true" ? true : false);
        LOG_DBG("Overriding parsed docOption accessibilityState[" << accessibilityState << ']');
    }
}

bool ClientSession::loadDocument(const char* /*buffer*/, int /*length*/,
                                 const StringVector& tokens,
                                 const std::shared_ptr<DocumentBroker>& docBroker)
{
    if (tokens.size() < 2)
    {
        // Failed loading ends connection.
        sendTextFrameAndLogError("error: cmd=load kind=syntax");
        return false;
    }

    _viewLoadStart = std::chrono::steady_clock::now();
    LOG_INF("Requesting document load from child.");
    try
    {
        std::string timestamp;
        int loadPart = -1;
        parseDocOptions(tokens, loadPart, timestamp);
        overrideDocOption();

        auto publicUri = docBroker->getPublicUri();
#ifdef _WIN32
        // See comment in RequestDetails::sanitizeURI()
        auto p = publicUri.getPath();
        if (p.length() > 3 && isalpha(p[0]) && p[1] == ':' && p[2] == '/')
            publicUri.setPath("/" + p);
#endif

        std::ostringstream oss;
        oss << std::boolalpha;
        oss << "load url=" << publicUri.toString();

#if ENABLE_SSL
        // if ssl client verification was disabled in online for the wopi server,
        // then exempt that host from ssl host verification also in core
        if (ssl::Manager::getClientVerification() == ssl::CertificateVerification::Disabled)
            oss << " verifyHost=false";
#endif

        if (!getUserId().empty() && !getUserName().empty())
        {
            std::string encodedUserId;
            Poco::URI::encode(getUserId(), "", encodedUserId);
            oss << " authorid=" << encodedUserId;
            encodedUserId.clear();
            Poco::URI::encode(COOLWSD::anonymizeUsername(getUserId()), "", encodedUserId);
            oss << " xauthorid=" << encodedUserId;

            std::string encodedUserName;
            Poco::URI::encode(getUserName(), "", encodedUserName);
            oss << " author=" << encodedUserName;
            encodedUserName.clear();
            Poco::URI::encode(COOLWSD::anonymizeUsername(getUserName()), "", encodedUserName);
            oss << " xauthor=" << encodedUserName;
        }

        if (!getUserExtraInfo().empty())
        {
            std::string encodedUserExtraInfo;
            Poco::URI::encode(getUserExtraInfo(), "", encodedUserExtraInfo);
            oss << " authorextrainfo=" << encodedUserExtraInfo; //TODO: could this include PII?
        }

        if (!getUserPrivateInfo().empty())
        {
            std::string encodedUserPrivateInfo;
            Poco::URI::encode(getUserPrivateInfo(), "", encodedUserPrivateInfo);
            oss << " authorprivateinfo=" << encodedUserPrivateInfo;
        }

        if (!getServerPrivateInfo().empty())
        {
            std::string encodedServerPrivateInfo;
            Poco::URI::encode(getServerPrivateInfo(), "", encodedServerPrivateInfo);
            oss << " serverprivateinfo=" << encodedServerPrivateInfo;
        }

        oss << " readonly=" << (isReadOnly() ? 1 : 0);

        if (isAllowChangeComments())
        {
            oss << " isAllowChangeComments=true";
        }

        if (isAllowManageRedlines())
        {
            oss << " isAllowManageRedlines=true";
        }

        if (loadPart >= 0)
        {
            oss << " part=" << loadPart;
        }

        if (getHaveDocPassword())
        {
            oss << " password=" << getDocPassword();
        }

        if (!getLang().empty())
        {
            oss << " lang=" << getLang();
        }

        if (!getDeviceFormFactor().empty())
        {
            oss << " deviceFormFactor=" << getDeviceFormFactor();
        }

        if (!getTimezone().empty())
        {
            oss << " timezone=" << getTimezone();
        }

        if (!getSpellOnline().empty())
        {
            oss << " spellOnline=" << getSpellOnline();
        }

        if (!getDarkTheme().empty())
        {
            oss << " darkTheme=" << getDarkTheme();
        }

        if (!getDarkBackground().empty())
        {
            oss << " darkBackground=" << getDarkBackground();
        }

        if (!getWatermarkText().empty())
        {
            std::string encodedWatermarkText;
            Poco::URI::encode(getWatermarkText(), "", encodedWatermarkText);
            oss << " watermarkText=" << encodedWatermarkText;
            oss << " watermarkOpacity="
                << ConfigUtil::getConfigValue<double>("watermark.opacity", 0.2);
        }

        if (ConfigUtil::hasProperty("security.enable_macros_execution"))
        {
            oss << " enableMacrosExecution="
                << ConfigUtil::getConfigValue<bool>("security.enable_macros_execution", false);
        }

        if (ConfigUtil::hasProperty("security.macro_security_level"))
        {
            oss << " macroSecurityLevel="
                << ConfigUtil::getConfigValue<int>("security.macro_security_level", 1);
        }

        if (!getInitialClientVisibleArea().empty())
        {
            oss << " clientvisiblearea=" << getInitialClientVisibleArea();
        }

        if (ConfigUtil::getConfigValue<bool>("accessibility.enable", false))
        {
            oss << " accessibilityState=" << getAccessibilityState();
        }

        if (!getDocOptions().empty())
        {
            oss << " options=" << getDocOptions();
        }

        if (_wopiFileInfo && !_wopiFileInfo->getTemplateSource().empty())
        {
            oss << " template=" << _wopiFileInfo->getTemplateSource();
        }

        if (!getBatchMode().empty())
        {
            oss << " batch=" << getBatchMode();
        }

        if (!getInFilterOption().empty())
        {
            oss << " infilterOptions=" << getInFilterOption();
        }
        else if (auto it = docBroker->getAdditionalFileUrisJailed().find("template"); it != docBroker->getAdditionalFileUrisJailed().end())
        {
            std::string options = R"({"TemplateURL":{"type":"string","value":")" +
                                  it->second + "\"}}";
            oss << " infilterOptions=" << options;
        }

#if ENABLE_FEATURE_LOCK
        sendLockedInfo();
#endif

#if ENABLE_FEATURE_RESTRICTION
        sendRestrictionInfo();
#endif
        if (docBroker->getIsFollowmeSlideShowOn())
        {
            sendTextFrame("slideshowfollow displayslide {\"currentSlide\": " + std::to_string(docBroker->getLeaderSlide()) +"}");
            sendTextFrame("slideshowfollow effect {\"currentEffect\": " + std::to_string(docBroker->getLeaderEffect()) +"}");
            sendTextFrame("slideshowfollow slideshowfollowon");
        }

        return forwardToChild(oss.str(), docBroker);;
    }
    catch (const Poco::SyntaxException&)
    {
        sendTextFrameAndLogError("error: cmd=load kind=uriinvalid");
    }

    return false;
}

#if ENABLE_FEATURE_LOCK
void ClientSession::sendLockedInfo()
{
    Poco::JSON::Object::Ptr lockInfo = new Poco::JSON::Object();
    CommandControl::LockManager::setTranslationPath(getLang());
    lockInfo->set("IsLockedUser", CommandControl::LockManager::isLockedUser());
    lockInfo->set("IsLockReadOnly", CommandControl::LockManager::isLockReadOnly());

    // Poco:Dynamic:Var does not support std::unordred_set so converted to std::vector
    std::vector<std::string> lockedCommandList(
        CommandControl::LockManager::getLockedCommandList().begin(),
        CommandControl::LockManager::getLockedCommandList().end());
    lockInfo->set("LockedCommandList", lockedCommandList);
    lockInfo->set("UnlockTitle", CommandControl::LockManager::getUnlockTitle());
    lockInfo->set("UnlockLink", CommandControl::LockManager::getUnlockLink());
    lockInfo->set("UnlockDescription", CommandControl::LockManager::getUnlockDescription());
    lockInfo->set("WriterHighlights", CommandControl::LockManager::getWriterHighlights());
    lockInfo->set("CalcHighlights", CommandControl::LockManager::getCalcHighlights());
    lockInfo->set("ImpressHighlights", CommandControl::LockManager::getImpressHighlights());
    lockInfo->set("DrawHighlights", CommandControl::LockManager::getDrawHighlights());

    const Poco::URI unlockImageUri = CommandControl::LockManager::getUnlockImageUri();
    if (!unlockImageUri.empty())
        lockInfo->set("UnlockImageUrlPath", unlockImageUri.getPath());
    CommandControl::LockManager::resetTransalatioPath();
    std::ostringstream ossLockInfo;
    lockInfo->stringify(ossLockInfo);
    const std::string lockInfoString = ossLockInfo.str();
    LOG_TRC("Sending feature locking info to client: " << lockInfoString);
    sendTextFrame("featurelock: " + lockInfoString);
}
#endif

#if ENABLE_FEATURE_RESTRICTION
void ClientSession::sendRestrictionInfo()
{
    Poco::JSON::Object::Ptr restrictionInfo = new Poco::JSON::Object();
    restrictionInfo->set("IsRestrictedUser",
                         CommandControl::RestrictionManager::isRestrictedUser());

    // Poco:Dynamic:Var does not support std::unordred_set so converted to std::vector
    std::vector<std::string> restrictedCommandList(
        CommandControl::RestrictionManager::getRestrictedCommandList().begin(),
        CommandControl::RestrictionManager::getRestrictedCommandList().end());
    restrictionInfo->set("RestrictedCommandList", restrictedCommandList);

    std::ostringstream ossRestrictionInfo;
    restrictionInfo->stringify(ossRestrictionInfo);
    const std::string restrictionInfoString = ossRestrictionInfo.str();
    LOG_TRC("Sending command restriction info to client: " << restrictionInfoString);
    sendTextFrame("restrictedCommands: " + restrictionInfoString);
}
#endif

bool ClientSession::getCommandValues(const char *buffer, int length, const StringVector& tokens,
                                     const std::shared_ptr<DocumentBroker>& docBroker)
{
    std::string command;
    if (tokens.size() != 2 || !getTokenString(tokens[1], "command", command))
        return sendTextFrameAndLogError("error: cmd=commandvalues kind=syntax");

    std::string cmdValues;
    if (docBroker->hasTileCache() && docBroker->tileCache().getTextStream(TileCache::StreamType::CmdValues, command, cmdValues))
        return sendTextFrame(cmdValues);

    return forwardToChild(std::string(buffer, length), docBroker);
}

bool ClientSession::sendTile(const char * /*buffer*/, int /*length*/, const StringVector& tokens,
                             const std::shared_ptr<DocumentBroker>& docBroker)
{
    try
    {
        docBroker->handleTileRequest(tokens, true, client_from_this());
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to process tile command: " << exc.what());
        return sendTextFrameAndLogError("error: cmd=tile kind=invalid");
    }

    return true;
}

bool ClientSession::sendCombinedTiles(const char* /*buffer*/, int /*length*/, const StringVector& tokens,
                                      const std::shared_ptr<DocumentBroker>& docBroker)
{
    try
    {
        TileCombined tileCombined = TileCombined::parse(tokens);
        tileCombined.setCanonicalViewId(getCanonicalViewId());
        if (tileCombined.hasDuplicates())
        {
            LOG_ERR("Dangerous, tilecombine with duplicates is not acceptable");
            return sendTextFrameAndLogError("error: cmd=tile kind=invalid");
        }
        docBroker->handleTileCombinedRequest(tileCombined, true, client_from_this());
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to process tilecombine command: " << exc.what());
        // Be forgiving and log instead of disconnecting.
        // return sendTextFrameAndLogError("error: cmd=tile kind=invalid");
    }

    return true;
}

bool ClientSession::handleGetSlideRequest(const StringVector& tokens,
                                          const std::shared_ptr<DocumentBroker>& docBroker)
{
    docBroker->handleGetSlideRequest(tokens, client_from_this());
    return true;
}

bool ClientSession::forwardToChild(const std::string& message,
                                   const std::shared_ptr<DocumentBroker>& docBroker)
{
    const bool binary = message.starts_with("paste") || message.starts_with("urp");
    return docBroker->forwardToChild(client_from_this(), message, binary);
}

bool ClientSession::filterMessage(const std::string& message) const
{
    bool allowed = true;
    StringVector tokens(StringVector::tokenize(message, ' '));

    // Set allowed flag to false depending on if particular WOPI properties are set
    if (tokens.equals(0, "downloadas"))
    {
        std::string id;
        if (tokens.size() >= 3)
            getTokenString(tokens[2], "id", id);
        allowed = filterDownloadAs(id);
    }
    else if (tokens.equals(0, "gettextselection"))
    {
        // Copying/pasting *within* the document is fine,
        // so keep .uno:Copy and .uno:Paste, but exporting is not.
        if (_wopiFileInfo && _wopiFileInfo->getDisableCopy())
        {
            allowed = false;
            LOG_WRN("WOPI host has disabled copying from the document");
        }
    }

    return allowed;
}

bool ClientSession::filterDownloadAs(const std::string& id) const
{
    bool allowed = true;

    if (!id.empty())
    {
        if (id == "print" && _wopiFileInfo && _wopiFileInfo->getDisablePrint())
        {
            allowed = false;
            LOG_WRN("WOPI host has disabled print for this session");
        }
        else if (id == "export" && _wopiFileInfo && _wopiFileInfo->getDisableExport())
        {
            allowed = false;
            LOG_WRN("WOPI host has disabled export for this session");
        }
        else if (id == "slideshow" && _wopiFileInfo &&
                 (_wopiFileInfo->getDisableExport() || !_wopiFileInfo->getWatermarkText().empty()))
        {
            allowed = false;
            LOG_WRN("WOPI host has disabled slideshow for this session");
        }
    }
    else
    {
        allowed = false;
        LOG_WRN("No value of id in downloadas message");
    }
    return allowed;
}

void ClientSession::setReadOnly(bool val)
{
    Session::setReadOnly(val);

    // Also inform the client.
    const std::string perm = isReadOnly() ? "readonly" : "edit";
    sendTextFrame("perm: " + perm);
}

void ClientSession::sendFileMode(const bool readOnly, const bool editComments, bool manageRedlines)
{
    std::string result = "filemode:{\"readOnly\": ";
    result += readOnly ? "true": "false";
    result += ", \"editComment\": ";
    result += editComments ? "true": "false";
    result += ", \"manageRedlines\": ";
    result += manageRedlines ? "true" : "false";

    // Add the view mode extensions list from configuration
    result += ", \"viewModeExtensions\": \"";
    result += COOLWSD::ViewModeFileExtensions;
    result += "\"";

    result += "}";
    sendTextFrame(result);
}

void ClientSession::setLockFailed(const std::string& reason)
{
    // TODO: make this "read-only" a special one with a notification (infobar? balloon tip?)
    //       and a button to unlock
    _isLockFailed = true;
    setReadOnly(true);
    sendTextFrame("lockfailed:" + reason);
}

bool ClientSession::attemptLock(const std::shared_ptr<DocumentBroker>& docBroker)
{
    if (!isReadOnly())
        return true;
    // We are only allowed to change into edit mode if the read-only mode is because of failed lock
    if (!_isLockFailed)
        return false;

    std::string failReason;
    const bool result = docBroker->attemptLock(*this, failReason);
    if (result)
        setReadOnly(false);
    else
        sendTextFrame("lockfailed:" + failReason);

    return result;
}

bool ClientSession::hasQueuedMessages() const
{
    return _senderQueue.size() > 0;
}

void ClientSession::writeQueuedMessages(std::size_t capacity)
{
    LOG_TRC("performing writes, up to " << capacity << " bytes");

    std::shared_ptr<Message> item;
    std::size_t wrote = 0;
    try
    {
        // Drain the queue, for efficient communication.
        while (capacity > wrote && _senderQueue.dequeue(item) && item)
        {
            const std::vector<char>& data = item->data();
            const auto size = data.size();
            assert(size && "Zero-sized messages must never be queued for sending.");

            if (item->isBinary())
            {
                Session::sendBinaryFrame(data.data(), size);
            }
            else
            {
                Session::sendTextFrame(data.data(), size);
            }

            wrote += size;
            LOG_TRC("wrote " << size << ", total " << wrote << " bytes");
        }
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to send message " << (item ? item->abbr() : "<empty-item>")
                                          << " to client: " << ex.what());
    }

    LOG_TRC("performed write, wrote " << wrote << " bytes");
}

// Insert our meta origin if we can
// Note: If @in is Poco::MemoryStream there is a bug in versions < 1.13.3 that
// Poco::BasicMemoryStreamBuf doesn't implement seekpos, so use of the single
// argument seekg fails, this can be worked around by using the double argument
// seekg variant which uses seekoff which was implemented
bool ClientSession::postProcessCopyPayload(std::istream& in, std::ostream& out)
{
    constexpr std::string_view textPlain = "text/plain";

    char data[textPlain.size()];
    in.read(data, textPlain.size());
    if (in.gcount() == textPlain.size() &&
        std::string_view(data, textPlain.size()) == textPlain)
    {
        // Single format and it's plain text (not HTML): no need to rewrite anything.
        return false;
    }

    // back to start
    in.clear();
    in.seekg(0, std::ios::beg);

    bool json = in.get() == '{';

    in.clear();
    in.seekg(0, std::ios::beg);

    // copy as far as body
    bool match = Util::copyToMatch(in, out, "<body");
    if (match)
    {
        // copy as far as the closing tag
        match = Util::copyToMatch(in, out, ">");
    }

    // cf. TileLayer.js /_dataTransferToDocument/
    if (match)
    {
        // write the output tag close
        out.write(">", 1);
        // skip the input tag close
        in.seekg(1, std::ios_base::cur);

        const std::string meta = getClipboardURI();
        LOG_TRC("Inject clipboard cool origin of '" << meta << "'");

        std::string origin = R"(<div id="meta-origin" data-coolorigin=")" + meta + "\">\n";
        if (json)
        {
            origin = R"(<div id=\"meta-origin\" data-coolorigin=\")" + meta + R"(\">\n)";
        }
        out.write(origin.data(), origin.size());

        // if there is a closing body tag, match style and write closing div tag before it
        if (Util::copyToMatch(in, out, "</body>"))
            out.write("</div>", 6);

        // write the remainder to out
        Poco::StreamCopier::copyStream(in, out);

        return true;
    }

    if (json)
    {
        // The content may not be json or any textual form. For example:
        // clipboardcontent: content.application/x-openoffice-svxb;windows_formatname="SVXB (StarView Bitmap/Animation)"
        LOG_DBG("Missing <body> in textselectioncontent/clipboardcontent payload");
    }

    return false;
}

// NB. also see browser/src/map/Clipboard.js that does this in JS for stubs.
// See also ClientSession::preProcessSetClipboardPayload() which removes the
// <div id="meta-origin"...>  tag added here.
void ClientSession::postProcessCopyPayload(const std::shared_ptr<Message>& payload)
{
    // Insert our meta origin if we can
    payload->rewriteDataBody([this](std::vector<char>& data) {
            const char* start = data.data();
            size_t size = data.size();

            std::ostringstream oss;

            std::string_view sv(start, size);
            std::string_view prefix("textselectioncontent:\n");
            if (sv.starts_with(prefix))
            {
                oss.write(prefix.data(), prefix.size());
                start += prefix.size();
                size -= prefix.size();
            }

            Poco::MemoryInputStream iss(start, size);
            if (postProcessCopyPayload(iss, oss))
            {
                std::string str(oss.str());
                data.assign(str.begin(), str.end());
                return true;
            }
            return false;
        });
}

bool ClientSession::handlePresentationInfo(const std::shared_ptr<Message>& payload, const std::shared_ptr<DocumentBroker>& docBroker)
{
    std::string json(payload->data().data(), payload->size());

    const auto iterator = json.find('{');
    const std::string prefix = json.substr(0, iterator);
    json.erase(0, iterator); // Remove the prefix to parse the purse JSON part.

    bool modified = false;

    Poco::JSON::Object::Ptr rootObject;
    try
    {
        if (JsonUtil::parseJSON(json, rootObject))
        {
            Poco::JSON::Array::Ptr slides = rootObject->getArray("slides");
            if (!slides.isNull() && slides->size() > 0)
            {
                for (size_t slideIndex = 0; slideIndex < slides->size(); slideIndex++)
                {
                    Poco::JSON::Object::Ptr slide = slides->getObject(slideIndex);
                    Poco::JSON::Array::Ptr videos = slide->getArray("videos");

                    if (!videos.isNull() && videos->size() > 0)
                    {
                        for (size_t videoIndex = 0; videoIndex < videos->size(); videoIndex++)
                        {
                            Poco::JSON::Object::Ptr video = videos->getObject(videoIndex);
                            const std::string id = JsonUtil::getJSONValue<std::string>(video, "id");
                            const std::string url = JsonUtil::getJSONValue<std::string>(video, "url");

                            if (!id.empty() && !url.empty())
                            {
                                std::string original =
                                    R"({ "id" : ")" + id + R"(", "url" : ")" + url + "\" }";
                                docBroker->addEmbeddedMedia(id, original); // Capture the original message with internal URL.

                                const std::string mediaUrl =
                                    Uri::encode(createPublicURI("media", id, false), "&");
                                video->set("url", mediaUrl); // Replace the url with the public one.
                                modified = true;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& exception)
    {
        LOG_ERR("unocommandresult parsing failure: " << exception.what());
    }

    if (modified)
    {
        std::ostringstream mediaStr;
        rootObject->stringify(mediaStr);
        const std::string newMessage = prefix + mediaStr.str();
        forwardToClient(std::make_shared<Message>(newMessage, Message::Dir::Out));
        return true;
    }

    return forwardToClient(payload);
}

bool ClientSession::handleKitToClientMessage(const std::shared_ptr<Message>& payload)
{
    LOG_TRC("handling kit-to-client [" << payload->abbr() << ']');
    const std::string& firstLine = payload->firstLine();

    const std::shared_ptr<DocumentBroker> docBroker = _docBroker.lock();
    if (!docBroker)
    {
        LOG_ERR("No DocBroker to handle kit-to-client message: " << firstLine);
        return false;
    }

    std::shared_ptr<StreamSocket> saveAsSocket = _saveAsSocket.lock();

    if constexpr (!Util::isMobileApp())
        COOLWSD::dumpOutgoingTrace(docBroker->getJailId(), getId(), firstLine);

    const auto& tokens = payload->tokens();
    if (tokens.equals(0, "downloadas:"))
    {
        std::string id;
        if (tokens.size() >= 4)
            getTokenString(tokens[3], "id", id);
        if (!filterDownloadAs(id))
        {
            LOG_WRN("Ignoring kit to client message of: " << firstLine);
            return false;
        }
    }
    else if (tokens.equals(0, "unocommandresult:"))
    {
        LOG_INF("Command: " << firstLine);
        const std::string stringJSON = payload->jsonString();
        if (!stringJSON.empty())
        {
            try
            {
                Poco::JSON::Parser parser;
                const Poco::Dynamic::Var parsedJSON = parser.parse(stringJSON);
                auto object = parsedJSON.extract<Poco::JSON::Object::Ptr>();
                if (object->get("commandName").toString() == ".uno:Save")
                {
                    // Capture isNextSaveAutosave flag before calling handleSaveResponse as it will reset the value!
                    // Add it to the JSON so clients can differentiate between manual saves and autosaves
                    const bool isAutosave = docBroker->isNextSaveAutosave();
                    object->set("isAutosave", isAutosave);

                    // Save to Storage and log result.
                    docBroker->handleSaveResponse(client_from_this(), object);

                    if (!isCloseFrame())
                    {
                        // create new payload with the updated JSON
                        std::ostringstream oss;
                        object->stringify(oss);
                        const std::string updatedMessage = "unocommandresult: " + oss.str();
                        forwardToClient(std::make_shared<Message>(updatedMessage, Message::Dir::Out));
                    }

                    return true;
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERR("Failed to handle [" << firstLine << "]: " << exception.what());
            }
        }
        else
        {
            LOG_WRN("Expected json unocommandresult. Ignoring: " << firstLine);
        }
    }
    else if (tokens.equals(0, "error:"))
    {
        std::string errorCommand;
        std::string errorKind;
        if (getTokenString(tokens[1], "cmd", errorCommand) &&
            getTokenString(tokens[2], "kind", errorKind))
        {
            if (errorCommand == "load")
            {
                LOG_ERR("Document load failed: " << errorKind);
                if (errorKind == "passwordrequired:to-view" ||
                    errorKind == "passwordrequired:to-modify" || errorKind == "wrongpassword")
                {
                    if (_isConvertTo)
                    {
                        abortConversion(docBroker, saveAsSocket, std::move(errorKind));
                    }
                    else
                    {
                        forwardToClient(payload);
                    }
                    return false;
                }

                // Handle all other load failures in convert-to mode.
                if (_isConvertTo)
                {
                    abortConversion(docBroker, saveAsSocket, std::move(errorKind));
                    return false;
                }
            }
            else if (_isConvertTo && errorCommand == "saveas")
            {
                // Conversion failed.
                abortConversion(docBroker, saveAsSocket, std::move(errorKind));
                return false;
            }
#if !MOBILEAPP
            else if (_aiToolLoop && _aiToolLoop->awaitingKitResponse &&
                     (errorCommand == "extractdocumentstructure" ||
                      errorCommand == "extractlinktargets" ||
                      errorCommand == "transformdocumentstructure"))
            {
                LOG_WRN("AIToolLoop: kit error for " << errorCommand << ": " << errorKind);
                _aiToolLoop->awaitingKitResponse = false;
                continueAIToolLoop(_aiToolLoop->pendingToolCallId,
                    "{\"error\":\"" + errorCommand + " failed: " + errorKind + "\"}");
                return true;
            }
            else
#endif
            {
                LOG_ERR(errorCommand << " error failure: " << errorKind);
            }
        }
    }
    else if (tokens.equals(0, "curpart:") && tokens.size() == 2)
    {
        //TODO: Should forward to client?
        int curPart;
        return getTokenInteger(tokens[1], "part", curPart);
    }
    else if (tokens.equals(0, "setpart:") && tokens.size() == 2)
    {
        if (!_isTextDocument)
        {
            int setPart;
            if (getTokenInteger(tokens[1], "part", setPart))
            {
                _clientSelectedPart = setPart;
            }
            else if (stringToInteger(tokens[1], setPart))
            {
                _clientSelectedPart = setPart;
            }
            else
                return false;
        }
    }
#if !MOBILEAPP
    else if (tokens.size() == 3 && (tokens.equals(0, "saveas:") || tokens.equals(0, "exportas:")))
    {
        return handleSaveAs(payload, docBroker, saveAsSocket);
    }

#elif defined(QTAPP) || defined(_WIN32)
    else if (tokens.size() == 3 && tokens.equals(0, "saveas:"))
    {
        // For mobile/desktop apps, the file has been saved directly by LOKit
        // Forward the saveas message to the client - Socket.js _renameOrSaveAsCallback()
        // will handle it and trigger load of the new document with a new FakeSocket.
        return forwardToClient(payload);
    }
#endif
    else if (tokens.size() == 2 && tokens.equals(0, "statechanged:"))
    {
        StringVector stateTokens(StringVector::tokenize(tokens[1], '='));
        if (stateTokens.size() == 2 && stateTokens.equals(0, ".uno:ModifiedStatus"))
        {
            // Always update the modified flag in the DocBroker faithfully.
            // Let it deal with the upload failure scenario and the admin console.
            docBroker->setModified(stateTokens.equals(1, "true"));
        }
        else
        {
            // Set the initial settings per the user's request.
            const std::pair<std::string, std::string> unoStatePair = Util::split(tokens[1], '=');

            if (!docBroker->isInitialSettingSet(unoStatePair.first))
            {
                docBroker->setInitialSetting(unoStatePair.first);
                if (unoStatePair.first == ".uno:TrackChanges")
                {
                    if ((unoStatePair.second == "true" &&
                         _wopiFileInfo && _wopiFileInfo->getDisableChangeTrackingRecord() == WopiStorage::WOPIFileInfo::TriState::True) ||
                        (unoStatePair.second == "false" &&
                         _wopiFileInfo && _wopiFileInfo->getDisableChangeTrackingRecord() == WopiStorage::WOPIFileInfo::TriState::False))
                    {
                        // Toggle the TrackChanges state.
                        LOG_DBG("Forcing " << unoStatePair.first << " toggle per user settings.");
                        forwardToChild("uno .uno:TrackChanges", docBroker);
                    }
                }
                else if (unoStatePair.first == ".uno:ShowTrackedChanges")
                {
                    if ((unoStatePair.second == "true" &&
                         _wopiFileInfo && _wopiFileInfo->getDisableChangeTrackingShow() == WopiStorage::WOPIFileInfo::TriState::True) ||
                        (unoStatePair.second == "false" &&
                         _wopiFileInfo && _wopiFileInfo->getDisableChangeTrackingShow() == WopiStorage::WOPIFileInfo::TriState::False))
                    {
                        // Toggle the ShowTrackChanges state.
                        LOG_DBG("Forcing " << unoStatePair.first << " toggle per user settings.");
                        forwardToChild("uno .uno:ShowTrackedChanges", docBroker);
                    }
                }
            }
        }
    }
    else if (tokens.equals(0, "textselectioncontent:"))
    {
        postProcessCopyPayload(payload);
        return forwardToClient(payload);
    }
    else if (tokens.equals(0, "presentationinfo:"))
    {
        return handlePresentationInfo(payload, docBroker);
    }
    else if (tokens.equals(0, "clipboardcontent:"))
    {
#if !MOBILEAPP // Most likely nothing of this makes sense in a mobile app

        // FIXME: Ash: we need to return different content depending
        // on whether this is a download-everything, or an individual
        // 'download' and/or providing our helpful / user page.

        // for now just for remote sockets.
        LOG_TRC("Got clipboard content of size " << payload->size() << " to send to "
                                                 << _clipSockets.size() << " sockets in state "
                                                 << name(_state));

        std::string clipFile;
        if (!getTokenString(tokens[1], "file", clipFile))
        {
            LOG_ERR("Bad syntax for: " << firstLine);
            return false;
        }

        LOG_TRC("clipboardcontent path: " << clipFile);

        std::string jailClipFile = getLocalPathToJail(clipFile, *docBroker);
        if (jailClipFile.empty())
            LOG_DBG("clipboardcontent produced no output in '" << clipFile << "'");

        bool empty = jailClipFile.empty() ? true : FileUtil::Stat(jailClipFile).size() == 0;

        if (!empty)
        {
            std::string postProcessedClipFile = COOLWSD::SavedClipboards->nextClipFileName();
            std::ofstream ofs(postProcessedClipFile, std::ofstream::out);
            std::ifstream ifs(jailClipFile, std::ifstream::in);
            bool postProcesed = postProcessCopyPayload(ifs, ofs);
            ifs.close();
            ofs.close();

            if (!postProcesed)
                FileUtil::removeFile(postProcessedClipFile);
            else
            {
                FileUtil::removeFile(jailClipFile);
                jailClipFile = std::move(postProcessedClipFile);
            }
        }

        // final cleanup via clipFileRemove dtor
        std::shared_ptr<FileUtil::OwnedFile> clipFileRemove;

        bool removeClipFile = true;
        if (!empty && (!_wopiFileInfo || !_wopiFileInfo->getDisableCopy()))
        {
            // returns same filename as its arg on rename failure
            std::string cacheFile = COOLWSD::SavedClipboards->insertClipboard(_clipboardKeys, jailClipFile);
            if (cacheFile != jailClipFile)
            {
                jailClipFile = std::move(cacheFile);
                removeClipFile = false;
            }
        }

        if (removeClipFile)
            clipFileRemove = std::make_shared<FileUtil::OwnedFile>(jailClipFile);

        for (const auto& it : _clipSockets)
        {
            auto socket = it.lock();
            if (!socket)
                continue;

            // The custom header for the clipboard of a living document.
            auto session = std::make_shared<http::ServerSession>();

            http::ServerSession::ResponseHeaders headers;
            headers.emplace_back("Last-Modified", Util::getHttpTimeNow());
            headers.emplace_back("Content-Type", "application/octet-stream");
            headers.emplace_back("X-Content-Type-Options", "nosniff");
            headers.emplace_back("X-COOL-Clipboard", "true");
            headers.emplace_back("Cache-Control", "no-cache");
            headers.emplace_back("Connection", "close");

            // on final session dtor clipFileRemove cleanup removes clipboard file
            session->setFinishedHandler([clipFileRemove](const std::shared_ptr<http::ServerSession>&) {});

            // Hand over socket to ServerSession which will async provide
            // clipboard content backed by jailClipFile
            session->asyncUpload(jailClipFile, std::move(headers));
            socket->setHandler(std::static_pointer_cast<ProtocolHandlerInterface>(session));

            LOG_INF("Queued " << (empty?"empty":"clipboard") << " response for send.");
        }
#endif
        _clipSockets.clear();
        return true;
    }
    else if (tokens.equals(0, "disconnected:"))
    {
        LOG_INF("End of disconnection handshake for " << getId());
        docBroker->finalRemoveSession(client_from_this());
        return true;
    }
    else if (tokens.equals(0, "graphicselection:") || tokens.equals(0, "graphicviewselection:"))
    {
        if (_thumbnailSession)
        {
            int x, y;
            if (stringToInteger(tokens[1], x) &&
                stringToInteger(tokens[2], y))
            {
                std::ostringstream renderThumbnailCmd;
                renderThumbnailCmd << "getthumbnail x=" << x << " y=" << y;
                docBroker->forwardToChild(client_from_this(), renderThumbnailCmd.str());
            }
        }

        if (payload->contains("url"))
        {
            std::string json(payload->data().data(), payload->size());
            const auto it = json.find('{');
            const std::string prefix = json.substr(0, it);
            json.erase(0, it); // Remove the prefix to parse the purse JSON part.

            Poco::JSON::Object::Ptr object;
            if (JsonUtil::parseJSON(json, object))
            {
                const std::string url = JsonUtil::getJSONValue<std::string>(object, "url");
                if (!url.empty())
                {
                    const std::string id = JsonUtil::getJSONValue<std::string>(object, "id");
                    if (!id.empty())
                    {
                        docBroker->addEmbeddedMedia(
                            id, json); // Capture the original message with internal URL.

                        const std::string mediaUrl =
                            Uri::encode(createPublicURI("media", id, /*encode=*/false), "&");
                        const std::string mediaVTT =
                            Uri::encode(createPublicURI("mediavtt", id, /*encode=*/false), "&");
                        object->set("url", mediaUrl); // Replace the url with the public one.
                        object->set("mimeType", "video/mp4"); //FIXME: get this from the source json
                        if (!mediaVTT.empty())
                        {
                            object->set("srt", mediaVTT);
                        }

                        std::ostringstream mediaStr;
                        object->stringify(mediaStr);
                        const std::string msg = prefix + mediaStr.str();
                        forwardToClient(std::make_shared<Message>(msg, Message::Dir::Out));
                        return true;
                    }

                    LOG_ERR("Invalid embeddedmedia json without id: " << json);
                }
            }
        }

        // Non-Media graphic selsection.
        forwardToClient(payload);
        return true;
    }
    else if (tokens.equals(0, "formfieldbutton:")) {
        // Do not send redundant messages
        if (_lastSentFormFielButtonMessage == firstLine)
            return true;
        _lastSentFormFielButtonMessage = firstLine;
    }
    else if (tokens.equals(0, "canonicalidchange:")) {
        int viewId, canonicalId;
        if (getTokenInteger(tokens[1], "viewid", viewId) &&
            getTokenInteger(tokens[2], "canonicalid", canonicalId))
        {
            _canonicalViewId = CanonicalViewId(canonicalId);
        }
    }
#if (ENABLE_FEATURE_LOCK || ENABLE_FEATURE_RESTRICTION || ENABLE_DEBUG) && !MOBILEAPP
    else if (tokens.equals(0, "status:") && !isViewLoaded())
    {
        std::ostringstream blockingCommandStatus;
        blockingCommandStatus << "blockingcommandstatus isRestrictedUser="
                              << (CommandControl::RestrictionManager::isRestrictedUser() ? "true"
                                                                                         : "false")
                              << " isLockedUser="
                              << (CommandControl::LockManager::isLockedUser() ? "true" : "false");
#if ENABLE_DEBUG
        // Enable testing feature restriction
        const std::string restrictedCmds =
            CommandControl::RestrictionManager::getRestrictedCommandListString();
        if (!restrictedCmds.empty())
            blockingCommandStatus << " test_restrictedCommands=" << restrictedCmds;
#endif
        docBroker->forwardToChild(client_from_this(), blockingCommandStatus.str());
    }
#endif
    if (!isDocPasswordProtected())
    {
        std::optional<bool> result =
            handleOpenDocKitToClientMessage(payload, docBroker, saveAsSocket);
        if (result)
        {
            return *result;
        }
    }
    else
    {
        LOG_INF("Ignoring notification on password protected document: " << firstLine);
    }

    // Forward everything else.
    return forwardToClient(payload);
}

std::optional<bool>
ClientSession::handleOpenDocKitToClientMessage(const std::shared_ptr<Message>& payload,
                                               const std::shared_ptr<DocumentBroker>& docBroker,
                                               const std::shared_ptr<StreamSocket>& saveAsSocket)
{
    const auto& tokens = payload->tokens();
    const std::string& firstLine = payload->firstLine();

    if (tokens.equals(0, "tile:"))
    {
        assert(false && "Tile traffic should go through the DocumentBroker-LoKit WS.");
    }
    else if (tokens.equals(0, "jsdialog:") && _state == ClientSession::SessionState::LOADING)
    {
        docBroker->setInteractive(true);
    }
    else if (tokens.equals(0, "loaded:"))
    {
        // We expect to be in the Loading state, as set in
        // DocumentBroker::addSession().
        if (_state == ClientSession::SessionState::LOADING)
        {
            setState(ClientSession::SessionState::LIVE);

            if (firstLine.find("isfirst=true") != std::string::npos)
            {
                // The document has just loaded.
                docBroker->setInteractive(false);
                docBroker->setLoaded();

                // Wopi post load actions.
                if (_wopiFileInfo && !_wopiFileInfo->getTemplateSource().empty())
                {
                    LOG_DBG("Uploading template [" << _wopiFileInfo->getTemplateSource()
                                                   << "] to storage after loading.");
                    docBroker->uploadAfterLoadingTemplate(client_from_this());
                }
            }

            docBroker->onViewLoaded(client_from_this());

#if !MOBILEAPP
            Admin::instance().setViewLoadDuration(
                docBroker->getDocKey(), getId(),
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - _viewLoadStart));
#endif
        }
        else
        {
            LOG_WRN("Document loaded while we are not loading. Likely the client gave up and "
                    "abandoned the document");
        }
    }
    else if (tokens.equals(0, "status:"))
    {
        // position cursor for thumbnail rendering
        if (_thumbnailSession)
        {
            //check whether we have a target!
            std::ostringstream cmd;
            cmd << "{";
            cmd << "\"Name\":"
                   "{"
                   "\"type\":\"string\","
                   "\"value\":\"URL\""
                   "},"
                   "\"URL\":"
                   "{"
                   "\"type\":\"string\","
                   "\"value\":\"#";
            cmd << getThumbnailTarget();
            cmd << "\"}}";

            const std::string renderThumbnailCmd = "uno .uno:OpenHyperLink " + cmd.str();
            docBroker->forwardToChild(client_from_this(), renderThumbnailCmd);
        }

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var statusJsonVar = parser.parse(firstLine.substr(7));
        const Poco::SharedPtr<Poco::JSON::Object>& statusJsonObject =
            statusJsonVar.extract<Poco::JSON::Object::Ptr>();

        if (statusJsonObject->has("selectedpart"))
            _clientSelectedPart =
                std::atoi(statusJsonObject->get("selectedpart").toString().c_str());

        if (statusJsonObject->has("mode"))
            _clientSelectedMode = std::atoi(statusJsonObject->get("mode").toString().c_str());
        if (statusJsonObject->has("type"))
            _isTextDocument = statusJsonObject->get("type").toString() == "text";
        if (statusJsonObject->has("viewid"))
            _kitViewId = std::atoi(statusJsonObject->get("viewid").toString().c_str());

        // Forward the status response to the client.
        return forwardToClient(payload);
    }
    else if (tokens.equals(0, "statusupdate:"))
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var statusJsonVar = parser.parse(firstLine.substr(13));
        const Poco::SharedPtr<Poco::JSON::Object>& statusJsonObject =
            statusJsonVar.extract<Poco::JSON::Object::Ptr>();

        if (statusJsonObject->has("mode"))
            _clientSelectedMode = std::atoi(statusJsonObject->get("mode").toString().c_str());
    }
    else if (tokens.equals(0, "commandvalues:"))
    {
        const std::string stringJSON = payload->jsonString();
        if (!stringJSON.empty())
        {
            try
            {
                Poco::JSON::Parser parser;
                const Poco::Dynamic::Var result = parser.parse(stringJSON);
                const auto& object = result.extract<Poco::JSON::Object::Ptr>();
                const std::string commandName =
                    object->has("commandName") ? object->get("commandName").toString() : "";
                if (commandName == ".uno:CharFontName" || commandName == ".uno:StyleApply")
                {
                    // other commands should not be cached
                    docBroker->tileCache().saveTextStream(TileCache::StreamType::CmdValues,
                                                          commandName, payload->data());
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERR("commandvalues parsing failure: " << exception.what());
            }
        }
    }
    else if (tokens.equals(0, "invalidatetiles:"))
    {
        assert(firstLine.size() == payload->size() &&
               "Unexpected multiline data in invalidatetiles");

        // First forward invalidation
        bool ret = forwardToClient(payload);

        handleTileInvalidation(firstLine, docBroker);
        return ret;
    }
    else if (tokens.equals(0, "statechanged:"))
    {
        if (_thumbnailSession)
        {
            // fallback in case we setup target at first character in the text document,
            // or not existing target and we will not enter invalidatecursor second time
            std::ostringstream renderThumbnailCmd;
            auto position = getThumbnailPosition();
            renderThumbnailCmd << "getthumbnail x=" << position.first << " y=" << position.second;
            docBroker->forwardToChild(client_from_this(), renderThumbnailCmd.str());
        }
    }
    else if (tokens.equals(0, "invalidatecursor:"))
    {
        assert(firstLine.size() == payload->size() &&
               "Unexpected multiline data in invalidatecursor");

        const std::string stringJSON = payload->jsonString();
        Poco::JSON::Parser parser;
        try
        {
            const Poco::Dynamic::Var result = parser.parse(stringJSON);
            const auto& object = result.extract<Poco::JSON::Object::Ptr>();
            std::string rectangle = object->get("rectangle").toString();
            StringVector rectangleTokens(StringVector::tokenize(std::move(rectangle), ','));
            int x = 0, y = 0, w = 0, h = 0;
            if (rectangleTokens.size() > 2 && stringToInteger(rectangleTokens[0], x) &&
                stringToInteger(rectangleTokens[1], y))
            {
                if (rectangleTokens.size() > 3)
                {
                    stringToInteger(rectangleTokens[2], w);
                    stringToInteger(rectangleTokens[3], h);
                }

                docBroker->invalidateCursor(x, y, w, h);

                // session used for thumbnailing and target already was set
                if (_thumbnailSession)
                {
                    setThumbnailPosition(std::make_pair(x, y));

                    bool cursorAlreadyAtTargetPosition = getThumbnailTarget().empty();
                    if (cursorAlreadyAtTargetPosition)
                    {
                        std::ostringstream renderThumbnailCmd;
                        renderThumbnailCmd << "getthumbnail x=" << x << " y=" << y;
                        docBroker->forwardToChild(client_from_this(), renderThumbnailCmd.str());
                    }
                    else
                    {
                        // this is initial cursor position message
                        // wait for second invalidatecursor message
                        // reset target so we will proceed next time
                        setThumbnailTarget(std::string());
                    }
                }
            }
            else
            {
                LOG_ERR("Unable to parse " << firstLine);
            }
        }
        catch (const std::exception& exception)
        {
            LOG_ERR("invalidatecursor parsing failure: " << exception.what());
        }
    }
#if !MOBILEAPP
    // don't sent it again, eg when some user joins
    else if (!_sentAudit && tokens.equals(0, "viewinfo:"))
    {
        bool status = forwardToClient(payload);

        if (docBroker)
        {
            _sentAudit = true;
            // send information about admin user
            const std::string admin = std::string("adminuser: ") + getIsAdminUserStatus();
            forwardToClient(std::make_shared<Message>(admin, Message::Dir::Out));

            // send server audit results after we received information about users (who is admin)
            const ServerAuditUtil& serverAudit = docBroker->getServerAudit();
            std::string audit =
                serverAudit.isDisabled() ? "disabled" : serverAudit.getResultsJSON();
            const std::string auditMessage = std::string("serveraudit: ") + audit;
            forwardToClient(std::make_shared<Message>(auditMessage, Message::Dir::Out));
        }

        return status;
    }
#endif
    else if (tokens.equals(0, "extractedlinktargets:"))
    {
#if !MOBILEAPP
        if (_aiToolLoop && _aiToolLoop->awaitingKitResponse)
        {
            _aiToolLoop->awaitingKitResponse = false;
            continueAIToolLoop(_aiToolLoop->pendingToolCallId, payload->jsonString());
            return true;
        }
#endif

        LOG_TRC("Sending extracted link targets response.");
        if (!saveAsSocket)
            LOG_ERR("Error in extractedlinktargets: not in isConvertTo mode");
        else
        {
            http::Response httpResponse(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(httpResponse);
            httpResponse.set("Last-Modified", Util::getHttpTimeNow());
            httpResponse.set("X-Content-Type-Options", "nosniff");
            httpResponse.setBody(payload->jsonString(), "application/json");
            saveAsSocket->sendAndShutdown(httpResponse);
        }

        // Now terminate.
        docBroker->closeDocument("extractedlinktargets");
        return true;
    }
    else if (tokens.equals(0, "extracteddocumentstructure:"))
    {
#if !MOBILEAPP
        if (_aiToolLoop && _aiToolLoop->awaitingKitResponse)
        {
            _aiToolLoop->awaitingKitResponse = false;
            continueAIToolLoop(_aiToolLoop->pendingToolCallId, payload->jsonString());
            return true;
        }
#endif

        LOG_TRC("Sending extracted document structure response.");
        if (!saveAsSocket)
            LOG_ERR("Error in extracteddocumentstructure: not in isConvertTo mode");
        else
        {
            http::Response httpResponse(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(httpResponse);
            httpResponse.set("Last-Modified", Util::getHttpTimeNow());
            httpResponse.set("X-Content-Type-Options", "nosniff");
            httpResponse.setBody(payload->jsonString(), "application/json");
            saveAsSocket->sendAndShutdown(httpResponse);
        }

        // Now terminate.
        docBroker->closeDocument("extracteddocumentstructure");
        return true;
    }
    else if (tokens.equals(0, "transformeddocumentstructure:"))
    {
#if !MOBILEAPP
        if (_aiToolLoop && _aiToolLoop->awaitingKitResponse)
        {
            _aiToolLoop->awaitingKitResponse = false;
            continueAIToolLoop(_aiToolLoop->pendingToolCallId, payload->jsonString());
            return true;
        }
#endif

        // TODO: This branch is unreachable. Kit never sends
        // transformeddocumentstructure: in the MCP/convert-to path -
        // the transform modifies the document in-place via
        // postUnoCommand(..., false), and ConvertToBroker::setLoaded()
        // triggers saveas which returns the modified document.
        LOG_TRC("Sending transformed document structure response.");
        if (!saveAsSocket)
            LOG_ERR("Error in transformeddocumentstructure: not in isConvertTo mode");
        else
        {
            http::Response httpResponse(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(httpResponse);
            httpResponse.set("Last-Modified", Util::getHttpTimeNow());
            httpResponse.set("X-Content-Type-Options", "nosniff");
            httpResponse.setBody(payload->jsonString(), "application/json");
            saveAsSocket->sendAndShutdown(httpResponse);
        }

        // Now terminate.
        docBroker->closeDocument("transformeddocumentstructure");
        return true;
    }
    else if (tokens.equals(0, "sendthumbnail:"))
    {
        LOG_TRC("Sending get-thumbnail response.");
        if (!saveAsSocket)
            LOG_ERR("Error in sendthumbnail: not in isConvertTo mode");
        else
        {
            bool error = false;

            if (firstLine.find("error") != std::string::npos)
                error = true;

            if (!error)
            {
                int firstLineSize = firstLine.size() + 1;
                std::string thumbnail(payload->data().data() + firstLineSize,
                                      payload->data().size() - firstLineSize);

                http::Response httpResponse(http::StatusCode::OK);
                FileServerRequestHandler::hstsHeaders(httpResponse);
                httpResponse.set("Last-Modified", Util::getHttpTimeNow());
                httpResponse.set("X-Content-Type-Options", "nosniff");
                httpResponse.setBody(std::move(thumbnail), "image/png");
                saveAsSocket->sendAndShutdown(httpResponse);
            }

            if (error)
            {
                http::Response httpResponse(http::StatusCode::InternalServerError);
                httpResponse.setContentLength(0);
                saveAsSocket->sendAndShutdown(httpResponse);
            }
        }

        docBroker->closeDocument("thumbnailgenerated");
    }

    // Fall-through, as we couldn't handle the message.
    return std::nullopt;
}

/// Map a file extension to a document type for password-protected icons.
static std::string getDocTypeFromExtension(const std::string& ext)
{
    static const std::unordered_map<std::string, std::string> extToType = {
        // Writer
        { "odt", "writer" }, { "fodt", "writer" }, { "doc", "writer" }, { "docx", "writer" },
        { "docm", "writer" }, { "dot", "writer" }, { "dotx", "writer" }, { "dotm", "writer" },
        { "rtf", "writer" }, { "txt", "writer" }, { "wpd", "writer" }, { "wps", "writer" },
        { "sxw", "writer" }, { "stw", "writer" }, { "ott", "writer" }, { "otm", "writer" },
        { "hwp", "writer" }, { "wri", "writer" }, { "abw", "writer" }, { "pages", "writer" },
        // Calc
        { "ods", "calc" }, { "fods", "calc" }, { "xls", "calc" }, { "xlsx", "calc" },
        { "xlsm", "calc" }, { "xlsb", "calc" }, { "xla", "calc" }, { "xltx", "calc" },
        { "xltm", "calc" }, { "csv", "calc" }, { "tsv", "calc" }, { "sxc", "calc" },
        { "stc", "calc" }, { "ots", "calc" }, { "dbf", "calc" }, { "numbers", "calc" },
        // Impress
        { "odp", "impress" }, { "fodp", "impress" }, { "ppt", "impress" }, { "pptx", "impress" },
        { "pptm", "impress" }, { "pot", "impress" }, { "potx", "impress" }, { "potm", "impress" },
        { "ppsx", "impress" }, { "sxi", "impress" }, { "sti", "impress" }, { "otp", "impress" },
        { "key", "impress" },
        // Draw
        { "odg", "draw" }, { "fodg", "draw" }, { "vsd", "draw" }, { "vss", "draw" },
        { "pub", "draw" }, { "sxd", "draw" }, { "std", "draw" }, { "otg", "draw" },
        { "cdr", "draw" }, { "wpg", "draw" }, { "cgm", "draw" }, { "emf", "draw" },
        { "wmf", "draw" },
    };

    auto it = extToType.find(ext);
    return (it != extToType.end()) ? it->second : "writer";
}

void ClientSession::abortConversion(const std::shared_ptr<DocumentBroker>& docBroker,
                                    const std::shared_ptr<StreamSocket>& saveAsSocket,
                                    std::string errorKind)
{
    assert(_isConvertTo && "Expected convert-to context");

    LOG_DBG("Conversion request of [" << docBroker->getDocKey() << "] failed: " << errorKind);
    if (!saveAsSocket)
        LOG_ERR("Error saveas socket missing in isConvertTo mode");
    else if (errorKind == "passwordrequired:to-view" ||
             errorKind == "passwordrequired:to-modify")
    {
        // Return a locked document icon as the thumbnail.
        const std::string docPath = docBroker->getPublicUri().getPath();
        const auto dotPos = docPath.find_last_of('.');
        const std::string ext = (dotPos != std::string::npos)
                                    ? docPath.substr(dotPos + 1)
                                    : std::string();
        const std::string docType = getDocTypeFromExtension(ext);

        const std::string iconPath = COOLWSD::FileServerRoot +
            "/browser/dist/images/password-protected-" + docType + ".png";

        std::vector<char> iconData;
        if (FileUtil::readFile(iconPath, iconData) > 0)
        {
            http::Response response(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(response);
            response.setBody(std::string(iconData.data(), iconData.size()), "image/png");
            response.set("X-ERROR-KIND", std::move(errorKind));
            saveAsSocket->sendAndShutdown(response);
        }
        else
        {
            LOG_ERR("Failed to read locked document icon: " << iconPath);
            http::Response response(http::StatusCode::Unauthorized);
            response.set("X-ERROR-KIND", std::move(errorKind));
            saveAsSocket->sendAndShutdown(response);
        }
    }
    else
    {
        http::Response response(http::StatusCode::InternalServerError);
        response.set("X-ERROR-KIND", std::move(errorKind));
        saveAsSocket->sendAndShutdown(response);
    }

    // Conversion failed, cleanup fake session.
    LOG_DBG("Removing save-as ClientSession after conversion error.");
    // Remove us.
    docBroker->removeSession(client_from_this());
    // Now terminate.
    docBroker->stop("Aborting saveas handler.");
}

#if !MOBILEAPP
bool ClientSession::handleSaveAs(const std::shared_ptr<Message>& payload,
                                 const std::shared_ptr<DocumentBroker>& docBroker,
                                 const std::shared_ptr<StreamSocket>& saveAsSocket)
{
    const auto& tokens = payload->tokens();
    const std::string& firstLine = payload->firstLine();

    bool isExportAs = tokens.equals(0, "exportas:");

    std::string encodedURL;
    if (!getTokenString(tokens[1], "url", encodedURL))
    {
        LOG_ERR("Bad syntax for: " << firstLine);
        // we must not return early with convert-to so that we clean up
        // the session
        if (!_isConvertTo)
        {
            sendTextFrameAndLogError("error: cmd=saveas kind=syntax");
            return false;
        }
    }

    std::string encodedWopiFilename;
    if (!_isConvertTo && !getTokenString(tokens[2], "filename", encodedWopiFilename))
    {
        LOG_ERR("Bad syntax for: " << firstLine);
        sendTextFrameAndLogError("error: cmd=saveas kind=syntax");
        return false;
    }

    // Save-as completed, inform the ClientSession.
    const std::string wopiFilename = Uri::decode(encodedWopiFilename);

    // URI constructor implicitly decodes when it gets std::string as param
    Poco::URI resultURL(encodedURL);

    // Prepend the jail path in the normal (non-nocaps) case
    if (resultURL.getScheme() == "file" && !COOLWSD::NoCapsForKit)
    {
        // getPath() already returns the decoded path (Poco::URI decodes
        // percent-encoded sequences internally), so no extra Uri::decode().
        std::string relative = resultURL.getPath();

        if (relative.size() > 0 && relative[0] == '/')
            relative = relative.substr(1);

        // Rewrite file:// URLs to be visible to the outside world.
        const Path path(FileUtil::buildLocalPathToJail(
            COOLWSD::EnableMountNamespaces, docBroker->getJailRoot(), std::move(relative)));
        if (Poco::File(path).exists())
        {
            // Encode path for special characters (i.e '%') since Poco::URI::setPath implicitly decodes the input param
            std::string encodedPath;
            Poco::URI::encode(path.toString(), "", encodedPath);

            resultURL.setPath(encodedPath);
        }
        else
        {
            // Blank for failure.
            LOG_DBG("SaveAs produced no output in '" << path.toString()
                                                     << "', producing blank url.");
            resultURL.clear();
        }
    }

    LOG_TRC("Save-as URL: " << resultURL.toString());

    if (!_isConvertTo)
    {
        // Normal SaveAs - save to Storage and log result.
        if (resultURL.getScheme() == "file" && !resultURL.getPath().empty())
        {
            // this also sends the saveas: result
            LOG_TRC("Save-as path: " << resultURL.getPath());
            docBroker->uploadAsToStorage(client_from_this(), resultURL.getPath(), wopiFilename,
                                         false, isExportAs);
        }
        else
            sendTextFrameAndLogError("error: cmd=storage kind=savefailed");
    }
    else
    {
        // using the convert-to REST API
        // TODO: Send back error when there is no output.
        if (!resultURL.getPath().empty())
        {
            LOG_TRC("Sending file: " << resultURL.getPath());

            const std::string fileName = Poco::Path(resultURL.getPath()).getFileName();
            http::Response response(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(response);
            if (!fileName.empty())
                response.set("Content-Disposition", "attachment; filename=\"" + fileName + '"');
            response.setContentType("application/octet-stream");

            if (!saveAsSocket)
                LOG_ERR("Error saveas socket missing in isConvertTo mode");
            else
                HttpHelper::sendFileAndShutdown(saveAsSocket, resultURL.getPath(), response);
        }

        // Conversion is done, cleanup this fake session.
        LOG_TRC("Removing save-as ClientSession after conversion.");

        // Remove us.
        docBroker->removeSession(client_from_this());

        // Now terminate.
        docBroker->stop("Finished saveas handler.");
    }

    return true;
}
#endif // !MOBILEAPP

bool ClientSession::forwardToClient(const std::shared_ptr<Message>& payload)
{
    if (isCloseFrame())
    {
        LOG_TRC("peer began the closing handshake. Dropping forward message [" << payload->abbr()
                                                                               << ']');
        return true;
    }

    enqueueSendMessage(payload);
    return true;
}

void ClientSession::enqueueSendMessage(const std::shared_ptr<Message>& data)
{
    if (isCloseFrame())
    {
        LOG_TRC("Connection closed, dropping message " << data->id());
        return;
    }

    const std::shared_ptr<DocumentBroker> docBroker = _docBroker.lock();
    LOG_CHECK_RET(docBroker && "Null DocumentBroker instance", );
    docBroker->ASSERT_CORRECT_THREAD();

    TileWireId wireId = 0;
    bool haveWireId = false;
    if (data->firstTokenMatches("tile:") ||
        data->firstTokenMatches("delta:"))
    {
        // Avoid sending tile or delta if it has the same wireID as the
        // previously sent tile
        wireId = TileDesc::parse(data->firstLine()).getWireId();
        haveWireId = true;
    }

    LOG_TRC("Enqueueing client message " << data->id());
    const std::size_t sizeBefore = _senderQueue.size();
    const std::size_t newSize = _senderQueue.enqueue(data);

    // Track sent tile
    if (haveWireId && sizeBefore != newSize)
        addTileOnFly(wireId);
}

void ClientSession::addTileOnFly(TileWireId wireId)
{
    _tilesOnFly.emplace_back(wireId, std::chrono::steady_clock::now());
}

size_t ClientSession::getTilesOnFlyUpperLimit() const
{
    // How many tiles we have on the visible area, set the upper limit accordingly
    Util::Rectangle normalizedVisArea = getNormalizedVisibleArea();

    float tilesOnFlyUpperLimit = 0;
    if (normalizedVisArea.hasSurface() && getTileWidthInTwips() != 0 && getTileHeightInTwips() != 0)
    {
        const int tilesFitOnWidth = (normalizedVisArea.getRight() / getTileWidthInTwips()) -
                                    (normalizedVisArea.getLeft() / getTileWidthInTwips()) + 1;
        const int tilesFitOnHeight = (normalizedVisArea.getBottom() / getTileHeightInTwips()) -
                                     (normalizedVisArea.getTop() / getTileHeightInTwips()) + 1;
        const int tilesInVisArea = tilesFitOnWidth * tilesFitOnHeight;

        tilesOnFlyUpperLimit = std::max(TILES_ON_FLY_MIN_UPPER_LIMIT, tilesInVisArea * 1.1f);
    }
    else
    {
        tilesOnFlyUpperLimit = 200; // Have a big number here to get all tiles requested by file opening
    }
    return tilesOnFlyUpperLimit;
}

void ClientSession::removeOutdatedTilesOnFly(const std::chrono::steady_clock::time_point now)
{
    size_t dropped = 0;
    const auto highTimeoutMs = std::chrono::milliseconds(TILE_ROUNDTRIP_TIMEOUT_MS);
    const auto lowTimeoutMs = std::chrono::milliseconds((int)(0.9 * TILE_ROUNDTRIP_TIMEOUT_MS));
    // Check only the beginning of the list, tiles are ordered by timestamp
    while(!_tilesOnFly.empty())
    {
        auto tileIter = _tilesOnFly.begin();
        const auto elapsedTimeMs = std::chrono::duration_cast<
            std::chrono::milliseconds>(now - tileIter->second);
        if (elapsedTimeMs > highTimeoutMs ||
            // once we start dropping - drop lots in a similar range of time
            (dropped > 0 && elapsedTimeMs > lowTimeoutMs))
        {
            LOG_TRC("Tracker tileID " << tileIter->first << " was dropped because of time out ("
                    << elapsedTimeMs
                    << "). Tileprocessed message did not arrive in time.");
            dropped++;
            _tilesOnFly.erase(tileIter);
        }
        else
            break;
    }
    if (dropped > 0)
        LOG_WRN("client not consuming tiles; stalled for " << (TILE_ROUNDTRIP_TIMEOUT_MS/1000) << " seconds: removed tracking for " << dropped << " on the fly tiles");
}

Util::Rectangle ClientSession::getNormalizedVisibleArea() const
{
    Util::Rectangle normalizedVisArea;
    normalizedVisArea.setLeft(std::max(_clientVisibleArea.getLeft(), 0));
    normalizedVisArea.setTop(std::max(_clientVisibleArea.getTop(), 0));
    normalizedVisArea.setRight(_clientVisibleArea.getRight());
    normalizedVisArea.setBottom(_clientVisibleArea.getBottom());
    return normalizedVisArea;
}

void ClientSession::onDisconnect()
{
    LOG_INF("Disconnected, current global number of connections (inclusive): "
            << COOLWSD::NumConnections);

    const std::shared_ptr<DocumentBroker> docBroker = getDocumentBroker();
    LOG_CHECK_RET(docBroker && "Null DocumentBroker instance", );
    docBroker->ASSERT_CORRECT_THREAD();
    const std::string docKey = docBroker->getDocKey();

    // Keep self alive, so that our own dtor runs only at the end of this function. Without this,
    // removeSession() may destroy us and then we can't call our own member functions anymore.
    std::shared_ptr<ClientSession> session = client_from_this();
    try
    {
        // Connection terminated. Destroy session.
        LOG_DBG("on docKey [" << docKey << "] terminated. Cleaning up");

        docBroker->removeSession(session);
    }
    catch (const UnauthorizedRequestException& exc)
    {
        LOG_ERR("Error in client request handler: " << exc.toString());
        const std::string status = "error: cmd=internal kind=unauthorized";
        LOG_TRC("Sending to Client [" << status << ']');
        sendTextFrame(status);
        // We are disconnecting, no need to close the socket here.
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Error in client request handler: " << exc.what());
    }

    try
    {
        if (isCloseFrame())
        {
            LOG_TRC("Normal close handshake.");
            // Client initiated close handshake
            // respond with close frame
            shutdownNormal();
        }
        else if (!SigUtil::getShutdownRequestFlag())
        {
            // something wrong, with internal exceptions
            LOG_TRC("Abnormal close handshake.");
            closeFrame();
            shutdownGoingAway();
        }
        else
        {
            LOG_TRC("Server recycling.");
            closeFrame();
            shutdownGoingAway();
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception while closing socket for docKey [" << docKey << "]: " << exc.what());
    }
}

void ClientSession::dumpState(std::ostream& os)
{
    Session::dumpState(os);
    const std::shared_ptr<DocumentBroker> docBroker = _docBroker.lock();

    os << "\t\tisLive: " << isLive()
       << "\n\t\tisViewLoaded: " << isViewLoaded()
       << "\n\t\tisDocumentOwner: " << isDocumentOwner()
       << "\n\t\tstate: " << name(_state)
       << "\n\t\tkeyEvents: " << _keyEvents
//       << "\n\t\tvisibleArea: " << _clientVisibleArea
       << "\n\t\tclientSelectedPart: " << _clientSelectedPart
       << "\n\t\ttile size Pixel: " << _tileWidthPixel << 'x' << _tileHeightPixel
       << "\n\t\ttile size Twips: " << _tileWidthTwips << 'x' << _tileHeightTwips
       << "\n\t\tkit ViewId: " << _kitViewId
       << "\n\t\tour URL (un-trusted): " << _serverURL.getSubURLForEndpoint("")
       << "\n\t\tisTextDocument: " << _isTextDocument
       << "\n\t\tclipboardKeys[0]: " << _clipboardKeys[0]
       << "\n\t\tclipboardKeys[1]: " << _clipboardKeys[1]
       << "\n\t\tclip sockets: " << _clipSockets.size()
       << "\n\t\tproxy access:: " << _proxyAccess
       << "\n\t\tclientSelectedMode: " << _clientSelectedMode
       << "\n\t\trequestedTiles: " << getRequestedTiles().size()
       << "\n\t\tbeingRendered: " << (!docBroker ? -1 : docBroker->tileCache().countTilesBeingRenderedForSession(client_from_this(), std::chrono::steady_clock::now()));

    if (_protocol)
    {
        uint64_t sent = 0;
        uint64_t recv = 0;
        _protocol->getIOStats(sent, recv);
        os << "\n\t\tsent: " << sent / 1024 << " Kbytes";
        os << "\n\t\trecv: " << recv / 1024 << " Kbytes";
        os << "\n\t\tsent/keystroke: " << sent / 1024. / _keyEvents << " Kbytes";
    }

    os << "\n\t\tonFlyUpperLimit: " << getTilesOnFlyUpperLimit();
    os << "\n\t\tonFlyCount: " << getTilesOnFlyCount();
    if (_tilesOnFly.size() > 0)
        os << " between wid: " << _tilesOnFly.front().first << " as of " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _tilesOnFly.front().second) << " ms "
           << " and wid: " << _tilesOnFly.back().first << " as of " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _tilesOnFly.back().second) << " ms ";

    os << '\n';
    _senderQueue.dumpState(os);

    // FIXME: need to dump other bits ...
}

const std::string &ClientSession::getOrCreateProxyAccess()
{
    if (_proxyAccess.empty())
        _proxyAccess = Util::rng::getHexString(
            ProxyAccessTokenLengthBytes);
    return _proxyAccess;
}

void ClientSession::handleTileInvalidation(const std::string& message,
    const std::shared_ptr<DocumentBroker>& docBroker)
{
    docBroker->invalidateTiles(message, getCanonicalViewId());

    // Skip requesting new tiles if we don't have client visible area data yet.
    if(!_clientVisibleArea.hasSurface() ||
       _tileWidthPixel == 0 || _tileHeightPixel == 0 ||
       _tileWidthTwips == 0 || _tileHeightTwips == 0 ||
       (_clientSelectedPart == -1 && !_isTextDocument))
    {
        LOG_TRC("No visible area received yet - skip invalidation");
        return;
    }

    // While saving / shutting down we can get big invalidatiions: ignore them
    if (isCloseFrame()) {
        LOG_TRC("Session [" << getId() << "] ignoring invalidation during close: '" << message);
        return;
    }

    int part = 0, mode = 0;
    TileWireId wireId = 0;
    Util::Rectangle invalidateRect = TileCache::parseInvalidateMsg(message, part, mode, wireId);

    constexpr SplitPaneName panes[4] = {
        TOPLEFT_PANE,
        TOPRIGHT_PANE,
        BOTTOMLEFT_PANE,
        BOTTOMRIGHT_PANE
    };
    Util::Rectangle paneRects[4];
    int numPanes = 0;
    for(int i = 0; i < 4; ++i)
    {
        if(!isSplitPane(panes[i]))
            continue;

        Util::Rectangle rect = getNormalizedVisiblePaneArea(panes[i]);
        if (rect.intersects(invalidateRect)) {
            paneRects[numPanes++] = rect;
        }
    }

    // We can ignore the invalidation if it's outside of all split-panes.
    if(!numPanes)
        return;

    if( part == -1 ) // If no part is specified we use the part used by the client
        part = _clientSelectedPart;

    CanonicalViewId canonicalViewId = getCanonicalViewId();

    std::vector<TileDesc> invalidTiles;
    if((part == _clientSelectedPart && mode == _clientSelectedMode) || _isTextDocument)
    {
        for(int paneIdx = 0; paneIdx < numPanes; ++paneIdx)
        {
            const Util::Rectangle& normalizedVisArea = paneRects[paneIdx];
            int lastVertTile = std::ceil(normalizedVisArea.getBottom() / static_cast<double>(_tileHeightTwips));
            int lastHoriTile = std::ceil(normalizedVisArea.getRight() / static_cast<double>(_tileWidthTwips));

            // Iterate through visible tiles
            for(int i = normalizedVisArea.getTop() / _tileHeightTwips; i <= lastVertTile; ++i)
            {
                for(int j = normalizedVisArea.getLeft() / _tileWidthTwips; j <= lastHoriTile; ++j)
                {
                    // Find tiles affected by invalidation
                    Util::Rectangle tileRect (j * _tileWidthTwips, i * _tileHeightTwips, _tileWidthTwips, _tileHeightTwips);
                    if(invalidateRect.intersects(tileRect))
                    {
                        TileDesc desc(canonicalViewId, part, mode,
                                      _tileWidthPixel, _tileHeightPixel,
                                      j * _tileWidthTwips, i * _tileHeightTwips,
                                      _tileWidthTwips, _tileHeightTwips, -1, 0, -1);

                        bool dup = false;
                        // Check we don't have duplicates
                        for (const auto &it : invalidTiles)
                        {
                            if (it == desc)
                            {
                                LOG_TRC("Duplicate tile skipped from invalidation " << desc.debugName());
                                dup = true;
                                break;
                            }
                        }

                        if (!dup)
                        {
                            invalidTiles.push_back(desc);

                            TileWireId makeDelta = 1;
                            // FIXME: mobile with no TileCache & flushed kit cache
                            // FIXME: out of (a)sync kit vs. TileCache re: keyframes ?
                            if (getDocumentBroker()->hasTileCache() &&
                                !getDocumentBroker()->tileCache().lookupTile(desc))
                                makeDelta = 0; // force keyframe
                            invalidTiles.back().setOldWireId(makeDelta);
                            invalidTiles.back().setWireId(0);
                        }
                    }
                }
            }
        }
    }

    if(!invalidTiles.empty())
    {
        TileCombined tileCombined = TileCombined::create(invalidTiles);
        tileCombined.setCanonicalViewId(canonicalViewId);
        docBroker->handleTileCombinedRequest(tileCombined, false, client_from_this());
    }
}

bool ClientSession::isSplitPane(const SplitPaneName paneName) const
{
    if (paneName == BOTTOMRIGHT_PANE)
        return true;

    if (paneName == TOPLEFT_PANE)
        return (_splitX && _splitY);

    if (paneName == TOPRIGHT_PANE)
        return _splitY;

    if (paneName == BOTTOMLEFT_PANE)
        return _splitX;

    return false;
}

Util::Rectangle ClientSession::getNormalizedVisiblePaneArea(const SplitPaneName paneName) const
{
    Util::Rectangle normalizedVisArea = getNormalizedVisibleArea();
    if (!_splitX && !_splitY)
        return paneName == BOTTOMRIGHT_PANE ? normalizedVisArea : Util::Rectangle();

    int freeStartX = normalizedVisArea.getLeft() + _splitX;
    int freeStartY = normalizedVisArea.getTop()  + _splitY;
    int freeWidth = normalizedVisArea.getWidth() - _splitX;
    int freeHeight = normalizedVisArea.getHeight() - _splitY;

    switch (paneName)
    {
    case BOTTOMRIGHT_PANE:
        return Util::Rectangle(freeStartX, freeStartY, freeWidth, freeHeight);
    case TOPLEFT_PANE:
        return (_splitX && _splitY) ? Util::Rectangle(0, 0, _splitX, _splitY) : Util::Rectangle();
    case TOPRIGHT_PANE:
        return _splitY ? Util::Rectangle(freeStartX, 0, freeWidth, _splitY) : Util::Rectangle();
    case BOTTOMLEFT_PANE:
        return _splitX ? Util::Rectangle(0, freeStartY, _splitX, freeHeight) : Util::Rectangle();
    default:
        assert(false && "Unknown split-pane name");
    }

    return Util::Rectangle();
}

bool ClientSession::isTileInsideVisibleArea(const TileDesc& tile) const
{
    if (!_splitX && !_splitY)
    {
        return tile.intersects( _clientVisibleArea );
    }

    constexpr SplitPaneName panes[4] = {
        TOPLEFT_PANE,
        TOPRIGHT_PANE,
        BOTTOMLEFT_PANE,
        BOTTOMRIGHT_PANE
    };

    for (int i = 0; i < 4; ++i)
    {
        if (!isSplitPane(panes[i]))
            continue;

        const Util::Rectangle paneRect = getNormalizedVisiblePaneArea(panes[i]);
        if( tile.intersects( paneRect ) )
            return true;
    }

    return false;
}

// This removes the <div id="meta-origin" ...> tag which was added in
// ClientSession::postProcessCopyPayload(), else the payload parsing
// in ChildSession::setClipboard() will fail.
// To see why, refer
// 1. ChildSession::getClipboard() where the data for various
//    flavours along with flavour-type and length fields are packed into the payload.
// 2. The clipboard payload parsing code in ClipboardData::read().
bool ClientSession::preProcessSetClipboardPayload(std::istream& in, std::ostream& out)
{
    if (!Util::copyToMatch(in, out, R"(<div id="meta-origin" data-coolorigin=")"))
        return false;

    const std::string_view endtag = "\">\n";
    // discard this tag
    if (!Util::seekToMatch(in, endtag))
    {
        LOG_DBG("Found unbalanced starting meta <div> tag in setclipboard payload.");
        return false;
    }
    in.seekg(endtag.size(), std::ios_base::cur);

    if (!Util::copyToMatch(in, out, "</div></body>"))
    {
        LOG_DBG("Found unbalanced ending meta <div> tag in setclipboard payload.");
        return false;
    }

    in.seekg(strlen("</div>"), std::ios_base::cur);

    // write the remainder to out
    Poco::StreamCopier::copyStream(in, out);

    return true;
}

std::string ClientSession::processSVGContent(const std::string& svg)
{
    const std::shared_ptr<DocumentBroker> docBroker = _docBroker.lock();
    if (!docBroker)
    {
        LOG_ERR("No DocBroker to process SVG content");
        return svg;
    }

    bool broken = false;
    std::ostringstream oss;
    std::string::size_type pos = 0;
    for (;;)
    {
        constexpr std::string_view prefix = "src=\"file:///tmp/";
        const auto start = svg.find(prefix, pos);
        if (start == std::string::npos)
        {
            // Copy the rest and finish.
            oss << svg.substr(pos);
            break;
        }

        const auto startFilename = start + prefix.size();
        const auto end = svg.find('"', startFilename);
        if (end == std::string::npos)
        {
            // Broken file; leave it as-is. Better to have no video than no slideshow.
            broken = true;
            break;
        }

        auto dot = svg.find('.', startFilename);
        if (dot == std::string::npos || dot > end)
            dot = end;

        const std::string id = svg.substr(startFilename, dot - startFilename);
        oss << svg.substr(pos, start - pos);

        // Store the original json with the internal, temporary, file URI.
        const std::string fileUrl = svg.substr(start + 5, end - start - 5);
        docBroker->addEmbeddedMedia(id, R"({ "action":"update","id":")" + id + R"(","url":")" +
                                            fileUrl + "\"}");

        const std::string mediaUrl =
            Uri::encode(createPublicURI("media", id, /*encode=*/false), "&");
        oss << "src=\"" << mediaUrl << '"';
        pos = end + 1;
    }

    return broken ? svg : oss.str();
}

std::string ClientSession::getIsAdminUserStatus() const
{
    return getIsAdminUser().has_value() ? (getIsAdminUser().value() ? "true" : "false") : "null";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
