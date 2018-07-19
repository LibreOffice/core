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

#include <ctime>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewimp.hxx>
#include <crsrsh.hxx>
#include <dflyobj.hxx>
#include <frmtool.hxx>
#include <dcontact.hxx>
#include <viewopt.hxx>
#include <dbg_lay.hxx>
#include <layouter.hxx>
#include <docstat.hxx>
#include <swevent.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <sfx2/event.hxx>

#include <ftnidx.hxx>
#include <vcl/svapp.hxx>
#include <editeng/opaqitem.hxx>
#include <SwSmartTagMgr.hxx>

#include <layact.hxx>
#include <swwait.hxx>
#include <fmtsrnd.hxx>
#include <docsh.hxx>

#include <tabfrm.hxx>
#include <ftnfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <flyfrms.hxx>
#include <mdiexp.hxx>
#include <sectfrm.hxx>
#include <acmplwrd.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <vector>

// Save some typing work to avoid accessing destroyed pages.
#define XCHECKPAGE \
            {   if ( IsAgain() ) \
                { \
                    if( bNoLoop ) \
                        rLayoutAccess.GetLayouter()->EndLoopControl(); \
                    return; \
                } \
            }

void SwLayAction::CheckWaitCursor()
{
    if (IsReschedule())
    {
        ::RescheduleProgress(m_pImp->GetShell()->GetDoc()->GetDocShell());
    }
    if ( !m_pWait && IsWaitAllowed() && IsPaint() &&
         ((std::clock() - m_nStartTicks) * 1000 / CLOCKS_PER_SEC >= CLOCKS_PER_SEC/2) )
    {
        m_pWait.reset( new SwWait( *m_pRoot->GetFormat()->GetDoc()->GetDocShell(), true ) );
    }
}

// Time over already?
inline void SwLayAction::CheckIdleEnd()
{
    if ( !IsInput() )
        m_bInput = bool(GetInputType()) && Application::AnyInput( GetInputType() );
}

void SwLayAction::SetStatBar( bool bNew )
{
    if ( bNew )
    {
        m_nEndPage = m_pRoot->GetPageNum();
        m_nEndPage += m_nEndPage * 10 / 100;
    }
    else
        m_nEndPage = USHRT_MAX;
}

