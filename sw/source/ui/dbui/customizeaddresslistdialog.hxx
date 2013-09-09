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
#ifndef _CUSTOMIZEADDRESSLISTDIALOG_HXX
#define _CUSTOMIZEADDRESSLISTDIALOG_HXX
#include <sfx2/basedlgs.hxx>

#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

struct SwCSVData;

class SwCustomizeAddressListDialog : public SfxModalDialog
{
    ListBox*     m_pFieldsLB;

    PushButton*  m_pAddPB;
    PushButton*  m_pDeletePB;
    PushButton*  m_pRenamePB;

    PushButton*  m_pUpPB;
    PushButton*  m_pDownPB;

    SwCSVData*   m_pNewData;

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
    Edit*     m_pFieldNameED;
    OKButton* m_pOK;
    const std::vector< OUString >& m_rCSVHeader;

    DECL_LINK(ModifyHdl_Impl, Edit*);
protected:
    SwAddRenameEntryDialog(Window* pParent, const OString& rID,
        const OUString& rUIXMLDescription, const std::vector< OUString >& rCSVHeader);
public:
    void                SetFieldName(const OUString& rName) {m_pFieldNameED->SetText(rName);}
    OUString            GetFieldName() const {return m_pFieldNameED->GetText();};

};

class SwAddEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwAddEntryDialog(Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, "AddEntryDialog",
            "modules/swriter/ui/addentrydialog.ui", rCSVHeader)
    {
    }
};

class SwRenameEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwRenameEntryDialog(Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, "RenameEntryDialog",
            "modules/swriter/ui/renameentrydialog.ui", rCSVHeader)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
