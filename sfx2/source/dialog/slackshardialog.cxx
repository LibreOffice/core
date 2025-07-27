/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/slackshardialog.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

// Include Slack API client
#include <ucb/source/ucp/slack/SlackApiClient.hxx>
#include <ucb/source/ucp/slack/slack_json.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

using namespace css;
using namespace css::uno;
using namespace css::ucb;

SlackShareDialog::SlackShareDialog(weld::Window* pParent,
                                   const rtl::OUString& sDocumentName,
                                   const rtl::OUString& sDocumentPath,
                                   sal_Int64 nDocumentSize,
                                   const Reference<io::XInputStream>& xDocumentStream)
    : GenericDialogController(pParent, u"sfx/ui/slackshardialog.ui"_ustr, u"SlackShareDialog"_ustr)
    , m_xContext(comphelper::getProcessComponentContext())
    , m_sDocumentName(sDocumentName)
    , m_sDocumentPath(sDocumentPath)
    , m_nDocumentSize(nDocumentSize)
    , m_xDocumentStream(xDocumentStream)
    , m_bAuthenticated(false)
    , m_bChannelsLoaded(false)
{
    try {
        SAL_WARN("sfx.slack", "SlackShareDialog constructor");

        if (!m_xBuilder) {
            SAL_WARN("sfx.slack", "Builder is null - UI file not loaded!");
            throw std::runtime_error("Failed to load UI file");
        }

        InitializeUI();
        SetupEventHandlers();

        // Create command environment for Slack API
        m_xCmdEnv = new ucbhelper::CommandEnvironment(
            Reference<task::XInteractionHandler>(),
            Reference<ucb::XProgressHandler>());

        // Initialize Slack API client
        try {
            m_pApiClient = std::make_unique<ucp::slack::SlackApiClient>(m_xCmdEnv);
            SAL_WARN("sfx.slack", "SlackApiClient initialized successfully");
        } catch (const std::exception& e) {
            SAL_WARN("sfx.slack", "Failed to initialize SlackApiClient: " << e.what());
            // Continue without API client - will show error during authentication
        }

        SAL_WARN("sfx.slack", "SlackShareDialog constructor completed successfully");
    } catch (const std::exception& e) {
        SAL_WARN("sfx.slack", "Exception in SlackShareDialog constructor: " << e.what());
        throw;
    }
}

SlackShareDialog::~SlackShareDialog() = default;

void SlackShareDialog::InitializeUI()
{
    SAL_WARN("sfx.slack", "InitializeUI called");

    // Get UI controls
    m_xDocumentNameLabel = m_xBuilder->weld_label(u"document_name_label"_ustr);
    m_xDocumentSizeLabel = m_xBuilder->weld_label(u"document_size_label"_ustr);
    m_xWorkspaceCombo = m_xBuilder->weld_combo_box_text(u"workspace_combo"_ustr);
    m_xChannelCombo = m_xBuilder->weld_combo_box(u"channel_combo"_ustr);
    m_xMessageText = m_xBuilder->weld_text_view(u"message_text"_ustr);
    m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);
    m_xStatusSpinner = m_xBuilder->weld_spinner(u"status_spinner"_ustr);
    m_xBtnShare = m_xBuilder->weld_button(u"btn_share"_ustr);
    m_xBtnCancel = m_xBuilder->weld_button(u"btn_cancel"_ustr);

    // Set document information
    if (m_xDocumentNameLabel) {
        rtl::OUString sLabel = u"Document: "_ustr + m_sDocumentName;
        m_xDocumentNameLabel->set_label(sLabel);
    }

    if (m_xDocumentSizeLabel) {
        rtl::OUString sLabel = u"Size: "_ustr + FormatFileSize(m_nDocumentSize);
        m_xDocumentSizeLabel->set_label(sLabel);
    }

    // Set initial state
    if (m_xBtnShare) {
        m_xBtnShare->set_sensitive(false);
    }

    if (m_xWorkspaceCombo) {
        m_xWorkspaceCombo->set_sensitive(false);
    }

    if (m_xChannelCombo) {
        m_xChannelCombo->set_sensitive(false);
    }

    ShowStatus(u"Connecting to Slack..."_ustr, true);
}

void SlackShareDialog::SetupEventHandlers()
{
    if (m_xBtnShare) {
        m_xBtnShare->connect_clicked(LINK(this, SlackShareDialog, OnShareClicked));
    }

    if (m_xBtnCancel) {
        m_xBtnCancel->connect_clicked(LINK(this, SlackShareDialog, OnCancelClicked));
    }

    if (m_xWorkspaceCombo) {
        m_xWorkspaceCombo->connect_changed(LINK(this, SlackShareDialog, OnWorkspaceSelected));
    }

    if (m_xChannelCombo) {
        m_xChannelCombo->connect_changed(LINK(this, SlackShareDialog, OnChannelSelected));
    }

    if (m_xMessageText) {
        m_xMessageText->connect_changed(LINK(this, SlackShareDialog, OnMessageChanged));
    }
}