bool SwLayAction::PaintWithoutFlys( const SwRect &rRect, const SwContentFrame *pCnt,
                                    const SwPageFrame *pPage )
{
    SwRegionRects aTmp( rRect );
    const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrame *pSelfFly = pCnt->FindFlyFrame();

    for ( size_t i = 0; i < rObjs.size() && !aTmp.empty(); ++i )
    {
        SdrObject *pO = rObjs[i]->DrawObj();
        if ( dynamic_cast< const SwVirtFlyDrawObj *>( pO ) ==  nullptr )
            continue;

        // do not consider invisible objects
        const IDocumentDrawModelAccess& rIDDMA = pPage->GetFormat()->getIDocumentDrawModelAccess();
        if ( !rIDDMA.IsVisibleLayerId( pO->GetLayer() ) )
        {
            continue;
        }

        SwFlyFrame *pFly = static_cast<SwVirtFlyDrawObj*>(pO)->GetFlyFrame();

        if ( pFly == pSelfFly || !rRect.IsOver( pFly->getFrameArea() ) )
            continue;

        if ( pSelfFly && pSelfFly->IsLowerOf( pFly ) )
            continue;

        if ( pFly->GetVirtDrawObj()->GetLayer() == rIDDMA.GetHellId() )
            continue;

        if ( pSelfFly )
        {
            const SdrObject *pTmp = pSelfFly->GetVirtDrawObj();
            if ( pO->GetLayer() == pTmp->GetLayer() )
            {
                if ( pO->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
                    // Only look at things above us, if inside the same layer
                    continue;
            }
            else
            {
                const bool bLowerOfSelf = pFly->IsLowerOf( pSelfFly );
                if ( !bLowerOfSelf && !pFly->GetFormat()->GetOpaque().GetValue() )
                    // Things from other layers are only interesting to us if
                    // they're not transparent or lie inwards
                    continue;
            }
        }

        //     Fly frame without a lower have to be subtracted from paint region.
        //     For checking, if fly frame contains transparent graphic or
        //     has surrounded contour, assure that fly frame has a lower
        if ( pFly->Lower() &&
             pFly->Lower()->IsNoTextFrame() &&
             ( static_cast<SwNoTextFrame*>(pFly->Lower())->IsTransparent() ||
               pFly->GetFormat()->GetSurround().IsContour() )
           )
        {
            continue;
        }

        //     vcl::Region of a fly frame with transparent background or a transparent
        //     shadow have not to be subtracted from paint region
        if ( pFly->IsBackgroundTransparent() )
        {
            continue;
        }

        aTmp -= pFly->getFrameArea();
    }

    bool bRetPaint = false;
    for ( SwRects::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it )
        bRetPaint |= m_pImp->GetShell()->AddPaintRect( *it );
    return bRetPaint;
}

inline bool SwLayAction::PaintContent_( const SwContentFrame *pContent,
                                      const SwPageFrame *pPage,
                                      const SwRect &rRect )
{
    if ( rRect.HasArea() )
    {
        if ( pPage->GetSortedObjs() )
            return PaintWithoutFlys( rRect, pContent, pPage );
        else
            return m_pImp->GetShell()->AddPaintRect( rRect );
    }
    return false;
}

/**
 * Depending of the type, the Content is output according to its changes, or the area
 * to be outputted is registered with the region, respectively.
 */
void SwLayAction::PaintContent( const SwContentFrame *pCnt,
                              const SwPageFrame *pPage,
                              const SwRect &rOldRect,
                              long nOldBottom )
{
    SwRectFnSet aRectFnSet(pCnt);

    if ( pCnt->IsCompletePaint() || !pCnt->IsTextFrame() )
    {
        SwRect aPaint( pCnt->GetPaintArea() );
        if ( !PaintContent_( pCnt, pPage, aPaint ) )
            pCnt->ResetCompletePaint();
    }
    else
    {
        // paint the area between printing bottom and frame bottom and
        // the area left and right beside the frame, if its height changed.
        long nOldHeight = aRectFnSet.GetHeight(rOldRect);
        long nNewHeight = aRectFnSet.GetHeight(pCnt->getFrameArea());
        const bool bHeightDiff = nOldHeight != nNewHeight;
        if( bHeightDiff )
        {
            // consider whole potential paint area.
            SwRect aDrawRect( pCnt->GetPaintArea() );
            if( nOldHeight > nNewHeight )
                nOldBottom = aRectFnSet.GetPrtBottom(*pCnt);
            aRectFnSet.SetTop( aDrawRect, nOldBottom );
            PaintContent_( pCnt, pPage, aDrawRect );
        }
        // paint content area
        SwRect aPaintRect = static_cast<SwTextFrame*>(const_cast<SwContentFrame*>(pCnt))->GetPaintSwRect();
        PaintContent_( pCnt, pPage, aPaintRect );
    }

    if ( pCnt->IsRetouche() && !pCnt->GetNext() )
    {
        const SwFrame *pTmp = pCnt;
        if( pCnt->IsInSct() )
        {
            const SwSectionFrame* pSct = pCnt->FindSctFrame();
            if( pSct->IsRetouche() && !pSct->GetNext() )
                pTmp = pSct;
        }
        SwRect aRect( pTmp->GetUpper()->GetPaintArea() );
        aRectFnSet.SetTop( aRect, aRectFnSet.GetPrtBottom(*pTmp) );
        if ( !PaintContent_( pCnt, pPage, aRect ) )
            pCnt->ResetRetouche();
    }
}

SwLayAction::SwLayAction( SwRootFrame *pRt, SwViewShellImp *pI ) :
    m_pRoot( pRt ),
    m_pImp( pI ),
    m_pOptTab( nullptr ),
    m_pWait( nullptr ),
    m_nPreInvaPage( USHRT_MAX ),
    m_nStartTicks( std::clock() ),
    m_nInputType( VclInputFlags::NONE ),
    m_nEndPage( USHRT_MAX ),
    m_nCheckPageNum( USHRT_MAX )
{
    m_bPaintExtraData = ::IsExtraData( m_pImp->GetShell()->GetDoc() );
    m_bPaint = m_bComplete = m_bWaitAllowed = m_bCheckPages = true;
    m_bInput = m_bAgain = m_bNextCycle = m_bCalcLayout = m_bIdle = m_bReschedule =
    m_bUpdateExpFields = m_bBrowseActionStop = m_bActionInProgress = false;
    // init new flag <mbFormatContentOnInterrupt>.
    mbFormatContentOnInterrupt = false;

    assert(!m_pImp->m_pLayAction); // there can be only one SwLayAction
    m_pImp->m_pLayAction = this;   // register there
}

SwLayAction::~SwLayAction()
{
    OSL_ENSURE( !m_pWait, "Wait object not destroyed" );
    m_pImp->m_pLayAction = nullptr;      // unregister
}

void SwLayAction::Reset()
{
    m_pOptTab = nullptr;
    m_nStartTicks = std::clock();
    m_nInputType = VclInputFlags::NONE;
    m_nEndPage = m_nPreInvaPage = m_nCheckPageNum = USHRT_MAX;
    m_bPaint = m_bComplete = m_bWaitAllowed = m_bCheckPages = true;
    m_bInput = m_bAgain = m_bNextCycle = m_bCalcLayout = m_bIdle = m_bReschedule =
    m_bUpdateExpFields = m_bBrowseActionStop = false;
}

bool SwLayAction::RemoveEmptyBrowserPages()
{
    // switching from the normal to the browser mode, empty pages may be
    // retained for an annoyingly long time, so delete them here
    bool bRet = false;
    const SwViewShell *pSh = m_pRoot->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
    {
        SwPageFrame *pPage = static_cast<SwPageFrame*>(m_pRoot->Lower());
        do
        {
            if ( (pPage->GetSortedObjs() && pPage->GetSortedObjs()->size()) ||
                 pPage->ContainsContent() )
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
            else
            {
                bRet = true;
                SwPageFrame *pDel = pPage;
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
                pDel->Cut();
                SwFrame::DestroyFrame(pDel);
            }
        } while ( pPage );
    }
    return bRet;
}

void SwLayAction::Action(OutputDevice* pRenderContext)
{
    m_bActionInProgress = true;

    //TurboMode? Hands-off during idle-format
    if ( IsPaint() && !IsIdle() && TurboAction() )
    {
        m_pWait.reset();
        m_pRoot->ResetTurboFlag();
        m_bActionInProgress = false;
        m_pRoot->DeleteEmptySct();
        return;
    }
    else if ( m_pRoot->GetTurbo() )
    {
        m_pRoot->DisallowTurbo();
        const SwFrame *pFrame = m_pRoot->GetTurbo();
        m_pRoot->ResetTurbo();
        pFrame->InvalidatePage();
    }
    m_pRoot->DisallowTurbo();

    if ( IsCalcLayout() )
        SetCheckPages( false );

    InternalAction(pRenderContext);
    m_bAgain |= RemoveEmptyBrowserPages();
    while ( IsAgain() )
    {
        m_bAgain = m_bNextCycle = false;
        InternalAction(pRenderContext);
        m_bAgain |= RemoveEmptyBrowserPages();
    }
    m_pRoot->DeleteEmptySct();

    m_pWait.reset();

    //Turbo-Action permitted again for all cases.
    m_pRoot->ResetTurboFlag();
    m_pRoot->ResetTurbo();

    SetCheckPages( true );

    m_bActionInProgress = false;
}

SwPageFrame* SwLayAction::CheckFirstVisPage( SwPageFrame *pPage )
{
    SwContentFrame *pCnt = pPage->FindFirstBodyContent();
    SwContentFrame *pChk = pCnt;
    bool bPageChgd = false;
    while ( pCnt && pCnt->IsFollow() )
        pCnt = pCnt->FindMaster();
    if ( pCnt && pChk != pCnt )
    {   bPageChgd = true;
        pPage = pCnt->FindPageFrame();
    }

    if ( !pPage->GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
    {
        SwFootnoteContFrame *pCont = pPage->FindFootnoteCont();
        if ( pCont )
        {
            pCnt = pCont->ContainsContent();
            pChk = pCnt;
            while ( pCnt && pCnt->IsFollow() )
                pCnt = static_cast<SwContentFrame*>(pCnt->FindPrev());
            if ( pCnt && pCnt != pChk )
            {
                if ( bPageChgd )
                {
                    // Use the 'topmost' page
                    SwPageFrame *pTmp = pCnt->FindPageFrame();
                    if ( pPage->GetPhyPageNum() > pTmp->GetPhyPageNum() )
                        pPage = pTmp;
                }
                else
                    pPage = pCnt->FindPageFrame();
            }
        }
    }
    return pPage;
}

// unlock position on start and end of page
// layout process.
static void unlockPositionOfObjects( SwPageFrame *pPageFrame )
{
    assert( pPageFrame );

    SwSortedObjs* pObjs = pPageFrame->GetSortedObjs();
    if ( pObjs )
    {
        for (SwAnchoredObject* pObj : *pObjs)
        {
            pObj->UnlockPosition();
        }
    }
}

void SwLayAction::InternalAction(OutputDevice* pRenderContext)
{
    OSL_ENSURE( m_pRoot->Lower()->IsPageFrame(), ":-( No page below the root.");

    m_pRoot->Calc(pRenderContext);

    // Figure out the first invalid page or the first one to be formatted,
    // respectively. A complete-action means the first invalid page.
    // However, the first page to be formatted might be the one having the
    // number 1.  If we're doing a fake formatting, the number of the first
    // page is the number of the first visible page.
    SwPageFrame *pPage = IsComplete() ? static_cast<SwPageFrame*>(m_pRoot->Lower()) :
                m_pImp->GetFirstVisPage(pRenderContext);
    if ( !pPage )
        pPage = static_cast<SwPageFrame*>(m_pRoot->Lower());

    // If there's a first-flow-Content in the first visible page that's also a Follow,
    // we switch the page back to the original master of that Content.
    if ( !IsComplete() )
        pPage = CheckFirstVisPage( pPage );
    sal_uInt16 nFirstPageNum = pPage->GetPhyPageNum();

    while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());

    IDocumentLayoutAccess& rLayoutAccess = m_pRoot->GetFormat()->getIDocumentLayoutAccess();
    bool bNoLoop = pPage && SwLayouter::StartLoopControl( m_pRoot->GetFormat()->GetDoc(), pPage );
    sal_uInt16 nPercentPageNum = 0;
    while ( (pPage && !IsInterrupt()) || m_nCheckPageNum != USHRT_MAX )
    {
        if (!pPage && m_nCheckPageNum != USHRT_MAX)
        {
            SwPageFrame *pPg = static_cast<SwPageFrame*>(m_pRoot->Lower());
            while (pPg && pPg->GetPhyPageNum() < m_nCheckPageNum)
                pPg = static_cast<SwPageFrame*>(pPg->GetNext());
            if (pPg)
                pPage = pPg;
            if (!pPage)
                break;

            SwPageFrame *pTmp = pPage->GetPrev() ?
                                        static_cast<SwPageFrame*>(pPage->GetPrev()) : pPage;
            SetCheckPages( true );
            SwFrame::CheckPageDescs( pPage, true, &pTmp );
            SetCheckPages( false );
            m_nCheckPageNum = USHRT_MAX;
            pPage = pTmp;
            continue;
        }

        if ( m_nEndPage != USHRT_MAX && pPage->GetPhyPageNum() > nPercentPageNum )
        {
            nPercentPageNum = pPage->GetPhyPageNum();
            ::SetProgressState( nPercentPageNum, m_pImp->GetShell()->GetDoc()->GetDocShell());
        }
        m_pOptTab = nullptr;

        // No Shortcut for Idle or CalcLayout
        if ( !IsIdle() && !IsComplete() && IsShortCut( pPage ) )
        {
            m_pRoot->DeleteEmptySct();
            XCHECKPAGE;
            if ( !IsInterrupt() &&
                 (m_pRoot->IsSuperfluous() || m_pRoot->IsAssertFlyPages()) )
            {
                if ( m_pRoot->IsAssertFlyPages() )
                    m_pRoot->AssertFlyPages();
                if ( m_pRoot->IsSuperfluous() )
                {
                    bool bOld = IsAgain();
                    m_pRoot->RemoveSuperfluous();
                    m_bAgain = bOld;
                }
                if ( IsAgain() )
                {
                    if( bNoLoop )
                        rLayoutAccess.GetLayouter()->EndLoopControl();
                    return;
                }
                pPage = static_cast<SwPageFrame*>(m_pRoot->Lower());
                while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
                    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
                while ( pPage && pPage->GetNext() &&
                        pPage->GetPhyPageNum() < nFirstPageNum )
                    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
                continue;
            }
            break;
        }
        else
        {
            m_pRoot->DeleteEmptySct();
            XCHECKPAGE;

            while ( !IsInterrupt() && !IsNextCycle() &&
                    ((pPage->GetSortedObjs() && pPage->IsInvalidFly()) || pPage->IsInvalid()) )
            {
                unlockPositionOfObjects( pPage );

                SwObjectFormatter::FormatObjsAtFrame( *pPage, *pPage, this );
                if ( !pPage->GetSortedObjs() )
                {
                    // If there are no (more) Flys, the flags are superfluous.
                    pPage->ValidateFlyLayout();
                    pPage->ValidateFlyContent();
                }
                // change condition
                while ( !IsInterrupt() && !IsNextCycle() &&
                        ( pPage->IsInvalid() ||
                          (pPage->GetSortedObjs() && pPage->IsInvalidFly()) ) )
                {
                    PROTOCOL( pPage, PROT::FileInit, DbgAction::NONE, nullptr)
                    XCHECKPAGE;

                    // new loop control
                    int nLoopControlRuns_1 = 0;
                    const int nLoopControlMax = 20;

                    while ( !IsNextCycle() && pPage->IsInvalidLayout() )
                    {
                        pPage->ValidateLayout();

                        if ( ++nLoopControlRuns_1 > nLoopControlMax )
                        {
                            OSL_FAIL( "LoopControl_1 in SwLayAction::InternalAction" );
                            break;
                        }

                        FormatLayout( pRenderContext, pPage );
                        XCHECKPAGE;
                    }
                    // change condition
                    if ( !IsNextCycle() &&
                         ( pPage->IsInvalidContent() ||
                           (pPage->GetSortedObjs() && pPage->IsInvalidFly()) ) )
                    {
                        pPage->ValidateFlyInCnt();
                        pPage->ValidateContent();
                        pPage->ValidateFlyLayout();
                        pPage->ValidateFlyContent();
                        if ( !FormatContent( pPage ) )
                        {
                            XCHECKPAGE;
                            pPage->InvalidateContent();
                            pPage->InvalidateFlyInCnt();
                            pPage->InvalidateFlyLayout();
                            pPage->InvalidateFlyContent();
                            if ( IsBrowseActionStop() )
                                m_bInput = true;
                        }
                    }
                    if( bNoLoop )
                        rLayoutAccess.GetLayouter()->LoopControl( pPage );
                }

                unlockPositionOfObjects( pPage );
            }

            // A previous page may be invalid again.
            XCHECKPAGE;
            if ( !pPage->GetSortedObjs() )
            {
                // If there are no (more) Flys, the flags are superfluous.
                pPage->ValidateFlyLayout();
                pPage->ValidateFlyContent();
            }
            if ( !IsInterrupt() )
            {
                SetNextCycle( false );

                if ( m_nPreInvaPage != USHRT_MAX )
                {
                    if( !IsComplete() && m_nPreInvaPage + 2 < nFirstPageNum )
                    {
                        m_pImp->SetFirstVisPageInvalid();
                        SwPageFrame *pTmpPage = m_pImp->GetFirstVisPage(pRenderContext);
                        nFirstPageNum = pTmpPage->GetPhyPageNum();
                        if( m_nPreInvaPage < nFirstPageNum )
                        {
                            m_nPreInvaPage = nFirstPageNum;
                            pPage = pTmpPage;
                        }
                    }
                    while ( pPage->GetPrev() && pPage->GetPhyPageNum() > m_nPreInvaPage )
                        pPage = static_cast<SwPageFrame*>(pPage->GetPrev());
                    m_nPreInvaPage = USHRT_MAX;
                }

                while ( pPage->GetPrev() &&
                        ( static_cast<SwPageFrame*>(pPage->GetPrev())->IsInvalid() ||
                          ( static_cast<SwPageFrame*>(pPage->GetPrev())->GetSortedObjs() &&
                            static_cast<SwPageFrame*>(pPage->GetPrev())->IsInvalidFly())) &&
                        (static_cast<SwPageFrame*>(pPage->GetPrev())->GetPhyPageNum() >=
                            nFirstPageNum) )
                {
                    pPage = static_cast<SwPageFrame*>(pPage->GetPrev());
                }

                // Continue to the next invalid page
                while ( pPage && !pPage->IsInvalid() &&
                        (!pPage->GetSortedObjs() || !pPage->IsInvalidFly()) )
                {
                    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
                }
                if( bNoLoop )
                    rLayoutAccess.GetLayouter()->LoopControl( pPage );
            }
            CheckIdleEnd();
        }
        if ( !pPage && !IsInterrupt() &&
             (m_pRoot->IsSuperfluous() || m_pRoot->IsAssertFlyPages()) )
        {
            if ( m_pRoot->IsAssertFlyPages() )
                m_pRoot->AssertFlyPages();
            if ( m_pRoot->IsSuperfluous() )
            {
                bool bOld = IsAgain();
                m_pRoot->RemoveSuperfluous();
                m_bAgain = bOld;
            }
            if ( IsAgain() )
            {
                if( bNoLoop )
                    rLayoutAccess.GetLayouter()->EndLoopControl();
                return;
            }
            pPage = static_cast<SwPageFrame*>(m_pRoot->Lower());
            while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
            while ( pPage && pPage->GetNext() &&
                    pPage->GetPhyPageNum() < nFirstPageNum )
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
        }
    }
    if ( IsInterrupt() && pPage )
    {
        // If we have input, we don't want to format content anymore, but
        // we still should clean the layout.
        // Otherwise, the following situation might arise:
        // The user enters some text at the end of the paragraph of the last
        // page, causing the paragraph to create a Follow for the next page.
        // Meanwhile the user continues typing, so we have input while
        // still formatting.
        // The paragraph on the new page has already been partially formatted,
        // and the new page has been fully formatted and is set to CompletePaint,
        // but hasn't added itself to the area to be output. Then we paint,
        // the CompletePaint of the page is reset because the new paragraph
        // already added itself, but the borders of the page haven't been painted
        // yet.
        // Oh well, with the inevitable following LayAction, the page doesn't
        // register itself, because it's (LayoutFrame) flags have been reset
        // already - the border of the page will never be painted.
        SwPageFrame *pPg = pPage;
        XCHECKPAGE;
        const SwRect &rVis = m_pImp->GetShell()->VisArea();

        while( pPg && pPg->getFrameArea().Bottom() < rVis.Top() )
            pPg = static_cast<SwPageFrame*>(pPg->GetNext());
        if( pPg != pPage )
            pPg = pPg ? static_cast<SwPageFrame*>(pPg->GetPrev()) : pPage;

        // set flag for interrupt content formatting
        mbFormatContentOnInterrupt = IsInput();
        long nBottom = rVis.Bottom();
        // #i42586# - format current page, if idle action is active
        // This is an optimization for the case that the interrupt is created by
        // the move of a form control object, which is represented by a window.
        while ( pPg && ( pPg->getFrameArea().Top() < nBottom ||
                         ( IsIdle() && pPg == pPage ) ) )
        {
            unlockPositionOfObjects( pPg );

            XCHECKPAGE;

            // new loop control
            int nLoopControlRuns_2 = 0;
            const int nLoopControlMax = 20;

            // special case: interrupt content formatting
            // conditions are incorrect and are too strict.
            // adjust interrupt formatting to normal page formatting - see above.
            while ( ( mbFormatContentOnInterrupt &&
                      ( pPg->IsInvalid() ||
                        ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) ) ||
                    ( !mbFormatContentOnInterrupt && pPg->IsInvalidLayout() ) )
            {
                XCHECKPAGE;
                // format also at-page anchored objects
                SwObjectFormatter::FormatObjsAtFrame( *pPg, *pPg, this );
                if ( !pPg->GetSortedObjs() )
                {
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyContent();
                }

                // new loop control
                int nLoopControlRuns_3 = 0;

                while ( pPg->IsInvalidLayout() )
                {
                    pPg->ValidateLayout();

                    if ( ++nLoopControlRuns_3 > nLoopControlMax )
                    {
                        OSL_FAIL( "LoopControl_3 in Interrupt formatting in SwLayAction::InternalAction" );
                        break;
                    }

                    FormatLayout( pRenderContext, pPg );
                    XCHECKPAGE;
                }

                if ( mbFormatContentOnInterrupt &&
                     ( pPg->IsInvalidContent() ||
                       ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) )
                {
                    pPg->ValidateFlyInCnt();
                    pPg->ValidateContent();
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyContent();

                    if ( ++nLoopControlRuns_2 > nLoopControlMax )
                    {
                        OSL_FAIL( "LoopControl_2 in Interrupt formatting in SwLayAction::InternalAction" );
                        break;
                    }

                    if ( !FormatContent( pPg ) )
                    {
                        XCHECKPAGE;
                        pPg->InvalidateContent();
                        pPg->InvalidateFlyInCnt();
                        pPg->InvalidateFlyLayout();
                        pPg->InvalidateFlyContent();
                    }
                    // we are satisfied if the content is formatted once complete.
                    else
                    {
                        break;
                    }
                }
            }

            unlockPositionOfObjects( pPg );
            pPg = static_cast<SwPageFrame*>(pPg->GetNext());
        }
        // reset flag for special interrupt content formatting.
        mbFormatContentOnInterrupt = false;
    }
    m_pOptTab = nullptr;
    if( bNoLoop )
        rLayoutAccess.GetLayouter()->EndLoopControl();
}

bool SwLayAction::TurboAction_( const SwContentFrame *pCnt )
{

    const SwPageFrame *pPage = nullptr;
    if ( !pCnt->isFrameAreaDefinitionValid() || pCnt->IsCompletePaint() || pCnt->IsRetouche() )
    {
        const SwRect aOldRect( pCnt->UnionFrame( true ) );
        const long   nOldBottom = pCnt->getFrameArea().Top() + pCnt->getFramePrintArea().Bottom();
        pCnt->Calc(m_pImp->GetShell()->GetOut());
        if ( pCnt->getFrameArea().Bottom() < aOldRect.Bottom() )
            pCnt->SetRetouche();

        pPage = pCnt->FindPageFrame();
        PaintContent( pCnt, pPage, aOldRect, nOldBottom );

        if ( !pCnt->GetValidLineNumFlag() && pCnt->IsTextFrame() )
        {
            const sal_uLong nAllLines = static_cast<const SwTextFrame*>(pCnt)->GetAllLines();
            const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pCnt))->RecalcAllLines();
            if ( nAllLines != static_cast<const SwTextFrame*>(pCnt)->GetAllLines() )
            {
                if ( IsPaintExtraData() )
                    m_pImp->GetShell()->AddPaintRect( pCnt->getFrameArea() );
                // This is to calculate the remaining LineNums on the page,
                // and we don't stop processing here. To perform this inside RecalcAllLines
                // would be expensive, because we would have to notify the page even
                // in unnecessary cases (normal actions).
                const SwContentFrame *pNxt = pCnt->GetNextContentFrame();
                while ( pNxt &&
                        (pNxt->IsInTab() || pNxt->IsInDocBody() != pCnt->IsInDocBody()) )
                    pNxt = pNxt->GetNextContentFrame();
                if ( pNxt )
                    pNxt->InvalidatePage();
            }
            return false;
        }

        if ( pPage->IsInvalidLayout() || (pPage->GetSortedObjs() && pPage->IsInvalidFly()) )
            return false;
    }
    if ( !pPage )
        pPage = pCnt->FindPageFrame();

    // format floating screen objects at content frame.
    if ( pCnt->IsTextFrame() &&
         !SwObjectFormatter::FormatObjsAtFrame( *const_cast<SwContentFrame*>(pCnt),
                                              *pPage, this ) )
    {
        return false;
    }

    if ( pPage->IsInvalidContent() )
        return false;
    return true;
}

