/*************************************************************************
 *
 *  $RCSfile: inpdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-24 11:38:19 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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
    aEditGB     (this, SW_RES(GB_EDIT       )),
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

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:37  hr
      initial import

      Revision 1.48  2000/09/18 16:05:30  willem.vandorp
      OpenOffice header added.

      Revision 1.47  2000/08/21 09:11:38  os
      Set expression input fields: show formula or Expand() depending on the formula content

      Revision 1.46  2000/03/31 08:00:44  jp
      Bug #74639#: allow changes on InputFields in readonly regions

      Revision 1.45  2000/02/11 14:46:53  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.44  1999/01/20 15:56:58  JP
      Task #58677#: Crsr in Readonly Bereichen zulassen


      Rev 1.43   20 Jan 1999 16:56:58   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.42   17 Dec 1997 17:58:46   OM
   Datumsformate fuer SetExpFields

      Rev 1.41   03 Nov 1997 13:18:10   MA
   precomp entfernt

      Rev 1.40   02 Oct 1997 15:21:44   OM
   Feldumstellung

      Rev 1.39   23 Jul 1997 20:25:54   HJS
   includes

      Rev 1.38   11 Nov 1996 09:56:08   MA
   ResMgr

      Rev 1.37   24 Oct 1996 13:36:22   JP
   String Umstellung: [] -> GetChar()

      Rev 1.36   30 Aug 1996 12:43:16   OS
   InputFldDlg mit Next-Button

      Rev 1.35   28 Aug 1996 12:10:58   OS
   includes

      Rev 1.34   01 Aug 1996 10:59:16   AMA
   New: Eingabe-Felder mit Zeilenumbruechen

      Rev 1.33   04 Jun 1996 18:41:52   JP
   bei GetFldType wurden die Parameter getauscht

      Rev 1.32   02 Apr 1996 14:32:34   OS
   *_cxx - define hinter hdrstop

      Rev 1.31   24 Nov 1995 16:57:42   OM
   PCH->PRECOMPILED

      Rev 1.30   08 Nov 1995 13:45:34   OM
   Change->Set

      Rev 1.29   30 Aug 1995 13:53:30   MA
   fix: sexport'iert

      Rev 1.28   21 Aug 1995 09:24:36   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.27   18 Jan 1995 18:54:20   ER
   fld.hxx -> *fld*.hxx

      Rev 1.26   25 Oct 1994 17:57:02   ER
   add: PCH

      Rev 1.25   17 Oct 1994 16:24:58   PK
   ausgeboxtes wieder reingeboxt

*************************************************************************/

