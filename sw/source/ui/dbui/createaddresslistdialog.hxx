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
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vector>

class SwAddressControl_Impl;
class SwMailMergeConfigItem;

// container of the created database
struct SwCSVData
{
    ::std::vector< OUString >                    aDBColumnHeaders;
    ::std::vector< ::std::vector< OUString> >     aDBData;
};

class SwFindEntryDialog;
class SwCreateAddressListDialog : public SfxModalDialog
{
    SwAddressControl_Impl*  m_pAddressControl;

    PushButton              *m_pNewPB;
    PushButton              *m_pDeletePB;
    PushButton              *m_pFindPB;
    PushButton              *m_pCustomizePB;

    PushButton              *m_pStartPB;
    PushButton              *m_pPrevPB;
    NumericField            *m_pSetNoNF;
    PushButton              *m_pNextPB;
    PushButton              *m_pEndPB;


    OKButton                *m_pOK;

    OUString                m_sAddressListFilterName;
    OUString                m_sURL;

    SwCSVData*              m_pCSVData;
    SwFindEntryDialog*      m_pFindDlg;

    DECL_LINK(NewHdl_Impl, void *);
    DECL_LINK(DeleteHdl_Impl, void *);
    DECL_LINK(FindHdl_Impl, void *);
    DECL_LINK(CustomizeHdl_Impl, PushButton*);
    DECL_LINK(OkHdl_Impl, void *);
    DECL_LINK(DBCursorHdl_Impl, PushButton*);
    DECL_LINK(DBNumCursorHdl_Impl, void *);

    void UpdateButtons();

public:
    SwCreateAddressListDialog(
            vcl::Window* pParent, const OUString& rURL, SwMailMergeConfigItem& rConfig);
    virtual ~SwCreateAddressListDialog();
    virtual void dispose() SAL_OVERRIDE;

    const OUString&         GetURL() const {    return m_sURL;    }
    void                    Find( const OUString& rSearch, sal_Int32 nColumn);
};

class SwFindEntryDialog : public ModelessDialog
{
    Edit*         m_pFindED;
    CheckBox*     m_pFindOnlyCB;
    ListBox*      m_pFindOnlyLB;

    PushButton*   m_pFindPB;
    CancelButton* m_pCancel;

    SwCreateAddressListDialog*  m_pParent;

    DECL_LINK(FindHdl_Impl, void *);
    DECL_LINK(FindEnableHdl_Impl, void *);
    DECL_LINK(CloseHdl_Impl, void *);

public:
    SwFindEntryDialog(SwCreateAddressListDialog* pParent);

    ListBox& GetFieldsListBox()
    {
        return *m_pFindOnlyLB;
    }
    OUString GetFindString() const
    {
        return m_pFindED->GetText();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