bool SwLayAction::TurboAction()
{
    bool bRet = true;

    if ( m_pRoot->GetTurbo() )
    {
        if ( !TurboAction_( m_pRoot->GetTurbo() ) )
        {
            CheckIdleEnd();
            bRet = false;
        }
        m_pRoot->ResetTurbo();
    }
    else
        bRet = false;
    return bRet;
}

static bool lcl_IsInvaLay( const SwFrame *pFrame, long nBottom )
{
    return !pFrame->isFrameAreaDefinitionValid() ||
         (pFrame->IsCompletePaint() && ( pFrame->getFrameArea().Top() < nBottom ) );
}

static const SwFrame *lcl_FindFirstInvaLay( const SwFrame *pFrame, long nBottom )
{
    OSL_ENSURE( pFrame->IsLayoutFrame(), "FindFirstInvaLay, no LayFrame" );

    if (lcl_IsInvaLay(pFrame, nBottom))
        return pFrame;
    pFrame = static_cast<const SwLayoutFrame*>(pFrame)->Lower();
    while ( pFrame )
    {
        if ( pFrame->IsLayoutFrame() )
        {
            if (lcl_IsInvaLay(pFrame, nBottom))
                return pFrame;
            const SwFrame *pTmp;
            if ( nullptr != (pTmp = lcl_FindFirstInvaLay( pFrame, nBottom )) )
                return pTmp;
        }
        pFrame = pFrame->GetNext();
    }
    return nullptr;
}

static const SwFrame *lcl_FindFirstInvaContent( const SwLayoutFrame *pLay, long nBottom,
                                     const SwContentFrame *pFirst )
{
    const SwContentFrame *pCnt = pFirst ? pFirst->GetNextContentFrame() :
                                      pLay->ContainsContent();
    while ( pCnt )
    {
        if ( !pCnt->isFrameAreaDefinitionValid() || pCnt->IsCompletePaint() )
        {
            if ( pCnt->getFrameArea().Top() <= nBottom )
                return pCnt;
        }

        if ( pCnt->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
            for (SwAnchoredObject* pObj : rObjs)
            {
                if ( auto pFly = dynamic_cast< const SwFlyFrame *>( pObj ) )
                {
                    if ( pFly->IsFlyInContentFrame() )
                    {
                        if ( static_cast<const SwFlyInContentFrame*>(pFly)->IsInvalid() ||
                             pFly->IsCompletePaint() )
                        {
                            if ( pFly->getFrameArea().Top() <= nBottom )
                                return pFly;
                        }
                        const SwFrame *pFrame = lcl_FindFirstInvaContent( pFly, nBottom, nullptr );
                        if ( pFrame && pFrame->getFrameArea().Bottom() <= nBottom )
                            return pFrame;
                    }
                }
            }
        }
        if ( pCnt->getFrameArea().Top() > nBottom && !pCnt->IsInTab() )
            return nullptr;
        pCnt = pCnt->GetNextContentFrame();
        if ( !pLay->IsAnLower( pCnt ) )
            break;
    }
    return nullptr;
}

