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
        CharClass aCC( LanguageTag( pSetFld->GetLanguage() ));
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
    OUString aTmp(comphelper::string::remove(aEditED.GetText(), '\r'));

    rSh.StartAllAction();
    bool bModified = false;
    if(pInpFld)
    {
        if(pUsrType)
        {
            if( !aTmp.equals(pUsrType->GetContent()) )
            {
                pUsrType->SetContent(aTmp);
                pUsrType->UpdateFlds();
                bModified = true;
            }
        }
        else if( !aTmp.equals(pInpFld->GetPar1()) )
        {
            pInpFld->SetPar1(aTmp);
            rSh.SwEditShell::UpdateFlds(*pInpFld);
            bModified = true;
        }
    }
    else if( !aTmp.equals(pSetFld->GetPar2()) )
    {
        pSetFld->SetPar2(aTmp);
        rSh.SwEditShell::UpdateFlds(*pSetFld);
        bModified = true;
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
