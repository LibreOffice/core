/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
/*-- 19.04.2004 12:09:46---------------------------------------------------
    container of the created database
  -----------------------------------------------------------------------*/
struct SwCSVData
{
    ::std::vector< ::rtl::OUString >                    aDBColumnHeaders;
    ::std::vector< ::std::vector< ::rtl::OUString> >     aDBData;
};
/*-- 08.04.2004 14:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
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

    DECL_LINK(NewHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(FindHdl_Impl, PushButton*);
    DECL_LINK(CustomizeHdl_Impl, PushButton*);
    DECL_LINK(OkHdl_Impl, PushButton*);
    DECL_LINK(DBCursorHdl_Impl, PushButton*);
    DECL_LINK(DBNumCursorHdl_Impl, NumericField*);

    void UpdateButtons();

public:
    SwCreateAddressListDialog(
            Window* pParent, const String& rURL, SwMailMergeConfigItem& rConfig);
    ~SwCreateAddressListDialog();

    const String&           GetURL() const {    return m_sURL;    }
    void                    Find( const String& rSearch, sal_Int32 nColumn);
};
/*-- 13.04.2004 13:30:21---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwFindEntryDialog : public ModelessDialog
{
    FixedText               m_aFindFT;
    Edit                    m_aFindED;
    CheckBox                m_aFindOnlyCB;
    ListBox                 m_aFindOnlyLB;

    PushButton              m_aFindPB;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    SwCreateAddressListDialog*  m_pParent;

    DECL_LINK(FindHdl_Impl, PushButton*);
    DECL_LINK(FindEnableHdl_Impl, Edit*);
    DECL_LINK(CloseHdl_Impl, PushButton*);

public:
    SwFindEntryDialog(SwCreateAddressListDialog* pParent);
    ~SwFindEntryDialog();

    ListBox&                GetFieldsListBox(){return m_aFindOnlyLB;}
    String                  GetFindString() const {return m_aFindED.GetText();}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
