/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/googledrivedialog.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>

// Include Google Drive API client
#include "../../../ucb/source/ucp/gdrive/GoogleDriveApiClient.hxx"
#include "../../../ucb/source/ucp/gdrive/gdrive_json.hxx"

#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace css;
using namespace css::uno;
using namespace css::ucb;

GoogleDriveDialog::GoogleDriveDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"sfx/ui/googledrivedialog.ui"_ustr, u"GoogleDriveDialog"_ustr)
    , m_xContext(comphelper::getProcessComponentContext())
    , m_sCurrentFolderId(u"root"_ustr)
    , m_bAuthenticated(false)
    , m_aAuthTimer("GoogleDriveAuthTimer")
    , m_aLoadTimer("GoogleDriveLoadTimer")
{
    try {
        SAL_WARN("sfx.googledrive", "GoogleDriveDialog constructor - getting UI controls");

        // Get UI controls with safety checks
        m_xFileList = m_xBuilder->weld_tree_view(u"file_list"_ustr);
        m_xBtnOpen = m_xBuilder->weld_button(u"btn_open"_ustr);
        m_xBtnCancel = m_xBuilder->weld_button(u"btn_cancel"_ustr);
        m_xStatusLabel = m_xBuilder->weld_label(u"status_label"_ustr);
        m_xProgressBar = m_xBuilder->weld_progress_bar(u"progress_bar"_ustr);
        m_xBtnBack = m_xBuilder->weld_button(u"btn_back"_ustr);
        m_xCurrentFolderLabel = m_xBuilder->weld_label(u"current_folder_label"_ustr);

        if (!m_xFileList || !m_xBtnOpen || !m_xBtnCancel || !m_xStatusLabel ||
            !m_xProgressBar || !m_xBtnBack || !m_xCurrentFolderLabel) {
            SAL_WARN("sfx.googledrive", "Failed to get one or more UI controls from dialog");
            return;
        }

        SAL_WARN("sfx.googledrive", "All UI controls obtained successfully");

        // Create command environment for Google Drive API
        m_xCmdEnv = new ucbhelper::CommandEnvironment(
            Reference<css::task::XInteractionHandler>(),
            Reference<css::ucb::XProgressHandler>());

        // Create Google Drive API client
        try {
            m_pApiClient = std::make_unique<ucp::gdrive::GoogleDriveApiClient>(m_xCmdEnv);
            SAL_WARN("sfx.googledrive", "GoogleDriveApiClient created successfully");
        } catch (const std::exception& e) {
            SAL_WARN("sfx.googledrive", "Failed to create GoogleDriveApiClient: " << e.what());
        }

        // Setup TreeView columns programmatically
        m_xFileList->clear();
        m_xFileList->set_column_title(0, u"Name"_ustr);
        m_xFileList->set_column_title(1, u"Type"_ustr);
        m_xFileList->set_column_title(2, u"Size"_ustr);
        m_xFileList->set_column_title(3, u"Modified"_ustr);

        // Setup file list columns
        std::vector<int> aWidths;
        aWidths.push_back(300); // Name
        aWidths.push_back(100); // Type
        aWidths.push_back(80);  // Size
        aWidths.push_back(120); // Modified
        m_xFileList->set_column_fixed_widths(aWidths);

        // Connect event handlers
        m_xFileList->connect_row_activated(LINK(this, GoogleDriveDialog, FileListDoubleClick));
        m_xFileList->connect_selection_changed(LINK(this, GoogleDriveDialog, FileListSelectionChanged));
        m_xBtnOpen->connect_clicked(LINK(this, GoogleDriveDialog, OpenButtonClicked));
        m_xBtnCancel->connect_clicked(LINK(this, GoogleDriveDialog, CancelButtonClicked));
        m_xBtnBack->connect_clicked(LINK(this, GoogleDriveDialog, BackButtonClicked));

        // Setup timers
        m_aAuthTimer.SetTimeout(3000); // Check authentication after 3 seconds
        m_aAuthTimer.SetInvokeHandler(LINK(this, GoogleDriveDialog, AuthTimerHandler));

        m_aLoadTimer.SetTimeout(500); // Load delay
        m_aLoadTimer.SetInvokeHandler(LINK(this, GoogleDriveDialog, LoadTimerHandler));

        // Initial state
        m_xBtnOpen->set_sensitive(false);
        m_xBtnBack->set_sensitive(false);
        m_xProgressBar->set_visible(false);

        // Set reasonable dialog size
        m_xDialog->set_size_request(600, 400);

        SetStatus(u"Connecting to Google Drive..."_ustr, true);

        // Add some test data if API client is not available
        if (!m_pApiClient) {
            SAL_WARN("sfx.googledrive", "No API client - adding test data");
            AddTestData();
        }

        SAL_WARN("sfx.googledrive", "GoogleDriveDialog constructor completed successfully");
    } catch (const std::exception& e) {
        SAL_WARN("sfx.googledrive", "Exception in GoogleDriveDialog constructor: " << e.what());
    } catch (...) {
        SAL_WARN("sfx.googledrive", "Unknown exception in GoogleDriveDialog constructor");
    }
}

