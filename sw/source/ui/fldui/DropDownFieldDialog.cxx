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
#include <vcl/msgbox.hxx>
#include <DropDownFieldDialog.hxx>
#include <flddropdown.hxx>

#include <memory>

using namespace ::com::sun::star;

// edit insert-field
sw::DropDownFieldDialog::DropDownFieldDialog(vcl::Window *pParent, SwWrtShell &rS,
                              SwField* pField, bool bPrevButton, bool bNextButton)
    : SvxStandardDialog(pParent, "DropdownFieldDialog",
        "modules/swriter/ui/dropdownfielddialog.ui")
    , rSh( rS )
    , pDropField(nullptr)
    , m_pPressedButton(nullptr)
{
    get(m_pListItemsLB, "list");
    m_pListItemsLB->SetDropDownLineCount(12);
    m_pListItemsLB->set_width_request(m_pListItemsLB->approximate_char_width()*32);
    get(m_pOKPB, "ok");
    get(m_pPrevPB, "prev");
    get(m_pNextPB, "next");
    get(m_pEditPB, "edit");
    Link<ListBox&, void> aDoubleLk = LINK(this, DropDownFieldDialog, DoubleClickHdl);
    m_pListItemsLB->SetDoubleClickHdl( aDoubleLk );

    Link<Button*, void> aEditButtonLk = LINK(this, DropDownFieldDialog, EditHdl);
    Link<Button*,void> aPrevButtonLk = LINK(this, DropDownFieldDialog, PrevHdl);
    Link<Button*, void> aNextButtonLk = LINK(this, DropDownFieldDialog, NextHdl);
    m_pEditPB->SetClickHdl(aEditButtonLk);
    if( bPrevButton || bNextButton )
    {
        m_pPrevPB->Show();
        m_pPrevPB->SetClickHdl(aPrevButtonLk);
        m_pPrevPB->Enable(bPrevButton);

        m_pNextPB->Show();
        m_pNextPB->SetClickHdl(aNextButtonLk);
        m_pNextPB->Enable(bNextButton);
    }
    if( SwFieldIds::Dropdown == pField->GetTyp()->Which() )
    {

        pDropField = static_cast<SwDropDownField*>(pField);
        OUString sTitle = GetText();
        sTitle += pDropField->GetPar2();
        SetText(sTitle);
        uno::Sequence< OUString > aItems = pDropField->GetItemSequence();
        const OUString* pArray = aItems.getConstArray();
        for(sal_Int32 i = 0; i < aItems.getLength(); i++)
            m_pListItemsLB->InsertEntry(pArray[i]);
        m_pListItemsLB->SelectEntry(pDropField->GetSelectedItem());
    }

    bool bEnable = !rSh.IsCursorReadonly();
    m_pOKPB->Enable( bEnable );

    m_pListItemsLB->GrabFocus();
}

sw::DropDownFieldDialog::~DropDownFieldDialog()
{
    disposeOnce();
}

void sw::DropDownFieldDialog::dispose()
{
    m_pListItemsLB.clear();
    m_pOKPB.clear();
    m_pPrevPB.clear();
    m_pNextPB.clear();
    m_pEditPB.clear();
    m_pPressedButton.clear();
    SvxStandardDialog::dispose();
}

void sw::DropDownFieldDialog::Apply()
{
    if(pDropField)
    {
        OUString sSelect = m_pListItemsLB->GetSelectedEntry();
        if(pDropField->GetPar1() != sSelect)
        {
            rSh.StartAllAction();

            std::unique_ptr<SwDropDownField> const pCopy(
                static_cast<SwDropDownField *>( pDropField->CopyField() ) );

            pCopy->SetPar1(sSelect);
            rSh.SwEditShell::UpdateFields(*pCopy);

            rSh.SetUndoNoResetModified();
            rSh.EndAllAction();
        }
    }
}

bool sw::DropDownFieldDialog::PrevButtonPressed() const
{
    return m_pPressedButton == m_pPrevPB;
}

bool sw::DropDownFieldDialog::NextButtonPressed() const
{
    return m_pPressedButton == m_pNextPB;
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, EditHdl, Button*, void)
{
    m_pPressedButton = m_pEditPB;
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, PrevHdl, Button*, void)
{
    m_pPressedButton = m_pPrevPB;
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, NextHdl, Button*, void)
{
    m_pPressedButton = m_pNextPB;
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(sw::DropDownFieldDialog, DoubleClickHdl, ListBox&, void)
{
    EndDialog(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
