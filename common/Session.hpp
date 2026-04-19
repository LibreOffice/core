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

#pragma once

#include <common/Log.hpp>
#include <net/Socket.hpp>
#include <wsd/TileDesc.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/Path.h>
#include <Poco/Types.h>

#include <atomic>
#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <type_traits>

class Session;

template<class T>
class SessionMap : public std::map<std::string, std::shared_ptr<T> >
{
    std::map<std::string, CanonicalViewId> _canonicalIds;
public:
    SessionMap() {
        static_assert(std::is_base_of_v<Session, T>, "sessions must have base of Session");
    }

    /// Generate a unique key for this set of view properties, only used by WSD
    CanonicalViewId createCanonicalId(const std::string &viewProps)
    {
        if (viewProps.empty())
            return CanonicalViewId::None;
        for (const auto& it : _canonicalIds)
        {
            if (it.first == viewProps)
                return it.second;
        }

        const CanonicalViewId id = static_cast<CanonicalViewId>(_canonicalIds.size() + 1000);
        _canonicalIds[viewProps] = id;
        return id;
    }

    /// Lookup one session in the map that matches this canonical view id, only used by Kit
    std::shared_ptr<T> findByCanonicalId(CanonicalViewId id) const
    {
        for (const auto &it : *this) {
            if (it.second->getCanonicalViewId() == id)
                return it.second;
        }
        return std::shared_ptr<T>();
    }
    void dumpState(std::ostream& oss)
    {
        for (const auto &it : *this) {
            oss << "\tsession '" << it.first << "'\n";
            it.second->dumpState(oss);
        }
    }
};

/// Base class of a WebSocket session.
class Session : public MessageHandlerInterface
{
public:
    const std::string& getId() const { return _id; }
    const std::string& getName() const { return _name; }
    bool isDisconnected() const { return _disconnected; }

    /// Sets the permission to write to storage (for a given document).
    /// If set to false, will setWritable(false).
    void setWritePermission(bool write)
    {
        _writePermission = write;
        if (!write)
        {
            // Disable writing.
            setWritable(false);
        }
    }

    /// Gets the permission to write to storage (for a given document).
    bool getWritePermission() const { return _writePermission; }

    /// Controls whether writing in the Storage is enabled in this session.
    /// If set to false, will setReadOnly(true) and setAllowChangeComments(false).
    void setWritable(bool writable)
    {
        _isWritable = writable;
        if (!writable)
        {
            setReadOnly(true);
            setAllowChangeComments(false);
            setAllowManageRedlines(false);
        }
    }

    /// True iff the session can write in the Storage.
    bool isWritable() const { return _isWritable; }

    /// Controls whether editing is enabled in this session.
    virtual void setReadOnly(bool readonly) { _isReadOnly = readonly; }
    bool isReadOnly() const { return _isReadOnly; }

    /// Controls whether commenting is enabled in this session.
    void setAllowChangeComments(bool allow) { _isAllowChangeComments = allow; }
    bool isAllowChangeComments() const { return _isAllowChangeComments; }

    /// Controls whether redline (change tracking) management is enabled in this session.
    void setAllowManageRedlines(bool allow) { _isAllowManageRedlines = allow; }
    bool isAllowManageRedlines() const { return _isAllowManageRedlines; }

    /// Returns true iff the view is either non-readonly or can change comments or manage redlines.
    bool isEditable() const { return !isReadOnly() || isAllowChangeComments() || isAllowManageRedlines(); }

    /// if certification verification was disabled for the wopi server
    bool isDisableVerifyHost() const { return _disableVerifyHost; }

    /// overridden to prepend client ids on messages by the Kit
    virtual bool sendBinaryFrame(const char* buffer, int length);
    virtual bool sendTextFrame(const char* buffer, int length);

    /// Get notified that the underlying transports disconnected
    void onDisconnect() override { /* ignore */ }

    bool hasQueuedMessages() const override
    {
        // queued in Socket output buffer
        return false;
    }

    // By default rely on the socket buffer.
    void writeQueuedMessages(std::size_t) override
    {
        assert(false);
    }

    /// Sends a WebSocket Text message.
    bool sendTextFrame(const std::string_view text)
    {
        return sendTextFrame(text.data(), text.size());
    }

