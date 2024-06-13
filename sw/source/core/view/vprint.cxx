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


#include <hintids.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdview.hxx>
#include <osl/diagnose.h>
#include <tools/UnitConversion.hxx>

#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <fesh.hxx>
#include <pam.hxx>
#include <viewimp.hxx>
#include <layact.hxx>
#include <ndtxt.hxx>
#include <viewopt.hxx>
#include <printdata.hxx>
#include <pagedesc.hxx>
#include <ptqueue.hxx>
#include <viscrs.hxx>
#include <fmtpdsc.hxx>
#include <PostItMgr.hxx>
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
        pNext( nullptr ),
        pSh( pNew ),
        aRect( rRect )
    {}
};

SwQueuedPaint *SwPaintQueue::s_pPaintQueue = nullptr;

namespace {

// saves some settings from the draw view
class SwDrawViewSave
{
    SdrView* pDV;
    bool bPrintControls;
public:
    explicit SwDrawViewSave( SdrView* pSdrView );
    ~SwDrawViewSave();
};

}

void SwPaintQueue::Add( SwViewShell *pNew, const SwRect &rNew )
{
    SwQueuedPaint *pPt = s_pPaintQueue;
    if (nullptr != pPt)
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
        s_pPaintQueue = pNQ;
}

void SwPaintQueue::Repaint()
{
    if (SwRootFrame::IsInPaint() || !s_pPaintQueue)
        return;

    SwQueuedPaint *pPt = s_pPaintQueue;
    do
    {   SwViewShell *pSh = pPt->pSh;
        CurrShell aCurr( pSh );
        if ( pSh->IsPreview() )
        {
            if ( pSh->GetWin() )
            {
                // for previewing, since rows/columns are known in PaintHdl (UI)
                pSh->GetWin()->Invalidate();
            }
        }
        else
            pSh->Paint(*pSh->GetOut(), pPt->aRect.SVRect());
        pPt = pPt->pNext;
    } while ( pPt );

    do
    {
        pPt = s_pPaintQueue;
        s_pPaintQueue = s_pPaintQueue->pNext;
        delete pPt;
    } while (s_pPaintQueue);
}

void SwPaintQueue::Remove( SwViewShell const *pSh )
{
    SwQueuedPaint *pPt = s_pPaintQueue;
    if (nullptr == pPt)
        return;

    SwQueuedPaint *pPrev = nullptr;
    while ( pPt && pPt->pSh != pSh )
    {
        pPrev = pPt;
        pPt = pPt->pNext;
    }
    if ( pPt )
    {
        if ( pPrev )
            pPrev->pNext = pPt->pNext;
        else if (pPt == s_pPaintQueue)
            s_pPaintQueue = nullptr;
        delete pPt;
    }
}

void SetSwVisArea( SwViewShell *pSh, const SwRect &rRect )
{
    OSL_ENSURE( !pSh->GetWin(), "Print with window?" );
    pSh->maVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    // calculate an offset for the rectangle of the n-th page to
    // move the start point of the output operation to a position
    // such that in the output device all pages will be painted
    // at the same position
    aPt.setX( -aPt.X() ); aPt.setY( -aPt.Y() );

    vcl::RenderContext *pOut = pSh->GetOut();

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
        maPrtOffset = Point();

        maPrtOffset += pOutDev->GetMapMode().GetOrigin();
        MapMode aMapMode( pOutDev->GetMapMode() );
        aMapMode.SetMapUnit( MapUnit::MapTwip );
        pOutDev->SetMapMode( aMapMode );
        pOutDev->SetLineColor();
        pOutDev->SetFillColor();
    }
    else
    {
        maPrtOffset.setX(0);
        maPrtOffset.setY(0);
    }

    if ( !mpWin )
        mpOut = pOutDev;
}

void SwViewShell::ChgAllPageOrientation( Orientation eOri )
{
    OSL_ENSURE( mnStartAction, "missing an Action" );
    CurrShell aCurr( this );

    const size_t nAll = GetDoc()->GetPageDescCnt();
    bool bNewOri = eOri != Orientation::Portrait;

    for( size_t i = 0; i < nAll; ++ i )
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
            SwFrameFormat& rFormat = aNew.GetMaster();
            SwFormatFrameSize aSz( rFormat.GetFrameSize() );
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
                rFormat.SetFormatAttr( aSz );
            }
            GetDoc()->ChgPageDesc( i, aNew );
        }
    }
}

