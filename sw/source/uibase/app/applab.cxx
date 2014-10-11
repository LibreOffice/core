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

#include <config_features.h>

#include <cstdarg>

#include <hintids.hxx>

#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/protitem.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <fmthdft.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <swwait.hxx>
#include <gloshdl.hxx>
#include <mdiexp.hxx>
#include <frmatr.hxx>
#include <paratr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <fldbas.hxx>
#include <swundo.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>
#include <dbmgr.hxx>
#include <fmtcol.hxx>
#include <expfld.hxx>
#include <fldmgr.hxx>
#include <label.hxx>
#include <labimg.hxx>
#include <section.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>

#include <app.hrc>
#include <poolfmt.hrc>
#include "swabstdlg.hxx"
#include "envelp.hrc"
#include <misc.hrc>

#include <IDocumentDeviceAccess.hxx>

#include <boost/scoped_ptr.hpp>
#include <appenv.hxx>

using namespace ::com::sun::star;

const char MASTER_LABEL[] = "MasterLabel";

static const SwFrmFmt *lcl_InsertBCText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt,
                        sal_uInt16 nCol, sal_uInt16 nRow )
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    //anchor frame to page
    aSet.Put( SwFmtAnchor( FLY_AT_PAGE, nPhyPageNum ) );
    aSet.Put( SwFmtHoriOrient( rItem.lLeft + static_cast<SwTwips>(nCol) * rItem.lHDist,
                               text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    aSet.Put( SwFmtVertOrient( rItem.lUpper + static_cast<SwTwips>(nRow) * rItem.lVDist,
                               text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, true,  &rFmt );  // Insert Fly
    OSL_ENSURE( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Frame was selected automatically

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    if(!rItem.bSynchron || !(nCol|nRow))
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialog creation failed!");
        ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
        if ( fnSetActGroup )
            (*fnSetActGroup)( rItem.sGlossaryGroup );
        SwGlossaryHdl* pGlosHdl = rSh.GetView().GetGlosHdl();
        pGlosHdl->SetCurGroup(rItem.sGlossaryGroup, true);
        pGlosHdl->InsertGlossary( rItem.sGlossaryBlockName );
    }

    return pFmt;
}

static const SwFrmFmt *lcl_InsertLabText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt, SwFldMgr& rFldMgr,
                        sal_uInt16 nCol, sal_uInt16 nRow, bool bLast )
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    //anchor frame to page
    aSet.Put( SwFmtAnchor( FLY_AT_PAGE, nPhyPageNum ) );
    aSet.Put( SwFmtHoriOrient( rItem.lLeft + static_cast<SwTwips>(nCol) * rItem.lHDist,
                               text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    aSet.Put( SwFmtVertOrient( rItem.lUpper + static_cast<SwTwips>(nRow) * rItem.lVDist,
                               text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, true,  &rFmt );  // Insert Fly
    OSL_ENSURE( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Frame was selected automatically

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    // If applicable "next dataset"
    OUString sDBName;
    if( (!rItem.bSynchron || !(nCol|nRow)) && !(sDBName = InsertLabEnvText( rSh, rFldMgr, rItem.aWriting )).isEmpty() && !bLast )
    {
        sDBName = comphelper::string::setToken(sDBName, 3, DB_DELIM, "True");
        SwInsertFld_Data aData(TYP_DBNEXTSETFLD, 0, sDBName, aEmptyOUStr, 0, &rSh );
        rFldMgr.InsertFld( aData );
    }

    return pFmt;
}

void SwModule::InsertLab(SfxRequest& rReq, bool bLabel)
{
    static sal_uInt16 nLabelTitleNo = 0;
    static sal_uInt16 nBCTitleNo = 0;

#if HAVE_FEATURE_DBCONNECTIVITY
    // Create DB-Manager
    boost::scoped_ptr<SwDBManager> pDBManager(new SwDBManager);
#endif

    // Read SwLabItem from Config
    SwLabCfgItem aLabCfg(bLabel);

    // Move up Dialog
    SfxItemSet aSet( GetPool(), FN_LABEL, FN_LABEL, 0 );
    aSet.Put( aLabCfg.GetItem() );

    SwAbstractDialogFactory* pDialogFactory = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pDialogFactory, "SwAbstractDialogFactory fail!");

    boost::scoped_ptr<AbstractSwLabDlg> pDlg(pDialogFactory->CreateSwLabDlg(0, aSet,
#if HAVE_FEATURE_DBCONNECTIVITY
                                                                            pDBManager.get(),
#else
                                                                            NULL,
#endif
                                                                            bLabel));
    OSL_ENSURE(pDlg, "Dialog creation failed!");

    if ( RET_OK == pDlg->Execute() )
    {
        // Read dialog, store item in config
        const SwLabItem& rItem = (const SwLabItem&) pDlg->
                                            GetOutputItemSet()->Get(FN_LABEL);
        aLabCfg.GetItem() = rItem;
        aLabCfg.Commit();

        // Create new document
        SfxObjectShellLock xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD));
        xDocSh->DoInitNew( 0 );

        // Printer
        Printer *pPrt = pDlg->GetPrt();
        if (pPrt)
        {
            SwDocShell *pDocSh = (SwDocShell*)(&*xDocSh);
            pDocSh->getIDocumentDeviceAccess()->setJobsetup(pPrt->GetJobSetup());
        }

        SfxViewFrame* pViewFrame = SfxViewFrame::DisplayNewDocument( *xDocSh, rReq );

        SwView      *pNewView = (SwView*) pViewFrame->GetViewShell();
        pNewView->AttrChangedNotify( &pNewView->GetWrtShell() );// So that SelectShell is being called.

        // Set document title
        OUString aTmp;
        if(bLabel)
        {
            aTmp = SW_RES( STR_LAB_TITLE);
            aTmp += OUString::number(++nLabelTitleNo );
        }
        else
        {
            aTmp = pDlg->GetBusinessCardStr();
            aTmp += OUString::number( ++nBCTitleNo );
        }
        xDocSh->SetTitle( aTmp );

        pViewFrame->GetFrame().Appear();

        // Determine Shell
        SwWrtShell *pSh = pNewView->GetWrtShellPtr();
        OSL_ENSURE( pSh, "missing WrtShell" );

        {   // block for locks the dispatcher!!

            SwWait aWait( (SwDocShell&)*xDocSh, true );

            SET_CURR_SHELL(pSh);
            pSh->SetLabelDoc(rItem.bSynchron);
            pSh->DoUndo( false );
            pSh->StartAllAction();

            pSh->SetNewDoc();       // Avoid performance problems

            SwPageDesc aDesc = pSh->GetPageDesc( 0 );
            SwFrmFmt&  rFmt  = aDesc.GetMaster();

            // Borders
            SvxLRSpaceItem aLRMargin( RES_LR_SPACE );
            SvxULSpaceItem aULMargin( RES_UL_SPACE );
            aLRMargin.SetLeft ((sal_uInt16) rItem.lLeft );
            aULMargin.SetUpper((sal_uInt16) rItem.lUpper);
            aLRMargin.SetRight( 0 );
            aULMargin.SetLower( 0 );
            rFmt.SetFmtAttr(aLRMargin);
            rFmt.SetFmtAttr(aULMargin);

            // Header and footer
            rFmt.SetFmtAttr(SwFmtHeader(false));
            aDesc.ChgHeaderShare(false);
            rFmt.SetFmtAttr(SwFmtFooter(false));
            aDesc.ChgFooterShare(false);

            aDesc.SetUseOn(nsUseOnPage::PD_ALL);                // Site numbering

            // Set page size
            long lPgWidth, lPgHeight;
            lPgWidth  = (rItem.lPWidth > MINLAY ? rItem.lPWidth : MINLAY);
            lPgHeight = (rItem.lPHeight > MINLAY ? rItem.lPHeight : MINLAY);
            rFmt.SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE, lPgWidth, lPgHeight ));
            // Numbering type
            SvxNumberType aType;
            aType.SetNumberingType(SVX_NUM_NUMBER_NONE);
            aDesc.SetNumType( aType );

            // Followup template
            const SwPageDesc &rFollow = pSh->GetPageDesc( pSh->GetCurPageDesc() );
            aDesc.SetFollow( &rFollow );

            pPrt = pSh->getIDocumentDeviceAccess()->getPrinter( true );
            SvxPaperBinItem aItem( RES_PAPER_BIN );
            aItem.SetValue((sal_Int8)pPrt->GetPaperBin());
            rFmt.SetFmtAttr(aItem);

            // Determine orientation of the resulting page
            aDesc.SetLandscape(rItem.lPWidth > rItem.lPHeight);

            pSh->ChgPageDesc( 0, aDesc );

            // Insert frame
            boost::scoped_ptr<SwFldMgr> pFldMgr(new SwFldMgr);
            pFldMgr->SetEvalExpFlds(false);

            // Prepare border template
            SwFrmFmt* pFmt = pSh->GetFrmFmtFromPool( RES_POOLFRM_LABEL );
            sal_Int32 iResultWidth = rItem.lLeft + (rItem.nCols - 1) * rItem.lHDist + rItem.lWidth - rItem.lPWidth;
            sal_Int32 iResultHeight = rItem.lUpper + (rItem.nRows - 1) * rItem.lVDist + rItem.lHeight - rItem.lPHeight;
            sal_Int32 iWidth = (iResultWidth > 0 ? rItem.lWidth - (iResultWidth / rItem.nCols) - 1 : rItem.lWidth);
            sal_Int32 iHeight = (iResultHeight > 0 ? rItem.lHeight - (iResultHeight / rItem.nRows) - 1 : rItem.lHeight);
            SwFmtFrmSize aFrmSize(  ATT_FIX_SIZE, iWidth, iHeight );
            pFmt->SetFmtAttr( aFrmSize );

            //frame represents label itself, no border space
            SvxULSpaceItem aFrmNoULSpace( 0, 0, RES_UL_SPACE );
            SvxLRSpaceItem aFrmNoLRSpace( 0, 0, 0, 0, RES_LR_SPACE );
            pFmt->SetFmtAttr( aFrmNoULSpace );
            pFmt->SetFmtAttr( aFrmNoLRSpace );

            const SwFrmFmt *pFirstFlyFmt = 0;
            if ( rItem.bPage )
            {
                SwFmtVertOrient aFrmVertOrient( pFmt->GetVertOrient() );
                aFrmVertOrient.SetVertOrient( text::VertOrientation::TOP );
                pFmt->SetFmtAttr(aFrmVertOrient);

                for ( sal_uInt16 i = 0; i < rItem.nRows; ++i )
                {
                    for ( sal_uInt16 j = 0; j < rItem.nCols; ++j )
                    {
                        pSh->Push();
                        const SwFrmFmt *pTmp = ( bLabel ?
                                                 lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr, j, i,
                                                   i == rItem.nRows - 1 && j == rItem.nCols - 1 ) :
                                                 lcl_InsertBCText( *pSh, rItem, *pFmt, j, i ) );
                        if (!(i|j))
                        {
                            pFirstFlyFmt = pTmp;

                            if (rItem.bSynchron)
                            {
                                // if there is no content in the fly then
                                // dont leave the fly!!!
                                pSh->Push();
                                pSh->SttDoc();
                                bool bInFly = 0 != pSh->WizzardGetFly();
                                pSh->Pop( bInFly );

                                if( bInFly )
                                    pSh->EndDoc(true);  // select all content
                                                        // in the fly
                                else
                                    pSh->SetMark();     // set only the mark

                                SwSectionData aSect(CONTENT_SECTION,
                                    OUString(MASTER_LABEL));
                                pSh->InsertSection(aSect);
                            }
                        }
                        else if (rItem.bSynchron)
                        {
                            SwSectionData aSect(FILE_LINK_SECTION,
                                    pSh->GetUniqueSectionName());
                            OUStringBuffer sLinkName;
                            sLinkName.append(sfx2::cTokenSeparator);
                            sLinkName.append(sfx2::cTokenSeparator);
                            sLinkName.append(MASTER_LABEL);
                            aSect.SetLinkFileName(sLinkName.makeStringAndClear());
                            aSect.SetProtectFlag(true);
                            pSh->Insert(".");   // Dummytext to allocate the Section
                            pSh->SttDoc();
                            pSh->EndDoc(true);  // Select everything in the frame
                            pSh->InsertSection(aSect);
                        }
                        pSh->Pop( false );
                    }
                }
            }
            else
            {
                pFirstFlyFmt = bLabel ?
                    lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr,
                            static_cast< sal_uInt16 >(rItem.nCol - 1),
                            static_cast< sal_uInt16 >(rItem.nRow - 1), true ) :
                    lcl_InsertBCText(*pSh, rItem, *pFmt,
                            static_cast< sal_uInt16 >(rItem.nCol - 1),
                            static_cast< sal_uInt16 >(rItem.nRow - 1));
            }

            //fill the user fields
            if(!bLabel)
            {
                uno::Reference< frame::XModel >  xModel = pSh->GetView().GetDocShell()->GetBaseModel();
                OSL_ENSURE(pDialogFactory, "SwAbstractDialogFactory fail!");
                SwLabDlgMethod SwLabDlgUpdateFieldInformation = pDialogFactory->GetSwLabDlgStaticMethod ();
                SwLabDlgUpdateFieldInformation(xModel, rItem);
            }

            pFldMgr->SetEvalExpFlds(true);
            pFldMgr->EvalExpFlds(pSh);

            pFldMgr.reset();

            if (pFirstFlyFmt)
                pSh->GotoFly(pFirstFlyFmt->GetName(), FLYCNTTYPE_ALL, false);

            pSh->EndAllAction();
            pSh->DoUndo( true );
        }

        if( rItem.aWriting.indexOf( '<' ) >= 0 )
        {
            // Open database browser on recently used database
            ShowDBObj( *pNewView, pSh->GetDBData() );
        }

        if( rItem.bSynchron )
        {
            SfxDispatcher* pDisp = pViewFrame->GetDispatcher();
            assert(pDisp && "No dispatcher in frame?");
            pDisp->Execute(FN_SYNC_LABELS, SfxCallMode::ASYNCHRON);
        }
        rReq.SetReturnValue(SfxVoidItem(bLabel ? FN_LABEL : FN_BUSINESS_CARD));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
