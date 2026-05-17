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

#include <common/Authorization.hpp>
#include <common/CharacterConverter.hpp>
#include <common/Log.hpp>
#include <net/HttpRequest.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/Storage.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>

/// WOPI protocol backed storage.
class WopiStorage : public StorageBase
{
public:
    class WOPIFileInfo final : public FileInfo
    {
        void init();

    public:
        enum class TriState
        {
            False,
            True,
            Unset
        };

        /// warning - removes items from object.
        WOPIFileInfo(const FileInfo& fileInfo, Poco::JSON::Object::Ptr& object,
                     const Poco::URI& uriObject);

        const std::string& getUserId() const { return _userId; }
        const std::string& getUsername() const { return _username; }
        const std::string& getUserExtraInfo() const { return _userExtraInfo; }
        const std::string& getUserPrivateInfo() const { return _userPrivateInfo; }
        const std::string& getServerPrivateInfo() const { return _serverPrivateInfo; }
        const std::string& getUserSettingsUri() const { return _userSettingsUri; }
        const std::string& getWatermarkText() const { return _watermarkText; }
        const std::string& getTemplateSaveAs() const { return _templateSaveAs; }
        const std::string& getTemplateSource() const { return _templateSource; }
        const std::string& getBreadcrumbDocName() const { return _breadcrumbDocName; }
        const std::string& getFileUrl() const { return _fileUrl; }
        const std::string& getPostMessageOrigin() const { return _postMessageOrigin; }
        const std::string& getHideUserList() const { return _hideUserList; }
        const std::string& getPresentationLeader() const { return _presentationLeader; }

        bool getUserCanWrite() const { return _userCanWrite; }
        void setHidePrintOption(bool hidePrintOption) { _hidePrintOption = hidePrintOption; }
        bool getHidePrintOption() const { return _hidePrintOption; }
        bool getHideSaveOption() const { return _hideSaveOption; }
        void setHideExportOption(bool hideExportOption) { _hideExportOption = hideExportOption; }
        bool getHideExportOption() const { return _hideExportOption; }
        void setHideRepairOption(bool hideRepairOption) { _hideRepairOption = hideRepairOption; }
        bool getHideRepairOption() const { return _hideRepairOption; }
        bool getEnableOwnerTermination() const { return _enableOwnerTermination; }
        bool getDisablePrint() const { return _disablePrint; }
        bool getDisableExport() const { return _disableExport; }
        bool getDisableCopy() const { return _disableCopy; }
        bool getDisableInactiveMessages() const { return _disableInactiveMessages; }
        bool getDownloadAsPostMessage() const { return _downloadAsPostMessage; }
        bool getUserCanNotWriteRelative() const { return _userCanNotWriteRelative; }
        bool getEnableInsertRemoteImage() const { return _enableInsertRemoteImage; }
        bool getEnableInsertRemoteFile() const { return _enableInsertRemoteFile; }
        bool getDisableInsertLocalImage() const { return _disableInsertLocalImage; }
        bool getEnableRemoteLinkPicker() const { return _enableRemoteLinkPicker; }
        bool getEnableRemoteAIContent() const { return _enableRemoteAIContent; }
        bool getDisableAISettings() const { return _disableAISettings; }
        bool getEnableShare() const { return _enableShare; }
        bool getSupportsRename() const { return _supportsRename; }
        bool getSupportsLocks() const { return _supportsLocks; }
        bool getUserCanRename() const { return _userCanRename; }
        bool getUserCanOnlyComment() const { return _userCanOnlyComment; }
        bool getUserCanOnlyManageRedlines() const { return _userCanOnlyManageRedlines; }

        const std::optional<bool> getIsAdminUser() const { return _isAdminUser; }
        const std::string& getIsAdminUserError() const { return _isAdminUserError; }

        TriState getDisableChangeTrackingShow() const { return _disableChangeTrackingShow; }
        TriState getDisableChangeTrackingRecord() const { return _disableChangeTrackingRecord; }
        TriState getHideChangeTrackingControls() const { return _hideChangeTrackingControls; }