void SwViewShell::ChgAllPageSize( Size const &rSz )
{
    OSL_ENSURE( mnStartAction, "missing an Action" );
    CurrShell aCurr( this );

    SwDoc* pMyDoc = GetDoc();
    const size_t nAll = pMyDoc->GetPageDescCnt();

    for( size_t i = 0; i < nAll; ++i )
    {
        const SwPageDesc &rOld = pMyDoc->GetPageDesc( i );
        SwPageDesc aNew( rOld );
        {
            ::sw::UndoGuard const ug(GetDoc()->GetIDocumentUndoRedo());
            GetDoc()->CopyPageDesc( rOld, aNew );
        }
        SwFrameFormat& rPgFormat = aNew.GetMaster();
        Size aSz( rSz );
        const bool bOri = aNew.GetLandscape();
        if( bOri  ? aSz.Height() > aSz.Width()
                  : aSz.Height() < aSz.Width() )
        {
            SwTwips aTmp = aSz.Height();
            aSz.setHeight( aSz.Width() );
            aSz.setWidth( aTmp );
        }

        SwFormatFrameSize aFrameSz( rPgFormat.GetFrameSize() );
        aFrameSz.SetSize( aSz );
        rPgFormat.SetFormatAttr( aFrameSz );
        pMyDoc->ChgPageDesc( i, aNew );
    }
}

void SwViewShell::CalcPagesForPrint( sal_uInt16 nMax )
{
    CurrShell aCurr( this );

    SwRootFrame* pMyLayout = GetLayout();

    const SwFrame *pPage = pMyLayout->Lower();
    SwLayAction aAction( pMyLayout, Imp() );

    pMyLayout->StartAllAction();
    for ( sal_uInt16 i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        pPage->Calc(GetOut());
        SwRect aOldVis( VisArea() );
        maVisArea = pPage->getFrameArea();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( false );
        aAction.SetWaitAllowed( false );
        aAction.SetReschedule( true );

        aAction.Action(GetOut());

        maVisArea = aOldVis; //reset due to the paints
        Imp()->SetFirstVisPageInvalid();
    }

    pMyLayout->EndAllAction();
}

