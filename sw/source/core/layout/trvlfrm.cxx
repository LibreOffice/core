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

#include <hints.hxx>
#include <comphelper/flagguard.hxx>
#include <tools/line.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <vcl/settings.hxx>
#include <fmtpdsc.hxx>
#include <fmtsrnd.hxx>
#include <pagedesc.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <ftnfrm.hxx>
#include <flyfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <txtfrm.hxx>
#include <viewopt.hxx>
#include <DocumentSettingManager.hxx>
#include <viscrs.hxx>
#include <dflyobj.hxx>
#include <crstate.hxx>
#include <dcontact.hxx>
#include <sortedobjs.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>

#include <cfloat>
#include <swselectionlist.hxx>

namespace {
    bool lcl_GetCrsrOfst_Objects( const SwPageFrm* pPageFrm, bool bSearchBackground,
           SwPosition *pPos, Point& rPoint, SwCrsrMoveState* pCMS  )
    {
        bool bRet = false;
        Point aPoint( rPoint );
        SwOrderIter aIter( pPageFrm );
        aIter.Top();
        while ( aIter() )
        {
            const SwVirtFlyDrawObj* pObj =
                                static_cast<const SwVirtFlyDrawObj*>(aIter());
            const SwAnchoredObject* pAnchoredObj = GetUserCall( aIter() )->GetAnchoredObj( aIter() );
            const SwFormatSurround& rSurround = pAnchoredObj->GetFrameFormat().GetSurround();
            const SvxOpaqueItem& rOpaque = pAnchoredObj->GetFrameFormat().GetOpaque();
            bool bInBackground = ( rSurround.GetSurround() == SURROUND_THROUGHT ) && !rOpaque.GetValue();

            bool bBackgroundMatches = ( bInBackground && bSearchBackground ) ||
                                      ( !bInBackground && !bSearchBackground );

            const SwFlyFrm* pFly = pObj ? pObj->GetFlyFrm() : 0;
            if ( pFly && bBackgroundMatches &&
                 ( ( pCMS && pCMS->m_bSetInReadOnly ) ||
                   !pFly->IsProtected() ) &&
                 pFly->GetCrsrOfst( pPos, aPoint, pCMS ) )
            {
                bRet = true;
                break;
            }

            if ( pCMS && pCMS->m_bStop )
                return false;
            aIter.Prev();
        }
        return bRet;
    }

    double lcl_getDistance( const SwRect& rRect, const Point& rPoint )
    {
        double nDist = 0.0;

        // If the point is inside the rectangle, then distance is 0
        // Otherwise, compute the distance to the center of the rectangle.
        if ( !rRect.IsInside( rPoint ) )
        {
            tools::Line aLine( rPoint, rRect.Center( ) );
            nDist = aLine.GetLength( );
        }

        return nDist;
    }
}

//For SwFlyFrm::GetCrsrOfst
class SwCrsrOszControl
{
public:
    // So the compiler can initialize the class already. No DTOR and member
    // as public members
    const SwFlyFrm *pEntry;
    const SwFlyFrm *pStk1;
    const SwFlyFrm *pStk2;

    bool ChkOsz( const SwFlyFrm *pFly )
        {
            bool bRet = true;
            if ( pFly != pStk1 && pFly != pStk2 )
            {
                pStk1 = pStk2;
                pStk2 = pFly;
                bRet  = false;
            }
            return bRet;
        }

    void Entry( const SwFlyFrm *pFly )
        {
            if ( !pEntry )
                pEntry = pStk1 = pFly;
        }

    void Exit( const SwFlyFrm *pFly )
        {
            if ( pFly == pEntry )
                pEntry = pStk1 = pStk2 = 0;
        }
};

static SwCrsrOszControl aOszCtrl = { 0, 0, 0 };

/** Searches the ContentFrm owning the PrtArea containing the point. */
bool SwLayoutFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                               SwCrsrMoveState* pCMS, bool ) const
{
    vcl::RenderContext* pRenderContext = getRootFrm()->GetCurrShell()->GetOut();
    bool bRet = false;
    const SwFrm *pFrm = Lower();
    while ( !bRet && pFrm )
    {
        pFrm->Calc(pRenderContext);

        // #i43742# New function
        const bool bContentCheck = pFrm->IsTextFrm() && pCMS && pCMS->m_bContentCheck;
        const SwRect aPaintRect( bContentCheck ?
                                 pFrm->UnionFrm() :
                                 pFrm->PaintArea() );

        if ( aPaintRect.IsInside( rPoint ) &&
             ( bContentCheck || pFrm->GetCrsrOfst( pPos, rPoint, pCMS ) ) )
            bRet = true;
        else
            pFrm = pFrm->GetNext();
        if ( pCMS && pCMS->m_bStop )
            return false;
    }
    return bRet;
}

/** Searches the page containing the searched point. */

bool SwPageFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                             SwCrsrMoveState* pCMS, bool bTestBackground ) const
{
    bool bRet = false;
    Point aPoint( rPoint );

    // check, if we have to adjust the point
    if ( !Frm().IsInside( aPoint ) )
    {
        aPoint.X() = std::max( aPoint.X(), Frm().Left() );
        aPoint.X() = std::min( aPoint.X(), Frm().Right() );
        aPoint.Y() = std::max( aPoint.Y(), Frm().Top() );
        aPoint.Y() = std::min( aPoint.Y(), Frm().Bottom() );
    }

    bool bTextRet = false;
    bool bBackRet = false;

    //Could it be a free flying one?
    //If his content should be protected, we can't set the Crsr in it, thus
    //all changes should be impossible.
    if ( GetSortedObjs() )
    {
        bRet = lcl_GetCrsrOfst_Objects( this, false, pPos, rPoint, pCMS );
    }

    if ( !bRet )
    {
        SwPosition aBackPos( *pPos );
        SwPosition aTextPos( *pPos );

        //We fix the StartPoint if no Content below the page 'answers' and then
        //start all over again one page before the current one.
        //However we can't use Flys in such a case.
        if ( SwLayoutFrm::GetCrsrOfst( &aTextPos, aPoint, pCMS ) )
        {
            bTextRet = true;
        }
        else
        {
            if ( pCMS && (pCMS->m_bStop || pCMS->m_bExactOnly) )
            {
                static_cast<SwCrsrMoveState*>(pCMS)->m_bStop = true;
                return false;
            }
            const SwContentFrm *pCnt = GetContentPos( aPoint, false, false, false, pCMS, false );
            if ( pCMS && pCMS->m_bStop )
                return false;

            OSL_ENSURE( pCnt, "Crsr is gone to a Black hole" );
            if( pCMS && pCMS->m_pFill && pCnt->IsTextFrm() )
                bTextRet = pCnt->GetCrsrOfst( &aTextPos, rPoint, pCMS );
            else
                bTextRet = pCnt->GetCrsrOfst( &aTextPos, aPoint, pCMS );

            if ( !bTextRet )
            {
                // Set point to pCnt, delete mark
                // this may happen, if pCnt is hidden
                aTextPos = SwPosition( *pCnt->GetNode(), SwIndex( const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pCnt->GetNode())), 0 ) );
                bTextRet = true;
            }
        }

        SwContentNode* pContentNode = aTextPos.nNode.GetNode().GetContentNode();
        bool bConsiderBackground = true;
        // If the text position is a clickable field, then that should have priority.
        if (pContentNode && pContentNode->IsTextNode())
        {
            SwTextNode* pTextNd = pContentNode->GetTextNode();
            SwTextAttr* pTextAttr = pTextNd->GetTextAttrForCharAt(aTextPos.nContent.GetIndex(), RES_TXTATR_FIELD);
            if (pTextAttr)
            {
                const SwField* pField = pTextAttr->GetFormatField().GetField();
                if (pField->IsClickable())
                    bConsiderBackground = false;
            }
        }

        // Check objects in the background if nothing else matched
        if ( GetSortedObjs() )
        {
            bBackRet = lcl_GetCrsrOfst_Objects( this, true, &aBackPos, rPoint, pCMS );
        }

        if ( ( bConsiderBackground && bTestBackground && bBackRet ) || !bTextRet )
        {
            bRet = bBackRet;
            (*pPos) = aBackPos;
        }
        else if (bTextRet && !bBackRet)
        {
            bRet = bTextRet;
            (*pPos) = aTextPos;
        }
        else
        {
            /* In order to provide a selection as accurable as possible when we have both
             * text and brackground object, then we compute the distance between both
             * would-be positions and the click point. The shortest distance wins.
             */
            double nTextDistance = 0;
            bool bValidTextDistance = false;
            if (pContentNode)
            {
                SwContentFrm* pTextFrm = pContentNode->getLayoutFrm( getRootFrm( ) );

                // try this again but prefer the "previous" position
                SwCrsrMoveState aMoveState;
                SwCrsrMoveState *const pState((pCMS) ? pCMS : &aMoveState);
                comphelper::FlagRestorationGuard g(
                        pState->m_bPosMatchesBounds, true);
                SwPosition prevTextPos(*pPos);
                SwLayoutFrm::GetCrsrOfst(&prevTextPos, aPoint, pState);

                SwRect aTextRect;
                pTextFrm->GetCharRect(aTextRect, prevTextPos);

                if (prevTextPos.nContent < pContentNode->Len())
                {
                    // aRextRect is just a line on the left edge of the
                    // previous character; to get a better measure from
                    // lcl_getDistance, extend that to a rectangle over
                    // the entire character.
                    SwPosition const nextTextPos(prevTextPos.nNode,
                            SwIndex(prevTextPos.nContent, +1));
                    SwRect nextTextRect;
                    pTextFrm->GetCharRect(nextTextRect, nextTextPos);
                    SWRECTFN(pTextFrm);
                    if ((aTextRect.*fnRect->fnGetTop)() ==
                        (nextTextRect.*fnRect->fnGetTop)()) // same line?
                    {
                        // need to handle mixed RTL/LTR portions somehow
                        if ((aTextRect.*fnRect->fnGetLeft)() <
                            (nextTextRect.*fnRect->fnGetLeft)())
                        {
                            (aTextRect.*fnRect->fnSetRight)(
                                    (nextTextRect.*fnRect->fnGetLeft)());
                        }
                        else // RTL
                        {
                            (aTextRect.*fnRect->fnSetLeft)(
                                    (nextTextRect.*fnRect->fnGetLeft)());
                        }
                    }
                }

                nTextDistance = lcl_getDistance(aTextRect, rPoint);
                bValidTextDistance = true;
            }

            double nBackDistance = 0;
            bool bValidBackDistance = false;
            SwContentNode* pBackNd = aBackPos.nNode.GetNode( ).GetContentNode( );
            if ( pBackNd && bConsiderBackground)
            {
                // FIXME There are still cases were we don't have the proper node here.
                SwContentFrm* pBackFrm = pBackNd->getLayoutFrm( getRootFrm( ) );
                SwRect rBackRect;
                if (pBackFrm)
                {
                    pBackFrm->GetCharRect( rBackRect, aBackPos );

                    nBackDistance = lcl_getDistance( rBackRect, rPoint );
                    bValidBackDistance = true;
                }
            }

            if ( bValidTextDistance && bValidBackDistance && basegfx::fTools::more( nTextDistance, nBackDistance ) )
            {
                bRet = bBackRet;
                (*pPos) = aBackPos;
            }
            else
            {
                bRet = bTextRet;
                (*pPos) = aTextPos;
            }
        }
    }

    if ( bRet )
        rPoint = aPoint;

    return bRet;
}

bool SwLayoutFrm::FillSelection( SwSelectionList& rList, const SwRect& rRect ) const
{
    bool bRet = false;
    if( rRect.IsOver(PaintArea()) )
    {
        const SwFrm* pFrm = Lower();
        while( pFrm )
        {
            pFrm->FillSelection( rList, rRect );
            pFrm = pFrm->GetNext();
        }
    }
    return bRet;
}

bool SwPageFrm::FillSelection( SwSelectionList& rList, const SwRect& rRect ) const
{
    bool bRet = false;
    if( rRect.IsOver(PaintArea()) )
    {
        bRet = SwLayoutFrm::FillSelection( rList, rRect );
        if( GetSortedObjs() )
        {
            const SwSortedObjs &rObjs = *GetSortedObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = rObjs[i];
                if( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) ==  nullptr )
                    continue;
                const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
                if( pFly->FillSelection( rList, rRect ) )
                    bRet = true;
            }
        }
    }
    return bRet;
}

