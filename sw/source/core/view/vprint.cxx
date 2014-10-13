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

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/view/XRenderable.hpp>

#include <hintids.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/prnmon.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <svx/svdview.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/oldprintadaptor.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <wdocsh.hxx>
#include <fesh.hxx>
#include <pam.hxx>
#include <viewimp.hxx>
#include <layact.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <shellres.hxx>
#include <viewopt.hxx>
#include <printdata.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <ptqueue.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <viscrs.hxx>
#include <fmtpdsc.hxx>
#include <globals.hrc>
#include "PostItMgr.hxx"
#include <vprint.hxx>

using namespace ::com::sun::star;

/// Painting buffer
class SwQueuedPaint
{
public:
    SwQueuedPaint *pNext;
    SwViewShell      *pSh;
    SwRect          aRect;

    SwQueuedPaint( SwViewShell *pNew, const SwRect &rRect ) :
        pNext( 0 ),
        pSh( pNew ),
        aRect( rRect )
    {}
};

SwQueuedPaint *SwPaintQueue::pQueue = 0;

// saves some settings from the draw view
class SwDrawViewSave
{
    SdrView* pDV;
    bool bPrintControls;
public:
    SwDrawViewSave( SdrView* pSdrView );
    ~SwDrawViewSave();
};

void SwPaintQueue::Add( SwViewShell *pNew, const SwRect &rNew )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        while ( pPt->pSh != pNew && pPt->pNext )
            pPt = pPt->pNext;
        if ( pPt->pSh == pNew )
        {
            pPt->aRect.Union( rNew );
            return;
        }
    }
    SwQueuedPaint *pNQ = new SwQueuedPaint( pNew, rNew );
    if ( pPt )
        pPt->pNext = pNQ;
    else
        pQueue = pNQ;
}

void SwPaintQueue::Repaint()
{
    if ( !SwRootFrm::IsInPaint() && pQueue )
    {
        SwQueuedPaint *pPt = pQueue;
        do
        {   SwViewShell *pSh = pPt->pSh;
            SET_CURR_SHELL( pSh );
            if ( pSh->IsPreview() )
            {
                if ( pSh->GetWin() )
                {
                    // for previewing, since rows/columns are known in PaintHdl (UI)
                    pSh->GetWin()->Invalidate();
                    pSh->GetWin()->Update();
                }
            }
            else
                pSh->Paint( pPt->aRect.SVRect() );
            pPt = pPt->pNext;
        } while ( pPt );

        do
        {   pPt = pQueue;
            pQueue = pQueue->pNext;
            delete pPt;
        } while ( pQueue );
    }
}

void SwPaintQueue::Remove( SwViewShell *pSh )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        SwQueuedPaint *pPrev = 0;
        while ( pPt && pPt->pSh != pSh )
        {
            pPrev = pPt;
            pPt = pPt->pNext;
        }
        if ( pPt )
        {
            if ( pPrev )
                pPrev->pNext = pPt->pNext;
            else if ( pPt == pQueue )
                pQueue = 0;
            delete pPt;
        }
    }
}

