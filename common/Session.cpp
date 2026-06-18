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
 * Base session class for document editing.
 * Classes: Session - Protocol message handling and session management
 */

#include <config.h>

#include "Session.hpp"

#include <common/Anonymizer.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>

#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <Poco/URI.h>

using namespace COOLProtocol;

using Poco::Exception;

Session::Session(const std::shared_ptr<ProtocolHandlerInterface>& protocol, const std::string& name,
                 const std::string& id, bool readOnly)
    : MessageHandlerInterface(protocol)
    , _id(id)
    , _name(name)
    , _watermarkOpacity(0.2)
    , _lastActivityTime(std::chrono::steady_clock::now())
    , _isAdminUser(std::nullopt)
    , _disconnected(false)
    , _isActive(true)
    , _isCloseFrame(false)
    , _writePermission(!readOnly)
    , _isWritable(!readOnly)
    , _isReadOnly(readOnly)
    , _isAllowChangeComments(false)
    , _haveDocPassword(false)
    , _isDocPasswordProtected(false)
    , _accessibilityState(false)
    , _disableVerifyHost(false)
{
}

Session::~Session()
{
}

bool Session::sendTextFrame(const char* buffer, const int length)
{
    if (!_protocol)
    {
        LOG_TRC("ERR - missing protocol " << getName() << ": Send: ["
                                          << getAbbreviatedMessage(buffer, length) << ']');
        return false;
    }

    LOG_TRC("Send: [" << getAbbreviatedMessage(buffer, length) << ']');
    return _protocol->sendTextMessage(std::string_view(buffer, length)) >= length;
}

bool Session::sendBinaryFrame(const char *buffer, int length)
{
    if (!_protocol)
    {
        LOG_TRC("ERR - missing protocol " << getName() << ": Send: " << std::to_string(length)
                                          << " binary bytes");
        return false;
    }

    LOG_TRC("Send: " << std::to_string(length) << " binary bytes");
    return _protocol->sendBinaryMessage(std::string_view(buffer, length)) >= length;
}

