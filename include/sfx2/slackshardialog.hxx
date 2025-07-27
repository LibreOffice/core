/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SLACKSHARDIALOG_HXX
#define INCLUDED_SFX2_SLACKSHARDIALOG_HXX

#include <sfx2/dllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <vcl/weld.hxx>
#include <memory>

namespace ucp { namespace slack { class SlackApiClient; } }

class SFX2_DLLPUBLIC SlackShareDialog final : public weld::GenericDialogController
{
private:
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::ucb::XCommandEnvironment> m_xCmdEnv;
    std::unique_ptr<ucp::slack::SlackApiClient> m_pApiClient;

    // Document information
    rtl::OUString m_sDocumentName;
    rtl::OUString m_sDocumentPath;
    sal_Int64 m_nDocumentSize;
    css::uno::Reference<css::io::XInputStream> m_xDocumentStream;

    // Selected sharing options
    rtl::OUString m_sSelectedWorkspaceId;
    rtl::OUString m_sSelectedChannelId;
    rtl::OUString m_sMessage;

    // Authentication state
    bool m_bAuthenticated;
    bool m_bChannelsLoaded;

    // UI Controls
    std::unique_ptr<weld::Label> m_xDocumentNameLabel;
    std::unique_ptr<weld::Label> m_xDocumentSizeLabel;
    std::unique_ptr<weld::ComboBox> m_xWorkspaceCombo;
    std::unique_ptr<weld::ComboBox> m_xChannelCombo;
    std::unique_ptr<weld::TextView> m_xMessageText;
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::Spinner> m_xStatusSpinner;
    std::unique_ptr<weld::Button> m_xBtnShare;
    std::unique_ptr<weld::Button> m_xBtnCancel;

public:
    SlackShareDialog(weld::Window* pParent,
                     const rtl::OUString& sDocumentName,
                     const rtl::OUString& sDocumentPath,
                     sal_Int64 nDocumentSize,
                     const css::uno::Reference<css::io::XInputStream>& xDocumentStream);

    virtual ~SlackShareDialog() override;

    // Main execution
    bool Execute();

    // Get sharing result
    rtl::OUString getSharedFileURL() const { return m_sSharedFileURL; }

private:
    // Initialization
    void InitializeUI();
    void SetupEventHandlers();

    // Authentication
    bool AuthenticateUser();
    void OnAuthenticationComplete();

    // Data loading
    void LoadWorkspaces();
    void LoadChannels();
    void OnWorkspaceChanged();
    void OnChannelChanged();

    // Sharing workflow
    void OnShare();
    bool ValidateInput();
    void PerformShare();
    void OnShareComplete(const rtl::OUString& sFileURL);
    void OnShareError(const rtl::OUString& sError);

    // Event handlers
    DECL_LINK(OnShareClicked, weld::Button&, void);
    DECL_LINK(OnCancelClicked, weld::Button&, void);
    DECL_LINK(OnWorkspaceSelected, weld::ComboBox&, void);
    DECL_LINK(OnChannelSelected, weld::ComboBox&, void);
    DECL_LINK(OnMessageChanged, weld::TextView&, void);

    // UI helpers
    void ShowStatus(const rtl::OUString& sMessage, bool bShowSpinner = false);
    void ShowError(const rtl::OUString& sError);
    void ShowSuccess(const rtl::OUString& sMessage);
    void UpdateShareButtonState();

    // Document info formatting
    rtl::OUString FormatFileSize(sal_Int64 nBytes);

    // Results
    rtl::OUString m_sSharedFileURL;
};

#endif // INCLUDED_SFX2_SLACKSHARDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