void SetSwVisArea( SwViewShell *pSh, const SwRect &rRect )
{
    OSL_ENSURE( !pSh->GetWin(), "Drucken mit Window?" );
    pSh->maVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    // calculate an offset for the rectangle of the n-th page to
    // move the start point of the output operation to a position
    // such that in the output device all pages will be painted
    // at the same position
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();

    OutputDevice *pOut = pSh->GetOut();

    MapMode aMapMode( pOut->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    pOut->SetMapMode( aMapMode );
}

void SwViewShell::InitPrt( OutputDevice *pOutDev )
{
    // For printing we use a negative offset (exactly the offset of OutputSize).
    // This is necessary because the origin is in the upper left corner of the
    // physical page while the output uses OutputOffset as origin.
    if ( pOutDev )
    {
        maPrtOffst = Point();

        maPrtOffst += pOutDev->GetMapMode().GetOrigin();
        MapMode aMapMode( pOutDev->GetMapMode() );
        aMapMode.SetMapUnit( MAP_TWIP );
        pOutDev->SetMapMode( aMapMode );
        pOutDev->SetLineColor();
        pOutDev->SetFillColor();
    }
    else
        maPrtOffst.X() = maPrtOffst.Y() = 0;

    if ( !mpWin )
        mpOut = pOutDev;
}

void SwViewShell::ChgAllPageOrientation( Orientation eOri )
{
    OSL_ENSURE( mnStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    const sal_uInt16 nAll = GetDoc()->GetPageDescCnt();
    bool bNewOri = eOri != ORIENTATION_PORTRAIT;

    for( sal_uInt16 i = 0; i < nAll; ++ i )
    {
        const SwPageDesc& rOld = GetDoc()->GetPageDesc( i );

        if( rOld.GetLandscape() != bNewOri )
        {
            SwPageDesc aNew( rOld );
            {
                ::sw::UndoGuard const ug(GetDoc()->GetIDocumentUndoRedo());
                GetDoc()->CopyPageDesc(rOld, aNew);
            }
            aNew.SetLandscape( bNewOri );
            SwFrmFmt& rFmt = aNew.GetMaster();
            SwFmtFrmSize aSz( rFmt.GetFrmSize() );
            // adjust size
            // PORTRAIT  -> higher than wide
            // LANDSCAPE -> wider than high
            // Height is the VarSize, width the FixSize (per Def.)
            if( bNewOri ? aSz.GetHeight() > aSz.GetWidth()
                        : aSz.GetHeight() < aSz.GetWidth() )
            {
                SwTwips aTmp = aSz.GetHeight();
                aSz.SetHeight( aSz.GetWidth() );
                aSz.SetWidth( aTmp );
                rFmt.SetFmtAttr( aSz );
            }
            GetDoc()->ChgPageDesc( i, aNew );
        }
    }
}

void SwViewShell::ChgAllPageSize( Size &rSz )
{
    OSL_ENSURE( mnStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    SwDoc* pMyDoc = GetDoc();
    const sal_uInt16 nAll = pMyDoc->GetPageDescCnt();

    for( sal_uInt16 i = 0; i < nAll; ++i )
    {
        const SwPageDesc &rOld = pMyDoc->GetPageDesc( i );
        SwPageDesc aNew( rOld );
        {
            ::sw::UndoGuard const ug(GetDoc()->GetIDocumentUndoRedo());
            GetDoc()->CopyPageDesc( rOld, aNew );
        }
        SwFrmFmt& rPgFmt = aNew.GetMaster();
        Size aSz( rSz );
        const bool bOri = aNew.GetLandscape();
        if( bOri  ? aSz.Height() > aSz.Width()
                  : aSz.Height() < aSz.Width() )
        {
            SwTwips aTmp = aSz.Height();
            aSz.Height() = aSz.Width();
            aSz.Width()  = aTmp;
        }

        SwFmtFrmSize aFrmSz( rPgFmt.GetFrmSize() );
        aFrmSz.SetSize( aSz );
        rPgFmt.SetFmtAttr( aFrmSz );
        pMyDoc->ChgPageDesc( i, aNew );
    }
}

void SwViewShell::CalcPagesForPrint( sal_uInt16 nMax )
{
    SET_CURR_SHELL( this );

    SwRootFrm* pMyLayout = GetLayout();

    const SwFrm *pPage = pMyLayout->Lower();
    SwLayAction aAction( pMyLayout, Imp() );

    pMyLayout->StartAllAction();
    for ( sal_uInt16 i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        pPage->Calc();
        SwRect aOldVis( VisArea() );
        maVisArea = pPage->Frm();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( false );
        aAction.SetWaitAllowed( false );
        aAction.SetReschedule( true );

        aAction.Action();

        maVisArea = aOldVis; //reset due to the paints
        Imp()->SetFirstVisPageInvalid();
    }

    pMyLayout->EndAllAction();
}

SwDoc * SwViewShell::FillPrtDoc( SwDoc *pPrtDoc, const SfxPrinter* pPrt)
{
    OSL_ENSURE( this->IsA( TYPE(SwFEShell) ),"SwViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    pPrtDoc->getIDocumentFieldsAccess().LockExpFlds();

    // use given printer
    //! Make a copy of it since it gets destroyed with the temporary document
    //! used for PDF export
    if (pPrt)
        pPrtDoc->getIDocumentDeviceAccess().setPrinter( new SfxPrinter(*pPrt), true, true );

    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( sal_uInt16 nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
        if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
            pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

    // JP 29.07.99 - Bug 67951 - set all Styles from the SourceDoc into
    //                              the PrintDoc - will be replaced!
    pPrtDoc->ReplaceStyles( *GetDoc() );

    SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
    SwShellCrsr *pFirstCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetNext());
    if( !pActCrsr->HasMark() ) // with a multi-selection the current cursor might be empty
    {
        pActCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetPrev());
    }

    // Y-position of the first selection
    Point aSelPoint;
    if( pFESh->IsTableMode() )
    {
        SwShellTableCrsr* pShellTblCrsr = pFESh->GetTableCrsr();

        const SwCntntNode* pCntntNode = pShellTblCrsr->GetNode().GetCntntNode();
        const SwCntntFrm *pCntntFrm = pCntntNode ? pCntntNode->getLayoutFrm( GetLayout(), 0, pShellTblCrsr->Start() ) : 0;
        if( pCntntFrm )
        {
            SwRect aCharRect;
            SwCrsrMoveState aTmpState( MV_NONE );
            pCntntFrm->GetCharRect( aCharRect, *pShellTblCrsr->Start(), &aTmpState );
            aSelPoint = Point( aCharRect.Left(), aCharRect.Top() );
        }
    }
    else if (pFirstCrsr)
    {
       aSelPoint = pFirstCrsr->GetSttPos();
    }

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aSelPoint );
    OSL_ENSURE( pPage, "no page found!" );

    // get page descriptor - fall back to the first one if pPage could not be found
    const SwPageDesc* pPageDesc = pPage ? pPrtDoc->FindPageDesc(
        pPage->GetPageDesc()->GetName() ) : &pPrtDoc->GetPageDesc( (sal_uInt16)0 );

    if( !pFESh->IsTableMode() && pActCrsr && pActCrsr->HasMark() )
    {   // Tweak paragraph attributes of last paragraph
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
        SwCntntNode *pLastNd =
            pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );
        // copy the paragraph attributes of the first paragraph
        if( pLastNd && pLastNd->IsTxtNode() )
            ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
    }

    // fill it with the selected content
    pFESh->Copy( pPrtDoc );

    // set the page style at the first paragraph
    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx ); // go to 1st ContentNode
        if( pFESh->IsTableMode() )
        {
            SwTableNode* pTNd = pCNd->FindTableNode();
            if( pTNd )
                pTNd->GetTable().GetFrmFmt()->SetFmtAttr( SwFmtPageDesc( pPageDesc ) );
        }
        else
        {
            pCNd->SetAttr( SwFmtPageDesc( pPageDesc ) );
            if( pFirstCrsr && pFirstCrsr->HasMark() )
            {
                SwTxtNode *pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwCntntNode *pFirstNd =
                        pFirstCrsr->GetCntntNode( (*pFirstCrsr->GetMark()) > (*pFirstCrsr->GetPoint()) );
                    // copy paragraph attributes of the first paragraph
                    if( pFirstNd && pFirstNd->IsTxtNode() )
                        ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                }
            }
        }
    }
    return pPrtDoc;
}

