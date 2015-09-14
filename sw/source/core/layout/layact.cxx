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
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewimp.hxx"
#include "crsrsh.hxx"
#include "dflyobj.hxx"
#include "frmtool.hxx"
#include "dcontact.hxx"
#include "viewopt.hxx"
#include "dbg_lay.hxx"
#include "layouter.hxx"
#include "docstat.hxx"
#include "swevent.hxx"
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <sfx2/event.hxx>

#include <ftnidx.hxx>
#include <vcl/svapp.hxx>
#include <editeng/opaqitem.hxx>
#include <SwSmartTagMgr.hxx>

#include "layact.hxx"
#include <swwait.hxx>
#include <fmtsrnd.hxx>
#include <docsh.hxx>

#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "mdiexp.hxx"
#include "sectfrm.hxx"
#include <acmplwrd.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <vector>

// SwLayAction static stuff

#define IS_FLYS (pPage->GetSortedObjs())
#define IS_INVAFLY (pPage->IsInvalidFly())

// Save some typing work to avoid accessing destroyed pages.
#if OSL_DEBUG_LEVEL > 1

static void BreakPoint()
{
    return;
}

#define XCHECKPAGE \
            {   if ( IsAgain() ) \
                {   BreakPoint(); \
                    if( bNoLoop ) \
                        pLayoutAccess->GetLayouter()->EndLoopControl(); \
                    return; \
                } \
            }
#else
#define XCHECKPAGE \
            {   if ( IsAgain() ) \
                { \
                    if( bNoLoop ) \
                        pLayoutAccess->GetLayouter()->EndLoopControl(); \
                    return; \
                } \
            }
#endif

#define RESCHEDULE \
    { \
        if ( IsReschedule() )  \
        { \
            ::RescheduleProgress( pImp->GetShell()->GetDoc()->GetDocShell() ); \
        } \
    }

void SwLayAction::CheckWaitCrsr()
{
    RESCHEDULE
    if ( !IsWait() && IsWaitAllowed() && IsPaint() &&
         ((std::clock() - GetStartTicks()) * 1000 / CLOCKS_PER_SEC >= CLOCKS_PER_SEC/2) )
    {
        pWait = new SwWait( *pRoot->GetFormat()->GetDoc()->GetDocShell(), true );
    }
}

// Time over already?
inline void SwLayAction::CheckIdleEnd()
{
    if ( !IsInput() )
        bInput = bool(GetInputType()) && Application::AnyInput( GetInputType() );
}

void SwLayAction::SetStatBar( bool bNew )
{
    if ( bNew )
    {
        nEndPage = pRoot->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
    }
    else
        nEndPage = USHRT_MAX;
}

bool SwLayAction::PaintWithoutFlys( const SwRect &rRect, const SwContentFrm *pCnt,
                                    const SwPageFrm *pPage )
{
    SwRegionRects aTmp( rRect );
    const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm *pSelfFly = pCnt->FindFlyFrm();

    for ( size_t i = 0; i < rObjs.size() && !aTmp.empty(); ++i )
    {
        SdrObject *pO = rObjs[i]->DrawObj();
        if ( !pO->ISA(SwVirtFlyDrawObj) )
            continue;

        // OD 2004-01-15 #110582# - do not consider invisible objects
        const IDocumentDrawModelAccess* pIDDMA = pPage->GetFormat()->getIDocumentDrawModelAccess();
        if ( !pIDDMA->IsVisibleLayerId( pO->GetLayer() ) )
        {
            continue;
        }

        SwFlyFrm *pFly = static_cast<SwVirtFlyDrawObj*>(pO)->GetFlyFrm();

        if ( pFly == pSelfFly || !rRect.IsOver( pFly->Frm() ) )
            continue;

        if ( pSelfFly && pSelfFly->IsLowerOf( pFly ) )
            continue;

        if ( pFly->GetVirtDrawObj()->GetLayer() == pIDDMA->GetHellId() )
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

        // OD 19.08.2002 #99657#
        //     Fly frame without a lower have to be subtracted from paint region.
        //     For checking, if fly frame contains transparent graphic or
        //     has surrounded contour, assure that fly frame has a lower
        if ( pFly->Lower() &&
             pFly->Lower()->IsNoTextFrm() &&
             ( static_cast<SwNoTextFrm*>(pFly->Lower())->IsTransparent() ||
               pFly->GetFormat()->GetSurround().IsContour() )
           )
        {
            continue;
        }

        // OD 19.08.2002 #99657#
        //     vcl::Region of a fly frame with transparent background or a transparent
        //     shadow have not to be subtracted from paint region
        if ( pFly->IsBackgroundTransparent() )
        {
            continue;
        }

        aTmp -= pFly->Frm();
    }

    bool bRetPaint = false;
    for ( SwRects::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it )
        bRetPaint |= pImp->GetShell()->AddPaintRect( *it );
    return bRetPaint;
}

inline bool SwLayAction::_PaintContent( const SwContentFrm *pContent,
                                      const SwPageFrm *pPage,
                                      const SwRect &rRect )
{
    if ( rRect.HasArea() )
    {
        if ( pPage->GetSortedObjs() )
            return PaintWithoutFlys( rRect, pContent, pPage );
        else
            return pImp->GetShell()->AddPaintRect( rRect );
    }
    return false;
}

/**
 * Depending of the type, the Content is output according to it's changes, or the area
 * to be outputted is registered with the region, respectively.
 */
void SwLayAction::PaintContent( const SwContentFrm *pCnt,
                              const SwPageFrm *pPage,
                              const SwRect &rOldRect,
                              long nOldBottom )
{
    SWRECTFN( pCnt )

    if ( pCnt->IsCompletePaint() || !pCnt->IsTextFrm() )
    {
        SwRect aPaint( pCnt->PaintArea() );
        if ( !_PaintContent( pCnt, pPage, aPaint ) )
            pCnt->ResetCompletePaint();
    }
    else
    {
        // paint the area between printing bottom and frame bottom and
        // the area left and right beside the frame, if its height changed.
        long nOldHeight = (rOldRect.*fnRect->fnGetHeight)();
        long nNewHeight = (pCnt->Frm().*fnRect->fnGetHeight)();
        const bool bHeightDiff = nOldHeight != nNewHeight;
        if( bHeightDiff )
        {
            // OD 05.11.2002 #94454# - consider whole potential paint area.
            SwRect aDrawRect( pCnt->PaintArea() );
            if( nOldHeight > nNewHeight )
                nOldBottom = (pCnt->*fnRect->fnGetPrtBottom)();
            (aDrawRect.*fnRect->fnSetTop)( nOldBottom );
            _PaintContent( pCnt, pPage, aDrawRect );
        }
        // paint content area
        SwRect aPaintRect = static_cast<SwTextFrm*>(const_cast<SwContentFrm*>(pCnt))->Paint();
        _PaintContent( pCnt, pPage, aPaintRect );
    }

    if ( pCnt->IsRetouche() && !pCnt->GetNext() )
    {
        const SwFrm *pTmp = pCnt;
        if( pCnt->IsInSct() )
        {
            const SwSectionFrm* pSct = pCnt->FindSctFrm();
            if( pSct->IsRetouche() && !pSct->GetNext() )
                pTmp = pSct;
        }
        SwRect aRect( pTmp->GetUpper()->PaintArea() );
        (aRect.*fnRect->fnSetTop)( (pTmp->*fnRect->fnGetPrtBottom)() );
        if ( !_PaintContent( pCnt, pPage, aRect ) )
            pCnt->ResetRetouche();
    }
}

SwLayAction::SwLayAction( SwRootFrm *pRt, SwViewShellImp *pI ) :
    pRoot( pRt ),
    pImp( pI ),
    pOptTab( 0 ),
    pWait( 0 ),
    nPreInvaPage( USHRT_MAX ),
    nStartTicks( std::clock() ),
    nInputType( VclInputFlags::NONE ),
    nEndPage( USHRT_MAX ),
    nCheckPageNum( USHRT_MAX )
{
    bPaintExtraData = ::IsExtraData( pImp->GetShell()->GetDoc() );
    bPaint = bComplete = bWaitAllowed = bCheckPages = true;
    bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
    bUpdateExpFields = bBrowseActionStop = bActionInProgress = false;
    // OD 14.04.2003 #106346# - init new flag <mbFormatContentOnInterrupt>.
    mbFormatContentOnInterrupt = false;

    assert(!pImp->pLayAct); // there can be only one SwLayAction
    pImp->pLayAct = this;   // register there
}

SwLayAction::~SwLayAction()
{
    OSL_ENSURE( !pWait, "Wait object not destroyed" );
    pImp->pLayAct = 0;      // unregister
}

void SwLayAction::Reset()
{
    pOptTab = 0;
    nStartTicks = std::clock();
    nInputType = VclInputFlags::NONE;
    nEndPage = nPreInvaPage = nCheckPageNum = USHRT_MAX;
    bPaint = bComplete = bWaitAllowed = bCheckPages = true;
    bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
    bUpdateExpFields = bBrowseActionStop = false;
}

