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
#ifndef _CREATEADDRESSLISTDIALOG_HXX
#define _CREATEADDRESSLISTDIALOG_HXX

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
    FixedInfo               m_aAddressInformation;
    SwAddressControl_Impl*  m_pAddressControl;

    PushButton              m_aNewPB;
    PushButton              m_aDeletePB;
    PushButton              m_aFindPB;
    PushButton              m_aCustomizePB;

    FixedInfo               m_aViewEntriesFI;
    PushButton              m_aStartPB;
    PushButton              m_aPrevPB;
    NumericField            m_aSetNoNF;
    PushButton              m_aNextPB;
    PushButton              m_aEndPB;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    String                  m_sAddressListFilterName;
    String                  m_sURL;

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
            Window* pParent, const String& rURL, SwMailMergeConfigItem& rConfig);
    ~SwCreateAddressListDialog();

    const String&           GetURL() const {    return m_sURL;    }
    void                    Find( const String& rSearch, sal_Int32 nColumn);
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