GoogleDriveDialog::~GoogleDriveDialog()
{
    m_aAuthTimer.Stop();
    m_aLoadTimer.Stop();
}

bool GoogleDriveDialog::Execute()
{
    try {
        SAL_WARN("sfx.googledrive", "GoogleDriveDialog::Execute() called");

        // Start authentication process
        StartAuthentication();

        // Run the dialog
        int nResult = run();
        SAL_WARN("sfx.googledrive", "Dialog run() returned: " << nResult);
        return nResult == RET_OK;
    } catch (const std::exception& e) {
        SAL_WARN("sfx.googledrive", "Exception in GoogleDriveDialog::Execute(): " << e.what());
        return false;
    } catch (...) {
        SAL_WARN("sfx.googledrive", "Unknown exception in GoogleDriveDialog::Execute()");
        return false;
    }
}

void GoogleDriveDialog::StartAuthentication()
{
    SAL_WARN("sfx.googledrive", "Starting Google Drive authentication");

    // Start real authentication - this will be triggered automatically
    // when the API client tries to access Google Drive
    SetStatus(u"Authenticating with Google Drive..."_ustr, true);

    // Start loading the root folder, which will trigger authentication
    LoadFolder();
}

void GoogleDriveDialog::CheckAuthenticationStatus()
{
    // This method is no longer needed since authentication happens automatically
    // in the API client when needed
    m_bAuthenticated = true;
    m_aAuthTimer.Stop();

    SetStatus(u"Loading files..."_ustr, true);
    LoadFolder();
}

IMPL_LINK_NOARG(GoogleDriveDialog, AuthTimerHandler, Timer*, void)
{
    // Simulate authentication completion after 3 seconds
    static int nCount = 0;
    nCount++;

    if (nCount >= 3)
    {
        CheckAuthenticationStatus();
    }
}

void GoogleDriveDialog::LoadFolder(const OUString& folderId)
{
    m_sCurrentFolderId = folderId;
    m_aCurrentFiles.clear();

    if (!m_pApiClient) {
        SAL_WARN("sfx.googledrive", "No API client available for folder loading");
        SetStatus(u"Error: Google Drive API not available"_ustr, false);
        return;
    }

    try {
        SAL_WARN("sfx.googledrive", "Loading folder: " << folderId);
        SetStatus(u"Loading files..."_ustr, true);

        // Use real Google Drive API to list folder contents
        m_aCurrentFiles = m_pApiClient->listFolder(folderId);

        SAL_WARN("sfx.googledrive", "Loaded " << m_aCurrentFiles.size() << " files from folder " << folderId);

        PopulateFileList();
        UpdateUI();
        SetStatus(u"Ready"_ustr, false);

    } catch (const std::exception& e) {
        SAL_WARN("sfx.googledrive", "Failed to load folder " << folderId << ": " << e.what());
        SetStatus(u"Error loading folder: "_ustr + OUString::fromUtf8(e.what()), false);

        // Fall back to showing an empty list
        PopulateFileList();
        UpdateUI();
    }
}

void GoogleDriveDialog::RefreshCurrentFolder()
{
    LoadFolder(m_sCurrentFolderId);
}