void Session::parseDocOptions(const StringVector& tokens, int& part, std::string& timestamp)
{
    // First token is the "load" command itself.
    std::size_t offset = 1;
    if (tokens.size() > 2 && tokens[1].find("part=") == 0)
    {
        (void)getTokenInteger(tokens[1], "part", part);
        ++offset;
    }

    for (std::size_t i = offset; i < tokens.size(); ++i)
    {
        std::string name;
        std::string value;
        if (!COOLProtocol::parseNameValuePair(tokens[i], name, value))
        {
            LOG_WRN("Unexpected doc options token [" << tokens[i] << "]. Skipping.");
            continue;
        }

        if (name == "url")
        {
            _docURL = std::move(value);
            ++offset;
        }
        else if (name == "jail")
        {
            _jailedFilePath = std::move(value);
            ++offset;
        }
        else if (name == "xjail")
        {
            _jailedFilePathAnonym = std::move(value);
            ++offset;
        }
        else if (name == "authorid")
        {
            _userId = Uri::decode(value);
            ++offset;
        }
        else if (name == "xauthorid")
        {
            _userIdAnonym = Uri::decode(value);
            ++offset;
        }
        else if (name == "author")
        {
            _userName = Uri::decode(value);
            ++offset;
        }
        else if (name == "xauthor")
        {
            _userNameAnonym = Uri::decode(value);
            ++offset;
        }
        else if (name == "authorextrainfo")
        {
            _userExtraInfo = Uri::decode(value);
            ++offset;
        }
        else if (name == "authorprivateinfo")
        {
            _userPrivateInfo = Uri::decode(value);
            ++offset;
        }
        else if (name == "signatureconfig")
        {
            if (_userPrivateInfo.empty())
            {
                LOG_WRN(
                    "signatureconfig: User private info not set, skipping signature configuration");
                ++offset;
                continue;
            }

            const std::string decodedSignatureData = Uri::decode(value);
            if (decodedSignatureData == "{}")
            {
                LOG_INF("signatureconfig: Empty signature data received, skipping processing");
                ++offset;
                continue;
            }

            Poco::JSON::Object::Ptr signatureDataObject;
            if (!JsonUtil::parseJSON(decodedSignatureData, signatureDataObject))
            {
                LOG_ERR("signatureconfig: Failed to parse signature data as JSON: "
                        << decodedSignatureData);
                ++offset;
                continue;
            }

            Poco::JSON::Object::Ptr userPrivateInfoObject;
            if (!JsonUtil::parseJSON(_userPrivateInfo, userPrivateInfoObject))
            {
                LOG_ERR("signatureconfig: Failed to parse user private info as JSON: "
                        << _userPrivateInfo);
                ++offset;
                continue;
            }

            setSignToUserPrivateConfig("SignatureCert", signatureDataObject,
                                             userPrivateInfoObject);
            setSignToUserPrivateConfig("SignatureKey", signatureDataObject,
                                             userPrivateInfoObject);
            setSignToUserPrivateConfig("SignatureCa", signatureDataObject,
                                             userPrivateInfoObject);

            _userPrivateInfo = JsonUtil::jsonToString(userPrivateInfoObject);
            LOG_INF("signatureconfig: Successfully updated user private info with signature data");

            ++offset;
        }
        else if (name == "serverprivateinfo")
        {
            _serverPrivateInfo = Uri::decode(value);
            ++offset;
        }
        else if (name == "readonly")
        {
            _isReadOnly = value != "0";
            ++offset;
        }
        else if (name == "password")
        {
            _docPassword = std::move(value);
            _haveDocPassword = true;
            ++offset;
        }
        else if (name == "lang")
        {
            if (value == "en")
                _lang = "en-US";
            else
                _lang = std::move(value);
            ++offset;
        }
        else if (name == "timezone")
        {
            _timeZone= std::move(value);
            ++offset;
        }
        else if (name == "watermarkText")
        {
            _watermarkText = Uri::decode(value);
            ++offset;
        }
        else if (name == "watermarkOpacity")
        {
            _watermarkOpacity = std::stod(value);
            ++offset;
        }
        else if (name == "timestamp")
        {
            timestamp = std::move(value);
            ++offset;
        }
        else if (name == "template")
        {
            _docTemplate = std::move(value);
            ++offset;
        }
        else if (name == "deviceFormFactor")
        {
            _deviceFormFactor = std::move(value);
            ++offset;
        }
        else if (name == "spellOnline")
        {
            _spellOnline = std::move(value);
            ++offset;
        }
        else if (name == "darkTheme")
        {
            _darkTheme = std::move(value);
            ++offset;
        }
        else if (name == "darkBackground")
        {
            _darkBackground = std::move(value);
            ++offset;
        }
        else if (name == "batch")
        {
            _batch = std::move(value);
            ++offset;
        }
        else if (name == "enableMacrosExecution")
        {
            _enableMacrosExecution = std::move(value);
            ++offset;
        }
        else if (name == "macroSecurityLevel")
        {
            _macroSecurityLevel = std::move(value);
            ++offset;
        }
        else if (name == "clientvisiblearea")
        {
            _initialClientVisibleArea = std::move(value);
            ++offset;
        }
        else if (name == "originaldocumenturl")
        {
            _originalDocUrl = std::move(value);
            ++offset;
        }
        else if (name == "accessibilityState")
        {
            _accessibilityState = value == "true";
            ++offset;
        }
        else if (name == "isAllowChangeComments")
        {
            _isAllowChangeComments = value == "true";
            ++offset;
        }
        else if (name == "isAllowManageRedlines")
        {
            _isAllowManageRedlines = value == "true";
            ++offset;
        }
        else if (name == "verifyHost")
        {
            _disableVerifyHost = value == "false";
            ++offset;
        }
        else if (name == "infilterOptions")
        {
            _inFilterOptions = std::move(value);
            ++offset;
        }
    }

    if (Anonymizer::enabled())
    {
        Anonymizer::mapAnonymized(_userId, _userIdAnonym);
        Anonymizer::mapAnonymized(_userName, _userNameAnonym);
        Anonymizer::mapAnonymized(_jailedFilePath, _jailedFilePathAnonym);
    }

    if (tokens.size() > offset)
    {
        if (getTokenString(tokens[offset], "options", _docOptions))
        {
            if (tokens.size() > offset + 1)
                _docOptions += tokens.cat(' ', offset + 1);
        }
    }

    // Disable spell check if the document is read-only
    disableSpellCheckIfReadOnly();
}