bool SwRootFrm::FillSelection( SwSelectionList& aSelList, const SwRect& rRect) const
{
    const SwFrm *pPage = Lower();
    const long nBottom = rRect.Bottom();
    while( pPage )
    {
        if( pPage->Frm().Top() < nBottom )
        {
            if( pPage->Frm().Bottom() > rRect.Top() )
                pPage->FillSelection( aSelList, rRect );
            pPage = pPage->GetNext();
        }
        else
            pPage = 0;
    }
    return !aSelList.isEmpty();
}

/** Primary passes the call to the first page.
 *
 *  @return false, if the passed Point gets changed
 */
bool SwRootFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                             SwCrsrMoveState* pCMS, bool bTestBackground ) const
{
    const bool bOldAction = IsCallbackActionEnabled();
    const_cast<SwRootFrm*>(this)->SetCallbackActionEnabled( false );
    OSL_ENSURE( (Lower() && Lower()->IsPageFrm()), "No PageFrm found." );
    if( pCMS && pCMS->m_pFill )
        static_cast<SwCrsrMoveState*>(pCMS)->m_bFillRet = false;
    Point aOldPoint = rPoint;

    // search for page containing rPoint. The borders around the pages are considered
    const SwPageFrm* pPage = GetPageAtPos( rPoint, 0, true );

    // #i95626#
    // special handling for <rPoint> beyond root frames area
    if ( !pPage &&
         rPoint.X() > Frm().Right() &&
         rPoint.Y() > Frm().Bottom() )
    {
        pPage = dynamic_cast<const SwPageFrm*>(Lower());
        while ( pPage && pPage->GetNext() )
        {
            pPage = dynamic_cast<const SwPageFrm*>(pPage->GetNext());
        }
    }
    if ( pPage )
    {
        pPage->SwPageFrm::GetCrsrOfst( pPos, rPoint, pCMS, bTestBackground );
    }

    const_cast<SwRootFrm*>(this)->SetCallbackActionEnabled( bOldAction );
    if( pCMS )
    {
        if( pCMS->m_bStop )
            return false;
        if( pCMS->m_pFill )
            return pCMS->m_bFillRet;
    }
    return aOldPoint == rPoint;
}

/**
 * If this is about a Content-carrying cell the Crsr will be force inserted into one of the ContentFrms
 * if there are no other options.
 *
 * There is no entry for protected cells.
 */
bool SwCellFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                             SwCrsrMoveState* pCMS, bool ) const
{
    vcl::RenderContext* pRenderContext = getRootFrm()->GetCurrShell()->GetOut();
    // cell frame does not necessarily have a lower (split table cell)
    if ( !Lower() )
        return false;

    if ( !(pCMS && pCMS->m_bSetInReadOnly) &&
         GetFormat()->GetProtect().IsContentProtected() )
        return false;

    if ( pCMS && pCMS->m_eState == MV_TBLSEL )
    {
        const SwTabFrm *pTab = FindTabFrm();
        if ( pTab->IsFollow() && pTab->IsInHeadline( *this ) )
        {
            static_cast<SwCrsrMoveState*>(pCMS)->m_bStop = true;
            return false;
        }
    }

    if ( Lower() )
    {
        if ( Lower()->IsLayoutFrm() )
            return SwLayoutFrm::GetCrsrOfst( pPos, rPoint, pCMS );
        else
        {
            Calc(pRenderContext);
            bool bRet = false;

            const SwFrm *pFrm = Lower();
            while ( pFrm && !bRet )
            {
                pFrm->Calc(pRenderContext);
                if ( pFrm->Frm().IsInside( rPoint ) )
                {
                    bRet = pFrm->GetCrsrOfst( pPos, rPoint, pCMS );
                    if ( pCMS && pCMS->m_bStop )
                        return false;
                }
                pFrm = pFrm->GetNext();
            }
            if ( !bRet )
            {
                const bool bFill = pCMS && pCMS->m_pFill;
                Point aPoint( rPoint );
                const SwContentFrm *pCnt = GetContentPos( rPoint, true );
                if( bFill && pCnt->IsTextFrm() )
                {
                    rPoint = aPoint;
                }
                pCnt->GetCrsrOfst( pPos, rPoint, pCMS );
            }
            return true;
        }
    }

    return false;
}

//Problem: If two Flys have the same size and share the same position then
//they end inside each other.
//Because we recursively check if a Point doesn't randomly lie inside an other
//fly which lies completely inside the current Fly we could trigger an endless
//loop with the mentioned situation above.
//Using the helper class SwCrsrOszControl we prevent the recursion. During
//a recursion GetCrsrOfst picks the one which lies on top.
bool SwFlyFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                            SwCrsrMoveState* pCMS, bool ) const
{
    vcl::RenderContext* pRenderContext = getRootFrm()->GetCurrShell()->GetOut();
    aOszCtrl.Entry( this );

    //If the Points lies inside the Fly, we try hard to set the Crsr inside it.
    //However if the Point sits inside a Fly which is completely located inside
    //the current one, we call GetCrsrOfst for it.
    Calc(pRenderContext);
    bool bInside = Frm().IsInside( rPoint ) && Lower();
    bool bRet = false;

    //If an Frm contains a graphic, but only text was requested, it basically
    //won't accept the Crsr.
    if ( bInside && pCMS && pCMS->m_eState == MV_SETONLYTEXT &&
         (!Lower() || Lower()->IsNoTextFrm()) )
        bInside = false;

    const SwPageFrm *pPage = FindPageFrm();
    if ( bInside && pPage && pPage->GetSortedObjs() )
    {
        SwOrderIter aIter( pPage );
        aIter.Top();
        while ( aIter() && !bRet )
        {
            const SwVirtFlyDrawObj* pObj = static_cast<const SwVirtFlyDrawObj*>(aIter());
            const SwFlyFrm* pFly = pObj ? pObj->GetFlyFrm() : 0;
            if ( pFly && pFly->Frm().IsInside( rPoint ) &&
                 Frm().IsInside( pFly->Frm() ) )
            {
                if ( aOszCtrl.ChkOsz( pFly ) )
                    break;
                bRet = pFly->GetCrsrOfst( pPos, rPoint, pCMS );
                if ( bRet )
                    break;
                if ( pCMS && pCMS->m_bStop )
                    return false;
            }
            aIter.Next();
        }
    }

    while ( bInside && !bRet )
    {
        const SwFrm *pFrm = Lower();
        while ( pFrm && !bRet )
        {
            pFrm->Calc(pRenderContext);
            if ( pFrm->Frm().IsInside( rPoint ) )
            {
                bRet = pFrm->GetCrsrOfst( pPos, rPoint, pCMS );
                if ( pCMS && pCMS->m_bStop )
                    return false;
            }
            pFrm = pFrm->GetNext();
        }
        if ( !bRet )
        {
            const bool bFill = pCMS && pCMS->m_pFill;
            Point aPoint( rPoint );
            const SwContentFrm *pCnt = GetContentPos( rPoint, true, false, false, pCMS );
            if ( pCMS && pCMS->m_bStop )
                return false;
            if( bFill && pCnt->IsTextFrm() )
            {
                rPoint = aPoint;
            }
            pCnt->GetCrsrOfst( pPos, rPoint, pCMS );
            bRet = true;
        }
    }
    aOszCtrl.Exit( this );
    return bRet;
}

/** Layout dependent cursor travelling */
bool SwContentFrm::LeftMargin(SwPaM *pPam) const
{
    if( &pPam->GetNode() != GetNode() )
        return false;
    const_cast<SwContentNode*>(GetNode())->
        MakeStartIndex(&pPam->GetPoint()->nContent);
    return true;
}

bool SwContentFrm::RightMargin(SwPaM *pPam, bool) const
{
    if( &pPam->GetNode() != GetNode() )
        return false;
    const_cast<SwContentNode*>(GetNode())->
        MakeEndIndex(&pPam->GetPoint()->nContent);
    return true;
}

static const SwContentFrm *lcl_GetNxtCnt( const SwContentFrm* pCnt )
{
    return pCnt->GetNextContentFrm();
}

static const SwContentFrm *lcl_GetPrvCnt( const SwContentFrm* pCnt )
{
    return pCnt->GetPrevContentFrm();
}

typedef const SwContentFrm *(*GetNxtPrvCnt)( const SwContentFrm* );

/// Frame in repeated headline?
static bool lcl_IsInRepeatedHeadline( const SwFrm *pFrm,
                                    const SwTabFrm** ppTFrm = 0 )
{
    const SwTabFrm *pTab = pFrm->FindTabFrm();
    if( ppTFrm )
        *ppTFrm = pTab;
    return pTab && pTab->IsFollow() && pTab->IsInHeadline( *pFrm );
}

/// Skip protected table cells. Optionally also skip repeated headlines.
//MA 1998-01-26: Chg also skip other protected areas
//FME: Skip follow flow cells
static const SwContentFrm * lcl_MissProtectedFrames( const SwContentFrm *pCnt,
                                                   GetNxtPrvCnt fnNxtPrv,
                                                   bool bMissHeadline,
                                                   bool bInReadOnly,
                                                   bool bMissFollowFlowLine )
{
    if ( pCnt && pCnt->IsInTab() )
    {
        bool bProtect = true;
        while ( pCnt && bProtect )
        {
            const SwLayoutFrm *pCell = pCnt->GetUpper();
            while ( pCell && !pCell->IsCellFrm() )
                pCell = pCell->GetUpper();
            if ( !pCell ||
                    (( ( bInReadOnly || !pCell->GetFormat()->GetProtect().IsContentProtected() ) &&
                      ( !bMissHeadline || !lcl_IsInRepeatedHeadline( pCell ) ) &&
                      ( !bMissFollowFlowLine || !pCell->IsInFollowFlowRow() ) &&
                       !pCell->IsCoveredCell()) ) )
                bProtect = false;
            else
                pCnt = (*fnNxtPrv)( pCnt );
        }
    }
    else if ( !bInReadOnly )
        while ( pCnt && pCnt->IsProtected() )
            pCnt = (*fnNxtPrv)( pCnt );

    return pCnt;
}

