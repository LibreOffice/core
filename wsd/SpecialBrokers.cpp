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
 * Implementation of special purpose brokers.
 * Classes: ThumbnailBroker, SystemTemplatesBroker
 */

#include <config.h>

#include "SpecialBrokers.hpp"

#include <common/Clipboard.hpp>
#include <common/CommandControl.hpp>
#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/FileServer.hpp>
#include <wsd/QuarantineUtil.hpp>
#include <wsd/TileCache.hpp>

#if !MOBILEAPP
#include <wopi/CheckFileInfo.hpp>
#include <net/HttpHelper.hpp>
#endif

#include <Poco/DigestStream.h>
#include <Poco/Exception.h>
#include <Poco/Path.h>
#include <Poco/SHA1Engine.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>

using namespace COOLProtocol;


void StatelessBatchBroker::removeFile(const std::string& uriOrig)
{
    // Remove and report errors on failure.
    FileUtil::removeFile(uriOrig);
    const std::string dir = Poco::Path(uriOrig).parent().toString();
    if (FileUtil::isEmptyDirectory(dir))
        FileUtil::removeFile(dir);
}

static std::atomic<std::size_t> convertToBrokerInstanceCounter;

std::size_t ConvertToBroker::getInstanceCount() { return convertToBrokerInstanceCounter; }

/// Split ",infilterOptions=..." out of the combined options string so that
/// import options go to the load command and export options go to saveas.
static std::pair<std::string, std::string> splitInFilterOptions(const std::string& options)
{
    const std::string tag = ",infilterOptions=";
    auto pos = options.find(tag);
    if (pos != std::string::npos)
        return { options.substr(0, pos), options.substr(pos + tag.size()) };
    return { options, std::string() };
}

ConvertToBroker::ConvertToBroker(const std::string& uri, const Poco::URI& uriPublic,
                                 const std::string& docKey, const std::string& format,
                                 const std::string& options, const std::string& lang)
    : StatelessBatchBroker(uri, uriPublic, docKey)
    , _format(format)
    , _options(splitInFilterOptions(options).first)
    , _inFilterOptions(splitInFilterOptions(options).second)
    , _lang(lang)
{
    LOG_TRC("Created ConvertToBroker: uri: ["
            << uri << "], uriPublic: [" << uriPublic.toString() << "], docKey: [" << docKey
            << "], format: [" << format << "], options: [" << _options << "], infilterOptions: ["
            << _inFilterOptions << "], lang: [" << lang << "].");

    CONFIG_STATIC const std::chrono::seconds limit_convert_secs(
        ConfigUtil::getConfigValue<std::chrono::seconds>("per_document.limit_convert_secs", 100));
    _limitLifeSeconds = limit_convert_secs;
    ++convertToBrokerInstanceCounter;
}

ConvertToBroker::~ConvertToBroker() {}

bool ConvertToBroker::startConversion(SocketDisposition& disposition, const std::string& id, const AdditionalFilePocoUris& additionalFileUrisPublic)
{
    std::shared_ptr<ConvertToBroker> docBroker =
        std::static_pointer_cast<ConvertToBroker>(shared_from_this());

    // Create a session to load the document.
    bool isReadOnly = docBroker->isReadOnly();
    if (isReadOnly && additionalFileUrisPublic.contains("compare"))
    {
        // Comparing means modifying the new document to have redlines against the baseline, so all
        // this to modify the throwaway document model.
        isReadOnly = false;
    }
    // FIXME: associate this with moveSocket (?)
    std::shared_ptr<ProtocolHandlerInterface> nullPtr;
    RequestDetails requestDetails("convert-to");
    _clientSession = std::make_shared<ClientSession>(nullPtr, id, docBroker, getPublicUri(),
                                                     isReadOnly, requestDetails,
                                                     additionalFileUrisPublic);
    _clientSession->construct();

    docBroker->setupTransfer(
        disposition,
        [docBroker](const std::shared_ptr<Socket>& moveSocket)
        {
            auto streamSocket = std::static_pointer_cast<StreamSocket>(moveSocket);
            docBroker->_clientSession->setSaveAsSocket(streamSocket);

            // First add and load the session.
            docBroker->addSession(docBroker->_clientSession);

            // Load the document manually and request saving in the target format.
            std::string encodedFrom;
            Poco::URI::encode(docBroker->getPublicUri().getPath(), "", encodedFrom);

            docBroker->sendStartMessage(docBroker->_clientSession, encodedFrom);

            // Save is done in the setLoaded
        });
    return true;
}