bool SwLayAction::RemoveEmptyBrowserPages()
{
    // switching from the normal to the browser mode, empty pages may be
    // retained for an annoyingly long time, so delete them here
    bool bRet = false;
    const SwViewShell *pSh = pRoot->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
    {
        SwPageFrm *pPage = static_cast<SwPageFrm*>(pRoot->Lower());
        do
        {
            if ( (pPage->GetSortedObjs() && pPage->GetSortedObjs()->size()) ||
                 pPage->ContainsContent() )
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            else
            {
                bRet = true;
                SwPageFrm *pDel = pPage;
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                pDel->Cut();
                SwFrm::DestroyFrm(pDel);
            }
        } while ( pPage );
    }
    return bRet;
}

void SwLayAction::Action(OutputDevice* pRenderContext)
{
    bActionInProgress = true;

    //TurboMode? Hands-off during idle-format
    if ( IsPaint() && !IsIdle() && TurboAction() )
    {
        delete pWait, pWait = 0;
        pRoot->ResetTurboFlag();
        bActionInProgress = false;
        pRoot->DeleteEmptySct();
        return;
    }
    else if ( pRoot->GetTurbo() )
    {
        pRoot->DisallowTurbo();
        const SwFrm *pFrm = pRoot->GetTurbo();
        pRoot->ResetTurbo();
        pFrm->InvalidatePage();
    }
    pRoot->DisallowTurbo();

    if ( IsCalcLayout() )
        SetCheckPages( false );

    InternalAction(pRenderContext);
    bAgain |= RemoveEmptyBrowserPages();
    while ( IsAgain() )
    {
        bAgain = bNextCycle = false;
        InternalAction(pRenderContext);
        bAgain |= RemoveEmptyBrowserPages();
    }
    pRoot->DeleteEmptySct();

    delete pWait, pWait = 0;

    //Turbo-Action permitted again for all cases.
    pRoot->ResetTurboFlag();
    pRoot->ResetTurbo();

    SetCheckPages( true );

    bActionInProgress = false;
}

SwPageFrm* SwLayAction::CheckFirstVisPage( SwPageFrm *pPage )
{
    SwContentFrm *pCnt = pPage->FindFirstBodyContent();
    SwContentFrm *pChk = pCnt;
    bool bPageChgd = false;
    while ( pCnt && pCnt->IsFollow() )
        pCnt = static_cast<SwContentFrm*>(pCnt)->FindMaster();
    if ( pCnt && pChk != pCnt )
    {   bPageChgd = true;
        pPage = pCnt->FindPageFrm();
    }

    if ( !pPage->GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
    {
        SwFootnoteContFrm *pCont = pPage->FindFootnoteCont();
        if ( pCont )
        {
            pCnt = pCont->ContainsContent();
            pChk = pCnt;
            while ( pCnt && pCnt->IsFollow() )
                pCnt = static_cast<SwContentFrm*>(pCnt->FindPrev());
            if ( pCnt && pCnt != pChk )
            {
                if ( bPageChgd )
                {
                    // Use the 'topmost' page
                    SwPageFrm *pTmp = pCnt->FindPageFrm();
                    if ( pPage->GetPhyPageNum() > pTmp->GetPhyPageNum() )
                        pPage = pTmp;
                }
                else
                    pPage = pCnt->FindPageFrm();
            }
        }
    }
    return pPage;
}

// #114798# - unlock position on start and end of page
// layout process.
static void unlockPositionOfObjects( SwPageFrm *pPageFrm )
{
    assert( pPageFrm );

    SwSortedObjs* pObjs = pPageFrm->GetSortedObjs();
    if ( pObjs )
    {
        for ( size_t i = 0; i < pObjs->size(); ++i )
        {
            SwAnchoredObject* pObj = (*pObjs)[i];
            pObj->UnlockPosition();
        }
    }
}

void SwLayAction::InternalAction(OutputDevice* pRenderContext)
{
    OSL_ENSURE( pRoot->Lower()->IsPageFrm(), ":-( No page below the root.");

    pRoot->Calc(pRenderContext);

    // Figure out the first invalid page or the first one to be formatted,
    // respectively. A complete-action means the first invalid page.
    // However, the first page to be formatted might be the one having the
    // number 1.  If we're doing a fake formatting, the number of the first
    // page is the number of the first visible page.
    SwPageFrm *pPage = IsComplete() ? static_cast<SwPageFrm*>(pRoot->Lower()) :
                pImp->GetFirstVisPage(pRenderContext);
    if ( !pPage )
        pPage = static_cast<SwPageFrm*>(pRoot->Lower());

    // If there's a first-flow-Content in the first visible page that's also a Follow,
    // we switch the page back to the original master of that Content.
    if ( !IsComplete() )
        pPage = CheckFirstVisPage( pPage );
    sal_uInt16 nFirstPageNum = pPage->GetPhyPageNum();

    while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());

    IDocumentLayoutAccess *pLayoutAccess = pRoot->GetFormat()->getIDocumentLayoutAccess();
    bool bNoLoop = pPage && SwLayouter::StartLoopControl( pRoot->GetFormat()->GetDoc(), pPage );
    sal_uInt16 nPercentPageNum = 0;
    while ( (pPage && !IsInterrupt()) || nCheckPageNum != USHRT_MAX )
    {
        if (!pPage || (nCheckPageNum != USHRT_MAX && pPage->GetPhyPageNum() >= nCheckPageNum))
        {
            if ( !pPage || pPage->GetPhyPageNum() > nCheckPageNum )
            {
                SwPageFrm *pPg = static_cast<SwPageFrm*>(pRoot->Lower());
                while ( pPg && pPg->GetPhyPageNum() < nCheckPageNum )
                    pPg = static_cast<SwPageFrm*>(pPg->GetNext());
                if ( pPg )
                    pPage = pPg;
                if ( !pPage )
                    break;
            }
            SwPageFrm *pTmp = pPage->GetPrev() ?
                                        static_cast<SwPageFrm*>(pPage->GetPrev()) : pPage;
            SetCheckPages( true );
            SwFrm::CheckPageDescs( pPage, true, &pTmp );
            SetCheckPages( false );
            nCheckPageNum = USHRT_MAX;
            pPage = pTmp;
            continue;
        }

        if ( nEndPage != USHRT_MAX && pPage->GetPhyPageNum() > nPercentPageNum )
        {
            nPercentPageNum = pPage->GetPhyPageNum();
            ::SetProgressState( nPercentPageNum, pImp->GetShell()->GetDoc()->GetDocShell());
        }
        pOptTab = 0;
             // No Shortcut for Idle or CalcLayout
        if ( !IsIdle() && !IsComplete() && IsShortCut( pPage ) )
        {
            pRoot->DeleteEmptySct();
            XCHECKPAGE;
            if ( !IsInterrupt() &&
                 (pRoot->IsSuperfluous() || pRoot->IsAssertFlyPages()) )
            {
                if ( pRoot->IsAssertFlyPages() )
                    pRoot->AssertFlyPages();
                if ( pRoot->IsSuperfluous() )
                {
                    bool bOld = IsAgain();
                    pRoot->RemoveSuperfluous();
                    bAgain = bOld;
                }
                if ( IsAgain() )
                {
                    if( bNoLoop )
                        pLayoutAccess->GetLayouter()->EndLoopControl();
                    return;
                }
                pPage = static_cast<SwPageFrm*>(pRoot->Lower());
                while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
                    pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                while ( pPage && pPage->GetNext() &&
                        pPage->GetPhyPageNum() < nFirstPageNum )
                    pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                continue;
            }
            break;
        }
        else
        {
            pRoot->DeleteEmptySct();
            XCHECKPAGE;

            while ( !IsInterrupt() && !IsNextCycle() &&
                    ((IS_FLYS && IS_INVAFLY) || pPage->IsInvalid()) )
            {
                unlockPositionOfObjects( pPage );

                // #i28701#
                SwObjectFormatter::FormatObjsAtFrm( *pPage, *pPage, this );
                if ( !IS_FLYS )
                {
                    // If there are no (more) Flys, the flags are superfluous.
                    pPage->ValidateFlyLayout();
                    pPage->ValidateFlyContent();
                }
                // #i28701# - change condition
                while ( !IsInterrupt() && !IsNextCycle() &&
                        ( pPage->IsInvalid() ||
                          (IS_FLYS && IS_INVAFLY) ) )
                {
                    PROTOCOL( pPage, PROT_FILE_INIT, 0, 0)
                    XCHECKPAGE;

                    // #i81146# new loop control
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
                    // #i28701# - change condition
                    if ( !IsNextCycle() &&
                         ( pPage->IsInvalidContent() ||
                           (IS_FLYS && IS_INVAFLY) ) )
                    {
                        pPage->ValidateFlyInCnt();
                        pPage->ValidateContent();
                        // #i28701#
                        pPage->ValidateFlyLayout();
                        pPage->ValidateFlyContent();
                        if ( !FormatContent( pPage ) )
                        {
                            XCHECKPAGE;
                            pPage->InvalidateContent();
                            pPage->InvalidateFlyInCnt();
                            // #i28701#
                            pPage->InvalidateFlyLayout();
                            pPage->InvalidateFlyContent();
                            if ( IsBrowseActionStop() )
                                bInput = true;
                        }
                    }
                    if( bNoLoop )
                        pLayoutAccess->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
                }

                unlockPositionOfObjects( pPage );
            }

            // A previous page may be invalid again.
            XCHECKPAGE;
            if ( !IS_FLYS )
            {
                // If there are no (more) Flys, the flags are superfluous.
                pPage->ValidateFlyLayout();
                pPage->ValidateFlyContent();
            }
            if ( !IsInterrupt() )
            {
                SetNextCycle( false );

                if ( nPreInvaPage != USHRT_MAX )
                {
                    if( !IsComplete() && nPreInvaPage + 2 < nFirstPageNum )
                    {
                        pImp->SetFirstVisPageInvalid();
                        SwPageFrm *pTmpPage = pImp->GetFirstVisPage(pRenderContext);
                        nFirstPageNum = pTmpPage->GetPhyPageNum();
                        if( nPreInvaPage < nFirstPageNum )
                        {
                            nPreInvaPage = nFirstPageNum;
                            pPage = pTmpPage;
                        }
                    }
                    while ( pPage->GetPrev() && pPage->GetPhyPageNum() > nPreInvaPage )
                        pPage = static_cast<SwPageFrm*>(pPage->GetPrev());
                    nPreInvaPage = USHRT_MAX;
                }

                while ( pPage->GetPrev() &&
                        ( static_cast<SwPageFrm*>(pPage->GetPrev())->IsInvalid() ||
                          ( static_cast<SwPageFrm*>(pPage->GetPrev())->GetSortedObjs() &&
                            static_cast<SwPageFrm*>(pPage->GetPrev())->IsInvalidFly())) &&
                        (static_cast<SwPageFrm*>(pPage->GetPrev())->GetPhyPageNum() >=
                            nFirstPageNum) )
                {
                    pPage = static_cast<SwPageFrm*>(pPage->GetPrev());
                }

                // Continue to the next invalid page
                while ( pPage && !pPage->IsInvalid() &&
                        (!IS_FLYS || !IS_INVAFLY) )
                {
                    pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                }
                if( bNoLoop )
                    pLayoutAccess->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
            }
            CheckIdleEnd();
        }
        if ( !pPage && !IsInterrupt() &&
             (pRoot->IsSuperfluous() || pRoot->IsAssertFlyPages()) )
        {
            if ( pRoot->IsAssertFlyPages() )
                pRoot->AssertFlyPages();
            if ( pRoot->IsSuperfluous() )
            {
                bool bOld = IsAgain();
                pRoot->RemoveSuperfluous();
                bAgain = bOld;
            }
            if ( IsAgain() )
            {
                if( bNoLoop )
                    pLayoutAccess->GetLayouter()->EndLoopControl();
                return;
            }
            pPage = static_cast<SwPageFrm*>(pRoot->Lower());
            while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            while ( pPage && pPage->GetNext() &&
                    pPage->GetPhyPageNum() < nFirstPageNum )
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
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
        // register itself, because it's (LayoutFrm) flags have been reset
        // already - the border of the page will never be painted.
        SwPageFrm *pPg = pPage;
        XCHECKPAGE;
        const SwRect &rVis = pImp->GetShell()->VisArea();

        while( pPg && pPg->Frm().Bottom() < rVis.Top() )
            pPg = static_cast<SwPageFrm*>(pPg->GetNext());
        if( pPg != pPage )
            pPg = pPg ? static_cast<SwPageFrm*>(pPg->GetPrev()) : pPage;

        // set flag for interrupt content formatting
        mbFormatContentOnInterrupt = IsInput();
        long nBottom = rVis.Bottom();
        // #i42586# - format current page, if idle action is active
        // This is an optimization for the case that the interrupt is created by
        // the move of a form control object, which is represented by a window.
        while ( pPg && ( pPg->Frm().Top() < nBottom ||
                         ( IsIdle() && pPg == pPage ) ) )
        {
            unlockPositionOfObjects( pPg );

            XCHECKPAGE;

            // #i81146# new loop control
            int nLoopControlRuns_2 = 0;
            const int nLoopControlMax = 20;

            // special case: interrupt content formatting
            // #i28701# - conditions are incorrect (macros IS_FLYS and IS_INVAFLY only
            //            works for <pPage>) and are too strict.
            // #i50432# - adjust interrupt formatting to normal page formatting - see above.
            while ( ( mbFormatContentOnInterrupt &&
                      ( pPg->IsInvalid() ||
                        ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) ) ||
                    ( !mbFormatContentOnInterrupt && pPg->IsInvalidLayout() ) )
            {
                XCHECKPAGE;
                // #i50432# - format also at-page anchored objects
                SwObjectFormatter::FormatObjsAtFrm( *pPg, *pPg, this );
                if ( !pPg->GetSortedObjs() )
                {
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyContent();
                }

                // #i81146# new loop control
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

                // #i50432#
                if ( mbFormatContentOnInterrupt &&
                     ( pPg->IsInvalidContent() ||
                       ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) )
                {
                    pPg->ValidateFlyInCnt();
                    pPg->ValidateContent();
                    // #i26945#
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
                        // #i26945#
                        pPg->InvalidateFlyLayout();
                        pPg->InvalidateFlyContent();
                    }
                    // #i46807# - we are statisfied, if the content is formatted once complete.
                    else
                    {
                        break;
                    }
                }
            }

            unlockPositionOfObjects( pPg );
            pPg = static_cast<SwPageFrm*>(pPg->GetNext());
        }
        // reset flag for special interrupt content formatting.
        mbFormatContentOnInterrupt = false;
    }
    pOptTab = 0;
    if( bNoLoop )
        pLayoutAccess->GetLayouter()->EndLoopControl();
}

bool SwLayAction::_TurboAction( const SwContentFrm *pCnt )
{

    const SwPageFrm *pPage = 0;
    if ( !pCnt->IsValid() || pCnt->IsCompletePaint() || pCnt->IsRetouche() )
    {
        const SwRect aOldRect( pCnt->UnionFrm( true ) );
        const long   nOldBottom = pCnt->Frm().Top() + pCnt->Prt().Bottom();
        pCnt->Calc(pImp->GetShell()->GetOut());
        if ( pCnt->Frm().Bottom() < aOldRect.Bottom() )
            pCnt->SetRetouche();

        pPage = pCnt->FindPageFrm();
        PaintContent( pCnt, pPage, aOldRect, nOldBottom );

        if ( !pCnt->GetValidLineNumFlag() && pCnt->IsTextFrm() )
        {
            const sal_uLong nAllLines = static_cast<const SwTextFrm*>(pCnt)->GetAllLines();
            const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pCnt))->RecalcAllLines();
            if ( nAllLines != static_cast<const SwTextFrm*>(pCnt)->GetAllLines() )
            {
                if ( IsPaintExtraData() )
                    pImp->GetShell()->AddPaintRect( pCnt->Frm() );
                // This is to calculate the remaining LineNums on the page,
                // and we don't stop processing here. To perform this inside RecalcAllLines
                // would be expensive, because we would have to notify the page even
                // in unnecessary cases (normal actions).
                const SwContentFrm *pNxt = pCnt->GetNextContentFrm();
                while ( pNxt &&
                        (pNxt->IsInTab() || pNxt->IsInDocBody() != pCnt->IsInDocBody()) )
                    pNxt = pNxt->GetNextContentFrm();
                if ( pNxt )
                    pNxt->InvalidatePage();
            }
            return false;
        }

        if ( pPage->IsInvalidLayout() || (IS_FLYS && IS_INVAFLY) )
            return false;
    }
    if ( !pPage )
        pPage = pCnt->FindPageFrm();

    // OD 2004-05-10 #i28701# - format floating screen objects at content frame.
    if ( pCnt->IsTextFrm() &&
         !SwObjectFormatter::FormatObjsAtFrm( *(const_cast<SwContentFrm*>(pCnt)),
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

    if ( pRoot->GetTurbo() )
    {
        if ( !_TurboAction( pRoot->GetTurbo() ) )
        {
            CheckIdleEnd();
            bRet = false;
        }
        pRoot->ResetTurbo();
    }
    else
        bRet = false;
    return bRet;
}

static bool lcl_IsInvaLay( const SwFrm *pFrm, long nBottom )
{
    if (
         !pFrm->IsValid() ||
         (pFrm->IsCompletePaint() && ( pFrm->Frm().Top() < nBottom ) )
       )
    {
        return true;
    }
    return false;
}

static const SwFrm *lcl_FindFirstInvaLay( const SwFrm *pFrm, long nBottom )
{
    OSL_ENSURE( pFrm->IsLayoutFrm(), "FindFirstInvaLay, no LayFrm" );

    if (lcl_IsInvaLay(pFrm, nBottom))
        return pFrm;
    pFrm = static_cast<const SwLayoutFrm*>(pFrm)->Lower();
    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
        {
            if (lcl_IsInvaLay(pFrm, nBottom))
                return pFrm;
            const SwFrm *pTmp;
            if ( 0 != (pTmp = lcl_FindFirstInvaLay( pFrm, nBottom )) )
                return pTmp;
        }
        pFrm = pFrm->GetNext();
    }
    return 0;
}

static const SwFrm *lcl_FindFirstInvaContent( const SwLayoutFrm *pLay, long nBottom,
                                     const SwContentFrm *pFirst )
{
    const SwContentFrm *pCnt = pFirst ? pFirst->GetNextContentFrm() :
                                      pLay->ContainsContent();
    while ( pCnt )
    {
        if ( !pCnt->IsValid() || pCnt->IsCompletePaint() )
        {
            if ( pCnt->Frm().Top() <= nBottom )
                return pCnt;
        }

        if ( pCnt->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                const SwAnchoredObject* pObj = rObjs[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pObj);
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        if ( static_cast<const SwFlyInCntFrm*>(pFly)->IsInvalid() ||
                             pFly->IsCompletePaint() )
                        {
                            if ( pFly->Frm().Top() <= nBottom )
                                return pFly;
                        }
                        const SwFrm *pFrm = lcl_FindFirstInvaContent( pFly, nBottom, 0 );
                        if ( pFrm && pFrm->Frm().Bottom() <= nBottom )
                            return pFrm;
                    }
                }
            }
        }
        if ( pCnt->Frm().Top() > nBottom && !pCnt->IsInTab() )
            return 0;
        pCnt = pCnt->GetNextContentFrm();
        if ( !pLay->IsAnLower( pCnt ) )
            break;
    }
    return 0;
}

// #i37877# - consider drawing objects
static const SwAnchoredObject* lcl_FindFirstInvaObj( const SwPageFrm* _pPage,
                                              long _nBottom )
{
    OSL_ENSURE( _pPage->GetSortedObjs(), "FindFirstInvaObj, no Objs" );

    for ( size_t i = 0; i < _pPage->GetSortedObjs()->size(); ++i )
    {
        const SwAnchoredObject* pObj = (*_pPage->GetSortedObjs())[i];
        if ( pObj->ISA(SwFlyFrm) )
        {
            const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pObj);
            if ( pFly->Frm().Top() <= _nBottom )
            {
                if ( pFly->IsInvalid() || pFly->IsCompletePaint() )
                    return pFly;

                const SwFrm* pTmp;
                if ( 0 != (pTmp = lcl_FindFirstInvaContent( pFly, _nBottom, 0 )) &&
                     pTmp->Frm().Top() <= _nBottom )
                    return pFly;
            }
        }
        else if ( pObj->ISA(SwAnchoredDrawObject) )
        {
            if ( !static_cast<const SwAnchoredDrawObject*>(pObj)->IsValidPos() )
            {
                return pObj;
            }
        }
    }
    return 0;
}

/* Returns True if the page lies directly below or right of the visible area.
 *
 * It's possible for things to change in such a way that the processing
 * (of the caller!) has to continue with the predecessor of the passed page.
 * The parameter might therefore get modified!
 * For BrowseMode, you may even activate the ShortCut if the invalid content
 * of the page lies below the visible area.
 */
