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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_CREATEADDRESSLISTDIALOG_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_CREATEADDRESSLISTDIALOG_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/weld.hxx>
#include <vector>

class SwAddressControl_Impl;
class SwMailMergeConfigItem;

// container of the created database
struct SwCSVData
{
    std::vector< OUString >                      aDBColumnHeaders;
    std::vector< std::vector< OUString> >         aDBData;
};

class SwFindEntryDialog;
class SwCreateAddressListDialog : public SfxDialogController
{
    OUString const          m_sAddressListFilterName;
    OUString                m_sURL;

    std::unique_ptr<SwCSVData>     m_pCSVData;
    std::unique_ptr<SwFindEntryDialog> m_xFindDlg;

    std::unique_ptr<SwAddressControl_Impl> m_xAddressControl;
    std::unique_ptr<weld::Button> m_xNewPB;
    std::unique_ptr<weld::Button> m_xDeletePB;
    std::unique_ptr<weld::Button> m_xFindPB;
    std::unique_ptr<weld::Button> m_xCustomizePB;
    std::unique_ptr<weld::Button> m_xStartPB;
    std::unique_ptr<weld::Button> m_xPrevPB;
    std::unique_ptr<weld::Entry> m_xSetNoED;
    std::unique_ptr<weld::SpinButton> m_xSetNoNF;
    std::unique_ptr<weld::Button> m_xNextPB;
    std::unique_ptr<weld::Button> m_xEndPB;
    std::unique_ptr<weld::Button> m_xOK;

    DECL_LINK(NewHdl_Impl, weld::Button&, void);
    DECL_LINK(DeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(FindHdl_Impl, weld::Button&, void);
    DECL_LINK(CustomizeHdl_Impl, weld::Button&, void);
    DECL_LINK(OkHdl_Impl, weld::Button&, void);
    DECL_LINK(DBCursorHdl_Impl, weld::Button&, void);
    DECL_LINK(DBNumCursorHdl_Impl, weld::Entry&, void);
    DECL_LINK(RefreshNum_Impl, weld::Widget&, void);
    void DBNumCursor();

    void UpdateButtons();

public:
    SwCreateAddressListDialog(weld::Window* pParent, const OUString& rURL, SwMailMergeConfigItem const & rConfig);
    virtual ~SwCreateAddressListDialog() override;

    const OUString&         GetURL() const { return m_sURL; }
    void                    Find( const OUString& rSearch, sal_Int32 nColumn);
};

class SwFindEntryDialog : public weld::GenericDialogController
{
    SwCreateAddressListDialog* m_pParent;

    std::unique_ptr<weld::Entry> m_xFindED;
    std::unique_ptr<weld::CheckButton> m_xFindOnlyCB;
    std::unique_ptr<weld::ComboBox> m_xFindOnlyLB;
    std::unique_ptr<weld::Button> m_xFindPB;
    std::unique_ptr<weld::Button> m_xCancel;

    DECL_LINK(FindHdl_Impl, weld::Button&, void);
    DECL_LINK(FindEnableHdl_Impl, weld::Entry&, void);
    DECL_LINK(CloseHdl_Impl, weld::Button&, void);

public:
    SwFindEntryDialog(SwCreateAddressListDialog* pParent);
    virtual ~SwFindEntryDialog() override;

    void show() { m_xDialog->show(); }
    void set_visible(bool bVisible) { m_xDialog->set_visible(bVisible); }
    void hide() { m_xDialog->hide(); }
    bool get_visible() const { return m_xDialog->get_visible(); }

    weld::ComboBox& GetFieldsListBox()
    {
        return *m_xFindOnlyLB;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
