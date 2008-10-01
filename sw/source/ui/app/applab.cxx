/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: applab.cxx,v $
 * $Revision: 1.34.46.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#if STLPORT_VERSION>=321
#include <cstdarg>
#endif


#include <hintids.hxx>

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/pbinitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/protitem.hxx>
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
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <fldbas.hxx>
#include <swundo.hxx>
#include <wrtsh.hxx>
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#include <fmtcol.hxx>
#include <expfld.hxx>
#include <fldmgr.hxx>
#include <label.hxx>
#include <labimg.hxx>
#include <section.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>

#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#include "swabstdlg.hxx"
#include "envelp.hrc"
#include <misc.hrc>

#include <IDocumentDeviceAccess.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

// steht im appenv.cxx
extern String InsertLabEnvText( SwWrtShell& , SwFldMgr& , const String& );

const char __FAR_DATA MASTER_LABEL[] = "MasterLabel";

// --------------------------------------------------------------------------

const SwFrmFmt *lcl_InsertBCText( SwWrtShell& rSh, const SwLabItem& rItem,
                        SwFrmFmt &rFmt,
                        sal_uInt16 nCol, sal_uInt16 nRow, sal_Bool bPage)
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    aSet.Put(SwFmtAnchor(bPage ? FLY_IN_CNTNT : FLY_PAGE, nPhyPageNum));
    if (!bPage)
    {
        aSet.Put(SwFmtHoriOrient(rItem.lLeft + nCol * rItem.lHDist,
                                                    text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
        aSet.Put(SwFmtVertOrient(rItem.lUpper + nRow * rItem.lVDist,
                                                    text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    }
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, sal_True,  &rFmt );  // Fly einfuegen
    ASSERT( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Rahmen wurde automatisch selektiert

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    //
    if(!rItem.bSynchron || !(nCol|nRow))
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
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
                        sal_uInt16 nCol, sal_uInt16 nRow, sal_Bool bLast, sal_Bool bPage)
{
    SfxItemSet aSet(rSh.GetAttrPool(), RES_ANCHOR, RES_ANCHOR,
                        RES_VERT_ORIENT, RES_VERT_ORIENT, RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
    sal_uInt16 nPhyPageNum, nVirtPageNum;
    rSh.GetPageNum( nPhyPageNum, nVirtPageNum );

    aSet.Put(SwFmtAnchor(bPage ? FLY_IN_CNTNT : FLY_PAGE, nPhyPageNum));
    if (!bPage)
    {
        aSet.Put(SwFmtHoriOrient(rItem.lLeft + nCol * rItem.lHDist,
                                                    text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
        aSet.Put(SwFmtVertOrient(rItem.lUpper + nRow * rItem.lVDist,
                                                    text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    }
    const SwFrmFmt *pFmt = rSh.NewFlyFrm(aSet, sal_True,  &rFmt );  // Fly einfuegen
    ASSERT( pFmt, "Fly not inserted" );

    rSh.UnSelectFrm();  //Rahmen wurde automatisch selektiert

    rSh.SetTxtFmtColl( rSh.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    // Ggf. "Naechster Datensatz"
    String sDBName;
    if( (!rItem.bSynchron || !(nCol|nRow)) && (sDBName = InsertLabEnvText( rSh, rFldMgr, rItem.aWriting )).Len() && !bLast )
    {
        sDBName.SetToken( 3, DB_DELIM, String::CreateFromAscii("True"));
        SwInsertFld_Data aData(TYP_DBNEXTSETFLD, 0, sDBName, aEmptyStr, 0, &rSh );
        rFldMgr.InsertFld( aData );
    }

    return pFmt;
}

// ----------------------------------------------------------------------------


void SwModule::InsertLab(SfxRequest& rReq, sal_Bool bLabel)
{
static sal_uInt16 nLabelTitleNo = 0;
static sal_uInt16 nBCTitleNo = 0;

    // DB-Manager anlegen
    SwNewDBMgr* pNewDBMgr = new SwNewDBMgr;

    // SwLabItem aus Config lesen
    SwLabCfgItem aLabCfg(bLabel);

    // Dialog hochfahren
    SfxItemSet aSet( GetPool(), FN_LABEL, FN_LABEL, 0 );
    aSet.Put( aLabCfg.GetItem() );

    SwAbstractDialogFactory* pDialogFactory = SwAbstractDialogFactory::Create();
    DBG_ASSERT(pDialogFactory, "SwAbstractDialogFactory fail!");

    AbstarctSwLabDlg* pDlg = pDialogFactory->CreateSwLabDlg( 0, aSet, pNewDBMgr, bLabel, DLG_LAB );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");

    if ( RET_OK == pDlg->Execute() )
    {
        // Dialog auslesen, Item in Config speichern
        const SwLabItem& rItem = (const SwLabItem&) pDlg->
                                            GetOutputItemSet()->Get(FN_LABEL);
        aLabCfg.GetItem() = rItem;
        aLabCfg.Commit();

        // Neues Dokument erzeugen.
        SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_STANDARD));
        xDocSh->DoInitNew( 0 );

        // Drucker
        Printer *pPrt = pDlg->GetPrt();
        if (pPrt)
        {
            SwDocShell *pDocSh = (SwDocShell*)(&*xDocSh);
            pDocSh->getIDocumentDeviceAccess()->setJobsetup(pPrt->GetJobSetup());
        }

        const SfxItemSet *pArgs = rReq.GetArgs();
        DBG_ASSERT( pArgs, "no arguments in SfxRequest")
        const SfxPoolItem* pFrameItem = 0;
        if(pArgs)
            pArgs->GetItemState(SID_DOCFRAME, FALSE, &pFrameItem);

        SfxViewFrame* pFrame = 0;
        if( pFrameItem )
        {
            SfxFrame* pFr = ((const SfxFrameItem*)pFrameItem)->GetFrame();
            xDocSh->PutItem(SfxBoolItem(SID_HIDDEN, TRUE));
            pFr->InsertDocument(xDocSh);
            pFrame = pFr->GetCurrentViewFrame();
        }
        else
        {
            pFrame = SfxViewFrame::CreateViewFrame( *xDocSh, 0, TRUE );
        }
        SwView      *pNewView = (SwView*) pFrame->GetViewShell();
        pNewView->AttrChangedNotify( &pNewView->GetWrtShell() );//Damit SelectShell gerufen wird.

        // Dokumenttitel setzen
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

        pFrame->GetFrame()->Appear();

        // Shell ermitteln
        SwWrtShell *pSh = pNewView->GetWrtShellPtr();
        ASSERT( pSh, "missing WrtShell" );

        {   // block for locks the dispatcher!!

            SwWait aWait( (SwDocShell&)*xDocSh, sal_True );

            SET_CURR_SHELL(pSh);
            pSh->SetLabelDoc(rItem.bSynchron);
            pSh->DoUndo( sal_False );
            pSh->StartAllAction();

            pSh->SetNewDoc();       // Performanceprobleme vermeiden

            SwPageDesc aDesc = pSh->GetPageDesc( 0 );
            SwFrmFmt&  rFmt  = aDesc.GetMaster();

            // Raender
            SvxLRSpaceItem aLRMargin( RES_LR_SPACE );
            SvxULSpaceItem aULMargin( RES_UL_SPACE );
            aLRMargin.SetLeft ((sal_uInt16) rItem.lLeft );
            aULMargin.SetUpper((sal_uInt16) rItem.lUpper);
            aLRMargin.SetRight(MINLAY/2);
            aULMargin.SetLower(MINLAY/2);
            rFmt.SetFmtAttr(aLRMargin);
            rFmt.SetFmtAttr(aULMargin);

            // Kopf- und Fusszeilen
            rFmt.SetFmtAttr(SwFmtHeader(sal_Bool(sal_False)));
            aDesc.ChgHeaderShare(sal_False);
            rFmt.SetFmtAttr(SwFmtFooter(sal_Bool(sal_False)));
            aDesc.ChgFooterShare(sal_False);


            aDesc.SetUseOn(nsUseOnPage::PD_ALL);                // Seitennumerierung

            // Einstellen der Seitengroesse
            rFmt.SetFmtAttr(SwFmtFrmSize(ATT_FIX_SIZE,
                                        rItem.lLeft  + rItem.nCols * rItem.lHDist + MINLAY,
                                        rItem.lUpper + rItem.nRows * rItem.lVDist + MINLAY));

            // Numerierungsart
            SvxNumberType aType;
            aType.SetNumberingType(SVX_NUM_NUMBER_NONE);
            aDesc.SetNumType( aType );

            // Folgevorlage
            const SwPageDesc &rFollow = pSh->GetPageDesc( pSh->GetCurPageDesc() );
            aDesc.SetFollow( &rFollow );

            pPrt = pSh->getIDocumentDeviceAccess()->getPrinter( true );
            SvxPaperBinItem aItem( RES_PAPER_BIN );
            aItem.SetValue((sal_Int8)pPrt->GetPaperBin());
            rFmt.SetFmtAttr(aItem);

            //determine orientation by calculating the width and height of the resulting page
            const int nResultWidth = rItem.lHDist * (rItem.nCols - 1) + rItem.lWidth + rItem.lLeft;
            const int nResultHeight = rItem.lVDist * (rItem.nRows - 1) + rItem.lHeight + rItem.lUpper;
            aDesc.SetLandscape(nResultWidth > nResultHeight);

            pSh->ChgPageDesc( 0, aDesc );

            // Rahmen einfuegen
            SwFldMgr*        pFldMgr = new SwFldMgr;
            pFldMgr->SetEvalExpFlds(sal_False);

            //fix(24446): Damit der Text der Ettiketten nicht im unbedruckbaren
            //Bereich landet stellen wir entsprechende Raender ein. Um das Handling
            //so Optimal wie moeglich zu halten stellen wir zunaechst an der
            //aktuellen Absatzvorlage keinen Rand als hartes Attribut ein (Damit die
            //Formatierung wg. der Zeichengeb. Rahmen passt. Dann stellen wir die
            //Standarabsatzvorlage anhand des unbedruckbaren Bereiches ein.
            const long nMin = pPrt->GetPageOffset().X() - rItem.lLeft;
            if ( nMin > 0 )
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                pSh->SetAttr( aLR );
                SwFmt *pStandard = pSh->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
                aLR.SetLeft ( sal_uInt16(nMin) );
                aLR.SetRight( sal_uInt16(nMin) );
                pStandard->SetFmtAttr( aLR );
            }

            // Rahmenvorlage vorbereiten
            SwFrmFmt* pFmt = pSh->GetFrmFmtFromPool( RES_POOLFRM_LABEL );
            SwFmtFrmSize aFrmSize(  ATT_FIX_SIZE,
                                    rItem.lHDist - (rItem.lHDist-rItem.lWidth),
                                    rItem.lVDist - (rItem.lVDist-rItem.lHeight));
            pFmt->SetFmtAttr(aFrmSize);

            SvxLRSpaceItem aFrmLRSpace( 0, (sal_uInt16)(rItem.lHDist - rItem.lWidth),
                                        0, 0,
                                        RES_LR_SPACE);
            pFmt->SetFmtAttr(aFrmLRSpace);

            SvxULSpaceItem aFrmULSpace( 0, (sal_uInt16)(rItem.lVDist - rItem.lHeight),
                                        RES_UL_SPACE);
            pFmt->SetFmtAttr(aFrmULSpace);

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
                        const SwFrmFmt *pTmp =
                                bLabel ?
                                lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr, j, i,
                                    i == rItem.nRows - 1 && j == rItem.nCols - 1,
                                    sal_True ) :
                                lcl_InsertBCText(*pSh, rItem, *pFmt, j, i, sal_True);
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

                                SwSection aSect( CONTENT_SECTION,
                                                String::CreateFromAscii(MASTER_LABEL));
                                pSh->InsertSection(aSect);
                            }
                        }
                        else if (rItem.bSynchron)
                        {
                            SwSection aSect(FILE_LINK_SECTION, pSh->GetUniqueSectionName());
                            String sLinkName(sfx2::cTokenSeperator);
                            sLinkName += sfx2::cTokenSeperator;
                            sLinkName += String::CreateFromAscii(MASTER_LABEL);
                            aSect.SetLinkFileName(sLinkName);
                            aSect.SetProtect();
                            pSh->Insert(aDotStr);   // Dummytext zum Zuweisen der Section
                            pSh->SttDoc();
                            pSh->EndDoc(sal_True);  // Alles im Rahmen selektieren
                            pSh->InsertSection(aSect);
                        }
                        pSh->Pop( sal_False );
                    }
                    if ( i + 1 != rItem.nRows )
                        pSh->SplitNode(); // Kleine Optimierung
                }
            }
            else
            {
                pFirstFlyFmt = bLabel ?
                    lcl_InsertLabText( *pSh, rItem, *pFmt, *pFldMgr,
                            static_cast< sal_uInt16 >(rItem.nCol - 1),
                            static_cast< sal_uInt16 >(rItem.nRow - 1), sal_True, sal_False ) :
                    lcl_InsertBCText(*pSh, rItem, *pFmt,
                            static_cast< sal_uInt16 >(rItem.nCol - 1),
                            static_cast< sal_uInt16 >(rItem.nRow - 1), sal_False);
            }

            //fill the user fields
            if(!bLabel)
            {
                uno::Reference< frame::XModel >  xModel = pSh->GetView().GetDocShell()->GetBaseModel();
                DBG_ASSERT(pDialogFactory, "SwAbstractDialogFactory fail!");
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
            // Datenbankbrowser mit zuletzt verwendeter Datenbank oeffnen
            ShowDBObj( *pNewView, pSh->GetDBData() );
        }

        if( rItem.bSynchron )
        {
            SfxDispatcher* pDisp = pFrame->GetDispatcher();
            ASSERT(pDisp, "Heute kein Dispatcher am Frame?");
            pDisp->Execute(FN_SYNC_LABELS, SFX_CALLMODE_ASYNCHRON);
        }
        rReq.SetReturnValue(SfxVoidItem(bLabel ? FN_LABEL : FN_BUSINESS_CARD));
    }
    delete pDlg;

    if( pNewDBMgr )
        delete pNewDBMgr;
}


