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


#include <time.h>
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "doc.hxx"
#include "IDocumentDrawModelAccess.hxx"
#include "IDocumentSettingAccess.hxx"
#include "IDocumentLayoutAccess.hxx"
#include "IDocumentStatistics.hxx"
#include "IDocumentTimerAccess.hxx"
#include "viewimp.hxx"
#include "crsrsh.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "dcontact.hxx"
#include "ndtxt.hxx"    // OnlineSpelling
#include "frmfmt.hxx"
#include "swregion.hxx"
#include "viewopt.hxx"  // test OnlineSpelling using internal TabPage
#include "pam.hxx"      // OnlineSpelling needed because of the current cursor position
#include "dbg_lay.hxx"
#include "layouter.hxx" // LoopControlling
#include "docstat.hxx"
#include "swevent.hxx"

#include <sfx2/event.hxx>

#include <ftnidx.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/brshitem.hxx>
#include <SwSmartTagMgr.hxx>

#define _LAYACT_CXX
#include "layact.hxx"
#include <swwait.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <tools/shl.hxx>
#include <sfx2/progress.hxx>
#include <docsh.hxx>

#include "swmodule.hxx"
#include "fmtline.hxx"
#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "mdiexp.hxx"
#include "fmtornt.hxx"
#include "sectfrm.hxx"
#include "lineinfo.hxx"
#include <acmplwrd.hxx>
// #i28701#
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <PostItMgr.hxx>
#include <vector>

//#pragma optimize("ity",on)

/*************************************************************************
|*
|*  SwLayAction static stuff
|*
|*************************************************************************/

#define IS_FLYS (pPage->GetSortedObjs())
#define IS_INVAFLY (pPage->IsInvalidFly())


// Save some typing work to avoid accessing destroyed pages.
#if OSL_DEBUG_LEVEL > 1

static void BreakPoint()
{
    return;
}

#define CHECKPAGE \
            {   if ( IsAgain() ) \
                {   BreakPoint(); \
                    return; \
                } \
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
#define CHECKPAGE \
            {   if ( IsAgain() ) \
                    return; \
            }

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
            if (pProgress) pProgress->Reschedule(); \
            ::RescheduleProgress( pImp->GetShell()->GetDoc()->GetDocShell() ); \
        } \
    }

inline sal_uLong Ticks()
{
    return 1000 * clock() / CLOCKS_PER_SEC;
}

void SwLayAction::CheckWaitCrsr()
{
    RESCHEDULE
    if ( !IsWait() && IsWaitAllowed() && IsPaint() &&
         ((Ticks() - GetStartTicks()) >= CLOCKS_PER_SEC/2) )
    {
        pWait = new SwWait( *pRoot->GetFmt()->GetDoc()->GetDocShell(), sal_True );
    }
}

/*************************************************************************
|*
|*  SwLayAction::CheckIdleEnd()
|*
|*************************************************************************/
// Time over already?
inline void SwLayAction::CheckIdleEnd()
{
    if ( !IsInput() )
        bInput = GetInputType() && Application::AnyInput( GetInputType() );
}

/*************************************************************************
|*
|*  SwLayAction::SetStatBar()
|*
|*************************************************************************/
void SwLayAction::SetStatBar( sal_Bool bNew )
{
    if ( bNew )
    {
        nEndPage = pRoot->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
    }
    else
        nEndPage = USHRT_MAX;
}

/*************************************************************************
|*
|*  SwLayAction::PaintCntnt()
|*
|*  Description        Depending of the type, the Cntnt is output
|*      according to it's changes, or the area to be outputted is
|*      registered with the region, respectively.
|*      PaintCntnt: fills the region
|*
|*************************************************************************/
sal_Bool SwLayAction::PaintWithoutFlys( const SwRect &rRect, const SwCntntFrm *pCnt,
                                    const SwPageFrm *pPage )
{
    SwRegionRects aTmp( rRect );
    const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm *pSelfFly = pCnt->FindFlyFrm();
    sal_uInt16 i;

    for ( i = 0; i < rObjs.Count() && !aTmp.empty(); ++i )
    {
        SdrObject *pO = rObjs[i]->DrawObj();
        if ( !pO->ISA(SwVirtFlyDrawObj) )
            continue;

        // OD 2004-01-15 #110582# - do not consider invisible objects
        const IDocumentDrawModelAccess* pIDDMA = pPage->GetFmt()->getIDocumentDrawModelAccess();
        if ( !pIDDMA->IsVisibleLayerId( pO->GetLayer() ) )
        {
            continue;
        }

        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();

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
                const sal_Bool bLowerOfSelf = pFly->IsLowerOf( pSelfFly );
                if ( !bLowerOfSelf && !pFly->GetFmt()->GetOpaque().GetValue() )
                    // Things from other layers are only interesting to us if
                    // they're not transparent or lie inwards
                    continue;
            }
        }

        /// OD 19.08.2002 #99657#
        ///     Fly frame without a lower have to be subtracted from paint region.
        ///     For checking, if fly frame contains transparent graphic or
        ///     has surrounded contour, assure that fly frame has a lower
        if ( pFly->Lower() &&
             pFly->Lower()->IsNoTxtFrm() &&
             ( ((SwNoTxtFrm*)pFly->Lower())->IsTransparent() ||
               pFly->GetFmt()->GetSurround().IsContour() )
           )
        {
            continue;
        }

        /// OD 19.08.2002 #99657#
        ///     Region of a fly frame with transparent background or a transparent
        ///     shadow have not to be subtracted from paint region
        if ( pFly->IsBackgroundTransparent() ||
             pFly->IsShadowTransparent() )
        {
            continue;
        }

        aTmp -= pFly->Frm();
    }

    sal_Bool bRetPaint = sal_False;
    for ( SwRects::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it )
        bRetPaint |= pImp->GetShell()->AddPaintRect( *it );
    return bRetPaint;
}

inline sal_Bool SwLayAction::_PaintCntnt( const SwCntntFrm *pCntnt,
                                      const SwPageFrm *pPage,
                                      const SwRect &rRect )
{
    if ( rRect.HasArea() )
    {
        if ( pPage->GetSortedObjs() )
            return PaintWithoutFlys( rRect, pCntnt, pPage );
        else
            return pImp->GetShell()->AddPaintRect( rRect );
    }
    return sal_False;
}

void SwLayAction::PaintCntnt( const SwCntntFrm *pCnt,
                              const SwPageFrm *pPage,
                              const SwRect &rOldRect,
                              long nOldBottom )
{
    SWRECTFN( pCnt )

    if ( pCnt->IsCompletePaint() || !pCnt->IsTxtFrm() )
    {
        SwRect aPaint( pCnt->PaintArea() );
        if ( !_PaintCntnt( pCnt, pPage, aPaint ) )
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
            //SwRect aDrawRect( pCnt->UnionFrm( sal_True ) );
            SwRect aDrawRect( pCnt->PaintArea() );
            if( nOldHeight > nNewHeight )
                nOldBottom = (pCnt->*fnRect->fnGetPrtBottom)();
            (aDrawRect.*fnRect->fnSetTop)( nOldBottom );
            _PaintCntnt( pCnt, pPage, aDrawRect );
        }
        // paint content area
        SwRect aPaintRect = static_cast<SwTxtFrm*>(const_cast<SwCntntFrm*>(pCnt))->Paint();
        _PaintCntnt( pCnt, pPage, aPaintRect );
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
        if ( !_PaintCntnt( pCnt, pPage, aRect ) )
            pCnt->ResetRetouche();
    }
}

/*************************************************************************
|*
|*  SwLayAction::SwLayAction()
|*
|*************************************************************************/
SwLayAction::SwLayAction( SwRootFrm *pRt, SwViewImp *pI ) :
    pRoot( pRt ),
    pImp( pI ),
    pOptTab( 0 ),
    pWait( 0 ),
    pProgress(NULL),
    nPreInvaPage( USHRT_MAX ),
    nStartTicks( Ticks() ),
    nInputType( 0 ),
    nEndPage( USHRT_MAX ),
    nCheckPageNum( USHRT_MAX )
{
    bPaintExtraData = ::IsExtraData( pImp->GetShell()->GetDoc() );
    bPaint = bComplete = bWaitAllowed = bCheckPages = sal_True;
    bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
    bUpdateExpFlds = bBrowseActionStop = bActionInProgress = sal_False;
    // OD 14.04.2003 #106346# - init new flag <mbFormatCntntOnInterrupt>.
    mbFormatCntntOnInterrupt = sal_False;

    pImp->pLayAct = this;   // register there
}

SwLayAction::~SwLayAction()
{
    OSL_ENSURE( !pWait, "Wait object not destroyed" );
    pImp->pLayAct = 0;      // unregister
}

