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

#include <cui/dlgname.hxx>

#include <comphelper/string.hxx>

/*************************************************************************
|*
|* Dialog for editing a name
|*
\************************************************************************/

SvxNameDialog::SvxNameDialog(weld::Window* pParent, const OUString& rName, const OUString& rDesc,
                             const OUString& rTitle)
    : GenericDialogController(pParent, "cui/ui/namedialog.ui", "NameDialog")
    , m_xEdtName(m_xBuilder->weld_entry("name_entry"))
    , m_xFtDescription(m_xBuilder->weld_label("description_label"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    m_xFtDescription->set_label(rDesc);
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);
    ModifyHdl(*m_xEdtName);
    m_xEdtName->connect_changed(LINK(this, SvxNameDialog, ModifyHdl));
    if (!rTitle.isEmpty())
        set_title(rTitle);
}

IMPL_LINK_NOARG(SvxNameDialog, ModifyHdl, weld::Entry&, void)
{
    // Do not allow empty names, unless custom CheckNameHdl is specified
    bool bEnable;
    if (m_aCheckNameHdl.IsSet())
        bEnable = m_aCheckNameHdl.Call(*this);
    else
        bEnable = !m_xEdtName->get_text().isEmpty();
    m_xBtnOK->set_sensitive(bEnable);
    // tdf#129032: feedback on reason to disabled controls
    m_xEdtName->set_message_type(bEnable ? weld::EntryMessageType::Normal
                                         : weld::EntryMessageType::Error);
    OUString rTip = "";
    if (!bEnable && m_aCheckNameTooltipHdl.IsSet())
        rTip = m_aCheckNameTooltipHdl.Call(*this);
    m_xBtnOK->set_tooltip_text(rTip);
    m_xEdtName->set_tooltip_text(rTip);
}

SvxNumberDialog::SvxNumberDialog(weld::Window* pParent, const OUString& rDesc, sal_Int64 nValue,
                                 sal_Int64 nMin, sal_Int64 nMax)
    : GenericDialogController(pParent, "cui/ui/numberdialog.ui", "NumberDialog")
    , m_xEdtNumber(m_xBuilder->weld_spin_button("number_spinbtn"))
    , m_xFtDescription(m_xBuilder->weld_label("description_label"))
{
    m_xFtDescription->set_label(rDesc);
    m_xEdtNumber->set_min(nMin);
    m_xEdtNumber->set_max(nMax);
    m_xEdtNumber->set_value(nValue);
}

SvxDecimalNumberDialog::SvxDecimalNumberDialog(weld::Window* pParent, const OUString& rDesc,
                                               double fValue)
    : GenericDialogController(pParent, "cui/ui/numberdialog.ui", "NumberDialog")
    , m_xEdtNumber(m_xBuilder->weld_formatted_spin_button("number_spinbtn"))
    , m_xFtDescription(m_xBuilder->weld_label("description_label"))
{
    m_xFtDescription->set_label(rDesc);
    m_xEdtNumber->GetFormatter().SetValue(fValue);
}

// #i68101#
// Dialog for editing Object Name
// plus uniqueness-callback-linkHandler

SvxObjectNameDialog::SvxObjectNameDialog(weld::Window* pParent, const OUString& rName)
    : GenericDialogController(pParent, "cui/ui/objectnamedialog.ui", "ObjectNameDialog")
    , m_xEdtName(m_xBuilder->weld_entry("object_name_entry"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    // set name
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);

    // activate name
    ModifyHdl(*m_xEdtName);
    m_xEdtName->connect_changed(LINK(this, SvxObjectNameDialog, ModifyHdl));
}

IMPL_LINK_NOARG(SvxObjectNameDialog, ModifyHdl, weld::Entry&, void)
{
    if (aCheckNameHdl.IsSet())
    {
        m_xBtnOK->set_sensitive(aCheckNameHdl.Call(*this));
    }
}

// #i68101#
// Dialog for editing Object Title and Description

SvxObjectTitleDescDialog::SvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle,
                                                   const OUString& rDescription,
                                                   bool const isDecorative)
    : GenericDialogController(pParent, "cui/ui/objecttitledescdialog.ui", "ObjectTitleDescDialog")
    , m_xTitleFT(m_xBuilder->weld_label("object_title_label"))
    , m_xEdtTitle(m_xBuilder->weld_entry("object_title_entry"))
    , m_xDescriptionFT(m_xBuilder->weld_label("desc_label"))
    , m_xEdtDescription(m_xBuilder->weld_text_view("desc_entry"))
    , m_xDecorativeCB(m_xBuilder->weld_check_button("decorative"))
{
    //lock height to initial height
    m_xEdtDescription->set_size_request(-1, m_xEdtDescription->get_text_height() * 5);
    // set title & desc
    m_xEdtTitle->set_text(rTitle);
    m_xEdtDescription->set_text(rDescription);

    // activate title
    m_xEdtTitle->select_region(0, -1);

    m_xDecorativeCB->set_active(isDecorative);
    m_xDecorativeCB->connect_toggled(LINK(this, SvxObjectTitleDescDialog, DecorativeHdl));
    DecorativeHdl(*m_xDecorativeCB);
}

