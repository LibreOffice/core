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


#include <cstdarg>

#include <hintids.hxx>

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

using namespace ::com::sun::star;
using ::rtl::OUString;

// is in appenv.cxx
extern String InsertLabEnvText( SwWrtShell& , SwFldMgr& , const String& );

const char MASTER_LABEL[] = "MasterLabel";

const SwFrmFmt *lcl_InsertBCText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt,
                        sal_uInt16 nCol, sal_uInt16 nRow )
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    //anchor frame to page
    aSet.Put( SwFmtAnchor( FLY_AT_PAGE, nPhyPageNum ) );
    aSet.Put( SwFmtHoriOrient( rItem.lLeft + nCol * rItem.lHDist,
                               text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    aSet.Put( SwFmtVertOrient( rItem.lUpper + nRow * rItem.lVDist,
                               text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, sal_True,  &rFmt );  // Insert Fly
    OSL_ENSURE( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Frame was selected automatically

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    if(!rItem.bSynchron || !(nCol|nRow))
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialogdiet fail!");
        ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
        if ( fnSetActGroup )
            (*fnSetActGroup)( rItem.sGlossaryGroup );
        SwGlossaryHdl* pGlosHdl = rSh.GetView().GetGlosHdl();
        pGlosHdl->SetCurGroup(rItem.sGlossaryGroup, sal_True);
        pGlosHdl->InsertGlossary( rItem.sGlossaryBlockName );
    }

    return pFmt;
}

const SwFrmFmt *lcl_InsertLabText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt, SwFldMgr& rFldMgr,
                        sal_uInt16 nCol, sal_uInt16 nRow, sal_Bool bLast )
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    //anchor frame to page
    aSet.Put( SwFmtAnchor( FLY_AT_PAGE, nPhyPageNum ) );
    aSet.Put( SwFmtHoriOrient( rItem.lLeft + nCol * rItem.lHDist,
                               text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    aSet.Put( SwFmtVertOrient( rItem.lUpper + nRow * rItem.lVDist,
                               text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ) );
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, sal_True,  &rFmt );  // Insert Fly
    OSL_ENSURE( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Frame was selected automatically

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    // If applicable "next dataset"
    String sDBName;
    if( (!rItem.bSynchron || !(nCol|nRow)) && (sDBName = InsertLabEnvText( rSh, rFldMgr, rItem.aWriting )).Len() && !bLast )
    {
        sDBName.SetToken( 3, DB_DELIM, rtl::OUString("True"));
        SwInsertFld_Data aData(TYP_DBNEXTSETFLD, 0, sDBName, aEmptyStr, 0, &rSh );
        rFldMgr.InsertFld( aData );
    }

    return pFmt;
}

void SwModule::InsertLab(SfxRequest& rReq, sal_Bool bLabel)
{
    static sal_uInt16 nLabelTitleNo = 0;
    static sal_uInt16 nBCTitleNo = 0;

    // Create DB-Manager
    SwNewDBMgr* pNewDBMgr = new SwNewDBMgr;

    // Read SwLabItem from Config
    SwLabCfgItem aLabCfg(bLabel);

    // Move up Dialog
    SfxItemSet aSet( GetPool(), FN_LABEL, FN_LABEL, 0 );
    aSet.Put( aLabCfg.GetItem() );

    SwAbstractDialogFactory* pDialogFactory = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pDialogFactory, "SwAbstractDialogFactory fail!");

    AbstractSwLabDlg* pDlg = pDialogFactory->CreateSwLabDlg( 0, aSet, pNewDBMgr, bLabel, DLG_LAB );
    OSL_ENSURE(pDlg, "Dialogdiet fail!");

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
        String aTmp;
        if(bLabel)
        {
            aTmp = String(SW_RES( STR_LAB_TITLE));
            aTmp += String::CreateFromInt32(++nLabelTitleNo );
        }
        else
        {
            aTmp = pDlg->GetBusinessCardStr();
            aTmp += String::CreateFromInt32( ++nBCTitleNo );
        }
        xDocSh->SetTitle( aTmp );

        pViewFrame->GetFrame().Appear();

        // Determine Shell
        SwWrtShell *pSh = pNewView->GetWrtShellPtr();
        OSL_ENSURE( pSh, "missing WrtShell" );

        {   // block for locks the dispatcher!!

            SwWait aWait( (SwDocShell&)*xDocSh, sal_True );

            SET_CURR_SHELL(pSh);
            pSh->SetLabelDoc(rItem.bSynchron);
            pSh->DoUndo( sal_False );
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
            rFmt.SetFmtAttr(SwFmtHeader(sal_Bool(sal_False)));
            aDesc.ChgHeaderShare(sal_False);
            rFmt.SetFmtAttr(SwFmtFooter(sal_Bool(sal_False)));
            aDesc.ChgFooterShare(sal_False);


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
            SwFldMgr*        pFldMgr = new SwFldMgr;
            pFldMgr->SetEvalExpFlds(sal_False);

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
                                sal_Bool bInFly = 0 != pSh->WizzardGetFly();
                                pSh->Pop( bInFly );

                                if( bInFly )
                                    pSh->EndDoc(sal_True);  // select all content
                                                        // in the fly
                                else
                                    pSh->SetMark();     // set only the mark

                                SwSectionData aSect(CONTENT_SECTION,
                                    rtl::OUString(MASTER_LABEL));
                                pSh->InsertSection(aSect);
                            }
                        }
                        else if (rItem.bSynchron)
                        {
                            SwSectionData aSect(FILE_LINK_SECTION,
                                    pSh->GetUniqueSectionName());
                            rtl::OUStringBuffer sLinkName;
                            sLinkName.append(sfx2::cTokenSeperator);
                            sLinkName.append(sfx2::cTokenSeperator);
                            sLinkName.append(MASTER_LABEL);
                            aSect.SetLinkFileName(sLinkName.makeStringAndClear());
                            aSect.SetProtectFlag(true);
                            pSh->Insert(aDotStr);   // Dummytext to allocate the Section
                            pSh->SttDoc();
                            pSh->EndDoc(sal_True);  // Select everything in the frame
                            pSh->InsertSection(aSect);
                        }
                        pSh->Pop( sal_False );
                    }
                }
            }
            else
            {
                pFirstFlyFmt = bLabel ?
                    lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr,
                            static_cast< sal_uInt16 >(rItem.nCol - 1),
                            static_cast< sal_uInt16 >(rItem.nRow - 1), sal_True ) :
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

            pFldMgr->SetEvalExpFlds(sal_True);
            pFldMgr->EvalExpFlds(pSh);

            delete pFldMgr;

            pSh->GotoFly(pFirstFlyFmt->GetName(), FLYCNTTYPE_ALL, sal_False);

            pSh->EndAllAction();
            pSh->DoUndo( sal_True );
        }

        if( rItem.aWriting.indexOf( '<' ) >= 0 )
        {
            // Open database browser on recently used database
            ShowDBObj( *pNewView, pSh->GetDBData() );
        }

        if( rItem.bSynchron )
        {
            SfxDispatcher* pDisp = pViewFrame->GetDispatcher();
            OSL_ENSURE(pDisp, "No dispatcher in frame?");
            pDisp->Execute(FN_SYNC_LABELS, SFX_CALLMODE_ASYNCHRON);
        }
        rReq.SetReturnValue(SfxVoidItem(bLabel ? FN_LABEL : FN_BUSINESS_CARD));
    }
    delete pDlg;
    delete pNewDBMgr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
