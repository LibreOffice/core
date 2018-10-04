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

    std::unique_ptr<SwCSVData> m_pNewData;

    DECL_LINK(AddRenameHdl_Impl, Button*, void);
    DECL_LINK(DeleteHdl_Impl, Button*, void);
    DECL_LINK(UpDownHdl_Impl, Button*, void);
    DECL_LINK(ListBoxSelectHdl_Impl, ListBox&, void);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(vcl::Window* pParent, const SwCSVData& rOldData);
    virtual ~SwCustomizeAddressListDialog() override;
    virtual void dispose() override;

    std::unique_ptr<SwCSVData>  ReleaseNewData() { return std::move(m_pNewData);}
};

class SwAddRenameEntryDialog : public SfxDialogController
{
    const std::vector< OUString >& m_rCSVHeader;
    std::unique_ptr<weld::Entry> m_xFieldNameED;
    std::unique_ptr<weld::Button> m_xOK;

    DECL_LINK(ModifyHdl_Impl, weld::Entry&, void);
protected:
    SwAddRenameEntryDialog(weld::Window* pParent, const OUString& rUIXMLDescription,
        const OString& rID, const std::vector< OUString >& rCSVHeader);

public:
    void                SetFieldName(const OUString& rName) { m_xFieldNameED->set_text(rName); }
    OUString            GetFieldName() const { return m_xFieldNameED->get_text(); }

};

class SwAddEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwAddEntryDialog(weld::Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, "modules/swriter/ui/addentrydialog.ui",
                                 "AddEntryDialog", rCSVHeader)
    {
    }
};

class SwRenameEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwRenameEntryDialog(weld::Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, "modules/swriter/ui/renameentrydialog.ui",
                                 "RenameEntryDialog", rCSVHeader)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
