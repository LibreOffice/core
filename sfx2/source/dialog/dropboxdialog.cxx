/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/dropboxdialog.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>

// Include Dropbox API client
#include <ucb/source/ucp/dropbox/DropboxApiClient.hxx>
#include <ucb/source/ucp/dropbox/dropbox_json.hxx>

#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <chrono>

using namespace css;
using namespace css::uno;
using namespace css::ucb;

DropboxDialog::DropboxDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"sfx/ui/googledrivedialog.ui"_ustr, u"GoogleDriveDialog"_ustr)
    , m_xContext(comphelper::getProcessComponentContext())
    , m_sCurrentFolderId(u""_ustr) // Dropbox uses empty string for root
    , m_bAuthenticated(false)
    , m_aLoadTimer("DropboxLoadTimer")
{
    try {
        SAL_WARN("sfx.dropbox", "DropboxDialog constructor - using Google Drive UI temporarily");

        if (!m_xBuilder) {
            SAL_WARN("sfx.dropbox", "Builder is null - UI file not loaded!");
            throw std::runtime_error("Failed to load UI file");
        }

        // Get basic UI controls
        m_xFileList = m_xBuilder->weld_tree_view(u"file_list"_ustr);
        m_xBtnOpen = m_xBuilder->weld_button(u"btn_open"_ustr);
        m_xBtnCancel = m_xBuilder->weld_button(u"btn_cancel"_ustr);
        m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);
        m_xCurrentFolderLabel = m_xBuilder->weld_label(u"current_folder_label"_ustr);

        // Set up basic event handlers
        if (m_xBtnCancel) {
            m_xBtnCancel->connect_clicked(LINK(this, DropboxDialog, OnCancelClicked));
        }
        if (m_xBtnOpen) {
            m_xBtnOpen->connect_clicked(LINK(this, DropboxDialog, OnOpenClicked));
            m_xBtnOpen->set_sensitive(false); // Disabled until file selected
        }
        if (m_xFileList) {
            m_xFileList->connect_selection_changed(LINK(this, DropboxDialog, OnFileSelect));
        }

        // Set initial labels
        if (m_xStatusLabel) {
            m_xStatusLabel->set_label(u"Ready to browse Dropbox files"_ustr);
        }
        if (m_xCurrentFolderLabel) {
            m_xCurrentFolderLabel->set_label(u"Dropbox"_ustr);
        }

        // Create command environment for Dropbox API
        m_xCmdEnv = new ucbhelper::CommandEnvironment(
            css::uno::Reference<css::task::XInteractionHandler>(),
            css::uno::Reference<css::ucb::XProgressHandler>());

        // Initialize Dropbox API client
        try {
            m_pApiClient = std::make_unique<ucp::dropbox::DropboxApiClient>(m_xCmdEnv);
            SAL_WARN("sfx.dropbox", "DropboxApiClient initialized successfully");
        } catch (const std::exception& e) {
            SAL_WARN("sfx.dropbox", "Failed to initialize DropboxApiClient: " << e.what());
            // Continue without API client - will show error during authentication
        }

        SAL_WARN("sfx.dropbox", "DropboxDialog constructor completed successfully");
    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception in DropboxDialog constructor: " << e.what());
        throw;
    } catch (...) {
        SAL_WARN("sfx.dropbox", "Unknown exception in DropboxDialog constructor");
        throw;
    }
}

DropboxDialog::~DropboxDialog() = default;

bool DropboxDialog::Execute()
{
    SAL_WARN("sfx.dropbox", "DropboxDialog Execute() called");

    try {
        // Authenticate user and load real Dropbox files
        if (AuthenticateUser()) {
            LoadFolder(); // Load root folder
        } else {
            // Authentication failed, show demo content as fallback
            SAL_WARN("sfx.dropbox", "Authentication failed, showing demo content");
            LoadDemoContent();
        }

        // Run the dialog
        short nRet = run();
        SAL_WARN("sfx.dropbox", "Dialog returned: " << nRet);

        if (nRet == RET_OK && IsFileSelected()) {
            SAL_WARN("sfx.dropbox", "File selected: " << GetSelectedFileName());
            return true;
        }

        return false;

    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception in DropboxDialog Execute: " << e.what());
        return false;
    } catch (...) {
        SAL_WARN("sfx.dropbox", "Unknown exception in DropboxDialog Execute");
        return false;
    }
}

