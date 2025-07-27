/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_GOOGLEDRIVEDIALOG_HXX
#define INCLUDED_SFX2_GOOGLEDRIVEDIALOG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>

namespace com::sun::star::uno { class XComponentContext; }
namespace ucp { namespace gdrive {
    class GoogleDriveApiClient;
    struct GDriveFileInfo;
}}

class SFX2_DLLPUBLIC GoogleDriveDialog : public weld::GenericDialogController
{
private:
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::ucb::XCommandEnvironment> m_xCmdEnv;

    // Google Drive API client
    std::unique_ptr<ucp::gdrive::GoogleDriveApiClient> m_pApiClient;

    // UI controls
    std::unique_ptr<weld::TreeView> m_xFileList;
    std::unique_ptr<weld::Button> m_xBtnOpen;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::ProgressBar> m_xProgressBar;
    std::unique_ptr<weld::Button> m_xBtnBack;
    std::unique_ptr<weld::Label> m_xCurrentFolderLabel;

    // State
    std::vector<ucp::gdrive::GDriveFileInfo> m_aCurrentFiles;
    std::vector<OUString> m_aFolderPath; // Stack of folder IDs for navigation
    OUString m_sCurrentFolderId;
    OUString m_sSelectedFileId;
    bool m_bAuthenticated;

    // Timer for async operations
    Timer m_aAuthTimer;
    Timer m_aLoadTimer;

public:
    GoogleDriveDialog(weld::Window* pParent);
    virtual ~GoogleDriveDialog() override;

    // Main interface
    OUString GetSelectedFileURL() const;
    bool Execute();

private:
    // Authentication
    void StartAuthentication();
    void CheckAuthenticationStatus();
    DECL_LINK(AuthTimerHandler, Timer*, void);

    // File operations
    void LoadFolder(const OUString& folderId = u"root"_ustr);
    void RefreshCurrentFolder();
    DECL_LINK(LoadTimerHandler, Timer*, void);

    // UI event handlers
    DECL_LINK(FileListDoubleClick, weld::TreeView&, bool);
    DECL_LINK(FileListSelectionChanged, weld::TreeView&, void);
    DECL_LINK(OpenButtonClicked, weld::Button&, void);
    DECL_LINK(CancelButtonClicked, weld::Button&, void);
    DECL_LINK(BackButtonClicked, weld::Button&, void);

    // Helper methods
    void PopulateFileList();
    void UpdateUI();
    void SetStatus(const OUString& sMessage, bool bShowProgress = false);
    void NavigateToFolder(const OUString& folderId, const OUString& folderName);
    void NavigateBack();
    OUString FormatFileSize(sal_Int64 nBytes) const;
    OUString GetFileTypeDescription(const OUString& sMimeType) const;

    // Error handling
    void ShowErrorMessage(const OUString& sError);

    // Test/fallback data
    void AddTestData();
};

#endif // INCLUDED_SFX2_GOOGLEDRIVEDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */