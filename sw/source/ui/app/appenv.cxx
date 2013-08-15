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


#include <cstdarg>

#include <hintids.hxx>

#include <comphelper/string.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>

#include <svtools/svmedit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/printer.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/formatbreakitem.hxx>
#include <fmthdft.hxx>
#include <swwait.hxx>
#include <paratr.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <frmatr.hxx>
#include <fldbas.hxx>
#include <swundo.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <dbmgr.hxx>
#include <fmtcol.hxx>
#include <frmmgr.hxx>
#include <fldmgr.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>
#include <SwStyleNameMapper.hxx>
#include <crsskip.hxx>

#include <cmdid.h>
#include <globals.hrc>
#include <app.hrc>
#include <poolfmt.hrc>
#include "swabstdlg.hxx"
#include "envelp.hrc"
#include "envimg.hxx"

#define ENV_NEWDOC      RET_OK
#define ENV_INSERT      RET_USER

// Function used for labels and envelopes in applab.cxx and appenv.cxx
String InsertLabEnvText( SwWrtShell& rSh, SwFldMgr& rFldMgr, const String& rText )
{
    String sRet;
    String aText(comphelper::string::remove(rText, '\r'));

    sal_Int32 nTokenPos = 0;
    while( -1 != nTokenPos )
    {
        String aLine = aText.GetToken( 0, '\n', nTokenPos );
        while ( aLine.Len() )
        {
            String sTmpText;
            bool bField = false;

            sal_uInt16 nPos = aLine.Search( '<' );
            if ( nPos )
            {
                sTmpText = aLine.Copy( 0, nPos );
                aLine.Erase( 0, nPos );
            }
            else
            {
                nPos = aLine.Search( '>' );
                if ( nPos == STRING_NOTFOUND )
                {
                    sTmpText = aLine;
                    aLine.Erase();
                }
                else
                {
                    sTmpText = aLine.Copy( 0, nPos + 1);
                    aLine.Erase( 0, nPos + 1);

                    // Database fields must contain at least 3 points!
                    String sDBName( sTmpText.Copy( 1, sTmpText.Len() - 2));
                    sal_uInt16 nCnt = comphelper::string::getTokenCount(sDBName, '.');
                    if (nCnt >= 3)
                    {
                        ::ReplacePoint(sDBName, true);
                        SwInsertFld_Data aData(TYP_DBFLD, 0, sDBName, aEmptyStr, 0, &rSh );
                        rFldMgr.InsertFld( aData );
                        sRet = sDBName;
                        bField = true;
                    }
                }
            }
            if ( !bField )
                rSh.Insert( sTmpText );
        }
        rSh.InsertLineBreak();
    }
    rSh.DelLeft();  // Again remove last linebreak

    return sRet;
}

static void lcl_CopyCollAttr(SwWrtShell* pOldSh, SwWrtShell* pNewSh, sal_uInt16 nCollId)
{
    sal_uInt16 nCollCnt = pOldSh->GetTxtFmtCollCount();
    SwTxtFmtColl* pColl;
    for( sal_uInt16 nCnt = 0; nCnt < nCollCnt; ++nCnt )
        if(nCollId == (pColl = &pOldSh->GetTxtFmtColl(nCnt))->GetPoolFmtId())
            pNewSh->GetTxtCollFromPool(nCollId)->SetFmtAttr(pColl->GetAttrSet());
}

