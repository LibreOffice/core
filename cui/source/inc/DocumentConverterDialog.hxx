/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX
#define INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include <rtl/ustring.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/weld.hxx>

class FileListUpdaterThread;

class DocumentConverterDialog : public weld::GenericDialogController
{
public:
    typedef std::vector<std::shared_ptr<const SfxFilter>> FilterList;

    DocumentConverterDialog(weld::Widget* pParent);
    ~DocumentConverterDialog();

private:
    DECL_LINK(ImportDocTypesSelChange, weld::ComboBox&, void);
    DECL_LINK(SelectDirClick, weld::Button&, void);
    DECL_LINK(DirChanged, weld::Entry&, void);
    DECL_LINK(IncludeSubdirsToggled, weld::ToggleButton&, void);

    DECL_LINK(AddFileToList, void*, void);

    void UpdateControlsState();
    void UpdateFileSelection();

    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Entry> m_xImportDir;
    std::unique_ptr<weld::CheckButton> m_xIncludeSubdirs;
    std::unique_ptr<weld::ComboBox> m_xImportDocTypes;
    std::unique_ptr<weld::Label> m_xImportMaskLabel;
    std::unique_ptr<weld::Entry> m_xImportMask;
    std::unique_ptr<weld::Entry> m_xExportDir;
    std::unique_ptr<weld::ComboBox> m_xExportDocTypes;
    std::unique_ptr<weld::Button> m_xSelectImportDir;
    std::unique_ptr<weld::Button> m_xSelectExportDir;
    std::unique_ptr<weld::TreeView> m_xSelectedFiles;

    std::vector<std::tuple<OUString, OUString, OUString>> m_aServiceList;
    std::unordered_map<OUString, FilterList> m_aImportLists;
    std::unordered_map<OUString, FilterList> m_aExportLists;

    std::vector<OUString> m_aSelectedFiles;
    rtl::Reference<FileListUpdaterThread> m_pFileListUpdater;
};

#endif // INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
