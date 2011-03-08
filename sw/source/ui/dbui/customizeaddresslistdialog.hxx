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
#ifndef _CUSTOMIZEADDRESSLISTDIALOG_HXX
#define _CUSTOMIZEADDRESSLISTDIALOG_HXX
#include <sfx2/basedlgs.hxx>

#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

struct SwCSVData;
/*-- 08.04.2004 14:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwCustomizeAddressListDialog : public SfxModalDialog
{
    FixedText               m_aFieldsFT;
    ListBox                 m_aFieldsLB;

    PushButton              m_aAddPB;
    PushButton              m_aDeletePB;
    PushButton              m_aRenamePB;

    ImageButton             m_aUpPB;
    ImageButton             m_aDownPB;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    SwCSVData*              m_pNewData;

    DECL_LINK(AddRenameHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(UpDownHdl_Impl, PushButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, ListBox*);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(Window* pParent, const SwCSVData& rOldData);
    ~SwCustomizeAddressListDialog();

    SwCSVData*    GetNewData();
};
/*-- 13.04.2004 13:30:21---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwAddRenameEntryDialog : public SfxModalDialog
{
    FixedText               m_aFieldNameFT;
    Edit                    m_aFieldNameED;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    const ::std::vector< ::rtl::OUString >& m_rCSVHeader;

    DECL_LINK(ModifyHdl_Impl, Edit*);
public:
    SwAddRenameEntryDialog(Window* pParent, bool bRename, const ::std::vector< ::rtl::OUString >& aCSVHeader);
    ~SwAddRenameEntryDialog();

    void                SetFieldName(const String& rName) {m_aFieldNameED.SetText(rName);}
    String              GetFieldName() const {return m_aFieldNameED.GetText();};

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