void ConvertToBroker::sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                                       const std::string& encodedFrom)
{
    // add batch mode, no interactive dialogs
    std::string load = "load url=" + encodedFrom + " batch=true";
    if (!getLang().empty())
        load += " lang=" + getLang();
    if (!_inFilterOptions.empty())
        load += " infilterOptions=" + _inFilterOptions;
    std::vector<char> loadRequest(load.begin(), load.end());
    clientSession->handleMessage(loadRequest);
}

void ExtractLinkTargetsBroker::sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                                                const std::string& encodedFrom)
{
    ConvertToBroker::sendStartMessage(clientSession, encodedFrom);

    const auto command = "extractlinktargets url=" + encodedFrom;
    forwardToChild(clientSession, command);
}

void ExtractDocumentStructureBroker::sendStartMessage(
    const std::shared_ptr<ClientSession>& clientSession, const std::string& encodedFrom)
{
    ConvertToBroker::sendStartMessage(clientSession, encodedFrom);

    std::string command = "extractdocumentstructure url=" + encodedFrom;
    if (!_filter.empty())
        command += " filter=" + _filter;
    forwardToChild(clientSession, command);
}

void TransformDocumentStructureBroker::sendStartMessage(
    const std::shared_ptr<ClientSession>& clientSession, const std::string& encodedFrom)
{
    ConvertToBroker::sendStartMessage(clientSession, encodedFrom);

    const auto command =
        "transformdocumentstructure url=" + encodedFrom + " transform=" + _transformJSON;
    forwardToChild(clientSession, command);
}

void GetThumbnailBroker::sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                                          const std::string& encodedFrom)
{
    clientSession->setThumbnailSession(true);
    clientSession->setThumbnailTarget(_target);

    ConvertToBroker::sendStartMessage(clientSession, encodedFrom);
}

void ConvertToBroker::dispose()
{
    if (!_uriOrig.empty())
    {
        convertToBrokerInstanceCounter--;
        removeFile(_uriOrig);
        _uriOrig.clear();
    }
}

void ConvertToBroker::setLoaded()
{
    DocumentBroker::setLoaded();

    if (isGetThumbnail())
        return;

    auto it = getAdditionalFileUrisJailed().find("compare");
    if (it != getAdditionalFileUrisJailed().end())
    {
        // We have a baseline to compare against, do that before saving.
        std::string unoCmd =
            "uno .uno:CompareDocuments { \"URL\": { \"type\": \"string\", \"value\": \"" +
            it->second +
            "\" }, \"NoAcceptDialog\": { \"type\": \"boolean\", \"value\": \"true\" } }";
        std::vector<char> unoRequest(unoCmd.begin(), unoCmd.end());
        _clientSession->handleMessage(unoRequest);
    }

    // FIXME: Check for security violations.
    Poco::Path toPath(getPublicUri().getPath());
    toPath.setExtension(_format);

    // file:///user/docs/filename.ext normally, file:///<jail-root>/user/docs/filename.ext in the nocaps case
    const std::string toJailURL = "file://" + (COOLWSD::NoCapsForKit ? getJailRoot() : "") +
                                  std::string(JAILED_DOCUMENT_ROOT) + toPath.getFileName();

    std::string encodedTo;
    Poco::URI::encode(toJailURL, "", encodedTo);

    // Convert it to the requested format.
    const std::string saveAsCmd =
        "saveas url=" + encodedTo + " format=" + _format + " options=" + _options;

    // Send the save request ...
    std::vector<char> saveasRequest(saveAsCmd.begin(), saveAsCmd.end());

    _clientSession->handleMessage(saveasRequest);
}

static std::atomic<std::size_t> renderSearchResultBrokerInstanceCounter;

std::size_t RenderSearchResultBroker::getInstanceCount()
{
    return renderSearchResultBrokerInstanceCounter;
}