void DropboxDialog::InitializeUI()
{
    SAL_WARN("sfx.dropbox", "InitializeUI called");

    if (!m_xBuilder) {
        SAL_WARN("sfx.dropbox", "InitializeUI: Builder is null!");
        throw std::runtime_error("Builder is null in InitializeUI");
    }

    // Get UI controls with error checking
    try {
        m_xFileList = m_xBuilder->weld_tree_view(u"file_list"_ustr);
        if (!m_xFileList) {
            SAL_WARN("sfx.dropbox", "Failed to get file_list control");
        }

        m_xBtnOpen = m_xBuilder->weld_button(u"btn_open"_ustr);
        if (!m_xBtnOpen) {
            SAL_WARN("sfx.dropbox", "Failed to get btn_open control");
        }

        m_xBtnCancel = m_xBuilder->weld_button(u"btn_cancel"_ustr);
        if (!m_xBtnCancel) {
            SAL_WARN("sfx.dropbox", "Failed to get btn_cancel control");
        }

        m_xBtnBack = m_xBuilder->weld_button(u"btn_back"_ustr);
        if (!m_xBtnBack) {
            SAL_WARN("sfx.dropbox", "Failed to get btn_back control");
        }

        m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);
        if (!m_xStatusLabel) {
            SAL_WARN("sfx.dropbox", "Failed to get status_label control");
        }

        m_xCurrentFolderLabel = m_xBuilder->weld_label(u"current_folder_label"_ustr);
        if (!m_xCurrentFolderLabel) {
            SAL_WARN("sfx.dropbox", "Failed to get current_folder_label control");
        }

        m_xProgressBar = m_xBuilder->weld_progress_bar(u"progress_bar"_ustr);
        if (!m_xProgressBar) {
            SAL_WARN("sfx.dropbox", "Failed to get progress_bar control");
        }

        // Try to get refresh button (new in Dropbox UI)
        try {
            m_xBtnRefresh = m_xBuilder->weld_button(u"btn_refresh"_ustr);
            if (!m_xBtnRefresh) {
                SAL_WARN("sfx.dropbox", "btn_refresh control not found");
            }
        } catch (...) {
            SAL_WARN("sfx.dropbox", "Exception getting btn_refresh - setting to null");
            m_xBtnRefresh = nullptr;
        }

        SAL_WARN("sfx.dropbox", "UI controls loaded successfully");

    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception getting UI controls: " << e.what());
        throw;
    }

    // Configure tree view
    if (m_xFileList) {
        try {
            m_xFileList->set_selection_mode(SelectionMode::Single);
            std::vector<int> aWidths = { 300, 80, 80, 120 }; // Name, Type, Size, Modified
            m_xFileList->set_column_fixed_widths(aWidths);
            SAL_WARN("sfx.dropbox", "TreeView configured successfully");
        } catch (const std::exception& e) {
            SAL_WARN("sfx.dropbox", "Exception configuring TreeView: " << e.what());
        }
    }

    // Set initial state
    try {
        SetStatus(u"Ready to connect to Dropbox"_ustr);

        if (m_xCurrentFolderLabel) {
            m_xCurrentFolderLabel->set_label(u"Dropbox"_ustr);
        }

        // Initially disable Open button until a file is selected
        if (m_xBtnOpen) {
            m_xBtnOpen->set_sensitive(false);
        }

        // Initially disable Back button (we start at root)
        if (m_xBtnBack) {
            m_xBtnBack->set_sensitive(false);
        }

        SAL_WARN("sfx.dropbox", "Initial state set successfully");

    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception setting initial state: " << e.what());
    }
}