// consider drawing objects
static const SwAnchoredObject* lcl_FindFirstInvaObj( const SwPageFrame* _pPage,
                                              long _nBottom )
{
    OSL_ENSURE( _pPage->GetSortedObjs(), "FindFirstInvaObj, no Objs" );

    for (SwAnchoredObject* pObj : *_pPage->GetSortedObjs())
    {
        if ( auto pFly = dynamic_cast< const SwFlyFrame *>( pObj )  )
        {
            if ( pFly->getFrameArea().Top() <= _nBottom )
            {
                if ( pFly->IsInvalid() || pFly->IsCompletePaint() )
                    return pFly;

                const SwFrame* pTmp;
                if ( nullptr != (pTmp = lcl_FindFirstInvaContent( pFly, _nBottom, nullptr )) &&
                     pTmp->getFrameArea().Top() <= _nBottom )
                    return pFly;
            }
        }
        else if ( auto pDrawObject = dynamic_cast< const SwAnchoredDrawObject *>( pObj ) )
        {
            if ( !pDrawObject->IsValidPos() )
            {
                return pObj;
            }
        }
    }
    return nullptr;
}

/* Returns True if the page lies directly below or right of the visible area.
 *
 * It's possible for things to change in such a way that the processing
 * (of the caller!) has to continue with the predecessor of the passed page.
 * The parameter might therefore get modified!
 * For BrowseMode, you may even activate the ShortCut if the invalid content
 * of the page lies below the visible area.
 */
