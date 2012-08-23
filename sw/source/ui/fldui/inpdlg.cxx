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

#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <inpdlg.hxx>
#include <fldmgr.hxx>

#include <fldui.hrc>
#include <inpdlg.hrc>


/*--------------------------------------------------------------------
    Description: edit field-insert
 --------------------------------------------------------------------*/

SwFldInputDlg::SwFldInputDlg( Window *pParent, SwWrtShell &rS,
                              SwField* pField, sal_Bool bNextButton ) :

    SvxStandardDialog(pParent,  SW_RES(DLG_FLD_INPUT)),

    rSh( rS ),
    pInpFld(0),
    pSetFld(0),
    pUsrType(0),

    aLabelED    (this, SW_RES(ED_LABEL  )),
    aEditED     (this, SW_RES(ED_EDIT   )),
    aEditFL     (this, SW_RES(FL_EDIT       )),

    aOKBT       (this, SW_RES(BT_OK     )),
    aCancelBT   (this, SW_RES(BT_CANCEL )),
    aNextBT     (this, SW_RES(PB_NEXT   )),
    aHelpBT     (this, SW_RES(PB_HELP    ))
{
    // switch font for Edit
    Font aFont(aEditED.GetFont());
    aFont.SetWeight(WEIGHT_LIGHT);
    aEditED.SetFont(aFont);

    if( bNextButton )
    {
        aNextBT.Show();
        aNextBT.SetClickHdl(LINK(this, SwFldInputDlg, NextHdl));
    }
    else
    {
        long nDiff = aCancelBT.GetPosPixel().Y() - aOKBT.GetPosPixel().Y();
        Point aPos = aHelpBT.GetPosPixel();
        aPos.Y() -= nDiff;
        aHelpBT.SetPosPixel(aPos);
    }

    // evaluation here
    String aStr;
    if( RES_INPUTFLD == pField->GetTyp()->Which() )
    {   // it is an input field
        //
        pInpFld = (SwInputField*)pField;
        aLabelED.SetText( pInpFld->GetPar2() );
        sal_uInt16 nSubType = pInpFld->GetSubType();

        switch(nSubType & 0xff)
        {
            case INP_TXT:
                aStr = pInpFld->GetPar1();
                break;

            case INP_USR:
                // user field
                if( 0 != ( pUsrType = (SwUserFieldType*)rSh.GetFldType(
                            RES_USERFLD, pInpFld->GetPar1() ) ) )
                    aStr = pUsrType->GetContent();
                break;
        }
    }
    else
    {
        // it is a SetExpression
        pSetFld = (SwSetExpField*)pField;
        String sFormula(pSetFld->GetFormula());
        //values are formatted - formulas are not
        CharClass aCC( SvxCreateLocale( pSetFld->GetLanguage() ));
        if( aCC.isNumeric( sFormula ))
        {
            aStr = pSetFld->ExpandField(true);
        }
        else
            aStr = sFormula;
        aLabelED.SetText( pSetFld->GetPromptText() );
    }

    // JP 31.3.00: Inputfields in readonly regions must be allowed to
    //              input any content. - 74639
    sal_Bool bEnable = !rSh.IsCrsrReadonly();

    aOKBT.Enable( bEnable );
    aEditED.SetReadOnly( !bEnable );

    if( aStr.Len() )
        aEditED.SetText(convertLineEnd(aStr, GetSystemLineEnd()));
    FreeResource();
}

SwFldInputDlg::~SwFldInputDlg()
{
}

void SwFldInputDlg::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
        aEditED.GrabFocus();
    SvxStandardDialog::StateChanged( nType );
}

/*--------------------------------------------------------------------
     Description:   Close
 --------------------------------------------------------------------*/

void SwFldInputDlg::Apply()
{
    rtl::OUString aTmp(comphelper::string::remove(aEditED.GetText(), '\r'));

    rSh.StartAllAction();
    sal_Bool bModified = sal_False;
    if(pInpFld)
    {
        if(pUsrType)
        {
            if( !aTmp.equals(pUsrType->GetContent()) )
            {
                pUsrType->SetContent(aTmp);
                pUsrType->UpdateFlds();
                bModified = sal_True;
            }
        }
        else if( !aTmp.equals(pInpFld->GetPar1()) )
        {
            pInpFld->SetPar1(aTmp);
            rSh.SwEditShell::UpdateFlds(*pInpFld);
            bModified = sal_True;
        }
    }
    else if( !aTmp.equals(pSetFld->GetPar2()) )
    {
        pSetFld->SetPar2(aTmp);
        rSh.SwEditShell::UpdateFlds(*pSetFld);
        bModified = sal_True;
    }

    if( bModified )
        rSh.SetUndoNoResetModified();

    rSh.EndAllAction();
}


IMPL_LINK_NOARG(SwFldInputDlg, NextHdl)
{
    EndDialog(RET_OK);
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
