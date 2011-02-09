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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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
#include "viewopt.hxx"  // OnlineSpelling ueber Internal-TabPage testen.
#include "pam.hxx"      // OnlineSpelling wg. der aktuellen Cursorposition
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

#define _SVSTDARR_BOOLS
#include <svl/svstdarr.hxx>

#define _LAYACT_CXX
#include "layact.hxx"
#include <swwait.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <tools/shl.hxx>
#include <sfx2/progress.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif

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
// --> OD 2004-06-28 #i28701#
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <PostItMgr.hxx>

// <--
//#pragma optimize("ity",on)

/*************************************************************************
|*
|*  SwLayAction Statisches Geraffel
|*
|*  Ersterstellung      MA 22. Dec. 93
|*  Letzte Aenderung    MA 22. Dec. 93
|*
|*************************************************************************/

#define IS_FLYS (pPage->GetSortedObjs())
#define IS_INVAFLY (pPage->IsInvalidFly())


//Sparen von Schreibarbeit um den Zugriff auf zerstoerte Seiten zu vermeiden.
#ifdef DBG_UTIL

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
|*  Ersterstellung      MA 12. Aug. 94
|*  Letzte Aenderung    MA 24. Jun. 96
|*
|*************************************************************************/
//Ist es wirklich schon soweit...
inline void SwLayAction::CheckIdleEnd()
{
    if ( !IsInput() )
        bInput = GetInputType() && Application::AnyInput( GetInputType() );
}

/*************************************************************************
|*
|*  SwLayAction::SetStatBar()
|*
|*  Ersterstellung      MA 10. Aug. 94
|*  Letzte Aenderung    MA 06. Aug. 95
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
|*  Beschreibung        Je nach Typ wird der Cntnt entsprechend seinen
|*      Veraenderungen ausgegeben bzw. wird die auszugebende Flaeche in der
|*      Region eingetragen.
|*      PaintCntnt:  fuellt die Region,
|*  Ersterstellung      BP 19. Jan. 92
|*  Letzte Aenderung    MA 10. Sep. 96
|*
|*************************************************************************/
sal_Bool SwLayAction::PaintWithoutFlys( const SwRect &rRect, const SwCntntFrm *pCnt,
                                    const SwPageFrm *pPage )
{
    SwRegionRects aTmp( rRect );
    const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm *pSelfFly = pCnt->FindFlyFrm();
    sal_uInt16 i;

    for ( i = 0; i < rObjs.Count() && aTmp.Count(); ++i )
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
                    //Im gleichen Layer werden nur obenliegende beachtet.
                    continue;
            }
            else
            {
                const sal_Bool bLowerOfSelf = pFly->IsLowerOf( pSelfFly );
                if ( !bLowerOfSelf && !pFly->GetFmt()->GetOpaque().GetValue() )
                    //Aus anderem Layer interessieren uns nur nicht transparente
                    //oder innenliegende
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
    const SwRect *pData = aTmp.GetData();
    for ( i = 0; i < aTmp.Count(); ++pData, ++i )
        bRetPaint |= pImp->GetShell()->AddPaintRect( *pData );
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
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 09. Jun. 95
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

    pImp->pLayAct = this;   //Anmelden
}

SwLayAction::~SwLayAction()
{
    ASSERT( !pWait, "Wait object not destroyed" );
    pImp->pLayAct = 0;      //Abmelden
}

/*************************************************************************
|*
|*  SwLayAction::Reset()
|*
|*  Ersterstellung      MA 11. Aug. 94
|*  Letzte Aenderung    MA 09. Jun. 95
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
|*  Ersterstellung      MA 10. Sep. 97
|*  Letzte Aenderung    MA 10. Sep. 97
|*
|*************************************************************************/

