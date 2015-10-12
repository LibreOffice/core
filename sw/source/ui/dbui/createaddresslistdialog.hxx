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
    VclPtr<SwAddressControl_Impl>   m_pAddressControl;

    VclPtr<PushButton>              m_pNewPB;
    VclPtr<PushButton>              m_pDeletePB;
    VclPtr<PushButton>              m_pFindPB;
    VclPtr<PushButton>              m_pCustomizePB;

    VclPtr<PushButton>              m_pStartPB;
    VclPtr<PushButton>              m_pPrevPB;
    VclPtr<NumericField>            m_pSetNoNF;
    VclPtr<PushButton>              m_pNextPB;
    VclPtr<PushButton>              m_pEndPB;


    VclPtr<OKButton>                m_pOK;

    OUString                m_sAddressListFilterName;
    OUString                m_sURL;

    SwCSVData*              m_pCSVData;
    VclPtr<SwFindEntryDialog>      m_pFindDlg;

    DECL_LINK_TYPED(NewHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DeleteHdl_Impl, Button*, void);
    DECL_LINK_TYPED(FindHdl_Impl, Button*, void);
    DECL_LINK_TYPED(CustomizeHdl_Impl, Button*, void);
    DECL_LINK_TYPED(OkHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DBCursorHdl_Impl, Button*, void);
    DECL_LINK(DBNumCursorHdl_Impl, void *);

    void UpdateButtons();

public:
    SwCreateAddressListDialog(
            vcl::Window* pParent, const OUString& rURL, SwMailMergeConfigItem& rConfig);
    virtual ~SwCreateAddressListDialog();
    virtual void dispose() override;

    const OUString&         GetURL() const {    return m_sURL;    }
    void                    Find( const OUString& rSearch, sal_Int32 nColumn);
};

class SwFindEntryDialog : public ModelessDialog
{
    VclPtr<Edit>         m_pFindED;
    VclPtr<CheckBox>     m_pFindOnlyCB;
    VclPtr<ListBox>      m_pFindOnlyLB;

    VclPtr<PushButton>   m_pFindPB;
    VclPtr<CancelButton> m_pCancel;

    VclPtr<SwCreateAddressListDialog>  m_pParent;

    DECL_LINK_TYPED(FindHdl_Impl, Button*, void);
    DECL_LINK(FindEnableHdl_Impl, void *);
    DECL_LINK_TYPED(CloseHdl_Impl, Button*, void);

public:
    SwFindEntryDialog(SwCreateAddressListDialog* pParent);
    virtual ~SwFindEntryDialog();
    virtual void dispose() override;

    ListBox& GetFieldsListBox()
    {
        return *m_pFindOnlyLB;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