bool SlackShareDialog::Execute()
{
    SAL_WARN("sfx.slack", "SlackShareDialog Execute() called");

    try {
        // Start authentication process
        if (AuthenticateUser()) {
            OnAuthenticationComplete();
        } else {
            ShowError(u"Failed to authenticate with Slack. Please check your credentials."_ustr);
        }

        // Run the dialog
        short nRet = run();
        SAL_WARN("sfx.slack", "Dialog returned: " << nRet);

        return (nRet == RET_OK && !m_sSharedFileURL.isEmpty());

    } catch (const std::exception& e) {
        SAL_WARN("sfx.slack", "Exception in SlackShareDialog Execute: " << e.what());
        ShowError(u"An error occurred while sharing to Slack."_ustr);
        return false;
    }
}

bool SlackShareDialog::AuthenticateUser()
{
    SAL_WARN("sfx.slack", "Starting Slack authentication");

    if (!m_pApiClient) {
        SAL_WARN("sfx.slack", "No API client available for authentication");
        return false;
    }

    try {
        // Check if already authenticated
        if (m_pApiClient->isAuthenticated()) {
            SAL_WARN("sfx.slack", "Already authenticated with Slack");
            m_bAuthenticated = true;
            return true;
        }

        // Perform authentication
        rtl::OUString accessToken = m_pApiClient->authenticate();
        if (!accessToken.isEmpty()) {
            SAL_WARN("sfx.slack", "Successfully authenticated with Slack");
            m_bAuthenticated = true;
            return true;
        } else {
            SAL_WARN("sfx.slack", "Authentication failed - no access token received");
            return false;
        }

    } catch (const std::exception& e) {
        SAL_WARN("sfx.slack", "Exception during authentication: " << e.what());
        return false;
    }
}

void SlackShareDialog::OnAuthenticationComplete()
{
    SAL_WARN("sfx.slack", "Authentication completed, loading workspaces and channels");

    ShowStatus(u"Loading workspaces..."_ustr, true);
    LoadWorkspaces();

    ShowStatus(u"Loading channels..."_ustr, true);
    LoadChannels();

    ShowStatus(u"Ready to share"_ustr, false);
    UpdateShareButtonState();
}

void SlackShareDialog::LoadWorkspaces()
{
    if (!m_pApiClient || !m_bAuthenticated) {
        return;
    }

    try {
        std::vector<ucp::slack::SlackWorkspace> workspaces = m_pApiClient->listWorkspaces();

        if (m_xWorkspaceCombo) {
            m_xWorkspaceCombo->clear();

            for (const auto& workspace : workspaces) {
                m_xWorkspaceCombo->append_text(workspace.name);

                // Store workspace ID (for now, assuming single workspace)
                if (m_sSelectedWorkspaceId.isEmpty()) {
                    m_sSelectedWorkspaceId = workspace.id;
                }
            }

            if (!workspaces.empty()) {
                m_xWorkspaceCombo->set_active(0);
                m_xWorkspaceCombo->set_sensitive(true);
            }
        }

        SAL_WARN("sfx.slack", "Loaded " << workspaces.size() << " workspaces");

    } catch (const std::exception& e) {
        SAL_WARN("sfx.slack", "Exception loading workspaces: " << e.what());
        ShowError(u"Failed to load Slack workspaces"_ustr);
    }
}

void SlackShareDialog::LoadChannels()
{
    if (!m_pApiClient || !m_bAuthenticated) {
        return;
    }

    try {
        std::vector<ucp::slack::SlackChannel> channels = m_pApiClient->listChannels(m_sSelectedWorkspaceId);

        if (m_xChannelCombo) {
            m_xChannelCombo->clear();

            for (const auto& channel : channels) {
                rtl::OUString displayName;
                if (channel.isPrivate) {
                    displayName = u"🔒 "_ustr + channel.name;
                } else {
                    displayName = u"# "_ustr + channel.name;
                }

                // Add to combo box (name, id, type)
                m_xChannelCombo->append(channel.id, displayName);
            }

            if (!channels.empty()) {
                m_xChannelCombo->set_active(0);
                m_xChannelCombo->set_sensitive(true);
                m_bChannelsLoaded = true;

                // Set selected channel ID
                m_sSelectedChannelId = channels[0].id;
            }
        }

        SAL_WARN("sfx.slack", "Loaded " << channels.size() << " channels");

    } catch (const std::exception& e) {
        SAL_WARN("sfx.slack", "Exception loading channels: " << e.what());
        ShowError(u"Failed to load Slack channels"_ustr);
    }
}

IMPL_LINK_NOARG(SlackShareDialog, OnShareClicked, weld::Button&, void)
{
    SAL_WARN("sfx.slack", "Share button clicked");

    if (ValidateInput()) {
        PerformShare();
    }
}