void Session::disableSpellCheckIfReadOnly()
{
    if (_isReadOnly)
    {
        _spellOnline = "false";
    }
}

void Session::disconnect()
{
    if (!_disconnected)
    {
        _disconnected = true;
        shutdown();
    }
}

void Session::shutdown(bool goingAway, const std::string_view statusMessage)
{
    LOG_TRC("Shutting down WS [" << getName() << "] " << (goingAway ? "going" : "normal")
                                 << " and reason [" << statusMessage << ']');

    // See protocol.txt for this application-level close frame.
    if (_protocol)
    {
        // skip the queue; FIXME: should we flush SessionClient's queue ?
        const std::string closeMsg = "close: " + std::string(statusMessage);
        _protocol->sendTextMessage(closeMsg);
        _protocol->shutdown(goingAway, statusMessage);
    }
}

void Session::handleMessage(const std::vector<char> &data)
{
    try
    {
        std::unique_ptr< std::vector<char> > replace;
        if (UnitBase::isUnitTesting() && !Util::isFuzzing() && UnitBase::get().filterSessionInput(this, data.data(), data.size(), replace))
        {
            if (replace && !replace->empty())
                _handleInput(replace->data(), replace->size());
            return;
        }

        if (!data.empty())
            _handleInput(data.data(), data.size());
    }
    catch (const Exception& exc)
    {
        LOG_ERR("Exception while handling ["
                << getAbbreviatedMessage(data) << "] in " << getName() << ": " << exc.displayText()
                << (exc.nested() ? " (" + exc.nested()->displayText() + ')' : ""));
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception while handling [" << getAbbreviatedMessage(data) << "]: " << exc.what());
    }
}

void Session::getIOStats(uint64_t &sent, uint64_t &recv)
{
    if (!_protocol)
    {
        LOG_TRC("ERR - missing protocol " << getName() << ": Get IO stats.");
        sent = 0; recv = 0;
        return;
    }

    _protocol->getIOStats(sent, recv);
}

void Session::dumpState(std::ostream& os)
{
    os << "\n\t\tid: " << _id
       << "\n\t\tname: " << _name
       << "\n\t\tdisconnected: " << _disconnected
       << "\n\t\tisActive: " << _isActive
       << "\n\t\tisCloseFrame: " << _isCloseFrame
       << "\n\t\twritePermission: " << _writePermission
       << "\n\t\tisWritable: " << _isWritable
       << "\n\t\tisReadOnly: " << _isReadOnly
       << "\n\t\tisAllowChangeComments: " << _isAllowChangeComments
       << "\n\t\tisAllowManageRedlines: " << _isAllowManageRedlines
       << "\n\t\tisEditable: " << isEditable()
       << "\n\t\tdocURL: " << _docURL
       << "\n\t\tjailedFilePath: " << Anonymizer::anonymizeUrl(_jailedFilePath)
       << "\n\t\tdocPwd: " << _docPassword
       << "\n\t\thaveDocPwd: " << _haveDocPassword
       << "\n\t\tisDocPwdProtected: " << _isDocPasswordProtected
       << "\n\t\tDocOptions: " << _docOptions
       << "\n\t\tuserId: " << Anonymizer::anonymize(_userId)
       << "\n\t\tuserName: " << Anonymizer::anonymize(_userName)
       << "\n\t\tlang: " << _lang
       << "\n\t\ttimezone: " << _timeZone
       << '\n';
}

void Session::setSignToUserPrivateConfig(const std::string& key,
                                         const Poco::JSON::Object::Ptr& signatureDataObject,
                                         Poco::JSON::Object::Ptr& userPrivateInfoObject)
{
    if (!signatureDataObject->has(key))
    {
        LOG_TRC("signatureconfig: Component " << key << " not found in signature data");
        return;
    }

    try
    {
        std::string componentValue = signatureDataObject->get(key).toString();
        LOG_TRC("signatureconfig: Processing component '" << key << "' with length "
                                                          << componentValue.length());
        componentValue = Util::replace(componentValue, "\\n", "\n");
        userPrivateInfoObject->set(key, componentValue);
        LOG_INF("signatureconfig: Successfully added " << key);
    }
    catch (const std::exception& e)
    {
        LOG_ERR("signatureconfig: Failed to process component '" << key << "': " << e.what());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