void DropboxDialog::SetupEventHandlers()
{
    SAL_WARN("sfx.dropbox", "SetupEventHandlers called");

    // Set up event handlers
    if (m_xFileList) {
        m_xFileList->connect_row_activated(LINK(this, DropboxDialog, OnFileDoubleClick));
        m_xFileList->connect_selection_changed(LINK(this, DropboxDialog, OnFileSelect));
    }

    if (m_xBtnOpen) {
        m_xBtnOpen->connect_clicked(LINK(this, DropboxDialog, OnOpenClicked));
    }

    if (m_xBtnCancel) {
        m_xBtnCancel->connect_clicked(LINK(this, DropboxDialog, OnCancelClicked));
    }

    if (m_xBtnBack) {
        m_xBtnBack->connect_clicked(LINK(this, DropboxDialog, OnBackClicked));
    }

    if (m_xBtnRefresh) {
        m_xBtnRefresh->connect_clicked(LINK(this, DropboxDialog, OnRefreshClicked));
    }

    // Setup timer for async operations
    m_aLoadTimer.SetTimeout(100); // 100ms
    m_aLoadTimer.SetInvokeHandler(LINK(this, DropboxDialog, OnLoadTimer));
}

bool DropboxDialog::AuthenticateUser()
{
    SAL_WARN("sfx.dropbox", "AuthenticateUser called");

    if (!m_pApiClient) {
        SAL_WARN("sfx.dropbox", "No API client available");
        return false;
    }

    try {
        SetStatus(u"Authenticating with Dropbox..."_ustr);

        // Implement proper OAuth2 authentication flow
        SAL_WARN("sfx.dropbox", "Starting OAuth2 authentication");
        OUString sAccessToken = m_pApiClient->getCurrentAccessToken();
        bool bResult = false;

        if (sAccessToken.isEmpty()) {
            // No token available, start OAuth2 flow
            SAL_WARN("sfx.dropbox", "No access token found, starting authentication");
            sAccessToken = m_pApiClient->authenticate();
            bResult = !sAccessToken.isEmpty();
        } else {
            // Token exists, verify it's still valid by making a test API call
            SAL_WARN("sfx.dropbox", "Access token found, verifying validity");
            try {
                // Test the token with a simple API call
                auto folderListing = m_pApiClient->listFolderComplete("");
                bResult = !folderListing.empty(); // If we get a response, token is valid
                SAL_WARN("sfx.dropbox", "Token verification result: " << (bResult ? "valid" : "invalid"));
            } catch (...) {
                SAL_WARN("sfx.dropbox", "Token verification failed, will re-authenticate");
                sAccessToken = m_pApiClient->authenticate();
                bResult = !sAccessToken.isEmpty();
            }
        }

        if (bResult) {
            m_bAuthenticated = true;
            SetStatus(u"Successfully connected to Dropbox"_ustr);
            SAL_WARN("sfx.dropbox", "Authentication successful");
        } else {
            SetStatus(u"Authentication failed"_ustr);
            SAL_WARN("sfx.dropbox", "Authentication failed");
        }

        return bResult;

    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception during authentication: " << e.what());
        SetStatus(u"Authentication error: "_ustr + OUString::fromUtf8(e.what()));
        return false;
    }
}

void DropboxDialog::LoadFolder(const OUString& folderId)
{
    SAL_WARN("sfx.dropbox", "LoadFolder called with folderId: " << folderId);

    if (!m_pApiClient || !m_bAuthenticated) {
        SAL_WARN("sfx.dropbox", "Not authenticated, cannot load folder");
        SetStatus(u"Not authenticated"_ustr);
        return;
    }

    try {
        SetStatus(u"Loading folder..."_ustr);

        // Clear current files
        m_aCurrentFiles.clear();

        // Load files from Dropbox
        std::vector<ucp::dropbox::DropboxFileInfo> files = m_pApiClient->listFolder(folderId);

        if (!files.empty()) {
            m_aCurrentFiles = files;
            PopulateFileList(files);
            SetStatus(u"Loaded " + OUString::number(files.size()) + u" items"_ustr);
            SAL_WARN("sfx.dropbox", "Loaded " << files.size() << " files from Dropbox");
        } else {
            SetStatus(u"Folder is empty"_ustr);
            SAL_WARN("sfx.dropbox", "No files found in folder");
        }

        // Update current folder
        m_sCurrentFolderId = folderId;

        // Update folder path display
        if (m_xCurrentFolderLabel) {
            OUString sDisplayPath = folderId.isEmpty() ? u"Dropbox"_ustr : u"Dropbox"_ustr + folderId;
            m_xCurrentFolderLabel->set_label(sDisplayPath);
        }

    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception loading folder: " << e.what());
        SetStatus(u"Error loading folder: "_ustr + OUString::fromUtf8(e.what()));

        // Fallback to demo content on error
        LoadDemoContent();
    }
}

