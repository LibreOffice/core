/*************************************************************************
 *
 *  $RCSfile: layact.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-13 09:57:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <time.h>
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "doc.hxx"
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

#include <ftnidx.hxx>

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#define _SVSTDARR_BOOLS
#include <svtools/svstdarr.hxx>

#define _LAYACT_CXX
#include "layact.hxx"

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#include "swmodule.hxx"
#include "fmtline.hxx"
#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "frmsh.hxx"
#include "mdiexp.hxx"
#include "fmtornt.hxx"
#include "sectfrm.hxx"
#include "lineinfo.hxx"
#include "scrrect.hxx"
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif

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
#ifndef PRODUCT

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
                        pDoc->GetLayouter()->EndLoopControl(); \
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
                        pDoc->GetLayouter()->EndLoopControl(); \
                    return; \
                } \
            }
#endif

#define RESCHEDULE \
    { \
        if ( IsReschedule() )  \
            ::RescheduleProgress( pImp->GetShell()->GetDoc()->GetDocShell() ); \
    }

inline ULONG Ticks()
{
    return 1000 * clock() / CLOCKS_PER_SEC;
}

void SwLayAction::CheckWaitCrsr()
{
    RESCHEDULE
    if ( !IsWait() && IsWaitAllowed() && IsPaint() &&
         ((Ticks() - GetStartTicks()) >= CLOCKS_PER_SEC/2) )
    {
        pWait = new SwWait( *pRoot->GetFmt()->GetDoc()->GetDocShell(), TRUE );
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
        bInput = GetInputType() && GetpApp()->AnyInput( GetInputType() );
}

/*************************************************************************
|*
|*  SwLayAction::SetStatBar()
|*
|*  Ersterstellung      MA 10. Aug. 94
|*  Letzte Aenderung    MA 06. Aug. 95
|*
|*************************************************************************/
void SwLayAction::SetStatBar( BOOL bNew )
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
BOOL SwLayAction::PaintWithoutFlys( const SwRect &rRect, const SwCntntFrm *pCnt,
                                    const SwPageFrm *pPage )
{
    SwRegionRects aTmp( rRect );
    const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm *pSelfFly = pCnt->FindFlyFrm();

    for ( USHORT i = 0; i < rObjs.Count() && aTmp.Count(); ++i )
    {
        SdrObject *pO = rObjs[i];
        if ( !pO->IsWriterFlyFrame() )
            continue;

        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();

        if ( pFly == pSelfFly || !rRect.IsOver( pFly->Frm() ) )
            continue;

        if ( pSelfFly && pSelfFly->IsLowerOf( pFly ) )
            continue;

        if ( pFly->GetVirtDrawObj()->GetLayer() == pFly->GetFmt()->GetDoc()->GetHellId() )
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
                const FASTBOOL bLowerOfSelf = pFly->IsLowerOf( pSelfFly );
                if ( !bLowerOfSelf && !pFly->GetFmt()->GetOpaque().GetValue() )
                    //Aus anderem Layer interessieren uns nur nicht transparente
                    //oder innenliegende
                    continue;
            }
        }

        if ( !pFly->Lower() ||
             (pFly->Lower()->IsNoTxtFrm() &&
              (((SwNoTxtFrm*)pFly->Lower())->IsTransparent() ||
               pFly->GetFmt()->GetSurround().IsContour())) )
        {
            continue;
        }
        aTmp -= pFly->Frm();
    }

    BOOL bPaint = FALSE;
    const SwRect *pData = aTmp.GetData();
    for ( i = 0; i < aTmp.Count(); ++pData, ++i )
        bPaint |= pImp->GetShell()->AddPaintRect( *pData );
    return bPaint;
}

inline BOOL SwLayAction::_PaintCntnt( const SwCntntFrm *pCntnt,
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
    return FALSE;
}

void SwLayAction::PaintCntnt( const SwCntntFrm *pCnt,
                              const SwPageFrm *pPage,
                              const SwRect &rOldRect, long nOldBottom )
{
    const BOOL bHeightDiff  = rOldRect.Height() != pCnt->Frm().Height();
    if ( pCnt->IsCompletePaint() || !pCnt->IsTxtFrm() )
    {
        SwRect aPaint( pCnt->PaintArea() );
        aPaint.Union( rOldRect );
        if ( !_PaintCntnt( pCnt, pPage, aPaint ) )
            pCnt->ResetCompletePaint();
    }
    else
    {
        if( bHeightDiff )
        {
            SwRect aDrawRect( pCnt->UnionFrm( TRUE ) );
            if( rOldRect.Height() < pCnt->Frm().Height() )
                aDrawRect.Top( nOldBottom );
            else
                aDrawRect.Top( pCnt->Frm().Top() + pCnt->Prt().Bottom() + 1 );
            _PaintCntnt( pCnt, pPage, aDrawRect );
        }
        _PaintCntnt( pCnt, pPage, ((SwTxtFrm*)pCnt)->Paint() );
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
        aRect.Top( pTmp->Frm().Top() + pTmp->Prt().Bottom() + 1 );
        if ( !_PaintCntnt( pCnt, pPage, aRect ) )
            pCnt->ResetRetouche();
    }
}

/*************************************************************************
|*
|*  SwLayAction::_AddScrollRect()
|*
|*  Ersterstellung      MA 04. Mar. 94
|*  Letzte Aenderung    MA 04. Mar. 94
|*
|*************************************************************************/
BOOL MA_FASTCALL lcl_IsOverObj( const SwFrm *pFrm, const SwPageFrm *pPage,
                       const SwRect &rRect1, const SwRect &rRect2,
                       const SwLayoutFrm *pLay )
{
    const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm *pSelfFly = pFrm->FindFlyFrm();
    const BOOL bInCnt = pSelfFly && pSelfFly->IsFlyInCntFrm() ? TRUE : FALSE;

    for ( USHORT j = 0; j < rObjs.Count(); ++j )
    {
        const SdrObject         *pObj = rObjs[j];
        const SwRect aRect( pObj->GetBoundRect() );
        if ( !rRect1.IsOver( aRect ) && !rRect2.IsOver( aRect ) )
            continue;       //Keine Ueberlappung, der naechste.

        const SwVirtFlyDrawObj *pFlyObj = pObj->IsWriterFlyFrame() ?
                                                (SwVirtFlyDrawObj*)pObj : 0;
        const SwFlyFrm *pFly = pFlyObj ? pFlyObj->GetFlyFrm() : 0;

        //Wenn der Rahmen innerhalb des LayFrm verankert ist, so darf er
        //mitgescrollt werden, wenn er nicht seitlich aus dem Rechteck
        //herausschaut.
        if ( pLay && pFlyObj && pFlyObj->GetFlyFrm()->IsLowerOf( pLay ) )
        {
             if ( pFly->Frm().Left() < rRect1.Left() ||
                  pFly->Frm().Right()> rRect1.Right() )
                return TRUE;
            continue;
        }

        if ( !pSelfFly )    //Nur wenn der Frm in einem Fly steht kann
            return TRUE;    //es Einschraenkungen geben.

        if ( !pFlyObj )     //Keine Einschraenkung fuer Zeichenobjekte.
            return TRUE;

        if ( pFly != pSelfFly )
        {
            //Flys unter dem eigenen nur dann abziehen, wenn sie innerhalb des
            //eigenen stehen.
            //Fuer inhaltsgebundene Flys alle Flys abziehen fuer die gilt, dass
            //pSelfFly nicht innerhalb von ihnen steht.
            if ( bInCnt )
            {
                const SwFlyFrm *pTmp = pSelfFly->GetAnchor()->FindFlyFrm();
                while ( pTmp )
                {
                    if ( pTmp == pFly )
                        return FALSE;
                    else
                        pTmp = pTmp->GetAnchor()->FindFlyFrm();
                }
            } else if ( pObj->GetOrdNum() < pSelfFly->GetVirtDrawObj()->GetOrdNum() )
            {
                const SwFlyFrm *pTmp = pFly;
                do
                {   if ( pTmp == pSelfFly )
                        return TRUE;
                    else
                        pTmp = pTmp->GetAnchor()->FindFlyFrm();
                } while ( pTmp );
            } else
                return TRUE;
        }
    }
    return FALSE;
}

