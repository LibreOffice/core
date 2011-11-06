/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

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
    Beschreibung: Feldeinfuegen bearbeiten
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
    // Font fuers Edit umschalten
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

    // Auswertung hier
    String aStr;
    if( RES_INPUTFLD == pField->GetTyp()->Which() )
    {   // Es ist eine Eingabefeld
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
                // Benutzerfeld
                if( 0 != ( pUsrType = (SwUserFieldType*)rSh.GetFldType(
                            RES_USERFLD, pInpFld->GetPar1() ) ) )
                    aStr = pUsrType->GetContent();
                break;
        }
    }
    else
    {
        // es ist eine SetExpression
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
                    /*!rSh.IsReadOnlyAvailable() || !rSh.HasReadonlySel()*/;
    aOKBT.Enable( bEnable );
    aEditED.SetReadOnly( !bEnable );

    if( aStr.Len() )
        aEditED.SetText( aStr.ConvertLineEnd() );
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
     Beschreibung:  Schliessen
 --------------------------------------------------------------------*/

void SwFldInputDlg::Apply()
{
    String aTmp( aEditED.GetText() );
    aTmp.EraseAllChars( '\r' );

    rSh.StartAllAction();
    sal_Bool bModified = sal_False;
    if(pInpFld)
    {
        if(pUsrType)
        {
            if( aTmp != pUsrType->GetContent() )
            {
                pUsrType->SetContent(aTmp);
                pUsrType->UpdateFlds();
                bModified = sal_True;
            }
        }
        else if( aTmp != pInpFld->GetPar1() )
        {
            pInpFld->SetPar1(aTmp);
            rSh.SwEditShell::UpdateFlds(*pInpFld);
            bModified = sal_True;
        }
    }
    else if( aTmp != pSetFld->GetPar2() )
    {
        pSetFld->SetPar2(aTmp);
        rSh.SwEditShell::UpdateFlds(*pSetFld);
        bModified = sal_True;
    }

    if( bModified )
        rSh.SetUndoNoResetModified();

    rSh.EndAllAction();
}


IMPL_LINK(SwFldInputDlg, NextHdl, PushButton*, EMPTYARG)
{
    EndDialog(RET_OK);
    return 0;
}