IMPL_LINK_NOARG(SlackShareDialog, OnCancelClicked, weld::Button&, void)
{
    SAL_WARN("sfx.slack", "Cancel button clicked");
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(SlackShareDialog, OnWorkspaceSelected, weld::ComboBoxText&, void)
{
    OnWorkspaceChanged();
}

IMPL_LINK_NOARG(SlackShareDialog, OnChannelSelected, weld::ComboBox&, void)
{
    OnChannelChanged();
}

IMPL_LINK_NOARG(SlackShareDialog, OnMessageChanged, weld::TextView&, void)
{
    UpdateShareButtonState();
}

void SlackShareDialog::OnWorkspaceChanged()
{
    // Reload channels for selected workspace
    LoadChannels();
    UpdateShareButtonState();
}

void SlackShareDialog::OnChannelChanged()
{
    if (m_xChannelCombo) {
        int nSelected = m_xChannelCombo->get_active();
        if (nSelected >= 0) {
            m_sSelectedChannelId = m_xChannelCombo->get_id(nSelected);
            SAL_WARN("sfx.slack", "Selected channel ID: " << m_sSelectedChannelId);
        }
    }
    UpdateShareButtonState();
}

bool SlackShareDialog::ValidateInput()
{
    if (m_sSelectedChannelId.isEmpty()) {
        ShowError(u"Please select a channel to share to"_ustr);
        return false;
    }

    if (!m_xDocumentStream.is()) {
        ShowError(u"Document stream not available"_ustr);
        return false;
    }

    return true;
}

void SlackShareDialog::PerformShare()
{
    SAL_WARN("sfx.slack", "Starting file share to channel: " << m_sSelectedChannelId);

    ShowStatus(u"Sharing file to Slack..."_ustr, true);

    // Disable UI during sharing
    if (m_xBtnShare) m_xBtnShare->set_sensitive(false);
    if (m_xBtnCancel) m_xBtnCancel->set_sensitive(false);

    try {
        // Get message text
        rtl::OUString sMessage;
        if (m_xMessageText) {
            sMessage = m_xMessageText->get_text();
        }

        // Perform the share
        rtl::OUString sSharedURL = m_pApiClient->shareFile(
            m_sDocumentName,
            m_xDocumentStream,
            m_nDocumentSize,
            m_sSelectedChannelId,
            sMessage
        );

        if (!sSharedURL.isEmpty()) {
            OnShareComplete(sSharedURL);
        } else {
            OnShareError(u"Failed to share file to Slack"_ustr);
        }

    } catch (const std::exception& e) {
        SAL_WARN("sfx.slack", "Exception during file share: " << e.what());
        OnShareError(u"An error occurred while sharing the file"_ustr);
    }
}

void SlackShareDialog::OnShareComplete(const rtl::OUString& sFileURL)
{
    SAL_WARN("sfx.slack", "File shared successfully: " << sFileURL);

    m_sSharedFileURL = sFileURL;
    ShowSuccess(u"File shared successfully to Slack!"_ustr);

    // Close dialog with success
    m_xDialog->response(RET_OK);
}

void SlackShareDialog::OnShareError(const rtl::OUString& sError)
{
    SAL_WARN("sfx.slack", "File share error: " << sError);

    ShowError(sError);

    // Re-enable UI
    if (m_xBtnShare) m_xBtnShare->set_sensitive(true);
    if (m_xBtnCancel) m_xBtnCancel->set_sensitive(true);
}

void SlackShareDialog::UpdateShareButtonState()
{
    bool bCanShare = m_bAuthenticated &&
                     m_bChannelsLoaded &&
                     !m_sSelectedChannelId.isEmpty() &&
                     m_xDocumentStream.is();

    if (m_xBtnShare) {
        m_xBtnShare->set_sensitive(bCanShare);
    }
}

void SlackShareDialog::ShowStatus(const rtl::OUString& sMessage, bool bShowSpinner)
{
    if (m_xStatusLabel) {
        m_xStatusLabel->set_label(sMessage);
    }

    if (m_xStatusSpinner) {
        if (bShowSpinner) {
            m_xStatusSpinner->start();
            m_xStatusSpinner->show();
        } else {
            m_xStatusSpinner->stop();
            m_xStatusSpinner->hide();
        }
    }
}

void SlackShareDialog::ShowError(const rtl::OUString& sError)
{
    ShowStatus(sError, false);
    // TODO: Could show error icon or change label color
}

void SlackShareDialog::ShowSuccess(const rtl::OUString& sMessage)
{
    ShowStatus(sMessage, false);
    // TODO: Could show success icon or change label color
}

rtl::OUString SlackShareDialog::FormatFileSize(sal_Int64 nBytes)
{
    rtl::OUStringBuffer sSize;

    if (nBytes >= 1024 * 1024 * 1024) {
        // GB
        sSize.append(OUString::number(nBytes / (1024 * 1024 * 1024)));
        sSize.append(u" GB"_ustr);
    } else if (nBytes >= 1024 * 1024) {
        // MB
        sSize.append(OUString::number(nBytes / (1024 * 1024)));
        sSize.append(u" MB"_ustr);
    } else if (nBytes >= 1024) {
        // KB
        sSize.append(OUString::number(nBytes / 1024));
        sSize.append(u" KB"_ustr);
    } else {
        // Bytes
        sSize.append(OUString::number(nBytes));
        sSize.append(u" bytes"_ustr);
    }

    return sSize.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
