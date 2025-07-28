/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_DROPBOXDIALOG_HXX
#define INCLUDED_SFX2_DROPBOXDIALOG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::ucb { class XCommandEnvironment; }
namespace com::sun::star::io { class XInputStream; }
namespace ucp { namespace dropbox {
    class DropboxApiClient;
    struct DropboxFileInfo;
}}

class SFX2_DLLPUBLIC DropboxDialog : public weld::GenericDialogController
{
private:
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::ucb::XCommandEnvironment> m_xCmdEnv;

    // Dropbox API client
    std::unique_ptr<ucp::dropbox::DropboxApiClient> m_pApiClient;

    // UI controls
    std::unique_ptr<weld::TreeView> m_xFileList;
    std::unique_ptr<weld::Button> m_xBtnOpen;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::ProgressBar> m_xProgressBar;
    std::unique_ptr<weld::Button> m_xBtnBack;
    std::unique_ptr<weld::Button> m_xBtnRefresh;
    std::unique_ptr<weld::Label> m_xCurrentFolderLabel;

    // State
    std::vector<ucp::dropbox::DropboxFileInfo> m_aCurrentFiles;
    std::vector<OUString> m_aFolderPath; // Stack of folder IDs for navigation
    OUString m_sCurrentFolderId;
    OUString m_sSelectedFileUrl;
    bool m_bAuthenticated;

    // Upload mode state
    bool m_bUploadMode;
    OUString m_sUploadFilePath;
    OUString m_sUploadFileName;

    // Timer for async operations
    Timer m_aLoadTimer;

public:
    explicit DropboxDialog(weld::Window* pParent);
    explicit DropboxDialog(weld::Window* pParent, const OUString& uploadFilePath, const OUString& uploadFileName);
    virtual ~DropboxDialog() override;

    bool Execute();
    OUString GetSelectedFileUrl() const { return m_sSelectedFileUrl; }

    // Upload functionality
    bool UploadFile(const OUString& filePath, const OUString& fileName);

private:
    void InitializeUI();
    void SetupEventHandlers();

    // Authentication
    bool AuthenticateUser();

    // File listing
    void LoadFolder(const OUString& folderId = OUString());
    void RefreshFileList();
    void PopulateFileList(const std::vector<ucp::dropbox::DropboxFileInfo>& files);

    // Event handlers
    DECL_LINK(OnFileDoubleClick, weld::TreeView&, bool);
    DECL_LINK(OnFileSelect, weld::TreeView&, void);
    DECL_LINK(OnOpenClicked, weld::Button&, void);
    DECL_LINK(OnCancelClicked, weld::Button&, void);
    DECL_LINK(OnBackClicked, weld::Button&, void);
    DECL_LINK(OnRefreshClicked, weld::Button&, void);
    DECL_LINK(OnLoadTimer, Timer*, void);

    // Utility methods
    void SetStatus(const OUString& text);
    void SetProgress(int percent = -1);
    void EnableControls(bool bEnable);
    bool IsFileSelected() const;
    OUString GetSelectedFileName() const;
    OUString GetSelectedFileId() const;
    bool IsSelectedItemFolder() const;

    // File operations
    bool DownloadAndOpenSelectedFile();
    OUString CreateTempFileFromStream(const css::uno::Reference<css::io::XInputStream>& xInputStream, const OUString& fileName);

    // Demo/development methods
    void LoadDemoContent();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