    private:
        /// User id of the user accessing the file
        std::string _userId;
        /// Obfuscated User id used for logging the UserId.
        std::string _obfuscatedUserId;
        /// Display Name of user accessing the file
        std::string _username;
        /// Extra public info per user, typically mail and other links, as json, shared with everyone.
        std::string _userExtraInfo;
        /// Private info per user, for API keys and other non-public information.
        std::string _userPrivateInfo;
        /// Private info per server, for API keys and other non-public information.
        std::string _serverPrivateInfo;
        /// Uri to get settings json for this user, for autotext location, etc.
        std::string _userSettingsUri;
        /// In case a watermark has to be rendered on each tile.
        std::string _watermarkText;
        /// In case we want to use this file as a template, it should be first re-saved under this name (using PutRelativeFile).
        std::string _templateSaveAs;
        /// In case we want to use this file as a template.
        std::string _templateSource;
        /// User readable string of document name to show in UI, if present.
        std::string _breadcrumbDocName;
        /// The optional FileUrl, used to download the document if provided.
        std::string _fileUrl;
        /// WOPI Post message property
        std::string _postMessageOrigin;
        /// If set to "true", user list on the status bar will be hidden
        /// If set to "mobile" | "tablet" | "desktop", will be hidden on a specified device
        /// (may be joint, delimited by commas eg. "mobile,tablet")
        std::string _hideUserList;
        /// error code if integration does not use isAdminUser field properly
        std::string _isAdminUserError;
        /// If we should disable change-tracking visibility by default (meaningful at loading).
        TriState _disableChangeTrackingShow = WOPIFileInfo::TriState::Unset;
        /// If we should disable change-tracking ability by default (meaningful at loading).
        TriState _disableChangeTrackingRecord = WOPIFileInfo::TriState::Unset;
        /// If we should hide change-tracking commands for this user.
        TriState _hideChangeTrackingControls = WOPIFileInfo::TriState::Unset;
        /// If user is considered as admin on the integrator side
        std::optional<bool> _isAdminUser = std::nullopt;
        /// If user accessing the file has write permission
        bool _userCanWrite = false;
        /// Hide print button from UI
        bool _hidePrintOption = false;
        /// Hide save button from UI
        bool _hideSaveOption = false;
        /// Hide 'Download as' button/menubar item from UI
        bool _hideExportOption = false;
        /// Hide the 'Repair' button/item from the UI
        bool _hideRepairOption = false;
        /// If WOPI host has enabled owner termination feature on
        bool _enableOwnerTermination = false;
        /// If WOPI host has allowed the user to print the document
        bool _disablePrint = false;
        /// If WOPI host has allowed the user to export the document
        bool _disableExport = false;
        /// If WOPI host has allowed the user to copy to/from the document
        bool _disableCopy = false;
        /// If WOPI host has allowed the cool to show texts on the overlay informing about
        /// inactivity, or if the integration is handling that.
        bool _disableInactiveMessages = false;
        /// For the (mobile) integrations, to indicate that the downloading for printing, exporting,
        /// or slideshows should be intercepted and sent as a postMessage instead of handling directly.
        bool _downloadAsPostMessage = false;
        /// If set to false, users can access the save-as functionality
        bool _userCanNotWriteRelative = true;
        /// If set to true, users can access the insert remote image functionality
        bool _enableInsertRemoteImage = false;
        /// If set to true, users can access the insert remote multimedia functionality
        bool _enableInsertRemoteFile = false;
        /// If set to true, users can't insert an image from the local machine
        bool _disableInsertLocalImage = false;
        /// If set to true, users can access the remote link picker functionality
        bool _enableRemoteLinkPicker = false;
        /// If set to true, users can insert remote AI-generated content
        bool _enableRemoteAIContent = false;
        /// If set to true, AI settings UI and AI features are disabled for the user
        bool _disableAISettings = false;
        /// If set to true, users can access the file share functionality
        bool _enableShare = false;
        /// If WOPI host supports locking
        bool _supportsLocks = false;
        /// If WOPI host supports rename
        bool _supportsRename = false;
        /// If user is allowed to rename the document
        bool _userCanRename = false;
        /// If user is limited to only writing/modifying comments
        bool _userCanOnlyComment = false;
        /// If user is limited to only managing redlines (accept/reject)
        bool _userCanOnlyManageRedlines = false;
        /// Used for directly starting follow me presentation
        std::string _presentationLeader;
    };

    WopiStorage(const Poco::URI& uri, const std::string& localStorePath,
                const std::string& jailPath)
        : StorageBase(uri, localStorePath, jailPath)
        , _wopiSaveDuration(std::chrono::milliseconds::zero())
        , _utf7Converter("UTF-8", "UTF-7")
        , _legacyServer(ConfigUtil::getConfigValue<bool>("storage.wopi.is_legacy_server", false))
    {
        LOG_INF("WopiStorage ctor with localStorePath: ["
                << localStorePath << "], jailPath: [" << jailPath << "], uri: ["
                << Anonymizer::anonymizeUrl(uri.toString())
                << "], legacy server: " << _legacyServer);
    }

    /// Signifies if the server is legacy or not, based on the headers
    /// it sent us on first contact.
    bool isLegacyServer() const { return _legacyServer; }

    /// Handles the response from CheckFileInfo, as converted into WOPIFileInfo.
    /// Also extracts the basic file information from the response
    /// which can then be obtained using getFileInfo()
    /// Also sets up the locking context for future operations.
    void handleWOPIFileInfo(const WOPIFileInfo& wopiFileInfo, LockContext& lockCtx);

    /// Update the locking state (check-in/out) of the associated file
    LockUpdateResult updateLockState(const Authorization& auth, LockContext& lockCtx,
                                     StorageBase::LockState lock,
                                     const Attributes& attribs) override;

    void updateLockStateAsync(const Authorization& auth, LockContext& lockCtx, LockState lock,
                              const Attributes& attribs, const std::shared_ptr<SocketPoll>& socketPoll,
                              const AsyncLockStateCallback& asyncLockStateCallback) override;

    /// uri format: http://server/<...>/wopi*/files/<id>/content
    std::string downloadStorageFileToLocal(const Authorization& auth, LockContext& lockCtx,
                                           const std::string& templateUri,
                                           AdditionalFilePaths& additionalFileLocalPaths) override;

    std::size_t
    uploadLocalFileToStorageAsync(const Authorization& auth, LockContext& lockCtx,
                                  const std::string& saveAsPath, const std::string& saveAsFilename,
                                  bool isRename, const Attributes&,
                                  const std::shared_ptr<SocketPoll>& socketPoll,
                                  const AsyncUploadCallback& asyncUploadCallback) override;

    /// Total time taken for making WOPI calls during uploading.
    std::chrono::milliseconds getWopiSaveDuration() const { return _wopiSaveDuration; }

    virtual AsyncUpload queryLocalFileToStorageAsyncUploadState() override
    {
        const AsyncUpload::State state =
            _uploadHttpSession ? AsyncUpload::State::Running : AsyncUpload::State::None;
        return AsyncUpload(state, UploadResult(UploadResult::Result::OK));
    }

protected:
    struct WopiUploadDetails
    {
        const std::string filePathAnonym;
        const std::string uriAnonym;
        const std::string httpResponseReason;
        const std::size_t size;
        const http::StatusCode httpResponseCode;
        const bool isSaveAs;
        const bool isRename;
    };

    /// Handles the response from the server when uploading the document.
    UploadResult handleUploadToStorageResponse(const WopiUploadDetails& details,
                                               std::string responseString);

private:
    /// Download the document from the given URI.
    /// Does not add authorization tokens or any other logic.
    std::string downloadDocument(const Poco::URI& uriObject, const std::string& uriAnonym,
                                 const Authorization& auth, unsigned redirectLimit);

    /// Create the HTTP request for a WOPI Lock/Unlock operation.
    http::Request createLockRequest(const Poco::URI& uriObject, const Authorization& auth,
                                    LockContext& lockCtx, LockState lock,
                                    const Attributes& attribs);

    /// Set a WOPI header with both COOL and legacy LOOL prefixes.
    void setWopiHeader(http::Request& httpRequest, const std::string& suffix,
                       const std::string& value);

    /// Returns the URI with the path anonymized, optionally appending a suffix.
    std::string getAnonymizedUri(const std::string& pathSuffix = std::string()) const
    {
        Poco::URI uriAnonym(getUri());
        uriAnonym.setPath(Anonymizer::anonymizeUrl(uriAnonym.getPath()) + pathSuffix);
        return uriAnonym.toString();
    }

private:
    /// A URL provided by the WOPI host to use for GetFile.
    std::string _fileUrl;

    // Time spend in saving the file from storage
    std::chrono::milliseconds _wopiSaveDuration;

    /// The http::Session used for uploading asynchronously.
    std::shared_ptr<http::Session> _uploadHttpSession;

    /// The http::Session used for locking asynchronously.
    std::shared_ptr<http::Session> _lockHttpSession;

    /// Filename converter to UTF-7.
    Util::CharacterConverter _utf7Converter;

    /// Whether or not this is a legacy server.
    const bool _legacyServer;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