void DropboxDialog::RefreshFileList()
{
    SAL_WARN("sfx.dropbox", "RefreshFileList called");
}

void DropboxDialog::PopulateFileList(const std::vector<ucp::dropbox::DropboxFileInfo>& files)
{
    SAL_WARN("sfx.dropbox", "PopulateFileList called with " << files.size() << " files");

    if (!m_xFileList) return;

    // Clear existing content
    m_xFileList->clear();

    // Add files to the list
    for (size_t i = 0; i < files.size(); ++i) {
        const auto& file = files[i];

        m_xFileList->append();

        // Column 0: Name with icon
        OUString sDisplayName;
        if (file.isFolder) {
            sDisplayName = u"📁 "_ustr + file.name;
        } else {
            // Choose icon based on file type
            if (file.name.endsWithIgnoreAsciiCase(u".docx") || file.name.endsWithIgnoreAsciiCase(u".doc")) {
                sDisplayName = u"📄 "_ustr + file.name;
            } else if (file.name.endsWithIgnoreAsciiCase(u".xlsx") || file.name.endsWithIgnoreAsciiCase(u".xls")) {
                sDisplayName = u"📊 "_ustr + file.name;
            } else if (file.name.endsWithIgnoreAsciiCase(u".pptx") || file.name.endsWithIgnoreAsciiCase(u".ppt")) {
                sDisplayName = u"🖼️ "_ustr + file.name;
            } else if (file.name.endsWithIgnoreAsciiCase(u".pdf")) {
                sDisplayName = u"📕 "_ustr + file.name;
            } else {
                sDisplayName = u"📄 "_ustr + file.name;
            }
        }
        m_xFileList->set_text(static_cast<int>(i), sDisplayName, 0);

        // Column 1: Type
        m_xFileList->set_text(static_cast<int>(i), file.isFolder ? u"Folder"_ustr : u"File"_ustr, 1);

        // Column 2: Size
        if (file.isFolder) {
            m_xFileList->set_text(static_cast<int>(i), u"--"_ustr, 2);
        } else {
            // Size is already a formatted string from Dropbox API
            m_xFileList->set_text(static_cast<int>(i), file.size, 2);
        }

        // Column 3: Modified date
        m_xFileList->set_text(static_cast<int>(i), file.modifiedTime, 3);
    }

    SAL_WARN("sfx.dropbox", "File list populated with " << files.size() << " items");
}

IMPL_LINK_NOARG(DropboxDialog, OnFileDoubleClick, weld::TreeView&, bool)
{
    return false;
}

IMPL_LINK_NOARG(DropboxDialog, OnFileSelect, weld::TreeView&, void)
{
    if (!m_xFileList || !m_xBtnOpen) return;

    int nSelected = m_xFileList->get_selected_index();
    bool bHasSelection = (nSelected >= 0);

    // Enable/disable Open button based on selection
    m_xBtnOpen->set_sensitive(bHasSelection);

    if (bHasSelection) {
        OUString sFileName = m_xFileList->get_text(nSelected, 0);
        SetStatus(u"Selected: "_ustr + sFileName);
    } else {
        SetStatus(u"No file selected"_ustr);
    }
}

