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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_CUSTOMIZEADDRESSLISTDIALOG_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_CUSTOMIZEADDRESSLISTDIALOG_HXX
#include <sfx2/basedlgs.hxx>

#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

struct SwCSVData;

class SwCustomizeAddressListDialog : public SfxModalDialog
{
    VclPtr<ListBox>     m_pFieldsLB;

    VclPtr<PushButton>  m_pAddPB;
    VclPtr<PushButton>  m_pDeletePB;
    VclPtr<PushButton>  m_pRenamePB;

    VclPtr<PushButton>  m_pUpPB;
    VclPtr<PushButton>  m_pDownPB;

    SwCSVData*   m_pNewData;

    DECL_LINK_TYPED(AddRenameHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DeleteHdl_Impl, Button*, void);
    DECL_LINK_TYPED(UpDownHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ListBoxSelectHdl_Impl, ListBox&, void);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(vcl::Window* pParent, const SwCSVData& rOldData);
    virtual ~SwCustomizeAddressListDialog();
    virtual void dispose() SAL_OVERRIDE;

    SwCSVData*    GetNewData() { return m_pNewData;}
};

class SwAddRenameEntryDialog : public SfxModalDialog
{
    VclPtr<Edit>     m_pFieldNameED;
    VclPtr<OKButton> m_pOK;
    const std::vector< OUString >& m_rCSVHeader;

    DECL_LINK(ModifyHdl_Impl, Edit*);
protected:
    SwAddRenameEntryDialog(vcl::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription, const std::vector< OUString >& rCSVHeader);
    virtual ~SwAddRenameEntryDialog();
    virtual void dispose() SAL_OVERRIDE;

public:
    void                SetFieldName(const OUString& rName) {m_pFieldNameED->SetText(rName);}
    OUString            GetFieldName() const {return m_pFieldNameED->GetText();};

};

class SwAddEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwAddEntryDialog(vcl::Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, "AddEntryDialog",
            "modules/swriter/ui/addentrydialog.ui", rCSVHeader)
    {
    }
};

class SwRenameEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwRenameEntryDialog(vcl::Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, "RenameEntryDialog",
            "modules/swriter/ui/renameentrydialog.ui", rCSVHeader)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