IMPL_LINK_NOARG(SvxObjectTitleDescDialog, DecorativeHdl, weld::Toggleable&, void)
{
    bool const bEnable(!m_xDecorativeCB->get_active());
    m_xEdtTitle->set_sensitive(bEnable);
    m_xTitleFT->set_sensitive(bEnable);
    m_xEdtDescription->set_sensitive(bEnable);
    m_xDescriptionFT->set_sensitive(bEnable);
}

SvxListDialog::SvxListDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/listdialog.ui", "ListDialog")
    , m_xList(m_xBuilder->weld_tree_view("assignlist"))
    , m_xAddBtn(m_xBuilder->weld_button("addbtn"))
    , m_xRemoveBtn(m_xBuilder->weld_button("removebtn"))
    , m_xEditBtn(m_xBuilder->weld_button("editbtn"))
{
    m_xList->set_size_request(m_xList->get_approximate_digit_width() * 54,
                              m_xList->get_height_rows(6));
    m_xAddBtn->connect_clicked(LINK(this, SvxListDialog, AddHdl_Impl));
    m_xRemoveBtn->connect_clicked(LINK(this, SvxListDialog, RemoveHdl_Impl));
    m_xEditBtn->connect_clicked(LINK(this, SvxListDialog, EditHdl_Impl));
    m_xList->connect_changed(LINK(this, SvxListDialog, SelectHdl_Impl));
    m_xList->connect_row_activated(LINK(this, SvxListDialog, DblClickHdl_Impl));

    SelectionChanged();
}

SvxListDialog::~SvxListDialog() {}

IMPL_LINK_NOARG(SvxListDialog, AddHdl_Impl, weld::Button&, void)
{
    SvxNameDialog aNameDlg(m_xDialog.get(), "", "blabla");

    if (!aNameDlg.run())
        return;
    OUString sNewText = comphelper::string::strip(aNameDlg.GetName(), ' ');
    if (!sNewText.isEmpty())
    {
        m_xList->insert_text(-1, sNewText);
        m_xList->select(-1);
    }
}

IMPL_LINK_NOARG(SvxListDialog, EditHdl_Impl, weld::Button&, void) { EditEntry(); }

IMPL_LINK_NOARG(SvxListDialog, SelectHdl_Impl, weld::TreeView&, void) { SelectionChanged(); }

IMPL_LINK_NOARG(SvxListDialog, DblClickHdl_Impl, weld::TreeView&, bool)
{
    EditEntry();
    return true;
}

IMPL_LINK_NOARG(SvxListDialog, RemoveHdl_Impl, weld::Button&, void)
{
    int nPos = m_xList->get_selected_index();
    if (nPos == -1)
        return;
    m_xList->remove(nPos);
    int nCount = m_xList->n_children();
    if (nCount)
    {
        if (nPos >= nCount)
            nPos = nCount - 1;
        m_xList->select(nPos);
    }
    SelectionChanged();
}

void SvxListDialog::SelectionChanged()
{
    bool bEnable = m_xList->get_selected_index() != -1;
    m_xRemoveBtn->set_sensitive(bEnable);
    m_xEditBtn->set_sensitive(bEnable);
}

std::vector<OUString> SvxListDialog::GetEntries() const
{
    int nCount = m_xList->n_children();
    std::vector<OUString> aList;
    aList.reserve(nCount);
    for (int i = 0; i < nCount; ++i)
        aList.push_back(m_xList->get_text(i));
    return aList;
}

void SvxListDialog::SetEntries(std::vector<OUString> const& rEntries)
{
    m_xList->clear();
    for (auto const& sEntry : rEntries)
    {
        m_xList->append_text(sEntry);
    }
    SelectionChanged();
}

void SvxListDialog::EditEntry()
{
    int nPos = m_xList->get_selected_index();
    if (nPos == -1)
        return;

    OUString sOldText(m_xList->get_selected_text());
    SvxNameDialog aNameDlg(m_xDialog.get(), sOldText, "blabla");

    if (!aNameDlg.run())
        return;
    OUString sNewText = comphelper::string::strip(aNameDlg.GetName(), ' ');
    if (!sNewText.isEmpty() && sNewText != sOldText)
    {
        m_xList->remove(nPos);
        m_xList->insert_text(nPos, sNewText);
        m_xList->select(nPos);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