static bool lcl_UpDown( SwPaM *pPam, const SwContentFrm *pStart,
                    GetNxtPrvCnt fnNxtPrv, bool bInReadOnly )
{
    OSL_ENSURE( &pPam->GetNode() == pStart->GetNode(),
            "lcl_UpDown doesn't work for others." );

    const SwContentFrm *pCnt = 0;

    //We have to cheat a little bit during a table selection: Go to the
    //beginning of the cell while going up and go to the end of the cell while
    //going down.
    bool bTableSel = false;
    if ( pStart->IsInTab() &&
        pPam->GetNode().StartOfSectionNode() !=
        pPam->GetNode( false ).StartOfSectionNode() )
    {
        bTableSel = true;
        const SwLayoutFrm  *pCell = pStart->GetUpper();
        while ( !pCell->IsCellFrm() )
            pCell = pCell->GetUpper();

        // Check, if cell has a Prev/Follow cell:
        const bool bFwd = ( fnNxtPrv == lcl_GetNxtCnt );
        const SwLayoutFrm* pTmpCell = bFwd ?
            static_cast<const SwCellFrm*>(pCell)->GetFollowCell() :
            static_cast<const SwCellFrm*>(pCell)->GetPreviousCell();

        const SwContentFrm* pTmpStart = pStart;
        while ( pTmpCell && 0 != ( pTmpStart = pTmpCell->ContainsContent() ) )
        {
            pCell = pTmpCell;
            pTmpCell = bFwd ?
                static_cast<const SwCellFrm*>(pCell)->GetFollowCell() :
                static_cast<const SwCellFrm*>(pCell)->GetPreviousCell();
        }
        const SwContentFrm *pNxt = pCnt = pTmpStart;

        while ( pCell->IsAnLower( pNxt ) )
        {
            pCnt = pNxt;
            pNxt = (*fnNxtPrv)( pNxt );
        }
    }

    pCnt = (*fnNxtPrv)( pCnt ? pCnt : pStart );
    pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, true, bInReadOnly, bTableSel );

    const SwTabFrm *pStTab = pStart->FindTabFrm();
    const SwTabFrm *pTable = 0;
    const bool bTab = pStTab || (pCnt && pCnt->IsInTab());
    bool bEnd = !bTab;

    const SwFrm* pVertRefFrm = pStart;
    if ( bTableSel && pStTab )
        pVertRefFrm = pStTab;
    SWRECTFN( pVertRefFrm )

    SwTwips nX = 0;
    if ( bTab )
    {
        // pStart or pCnt is inside a table. nX will be used for travelling:
        SwRect aRect( pStart->Frm() );
        pStart->GetCharRect( aRect, *pPam->GetPoint() );
        Point aCenter = aRect.Center();
        nX = bVert ? aCenter.Y() : aCenter.X();

        pTable = pCnt ? pCnt->FindTabFrm() : 0;
        if ( !pTable )
            pTable = pStTab;

        if ( pStTab &&
            !pStTab->GetUpper()->IsInTab() &&
            !pTable->GetUpper()->IsInTab() )
        {
            const SwFrm *pCell = pStart->GetUpper();
            while ( pCell && !pCell->IsCellFrm() )
                pCell = pCell->GetUpper();
            OSL_ENSURE( pCell, "could not find the cell" );
            nX =  (pCell->Frm().*fnRect->fnGetLeft)() +
                  (pCell->Frm().*fnRect->fnGetWidth)() / 2;

            //The flow leads from one table to the next. The X-value needs to be
            //corrected based on the middle of the starting cell by the amount
            //of the offset of the tables.
            if ( pStTab != pTable )
            {
                nX += (pTable->Frm().*fnRect->fnGetLeft)() -
                      (pStTab->Frm().*fnRect->fnGetLeft)();
            }
        }

        // Restrict nX to the left and right borders of pTab:
        // (is this really necessary?)
        if (pTable && !pTable->GetUpper()->IsInTab())
        {
            const bool bRTL = pTable->IsRightToLeft();
            const long nPrtLeft = bRTL ?
                                (pTable->*fnRect->fnGetPrtRight)() :
                                (pTable->*fnRect->fnGetPrtLeft)();
            if ( bRTL != (nX < nPrtLeft) )
                nX = nPrtLeft;
            else
            {
                   const long nPrtRight = bRTL ?
                                    (pTable->*fnRect->fnGetPrtLeft)() :
                                    (pTable->*fnRect->fnGetPrtRight)();
                if ( bRTL != (nX > nPrtRight) )
                    nX = nPrtRight;
            }
        }
    }

    do
    {
        //If I'm in the DocumentBody, I wan't to stay there.
        if ( pStart->IsInDocBody() )
        {
            while ( pCnt && (!pCnt->IsInDocBody() ||
                             (pCnt->IsTextFrm() && static_cast<const SwTextFrm*>(pCnt)->IsHiddenNow())))
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, true, bInReadOnly, bTableSel );
            }
        }

        //If I'm in the FootNoteArea, I try to reach the next FootNoteArea in
        //case of necessity.
        else if ( pStart->IsInFootnote() )
        {
            while ( pCnt && (!pCnt->IsInFootnote() ||
                            (pCnt->IsTextFrm() && static_cast<const SwTextFrm*>(pCnt)->IsHiddenNow())))
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, true, bInReadOnly, bTableSel );
            }
        }

        //In Flys we can go ahead blindly as long as we find a Content.
        else if ( pStart->IsInFly() )
        {
            if ( pCnt && pCnt->IsTextFrm() && static_cast<const SwTextFrm*>(pCnt)->IsHiddenNow() )
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, true, bInReadOnly, bTableSel );
            }
        }

        //Otherwise I'll just refuse to leave to current area.
        else if ( pCnt )
        {
            const SwFrm *pUp = pStart->GetUpper();
            while (pUp && pUp->GetUpper() && !(pUp->GetType() & FRM_HEADFOOT))
                pUp = pUp->GetUpper();
            bool bSame = false;
            const SwFrm *pCntUp = pCnt->GetUpper();
            while ( pCntUp && !bSame )
            {
                if ( pUp == pCntUp )
                    bSame = true;
                else
                    pCntUp = pCntUp->GetUpper();
            }
            if ( !bSame )
                pCnt = 0;
            else if ( pCnt && pCnt->IsTextFrm() && static_cast<const SwTextFrm*>(pCnt)->IsHiddenNow() ) // i73332
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, true, bInReadOnly, bTableSel );
            }
        }

        if ( bTab )
        {
            if ( !pCnt )
                bEnd = true;
            else
            {
                const SwTabFrm *pTab = pCnt->FindTabFrm();
                if( !pTab )
                    bEnd = true;
                else
                {
                    if ( pTab != pTable )
                    {
                        //The flow leads from one table to the next. The X-value
                        //needs to be corrected by the amount of the offset of
                        //the tables
                         if ( pTable &&
                              !pTab->GetUpper()->IsInTab() &&
                            !pTable->GetUpper()->IsInTab() )
                            nX += pTab->Frm().Left() - pTable->Frm().Left();
                        pTable = pTab;
                    }
                    const SwLayoutFrm *pCell = pCnt->GetUpper();
                    while ( pCell && !pCell->IsCellFrm() )
                        pCell = pCell->GetUpper();

                    Point aInsideCell;
                    Point aInsideCnt;
                    if ( pCell )
                    {
                        long nTmpTop = (pCell->Frm().*fnRect->fnGetTop)();
                        if ( bVert )
                        {
                            if ( nTmpTop )
                                --nTmpTop;

                            aInsideCell = Point( nTmpTop, nX );
                        }
                        else
                            aInsideCell = Point( nX, nTmpTop );
                    }

                    long nTmpTop = (pCnt->Frm().*fnRect->fnGetTop)();
                    if ( bVert )
                    {
                        if ( nTmpTop )
                            --nTmpTop;

                        aInsideCnt = Point( nTmpTop, nX );
                    }
                    else
                        aInsideCnt = Point( nX, nTmpTop );

                    if ( pCell && pCell->Frm().IsInside( aInsideCell ) )
                    {
                        bEnd = true;
                        //Get the right Content out of the cell.
                        if ( !pCnt->Frm().IsInside( aInsideCnt ) )
                        {
                            pCnt = pCell->ContainsContent();
                            if ( fnNxtPrv == lcl_GetPrvCnt )
                                while ( pCell->IsAnLower(pCnt->GetNextContentFrm()) )
                                    pCnt = pCnt->GetNextContentFrm();
                        }
                    }
                    else if ( pCnt->Frm().IsInside( aInsideCnt ) )
                        bEnd = true;
                }
            }
            if ( !bEnd )
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, true, bInReadOnly, bTableSel );
            }
        }

    } while ( !bEnd ||
              (pCnt && pCnt->IsTextFrm() && static_cast<const SwTextFrm*>(pCnt)->IsHiddenNow()));

    if( pCnt )
    {   // set the Point on the Content-Node
        SwContentNode *pCNd = const_cast<SwContentNode*>(pCnt->GetNode());
        pPam->GetPoint()->nNode = *pCNd;
        if ( fnNxtPrv == lcl_GetPrvCnt )
            pCNd->MakeEndIndex( &pPam->GetPoint()->nContent );
        else
            pCNd->MakeStartIndex( &pPam->GetPoint()->nContent );
        return true;
    }
    return false;
}

bool SwContentFrm::UnitUp( SwPaM* pPam, const SwTwips, bool bInReadOnly ) const
{
    return ::lcl_UpDown( pPam, this, lcl_GetPrvCnt, bInReadOnly );
}

bool SwContentFrm::UnitDown( SwPaM* pPam, const SwTwips, bool bInReadOnly ) const
{
    return ::lcl_UpDown( pPam, this, lcl_GetNxtCnt, bInReadOnly );
}

/** Returns the number of the current page.
 *
 * If the method gets a PaM then the current page is the one in which the PaM sits. Otherwise the
 * current page is the first one inside the VisibleArea. We only work on available pages!
 */
sal_uInt16 SwRootFrm::GetCurrPage( const SwPaM *pActualCrsr ) const
{
    OSL_ENSURE( pActualCrsr, "got no page cursor" );
    SwFrm const*const pActFrm = pActualCrsr->GetPoint()->nNode.GetNode().
                                    GetContentNode()->getLayoutFrm( this, 0,
                                                    pActualCrsr->GetPoint(),
                                                    false );
    return pActFrm->FindPageFrm()->GetPhyPageNum();
}

/** Returns a PaM which sits at the beginning of the requested page.
 *
 * Formatting is done as far as necessary.
 * The PaM sits on the last page, if the page number was chosen to big.
 *
 * @return Null, if the operation was not possible.
 */
sal_uInt16 SwRootFrm::SetCurrPage( SwCursor* pToSet, sal_uInt16 nPageNum )
{
    vcl::RenderContext* pRenderContext = GetCurrShell() ? GetCurrShell()->GetOut() : 0;
    OSL_ENSURE( Lower() && Lower()->IsPageFrm(), "No page available." );

    SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
    bool bEnd =false;
    while ( !bEnd && pPage->GetPhyPageNum() != nPageNum )
    {   if ( pPage->GetNext() )
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
        else
        {   //Search the first ContentFrm and format until a new page is started
            //or until the ContentFrm are all done.
            const SwContentFrm *pContent = pPage->ContainsContent();
            while ( pContent && pPage->IsAnLower( pContent ) )
            {
                pContent->Calc(pRenderContext);
                pContent = pContent->GetNextContentFrm();
            }
            //Either this is a new page or we found the last page.
            if ( pPage->GetNext() )
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            else
                bEnd = true;
        }
    }
    //pPage now points to the 'requested' page. Now we have to create the PaM
    //on the beginning of the first ContentFrm in the body-text.
    //If this is a footnote-page, the PaM will be set in the first footnote.
    const SwContentFrm *pContent = pPage->ContainsContent();
    if ( pPage->IsFootnotePage() )
        while ( pContent && !pContent->IsInFootnote() )
            pContent = pContent->GetNextContentFrm();
    else
        while ( pContent && !pContent->IsInDocBody() )
            pContent = pContent->GetNextContentFrm();
    if ( pContent )
    {
        SwContentNode* pCNd = const_cast<SwContentNode*>(pContent->GetNode());
        pToSet->GetPoint()->nNode = *pCNd;
        pCNd->MakeStartIndex( &pToSet->GetPoint()->nContent );
        pToSet->GetPoint()->nContent = static_cast<const SwTextFrm*>(pContent)->GetOfst();

        SwShellCrsr* pSCrsr = dynamic_cast<SwShellCrsr*>(pToSet);
        if( pSCrsr )
        {
            Point &rPt = pSCrsr->GetPtPos();
            rPt = pContent->Frm().Pos();
            rPt += pContent->Prt().Pos();
        }
        return pPage->GetPhyPageNum();
    }
    return 0;
}

SwContentFrm *GetFirstSub( const SwLayoutFrm *pLayout )
{
    return const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(pLayout))->FindFirstBodyContent();
}

SwContentFrm *GetLastSub( const SwLayoutFrm *pLayout )
{
    return const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(pLayout))->FindLastBodyContent();
}

SwLayoutFrm *GetNextFrm( const SwLayoutFrm *pFrm )
{
    SwLayoutFrm *pNext =
        (pFrm->GetNext() && pFrm->GetNext()->IsLayoutFrm()) ?
                                            const_cast<SwLayoutFrm*>(static_cast<const SwLayoutFrm*>(pFrm->GetNext())) : 0;
    // #i39402# in case of an empty page
    if(pNext && !pNext->ContainsContent())
        pNext = (pNext->GetNext() && pNext->GetNext()->IsLayoutFrm()) ?
                                            static_cast<SwLayoutFrm*>(pNext->GetNext()) : 0;
    return pNext;
}

SwLayoutFrm *GetThisFrm( const SwLayoutFrm *pFrm )
{
    return const_cast<SwLayoutFrm*>(pFrm);
}

SwLayoutFrm *GetPrevFrm( const SwLayoutFrm *pFrm )
{
    SwLayoutFrm *pPrev =
        (pFrm->GetPrev() && pFrm->GetPrev()->IsLayoutFrm()) ?
                                            const_cast<SwLayoutFrm*>(static_cast<const SwLayoutFrm*>(pFrm->GetPrev())) : 0;
    // #i39402# in case of an empty page
    if(pPrev && !pPrev->ContainsContent())
        pPrev = (pPrev->GetPrev() && pPrev->GetPrev()->IsLayoutFrm()) ?
                                            static_cast<SwLayoutFrm*>(pPrev->GetPrev()) : 0;
    return pPrev;
}

//Now we can also initialize de function pointers;
//they are declared in cshtyp.hxx
SwPosPage fnPageStart = GetFirstSub;
SwPosPage fnPageEnd = GetLastSub;
SwWhichPage fnPagePrev = GetPrevFrm;
SwWhichPage fnPageCurr = GetThisFrm;
SwWhichPage fnPageNext = GetNextFrm;

/**
 * Returns the first/last Contentframe (controlled using the parameter fnPosPage)
 * of the current/previous/next page (controlled using the parameter fnWhichPage).
 */
