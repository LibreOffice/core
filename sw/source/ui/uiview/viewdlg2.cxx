/*************************************************************************
 *
 *  $RCSfile: viewdlg2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif


#ifndef _FLDMGR_HXX //autogen
#include <fldmgr.hxx>
#endif
#ifndef _EXPFLD_HXX //autogen
#include <expfld.hxx>
#endif

#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif

#include "swmodule.hxx"
#include "view.hxx"
#include "wview.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "cption.hxx"
#include "caption.hxx"
#include "insfnote.hxx"
#include "poolfmt.hxx"
#include "edtwin.hxx"

#include "view.hrc"

extern String* pOldGrfCat;
extern String* pOldTabCat;
extern String* pOldFrmCat;
extern String* pOldDrwCat;

/* -----------------06.11.98 13:45-------------------
 *
 * --------------------------------------------------*/

void SwView::ExecDlgExt(SfxRequest &rReq)
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    ModalDialog *pDialog = 0;
    const SfxItemSet* pOutSet = 0;

    switch ( rReq.GetSlot() )
    {
        case FN_INSERT_CAPTION:
        {
            pDialog = new SwCaptionDialog( pMDI, *this );
            break;
        }
        case  FN_EDIT_FOOTNOTE:
        {
            pDialog = new SwInsFootNoteDlg( pMDI, *pWrtShell, TRUE );
            pDialog->SetHelpId(FN_EDIT_FOOTNOTE);
            pDialog->SetText( SW_RESSTR(STR_EDIT_FOOTNOTE) );
            break;
        }
    }
    if ( pDialog )
    {
        pDialog->Execute();
        delete pDialog;
    }
}

/* -----------------06.11.98 14:53-------------------
 *
 * --------------------------------------------------*/

void SwView::AutoCaption(const USHORT nType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    if (pModOpt->IsInsWithCaption(bWeb))
    {
        const InsCaptionOpt *pOpt = pModOpt->GetCapOption(bWeb, (SwCapObjType)nType, pOleId);
        if (pOpt && pOpt->UseCaption() == TRUE)
            InsertCaption(pOpt);
    }
}

/* -----------------06.11.98 12:58-------------------
 *
 * --------------------------------------------------*/