IMPL_LINK_NOARG(GoogleDriveDialog, LoadTimerHandler, Timer*, void)
{
    RefreshCurrentFolder();
}

void GoogleDriveDialog::PopulateFileList()
{
    SAL_WARN("sfx.googledrive", "PopulateFileList: Populating " + OUString::number(m_aCurrentFiles.size()) + " files");

    if (!m_xFileList) {
        SAL_WARN("sfx.googledrive", "PopulateFileList: m_xFileList is null!");
        return;
    }

    m_xFileList->clear();

    // If no files, show a message
    if (m_aCurrentFiles.empty()) {
        m_xFileList->append();
        m_xFileList->set_text(0, u"(No files found)"_ustr, 0);
        m_xFileList->set_text(0, u"-"_ustr, 1);
        m_xFileList->set_text(0, u"-"_ustr, 2);
        m_xFileList->set_text(0, u"-"_ustr, 3);
        return;
    }

    for (const auto& fileInfo : m_aCurrentFiles)
    {
        SAL_WARN("sfx.googledrive", "PopulateFileList: Adding file: " + fileInfo.name);

        // Create a new row with all columns at once
        std::unique_ptr<weld::TreeIter> xIter = m_xFileList->make_iterator();

        // Name column (with folder icon for folders)
        OUString sDisplayName = fileInfo.name;
        if (fileInfo.isFolder)
            sDisplayName = u"[DIR] " + sDisplayName;
        else
            sDisplayName = u"      " + sDisplayName;

        // Prepare all column values
        OUString sType = GetFileTypeDescription(fileInfo.mimeType);
        OUString sSize = u"-"_ustr;
        if (!fileInfo.isFolder && !fileInfo.size.isEmpty())
        {
            try {
                sal_Int64 nSize = fileInfo.size.toInt64();
                sSize = FormatFileSize(nSize);
            } catch (...) {
                sSize = fileInfo.size;
            }
        }

        OUString sModified = fileInfo.modifiedTime.isEmpty() ? u"-"_ustr : fileInfo.modifiedTime;

        // Insert the row
        m_xFileList->insert(nullptr, -1, &sDisplayName, &fileInfo.id, nullptr, nullptr, false, xIter.get());
        m_xFileList->set_text(*xIter, sType, 1);
        m_xFileList->set_text(*xIter, sSize, 2);
        m_xFileList->set_text(*xIter, sModified, 3);
    }

    SAL_WARN("sfx.googledrive", "PopulateFileList: Completed, tree has " + OUString::number(m_xFileList->n_children()) + " children");
}

void GoogleDriveDialog::UpdateUI()
{
    // Update current folder label
    if (m_sCurrentFolderId == u"root")
        m_xCurrentFolderLabel->set_label(u"Google Drive"_ustr);
    else
        m_xCurrentFolderLabel->set_label(u"Google Drive / Folder"_ustr); // TODO: Show actual path

    // Update back button
    m_xBtnBack->set_sensitive(!m_aFolderPath.empty());

    // Update open button based on selection
    FileListSelectionChanged(*m_xFileList);
}

IMPL_LINK_NOARG(GoogleDriveDialog, FileListDoubleClick, weld::TreeView&, bool)
{
    SAL_WARN("sfx.googledrive", "FileListDoubleClick called");

    std::unique_ptr<weld::TreeIter> xIter = m_xFileList->make_iterator();
    if (!m_xFileList->get_selected(xIter.get()))
    {
        SAL_WARN("sfx.googledrive", "No selection on double click");
        return false;
    }

    OUString sFileId = m_xFileList->get_id(*xIter);
    SAL_WARN("sfx.googledrive", "Double-clicked file ID: " + sFileId);

    // Find the file info
    auto it = std::find_if(m_aCurrentFiles.begin(), m_aCurrentFiles.end(),
                          [&sFileId](const ucp::gdrive::GDriveFileInfo& info) { return info.id == sFileId; });

    if (it != m_aCurrentFiles.end())
    {
        SAL_WARN("sfx.googledrive", "Double-clicked file: " + it->name);

        if (it->isFolder)
        {
            SAL_WARN("sfx.googledrive", "Navigating to folder: " + it->name);
            // Navigate to folder
            NavigateToFolder(sFileId, it->name);
        }
        else
        {
            SAL_WARN("sfx.googledrive", "Opening file: " + it->name);
            // Open file (same as clicking Open button)
            m_sSelectedFileId = sFileId;
            m_xDialog->response(RET_OK);
        }
    }
    else
    {
        SAL_WARN("sfx.googledrive", "Double-clicked file not found in current files list");
    }

    return true;
}