RenderSearchResultBroker::RenderSearchResultBroker(
    std::string const& uri, Poco::URI const& uriPublic, std::string const& docKey,
    std::shared_ptr<std::vector<char>> const& searchResultContent)
    : StatelessBatchBroker(uri, uriPublic, docKey)
    , _searchResultContent(searchResultContent)
{
    LOG_TRC("Created RenderSearchResultBroker: uri: ["
            << uri << "], uriPublic: [" << uriPublic.toString() << "], docKey: [" << docKey
            << "].");
    convertToBrokerInstanceCounter++;
}

RenderSearchResultBroker::~RenderSearchResultBroker() {}

bool RenderSearchResultBroker::executeCommand(SocketDisposition& disposition, std::string const& id)
{
    std::shared_ptr<RenderSearchResultBroker> docBroker =
        std::static_pointer_cast<RenderSearchResultBroker>(shared_from_this());

    const bool isReadOnly = true;

    std::shared_ptr<ProtocolHandlerInterface> emptyProtocolHandler;
    RequestDetails requestDetails("render-search-result");
    _clientSession = std::make_shared<ClientSession>(emptyProtocolHandler, id, docBroker,
                                                     getPublicUri(), isReadOnly, requestDetails);
    _clientSession->construct();

    docBroker->setupTransfer(
        disposition,
        [docBroker](std::shared_ptr<Socket> const& moveSocket)
        {
            docBroker->setResponseSocket(std::static_pointer_cast<StreamSocket>(moveSocket));

            // First add and load the session.
            docBroker->addSession(docBroker->_clientSession);

            // Load the document manually.
            std::string encodedFrom;
            Poco::URI::encode(docBroker->getPublicUri().getPath(), "", encodedFrom);
            // add batch mode, no interactive dialogs
            const std::string _load = "load url=" + encodedFrom + " batch=true";
            std::vector<char> loadRequest(_load.begin(), _load.end());
            docBroker->_clientSession->handleMessage(loadRequest);
        });

    return true;
}

void RenderSearchResultBroker::setLoaded()
{
    DocumentBroker::setLoaded();

    // Send the rendersearchresult request ...
    const std::string renderSearchResultCmd = "rendersearchresult ";
    std::vector<char> renderSearchResultRequest(renderSearchResultCmd.begin(),
                                                renderSearchResultCmd.end());
    renderSearchResultRequest.resize(renderSearchResultCmd.size() + _searchResultContent->size());
    std::copy(_searchResultContent->begin(), _searchResultContent->end(),
              renderSearchResultRequest.begin() + renderSearchResultCmd.size());
    _clientSession->handleMessage(renderSearchResultRequest);
}

void RenderSearchResultBroker::dispose()
{
    if (!_uriOrig.empty())
    {
        renderSearchResultBrokerInstanceCounter--;
        removeFile(_uriOrig);
        _uriOrig.clear();
    }
}

bool RenderSearchResultBroker::handleInput(const std::shared_ptr<Message>& message)
{
    bool result = DocumentBroker::handleInput(message);

    if (result)
    {
        auto const& messageData = message->data();

        static std::string commandString = "rendersearchresult:\n";
        static std::vector<char> commandStringVector(commandString.begin(), commandString.end());

        if (messageData.size() >= commandStringVector.size())
        {
            bool equals = std::equal(commandStringVector.begin(), commandStringVector.end(),
                                      messageData.begin());
            if (equals)
            {
                _responseData.resize(messageData.size() - commandStringVector.size());
                std::copy(messageData.begin() + commandStringVector.size(), messageData.end(),
                          _responseData.begin());

                http::Response httpResponse(http::StatusCode::OK);
                FileServerRequestHandler::hstsHeaders(httpResponse);
                // really not ideal that the response works only with std::string
                httpResponse.setBody(std::string(_responseData.data(), _responseData.size()),
                                     "image/png");
                httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);

                std::shared_ptr<StreamSocket> socket(_socket.lock());
                if (socket)
                    socket->sendAndShutdown(httpResponse);
                else
                    LOG_ERR("Invalid socket while sending rendersearchresult response");

                removeSession(_clientSession);
                stop("Finished RenderSearchResult handler.");
            }
        }
    }
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