/*************************************************************************
|*
|*  SwLayAction::Reset()
|*
|*************************************************************************/
void SwLayAction::Reset()
{
    pOptTab = 0;
    nStartTicks = Ticks();
    nInputType = 0;
    nEndPage = nPreInvaPage = nCheckPageNum = USHRT_MAX;
    bPaint = bComplete = bWaitAllowed = bCheckPages = sal_True;
    bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
    bUpdateExpFlds = bBrowseActionStop = sal_False;
}

/*************************************************************************
|*
|*  SwLayAction::RemoveEmptyBrowserPages()
|*
|*************************************************************************/

sal_Bool SwLayAction::RemoveEmptyBrowserPages()
{
    // switching from the normal to the browser mode, empty pages may be
    // retained for an annoyingly long time, so delete them here
    sal_Bool bRet = sal_False;
    const ViewShell *pSh = pRoot->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
    {
        SwPageFrm *pPage = (SwPageFrm*)pRoot->Lower();
        do
        {
            if ( (pPage->GetSortedObjs() && pPage->GetSortedObjs()->Count()) ||
                 pPage->ContainsCntnt() )
                pPage = (SwPageFrm*)pPage->GetNext();
            else
            {
                bRet = sal_True;
                SwPageFrm *pDel = pPage;
                pPage = (SwPageFrm*)pPage->GetNext();
                pDel->Cut();
                delete pDel;
            }
        } while ( pPage );
    }
    return bRet;
}