bool SwLayAction::IsShortCut( SwPageFrame *&prPage )
{
    vcl::RenderContext* pRenderContext = m_pImp->GetShell()->GetOut();
    bool bRet = false;
    const SwViewShell *pSh = m_pRoot->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    // If the page is not valid, we quickly format it, otherwise
    // there's gonna be no end of trouble
    if ( !prPage->isFrameAreaDefinitionValid() )
    {
        if ( bBrowse )
        {
            // format complete page
            // Thus, loop on all lowers of the page <prPage>, instead of only
            // format its first lower.
            // NOTE: In online layout (bBrowse == true) a page can contain
            //     a header frame and/or a footer frame beside the body frame.
            prPage->Calc(pRenderContext);
            SwFrame* pPageLowerFrame = prPage->Lower();
            while ( pPageLowerFrame )
            {
                pPageLowerFrame->Calc(pRenderContext);
                pPageLowerFrame = pPageLowerFrame->GetNext();
            }
        }
        else
            FormatLayout( pSh ? pSh->GetOut() : nullptr, prPage );
        if ( IsAgain() )
            return false;
    }

    const SwRect &rVis = m_pImp->GetShell()->VisArea();
    if ( (prPage->getFrameArea().Top() >= rVis.Bottom()) ||
         (prPage->getFrameArea().Left()>= rVis.Right()) )
    {
        bRet = true;

        // This is going to be a bit nasty: The first ContentFrame of this
        // page in the Body text needs formatting; if it changes the page during
        // that process, I need to start over a page further back, because we
        // have been processing a PageBreak.
        // Even more uncomfortable: The next ContentFrame must be formatted,
        // because it's possible for empty pages to exist temporarily (for example
        // a paragraph across multiple pages gets deleted or reduced in size).

        // This is irrelevant for the browser, if the last Cnt above it
        // isn't visible anymore.

        const SwPageFrame *p2ndPage = prPage;
        const SwContentFrame *pContent;
        const SwLayoutFrame* pBody = p2ndPage->FindBodyCont();
        if( p2ndPage->IsFootnotePage() && pBody )
            pBody = static_cast<const SwLayoutFrame*>(pBody->GetNext());
        pContent = pBody ? pBody->ContainsContent() : nullptr;
        while ( p2ndPage && !pContent )
        {
            p2ndPage = static_cast<const SwPageFrame*>(p2ndPage->GetNext());
            if( p2ndPage )
            {
                pBody = p2ndPage->FindBodyCont();
                if( p2ndPage->IsFootnotePage() && pBody )
                    pBody = static_cast<const SwLayoutFrame*>(pBody->GetNext());
                pContent = pBody ? pBody->ContainsContent() : nullptr;
            }
        }
        if ( pContent )
        {
            bool bTstCnt = true;
            if ( bBrowse )
            {
                // Is the Cnt before already invisible?
                const SwFrame *pLst = pContent;
                if ( pLst->IsInTab() )
                    pLst = pContent->FindTabFrame();
                if ( pLst->IsInSct() )
                    pLst = pContent->FindSctFrame();
                pLst = pLst->FindPrev();
                if ( pLst &&
                     (pLst->getFrameArea().Top() >= rVis.Bottom() ||
                      pLst->getFrameArea().Left()>= rVis.Right()) )
                {
                    bTstCnt = false;
                }
            }

            if ( bTstCnt )
            {
                // check after each frame calculation,
                // if the content frame has changed the page. If yes, no other
                // frame calculation is performed
                bool bPageChg = false;

                if ( pContent->IsInSct() )
                {
                    const SwSectionFrame *pSct = const_cast<SwFrame*>(static_cast<SwFrame const *>(pContent))->ImplFindSctFrame();
                    if ( !pSct->isFrameAreaDefinitionValid() )
                    {
                        pSct->Calc(pRenderContext);
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return false;

                        bPageChg = pContent->FindPageFrame() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( !bPageChg && !pContent->isFrameAreaDefinitionValid() )
                {
                    pContent->Calc(pRenderContext);
                    pContent->SetCompletePaint();
                    if ( IsAgain() )
                        return false;

                    bPageChg = pContent->FindPageFrame() != p2ndPage &&
                               prPage->GetPrev();
                }

                if ( !bPageChg && pContent->IsInTab() )
                {
                    const SwTabFrame *pTab = const_cast<SwFrame*>(static_cast<SwFrame const *>(pContent))->ImplFindTabFrame();
                    if ( !pTab->isFrameAreaDefinitionValid() )
                    {
                        pTab->Calc(pRenderContext);
                        pTab->SetCompletePaint();
                        if ( IsAgain() )
                            return false;

                        bPageChg = pContent->FindPageFrame() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( !bPageChg && pContent->IsInSct() )
                {
                    const SwSectionFrame *pSct = const_cast<SwFrame*>(static_cast<SwFrame const *>(pContent))->ImplFindSctFrame();
                    if ( !pSct->isFrameAreaDefinitionValid() )
                    {
                        pSct->Calc(pRenderContext);
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return false;

                        bPageChg = pContent->FindPageFrame() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( bPageChg )
                {
                    bRet = false;
                    const SwPageFrame* pTmp = pContent->FindPageFrame();
                    if ( pTmp->GetPhyPageNum() < prPage->GetPhyPageNum() &&
                         pTmp->IsInvalid() )
                    {
                        prPage = const_cast<SwPageFrame*>(pTmp);
                    }
                    else
                    {
                        prPage = static_cast<SwPageFrame*>(prPage->GetPrev());
                    }
                }
                // no shortcut, if at previous page
                // an anchored object is registered, whose anchor is <pContent>.
                else if ( prPage->GetPrev() )
                {
                    SwSortedObjs* pObjs =
                        static_cast<SwPageFrame*>(prPage->GetPrev())->GetSortedObjs();
                    if ( pObjs )
                    {
                        for (SwAnchoredObject* pObj : *pObjs)
                        {
                            if ( pObj->GetAnchorFrameContainingAnchPos() == pContent )
                            {
                                bRet = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !bRet && bBrowse )
    {
        const long nBottom = rVis.Bottom();
        const SwAnchoredObject* pObj( nullptr );
        if ( prPage->GetSortedObjs() &&
             (prPage->IsInvalidFlyLayout() || prPage->IsInvalidFlyContent()) &&
             nullptr != (pObj = lcl_FindFirstInvaObj( prPage, nBottom )) &&
             pObj->GetObjRect().Top() <= nBottom )
        {
            return false;
        }
        const SwFrame* pFrame( nullptr );
        if ( prPage->IsInvalidLayout() &&
             nullptr != (pFrame = lcl_FindFirstInvaLay( prPage, nBottom )) &&
             pFrame->getFrameArea().Top() <= nBottom )
        {
            return false;
        }
        if ( (prPage->IsInvalidContent() || prPage->IsInvalidFlyInCnt()) &&
             nullptr != (pFrame = lcl_FindFirstInvaContent( prPage, nBottom, nullptr )) &&
             pFrame->getFrameArea().Top() <= nBottom )
        {
            return false;
        }
        bRet = true;
    }
    return bRet;
}

// introduce support for vertical layout
bool SwLayAction::FormatLayout( OutputDevice *pRenderContext, SwLayoutFrame *pLay, bool bAddRect )
{
    OSL_ENSURE( !IsAgain(), "Attention to the invalid page." );
    if ( IsAgain() )
        return false;

    bool bChanged = false;
    bool bAlreadyPainted = false;
    // remember frame at complete paint
    SwRect aFrameAtCompletePaint;

    if ( !pLay->isFrameAreaDefinitionValid() || pLay->IsCompletePaint() )
    {
        if ( pLay->GetPrev() && !pLay->GetPrev()->isFrameAreaDefinitionValid() )
            pLay->GetPrev()->SetCompletePaint();

        SwRect aOldFrame( pLay->getFrameArea() );
        SwRect aOldRect( aOldFrame );
        if( pLay->IsPageFrame() )
        {
            aOldRect = static_cast<SwPageFrame*>(pLay)->GetBoundRect(pRenderContext);
        }

        {
            SwFrameDeleteGuard aDeleteGuard(pLay);
            pLay->Calc(pRenderContext);
        }

        if ( aOldFrame != pLay->getFrameArea() )
            bChanged = true;

        bool bNoPaint = false;
        if ( pLay->IsPageBodyFrame() &&
             pLay->getFrameArea().Pos() == aOldRect.Pos() &&
             pLay->Lower() )
        {
            const SwViewShell *pSh = pLay->getRootFrame()->GetCurrShell();
            // Limitations because of headers / footers
            if( pSh && pSh->GetViewOptions()->getBrowseMode() &&
                !( pLay->IsCompletePaint() && pLay->FindPageFrame()->FindFootnoteCont() ) )
                bNoPaint = true;
        }

        if ( !bNoPaint && IsPaint() && bAddRect && (pLay->IsCompletePaint() || bChanged) )
        {
            SwRect aPaint( pLay->getFrameArea() );
            // consider border and shadow for
            // page frames -> enlarge paint rectangle correspondingly.
            if ( pLay->IsPageFrame() )
            {
                SwPageFrame* pPageFrame = static_cast<SwPageFrame*>(pLay);
                aPaint = pPageFrame->GetBoundRect(pRenderContext);
            }

            bool bPageInBrowseMode = pLay->IsPageFrame();
            if( bPageInBrowseMode )
            {
                const SwViewShell *pSh = pLay->getRootFrame()->GetCurrShell();
                if( !pSh || !pSh->GetViewOptions()->getBrowseMode() )
                    bPageInBrowseMode = false;
            }
            if( bPageInBrowseMode )
            {
                // NOTE: no vertical layout in online layout
                // Is the change even visible?
                if ( pLay->IsCompletePaint() )
                {
                    m_pImp->GetShell()->AddPaintRect( aPaint );
                    bAddRect = false;
                }
                else
                {
                    SwRegionRects aRegion( aOldRect );
                    aRegion -= aPaint;
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        m_pImp->GetShell()->AddPaintRect( aRegion[i] );
                    aRegion.ChangeOrigin( aPaint );
                    aRegion.clear();
                    aRegion.push_back( aPaint );
                    aRegion -= aOldRect;
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        m_pImp->GetShell()->AddPaintRect( aRegion[i] );
                }
            }
            else
            {
                m_pImp->GetShell()->AddPaintRect( aPaint );
                bAlreadyPainted = true;
                // remember frame at complete paint
                aFrameAtCompletePaint = pLay->getFrameArea();
            }

            // provide paint of spacing
            // between pages (not only for in online mode).
            if ( pLay->IsPageFrame() )
            {
                const SwViewShell *pSh = pLay->getRootFrame()->GetCurrShell();
                const SwTwips nHalfDocBorder = pSh ? pSh->GetViewOptions()->GetGapBetweenPages()
                                                   : SwViewOption::defGapBetweenPages;
                const bool bLeftToRightViewLayout = m_pRoot->IsLeftToRightViewLayout();
                const bool bPrev = bLeftToRightViewLayout ? pLay->GetPrev() : pLay->GetNext();
                const bool bNext = bLeftToRightViewLayout ? pLay->GetNext() : pLay->GetPrev();
                SwPageFrame* pPageFrame = static_cast<SwPageFrame*>(pLay);
                SwRect aPageRect( pLay->getFrameArea() );

                if(pSh)
                {
                    SwPageFrame::GetBorderAndShadowBoundRect(aPageRect, pSh,
                        pRenderContext,
                        aPageRect, pPageFrame->IsLeftShadowNeeded(), pPageFrame->IsRightShadowNeeded(),
                        pPageFrame->SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
                }

                if ( bPrev )
                {
                    // top
                    SwRect aSpaceToPrevPage( aPageRect );
                    aSpaceToPrevPage.Top( aSpaceToPrevPage.Top() - nHalfDocBorder );
                    aSpaceToPrevPage.Bottom( pLay->getFrameArea().Top() );
                    if(aSpaceToPrevPage.Height() > 0 && aSpaceToPrevPage.Width() > 0)
                        m_pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );

                    // left
                    aSpaceToPrevPage = aPageRect;
                    aSpaceToPrevPage.Left( aSpaceToPrevPage.Left() - nHalfDocBorder );
                    aSpaceToPrevPage.Right( pLay->getFrameArea().Left() );
                    if(aSpaceToPrevPage.Height() > 0 && aSpaceToPrevPage.Width() > 0)
                        m_pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );
                }
                if ( bNext )
                {
                    // bottom
                    SwRect aSpaceToNextPage( aPageRect );
                    aSpaceToNextPage.Bottom( aSpaceToNextPage.Bottom() + nHalfDocBorder );
                    aSpaceToNextPage.Top( pLay->getFrameArea().Bottom() );
                    if(aSpaceToNextPage.Height() > 0 && aSpaceToNextPage.Width() > 0)
                        m_pImp->GetShell()->AddPaintRect( aSpaceToNextPage );

                    // right
                    aSpaceToNextPage = aPageRect;
                    aSpaceToNextPage.Right( aSpaceToNextPage.Right() + nHalfDocBorder );
                    aSpaceToNextPage.Left( pLay->getFrameArea().Right() );
                    if(aSpaceToNextPage.Height() > 0 && aSpaceToNextPage.Width() > 0)
                        m_pImp->GetShell()->AddPaintRect( aSpaceToNextPage );
                }
            }
        }
        pLay->ResetCompletePaint();
    }

    if ( IsPaint() && bAddRect &&
         !pLay->GetNext() && pLay->IsRetoucheFrame() && pLay->IsRetouche() )
    {
        // vertical layout support
        SwRectFnSet aRectFnSet(pLay);
        SwRect aRect( pLay->GetUpper()->GetPaintArea() );
        aRectFnSet.SetTop( aRect, aRectFnSet.GetPrtBottom(*pLay) );
        if ( !m_pImp->GetShell()->AddPaintRect( aRect ) )
            pLay->ResetRetouche();
    }

    if( bAlreadyPainted )
        bAddRect = false;

    CheckWaitCursor();

    if ( IsAgain() )
        return false;

    // Now, deal with the lowers that are LayoutFrames

    if ( pLay->IsFootnoteFrame() ) // no LayFrames as Lower
        return bChanged;

    SwFrame *pLow = pLay->Lower();
    bool bTabChanged = false;
    while ( pLow && pLow->GetUpper() == pLay )
    {
        if ( pLow->IsLayoutFrame() )
        {
            if ( pLow->IsTabFrame() )
                bTabChanged |= FormatLayoutTab( static_cast<SwTabFrame*>(pLow), bAddRect );
            // Skip the ones already registered for deletion
            else if( !pLow->IsSctFrame() || static_cast<SwSectionFrame*>(pLow)->GetSection() )
                bChanged |= FormatLayout( pRenderContext, static_cast<SwLayoutFrame*>(pLow), bAddRect );
        }
        else if ( m_pImp->GetShell()->IsPaintLocked() )
            // Shortcut to minimize the cycles. With Lock, the
            // paint is coming either way (primarily for browse)
            pLow->OptCalc();

        if ( IsAgain() )
            return false;
        pLow = pLow->GetNext();
    }
    // add complete frame area as paint area, if frame
    // area has been already added and after formatting its lowers the frame area
    // is enlarged.
    SwRect aBoundRect(pLay->IsPageFrame() ? static_cast<SwPageFrame*>(pLay)->GetBoundRect(pRenderContext) : pLay->getFrameArea() );

    if ( bAlreadyPainted &&
         ( aBoundRect.Width() > aFrameAtCompletePaint.Width() ||
           aBoundRect.Height() > aFrameAtCompletePaint.Height() )
       )
    {
        m_pImp->GetShell()->AddPaintRect( aBoundRect );
    }
    return bChanged || bTabChanged;
}

void SwLayAction::FormatLayoutFly( SwFlyFrame* pFly )
{
    vcl::RenderContext* pRenderContext = m_pImp->GetShell()->GetOut();
    OSL_ENSURE( !IsAgain(), "Attention to the invalid page." );
    if ( IsAgain() )
        return;

    bool bChanged = false;
    bool bAddRect = true;

    if ( !pFly->isFrameAreaDefinitionValid() || pFly->IsCompletePaint() || pFly->IsInvalid() )
    {
        // The Frame has changed, now it's getting formatted.
        const SwRect aOldRect( pFly->getFrameArea() );
        pFly->Calc(pRenderContext);
        bChanged = aOldRect != pFly->getFrameArea();

        if ( IsPaint() && (pFly->IsCompletePaint() || bChanged) &&
                    pFly->getFrameArea().Top() > 0 && pFly->getFrameArea().Left() > 0 )
            m_pImp->GetShell()->AddPaintRect( pFly->getFrameArea() );

        if ( bChanged )
            pFly->Invalidate();
        else
            pFly->Validate();

        bAddRect = false;
        pFly->ResetCompletePaint();
    }

    if ( IsAgain() )
        return;

    // Now, deal with the lowers that are LayoutFrames
    bool bTabChanged = false;
    SwFrame *pLow = pFly->Lower();
    while ( pLow )
    {
        if ( pLow->IsLayoutFrame() )
        {
            if ( pLow->IsTabFrame() )
                bTabChanged |= FormatLayoutTab( static_cast<SwTabFrame*>(pLow), bAddRect );
            else
                bChanged |= FormatLayout( m_pImp->GetShell()->GetOut(), static_cast<SwLayoutFrame*>(pLow), bAddRect );
        }
        pLow = pLow->GetNext();
    }
}

// Implement vertical layout support
bool SwLayAction::FormatLayoutTab( SwTabFrame *pTab, bool bAddRect )
{
    OSL_ENSURE( !IsAgain(), "8-) Attention to the invalid page." );
    if ( IsAgain() || !pTab->Lower() )
        return false;

    vcl::RenderContext* pRenderContext = m_pImp->GetShell()->GetOut();
    IDocumentTimerAccess& rTimerAccess = m_pRoot->GetFormat()->getIDocumentTimerAccess();
    rTimerAccess.BlockIdling();

    bool bChanged = false;
    bool bPainted = false;

    const SwPageFrame *pOldPage = pTab->FindPageFrame();

    // vertical layout support
    SwRectFnSet aRectFnSet(pTab);

    if ( !pTab->isFrameAreaDefinitionValid() || pTab->IsCompletePaint() || pTab->IsComplete() )
    {
        if ( pTab->GetPrev() && !pTab->GetPrev()->isFrameAreaDefinitionValid() )
        {
            pTab->GetPrev()->SetCompletePaint();
        }

        const SwRect aOldRect( pTab->getFrameArea() );
        pTab->SetLowersFormatted( false );
        pTab->Calc(pRenderContext);
        if ( aOldRect != pTab->getFrameArea() )
        {
            bChanged = true;
        }
        const SwRect aPaintFrame = pTab->GetPaintArea();

        if ( IsPaint() && bAddRect )
        {
            // add condition <pTab->getFrameArea().HasArea()>
            if ( !pTab->IsCompletePaint() &&
                 pTab->IsComplete() &&
                 ( pTab->getFrameArea().SSize() != pTab->getFramePrintArea().SSize() ||
                   // vertical layout support
                   aRectFnSet.GetLeftMargin(*pTab) ) &&
                 pTab->getFrameArea().HasArea()
               )
            {
                // re-implement calculation of margin rectangles.
                SwRect aMarginRect;

                SwTwips nLeftMargin = aRectFnSet.GetLeftMargin(*pTab);
                if ( nLeftMargin > 0)
                {
                    aMarginRect = pTab->getFrameArea();
                    aRectFnSet.SetWidth( aMarginRect, nLeftMargin );
                    m_pImp->GetShell()->AddPaintRect( aMarginRect );
                }

                if ( aRectFnSet.GetRightMargin(*pTab) > 0)
                {
                    aMarginRect = pTab->getFrameArea();
                    aRectFnSet.SetLeft( aMarginRect, aRectFnSet.GetPrtRight(*pTab) );
                    m_pImp->GetShell()->AddPaintRect( aMarginRect );
                }

                SwTwips nTopMargin = aRectFnSet.GetTopMargin(*pTab);
                if ( nTopMargin > 0)
                {
                    aMarginRect = pTab->getFrameArea();
                    aRectFnSet.SetHeight( aMarginRect, nTopMargin );
                    m_pImp->GetShell()->AddPaintRect( aMarginRect );
                }

                if ( aRectFnSet.GetBottomMargin(*pTab) > 0)
                {
                    aMarginRect = pTab->getFrameArea();
                    aRectFnSet.SetTop( aMarginRect, aRectFnSet.GetPrtBottom(*pTab) );
                    m_pImp->GetShell()->AddPaintRect( aMarginRect );
                }
            }
            else if ( pTab->IsCompletePaint() )
            {
                m_pImp->GetShell()->AddPaintRect( aPaintFrame );
                bAddRect = false;
                bPainted = true;
            }

            if ( pTab->IsRetouche() && !pTab->GetNext() )
            {
                SwRect aRect( pTab->GetUpper()->GetPaintArea() );
                // vertical layout support
                aRectFnSet.SetTop( aRect, aRectFnSet.GetPrtBottom(*pTab) );
                if ( !m_pImp->GetShell()->AddPaintRect( aRect ) )
                    pTab->ResetRetouche();
            }
        }
        else
            bAddRect = false;

        if ( pTab->IsCompletePaint() && !m_pOptTab )
            m_pOptTab = pTab;
        pTab->ResetCompletePaint();
    }
    if ( IsPaint() && bAddRect && pTab->IsRetouche() && !pTab->GetNext() )
    {
        // set correct rectangle for retouche: area between bottom of table frame
        // and bottom of paint area of the upper frame.
        SwRect aRect( pTab->GetUpper()->GetPaintArea() );
        // vertical layout support
        aRectFnSet.SetTop( aRect, aRectFnSet.GetPrtBottom(*pTab) );
        if ( !m_pImp->GetShell()->AddPaintRect( aRect ) )
            pTab->ResetRetouche();
    }

    CheckWaitCursor();

    rTimerAccess.UnblockIdling();

    // Ugly shortcut!
    if ( pTab->IsLowersFormatted() &&
         (bPainted || !m_pImp->GetShell()->VisArea().IsOver( pTab->getFrameArea())) )
        return false;

    // Now, deal with the lowers
    if ( IsAgain() )
        return false;

    // for safety reasons:
    // check page number before formatting lowers.
    if ( pOldPage->GetPhyPageNum() > (pTab->FindPageFrame()->GetPhyPageNum() + 1) )
        SetNextCycle( true );

    // format lowers, only if table frame is valid
    if ( pTab->isFrameAreaDefinitionValid() )
    {
        SwLayoutFrame *pLow = static_cast<SwLayoutFrame*>(pTab->Lower());
        while ( pLow )
        {
            bChanged |= FormatLayout( m_pImp->GetShell()->GetOut(), pLow, bAddRect );
            if ( IsAgain() )
                return false;
            pLow = static_cast<SwLayoutFrame*>(pLow->GetNext());
        }
    }

    return bChanged;
}

bool SwLayAction::FormatContent( const SwPageFrame *pPage )
{
    const SwContentFrame *pContent = pPage->ContainsContent();
    const SwViewShell *pSh = m_pRoot->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    while ( pContent && pPage->IsAnLower( pContent ) )
    {
        // If the content didn't change, we can use a few shortcuts.
        const bool bFull = !pContent->isFrameAreaDefinitionValid() || pContent->IsCompletePaint() ||
                           pContent->IsRetouche() || pContent->GetDrawObjs();
        if ( bFull )
        {
            // We do this so we don't have to search later on.
            const bool bNxtCnt = IsCalcLayout() && !pContent->GetFollow();
            const SwContentFrame *pContentNext = bNxtCnt ? pContent->GetNextContentFrame() : nullptr;
            const SwContentFrame *pContentPrev = pContent->GetPrev() ? pContent->GetPrevContentFrame() : nullptr;

            const SwLayoutFrame*pOldUpper  = pContent->GetUpper();
            const SwTabFrame *pTab = pContent->FindTabFrame();
            const bool bInValid = !pContent->isFrameAreaDefinitionValid() || pContent->IsCompletePaint();
            const bool bOldPaint = IsPaint();
            m_bPaint = bOldPaint && !(pTab && pTab == m_pOptTab);
            FormatContent_( pContent, pPage );
            // reset <bPaint> before format objects
            m_bPaint = bOldPaint;

            // format floating screen object at content frame.
            // No format, if action flag <bAgain> is set or action is interrupted.
            // allow format on interruption of action, if
            // it's the format for this interrupt
            // pass correct page frame
            // to the object formatter.
            if ( !IsAgain() &&
                 ( !IsInterrupt() || mbFormatContentOnInterrupt ) &&
                 pContent->IsTextFrame() &&
                 !SwObjectFormatter::FormatObjsAtFrame( *const_cast<SwContentFrame*>(pContent),
                                                      *(pContent->FindPageFrame()), this ) )
            {
                return false;
            }

            if ( !pContent->GetValidLineNumFlag() && pContent->IsTextFrame() )
            {
                const sal_uLong nAllLines = static_cast<const SwTextFrame*>(pContent)->GetAllLines();
                const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pContent))->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != static_cast<const SwTextFrame*>(pContent)->GetAllLines() )
                    m_pImp->GetShell()->AddPaintRect( pContent->getFrameArea() );
            }

            if ( IsAgain() )
                return false;

            // Temporarily interrupt processing if layout or Flys become invalid again.
            // However not for the BrowseView: The layout is getting invalid
            // all the time because the page height gets adjusted.
            // The same applies if the user wants to continue working and at least one
            // paragraph has been processed.
            if (!pTab || !bInValid)
            {
                CheckIdleEnd();
                // consider interrupt formatting.
                if ( ( IsInterrupt() && !mbFormatContentOnInterrupt ) ||
                     ( !bBrowse && pPage->IsInvalidLayout() ) ||
                     // consider interrupt formatting
                     ( pPage->GetSortedObjs() && pPage->IsInvalidFly() && !mbFormatContentOnInterrupt )
                   )
                    return false;
            }
            if ( pOldUpper != pContent->GetUpper() )
            {
                const sal_uInt16 nCurNum = pContent->FindPageFrame()->GetPhyPageNum();
                if (  nCurNum < pPage->GetPhyPageNum() )
                    m_nPreInvaPage = nCurNum;

                // If the frame flowed backwards more than one page, we need to
                // start over again from the beginning, so nothing gets left out.
                if ( !IsCalcLayout() && pPage->GetPhyPageNum() > nCurNum+1 )
                {
                    SetNextCycle( true );
                    // consider interrupt formatting
                    if ( !mbFormatContentOnInterrupt )
                    {
                        return false;
                    }
                }
            }
            // If the frame moved forwards to the next page, we re-run through
            // the predecessor.
            // This way, we catch predecessors which are now responsible for
            // retouching, but the footers will be touched also.
            bool bSetContent = true;
            if ( pContentPrev )
            {
                if ( !pContentPrev->isFrameAreaDefinitionValid() && pPage->IsAnLower( pContentPrev ) )
                {
                    pPage->InvalidateContent();
                }

                if ( pOldUpper != pContent->GetUpper() &&
                     pPage->GetPhyPageNum() < pContent->FindPageFrame()->GetPhyPageNum() )
                {
                    pContent = pContentPrev;
                    bSetContent = false;
                }
            }
            if ( bSetContent )
            {
                if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
                     pContent->getFrameArea().Top() > m_pImp->GetShell()->VisArea().Bottom())
                {
                    const long nBottom = m_pImp->GetShell()->VisArea().Bottom();
                    const SwFrame *pTmp = lcl_FindFirstInvaContent( pPage,
                                                            nBottom, pContent );
                    if ( !pTmp )
                    {
                        if ( (!(pPage->GetSortedObjs() && pPage->IsInvalidFly()) ||
                              !lcl_FindFirstInvaObj( pPage, nBottom )) &&
                              (!pPage->IsInvalidLayout() ||
                               !lcl_FindFirstInvaLay( pPage, nBottom )))
                            SetBrowseActionStop( true );
                        // consider interrupt formatting.
                        if ( !mbFormatContentOnInterrupt )
                        {
                            return false;
                        }
                    }
                }
                pContent = bNxtCnt ? pContentNext : pContent->GetNextContentFrame();
            }

            if (IsReschedule())
            {
                ::RescheduleProgress(m_pImp->GetShell()->GetDoc()->GetDocShell());
            }
        }
        else
        {
            if ( !pContent->GetValidLineNumFlag() && pContent->IsTextFrame() )
            {
                const sal_uLong nAllLines = static_cast<const SwTextFrame*>(pContent)->GetAllLines();
                const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pContent))->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != static_cast<const SwTextFrame*>(pContent)->GetAllLines() )
                    m_pImp->GetShell()->AddPaintRect( pContent->getFrameArea() );
            }

            // Do this if the frame has been formatted before.
            if ( pContent->IsTextFrame() && static_cast<const SwTextFrame*>(pContent)->HasRepaint() &&
                  IsPaint() )
                PaintContent( pContent, pPage, pContent->getFrameArea(), pContent->getFrameArea().Bottom());
            if ( IsIdle() )
            {
                CheckIdleEnd();
                // consider interrupt formatting.
                if ( IsInterrupt() && !mbFormatContentOnInterrupt )
                    return false;
            }
            if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
                 pContent->getFrameArea().Top() > m_pImp->GetShell()->VisArea().Bottom())
            {
                const long nBottom = m_pImp->GetShell()->VisArea().Bottom();
                const SwFrame *pTmp = lcl_FindFirstInvaContent( pPage,
                                                    nBottom, pContent );
                if ( !pTmp )
                {
                    if ( (!(pPage->GetSortedObjs() && pPage->IsInvalidFly()) ||
                            !lcl_FindFirstInvaObj( pPage, nBottom )) &&
                            (!pPage->IsInvalidLayout() ||
                            !lcl_FindFirstInvaLay( pPage, nBottom )))
                        SetBrowseActionStop( true );
                    // consider interrupt formatting.
                    if ( !mbFormatContentOnInterrupt )
                    {
                        return false;
                    }
                }
            }
            pContent = pContent->GetNextContentFrame();
        }
    }
    CheckWaitCursor();
    // consider interrupt formatting.
    return !IsInterrupt() || mbFormatContentOnInterrupt;
}

void SwLayAction::FormatContent_( const SwContentFrame *pContent, const SwPageFrame  *pPage )
{
    // We probably only ended up here because the Content holds DrawObjects.
    const bool bDrawObjsOnly = pContent->isFrameAreaDefinitionValid() && !pContent->IsCompletePaint() && !pContent->IsRetouche();
    SwRectFnSet aRectFnSet(pContent);
    if ( !bDrawObjsOnly && IsPaint() )
    {
        const SwRect aOldRect( pContent->UnionFrame() );
        const long nOldBottom = aRectFnSet.GetPrtBottom(*pContent);
        pContent->OptCalc();
        if( IsAgain() )
            return;
        if( aRectFnSet.YDiff( aRectFnSet.GetBottom(pContent->getFrameArea()),
                                aRectFnSet.GetBottom(aOldRect) ) < 0 )
        {
            pContent->SetRetouche();
        }
        PaintContent( pContent, pContent->FindPageFrame(), aOldRect, nOldBottom);
    }
    else
    {
        if ( IsPaint() && pContent->IsTextFrame() && static_cast<const SwTextFrame*>(pContent)->HasRepaint() )
            PaintContent( pContent, pPage, pContent->getFrameArea(),
                        aRectFnSet.GetBottom(pContent->getFrameArea()) );
        pContent->OptCalc();
    }
}

void SwLayAction::FormatFlyContent( const SwFlyFrame *pFly )
{
    const SwContentFrame *pContent = pFly->ContainsContent();

    while ( pContent )
    {
        FormatContent_( pContent, pContent->FindPageFrame() );

        // format floating screen objects at content text frame
        // pass correct page frame to the object formatter.
        if ( pContent->IsTextFrame() &&
             !SwObjectFormatter::FormatObjsAtFrame(
                                            *const_cast<SwContentFrame*>(pContent),
                                            *(pContent->FindPageFrame()), this ) )
        {
            // restart format with first content
            pContent = pFly->ContainsContent();
            continue;
        }

        if ( !pContent->GetValidLineNumFlag() && pContent->IsTextFrame() )
        {
            const sal_uLong nAllLines = static_cast<const SwTextFrame*>(pContent)->GetAllLines();
            const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pContent))->RecalcAllLines();
            if ( IsPaintExtraData() && IsPaint() &&
                 nAllLines != static_cast<const SwTextFrame*>(pContent)->GetAllLines() )
                m_pImp->GetShell()->AddPaintRect( pContent->getFrameArea() );
        }

        if ( IsAgain() )
            return;

        // If there's input, we interrupt processing.
        if ( !pFly->IsFlyInContentFrame() )
        {
            CheckIdleEnd();
            // consider interrupt formatting.
            if ( IsInterrupt() && !mbFormatContentOnInterrupt )
                return;
        }
        pContent = pContent->GetNextContentFrame();
    }
    CheckWaitCursor();
}

bool SwLayIdle::DoIdleJob_( const SwContentFrame *pCnt, IdleJobType eJob )
{
    OSL_ENSURE( pCnt->IsTextFrame(), "NoText neighbour of Text" );
    // robust against misuse by e.g. #i52542#
    if( !pCnt->IsTextFrame() )
        return false;

    SwTextFrame const*const pTextFrame(static_cast<SwTextFrame const*>(pCnt));
    // sw_redlinehide: spell check only the nodes with visible content?
    SwTextNode* pTextNode = const_cast<SwTextNode*>(pTextFrame->GetTextNodeForParaProps());

    bool bProcess = false;
    for (size_t i = 0; pTextNode; )
    {
        switch ( eJob )
        {
            case ONLINE_SPELLING :
                bProcess = pTextNode->IsWrongDirty(); break;
            case AUTOCOMPLETE_WORDS :
                bProcess = pTextNode->IsAutoCompleteWordDirty(); break;
            case WORD_COUNT :
                bProcess = pTextNode->IsWordCountDirty(); break;
            case SMART_TAGS :
                bProcess = pTextNode->IsSmartTagDirty(); break;
        }
        if (bProcess)
        {
            break;
        }
        if (sw::MergedPara const* pMerged = pTextFrame->GetMergedPara())
        {
            while (true)
            {
                ++i;
                if (i < pMerged->extents.size())
                {
                    if (pMerged->extents[i].pNode != pTextNode)
                    {
                        pTextNode = pMerged->extents[i].pNode;
                        break;
                    }
                }
                else
                {
                    pTextNode = nullptr;
                    break;
                }
            }
        }
        else
            pTextNode = nullptr;
    }

    if( bProcess )
    {
        assert(pTextNode);
        SwViewShell *pSh = pImp->GetShell();
        if( COMPLETE_STRING == nTextPos )
        {
            --nTextPos;
            if( dynamic_cast< const SwCursorShell *>( pSh ) != nullptr  && !static_cast<SwCursorShell*>(pSh)->IsTableMode() )
            {
                SwPaM *pCursor = static_cast<SwCursorShell*>(pSh)->GetCursor();
                if( !pCursor->HasMark() && !pCursor->IsMultiSelection() )
                {
                    pContentNode = pCursor->GetContentNode();
                    nTextPos =  pCursor->GetPoint()->nContent.GetIndex();
                }
            }
        }
        sal_Int32 const nPos((pContentNode && pTextNode == pContentNode)
                ? nTextPos
                : COMPLETE_STRING);

        switch ( eJob )
        {
            case ONLINE_SPELLING :
            {
                SwRect aRepaint( const_cast<SwTextFrame*>(pTextFrame)->AutoSpell_(*pTextNode, nPos) );
                // PENDING should stop idle spell checking
                bPageValid = bPageValid && (SwTextNode::WrongState::TODO != pTextNode->GetWrongDirty());
                if ( aRepaint.HasArea() )
                    pImp->GetShell()->InvalidateWindows( aRepaint );
                if (Application::AnyInput(VCL_INPUT_ANY & VclInputFlags(~VclInputFlags::TIMER)))
                    return true;
                break;
            }
            case AUTOCOMPLETE_WORDS :
                const_cast<SwTextFrame*>(pTextFrame)->CollectAutoCmplWrds(*pTextNode, nPos);
                // note: bPageValid remains true here even if the cursor
                // position is skipped, so no PENDING state needed currently
                if (Application::AnyInput(VCL_INPUT_ANY & VclInputFlags(~VclInputFlags::TIMER)))
                    return true;
                break;
            case WORD_COUNT :
            {
                const sal_Int32 nEnd = pTextNode->GetText().getLength();
                SwDocStat aStat;
                pTextNode->CountWords( aStat, 0, nEnd );
                if ( Application::AnyInput() )
                    return true;
                break;
            }
            case SMART_TAGS :
            {
                try {
                    const SwRect aRepaint( const_cast<SwTextFrame*>(pTextFrame)->SmartTagScan(*pTextNode) );
                    bPageValid = bPageValid && !pTextNode->IsSmartTagDirty();
                    if ( aRepaint.HasArea() )
                        pImp->GetShell()->InvalidateWindows( aRepaint );
                } catch( const css::uno::RuntimeException& e) {
                    // handle smarttag problems gracefully and provide diagnostics
                    SAL_WARN( "sw.core", "SMART_TAGS: " << e);
                }
                if (Application::AnyInput(VCL_INPUT_ANY & VclInputFlags(~VclInputFlags::TIMER)))
                    return true;
                break;
            }
        }
    }

    // The Flys that are anchored to the paragraph need to be considered too.
    if ( pCnt->GetDrawObjs() )
    {
        const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
        for (SwAnchoredObject* pObj : rObjs)
        {
            if ( dynamic_cast< const SwFlyFrame *>( pObj ) !=  nullptr )
            {
                SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pObj);
                if ( pFly->IsFlyInContentFrame() )
                {
                    const SwContentFrame *pC = pFly->ContainsContent();
                    while( pC )
                    {
                        if ( pC->IsTextFrame() )
                        {
                            if ( DoIdleJob_( pC, eJob ) )
                                return true;
                        }
                        pC = pC->GetNextContentFrame();
                    }
                }
            }
        }
    }
    return false;
}

