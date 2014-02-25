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
#include <fldui.hrc>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Description: edit insert-field
 --------------------------------------------------------------------*/
sw::DropDownFieldDialog::DropDownFieldDialog(Window *pParent, SwWrtShell &rS,
                              SwField* pField, sal_Bool bNextButton)
    : SvxStandardDialog(pParent, "DropdownFieldDialog",
        "modules/swriter/ui/dropdownfielddialog.ui")
    , rSh( rS )
    , pDropField(0)
{
    get(m_pListItemsLB, "list");
    m_pListItemsLB->SetDropDownLineCount(12);
    m_pListItemsLB->set_width_request(m_pListItemsLB->approximate_char_width()*32);
    get(m_pOKPB, "ok");
    get(m_pNextPB, "next");
    get(m_pEditPB, "edit");

    Link aButtonLk = LINK(this, DropDownFieldDialog, ButtonHdl);
    m_pEditPB->SetClickHdl(aButtonLk);
    if( bNextButton )
    {
        m_pNextPB->Show();
        m_pNextPB->SetClickHdl(aButtonLk);
    }
    if( RES_DROPDOWN == pField->GetTyp()->Which() )
    {

        pDropField = (SwDropDownField*)pField;
        OUString sTitle = GetText();
        sTitle += pDropField->GetPar2();
        SetText(sTitle);
        uno::Sequence< OUString > aItems = pDropField->GetItemSequence();
        const OUString* pArray = aItems.getConstArray();
        for(sal_Int32 i = 0; i < aItems.getLength(); i++)
            m_pListItemsLB->InsertEntry(pArray[i]);
        m_pListItemsLB->SelectEntry(pDropField->GetSelectedItem());
    }

    sal_Bool bEnable = !rSh.IsCrsrReadonly();
    m_pOKPB->Enable( bEnable );

    m_pListItemsLB->GrabFocus();
}

void sw::DropDownFieldDialog::Apply()
{
    if(pDropField)
    {
        OUString sSelect = m_pListItemsLB->GetSelectEntry();
        if(pDropField->GetPar1() != sSelect)
        {
            rSh.StartAllAction();

            boost::scoped_ptr<SwDropDownField> const pCopy(
                static_cast<SwDropDownField *>( pDropField->CopyField() ) );

            pCopy->SetPar1(sSelect);
            rSh.SwEditShell::UpdateFlds(*pCopy);

            rSh.SetUndoNoResetModified();
            rSh.EndAllAction();
        }
    }
}

IMPL_LINK(sw::DropDownFieldDialog, ButtonHdl, PushButton*, pButton)
{
    EndDialog(m_pNextPB == pButton ? RET_OK : RET_YES );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