IMPL_LINK_NOARG(GoogleDriveDialog, FileListSelectionChanged, weld::TreeView&, void)
{
    SAL_WARN("sfx.googledrive", "FileListSelectionChanged called");

    std::unique_ptr<weld::TreeIter> xIter = m_xFileList->make_iterator();
    if (!m_xFileList->get_selected(xIter.get()))
    {
        SAL_WARN("sfx.googledrive", "No selection");
        m_xBtnOpen->set_sensitive(false);
        m_sSelectedFileId.clear();
        return;
    }

    OUString sFileId = m_xFileList->get_id(*xIter);
    SAL_WARN("sfx.googledrive", "Selected file ID: " + sFileId);

    // Find the file info
    auto it = std::find_if(m_aCurrentFiles.begin(), m_aCurrentFiles.end(),
                          [&sFileId](const ucp::gdrive::GDriveFileInfo& info) { return info.id == sFileId; });

    if (it != m_aCurrentFiles.end())
    {
        m_sSelectedFileId = sFileId;
        SAL_WARN("sfx.googledrive", "Selected file: " + it->name + " (folder: " + (it->isFolder ? u"yes" : u"no") + ")");

        // Enable Open button for both files and folders
        m_xBtnOpen->set_sensitive(true);

        if (it->isFolder)
            m_xBtnOpen->set_label(u"Open Folder"_ustr);
        else
            m_xBtnOpen->set_label(u"Open File"_ustr);
    }
    else
    {
        SAL_WARN("sfx.googledrive", "File not found in current files list");
        m_xBtnOpen->set_sensitive(false);
        m_sSelectedFileId.clear();
    }
}

IMPL_LINK_NOARG(GoogleDriveDialog, OpenButtonClicked, weld::Button&, void)
{
    if (m_sSelectedFileId.isEmpty())
        return;

    // Find the selected file
    auto it = std::find_if(m_aCurrentFiles.begin(), m_aCurrentFiles.end(),
                          [this](const ucp::gdrive::GDriveFileInfo& info) { return info.id == m_sSelectedFileId; });

    if (it != m_aCurrentFiles.end())
    {
        if (it->isFolder)
        {
            NavigateToFolder(m_sSelectedFileId, it->name);
        }
        else
        {
            m_xDialog->response(RET_OK);
        }
    }
}