// TODO: there is already a GetPageByPageNum, but it checks some physical page
// number; unsure if we want that here, should find out what that is...
SwPageFrm const*
sw_getPage(SwRootFrm const& rLayout, sal_Int32 const nPage)
{
    // yes this is O(n^2) but at least it does not crash...
    SwPageFrm const* pPage = dynamic_cast<const SwPageFrm*>(rLayout.Lower());
    for (sal_Int32 i = nPage; pPage && (i > 0); --i)
    {
        if (1 == i) { // note: nPage is 1-based, i.e. 0 is invalid!
            return pPage;
        }
        pPage = dynamic_cast<SwPageFrm const*>(pPage->GetNext());
    }
    OSL_ENSURE(pPage, "ERROR: SwPageFrm expected");
    OSL_FAIL("non-existent page requested");
    return 0;
}

bool SwViewShell::PrintOrPDFExport(
    OutputDevice *pOutDev,
    SwPrintData const& rPrintData,
    sal_Int32 nRenderer     /* the index in the vector of pages to be printed */ )
{
    // CAUTION: Do also always update the printing routines in viewpg.cxx (PrintProspect)!

    const sal_Int32 nMaxRenderer = rPrintData.GetRenderData().GetPagesToPrint().size() - 1;
    OSL_ENSURE( 0 <= nRenderer && nRenderer <= nMaxRenderer, "nRenderer out of bounds");
    if (!pOutDev || nMaxRenderer < 0 || nRenderer < 0 || nRenderer > nMaxRenderer)
        return false;

    // save settings of OutputDevice (should be done always since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Push();

    // fdo#36815 for comments in margins print to a metafile
    // and then scale that metafile down so that the comments
    // will fit on the real page, and replay that scaled
    // output to the real outputdevice
    GDIMetaFile *pOrigRecorder(NULL);
    GDIMetaFile *pMetaFile(NULL);
    sal_Int16 nPostItMode = rPrintData.GetPrintPostIts();
    if (nPostItMode == POSTITS_INMARGINS)
    {
        //get and disable the existing recorder
        pOrigRecorder = pOutDev->GetConnectMetaFile();
        pOutDev->SetConnectMetaFile(NULL);
        // turn off output to the device
        pOutDev->EnableOutput(false);
        // just record the rendering commands to the metafile
        // instead
        pMetaFile = new GDIMetaFile;
        pMetaFile->SetPrefSize(pOutDev->GetOutputSize());
        pMetaFile->SetPrefMapMode(pOutDev->GetMapMode());
        pMetaFile->Record(pOutDev);
    }

    // Print/PDF export for (multi-)selection has already generated a
    // temporary document with the selected text.
    // (see XRenderable implementation in unotxdoc.cxx)
    // It is implemented this way because PDF export calls this Prt function
    // once per page and we do not like to always have the temporary document
    // to be created that often here.
    SwViewShell *pShell = new SwViewShell(*this, 0, pOutDev);

    SdrView *pDrawView = pShell->GetDrawView();
    if (pDrawView)
    {
        pDrawView->SetBufferedOutputAllowed( false );
        pDrawView->SetBufferedOverlayAllowed( false );
    }

    {   // additional scope so that the CurrShell is reset before destroying the shell

        SET_CURR_SHELL( pShell );

        //JP 01.02.99: Bug 61335 - the ReadOnly flag is never copied
        if( mpOpt->IsReadonly() )
            pShell->mpOpt->SetReadonly( true );

        // save options at draw view:
        SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );

        pShell->PrepareForPrint( rPrintData );

        const sal_Int32 nPage = rPrintData.GetRenderData().GetPagesToPrint()[ nRenderer ];
        OSL_ENSURE( nPage < 0 ||
            rPrintData.GetRenderData().GetValidPagesSet().count( nPage ) == 1,
            "SwViewShell::PrintOrPDFExport: nPage not valid" );
        SwViewShell *const pViewSh2 = (nPage < 0)
                ? rPrintData.GetRenderData().m_pPostItShell.get()// post-it page
                : pShell; // a 'regular' page, not one from the post-it doc

        SwPageFrm const*const pStPage =
            sw_getPage(*pViewSh2->GetLayout(), abs(nPage));
        OSL_ENSURE( pStPage, "failed to get start page" );
        if (!pStPage)
        {
            return false;
        }

        //!! applying view options and formatting the document should now only be done in getRendererCount!

        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

        pShell->InitPrt(pOutDev);

        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

        pStPage->GetUpper()->Paint( pStPage->Frm(), &rPrintData );

        SwPaintQueue::Repaint();

        if (nPostItMode == POSTITS_INMARGINS)
        {
            SwPostItMgr *pPostItManager = pShell->GetPostItMgr();
            pPostItManager->CalcRects();
            pPostItManager->LayoutPostIts();
            pPostItManager->DrawNotesForPage(pOutDev, nPage-1);

            //Stop recording now
            pMetaFile->Stop();
            pMetaFile->WindStart();
            //Enable output to the device again
            pOutDev->EnableOutput(true);
            //Restore the original recorder
            pOutDev->SetConnectMetaFile(pOrigRecorder);

            //Now scale the recorded page down so the notes
            //will fit in the final page
            double fScale = 0.75;
            long nOrigHeight = pStPage->Frm().Height();
            long nNewHeight = nOrigHeight*fScale;
            long nShiftY = (nOrigHeight-nNewHeight)/2;
            pMetaFile->Scale( fScale, fScale );
            pMetaFile->WindStart();
            //Move the scaled page down to center it
            //the other variant of Move does not map pixels
            //back to the logical units correctly
            pMetaFile->Move(0, convertTwipToMm100(nShiftY), pOutDev->GetDPIX(), pOutDev->GetDPIY());
            pMetaFile->WindStart();

            //play back the scaled page
            pMetaFile->Play(pOutDev);
            delete pMetaFile;
        }
    }

    delete pShell;

    // restore settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Pop();

    return true;
}

void SwViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect )
{
    // For printing a shell is needed. Either the Doc already has one, than we
    // create a new view, or it has none, than we create the first view.
    SwViewShell *pSh;
    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
        pSh = new SwViewShell( *pDoc->getIDocumentLayoutAccess().GetCurrentViewShell(), 0, pOleOut,VSHELLFLAG_SHARELAYOUT );
    else
        pSh = new SwViewShell( *pDoc, 0, pOpt, pOleOut);

    {
        SET_CURR_SHELL( pSh );
        pSh->PrepareForPrint( rOptions );
        pSh->SetPrtFormatOption( true );

        SwRect aSwRect( rRect );
        pSh->maVisArea = aSwRect;

        if ( pSh->GetViewOptions()->getBrowseMode() &&
             pSh->GetNext() == pSh )
        {
            pSh->CheckBrowseView( false );
            pSh->GetLayout()->Lower()->InvalidateSize();
        }

        // CalcPagesForPrint() should not be necessary here. The pages in the
        // visible area will be formatted in SwRootFrm::Paint().
        // Removing this gives us a performance gain during saving the
        // document because the thumbnail creation will not trigger a complete
        // formatting of the document.

        pOleOut->Push( PushFlags::CLIPREGION );
        pOleOut->IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->Paint( aSwRect );

        pOleOut->Pop();
        // first the CurrShell object needs to be destroyed!
    }
    delete pSh;
}

