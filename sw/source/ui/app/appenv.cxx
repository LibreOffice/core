/*************************************************************************
 *
 *  $RCSfile: appenv.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-28 12:32:43 $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <sfx2/request.hxx>

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
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _ENVLOP_HXX
#include <envlop.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

#define ENV_NEWDOC      RET_OK
#define ENV_INSERT      RET_USER
#define ENV_CANCEL      SHRT_MAX


// --------------------------------------------------------------------------


// Funktion wird fuer Etiketten und Briefumschlaege benutzt!
//  im applab.cxx und appenv.cxx
String InsertLabEnvText( SwWrtShell& rSh, SwFldMgr& rFldMgr, const String& rText )
{
    String sRet;
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
                        ::ReplacePoint(sDBName, TRUE);
                        SwInsertFld_Data aData(TYP_DBFLD, 0, sDBName, aEmptyStr, 0, &rSh );
                        rFldMgr.InsertFld( aData );
                        sRet = sDBName;
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

    return sRet;
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


void SwModule::InsertEnv( SfxRequest& rReq )
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

    //Haben wir schon einen Briefumschlag.
    BOOL bEnvChange = FALSE;

    SfxItemSet aSet(GetPool(), FN_ENVELOP, FN_ENVELOP, 0);
    aSet.Put(aEnvCfg.GetItem());

    SfxPrinter* pTempPrinter = pSh->GetPrt( TRUE );
    if(pOldSh )
    {
        const SwPageDesc& rCurPageDesc = pOldSh->GetPageDesc(pOldSh->GetCurPageDesc());
        String sJacket;
        SwStyleNameMapper::FillUIName( RES_POOLPAGE_JAKET, sJacket );
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
    SwEnvDlg* pDlg = NULL;
    short nMode = ENV_INSERT;

    SFX_REQUEST_ARG( rReq, pItem, SwEnvItem, FN_ENVELOP, sal_False );
    if ( !pItem )
    {
        pDlg = new SwEnvDlg( pParent, aSet, pOldSh, pTempPrinter, !bEnvChange);
        nMode = pDlg->Execute();
    }
    else
    {
        SFX_REQUEST_ARG( rReq, pBoolItem, SfxBoolItem, FN_PARAM_1, sal_False );
        if ( pBoolItem && pBoolItem->GetValue() )
            nMode = ENV_NEWDOC;
    }

    /*TODO #111050# call public made method of sfx (which was protected before!)
      to force missing event OnNew ... */
    if (nMode == ENV_NEWDOC)
        xDocSh->Stamp_SetActivateEvent(SFX_EVENT_CREATEDOC);

    if (nMode == ENV_NEWDOC || nMode == ENV_INSERT)
    {
        SwWait aWait( (SwDocShell&)*xDocSh, TRUE );

        // Dialog auslesen, Item in Config speichern
        const SwEnvItem& rItem = pItem ? *pItem : (const SwEnvItem&) pDlg->GetOutputItemSet()->Get(FN_ENVELOP);
        aEnvCfg.GetItem() = rItem;
        aEnvCfg.Commit();

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
            //#i4251# selected text or objects in the document should
            //not be deleted on inserting envelopes
            pSh->EnterStdMode();
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
                pSh->Right( CRSR_SKIP_CHARS, FALSE, 1, FALSE );
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
        SvxNumberType aType;
        aType.SetNumberingType(SVX_NUM_NUMBER_NONE);
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
        aMgr.SetSizeType(ATT_VAR_SIZE);

        //Defaults ueberschreiben!
        aMgr.GetAttrSet().Put( SvxBoxItem() );
        aMgr.SetULSpace( 0L, 0L );
        aMgr.SetLRSpace( 0L, 0L );

        // Absender
        if (rItem.bSend)
        {
            pSh->SwCrsrShell::SttDoc();
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

            if ( rItem.aAddrText.indexOf('<') >= 0 )
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
                ShowDBObj(*pView, pSh->GetDBData());
            }
        }

        if ( !pItem )
        {
            rReq.AppendItem( rItem );
            if ( nMode == ENV_NEWDOC )
                rReq.AppendItem( SfxBoolItem( FN_PARAM_1, TRUE ) );
        }

        rReq.Done();
    }
    else    //Abbruch
    {
        rReq.Ignore();

        xDocSh->DoClose();
        --nTitleNo;

        // Pointer auf oberste View restaurieren
        if (pOldSh)
            SetView(&pOldSh->GetView());
    }
    delete pDlg;
}


