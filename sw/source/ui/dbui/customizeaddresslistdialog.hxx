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
#include <vcl/weld.hxx>

#include "createaddresslistdialog.hxx"

struct SwCSVData;

class SwCustomizeAddressListDialog : public SfxDialogController
{
    std::unique_ptr<SwCSVData> m_xNewData;
    std::unique_ptr<weld::TreeView> m_xFieldsLB;
    std::unique_ptr<weld::Button> m_xAddPB;
    std::unique_ptr<weld::Button> m_xDeletePB;
    std::unique_ptr<weld::Button> m_xRenamePB;
    std::unique_ptr<weld::Button> m_xUpPB;
    std::unique_ptr<weld::Button> m_xDownPB;

    DECL_LINK(AddRenameHdl_Impl, weld::Button&, void);
    DECL_LINK(DeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(UpDownHdl_Impl, weld::Button&, void);
    DECL_LINK(ListBoxSelectHdl_Impl, weld::TreeView&, void);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(weld::Window* pParent, const SwCSVData& rOldData);
    virtual ~SwCustomizeAddressListDialog() override;

    std::unique_ptr<SwCSVData>  ReleaseNewData() { return std::move(m_xNewData);}
};

class SwAddRenameEntryDialog : public SfxDialogController
{
    const std::vector< OUString >& m_rCSVHeader;
    std::unique_ptr<weld::Entry> m_xFieldNameED;
    std::unique_ptr<weld::Button> m_xOK;

    DECL_LINK(ModifyHdl_Impl, weld::Entry&, void);
protected:
    SwAddRenameEntryDialog(weld::Window* pParent, const OUString& rUIXMLDescription,
        const OUString& rID, const std::vector< OUString >& rCSVHeader);

public:
    void                SetFieldName(const OUString& rName) { m_xFieldNameED->set_text(rName); }
    OUString            GetFieldName() const { return m_xFieldNameED->get_text(); }

};

class SwAddEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwAddEntryDialog(weld::Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, u"modules/swriter/ui/addentrydialog.ui"_ustr,
                                 u"AddEntryDialog"_ustr, rCSVHeader)
    {
    }
};

class SwRenameEntryDialog : public SwAddRenameEntryDialog
{
public:
    SwRenameEntryDialog(weld::Window* pParent, const std::vector< OUString >& rCSVHeader)
        : SwAddRenameEntryDialog(pParent, u"modules/swriter/ui/renameentrydialog.ui"_ustr,
                                 u"RenameEntryDialog"_ustr, rCSVHeader)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