void SwView::InsertCaption(const InsCaptionOpt *pOpt)
{
    if (!pOpt)
        return;

    const String &rName = pOpt->GetCategory();

    // Existiert Pool-Vorlage gleichen Namens?
    SwWrtShell &rSh = GetWrtShell();
    USHORT nPoolId = rSh.GetPoolId(rName, GET_POOLID_TXTCOLL);
    if( USHRT_MAX != nPoolId )
        rSh.GetTxtCollFromPool(nPoolId);
        // Pool-Vorlage existiert nicht: Existiert sie am Dokument?
    else if( !rSh.GetParaStyle(rName) )
    {
        // Sie existiert auch nicht am Dokument: erzeugen
        SwTxtFmtColl* pDerivedFrom = rSh.GetTxtCollFromPool(RES_POOLCOLL_LABEL);
        rSh.MakeTxtFmtColl(rName, pDerivedFrom);
    }

    USHORT eType = (SwWrtShell::SelectionType)rSh.GetSelectionType();
    if (eType & SwWrtShell::SEL_OLE)
        eType = SwWrtShell::SEL_GRF;

    // SwLabelType
    const USHORT eT = eType & SwWrtShell::SEL_TBL ? LTYPE_TABLE :
                      eType & SwWrtShell::SEL_FRM ? LTYPE_FLY :
                      eType == SwWrtShell::SEL_TXT ? LTYPE_FLY :
                      eType & SwWrtShell::SEL_DRW ? LTYPE_DRAW :
                                                    LTYPE_OBJECT;

    SwFldMgr aMgr(&rSh);
    SwSetExpFieldType* pFldType =
            (SwSetExpFieldType*)aMgr.GetFldType(RES_SETEXPFLD, rName);
    if (!pFldType)
    {
        // Neuen Feldtypen erzeugen
        SwSetExpFieldType aSwSetExpFieldType(rSh.GetDoc(), rName, GSE_SEQ);
        aMgr.InsertFldType(aSwSetExpFieldType);
        pFldType = (SwSetExpFieldType*)aMgr.GetFldType(RES_SETEXPFLD, rName);
    }

    if (!pOpt->IgnoreSeqOpts())
    {
        if (pFldType)
        {
            pFldType->SetDelimiter(pOpt->GetSeparator());
            pFldType->SetOutlineLvl(pOpt->GetLevel());
        }
    }

    USHORT       nID    = 0;
    SwFieldType* pType  = 0;
    const USHORT nCount = aMgr.GetFldTypeCount();
    for (USHORT i = 0; i < nCount; ++i)
    {
        pType = aMgr.GetFldType(USHRT_MAX, i);
        String aTmpName( pType->GetName() );
        if (aTmpName == rName && pType->Which() == RES_SETEXPFLD)
        {
            nID = i;
            break;
        }
    }
    rSh.StartAllAction();

    GetWrtShell().InsertLabel( (SwLabelType)eT,
                                pOpt->GetCaption(),
                                !pOpt->GetPos(),
                                nID,
                                pOpt->CopyAttributes() );
    // Nummernformat setzen
    ((SwSetExpFieldType*)pType)->SetSeqFormat(pOpt->GetNumType());

    rSh.UpdateExpFlds( TRUE );

    rSh.EndAllAction();

    if ( rSh.IsFrmSelected() )
    {
        GetEditWin().StopInsFrm();
        rSh.EnterSelFrmMode();
    }

    // Kategorie merken
    String** ppStr = 0;
    if (eType & SwWrtShell::SEL_GRF)
        ppStr = &pOldGrfCat;
    else if( eType & SwWrtShell::SEL_TBL)
        ppStr = &pOldTabCat;
    else if( eType & SwWrtShell::SEL_FRM)
        ppStr = &pOldFrmCat;
    else if( eType == SwWrtShell::SEL_TXT)
        ppStr = &pOldFrmCat;
    else if( eType & SwWrtShell::SEL_DRW)
        ppStr = &pOldDrwCat;

    if( ppStr )
    {
        if( !*ppStr )
            *ppStr = new String( rName );
        else
            **ppStr = rName;
    }
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.56  2000/09/18 16:06:13  willem.vandorp
    OpenOffice header added.

    Revision 1.55  2000/05/26 07:21:35  os
    old SW Basic API Slots removed

    Revision 1.54  1999/06/09 08:44:54  OS
    #66732# Insert caption: copy border and shadow optionally


      Rev 1.53   09 Jun 1999 10:44:54   OS
   #66732# Insert caption: copy border and shadow optionally

      Rev 1.52   17 Feb 1999 08:40:28   OS
   #58158# Einfuegen TabPage auch in HTML-Docs

      Rev 1.51   25 Jan 1999 14:10:32   HR
   include editwin.hxx

      Rev 1.50   21 Jan 1999 18:53:36   MA
   #60697# die bloeden Stati pflegen

      Rev 1.49   10 Dec 1998 09:47:16   MIB
   #60060#: Beschriftungen fuer Zeichen-Objekte

      Rev 1.48   06 Nov 1998 17:05:58   OM
   #58158# Automatische Beschriftung

      Rev 1.47   21 Nov 1997 15:00:18   MA
   includes

      Rev 1.46   03 Nov 1997 13:58:28   MA
   precomp entfernt

      Rev 1.45   07 Apr 1997 19:00:26   MH
   chg: header

      Rev 1.44   11 Nov 1996 11:10:50   MA
   ResMgr

      Rev 1.43   29 Jul 1996 15:47:16   MA
   includes

      Rev 1.42   23 Feb 1996 17:04:58   OM
   OpenINetDlg entfernt

      Rev 1.41   20 Feb 1996 10:08:20   OM
   Methode fuer Internet-Open-Dlg

      Rev 1.40   30 Nov 1995 15:16:04   OS
   div. Slots verschoben

      Rev 1.39   24 Nov 1995 16:58:00   OM
   PCH->PRECOMPILED

      Rev 1.38   08 Nov 1995 13:07:44   OS
   Change => Set

      Rev 1.37   06 Nov 1995 17:20:04   OS
   kleinere Optimierung

      Rev 1.36   31 Oct 1995 18:42:46   OM
   GetFrameWindow entfernt

      Rev 1.35   30 Oct 1995 18:51:18   OS
   FN_POSTIT ueber Slot einfuegen

      Rev 1.34   18 Sep 1995 18:30:08   OM
   HelpIDs

      Rev 1.33   24 Aug 1995 14:31:54   MA
   swstddlg -> svxstandarddialog

      Rev 1.32   21 Aug 1995 09:26:36   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.31   23 Jun 1995 07:26:34   OS
   FALSe gibt's nicht

      Rev 1.30   22 Jun 1995 20:03:46   MA
   fix/opt: FrmMgr mit Shell erzeugen.

      Rev 1.29   09 Jun 1995 14:25:34   OM
   Alten DB-Manager entfernt

      Rev 1.28   05 Apr 1995 19:06:34   OS
   Fix fuer Tabellen-und Rahmenhintergrund Bug 10891

      Rev 1.27   04 Apr 1995 14:09:56   MA
   Optimiert, includes gespart.

      Rev 1.26   03 Apr 1995 19:41:52   OS
   Titel fuer Edit-Footnote Fix 11252

      Rev 1.25   28 Mar 1995 19:39:54   OM
   Aufgeteilt auf viewdlg2 und 4

      Rev 1.24   24 Mar 1995 20:17:22   PK
   pkbla ist out

      Rev 1.23   04 Mar 1995 14:08:04   MA
   fix: GPF Umrandung fuer Tabelle.

      Rev 1.22   02 Mar 1995 18:49:34   PK
   pkbla-geraffel wieder uebersetzbar

      Rev 1.21   01 Mar 1995 11:08:48   MA
   weg: Sw-BoxItem, neu: KeepItem.

      Rev 1.20   16 Feb 1995 19:33:58   PK
   swborderdlg: einsatz von swflyfrmattrmgr

      Rev 1.19   08 Feb 1995 18:39:38   PK
   bla-geraffel mit #ifdef pkbla

      Rev 1.18   08 Feb 1995 10:21:44   SWG
   blaout kommentiert.

      Rev 1.17   08 Feb 1995 10:18:18   SWG
   blaout kommentiert.

      Rev 1.16   07 Feb 1995 19:34:38   PK
   umrandungen, bla-geraffel

      Rev 1.14   26 Jan 1995 16:58:44   PK
   gebrauch von get- und settabborders

      Rev 1.13   24 Jan 1995 13:29:06   PK
   Map entfernt

      Rev 1.12   18 Jan 1995 13:22:58   MS
   Precompiled hd

      Rev 1.11   13 Jan 1995 12:17:06   MS
   Aufteilung DBMgr

------------------------------------------------------------------------*/