bool SwLayIdle::DoIdleJob( IdleJobType eJob, bool bVisAreaOnly )
{
    // Spellcheck all contents of the pages. Either only the
    // visible ones or all of them.
    const SwViewShell* pViewShell = pImp->GetShell();
    const SwViewOption* pViewOptions = pViewShell->GetViewOptions();
    const SwDoc* pDoc = pViewShell->GetDoc();

    switch ( eJob )
    {
        case ONLINE_SPELLING :
            if( !pViewOptions->IsOnlineSpell() )
                return false;
            break;
        case AUTOCOMPLETE_WORDS :
            if( !SwViewOption::IsAutoCompleteWords() ||
                 SwDoc::GetAutoCompleteWords().IsLockWordLstLocked())
                return false;
            break;
        case WORD_COUNT :
            if ( !pViewShell->getIDocumentStatistics().GetDocStat().bModified )
                return false;
            break;
        case SMART_TAGS :
            if ( pDoc->GetDocShell()->IsHelpDocument() ||
                 pDoc->isXForms() ||
                !SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                return false;
            break;
        default: OSL_FAIL( "Unknown idle job type" );
    }

    SwPageFrame *pPage;
    if ( bVisAreaOnly )
        pPage = pImp->GetFirstVisPage(pViewShell->GetOut());
    else
        pPage = static_cast<SwPageFrame*>(pRoot->Lower());

    pContentNode = nullptr;
    nTextPos = COMPLETE_STRING;

    while ( pPage )
    {
        bPageValid = true;
        const SwContentFrame *pCnt = pPage->ContainsContent();
        while( pCnt && pPage->IsAnLower( pCnt ) )
        {
            if ( DoIdleJob_( pCnt, eJob ) )
                return true;
            pCnt = pCnt->GetNextContentFrame();
        }
        if ( pPage->GetSortedObjs() )
        {
            for ( size_t i = 0; pPage->GetSortedObjs() &&
                                i < pPage->GetSortedObjs()->size(); ++i )
            {
                const SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                if ( auto pFly = dynamic_cast< const SwFlyFrame *>( pObj ) )
                {
                    const SwContentFrame *pC = pFly->ContainsContent();
                    while( pC )
                    {
                        if ( pC->IsTextFrame() )
                        {
                            if ( DoIdleJob_( pC, eJob ) )
                                return true;
                        }
                        pC = pC->GetNextContentFrame();
                    }
                }
            }
        }

        if( bPageValid )
        {
            switch ( eJob )
            {
                case ONLINE_SPELLING : pPage->ValidateSpelling(); break;
                case AUTOCOMPLETE_WORDS : pPage->ValidateAutoCompleteWords(); break;
                case WORD_COUNT : pPage->ValidateWordCount(); break;
                case SMART_TAGS : pPage->ValidateSmartTags(); break;
            }
        }

        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
        if ( pPage && bVisAreaOnly &&
             !pPage->getFrameArea().IsOver( pImp->GetShell()->VisArea()))
             break;
    }
    return false;
}

#if HAVE_FEATURE_DESKTOP && defined DBG_UTIL
void SwLayIdle::ShowIdle( Color eColor )
{
    if ( !m_bIndicator )
    {
        m_bIndicator = true;
        vcl::Window *pWin = pImp->GetShell()->GetWin();
        if (pWin && !pWin->SupportsDoubleBuffering()) // FIXME make this work with double-buffering
        {
            tools::Rectangle aRect( 0, 0, 5, 5 );
            aRect = pWin->PixelToLogic( aRect );
            // Depending on if idle layout is in progress or not, draw a "red square" or a "green square".
            pWin->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
            pWin->SetFillColor( eColor );
            pWin->SetLineColor();
            pWin->DrawRect( aRect );
            pWin->Pop();
        }
    }
}
#define SHOW_IDLE( Color ) ShowIdle( Color )
#else
#define SHOW_IDLE( Color )
#endif // DBG_UTIL

SwLayIdle::SwLayIdle( SwRootFrame *pRt, SwViewShellImp *pI ) :
    pRoot( pRt ),
    pImp( pI )
#ifdef DBG_UTIL
    , m_bIndicator( false )
#endif
{
    SAL_INFO("sw.idle", "SwLayIdle() entry");

    pImp->m_pIdleAct = this;

    SHOW_IDLE( COL_LIGHTRED );

    pImp->GetShell()->EnableSmooth( false );

    // First, spellcheck the visible area. Only if there's nothing
    // to do there, we trigger the IdleFormat.
    if ( !DoIdleJob( SMART_TAGS, true ) &&
         !DoIdleJob( ONLINE_SPELLING, true ) &&
         !DoIdleJob( AUTOCOMPLETE_WORDS, true ) )
    {
        // Format, then register repaint rectangles with the SwViewShell if necessary.
        // This requires running artificial actions, so we don't get undesired
        // effects when for instance the page count gets changed.
        // We remember the shells where the cursor is visible, so we can make
        // it visible again if needed after a document change.
        std::vector<bool> aBools;
        for(SwViewShell& rSh : pImp->GetShell()->GetRingContainer())
        {
            ++rSh.mnStartAction;
            bool bVis = false;
            if ( dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr )
            {
                bVis = static_cast<SwCursorShell*>(&rSh)->GetCharRect().IsOver(rSh.VisArea());
            }
            aBools.push_back( bVis );
        }

        bool bInterrupt(false);
        {
            SwLayAction aAction( pRoot, pImp );
            aAction.SetInputType( VCL_INPUT_ANY );
            aAction.SetIdle( true );
            aAction.SetWaitAllowed( false );
            aAction.Action(pImp->GetShell()->GetOut());
            bInterrupt = aAction.IsInterrupt();
        }

        // Further start/end actions only happen if there were paints started
        // somewhere or if the visibility of the CharRects has changed.
        bool bActions = false;
        size_t nBoolIdx = 0;
        for(SwViewShell& rSh : pImp->GetShell()->GetRingContainer())
        {
            --rSh.mnStartAction;

            if ( rSh.Imp()->GetRegion() )
                bActions = true;
            else
            {
                SwRect aTmp( rSh.VisArea() );
                rSh.UISizeNotify();

                // Are we supposed to crash if rSh isn't a cursor shell?!
                // bActions |= aTmp != rSh.VisArea() ||
                //             aBools[nBoolIdx] != ((SwCursorShell*)&rSh)->GetCharRect().IsOver( rSh.VisArea() );

                // aBools[ i ] is true, if the i-th shell is a cursor shell (!!!)
                // and the cursor is visible.
                bActions |= aTmp != rSh.VisArea();
                if ( aTmp == rSh.VisArea() && dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr )
                {
                    bActions |= aBools[nBoolIdx] !=
                                 static_cast<SwCursorShell*>(&rSh)->GetCharRect().IsOver( rSh.VisArea() );
                }
            }

            ++nBoolIdx;
        }

        if ( bActions )
        {
            // Prepare start/end actions via CursorShell, so the cursor, selection
            // and VisArea can be set correctly.
            nBoolIdx = 0;
            for(SwViewShell& rSh : pImp->GetShell()->GetRingContainer())
            {
                SwCursorShell* pCursorShell = nullptr;
                if(dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr)
                    pCursorShell = static_cast<SwCursorShell*>(&rSh);

                if ( pCursorShell )
                    pCursorShell->SttCursorMove();

                // If there are accrued paints, it's best to simply invalidate
                // the whole window. Otherwise there would arise paint problems whose
                // solution would be disproportionally expensive.
                SwViewShellImp *pViewImp = rSh.Imp();
                bool bUnlock = false;
                if ( pViewImp->GetRegion() )
                {
                    pViewImp->DelRegion();

                    // Cause a repaint with virtual device.
                    rSh.LockPaint();
                    bUnlock = true;
                }

                if ( pCursorShell )
                    // If the Cursor was visible, we need to make it visible again.
                    // Otherwise, EndCursorMove with true for IdleEnd
                    pCursorShell->EndCursorMove( !aBools[nBoolIdx] );
                if( bUnlock )
                {
                    if( pCursorShell )
                    {
                        // UnlockPaint overwrite the selection from the
                        // CursorShell and calls the virtual method paint
                        // to fill the virtual device. This fill don't have
                        // paint the selection! -> Set the focus flag at
                        // CursorShell and it doesn't paint the selection.
                        pCursorShell->ShellLoseFocus();
                        pCursorShell->UnlockPaint( true );
                        pCursorShell->ShellGetFocus();
                    }
                    else
                        rSh.UnlockPaint( true );
                }
                ++nBoolIdx;

            }
        }

        if (!bInterrupt)
        {
            if ( !DoIdleJob( WORD_COUNT, false ) )
                if ( !DoIdleJob( SMART_TAGS, false ) )
                    if ( !DoIdleJob( ONLINE_SPELLING, false ) )
                        DoIdleJob( AUTOCOMPLETE_WORDS, false );
        }

        bool bInValid = false;
        const SwViewOption& rVOpt = *pImp->GetShell()->GetViewOptions();
        const SwViewShell* pViewShell = pImp->GetShell();
        // See conditions in DoIdleJob()
        const bool bSpell     = rVOpt.IsOnlineSpell();
        const bool bACmplWrd  = SwViewOption::IsAutoCompleteWords();
        const bool bWordCount = pViewShell->getIDocumentStatistics().GetDocStat().bModified;
        const bool bSmartTags = !pViewShell->GetDoc()->GetDocShell()->IsHelpDocument() &&
                                !pViewShell->GetDoc()->isXForms() &&
                                SwSmartTagMgr::Get().IsSmartTagsEnabled();

        SwPageFrame *pPg = static_cast<SwPageFrame*>(pRoot->Lower());
        do
        {
            bInValid = pPg->IsInvalidContent()    || pPg->IsInvalidLayout() ||
                       pPg->IsInvalidFlyContent() || pPg->IsInvalidFlyLayout() ||
                       pPg->IsInvalidFlyInCnt() ||
                       (bSpell && pPg->IsInvalidSpelling()) ||
                       (bACmplWrd && pPg->IsInvalidAutoCompleteWords()) ||
                       (bWordCount && pPg->IsInvalidWordCount()) ||
                       (bSmartTags && pPg->IsInvalidSmartTags());

            pPg = static_cast<SwPageFrame*>(pPg->GetNext());

        } while ( pPg && !bInValid );

        if ( !bInValid )
        {
            pRoot->ResetIdleFormat();
            SfxObjectShell* pDocShell = pImp->GetShell()->GetDoc()->GetDocShell();
            pDocShell->Broadcast( SfxEventHint( SfxEventHintId::SwEventLayoutFinished, SwDocShell::GetEventName(STR_SW_EVENT_LAYOUT_FINISHED), pDocShell ) );
        }
    }

    pImp->GetShell()->EnableSmooth( true );

    if( pImp->IsAccessible() )
        pImp->FireAccessibleEvents();

    SAL_INFO("sw.idle", "SwLayIdle() return");

#ifdef DBG_UTIL
    if ( m_bIndicator && pImp->GetShell()->GetWin() )
    {
        // Do not invalidate indicator, this may cause a endless loop. Instead, just repaint it
        // This should be replaced by an overlay object in the future, anyways. Since it's only for debug
        // purposes, it is not urgent.
            m_bIndicator = false; SHOW_IDLE( COL_LIGHTGREEN );
    }
#endif
}

SwLayIdle::~SwLayIdle()
{
    pImp->m_pIdleAct = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