IMPL_LINK_NOARG(GoogleDriveDialog, CancelButtonClicked, weld::Button&, void)
{
    SAL_WARN("sfx.googledrive", "Cancel button clicked");
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(GoogleDriveDialog, BackButtonClicked, weld::Button&, void)
{
    NavigateBack();
}

void GoogleDriveDialog::NavigateToFolder(const OUString& folderId, const OUString& /* folderName */)
{
    // Add current folder to path stack
    m_aFolderPath.push_back(m_sCurrentFolderId);

    SetStatus(u"Loading folder..."_ustr, true);
    LoadFolder(folderId);
}

void GoogleDriveDialog::NavigateBack()
{
    if (m_aFolderPath.empty())
        return;

    // Get parent folder ID from stack
    OUString sParentId = m_aFolderPath.back();
    m_aFolderPath.pop_back();

    SetStatus(u"Loading folder..."_ustr, true);
    LoadFolder(sParentId);
}

void GoogleDriveDialog::SetStatus(const OUString& sMessage, bool bShowProgress)
{
    m_xStatusLabel->set_label(sMessage);
    m_xProgressBar->set_visible(bShowProgress);

    if (bShowProgress)
    {
        // Note: pulse() method may not be available in all weld implementations
        // m_xProgressBar->pulse();
    }
}

OUString GoogleDriveDialog::FormatFileSize(sal_Int64 nBytes) const
{
    if (nBytes < 1024)
        return OUString::number(nBytes) + " B";
    else if (nBytes < 1024 * 1024)
        return OUString::number(nBytes / 1024) + " KB";
    else if (nBytes < 1024 * 1024 * 1024)
        return OUString::number(nBytes / (1024 * 1024)) + " MB";
    else
        return OUString::number(nBytes / (1024 * 1024 * 1024)) + " GB";
}

OUString GoogleDriveDialog::GetFileTypeDescription(const OUString& sMimeType) const
{
    if (sMimeType == u"application/vnd.google-apps.folder")
        return u"Folder"_ustr;
    else if (sMimeType == u"application/vnd.oasis.opendocument.text")
        return u"Writer Document"_ustr;
    else if (sMimeType == u"application/vnd.oasis.opendocument.spreadsheet")
        return u"Calc Spreadsheet"_ustr;
    else if (sMimeType.startsWith(u"application/vnd.openxmlformats"))
        return u"Office Document"_ustr;
    else if (sMimeType.startsWith(u"text/"))
        return u"Text File"_ustr;
    else if (sMimeType.startsWith(u"image/"))
        return u"Image"_ustr;
    else
        return u"File"_ustr;
}

OUString GoogleDriveDialog::GetSelectedFileURL() const
{
    if (m_sSelectedFileId.isEmpty())
        return OUString();

    return u"gdrive://" + m_sSelectedFileId;
}

void GoogleDriveDialog::ShowErrorMessage(const OUString& sError)
{
    SetStatus(u"Error: " + sError, false);
    SAL_WARN("sfx.googledrive", "GoogleDriveDialog error: " << sError);
}

void GoogleDriveDialog::AddTestData()
{
    SAL_WARN("sfx.googledrive", "Adding test data for dialog testing");

    m_aCurrentFiles.clear();

    // Add some test files and folders
    ucp::gdrive::GDriveFileInfo folder1;
    folder1.id = u"test_folder_1"_ustr;
    folder1.name = u"My Documents"_ustr;
    folder1.mimeType = u"application/vnd.google-apps.folder"_ustr;
    folder1.isFolder = true;
    folder1.modifiedTime = u"2024-01-15T10:30:00Z"_ustr;
    m_aCurrentFiles.push_back(folder1);

    ucp::gdrive::GDriveFileInfo folder2;
    folder2.id = u"test_folder_2"_ustr;
    folder2.name = u"Photos"_ustr;
    folder2.mimeType = u"application/vnd.google-apps.folder"_ustr;
    folder2.isFolder = true;
    folder2.modifiedTime = u"2024-01-10T15:20:00Z"_ustr;
    m_aCurrentFiles.push_back(folder2);

    ucp::gdrive::GDriveFileInfo file1;
    file1.id = u"test_file_1"_ustr;
    file1.name = u"Document.odt"_ustr;
    file1.mimeType = u"application/vnd.oasis.opendocument.text"_ustr;
    file1.isFolder = false;
    file1.size = u"52428"_ustr;
    file1.modifiedTime = u"2024-01-12T09:15:00Z"_ustr;
    m_aCurrentFiles.push_back(file1);

    ucp::gdrive::GDriveFileInfo file2;
    file2.id = u"test_file_2"_ustr;
    file2.name = u"Spreadsheet.ods"_ustr;
    file2.mimeType = u"application/vnd.oasis.opendocument.spreadsheet"_ustr;
    file2.isFolder = false;
    file2.size = u"18675"_ustr;
    file2.modifiedTime = u"2024-01-08T14:45:00Z"_ustr;
    m_aCurrentFiles.push_back(file2);

    ucp::gdrive::GDriveFileInfo file3;
    file3.id = u"test_file_3"_ustr;
    file3.name = u"Image.jpg"_ustr;
    file3.mimeType = u"image/jpeg"_ustr;
    file3.isFolder = false;
    file3.size = u"2048576"_ustr;
    file3.modifiedTime = u"2024-01-05T11:00:00Z"_ustr;
    m_aCurrentFiles.push_back(file3);

    // Update the UI
    PopulateFileList();
    UpdateUI();
    SetStatus(u"Test data loaded (Google Drive unavailable)"_ustr, false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
