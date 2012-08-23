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

#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <vcl/msgbox.hxx>
#include <DropDownFieldDialog.hxx>
#include <flddropdown.hxx>
#include <fldui.hrc>
#include <DropDownFieldDialog.hrc>

using namespace ::com::sun::star;


/*--------------------------------------------------------------------
    Description: edit insert-field
 --------------------------------------------------------------------*/
sw::DropDownFieldDialog::DropDownFieldDialog( Window *pParent, SwWrtShell &rS,
                              SwField* pField, sal_Bool bNextButton ) :

    SvxStandardDialog(pParent,  SW_RES(DLG_FLD_DROPDOWN)),
    aItemsFL(       this, SW_RES( FL_ITEMS       )),
    aListItemsLB(   this, SW_RES( LB_LISTITEMS   )),

    aOKPB(          this, SW_RES( PB_OK          )),
    aCancelPB(      this, SW_RES( PB_CANCEL      )),
    aNextPB(        this, SW_RES( PB_NEXT        )),
    aHelpPB(        this, SW_RES( PB_HELP        )),

    aEditPB(        this, SW_RES( PB_EDIT        )),

    rSh( rS ),
    pDropField(0)
{
    Link aButtonLk = LINK(this, DropDownFieldDialog, ButtonHdl);
    aEditPB.SetClickHdl(aButtonLk);
    if( bNextButton )
    {
        aNextPB.Show();
        aNextPB.SetClickHdl(aButtonLk);
    }
    else
    {
        long nDiff = aCancelPB.GetPosPixel().Y() - aOKPB.GetPosPixel().Y();
        Point aPos = aHelpPB.GetPosPixel();
        aPos.Y() -= nDiff;
        aHelpPB.SetPosPixel(aPos);
    }
    if( RES_DROPDOWN == pField->GetTyp()->Which() )
    {
        //
        pDropField = (SwDropDownField*)pField;
        String sTitle = GetText();
        sTitle += pDropField->GetPar2();
        SetText(sTitle);
        uno::Sequence< rtl::OUString > aItems = pDropField->GetItemSequence();
        const rtl::OUString* pArray = aItems.getConstArray();
        for(sal_Int32 i = 0; i < aItems.getLength(); i++)
            aListItemsLB.InsertEntry(pArray[i]);
        aListItemsLB.SelectEntry(pDropField->GetSelectedItem());
    }

    sal_Bool bEnable = !rSh.IsCrsrReadonly();
    aOKPB.Enable( bEnable );

    aListItemsLB.GrabFocus();
    FreeResource();
}

sw::DropDownFieldDialog::~DropDownFieldDialog()
{
}

void sw::DropDownFieldDialog::Apply()
{
    if(pDropField)
    {
        rtl::OUString sSelect = aListItemsLB.GetSelectEntry();
        if(pDropField->GetPar1() != sSelect)
        {
            rSh.StartAllAction();

            ::std::auto_ptr<SwDropDownField> const pCopy(
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
    EndDialog(&aNextPB == pButton ? RET_OK : RET_YES );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
