/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include "PlacesListBox.hxx"

#include <vcl/idle.hxx>

#include <deque>
#include <memory>
#include <vector>

class CheckBox;
class SvtFileDialog;

#define FILEDIALOG_DEF_EXTSEP       ';'
#define FILEDIALOG_DEF_WILDCARD     '*'


// SvtFileDialogFilter_Impl


/*  [Description]

    Instances of this class represent a filter.
*/

class SvtFileDialogFilter_Impl
{
private:
    OUString m_aName;    // name of the entry
    OUString m_aType;    // filter wildcard - if empty, the entry marks a group

public:
    SvtFileDialogFilter_Impl( const OUString& rName, const OUString& rType );
    ~SvtFileDialogFilter_Impl();

    const OUString&     GetName() const     { return m_aName; }
    const OUString&     GetType() const     { return m_aType; }
    OUString            GetExtension() const    { return m_aType.getLength() > 2 ? m_aType.copy( 2 ) : OUString(); }

    bool            isGroupSeparator() const    { return m_aType.isEmpty(); }
};

typedef std::deque<std::unique_ptr<SvtFileDialogFilter_Impl>> SvtFileDialogFilterList_Impl;

enum SvtFileDlgMode
{
    FILEDLG_MODE_OPEN = 0,
    FILEDLG_MODE_SAVE = 1
};

enum SvtFileDlgType
{
    FILEDLG_TYPE_FILEDLG = 0,
    FILEDLG_TYPE_PATHDLG
};

class SvtUpButton_Impl
{
private:
    std::unique_ptr<weld::Toolbar> m_xToolbar;
    std::unique_ptr<weld::Menu> m_xMenu;
    SvtFileDialog* m_pDlg;

    std::vector<OUString> aURLs;

public:
    SvtUpButton_Impl(std::unique_ptr<weld::Toolbar> xToolbar,
                     std::unique_ptr<weld::Menu> xMenu,
                     SvtFileDialog* pDlg);

    void set_help_id(const OString& rHelpId) { m_xToolbar->set_help_id(rHelpId); }
    void show() { m_xToolbar->show(); }

    void FillURLMenu();

    weld::Widget* getWidget() { return m_xToolbar.get(); }

private:

    DECL_LINK(SelectHdl, const OString&, void);
    DECL_LINK(ClickHdl, const OString&, void);
};

class SvtURLBox;
class SvtExpFileDlg_Impl
{
private:
    const SvtFileDialogFilter_Impl* m_pCurFilter;
    OUString                        m_sCurrentFilterDisplayName;    // may differ from m_pCurFilter->GetName in case it is a cached entry

    css::uno::Sequence< OUString > m_aDenyList;

public:
    SvtFileDialogFilterList_Impl    m_aFilter;
    std::unique_ptr<SvtFileDialogFilter_Impl> m_xUserFilter;

    std::unique_ptr<weld::Label> m_xFtFileName;
    std::unique_ptr<SvtURLBox> m_xEdFileName;

    std::unique_ptr<weld::Label> m_xSharedLabel;
    std::unique_ptr<weld::ComboBox> m_xSharedListBox;

    std::unique_ptr<weld::Label> m_xFtFileType;
    std::unique_ptr<weld::ComboBox> m_xLbFilter;
    std::unique_ptr<weld::Button> m_xBtnFileOpen;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnHelp;
    std::unique_ptr<SvtUpButton_Impl> m_xBtnUp;
    std::unique_ptr<weld::Button> m_xBtnNewFolder;
    std::unique_ptr<weld::CheckButton> m_xCbPassword;
    std::unique_ptr<weld::CheckButton> m_xCbGPGEncrypt;
    std::unique_ptr<SvtURLBox> m_xEdCurrentPath;
    std::unique_ptr<weld::CheckButton> m_xCbAutoExtension;
    std::unique_ptr<weld::CheckButton> m_xCbOptions;

    std::unique_ptr<PlacesListBox> m_xPlaces;
    std::unique_ptr<weld::Button> m_xBtnConnectToServer;

    SvtFileDlgMode m_eMode;
    SvtFileDlgType m_eDlgType;
    PickerFlags m_nStyle;

    OUString m_aStdDir;

    // delay filter when traveling the filterbox
    Idle m_aFilterIdle;

    // shows OpenHdl_Imp() if the open was triggered by a double click
    bool m_bDoubleClick;

    // MultiSelection?
    bool m_bMultiSelection;

    // remember sizes
    OUString m_aIniKey;

    explicit SvtExpFileDlg_Impl();
    ~SvtExpFileDlg_Impl();

    void             SetDenyList( const css::uno::Sequence< OUString >& rDenyList ) { m_aDenyList = rDenyList; }
    const css::uno::Sequence< OUString >& GetDenyList() const { return m_aDenyList; }
    void                    SetStandardDir( const OUString& rDir );
    const OUString&  GetStandardDir() const { return m_aStdDir; }

    // access to the filter listbox only as weld::Widget* - we want to maintain the entries/userdata ourself
    weld::Widget* GetFilterListControl() { return m_xLbFilter.get(); }
    const weld::Widget*  GetFilterListControl() const { return m_xLbFilter.get(); }
    void SetFilterListSelectHdl(const Link<weld::ComboBox&, void>& rHandler)
    {
        m_xLbFilter->connect_changed(rHandler);
    }

    // inits the listbox for the filters from the filter list (_pFilter)
            void            InitFilterList( );
    bool HasFilterListEntry( const OUString& rFilterName )
    {
        return m_xLbFilter->find_text(rFilterName) != -1;
    }

    void SelectFilterListEntry( const OUString& rFilterName )
    {
        m_xLbFilter->set_active_text(rFilterName);
    }

    void            InsertFilterListEntry( const SvtFileDialogFilter_Impl* _pFilterDesc );
    // _pFilterDesc must already have been added to _pFilter
    SvtFileDialogFilter_Impl* GetSelectedFilterEntry( OUString& rDisplayName ) const
    {
        rDisplayName = m_xLbFilter->get_active_text();
        return reinterpret_cast<SvtFileDialogFilter_Impl*>(m_xLbFilter->get_active_id().toInt64());
    }

    // access to the current filter via methods only - need to care for consistency between m_pCurFilter and m_sCurrentFilterDisplayName
    const SvtFileDialogFilter_Impl* GetCurFilter( ) const
    {
        return m_pCurFilter;
    }

    const OUString& GetCurFilterDisplayName() const
    {
        return m_sCurrentFilterDisplayName;
    }

    void SetCurFilter( SvtFileDialogFilter_Impl const * _pFilter, const OUString& rDisplayName );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