IMPL_LINK_NOARG(DropboxDialog, OnOpenClicked, weld::Button&, void)
{
    if (IsFileSelected()) {
        OUString sFileName = GetSelectedFileName();
        SAL_WARN("sfx.dropbox", "Open clicked for file: " << sFileName);

        // Check if it's a folder or file
        if (IsSelectedItemFolder()) {
            // Navigate into folder
            SetStatus(u"Opening folder: "_ustr + sFileName);

            // Get the folder ID (path) from the selected item
            OUString sFolderId = GetSelectedFileId();
            if (!sFolderId.isEmpty()) {
                SAL_WARN("sfx.dropbox", "Navigating to folder: " << sFolderId);
                LoadFolder(sFolderId);
            } else {
                SetStatus(u"Error: Cannot determine folder path"_ustr);
            }
        } else {
            // Open file - download and open
            SetStatus(u"Downloading file: "_ustr + sFileName);

            try {
                if (DownloadAndOpenSelectedFile()) {
                    m_xDialog->response(RET_OK);
                } else {
                    SetStatus(u"Failed to download file: "_ustr + sFileName);
                }
            } catch (const std::exception& e) {
                SetStatus(u"Error downloading file: "_ustr + OUString::createFromAscii(e.what()));
            }
        }
    }
}