/// Check if the DocNodesArray contains fields.
bool SwViewShell::IsAnyFieldInDoc() const
{
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = mpDoc->GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pItem = mpDoc->GetAttrPool().GetItem2( RES_TXTATR_FIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                return true;
            }
        }
    }

    nMaxItems = mpDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INPUTFIELD );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pItem = mpDoc->GetAttrPool().GetItem2( RES_TXTATR_INPUTFIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                return true;
            }
        }
    }

    return false;
}

///  Saves some settings at the draw view
SwDrawViewSave::SwDrawViewSave( SdrView* pSdrView )
    : pDV( pSdrView )
    , bPrintControls(true)
{
    if ( pDV )
    {
        bPrintControls = pDV->IsLayerPrintable( "Controls" );
    }
}

SwDrawViewSave::~SwDrawViewSave()
{
    if ( pDV )
    {
        pDV->SetLayerPrintable( "Controls", bPrintControls );
    }
}

// OD 09.01.2003 #i6467# - method also called for page preview
void SwViewShell::PrepareForPrint( const SwPrintData &rOptions )
 {
    mpOpt->SetGraphic  ( rOptions.bPrintGraphic );
    mpOpt->SetTable    ( rOptions.bPrintTable );
    mpOpt->SetDraw     ( rOptions.bPrintDraw  );
    mpOpt->SetControl  ( rOptions.bPrintControl );
    mpOpt->SetPageBack ( rOptions.bPrintPageBackground );
    mpOpt->SetBlackFont( rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        // OD 09.01.2003 #i6467# - consider, if view shell belongs to page preview
        if ( !IsPreview() )
        {
            pDrawView->SetLayerPrintable( "Controls", rOptions.bPrintControl );
        }
        else
        {
            pDrawView->SetLayerVisible( "Controls", rOptions.bPrintControl );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