bool GetFrmInPage( const SwContentFrm *pCnt, SwWhichPage fnWhichPage,
                   SwPosPage fnPosPage, SwPaM *pPam )
{
    //First find the requested page, at first the current, then the one which
    //was requests through fnWichPage.
    const SwLayoutFrm *pLayoutFrm = pCnt->FindPageFrm();
    if ( !pLayoutFrm || (0 == (pLayoutFrm = (*fnWhichPage)(pLayoutFrm))) )
        return false;

    //Now the desired ContentFrm below the page
    if( 0 == (pCnt = (*fnPosPage)(pLayoutFrm)) )
        return false;
    else
    {
        // repeated headlines in tables
        if ( pCnt->IsInTab() && fnPosPage == GetFirstSub )
        {
            const SwTabFrm* pTab = pCnt->FindTabFrm();
            if ( pTab->IsFollow() )
            {
                if ( pTab->IsInHeadline( *pCnt ) )
                {
                    SwLayoutFrm* pRow = pTab->GetFirstNonHeadlineRow();
                    if ( pRow )
                    {
                        // We are in the first line of a follow table
                        // with repeated headings.
                        // To actually make a "real" move we take the first content
                        // of the next row
                        pCnt = pRow->ContainsContent();
                        if ( ! pCnt )
                            return false;
                    }
                }
            }
        }

        SwContentNode *pCNd = const_cast<SwContentNode*>(pCnt->GetNode());
        pPam->GetPoint()->nNode = *pCNd;
        sal_Int32 nIdx;
        if( fnPosPage == GetFirstSub )
            nIdx = static_cast<const SwTextFrm*>(pCnt)->GetOfst();
        else
            nIdx = pCnt->GetFollow() ?
                    static_cast<const SwTextFrm*>(pCnt)->GetFollow()->GetOfst()-1 : pCNd->Len();
        pPam->GetPoint()->nContent.Assign( pCNd, nIdx );
        return true;
    }
}

static sal_uInt64 CalcDiff(const Point &rPt1, const Point &rPt2)
{
    //Calculate the distance between the two points.
    //'delta' X^2 + 'delta'Y^2 = 'distance'^2
    sal_uInt64 dX = std::max( rPt1.X(), rPt2.X() ) -
               std::min( rPt1.X(), rPt2.X() ),
          dY = std::max( rPt1.Y(), rPt2.Y() ) -
               std::min( rPt1.Y(), rPt2.Y() );
    return (dX * dX) + (dY * dY);
}

/** Check if the point lies inside the page part in which also the ContentFrame lies.
 *
 * In this context header, page body, footer and footnote-container count as page part.
 * This will suit the purpose that the ContentFrm which lies in the "right" page part will be
 * accepted instead of one which doesn't lie there although his distance to the point is shorter.
 */
static const SwLayoutFrm* lcl_Inside( const SwContentFrm *pCnt, Point& rPt )
{
    const SwLayoutFrm* pUp = pCnt->GetUpper();
    while( pUp )
    {
        if( pUp->IsPageBodyFrm() || pUp->IsFooterFrm() || pUp->IsHeaderFrm() )
        {
            if( rPt.Y() >= pUp->Frm().Top() && rPt.Y() <= pUp->Frm().Bottom() )
                return pUp;
            return NULL;
        }
        if( pUp->IsFootnoteContFrm() )
            return pUp->Frm().IsInside( rPt ) ? pUp : NULL;
        pUp = pUp->GetUpper();
    }
    return NULL;
}

/** Search for the nearest Content to pass.
 *
 * Considers the previous, the current and the next page.
 * If no content is found, the area gets expanded until one is found.
 *
 * @return The 'semantically correct' position inside the PrtArea of the found ContentFrm.
 */
const SwContentFrm *SwLayoutFrm::GetContentPos( Point& rPoint,
                                            const bool bDontLeave,
                                            const bool bBodyOnly,
                                            const bool bCalc,
                                            const SwCrsrMoveState *pCMS,
                                            const bool bDefaultExpand ) const
{
    vcl::RenderContext* pRenderContext = getRootFrm()->GetCurrShell()->GetOut();
    //Determine the first ContentFrm.
    const SwLayoutFrm *pStart = (!bDontLeave && bDefaultExpand && GetPrev()) ?
                                    static_cast<const SwLayoutFrm*>(GetPrev()) : this;
    const SwContentFrm *pContent = pStart->ContainsContent();

    if ( !pContent && (GetPrev() && !bDontLeave) )
        pContent = ContainsContent();

    if ( bBodyOnly && pContent && !pContent->IsInDocBody() )
        while ( pContent && !pContent->IsInDocBody() )
            pContent = pContent->GetNextContentFrm();

    const SwContentFrm *pActual= pContent;
    const SwLayoutFrm *pInside = NULL;
    sal_uInt16 nMaxPage = GetPhyPageNum() + (bDefaultExpand ? 1 : 0);
    Point aPoint = rPoint;
    sal_uInt64 nDistance = SAL_MAX_UINT64;

    while ( true )  //A loop to be sure we always find one.
    {
        while ( pContent &&
                ((!bDontLeave || IsAnLower( pContent )) &&
                (pContent->GetPhyPageNum() <= nMaxPage)) )
        {
            if ( ( bCalc || pContent->Frm().Width() ) &&
                 ( !bBodyOnly || pContent->IsInDocBody() ) )
            {
                //If the Content lies in a protected area (cell, Footnote, section),
                //we search the next Content which is not protected.
                const SwContentFrm *pComp = pContent;
                pContent = ::lcl_MissProtectedFrames( pContent, lcl_GetNxtCnt, false,
                                        pCMS && pCMS->m_bSetInReadOnly, false );
                if ( pComp != pContent )
                    continue;

                if ( !pContent->IsTextFrm() || !static_cast<const SwTextFrm*>(pContent)->IsHiddenNow() )
                {
                    if ( bCalc )
                        pContent->Calc(pRenderContext);

                    SwRect aCntFrm( pContent->UnionFrm() );
                    if ( aCntFrm.IsInside( rPoint ) )
                    {
                        pActual = pContent;
                        aPoint = rPoint;
                        break;
                    }
                    //The distance from rPoint to the nearest Point of pContent
                    //will now be calculated.
                    Point aContentPoint( rPoint );

                    //First set the vertical position
                    if ( aCntFrm.Top() > aContentPoint.Y() )
                        aContentPoint.Y() = aCntFrm.Top();
                    else if ( aCntFrm.Bottom() < aContentPoint.Y() )
                        aContentPoint.Y() = aCntFrm.Bottom();

                    //Now the horizontal position
                    if ( aCntFrm.Left() > aContentPoint.X() )
                        aContentPoint.X() = aCntFrm.Left();
                    else if ( aCntFrm.Right() < aContentPoint.X() )
                        aContentPoint.X() = aCntFrm.Right();

                    // pInside is a page area in which the point lies. As soon
                    // as pInside != 0 only frames are accepted which are
                    // placed inside.
                    if( !pInside || ( pInside->IsAnLower( pContent ) &&
                        ( !pContent->IsInFootnote() || pInside->IsFootnoteContFrm() ) ) )
                    {
                        const sal_uInt64 nDiff = ::CalcDiff(aContentPoint, rPoint);
                        bool bBetter = nDiff < nDistance;  // This one is nearer
                        if( !pInside )
                        {
                            pInside = lcl_Inside( pContent, rPoint );
                            if( pInside )  // In the "right" page area
                                bBetter = true;
                        }
                        if( bBetter )
                        {
                            aPoint = aContentPoint;
                            nDistance = nDiff;
                            pActual = pContent;
                        }
                    }
                }
            }
            pContent = pContent->GetNextContentFrm();
            if ( bBodyOnly )
                while ( pContent && !pContent->IsInDocBody() )
                    pContent = pContent->GetNextContentFrm();
        }
        if ( !pActual )
        {   //If we not yet found one we have to expand the searched
            //area, sometime we will find one!
            //MA 1997-01-09: Opt for many empty pages - if we only search inside
            //the body, we can expand the searched area sufficiently in one step.
            if ( bBodyOnly )
            {
                while ( !pContent && pStart->GetPrev() )
                {
                    ++nMaxPage;
                    if( !pStart->GetPrev()->IsLayoutFrm() )
                        return 0;
                    pStart = static_cast<const SwLayoutFrm*>(pStart->GetPrev());
                    pContent = pStart->IsInDocBody()
                                ? pStart->ContainsContent()
                                : pStart->FindPageFrm()->FindFirstBodyContent();
                }
                if ( !pContent )  // Somewhere down the road we have to start with one!
                {
                    pContent = pStart->FindPageFrm()->GetUpper()->ContainsContent();
                    while ( pContent && !pContent->IsInDocBody() )
                        pContent = pContent->GetNextContentFrm();
                    if ( !pContent )
                        return 0;   // There is no document content yet!
                }
            }
            else
            {
                ++nMaxPage;
                if ( pStart->GetPrev() )
                {
                    if( !pStart->GetPrev()->IsLayoutFrm() )
                        return 0;
                    pStart = static_cast<const SwLayoutFrm*>(pStart->GetPrev());
                    pContent = pStart->ContainsContent();
                }
                else // Somewhere down the road we have to start with one!
                    pContent = pStart->FindPageFrm()->GetUpper()->ContainsContent();
            }
            pActual = pContent;
        }
        else
            break;
    }

    OSL_ENSURE( pActual, "no Content found." );
    OSL_ENSURE( !bBodyOnly || pActual->IsInDocBody(), "Content not in Body." );

    //Special case for selecting tables not in repeated TableHeadlines.
    if ( pActual->IsInTab() && pCMS && pCMS->m_eState == MV_TBLSEL )
    {
        const SwTabFrm *pTab = pActual->FindTabFrm();
        if ( pTab->IsFollow() && pTab->IsInHeadline( *pActual ) )
        {
            const_cast<SwCrsrMoveState*>(pCMS)->m_bStop = true;
            return 0;
        }
    }

    //A small correction at the first/last
    Size aActualSize( pActual->Prt().SSize() );
    if ( aActualSize.Height() > pActual->GetUpper()->Prt().Height() )
        aActualSize.Height() = pActual->GetUpper()->Prt().Height();

    SWRECTFN( pActual )
    if ( !pActual->GetPrev() &&
         (*fnRect->fnYDiff)( (pActual->*fnRect->fnGetPrtTop)(),
                              bVert ? rPoint.X() : rPoint.Y() ) > 0 )
    {
        aPoint.Y() = pActual->Frm().Top() + pActual->Prt().Top();
        aPoint.X() = pActual->Frm().Left() +
                        ( pActual->IsRightToLeft() || bVert ?
                          pActual->Prt().Right() :
                          pActual->Prt().Left() );
    }
    else if ( !pActual->GetNext() &&
              (*fnRect->fnYDiff)( (pActual->*fnRect->fnGetPrtBottom)(),
                                   bVert ? rPoint.X() : rPoint.Y() ) < 0 )
    {
        aPoint.Y() = pActual->Frm().Top() + pActual->Prt().Bottom();
        aPoint.X() = pActual->Frm().Left() +
                        ( pActual->IsRightToLeft() || bVert ?
                          pActual->Prt().Left() :
                          pActual->Prt().Right() );
    }

    //Bring the Point in to the PrtArea
    if ( bCalc )
        pActual->Calc(pRenderContext);
    const SwRect aRect( pActual->Frm().Pos() + pActual->Prt().Pos(),
                        aActualSize );
    if ( aPoint.Y() < aRect.Top() )
        aPoint.Y() = aRect.Top();
    else if ( aPoint.Y() > aRect.Bottom() )
        aPoint.Y() = aRect.Bottom();
    if ( aPoint.X() < aRect.Left() )
        aPoint.X() = aRect.Left();
    else if ( aPoint.X() > aRect.Right() )
        aPoint.X() = aRect.Right();
    rPoint = aPoint;
    return pActual;
}

