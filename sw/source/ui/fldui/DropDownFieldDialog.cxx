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

#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <DropDownFieldDialog.hxx>
#include <flddropdown.hxx>

#include <memory>

using namespace ::com::sun::star;

// edit insert-field
sw::DropDownFieldDialog::DropDownFieldDialog(weld::Window *pParent, SwWrtShell &rS,
                              SwField* pField, bool bPrevButton, bool bNextButton)
    : GenericDialogController(pParent, "modules/swriter/ui/dropdownfielddialog.ui", "DropdownFieldDialog")
    , m_rSh( rS )
    , m_pDropField(nullptr)
    , m_pPressedButton(nullptr)
    , m_xListItemsLB(m_xBuilder->weld_tree_view("list"))
    , m_xOKPB(m_xBuilder->weld_button("ok"))
    , m_xPrevPB(m_xBuilder->weld_button("prev"))
    , m_xNextPB(m_xBuilder->weld_button("next"))
    , m_xEditPB(m_xBuilder->weld_button("edit"))
{
    m_xListItemsLB->set_size_request(m_xListItemsLB->get_approximate_digit_width() * 24,
                                     m_xListItemsLB->get_height_rows(12));
    Link<weld::TreeView&, void> aDoubleLk = LINK(this, DropDownFieldDialog, DoubleClickHdl);
    m_xListItemsLB->connect_row_activated( aDoubleLk );

    Link<weld::Button&, void> aEditButtonLk = LINK(this, DropDownFieldDialog, EditHdl);
    Link<weld::Button&,void> aPrevButtonLk = LINK(this, DropDownFieldDialog, PrevHdl);
    Link<weld::Button&, void> aNextButtonLk = LINK(this, DropDownFieldDialog, NextHdl);
    m_xEditPB->connect_clicked(aEditButtonLk);
    if( bPrevButton || bNextButton )
    {
        m_xPrevPB->show();
        m_xPrevPB->connect_clicked(aPrevButtonLk);
        m_xPrevPB->set_sensitive(bPrevButton);

        m_xNextPB->show();
        m_xNextPB->connect_clicked(aNextButtonLk);
        m_xNextPB->set_sensitive(bNextButton);
    }
    if( SwFieldIds::Dropdown == pField->GetTyp()->Which() )
    {

        m_pDropField = static_cast<SwDropDownField*>(pField);
        OUString sTitle = m_xDialog->get_title();
        sTitle += m_pDropField->GetPar2();
        m_xDialog->set_title(sTitle);
        uno::Sequence< OUString > aItems = m_pDropField->GetItemSequence();
        const OUString* pArray = aItems.getConstArray();
        for (sal_Int32 i = 0; i < aItems.getLength(); ++i)
            m_xListItemsLB->append_text(pArray[i]);
        m_xListItemsLB->select_text(m_pDropField->GetSelectedItem());
    }

    bool bEnable = !m_rSh.IsCursorReadonly();
    m_xOKPB->set_sensitive(bEnable);

    m_xListItemsLB->grab_focus();
}

sw::DropDownFieldDialog::~DropDownFieldDialog()
{
}

void sw::DropDownFieldDialog::Apply()
{
    if (m_pDropField)
    {
        OUString sSelect = m_xListItemsLB->get_selected_text();
        if (m_pDropField->GetPar1() != sSelect)
        {
            m_rSh.StartAllAction();

            std::unique_ptr<SwDropDownField> const pCopy(
                static_cast<SwDropDownField*>(m_pDropField->CopyField()));

            pCopy->SetPar1(sSelect);
            m_rSh.SwEditShell::UpdateOneField(*pCopy);

            m_rSh.SetUndoNoResetModified();
            m_rSh.EndAllAction();
        }
    }
}

bool sw::DropDownFieldDialog::PrevButtonPressed() const
{
    return m_pPressedButton == m_xPrevPB.get();
}

bool sw::DropDownFieldDialog::NextButtonPressed() const
{
    return m_pPressedButton == m_xNextPB.get();
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, EditHdl, weld::Button&, void)
{
    m_pPressedButton = m_xEditPB.get();
    m_xDialog->response(RET_YES);
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, PrevHdl, weld::Button&, void)
{
    m_pPressedButton = m_xPrevPB.get();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, NextHdl, weld::Button&, void)
{
    m_pPressedButton = m_xNextPB.get();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, DoubleClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
