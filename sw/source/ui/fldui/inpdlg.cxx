/*************************************************************************
 *
 *  $RCSfile: inpdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:28:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#define _INPDLG_CXX

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _INPDLG_HXX
#include <inpdlg.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif

#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _INPDLG_HRC
#include <inpdlg.hrc>
#endif


/*--------------------------------------------------------------------
    Beschreibung: Feldeinfuegen bearbeiten
 --------------------------------------------------------------------*/

SwFldInputDlg::SwFldInputDlg( Window *pParent, SwWrtShell &rS,
                              SwField* pField, BOOL bNextButton ) :

    SvxStandardDialog(pParent,  SW_RES(DLG_FLD_INPUT)),

    rSh( rS ),
    aLabelED    (this, SW_RES(ED_LABEL  )),
    aEditED     (this, SW_RES(ED_EDIT   )),
    aEditFL     (this, SW_RES(FL_EDIT       )),
    aOKBT       (this, SW_RES(BT_OK     )),
    aCancelBT   (this, SW_RES(BT_CANCEL )),
    aNextBT     (this, SW_RES(PB_NEXT   )),
    aHelpBT     (this, SW_RES(PB_HELP    )),
    pInpFld(0),
    pSetFld(0),
    pUsrType(0)
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
        USHORT nSubType = pInpFld->GetSubType();

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
            aStr = pSetFld->Expand();
        else
            aStr = sFormula;
        aLabelED.SetText( pSetFld->GetPromptText() );
    }

    // JP 31.3.00: Inputfields in readonly regions must be allowed to
    //              input any content. - 74639
    BOOL bEnable = !rSh.IsCrsrReadonly();
                    /*!rSh.IsReadOnlyAvailable() || !rSh.HasReadonlySel()*/;
    aOKBT.Enable( bEnable );
    aEditED.SetReadOnly( !bEnable );

    if( aStr.Len() )
        aEditED.SetText( aStr.ConvertLineEnd() );
    aEditED.GrabFocus();
    FreeResource();
}

SwFldInputDlg::~SwFldInputDlg()
{
}

/*--------------------------------------------------------------------
     Beschreibung:  Schliessen
 --------------------------------------------------------------------*/

void SwFldInputDlg::Apply()
{
    String aTmp( aEditED.GetText() );
    aTmp.EraseAllChars( '\r' );

    rSh.StartAllAction();
    BOOL bModified = FALSE;
    if(pInpFld)
    {
        if(pUsrType)
        {
            if( aTmp != pUsrType->GetContent() )
            {
                pUsrType->SetContent(aTmp);
                pUsrType->UpdateFlds();
                bModified = TRUE;
            }
        }
        else if( aTmp != pInpFld->GetPar1() )
        {
            pInpFld->SetPar1(aTmp);
            rSh.SwEditShell::UpdateFlds(*pInpFld);
            bModified = TRUE;
        }
    }
    else if( aTmp != pSetFld->GetPar2() )
    {
        pSetFld->SetPar2(aTmp);
        rSh.SwEditShell::UpdateFlds(*pSetFld);
        bModified = TRUE;
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