bool SwLayAction::IsShortCut( SwPageFrm *&prPage )
{
    vcl::RenderContext* pRenderContext = pImp->GetShell()->GetOut();
    bool bRet = false;
    const SwViewShell *pSh = pRoot->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    // If the page is not valid, we quickly format it, otherwise
    // there's gonna be no end of trouble
    if ( !prPage->IsValid() )
    {
        if ( bBrowse )
        {
            // OD 15.10.2002 #103517# - format complete page
            // Thus, loop on all lowers of the page <prPage>, instead of only
            // format its first lower.
            // NOTE: In online layout (bBrowse == true) a page can contain
            //     a header frame and/or a footer frame beside the body frame.
            prPage->Calc(pRenderContext);
            SwFrm* pPageLowerFrm = prPage->Lower();
            while ( pPageLowerFrm )
            {
                pPageLowerFrm->Calc(pRenderContext);
                pPageLowerFrm = pPageLowerFrm->GetNext();
            }
        }
        else
            FormatLayout( pSh ? pSh->GetOut() : 0, prPage );
        if ( IsAgain() )
            return false;
    }

    const SwRect &rVis = pImp->GetShell()->VisArea();
    if ( (prPage->Frm().Top() >= rVis.Bottom()) ||
         (prPage->Frm().Left()>= rVis.Right()) )
    {
        bRet = true;

        // This is going to be a bit nasty: The first ContentFrm of this
        // page in the Body text needs formatting; if it changes the page during
        // that process, I need to start over a page further back, because we
        // have been processing a PageBreak.
        // Even more uncomfortable: The next ContentFrm must be formatted,
        // because it's possible for empty pages to exist temporarily (for example
        // a paragraph across multiple pages gets deleted or reduced in size).

        // This is irrelevant for the browser, if the last Cnt above it
        // isn't visible anymore.

        const SwPageFrm *p2ndPage = prPage;
        const SwContentFrm *pContent;
        const SwLayoutFrm* pBody = p2ndPage->FindBodyCont();
        if( p2ndPage->IsFootnotePage() && pBody )
            pBody = static_cast<const SwLayoutFrm*>(pBody->GetNext());
        pContent = pBody ? pBody->ContainsContent() : 0;
        while ( p2ndPage && !pContent )
        {
            p2ndPage = static_cast<const SwPageFrm*>(p2ndPage->GetNext());
            if( p2ndPage )
            {
                pBody = p2ndPage->FindBodyCont();
                if( p2ndPage->IsFootnotePage() && pBody )
                    pBody = static_cast<const SwLayoutFrm*>(pBody->GetNext());
                pContent = pBody ? pBody->ContainsContent() : 0;
            }
        }
        if ( pContent )
        {
            bool bTstCnt = true;
            if ( bBrowse )
            {
                // Is the Cnt before already invisible?
                const SwFrm *pLst = pContent;
                if ( pLst->IsInTab() )
                    pLst = pContent->FindTabFrm();
                if ( pLst->IsInSct() )
                    pLst = pContent->FindSctFrm();
                pLst = pLst->FindPrev();
                if ( pLst &&
                     (pLst->Frm().Top() >= rVis.Bottom() ||
                      pLst->Frm().Left()>= rVis.Right()) )
                {
                    bTstCnt = false;
                }
            }

            if ( bTstCnt )
            {
                // #i27756# - check after each frame calculation,
                // if the content frame has changed the page. If yes, no other
                // frame calculation is performed
                bool bPageChg = false;

                if ( pContent->IsInSct() )
                {
                    const SwSectionFrm *pSct = ((SwFrm*)pContent)->ImplFindSctFrm();
                    if ( !pSct->IsValid() )
                    {
                        pSct->Calc(pRenderContext);
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return false;
                        // #i27756#
                        bPageChg = pContent->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( !bPageChg && !pContent->IsValid() )
                {
                    pContent->Calc(pRenderContext);
                    pContent->SetCompletePaint();
                    if ( IsAgain() )
                        return false;
                    // #i27756#
                    bPageChg = pContent->FindPageFrm() != p2ndPage &&
                               prPage->GetPrev();
                }

                if ( !bPageChg && pContent->IsInTab() )
                {
                    const SwTabFrm *pTab = ((SwFrm*)pContent)->ImplFindTabFrm();
                    if ( !pTab->IsValid() )
                    {
                        pTab->Calc(pRenderContext);
                        pTab->SetCompletePaint();
                        if ( IsAgain() )
                            return false;
                        // #i27756#
                        bPageChg = pContent->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( !bPageChg && pContent->IsInSct() )
                {
                    const SwSectionFrm *pSct = ((SwFrm*)pContent)->ImplFindSctFrm();
                    if ( !pSct->IsValid() )
                    {
                        pSct->Calc(pRenderContext);
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return false;
                        // #i27756#
                        bPageChg = pContent->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                // #i27756#
                if ( bPageChg )
                {
                    bRet = false;
                    const SwPageFrm* pTmp = pContent->FindPageFrm();
                    if ( pTmp->GetPhyPageNum() < prPage->GetPhyPageNum() &&
                         pTmp->IsInvalid() )
                    {
                        prPage = const_cast<SwPageFrm*>(pTmp);
                    }
                    else
                    {
                        prPage = static_cast<SwPageFrm*>(prPage->GetPrev());
                    }
                }
                // #121980# - no shortcut, if at previous page
                // an anchored object is registered, whose anchor is <pContent>.
                else if ( prPage->GetPrev() )
                {
                    SwSortedObjs* pObjs =
                        static_cast<SwPageFrm*>(prPage->GetPrev())->GetSortedObjs();
                    if ( pObjs )
                    {
                        for ( size_t i = 0; i < pObjs->size(); ++i )
                        {
                            SwAnchoredObject* pObj = (*pObjs)[i];
                            if ( pObj->GetAnchorFrmContainingAnchPos() == pContent )
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
        const SwAnchoredObject* pObj( 0L );
        if ( prPage->GetSortedObjs() &&
             (prPage->IsInvalidFlyLayout() || prPage->IsInvalidFlyContent()) &&
             0 != (pObj = lcl_FindFirstInvaObj( prPage, nBottom )) &&
             pObj->GetObjRect().Top() <= nBottom )
        {
            return false;
        }
        const SwFrm* pFrm( 0L );
        if ( prPage->IsInvalidLayout() &&
             0 != (pFrm = lcl_FindFirstInvaLay( prPage, nBottom )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return false;
        }
        if ( (prPage->IsInvalidContent() || prPage->IsInvalidFlyInCnt()) &&
             0 != (pFrm = lcl_FindFirstInvaContent( prPage, nBottom, 0 )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return false;
        }
        bRet = true;
    }
    return bRet;
}

// OD 15.11.2002 #105155# - introduce support for vertical layout
bool SwLayAction::FormatLayout( OutputDevice *pRenderContext, SwLayoutFrm *pLay, bool bAddRect )
{
    OSL_ENSURE( !IsAgain(), "Attention to the invalid page." );
    if ( IsAgain() )
        return false;

    bool bChanged = false;
    bool bAlreadyPainted = false;
    // OD 11.11.2002 #104414# - remember frame at complete paint
    SwRect aFrmAtCompletePaint;

    if ( !pLay->IsValid() || pLay->IsCompletePaint() )
    {
        if ( pLay->GetPrev() && !pLay->GetPrev()->IsValid() )
            pLay->GetPrev()->SetCompletePaint();

        SwRect aOldFrame( pLay->Frm() );
        SwRect aOldRect( aOldFrame );
        if( pLay->IsPageFrm() )
        {
            aOldRect = static_cast<SwPageFrm*>(pLay)->GetBoundRect(pRenderContext);
        }

        pLay->Calc(pRenderContext);

        if ( aOldFrame != pLay->Frm() )
            bChanged = true;

        bool bNoPaint = false;
        if ( pLay->IsPageBodyFrm() &&
             pLay->Frm().Pos() == aOldRect.Pos() &&
             pLay->Lower() )
        {
            const SwViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
            // Limitations because of headers / footers
            if( pSh && pSh->GetViewOptions()->getBrowseMode() &&
                !( pLay->IsCompletePaint() && pLay->FindPageFrm()->FindFootnoteCont() ) )
                bNoPaint = true;
        }

        if ( !bNoPaint && IsPaint() && bAddRect && (pLay->IsCompletePaint() || bChanged) )
        {
            SwRect aPaint( pLay->Frm() );
            // OD 13.02.2003 #i9719#, #105645# - consider border and shadow for
            // page frames -> enlarge paint rectangle correspondingly.
            if ( pLay->IsPageFrm() )
            {
                SwPageFrm* pPageFrm = static_cast<SwPageFrm*>(pLay);
                aPaint = pPageFrm->GetBoundRect(pRenderContext);
            }

            bool bPageInBrowseMode = pLay->IsPageFrm();
            if( bPageInBrowseMode )
            {
                const SwViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
                if( !pSh || !pSh->GetViewOptions()->getBrowseMode() )
                    bPageInBrowseMode = false;
            }
            if( bPageInBrowseMode )
            {
                // NOTE: no vertical layout in online layout
                // Is the change even visible?
                if ( pLay->IsCompletePaint() )
                {
                    pImp->GetShell()->AddPaintRect( aPaint );
                    bAddRect = false;
                }
                else
                {
                    SwRegionRects aRegion( aOldRect );
                    aRegion -= aPaint;
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                    aRegion.ChangeOrigin( aPaint );
                    aRegion.clear();
                    aRegion.push_back( aPaint );
                    aRegion -= aOldRect;
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                }
            }
            else
            {
                pImp->GetShell()->AddPaintRect( aPaint );
                bAlreadyPainted = true;
                // OD 11.11.2002 #104414# - remember frame at complete paint
                aFrmAtCompletePaint = pLay->Frm();
            }

            // OD 13.02.2003 #i9719#, #105645# - provide paint of spacing
            // between pages (not only for in online mode).
            if ( pLay->IsPageFrm() )
            {
                const SwTwips nHalfDocBorder = GAPBETWEENPAGES;
                const bool bLeftToRightViewLayout = pRoot->IsLeftToRightViewLayout();
                const bool bPrev = bLeftToRightViewLayout ? pLay->GetPrev() : pLay->GetNext();
                const bool bNext = bLeftToRightViewLayout ? pLay->GetNext() : pLay->GetPrev();
                SwPageFrm* pPageFrm = static_cast<SwPageFrm*>(pLay);
                const SwViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
                SwRect aPageRect( pLay->Frm() );

                if(pSh)
                {
                    SwPageFrm::GetBorderAndShadowBoundRect(aPageRect, pSh,
                        pRenderContext,
                        aPageRect, pPageFrm->IsLeftShadowNeeded(), pPageFrm->IsRightShadowNeeded(),
                        pPageFrm->SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
                }

                if ( bPrev )
                {
                    // top
                    SwRect aSpaceToPrevPage( aPageRect );
                    aSpaceToPrevPage.Top( aSpaceToPrevPage.Top() - nHalfDocBorder );
                    aSpaceToPrevPage.Bottom( pLay->Frm().Top() );
                    if(aSpaceToPrevPage.Height() > 0 && aSpaceToPrevPage.Width() > 0)
                        pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );

                    // left
                    aSpaceToPrevPage = aPageRect;
                    aSpaceToPrevPage.Left( aSpaceToPrevPage.Left() - nHalfDocBorder );
                    aSpaceToPrevPage.Right( pLay->Frm().Left() );
                    if(aSpaceToPrevPage.Height() > 0 && aSpaceToPrevPage.Width() > 0)
                        pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );
                }
                if ( bNext )
                {
                    // bottom
                    SwRect aSpaceToNextPage( aPageRect );
                    aSpaceToNextPage.Bottom( aSpaceToNextPage.Bottom() + nHalfDocBorder );
                    aSpaceToNextPage.Top( pLay->Frm().Bottom() );
                    if(aSpaceToNextPage.Height() > 0 && aSpaceToNextPage.Width() > 0)
                        pImp->GetShell()->AddPaintRect( aSpaceToNextPage );

                    // right
                    aSpaceToNextPage = aPageRect;
                    aSpaceToNextPage.Right( aSpaceToNextPage.Right() + nHalfDocBorder );
                    aSpaceToNextPage.Left( pLay->Frm().Right() );
                    if(aSpaceToNextPage.Height() > 0 && aSpaceToNextPage.Width() > 0)
                        pImp->GetShell()->AddPaintRect( aSpaceToNextPage );
                }
            }
        }
        pLay->ResetCompletePaint();
    }

    if ( IsPaint() && bAddRect &&
         !pLay->GetNext() && pLay->IsRetoucheFrm() && pLay->IsRetouche() )
    {
        // OD 15.11.2002 #105155# - vertical layout support
        SWRECTFN( pLay );
        SwRect aRect( pLay->GetUpper()->PaintArea() );
        (aRect.*fnRect->fnSetTop)( (pLay->*fnRect->fnGetPrtBottom)() );
        if ( !pImp->GetShell()->AddPaintRect( aRect ) )
            pLay->ResetRetouche();
    }

    if( bAlreadyPainted )
        bAddRect = false;

    CheckWaitCrsr();

    if ( IsAgain() )
        return false;

    // Now, deal with the lowers that are LayoutFrms

    if ( pLay->IsFootnoteFrm() ) // no LayFrms as Lower
        return bChanged;

    SwFrm *pLow = pLay->Lower();
    bool bTabChanged = false;
    while ( pLow && pLow->GetUpper() == pLay )
    {
        if ( pLow->IsLayoutFrm() )
        {
            if ( pLow->IsTabFrm() )
                bTabChanged |= FormatLayoutTab( static_cast<SwTabFrm*>(pLow), bAddRect );
            // Skip the ones already registered for deletion
            else if( !pLow->IsSctFrm() || static_cast<SwSectionFrm*>(pLow)->GetSection() )
                bChanged |= FormatLayout( pRenderContext, static_cast<SwLayoutFrm*>(pLow), bAddRect );
        }
        else if ( pImp->GetShell()->IsPaintLocked() )
            // Shortcut to minimize the cycles. With Lock, the
            // paint is coming either way (primarily for browse)
            pLow->OptCalc();

        if ( IsAgain() )
            return false;
        pLow = pLow->GetNext();
    }
    // OD 11.11.2002 #104414# - add complete frame area as paint area, if frame
    // area has been already added and after formatting its lowers the frame area
    // is enlarged.
    SwRect aBoundRect(pLay->IsPageFrm() ? static_cast<SwPageFrm*>(pLay)->GetBoundRect(pRenderContext) : pLay->Frm() );

    if ( bAlreadyPainted &&
         ( aBoundRect.Width() > aFrmAtCompletePaint.Width() ||
           aBoundRect.Height() > aFrmAtCompletePaint.Height() )
       )
    {
        pImp->GetShell()->AddPaintRect( aBoundRect );
    }
    return bChanged || bTabChanged;
}

bool SwLayAction::FormatLayoutFly( SwFlyFrm* pFly )
{
    vcl::RenderContext* pRenderContext = pImp->GetShell()->GetOut();
    OSL_ENSURE( !IsAgain(), "Attention to the invalid page." );
    if ( IsAgain() )
        return false;

    bool bChanged = false;
    bool bAddRect = true;

    if ( !pFly->IsValid() || pFly->IsCompletePaint() || pFly->IsInvalid() )
    {
        // The Frame has changed, now it's getting formatted.
        const SwRect aOldRect( pFly->Frm() );
        pFly->Calc(pRenderContext);
        bChanged = aOldRect != pFly->Frm();

        if ( IsPaint() && (pFly->IsCompletePaint() || bChanged) &&
                    pFly->Frm().Top() > 0 && pFly->Frm().Left() > 0 )
            pImp->GetShell()->AddPaintRect( pFly->Frm() );

        if ( bChanged )
            pFly->Invalidate();
        else
            pFly->Validate();

        bAddRect = false;
        pFly->ResetCompletePaint();
    }

    if ( IsAgain() )
        return false;

    // Now, deal with the lowers that are LayoutFrms
    bool bTabChanged = false;
    SwFrm *pLow = pFly->Lower();
    while ( pLow )
    {
        if ( pLow->IsLayoutFrm() )
        {
            if ( pLow->IsTabFrm() )
                bTabChanged |= FormatLayoutTab( static_cast<SwTabFrm*>(pLow), bAddRect );
            else
                bChanged |= FormatLayout( pImp->GetShell()->GetOut(), static_cast<SwLayoutFrm*>(pLow), bAddRect );
        }
        pLow = pLow->GetNext();
    }
    return bChanged || bTabChanged;
}

// OD 31.10.2002 #104100#
// Implement vertical layout support
bool SwLayAction::FormatLayoutTab( SwTabFrm *pTab, bool bAddRect )
{
    OSL_ENSURE( !IsAgain(), "8-) Attention to the invalid page." );
    if ( IsAgain() || !pTab->Lower() )
        return false;

    vcl::RenderContext* pRenderContext = pImp->GetShell()->GetOut();
    IDocumentTimerAccess *pTimerAccess = pRoot->GetFormat()->getIDocumentTimerAccess();
    pTimerAccess->BlockIdling();

    bool bChanged = false;
    bool bPainted = false;

    const SwPageFrm *pOldPage = pTab->FindPageFrm();

    // OD 31.10.2002 #104100# - vertical layout support
    // use macro to declare and init <bool bVert>, <bool bRev> and
    // <SwRectFn fnRect> for table frame <pTab>.
    SWRECTFN( pTab );

    if ( !pTab->IsValid() || pTab->IsCompletePaint() || pTab->IsComplete() )
    {
        if ( pTab->GetPrev() && !pTab->GetPrev()->IsValid() )
        {
            pTab->GetPrev()->SetCompletePaint();
        }

        const SwRect aOldRect( pTab->Frm() );
        pTab->SetLowersFormatted( false );
        pTab->Calc(pRenderContext);
        if ( aOldRect != pTab->Frm() )
        {
            bChanged = true;
        }
        const SwRect aPaintFrm = pTab->PaintArea();

        if ( IsPaint() && bAddRect )
        {
            // OD 01.11.2002 #104100# - add condition <pTab->Frm().HasArea()>
            if ( !pTab->IsCompletePaint() &&
                 pTab->IsComplete() &&
                 ( pTab->Frm().SSize() != pTab->Prt().SSize() ||
                   // OD 31.10.2002 #104100# - vertical layout support
                   (pTab->*fnRect->fnGetLeftMargin)() ) &&
                 pTab->Frm().HasArea()
               )
            {
                // OD 01.11.2002 #104100# - re-implement calculation of margin rectangles.
                SwRect aMarginRect;

                SwTwips nLeftMargin = (pTab->*fnRect->fnGetLeftMargin)();
                if ( nLeftMargin > 0)
                {
                    aMarginRect = pTab->Frm();
                    (aMarginRect.*fnRect->fnSetWidth)( nLeftMargin );
                    pImp->GetShell()->AddPaintRect( aMarginRect );
                }

                if ( (pTab->*fnRect->fnGetRightMargin)() > 0)
                {
                    aMarginRect = pTab->Frm();
                    (aMarginRect.*fnRect->fnSetLeft)( (pTab->*fnRect->fnGetPrtRight)() );
                    pImp->GetShell()->AddPaintRect( aMarginRect );
                }

                SwTwips nTopMargin = (pTab->*fnRect->fnGetTopMargin)();
                if ( nTopMargin > 0)
                {
                    aMarginRect = pTab->Frm();
                    (aMarginRect.*fnRect->fnSetHeight)( nTopMargin );
                    pImp->GetShell()->AddPaintRect( aMarginRect );
                }

                if ( (pTab->*fnRect->fnGetBottomMargin)() > 0)
                {
                    aMarginRect = pTab->Frm();
                    (aMarginRect.*fnRect->fnSetTop)( (pTab->*fnRect->fnGetPrtBottom)() );
                    pImp->GetShell()->AddPaintRect( aMarginRect );
                }
            }
            else if ( pTab->IsCompletePaint() )
            {
                pImp->GetShell()->AddPaintRect( aPaintFrm );
                bAddRect = false;
                bPainted = true;
            }

            if ( pTab->IsRetouche() && !pTab->GetNext() )
            {
                SwRect aRect( pTab->GetUpper()->PaintArea() );
                // OD 04.11.2002 #104100# - vertical layout support
                (aRect.*fnRect->fnSetTop)( (pTab->*fnRect->fnGetPrtBottom)() );
                if ( !pImp->GetShell()->AddPaintRect( aRect ) )
                    pTab->ResetRetouche();
            }
        }
        else
            bAddRect = false;

        if ( pTab->IsCompletePaint() && !pOptTab )
            pOptTab = pTab;
        pTab->ResetCompletePaint();
    }
    if ( IsPaint() && bAddRect && pTab->IsRetouche() && !pTab->GetNext() )
    {
        // OD 04.10.2002 #102779#
        // set correct rectangle for retouche: area between bottom of table frame
        // and bottom of paint area of the upper frame.
        SwRect aRect( pTab->GetUpper()->PaintArea() );
        // OD 04.11.2002 #104100# - vertical layout support
        (aRect.*fnRect->fnSetTop)( (pTab->*fnRect->fnGetPrtBottom)() );
        if ( !pImp->GetShell()->AddPaintRect( aRect ) )
            pTab->ResetRetouche();
    }

    CheckWaitCrsr();

    pTimerAccess->UnblockIdling();

    // Ugly shortcut!
    if ( pTab->IsLowersFormatted() &&
         (bPainted || !pImp->GetShell()->VisArea().IsOver( pTab->Frm())) )
        return false;

    // Now, deal with the lowers
    if ( IsAgain() )
        return false;

    // OD 20.10.2003 #112464# - for savety reasons:
    // check page number before formatting lowers.
    if ( pOldPage->GetPhyPageNum() > (pTab->FindPageFrm()->GetPhyPageNum() + 1) )
        SetNextCycle( true );

    // OD 20.10.2003 #112464# - format lowers, only if table frame is valid
    if ( pTab->IsValid() )
    {
        SwLayoutFrm *pLow = static_cast<SwLayoutFrm*>(pTab->Lower());
        while ( pLow )
        {
            bChanged |= FormatLayout( pImp->GetShell()->GetOut(), pLow, bAddRect );
            if ( IsAgain() )
                return false;
            pLow = static_cast<SwLayoutFrm*>(pLow->GetNext());
        }
    }

    return bChanged;
}

bool SwLayAction::FormatContent( const SwPageFrm *pPage )
{
    const SwContentFrm *pContent = pPage->ContainsContent();
    const SwViewShell *pSh = pRoot->GetCurrShell();
    const bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    while ( pContent && pPage->IsAnLower( pContent ) )
    {
        // If the Content didn't change, we can use a few shortcuts.
        const bool bFull = !pContent->IsValid() || pContent->IsCompletePaint() ||
                           pContent->IsRetouche() || pContent->GetDrawObjs();
        if ( bFull )
        {
            // We do this so we don't have to search later on.
            const bool bNxtCnt = IsCalcLayout() && !pContent->GetFollow();
            const SwContentFrm *pContentNext = bNxtCnt ? pContent->GetNextContentFrm() : 0;
            const SwContentFrm *pContentPrev = pContent->GetPrev() ? pContent->GetPrevContentFrm() : 0;

            const SwLayoutFrm*pOldUpper  = pContent->GetUpper();
            const SwTabFrm *pTab = pContent->FindTabFrm();
            const bool bInValid = !pContent->IsValid() || pContent->IsCompletePaint();
            const bool bOldPaint = IsPaint();
            bPaint = bOldPaint && !(pTab && pTab == pOptTab);
            _FormatContent( pContent, pPage );
            // #i26945# - reset <bPaint> before format objects
            bPaint = bOldPaint;

            // OD 2004-05-10 #i28701# - format floating screen object at content frame.
            // No format, if action flag <bAgain> is set or action is interrupted.
            // OD 2004-08-30 #117736# - allow format on interruption of action, if
            // it's the format for this interrupt
            // #i23129#, #i36347# - pass correct page frame
            // to the object formatter.
            if ( !IsAgain() &&
                 ( !IsInterrupt() || mbFormatContentOnInterrupt ) &&
                 pContent->IsTextFrm() &&
                 !SwObjectFormatter::FormatObjsAtFrm( *(const_cast<SwContentFrm*>(pContent)),
                                                      *(pContent->FindPageFrm()), this ) )
            {
                return false;
            }

            if ( !pContent->GetValidLineNumFlag() && pContent->IsTextFrm() )
            {
                const sal_uLong nAllLines = static_cast<const SwTextFrm*>(pContent)->GetAllLines();
                const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pContent))->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != static_cast<const SwTextFrm*>(pContent)->GetAllLines() )
                    pImp->GetShell()->AddPaintRect( pContent->Frm() );
            }

            if ( IsAgain() )
                return false;

            // Temporarily interrupt processing if layout or Flys become invalid again.
            // However not for the BrowseView: The layout is getting invalid
            // all the time because the page height gets adjusted.
            // The same applies if the user wants to continue working and at least one
            // paragraph has been processed.
            if ( (!pTab || (pTab && !bInValid)) )
            {
                CheckIdleEnd();
                // OD 14.04.2003 #106346# - consider interrupt formatting.
                if ( ( IsInterrupt() && !mbFormatContentOnInterrupt ) ||
                     ( !bBrowse && pPage->IsInvalidLayout() ) ||
                     // OD 07.05.2003 #109435# - consider interrupt formatting
                     ( IS_FLYS && IS_INVAFLY && !mbFormatContentOnInterrupt )
                   )
                    return false;
            }
            if ( pOldUpper != pContent->GetUpper() )
            {
                const sal_uInt16 nCurNum = pContent->FindPageFrm()->GetPhyPageNum();
                if (  nCurNum < pPage->GetPhyPageNum() )
                    nPreInvaPage = nCurNum;

                // If the Frm flowed backwards more than one page, we need to
                // start over again from the beginning, so nothing gets left out.
                if ( !IsCalcLayout() && pPage->GetPhyPageNum() > nCurNum+1 )
                {
                    SetNextCycle( true );
                    // OD 07.05.2003 #109435# - consider interrupt formatting
                    if ( !mbFormatContentOnInterrupt )
                    {
                        return false;
                    }
                }
            }
            // If the Frame moved forwards to the next page, we re-run through
            // the predecessor.
            // This way, we catch predecessors which are now responsible for
            // retouching, but the footers will be touched also.
            bool bSetContent = true;
            if ( pContentPrev )
            {
                if ( !pContentPrev->IsValid() && pPage->IsAnLower( pContentPrev ) )
                    pPage->InvalidateContent();
                if ( pOldUpper != pContent->GetUpper() &&
                     pPage->GetPhyPageNum() < pContent->FindPageFrm()->GetPhyPageNum() )
                {
                    pContent = pContentPrev;
                    bSetContent = false;
                }
            }
            if ( bSetContent )
            {
                if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
                     pContent->Frm().Top() > pImp->GetShell()->VisArea().Bottom())
                {
                    const long nBottom = pImp->GetShell()->VisArea().Bottom();
                    const SwFrm *pTmp = lcl_FindFirstInvaContent( pPage,
                                                            nBottom, pContent );
                    if ( !pTmp )
                    {
                        if ( (!(IS_FLYS && IS_INVAFLY) ||
                              !lcl_FindFirstInvaObj( pPage, nBottom )) &&
                              (!pPage->IsInvalidLayout() ||
                               !lcl_FindFirstInvaLay( pPage, nBottom )))
                            SetBrowseActionStop( true );
                        // OD 14.04.2003 #106346# - consider interrupt formatting.
                        if ( !mbFormatContentOnInterrupt )
                        {
                            return false;
                        }
                    }
                }
                pContent = bNxtCnt ? pContentNext : pContent->GetNextContentFrm();
            }

            RESCHEDULE;
        }
        else
        {
            if ( !pContent->GetValidLineNumFlag() && pContent->IsTextFrm() )
            {
                const sal_uLong nAllLines = static_cast<const SwTextFrm*>(pContent)->GetAllLines();
                const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pContent))->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != static_cast<const SwTextFrm*>(pContent)->GetAllLines() )
                    pImp->GetShell()->AddPaintRect( pContent->Frm() );
            }

            // Do this if the Frm has been formatted before.
            if ( pContent->IsTextFrm() && static_cast<const SwTextFrm*>(pContent)->HasRepaint() &&
                  IsPaint() )
                PaintContent( pContent, pPage, pContent->Frm(), pContent->Frm().Bottom());
            if ( IsIdle() )
            {
                CheckIdleEnd();
                // OD 14.04.2003 #106346# - consider interrupt formatting.
                if ( IsInterrupt() && !mbFormatContentOnInterrupt )
                    return false;
            }
            if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
                 pContent->Frm().Top() > pImp->GetShell()->VisArea().Bottom())
            {
                const long nBottom = pImp->GetShell()->VisArea().Bottom();
                const SwFrm *pTmp = lcl_FindFirstInvaContent( pPage,
                                                    nBottom, pContent );
                if ( !pTmp )
                {
                    if ( (!(IS_FLYS && IS_INVAFLY) ||
                            !lcl_FindFirstInvaObj( pPage, nBottom )) &&
                            (!pPage->IsInvalidLayout() ||
                            !lcl_FindFirstInvaLay( pPage, nBottom )))
                        SetBrowseActionStop( true );
                    // OD 14.04.2003 #106346# - consider interrupt formatting.
                    if ( !mbFormatContentOnInterrupt )
                    {
                        return false;
                    }
                }
            }
            pContent = pContent->GetNextContentFrm();
        }
    }
    CheckWaitCrsr();
    // OD 14.04.2003 #106346# - consider interrupt formatting.
    return !IsInterrupt() || mbFormatContentOnInterrupt;
}

void SwLayAction::_FormatContent( const SwContentFrm *pContent,
                                const SwPageFrm  *pPage )
{
    // We probably only ended up here because the Content holds DrawObjects.
    const bool bDrawObjsOnly = pContent->IsValid() && !pContent->IsCompletePaint() &&
                         !pContent->IsRetouche();
    SWRECTFN( pContent )
    if ( !bDrawObjsOnly && IsPaint() )
    {
        const SwRect aOldRect( pContent->UnionFrm() );
        const long nOldBottom = (pContent->*fnRect->fnGetPrtBottom)();
        pContent->OptCalc();
        if( IsAgain() )
            return;
        if( (*fnRect->fnYDiff)( (pContent->Frm().*fnRect->fnGetBottom)(),
                                (aOldRect.*fnRect->fnGetBottom)() ) < 0 )
        {
            pContent->SetRetouche();
        }
        PaintContent( pContent, pContent->FindPageFrm(), aOldRect, nOldBottom);
    }
    else
    {
        if ( IsPaint() && pContent->IsTextFrm() && static_cast<const SwTextFrm*>(pContent)->HasRepaint() )
            PaintContent( pContent, pPage, pContent->Frm(),
                        (pContent->Frm().*fnRect->fnGetBottom)() );
        pContent->OptCalc();
    }
}

/// Returns true if all Contents of the Fly have been processed completely.
/// Returns false if processing has been interrupted prematurely.
bool SwLayAction::_FormatFlyContent( const SwFlyFrm *pFly )
{
    const SwContentFrm *pContent = pFly->ContainsContent();

    while ( pContent )
    {
        // OD 2004-05-10 #i28701#
        _FormatContent( pContent, pContent->FindPageFrm() );

        // #i28701# - format floating screen objects
        // at content text frame
        // #i23129#, #i36347# - pass correct page frame
        // to the object formatter.
        if ( pContent->IsTextFrm() &&
             !SwObjectFormatter::FormatObjsAtFrm(
                                            *(const_cast<SwContentFrm*>(pContent)),
                                            *(pContent->FindPageFrm()), this ) )
        {
            // restart format with first content
            pContent = pFly->ContainsContent();
            continue;
        }

        if ( !pContent->GetValidLineNumFlag() && pContent->IsTextFrm() )
        {
            const sal_uLong nAllLines = static_cast<const SwTextFrm*>(pContent)->GetAllLines();
            const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pContent))->RecalcAllLines();
            if ( IsPaintExtraData() && IsPaint() &&
                 nAllLines != static_cast<const SwTextFrm*>(pContent)->GetAllLines() )
                pImp->GetShell()->AddPaintRect( pContent->Frm() );
        }

        if ( IsAgain() )
            return false;

        // If there's input, we interrupt processing.
        if ( !pFly->IsFlyInCntFrm() )
        {
            CheckIdleEnd();
            // OD 14.04.2003 #106346# - consider interrupt formatting.
            if ( IsInterrupt() && !mbFormatContentOnInterrupt )
                return false;
        }
        pContent = pContent->GetNextContentFrm();
    }
    CheckWaitCrsr();
    // OD 14.04.2003 #106346# - consider interrupt formatting.
    return !(IsInterrupt() && !mbFormatContentOnInterrupt);
}

bool SwLayIdle::_DoIdleJob( const SwContentFrm *pCnt, IdleJobType eJob )
{
    OSL_ENSURE( pCnt->IsTextFrm(), "NoText neighbour of Text" );
    // robust against misuse by e.g. #i52542#
    if( !pCnt->IsTextFrm() )
        return false;

    const SwTextNode* pTextNode = pCnt->GetNode()->GetTextNode();

    bool bProcess = false;
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

    if( bProcess )
    {
        SwViewShell *pSh = pImp->GetShell();
        if( COMPLETE_STRING == nTextPos )
        {
            --nTextPos;
            if( pSh->ISA(SwCrsrShell) && !static_cast<SwCrsrShell*>(pSh)->IsTableMode() )
            {
                SwPaM *pCrsr = static_cast<SwCrsrShell*>(pSh)->GetCrsr();
                if( !pCrsr->HasMark() && !pCrsr->IsMultiSelection() )
                {
                    pContentNode = pCrsr->GetContentNode();
                    nTextPos =  pCrsr->GetPoint()->nContent.GetIndex();
                }
            }
        }

        switch ( eJob )
        {
            case ONLINE_SPELLING :
            {
                SwRect aRepaint( const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pCnt))->_AutoSpell( pContentNode, nTextPos ) );
                // tdf#92036 PENDING should stop idle spell checking
                bPageValid = bPageValid && (SwTextNode::WrongState::TODO != pTextNode->GetWrongDirty());
                if( !bPageValid )
                    bAllValid = false;
                if ( aRepaint.HasArea() )
                    pImp->GetShell()->InvalidateWindows( aRepaint );
                if ( Application::AnyInput( VCL_INPUT_MOUSEANDKEYBOARD|VclInputFlags::OTHER|VclInputFlags::PAINT ) )
                    return true;
                break;
            }
            case AUTOCOMPLETE_WORDS :
                const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pCnt))->CollectAutoCmplWrds( pContentNode, nTextPos );
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
                if ( Application::AnyInput( VCL_INPUT_ANY ) )
                    return true;
                break;
            }
            case SMART_TAGS :
            {
                try {
                    const SwRect aRepaint( const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pCnt))->SmartTagScan( pContentNode, nTextPos ) );
                    bPageValid = bPageValid && !pTextNode->IsSmartTagDirty();
                    if( !bPageValid )
                        bAllValid = false;
                    if ( aRepaint.HasArea() )
                        pImp->GetShell()->InvalidateWindows( aRepaint );
                } catch( const ::com::sun::star::uno::RuntimeException& e) {
                    // #i122885# handle smarttag problems gracefully and provide diagnostics
                    SAL_WARN( "sw.core", "SMART_TAGS Exception:" << e.Message);
                }
                if ( Application::AnyInput( VCL_INPUT_MOUSEANDKEYBOARD|VclInputFlags::OTHER|VclInputFlags::PAINT ) )
                    return true;
                break;
            }
        }
    }

    // The Flys that are anchored to the paragraph need to be considered too.
    if ( pCnt->GetDrawObjs() )
    {
        const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);
                if ( pFly->IsFlyInCntFrm() )
                {
                    const SwContentFrm *pC = pFly->ContainsContent();
                    while( pC )
                    {
                        if ( pC->IsTextFrm() )
                        {
                            if ( _DoIdleJob( pC, eJob ) )
                                return true;
                        }
                        pC = pC->GetNextContentFrm();
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
            if ( !pViewShell->getIDocumentStatistics()->GetDocStat().bModified )
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

    SwPageFrm *pPage;
    if ( bVisAreaOnly )
        pPage = pImp->GetFirstVisPage(pViewShell->GetOut());
    else
        pPage = static_cast<SwPageFrm*>(pRoot->Lower());

    pContentNode = NULL;
    nTextPos = COMPLETE_STRING;

    while ( pPage )
    {
        bPageValid = true;
        const SwContentFrm *pCnt = pPage->ContainsContent();
        while( pCnt && pPage->IsAnLower( pCnt ) )
        {
            if ( _DoIdleJob( pCnt, eJob ) )
                return true;
            pCnt = pCnt->GetNextContentFrm();
        }
        if ( pPage->GetSortedObjs() )
        {
            for ( size_t i = 0; pPage->GetSortedObjs() &&
                                i < pPage->GetSortedObjs()->size(); ++i )
            {
                const SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pObj);
                    const SwContentFrm *pC = pFly->ContainsContent();
                    while( pC )
                    {
                        if ( pC->IsTextFrm() )
                        {
                            if ( _DoIdleJob( pC, eJob ) )
                                return true;
                        }
                        pC = pC->GetNextContentFrm();
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

        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
        if ( pPage && bVisAreaOnly &&
             !pPage->Frm().IsOver( pImp->GetShell()->VisArea()))
             break;
    }
    return false;
}

#if HAVE_FEATURE_DESKTOP && defined DBG_UTIL
void SwLayIdle::ShowIdle( ColorData eColorData )
{
    if ( !m_bIndicator )
    {
        m_bIndicator = true;
        vcl::Window *pWin = pImp->GetShell()->GetWin();
        if (pWin && !pWin->SupportsDoubleBuffering()) // FIXME make this work with double-buffering
        {
            Rectangle aRect( 0, 0, 5, 5 );
            aRect = pWin->PixelToLogic( aRect );
            // OD 2004-04-23 #116347#
            pWin->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
            pWin->SetFillColor( eColorData );
            pWin->SetLineColor();
            pWin->DrawRect( aRect );
            pWin->Pop();
        }
    }
}
#define SHOW_IDLE( ColorData ) ShowIdle( ColorData )
#else
#define SHOW_IDLE( ColorData )
#endif // DBG_UTIL

SwLayIdle::SwLayIdle( SwRootFrm *pRt, SwViewShellImp *pI ) :
    pRoot( pRt ),
    pImp( pI )
#ifdef DBG_UTIL
    , m_bIndicator( false )
#endif
{
    SAL_INFO("sw.idle", "SwLayIdle() entry");

    pImp->pIdleAct = this;

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
            if ( rSh.ISA(SwCrsrShell) )
            {
                bVis = static_cast<SwCrsrShell*>(&rSh)->GetCharRect().IsOver(rSh.VisArea());
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

                // #137134#
                // Are we supposed to crash if rSh isn't a cursor shell?!
                // bActions |= aTmp != rSh.VisArea() ||
                //             aBools[nBoolIdx] != ((SwCrsrShell*)&rSh)->GetCharRect().IsOver( rSh.VisArea() );

                // aBools[ i ] is true, if the i-th shell is a cursor shell (!!!)
                // and the cursor is visible.
                bActions |= aTmp != rSh.VisArea();
                if ( aTmp == rSh.VisArea() && rSh.ISA(SwCrsrShell) )
                {
                    bActions |= aBools[nBoolIdx] !=
                                 static_cast<SwCrsrShell*>(&rSh)->GetCharRect().IsOver( rSh.VisArea() );
                }
            }

            ++nBoolIdx;
        }

        if ( bActions )
        {
            // Prepare start/end actions via CrsrShell, so the cursor, selection
            // and VisArea can be set correctly.
            nBoolIdx = 0;
            for(SwViewShell& rSh : pImp->GetShell()->GetRingContainer())
            {
                SwCrsrShell* pCrsrShell = nullptr;
                if(rSh.IsA( TYPE(SwCrsrShell) ))
                    pCrsrShell = static_cast<SwCrsrShell*>(&rSh);

                if ( pCrsrShell )
                    pCrsrShell->SttCrsrMove();

                // If there are accrued paints, it's best to simply invalidate
                // the whole window. Otherwise there would arise paint problems whose
                // solution would be disproportionally expensive.
                //fix(18176):
                SwViewShellImp *pViewImp = rSh.Imp();
                bool bUnlock = false;
                if ( pViewImp->GetRegion() )
                {
                    pViewImp->DelRegion();

                    // Cause a repaint with virtual device.
                    rSh.LockPaint();
                    bUnlock = true;
                }

                if ( pCrsrShell )
                    // If the Crsr was visible, we need to make it visible again.
                    // Otherwise, EndCrsrMove with true for IdleEnd
                    pCrsrShell->EndCrsrMove( !aBools[nBoolIdx] );
                if( bUnlock )
                {
                    if( pCrsrShell )
                    {
                        // UnlockPaint overwrite the selection from the
                        // CrsrShell and calls the virtual method paint
                        // to fill the virtual device. This fill dont have
                        // paint the selection! -> Set the focus flag at
                        // CrsrShell and it dont paint the selection.
                        pCrsrShell->ShLooseFcs();
                        pCrsrShell->UnlockPaint( true );
                        pCrsrShell->ShGetFcs( false );
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
        const bool bWordCount = pViewShell->getIDocumentStatistics()->GetDocStat().bModified;
        const bool bSmartTags = !pViewShell->GetDoc()->GetDocShell()->IsHelpDocument() &&
                                !pViewShell->GetDoc()->isXForms() &&
                                SwSmartTagMgr::Get().IsSmartTagsEnabled();

        SwPageFrm *pPg = static_cast<SwPageFrm*>(pRoot->Lower());
        do
        {
            bInValid = pPg->IsInvalidContent()    || pPg->IsInvalidLayout() ||
                       pPg->IsInvalidFlyContent() || pPg->IsInvalidFlyLayout() ||
                       pPg->IsInvalidFlyInCnt() ||
                       (bSpell && pPg->IsInvalidSpelling()) ||
                       (bACmplWrd && pPg->IsInvalidAutoCompleteWords()) ||
                       (bWordCount && pPg->IsInvalidWordCount()) ||
                       (bSmartTags && pPg->IsInvalidSmartTags());

            pPg = static_cast<SwPageFrm*>(pPg->GetNext());

        } while ( pPg && !bInValid );

        if ( !bInValid )
        {
            pRoot->ResetIdleFormat();
            SfxObjectShell* pDocShell = pImp->GetShell()->GetDoc()->GetDocShell();
            pDocShell->Broadcast( SfxEventHint( SW_EVENT_LAYOUT_FINISHED, SwDocShell::GetEventName(STR_SW_EVENT_LAYOUT_FINISHED), pDocShell ) );
        }
    }

    pImp->GetShell()->EnableSmooth( true );

    if( pImp->IsAccessible() )
        pImp->FireAccessibleEvents();

    SAL_INFO("sw.idle", "SwLayIdle() return");

#ifdef DBG_UTIL
    if ( m_bIndicator && pImp->GetShell()->GetWin() )
    {
        // #i75172# Do not invalidate indicator, this may cause a endless loop. Instead, just repaint it
        // This should be replaced by an overlay object in the future, anyways. Since it's only for debug
        // purposes, it is not urgent.
            m_bIndicator = false; SHOW_IDLE( COL_LIGHTGREEN );
    }
#endif
}

SwLayIdle::~SwLayIdle()
{
    pImp->pIdleAct = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