/** Same as SwLayoutFrm::GetContentPos(). Specialized for fields and border. */
void SwPageFrm::GetContentPosition( const Point &rPt, SwPosition &rPos ) const
{
    //Determine the first ContentFrm.
    const SwContentFrm *pContent = ContainsContent();
    if ( pContent )
    {
        //Look back one more (if possible).
        const SwContentFrm *pTmp = pContent->GetPrevContentFrm();
        while ( pTmp && !pTmp->IsInDocBody() )
            pTmp = pTmp->GetPrevContentFrm();
        if ( pTmp )
            pContent = pTmp;
    }
    else
        pContent = GetUpper()->ContainsContent();

    const SwContentFrm *pAct = pContent;
    Point aAct       = rPt;
    sal_uInt64 nDist = SAL_MAX_UINT64;

    while ( pContent )
    {
        SwRect aCntFrm( pContent->UnionFrm() );
        if ( aCntFrm.IsInside( rPt ) )
        {
            //This is the nearest one.
            pAct = pContent;
            break;
        }

        //Calculate the distance from rPt to the nearest point of pContent.
        Point aPoint( rPt );

        //Calculate the vertical position first
        if ( aCntFrm.Top() > rPt.Y() )
            aPoint.Y() = aCntFrm.Top();
        else if ( aCntFrm.Bottom() < rPt.Y() )
            aPoint.Y() = aCntFrm.Bottom();

        //And now the horizontal position
        if ( aCntFrm.Left() > rPt.X() )
            aPoint.X() = aCntFrm.Left();
        else if ( aCntFrm.Right() < rPt.X() )
            aPoint.X() = aCntFrm.Right();

        const sal_uInt64 nDiff = ::CalcDiff( aPoint, rPt );
        if ( nDiff < nDist )
        {
            aAct    = aPoint;
            nDist   = nDiff;
            pAct    = pContent;
        }
        else if ( aCntFrm.Top() > Frm().Bottom() )
            //In terms of fields, it's not possible to be closer any more!
            break;

        pContent = pContent->GetNextContentFrm();
        while ( pContent && !pContent->IsInDocBody() )
            pContent = pContent->GetNextContentFrm();
    }

    //Bring the point into the PrtArea.
    const SwRect aRect( pAct->Frm().Pos() + pAct->Prt().Pos(), pAct->Prt().SSize() );
    if ( aAct.Y() < aRect.Top() )
        aAct.Y() = aRect.Top();
    else if ( aAct.Y() > aRect.Bottom() )
        aAct.Y() = aRect.Bottom();
    if ( aAct.X() < aRect.Left() )
        aAct.X() = aRect.Left();
    else if ( aAct.X() > aRect.Right() )
        aAct.X() = aRect.Right();

    if( !pAct->IsValid() )
    {
        // ContentFrm not formatted -> always on node-beginning
        SwContentNode* pCNd = const_cast<SwContentNode*>(pAct->GetNode());
        OSL_ENSURE( pCNd, "Where is my ContentNode?" );
        rPos.nNode = *pCNd;
        rPos.nContent.Assign( pCNd, 0 );
    }
    else
    {
        SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
        pAct->GetCrsrOfst( &rPos, aAct, &aTmpState );
    }
}

// #123110# - helper class to disable creation of an action
// by a callback event - e.g., change event from a drawing object
class DisableCallbackAction
{
    private:
        SwRootFrm& mrRootFrm;
        bool mbOldCallbackActionState;

    public:
        explicit DisableCallbackAction( const SwRootFrm& _rRootFrm ) :
            mrRootFrm( const_cast<SwRootFrm&>(_rRootFrm) ),
            mbOldCallbackActionState( _rRootFrm.IsCallbackActionEnabled() )
        {
            mrRootFrm.SetCallbackActionEnabled( false );
        }

        ~DisableCallbackAction()
        {
            mrRootFrm.SetCallbackActionEnabled( mbOldCallbackActionState );
        }
};

/** Search the nearest Content to the passed point.
 *
 * Only search inside the BodyText.
 * @note Only the nearest vertically one will be searched.
 * @note JP 11.10.2001: only in tables we try to find the right column - Bug 72294
 */
Point SwRootFrm::GetNextPrevContentPos( const Point& rPoint, bool bNext ) const
{
    vcl::RenderContext* pRenderContext = GetCurrShell() ? GetCurrShell()->GetOut() : 0;
    // #123110# - disable creation of an action by a callback
    // event during processing of this method. Needed because formatting is
    // triggered by this method.
    DisableCallbackAction aDisableCallbackAction( *this );
    //Search the first ContentFrm and his successor in the body area.
    //To be efficient (and not formatting too much) we'll start at the correct
    //page.
    const SwLayoutFrm *pPage = static_cast<const SwLayoutFrm*>(Lower());
    if( pPage )
        while( pPage->GetNext() && pPage->Frm().Bottom() < rPoint.Y() )
            pPage = static_cast<const SwLayoutFrm*>(pPage->GetNext());

    const SwContentFrm *pCnt = pPage ? pPage->ContainsContent() : ContainsContent();
    while ( pCnt && !pCnt->IsInDocBody() )
        pCnt = pCnt->GetNextContentFrm();

    if ( !pCnt )
        return Point( 0, 0 );

    pCnt->Calc(pRenderContext);
    if( !bNext )
    {
        // As long as the point lies before the first ContentFrm and there are
        // still precedent pages I'll go to the next page.
        while ( rPoint.Y() < pCnt->Frm().Top() && pPage->GetPrev() )
        {
            pPage = static_cast<const SwLayoutFrm*>(pPage->GetPrev());
            pCnt = pPage->ContainsContent();
            while ( !pCnt )
            {
                pPage = static_cast<const SwLayoutFrm*>(pPage->GetPrev());
                if ( pPage )
                    pCnt = pPage->ContainsContent();
                else
                    return ContainsContent()->UnionFrm().Pos();
            }
            pCnt->Calc(pRenderContext);
        }
    }

    //Does the point lie above the first ContentFrm?
    if ( rPoint.Y() < pCnt->Frm().Top() && !lcl_IsInRepeatedHeadline( pCnt ) )
        return pCnt->UnionFrm().Pos();

    Point aRet(0, 0);
    do
    {
        //Does the point lie in the current ContentFrm?
        SwRect aCntFrm( pCnt->UnionFrm() );
        if ( aCntFrm.IsInside( rPoint ) && !lcl_IsInRepeatedHeadline( pCnt ))
        {
            aRet = rPoint;
            break;
        }

        //Is the current one the last ContentFrm?
        //If the next ContentFrm lies behind the point, then the current on is the
        //one we searched.
        const SwContentFrm *pNxt = pCnt->GetNextContentFrm();
        while ( pNxt && !pNxt->IsInDocBody() )
            pNxt = pNxt->GetNextContentFrm();

        //Does the point lie behind the last ContentFrm?
        if ( !pNxt )
        {
            aRet = Point( aCntFrm.Right(), aCntFrm.Bottom() );
            break;
        }

        //If the next ContentFrm lies behind the point then it is the one we
        //searched.
        const SwTabFrm* pTFrm;
        pNxt->Calc(pRenderContext);
        if( pNxt->Frm().Top() > rPoint.Y() &&
            !lcl_IsInRepeatedHeadline( pCnt, &pTFrm ) &&
            ( !pTFrm || pNxt->Frm().Left() > rPoint.X() ))
        {
            if (bNext)
                aRet = pNxt->Frm().Pos();
            else
                aRet = Point( aCntFrm.Right(), aCntFrm.Bottom() );
            break;
        }
        pCnt = pNxt;
    }
    while (pCnt);
    return aRet;
}

/** Returns the absolute document position of the desired page.
 *
 * Formatting is done only as far as needed and only if bFormat=true.
 * Pos is set to the one of the last page, if the page number was chosen to big.
 *
 * @return Null, if the operation failed.
 */
Point SwRootFrm::GetPagePos( sal_uInt16 nPageNum ) const
{
    OSL_ENSURE( Lower() && Lower()->IsPageFrm(), "No page available." );

    const SwPageFrm *pPage = static_cast<const SwPageFrm*>(Lower());
    while ( true )
    {
        if ( pPage->GetPhyPageNum() >= nPageNum || !pPage->GetNext() )
            break;
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
    }
    return pPage->Frm().Pos();
}

/** get page frame by phyiscal page number
 *
 * @return pointer to the page frame with the given physical page number
 */
SwPageFrm* SwRootFrm::GetPageByPageNum( sal_uInt16 _nPageNum ) const
{
    const SwPageFrm* pPageFrm = static_cast<const SwPageFrm*>( Lower() );
    while ( pPageFrm && pPageFrm->GetPhyPageNum() < _nPageNum )
    {
          pPageFrm = static_cast<const SwPageFrm*>( pPageFrm->GetNext() );
    }

    if ( pPageFrm && pPageFrm->GetPhyPageNum() == _nPageNum )
    {
        return const_cast<SwPageFrm*>( pPageFrm );
    }
    else
    {
        return 0;
    }
}

/**
 * @return true, when the given physical pagenumber does't exist or this page is an empty page.
 */
bool SwRootFrm::IsDummyPage( sal_uInt16 nPageNum ) const
{
    if( !Lower() || !nPageNum || nPageNum > GetPageNum() )
        return true;

    const SwPageFrm *pPage = static_cast<const SwPageFrm*>(Lower());
    while( pPage && nPageNum < pPage->GetPhyPageNum() )
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
    return !pPage || pPage->IsEmptyPage();
}

/** Is the Frm or rather the Section in which it lies protected?
 *
 * Also Fly in Fly in ... and Footnotes
 */
bool SwFrm::IsProtected() const
{
    if (this->IsContentFrm() && static_cast<const SwContentFrm*>(this)->GetNode())
    {
        const SwDoc *pDoc=static_cast<const SwContentFrm*>(this)->GetNode()->GetDoc();
        bool isFormProtected=pDoc->GetDocumentSettingManager().get(DocumentSettingId::PROTECT_FORM );
        if (isFormProtected)
        {
            return false; // TODO a hack for now, well deal with it later, I we return true here we have a "double" locking
        }
    }
    //The Frm can be protected in borders, cells or sections.
    //Also goes up FlyFrms recursive and from footnote to anchor.
    const SwFrm *pFrm = this;
    do
    {
        if ( pFrm->IsContentFrm() )
        {
            if ( static_cast<const SwContentFrm*>(pFrm)->GetNode() &&
                 static_cast<const SwContentFrm*>(pFrm)->GetNode()->IsInProtectSect() )
                return true;
        }
        else
        {
            if ( static_cast<const SwLayoutFrm*>(pFrm)->GetFormat() &&
                 static_cast<const SwLayoutFrm*>(pFrm)->GetFormat()->
                 GetProtect().IsContentProtected() )
                return true;
            if ( pFrm->IsCoveredCell() )
                return true;
        }
        if ( pFrm->IsFlyFrm() )
        {
            //In a chain the protection of the content can be specified by the
            //master of the chain.
            if ( static_cast<const SwFlyFrm*>(pFrm)->GetPrevLink() )
            {
                const SwFlyFrm *pMaster = static_cast<const SwFlyFrm*>(pFrm);
                do
                {   pMaster = pMaster->GetPrevLink();
                } while ( pMaster->GetPrevLink() );
                if ( pMaster->IsProtected() )
                    return true;
            }
            pFrm = static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm();
        }
        else if ( pFrm->IsFootnoteFrm() )
            pFrm = static_cast<const SwFootnoteFrm*>(pFrm)->GetRef();
        else
            pFrm = pFrm->GetUpper();

    } while ( pFrm );

    return false;
}

/** @return the physical page number */
sal_uInt16 SwFrm::GetPhyPageNum() const
{
    const SwPageFrm *pPage = FindPageFrm();
    return pPage ? pPage->GetPhyPageNum() : 0;
}

/** Decides if the page want to be a rightpage or not.
 *
 * If the first content of the page has a page descriptor, we take the follow
 * of the page descriptor of the last not empty page. If this descriptor allows
 * only right(left) pages and the page isn't an empty page then it want to be
 * such right(left) page. If the descriptor allows right and left pages, we
 * look for a number offset in the first content. If there is one, odd number
 * results right pages, even number results left pages.
 * If there is no number offset, we take the physical page number instead,
 * but a previous empty page don't count.
 */
bool SwFrm::WannaRightPage() const
{
    const SwPageFrm *pPage = FindPageFrm();
    if ( !pPage || !pPage->GetUpper() )
        return true;

    const SwFrm *pFlow = pPage->FindFirstBodyContent();
    const SwPageDesc *pDesc = 0;
    ::boost::optional<sal_uInt16> oPgNum;
    if ( pFlow )
    {
        if ( pFlow->IsInTab() )
            pFlow = pFlow->FindTabFrm();
        const SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pFlow );
        if ( !pTmp->IsFollow() )
        {
            const SwFormatPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            pDesc = rPgDesc.GetPageDesc();
            oPgNum = rPgDesc.GetNumOffset();
        }
    }
    if ( !pDesc )
    {
        SwPageFrm *pPrv = const_cast<SwPageFrm*>(static_cast<const SwPageFrm*>(pPage->GetPrev()));
        if( pPrv && pPrv->IsEmptyPage() )
            pPrv = static_cast<SwPageFrm*>(pPrv->GetPrev());
        if( pPrv )
            pDesc = pPrv->GetPageDesc()->GetFollow();
        else
        {
            const SwDoc* pDoc = pPage->GetFormat()->GetDoc();
            pDesc = &pDoc->GetPageDesc( 0 );
        }
    }
    OSL_ENSURE( pDesc, "No pagedescriptor" );
    bool bOdd;
    if( oPgNum )
        bOdd = (oPgNum.get() % 2) != 0;
    else
    {
        bOdd = pPage->OnRightPage();
        if( pPage->GetPrev() && static_cast<const SwPageFrm*>(pPage->GetPrev())->IsEmptyPage() )
            bOdd = !bOdd;
    }
    if( !pPage->IsEmptyPage() )
    {
        if( !pDesc->GetRightFormat() )
            bOdd = false;
        else if( !pDesc->GetLeftFormat() )
            bOdd = true;
    }
    return bOdd;
}

