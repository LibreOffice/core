/*************************************************************************
 *
 *  $RCSfile: appenv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:31 $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <svtools/svmedit.hxx>
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#include "paratr.hxx"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "docsh.hxx"
#include "frmatr.hxx"
#include "fldbas.hxx"
#include "swundo.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "doc.hxx"
#include "dbmgr.hxx"
#include "envlop.hxx"
#include "fmtcol.hxx"
#include "frmmgr.hxx"
#include "fldmgr.hxx"
#include "pagedesc.hxx"
#include "poolfmt.hxx"
#include "expfld.hxx"
#include "app.hrc"
#include "poolfmt.hrc"

#define ENV_NEWDOC      RET_OK
#define ENV_INSERT      RET_USER
#define ENV_CANCEL      SHRT_MAX


// --------------------------------------------------------------------------


// Funktion wird fuer Etiketten und Briefumschlaege benutzt!
//  im applab.cxx und appenv.cxx
BOOL InsertLabEnvText( SwWrtShell& rSh, SwFldMgr& rFldMgr, const String& rText )
{
    BOOL bRet = FALSE;
    String aText(rText);
    aText.EraseAllChars( '\r' );


    USHORT nTokenPos = 0;
    while( STRING_NOTFOUND != nTokenPos )
    {
        String aLine = aText.GetToken( 0, '\n', nTokenPos );
        while ( aLine.Len() )
        {
            String sTmpText;
            BOOL bField = FALSE;

            USHORT nPos = aLine.Search( '<' );
            if ( nPos )
            {
                sTmpText = aLine.Copy( 0, nPos );
                aLine.Erase( 0, nPos );
//              sTmpText = aLine.Cut( 0, nPos );
            }
            else
            {
                nPos = aLine.Search( '>' );
                if ( nPos == STRING_NOTFOUND )
                {
                    sTmpText = aLine;
                    aLine.Erase();
//                  sTmpText = aLine.Cut();
                }
                else
                {
                    sTmpText = aLine.Copy( 0, nPos + 1);
                    aLine.Erase( 0, nPos + 1);
//                  sTmpText = aLine.Cut( 0, nPos + 1 );

                    // Datenbankfelder muesen mind. 3 Punkte beinhalten!
                    String sDBName( sTmpText.Copy( 1, sTmpText.Len() - 2));
                    USHORT nCnt = sDBName.GetTokenCount('.');
                    if (nCnt >= 3)
                    {
                        ::ReplacePoint(sDBName);
                        rFldMgr.InsertFld( TYP_DBFLD, 0, sDBName, aEmptyStr, 0, &rSh );
                        bRet = TRUE;
                        bField = TRUE;
                    }
                }
            }
            if ( !bField )
                rSh.Insert( sTmpText );
        }
        rSh.InsertLineBreak();
    }
    rSh.DelLeft();  // Letzten Linebreak wieder l”schen

    return bRet;
}

// ----------------------------------------------------------------------------


void lcl_CopyCollAttr(SwWrtShell* pOldSh, SwWrtShell* pNewSh, USHORT nCollId)
{
    USHORT nCollCnt = pOldSh->GetTxtFmtCollCount();
    SwTxtFmtColl* pColl;
    for( USHORT nCnt = 0; nCnt < nCollCnt; ++nCnt )
        if(nCollId == (pColl = &pOldSh->GetTxtFmtColl(nCnt))->GetPoolFmtId())
            pNewSh->GetTxtCollFromPool(nCollId)->SetAttr(pColl->GetAttrSet());
}

// ----------------------------------------------------------------------------


void SwModule::InsertEnv()
{
static USHORT nTitleNo = 0;

    SwDocShell      *pMyDocSh;
    SfxViewFrame    *pFrame;
    SwView          *pView;
    SwWrtShell      *pOldSh,
                    *pSh;

    //aktuelle Shell besorgen
    pMyDocSh = (SwDocShell*) SfxObjectShell::Current();
    pOldSh   = pMyDocSh ? pMyDocSh->GetWrtShell() : 0;

    // Neues Dokument erzeugen (kein Show!)
    SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
    xDocSh->DoInitNew( 0 );
    pFrame = SFX_APP()->CreateViewFrame( *xDocSh, 0, TRUE );
    pView = (SwView*) pFrame->GetViewShell();
    pView->AttrChangedNotify( &pView->GetWrtShell() );//Damit SelectShell gerufen wird.
    pSh = pView->GetWrtShellPtr();

    String aTmp( SW_RES(STR_ENV_TITLE) );
    aTmp += String::CreateFromInt32( ++nTitleNo );
    xDocSh->SetTitle( aTmp );

    // Ggf. alte Collections "Absender" und "Empfaenger" in neues
    // Dokument kopieren
    if ( pOldSh )
    {
        ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_JAKETADRESS);
        ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_SENDADRESS);
    }

    // SwEnvItem aus Config lesen
    SwEnvCfgItem aEnvCfg;
    aEnvCfg.Initialize();

    //Haben wir schon einen Briefumschlag.
    BOOL bEnvChange = FALSE;

    SfxItemSet aSet(GetPool(), FN_ENVELOP, FN_ENVELOP, 0);
    aSet.Put(aEnvCfg.aEnvItem);

    SfxPrinter* pTempPrinter = pSh->GetPrt( TRUE );
    if(pOldSh )
    {
        const SwPageDesc& rCurPageDesc = pOldSh->GetPageDesc(pOldSh->GetCurPageDesc());
        String sJacket; GetDocPoolNm( RES_POOLPAGE_JAKET, sJacket );
        bEnvChange = rCurPageDesc.GetName() == sJacket;
        if(pOldSh->GetPrt(FALSE))
        {
            pSh->GetDoc()->SetJobsetup(*pOldSh->GetDoc()->GetJobsetup());
            //#69563# if it isn't the same printer then the pointer has been invalidated!
            pTempPrinter = pSh->GetPrt( TRUE );
        }
        pTempPrinter->SetPaperBin(rCurPageDesc.GetMaster().GetPaperBin().GetValue());

    }

    Window *pParent = pOldSh ? pOldSh->GetWin() : 0;
    SwEnvDlg* pDlg = new SwEnvDlg( pParent, aSet,
                                   pOldSh, pTempPrinter,
                                   !bEnvChange);

    short nMode = pDlg->Execute();

    if (nMode == ENV_NEWDOC || nMode == ENV_INSERT)
    {
        SwWait aWait( (SwDocShell&)*xDocSh, TRUE );

        // Dialog auslesen, Item in Config speichern
        const SwEnvItem& rItem = (const SwEnvItem&) pDlg->
                                            GetOutputItemSet()->Get(FN_ENVELOP);
        aEnvCfg.aEnvItem = rItem;
        aEnvCfg.SetDefault(FALSE);
        aEnvCfg.StoreConfig();

        //Wenn wir Drucken uebernehmen wir den eingestellten Jobsetup aus
        //dem Dialog. Die Informationen muessen hier vor dem evtl. zerstoeren
        //der neuen Shell gesetzt werden, weil deren Drucker an den Dialog
        //gereicht wurde.
        if ( nMode != ENV_NEWDOC )
        {
            ASSERT(pOldSh, "Kein Dokument - war 'Einfuegen' nicht disabled???");
            SvxPaperBinItem aItem;
            aItem.SetValue((BYTE)pSh->GetPrt()->GetPaperBin());
            pOldSh->GetPageDescFromPool(RES_POOLPAGE_JAKET)->GetMaster().SetAttr(aItem);
        }

        SwWrtShell *pTmp = nMode == ENV_INSERT ? pOldSh : pSh;
        const SwPageDesc* pFollow = 0;
        SwTxtFmtColl *pSend = pTmp->GetTxtCollFromPool( RES_POOLCOLL_SENDADRESS ),
                     *pAddr = pTmp->GetTxtCollFromPool( RES_POOLCOLL_JAKETADRESS);
        const String &rSendMark = pSend->GetName();
        const String &rAddrMark = pAddr->GetName();

        if (nMode == ENV_INSERT)
        {

            SetView(&pOldSh->GetView()); // Pointer auf oberste View restaurieren

            //Neues Dok wieder loeschen
            xDocSh->DoClose();
            pSh = pOldSh;

            // Los geht's (Einfuegen)
            pSh->StartUndo(UIUNDO_INSERT_ENVELOPE);
            pSh->StartAllAction();
            pSh->SwCrsrShell::SttDoc();

            if (bEnvChange)
            {
                // Folgevorlage: Seite 2
                pFollow = pSh->GetPageDesc(pSh->GetCurPageDesc()).GetFollow();

                // Text der ersten Seite loeschen
                if ( !pSh->SttNxtPg(TRUE) )
                    pSh->EndPg(TRUE);
                pSh->DelRight();
                // Rahmen der ersten Seite loeschen
                if( pSh->GotoFly( rSendMark ) )
                {
                    pSh->EnterSelFrmMode();
                    pSh->DelRight();
                }
                if ( pSh->GotoFly( rAddrMark ) )
                {
                    pSh->EnterSelFrmMode();
                    pSh->DelRight();
                }
                pSh->SwCrsrShell::SttDoc();
            }
            else
                // Folgevorlage: Seite 1
                pFollow = &pSh->GetPageDesc(pSh->GetCurPageDesc());

            // Seitenumbruch einfuegen
            if ( pSh->IsCrsrInTbl() )
            {
                pSh->SplitNode();
                pSh->Right();
                SfxItemSet aSet( pSh->GetAttrPool(), RES_BREAK, RES_BREAK, 0 );
                aSet.Put( SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE) );
                pSh->SetTblAttr( aSet );
            }
            else
                pSh->InsertPageBreak(0, FALSE);
            pSh->SwCrsrShell::SttDoc();
        }
        else
        {
            pFollow = &pSh->GetPageDesc(pSh->GetCurPageDesc());
            // Los geht's (Drucken)
            pSh->StartAllAction();
            pSh->DoUndo(FALSE);

            // Neue Collections "Absender" und "Empfaenger" wieder in neues
            // Dokument kopieren
            if ( pOldSh )
            {
                ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_JAKETADRESS);
                ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_SENDADRESS);
            }
        }

        SET_CURR_SHELL(pSh);
        pSh->SetNewDoc();       // Performanceprobleme vermeiden

        // Flys dieser Seite merken
        SvPtrarr aFlyArr(0, 5);
        if( ENV_NEWDOC != nMode && !bEnvChange )
            pSh->GetPageObjs( aFlyArr );

        // Page-Desc ermitteln
        SwPageDesc* pDesc = pSh->GetPageDescFromPool(RES_POOLPAGE_JAKET);
        SwFrmFmt&   rFmt  = pDesc->GetMaster();

        Printer *pPrt = pSh->GetPrt( TRUE );

        // Raender (setzen sich zusammen aus Shift-Offset und
        // Ausrichtung)
        Size aPaperSize = pPrt->PixelToLogic( pPrt->GetPaperSizePixel(),
                                              MAP_TWIP);
        if ( !aPaperSize.Width() && !aPaperSize.Height() )
        {
            aPaperSize.Width() = lA4Width;
            aPaperSize.Height()= lA4Height;
        }
        if ( aPaperSize.Width() > aPaperSize.Height() )
            Swap( aPaperSize );

        long lLeft  = rItem.lShiftRight,
             lUpper = rItem.lShiftDown;

        USHORT nPageW = (USHORT) Max(rItem.lWidth, rItem.lHeight),
               nPageH = (USHORT) Min(rItem.lWidth, rItem.lHeight);

        switch (rItem.eAlign)
        {
            case ENV_HOR_LEFT: break;
            case ENV_HOR_CNTR: lLeft  += Max(0L, long(aPaperSize.Width() - nPageW)) / 2;
                               break;
            case ENV_HOR_RGHT: lLeft  += Max(0L, long(aPaperSize.Width() - nPageW));
                               break;
            case ENV_VER_LEFT: lUpper += Max(0L, long(aPaperSize.Width() - nPageH));
                               break;
            case ENV_VER_CNTR: lUpper += Max(0L, long(aPaperSize.Width() - nPageH)) / 2;
                               break;
            case ENV_VER_RGHT: break;
        }
        SvxLRSpaceItem aLRMargin;
        SvxULSpaceItem aULMargin;
        aLRMargin.SetLeft ((USHORT) lLeft );
        aULMargin.SetUpper((USHORT) lUpper);
        aLRMargin.SetRight(0);
        aULMargin.SetLower(0);
        rFmt.SetAttr(aLRMargin);
        rFmt.SetAttr(aULMargin);

        // Kopf-, Fusszeilen
        rFmt.SetAttr(SwFmtHeader(BOOL(FALSE)));
        pDesc->ChgHeaderShare(FALSE);
        rFmt.SetAttr(SwFmtFooter(BOOL(FALSE)));
        pDesc->ChgFooterShare(FALSE);

        // Seitennumerierung
        pDesc->SetUseOn(PD_ALL);

        // Einstellen der Seitengroesse
        rFmt.SetAttr(SwFmtFrmSize(ATT_FIX_SIZE,
                                            nPageW + lLeft, nPageH + lUpper));

        // Einstellen der Numerierungsart der Seite
        SwNumType aType;
        aType.eType = SVX_NUM_NUMBER_NONE;
        pDesc->SetNumType(aType);

        // Folgevorlage
        if (pFollow)
            pDesc->SetFollow(pFollow);

        // Landscape
        pDesc->SetLandscape( rItem.eAlign >= ENV_VER_LEFT &&
                             rItem.eAlign <= ENV_VER_RGHT);

        // Page-Desc anwenden

        USHORT nPos;
        pSh->FindPageDescByName( pDesc->GetName(),
                                    FALSE,
                                    &nPos );


        pSh->ChgPageDesc( nPos, *pDesc);
        pSh->ChgCurPageDesc(*pDesc);

        // Rahmen einfuegen
        SwFlyFrmAttrMgr aMgr(FALSE, pSh, FRMMGR_TYPE_ENVELP);
        SwFldMgr aFldMgr;

        // Absender
        if (rItem.bSend)
        {
            pSh->SwCrsrShell::SttDoc();
            aMgr.SetSizeType(ATT_VAR_SIZE);

            //Defaults ueberschreiben!
            aMgr.GetAttrSet().Put( SvxBoxItem() );
            aMgr.SetULSpace( 0L, 0L );
            aMgr.SetLRSpace( 0L, 0L );

            aMgr.InsertFlyFrm(FLY_PAGE,
                Point(rItem.lSendFromLeft + lLeft, rItem.lSendFromTop  + lUpper),
                Size (rItem.lAddrFromLeft - rItem.lSendFromLeft, 0));

            pSh->EnterSelFrmMode();
            pSh->SetFlyName( rSendMark );
            pSh->UnSelectFrm();
            pSh->LeaveSelFrmMode();
            pSh->SetTxtFmtColl( pSend );
            InsertLabEnvText( *pSh, aFldMgr, rItem.aSendText );
            aMgr.UpdateAttrMgr();
        }

        // Empfaenger
        pSh->SwCrsrShell::SttDoc();

        aMgr.SetSizeType(ATT_VAR_SIZE);
        aMgr.InsertFlyFrm(FLY_PAGE,
            Point(rItem.lAddrFromLeft + lLeft, rItem.lAddrFromTop  + lUpper),
            Size (nPageW - rItem.lAddrFromLeft - 566, 0));
        pSh->EnterSelFrmMode();
        pSh->SetFlyName( rAddrMark );
        pSh->UnSelectFrm();
        pSh->LeaveSelFrmMode();
        pSh->SetTxtFmtColl( pAddr );
        InsertLabEnvText(*pSh, aFldMgr, rItem.aAddrText);

        // Flys auf die "alten" Seiten verschieben
        if (aFlyArr.Count())
            pSh->SetPageObjsNewPage(aFlyArr, 1);

        // Fertig
        pSh->SwCrsrShell::SttDoc();

        pSh->EndAllAction();

        if (nMode == ENV_NEWDOC)
            pSh->DoUndo(TRUE);
        else
            pSh->EndUndo(UIUNDO_INSERT_ENVELOPE);

        if (nMode == ENV_NEWDOC)
        {
            pFrame->GetFrame()->Appear();

            if ( rItem.aAddrText.Search('<') != STRING_NOTFOUND )
            {
                static USHORT __READONLY_DATA aInva[] =
                                    {
                                        SID_SBA_BRW_UPDATE,
                                        SID_SBA_BRW_INSERT,
                                        SID_SBA_BRW_MERGE,
                                        0
                                    };
                pFrame->GetBindings().Invalidate( aInva );

                // Datenbankbeamer oeffnen
                String sDBName(pSh->GetDBName());
                ShowDBObj(*pSh, sDBName);
            }
        }
    }
    else    //Abbruch
    {
        xDocSh->DoClose();
        --nTitleNo;

        // Pointer auf oberste View restaurieren
        if (pOldSh)
            SetView(&pOldSh->GetView());
    }
    delete pDlg;
}

/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.103  2000/09/18 16:05:09  willem.vandorp
    OpenOffice header added.

    Revision 1.102  2000/09/07 15:59:19  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.101  2000/07/26 16:33:02  jp
    use the new function GetDocPoolNm to get the collectionames

    Revision 1.100  2000/07/18 12:50:07  os
    replace ofadbmgr

    Revision 1.99  2000/06/30 08:52:03  os
    #76541# string assertions removed

    Revision 1.98  2000/05/23 17:52:04  jp
    Bugfixes for Unicode

    Revision 1.97  2000/04/20 12:49:34  os
    GetName() returns String&

    Revision 1.96  2000/04/11 08:01:30  os
    UNICODE

    Revision 1.95  2000/03/08 17:21:49  os
    GetAppWindow() - misuse as parent window eliminated

    Revision 1.94  2000/02/11 14:42:22  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.93  1999/11/19 12:58:47  os
    #69563# after SetJobsetup the printer may be invalid

    Revision 1.92  1999/11/11 14:34:40  hr
    #65293# STLPORT 3.2.1

    Revision 1.91  1999/10/05 10:18:05  os
    #67889# some printer problems solved

    Revision 1.90  1999/10/01 13:36:53  os
    apply AutoText to business cards

    Revision 1.89  1999/07/08 13:59:32  MA
    Use internal object to toggle wait cursor


      Rev 1.88   08 Jul 1999 15:59:32   MA
   Use internal object to toggle wait cursor

      Rev 1.87   10 Jun 1999 10:52:08   JP
   have to change: no AppWin from SfxApp

      Rev 1.86   01 Mar 1999 16:20:52   MA
   #62490# Altlast entfernt (Drucken und Briefumschlaege/Etiketten und Datenbank)

      Rev 1.85   17 Nov 1998 10:49:46   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.84   12 Nov 1998 19:19:20   MA
   #59315# Tabelle am Dokumentanfang beruecksichtigen

      Rev 1.83   14 Mar 1998 17:05:12   OM
   Gelinkte Etiketten

      Rev 1.82   05 Feb 1998 16:34:32   OS
   Change: hidden ViewFrame anlegen

      Rev 1.81   24 Nov 1997 14:22:34   MA
   includes

      Rev 1.80   30 Sep 1997 08:40:28   OS
   include

      Rev 1.79   12 Sep 1997 10:38:30   OS
   ITEMID_* definiert

      Rev 1.78   02 Sep 1997 09:56:50   OM
   SDB-Headeranpassung

      Rev 1.77   01 Sep 1997 13:06:32   OS
   DLL-Umstellung

      Rev 1.76   15 Aug 1997 11:45:06   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.75   12 Aug 1997 14:42:40   OS
   Header-Umstellung

      Rev 1.74   08 Aug 1997 17:26:50   OM
   Headerfile-Umstellung

      Rev 1.73   07 Aug 1997 14:59:28   OM
   Headerfile-Umstellung

      Rev 1.72   08 Jul 1997 14:04:10   OS
   ConfigItems von der App ans Module

      Rev 1.71   11 Apr 1997 12:04:16   NF
   includes...

      Rev 1.70   08 Apr 1997 15:11:08   MH
   chg: header

      Rev 1.69   08 Apr 1997 08:32:20   MA
   includes

      Rev 1.68   13 Mar 1997 16:28:48   OM
   DB-Feldbefehle korrekt erkennen

      Rev 1.67   26 Feb 1997 03:46:12   OM
   #37071# Richtige Datenbank nach Briefumschlag und Etiketten anzeigen

      Rev 1.66   25 Feb 1997 22:48:58   OM
   Datenbankfelder richtig einfuegen

      Rev 1.65   11 Feb 1997 16:51:54   OM
   Eingabefeld ueber Basic ohne Dialog einfuegen

      Rev 1.64   16 Dec 1996 10:59:24   OM
   Drucken aus DB-Browser angefangen

      Rev 1.63   11 Dec 1996 09:26:14   MA
   Entschlackt

      Rev 1.62   06 Dec 1996 13:53:24   MA
   Umbrueche fuer Tabellen

      Rev 1.61   05 Dec 1996 15:23:54   OM
   Nach Briefumschlag und Etiketten DB-Browser oeffnen

      Rev 1.60   03 Dec 1996 17:41:12   AMA
   Chg: Der Drucker wird nur im !Browsemodus angelegt.

      Rev 1.59   08 Nov 1996 19:39:34   MA
   ResMgr

      Rev 1.58   08 Nov 1996 17:38:14   OM
   DB-Mode fuer Serienbriefe und Etiketten zum Teil wiederbelebt

      Rev 1.57   24 Oct 1996 13:36:04   JP
   String Umstellung: [] -> GetChar()

      Rev 1.56   18 Oct 1996 12:12:46   MA
   fixes und opts

      Rev 1.55   17 Oct 1996 16:54:36   MA
   #31660# und diverse weitere fixes

      Rev 1.54   26 Sep 1996 09:03:30   MA
   defautls fuer Rahmen + Aufraeumarbeiten

      Rev 1.53   25 Sep 1996 14:10:24   OM
   Neue Datenbanktrenner

-------------------------------------------------------------------------*/