void SwLayAction::_AddScrollRect( const SwCntntFrm *pCntnt,
                                  const SwPageFrm *pPage,
                                  const SwTwips nOfst,
                                  const SwTwips nOldBottom )
{
    FASTBOOL bScroll = TRUE;
    SwRect aPaintRect( pCntnt->PaintArea() );

    //Wenn altes oder neues Rechteck mit einem Fly ueberlappen, in dem der
    //Cntnt nicht selbst steht, so ist nichts mit Scrollen.
    if ( pPage->GetSortedObjs() )
    {
        SwRect aRect( aPaintRect );
        aPaintRect.Pos().Y() -= nOfst;
        if ( ::lcl_IsOverObj( pCntnt, pPage, aPaintRect, aRect, 0 ) )
            bScroll = FALSE;
        aPaintRect.Pos().Y() += nOfst;
    }
    if ( bScroll && pPage->GetFmt()->GetBackground().GetGraphicPos() != GPOS_NONE )
        bScroll = FALSE;

    aPaintRect.Intersection( pCntnt->UnionFrm( TRUE ) );
    if ( bScroll )
    {
        if( aPaintRect.HasArea() )
            pImp->GetShell()->AddScrollRect( pCntnt, aPaintRect, nOfst );
        if ( pCntnt->IsRetouche() && !pCntnt->GetNext() )
        {
            SwRect aRect( pCntnt->GetUpper()->PaintArea() );
            aRect.Top( pCntnt->Frm().Top() + pCntnt->Prt().Bottom() + 1 );
            if ( !pImp->GetShell()->AddPaintRect( aRect ) )
                pCntnt->ResetRetouche();
        }
        pCntnt->ResetCompletePaint();
    }
    else if( aPaintRect.HasArea() )
    {
        aPaintRect.Pos().Y() -= nOfst;
        PaintCntnt( pCntnt, pPage, aPaintRect, nOldBottom );
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
    nPreInvaPage( USHRT_MAX ),
    nCheckPageNum( USHRT_MAX ),
    nStartTicks( Ticks() ),
    nInputType( 0 ),
    nEndPage( USHRT_MAX )
{
    bPaintExtraData = ::IsExtraData( pImp->GetShell()->GetDoc() );
    bPaint = bComplete = bWaitAllowed = bCheckPages = TRUE;
    bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
    bUpdateExpFlds = bBrowseActionStop = bActionInProgress = FALSE;
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
    bPaint = bComplete = bWaitAllowed = bCheckPages = TRUE;
    bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
    bUpdateExpFlds = bBrowseActionStop = FALSE;
}

/*************************************************************************
|*
|*  SwLayAction::RemoveEmptyBrowserPages()
|*
|*  Ersterstellung      MA 10. Sep. 97
|*  Letzte Aenderung    MA 10. Sep. 97
|*
|*************************************************************************/

BOOL SwLayAction::RemoveEmptyBrowserPages()
{
    //Beim umschalten vom normalen in den Browsermodus bleiben u.U. einige
    //unangenehm lange stehen. Diese beseiten wir mal schnell.
    BOOL bRet = FALSE;
    if ( pRoot->GetFmt()->GetDoc()->IsBrowseMode() )
    {
        SwPageFrm *pPage = (SwPageFrm*)pRoot->Lower();
        do
        {
            if ( (pPage->GetSortedObjs() && pPage->GetSortedObjs()->Count()) ||
                 pPage->ContainsCntnt() )
                pPage = (SwPageFrm*)pPage->GetNext();
            else
            {
                bRet = TRUE;
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
    bActionInProgress = TRUE;
    //TurboMode? Disqualifiziert fuer Idle-Format.
    if ( IsPaint() && !IsIdle() && TurboAction() )
    {
        delete pWait, pWait = 0;
        pRoot->ResetTurboFlag();
        bActionInProgress = FALSE;
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
        SetCheckPages( FALSE );

    InternalAction();
    bAgain |= RemoveEmptyBrowserPages();
    while ( IsAgain() )
    {
        bAgain = bNextCycle = FALSE;
        InternalAction();
        bAgain |= RemoveEmptyBrowserPages();
    }
    pRoot->DeleteEmptySct();

    delete pWait, pWait = 0;

    //Turbo-Action ist auf jedenfall wieder erlaubt.
    pRoot->ResetTurboFlag();
    pRoot->ResetTurbo();

    if ( IsInput() )
        pImp->GetShell()->SetNoNextScroll();
    SetCheckPages( TRUE );
    bActionInProgress = FALSE;
}

SwPageFrm *SwLayAction::CheckFirstVisPage( SwPageFrm *pPage )
{
    SwCntntFrm *pCnt = pPage->FindFirstBodyCntnt();
    SwCntntFrm *pChk = pCnt;
    BOOL bPageChgd = FALSE;
    while ( pCnt && pCnt->IsFollow() )
        pCnt = (SwCntntFrm*)pCnt->FindPrev();
    if ( pCnt && pChk != pCnt )
    {   bPageChgd = TRUE;
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
    const USHORT nFirstPageNum = pPage->GetPhyPageNum();

    while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
        pPage = (SwPageFrm*)pPage->GetNext();

    SwDoc* pDoc = pRoot->GetFmt()->GetDoc();
    BOOL bNoLoop = pPage ? SwLayouter::StartLoopControl( pDoc, pPage ) : NULL;
    USHORT nPercentPageNum = 0;
    while ( (pPage && !IsInput()) || nCheckPageNum != USHRT_MAX )
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
            SetCheckPages( TRUE );
            SwFrm::CheckPageDescs( pPage );
            SetCheckPages( FALSE );
            nCheckPageNum = USHRT_MAX;
            pPage = pTmp;
            continue;
        }

#ifdef MA_DEBUG
        static USHORT nStop = USHRT_MAX;
        if ( pPage->GetPhyPageNum() == nStop )
        {
            int bla = 5;
        }
        Window *pWin = pImp->GetShell()->GetWin();
        if ( pWin )
        {
            pWin->Push( PUSH_FILLCOLOR );
            pWin->SetFillColor( COL_WHITE );
            Point aOfst( pImp->GetShell()->VisArea().Pos() );
            pWin->DrawRect( Rectangle( aOfst, Size( 2000, 1000 )));
            pWin->DrawText( Point( 500, 500 ) + aOfst, pPage->GetPhyPageNum() );
            pWin->Pop();
        }
#endif
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
            if ( !IsInput() &&
                 (pRoot->IsSuperfluous() || pRoot->IsAssertFlyPages()) )
            {
                if ( pRoot->IsAssertFlyPages() )
                    pRoot->AssertFlyPages();
                if ( pRoot->IsSuperfluous() )
                {
                    BOOL bOld = IsAgain();
                    pRoot->RemoveSuperfluous();
                    bAgain = bOld;
                }
                if ( IsAgain() )
                {
                    if( bNoLoop )
                        pDoc->GetLayouter()->EndLoopControl();
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
            //Erst das Layout der Seite formatieren. Erst wenn das Layout
            //stabil ist lohnt sich die Inhaltsformatiertung.
            //Wenn durch die Inhaltsformatierung das Layout wieder ungueltig
            //wird, so wird die Inhaltsformatierung abgebrochen und das
            //Layout wird wieder stabilisiert.
            //Keine Angst: im Normafall kommt es nicht zu Oszillationen.
            //Das Spielchen spielen wir zweimal. erst fuer die Flys, dann
            //fuer den Rest.
            //Die Flys haben Vorrang, d.h. wenn sich an den Flys waehrend der
            //Formatierung des Bodys etwas aendert wird die Body-Formatierung
            //unterbrochen und wieder bei den Flys angefangen.

            while ( !IsInput() && !IsNextCycle() &&
                    ((IS_FLYS && IS_INVAFLY) || pPage->IsInvalid()) )
            {
                while ( !IsInput() && IS_INVAFLY && IS_FLYS )
                {
                    XCHECKPAGE;
                    if ( pPage->IsInvalidFlyLayout() )
                    {
                        pPage->ValidateFlyLayout();
                        FormatFlyLayout( pPage );
                        XCHECKPAGE;
                    }
                    if ( pPage->IsInvalidFlyCntnt() && IS_FLYS )
                    {
                        pPage->ValidateFlyCntnt();
                        if ( !FormatFlyCntnt( pPage ) )
                        {   XCHECKPAGE;
                            pPage->InvalidateFlyCntnt();
                        }
                    }
                }
                if ( !IS_FLYS )
                {
                    //Wenn keine Flys (mehr) da sind, sind die Flags
                    //mehr als fluessig.
                    pPage->ValidateFlyLayout();
                    pPage->ValidateFlyCntnt();
                }
                while ( !IsInput() && !IsNextCycle() && pPage->IsInvalid() &&
                        (!IS_FLYS || (IS_FLYS && !IS_INVAFLY)) )
                {
                    PROTOCOL( pPage, PROT_FILE_INIT, 0, 0)
                    XCHECKPAGE;
                    while ( !IsNextCycle() && pPage->IsInvalidLayout() )
                    {
                        pPage->ValidateLayout();
                        FormatLayout( pPage );
                        XCHECKPAGE;
                    }
                    if ( !IsNextCycle() && pPage->IsInvalidCntnt() &&
                         (!IS_FLYS || (IS_FLYS && !IS_INVAFLY)) )
                    {
                        pPage->ValidateFlyInCnt();
                        pPage->ValidateCntnt();
                        if ( !FormatCntnt( pPage ) )
                        {
                            XCHECKPAGE;
                            pPage->InvalidateCntnt();
                            pPage->InvalidateFlyInCnt();
                            if ( IsBrowseActionStop() )
                                bInput = TRUE;
                        }
                    }
                    if( bNoLoop )
                        pDoc->GetLayouter()->LoopControl( pPage, LOOP_PAGE );

                }
            }
            //Eine vorige Seite kann wieder invalid sein.
            XCHECKPAGE;
            if ( !IS_FLYS )
            {
                //Wenn keine Flys (mehr) da sind, sind die Flags
                //mehr als fluessig.
                pPage->ValidateFlyLayout();
                pPage->ValidateFlyCntnt();
            }
            if ( !IsInput() )
            {
                SetNextCycle( FALSE );

                if ( nPreInvaPage != USHRT_MAX )
                {
                    while ( pPage->GetPrev() && pPage->GetPhyPageNum() > nPreInvaPage )
                        pPage = (SwPageFrm*)pPage->GetPrev();
                    nPreInvaPage = USHRT_MAX;
                }

                //Ist eine Vorseite invalid?
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
                        (!IS_FLYS || (IS_FLYS && !IS_INVAFLY)) )
                {
                    pPage = (SwPageFrm*)pPage->GetNext();
                }
                if( bNoLoop )
                    pDoc->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
            }
        }
        if ( !pPage && !IsInput() &&
             (pRoot->IsSuperfluous() || pRoot->IsAssertFlyPages()) )
        {
            if ( pRoot->IsAssertFlyPages() )
                pRoot->AssertFlyPages();
            if ( pRoot->IsSuperfluous() )
            {
                BOOL bOld = IsAgain();
                pRoot->RemoveSuperfluous();
                bAgain = bOld;
            }
            if ( IsAgain() )
            {
                if( bNoLoop )
                    pDoc->GetLayouter()->EndLoopControl();
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
    if ( IsInput() && pPage )
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
        while ( pPg && !(pPg->Frm().Top() >= rVis.Bottom() ||
                         pPg->Frm().Left()>= rVis.Right()) )
        {
            XCHECKPAGE;
            while ( pPg->IsInvalidLayout() )
            {
                pPg->ValidateLayout();
                FormatLayout( pPg );
                XCHECKPAGE;
            }
            pPg = (SwPageFrm*)pPg->GetNext();
        }
    }
    pOptTab = 0;
    if( bNoLoop )
        pDoc->GetLayouter()->EndLoopControl();
}
/*************************************************************************
|*
|*  SwLayAction::TurboAction(), _TurboAction()
|*
|*  Ersterstellung      MA 04. Dec. 92
|*  Letzte Aenderung    MA 15. Aug. 93
|*
|*************************************************************************/
BOOL SwLayAction::_TurboAction( const SwCntntFrm *pCnt )
{

    const SwPageFrm *pPage = 0;
    if ( !pCnt->IsValid() || pCnt->IsCompletePaint() || pCnt->IsRetouche() )
    {
        const SwRect aOldRect( pCnt->UnionFrm( TRUE ) );
        const long   nOldBottom = pCnt->Frm().Top() + pCnt->Prt().Bottom();
        pCnt->Calc();
        if ( pCnt->Frm().Bottom() < aOldRect.Bottom() )
            pCnt->SetRetouche();

        pPage = pCnt->FindPageFrm();
        PaintCntnt( pCnt, pPage, aOldRect, nOldBottom );

        if ( !pCnt->GetValidLineNumFlag() && pCnt->IsTxtFrm() )
        {
            const ULONG nAllLines = ((SwTxtFrm*)pCnt)->GetAllLines();
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
            return FALSE;
        }

        if ( pPage->IsInvalidLayout() || (IS_FLYS && IS_INVAFLY) )
            return FALSE;
    }
    if ( !pPage )
        pPage = pCnt->FindPageFrm();
    //Die im Absatz verankerten Flys wollen auch beachtet werden.
    if ( pPage->IsInvalidFlyInCnt() && pCnt->GetDrawObjs() )
    {
        const SwDrawObjs *pObjs = pCnt->GetDrawObjs();
        for ( USHORT i = 0; i < pObjs->Count(); ++i )
        {
            SdrObject *pO = (*pObjs)[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if ( pFly->IsFlyInCntFrm() && ((SwFlyInCntFrm*)pFly)->IsInvalid() )
                {
                    FormatFlyInCnt( (SwFlyInCntFrm*)pFly );
                    pObjs = pCnt->GetDrawObjs();
                }
            }
        }
    }
    if ( pPage->IsInvalidCntnt() )
        return FALSE;
    return TRUE;
}

BOOL SwLayAction::TurboAction()
{
    BOOL bRet = TRUE;

    if ( pRoot->GetTurbo() )
    {
        if ( !_TurboAction( pRoot->GetTurbo() ) )
        {
            CheckIdleEnd();
            bRet = FALSE;
        }
        pRoot->ResetTurbo();
    }
    else
        bRet = FALSE;
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
const SwFrm *lcl_FindFirstInvaLay( const SwFrm *pFrm, long nBottom )
{
    ASSERT( pFrm->IsLayoutFrm(), "FindFirstInvaLay, no LayFrm" );

    if ( !pFrm->IsValid() || pFrm->IsCompletePaint() &&
         pFrm->Frm().Top() < nBottom )
        return pFrm;
    pFrm = ((SwLayoutFrm*)pFrm)->Lower();
    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
        {
            if ( !pFrm->IsValid() || pFrm->IsCompletePaint() &&
                 pFrm->Frm().Top() < nBottom )
                return pFrm;
            const SwFrm *pTmp;
            if ( 0 != (pTmp = ::lcl_FindFirstInvaLay( pFrm, nBottom )) )
                return pTmp;
        }
        pFrm = pFrm->GetNext();
    }
    return 0;
}

const SwFrm *lcl_FindFirstInvaCntnt( const SwLayoutFrm *pLay, long nBottom,
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
            const SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                const SdrObject *pO = rObjs[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    const SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
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

const SwFrm *lcl_FindFirstInvaFly( const SwPageFrm *pPage, long nBottom )
{
    ASSERT( pPage->GetSortedObjs(), "FindFirstInvaFly, no Flys" )

    for ( USHORT i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
    {
        SdrObject *pO = (*pPage->GetSortedObjs())[i];
        if ( pO->IsWriterFlyFrame() )
        {
            const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
            if ( pFly->Frm().Top() <= nBottom )
            {
                if ( pFly->IsInvalid() || pFly->IsCompletePaint() )
                    return pFly;

                const SwFrm *pTmp;
                if ( 0 != (pTmp = lcl_FindFirstInvaCntnt( pFly, nBottom, 0 )) &&
                     pTmp->Frm().Top() <= nBottom )
                    return pTmp;
            }
        }
    }
    return 0;
}

BOOL SwLayAction::IsShortCut( SwPageFrm *&prPage )
{
    BOOL bRet = FALSE;
    const FASTBOOL bBrowse = pRoot->GetFmt()->GetDoc()->IsBrowseMode();

    //Wenn die Seite nicht Gueltig ist wird sie schnell formatiert, sonst
    //gibts nix als Aerger.
    if ( !prPage->IsValid() )
    {
        if ( bBrowse )
        {
            prPage->Calc();
            if ( prPage->Lower() )
                prPage->Lower()->Calc();
        }
        else
            FormatLayout( prPage );
        if ( IsAgain() )
            return FALSE;
    }


    const SwRect &rVis = pImp->GetShell()->VisArea();
    if ( (prPage->Frm().Top() >= rVis.Bottom()) ||
         (prPage->Frm().Left()>= rVis.Right()) )
    {
        bRet = TRUE;

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
            FASTBOOL bTstCnt = TRUE;
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
                    bTstCnt = FALSE;
                }
            }

            if ( bTstCnt )
            {
                if ( pCntnt->IsInSct() )
                {
                    const SwSectionFrm *pSct = ((SwFrm*)pCntnt)->ImplFindSctFrm();
                    if ( !pSct->IsValid() )
                    {
                        pSct->Calc();
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return FALSE;
                    }
                }
                if ( !pCntnt->IsValid() )
                {   pCntnt->Calc();
                    pCntnt->SetCompletePaint();
                    if ( IsAgain() )
                        return FALSE;
                }
                if ( pCntnt->IsInTab() )
                {
                    const SwTabFrm *pTab = ((SwFrm*)pCntnt)->ImplFindTabFrm();
                    if ( !pTab->IsValid() )
                    {
                        pTab->Calc();
                        pTab->SetCompletePaint();
                        if ( IsAgain() )
                            return FALSE;
                    }
                }
                if ( pCntnt->IsInSct() )
                {
                    const SwSectionFrm *pSct = ((SwFrm*)pCntnt)->ImplFindSctFrm();
                    if ( !pSct->IsValid() )
                    {
                        pSct->Calc();
                        pSct->SetCompletePaint();
                        if ( IsAgain() )
                            return FALSE;
                    }
                }
#ifdef USED
                if ( (pCntnt->FindPageFrm() != p2ndPage) &&
                     prPage->GetPrev() )
                {
                    prPage = (SwPageFrm*)prPage->GetPrev();
                    bRet = FALSE;
                }
#else
                const SwPageFrm* pTmp = pCntnt->FindPageFrm();
                if ( pTmp != p2ndPage && prPage->GetPrev() )
                {
                    bRet = FALSE;
                    if( pTmp->GetPhyPageNum() < prPage->GetPhyPageNum()
                        && pTmp->IsInvalid() )
                        prPage = (SwPageFrm*)pTmp;
                    else
                        prPage = (SwPageFrm*)prPage->GetPrev();
                }
#endif
            }
        }
    }

    if ( !bRet && bBrowse )
    {
        const long nBottom = rVis.Bottom();
        const SwFrm *pFrm;
        if ( prPage->GetSortedObjs() &&
             (prPage->IsInvalidFlyLayout() || prPage->IsInvalidFlyCntnt()) &&
             0 != (pFrm = lcl_FindFirstInvaFly( prPage, nBottom )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return FALSE;
        }
        if ( prPage->IsInvalidLayout() &&
             0 != (pFrm = lcl_FindFirstInvaLay( prPage, nBottom )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return FALSE;
        }
        if ( (prPage->IsInvalidCntnt() || prPage->IsInvalidFlyInCnt()) &&
             0 != (pFrm = lcl_FindFirstInvaCntnt( prPage, nBottom, 0 )) &&
             pFrm->Frm().Top() <= nBottom )
        {
            return FALSE;
        }
        bRet = TRUE;
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwLayAction::ChkFlyAnchor()
|*
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 02. Sep. 96
|*
|*************************************************************************/
void SwLayAction::ChkFlyAnchor( SwFlyFrm *pFly, const SwPageFrm *pPage )
{
    //Wenn der Fly innerhalb eines anderen Rahmens gebunden ist, so sollte
    //dieser zuerst Formatiert werden.

    if ( pFly->GetAnchor()->IsInTab() )
        pFly->GetAnchor()->FindTabFrm()->Calc();

    SwFlyFrm *pAnch = pFly->GetAnchor()->FindFlyFrm();
    if ( pAnch )
    {
        ChkFlyAnchor( pAnch, pPage );
        CHECKPAGE;
        while ( pPage == pAnch->FindPageFrm() && FormatLayoutFly( pAnch ) )
            /* do nothing */;
    }
}


/*************************************************************************
|*
|*  SwLayAction::FormatFlyLayout()
|*
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 03. Jun. 96
|*
|*************************************************************************/
void SwLayAction::FormatFlyLayout( const SwPageFrm *pPage )
{
    for ( USHORT i = 0; pPage->GetSortedObjs() &&
                        i < pPage->GetSortedObjs()->Count(); ++i )
    {
        SdrObject *pO = (*pPage->GetSortedObjs())[i];
        if ( pO->IsWriterFlyFrame() )
        {
            const USHORT nOld = i;
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
            ChkFlyAnchor( pFly, pPage );
            if ( IsAgain() )
                return;
            while ( pPage == pFly->FindPageFrm() )
            {
                SwFrmFmt *pFmt = pFly->GetFmt();
                if( FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId() &&
                    pFly->GetAnchor() &&
                    ( REL_CHAR == pFmt->GetHoriOrient().GetRelationOrient() ||
                      REL_CHAR == pFmt->GetVertOrient().GetRelationOrient() ) )
                    _FormatCntnt( (SwCntntFrm*)pFly->GetAnchor(), pPage );
                 if( !FormatLayoutFly( pFly ) )
                    break;
            }
            CHECKPAGE;
            if ( !IS_FLYS )
                break;
            if ( nOld > pPage->GetSortedObjs()->Count() )
                i -= nOld - pPage->GetSortedObjs()->Count();
            else
            {   //Positionswechsel!
                USHORT nAct;
                pPage->GetSortedObjs()->Seek_Entry(pFly->GetVirtDrawObj(),&nAct);
                if ( nAct < i )
                    i = nAct;
                else if ( nAct > i )
                    --i;
            }
        }
    }
}
/*************************************************************************
|*
|*  SwLayAction::FormatLayout(), FormatLayoutFly, FormatLayoutTab()
|*
|*  Ersterstellung      MA 30. Oct. 92
|*  Letzte Aenderung    MA 18. May. 98
|*
|*************************************************************************/
BOOL SwLayAction::FormatLayout( SwLayoutFrm *pLay, BOOL bAddRect )
{
    ASSERT( !IsAgain(), "Ungueltige Seite beachten." );
    if ( IsAgain() )
        return FALSE;

    BOOL bChanged = FALSE;
    BOOL bAlreadyPainted = FALSE;

    if ( !pLay->IsValid() || pLay->IsCompletePaint() )
    {
        if ( pLay->GetPrev() && !pLay->GetPrev()->IsValid() )
            pLay->GetPrev()->SetCompletePaint();

        SwRect aOldRect( pLay->Frm() );
        pLay->Calc();
        if ( aOldRect != pLay->Frm() )
            bChanged = TRUE;

        FASTBOOL bNoPaint = FALSE;
        if ( pLay->IsPageBodyFrm() && pLay->Frm().Pos() == aOldRect.Pos() &&
             pLay->Lower() && pLay->GetFmt()->GetDoc()->IsBrowseMode() )
        {
            //HotFix: Vobis Homepage, nicht so genau hinsehen, sonst
            //rpaints

            //Einschraenkungen wegen Kopf-/Fusszeilen
            if ( !(pLay->IsCompletePaint() &&
                   (pLay->GetFmt()->GetDoc()->IsHeadInBrowse() ||
                    pLay->GetFmt()->GetDoc()->IsFootInBrowse() ||
                    pLay->FindPageFrm()->FindFtnCont())))
            {
                bNoPaint = TRUE;
            }
        }

        if ( !bNoPaint && IsPaint() && bAddRect && (pLay->IsCompletePaint() || bChanged) )
        {
            SwRect aPaint( pLay->Frm() );
            aPaint.Pos().Y() -= 1;
            aPaint.Pos().X() -= 1;
            aPaint.SSize().Height() += 2;
            aPaint.SSize().Width()  += 2;
            if ( pLay->IsPageFrm() && pLay->GetFmt()->GetDoc()->IsBrowseMode() )
            {
                //Ist die Aenderung ueberhaupt sichtbar?
                if ( pLay->IsCompletePaint() )
                {
                    pImp->GetShell()->AddPaintRect( aPaint );
                    bAddRect = FALSE;
                }
                else
                {
                    SwRegionRects aRegion( aOldRect );
                    aRegion -= aPaint;
                    for ( USHORT i = 0; i < aRegion.Count(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                    aRegion.ChangeOrigin( aPaint );
                    aRegion.Remove( 0, aRegion.Count() );
                    aRegion.Insert( aPaint, 0 );
                    aRegion -= aOldRect;
                    for ( i = 0; i < aRegion.Count(); ++i )
                        pImp->GetShell()->AddPaintRect( aRegion[i] );
                }

                //ggf. die Zwischenraeume mit invalidieren.
                if ( pLay->GetPrev() )
                {
                    SwRect aTmp( aPaint );
                    long nTmp = aTmp.Top() - DOCUMENTBORDER/2;
                    if ( nTmp >= 0 )
                        aTmp.Top( nTmp );
                    aTmp.Bottom( pLay->Frm().Top() );
                    pImp->GetShell()->AddPaintRect( aTmp );
                }
                if ( pLay->GetNext() )
                {
                    aPaint.Bottom( aPaint.Bottom() + DOCUMENTBORDER/2 );
                    aPaint.Top( pLay->Frm().Bottom() );
                    pImp->GetShell()->AddPaintRect( aPaint );
                }
            }
            else
            {
                pImp->GetShell()->AddPaintRect( aPaint );
                bAlreadyPainted = TRUE;
            }
        }
        pLay->ResetCompletePaint();
    }

    if ( IsPaint() && bAddRect &&
         !pLay->GetNext() && pLay->IsRetoucheFrm() && pLay->IsRetouche() )
    {
        SwRect aRect( pLay->GetUpper()->Prt() );
        aRect += pLay->GetUpper()->Frm().Pos();
        aRect.Top( pLay->Frm().Top() + pLay->Prt().Bottom() + 1 );
        if ( !pImp->GetShell()->AddPaintRect( aRect ) )
            pLay->ResetRetouche();
    }

    if( bAlreadyPainted )
        bAddRect = FALSE;

    CheckWaitCrsr();

    if ( IsAgain() )
        return FALSE;

    //Jetzt noch diejenigen Lowers versorgen die LayoutFrm's sind

    if ( pLay->IsFtnFrm() ) //Hat keine LayFrms als Lower.
        return bChanged;

    SwFrm *pLow = pLay->Lower();
    BOOL bTabChanged = FALSE;
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
            return FALSE;
        pLow = pLow->GetNext();
    }
    return bChanged || bTabChanged;
}

BOOL SwLayAction::FormatLayoutFly( SwFlyFrm *pFly, BOOL bAddRect )
{
    ASSERT( !IsAgain(), "Ungueltige Seite beachten." );
    if ( IsAgain() )
        return FALSE;

    BOOL bChanged = FALSE;

    if ( !pFly->IsValid() || pFly->IsCompletePaint() || pFly->IsInvalid() )
    {
        //Der Frame hat sich veraendert, er wird jetzt Formatiert
        const SwRect aOldRect( pFly->Frm() );
        pFly->Calc();
        bChanged = aOldRect != pFly->Frm();

        if ( IsPaint() && bAddRect && (pFly->IsCompletePaint() || bChanged) &&
             pFly->Frm().Top() > 0 && pFly->Frm().Left() > 0 )
            pImp->GetShell()->AddPaintRect( pFly->Frm() );

        if ( bChanged )
            pFly->Invalidate();
        else
            pFly->Validate();
        bAddRect = FALSE;
        pFly->ResetCompletePaint();
    }

    if ( IsAgain() )
        return FALSE;

    //Jetzt noch diejenigen Lowers versorgen die LayoutFrm's sind
    BOOL bTabChanged = FALSE;
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

BOOL MA_FASTCALL lcl_AreLowersScrollable( const SwLayoutFrm *pLay )
{
    const SwFrm *pLow = pLay->Lower();
    while ( pLow )
    {
        if ( pLow->IsCompletePaint() || !pLow->IsValid() )
            return FALSE;
        if ( pLow->IsLayoutFrm() && !::lcl_AreLowersScrollable( (SwLayoutFrm*)pLow ))
            return FALSE;
        pLow = pLow->GetNext();
    }
    return TRUE;
}

SwLayoutFrm * MA_FASTCALL lcl_IsTabScrollable( SwTabFrm *pTab )
{
    //returnt die erste unveraenderte Zeile, oder 0 wenn nicht
    //gescrollt werden darf.
    if ( !pTab->IsCompletePaint() )
    {
        SwLayoutFrm *pUnchgdRow = 0;
        SwLayoutFrm *pRow = (SwLayoutFrm*)pTab->Lower();
        while ( pRow )
        {
            if ( !::lcl_AreLowersScrollable( pRow ) )
                pUnchgdRow = 0;
            else if ( !pUnchgdRow )
                pUnchgdRow = pRow;
            pRow = (SwLayoutFrm*)pRow->GetNext();
        }
        return pUnchgdRow;
    }
    return 0;
}

void MA_FASTCALL lcl_ValidateLowers( SwLayoutFrm *pLay, const SwTwips nOfst,
                        SwLayoutFrm *pRow, SwPageFrm *pPage,
                        BOOL bResetOnly )
{
    pLay->ResetCompletePaint();
    SwFrm *pLow = pLay->Lower();
    if ( pRow )
        while ( pLow != pRow )
            pLow = pLow->GetNext();

    while ( pLow )
    {
        if ( !bResetOnly )
        {
            pLow->Frm().Pos().Y() += nOfst;
//#55435# schon hier wuerden die zeichengeb. Rahmen formatiert und dann unten
//faelschlich noch einmal verschoben.
            //pLow->Calc();
        }
        if ( pLow->IsLayoutFrm() )
            ::lcl_ValidateLowers( (SwLayoutFrm*)pLow, nOfst, 0, pPage, bResetOnly);
        else
        {
            pLow->ResetCompletePaint();
            if ( pLow->GetDrawObjs() )
            {
                for ( USHORT i = 0; i < pLow->GetDrawObjs()->Count(); ++i )
                {
                    SdrObject *pO = (*pLow->GetDrawObjs())[i];
                    if ( pO->IsWriterFlyFrame() )
                    {
                        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                        if ( !bResetOnly )
                        {
                            pFly->Frm().Pos().Y() += nOfst;
                            if ( pFly->IsFlyInCntFrm() )
                                ((SwFlyInCntFrm*)pFly)->AddRefOfst( nOfst );
                        }
                        ::lcl_ValidateLowers( pFly, nOfst, 0, pPage, bResetOnly);
                    }
                    else
                    {
                        pO->SetAnchorPos( pLow->Frm().Pos() );
                        SwFrmFmt *pFrmFmt = FindFrmFmt( pO );
                        if( !pFrmFmt ||
                            FLY_IN_CNTNT != pFrmFmt->GetAnchor().GetAnchorId() )
                            ((SwDrawContact*)pO->GetUserCall())->ChkPage();
                    }
                }
            }
        }
        if ( !bResetOnly )
            pLow->Calc();           //#55435# Stabil halten.
        pLow = pLow->GetNext();
    }
}

void MA_FASTCALL lcl_AddScrollRectTab( SwTabFrm *pTab, SwLayoutFrm *pRow,
                               const SwRect &rRect,
                               const SwTwips nOfst)
{
    //Wenn altes oder neues Rechteck mit einem Fly ueberlappen, in dem der
    //Frm nicht selbst steht, so ist nichts mit Scrollen.
    const SwPageFrm *pPage = pTab->FindPageFrm();
    SwRect aRect( rRect );
    aRect.Pos().Y() += nOfst;
    if ( pPage->GetSortedObjs() )
    {
        if ( ::lcl_IsOverObj( pTab, pPage, rRect, aRect, pTab ) )
            return;
    }
    if ( pPage->GetFmt()->GetBackground().GetGraphicPos() != GPOS_NONE )
        return;

    ViewShell *pSh = pPage->GetShell();
    if ( pSh )
        pSh->AddScrollRect( pTab, aRect, nOfst );
    ::lcl_ValidateLowers( pTab, nOfst, pRow, pTab->FindPageFrm(),
                                                pTab->IsLowersFormatted() );
}

BOOL CheckPos( SwFrm *pFrm )
{
    if ( !pFrm->GetValidPosFlag() )
    {
        Point aOld( pFrm->Frm().Pos() );
        pFrm->MakePos();
        if ( aOld != pFrm->Frm().Pos() )
        {
            pFrm->Frm().Pos( aOld );
            pFrm->_InvalidatePos();
            return FALSE;
        }
    }
    return TRUE;
}

BOOL SwLayAction::FormatLayoutTab( SwTabFrm *pTab, BOOL bAddRect )
{
    ASSERT( !IsAgain(), "8-) Ungueltige Seite beachten." );
    if ( IsAgain() )
        return FALSE;

    BOOL bChanged = FALSE;
    FASTBOOL bPainted = FALSE;

    const SwPageFrm *pOldPage = pTab->FindPageFrm();

    if ( !pTab->IsValid() || pTab->IsCompletePaint() || pTab->IsComplete() )
    {
        if ( pTab->GetPrev() && !pTab->GetPrev()->IsValid() )
            pTab->GetPrev()->SetCompletePaint();

        //Potenzielles Scrollrect ist die ganze Tabelle. Da bereits ein
        //Wachstum innerhalb der Tabelle - und damit der Tabelle selbst -
        //stattgefunden haben kann, muss die untere Kante durch die
        //Unterkante der letzten Zeile bestimmt werden.
        SwLayoutFrm *pRow;
        SwRect aScrollRect( pTab->PaintArea() );
        if ( IsPaint() || bAddRect )
        {
            pRow = (SwLayoutFrm*)pTab->Lower();
            while ( pRow->GetNext() )
                pRow = (SwLayoutFrm*)pRow->GetNext();
            aScrollRect.Bottom( pRow->Frm().Bottom() );
            //Die Oberkante wird ggf. durch die erste unveraenderte Zeile bestimmt.
            pRow = ::lcl_IsTabScrollable( pTab );
            if ( pRow && pRow != pTab->Lower() )
                aScrollRect.Top( pRow->Frm().Top() );
        }

        const SwFrm *pOldUp = pTab->GetUpper();

        SwRect aOldRect( pTab->Frm() );
        pTab->SetLowersFormatted( FALSE );
        pTab->Calc();
        if ( aOldRect != pTab->Frm() )
            bChanged = TRUE;
        SwRect aPaintFrm = pTab->PaintArea();

        if ( IsPaint() && bAddRect )
        {
            if ( pRow && pOldUp == pTab->GetUpper() &&
                 pTab->Frm().SSize() == aOldRect.SSize() &&
                 pTab->Frm().Left()  == aOldRect.Left() &&
                 pTab->IsAnLower( pRow ) )
            {
                SwTwips nOfst;
                if ( pRow->GetPrev() )
                {
                    if ( pRow->GetPrev()->IsValid() ||
                         ::CheckPos( pRow->GetPrev() ) )
                        nOfst = (pRow->GetPrev()->Frm().Bottom()+1) - pRow->Frm().Top();
                    else
                        nOfst = 0;
                }
                else
                    nOfst = pTab->Frm().Top() - aOldRect.Top();

                if ( nOfst )
                {
                     ::lcl_AddScrollRectTab( pTab, pRow, aScrollRect, nOfst );
                     bPainted = TRUE;
                }
            }

            if ( !pTab->IsCompletePaint() && pTab->IsComplete() &&
                 ( pTab->Frm().SSize() != pTab->Prt().SSize() ||
                   pTab->Prt().Left() ) )
            {
                if ( pTab->Frm().Height() != pTab->Prt().Height() )
                {
                    if ( pTab->Prt().Top() )
                    {
                        aOldRect = pTab->Frm();
                        aOldRect.Height( pTab->Prt().Top() );
                        pImp->GetShell()->AddPaintRect( aOldRect );
                    }
                    aOldRect.Height( pTab->Frm().Height() );
                    aOldRect.Top( aOldRect.Top() + pTab->Prt().Height() );
                    if ( aOldRect.HasArea() )
                        pImp->GetShell()->AddPaintRect( aOldRect );
                }
                if ( pTab->Frm().Width() != pTab->Prt().Width() ||
                     pTab->Prt().Left() )
                {
                    if ( pTab->Prt().Left() > 0 )
                    {
                        aOldRect = pTab->Frm();
                        aOldRect.Width( pTab->Prt().Left() );
                        pImp->GetShell()->AddPaintRect( aOldRect );
                    }
                    aOldRect.Width( pTab->Frm().Width() );
                    if( pTab->Prt().Left() + pTab->Prt().Width() <
                        aOldRect.Left() + aOldRect.Width() )
                    {
                        aOldRect.Left( aOldRect.Left() + pTab->Prt().Right() );
                        if ( aOldRect.HasArea() )
                            pImp->GetShell()->AddPaintRect( aOldRect );
                    }
                }
            }
            else if ( pTab->IsCompletePaint() )
            {
                pImp->GetShell()->AddPaintRect( aPaintFrm );
                bAddRect = FALSE;
                bPainted = TRUE;
            }
            if ( pTab->IsRetouche() && !pTab->GetNext() )
            {
                SwRect aRect( pTab->GetUpper()->PaintArea() );
                aRect.Top( pTab->Frm().Top() + pTab->Prt().Bottom() - 1 );
                if ( !pImp->GetShell()->AddPaintRect( aRect ) )
                    pTab->ResetRetouche();
            }
        }
        else
            bAddRect = FALSE;

        if ( pTab->IsCompletePaint() && !pOptTab )
            pOptTab = pTab;
        pTab->ResetCompletePaint();
    }
    if ( IsPaint() && bAddRect && pTab->IsRetouche() && !pTab->GetNext() )
    {
        SwRect aRect( pTab->PaintArea() );
        aRect.Top( pTab->Frm().Top() + pTab->Prt().Bottom() + 1 );
        if ( !pImp->GetShell()->AddPaintRect( aRect ) )
            pTab->ResetRetouche();
    }

    CheckWaitCrsr();

    //Heftige Abkuerzung!
    if ( pTab->IsLowersFormatted() &&
         (bPainted || !pImp->GetShell()->VisArea().IsOver( pTab->Frm())) )
        return FALSE;

    //Jetzt noch die Lowers versorgen
    if ( IsAgain() )
        return FALSE;
    SwLayoutFrm *pLow = (SwLayoutFrm*)pTab->Lower();
    while ( pLow )
    {
        bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
        if ( IsAgain() )
            return FALSE;
        pLow = (SwLayoutFrm*)pLow->GetNext();
    }

    if ( pOldPage->GetPhyPageNum() > (pTab->FindPageFrm()->GetPhyPageNum() + 1) )
        SetNextCycle( TRUE );

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
BOOL SwLayAction::FormatCntnt( const SwPageFrm *pPage )
{
    const SwCntntFrm *pCntnt = pPage->ContainsCntnt();
    const FASTBOOL bBrowse = pRoot->GetFmt()->GetDoc()->IsBrowseMode();

    while ( pCntnt && pPage->IsAnLower( pCntnt ) )
    {
        //Wenn der Cntnt sich eh nicht veraendert koennen wir ein paar
        //Abkuerzungen nutzen.
        const BOOL bFull = !pCntnt->IsValid() || pCntnt->IsCompletePaint() ||
                           pCntnt->IsRetouche() || pCntnt->GetDrawObjs();
        if ( bFull )
        {
            //Damit wir nacher nicht suchen muessen.
            const BOOL bNxtCnt = IsCalcLayout() && !pCntnt->GetFollow();
            const SwCntntFrm *pCntntNext = bNxtCnt ? pCntnt->GetNextCntntFrm() : 0;
            const SwCntntFrm *pCntntPrev = pCntnt->GetPrev() ? pCntnt->GetPrevCntntFrm() : 0;

            const SwLayoutFrm*pOldUpper  = pCntnt->GetUpper();
            const SwTabFrm *pTab = pCntnt->FindTabFrm();
            const BOOL bInValid = !pCntnt->IsValid() || pCntnt->IsCompletePaint();
            const BOOL bOldPaint = IsPaint();
            bPaint = bOldPaint && !(pTab && pTab == pOptTab);
            _FormatCntnt( pCntnt, pPage );
            bPaint = bOldPaint;

            if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
            {
                const ULONG nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
                ((SwTxtFrm*)pCntnt)->RecalcAllLines();
                if ( IsPaintExtraData() && IsPaint() &&
                     nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
                    pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
            }

            if ( IsAgain() )
                return FALSE;

            //Wenn Layout oder Flys wieder Invalid sind breche ich die Verarbeitung
            //vorlaeufig ab - allerdings nicht fuer die BrowseView, denn dort wird
            //das Layout staendig ungueltig, weil die Seitenhoehe angepasst wird.
            //Desgleichen wenn der Benutzer weiterarbeiten will und mindestens ein
            //Absatz verarbeitet wurde.
            if ( (!pTab || (pTab && !bInValid)) )
            {
                CheckIdleEnd();
                if ( IsInput() || (!bBrowse && pPage->IsInvalidLayout()) ||
                     (IS_FLYS && IS_INVAFLY) )
                    return FALSE;
            }
            if ( pOldUpper != pCntnt->GetUpper() )
            {
                const USHORT nCurNum = pCntnt->FindPageFrm()->GetPhyPageNum();
                if (  nCurNum < pPage->GetPhyPageNum() )
                    nPreInvaPage = nCurNum;

                //Wenn der Frm mehr als eine Seite rueckwaerts geflossen ist, so
                //fangen wir nocheinmal von vorn an damit wir nichts auslassen.
                if ( !IsCalcLayout() && pPage->GetPhyPageNum() > nCurNum+1 )
                {
                    SetNextCycle( TRUE );
                    return FALSE;
                }
            }
            //Wenn der Frame die Seite vorwaerts gewechselt hat, so lassen wir
            //den Vorgaenger nocheinmal durchlaufen.
            //So werden einerseits Vorgaenger erwischt, die jetzt fr Retouche
            //verantwortlich sind, andererseits werden die Fusszeilen
            //auch angefasst.
            FASTBOOL bSetCntnt = TRUE;
            if ( pCntntPrev )
            {
                if ( !pCntntPrev->IsValid() )
                    pPage->InvalidateCntnt();
                if ( pOldUpper != pCntnt->GetUpper() &&
                     pPage->GetPhyPageNum() < pCntnt->FindPageFrm()->GetPhyPageNum() )
                {
                    pCntnt = pCntntPrev;
                    bSetCntnt = FALSE;
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
                              !lcl_FindFirstInvaFly( pPage, nBottom )) &&
                              (!pPage->IsInvalidLayout() ||
                               !lcl_FindFirstInvaLay( pPage, nBottom )))
                            SetBrowseActionStop( TRUE );
                        return FALSE;
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
                const ULONG nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
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
                if ( IsInput() )
                    return FALSE;
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
                            !lcl_FindFirstInvaFly( pPage, nBottom )) &&
                            (!pPage->IsInvalidLayout() ||
                            !lcl_FindFirstInvaLay( pPage, nBottom )))
                        SetBrowseActionStop( TRUE );
                    return FALSE;
                }
            }
            pCntnt = pCntnt->GetNextCntntFrm();
        }
    }
    CheckWaitCrsr();
    return !IsInput();
}
/*************************************************************************
|*
|*  SwLayAction::_FormatCntnt()
|*
|*  Beschreibung        Returnt TRUE wenn der Absatz verarbeitet wurde,
|*                      FALSE wenn es nichts zu verarbeiten gab.
|*  Ersterstellung      MA 07. Dec. 92
|*  Letzte Aenderung    MA 11. Mar. 98
|*
|*************************************************************************/
void SwLayAction::_FormatCntnt( const SwCntntFrm *pCntnt,
                                const SwPageFrm *pPage )
{
    //wird sind hier evtl. nur angekommen, weil der Cntnt DrawObjekte haelt.
    const BOOL bDrawObjsOnly = pCntnt->IsValid() && !pCntnt->IsCompletePaint() &&
                         !pCntnt->IsRetouche();

    if ( !bDrawObjsOnly && IsPaint() )
    {
        const BOOL bPosOnly = !pCntnt->GetValidPosFlag() &&
                              !pCntnt->IsCompletePaint() &&
                              pCntnt->GetValidSizeFlag() &&
                              pCntnt->GetValidPrtAreaFlag() &&
                              ( !pCntnt->IsTxtFrm() ||
                                !((SwTxtFrm*)pCntnt)->HasAnimation() );
        const SwFrm *pOldUp = pCntnt->GetUpper();
        const SwRect aOldRect( pCntnt->UnionFrm() );
        const long   nOldBottom = pCntnt->Frm().Top() + pCntnt->Prt().Bottom();
        pCntnt->OptCalc();
        if( IsAgain() )
            return;
        if ( pCntnt->Frm().Bottom() < aOldRect.Bottom() )
            pCntnt->SetRetouche();
        const SwRect aNewRect( pCntnt->UnionFrm() );
        if ( bPosOnly && aNewRect.Top() != aOldRect.Top() &&
             !pCntnt->IsInTab() && !pCntnt->IsInSct() &&
             pOldUp == pCntnt->GetUpper() &&
             aNewRect.Left() == aOldRect.Left() &&
             aNewRect.SSize() == aOldRect.SSize() )
        {
            _AddScrollRect( pCntnt, pPage, pCntnt->Frm().Top() - aOldRect.Top(),
                            nOldBottom);
        }
        else
            PaintCntnt( pCntnt, pCntnt->FindPageFrm(), aOldRect, nOldBottom);
    }
    else
    {
        if ( IsPaint() && pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() )
            PaintCntnt( pCntnt, pPage, pCntnt->Frm(), pCntnt->Frm().Bottom());
        pCntnt->OptCalc();
    }

    //Die im Absatz verankerten Flys wollen auch mitspielen.
    if ( pCntnt->GetDrawObjs() )
    {
        const SwDrawObjs *pObjs = pCntnt->GetDrawObjs();
        for ( USHORT i = 0; i < pObjs->Count(); ++i )
        {
            SdrObject *pO = (*pObjs)[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if ( pFly->IsFlyInCntFrm() && ((SwFlyInCntFrm*)pFly)->IsInvalid() )
                {
                    FormatFlyInCnt( (SwFlyInCntFrm*)pFly );
                    pObjs = pCntnt->GetDrawObjs();
                    CHECKPAGE;
                }
            }
        }
    }
}
/*************************************************************************
|*
|*  SwLayAction::FormatFlyCntnt()
|*
|*      - Returnt TRUE wenn der Inhalt aller Flys vollstaendig verarbeitet
|*        wurde, FALSE bei einem vorzeitigen Abbruch.
|*  Ersterstellung      MA 02. Dec. 92
|*  Letzte Aenderung    MA 16. Sep. 93
|*
|*************************************************************************/
BOOL SwLayAction::FormatFlyCntnt( const SwPageFrm *pPage )
{
    for ( USHORT i = 0; pPage->GetSortedObjs() &&
                        i < pPage->GetSortedObjs()->Count(); ++i )
    {
        if ( IsAgain() )
            return FALSE;
        SdrObject *pO = (*pPage->GetSortedObjs())[i];
        if ( pO->IsWriterFlyFrame() )
        {
            const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
            if ( !_FormatFlyCntnt( pFly ) )
                return FALSE;
        }
    }
    return TRUE;
}
/*************************************************************************
|*
|*  SwLayAction::FormatFlyInCnt()
|*
|*  Beschreibung        Da die Flys im Cntnt nix mit der Seite am Hut
|*      (bzw. in den Bits ;-)) haben werden sie vom Cntnt (FormatCntnt)
|*      gerufen und hier verarbeitet. Die Verarebeitungsmimik ist
|*      prinzipiell die gleich wie bei Seiten nur nicht ganz so
|*      kompliziert (SwLayAction::Action()).
|*      - Returnt TRUE wenn der Fly vollstaendig verbeitet wurde, FALSE bei
|*        einem vorzeitigen Abbruch.
|*  Ersterstellung      MA 04. Dec. 92
|*  Letzte Aenderung    MA 24. Jun. 96
|*
|*************************************************************************/
void SwLayAction::FormatFlyInCnt( SwFlyInCntFrm *pFly )
{
    //Wg. Aenderung eine kleine Vorsichtsmassnahme. Es wird jetzt vor der
    //Cntntformatierung das Flag validiert und wenn die Formatierung mit
    //FALSE returnt wird halt wieder invalidiert.
    while ( pFly->IsInvalid() )
    {
        if ( pFly->IsInvalidLayout() )
        {
            while ( FormatLayoutFly( pFly ) )
            {
                if ( IsAgain() )
                    return;
            }
            if ( IsAgain() )
                return;
            pFly->ValidateLayout();
        }
        if ( pFly->IsInvalidCntnt() )
        {
            pFly->ValidateCntnt();
            if ( !_FormatFlyCntnt( pFly ) )
                pFly->InvalidateCntnt();
        }
    }
    CheckWaitCrsr();
}
/*************************************************************************
|*
|*  SwLayAction::_FormatFlyCntnt()
|*
|*  Beschreibung:
|*      - Returnt TRUE wenn alle Cntnts des Flys vollstaendig verarbeitet
|*        wurden. FALSE wenn vorzeitig unterbrochen wurde.
|*  Ersterstellung      MA 02. Dec. 92
|*  Letzte Aenderung    MA 24. Jun. 96
|*
|*************************************************************************/
BOOL SwLayAction::_FormatFlyCntnt( const SwFlyFrm *pFly )
{
    BOOL bOneProcessed = FALSE;
    const SwCntntFrm *pCntnt = pFly->ContainsCntnt();

    while ( pCntnt )
    {
        if ( __FormatFlyCntnt( pCntnt ) )
            bOneProcessed = TRUE;

        if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
        {
            const ULONG nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
            ((SwTxtFrm*)pCntnt)->RecalcAllLines();
            if ( IsPaintExtraData() && IsPaint() &&
                 nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
                pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
        }

        if ( IsAgain() )
            return FALSE;

        //wenn eine Eingabe anliegt breche ich die Verarbeitung ab.
        if ( bOneProcessed && !pFly->IsFlyInCntFrm() )
        {
            CheckIdleEnd();
            if ( IsInput() )
                return FALSE;
        }
        pCntnt = pCntnt->GetNextCntntFrm();
    }
    CheckWaitCrsr();
    return !IsInput();
}

/*************************************************************************
|*
|*  SwLayAction::__FormatFlyCntnt()
|*
|*  Beschreibung:
|*      - Returnt TRUE, wenn der Cntnt verarbeitet,
|*        d.h. Kalkuliert und/oder gepaintet wurde.
|*
|*  Ersterstellung      MA 05. Jan. 93
|*  Letzte Aenderung    MA 18. May. 95
|*
|*************************************************************************/
BOOL SwLayAction::__FormatFlyCntnt( const SwCntntFrm *pCntnt )
{
    BOOL bRet = FALSE;
    if ( !pCntnt->IsValid() || pCntnt->IsCompletePaint() ||
         pCntnt->IsRetouche() )
    {
        if ( IsPaint() )
        {
            const SwRect aOldRect( pCntnt->UnionFrm( TRUE ) );
            const long   nOldBottom = pCntnt->Frm().Top() + pCntnt->Prt().Bottom();
            pCntnt->OptCalc();
            if ( pCntnt->Frm().Bottom() <  aOldRect.Bottom() )
                pCntnt->SetRetouche();
            PaintCntnt( pCntnt, pCntnt->FindPageFrm(), aOldRect, nOldBottom );
        }
        else
            pCntnt->OptCalc();
        if( IsAgain() )
            return FALSE;
        bRet = TRUE;
    }
    else
    {
        //Falls der Frm schon vor der Abarbeitung hier formatiert wurde.
        if ( pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() &&
             IsPaint() )
            PaintCntnt( pCntnt, pCntnt->FindPageFrm(), pCntnt->Frm(), pCntnt->Frm().Bottom());
    }
    //Die im Absatz verankerten Flys wollen auch mitspielen.
    if ( pCntnt->GetDrawObjs() )
    {
        const SwDrawObjs *pObjs = pCntnt->GetDrawObjs();
        for ( USHORT i = 0; i < pObjs->Count(); ++i )
        {
            SdrObject *pO = (*pObjs)[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if ( pFly->IsFlyInCntFrm() && ((SwFlyInCntFrm*)pFly)->IsInvalid() )
                {
                    FormatFlyInCnt( (SwFlyInCntFrm*)pFly );
                    if ( IsAgain() )
                        return FALSE;
                    pObjs = pCntnt->GetDrawObjs();
                }
            }
        }
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwLayAction::FormatSpelling(), _FormatSpelling()
|*
|*  Ersterstellung      AMA 01. Feb. 96
|*  Letzte Aenderung    AMA 01. Feb. 96
|*
|*************************************************************************/
BOOL SwLayIdle::_FormatSpelling( const SwCntntFrm *pCnt )
{
    ASSERT( pCnt->IsTxtFrm(), "NoTxt neighbour of Txt" );
    if( pCnt->GetNode()->IsWrongDirty() )
    {
        if( STRING_LEN == nTxtPos )
        {
            --nTxtPos;
            ViewShell *pSh = pImp->GetShell();
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
        SwRect aRepaint( ((SwTxtFrm*)pCnt)->_AutoSpell( pCntntNode, nTxtPos ) );
        bPageValid = bPageValid && !pCnt->GetNode()->IsWrongDirty();
        if( !bPageValid )
            bAllValid = FALSE;
        if ( aRepaint.HasArea() )
            pImp->GetShell()->InvalidateWindows( aRepaint );
        if ( GetpApp()->AnyInput( INPUT_ANY ) )
            return TRUE;
    }
    //Die im Absatz verankerten Flys wollen auch mitspielen.
    if ( pCnt->GetDrawObjs() )
    {
        const SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
        for ( USHORT i = 0; i < rObjs.Count(); ++i )
        {
            SdrObject *pO = rObjs[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if ( pFly->IsFlyInCntFrm() )
                {
                    const SwCntntFrm *pC = pFly->ContainsCntnt();
                    while( pC )
                    {
                        if ( pC->IsTxtFrm() && _FormatSpelling( pC ) )
                            return TRUE;
                        pC = pC->GetNextCntntFrm();
                    }
                }
            }
        }
    }
    return FALSE;
}

BOOL SwLayIdle::FormatSpelling( BOOL bVisAreaOnly )
{
    //Spellchecken aller Inhalte der Seiten. Entweder nur der sichtbaren
    //Seiten oder eben aller. Nach dem Checken jedes Absatzes wird abgebrochen
    //wenn ein Input anliegt.
    if( !pImp->GetShell()->GetViewOptions()->IsOnlineSpell() )
        return FALSE;
    SwPageFrm *pPage;
    if ( bVisAreaOnly )
        pPage = pImp->GetFirstVisPage();
    else
        pPage = (SwPageFrm*)pRoot->Lower();

    pCntntNode = NULL;
    nTxtPos = STRING_LEN;

    while ( pPage )
    {
        bPageValid = TRUE;
        const SwCntntFrm *pCnt = pPage->ContainsCntnt();
        while( pCnt && pPage->IsAnLower( pCnt ) )
        {
            if ( _FormatSpelling( pCnt ) )
                return TRUE;
            pCnt = pCnt->GetNextCntntFrm();
        }
        if ( pPage->GetSortedObjs() )
        {
            for ( USHORT i = 0; pPage->GetSortedObjs() &&
                                i < pPage->GetSortedObjs()->Count(); ++i )
            {
                SdrObject *pO = (*pPage->GetSortedObjs())[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    const SwCntntFrm *pC = pFly->ContainsCntnt();
                    while( pC )
                    {
                        if ( pC->IsTxtFrm() && _FormatSpelling( pC ) )
                            return TRUE;
                        pC = pC->GetNextCntntFrm();
                    }
                }
            }
        }
        if( bPageValid )
            pPage->ValidateSpelling();
        pPage = (SwPageFrm*)pPage->GetNext();
        if ( pPage && bVisAreaOnly &&
             !pPage->Frm().IsOver( pImp->GetShell()->VisArea()))
            return  FALSE;
    }
    return FALSE;
}

BOOL SwLayIdle::_CollectAutoCmplWords( const SwCntntFrm *pCnt,
                                        BOOL bVisAreaOnly )
{
    ASSERT( pCnt->IsTxtFrm(), "NoTxt neighbour of Txt" );
    if( pCnt->GetNode()->IsAutoCompleteWordDirty() )
    {
        if( STRING_LEN == nTxtPos )
        {
            --nTxtPos;
            ViewShell *pSh = pImp->GetShell();
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

        ((SwTxtFrm*)pCnt)->CollectAutoCmplWrds( pCntntNode, nTxtPos,
                                                bVisAreaOnly );
/*      bPageValid = bPageValid && !pCnt->GetNode()->IsAutoCompleteWordDirty();

        if( !bPageValid )
            bAllValid = FALSE;
*/
        if ( GetpApp()->AnyInput( INPUT_ANY ) )
            return TRUE;
    }

    //Die im Absatz verankerten Flys wollen auch mitspielen.
    if ( pCnt->GetDrawObjs() )
    {
        const SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
        for ( USHORT i = 0; i < rObjs.Count(); ++i )
        {
            SdrObject *pO = rObjs[i];
            if ( pO->IsWriterFlyFrame() )
            {
                SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                if ( pFly->IsFlyInCntFrm() )
                {
                    const SwCntntFrm *pC = pFly->ContainsCntnt();
                    while( pC )
                    {
                        if( pC->IsTxtFrm() &&
                            _CollectAutoCmplWords( pC, bVisAreaOnly ) )
                            return TRUE;
                        pC = pC->GetNextCntntFrm();
                    }
                }
            }
        }
    }
    return FALSE;
}

BOOL SwLayIdle::CollectAutoCmplWords( BOOL bVisAreaOnly )
{
    //Worte aller Inhalte der Seiten zusammensammeln, um eine
    //AutoComplete-Liste zu erstellen. Entweder nur der sichtbaren
    //Seiten oder eben aller. Falls ein Input am Ende einer Seite anliegt,
    // so wird abgebrochen.
    if( !pImp->GetShell()->GetViewOptions()->IsAutoCompleteWords() ||
        pImp->GetShell()->GetDoc()->GetAutoCompleteWords().IsLockWordLstLocked())
        return FALSE;

    SwPageFrm *pPage;
    if ( bVisAreaOnly )
        pPage = pImp->GetFirstVisPage();
    else
        pPage = (SwPageFrm*)pRoot->Lower();

    pCntntNode = NULL;
    nTxtPos = STRING_LEN;

    while ( pPage )
    {
        bPageValid = TRUE;
        const SwCntntFrm *pCnt = pPage->ContainsCntnt();
        while( pCnt && pPage->IsAnLower( pCnt ) )
        {
            if ( _CollectAutoCmplWords( pCnt, bVisAreaOnly ) )
                return TRUE;
            pCnt = pCnt->GetNextCntntFrm();
        }
        if ( pPage->GetSortedObjs() )
        {
            for ( USHORT i = 0; pPage->GetSortedObjs() &&
                                i < pPage->GetSortedObjs()->Count(); ++i )
            {
                SdrObject *pO = (*pPage->GetSortedObjs())[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    const SwCntntFrm *pC = pFly->ContainsCntnt();
                    while( pC )
                    {
                        if ( pC->IsTxtFrm() &&
                            _CollectAutoCmplWords( pC, bVisAreaOnly ) )
                            return TRUE;
                        pC = pC->GetNextCntntFrm();
                    }
                }
            }
        }

        if( bPageValid )
            pPage->ValidateAutoCompleteWords();

        pPage = (SwPageFrm*)pPage->GetNext();
        if ( pPage && bVisAreaOnly &&
             !pPage->Frm().IsOver( pImp->GetShell()->VisArea()))
            break;
    }
    return FALSE;
}

#ifndef PRODUCT
#ifdef DEBUG

/*************************************************************************
|*
|*  void SwLayIdle::SwLayIdle()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 09. Jun. 94
|*
|*************************************************************************/
void SwLayIdle::ShowIdle( ColorName eName )
{
    if ( !bIndicator )
    {
        bIndicator = TRUE;
        Window *pWin = pImp->GetShell()->GetWin();
        if ( pWin )
        {
            Rectangle aRect( 0, 0, 5, 5 );
            aRect = pWin->PixelToLogic( aRect );
            pWin->Push( PUSH_FILLCOLOR );
            pWin->SetFillColor( eName );
            pWin->DrawRect( aRect );
            pWin->Pop();
        }
    }
}
#define SHOW_IDLE( ColorName ) ShowIdle( ColorName )
#else
#define SHOW_IDLE( ColorName )
#endif
#else
#define SHOW_IDLE( ColorName )
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
#ifndef PRODUCT
#ifdef DEBUG
    , bIndicator( FALSE )
#endif
#endif
{
    pImp->pIdleAct = this;

    SHOW_IDLE( COL_LIGHTRED );

    pImp->GetShell()->EnableSmooth( FALSE );

    //Zuerst den Sichtbaren Bereich Spellchecken, nur wenn dort nichts
    //zu tun war wird das IdleFormat angestossen.
    if ( !FormatSpelling( TRUE ) && !CollectAutoCmplWords( TRUE ))
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
            BOOL bVis = FALSE;
            if ( pSh->ISA(SwCrsrShell) )
            {
#ifdef SW_CRSR_TIMER
                ((SwCrsrShell*)pSh)->ChgCrsrTimerFlag( FALSE );
#endif
                bVis = ((SwCrsrShell*)pSh)->GetCharRect().IsOver(pSh->VisArea());
            }
            aBools.Insert( bVis, aBools.Count() );
            pSh = (ViewShell*)pSh->GetNext();
        } while ( pSh != pImp->GetShell() );

        SwLayAction aAction( pRoot, pImp );
        aAction.SetInputType( INPUT_ANY );
        aAction.SetIdle( TRUE );
        aAction.SetWaitAllowed( FALSE );
        aAction.Action();

        //Weitere Start-/EndActions nur auf wenn irgendwo Paints aufgelaufen
        //sind oder wenn sich die Sichtbarkeit des CharRects veraendert hat.
        FASTBOOL bActions = FALSE;
        USHORT nBoolIdx = 0;
        do
        {   --pSh->nStartAction;

            if ( pSh->Imp()->GetRegion() || pSh->Imp()->GetScrollRects() )
                bActions = TRUE;
            else
            {
                SwRect aTmp( pSh->VisArea() );
                pSh->UISizeNotify();

                bActions |= aTmp != pSh->VisArea() ||
                            aBools[nBoolIdx] !=
                                   ((SwCrsrShell*)pSh)->GetCharRect().IsOver(pSh->VisArea());
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
            {   FASTBOOL bCrsrShell = pSh->IsA( TYPE(SwCrsrShell) );

                if ( bCrsrShell )
                    ((SwCrsrShell*)pSh)->SttCrsrMove();
//              else
//                  pSh->StartAction();

                //Wenn Paints aufgelaufen sind, ist es am sinnvollsten schlicht das
                //gesamte Window zu invalidieren. Anderfalls gibt es Paintprobleme
                //deren Loesung unverhaeltnissmaessig aufwendig waere.
                //fix(18176):
                SwViewImp *pImp = pSh->Imp();
                FASTBOOL bUnlock = FALSE;
                if ( pImp->GetRegion() || pImp->GetScrollRects() )
                {
                    pImp->DelRegions();

                    //Fuer Repaint mit virtuellem Device sorgen.
                    pSh->LockPaint();
                    bUnlock = TRUE;
                }

                if ( bCrsrShell )
                    //Wenn der Crsr sichbar war wieder sichbar machen, sonst
                    //EndCrsrMove mit TRUE fuer IdleEnd.
                    ((SwCrsrShell*)pSh)->EndCrsrMove( TRUE^aBools[nBoolIdx] );
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
                        pSh->UnlockPaint( TRUE );
                        ((SwCrsrShell*)pSh)->ShGetFcs( FALSE );
                    }
                    else
                        pSh->UnlockPaint( TRUE );
                }

                pSh = (ViewShell*)pSh->GetNext();
                ++nBoolIdx;

            } while ( pSh != pImp->GetShell() );
        }

        if ( !aAction.IsInput() )
        {
            if( !FormatSpelling( FALSE ))
                CollectAutoCmplWords( FALSE );
        }

        FASTBOOL bInValid;
        const SwViewOption& rVOpt = *pImp->GetShell()->GetViewOptions();
        FASTBOOL bSpell = rVOpt.IsOnlineSpell(),
                 bACmplWrd = rVOpt.IsAutoCompleteWords();
        SwPageFrm *pPg = (SwPageFrm*)pRoot->Lower();
        do
        {   bInValid = pPg->IsInvalidCntnt()    || pPg->IsInvalidLayout() ||
                       pPg->IsInvalidFlyCntnt() || pPg->IsInvalidFlyLayout() ||
                       pPg->IsInvalidFlyInCnt() ||
                       (bSpell && pPg->IsInvalidSpelling()) ||
                       (bACmplWrd && pPg->IsInvalidAutoCompleteWords());

            pPg = (SwPageFrm*)pPg->GetNext();

        } while ( pPg && TRUE^bInValid );

        if ( TRUE^bInValid )
            pRoot->ResetIdleFormat();
    }

    pImp->GetShell()->EnableSmooth( TRUE );

#ifndef PRODUCT
#ifdef DEBUG
    if ( bIndicator && pImp->GetShell()->GetWin() )
    {
        Rectangle aRect( 0, 0, 5, 5 );
        aRect = pImp->GetShell()->GetWin()->PixelToLogic( aRect );
        pImp->GetShell()->GetWin()->Invalidate( aRect );
    }
#endif
#endif
}

SwLayIdle::~SwLayIdle()
{
    pImp->pIdleAct = 0;
}