bool SwFrm::OnFirstPage() const
{
    bool bRet = false;
    const SwPageFrm *pPage = FindPageFrm();

    if (pPage)
    {
        const SwPageFrm* pPrevFrm = dynamic_cast<const SwPageFrm*>(pPage->GetPrev());
        if (pPrevFrm)
        {
            const SwPageDesc* pDesc = pPage->GetPageDesc();
            bRet = pPrevFrm->GetPageDesc() != pDesc;
        }
        else
            bRet = true;
    }
    return bRet;
}

void SwFrm::Calc(vcl::RenderContext* pRenderContext) const
{
    if ( !mbValidPos || !mbValidPrtArea || !mbValidSize )
        const_cast<SwFrm*>(this)->PrepareMake(pRenderContext);
}

Point SwFrm::GetRelPos() const
{
    Point aRet( maFrm.Pos() );
    // here we cast since SwLayoutFrm is declared only as forwarded
    aRet -= GetUpper()->Prt().Pos();
    aRet -= GetUpper()->Frm().Pos();
    return aRet;
}

/** @return the virtual page number with the offset. */
sal_uInt16 SwFrm::GetVirtPageNum() const
{
    const SwPageFrm *pPage = FindPageFrm();
    if ( !pPage || !pPage->GetUpper() )
        return 0;

    sal_uInt16 nPhyPage = pPage->GetPhyPageNum();
    if ( !(static_cast<const SwRootFrm*>(pPage->GetUpper()))->IsVirtPageNum() )
        return nPhyPage;

    //Search the nearest section using the virtual page number.
    //Because searching backwards needs a lot of time we search specific using
    //the dependencies. From the PageDescs we get the attributes and from the
    //attributes we get the sections.
    const SwPageFrm *pVirtPage = 0;
    const SwFrm *pFrm = 0;
    const SfxItemPool &rPool = pPage->GetFormat()->GetDoc()->GetAttrPool();
    sal_uInt32 nMaxItems = rPool.GetItemCount2( RES_PAGEDESC );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        const SfxPoolItem* pItem = rPool.GetItem2( RES_PAGEDESC, n );
        if ( 0 == pItem )
            continue;

        const SwFormatPageDesc *pDesc = static_cast<const SwFormatPageDesc*>(pItem);
        if ( pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
        {
            const SwModify *pMod = pDesc->GetDefinedIn();
            SwVirtPageNumInfo aInfo( pPage );
            pMod->GetInfo( aInfo );
            if ( aInfo.GetPage() )
            {
                if( !pVirtPage || aInfo.GetPage()->GetPhyPageNum() > pVirtPage->GetPhyPageNum() )
                {
                    pVirtPage = aInfo.GetPage();
                    pFrm = aInfo.GetFrm();
                }
            }
        }
    }
    if ( pFrm )
    {
        ::boost::optional<sal_uInt16> oNumOffset = pFrm->GetAttrSet()->GetPageDesc().GetNumOffset();
        if (oNumOffset)
        {
            return nPhyPage - pFrm->GetPhyPageNum() + oNumOffset.get();
        }
        else
        {
            return nPhyPage - pFrm->GetPhyPageNum();
        }
    }
    return nPhyPage;
}

/** Determines and sets those cells which are enclosed by the selection. */
bool SwRootFrm::MakeTableCrsrs( SwTableCursor& rTableCrsr )
{
    //Find Union-Rects and tables (Follows) of the selection.
    OSL_ENSURE( rTableCrsr.GetContentNode() && rTableCrsr.GetContentNode( false ),
            "Tabselection nicht auf Cnt." );

    bool bRet = false;

    // For new table models there's no need to ask the layout..
    if( rTableCrsr.NewTableSelection() )
        return true;

    Point aPtPt, aMkPt;
    {
        SwShellCrsr* pShCrsr = dynamic_cast<SwShellCrsr*>(&rTableCrsr);

        if( pShCrsr )
        {
            aPtPt = pShCrsr->GetPtPos();
            aMkPt = pShCrsr->GetMkPos();
        }
    }

    // #151012# Made code robust here
    const SwContentNode* pTmpStartNode = rTableCrsr.GetContentNode();
    const SwContentNode* pTmpEndNode   = rTableCrsr.GetContentNode(false);

    const SwFrm* pTmpStartFrm = pTmpStartNode ? pTmpStartNode->getLayoutFrm( this, &aPtPt, 0, false ) : 0;
    const SwFrm* pTmpEndFrm   = pTmpEndNode   ?   pTmpEndNode->getLayoutFrm( this, &aMkPt, 0, false ) : 0;

    const SwLayoutFrm* pStart = pTmpStartFrm ? pTmpStartFrm->GetUpper() : 0;
    const SwLayoutFrm* pEnd   = pTmpEndFrm   ? pTmpEndFrm->GetUpper() : 0;

    OSL_ENSURE( pStart && pEnd, "MakeTableCrsrs: Good to have the code robust here!" );

    /* #109590# Only change table boxes if the frames are
        valid. Needed because otherwise the table cursor after moving
        table cells by dnd resulted in an empty tables cursor.  */
    if ( pStart && pEnd && pStart->IsValid() && pEnd->IsValid())
    {
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd );

        SwSelBoxes aNew;

        const bool bReadOnlyAvailable = rTableCrsr.IsReadOnlyAvailable();

        for ( size_t i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            // Skip any repeated headlines in the follow:
            SwLayoutFrm* pRow = pTable->IsFollow() ?
                                pTable->GetFirstNonHeadlineRow() :
                                const_cast<SwLayoutFrm*>(static_cast<const SwLayoutFrm*>(pTable->Lower()));

            while ( pRow )
            {
                if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while ( pCell && pRow->IsAnLower( pCell ) )
                    {
                        OSL_ENSURE( pCell->IsCellFrm(), "Frame without cell" );
                        if( IsFrmInTableSel( pUnion->GetUnion(), pCell ) &&
                            (bReadOnlyAvailable ||
                             !pCell->GetFormat()->GetProtect().IsContentProtected()))
                        {
                            SwTableBox* pInsBox = const_cast<SwTableBox*>(
                                static_cast<const SwCellFrm*>(pCell)->GetTabBox());
                            aNew.insert( pInsBox );
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = static_cast<const SwLayoutFrm*>(pCell->GetNext());
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                        {
                            const SwLayoutFrm* pLastCell = pCell;
                            do
                            {
                                pCell = pCell->GetNextLayoutLeaf();
                            } while ( pCell && pLastCell->IsAnLower( pCell ) );
                            // For sections with columns
                            if( pCell && pCell->IsInTab() )
                            {
                                while( !pCell->IsCellFrm() )
                                {
                                    pCell = pCell->GetUpper();
                                    OSL_ENSURE( pCell, "Where's my cell?" );
                                }
                            }
                        }
                    }
                }
                pRow = static_cast<SwLayoutFrm*>(pRow->GetNext());
            }
        }

        rTableCrsr.ActualizeSelection( aNew );
        bRet = true;
    }

    return bRet;
}

inline void Sub( SwRegionRects& rRegion, const SwRect& rRect )
{
    if( rRect.Width() > 1 && rRect.Height() > 1 &&
        rRect.IsOver( rRegion.GetOrigin() ))
        rRegion -= rRect;
}

inline void Add( SwRegionRects& rRegion, const SwRect& rRect )
{
    if( rRect.Width() > 1 && rRect.Height() > 1 )
        rRegion += rRect;
}

/*
 * The following situations can happen:
 *  1. Start and end lie in one screen-row and in the same node
 *     -> one rectangle out of start and end; and we're okay
 *  2. Start and end lie in one frame (therefore in the same node!)
 *     -> expand start to the right, end to the left and if more than two
 *        screen-rows are involved - calculate the in-between
 *  3. Start and end lie in different frames
 *     -> expand start to the right until frame-end, calculate Rect
 *        expand end to the left until frame-start, calculate Rect
 *        and if more than two frames are involved add the PrtArea of all
 *        frames which lie in between
 *
 * Big reorganization because of the FlyFrm - those need to be locked out.
 * Exceptions:  - The Fly in which the selection took place (if it took place
 *                 in a Fly)
 *              - The Flys which are underrun by the text
 *              - The Flys which are anchored to somewhere inside the selection.
 * Functioning: First a SwRegion with a root gets initialized.
 *              Out of the region the inverted sections are cut out. The
 *              section gets compressed and finally inverted and thereby the
 *              inverted rectangles are available.
 *              In the end the Flys are cut out of the section.
 */