sal_Bool SwLayAction::RemoveEmptyBrowserPages()
{
    //Beim umschalten vom normalen in den Browsermodus bleiben u.U. einige
    //unangenehm lange stehen. Diese beseiten wir mal schnell.
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
|*  Ersterstellung      MA 10. Aug. 94
|*  Letzte Aenderung    MA 06. Aug. 95
|*
|*************************************************************************/
void SwLayAction::Action()
{
    bActionInProgress = sal_True;

    //TurboMode? Disqualifiziert fuer Idle-Format.
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

    //Turbo-Action ist auf jedenfall wieder erlaubt.
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

    if ( pPage->GetFmt()->GetDoc()->GetFtnIdxs().Count() )
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
                    //Die 'oberste' Seite benutzten.
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
// --> OD 2004-11-03 #i114798# - unlock position on start and end of page
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
// <--

void SwLayAction::InternalAction()
{
    ASSERT( pRoot->Lower()->IsPageFrm(), ":-( Keine Seite unterhalb der Root.");

    pRoot->Calc();

    //Die erste ungueltige bzw. zu formatierende Seite ermitteln.
    //Bei einer Complete-Action ist es die erste ungueltige; mithin ist die
    //erste zu formatierende Seite diejenige Seite mit der Numemr eins.
    //Bei einer Luegen-Formatierung ist die Nummer der erste Seite die Nummer
    //der ersten Sichtbaren Seite.
    SwPageFrm *pPage = IsComplete() ? (SwPageFrm*)pRoot->Lower() :
                pImp->GetFirstVisPage();
    if ( !pPage )
        pPage = (SwPageFrm*)pRoot->Lower();

    //Wenn ein "Erster-Fliess-Cntnt" innerhalb der der ersten sichtbaren Seite
    //ein Follow ist, so schalten wir die Seite zurueck auf den Ur-Master dieses
    //Cntnt's
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
             //Kein ShortCut fuer Idle oder CalcLayout
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

            // OD 2004-05-12 #i28701# - scope for instance of class
            // <NotifyLayoutOfPageInProgress>
            {
                NotifyLayoutOfPageInProgress aLayoutOfPageInProgress( *pPage );

                while ( !IsInterrupt() && !IsNextCycle() &&
                        ((IS_FLYS && IS_INVAFLY) || pPage->IsInvalid()) )
                {
                    // OD 2004-05-10 #i28701#
                    SwObjectFormatter::FormatObjsAtFrm( *pPage, *pPage, this );
                    if ( !IS_FLYS )
                    {
                        //Wenn keine Flys (mehr) da sind, sind die Flags
                        //mehr als fluessig.
                        pPage->ValidateFlyLayout();
                        pPage->ValidateFlyCntnt();
                    }
                    // OD 2004-05-10 #i28701# - change condition
                    while ( !IsInterrupt() && !IsNextCycle() &&
                            ( pPage->IsInvalid() ||
                              (IS_FLYS && IS_INVAFLY) ) )
                    {
                        PROTOCOL( pPage, PROT_FILE_INIT, 0, 0)
                        XCHECKPAGE;

                        // FME 2007-08-30 #i81146# new loop control
                        sal_uInt16 nLoopControlRuns_1 = 0;
                        const sal_uInt16 nLoopControlMax = 20;

                        while ( !IsNextCycle() && pPage->IsInvalidLayout() )
                        {
                            pPage->ValidateLayout();

                            if ( ++nLoopControlRuns_1 > nLoopControlMax )
                            {
#if OSL_DEBUG_LEVEL > 1
                                ASSERT( false, "LoopControl_1 in SwLayAction::InternalAction" )
#endif
                                break;
                            }

                            FormatLayout( pPage );
                            XCHECKPAGE;
                        }
                        // OD 2004-05-10 #i28701# - change condition
                        if ( !IsNextCycle() &&
                             ( pPage->IsInvalidCntnt() ||
                               (IS_FLYS && IS_INVAFLY) ) )
                        {
                            pPage->ValidateFlyInCnt();
                            pPage->ValidateCntnt();
                            // --> OD 2004-05-10 #i28701#
                            pPage->ValidateFlyLayout();
                            pPage->ValidateFlyCntnt();
                            // <--
                            if ( !FormatCntnt( pPage ) )
                            {
                                XCHECKPAGE;
                                pPage->InvalidateCntnt();
                                pPage->InvalidateFlyInCnt();
                                // --> OD 2004-05-10 #i28701#
                                pPage->InvalidateFlyLayout();
                                pPage->InvalidateFlyCntnt();
                                // <--
                                if ( IsBrowseActionStop() )
                                    bInput = sal_True;
                            }
                        }
                        if( bNoLoop )
                            pLayoutAccess->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
                    }
                }
            } // end of scope for instance of class <NotifyLayoutOfPageInProgress>


            //Eine vorige Seite kann wieder invalid sein.
            XCHECKPAGE;
            if ( !IS_FLYS )
            {
                //Wenn keine Flys (mehr) da sind, sind die Flags
                //mehr als fluessig.
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

                //Weiter bis zur naechsten invaliden Seite.
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
        //Wenn ein Input anliegt wollen wir keinen Inhalt mehr Formatieren,
        //Das Layout muessen wir aber schon in Ordnung bringen.
        //Andernfalls kann folgende Situation auftreten (Bug: 3244):
        //Am Ende des Absatz der letzten Seite wird Text eingegeben, so das
        //der Absatz einen Follow fuer die nachste Seite erzeugt, ausserdem
        //wird gleich schnell weitergetippt - Es liegt waehrend der
        //Verarbeitung ein Input an. Der Absatz auf der neuen Seite wurde
        //bereits anformatiert, die neue Seite ist Formatiert und steht
        //auf CompletePaint, hat sich aber noch nicht im Auszugebenden Bereich
        //eingetragen. Es wird gepaintet, das CompletePaint der Seite wird
        //zurueckgesetzt weil der neue Absatz sich bereits eingetragen hatte,
        //aber die Raender der Seite werden nicht gepaintet. Naja, bei der
        //zwangslaeufig auftretenden naechsten LayAction traegt sich die Seite
        //nicht mehr ein, weil ihre (LayoutFrm-)Flags bereits zurueckgesetzt
        //wurden -- Der Rand der Seite wird nie gepaintet.
        SwPageFrm *pPg = pPage;
        XCHECKPAGE;
        const SwRect &rVis = pImp->GetShell()->VisArea();

        while( pPg && pPg->Frm().Bottom() < rVis.Top() )
            pPg = (SwPageFrm*)pPg->GetNext();
        if( pPg != pPage )
            pPg = pPg ? (SwPageFrm*)pPg->GetPrev() : pPage;

        // OD 14.04.2003 #106346# - set flag for interrupt content formatting
        mbFormatCntntOnInterrupt = IsInput() && !IsStopPrt();
        long nBottom = rVis.Bottom();
        // --> OD 2005-02-15 #i42586# - format current page, if idle action is active
        // This is an optimization for the case that the interrupt is created by
        // the move of a form control object, which is represented by a window.
        while ( pPg && ( pPg->Frm().Top() < nBottom ||
                         ( IsIdle() && pPg == pPage ) ) )
        // <--
        {
            // --> OD 2004-10-11 #i26945# - follow-up of #i28701#
            NotifyLayoutOfPageInProgress aLayoutOfPageInProgress( *pPg );

            XCHECKPAGE;

            // FME 2007-08-30 #i81146# new loop control
            sal_uInt16 nLoopControlRuns_2 = 0;
            const sal_uInt16 nLoopControlMax = 20;

            // OD 14.04.2003 #106346# - special case: interrupt content formatting
            // --> OD 2004-07-08 #i28701# - conditions, introduced by #106346#,
            // are incorrect (marcos IS_FLYS and IS_INVAFLY only works for <pPage>)
            // and are too strict.
            // --> OD 2005-06-09 #i50432# - adjust interrupt formatting to
            // normal page formatting - see above.
            while ( ( mbFormatCntntOnInterrupt &&
                      ( pPg->IsInvalid() ||
                        ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) ) ||
                    ( !mbFormatCntntOnInterrupt && pPg->IsInvalidLayout() ) )
            {
                XCHECKPAGE;
                // --> OD 2005-06-09 #i50432# - format also at-page anchored objects
                SwObjectFormatter::FormatObjsAtFrm( *pPg, *pPg, this );
                // <--
                // --> OD 2005-06-09 #i50432#
                if ( !pPg->GetSortedObjs() )
                {
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyCntnt();
                }
                // <--

                // FME 2007-08-30 #i81146# new loop control
                sal_uInt16 nLoopControlRuns_3 = 0;

                while ( pPg->IsInvalidLayout() )
                {
                    pPg->ValidateLayout();

                    if ( ++nLoopControlRuns_3 > nLoopControlMax )
                    {
#if OSL_DEBUG_LEVEL > 1
                        ASSERT( false, "LoopControl_3 in Interrupt formatting in SwLayAction::InternalAction" )
#endif
                        break;
                    }

                    FormatLayout( pPg );
                    XCHECKPAGE;
                }

                // --> OD 2005-06-09 #i50432#
                if ( mbFormatCntntOnInterrupt &&
                     ( pPg->IsInvalidCntnt() ||
                       ( pPg->GetSortedObjs() && pPg->IsInvalidFly() ) ) )
                // <--
                {
                    pPg->ValidateFlyInCnt();
                    pPg->ValidateCntnt();
                    // --> OD 2004-05-10 #i26945# - follow-up of fix #117736#
                    pPg->ValidateFlyLayout();
                    pPg->ValidateFlyCntnt();
                    // <--

                    if ( ++nLoopControlRuns_2 > nLoopControlMax )
                    {
#if OSL_DEBUG_LEVEL > 1
                        ASSERT( false, "LoopControl_2 in Interrupt formatting in SwLayAction::InternalAction" )
#endif
                        break;
                    }

                    if ( !FormatCntnt( pPg ) )
                    {
                        XCHECKPAGE;
                        pPg->InvalidateCntnt();
                        pPg->InvalidateFlyInCnt();
                        // --> OD 2004-05-10 #i26945# - follow-up of fix #117736#
                        pPg->InvalidateFlyLayout();
                        pPg->InvalidateFlyCntnt();
                        // <--
                    }
                    // --> OD 2005-04-06 #i46807# - we are statisfied, if the
                    // content is formatted once complete.
                    else
                    {
                        break;
                    }
                    // <--
                }
            }
            // <--
            pPg = (SwPageFrm*)pPg->GetNext();
        }
        // OD 14.04.2003 #106346# - reset flag for special interrupt content formatting.
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
|*  Ersterstellung      MA 04. Dec. 92
|*  Letzte Aenderung    MA 15. Aug. 93
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
                //Damit die restlichen LineNums auf der Seite bereichnet werden
                //und nicht hier abgebrochen wird.
                //Das im RecalcAllLines zu erledigen waere teuer, weil dort
                //auch in unnoetigen Faellen (normale Action) auch immer die
                //Seite benachrichtigt werden muesste.
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
|*  Beschreibung:       Liefert ein True, wenn die Seite vollstaendig unter
|*      oder rechts neben dem sichbaren Bereich liegt.
|*      Es kann passieren, dass sich die Verhaeltnisse derart aendern, dass
|*      die Verarbeitung (des Aufrufers!) mit der Vorgaengerseite der
|*      uebergebenen Seite weitergefuehrt werden muss. Der Paramter wird also
|*      ggf. veraendert!
|*      Fuer den BrowseMode kann auch dann der ShortCut aktiviert werden,
|*      wenn der ungueltige Inhalt der Seite unterhalb des sichbaren
|*      bereiches liegt.
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 18. Jul. 96
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
    ASSERT( pFrm->IsLayoutFrm(), "FindFirstInvaLay, no LayFrm" );

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

// --> OD 2005-02-21 #i37877# - consider drawing objects
static const SwAnchoredObject* lcl_FindFirstInvaObj( const SwPageFrm* _pPage,
                                              long _nBottom )
{
    ASSERT( _pPage->GetSortedObjs(), "FindFirstInvaObj, no Objs" )

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
// <--

sal_Bool SwLayAction::IsShortCut( SwPageFrm *&prPage )
{
    sal_Bool bRet = sal_False;
    const ViewShell *pSh = pRoot->GetCurrShell();
    const sal_Bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    //Wenn die Seite nicht Gueltig ist wird sie schnell formatiert, sonst
    //gibts nix als Aerger.
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

        //Jetzt wird es ein bischen unangenehm: Der erste CntntFrm dieser Seite
        //im Bodytext muss Formatiert werden, wenn er dabei die Seite
        //wechselt, muss ich nochmal eine Seite zuvor anfangen, denn
        //es wurde ein PageBreak verarbeitet.
//Noch unangenehmer: Der naechste CntntFrm ueberhaupt muss
        //Formatiert werden, denn es kann passieren, dass kurzfristig
        //leere Seiten existieren (Bsp. Absatz ueber mehrere Seiten
        //wird geloescht oder verkleinert).

        //Ist fuer den Browser uninteressant, wenn der letzte Cnt davor bereits
        //nicht mehr sichbar ist.

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
                //Der Cnt davor schon nicht mehr sichtbar?
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
                // --> OD 2004-06-04 #i27756# - check after each frame calculation,
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
                        // --> OD 2004-06-04 #i27756#
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
                    // --> OD 2004-06-04 #i27756#
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
                        // --> OD 2004-06-04 #i27756#
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
                        // --> OD 2004-06-04 #i27756#
                        bPageChg = pCntnt->FindPageFrm() != p2ndPage &&
                                   prPage->GetPrev();
                    }
                }

                // --> OD 2004-06-04 #i27756#
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
                // --> OD 2005-04-25 #121980# - no shortcut, if at previous page
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
                // <--
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
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 18. May. 98
|*
|*************************************************************************/
// OD 15.11.2002 #105155# - introduce support for vertical layout
sal_Bool SwLayAction::FormatLayout( SwLayoutFrm *pLay, sal_Bool bAddRect )
{
    ASSERT( !IsAgain(), "Ungueltige Seite beachten." );
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

        SwRect aOldRect( pLay->Frm() );
        pLay->Calc();
        if ( aOldRect != pLay->Frm() )
            bChanged = sal_True;

        sal_Bool bNoPaint = sal_False;
        if ( pLay->IsPageBodyFrm() &&
             pLay->Frm().Pos() == aOldRect.Pos() &&
             pLay->Lower() )
        {
            const ViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
            //Einschraenkungen wegen Kopf-/Fusszeilen
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
                const int nBorderWidth =
                        pImp->GetShell()->GetOut()->PixelToLogic( Size( pPageFrm->BorderPxWidth(), 0 ) ).Width();
                const int nShadowWidth =
                        pImp->GetShell()->GetOut()->PixelToLogic( Size( pPageFrm->ShadowPxWidth(), 0 ) ).Width();

                //mod #i6193# added sidebar width
                const SwPostItMgr* pPostItMgr = pImp->GetShell()->GetPostItMgr();
                const int nSidebarWidth = pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() ? pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth() : 0;
                switch ( pPageFrm->SidebarPosition() )
                {
                    case sw::sidebarwindows::SIDEBAR_LEFT:
                    {
                        aPaint.Left( aPaint.Left() - nBorderWidth - nSidebarWidth);
                        aPaint.Right( aPaint.Right() + nBorderWidth + nShadowWidth);
                    }
                    break;
                    case sw::sidebarwindows::SIDEBAR_RIGHT:
                    {
                        aPaint.Left( aPaint.Left() - nBorderWidth );
                        aPaint.Right( aPaint.Right() + nBorderWidth + nShadowWidth + nSidebarWidth);
                    }
                    break;
                    case sw::sidebarwindows::SIDEBAR_NONE:
                        // nothing to do
                    break;
                }
                aPaint.Top( aPaint.Top() - nBorderWidth );
                aPaint.Bottom( aPaint.Bottom() + nBorderWidth + nShadowWidth);
            }

            sal_Bool bPageInBrowseMode = pLay->IsPageFrm();
            if( bPageInBrowseMode )
            {
                const ViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
                if( !pSh || !pSh->GetViewOptions()->getBrowseMode() )
                    bPageInBrowseMode = sal_False;
            }
            if( bPageInBrowseMode )
            {
                // NOTE: no vertical layout in online layout
                //Ist die Aenderung ueberhaupt sichtbar?
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
                    for ( i = 0; i < aRegion.Count(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                    aRegion.ChangeOrigin( aPaint );
                    aRegion.Remove( 0, aRegion.Count() );
                    aRegion.Insert( aPaint, 0 );
                    aRegion -= aOldRect;
                    for ( i = 0; i < aRegion.Count(); ++i )
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

                if ( bPrev )
                {
                    // top
                    SwRect aSpaceToPrevPage( pLay->Frm() );
                    const SwTwips nTop = aSpaceToPrevPage.Top() - nHalfDocBorder;
                    if ( nTop >= 0 )
                        aSpaceToPrevPage.Top( nTop );
                    aSpaceToPrevPage.Bottom( pLay->Frm().Top() );
                    pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );

                    // left
                    aSpaceToPrevPage = pLay->Frm();
                    const SwTwips nLeft = aSpaceToPrevPage.Left() - nHalfDocBorder;
                    if ( nLeft >= 0 )
                        aSpaceToPrevPage.Left( nLeft );
                    aSpaceToPrevPage.Right( pLay->Frm().Left() );
                    pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );
                }
                if ( bNext )
                {
                    // bottom
                    SwRect aSpaceToNextPage( pLay->Frm() );
                    aSpaceToNextPage.Bottom( aSpaceToNextPage.Bottom() + nHalfDocBorder );
                    aSpaceToNextPage.Top( pLay->Frm().Bottom() );
                    pImp->GetShell()->AddPaintRect( aSpaceToNextPage );

                    // right
                    aSpaceToNextPage = pLay->Frm();
                    aSpaceToNextPage.Right( aSpaceToNextPage.Right() + nHalfDocBorder );
                    aSpaceToNextPage.Left( pLay->Frm().Right() );
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

    //Jetzt noch diejenigen Lowers versorgen die LayoutFrm's sind

    if ( pLay->IsFtnFrm() ) //Hat keine LayFrms als Lower.
        return bChanged;

    SwFrm *pLow = pLay->Lower();
    sal_Bool bTabChanged = sal_False;
    while ( pLow && pLow->GetUpper() == pLay )
    {
        if ( pLow->IsLayoutFrm() )
        {
            if ( pLow->IsTabFrm() )
                bTabChanged |= FormatLayoutTab( (SwTabFrm*)pLow, bAddRect );
            // bereits zum Loeschen angemeldete Ueberspringen
            else if( !pLow->IsSctFrm() || ((SwSectionFrm*)pLow)->GetSection() )
                bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
        }
        else if ( pImp->GetShell()->IsPaintLocked() )
            //Abkuerzung im die Zyklen zu minimieren, bei Lock kommt das
            //Paint sowieso (Primaer fuer Browse)
            pLow->OptCalc();

        if ( IsAgain() )
            return sal_False;
        pLow = pLow->GetNext();
    }
    // OD 11.11.2002 #104414# - add complete frame area as paint area, if frame
    // area has been already added and after formating its lowers the frame area
    // is enlarged.
    if ( bAlreadyPainted &&
         ( pLay->Frm().Width() > aFrmAtCompletePaint.Width() ||
           pLay->Frm().Height() > aFrmAtCompletePaint.Height() )
       )
    {
        pImp->GetShell()->AddPaintRect( pLay->Frm() );
    }
    return bChanged || bTabChanged;
}

sal_Bool SwLayAction::FormatLayoutFly( SwFlyFrm* pFly )
{
    ASSERT( !IsAgain(), "Ungueltige Seite beachten." );
    if ( IsAgain() )
        return sal_False;

    sal_Bool bChanged = false;
    sal_Bool bAddRect = true;

    if ( !pFly->IsValid() || pFly->IsCompletePaint() || pFly->IsInvalid() )
    {
        //Der Frame hat sich veraendert, er wird jetzt Formatiert
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
        bAddRect = false;
        pFly->ResetCompletePaint();
    }

    if ( IsAgain() )
        return sal_False;

    //Jetzt noch diejenigen Lowers versorgen die LayoutFrm's sind
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
    ASSERT( !IsAgain(), "8-) Ungueltige Seite beachten." );
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

    //Heftige Abkuerzung!
    if ( pTab->IsLowersFormatted() &&
         (bPainted || !pImp->GetShell()->VisArea().IsOver( pTab->Frm())) )
        return sal_False;

    //Jetzt noch die Lowers versorgen
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
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 16. Nov. 95
|*
|*************************************************************************/
sal_Bool SwLayAction::FormatCntnt( const SwPageFrm *pPage )
{
    const SwCntntFrm *pCntnt = pPage->ContainsCntnt();
    const ViewShell *pSh = pRoot->GetCurrShell();
    const sal_Bool bBrowse = pSh && pSh->GetViewOptions()->getBrowseMode();

    while ( pCntnt && pPage->IsAnLower( pCntnt ) )
    {
        //Wenn der Cntnt sich eh nicht veraendert koennen wir ein paar
        //Abkuerzungen nutzen.
        const sal_Bool bFull = !pCntnt->IsValid() || pCntnt->IsCompletePaint() ||
                           pCntnt->IsRetouche() || pCntnt->GetDrawObjs();
        if ( bFull )
        {
            //Damit wir nacher nicht suchen muessen.
            const sal_Bool bNxtCnt = IsCalcLayout() && !pCntnt->GetFollow();
            const SwCntntFrm *pCntntNext = bNxtCnt ? pCntnt->GetNextCntntFrm() : 0;
            const SwCntntFrm *pCntntPrev = pCntnt->GetPrev() ? pCntnt->GetPrevCntntFrm() : 0;

            const SwLayoutFrm*pOldUpper  = pCntnt->GetUpper();
            const SwTabFrm *pTab = pCntnt->FindTabFrm();
            const sal_Bool bInValid = !pCntnt->IsValid() || pCntnt->IsCompletePaint();
            const sal_Bool bOldPaint = IsPaint();
            bPaint = bOldPaint && !(pTab && pTab == pOptTab);
            _FormatCntnt( pCntnt, pPage );
            // --> OD 2004-11-05 #i26945# - reset <bPaint> before format objects
            bPaint = bOldPaint;
            // <--

            // OD 2004-05-10 #i28701# - format floating screen object at content frame.
            // No format, if action flag <bAgain> is set or action is interrupted.
            // OD 2004-08-30 #117736# - allow format on interruption of action, if
            // it's the format for this interrupt
            // --> OD 2004-11-01 #i23129#, #i36347# - pass correct page frame
            // to the object formatter.
            if ( !IsAgain() &&
                 ( !IsInterrupt() || mbFormatCntntOnInterrupt ) &&
                 pCntnt->IsTxtFrm() &&
                 !SwObjectFormatter::FormatObjsAtFrm( *(const_cast<SwCntntFrm*>(pCntnt)),
                                                      *(pCntnt->FindPageFrm()), this ) )
            // <--
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

            //Wenn Layout oder Flys wieder Invalid sind breche ich die Verarbeitung
            //vorlaeufig ab - allerdings nicht fuer die BrowseView, denn dort wird
            //das Layout staendig ungueltig, weil die Seitenhoehe angepasst wird.
            //Desgleichen wenn der Benutzer weiterarbeiten will und mindestens ein
            //Absatz verarbeitet wurde.
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

                //Wenn der Frm mehr als eine Seite rueckwaerts geflossen ist, so
                //fangen wir nocheinmal von vorn an damit wir nichts auslassen.
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
            //Wenn der Frame die Seite vorwaerts gewechselt hat, so lassen wir
            //den Vorgaenger nocheinmal durchlaufen.
            //So werden einerseits Vorgaenger erwischt, die jetzt f?r Retouche
            //verantwortlich sind, andererseits werden die Fusszeilen
            //auch angefasst.
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

            //Falls der Frm schon vor der Abarbeitung hier formatiert wurde.
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
|*  Beschreibung        Returnt sal_True wenn der Absatz verarbeitet wurde,
|*                      sal_False wenn es nichts zu verarbeiten gab.
|*  Ersterstellung      MA 07. Dec. 92
|*  Letzte Aenderung    MA 11. Mar. 98
|*
|*************************************************************************/
void SwLayAction::_FormatCntnt( const SwCntntFrm *pCntnt,
                                const SwPageFrm  *pPage )
{
    //wird sind hier evtl. nur angekommen, weil der Cntnt DrawObjekte haelt.
    const sal_Bool bDrawObjsOnly = pCntnt->IsValid() && !pCntnt->IsCompletePaint() &&
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
|*  Beschreibung:
|*      - Returnt sal_True wenn alle Cntnts des Flys vollstaendig verarbeitet
|*        wurden. sal_False wenn vorzeitig unterbrochen wurde.
|*  Ersterstellung      MA 02. Dec. 92
|*  Letzte Aenderung    MA 24. Jun. 96
|*
|*************************************************************************/
sal_Bool SwLayAction::_FormatFlyCntnt( const SwFlyFrm *pFly )
{
    const SwCntntFrm *pCntnt = pFly->ContainsCntnt();

    while ( pCntnt )
    {
        // OD 2004-05-10 #i28701#
        _FormatCntnt( pCntnt, pCntnt->FindPageFrm() );

        // --> OD 2004-07-23 #i28701# - format floating screen objects
        // at content text frame
        // --> OD 2004-11-02 #i23129#, #i36347# - pass correct page frame
        // to the object formatter.
        if ( pCntnt->IsTxtFrm() &&
             !SwObjectFormatter::FormatObjsAtFrm(
                                            *(const_cast<SwCntntFrm*>(pCntnt)),
                                            *(pCntnt->FindPageFrm()), this ) )
        // <--
        {
            // restart format with first content
            pCntnt = pFly->ContainsCntnt();
            continue;
        }
        // <--

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

        //wenn eine Eingabe anliegt breche ich die Verarbeitung ab.
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
|*  Ersterstellung      AMA 01. Feb. 96
|*  Letzte Aenderung    AMA 01. Feb. 96
|*
|*************************************************************************/
sal_Bool SwLayIdle::_DoIdleJob( const SwCntntFrm *pCnt, IdleJobType eJob )
{
    ASSERT( pCnt->IsTxtFrm(), "NoTxt neighbour of Txt" );
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
                if ( Application::AnyInput( INPUT_MOUSEANDKEYBOARD|INPUT_OTHER|INPUT_PAINT ) )
                    return sal_True;
                break;
            }
            case AUTOCOMPLETE_WORDS :
                ((SwTxtFrm*)pCnt)->CollectAutoCmplWrds( pCntntNode, nTxtPos );
                if ( Application::AnyInput( INPUT_ANY ) )
                    return sal_True;
                break;
            case WORD_COUNT :
            {
                const xub_StrLen nEnd = pTxtNode->GetTxt().Len();
                SwDocStat aStat;
                pTxtNode->CountWords( aStat, 0, nEnd );
                if ( Application::AnyInput( INPUT_ANY ) )
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
                if ( Application::AnyInput( INPUT_MOUSEANDKEYBOARD|INPUT_OTHER|INPUT_PAINT ) )
                    return sal_True;
                break;
            }
        }
    }

    //Die im Absatz verankerten Flys wollen auch mitspielen.
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
    //Spellchecken aller Inhalte der Seiten. Entweder nur der sichtbaren
    //Seiten oder eben aller.
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
        default: ASSERT( false, "Unknown idle job type" )
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
#if OSL_DEBUG_LEVEL > 1

/*************************************************************************
|*
|*  void SwLayIdle::SwLayIdle()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 09. Jun. 94
|*
|*************************************************************************/
void SwLayIdle::ShowIdle( ColorData eColorData )
{
    if ( !bIndicator )
    {
        bIndicator = sal_True;
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
#endif
#else
#define SHOW_IDLE( ColorData )
#endif

/*************************************************************************
|*
|*  void SwLayIdle::SwLayIdle()
|*
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 23. May. 95
|*
|*************************************************************************/
SwLayIdle::SwLayIdle( SwRootFrm *pRt, SwViewImp *pI ) :
    pRoot( pRt ),
    pImp( pI )
#ifdef DBG_UTIL
#if OSL_DEBUG_LEVEL > 1
    , bIndicator( sal_False )
#endif
#endif
{
    pImp->pIdleAct = this;

    SHOW_IDLE( COL_LIGHTRED );

    pImp->GetShell()->EnableSmooth( sal_False );

    //Zuerst den Sichtbaren Bereich Spellchecken, nur wenn dort nichts
    //zu tun war wird das IdleFormat angestossen.
    if ( !DoIdleJob( SMART_TAGS, sal_True ) &&
         !DoIdleJob( ONLINE_SPELLING, sal_True ) &&
         !DoIdleJob( AUTOCOMPLETE_WORDS, sal_True ) ) // SMARTTAGS
    {
        //Formatieren und ggf. Repaint-Rechtecke an der ViewShell vormerken.
        //Dabei muessen kuenstliche Actions laufen, damit es z.B. bei
        //Veraenderungen der Seitenzahl nicht zu unerwuenschten Effekten kommt.
        //Wir merken uns bei welchen Shells der Cursor sichtbar ist, damit
        //wir ihn bei Dokumentaenderung ggf. wieder sichbar machen koennen.
        SvBools aBools;
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
        aAction.SetInputType( INPUT_ANY );
        aAction.SetIdle( sal_True );
        aAction.SetWaitAllowed( sal_False );
        aAction.Action();

        //Weitere Start-/EndActions nur auf wenn irgendwo Paints aufgelaufen
        //sind oder wenn sich die Sichtbarkeit des CharRects veraendert hat.
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

                // --> FME 2006-08-03 #137134#
                // Are we supposed to crash if pSh isn't a cursor shell?!
                // bActions |= aTmp != pSh->VisArea() ||
                //             aBools[nBoolIdx] != ((SwCrsrShell*)pSh)->GetCharRect().IsOver( pSh->VisArea() );

                // aBools[ i ] is true, if the i-th shell is a cursor shell (!!!)
                // and the cursor is visible.
                bActions |= aTmp != pSh->VisArea();
                if ( aTmp == pSh->VisArea() && pSh->ISA(SwCrsrShell) )
                {
                    bActions |= aBools[nBoolIdx] !=
                                static_cast<SwCrsrShell*>(pSh)->GetCharRect().IsOver( pSh->VisArea() );
                }
            }

            pSh = (ViewShell*)pSh->GetNext();
            ++nBoolIdx;
        } while ( pSh != pImp->GetShell() );

        if ( bActions )
        {
            //Start- EndActions aufsetzen. ueber die CrsrShell, damit der
            //Cursor/Selektion und die VisArea korrekt gesetzt werden.
            nBoolIdx = 0;
            do
            {
                sal_Bool bCrsrShell = pSh->IsA( TYPE(SwCrsrShell) );

                if ( bCrsrShell )
                    ((SwCrsrShell*)pSh)->SttCrsrMove();
//              else
//                  pSh->StartAction();

                //Wenn Paints aufgelaufen sind, ist es am sinnvollsten schlicht das
                //gesamte Window zu invalidieren. Anderfalls gibt es Paintprobleme
                //deren Loesung unverhaeltnissmaessig aufwendig waere.
                //fix(18176):
                SwViewImp *pViewImp = pSh->Imp();
                sal_Bool bUnlock = sal_False;
                if ( pViewImp->GetRegion() )
                {
                    pViewImp->DelRegion();

                    //Fuer Repaint mit virtuellem Device sorgen.
                    pSh->LockPaint();
                    bUnlock = sal_True;
                }

                if ( bCrsrShell )
                    //Wenn der Crsr sichbar war wieder sichbar machen, sonst
                    //EndCrsrMove mit sal_True fuer IdleEnd.
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
#if OSL_DEBUG_LEVEL > 1
    if ( bIndicator && pImp->GetShell()->GetWin() )
    {
        // #i75172# Do not invalidate indicator, this may cause a endless loop. Instead, just repaint it
        // This should be replaced by an overlay object in the future, anyways. Since it's only for debug
        // purposes, it is not urgent.
        static bool bCheckWithoutInvalidating(true);
        if(bCheckWithoutInvalidating)
        {
            bIndicator = false; SHOW_IDLE( COL_LIGHTGREEN );
        }
        else
        {
            Rectangle aRect( 0, 0, 5, 5 );
            aRect = pImp->GetShell()->GetWin()->PixelToLogic( aRect );
            pImp->GetShell()->GetWin()->Invalidate( aRect );
        }
    }
#endif
#endif
}

SwLayIdle::~SwLayIdle()
{
    pImp->pIdleAct = 0;
}

