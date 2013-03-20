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
#if 1
#include <sfx2/basedlgs.hxx>

#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

struct SwCSVData;

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
    DECL_LINK(DeleteHdl_Impl, void *);
    DECL_LINK(UpDownHdl_Impl, PushButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, void *);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(Window* pParent, const SwCSVData& rOldData);
    ~SwCustomizeAddressListDialog();

    SwCSVData*    GetNewData();
};

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