void SwModule::InsertEnv( SfxRequest& rReq )
{
    static sal_uInt16 nTitleNo = 0;

    SwDocShell      *pMyDocSh;
    SfxViewFrame    *pFrame;
    SwView          *pNewView;
    SwWrtShell      *pOldSh,
                    *pSh;

    // Get current shell
    pMyDocSh = (SwDocShell*) SfxObjectShell::Current();
    pOldSh   = pMyDocSh ? pMyDocSh->GetWrtShell() : 0;

    // Create new document (don't show!)
    SfxObjectShellLock xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
    xDocSh->DoInitNew( 0 );
    pFrame = SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
    pNewView = (SwView*) pFrame->GetViewShell();
    pNewView->AttrChangedNotify( &pNewView->GetWrtShell() ); // so that SelectShell is being called
    pSh = pNewView->GetWrtShellPtr();

    String aTmp( SW_RES(STR_ENV_TITLE) );
    aTmp += OUString::number( ++nTitleNo );
    xDocSh->SetTitle( aTmp );

    // if applicable, copy the old Collections "Sender" and "Receiver" to
    // a new document
    if ( pOldSh )
    {
        ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_JAKETADRESS);
        ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_SENDADRESS);
    }

    // Read SwEnvItem from config
    SwEnvCfgItem aEnvCfg;

    // Check if there's already an envelope.
    bool bEnvChange = false;

    SfxItemSet aSet(GetPool(), FN_ENVELOP, FN_ENVELOP, 0);
    aSet.Put(aEnvCfg.GetItem());

    SfxPrinter* pTempPrinter = pSh->getIDocumentDeviceAccess()->getPrinter( true );
    if(pOldSh )
    {
        const SwPageDesc& rCurPageDesc = pOldSh->GetPageDesc(pOldSh->GetCurPageDesc());
        OUString sJacket;
        SwStyleNameMapper::FillUIName( RES_POOLPAGE_JAKET, sJacket );
        bEnvChange = rCurPageDesc.GetName() == sJacket;

        IDocumentDeviceAccess* pIDDA_old = pOldSh->getIDocumentDeviceAccess();
        if( pIDDA_old->getPrinter( false ) )
        {
            IDocumentDeviceAccess* pIDDA = pSh->getIDocumentDeviceAccess();
            pIDDA->setJobsetup( *pIDDA_old->getJobsetup() );
            //#69563# if it isn't the same printer then the pointer has been invalidated!
            pTempPrinter = pIDDA->getPrinter( true );
        }
        pTempPrinter->SetPaperBin(rCurPageDesc.GetMaster().GetPaperBin().GetValue());

    }

    Window *pParent = pOldSh ? pOldSh->GetWin() : 0;
    SfxAbstractTabDialog * pDlg=NULL;
    short nMode = ENV_INSERT;

    SFX_REQUEST_ARG( rReq, pItem, SwEnvItem, FN_ENVELOP, sal_False );
    if ( !pItem )
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        pDlg = pFact->CreateSwEnvDlg( pParent, aSet, pOldSh, pTempPrinter, !bEnvChange );
        OSL_ENSURE(pDlg, "Dialogdiet fail!");
        nMode = pDlg->Execute();
    }
    else
    {
        SFX_REQUEST_ARG( rReq, pBoolItem, SfxBoolItem, FN_PARAM_1, sal_False );
        if ( pBoolItem && pBoolItem->GetValue() )
            nMode = ENV_NEWDOC;
    }

    if (nMode == ENV_NEWDOC || nMode == ENV_INSERT)
    {
        SwWait aWait( (SwDocShell&)*xDocSh, sal_True );

        // Read dialog and save item to config
        const SwEnvItem& rItem = pItem ? *pItem : (const SwEnvItem&) pDlg->GetOutputItemSet()->Get(FN_ENVELOP);
        aEnvCfg.GetItem() = rItem;
        aEnvCfg.Commit();

        // When we print we take the Jobsetup that is set up in the dialog.
        // Information has to be set here, before a possible destruction of
        // the new shell because the shell's printer has been handed to the
        // dialog.
        if ( nMode != ENV_NEWDOC )
        {
            OSL_ENSURE(pOldSh, "No document - wasn't 'Insert' disabled???");
            SvxPaperBinItem aItem( RES_PAPER_BIN );
            aItem.SetValue((sal_uInt8)pSh->getIDocumentDeviceAccess()->getPrinter(true)->GetPaperBin());
            pOldSh->GetPageDescFromPool(RES_POOLPAGE_JAKET)->GetMaster().SetFmtAttr(aItem);
        }

        SwWrtShell *pTmp = nMode == ENV_INSERT ? pOldSh : pSh;
        const SwPageDesc* pFollow = 0;
        SwTxtFmtColl *pSend = pTmp->GetTxtCollFromPool( RES_POOLCOLL_SENDADRESS ),
                     *pAddr = pTmp->GetTxtCollFromPool( RES_POOLCOLL_JAKETADRESS);
        const OUString sSendMark = pSend->GetName();
        const OUString sAddrMark = pAddr->GetName();

        if (nMode == ENV_INSERT)
        {

            SetView(&pOldSh->GetView()); // Set pointer to top view

            // Delete new document
            xDocSh->DoClose();
            pSh = pOldSh;
            //#i4251# selected text or objects in the document should
            //not be deleted on inserting envelopes
            pSh->EnterStdMode();
            // Here it goes (insert)
            pSh->StartUndo(UNDO_UI_INSERT_ENVELOPE, NULL);
            pSh->StartAllAction();
            pSh->SttEndDoc(sal_True);

            if (bEnvChange)
            {
                // followup template: page 2
                pFollow = pSh->GetPageDesc(pSh->GetCurPageDesc()).GetFollow();

                // Delete text from the first page
                if ( !pSh->SttNxtPg(sal_True) )
                    pSh->EndPg(sal_True);
                pSh->DelRight();
                // Delete frame of the first page
                if ( pSh->GotoFly(sSendMark) )
                {
                    pSh->EnterSelFrmMode();
                    pSh->DelRight();
                }
                if ( pSh->GotoFly(sAddrMark) )
                {
                    pSh->EnterSelFrmMode();
                    pSh->DelRight();
                }
                pSh->SttEndDoc(sal_True);
            }
            else
                // Followup template: page 1
                pFollow = &pSh->GetPageDesc(pSh->GetCurPageDesc());

            // Insert page break
            if ( pSh->IsCrsrInTbl() )
            {
                pSh->SplitNode();
                pSh->Right( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
                SfxItemSet aBreakSet( pSh->GetAttrPool(), RES_BREAK, RES_BREAK, 0 );
                aBreakSet.Put( SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK) );
                pSh->SetTblAttr( aBreakSet );
            }
            else
                pSh->InsertPageBreak(0, sal_False);
            pSh->SttEndDoc(sal_True);
        }
        else
        {
            pFollow = &pSh->GetPageDesc(pSh->GetCurPageDesc());
            // Let's go (print)
            pSh->StartAllAction();
            pSh->DoUndo(sal_False);

            // Again, copy the new collections "Sender" and "Receiver" to
            // a new document
            if ( pOldSh )
            {
                ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_JAKETADRESS);
                ::lcl_CopyCollAttr(pOldSh, pSh, RES_POOLCOLL_SENDADRESS);
            }
        }

        SET_CURR_SHELL(pSh);
        pSh->SetNewDoc();   // Avoid performance problems

        // Remember Flys of this site
        std::vector<SwFrmFmt*> aFlyArr;
        if( ENV_NEWDOC != nMode && !bEnvChange )
            pSh->GetPageObjs( aFlyArr );

        // Get page description
        SwPageDesc* pDesc = pSh->GetPageDescFromPool(RES_POOLPAGE_JAKET);
        SwFrmFmt&   rFmt  = pDesc->GetMaster();

        Printer *pPrt = pSh->getIDocumentDeviceAccess()->getPrinter( true );

    // Borders (are put together by Shift-Offset and alignment)
        Size aPaperSize = pPrt->PixelToLogic( pPrt->GetPaperSizePixel(),
                                              MAP_TWIP);
        if ( !aPaperSize.Width() && !aPaperSize.Height() )
                    aPaperSize = SvxPaperInfo::GetPaperSize(PAPER_A4);
        if ( aPaperSize.Width() > aPaperSize.Height() )
            Swap( aPaperSize );

        long lLeft  = rItem.lShiftRight,
             lUpper = rItem.lShiftDown;

        sal_uInt16 nPageW = (sal_uInt16) std::max(rItem.lWidth, rItem.lHeight),
               nPageH = (sal_uInt16) std::min(rItem.lWidth, rItem.lHeight);

        switch (rItem.eAlign)
        {
            case ENV_HOR_LEFT: break;
            case ENV_HOR_CNTR: lLeft  += std::max(0L, long(aPaperSize.Width() - nPageW)) / 2;
                               break;
            case ENV_HOR_RGHT: lLeft  += std::max(0L, long(aPaperSize.Width() - nPageW));
                               break;
            case ENV_VER_LEFT: lUpper += std::max(0L, long(aPaperSize.Width() - nPageH));
                               break;
            case ENV_VER_CNTR: lUpper += std::max(0L, long(aPaperSize.Width() - nPageH)) / 2;
                               break;
            case ENV_VER_RGHT: break;
        }
        SvxLRSpaceItem aLRMargin( RES_LR_SPACE );
        SvxULSpaceItem aULMargin( RES_UL_SPACE );
        aLRMargin.SetLeft ((sal_uInt16) lLeft );
        aULMargin.SetUpper((sal_uInt16) lUpper);
        aLRMargin.SetRight(0);
        aULMargin.SetLower(0);
        rFmt.SetFmtAttr(aLRMargin);
        rFmt.SetFmtAttr(aULMargin);

        // Header and footer
        rFmt.SetFmtAttr(SwFmtHeader(sal_False));
        pDesc->ChgHeaderShare(sal_False);
        rFmt.SetFmtAttr(SwFmtFooter(sal_False));
        pDesc->ChgFooterShare(sal_False);

        // Page numbering
        pDesc->SetUseOn(nsUseOnPage::PD_ALL);

        // Page size
        rFmt.SetFmtAttr(SwFmtFrmSize(ATT_FIX_SIZE,
                                            nPageW + lLeft, nPageH + lUpper));

        // Set type of page numbering
        SvxNumberType aType;
        aType.SetNumberingType(SVX_NUM_NUMBER_NONE);
        pDesc->SetNumType(aType);

        // Followup template
        if (pFollow)
            pDesc->SetFollow(pFollow);

        // Landscape
        pDesc->SetLandscape( rItem.eAlign >= ENV_VER_LEFT &&
                             rItem.eAlign <= ENV_VER_RGHT);

        // Apply page description

        sal_uInt16 nPos;
        pSh->FindPageDescByName( pDesc->GetName(),
                                    sal_False,
                                    &nPos );


        pSh->ChgPageDesc( nPos, *pDesc);
        pSh->ChgCurPageDesc(*pDesc);

        // Insert Frame
        SwFlyFrmAttrMgr aMgr(sal_False, pSh, FRMMGR_TYPE_ENVELP);
        SwFldMgr aFldMgr;
        aMgr.SetHeightSizeType(ATT_VAR_SIZE);

        // Overwrite defaults!
        aMgr.GetAttrSet().Put( SvxBoxItem(RES_BOX) );
        aMgr.SetULSpace( 0L, 0L );
        aMgr.SetLRSpace( 0L, 0L );

        // Sender
        if (rItem.bSend)
        {
            pSh->SttEndDoc(sal_True);
            aMgr.InsertFlyFrm(FLY_AT_PAGE,
                Point(rItem.lSendFromLeft + lLeft, rItem.lSendFromTop  + lUpper),
                Size (rItem.lAddrFromLeft - rItem.lSendFromLeft, 0));

            pSh->EnterSelFrmMode();
            pSh->SetFlyName(sSendMark);
            pSh->UnSelectFrm();
            pSh->LeaveSelFrmMode();
            pSh->SetTxtFmtColl( pSend );
            InsertLabEnvText( *pSh, aFldMgr, rItem.aSendText );
            aMgr.UpdateAttrMgr();
        }

        // Addressee
        pSh->SttEndDoc(sal_True);

        aMgr.InsertFlyFrm(FLY_AT_PAGE,
            Point(rItem.lAddrFromLeft + lLeft, rItem.lAddrFromTop  + lUpper),
            Size (nPageW - rItem.lAddrFromLeft - 566, 0));
        pSh->EnterSelFrmMode();
        pSh->SetFlyName(sAddrMark);
        pSh->UnSelectFrm();
        pSh->LeaveSelFrmMode();
        pSh->SetTxtFmtColl( pAddr );
        InsertLabEnvText(*pSh, aFldMgr, rItem.aAddrText);

        // Move Flys to the "old" pages
        if (!aFlyArr.empty())
            pSh->SetPageObjsNewPage(aFlyArr, 1);

        // Finished
        pSh->SttEndDoc(sal_True);

        pSh->EndAllAction();

        if (nMode == ENV_NEWDOC)
            pSh->DoUndo(sal_True);
        else
            pSh->EndUndo(UNDO_UI_INSERT_ENVELOPE);

        if (nMode == ENV_NEWDOC)
        {
            pFrame->GetFrame().Appear();

            if ( rItem.aAddrText.indexOf('<') >= 0 )
            {
                static sal_uInt16 const aInva[] =
                                    {
                                        SID_SBA_BRW_UPDATE,
                                        SID_SBA_BRW_INSERT,
                                        SID_SBA_BRW_MERGE,
                                        0
                                    };
                pFrame->GetBindings().Invalidate( aInva );

                // Open database beamer
                ShowDBObj(*pNewView, pSh->GetDBData());
            }
        }

        if ( !pItem )
        {
            rReq.AppendItem( rItem );
            if ( nMode == ENV_NEWDOC )
                rReq.AppendItem( SfxBoolItem( FN_PARAM_1, sal_True ) );
        }

        rReq.Done();
    }
    else    // Abort
    {
        rReq.Ignore();

        xDocSh->DoClose();
        --nTitleNo;

        // Set pointer to top view
        if (pOldSh)
            SetView(&pOldSh->GetView());
    }
    delete pDlg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