void SwViewShell::FillPrtDoc( SwDoc& rPrtDoc, const SfxPrinter* pPrt)
{
    assert( dynamic_cast<const SwFEShell*>( this) && "SwViewShell::Prt for FEShell only");
    SwFEShell* pFESh = static_cast<SwFEShell*>(this);
    rPrtDoc.getIDocumentFieldsAccess().LockExpFields();

    // use given printer
    //! Make a copy of it since it gets destroyed with the temporary document
    //! used for PDF export
    if (pPrt)
        rPrtDoc.getIDocumentDeviceAccess().setPrinter( VclPtr<SfxPrinter>::Create(*pPrt), true, true );

    const SfxItemPool& rPool = GetAttrPool();
    for( sal_uInt16 nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
    {
        const SfxPoolItem* pCpyItem = rPool.GetUserDefaultItem( nWh );
        if( nullptr != pCpyItem )
            rPrtDoc.GetAttrPool().SetUserDefaultItem( *pCpyItem );
    }

    // JP 29.07.99 - Bug 67951 - set all Styles from the SourceDoc into
    //                              the PrintDoc - will be replaced!
    rPrtDoc.ReplaceStyles( *GetDoc() );

    SwShellCursor *pActCursor = pFESh->GetCursor_();
    SwShellCursor *pFirstCursor = pActCursor->GetNext();
    if( !pActCursor->HasMark() ) // with a multi-selection the current cursor might be empty
    {
        pActCursor = pActCursor->GetPrev();
    }

    // Y-position of the first selection
    Point aSelPoint;
    if( pFESh->IsTableMode() )
    {
        SwShellTableCursor* pShellTableCursor = pFESh->GetTableCursor();

        const SwContentNode *const pContentNode =
            pShellTableCursor->Start()->GetNode().GetContentNode();
        const SwContentFrame *const pContentFrame = pContentNode ? pContentNode->getLayoutFrame(GetLayout(), pShellTableCursor->Start()) : nullptr;
        if( pContentFrame )
        {
            SwRect aCharRect;
            SwCursorMoveState aTmpState( CursorMoveState::NONE );
            pContentFrame->GetCharRect( aCharRect, *pShellTableCursor->Start(), &aTmpState );
            aSelPoint = Point( aCharRect.Left(), aCharRect.Top() );
        }
    }
    else if (pFirstCursor)
    {
       aSelPoint = pFirstCursor->GetSttPos();
    }

    const SwPageFrame* pPage = GetLayout()->GetPageAtPos( aSelPoint );
    OSL_ENSURE( pPage, "no page found!" );

    // get page descriptor - fall back to the first one if pPage could not be found
    const SwPageDesc* pPageDesc = pPage ? rPrtDoc.FindPageDesc(
        pPage->GetPageDesc()->GetName() ) : &rPrtDoc.GetPageDesc( 0 );

    if( !pFESh->IsTableMode() && pActCursor && pActCursor->HasMark() )
    {   // Tweak paragraph attributes of last paragraph
        SwNodeIndex aNodeIdx( *rPrtDoc.GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTextNode* pTextNd = SwNodes::GoNext(&aNodeIdx)->GetTextNode();
        SwContentNode *pLastNd =
            (*pActCursor->GetMark()) <= (*pActCursor->GetPoint())
            ? pActCursor->GetPointContentNode()
            : pActCursor->GetMarkContentNode();
        // copy the paragraph attributes of the first paragraph
        if( pLastNd && pLastNd->IsTextNode() )
            static_cast<SwTextNode*>(pLastNd)->CopyCollFormat( *pTextNd );
    }

    // fill it with the selected content
    pFESh->Copy(rPrtDoc);

    // set the page style at the first paragraph
    {
        SwNodeIndex aNodeIdx( *rPrtDoc.GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwContentNode* pCNd = SwNodes::GoNext(&aNodeIdx); // go to 1st ContentNode
        if( pFESh->IsTableMode() )
        {
            SwTableNode* pTNd = pCNd->FindTableNode();
            if( pTNd )
                pTNd->GetTable().GetFrameFormat()->SetFormatAttr( SwFormatPageDesc( pPageDesc ) );
        }
        else
        {
            pCNd->SetAttr( SwFormatPageDesc( pPageDesc ) );
            if( pFirstCursor && pFirstCursor->HasMark() )
            {
                SwTextNode *pTextNd = pCNd->GetTextNode();
                if( pTextNd )
                {
                    SwContentNode *pFirstNd =
                        (*pFirstCursor->GetMark()) > (*pFirstCursor->GetPoint())
                        ? pFirstCursor->GetPointContentNode()
                        : pFirstCursor->GetMarkContentNode();
                    // copy paragraph attributes of the first paragraph
                    if( pFirstNd && pFirstNd->IsTextNode() )
                        static_cast<SwTextNode*>(pFirstNd)->CopyCollFormat( *pTextNd );
                }
            }
        }
    }
}

// TODO: there is already a GetPageByPageNum, but it checks some physical page
// number; unsure if we want that here, should find out what that is...
SwPageFrame const*
sw_getPage(SwRootFrame const& rLayout, sal_Int32 const nPage)
{
    // yes this is O(n^2) but at least it does not crash...
    SwPageFrame const* pPage = dynamic_cast<const SwPageFrame*>(rLayout.Lower());
    for (sal_Int32 i = nPage; pPage && (i > 0); --i)
    {
        if (1 == i) { // note: nPage is 1-based, i.e. 0 is invalid!
            return pPage;
        }
        pPage = dynamic_cast<SwPageFrame const*>(pPage->GetNext());
    }
    OSL_ENSURE(pPage, "ERROR: SwPageFrame expected");
    OSL_FAIL("non-existent page requested");
    return nullptr;
}

namespace sw
{
    // tdf#91680 Reserve space in margin for comments only if there are comments
    bool IsShrinkPageForPostIts(SwViewShell const& rShell, SwPrintData const& rPrintData)
    {
        SwPostItMode const nPostItMode(rPrintData.GetPrintPostIts());
        return nPostItMode == SwPostItMode::InMargins
            && sw_GetPostIts(rShell.GetDoc()->getIDocumentFieldsAccess(), nullptr);
    }
}

bool SwViewShell::PrintOrPDFExport(
    OutputDevice *pOutDev,
    SwPrintData const& rPrintData,
    sal_Int32 nRenderer,     /* the index in the vector of pages to be printed */
    bool bIsPDFExport )
{
    // CAUTION: Do also always update the printing routines in viewpg.cxx (PrintProspect)!

    const sal_Int32 nMaxRenderer = rPrintData.GetRenderData().GetPagesToPrint().size() - 1;
    OSL_ENSURE( 0 <= nRenderer && nRenderer <= nMaxRenderer, "nRenderer out of bounds");
    if (!pOutDev || nMaxRenderer < 0 || nRenderer < 0 || nRenderer > nMaxRenderer)
        return false;

    // save settings of OutputDevice (should be done always since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Push();


    const bool bHasPostItsToPrintInMargins(::sw::IsShrinkPageForPostIts(*this, rPrintData));
    ::std::optional<tools::Long> oOrigHeight;

    // Print/PDF export for (multi-)selection has already generated a
    // temporary document with the selected text.
    // (see XRenderable implementation in unotxdoc.cxx)
    // It is implemented this way because PDF export calls this Prt function
    // once per page and we do not like to always have the temporary document
    // to be created that often here.
    std::unique_ptr<SwViewShell> pShell(new SwViewShell(*this, nullptr, pOutDev));

    SdrView *pDrawView = pShell->GetDrawView();
    if (pDrawView)
    {
        pDrawView->SetBufferedOutputAllowed( false );
        pDrawView->SetBufferedOverlayAllowed( false );
    }

    {   // additional scope so that the CurrShell is reset before destroying the shell

        CurrShell aCurr( pShell.get() );

        //JP 01.02.99: Bug 61335 - the ReadOnly flag is never copied
        if( mpOpt->IsReadonly() )
            pShell->mpOpt->SetReadonly( true );

        // save options at draw view:
        SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );
        pShell->PrepareForPrint( rPrintData, bIsPDFExport );

        const sal_Int32 nPage = rPrintData.GetRenderData().GetPagesToPrint()[ nRenderer ];
        OSL_ENSURE( nPage < 0 ||
            rPrintData.GetRenderData().GetValidPagesSet().count( nPage ) == 1,
            "SwViewShell::PrintOrPDFExport: nPage not valid" );
        SwViewShell *const pViewSh2 = (nPage < 0)
                ? rPrintData.GetRenderData().m_pPostItShell.get()// post-it page
                : pShell.get(); // a 'regular' page, not one from the post-it doc

        SwPageFrame const*const pStPage =
            sw_getPage(*pViewSh2->GetLayout(), abs(nPage));
        OSL_ENSURE( pStPage, "failed to get start page" );
        if (!pStPage)
        {
            return false;
        }

        //!! applying view options and formatting the document should now only be done in getRendererCount!

        ::SetSwVisArea( pViewSh2, pStPage->getFrameArea() );

        pShell->InitPrt(pOutDev);

        ::SetSwVisArea( pViewSh2, pStPage->getFrameArea() );

        pStPage->GetUpper()->PaintSwFrame( *pOutDev, pStPage->getFrameArea() );

        SwPaintQueue::Repaint();

        SwPostItMgr *pPostItManager = bHasPostItsToPrintInMargins ? pShell->GetPostItMgr() : nullptr;

        if (pPostItManager)
        {
            pPostItManager->CalcRects();
            pPostItManager->LayoutPostIts();
            pPostItManager->DrawNotesForPage(pOutDev, nPage-1);
            oOrigHeight.emplace(pStPage->getFrameArea().Height());
        }
    }

    pShell.reset();

    // restore settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Pop();

    // avoid a warning about unbalanced Push/Pop by doing this last:
    if (oOrigHeight)
    {
        // fdo#36815 Now scale the recorded page down so the comments in
        // margins will fit in the final page
        double fScale = 0.75;
        tools::Long nNewHeight = *oOrigHeight*fScale;
        tools::Long nShiftY = (*oOrigHeight-nNewHeight)/2;
        GDIMetaFile *const pMetaFile = pOutDev->GetConnectMetaFile();
        pMetaFile->ScaleActions(fScale, fScale);
        //Move the scaled page down to center it
        //the other variant of Move does not map pixels
        //back to the logical units correctly
        pMetaFile->Move(0, convertTwipToMm100(nShiftY), pOutDev->GetDPIX(), pOutDev->GetDPIY());
    }

    return true;
}

void SwViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                           vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect,
                           bool bOutputForScreen )
{
    // For printing a shell is needed. Either the Doc already has one, then we
    // create a new view, or it has none, then we create the first view.
    std::unique_ptr<SwViewShell> pSh;
    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
        pSh.reset(new SwViewShell( *pDoc->getIDocumentLayoutAccess().GetCurrentViewShell(), nullptr, &rRenderContext,VSHELLFLAG_SHARELAYOUT ));
    else
        pSh.reset(new SwViewShell( *pDoc, nullptr, pOpt, &rRenderContext));

    pSh->setOutputToWindow(bOutputForScreen);

    {
        CurrShell aCurr( pSh.get() );
        pSh->PrepareForPrint( rOptions );
        pSh->SetPrtFormatOption( true );

        SwRect aSwRect( rRect );
        pSh->maVisArea = aSwRect;

        if ( pSh->GetViewOptions()->getBrowseMode() &&
             pSh->GetRingContainer().size() == 1 )
        {
            pSh->InvalidateLayout( false );
            pSh->GetLayout()->Lower()->InvalidateSize();
        }

        // CalcPagesForPrint() should not be necessary here. The pages in the
        // visible area will be formatted in SwRootFrame::PaintSwFrame().
        // Removing this gives us a performance gain during saving the
        // document because the thumbnail creation will not trigger a complete
        // formatting of the document.

        rRenderContext.Push( vcl::PushFlags::CLIPREGION );
        rRenderContext.IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->PaintSwFrame( rRenderContext, aSwRect );

        rRenderContext.Pop();
        // first the CurrShell object needs to be destroyed!
    }
}

/// Check if the DocNodesArray contains fields.
bool SwViewShell::IsAnyFieldInDoc() const
{
    ItemSurrogates aSurrogates;
    mxDoc->GetAttrPool().GetItemSurrogates(aSurrogates, RES_TXTATR_FIELD);
    for (const SfxPoolItem* pItem : aSurrogates)
    {
        auto pFormatField = dynamic_cast<const SwFormatField*>(pItem);
        if(pFormatField)
        {
            const SwTextField* pTextField = pFormatField->GetTextField();
            if( pTextField && pTextField->GetTextNode().GetNodes().IsDocNodes() )
            {
                return true;
            }
        }
    }

    mxDoc->GetAttrPool().GetItemSurrogates(aSurrogates, RES_TXTATR_INPUTFIELD);
    for (const SfxPoolItem* pItem : aSurrogates)
    {
        const SwFormatField* pFormatField = dynamic_cast<const SwFormatField*>(pItem);
        if(pFormatField)
        {
            const SwTextField* pTextField = pFormatField->GetTextField();
            if( pTextField && pTextField->GetTextNode().GetNodes().IsDocNodes() )
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
        bPrintControls = pDV->IsLayerPrintable( u"Controls"_ustr );
    }
}

SwDrawViewSave::~SwDrawViewSave()
{
    if ( pDV )
    {
        pDV->SetLayerPrintable( u"Controls"_ustr, bPrintControls );
    }
}

// OD 09.01.2003 #i6467# - method also called for page preview
void SwViewShell::PrepareForPrint( const SwPrintData &rOptions, bool bIsPDFExport )
 {
    mpOpt->SetGraphic  ( rOptions.m_bPrintGraphic );
    mpOpt->SetDraw     ( rOptions.m_bPrintGraphic );
    mpOpt->SetControl  ( rOptions.m_bPrintControl );
    mpOpt->SetPageBack ( rOptions.m_bPrintPageBackground );
    // Font should not be black if it's a PDF Export
    mpOpt->SetBlackFont( rOptions.m_bPrintBlackFont && !bIsPDFExport );

    if ( !HasDrawView() )
        return;

    SdrView *pDrawView = GetDrawView();
    // OD 09.01.2003 #i6467# - consider, if view shell belongs to page preview
    if ( !IsPreview() )
    {
        pDrawView->SetLayerPrintable( u"Controls"_ustr, rOptions.m_bPrintControl );
    }
    else
    {
        pDrawView->SetLayerVisible( u"Controls"_ustr, rOptions.m_bPrintControl );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