void SwRootFrm::CalcFrmRects(SwShellCrsr &rCrsr)
{
    SwPosition *pStartPos = rCrsr.Start(),
               *pEndPos   = rCrsr.GetPoint() == pStartPos ? rCrsr.GetMark() : rCrsr.GetPoint();

    SwViewShell *pSh = GetCurrShell();

    bool bIgnoreVisArea = true;
    if (pSh)
        bIgnoreVisArea = pSh->GetViewOptions()->IsPDFExport() || pSh->isTiledRendering();

    // #i12836# enhanced pdf
    SwRegionRects aRegion( !bIgnoreVisArea ?
                           pSh->VisArea() :
                           Frm() );
    if( !pStartPos->nNode.GetNode().IsContentNode() ||
        !pStartPos->nNode.GetNode().GetContentNode()->getLayoutFrm(this) ||
        ( pStartPos->nNode != pEndPos->nNode &&
          ( !pEndPos->nNode.GetNode().IsContentNode() ||
            !pEndPos->nNode.GetNode().GetContentNode()->getLayoutFrm(this) ) ) )
    {
        return;
    }

    //First obtain the ContentFrms for the start and the end - those are needed
    //anyway.
    SwContentFrm const* pStartFrm = pStartPos->nNode.GetNode().
        GetContentNode()->getLayoutFrm( this, &rCrsr.GetSttPos(), pStartPos );

    SwContentFrm const* pEndFrm   = pEndPos->nNode.GetNode().
        GetContentNode()->getLayoutFrm( this, &rCrsr.GetEndPos(), pEndPos );

    OSL_ENSURE( (pStartFrm && pEndFrm), "Keine ContentFrms gefunden." );

    //Do not subtract the FlyFrms in which selected Frames lie.
    SwSortedObjs aSortObjs;
    if ( pStartFrm->IsInFly() )
    {
        const SwAnchoredObject* pObj = pStartFrm->FindFlyFrm();
        OSL_ENSURE( pObj, "No Start Object." );
        if (pObj) aSortObjs.Insert( *(const_cast<SwAnchoredObject*>(pObj)) );
        const SwAnchoredObject* pObj2 = pEndFrm->FindFlyFrm();
        OSL_ENSURE( pObj2, "SwRootFrm::CalcFrmRects(..) - FlyFrame missing - looks like an invalid selection" );
        if ( pObj2 != NULL && pObj2 != pObj )
        {
            aSortObjs.Insert( *(const_cast<SwAnchoredObject*>(pObj2)) );
        }
    }

    // if a selection which is not allowed exists, we correct what is not
    // allowed (header/footer/table-headline) for two pages.
    do {    // middle check loop
        const SwLayoutFrm* pSttLFrm = pStartFrm->GetUpper();
        const sal_uInt16 cHdFtTableHd = FRM_HEADER | FRM_FOOTER | FRM_TAB;
        while( pSttLFrm &&
            ! (cHdFtTableHd & pSttLFrm->GetType() ))
            pSttLFrm = pSttLFrm->GetUpper();
        if( !pSttLFrm )
            break;
        const SwLayoutFrm* pEndLFrm = pEndFrm->GetUpper();
        while( pEndLFrm &&
            ! (cHdFtTableHd & pEndLFrm->GetType() ))
            pEndLFrm = pEndLFrm->GetUpper();
        if( !pEndLFrm )
            break;

        OSL_ENSURE( pEndLFrm->GetType() == pSttLFrm->GetType(),
            "Selection over different content" );
        switch( pSttLFrm->GetType() )
        {
        case FRM_HEADER:
        case FRM_FOOTER:
            // On different pages? Then always on the start-page
            if( pEndLFrm->FindPageFrm() != pSttLFrm->FindPageFrm() )
            {
                // Set end- to the start-ContentFrame
                if( pStartPos == rCrsr.GetPoint() )
                    pEndFrm = pStartFrm;
                else
                    pStartFrm = pEndFrm;
            }
            break;
        case FRM_TAB:
            // On different pages? Then check for table-headline
            {
                const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>(pSttLFrm);
                if( ( pTabFrm->GetFollow() ||
                    static_cast<const SwTabFrm*>(pEndLFrm)->GetFollow() ) &&
                    pTabFrm->GetTable()->GetRowsToRepeat() > 0 &&
                    pTabFrm->GetLower() != static_cast<const SwTabFrm*>(pEndLFrm)->GetLower() &&
                    ( lcl_IsInRepeatedHeadline( pStartFrm ) ||
                    lcl_IsInRepeatedHeadline( pEndFrm ) ) )
                {
                    // Set end- to the start-ContentFrame
                    if( pStartPos == rCrsr.GetPoint() )
                        pEndFrm = pStartFrm;
                    else
                        pStartFrm = pEndFrm;
                }
            }
            break;
        }
    } while( false );

    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.m_b2Lines = true;
    aTmpState.m_bNoScroll = true;
    aTmpState.m_nCursorBidiLevel = pStartFrm->IsRightToLeft() ? 1 : 0;

    //ContentRects to Start- and EndFrms.
    SwRect aStRect, aEndRect;
    pStartFrm->GetCharRect( aStRect, *pStartPos, &aTmpState );
    Sw2LinesPos *pSt2Pos = aTmpState.m_p2Lines;
    aTmpState.m_p2Lines = NULL;
    aTmpState.m_nCursorBidiLevel = pEndFrm->IsRightToLeft() ? 1 : 0;

    pEndFrm->GetCharRect( aEndRect, *pEndPos, &aTmpState );
    Sw2LinesPos *pEnd2Pos = aTmpState.m_p2Lines;

    SwRect aStFrm ( pStartFrm->UnionFrm( true ) );
    aStFrm.Intersection( pStartFrm->PaintArea() );
    SwRect aEndFrm( pStartFrm == pEndFrm ? aStFrm : pEndFrm->UnionFrm( true ) );
    if( pStartFrm != pEndFrm )
    {
        aEndFrm.Intersection( pEndFrm->PaintArea() );
    }
    SWRECTFN( pStartFrm )
    const bool bR2L = pStartFrm->IsRightToLeft();
    const bool bEndR2L = pEndFrm->IsRightToLeft();

    // If there's no doubleline portion involved or start and end are both
    // in the same doubleline portion, all works fine, but otherwise
    // we need the following...
    if( pSt2Pos != pEnd2Pos && ( !pSt2Pos || !pEnd2Pos ||
        pSt2Pos->aPortion != pEnd2Pos->aPortion ) )
    {
        // If we have a start(end) position inside a doubleline portion
        // the surrounded part of the doubleline portion is subtracted
        // from the region and the aStRect(aEndRect) is set to the
        // end(start) of the doubleline portion.
        if( pSt2Pos )
        {
            SwRect aTmp( aStRect );

            // BiDi-Portions are swimming against the current.
            const bool bPorR2L = ( MultiPortionType::BIDI == pSt2Pos->nMultiType ) ?
                ! bR2L :
            bR2L;

            if( MultiPortionType::BIDI == pSt2Pos->nMultiType &&
                (pSt2Pos->aPortion2.*fnRect->fnGetWidth)() )
            {
                // nested bidi portion
                long nRightAbs = (pSt2Pos->aPortion.*fnRect->fnGetRight)();
                nRightAbs -= (pSt2Pos->aPortion2.*fnRect->fnGetLeft)();
                long nLeftAbs = nRightAbs - (pSt2Pos->aPortion2.*fnRect->fnGetWidth)();

                (aTmp.*fnRect->fnSetRight)( nRightAbs );

                if ( ! pEnd2Pos || pEnd2Pos->aPortion != pSt2Pos->aPortion )
                {
                    SwRect aTmp2( pSt2Pos->aPortion );
                    (aTmp2.*fnRect->fnSetRight)( nLeftAbs );
                    aTmp2.Intersection( aEndFrm );
                    Sub( aRegion, aTmp2 );
                }
            }
            else
            {
                if( bPorR2L )
                    (aTmp.*fnRect->fnSetLeft)(
                        (pSt2Pos->aPortion.*fnRect->fnGetLeft)() );
                else
                    (aTmp.*fnRect->fnSetRight)(
                        (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
            }

            if( MultiPortionType::ROT_90 == pSt2Pos->nMultiType ||
                (pSt2Pos->aPortion.*fnRect->fnGetTop)() ==
                (aTmp.*fnRect->fnGetTop)() )
            {
                (aTmp.*fnRect->fnSetTop)(
                    (pSt2Pos->aLine.*fnRect->fnGetTop)() );
            }

            aTmp.Intersection( aStFrm );
            Sub( aRegion, aTmp );

            SwTwips nTmp = (pSt2Pos->aLine.*fnRect->fnGetBottom)();
            if( MultiPortionType::ROT_90 != pSt2Pos->nMultiType &&
                (aStRect.*fnRect->fnBottomDist)( nTmp ) > 0 )
            {
                (aTmp.*fnRect->fnSetTop)( (aTmp.*fnRect->fnGetBottom)() );
                (aTmp.*fnRect->fnSetBottom)( nTmp );
                if( (aStRect.*fnRect->fnBottomDist)(
                    (pSt2Pos->aPortion.*fnRect->fnGetBottom)() ) > 0 )
                {
                    if( bPorR2L )
                        (aTmp.*fnRect->fnSetRight)(
                            (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
                    else
                        (aTmp.*fnRect->fnSetLeft)(
                            (pSt2Pos->aPortion.*fnRect->fnGetLeft)() );
                }
                aTmp.Intersection( aStFrm );
                Sub( aRegion, aTmp );
            }

            aStRect = pSt2Pos->aLine;
            (aStRect.*fnRect->fnSetLeft)( bR2L ?
                    (pSt2Pos->aPortion.*fnRect->fnGetLeft)() :
                    (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
            (aStRect.*fnRect->fnSetWidth)( 1 );
        }

        if( pEnd2Pos )
        {
            SWRECTFNX( pEndFrm )
            SwRect aTmp( aEndRect );

            // BiDi-Portions are swimming against the current.
            const bool bPorR2L = ( MultiPortionType::BIDI == pEnd2Pos->nMultiType ) ?
                                       ! bEndR2L :
                                         bEndR2L;

            if( MultiPortionType::BIDI == pEnd2Pos->nMultiType &&
                (pEnd2Pos->aPortion2.*fnRectX->fnGetWidth)() )
            {
                // nested bidi portion
                long nRightAbs = (pEnd2Pos->aPortion.*fnRectX->fnGetRight)();
                nRightAbs = nRightAbs - (pEnd2Pos->aPortion2.*fnRectX->fnGetLeft)();
                long nLeftAbs = nRightAbs - (pEnd2Pos->aPortion2.*fnRectX->fnGetWidth)();

                (aTmp.*fnRectX->fnSetLeft)( nLeftAbs );

                if ( ! pSt2Pos || pSt2Pos->aPortion != pEnd2Pos->aPortion )
                {
                    SwRect aTmp2( pEnd2Pos->aPortion );
                    (aTmp2.*fnRectX->fnSetLeft)( nRightAbs );
                    aTmp2.Intersection( aEndFrm );
                    Sub( aRegion, aTmp2 );
                }
            }
            else
            {
                if ( bPorR2L )
                    (aTmp.*fnRectX->fnSetRight)(
                        (pEnd2Pos->aPortion.*fnRectX->fnGetRight)() );
                else
                    (aTmp.*fnRectX->fnSetLeft)(
                        (pEnd2Pos->aPortion.*fnRectX->fnGetLeft)() );
            }

            if( MultiPortionType::ROT_90 == pEnd2Pos->nMultiType ||
                (pEnd2Pos->aPortion.*fnRectX->fnGetBottom)() ==
                (aEndRect.*fnRectX->fnGetBottom)() )
            {
                (aTmp.*fnRectX->fnSetBottom)(
                    (pEnd2Pos->aLine.*fnRectX->fnGetBottom)() );
            }

            aTmp.Intersection( aEndFrm );
            Sub( aRegion, aTmp );

            // The next statement means neither ruby nor rotate(90):
            if( !( MultiPortionType::RUBY == pEnd2Pos->nMultiType ) )
            {
                SwTwips nTmp = (pEnd2Pos->aLine.*fnRectX->fnGetTop)();
                if( (aEndRect.*fnRectX->fnGetTop)() != nTmp )
                {
                    (aTmp.*fnRectX->fnSetBottom)(
                        (aTmp.*fnRectX->fnGetTop)() );
                    (aTmp.*fnRectX->fnSetTop)( nTmp );
                    if( (aEndRect.*fnRectX->fnGetTop)() !=
                        (pEnd2Pos->aPortion.*fnRectX->fnGetTop)() )
                    {
                        if( bPorR2L )
                            (aTmp.*fnRectX->fnSetLeft)(
                                (pEnd2Pos->aPortion.*fnRectX->fnGetLeft)() );
                        else
                            (aTmp.*fnRectX->fnSetRight)(
                                (pEnd2Pos->aPortion.*fnRectX->fnGetRight)() );
                    }
                    aTmp.Intersection( aEndFrm );
                    Sub( aRegion, aTmp );
                }
            }

            aEndRect = pEnd2Pos->aLine;
            (aEndRect.*fnRectX->fnSetLeft)( bEndR2L ?
                    (pEnd2Pos->aPortion.*fnRectX->fnGetRight)() :
                    (pEnd2Pos->aPortion.*fnRectX->fnGetLeft)() );
            (aEndRect.*fnRectX->fnSetWidth)( 1 );
        }
    }
    else if( pSt2Pos && pEnd2Pos &&
             MultiPortionType::BIDI == pSt2Pos->nMultiType &&
             MultiPortionType::BIDI == pEnd2Pos->nMultiType &&
             pSt2Pos->aPortion == pEnd2Pos->aPortion &&
             pSt2Pos->aPortion2 != pEnd2Pos->aPortion2 )
    {
        // This is the ugly special case, where the selection starts and
        // ends in the same bidi portion but one start or end is inside a
        // nested bidi portion.

        if ( (pSt2Pos->aPortion2.*fnRect->fnGetWidth)() )
        {
            SwRect aTmp( aStRect );
            long nRightAbs = (pSt2Pos->aPortion.*fnRect->fnGetRight)();
            nRightAbs -= (pSt2Pos->aPortion2.*fnRect->fnGetLeft)();
            long nLeftAbs = nRightAbs - (pSt2Pos->aPortion2.*fnRect->fnGetWidth)();

            (aTmp.*fnRect->fnSetRight)( nRightAbs );
            aTmp.Intersection( aStFrm );
            Sub( aRegion, aTmp );

            aStRect = pSt2Pos->aLine;
            (aStRect.*fnRect->fnSetLeft)( bR2L ? nRightAbs : nLeftAbs );
            (aStRect.*fnRect->fnSetWidth)( 1 );
        }

        SWRECTFNX( pEndFrm )
        if ( (pEnd2Pos->aPortion2.*fnRectX->fnGetWidth)() )
        {
            SwRect aTmp( aEndRect );
            long nRightAbs = (pEnd2Pos->aPortion.*fnRectX->fnGetRight)();
            nRightAbs -= (pEnd2Pos->aPortion2.*fnRectX->fnGetLeft)();
            long nLeftAbs = nRightAbs - (pEnd2Pos->aPortion2.*fnRectX->fnGetWidth)();

            (aTmp.*fnRectX->fnSetLeft)( nLeftAbs );
            aTmp.Intersection( aEndFrm );
            Sub( aRegion, aTmp );

            aEndRect = pEnd2Pos->aLine;
            (aEndRect.*fnRectX->fnSetLeft)( bEndR2L ? nLeftAbs : nRightAbs );
            (aEndRect.*fnRectX->fnSetWidth)( 1 );
        }
    }

    // The charrect may be outside the paintarea (for cursortravelling)
    // but the selection has to be restricted to the paintarea
    if( aStRect.Left() < aStFrm.Left() )
        aStRect.Left( aStFrm.Left() );
    else if( aStRect.Left() > aStFrm.Right() )
        aStRect.Left( aStFrm.Right() );
    SwTwips nTmp = aStRect.Right();
    if( nTmp < aStFrm.Left() )
        aStRect.Right( aStFrm.Left() );
    else if( nTmp > aStFrm.Right() )
        aStRect.Right( aStFrm.Right() );
    if( aEndRect.Left() < aEndFrm.Left() )
        aEndRect.Left( aEndFrm.Left() );
    else if( aEndRect.Left() > aEndFrm.Right() )
        aEndRect.Left( aEndFrm.Right() );
    nTmp = aEndRect.Right();
    if( nTmp < aEndFrm.Left() )
        aEndRect.Right( aEndFrm.Left() );
    else if( nTmp > aEndFrm.Right() )
        aEndRect.Right( aEndFrm.Right() );

    if( pStartFrm == pEndFrm )
    {
        bool bSameRotatedOrBidi = pSt2Pos && pEnd2Pos &&
            ( MultiPortionType::BIDI == pSt2Pos->nMultiType ) &&
            pSt2Pos->aPortion == pEnd2Pos->aPortion;
        //case 1: (Same frame and same row)
        if( bSameRotatedOrBidi ||
            (aStRect.*fnRect->fnGetTop)() == (aEndRect.*fnRect->fnGetTop)() )
        {
            Point aTmpSt( aStRect.Pos() );
            Point aTmpEnd( aEndRect.Right(), aEndRect.Bottom() );
            if( bSameRotatedOrBidi || bR2L )
            {
                if( aTmpSt.Y() > aTmpEnd.Y() )
                {
                    long nTmpY = aTmpEnd.Y();
                    aTmpEnd.Y() = aTmpSt.Y();
                    aTmpSt.Y() = nTmpY;
                }
                if( aTmpSt.X() > aTmpEnd.X() )
                {
                    long nTmpX = aTmpEnd.X();
                    aTmpEnd.X() = aTmpSt.X();
                    aTmpSt.X() = nTmpX;
                }
            }

            SwRect aTmp = SwRect( aTmpSt, aTmpEnd );
            // Bug 34888: If content is selected which doesn't take space
            //            away (i.e. PostIts, RefMarks, TOXMarks), then at
            //            least set the width of the Crsr.
            if( 1 == (aTmp.*fnRect->fnGetWidth)() &&
                pStartPos->nContent.GetIndex() !=
                pEndPos->nContent.GetIndex() )
            {
                OutputDevice* pOut = pSh->GetOut();
                long nCrsrWidth = pOut->GetSettings().GetStyleSettings().
                    GetCursorSize();
                (aTmp.*fnRect->fnSetWidth)( pOut->PixelToLogic(
                    Size( nCrsrWidth, 0 ) ).Width() );
            }
            aTmp.Intersection( aStFrm );
            Sub( aRegion, aTmp );
        }
        //case 2: (Same frame, but not the same line)
        else
        {
            SwTwips lLeft, lRight;
            if( pSt2Pos && pEnd2Pos && pSt2Pos->aPortion == pEnd2Pos->aPortion )
            {
                lLeft = (pSt2Pos->aPortion.*fnRect->fnGetLeft)();
                lRight = (pSt2Pos->aPortion.*fnRect->fnGetRight)();
            }
            else
            {
                lLeft = (pStartFrm->Frm().*fnRect->fnGetLeft)() +
                    (pStartFrm->Prt().*fnRect->fnGetLeft)();
                lRight = (aEndFrm.*fnRect->fnGetRight)();
            }
            if( lLeft < (aStFrm.*fnRect->fnGetLeft)() )
                lLeft = (aStFrm.*fnRect->fnGetLeft)();
            if( lRight > (aStFrm.*fnRect->fnGetRight)() )
                lRight = (aStFrm.*fnRect->fnGetRight)();
            SwRect aSubRect( aStRect );
            //First line
            if( bR2L )
                (aSubRect.*fnRect->fnSetLeft)( lLeft );
            else
                (aSubRect.*fnRect->fnSetRight)( lRight );
            Sub( aRegion, aSubRect );

            //If there's at least a twips between start- and endline,
            //so the whole area between will be added.
            SwTwips aTmpBottom = (aStRect.*fnRect->fnGetBottom)();
            SwTwips aTmpTop = (aEndRect.*fnRect->fnGetTop)();
            if( aTmpBottom != aTmpTop )
            {
                (aSubRect.*fnRect->fnSetLeft)( lLeft );
                (aSubRect.*fnRect->fnSetRight)( lRight );
                (aSubRect.*fnRect->fnSetTop)( aTmpBottom );
                (aSubRect.*fnRect->fnSetBottom)( aTmpTop );
                Sub( aRegion, aSubRect );
            }
            //and the last line
            aSubRect = aEndRect;
            if( bR2L )
                (aSubRect.*fnRect->fnSetRight)( lRight );
            else
                (aSubRect.*fnRect->fnSetLeft)( lLeft );
            Sub( aRegion, aSubRect );
        }
    }
    //case 3: (Different frames, maybe with other frames between)
    else
    {
        //The startframe first...
        SwRect aSubRect( aStRect );
        if( bR2L )
            (aSubRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)());
        else
            (aSubRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)());
        Sub( aRegion, aSubRect );
        SwTwips nTmpTwips = (aStRect.*fnRect->fnGetBottom)();
        if( (aStFrm.*fnRect->fnGetBottom)() != nTmpTwips )
        {
            aSubRect = aStFrm;
            (aSubRect.*fnRect->fnSetTop)( nTmpTwips );
            Sub( aRegion, aSubRect );
        }

        //Now the frames between, if there are any
        bool const bBody = pStartFrm->IsInDocBody();
        const SwTableBox* pCellBox = pStartFrm->GetUpper()->IsCellFrm() ?
            static_cast<const SwCellFrm*>(pStartFrm->GetUpper())->GetTabBox() : 0;
        if (pSh->IsSelectAll())
            pCellBox = 0;

        const SwContentFrm *pContent = pStartFrm->GetNextContentFrm();
        SwRect aPrvRect;

        OSL_ENSURE( pContent,
            "<SwRootFrm::CalcFrmRects(..)> - no content frame. This is a serious defect" );
        while ( pContent && pContent != pEndFrm )
        {
            if ( pContent->IsInFly() )
            {
                const SwAnchoredObject* pObj = pContent->FindFlyFrm();
                aSortObjs.Insert( *(const_cast<SwAnchoredObject*>(pObj)) );
            }

            // Consider only frames which have the same IsInDocBody value like pStartFrm
            // If pStartFrm is inside a SwCellFrm, consider only frames which are inside the
            // same cell frame (or its follow cell)
            const SwTableBox* pTmpCellBox = pContent->GetUpper()->IsCellFrm() ?
                static_cast<const SwCellFrm*>(pContent->GetUpper())->GetTabBox() : 0;
            if (pSh->IsSelectAll())
                pTmpCellBox = 0;
            if ( bBody == pContent->IsInDocBody() &&
                ( !pCellBox || pCellBox == pTmpCellBox ) )
            {
                SwRect aCRect( pContent->UnionFrm( true ) );
                aCRect.Intersection( pContent->PaintArea() );
                if( aCRect.IsOver( aRegion.GetOrigin() ))
                {
                    SwRect aTmp( aPrvRect );
                    aTmp.Union( aCRect );
                    if ( (aPrvRect.Height() * aPrvRect.Width() +
                        aCRect.Height()   * aCRect.Width()) ==
                        (aTmp.Height() * aTmp.Width()) )
                    {
                        aPrvRect.Union( aCRect );
                    }
                    else
                    {
                        if ( aPrvRect.HasArea() )
                            Sub( aRegion, aPrvRect );
                        aPrvRect = aCRect;
                    }
                }
            }
            pContent = pContent->GetNextContentFrm();
            OSL_ENSURE( pContent,
                "<SwRootFrm::CalcFrmRects(..)> - no content frame. This is a serious defect!" );
        }
        if ( aPrvRect.HasArea() )
            Sub( aRegion, aPrvRect );

        //At least the endframe...
        bVert = pEndFrm->IsVertical();
        bRev = pEndFrm->IsReverse();
        fnRect = bVert ? ( bRev ? fnRectVL2R : ( pEndFrm->IsVertLR() ? fnRectVertL2R : fnRectVert ) ) :
            ( bRev ? fnRectB2T : fnRectHori );
        nTmpTwips = (aEndRect.*fnRect->fnGetTop)();
        if( (aEndFrm.*fnRect->fnGetTop)() != nTmpTwips )
        {
            aSubRect = aEndFrm;
            (aSubRect.*fnRect->fnSetBottom)( nTmpTwips );
            Sub( aRegion, aSubRect );
        }
        aSubRect = aEndRect;
        if( bEndR2L )
            (aSubRect.*fnRect->fnSetRight)((aEndFrm.*fnRect->fnGetRight)());
        else
            (aSubRect.*fnRect->fnSetLeft)( (aEndFrm.*fnRect->fnGetLeft)() );
        Sub( aRegion, aSubRect );
    }

    aRegion.Invert();
    delete pSt2Pos;
    delete pEnd2Pos;

    // Cut out Flys during loop. We don't cut out Flys when:
    // - the Lower is StartFrm/EndFrm (FlyInCnt and all other Flys which again
    //   sit in it)
    // - if in the Z-order we have Flys above those in which the StartFrm is
    //   placed
    // - if they are anchored to inside the selection and thus part of it
    const SwPageFrm *pPage      = pStartFrm->FindPageFrm();
    const SwPageFrm *pEndPage   = pEndFrm->FindPageFrm();

    while ( pPage )
    {
        if ( pPage->GetSortedObjs() )
        {
            const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) ==  nullptr )
                    continue;
                const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
                const SwVirtFlyDrawObj* pObj = pFly->GetVirtDrawObj();
                const SwFormatSurround &rSur = pFly->GetFormat()->GetSurround();
                const SwPosition* anchoredAt = pAnchoredObj->GetFrameFormat().GetAnchor().GetContentAnchor();
                bool inSelection = ( anchoredAt != NULL && *pStartPos <= *anchoredAt && *anchoredAt < *pEndPos );
                if( anchoredAt != NULL && *anchoredAt == *pEndPos )
                {
                    const SwNodes& nodes = anchoredAt->GetDoc()->GetNodes();
                    if( *pEndPos == SwPosition( nodes.GetEndOfContent()))
                        inSelection = true;
                    else
                    {
                        SwNodeIndex idx( nodes.GetEndOfContent());
                     if( SwContentNode* last = SwNodes::GoPrevious( &idx ))
                        inSelection = *pEndPos == SwPosition( *last, last->Len());
                    }
                }
                if( inSelection )
                        Add( aRegion, pFly->Frm() );
                else if ( !pFly->IsAnLower( pStartFrm ) &&
                    (rSur.GetSurround() != SURROUND_THROUGHT &&
                    !rSur.IsContour()) )
                {
                    if ( aSortObjs.Contains( *pAnchoredObj ) )
                        continue;

                    bool bSub = true;
                    const sal_uInt32 nPos = pObj->GetOrdNum();
                    for ( size_t k = 0; bSub && k < aSortObjs.size(); ++k )
                    {
                        OSL_ENSURE( dynamic_cast< const SwFlyFrm *>( aSortObjs[k] ) !=  nullptr,
                            "<SwRootFrm::CalcFrmRects(..)> - object in <aSortObjs> of unexpected type" );
                        const SwFlyFrm* pTmp = static_cast<SwFlyFrm*>(aSortObjs[k]);
                        do
                        {
                            if ( nPos < pTmp->GetVirtDrawObj()->GetOrdNumDirect() )
                            {
                                bSub = false;
                            }
                            else
                            {
                                pTmp = pTmp->GetAnchorFrm()->FindFlyFrm();
                            }
                        } while ( bSub && pTmp );
                    }
                    if ( bSub )
                        Sub( aRegion, pFly->Frm() );
                }
            }
        }
        if ( pPage == pEndPage )
            break;
        else
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
    }

    //Because it looks better, we close the DropCaps.
    SwRect aDropRect;
    if ( pStartFrm->IsTextFrm() )
    {
        if ( static_cast<const SwTextFrm*>(pStartFrm)->GetDropRect( aDropRect ) )
            Sub( aRegion, aDropRect );
    }
    if ( pEndFrm != pStartFrm && pEndFrm->IsTextFrm() )
    {
        if ( static_cast<const SwTextFrm*>(pEndFrm)->GetDropRect( aDropRect ) )
            Sub( aRegion, aDropRect );
    }

    rCrsr.assign( aRegion.begin(), aRegion.end() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