/*************************************************************************
|*
|*  SwLayAction::Action()
|*
|*************************************************************************/
void SwLayAction::Action()
{
    bActionInProgress = sal_True;

    //TurboMode? Hands-off during idle-format
    if ( IsPaint() && !IsIdle() && TurboAction() )
    {
        delete pWait, pWait = 0;
        pRoot->ResetTurboFlag();
        bActionInProgress = sal_False;
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
        SetCheckPages( sal_False );

    InternalAction();
    bAgain |= RemoveEmptyBrowserPages();
    while ( IsAgain() )
    {
        bAgain = bNextCycle = sal_False;
        InternalAction();
        bAgain |= RemoveEmptyBrowserPages();
    }
    pRoot->DeleteEmptySct();

    delete pWait, pWait = 0;

    //Turbo-Action permitted again for all cases.
    pRoot->ResetTurboFlag();
    pRoot->ResetTurbo();

    SetCheckPages( sal_True );

    bActionInProgress = sal_False;
}

SwPageFrm* SwLayAction::CheckFirstVisPage( SwPageFrm *pPage )
{
    SwCntntFrm *pCnt = pPage->FindFirstBodyCntnt();
    SwCntntFrm *pChk = pCnt;
    sal_Bool bPageChgd = sal_False;
    while ( pCnt && pCnt->IsFollow() )
        pCnt = static_cast<SwCntntFrm*>(pCnt)->FindMaster();
    if ( pCnt && pChk != pCnt )
    {   bPageChgd = sal_True;
        pPage = pCnt->FindPageFrm();
    }

    if ( !pPage->GetFmt()->GetDoc()->GetFtnIdxs().empty() )
    {
        SwFtnContFrm *pCont = pPage->FindFtnCont();
        if ( pCont )
        {
            pCnt = pCont->ContainsCntnt();
            pChk = pCnt;
            while ( pCnt && pCnt->IsFollow() )
                pCnt = (SwCntntFrm*)pCnt->FindPrev();
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

// OD 2004-05-12 #i28701#
// #i114798# - unlock position on start and end of page
// layout process.
class NotifyLayoutOfPageInProgress
{
    private:
        SwPageFrm& mrPageFrm;

        void _UnlockPositionOfObjs()
        {
            SwSortedObjs* pObjs = mrPageFrm.GetSortedObjs();
            if ( pObjs )
            {
                sal_uInt32 i = 0;
                for ( ; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pObj = (*pObjs)[i];
                    pObj->UnlockPosition();
                }
            }
        }
    public:
        NotifyLayoutOfPageInProgress( SwPageFrm& _rPageFrm )
            : mrPageFrm( _rPageFrm )
        {
            _UnlockPositionOfObjs();
            _rPageFrm.SetLayoutInProgress( true );
        }
        ~NotifyLayoutOfPageInProgress()
        {
            mrPageFrm.SetLayoutInProgress( false );
            _UnlockPositionOfObjs();
        }
};

void SwLayAction::InternalAction()
{
    OSL_ENSURE( pRoot->Lower()->IsPageFrm(), ":-( No page below the root.");

    pRoot->Calc();

    // Figure out the first invalid page or the first one to be formatted,
    // respectively. A complete-action means the first invalid page.
    // However, the first page to be formatted might be the one having the
    // number 1.  If we're doing a fake formatting, the number of the first
    // page is the number of the first visible page.
    SwPageFrm *pPage = IsComplete() ? (SwPageFrm*)pRoot->Lower() :
                pImp->GetFirstVisPage();
    if ( !pPage )
        pPage = (SwPageFrm*)pRoot->Lower();

    // If there's a first-flow-Cntnt in the first visible page that's also a Follow,
    // we switch the page back to the original master of that Cntnt.
    if ( !IsComplete() )
        pPage = CheckFirstVisPage( pPage );
    sal_uInt16 nFirstPageNum = pPage->GetPhyPageNum();

    while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
        pPage = (SwPageFrm*)pPage->GetNext();

    IDocumentLayoutAccess *pLayoutAccess = pRoot->GetFmt()->getIDocumentLayoutAccess();
    sal_Bool bNoLoop = pPage ? SwLayouter::StartLoopControl( pRoot->GetFmt()->GetDoc(), pPage ) : sal_False;
    sal_uInt16 nPercentPageNum = 0;
    while ( (pPage && !IsInterrupt()) || nCheckPageNum != USHRT_MAX )
    {
        if ( !pPage && nCheckPageNum != USHRT_MAX &&
             (!pPage || pPage->GetPhyPageNum() >= nCheckPageNum) )
        {
            if ( !pPage || pPage->GetPhyPageNum() > nCheckPageNum )
            {
                SwPageFrm *pPg = (SwPageFrm*)pRoot->Lower();
                while ( pPg && pPg->GetPhyPageNum() < nCheckPageNum )
                    pPg = (SwPageFrm*)pPg->GetNext();
                if ( pPg )
                    pPage = pPg;
                if ( !pPage )
                    break;
            }
            SwPageFrm *pTmp = pPage->GetPrev() ?
                                        (SwPageFrm*)pPage->GetPrev() : pPage;
            SetCheckPages( sal_True );
            SwFrm::CheckPageDescs( pPage );
            SetCheckPages( sal_False );
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
                    sal_Bool bOld = IsAgain();
                    pRoot->RemoveSuperfluous();
                    bAgain = bOld;
                }
                if ( IsAgain() )
                {
                    if( bNoLoop )
                        pLayoutAccess->GetLayouter()->EndLoopControl();
                    return;
                }
                pPage = (SwPageFrm*)pRoot->Lower();
                while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
                    pPage = (SwPageFrm*)pPage->GetNext();
                while ( pPage && pPage->GetNext() &&
                        pPage->GetPhyPageNum() < nFirstPageNum )
                    pPage = (SwPageFrm*)pPage->GetNext();
                continue;
            }
            break;
        }
        else
        {
            pRoot->DeleteEmptySct();
            XCHECKPAGE;

            // #i28701# - scope for instance of class <NotifyLayoutOfPageInProgress>
            {
                NotifyLayoutOfPageInProgress aLayoutOfPageInProgress( *pPage );

                while ( !IsInterrupt() && !IsNextCycle() &&
                        ((IS_FLYS && IS_INVAFLY) || pPage->IsInvalid()) )
                {
                    // #i28701#
                    SwObjectFormatter::FormatObjsAtFrm( *pPage, *pPage, this );
                    if ( !IS_FLYS )
                    {
                        // If there are no (more) Flys, the flags are superfluous.
                        pPage->ValidateFlyLayout();
                        pPage->ValidateFlyCntnt();
                    }
                    // #i28701# - change condition
                    while ( !IsInterrupt() && !IsNextCycle() &&
                            ( pPage->IsInvalid() ||
                              (IS_FLYS && IS_INVAFLY) ) )
                    {
                        PROTOCOL( pPage, PROT_FILE_INIT, 0, 0)
                        XCHECKPAGE;

                        // #i81146# new loop control
                        sal_uInt16 nLoopControlRuns_1 = 0;
                        const sal_uInt16 nLoopControlMax = 20;

                        while ( !IsNextCycle() && pPage->IsInvalidLayout() )
                        {
                            pPage->ValidateLayout();

                            if ( ++nLoopControlRuns_1 > nLoopControlMax )
                            {
                                OSL_FAIL( "LoopControl_1 in SwLayAction::InternalAction" );
                                break;
                            }

                            FormatLayout( pPage );
                            XCHECKPAGE;
                        }
                        // #i28701# - change condition
                        if ( !IsNextCycle() &&
                             ( pPage->IsInvalidCntnt() ||
                               (IS_FLYS && IS_INVAFLY) ) )
                        {
                            pPage->ValidateFlyInCnt();
                            pPage->ValidateCntnt();
                            // #i28701#
                            pPage->ValidateFlyLayout();
                            pPage->ValidateFlyCntnt();
                            if ( !FormatCntnt( pPage ) )
                            {
                                XCHECKPAGE;
                                pPage->InvalidateCntnt();
                                pPage->InvalidateFlyInCnt();
                                // #i28701#
                                pPage->InvalidateFlyLayout();
                                pPage->InvalidateFlyCntnt();
                                if ( IsBrowseActionStop() )
                                    bInput = sal_True;
                            }
                        }
                        if( bNoLoop )
                            pLayoutAccess->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
                    }
                }
            } // end of scope for instance of class <NotifyLayoutOfPageInProgress>


            // A previous page may be invalid again.
            XCHECKPAGE;
            if ( !IS_FLYS )
            {
                // If there are no (more) Flys, the flags are superfluous.
                pPage->ValidateFlyLayout();
                pPage->ValidateFlyCntnt();
            }
            if ( !IsInterrupt() )
            {
                SetNextCycle( sal_False );

                if ( nPreInvaPage != USHRT_MAX )
                {
                    if( !IsComplete() && nPreInvaPage + 2 < nFirstPageNum )
                    {
                        pImp->SetFirstVisPageInvalid();
                        SwPageFrm *pTmpPage = pImp->GetFirstVisPage();
                        nFirstPageNum = pTmpPage->GetPhyPageNum();
                        if( nPreInvaPage < nFirstPageNum )
                        {
                            nPreInvaPage = nFirstPageNum;
                            pPage = pTmpPage;
                        }
                    }
                    while ( pPage->GetPrev() && pPage->GetPhyPageNum() > nPreInvaPage )
                        pPage = (SwPageFrm*)pPage->GetPrev();
                    nPreInvaPage = USHRT_MAX;
                }

                while ( pPage->GetPrev() &&
                        ( ((SwPageFrm*)pPage->GetPrev())->IsInvalid() ||
                          ( ((SwPageFrm*)pPage->GetPrev())->GetSortedObjs() &&
                            ((SwPageFrm*)pPage->GetPrev())->IsInvalidFly())) &&
                        (((SwPageFrm*)pPage->GetPrev())->GetPhyPageNum() >=
                            nFirstPageNum) )
                {
                    pPage = (SwPageFrm*)pPage->GetPrev();
                }

                // Continue to the next invalid page
                while ( pPage && !pPage->IsInvalid() &&
                        (!IS_FLYS || !IS_INVAFLY) )
                {
                    pPage = (SwPageFrm*)pPage->GetNext();
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
                sal_Bool bOld = IsAgain();
                pRoot->RemoveSuperfluous();
                bAgain = bOld;
            }
            if ( IsAgain() )
            {
                if( bNoLoop )
                    pLayoutAccess->GetLayouter()->EndLoopControl();
                return;
            }
            pPage = (SwPageFrm*)pRoot->Lower();
            while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
                pPage = (SwPageFrm*)pPage->GetNext();
            while ( pPage && pPage->GetNext() &&
                    pPage->GetPhyPageNum() < nFirstPageNum )
                pPage = (SwPageFrm*)pPage->GetNext();
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
            pPg = (SwPageFrm*)pPg->GetNext();
        if( pPg != pPage )
            pPg = pPg ? (SwPageFrm*)pPg->GetPrev() : pPage;

        // set flag for interrupt content formatting
        mbFormatCntntOnInterrupt = IsInput() && !IsStopPrt();
        long nBottom = rVis.Bottom();
        // #i42586# - format current page, if idle action is active
        // This is an optimization for the case that the interrupt is created by
        // the move of a form control object, which is represented by a window.
        while ( pPg && ( pPg->Frm().Top() < nBottom ||
                         ( IsIdle() && pPg == pPage ) ) )
        {
            // #i26945# - follow-up of #i28701#
            NotifyLayoutOfPageInProgress aLayoutOfPageInProgress( *pPg );

            XCHECKPAGE;

            // #i81146# new loop control
            sal_uInt16 nLoopControlRuns_2 = 0;
            const sal_uInt16 nLoopControlMax = 20;

            // special case: interrupt content formatting
            // #i28701# - conditions are incorrect (macros IS_FLYS and IS_INVAFLY only
            //            works for <pPage>) and are too strict.
            // #i50432# - adjust interrupt formatting to normal page formatting - see above.
            while ( ( mbFormatCntntOnInterrupt &&
                      ( pPg->IsInvalid() ||
                        ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) ) ||
                    ( !mbFormatCntntOnInterrupt && pPg->IsInvalidLayout() ) )
            {
                XCHECKPAGE;
                // #i50432# - format also at-page anchored objects
                SwObjectFormatter::FormatObjsAtFrm( *pPg, *pPg, this );
                if ( !pPg->GetSortedObjs() )
                {
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyCntnt();
                }

                // #i81146# new loop control
                sal_uInt16 nLoopControlRuns_3 = 0;

                while ( pPg->IsInvalidLayout() )
                {
                    pPg->ValidateLayout();

                    if ( ++nLoopControlRuns_3 > nLoopControlMax )
                    {
                        OSL_FAIL( "LoopControl_3 in Interrupt formatting in SwLayAction::InternalAction" );
                        break;
                    }

                    FormatLayout( pPg );
                    XCHECKPAGE;
                }

                // #i50432#
                if ( mbFormatCntntOnInterrupt &&
                     ( pPg->IsInvalidCntnt() ||
                       ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) )
                {
                    pPg->ValidateFlyInCnt();
                    pPg->ValidateCntnt();
                    // #i26945#
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyCntnt();

                    if ( ++nLoopControlRuns_2 > nLoopControlMax )
                    {
                        OSL_FAIL( "LoopControl_2 in Interrupt formatting in SwLayAction::InternalAction" );
                        break;
                    }

                    if ( !FormatCntnt( pPg ) )
                    {
                        XCHECKPAGE;
                        pPg->InvalidateCntnt();
                        pPg->InvalidateFlyInCnt();
                        // #i26945#
                        pPg->InvalidateFlyLayout();
                        pPg->InvalidateFlyCntnt();
                    }
                    // #i46807# - we are statisfied, if the content is formatted once complete.
                    else
                    {
                        break;
                    }
                }
            }
            pPg = (SwPageFrm*)pPg->GetNext();
        }
        // reset flag for special interrupt content formatting.
        mbFormatCntntOnInterrupt = sal_False;
    }
    pOptTab = 0;
    if( bNoLoop )
        pLayoutAccess->GetLayouter()->EndLoopControl();
}
/*************************************************************************
|*
|*  SwLayAction::TurboAction(), _TurboAction()
|*
|*************************************************************************/
sal_Bool SwLayAction::_TurboAction( const SwCntntFrm *pCnt )
{

    const SwPageFrm *pPage = 0;
    if ( !pCnt->IsValid() || pCnt->IsCompletePaint() || pCnt->IsRetouche() )
    {
        const SwRect aOldRect( pCnt->UnionFrm( sal_True ) );
        const long   nOldBottom = pCnt->Frm().Top() + pCnt->Prt().Bottom();
        pCnt->Calc();
        if ( pCnt->Frm().Bottom() < aOldRect.Bottom() )
            pCnt->SetRetouche();

        pPage = pCnt->FindPageFrm();
        PaintCntnt( pCnt, pPage, aOldRect, nOldBottom );

        if ( !pCnt->GetValidLineNumFlag() && pCnt->IsTxtFrm() )
        {
            const sal_uLong nAllLines = ((SwTxtFrm*)pCnt)->GetAllLines();
            ((SwTxtFrm*)pCnt)->RecalcAllLines();
            if ( nAllLines != ((SwTxtFrm*)pCnt)->GetAllLines() )
            {
                if ( IsPaintExtraData() )
                    pImp->GetShell()->AddPaintRect( pCnt->Frm() );
                // This is to calculate the remaining LineNums on the page,
                // and we don't stop processing here. To perform this inside RecalcAllLines
                // would be expensive, because we would have to notify the page even
                // in unnecessary cases (normal actions).
                const SwCntntFrm *pNxt = pCnt->GetNextCntntFrm();
                while ( pNxt &&
                        (pNxt->IsInTab() || pNxt->IsInDocBody() != pCnt->IsInDocBody()) )
                    pNxt = pNxt->GetNextCntntFrm();
                if ( pNxt )
                    pNxt->InvalidatePage();
            }
            return sal_False;
        }

        if ( pPage->IsInvalidLayout() || (IS_FLYS && IS_INVAFLY) )
            return sal_False;
    }
    if ( !pPage )
        pPage = pCnt->FindPageFrm();

    // OD 2004-05-10 #i28701# - format floating screen objects at content frame.
    if ( pCnt->IsTxtFrm() &&
         !SwObjectFormatter::FormatObjsAtFrm( *(const_cast<SwCntntFrm*>(pCnt)),
                                              *pPage, this ) )
    {
        return sal_False;
    }

    if ( pPage->IsInvalidCntnt() )
        return sal_False;
    return sal_True;
}

sal_Bool SwLayAction::TurboAction()
{
    sal_Bool bRet = sal_True;

    if ( pRoot->GetTurbo() )
    {
        if ( !_TurboAction( pRoot->GetTurbo() ) )
        {
            CheckIdleEnd();
            bRet = sal_False;
        }
        pRoot->ResetTurbo();
    }
    else
        bRet = sal_False;
    return bRet;
}
/*************************************************************************
|*
|*  SwLayAction::IsShortCut()
|*
|*  Description:       Returns True if the page lies directly below or
|*      right of the visible area.
|*      It's possible for things to change in such a way that the processing
|*      (of the caller!) has to continue with the predecessor of the passed
|*      page. The parameter might therefore get modified!
|*      For BrowseMode, you may even activate the ShortCut if the invalid
|*      content of the page lies below the visible area.
|*
|*************************************************************************/
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
    pFrm = ((SwLayoutFrm*)pFrm)->Lower();
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

static const SwFrm *lcl_FindFirstInvaCntnt( const SwLayoutFrm *pLay, long nBottom,
                                     const SwCntntFrm *pFirst )
{
    const SwCntntFrm *pCnt = pFirst ? pFirst->GetNextCntntFrm() :
                                      pLay->ContainsCntnt();
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
            for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
            {
                const SwAnchoredObject* pObj = rObjs[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pObj);
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        if ( ((SwFlyInCntFrm*)pFly)->IsInvalid() ||
                             pFly->IsCompletePaint() )
                        {
                            if ( pFly->Frm().Top() <= nBottom )
                                return pFly;
                        }
                        const SwFrm *pFrm = lcl_FindFirstInvaCntnt( pFly, nBottom, 0 );
                        if ( pFrm && pFrm->Frm().Bottom() <= nBottom )
                            return pFrm;
                    }
                }
            }
        }
        if ( pCnt->Frm().Top() > nBottom && !pCnt->IsInTab() )
            return 0;
        pCnt = pCnt->GetNextCntntFrm();
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

    for ( sal_uInt16 i = 0; i < _pPage->GetSortedObjs()->Count(); ++i )
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
                if ( 0 != (pTmp = lcl_FindFirstInvaCntnt( pFly, _nBottom, 0 )) &&
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

sal_Bool SwLayAction::IsShortCut( SwPageFrm *&prPage )
{
    sal_Bool bRet = sal_False;
    const ViewShell *pSh = pRoot->GetCurrShell();
    const sal_Bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    // If the page is not valid, we quickly format it, otherwise
    // there's gonna be no end of trouble
    if ( !prPage->IsValid() )
    {
        if ( bBrowse )
        {
            /// OD 15.10.2002 #103517# - format complete page
            /// Thus, loop on all lowers of the page <prPage>, instead of only
            /// format its first lower.
            /// NOTE: In online layout (bBrowse == sal_True) a page can contain
            ///     a header frame and/or a footer frame beside the body frame.
            prPage->Calc();
            SwFrm* pPageLowerFrm = prPage->Lower();
            while ( pPageLowerFrm )
            {
                pPageLowerFrm->Calc();
                pPageLowerFrm = pPageLowerFrm->GetNext();
            }
        }
        else
            FormatLayout( prPage );
        if ( IsAgain() )
            return sal_False;
    }


    const SwRect &rVis = pImp->GetShell()->VisArea();
    if ( (prPage->Frm().Top() >= rVis.Bottom()) ||
         (prPage->Frm().Left()>= rVis.Right()) )
    {
        bRet = sal_True;

        // This is going to be a bit nasty: The first CntntFrm of this
        // page in the Body text needs formatting; if it changes the page during
        // that process, I need to start over a page further back, because we
        // have been processing a PageBreak.
        // Even more uncomfortable: The next CntntFrm must be formatted,
        // because it's possible for empty pages to exist temporarily (for example
        // a paragraph across multiple pages gets deleted or reduced in size).

        // This is irrelevant for the browser, if the last Cnt above it
        // isn't visible anymore.

        const SwPageFrm *p2ndPage = prPage;
        const SwCntntFrm *pCntnt;
        const SwLayoutFrm* pBody = p2ndPage->FindBodyCont();
        if( p2ndPage->IsFtnPage() && pBody )
            pBody = (SwLayoutFrm*)pBody->GetNext();
        pCntnt = pBody ? pBody->ContainsCntnt() : 0;
        while ( p2ndPage && !pCntnt )
        {
            p2ndPage = (SwPageFrm*)p2ndPage->GetNext();
            if( p2ndPage )
            {
                pBody = p2ndPage->FindBodyCont();
                if( p2ndPage->IsFtnPage() && pBody )
                    pBody = (SwLayoutFrm*)pBody->GetNext();
                pCntnt = pBody ? pBody->ContainsCntnt() : 0;
            }
        }
        if ( pCntnt )
        {
            sal_Bool bTstCnt = sal_True;
            if ( bBrowse )
            {
                // Is the Cnt before already invisible?
                const SwFrm *pLst = pCntnt;
                if ( pLst->IsInTab() )
                    pLst = pCntnt->FindTabFrm();
                if ( pLst->IsInSct() )
                    pLst = pCntnt->FindSctFrm();
                pLst = pLst->FindPrev();
                if ( pLst &&
                     (pLst->Frm().Top() >= rVis.Bottom() ||
                      pLst->Frm().Left()>= rVis.Right()) )
                {
                    bTstCnt = sal_False;
                }
            }

            if ( bTstCnt )
            {
                // #i27756# - check after each frame calculation,
                // if the content frame has changed the page. If yes, no other
                // frame calculation is performed
                bool bPageChg = false;

                if ( pCntnt->IsInSct() )
                {
                    const SwSectionFrm *pSct = ((SwFrm*)pCntnt)->ImplFindSctFrm();
                    if ( !pSct->IsValid() )
                    {
                        pSct->Calc();
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return sal_False;
                        // #i27756#
                        bPageChg = pCntnt->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( !bPageChg && !pCntnt->IsValid() )
                {
                    pCntnt->Calc();
                    pCntnt->SetCompletePaint();
                    if ( IsAgain() )
                        return sal_False;
                    // #i27756#
                    bPageChg = pCntnt->FindPageFrm() != p2ndPage &&
                               prPage->GetPrev();
                }

                if ( !bPageChg && pCntnt->IsInTab() )
                {
                    const SwTabFrm *pTab = ((SwFrm*)pCntnt)->ImplFindTabFrm();
                    if ( !pTab->IsValid() )
                    {
                        pTab->Calc();
                        pTab->SetCompletePaint();
                        if ( IsAgain() )
                            return sal_False;
                        // #i27756#
                        bPageChg = pCntnt->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                if ( !bPageChg && pCntnt->IsInSct() )
                {
                    const SwSectionFrm *pSct = ((SwFrm*)pCntnt)->ImplFindSctFrm();
                    if ( !pSct->IsValid() )
                    {
                        pSct->Calc();
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return sal_False;
                        // #i27756#
                        bPageChg = pCntnt->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                // #i27756#
                if ( bPageChg )
                {
                    bRet = sal_False;
                    const SwPageFrm* pTmp = pCntnt->FindPageFrm();
                    if ( pTmp->GetPhyPageNum() < prPage->GetPhyPageNum() &&
                         pTmp->IsInvalid() )
                    {
                        prPage = (SwPageFrm*)pTmp;
                    }
                    else
                    {
                        prPage = (SwPageFrm*)prPage->GetPrev();
                    }
                }
                // #121980# - no shortcut, if at previous page
                // an anchored object is registered, whose anchor is <pCntnt>.
                else if ( prPage->GetPrev() &&
                          static_cast<SwPageFrm*>(prPage->GetPrev())->GetSortedObjs() )
                {
                    SwSortedObjs* pObjs =
                        static_cast<SwPageFrm*>(prPage->GetPrev())->GetSortedObjs();
                    if ( pObjs )
                    {
                        sal_uInt32 i = 0;
                        for ( ; i < pObjs->Count(); ++i )
                        {
                            SwAnchoredObject* pObj = (*pObjs)[i];
                            if ( pObj->GetAnchorFrmContainingAnchPos() == pCntnt )
                            {
                                bRet = sal_False;
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
             (prPage->IsInvalidFlyLayout() || prPage->IsInvalidFlyCntnt()) &&
             0 != (pObj = lcl_FindFirstInvaObj( prPage, nBottom )) &&
             pObj->GetObjRect().Top() <= nBottom )
        {
            return sal_False;
        }
        const SwFrm* pFrm( 0L );
        if ( prPage->IsInvalidLayout() &&
             0 != (pFrm = lcl_FindFirstInvaLay( prPage, nBottom )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return sal_False;
        }
        if ( (prPage->IsInvalidCntnt() || prPage->IsInvalidFlyInCnt()) &&
             0 != (pFrm = lcl_FindFirstInvaCntnt( prPage, nBottom, 0 )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return sal_False;
        }
        bRet = sal_True;
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwLayAction::FormatLayout(), FormatLayoutFly, FormatLayoutTab()
|*
|*************************************************************************/
// OD 15.11.2002 #105155# - introduce support for vertical layout
sal_Bool SwLayAction::FormatLayout( SwLayoutFrm *pLay, sal_Bool bAddRect )
{
    OSL_ENSURE( !IsAgain(), "Attention to the invalid page." );
    if ( IsAgain() )
        return sal_False;

    sal_Bool bChanged = sal_False;
    sal_Bool bAlreadyPainted = sal_False;
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
            aOldRect = static_cast<SwPageFrm*>(pLay)->GetBoundRect();
        }

        pLay->Calc();
        if ( aOldFrame != pLay->Frm() )
            bChanged = sal_True;

        sal_Bool bNoPaint = sal_False;
        if ( pLay->IsPageBodyFrm() &&
             pLay->Frm().Pos() == aOldRect.Pos() &&
             pLay->Lower() )
        {
            const ViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
            // Limitations because of headers / footers
            if( pSh && pSh->GetViewOptions()->getBrowseMode() &&
                !( pLay->IsCompletePaint() && pLay->FindPageFrm()->FindFtnCont() ) )
                bNoPaint = sal_True;
        }

        if ( !bNoPaint && IsPaint() && bAddRect && (pLay->IsCompletePaint() || bChanged) )
        {
            SwRect aPaint( pLay->Frm() );
            // OD 13.02.2003 #i9719#, #105645# - consider border and shadow for
            // page frames -> enlarge paint rectangle correspondingly.
            if ( pLay->IsPageFrm() )
            {
                SwPageFrm* pPageFrm = static_cast<SwPageFrm*>(pLay);
                aPaint = pPageFrm->GetBoundRect();
            }

            bool bPageInBrowseMode = pLay->IsPageFrm();
            if( bPageInBrowseMode )
            {
                const ViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
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
                    bAddRect = sal_False;
                }
                else
                {
                    sal_uInt16 i;

                    SwRegionRects aRegion( aOldRect );
                    aRegion -= aPaint;
                    for ( i = 0; i < aRegion.size(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                    aRegion.ChangeOrigin( aPaint );
                    aRegion.clear();
                    aRegion.push_back( aPaint );
                    aRegion -= aOldRect;
                    for ( i = 0; i < aRegion.size(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                }

            }
            else
            {
                pImp->GetShell()->AddPaintRect( aPaint );
                bAlreadyPainted = sal_True;
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
                const ViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
                SwRect aPageRect( pLay->Frm() );

                if(pSh)
                {
                    SwPageFrm::GetBorderAndShadowBoundRect(aPageRect, pSh,
                        aPageRect, pPageFrm->IsLeftShadowNeeded(), pPageFrm->IsRightShadowNeeded(),
                        pPageFrm->SidebarPosition() == sw::sidebarwindows::SIDEBAR_RIGHT);
                }

                if ( bPrev )
                {
                    // top
                    SwRect aSpaceToPrevPage( aPageRect );
                    aSpaceToPrevPage.Top( aSpaceToPrevPage.Top() - nHalfDocBorder );
                    aSpaceToPrevPage.Bottom( pLay->Frm().Top() );
                    if(aSpaceToPrevPage.Height() > 0 && aSpaceToPrevPage.Width() > 0)
                        pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );

                    pSh->GetOut()->DrawRect( aSpaceToPrevPage.SVRect() );

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
        bAddRect = sal_False;

    CheckWaitCrsr();

    if ( IsAgain() )
        return sal_False;

    // Now, deal with the lowers that are LayoutFrms

    if ( pLay->IsFtnFrm() ) // no LayFrms as Lower
        return bChanged;

    SwFrm *pLow = pLay->Lower();
    sal_Bool bTabChanged = sal_False;
    while ( pLow && pLow->GetUpper() == pLay )
    {
        if ( pLow->IsLayoutFrm() )
        {
            if ( pLow->IsTabFrm() )
                bTabChanged |= FormatLayoutTab( (SwTabFrm*)pLow, bAddRect );
            // Skip the ones already registered for deletion
            else if( !pLow->IsSctFrm() || ((SwSectionFrm*)pLow)->GetSection() )
                bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
        }
        else if ( pImp->GetShell()->IsPaintLocked() )
            // Shortcut to minimize the cycles. With Lock, the
            // paint is coming either way (primarily for browse)
            pLow->OptCalc();

        if ( IsAgain() )
            return sal_False;
        pLow = pLow->GetNext();
    }
    // OD 11.11.2002 #104414# - add complete frame area as paint area, if frame
    // area has been already added and after formating its lowers the frame area
    // is enlarged.
    SwRect aBoundRect(pLay->IsPageFrm() ? static_cast<SwPageFrm*>(pLay)->GetBoundRect() : pLay->Frm() );

    if ( bAlreadyPainted &&
         ( aBoundRect.Width() > aFrmAtCompletePaint.Width() ||
           aBoundRect.Height() > aFrmAtCompletePaint.Height() )
       )
    {
        pImp->GetShell()->AddPaintRect( aBoundRect );
    }
    return bChanged || bTabChanged;
}

sal_Bool SwLayAction::FormatLayoutFly( SwFlyFrm* pFly )
{
    OSL_ENSURE( !IsAgain(), "Attention to the invalid page." );
    if ( IsAgain() )
        return sal_False;

    sal_Bool bChanged = false;
    sal_Bool bAddRect = true;

    if ( !pFly->IsValid() || pFly->IsCompletePaint() || pFly->IsInvalid() )
    {
        // The Frame has changed, now it's getting formatted.
        const SwRect aOldRect( pFly->Frm() );
        pFly->Calc();
        bChanged = aOldRect != pFly->Frm();

        if ( IsPaint() && (pFly->IsCompletePaint() || bChanged) &&
                    pFly->Frm().Top() > 0 && pFly->Frm().Left() > 0 )
            pImp->GetShell()->AddPaintRect( pFly->Frm() );

        if ( bChanged )
            pFly->Invalidate();
        else
            pFly->Validate();
/*
        //mba: it's unclear why we should invalidate always, so I remove it
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if ( IsPaint() && bAddRect && pFly->Frm().Top() > 0 && pFly->Frm().Left() > 0 )
            pImp->GetShell()->AddPaintRect( pFly->Frm() );

        pFly->Invalidate();
*/
        bAddRect = false;
        pFly->ResetCompletePaint();
    }

    if ( IsAgain() )
        return sal_False;

    // Now, deal with the lowers that are LayoutFrms
    sal_Bool bTabChanged = false;
    SwFrm *pLow = pFly->Lower();
    while ( pLow )
    {
        if ( pLow->IsLayoutFrm() )
        {
            if ( pLow->IsTabFrm() )
                bTabChanged |= FormatLayoutTab( (SwTabFrm*)pLow, bAddRect );
            else
                bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
        }
        pLow = pLow->GetNext();
    }
    return bChanged || bTabChanged;
}

// OD 31.10.2002 #104100#
// Implement vertical layout support
sal_Bool SwLayAction::FormatLayoutTab( SwTabFrm *pTab, sal_Bool bAddRect )
{
    OSL_ENSURE( !IsAgain(), "8-) Attention to the invalid page." );
    if ( IsAgain() || !pTab->Lower() )
        return sal_False;

    IDocumentTimerAccess *pTimerAccess = pRoot->GetFmt()->getIDocumentTimerAccess();
    pTimerAccess->BlockIdling();

    sal_Bool bChanged = sal_False;
    sal_Bool bPainted = sal_False;

    const SwPageFrm *pOldPage = pTab->FindPageFrm();

    // OD 31.10.2002 #104100# - vertical layout support
    // use macro to declare and init <sal_Bool bVert>, <sal_Bool bRev> and
    // <SwRectFn fnRect> for table frame <pTab>.
    SWRECTFN( pTab );

    if ( !pTab->IsValid() || pTab->IsCompletePaint() || pTab->IsComplete() )
    {
        if ( pTab->GetPrev() && !pTab->GetPrev()->IsValid() )
        {
            pTab->GetPrev()->SetCompletePaint();
        }

        const SwRect aOldRect( pTab->Frm() );
        pTab->SetLowersFormatted( sal_False );
        pTab->Calc();
        if ( aOldRect != pTab->Frm() )
        {
            bChanged = sal_True;
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
                bAddRect = sal_False;
                bPainted = sal_True;
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
            bAddRect = sal_False;

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
        return sal_False;

    // Now, deal with the lowers
    if ( IsAgain() )
        return sal_False;

    // OD 20.10.2003 #112464# - for savety reasons:
    // check page number before formatting lowers.
    if ( pOldPage->GetPhyPageNum() > (pTab->FindPageFrm()->GetPhyPageNum() + 1) )
        SetNextCycle( sal_True );

    // OD 20.10.2003 #112464# - format lowers, only if table frame is valid
    if ( pTab->IsValid() )
    {
        SwLayoutFrm *pLow = (SwLayoutFrm*)pTab->Lower();
        while ( pLow )
        {
            bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
            if ( IsAgain() )
                return sal_False;
            pLow = (SwLayoutFrm*)pLow->GetNext();
        }
    }

    return bChanged;
}

/*************************************************************************
|*
|*  SwLayAction::FormatCntnt()
|*
|*************************************************************************/
sal_Bool SwLayAction::FormatCntnt( const SwPageFrm *pPage )
{
    const SwCntntFrm *pCntnt = pPage->ContainsCntnt();
    const ViewShell *pSh = pRoot->GetCurrShell();
    const sal_Bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    while ( pCntnt && pPage->IsAnLower( pCntnt ) )
    {
        // If the Cntnt didn't change, we can use a few shortcuts.
        const sal_Bool bFull = !pCntnt->IsValid() || pCntnt->IsCompletePaint() ||
                           pCntnt->IsRetouche() || pCntnt->GetDrawObjs();
        if ( bFull )
        {
            // We do this so we don't have to search later on.
            const sal_Bool bNxtCnt = IsCalcLayout() && !pCntnt->GetFollow();
            const SwCntntFrm *pCntntNext = bNxtCnt ? pCntnt->GetNextCntntFrm() : 0;
            const SwCntntFrm *pCntntPrev = pCntnt->GetPrev() ? pCntnt->GetPrevCntntFrm() : 0;

            const SwLayoutFrm*pOldUpper  = pCntnt->GetUpper();
            const SwTabFrm *pTab = pCntnt->FindTabFrm();
            const sal_Bool bInValid = !pCntnt->IsValid() || pCntnt->IsCompletePaint();
            const sal_Bool bOldPaint = IsPaint();
            bPaint = bOldPaint && !(pTab && pTab == pOptTab);
            _FormatCntnt( pCntnt, pPage );
            // #i26945# - reset <bPaint> before format objects
            bPaint = bOldPaint;

            // OD 2004-05-10 #i28701# - format floating screen object at content frame.
            // No format, if action flag <bAgain> is set or action is interrupted.
            // OD 2004-08-30 #117736# - allow format on interruption of action, if
            // it's the format for this interrupt
            // #i23129#, #i36347# - pass correct page frame
            // to the object formatter.
            if ( !IsAgain() &&
                 ( !IsInterrupt() || mbFormatCntntOnInterrupt ) &&
                 pCntnt->IsTxtFrm() &&
                 !SwObjectFormatter::FormatObjsAtFrm( *(const_cast<SwCntntFrm*>(pCntnt)),
                                                      *(pCntnt->FindPageFrm()), this ) )
            {
                return sal_False;
            }

            if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
            {
                const sal_uLong nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
                ((SwTxtFrm*)pCntnt)->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
                    pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
            }

            if ( IsAgain() )
                return sal_False;

            // Temporarily interrupt processing if layout or Flys become invalid again.
            // However not for the BrowseView: The layout is getting invalid
            // all the time because the page height gets adjusted.
            // The same applies if the user wants to continue working and at least one
            // paragraph has been processed.
            if ( (!pTab || (pTab && !bInValid)) )
            {
                CheckIdleEnd();
                // OD 14.04.2003 #106346# - consider interrupt formatting.
                if ( ( IsInterrupt() && !mbFormatCntntOnInterrupt ) ||
                     ( !bBrowse && pPage->IsInvalidLayout() ) ||
                     // OD 07.05.2003 #109435# - consider interrupt formatting
                     ( IS_FLYS && IS_INVAFLY && !mbFormatCntntOnInterrupt )
                   )
                    return sal_False;
            }
            if ( pOldUpper != pCntnt->GetUpper() )
            {
                const sal_uInt16 nCurNum = pCntnt->FindPageFrm()->GetPhyPageNum();
                if (  nCurNum < pPage->GetPhyPageNum() )
                    nPreInvaPage = nCurNum;

                // If the Frm flowed backwards more than one page, we need to
                // start over again from the beginning, so nothing gets left out.
                if ( !IsCalcLayout() && pPage->GetPhyPageNum() > nCurNum+1 )
                {
                    SetNextCycle( sal_True );
                    // OD 07.05.2003 #109435# - consider interrupt formatting
                    if ( !mbFormatCntntOnInterrupt )
                    {
                        return sal_False;
                    }
                }
            }
            // If the Frame moved forwards to the next page, we re-run through
            // the predecessor.
            // This way, we catch predecessors which are now responsible for
            // retouching, but the footers will be touched also.
            sal_Bool bSetCntnt = sal_True;
            if ( pCntntPrev )
            {
                if ( !pCntntPrev->IsValid() && pPage->IsAnLower( pCntntPrev ) )
                    pPage->InvalidateCntnt();
                if ( pOldUpper != pCntnt->GetUpper() &&
                     pPage->GetPhyPageNum() < pCntnt->FindPageFrm()->GetPhyPageNum() )
                {
                    pCntnt = pCntntPrev;
                    bSetCntnt = sal_False;
                }
            }
            if ( bSetCntnt )
            {
                if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
                     pCntnt->Frm().Top() > pImp->GetShell()->VisArea().Bottom())
                {
                    const long nBottom = pImp->GetShell()->VisArea().Bottom();
                    const SwFrm *pTmp = lcl_FindFirstInvaCntnt( pPage,
                                                            nBottom, pCntnt );
                    if ( !pTmp )
                    {
                        if ( (!(IS_FLYS && IS_INVAFLY) ||
                              !lcl_FindFirstInvaObj( pPage, nBottom )) &&
                              (!pPage->IsInvalidLayout() ||
                               !lcl_FindFirstInvaLay( pPage, nBottom )))
                            SetBrowseActionStop( sal_True );
                        // OD 14.04.2003 #106346# - consider interrupt formatting.
                        if ( !mbFormatCntntOnInterrupt )
                        {
                            return sal_False;
                        }
                    }
                }
                pCntnt = bNxtCnt ? pCntntNext : pCntnt->GetNextCntntFrm();
            }

            RESCHEDULE;
        }
        else
        {
            if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
            {
                const sal_uLong nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
                ((SwTxtFrm*)pCntnt)->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
                    pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
            }

            // Do this if the Frm has been formatted before.
            if ( pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() &&
                  IsPaint() )
                PaintCntnt( pCntnt, pPage, pCntnt->Frm(), pCntnt->Frm().Bottom());
            if ( IsIdle() )
            {
                CheckIdleEnd();
                // OD 14.04.2003 #106346# - consider interrupt formatting.
                if ( IsInterrupt() && !mbFormatCntntOnInterrupt )
                    return sal_False;
            }
            if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
                 pCntnt->Frm().Top() > pImp->GetShell()->VisArea().Bottom())
            {
                const long nBottom = pImp->GetShell()->VisArea().Bottom();
                const SwFrm *pTmp = lcl_FindFirstInvaCntnt( pPage,
                                                    nBottom, pCntnt );
                if ( !pTmp )
                {
                    if ( (!(IS_FLYS && IS_INVAFLY) ||
                            !lcl_FindFirstInvaObj( pPage, nBottom )) &&
                            (!pPage->IsInvalidLayout() ||
                            !lcl_FindFirstInvaLay( pPage, nBottom )))
                        SetBrowseActionStop( sal_True );
                    // OD 14.04.2003 #106346# - consider interrupt formatting.
                    if ( !mbFormatCntntOnInterrupt )
                    {
                        return sal_False;
                    }
                }
            }
            pCntnt = pCntnt->GetNextCntntFrm();
        }
    }
    CheckWaitCrsr();
    // OD 14.04.2003 #106346# - consider interrupt formatting.
    return !IsInterrupt() || mbFormatCntntOnInterrupt;
}
/*************************************************************************
|*
|*  SwLayAction::_FormatCntnt()
|*
|*  Description         Returns sal_True if the paragraph has been processed,
|*                      sal_False if there wasn't anything to be processed.
|*
|*************************************************************************/
void SwLayAction::_FormatCntnt( const SwCntntFrm *pCntnt,
                                const SwPageFrm  *pPage )
{
    // We probably only ended up here because the Cntnt holds DrawObjects.
    const bool bDrawObjsOnly = pCntnt->IsValid() && !pCntnt->IsCompletePaint() &&
                         !pCntnt->IsRetouche();
    SWRECTFN( pCntnt )
    if ( !bDrawObjsOnly && IsPaint() )
    {
        const SwRect aOldRect( pCntnt->UnionFrm() );
        const long nOldBottom = (pCntnt->*fnRect->fnGetPrtBottom)();
        pCntnt->OptCalc();
        if( IsAgain() )
            return;
        if( (*fnRect->fnYDiff)( (pCntnt->Frm().*fnRect->fnGetBottom)(),
                                (aOldRect.*fnRect->fnGetBottom)() ) < 0 )
        {
            pCntnt->SetRetouche();
        }
        PaintCntnt( pCntnt, pCntnt->FindPageFrm(), aOldRect, nOldBottom);
    }
    else
    {
        if ( IsPaint() && pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() )
            PaintCntnt( pCntnt, pPage, pCntnt->Frm(),
                        (pCntnt->Frm().*fnRect->fnGetBottom)() );
        pCntnt->OptCalc();
    }
}

/*************************************************************************
|*
|*  SwLayAction::_FormatFlyCntnt()
|*
|*  Description:
|*      - Returns sal_True if all Cntnts of the Fly have been processed completely.
|*        Returns sal_False if processing has been interrupted prematurely.
|*
|*************************************************************************/
sal_Bool SwLayAction::_FormatFlyCntnt( const SwFlyFrm *pFly )
{
    const SwCntntFrm *pCntnt = pFly->ContainsCntnt();

    while ( pCntnt )
    {
        // OD 2004-05-10 #i28701#
        _FormatCntnt( pCntnt, pCntnt->FindPageFrm() );

        // #i28701# - format floating screen objects
        // at content text frame
        // #i23129#, #i36347# - pass correct page frame
        // to the object formatter.
        if ( pCntnt->IsTxtFrm() &&
             !SwObjectFormatter::FormatObjsAtFrm(
                                            *(const_cast<SwCntntFrm*>(pCntnt)),
                                            *(pCntnt->FindPageFrm()), this ) )
        {
            // restart format with first content
            pCntnt = pFly->ContainsCntnt();
            continue;
        }

        if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
        {
            const sal_uLong nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
            ((SwTxtFrm*)pCntnt)->RecalcAllLines();
            if ( IsPaintExtraData() && IsPaint() &&
                 nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
                pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
        }

        if ( IsAgain() )
            return sal_False;

        // If there's input, we interrupt processing.
        if ( !pFly->IsFlyInCntFrm() )
        {
            CheckIdleEnd();
            // OD 14.04.2003 #106346# - consider interrupt formatting.
            if ( IsInterrupt() && !mbFormatCntntOnInterrupt )
                return sal_False;
        }
        pCntnt = pCntnt->GetNextCntntFrm();
    }
    CheckWaitCrsr();
    // OD 14.04.2003 #106346# - consider interrupt formatting.
    return !(IsInterrupt() && !mbFormatCntntOnInterrupt);
}

sal_Bool SwLayAction::IsStopPrt() const
{
    sal_Bool bResult = sal_False;

    if (pImp != NULL && pProgress != NULL)
        bResult = pImp->IsStopPrt();

    return bResult;
}

/*************************************************************************
|*
|*  SwLayAction::FormatSpelling(), _FormatSpelling()
|*
|*************************************************************************/
sal_Bool SwLayIdle::_DoIdleJob( const SwCntntFrm *pCnt, IdleJobType eJob )
{
    OSL_ENSURE( pCnt->IsTxtFrm(), "NoTxt neighbour of Txt" );
    // robust against misuse by e.g. #i52542#
    if( !pCnt->IsTxtFrm() )
        return sal_False;

    const SwTxtNode* pTxtNode = pCnt->GetNode()->GetTxtNode();

    bool bProcess = false;
    switch ( eJob )
    {
        case ONLINE_SPELLING :
            bProcess = pTxtNode->IsWrongDirty(); break;
        case AUTOCOMPLETE_WORDS :
            bProcess = pTxtNode->IsAutoCompleteWordDirty(); break;
        case WORD_COUNT :
            bProcess = pTxtNode->IsWordCountDirty(); break;
        case SMART_TAGS :   // SMARTTAGS
            bProcess = pTxtNode->IsSmartTagDirty(); break;
    }

    if( bProcess )
    {
        ViewShell *pSh = pImp->GetShell();
        if( STRING_LEN == nTxtPos )
        {
            --nTxtPos;
            if( pSh->ISA(SwCrsrShell) && !((SwCrsrShell*)pSh)->IsTableMode() )
            {
                SwPaM *pCrsr = ((SwCrsrShell*)pSh)->GetCrsr();
                if( !pCrsr->HasMark() && pCrsr == pCrsr->GetNext() )
                {
                    pCntntNode = pCrsr->GetCntntNode();
                    nTxtPos =  pCrsr->GetPoint()->nContent.GetIndex();
                }
            }
        }

        switch ( eJob )
        {
            case ONLINE_SPELLING :
            {
                SwRect aRepaint( ((SwTxtFrm*)pCnt)->_AutoSpell( pCntntNode,  *pSh->GetViewOptions(), nTxtPos ) );
                bPageValid = bPageValid && !pTxtNode->IsWrongDirty();
                if( !bPageValid )
                    bAllValid = sal_False;
                if ( aRepaint.HasArea() )
                    pImp->GetShell()->InvalidateWindows( aRepaint );
                if ( Application::AnyInput( VCL_INPUT_MOUSEANDKEYBOARD|VCL_INPUT_OTHER|VCL_INPUT_PAINT ) )
                    return sal_True;
                break;
            }
            case AUTOCOMPLETE_WORDS :
                ((SwTxtFrm*)pCnt)->CollectAutoCmplWrds( pCntntNode, nTxtPos );
                if ( Application::AnyInput( VCL_INPUT_ANY ) )
                    return sal_True;
                break;
            case WORD_COUNT :
            {
                const xub_StrLen nEnd = pTxtNode->GetTxt().Len();
                SwDocStat aStat;
                pTxtNode->CountWords( aStat, 0, nEnd );
                if ( Application::AnyInput( VCL_INPUT_ANY ) )
                    return sal_True;
                break;
            }
            case SMART_TAGS : // SMARTTAGS
            {
                const SwRect aRepaint( ((SwTxtFrm*)pCnt)->SmartTagScan( pCntntNode, nTxtPos ) );
                bPageValid = bPageValid && !pTxtNode->IsSmartTagDirty();
                if( !bPageValid )
                    bAllValid = sal_False;
                if ( aRepaint.HasArea() )
                    pImp->GetShell()->InvalidateWindows( aRepaint );
                if ( Application::AnyInput( VCL_INPUT_MOUSEANDKEYBOARD|VCL_INPUT_OTHER|VCL_INPUT_PAINT ) )
                    return sal_True;
                break;
            }
        }
    }

    // The Flys that are anchored to the paragraph need to be considered too.
    if ( pCnt->GetDrawObjs() )
    {
        const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
        for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);
                if ( pFly->IsFlyInCntFrm() )
                {
                    const SwCntntFrm *pC = pFly->ContainsCntnt();
                    while( pC )
                    {
                        if ( pC->IsTxtFrm() )
                        {
                            if ( _DoIdleJob( pC, eJob ) )
                                return sal_True;
                        }
                        pC = pC->GetNextCntntFrm();
                    }
                }
            }
        }
    }
    return sal_False;
}

sal_Bool SwLayIdle::DoIdleJob( IdleJobType eJob, sal_Bool bVisAreaOnly )
{
    // Spellcheck all contents of the pages. Either only the
    // visible ones or all of them.
    const ViewShell* pViewShell = pImp->GetShell();
    const SwViewOption* pViewOptions = pViewShell->GetViewOptions();
    const SwDoc* pDoc = pViewShell->GetDoc();

    switch ( eJob )
    {
        case ONLINE_SPELLING :
            if( !pViewOptions->IsOnlineSpell() )
                return sal_False;
            break;
        case AUTOCOMPLETE_WORDS :
            if( !pViewOptions->IsAutoCompleteWords() ||
                 pDoc->GetAutoCompleteWords().IsLockWordLstLocked())
                return sal_False;
            break;
        case WORD_COUNT :
            if ( !pViewShell->getIDocumentStatistics()->GetDocStat().bModified )
                return sal_False;
            break;
        case SMART_TAGS :
            if ( pDoc->GetDocShell()->IsHelpDocument() ||
                 pDoc->isXForms() ||
                !SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                return sal_False;
            break;
        default: OSL_FAIL( "Unknown idle job type" );
    }

    SwPageFrm *pPage;
    if ( bVisAreaOnly )
        pPage = pImp->GetFirstVisPage();
    else
        pPage = (SwPageFrm*)pRoot->Lower();

    pCntntNode = NULL;
    nTxtPos = STRING_LEN;

    while ( pPage )
    {
        bPageValid = sal_True;
        const SwCntntFrm *pCnt = pPage->ContainsCntnt();
        while( pCnt && pPage->IsAnLower( pCnt ) )
        {
            if ( _DoIdleJob( pCnt, eJob ) )
                return sal_True;
            pCnt = pCnt->GetNextCntntFrm();
        }
        if ( pPage->GetSortedObjs() )
        {
            for ( sal_uInt16 i = 0; pPage->GetSortedObjs() &&
                                i < pPage->GetSortedObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pObj);
                    const SwCntntFrm *pC = pFly->ContainsCntnt();
                    while( pC )
                    {
                        if ( pC->IsTxtFrm() )
                        {
                            if ( _DoIdleJob( pC, eJob ) )
                                return sal_True;
                        }
                        pC = pC->GetNextCntntFrm();
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
                case SMART_TAGS : pPage->ValidateSmartTags(); break; // SMARTTAGS
            }
        }

        pPage = (SwPageFrm*)pPage->GetNext();
        if ( pPage && bVisAreaOnly &&
             !pPage->Frm().IsOver( pImp->GetShell()->VisArea()))
             break;
    }
    return sal_False;
}

#ifdef DBG_UTIL
/*************************************************************************
|*
|*  void SwLayIdle::SwLayIdle()
|*
|*************************************************************************/
void SwLayIdle::ShowIdle( ColorData eColorData )
{
    if ( !m_bIndicator )
    {
        m_bIndicator = true;
        Window *pWin = pImp->GetShell()->GetWin();
        if ( pWin )
        {
            Rectangle aRect( 0, 0, 5, 5 );
            aRect = pWin->PixelToLogic( aRect );
            // OD 2004-04-23 #116347#
            pWin->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
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

/*************************************************************************
|*
|*  void SwLayIdle::SwLayIdle()
|*
|*************************************************************************/
SwLayIdle::SwLayIdle( SwRootFrm *pRt, SwViewImp *pI ) :
    pRoot( pRt ),
    pImp( pI )
#ifdef DBG_UTIL
    , m_bIndicator( false )
#endif
{
    pImp->pIdleAct = this;

    SHOW_IDLE( COL_LIGHTRED );

    pImp->GetShell()->EnableSmooth( sal_False );

    // First, spellcheck the visible area. Only if there's nothing
    // to do there, we trigger the IdleFormat.
    if ( !DoIdleJob( SMART_TAGS, sal_True ) &&
         !DoIdleJob( ONLINE_SPELLING, sal_True ) &&
         !DoIdleJob( AUTOCOMPLETE_WORDS, sal_True ) ) // SMARTTAGS
    {
        // Format, then register repaint rectangles with the ViewShell if necessary.
        // This requires running artificial actions, so we don't get undesired
        // effects when for instance the page count gets changed.
        // We remember the shells where the cursor is visible, so we can make
        // it visible again if needed after a document change.
        std::vector<bool> aBools;
        ViewShell *pSh = pImp->GetShell();
        do
        {   ++pSh->nStartAction;
            sal_Bool bVis = sal_False;
            if ( pSh->ISA(SwCrsrShell) )
            {
#ifdef SW_CRSR_TIMER
                ((SwCrsrShell*)pSh)->ChgCrsrTimerFlag( sal_False );
#endif
                bVis = ((SwCrsrShell*)pSh)->GetCharRect().IsOver(pSh->VisArea());
            }
            aBools.push_back( bVis );
            pSh = (ViewShell*)pSh->GetNext();
        } while ( pSh != pImp->GetShell() );

        SwLayAction aAction( pRoot, pImp );
        aAction.SetInputType( VCL_INPUT_ANY );
        aAction.SetIdle( sal_True );
        aAction.SetWaitAllowed( sal_False );
        aAction.Action();

        // Further start/end actions only happen if there were paints started
        // somewhere or if the visibility of the CharRects has changed.
        sal_Bool bActions = sal_False;
        sal_uInt16 nBoolIdx = 0;
        do
        {
            --pSh->nStartAction;

            if ( pSh->Imp()->GetRegion() )
                bActions = sal_True;
            else
            {
                SwRect aTmp( pSh->VisArea() );
                pSh->UISizeNotify();

                // #137134#
                // Are we supposed to crash if pSh isn't a cursor shell?!
                // bActions |= aTmp != pSh->VisArea() ||
                //             aBools[nBoolIdx] != ((SwCrsrShell*)pSh)->GetCharRect().IsOver( pSh->VisArea() );

                // aBools[ i ] is true, if the i-th shell is a cursor shell (!!!)
                // and the cursor is visible.
                bActions |= aTmp != pSh->VisArea();
                if ( aTmp == pSh->VisArea() && pSh->ISA(SwCrsrShell) )
                {
                    bActions |= ((sal_Bool) aBools[nBoolIdx]) !=
                                static_cast<SwCrsrShell*>(pSh)->GetCharRect().IsOver( pSh->VisArea() );
                }
            }

            pSh = (ViewShell*)pSh->GetNext();
            ++nBoolIdx;
        } while ( pSh != pImp->GetShell() );

        if ( bActions )
        {
            // Prepare start/end actions via CrsrShell, so the cursor, selection
            // and VisArea can be set correctly.
            nBoolIdx = 0;
            do
            {
                sal_Bool bCrsrShell = pSh->IsA( TYPE(SwCrsrShell) );

                if ( bCrsrShell )
                    ((SwCrsrShell*)pSh)->SttCrsrMove();
//              else
//                  pSh->StartAction();

                // If there are accrued paints, it's best to simply invalidate
                // the whole window. Otherwise there would arise paint problems whose
                // solution would be disproportionally expensive.
                //fix(18176):
                SwViewImp *pViewImp = pSh->Imp();
                sal_Bool bUnlock = sal_False;
                if ( pViewImp->GetRegion() )
                {
                    pViewImp->DelRegion();

                    // Cause a repaint with virtual device.
                    pSh->LockPaint();
                    bUnlock = sal_True;
                }

                if ( bCrsrShell )
                    // If the Crsr was visible, we need to make it visible again.
                    // Otherwise, EndCrsrMove with sal_True for IdleEnd
                    ((SwCrsrShell*)pSh)->EndCrsrMove( sal_True^aBools[nBoolIdx] );
//              else
//                  pSh->EndAction();
                if( bUnlock )
                {
                    if( bCrsrShell )
                    {
                        // UnlockPaint overwrite the selection from the
                        // CrsrShell and calls the virtual method paint
                        // to fill the virtual device. This fill dont have
                        // paint the selection! -> Set the focus flag at
                        // CrsrShell and it dont paint the selection.
                        ((SwCrsrShell*)pSh)->ShLooseFcs();
                        pSh->UnlockPaint( sal_True );
                        ((SwCrsrShell*)pSh)->ShGetFcs( sal_False );
                    }
                    else
                        pSh->UnlockPaint( sal_True );
                }

                pSh = (ViewShell*)pSh->GetNext();
                ++nBoolIdx;

            } while ( pSh != pImp->GetShell() );
        }

        if ( !aAction.IsInterrupt() )
        {
            if ( !DoIdleJob( WORD_COUNT, sal_False ) )
                if ( !DoIdleJob( SMART_TAGS, sal_False ) )
                    if ( !DoIdleJob( ONLINE_SPELLING, sal_False ) )
                        DoIdleJob( AUTOCOMPLETE_WORDS, sal_False ); // SMARTTAGS
        }

        bool bInValid = false;
        const SwViewOption& rVOpt = *pImp->GetShell()->GetViewOptions();
        const ViewShell* pViewShell = pImp->GetShell();
        // See conditions in DoIdleJob()
        const sal_Bool bSpell     = rVOpt.IsOnlineSpell();
        const sal_Bool bACmplWrd  = rVOpt.IsAutoCompleteWords();
        const sal_Bool bWordCount = pViewShell->getIDocumentStatistics()->GetDocStat().bModified;
        const sal_Bool bSmartTags = !pViewShell->GetDoc()->GetDocShell()->IsHelpDocument() &&
                                !pViewShell->GetDoc()->isXForms() &&
                                SwSmartTagMgr::Get().IsSmartTagsEnabled(); // SMARTTAGS

        SwPageFrm *pPg = (SwPageFrm*)pRoot->Lower();
        do
        {
            bInValid = pPg->IsInvalidCntnt()    || pPg->IsInvalidLayout() ||
                       pPg->IsInvalidFlyCntnt() || pPg->IsInvalidFlyLayout() ||
                       pPg->IsInvalidFlyInCnt() ||
                       (bSpell && pPg->IsInvalidSpelling()) ||
                       (bACmplWrd && pPg->IsInvalidAutoCompleteWords()) ||
                       (bWordCount && pPg->IsInvalidWordCount()) ||
                       (bSmartTags && pPg->IsInvalidSmartTags()); // SMARTTAGS

            pPg = (SwPageFrm*)pPg->GetNext();

        } while ( pPg && !bInValid );

        if ( !bInValid )
        {
            pRoot->ResetIdleFormat();
            SfxObjectShell* pDocShell = pImp->GetShell()->GetDoc()->GetDocShell();
            pDocShell->Broadcast( SfxEventHint( SW_EVENT_LAYOUT_FINISHED, SwDocShell::GetEventName(STR_SW_EVENT_LAYOUT_FINISHED), pDocShell ) );
        }
    }

    pImp->GetShell()->EnableSmooth( sal_True );

    if( pImp->IsAccessible() )
        pImp->FireAccessibleEvents();

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