    bool sendTextFrameAndLogError(const std::string_view text, LOG_CAPTURE_CALLER_DECLARATION)
    {
        LOG_ERR(text);
        return sendTextFrame(text.data(), text.size());
    }

    virtual void handleMessage(const std::vector<char> &data) override;

    /// Invoked when we want to disconnect a session.
    virtual void disconnect();

    /// clean & normal shutdown
    void shutdownNormal(const std::string_view statusMessage = std::string_view())
    {
        shutdown(false, statusMessage);
    }

    /// abnormal / hash shutdown end-point going away
    void shutdownGoingAway(const std::string_view statusMessage = std::string_view())
    {
        shutdown(true, statusMessage);
    }

    bool isActive() const { return _isActive; }
    void setIsActive(bool active) { _isActive = active; }

    /// Returns the inactivity time of the client in milliseconds.
    double getInactivityMS(const std::chrono::steady_clock::time_point now) const
    {
        const auto duration = now - _lastActivityTime;
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    /// Returns the inactivity time of the client in milliseconds.
    double getInactivityMS() const
    {
        return getInactivityMS(std::chrono::steady_clock::now());
    }

    void closeFrame() { _isCloseFrame = true; };
    bool isCloseFrame() const { return _isCloseFrame; }

    void getIOStats(uint64_t &sent, uint64_t &recv);

    void setUserId(const std::string& userId) { _userId = userId; }

    const std::string& getUserId() const { return _userId; }

    void setWatermarkText(const std::string& watermarkText) { _watermarkText = watermarkText; }

    void setIsAdminUser(const std::optional<bool> isAdminUser) { _isAdminUser = isAdminUser; }

    void setUserExtraInfo(const std::string& userExtraInfo) { _userExtraInfo = userExtraInfo; }

    void setUserPrivateInfo(const std::string& userPrivateInfo) { _userPrivateInfo = userPrivateInfo; }

    void setServerPrivateInfo(const std::string& serverPrivateInfo) { _serverPrivateInfo = serverPrivateInfo; }

    void setUserName(const std::string& userName) { _userName = userName; }

    const std::string& getUserName() const {return _userName; }

    const std::string& getUserNameAnonym() const { return _userNameAnonym; }

    bool isDocPasswordProtected() const { return _isDocPasswordProtected; }

    const std::string& getDocOptions() const { return _docOptions; }

    bool hasWatermark() const { return !_watermarkText.empty() && _watermarkOpacity > 0.0; }

    const std::string& getWatermarkText() const { return _watermarkText; }

    double getWatermarkOpacity() const { return _watermarkOpacity; }

    const std::string& getLang() const { return _lang; }

    const std::string& getTimezone() const { return _timeZone; }

    bool getHaveDocPassword() const { return _haveDocPassword; }

    void setHaveDocPassword(const bool val) { _haveDocPassword = val; }

    void setDocPassword(const std::string& password) { _docPassword = password; }

    const std::string& getDocPassword() const { return _docPassword; }

    std::optional<bool> getIsAdminUser() const { return _isAdminUser; }

    const std::string& getUserExtraInfo() const { return _userExtraInfo; }

    const std::string& getUserPrivateInfo() const { return _userPrivateInfo; }

    const std::string& getServerPrivateInfo() const { return _serverPrivateInfo; }

    const std::string& getDocURL() const { return  _docURL; }

    const std::string& getJailedFilePath() const { return _jailedFilePath; }

    const std::string& getJailedFilePathAnonym() const { return _jailedFilePathAnonym; }

    const std::string& getDeviceFormFactor() const { return _deviceFormFactor; }

    const std::string& getSpellOnline() const { return _spellOnline; }

    void setSpellOnline(const std::string& val) { _spellOnline = val; }

    const std::string& getDarkTheme() const { return _darkTheme; }

    void setDarkTheme(const std::string& val) { _darkTheme = val; }

    const std::string& getDarkBackground() const { return _darkBackground; }

    void setDarkBackground(const std::string& val) { _darkBackground = val; }

    const std::string& getBatchMode() const { return _batch; }

    const std::string& getEnableMacrosExecution() const { return _enableMacrosExecution; }

    const std::string& getMacroSecurityLevel() const { return _macroSecurityLevel; }

    const std::string& getInitialClientVisibleArea() const { return _initialClientVisibleArea; }

    bool getAccessibilityState() const { return _accessibilityState; }

    void setAccessibilityState(bool val) { _accessibilityState = val; }

    void disableSpellCheckIfReadOnly();

    const std::string& getDocTemplate() const { return _docTemplate; }

    const std::string& getInFilterOption() const { return _inFilterOptions; }

    const std::string& getZoteroAPIKey() const { return _zoteroAPIKey; }

    void setZoteroAPIKey(const std::string& val) { _zoteroAPIKey = val; }

    const std::string& getAIProviderAPIKey() const { return _aiProviderAPIKey; }
    void setAIProviderAPIKey(const std::string& val) { _aiProviderAPIKey = val; }

    const std::string& getAIProviderModel() const { return _aiProviderModel; }
    void setAIProviderModel(const std::string& val) { _aiProviderModel = val; }

    const std::string& getAIProviderURL() const { return _aiProviderURL; }
    void setAIProviderURL(const std::string& val) { _aiProviderURL = val; }

    const std::string& getAIImageModel() const { return _aiImageModel; }
    void setAIImageModel(const std::string& val) { _aiImageModel = val; }

    const std::string& getAIImageProviderAPIKey() const { return _aiImageProviderAPIKey; }
    void setAIImageProviderAPIKey(const std::string& val) { _aiImageProviderAPIKey = val; }

    const std::string& getAIImageProviderURL() const { return _aiImageProviderURL; }
    void setAIImageProviderURL(const std::string& val) { _aiImageProviderURL = val; }

    const std::string& getAIImageSize() const { return _aiImageSize; }
    void setAIImageSize(const std::string& val) { _aiImageSize = val; }

    const std::string& getAIRequestTimeout() const { return _aiRequestTimeout; }
    void setAIRequestTimeout(const std::string& val) { _aiRequestTimeout = val; }
    int getAIRequestTimeoutSeconds() const
    {
        if (_aiRequestTimeout.empty())
            return 120;
        try {
            int val = std::stoi(_aiRequestTimeout);
            if (val < 10) val = 10;
            return val;
        } catch (...) {
            return 120;
        }
    }

    const std::string& getSignatureCertificate() const { return _signatureCertificate; }
    void setSignatureCertificate(const std::string& cert) { _signatureCertificate = cert; }

    const std::string& getSignatureKey() const { return _signatureKey; }
    void setSignatureKey(const std::string& key) { _signatureKey = key; }

    const std::string& getSignatureCa() const { return _signatureCa; }
    void setSignatureCa(const std::string& ca) { _signatureCa = ca; }
protected:
    Session(const std::shared_ptr<ProtocolHandlerInterface> &handler,
            const std::string& name, const std::string& id, bool readonly);
    virtual ~Session();

    /// Parses the options of the "load" command,
    /// shared between MasterProcessSession::loadDocument() and ChildProcessSession::loadDocument().
    void parseDocOptions(const StringVector& tokens, int& part, std::string& timestamp);

    void updateLastActivityTime()
    {
        _lastActivityTime = std::chrono::steady_clock::now();
    }

    void dumpState(std::ostream& os) override;

    std::string getLogPrefix() const { return _name + ": "; }
    void logPrefix(std::ostream& os) const { os <<  _name + ": "; }

    void setSignToUserPrivateConfig(const std::string& key,
                                    const Poco::JSON::Object::Ptr& signatureDataObject,
                                    Poco::JSON::Object::Ptr& userPrivateInfoObject);

private:
    void shutdown(bool goingAway = false,
                  const std::string_view statusMessage = std::string_view());

    virtual bool _handleInput(const char* buffer, int length) = 0;

    /// A session ID specific to an end-to-end connection (from user to lokit).
    const std::string _id;

    /// A readable name that identifies our peer and ID.
    const std::string _name;

    /// The actual URL, also in the child, even if the child never accesses that.
    std::string _docURL;

    /// The Jailed document path.
    std::string _jailedFilePath;

    /// The Jailed document path, anonymized for logging.
    std::string _jailedFilePathAnonym;

    /// Password provided, if any, to open the document
    std::string _docPassword;

    /// Document options: a JSON string, containing options (rendering, also possibly load in the future).
    std::string _docOptions;

    /// Id of the user to whom the session belongs to.
    std::string _userId;

    /// Id of the user to whom the session belongs to, anonymized for logging.
    std::string _userIdAnonym;

    /// Name of the user to whom the session belongs to.
    std::string _userName;

    /// Name of the user to whom the session belongs to, anonymized for logging.
    std::string _userNameAnonym;

    /// Extra info per user, mostly mail, avatar, links, etc.
    std::string _userExtraInfo;

    /// Private info per user, not shared with others.
    std::string _userPrivateInfo;

    /// Private info per server, shared with others.
    std::string _serverPrivateInfo;

    /// In case a watermark has to be rendered on each tile.
    std::string _watermarkText;

    /// Language for the document based on what the user has in the UI.
    std::string _lang;

    /// Timezone of the user.
    std::string _timeZone;

    /// The form factor of the device where the client is running: desktop, tablet, mobile.
    std::string _deviceFormFactor;

    /// The start value of Auto Spell Checking whether it is enabled or disabled on start.
    std::string _spellOnline;

    /// The start value for Dark Theme whether it is active or not on start.
    std::string _darkTheme;
    ///
    /// The start value for Dark Background whether it is active or not on start.
    std::string _darkBackground;

    /// Disable dialogs interactivity.
    std::string _batch;

    /// Specifies whether the macro execution is enabled in general.
    std::string _enableMacrosExecution;

    /// Level of Macro security.
    std::string _macroSecurityLevel;

    std::string _initialClientVisibleArea;

    // The url of the template file used to create the document
    std::string _docTemplate;

    /// Opacity in case a watermark has to be rendered on each tile.
    double _watermarkOpacity;

    /// Time of the last interactive event being received
    std::chrono::steady_clock::time_point _lastActivityTime;

    /// If user is admin on the integrator side
    std::optional<bool> _isAdminUser;

    /// True if we have been disconnected.
    std::atomic<bool> _disconnected;
    /// True if the user is active, otherwise false (switched tabs).
    std::atomic<bool> _isActive;
    // Whether websocket received close frame.  Closing Handshake
    std::atomic<bool> _isCloseFrame;

    /// Whether the session has write permission in storage, as received from WOPI or URL parameters.
    /// This doesn't change once set.
    bool _writePermission;

    /// Whether the session can write in storage. May be disabled on error (e.g. low storage).
    /// Note: A read-only document may still be writable (if _isAllowChangeComments or
    /// _isAllowManageRedlines is true), f.e. PDF.
    bool _isWritable;

    /// Whether the session can edit the document. Disabled when we fail to lock, for example.
    bool _isReadOnly;

    /// Whether the session can add/change comments.
    /// Must have _isWritable=true, regardless of _isReadOnly.
    bool _isAllowChangeComments;

    /// Whether the session can add/change comments.
    /// Must have _isWritable=true, regardless of _isReadOnly.
    bool _isAllowManageRedlines = false;

    /// If password is provided or not
    bool _haveDocPassword;

    /// Whether document is password protected
    bool _isDocPasswordProtected;

    /// Specifies whether accessibility support is enabled for this session.
    bool _accessibilityState;

    /// Specifies whether certification verification for the wopi server
    /// should be disabled in core
    bool _disableVerifyHost;

    /// Used in convert-to apis to specify loading options
    std::string _inFilterOptions;

    /// Zotero API Key
    std::string _zoteroAPIKey;

    /// AI Provider API key
    std::string _aiProviderAPIKey;

    // AI Provider model, e.g. "gpt-4", "gpt-3.5-turbo", etc.
    std::string _aiProviderModel;

    // AI Provider custom URL, if not using the default one for the specified model.
    std::string _aiProviderURL;

    // AI image generation model
    std::string _aiImageModel;

    // AI image generation API key (optional, falls back to _aiProviderAPIKey)
    std::string _aiImageProviderAPIKey;

    // AI image generation base URL (optional, falls back to _aiProviderURL)
    std::string _aiImageProviderURL;

    // AI image generation size (e.g. "1024x1024")
    std::string _aiImageSize;

    // AI request timeout in seconds (default 120, range 10-300)
    std::string _aiRequestTimeout;

    /// Digital signature certificate, key, and CA
    std::string _signatureCertificate;
    std::string _signatureKey;
    std::string _signatureCa;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