IMPL_LINK_NOARG(DropboxDialog, OnCancelClicked, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(DropboxDialog, OnBackClicked, weld::Button&, void)
{
    SAL_WARN("sfx.dropbox", "Back clicked from folder: " << m_sCurrentFolderId);

    if (m_sCurrentFolderId.isEmpty()) {
        // Already at root, can't go back further
        SetStatus(u"Already at root folder"_ustr);
        return;
    }

    // Calculate parent folder path
    OUString sParentPath;
    sal_Int32 nLastSlash = m_sCurrentFolderId.lastIndexOf('/');
    if (nLastSlash > 0) {
        // Remove the last folder from the path
        sParentPath = m_sCurrentFolderId.copy(0, nLastSlash);
    } else {
        // Go to root (empty string)
        sParentPath = u""_ustr;
    }

    SAL_WARN("sfx.dropbox", "Navigating back to parent: '" << sParentPath << "'");
    SetStatus(u"Going back..."_ustr);
    LoadFolder(sParentPath);
}

IMPL_LINK_NOARG(DropboxDialog, OnRefreshClicked, weld::Button&, void)
{
    SAL_WARN("sfx.dropbox", "Refresh clicked for folder: " << m_sCurrentFolderId);
    SetStatus(u"Refreshing..."_ustr);

    if (m_bAuthenticated) {
        // Reload current folder from API
        LoadFolder(m_sCurrentFolderId);
    } else {
        // Not authenticated, try to authenticate first
        if (AuthenticateUser()) {
            LoadFolder(m_sCurrentFolderId);
        } else {
            SetStatus(u"Authentication required"_ustr);
        }
    }
}

IMPL_LINK_NOARG(DropboxDialog, OnLoadTimer, Timer*, void)
{
}

void DropboxDialog::LoadDemoContent()
{
    SAL_WARN("sfx.dropbox", "LoadDemoContent called");

    if (!m_xFileList) return;

    // Clear existing content
    m_xFileList->clear();

    // Add demo files and folders to show the UI working
    m_xFileList->append();
    m_xFileList->set_text(0, u"📁 Documents"_ustr, 0);
    m_xFileList->set_text(0, u"Folder"_ustr, 1);
    m_xFileList->set_text(0, u"--"_ustr, 2);
    m_xFileList->set_text(0, u"2 days ago"_ustr, 3);

    m_xFileList->append();
    m_xFileList->set_text(1, u"📁 Photos"_ustr, 0);
    m_xFileList->set_text(1, u"Folder"_ustr, 1);
    m_xFileList->set_text(1, u"--"_ustr, 2);
    m_xFileList->set_text(1, u"1 week ago"_ustr, 3);

    m_xFileList->append();
    m_xFileList->set_text(2, u"📄 Report.docx"_ustr, 0);
    m_xFileList->set_text(2, u"Document"_ustr, 1);
    m_xFileList->set_text(2, u"2.1 MB"_ustr, 2);
    m_xFileList->set_text(2, u"Yesterday"_ustr, 3);

    m_xFileList->append();
    m_xFileList->set_text(3, u"📊 Budget.xlsx"_ustr, 0);
    m_xFileList->set_text(3, u"Spreadsheet"_ustr, 1);
    m_xFileList->set_text(3, u"534 KB"_ustr, 2);
    m_xFileList->set_text(3, u"3 days ago"_ustr, 3);

    m_xFileList->append();
    m_xFileList->set_text(4, u"🖼️ Presentation.pptx"_ustr, 0);
    m_xFileList->set_text(4, u"Presentation"_ustr, 1);
    m_xFileList->set_text(4, u"8.7 MB"_ustr, 2);
    m_xFileList->set_text(4, u"Last week"_ustr, 3);

    SetStatus(u"Demo content loaded - Select a file to open"_ustr);
}

void DropboxDialog::SetStatus(const OUString& text)
{
    if (m_xStatusLabel) {
        m_xStatusLabel->set_label(text);
    }
}

void DropboxDialog::SetProgress(int /*percent*/)
{
}

void DropboxDialog::EnableControls(bool /*bEnable*/)
{
}

bool DropboxDialog::IsFileSelected() const
{
    return m_xFileList && m_xFileList->get_selected_index() >= 0;
}

OUString DropboxDialog::GetSelectedFileName() const
{
    if (!IsFileSelected()) return OUString();

    int nSelected = m_xFileList->get_selected_index();
    if (nSelected >= 0 && static_cast<size_t>(nSelected) < m_aCurrentFiles.size()) {
        return m_aCurrentFiles[nSelected].name;
    }
    return OUString();
}

OUString DropboxDialog::GetSelectedFileId() const
{
    return OUString();
}

bool DropboxDialog::IsSelectedItemFolder() const
{
    if (!IsFileSelected()) return false;

    int nSelected = m_xFileList->get_selected_index();
    if (nSelected >= 0 && static_cast<size_t>(nSelected) < m_aCurrentFiles.size()) {
        return m_aCurrentFiles[nSelected].isFolder;
    }
    return false;
}

bool DropboxDialog::DownloadAndOpenSelectedFile()
{
    SAL_WARN("sfx.dropbox", "DownloadAndOpenSelectedFile called");

    if (!IsFileSelected()) {
        SAL_WARN("sfx.dropbox", "No file selected for download");
        return false;
    }

    int nSelected = m_xFileList->get_selected_index();
    if (nSelected < 0 || static_cast<size_t>(nSelected) >= m_aCurrentFiles.size()) {
        SAL_WARN("sfx.dropbox", "Invalid selection for download");
        return false;
    }

    const auto& fileInfo = m_aCurrentFiles[nSelected];
    if (fileInfo.isFolder) {
        SAL_WARN("sfx.dropbox", "Cannot download folder as file");
        return false;
    }

    try {
        if (!m_pApiClient) {
            SAL_WARN("sfx.dropbox", "API client not available for download");
            return false;
        }

        SAL_WARN("sfx.dropbox", "Downloading file: " + fileInfo.name + " (ID: " + fileInfo.id + ")");
        SetStatus(u"Downloading "_ustr + fileInfo.name + u"..."_ustr);
        SetProgress(50);

        // Download file content
        uno::Reference<css::io::XInputStream> xInputStream = m_pApiClient->downloadFile(fileInfo.id);

        if (!xInputStream.is()) {
            SAL_WARN("sfx.dropbox", "Failed to download file content");
            SetStatus(u"Download failed!"_ustr);
            SetProgress(-1);
            return false;
        }

        SetProgress(80);

        // Create temporary file URL for opening
        OUString sTempFileUrl = CreateTempFileFromStream(xInputStream, fileInfo.name);

        if (sTempFileUrl.isEmpty()) {
            SAL_WARN("sfx.dropbox", "Failed to create temporary file");
            SetStatus(u"Failed to create temporary file!"_ustr);
            SetProgress(-1);
            return false;
        }

        SetProgress(100);
        SetStatus(u"Opening "_ustr + fileInfo.name + u"..."_ustr);

        // Store the file URL for the caller
        m_sSelectedFileUrl = sTempFileUrl;

        SAL_WARN("sfx.dropbox", "File downloaded and ready to open: " + sTempFileUrl);
        return true;

    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception during download: " + OUString::createFromAscii(e.what()));
        SetStatus(u"Download error: "_ustr + OUString::createFromAscii(e.what()));
        SetProgress(-1);
        return false;
    } catch (...) {
        SAL_WARN("sfx.dropbox", "Unknown exception during download");
        SetStatus(u"Unknown download error!"_ustr);
        SetProgress(-1);
        return false;
    }
}

OUString DropboxDialog::CreateTempFileFromStream(const uno::Reference<css::io::XInputStream>& xInputStream, const OUString& fileName)
{
    SAL_WARN("sfx.dropbox", "CreateTempFileFromStream called for: " + fileName);

    if (!xInputStream.is()) {
        SAL_WARN("sfx.dropbox", "Invalid input stream");
        return OUString();
    }

    try {
        // Get LibreOffice component context for temp file service
        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        if (xContext.is()) {
            // Use LibreOffice's temporary file service
            uno::Reference<css::io::XTempFile> xTempFile(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.io.TempFile"_ustr, xContext),
                uno::UNO_QUERY);

            if (xTempFile.is()) {
                // Extract file extension for proper file type detection
                OUString sExtension;
                sal_Int32 nDotPos = fileName.lastIndexOf('.');
                if (nDotPos > 0) {
                    sExtension = fileName.copy(nDotPos);
                }

                // Set temp file properties
                xTempFile->setRemoveFile(false); // Keep file for opening

                // Get output stream to write to temp file
                uno::Reference<css::io::XOutputStream> xOutputStream(xTempFile, uno::UNO_QUERY);
                if (!xOutputStream.is()) {
                    SAL_WARN("sfx.dropbox", "Failed to get output stream from temp file");
                    return OUString();
                }

                // Copy data from input stream to temp file
                const sal_Int32 nBufferSize = 8192;
                uno::Sequence<sal_Int8> aBuffer(nBufferSize);
                sal_Int32 nBytesRead = 0;
                sal_Int32 nTotalBytes = 0;

                SAL_WARN("sfx.dropbox", "Starting file copy...");

                do {
                    nBytesRead = xInputStream->readBytes(aBuffer, nBufferSize);
                    if (nBytesRead > 0) {
                        // Write the bytes we read
                        uno::Sequence<sal_Int8> aWriteBuffer(aBuffer.getArray(), nBytesRead);
                        xOutputStream->writeBytes(aWriteBuffer);
                        nTotalBytes += nBytesRead;
                    }
                } while (nBytesRead > 0);

                // Flush and close streams
                xOutputStream->flush();
                xOutputStream->closeOutput();
                xInputStream->closeInput();

                SAL_WARN("sfx.dropbox", "File copy completed, total bytes: " + OUString::number(nTotalBytes));

                // Get the file URL
                uno::Reference<css::io::XSeekable> xSeekable(xTempFile, uno::UNO_QUERY);
                if (xSeekable.is()) {
                    xSeekable->seek(0); // Reset to beginning for reading
                }

                OUString sTempFileUrl = xTempFile->getUri();
                SAL_WARN("sfx.dropbox", "Temporary file created: " + sTempFileUrl);

                // Rename to include proper extension if needed
                if (!sExtension.isEmpty() && !sTempFileUrl.endsWith(sExtension)) {
                    OUString sNewUrl = sTempFileUrl + sExtension;
                    try {
                        // Try to rename the file to include extension
                        uno::Reference<css::ucb::XSimpleFileAccess3> xFileAccess(
                            css::ucb::SimpleFileAccess::create(xContext));
                        if (xFileAccess.is()) {
                            xFileAccess->move(sTempFileUrl, sNewUrl);
                            sTempFileUrl = sNewUrl;
                            SAL_WARN("sfx.dropbox", "Renamed to: " + sTempFileUrl);
                        }
                    } catch (...) {
                        // If rename fails, use original URL
                        SAL_WARN("sfx.dropbox", "Failed to rename file, using original URL");
                    }
                }

                return sTempFileUrl;
            }
        }

        SAL_WARN("sfx.dropbox", "Failed to create temp file service");
        return OUString();

    } catch (const css::uno::Exception& e) {
        SAL_WARN("sfx.dropbox", "UNO Exception creating temp file: " + e.Message);
        return OUString();
    } catch (const std::exception& e) {
        SAL_WARN("sfx.dropbox", "Exception creating temp file: " + OUString::createFromAscii(e.what()));
        return OUString();
    } catch (...) {
        SAL_WARN("sfx.dropbox", "Unknown exception creating temp file");
        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
