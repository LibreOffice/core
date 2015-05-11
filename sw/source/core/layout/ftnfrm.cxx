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

#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <ftnidx.hxx>
#include <pagefrm.hxx>
#include <colfrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>
#include <ftnfrm.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <pagedesc.hxx>
#include <ftninfo.hxx>
#include <sectfrm.hxx>
#include <objectformatter.hxx>
#include "viewopt.hxx"
#include <calbck.hxx>

#define ENDNOTE 0x80000000

/// Search the position of an attribute in the FtnArray at the document,
/// because all footnotes are located there, ordered by their index.
static sal_uLong lcl_FindFtnPos( const SwDoc *pDoc, const SwTxtFtn *pAttr )
{
    const SwFtnIdxs &rFtnIdxs = pDoc->GetFtnIdxs();

    SwTxtFtn* pBla = const_cast<SwTxtFtn*>(pAttr);
    SwFtnIdxs::const_iterator it = rFtnIdxs.find( pBla );
    if ( it != rFtnIdxs.end() )
    {
        sal_uLong nRet = it - rFtnIdxs.begin();
        if( pAttr->GetFtn().IsEndNote() )
            return nRet + ENDNOTE;
        return nRet;
    }
    OSL_ENSURE( !pDoc, "FtnPos not found." );
    return 0;
}

bool SwFtnFrm::operator<( const SwTxtFtn* pTxtFtn ) const
{
    const SwDoc* pDoc = GetFmt()->GetDoc();
    OSL_ENSURE( pDoc, "SwFtnFrm: Missing doc!" );
    return lcl_FindFtnPos( pDoc, GetAttr() ) <
           lcl_FindFtnPos( pDoc, pTxtFtn );
}

/*
|*
|*  bool lcl_NextFtnBoss( SwFtnBossFrm* pBoss, SwPageFrm* pPage)
|*  sets pBoss on the next SwFtnBossFrm, which can either be a column
|*  or a page (without columns). If the page changes meanwhile,
|*  pPage contains the new page and this function returns true.
|*
|*/

static bool lcl_NextFtnBoss( SwFtnBossFrm* &rpBoss, SwPageFrm* &rpPage,
    bool bDontLeave )
{
    if( rpBoss->IsColumnFrm() )
    {
        if( rpBoss->GetNext() )
        {
            rpBoss = static_cast<SwFtnBossFrm*>(rpBoss->GetNext()); //next column
            return false;
        }
        if( rpBoss->IsInSct() )
        {
            SwSectionFrm* pSct = rpBoss->FindSctFrm()->GetFollow();
            if( pSct )
            {
                OSL_ENSURE( pSct->Lower() && pSct->Lower()->IsColumnFrm(),
                        "Where's the column?" );
                rpBoss = static_cast<SwColumnFrm*>(pSct->Lower());
                SwPageFrm* pOld = rpPage;
                rpPage = pSct->FindPageFrm();
                return pOld != rpPage;
            }
            else if( bDontLeave )
            {
                rpPage = NULL;
                rpBoss = NULL;
                return false;
            }
        }
    }
    rpPage = static_cast<SwPageFrm*>(rpPage->GetNext()); // next page
    rpBoss = rpPage;
    if( rpPage )
    {
        SwLayoutFrm* pBody = rpPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            rpBoss = static_cast<SwFtnBossFrm*>(pBody->Lower()); // first column
    }
    return true;
}

/// @returns column number if pBoss is a column, otherwise 0.
static sal_uInt16 lcl_ColumnNum( const SwFrm* pBoss )
{
    sal_uInt16 nRet = 0;
    if( !pBoss->IsColumnFrm() )
        return 0;
    const SwFrm* pCol;
    if( pBoss->IsInSct() )
    {
        pCol = pBoss->GetUpper()->FindColFrm();
        if( pBoss->GetNext() || pBoss->GetPrev() )
        {
            while( pBoss )
            {
                ++nRet;                     // Section columns
                pBoss = pBoss->GetPrev();
            }
        }
    }
    else
        pCol = pBoss;
    while( pCol )
    {
        nRet += 256;                    // Page columns
        pCol = pCol->GetPrev();
    }
    return nRet;
}

SwFtnContFrm::SwFtnContFrm( SwFrmFmt *pFmt, SwFrm* pSib ):
    SwLayoutFrm( pFmt, pSib )
{
    mnFrmType = FRM_FTNCONT;
}


// lcl_Undersize(..) walks over a SwFrm and its contents
// and returns the sum of all requested TxtFrm magnifications.

static long lcl_Undersize( const SwFrm* pFrm )
{
    long nRet = 0;
    SWRECTFN( pFrm )
    if( pFrm->IsTxtFrm() )
    {
        if( static_cast<const SwTxtFrm*>(pFrm)->IsUndersized() )
        {
            // Does this TxtFrm would like to be a little bit bigger?
            nRet = static_cast<const SwTxtFrm*>(pFrm)->GetParHeight() -
                    (pFrm->Prt().*fnRect->fnGetHeight)();
            if( nRet < 0 )
                nRet = 0;
        }
    }
    else if( pFrm->IsLayoutFrm() )
    {
        const SwFrm* pNxt = static_cast<const SwLayoutFrm*>(pFrm)->Lower();
        while( pNxt )
        {
            nRet += lcl_Undersize( pNxt );
            pNxt = pNxt->GetNext();
        }
    }
    return nRet;
}

/// "format" the frame (Fixsize is not set here).
void SwFtnContFrm::Format( const SwBorderAttrs * )
{
    // calculate total border, only one distance to the top
    const SwPageFrm* pPage = FindPageFrm();
    const SwPageFtnInfo &rInf = pPage->GetPageDesc()->GetFtnInfo();
    const SwTwips nBorder = rInf.GetTopDist() + rInf.GetBottomDist() +
                            rInf.GetLineWidth();
    SWRECTFN( this )
    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = true;
        (Prt().*fnRect->fnSetTop)( nBorder );
        (Prt().*fnRect->fnSetWidth)( (Frm().*fnRect->fnGetWidth)() );
        (Prt().*fnRect->fnSetHeight)((Frm().*fnRect->fnGetHeight)() - nBorder );
        if( (Prt().*fnRect->fnGetHeight)() < 0 && !pPage->IsFtnPage() )
            mbValidSize = false;
    }

    if ( !mbValidSize )
    {
        bool bGrow = pPage->IsFtnPage();
        if( bGrow )
        {
            const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bGrow = false;
        }
        if( bGrow )
                Grow( LONG_MAX, false );
        else
        {
            // VarSize is determined based on the content plus the borders
            SwTwips nRemaining = 0;
            SwFrm *pFrm = m_pLower;
            while ( pFrm )
            {   // lcl_Undersize(..) respects (recursively) TxtFrms, which
                // would like to be bigger. They are created especially in
                // columnized borders, if these do not have their maximum
                // size yet.
                nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)() + lcl_Undersize( pFrm );
                pFrm = pFrm->GetNext();
            }
            // add the own border
            nRemaining += nBorder;

            SwTwips nDiff;
            if( IsInSct() )
            {
                nDiff = -(Frm().*fnRect->fnBottomDist)(
                                        (GetUpper()->*fnRect->fnGetPrtBottom)() );
                if( nDiff > 0 )
                {
                    if( nDiff > (Frm().*fnRect->fnGetHeight)() )
                        nDiff = (Frm().*fnRect->fnGetHeight)();
                    (Frm().*fnRect->fnAddBottom)( -nDiff );
                    (Prt().*fnRect->fnAddHeight)( -nDiff );
                }
            }
            nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
            if ( nDiff > 0 )
                Shrink( nDiff );
            else if ( nDiff < 0 )
            {
                Grow( -nDiff );
                // It may happen that there is less space available,
                // than what the border needs - the size of the PrtArea
                // will then be negative.
                SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
                if( nPrtHeight < 0 )
                {
                    const SwTwips nTmpDiff = std::max( (Prt().*fnRect->fnGetTop)(),
                                                -nPrtHeight );
                    (Prt().*fnRect->fnSubTop)( nTmpDiff );
                }
            }
        }
        mbValidSize = true;
    }
}

SwTwips SwFtnContFrm::GrowFrm( SwTwips nDist, bool bTst, bool )
{
    // No check if FixSize since FtnContainer are variable up to their max. height.
    // If the max. height is LONG_MAX, take as much space as needed.
    // If the page is a special footnote page, take also as much as possible.
    assert(GetUpper() && GetUpper()->IsFtnBossFrm());

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetHeight)() > 0 &&
         nDist > ( LONG_MAX - (Frm().*fnRect->fnGetHeight)() ) )
        nDist = LONG_MAX - (Frm().*fnRect->fnGetHeight)();

    SwFtnBossFrm *pBoss = static_cast<SwFtnBossFrm*>(GetUpper());
    if( IsInSct() )
    {
        SwSectionFrm* pSect = FindSctFrm();
        OSL_ENSURE( pSect, "GrowFrm: Missing SectFrm" );
        // In a section, which has to maximize, a footnotecontainer is allowed
        // to grow, when the section can't grow anymore.
        if( !bTst && !pSect->IsColLocked() &&
            pSect->ToMaximize( false ) && pSect->Growable() )
        {
            pSect->InvalidateSize();
            return 0;
        }
    }
    const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    SwPageFrm *pPage = pBoss->FindPageFrm();
    if ( bBrowseMode || !pPage->IsFtnPage() )
    {
        if ( pBoss->GetMaxFtnHeight() != LONG_MAX )
        {
            nDist = std::min( nDist, pBoss->GetMaxFtnHeight()
                         - (Frm().*fnRect->fnGetHeight)() );
            if ( nDist <= 0 )
                return 0L;
        }
        // FtnBoss also influences the max value
        if( !IsInSct() )
        {
            const SwTwips nMax = pBoss->GetVarSpace();
            if ( nDist > nMax )
                nDist = nMax;
            if ( nDist <= 0 )
                return 0L;
        }
    }
    else if( nDist > (GetPrev()->Frm().*fnRect->fnGetHeight)() )
        // do not use more space than the body has
        nDist = (GetPrev()->Frm().*fnRect->fnGetHeight)();

    long nAvail = 0;
    if ( bBrowseMode )
    {
        nAvail = GetUpper()->Prt().Height();
        const SwFrm *pAvail = GetUpper()->Lower();
        do
        {   nAvail -= pAvail->Frm().Height();
            pAvail = pAvail->GetNext();
        } while ( pAvail );
        if ( nAvail > nDist )
            nAvail = nDist;
    }

    if ( !bTst )
    {
        (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)() + nDist );

        if( IsVertical() && !IsVertLR() && !IsReverse() )
            Frm().Pos().X() -= nDist;
    }
    long nGrow = nDist - nAvail,
         nReal = 0;
    if ( nGrow > 0 )
    {
        sal_uInt8 nAdjust = pBoss->NeighbourhoodAdjustment( this );
        if( NA_ONLY_ADJUST == nAdjust )
            nReal = AdjustNeighbourhood( nGrow, bTst );
        else
        {
            if( NA_GROW_ADJUST == nAdjust )
            {
                SwFrm* pFtn = Lower();
                if( pFtn )
                {
                    while( pFtn->GetNext() )
                        pFtn = pFtn->GetNext();
                    if( static_cast<SwFtnFrm*>(pFtn)->GetAttr()->GetFtn().IsEndNote() )
                    {
                        nReal = AdjustNeighbourhood( nGrow, bTst );
                        nAdjust = NA_GROW_SHRINK; // no more AdjustNeighbourhood
                    }
                }
            }
            nReal += pBoss->Grow( nGrow - nReal, bTst );
            if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
                  && nReal < nGrow )
                nReal += AdjustNeighbourhood( nGrow - nReal, bTst );
        }
    }

    nReal += nAvail;

    if ( !bTst )
    {
        if ( nReal != nDist )
        {
            nDist -= nReal;
            // We can only respect the boundless wish so much
            Frm().SSize().Height() -= nDist;

            if( IsVertical() && !IsVertLR() && !IsReverse() )
                Frm().Pos().X() += nDist;
        }

        // growing happens upwards, so successors to not need to be invalidated
        if( nReal )
        {
            _InvalidateSize();
            _InvalidatePos();
            InvalidatePage( pPage );
        }
    }
    return nReal;
}

SwTwips SwFtnContFrm::ShrinkFrm( SwTwips nDiff, bool bTst, bool bInfo )
{
    SwPageFrm *pPage = FindPageFrm();
    bool bShrink = false;
    if ( pPage )
    {
        if( !pPage->IsFtnPage() )
            bShrink = true;
        else
        {
            const SwViewShell *pSh = getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bShrink = true;
        }
    }
    if( bShrink )
    {
        SwTwips nRet = SwLayoutFrm::ShrinkFrm( nDiff, bTst, bInfo );
        if( IsInSct() && !bTst )
            FindSctFrm()->InvalidateNextPos();
        if ( !bTst && nRet )
        {
            _InvalidatePos();
            InvalidatePage( pPage );
        }
        return nRet;
    }
    return 0;
}

SwFtnFrm::SwFtnFrm( SwFrmFmt *pFmt, SwFrm* pSib, SwCntntFrm *pCnt, SwTxtFtn *pAt ):
    SwLayoutFrm( pFmt, pSib ),
    pFollow( 0 ),
    pMaster( 0 ),
    pRef( pCnt ),
    pAttr( pAt ),
    bBackMoveLocked( false ),
    // #i49383#
    mbUnlockPosOfLowerObjs( true )
{
    mnFrmType = FRM_FTN;
}

void SwFtnFrm::InvalidateNxtFtnCnts( SwPageFrm *pPage )
{
    if ( GetNext() )
    {
        SwFrm *pCnt = static_cast<SwLayoutFrm*>(GetNext())->ContainsAny();
        if( pCnt )
        {
            pCnt->InvalidatePage( pPage );
            pCnt->_InvalidatePrt();
            do
            {   pCnt->_InvalidatePos();
                if( pCnt->IsSctFrm() )
                {
                    SwFrm* pTmp = static_cast<SwSectionFrm*>(pCnt)->ContainsAny();
                    if( pTmp )
                        pTmp->_InvalidatePos();
                }
                pCnt->GetUpper()->_InvalidateSize();
                pCnt = pCnt->FindNext();
            } while ( pCnt && GetUpper()->IsAnLower( pCnt ) );
        }
    }
}

#ifdef DBG_UTIL
SwTwips SwFtnFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    static sal_uInt16 nNum = USHRT_MAX;
    SwTxtFtn* pTxtFtn = GetAttr();
    if ( pTxtFtn->GetFtn().GetNumber() == nNum )
    {
        int bla = 5;
        (void)bla;

    }
    return SwLayoutFrm::GrowFrm( nDist, bTst, bInfo );
}

SwTwips SwFtnFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    static sal_uInt16 nNum = USHRT_MAX;
    if( nNum != USHRT_MAX )
    {
        SwTxtFtn* pTxtFtn = GetAttr();
        if( pTxtFtn->GetFtn().GetNumber() == nNum )
        {
            int bla = 5;
            (void)bla;
        }
    }
    return SwLayoutFrm::ShrinkFrm( nDist, bTst, bInfo );
}
#endif

void SwFtnFrm::Cut()
{
    if ( GetNext() )
        GetNext()->InvalidatePos();
    else if ( GetPrev() )
        GetPrev()->SetRetouche();

    // first move then shrink Upper
    SwLayoutFrm *pUp = GetUpper();

    // correct chaining
    SwFtnFrm *pFtn = this;
    if ( pFtn->GetFollow() )
        pFtn->GetFollow()->SetMaster( pFtn->GetMaster() );
    if ( pFtn->GetMaster() )
        pFtn->GetMaster()->SetFollow( pFtn->GetFollow() );
    pFtn->SetFollow( 0 );
    pFtn->SetMaster( 0 );

    // cut all connections
    RemoveFromLayout();

    if ( pUp )
    {
        // The last footnote takes its container along
        if ( !pUp->Lower() )
        {
            SwPageFrm *pPage = pUp->FindPageFrm();
            if ( pPage )
            {
                SwLayoutFrm *pBody = pPage->FindBodyCont();
                if( pBody && !pBody->ContainsCntnt() )
                    pPage->getRootFrm()->SetSuperfluous();
            }
            SwSectionFrm* pSect = pUp->FindSctFrm();
            pUp->Cut();
            SwFrm::DestroyFrm(pUp);
            // If the last footnote container was removed from a column
            // section without a Follow, then this section can be shrunk.
            if( pSect && !pSect->ToMaximize( false ) && !pSect->IsColLocked() )
                pSect->_InvalidateSize();
        }
        else
        {   if ( Frm().Height() )
                pUp->Shrink( Frm().Height() );
            pUp->SetCompletePaint();
            pUp->InvalidatePage();
        }
    }
}

void SwFtnFrm::Paste(  SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "no parent in Paste." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is CntntFrm." );
    OSL_ENSURE( pParent != this, "I am my own parent." );
    OSL_ENSURE( pSibling != this, "I am my own sibling." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I am still somewhere registered." );

    // insert into tree structure
    InsertBefore( static_cast<SwLayoutFrm*>(pParent), pSibling );

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetWidth)()!=(pParent->Prt().*fnRect->fnGetWidth)() )
        _InvalidateSize();
    _InvalidatePos();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    if ( GetNext() )
        GetNext()->_InvalidatePos();
    if( (Frm().*fnRect->fnGetHeight)() )
        pParent->Grow( (Frm().*fnRect->fnGetHeight)() );

    // If the predecessor is the master and/or the successor is the Follow,
    // then take their content and destroy them.
    if ( GetPrev() && GetPrev() == GetMaster() )
    { OSL_ENSURE( SwFlowFrm::CastFlowFrm( GetPrev()->GetLower() ),
                "Footnote without content?" );
        (SwFlowFrm::CastFlowFrm( GetPrev()->GetLower()))->
            MoveSubTree( this, GetLower() );
        SwFrm *pDel = GetPrev();
        pDel->Cut();
        SwFrm::DestroyFrm(pDel);
    }
    if ( GetNext() && GetNext() == GetFollow() )
    { OSL_ENSURE( SwFlowFrm::CastFlowFrm( GetNext()->GetLower() ),
                "Footnote without content?" );
        (SwFlowFrm::CastFlowFrm( GetNext()->GetLower()))->MoveSubTree( this );
        SwFrm *pDel = GetNext();
        pDel->Cut();
        SwFrm::DestroyFrm(pDel);
    }
#if OSL_DEBUG_LEVEL > 0
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( GetPrev() )
    {
        OSL_ENSURE( lcl_FindFtnPos( pDoc, static_cast<SwFtnFrm*>(GetPrev())->GetAttr() ) <=
                lcl_FindFtnPos( pDoc, GetAttr() ), "Prev is not FtnPrev" );
    }
    if ( GetNext() )
    {
        OSL_ENSURE( lcl_FindFtnPos( pDoc, GetAttr() ) <=
                lcl_FindFtnPos( pDoc, static_cast<SwFtnFrm*>(GetNext())->GetAttr() ),
                "Next is not FtnNext" );
    }
#endif
    InvalidateNxtFtnCnts( pPage );
}

/// Return the next layout leaf in that the frame can be moved.
/// New pages will only be created if specified by the parameter.
SwLayoutFrm *SwFrm::GetNextFtnLeaf( MakePageType eMakePage )
{
    SwFtnBossFrm *pOldBoss = FindFtnBossFrm();
    SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
    SwPageFrm* pPage;
    SwFtnBossFrm *pBoss = pOldBoss->IsColumnFrm() ?
        static_cast<SwFtnBossFrm*>(pOldBoss->GetNext()) : 0; // next column, if existing
    if( pBoss )
        pPage = NULL;
    else
    {
        if( pOldBoss->GetUpper()->IsSctFrm() )
        {   // this can only be in a column area
            SwLayoutFrm* pNxt = pOldBoss->GetNextSctLeaf( eMakePage );
            if( pNxt )
            {
                OSL_ENSURE( pNxt->IsColBodyFrm(), "GetNextFtnLeaf: Funny Leaf" );
                pBoss = static_cast<SwFtnBossFrm*>(pNxt->GetUpper());
                pPage = pBoss->FindPageFrm();
            }
            else
                return 0;
        }
        else
        {
            // next page
            pPage = static_cast<SwPageFrm*>(pOldPage->GetNext());
            // skip empty pages
            if( pPage && pPage->IsEmptyPage() )
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            pBoss = pPage;
        }
    }
    // What do we have until here?
    // pBoss != NULL, pPage==NULL => pBoss is the next column on the same page
    // pBoss != NULL, pPage!=NULL => pBoss and pPage are the following page (empty pages skipped)
    // pBoss == NULL => pPage == NULL, so there are no following pages

    // If the footnote has already a Follow we do not need to search.
    // However, if there are unwanted empty columns/pages between Ftn and Follow,
    // create another Follow on the next best column/page and the rest will sort itself out.
    SwFtnFrm *pFtn = FindFtnFrm();
    if ( pFtn && pFtn->GetFollow() )
    {
        SwFtnBossFrm* pTmpBoss = pFtn->GetFollow()->FindFtnBossFrm();
        // Following cases will be handled:
        // 1. both "FtnBoss"es are neighboring columns/pages
        // 2. the new one is the first column of a neighboring page
        // 3. the new one is the first column in a section of the next page
        while( pTmpBoss != pBoss && pTmpBoss && !pTmpBoss->GetPrev() )
            pTmpBoss = pTmpBoss->GetUpper()->FindFtnBossFrm();
        if( pTmpBoss == pBoss )
            return pFtn->GetFollow();
    }

    // If no pBoss could be found or it is a "wrong" page, we need a new page.
    if ( !pBoss || ( pPage && pPage->IsEndNotePage() && !pOldPage->IsEndNotePage() ) )
    {
        if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
        {
            pBoss = InsertPage( pOldPage, pOldPage->IsFtnPage() );
            static_cast<SwPageFrm*>(pBoss)->SetEndNotePage( pOldPage->IsEndNotePage() );
        }
        else
            return 0;
    }
    if( pBoss->IsPageFrm() )
    {
        // If this page has columns, then go to the first one
        SwLayoutFrm* pLay = pBoss->FindBodyCont();
        if( pLay && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
            pBoss = static_cast<SwFtnBossFrm*>(pLay->Lower());
    }
    // found column/page - add myself
    SwFtnContFrm *pCont = pBoss->FindFtnCont();
    if ( !pCont && pBoss->GetMaxFtnHeight() &&
         ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        pCont = pBoss->MakeFtnCont();
    return pCont;
}

/// Get the preceding layout leaf in that the frame can be moved.
SwLayoutFrm *SwFrm::GetPrevFtnLeaf( MakePageType eMakeFtn )
{
    // The predecessor of a footnote is (if possible)
    // the master of the chain of the footnote.
    SwFtnFrm *pFtn = FindFtnFrm();
    SwLayoutFrm *pRet = pFtn->GetMaster();

    SwFtnBossFrm* pOldBoss = FindFtnBossFrm();
    SwPageFrm *pOldPage = pOldBoss->FindPageFrm();

    if ( !pOldBoss->GetPrev() && !pOldPage->GetPrev() )
        return pRet; // there is neither a predecessor column nor page

    if ( !pRet )
    {
        bool bEndn = pFtn->GetAttr()->GetFtn().IsEndNote();
        SwFrm* pTmpRef = NULL;
        if( bEndn && pFtn->IsInSct() )
        {
            SwSectionFrm* pSect = pFtn->FindSctFrm();
            if( pSect->IsEndnAtEnd() )
                pTmpRef = pSect->FindLastCntnt( FINDMODE_LASTCNT );
        }
        if( !pTmpRef )
            pTmpRef = pFtn->GetRef();
        SwFtnBossFrm* pStop = pTmpRef->FindFtnBossFrm( !bEndn );

        const sal_uInt16 nNum = pStop->GetPhyPageNum();

        // Do not leave the corresponding page if the footnote should
        // be shown at the document ending or the footnote is an endnote.
        const bool bEndNote = pOldPage->IsEndNotePage();
        const bool bFtnEndDoc = pOldPage->IsFtnPage();
        SwFtnBossFrm* pNxtBoss = pOldBoss;
        SwSectionFrm *pSect = pNxtBoss->GetUpper()->IsSctFrm() ?
                              static_cast<SwSectionFrm*>(pNxtBoss->GetUpper()) : 0;

        do
        {
            if( pNxtBoss->IsColumnFrm() && pNxtBoss->GetPrev() )
                pNxtBoss = static_cast<SwFtnBossFrm*>(pNxtBoss->GetPrev());  // one column backwards
            else // one page backwards
            {
                SwLayoutFrm* pBody = 0;
                if( pSect )
                {
                    if( pSect->IsFtnLock() )
                    {
                        if( pNxtBoss == pOldBoss )
                            return 0;
                        pStop = pNxtBoss;
                    }
                    else
                    {
                        pSect = pSect->FindMaster();
                        if( !pSect || !pSect->Lower() )
                            return 0;
                        OSL_ENSURE( pSect->Lower()->IsColumnFrm(),
                                "GetPrevFtnLeaf: Where's the column?" );
                        pNxtBoss = static_cast<SwFtnBossFrm*>(pSect->Lower());
                        pBody = pSect;
                    }
                }
                else
                {
                    SwPageFrm* pPage = static_cast<SwPageFrm*>(pNxtBoss->FindPageFrm()->GetPrev());
                    if( !pPage || pPage->GetPhyPageNum() < nNum ||
                        bEndNote != pPage->IsEndNotePage() || bFtnEndDoc != pPage->IsFtnPage() )
                        return NULL; // no further pages found
                    pNxtBoss = pPage;
                    pBody = pPage->FindBodyCont();
                }
                // We have the previous page, we might need to find the last column of it
                if( pBody )
                {
                    if ( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
                    {
                        pNxtBoss = static_cast<SwFtnBossFrm*>(pBody->GetLastLower());
                    }
                }
            }
            SwFtnContFrm *pCont = pNxtBoss->FindFtnCont();
            if ( pCont )
            {
                pRet = pCont;
                break;
            }
            if ( pStop == pNxtBoss )
            {
                // Reached the column/page of the reference.
                // Try to add a container and paste our content.
                if ( eMakeFtn == MAKEPAGE_FTN && pNxtBoss->GetMaxFtnHeight() )
                    pRet = pNxtBoss->MakeFtnCont();
                break;
            }
        } while( !pRet );
    }
    if ( pRet )
    {
        const SwFtnBossFrm* pNewBoss = pRet->FindFtnBossFrm();
        bool bJump = false;
        if( pOldBoss->IsColumnFrm() && pOldBoss->GetPrev() ) // a previous column exists
            bJump = pOldBoss->GetPrev() != (SwFrm*)pNewBoss; // did we chose it?
        else if( pNewBoss->IsColumnFrm() && pNewBoss->GetNext() )
            bJump = true; // there is another column after the boss (not the old boss)
        else
        {
            // Will be reached only if old and new boss are both either pages or the last (new)
            // or first (old) column of a page. In this case, check if pages were skipped.
            const sal_uInt16 nDiff = pOldPage->GetPhyPageNum() - pRet->FindPageFrm()->GetPhyPageNum();
            if ( nDiff > 2 ||
                 (nDiff > 1 && !static_cast<SwPageFrm*>(pOldPage->GetPrev())->IsEmptyPage()) )
                bJump = true;
        }
        if( bJump )
            SwFlowFrm::SetMoveBwdJump( true );
    }
    return pRet;
}

bool SwFrm::IsFtnAllowed() const
{
    if ( !IsInDocBody() )
        return false;

    if ( IsInTab() )
    {
        // no footnotes in repeated headlines
        const SwTabFrm *pTab = const_cast<SwFrm*>(this)->ImplFindTabFrm();
        if ( pTab->IsFollow() )
            return !pTab->IsInHeadline( *this );
    }
    return true;
}

void SwRootFrm::UpdateFtnNums()
{
    // page numbering only if set at the document
    if ( GetFmt()->GetDoc()->GetFtnInfo().eNum == FTNNUM_PAGE )
    {
        SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
        while ( pPage && !pPage->IsFtnPage() )
        {
            pPage->UpdateFtnNum();
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
        }
    }
}

/// remove all footnotes (not the references) and all footnote pages
void sw_RemoveFtns( SwFtnBossFrm* pBoss, bool bPageOnly, bool bEndNotes )
{
    do
    {
        SwFtnContFrm *pCont = pBoss->FindFtnCont();
        if ( pCont )
        {
            SwFtnFrm *pFtn = static_cast<SwFtnFrm*>(pCont->Lower());
            OSL_ENSURE( pFtn, "Footnote content without footnote." );
            if ( bPageOnly )
                while ( pFtn->GetMaster() )
                    pFtn = pFtn->GetMaster();
            do
            {
                SwFtnFrm *pNxt = static_cast<SwFtnFrm*>(pFtn->GetNext());
                if ( !pFtn->GetAttr()->GetFtn().IsEndNote() ||
                        bEndNotes )
                {
                    pFtn->GetRef()->Prepare( PREP_FTN, (void*)pFtn->GetAttr() );
                    if ( bPageOnly && !pNxt )
                        pNxt = pFtn->GetFollow();
                    pFtn->Cut();
                    SwFrm::DestroyFrm(pFtn);
                }
                pFtn = pNxt;

            } while ( pFtn );
        }
        if( !pBoss->IsInSct() )
        {
            // A sectionframe with the Ftn/EndnAtEnd-flags may contain
            // foot/endnotes. If the last lower frame of the bodyframe is
            // a multicolumned sectionframe, it may contain footnotes, too.
            SwLayoutFrm* pBody = pBoss->FindBodyCont();
            if( pBody && pBody->Lower() )
            {
                SwFrm* pLow = pBody->Lower();
                while (pLow)
                {
                    if( pLow->IsSctFrm() && ( !pLow->GetNext() ||
                        static_cast<SwSectionFrm*>(pLow)->IsAnyNoteAtEnd() ) &&
                        static_cast<SwSectionFrm*>(pLow)->Lower() &&
                        static_cast<SwSectionFrm*>(pLow)->Lower()->IsColumnFrm() )
                        sw_RemoveFtns( static_cast<SwColumnFrm*>(static_cast<SwSectionFrm*>(pLow)->Lower()),
                            bPageOnly, bEndNotes );
                    pLow = pLow->GetNext();
                }
            }
        }
        // is there another column?
        pBoss = pBoss->IsColumnFrm() ? static_cast<SwColumnFrm*>(pBoss->GetNext()) : NULL;
    } while( pBoss );
}

void SwRootFrm::RemoveFtns( SwPageFrm *pPage, bool bPageOnly, bool bEndNotes )
{
    if ( !pPage )
        pPage = static_cast<SwPageFrm*>(Lower());

    do
    {   // On columned pages we have to clean up in all columns
        SwFtnBossFrm* pBoss;
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = static_cast<SwFtnBossFrm*>(pBody->Lower()); // the first column
        else
            pBoss = pPage; // no columns
        sw_RemoveFtns( pBoss, bPageOnly, bEndNotes );
        if ( !bPageOnly )
        {
            if ( pPage->IsFtnPage() &&
                 (!pPage->IsEndNotePage() || bEndNotes) )
            {
                SwFrm *pDel = pPage;
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                pDel->Cut();
                SwFrm::DestroyFrm(pDel);
            }
            else
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
        }
        else
            break;

    } while ( pPage );
}

/// Change the page template of the footnote pages
void SwRootFrm::CheckFtnPageDescs( bool bEndNote )
{
    SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
    while ( pPage && !pPage->IsFtnPage() )
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    while ( pPage && pPage->IsEndNotePage() != bEndNote )
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());

    if ( pPage )
        SwFrm::CheckPageDescs( pPage, false );
}

/** Insert a footnote container
 *
 * A footnote container is always placed directly behind the body text.
 *
 * The frame format (FrmFmt) is always the default frame format.
 *
 * @return footnote container frame
 */
SwFtnContFrm *SwFtnBossFrm::MakeFtnCont()
{
    SAL_WARN_IF(FindFtnCont(), "sw.core", "footnote container exists already");

    SwFtnContFrm *pNew = new SwFtnContFrm( GetFmt()->GetDoc()->GetDfltFrmFmt(), this );
    SwLayoutFrm *pLay = FindBodyCont();
    pNew->Paste( this, pLay->GetNext() );
    return pNew;
}

SwFtnContFrm *SwFtnBossFrm::FindFtnCont()
{
    SwFrm *pFrm = Lower();
    while( pFrm && !pFrm->IsFtnContFrm() )
        pFrm = pFrm->GetNext();

#if OSL_DEBUG_LEVEL > 0
    if ( pFrm )
    {
        SwFrm *pFtn = pFrm->GetLower();
        OSL_ENSURE( pFtn, "Content without footnote." );
        while ( pFtn )
        {
            OSL_ENSURE( pFtn->IsFtnFrm(), "Neighbor of footnote is not a footnote." );
            pFtn = pFtn->GetNext();
        }
    }
#endif

    return static_cast<SwFtnContFrm*>(pFrm);
}

/// Search the next available footnote container.
SwFtnContFrm *SwFtnBossFrm::FindNearestFtnCont( bool bDontLeave )
{
    SwFtnContFrm *pCont = 0;
    if ( !GetFmt()->GetDoc()->GetFtnIdxs().empty() )
    {
        pCont = FindFtnCont();
        if ( !pCont )
        {
            SwPageFrm *pPage = FindPageFrm();
            SwFtnBossFrm* pBoss = this;
            bool bEndNote = pPage->IsEndNotePage();
            do
            {
                bool bChgPage = lcl_NextFtnBoss( pBoss, pPage, bDontLeave );
                // Found another boss? When changing pages, also the endnote flag must match.
                if( pBoss && ( !bChgPage || pPage->IsEndNotePage() == bEndNote ) )
                    pCont = pBoss->FindFtnCont();
            } while ( !pCont && pPage );
        }
    }
    return pCont;
}

SwFtnFrm *SwFtnBossFrm::FindFirstFtn()
{
    // search for the nearest footnote container
    SwFtnContFrm *pCont = FindNearestFtnCont();
    if ( !pCont )
        return 0;

    // Starting from the first footnote, search the first
    // footnote that is referenced by the current column/page

    SwFtnFrm *pRet = static_cast<SwFtnFrm*>(pCont->Lower());
    const sal_uInt16 nRefNum = FindPageFrm()->GetPhyPageNum();
    const sal_uInt16 nRefCol = lcl_ColumnNum( this );
    sal_uInt16 nPgNum, nColNum; // page number, column number
    SwFtnBossFrm* pBoss;
    SwPageFrm* pPage;
    if( pRet )
    {
        pBoss = pRet->GetRef()->FindFtnBossFrm();
        OSL_ENSURE( pBoss, "FindFirstFtn: No boss found" );
        if( !pBoss )
            return NULL; // ?There must be a bug, but no GPF
        pPage = pBoss->FindPageFrm();
        nPgNum = pPage->GetPhyPageNum();
        if ( nPgNum == nRefNum )
        {
            nColNum = lcl_ColumnNum( pBoss );
            if( nColNum == nRefCol )
                return pRet; // found
            else if( nColNum > nRefCol )
                return NULL; // at least one column too far
        }
        else if ( nPgNum > nRefNum )
            return NULL;    // at least one column too far
    }
    else
        return NULL;
    // Done if Ref is on a subsequent page or on the same page in a subsequent column

    do
    {
        while ( pRet->GetFollow() )
            pRet = pRet->GetFollow();

        SwFtnFrm *pNxt = static_cast<SwFtnFrm*>(pRet->GetNext());
        if ( !pNxt )
        {
            pBoss = pRet->FindFtnBossFrm();
            pPage = pBoss->FindPageFrm();
            lcl_NextFtnBoss( pBoss, pPage, false ); // next FtnBoss
            pCont = pBoss ? pBoss->FindNearestFtnCont() : 0;
            if ( pCont )
                pNxt = static_cast<SwFtnFrm*>(pCont->Lower());
        }
        if ( pNxt )
        {
            pRet = pNxt;
            pBoss = pRet->GetRef()->FindFtnBossFrm();
            pPage = pBoss->FindPageFrm();
            nPgNum = pPage->GetPhyPageNum();
            if ( nPgNum == nRefNum )
            {
                nColNum = lcl_ColumnNum( pBoss );
                if( nColNum == nRefCol )
                    break; // found
                else if( nColNum > nRefCol )
                    pRet = 0; // at least one column too far
            }
            else if ( nPgNum > nRefNum )
                pRet = 0;   // at least a page too far
        }
        else
            pRet = 0;   // there is none
    } while( pRet );
    return pRet;
}

/// Get the first footnote of a given content
const SwFtnFrm *SwFtnBossFrm::FindFirstFtn( SwCntntFrm *pCnt ) const
{
    const SwFtnFrm *pRet = const_cast<SwFtnBossFrm*>(this)->FindFirstFtn();
    if ( pRet )
    {
        const sal_uInt16 nColNum = lcl_ColumnNum( this );
        const sal_uInt16 nPageNum = GetPhyPageNum();
        while ( pRet && (pRet->GetRef() != pCnt) )
        {
            while ( pRet->GetFollow() )
                pRet = pRet->GetFollow();

            if ( pRet->GetNext() )
                pRet = static_cast<const SwFtnFrm*>(pRet->GetNext());
            else
            {   SwFtnBossFrm *pBoss = const_cast<SwFtnBossFrm*>(pRet->FindFtnBossFrm());
                SwPageFrm *pPage = pBoss->FindPageFrm();
                lcl_NextFtnBoss( pBoss, pPage, false ); // next FtnBoss
                SwFtnContFrm *pCont = pBoss ? pBoss->FindNearestFtnCont() : 0;
                pRet = pCont ? static_cast<SwFtnFrm*>(pCont->Lower()) : 0;
            }
            if ( pRet )
            {
                const SwFtnBossFrm* pBoss = pRet->GetRef()->FindFtnBossFrm();
                if( pBoss->GetPhyPageNum() != nPageNum ||
                    nColNum != lcl_ColumnNum( pBoss ) )
                pRet = 0;
            }
        }
    }
    return pRet;
}

void SwFtnBossFrm::ResetFtn( const SwFtnFrm *pCheck )
{
    // Destroy the incarnations of footnotes to an attribute, if they don't
    // belong to pAssumed
    OSL_ENSURE( !pCheck->GetMaster(), "given master is not a Master." );

    SwNodeIndex aIdx( *pCheck->GetAttr()->GetStartNode(), 1 );
    SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
    if ( !pNd )
        pNd = pCheck->GetFmt()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    SwIterator<SwFrm,SwCntntNode> aIter( *pNd );
    SwFrm* pFrm = aIter.First();
    while( pFrm )
    {
            if( pFrm->getRootFrm() == pCheck->getRootFrm() )
            {
            SwFrm *pTmp = pFrm->GetUpper();
            while ( pTmp && !pTmp->IsFtnFrm() )
                pTmp = pTmp->GetUpper();

            SwFtnFrm *pFtn = static_cast<SwFtnFrm*>(pTmp);
            while ( pFtn && pFtn->GetMaster() )
                pFtn = pFtn->GetMaster();
            if ( pFtn != pCheck )
            {
                while ( pFtn )
                {
                    SwFtnFrm *pNxt = pFtn->GetFollow();
                    pFtn->Cut();
                    SwFrm::DestroyFrm(pFtn);
                    pFtn = pNxt;
                }
            }
        }

        pFrm = aIter.Next();
    }
}

void SwFtnBossFrm::InsertFtn( SwFtnFrm* pNew )
{
    // Place the footnote in front of the footnote whose attribute
    // is in front of the new one (get position via the Doc).
    // If there is no footnote in this footnote-boss yet, create a new container.
    // If there is a container but no footnote for this footnote-boss yet, place
    // the footnote behind the last footnote of the closest previous column/page.

    ResetFtn( pNew );
    SwFtnFrm *pSibling = FindFirstFtn();
    bool bDontLeave = false;

    // Ok, a sibling has been found, but is the sibling in an acceptable
    // environment?
    if( IsInSct() )
    {
        SwSectionFrm* pMySect = ImplFindSctFrm();
        bool bEndnt = pNew->GetAttr()->GetFtn().IsEndNote();
        if( bEndnt )
        {
            const SwSectionFmt* pEndFmt = pMySect->GetEndSectFmt();
            bDontLeave = 0 != pEndFmt;
            if( pSibling )
            {
                if( pEndFmt )
                {
                    if( !pSibling->IsInSct() ||
                        !pSibling->ImplFindSctFrm()->IsDescendantFrom( pEndFmt ) )
                        pSibling = NULL;
                }
                else if( pSibling->IsInSct() )
                    pSibling = NULL;
            }
        }
        else
        {
            bDontLeave = pMySect->IsFtnAtEnd();
            if( pSibling )
            {
                if( pMySect->IsFtnAtEnd() )
                {
                    if( !pSibling->IsInSct() ||
                        !pMySect->IsAnFollow( pSibling->ImplFindSctFrm() ) )
                        pSibling = NULL;
                }
                else if( pSibling->IsInSct() )
                    pSibling = NULL;
            }
        }
    }

    if( pSibling && pSibling->FindPageFrm()->IsEndNotePage() !=
        FindPageFrm()->IsEndNotePage() )
        pSibling = NULL;

    // use the Doc to find out the position
    SwDoc *pDoc = GetFmt()->GetDoc();
    const sal_uLong nStPos = ::lcl_FindFtnPos( pDoc, pNew->GetAttr() );

    sal_uLong nCmpPos = 0;
    sal_uLong nLastPos = 0;
    SwFtnContFrm *pParent = 0;
    if( pSibling )
    {
        nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
        if( nCmpPos > nStPos )
            pSibling = NULL;
    }

    if ( !pSibling )
    {   pParent = FindFtnCont();
        if ( !pParent )
        {
            // There is no footnote container yet. Before creating one, keep in mind that
            // there might exist another following footnote that must be placed before the
            // new inserted one e.g. because it was divided over multiple pages etc.
            pParent = FindNearestFtnCont( bDontLeave );
            if ( pParent )
            {
                SwFtnFrm *pFtn = static_cast<SwFtnFrm*>(pParent->Lower());
                if ( pFtn )
                {

                    nCmpPos = ::lcl_FindFtnPos( pDoc, pFtn->GetAttr() );
                    if ( nCmpPos > nStPos )
                        pParent = 0;
                }
                else
                    pParent = 0;
            }
        }
        if ( !pParent )
            // here, we are sure that we can create a footnote container
            pParent = MakeFtnCont();
        else
        {
            // Based on the first footnote below the Parent, search for the first footnote whose
            // index is after the index of the newly inserted, to place the new one correctly
            pSibling = static_cast<SwFtnFrm*>(pParent->Lower());
            if ( !pSibling )
            { OSL_ENSURE( false, "Could not find space for footnote.");
                return;
            }
            nCmpPos  = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );

            SwFtnBossFrm *pNxtB; // remember the last one to not
            SwFtnFrm  *pLastSib = 0;    // go too far.

            while ( pSibling && nCmpPos <= nStPos )
            {
                pLastSib = pSibling; // potential candidate
                nLastPos = nCmpPos;

                while ( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();

                if ( pSibling->GetNext() )
                {
                    pSibling = static_cast<SwFtnFrm*>(pSibling->GetNext());
                    OSL_ENSURE( !pSibling->GetMaster() || ( ENDNOTE > nStPos &&
                            pSibling->GetAttr()->GetFtn().IsEndNote() ),
                            "InsertFtn: Master expected I" );
                }
                else
                {
                    pNxtB = pSibling->FindFtnBossFrm();
                    SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                    bool bEndNote = pSibPage->IsEndNotePage();
                    bool bChgPage = lcl_NextFtnBoss( pNxtB, pSibPage, bDontLeave );
                    // When changing pages, also the endnote flag must match.
                    SwFtnContFrm *pCont = pNxtB && ( !bChgPage ||
                        pSibPage->IsEndNotePage() == bEndNote )
                        ? pNxtB->FindNearestFtnCont( bDontLeave ) : 0;
                    if( pCont )
                        pSibling = static_cast<SwFtnFrm*>(pCont->Lower());
                    else // no further FtnContainer, insert after pSibling
                        break;
                }
                if ( pSibling )
                {
                    nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
                    OSL_ENSURE( nCmpPos > nLastPos, "InsertFtn: Order of FtnFrm's buggy" );
                }
            }
            // pLastSib is the last footnote before the new one and
            // pSibling is empty or the first one afterw the new one
            if ( pSibling && pLastSib && (pSibling != pLastSib) )
            {
                // too far?
                if ( nCmpPos > nStPos )
                    pSibling = pLastSib;
            }
            else if ( !pSibling )
            {
                // Last chance: Take the last footnote of the parent.
                // Special case that happens e.g. when moving paragraphs with multiple footnotes.
                // To keep the order, use the parent of the last inspected footnote.
                pSibling = pLastSib;
                while( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();
                OSL_ENSURE( !pSibling->GetNext(), "InsertFtn: Who's that guy?" );
            }
        }
    }
    else
    {
        // First footnote of the column/page found. Now search from there for the first one on the
        // same column/page whose index is after the given one. The last one found is the predecessor.
        SwFtnBossFrm* pBoss = pNew->GetRef()->FindFtnBossFrm(
            !pNew->GetAttr()->GetFtn().IsEndNote() );
        sal_uInt16 nRefNum = pBoss->GetPhyPageNum();    // page number of the new footnote
        sal_uInt16 nRefCol = lcl_ColumnNum( pBoss );    // column number of the new footnote
        bool bEnd = false;
        SwFtnFrm *pLastSib = 0;
        while ( pSibling && !bEnd && (nCmpPos <= nStPos) )
        {
            pLastSib = pSibling;
            nLastPos = nCmpPos;

            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();

            SwFtnFrm *pFoll = static_cast<SwFtnFrm*>(pSibling->GetNext());
            if ( pFoll )
            {
                pBoss = pSibling->GetRef()->FindFtnBossFrm( !pSibling->
                                            GetAttr()->GetFtn().IsEndNote() );
                sal_uInt16 nTmpRef;
                if( nStPos >= ENDNOTE ||
                    (nTmpRef = pBoss->GetPhyPageNum()) < nRefNum ||
                    ( nTmpRef == nRefNum && lcl_ColumnNum( pBoss ) <= nRefCol ))
                    pSibling = pFoll;
                else
                    bEnd = true;
            }
            else
            {
                SwFtnBossFrm* pNxtB = pSibling->FindFtnBossFrm();
                SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                bool bEndNote = pSibPage->IsEndNotePage();
                bool bChgPage = lcl_NextFtnBoss( pNxtB, pSibPage, bDontLeave );
                // When changing pages, also the endnote flag must match.
                SwFtnContFrm *pCont = pNxtB && ( !bChgPage ||
                    pSibPage->IsEndNotePage() == bEndNote )
                    ? pNxtB->FindNearestFtnCont( bDontLeave ) : 0;
                if ( pCont )
                    pSibling = static_cast<SwFtnFrm*>(pCont->Lower());
                else
                    bEnd = true;
            }
            if ( !bEnd && pSibling )
                nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
            if ( pSibling && pLastSib && (pSibling != pLastSib) )
            {
                // too far?
                if ( (nLastPos < nCmpPos) && (nCmpPos > nStPos) )
                {
                    pSibling = pLastSib;
                    bEnd = true;
                }
            }
        }
    }
    if ( pSibling )
    {
        nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
        if ( nCmpPos < nStPos )
        {
            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();
            pParent = static_cast<SwFtnContFrm*>(pSibling->GetUpper());
            pSibling = static_cast<SwFtnFrm*>(pSibling->GetNext());
        }
        else
        {
            if( pSibling->GetMaster() )
            {
                if( ENDNOTE > nCmpPos || nStPos >= ENDNOTE )
                {
                    OSL_FAIL( "InsertFtn: Master expected II" );
                    do
                        pSibling = pSibling->GetMaster();
                    while ( pSibling->GetMaster() );
                }
            }
            pParent = static_cast<SwFtnContFrm*>(pSibling->GetUpper());
        }
    }
    OSL_ENSURE( pParent, "paste in space?" );
    pNew->Paste( pParent, pSibling );
}

void SwFtnBossFrm::AppendFtn( SwCntntFrm *pRef, SwTxtFtn *pAttr )
{
    // If the footnote already exists, do nothing.
    if ( FindFtn( pRef, pAttr ) )
        return;

    // If footnotes are inserted at the end of the document,
    // we only need to search from the relevant page on.
    // If there is none yet, we need to create one.
    // If it is an Endnote, we need to search for or create an
    // Endnote page.
    SwDoc *pDoc = GetFmt()->GetDoc();
    SwFtnBossFrm *pBoss = this;
    SwPageFrm *pPage = FindPageFrm();
    SwPageFrm *pMyPage = pPage;
    bool bChgPage = false;
    bool bEnd = false;
    if ( pAttr->GetFtn().IsEndNote() )
    {
        bEnd = true;
        if( GetUpper()->IsSctFrm() &&
            static_cast<SwSectionFrm*>(GetUpper())->IsEndnAtEnd() )
        {
            SwFrm* pLast =
                static_cast<SwSectionFrm*>(GetUpper())->FindLastCntnt( FINDMODE_ENDNOTE );
            if( pLast )
            {
                pBoss = pLast->FindFtnBossFrm();
                pPage = pBoss->FindPageFrm();
            }
        }
        else
        {
            while ( pPage->GetNext() && !pPage->IsEndNotePage() )
            {
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                bChgPage = true;
            }
            if ( !pPage->IsEndNotePage() )
            {
                SwPageDesc *pDesc = pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
                pPage = ::InsertNewPage( *pDesc, pPage->GetUpper(),
                        !pPage->OnRightPage(), false, false, true, 0 );
                pPage->SetEndNotePage( true );
                bChgPage = true;
            }
            else
            {
                // At least we can search the approximately correct page.
                // To ensure to be finished in finite time even if hundreds
                // of footnotes exist
                SwPageFrm *pNxt = static_cast<SwPageFrm*>(pPage->GetNext());
                const sal_uLong nStPos = ::lcl_FindFtnPos( pDoc, pAttr );
                while ( pNxt && pNxt->IsEndNotePage() )
                {
                    SwFtnContFrm *pCont = pNxt->FindFtnCont();
                    if ( pCont && pCont->Lower() )
                    {
                        OSL_ENSURE( pCont->Lower()->IsFtnFrm(), "no footnote in the container" );
                        if ( nStPos > ::lcl_FindFtnPos( pDoc,
                                        static_cast<SwFtnFrm*>(pCont->Lower())->GetAttr()))
                        {
                            pPage = pNxt;
                            pNxt = static_cast<SwPageFrm*>(pPage->GetNext());
                            continue;
                        }
                    }
                    break;
                }
            }
        }
    }
    else if( FTNPOS_CHAPTER == pDoc->GetFtnInfo().ePos && ( !GetUpper()->
             IsSctFrm() || !static_cast<SwSectionFrm*>(GetUpper())->IsFtnAtEnd() ) )
    {
        while ( pPage->GetNext() && !pPage->IsFtnPage() &&
                !static_cast<SwPageFrm*>(pPage->GetNext())->IsEndNotePage() )
        {
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            bChgPage = true;
        }

        if ( !pPage->IsFtnPage() )
        {
            SwPageDesc *pDesc = pDoc->GetFtnInfo().GetPageDesc( *pDoc );
            pPage = ::InsertNewPage( *pDesc, pPage->GetUpper(),
                !pPage->OnRightPage(), false, false, true, pPage->GetNext() );
            bChgPage = true;
        }
        else
        {
            // We can at least search the approximately correct page
            // to ensure that we will finish in finite time even if
            // hundreds of footnotes exist.
            SwPageFrm *pNxt = static_cast<SwPageFrm*>(pPage->GetNext());
            const sal_uLong nStPos = ::lcl_FindFtnPos( pDoc, pAttr );
            while ( pNxt && pNxt->IsFtnPage() && !pNxt->IsEndNotePage() )
            {
                SwFtnContFrm *pCont = pNxt->FindFtnCont();
                if ( pCont && pCont->Lower() )
                {
                    OSL_ENSURE( pCont->Lower()->IsFtnFrm(), "no footnote in the container" );
                    if ( nStPos > ::lcl_FindFtnPos( pDoc,
                                        static_cast<SwFtnFrm*>(pCont->Lower())->GetAttr()))
                    {
                        pPage = pNxt;
                        pNxt = static_cast<SwPageFrm*>(pPage->GetNext());
                        continue;
                    }
                }
                break;
            }
        }
    }

    // For now, create a footnote and the corresponding content frames
    if ( !pAttr->GetStartNode() )
    {
        OSL_ENSURE( false, "no footnote content." );
        return;
    }

    // If there is already a footnote content on the column/page,
    // another one cannot be created in a column area.
    if( pBoss->IsInSct() && pBoss->IsColumnFrm() && !pPage->IsFtnPage() )
    {
        SwSectionFrm* pSct = pBoss->FindSctFrm();
        if( bEnd ? !pSct->IsEndnAtEnd() : !pSct->IsFtnAtEnd() )
        {
            SwFtnContFrm* pFtnCont = pSct->FindFtnBossFrm(!bEnd)->FindFtnCont();
            if( pFtnCont )
            {
                SwFtnFrm* pTmp = static_cast<SwFtnFrm*>(pFtnCont->Lower());
                if( bEnd )
                    while( pTmp && !pTmp->GetAttr()->GetFtn().IsEndNote() )
                        pTmp = static_cast<SwFtnFrm*>(pTmp->GetNext());
                if( pTmp && *pTmp < pAttr )
                    return;
            }
        }
    }

    SwFtnFrm *pNew = new SwFtnFrm( pDoc->GetDfltFrmFmt(), this, pRef, pAttr );
    {
        SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
        ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
    }
    // If the page was changed or newly created,
    // we need to place ourselves in the first column
    if( bChgPage )
    {
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        OSL_ENSURE( pBody, "AppendFtn: NoPageBody?" );
        if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = static_cast<SwFtnBossFrm*>(pBody->Lower());
        else
            pBoss = pPage; // page if no columns exist
    }
    pBoss->InsertFtn( pNew );
    if ( pNew->GetUpper() ) // inserted or not?
    {
        ::RegistFlys( pNew->FindPageFrm(), pNew );
        SwSectionFrm* pSect = FindSctFrm();
        // The content of a FtnContainer in a (column) section only need to be calculated
        // if the section stretches already to the bottom edge of the Upper.
        if( pSect && !pSect->IsJoinLocked() && ( bEnd ? !pSect->IsEndnAtEnd() :
            !pSect->IsFtnAtEnd() ) && pSect->Growable() )
            pSect->InvalidateSize();
        else
        {
            // #i49383# - disable unlock of position of
            // lower objects during format of footnote content.
            const bool bOldFtnFrmLocked( pNew->IsColLocked() );
            pNew->ColLock();
            pNew->KeepLockPosOfLowerObjs();
            // #i57914# - adjust fix #i49383#
            SwCntntFrm *pCnt = pNew->ContainsCntnt();
            while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
            {
                pCnt->Calc();
                // #i49383# - format anchored objects
                if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                              *(pCnt->FindPageFrm()) ) )
                    {
                        // restart format with first content
                        pCnt = pNew->ContainsCntnt();
                        continue;
                    }
                }
                pCnt = pCnt->FindNextCnt();
            }
            // #i49383#
            if ( !bOldFtnFrmLocked )
            {
                pNew->ColUnlock();
            }
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pNew->UnlockPosOfLowerObjs();
            pNew->Calc();
            // #i57914# - adjust fix #i49383#
            if ( !bOldFtnFrmLocked && !pNew->GetLower() &&
                 !pNew->IsColLocked() && !pNew->IsBackMoveLocked() )
            {
                pNew->Cut();
                SwFrm::DestroyFrm(pNew);
            }
        }
        pMyPage->UpdateFtnNum();
    }
    else
        SwFrm::DestroyFrm(pNew);
}

SwFtnFrm *SwFtnBossFrm::FindFtn( const SwCntntFrm *pRef, const SwTxtFtn *pAttr )
{
    // the easiest and savest way goes via the attribute
    OSL_ENSURE( pAttr->GetStartNode(), "FtnAtr without StartNode." );
    SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
    SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
    if ( !pNd )
        pNd = pRef->GetAttrSet()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    if ( !pNd )
        return 0;
    SwIterator<SwFrm,SwCntntNode> aIter( *pNd );
    SwFrm* pFrm = aIter.First();
    if( pFrm )
        do
        {
                pFrm = pFrm->GetUpper();
                // #i28500#, #i27243# Due to the endnode collector, there are
                // SwFtnFrms, which are not in the layout. Therefore the
                // bInfFtn flags are not set correctly, and a cell of FindFtnFrm
                // would return 0. Therefore we better call ImplFindFtnFrm().
                SwFtnFrm *pFtn = pFrm->ImplFindFtnFrm();
                if ( pFtn && pFtn->GetRef() == pRef )
                {
                    // The following condition becomes true, if the whole
                    // footnotecontent is a section. While no frames exist,
                    // the HiddenFlag of the section is set, this causes
                    // the GoNextSection-function leaves the footnote.
                    if( pFtn->GetAttr() != pAttr )
                        return 0;
                    while ( pFtn && pFtn->GetMaster() )
                        pFtn = pFtn->GetMaster();
                    return pFtn;
                }

        } while ( 0 != (pFrm = aIter.Next()) );

    return 0;
}

void SwFtnBossFrm::RemoveFtn( const SwCntntFrm *pRef, const SwTxtFtn *pAttr,
                              bool bPrep )
{
    SwFtnFrm *pFtn = FindFtn( pRef, pAttr );
    if( pFtn )
    {
        do
        {
            SwFtnFrm *pFoll = pFtn->GetFollow();
            pFtn->Cut();
            SwFrm::DestroyFrm(pFtn);
            pFtn = pFoll;
        } while ( pFtn );
        if( bPrep && pRef->IsFollow() )
        {
            OSL_ENSURE( pRef->IsTxtFrm(), "NoTxtFrm has Footnote?" );
            SwTxtFrm* pMaster = pRef->FindMaster();
            if( !pMaster->IsLocked() )
                pMaster->Prepare( PREP_FTN_GONE );
        }
    }
    FindPageFrm()->UpdateFtnNum();
}

void SwFtnBossFrm::ChangeFtnRef( const SwCntntFrm *pOld, const SwTxtFtn *pAttr,
                                 SwCntntFrm *pNew )
{
    SwFtnFrm *pFtn = FindFtn( pOld, pAttr );
    while ( pFtn )
    {
        pFtn->SetRef( pNew );
        pFtn = pFtn->GetFollow();
    }
}

/// OD 03.04.2003 #108446# - add parameter <_bCollectOnlyPreviousFtns> in
/// order to control, if only footnotes, which are positioned before the
/// footnote boss frame <this> have to be collected.
void SwFtnBossFrm::CollectFtns( const SwCntntFrm* _pRef,
                                SwFtnBossFrm*     _pOld,
                                SwFtnFrms&        _rFtnArr,
                                const bool    _bCollectOnlyPreviousFtns )
{
    SwFtnFrm *pFtn = _pOld->FindFirstFtn();
    while( !pFtn )
    {
        if( _pOld->IsColumnFrm() )
        {
            // visit columns
            while ( !pFtn && _pOld->GetPrev() )
            {
                // Still no problem if no footnote was found yet. The loop is needed to pick up
                // following rows in tables. In all other cases it might correct bad contexts.
                _pOld = static_cast<SwFtnBossFrm*>(_pOld->GetPrev());
                pFtn = _pOld->FindFirstFtn();
            }
        }
        if( !pFtn )
        {
            // previous page
            SwPageFrm* pPg;
            for ( SwFrm* pTmp = _pOld;
                  0 != ( pPg = static_cast<SwPageFrm*>(pTmp->FindPageFrm()->GetPrev()))
                    && pPg->IsEmptyPage() ;
                )
            {
                pTmp = pPg;
            }
            if( !pPg )
                return;

            SwLayoutFrm* pBody = pPg->FindBodyCont();
            if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            {
                // multiple columns on one page => search last column
                _pOld = static_cast<SwFtnBossFrm*>(pBody->GetLastLower());
            }
            else
                _pOld = pPg; // single column page
            pFtn = _pOld->FindFirstFtn();
        }
    }
    // OD 03.04.2003 #108446# - consider new parameter <_bCollectOnlyPreviousFtns>
    SwFtnBossFrm* pRefBossFrm = NULL;
    if ( _bCollectOnlyPreviousFtns )
    {
        pRefBossFrm = this;
    }
    _CollectFtns( _pRef, pFtn, _rFtnArr, _bCollectOnlyPreviousFtns, pRefBossFrm );
}

inline void FtnInArr( SwFtnFrms& rFtnArr, SwFtnFrm* pFtn )
{
    if ( rFtnArr.end() == std::find( rFtnArr.begin(), rFtnArr.end(), pFtn ) )
        rFtnArr.push_back( pFtn );
}

/// OD 03.04.2003 #108446# - add parameters <_bCollectOnlyPreviousFtns> and
/// <_pRefFtnBossFrm> in order to control, if only footnotes, which are positioned
/// before the given reference footnote boss frame have to be collected.
/// Note: if parameter <_bCollectOnlyPreviousFtns> is true, then parameter
/// <_pRefFtnBossFrm> have to be referenced to an object.
/// Adjust parameter names.
void SwFtnBossFrm::_CollectFtns( const SwCntntFrm*   _pRef,
                                 SwFtnFrm*           _pFtn,
                                 SwFtnFrms&          _rFtnArr,
                                 bool                _bCollectOnlyPreviousFtns,
                                 const SwFtnBossFrm* _pRefFtnBossFrm)
{
    // OD 03.04.2003 #108446# - assert, that no reference footnote boss frame
    // is set, in spite of the order, that only previous footnotes has to be
    // collected.
    OSL_ENSURE( !_bCollectOnlyPreviousFtns || _pRefFtnBossFrm,
            "<SwFtnBossFrm::_CollectFtns(..)> - No reference footnote boss frame for collecting only previous footnotes set.\nCrash will be caused!" );

    // Collect all footnotes referenced by pRef (attribute by attribute), combine them
    // (the content might be divided over multiple pages) and cut them.

    // For robustness, we do not log the corresponding footnotes here. If a footnote
    // is touched twice, there might be a crash. This allows this function here to
    // also handle corrupt layouts in some degrees (without loops or even crashes).
    SwFtnFrms aNotFtnArr;

    // here we have a footnote placed in front of the first one of the reference
    OSL_ENSURE( !_pFtn->GetMaster() || _pFtn->GetRef() != _pRef, "move FollowFtn?" );
    while ( _pFtn->GetMaster() )
        _pFtn = _pFtn->GetMaster();

    bool bFound = false;

    do
    {
        // Search for the next footnote in this column/page so that
        // we do not start from zero again after cutting one footnote.
        SwFtnFrm *pNxtFtn = _pFtn;
        while ( pNxtFtn->GetFollow() )
            pNxtFtn = pNxtFtn->GetFollow();
        pNxtFtn = static_cast<SwFtnFrm*>(pNxtFtn->GetNext());

        if ( !pNxtFtn )
        {
            SwFtnBossFrm* pBoss = _pFtn->FindFtnBossFrm();
            SwPageFrm* pPage = pBoss->FindPageFrm();
            do
            {
                lcl_NextFtnBoss( pBoss, pPage, false );
                if( pBoss )
                {
                    SwLayoutFrm* pCont = pBoss->FindFtnCont();
                    if( pCont )
                    {
                        pNxtFtn = static_cast<SwFtnFrm*>(pCont->Lower());
                        if( pNxtFtn )
                        {
                            while( pNxtFtn->GetMaster() )
                                pNxtFtn = pNxtFtn->GetMaster();
                            if( pNxtFtn == _pFtn )
                                pNxtFtn = NULL;
                        }
                    }
                }
            } while( !pNxtFtn && pBoss );
        }
        else if( !pNxtFtn->GetAttr()->GetFtn().IsEndNote() )
        { OSL_ENSURE( !pNxtFtn->GetMaster(), "_CollectFtn: Master expected" );
            while ( pNxtFtn->GetMaster() )
                pNxtFtn = pNxtFtn->GetMaster();
        }
        if ( pNxtFtn == _pFtn )
        {
            OSL_FAIL(   "_CollectFtn: Vicious circle" );
            pNxtFtn = 0;
        }

        // OD 03.04.2003 #108446# - determine, if found footnote has to be collected.
        bool bCollectFoundFtn = false;
        if ( _pFtn->GetRef() == _pRef && !_pFtn->GetAttr()->GetFtn().IsEndNote() )
        {
            if ( _bCollectOnlyPreviousFtns )
            {
                SwFtnBossFrm* pBossOfFoundFtn = _pFtn->FindFtnBossFrm( true );
                OSL_ENSURE( pBossOfFoundFtn,
                        "<SwFtnBossFrm::_CollectFtns(..)> - footnote boss frame of found footnote frame missing.\nWrong layout!" );
                if ( !pBossOfFoundFtn ||    // don't crash, if no footnote boss is found.
                     pBossOfFoundFtn->IsBefore( _pRefFtnBossFrm )
                   )
                {
                    bCollectFoundFtn = true;
                }
            }
            else
            {
                bCollectFoundFtn = true;
            }
        }

        if ( bCollectFoundFtn )
        {
            OSL_ENSURE( !_pFtn->GetMaster(), "move FollowFtn?" );
            SwFtnFrm *pNxt = _pFtn->GetFollow();
            while ( pNxt )
            {
                SwFrm *pCnt = pNxt->ContainsAny();
                if ( pCnt )
                {
                    // destroy the follow on the way as it is empty
                    do
                    {   SwFrm *pNxtCnt = pCnt->GetNext();
                        pCnt->Cut();
                        pCnt->Paste( _pFtn );
                        pCnt = pNxtCnt;
                    } while ( pCnt );
                }
                else
                {
                    OSL_ENSURE( !pNxt, "footnote without content?" );
                    pNxt->Cut();
                    SwFrm::DestroyFrm(pNxt);
                }
                pNxt = _pFtn->GetFollow();
            }
            _pFtn->Cut();
            FtnInArr( _rFtnArr, _pFtn );
            bFound = true;
        }
        else
        {
            FtnInArr( aNotFtnArr, _pFtn );
            if( bFound )
                break;
        }
        if ( pNxtFtn &&
             _rFtnArr.end() == std::find( _rFtnArr.begin(), _rFtnArr.end(), pNxtFtn ) &&
             aNotFtnArr.end() == std::find( aNotFtnArr.begin(), aNotFtnArr.end(), pNxtFtn ) )
            _pFtn = pNxtFtn;
        else
            break;
    }
    while ( _pFtn );
}

void SwFtnBossFrm::_MoveFtns( SwFtnFrms &rFtnArr, bool bCalc )
{
    // All footnotes referenced by pRef need to be moved
    // to a new position (based on the new column/page)
    const sal_uInt16 nMyNum = FindPageFrm()->GetPhyPageNum();
    const sal_uInt16 nMyCol = lcl_ColumnNum( this );
    SWRECTFN( this )

    // #i21478# - keep last inserted footnote in order to
    // format the content of the following one.
    SwFtnFrm* pLastInsertedFtn = 0L;
    for ( size_t i = 0; i < rFtnArr.size(); ++i )
    {
        SwFtnFrm *pFtn = rFtnArr[i];

        SwFtnBossFrm* pRefBoss = pFtn->GetRef()->FindFtnBossFrm( true );
        if( pRefBoss != this )
        {
            const sal_uInt16 nRefNum = pRefBoss->FindPageFrm()->GetPhyPageNum();
            const sal_uInt16 nRefCol = lcl_ColumnNum( this );
            if( nRefNum < nMyNum || ( nRefNum == nMyNum && nRefCol <= nMyCol ) )
                pRefBoss = this;
        }
        pRefBoss->InsertFtn( pFtn );

        if ( pFtn->GetUpper() ) // robust, e.g. with duplicates
        {
            // First condense the content so that footnote frames that do not fit on the page
            // do not do too much harm (Loop 66312). So, the footnote content first grows as
            // soon as the content gets formatted and it is sure that it fits on the page.
            SwFrm *pCnt = pFtn->ContainsAny();
            while( pCnt )
            {
                if( pCnt->IsLayoutFrm() )
                {
                    SwFrm* pTmp = static_cast<SwLayoutFrm*>(pCnt)->ContainsAny();
                    while( pTmp && static_cast<SwLayoutFrm*>(pCnt)->IsAnLower( pTmp ) )
                    {
                        pTmp->Prepare( PREP_MOVEFTN );
                        (pTmp->Frm().*fnRect->fnSetHeight)(0);
                        (pTmp->Prt().*fnRect->fnSetHeight)(0);
                        pTmp = pTmp->FindNext();
                    }
                }
                else
                    pCnt->Prepare( PREP_MOVEFTN );
                (pCnt->Frm().*fnRect->fnSetHeight)(0);
                (pCnt->Prt().*fnRect->fnSetHeight)(0);
                pCnt = pCnt->GetNext();
            }
            (pFtn->Frm().*fnRect->fnSetHeight)(0);
            (pFtn->Prt().*fnRect->fnSetHeight)(0);
            pFtn->Calc();
            pFtn->GetUpper()->Calc();

            if( bCalc )
            {
                SwTxtFtn *pAttr = pFtn->GetAttr();
                pCnt = pFtn->ContainsAny();
                bool bUnlock = !pFtn->IsBackMoveLocked();
                pFtn->LockBackMove();

                // #i49383# - disable unlock of position of
                // lower objects during format of footnote content.
                pFtn->KeepLockPosOfLowerObjs();
                // #i57914# - adjust fix #i49383#

                while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
                {
                    pCnt->_InvalidatePos();
                    pCnt->Calc();
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pCnt = pFtn->ContainsAny();
                            continue;
                        }
                    }
                    if( pCnt->IsSctFrm() )
                    {
                        // If the area is not empty, iterate also over the content
                        SwFrm* pTmp = static_cast<SwSectionFrm*>(pCnt)->ContainsAny();
                        if( pTmp )
                            pCnt = pTmp;
                        else
                            pCnt = pCnt->FindNext();
                    }
                    else
                        pCnt = pCnt->FindNext();
                }
                if( bUnlock )
                {
                    pFtn->UnlockBackMove();
                    if( !pFtn->ContainsAny() && !pFtn->IsColLocked() )
                    {
                        pFtn->Cut();
                        SwFrm::DestroyFrm(pFtn);
                        // #i21478#
                        pFtn = 0L;
                    }
                }
                // #i49383#
                if ( pFtn )
                {
                    // #i57914# - adjust fix #i49383#
                    // enable lock of lower object position before format of footnote frame.
                    pFtn->UnlockPosOfLowerObjs();
                    pFtn->Calc();
                }
            }
        }
        else
        { OSL_ENSURE( !pFtn->GetMaster() && !pFtn->GetFollow(),
                    "DelFtn and Master/Follow?" );
            SwFrm::DestroyFrm(pFtn);
            // #i21478#
            pFtn = 0L;
        }

        // #i21478#
        if ( pFtn )
        {
            pLastInsertedFtn = pFtn;
        }
    }

    // #i21478# - format content of footnote following
    // the new inserted ones.
    if ( bCalc && pLastInsertedFtn )
    {
        if ( pLastInsertedFtn->GetNext() )
        {
            SwFtnFrm* pNextFtn = static_cast<SwFtnFrm*>(pLastInsertedFtn->GetNext());
            SwTxtFtn* pAttr = pNextFtn->GetAttr();
            SwFrm* pCnt = pNextFtn->ContainsAny();

            bool bUnlock = !pNextFtn->IsBackMoveLocked();
            pNextFtn->LockBackMove();
            // #i49383# - disable unlock of position of
            // lower objects during format of footnote content.
            pNextFtn->KeepLockPosOfLowerObjs();
            // #i57914# - adjust fix #i49383#

            while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
            {
                pCnt->_InvalidatePos();
                pCnt->Calc();
                // #i49383# - format anchored objects
                if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                              *(pCnt->FindPageFrm()) ) )
                    {
                        // restart format with first content
                        pCnt = pNextFtn->ContainsAny();
                        continue;
                    }
                }
                if( pCnt->IsSctFrm() )
                {
                    // If the area is not empty, iterate also over the content
                    SwFrm* pTmp = static_cast<SwSectionFrm*>(pCnt)->ContainsAny();
                    if( pTmp )
                        pCnt = pTmp;
                    else
                        pCnt = pCnt->FindNext();
                }
                else
                    pCnt = pCnt->FindNext();
            }
            if( bUnlock )
            {
                pNextFtn->UnlockBackMove();
            }
            // #i49383#
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pNextFtn->UnlockPosOfLowerObjs();
            pNextFtn->Calc();
        }
    }
}

void SwFtnBossFrm::MoveFtns( const SwCntntFrm *pSrc, SwCntntFrm *pDest,
                             SwTxtFtn *pAttr )
{
    if( ( GetFmt()->GetDoc()->GetFtnInfo().ePos == FTNPOS_CHAPTER &&
        (!GetUpper()->IsSctFrm() || !static_cast<SwSectionFrm*>(GetUpper())->IsFtnAtEnd()))
        || pAttr->GetFtn().IsEndNote() )
        return;

    OSL_ENSURE( this == pSrc->FindFtnBossFrm( true ),
            "SwPageFrm::MoveFtns: source frame isn't on that FtnBoss" );

    SwFtnFrm *pFtn = FindFirstFtn();
    if( pFtn )
    {
        ChangeFtnRef( pSrc, pAttr, pDest );
        SwFtnBossFrm *pDestBoss = pDest->FindFtnBossFrm( true );
        OSL_ENSURE( pDestBoss, "+SwPageFrm::MoveFtns: no destination boss" );
        if( pDestBoss )     // robust
        {
            SwFtnFrms aFtnArr;
            SwFtnBossFrm::_CollectFtns( pDest, pFtn, aFtnArr );
            if ( !aFtnArr.empty() )
            {
                pDestBoss->_MoveFtns( aFtnArr, true );
                SwPageFrm* pSrcPage = FindPageFrm();
                SwPageFrm* pDestPage = pDestBoss->FindPageFrm();
                // update FtnNum only at page change
                if( pSrcPage != pDestPage )
                {
                    if( pSrcPage->GetPhyPageNum() > pDestPage->GetPhyPageNum() )
                        pSrcPage->UpdateFtnNum();
                    pDestPage->UpdateFtnNum();
                }
            }
        }
    }
}

void SwFtnBossFrm::RearrangeFtns( const SwTwips nDeadLine, const bool bLock,
                                  const SwTxtFtn *pAttr )
{
    // Format all footnotes of a column/page so that they might change the column/page.

    SwSaveFtnHeight aSave( this, nDeadLine );
    SwFtnFrm *pFtn = FindFirstFtn();
    if( pFtn && pFtn->GetPrev() && bLock )
    {
        SwFtnFrm* pFirst = static_cast<SwFtnFrm*>(pFtn->GetUpper()->Lower());
        SwFrm* pCntnt = pFirst->ContainsAny();
        if( pCntnt )
        {
            bool bUnlock = !pFirst->IsBackMoveLocked();
            pFirst->LockBackMove();
            pFirst->Calc();
            pCntnt->Calc();
            // #i49383# - format anchored objects
            if ( pCntnt->IsTxtFrm() && pCntnt->IsValid() )
            {
                SwObjectFormatter::FormatObjsAtFrm( *pCntnt,
                                                    *(pCntnt->FindPageFrm()) );
            }
            if( bUnlock )
                pFirst->UnlockBackMove();
        }
        pFtn = FindFirstFtn();
    }
    SwDoc *pDoc = GetFmt()->GetDoc();
    const sal_uLong nFtnPos = pAttr ? ::lcl_FindFtnPos( pDoc, pAttr ) : 0;
    SwFrm *pCnt = pFtn ? pFtn->ContainsAny() : 0;
    if ( pCnt )
    {
        bool bMore = true;
        bool bStart = pAttr == 0; // If no attribute is given, process all
        // #i49383# - disable unlock of position of
        // lower objects during format of footnote and footnote content.
        SwFtnFrm* pLastFtnFrm( 0L );
        // footnote frame needs to be locked, if <bLock> isn't set.
        bool bUnlockLastFtnFrm( false );
        do
        {
            if( !bStart )
                bStart = ::lcl_FindFtnPos( pDoc, pCnt->FindFtnFrm()->GetAttr() )
                         == nFtnPos;
            if( bStart )
            {
                pCnt->_InvalidatePos();
                pCnt->_InvalidateSize();
                pCnt->Prepare( PREP_ADJUST_FRM );
                SwFtnFrm* pFtnFrm = pCnt->FindFtnFrm();
                // #i49383#
                if ( pFtnFrm != pLastFtnFrm )
                {
                    if ( pLastFtnFrm )
                    {
                        if ( !bLock && bUnlockLastFtnFrm )
                        {
                            pLastFtnFrm->ColUnlock();
                        }
                        // #i57914# - adjust fix #i49383#
                        // enable lock of lower object position before format of footnote frame.
                        pLastFtnFrm->UnlockPosOfLowerObjs();
                        pLastFtnFrm->Calc();
                        if ( !bLock && bUnlockLastFtnFrm &&
                             !pLastFtnFrm->GetLower() &&
                             !pLastFtnFrm->IsColLocked() &&
                             !pLastFtnFrm->IsBackMoveLocked() )
                        {
                            pLastFtnFrm->Cut();
                            SwFrm::DestroyFrm(pLastFtnFrm);
                            pLastFtnFrm = 0L;
                        }
                    }
                    if ( !bLock )
                    {
                        bUnlockLastFtnFrm = !pFtnFrm->IsColLocked();
                        pFtnFrm->ColLock();
                    }
                    pFtnFrm->KeepLockPosOfLowerObjs();
                    pLastFtnFrm = pFtnFrm;
                }
                // OD 30.10.2002 #97265# - invalidate position of footnote
                // frame, if it's below its footnote container, in order to
                // assure its correct position, probably calculating its previous
                // footnote frames.
                {
                    SWRECTFN( this );
                    SwFrm* aFtnContFrm = pFtnFrm->GetUpper();
                    if ( (pFtnFrm->Frm().*fnRect->fnTopDist)((aFtnContFrm->*fnRect->fnGetPrtBottom)()) > 0 )
                    {
                        pFtnFrm->_InvalidatePos();
                    }
                }
                if ( bLock )
                {
                    bool bUnlock = !pFtnFrm->IsBackMoveLocked();
                    pFtnFrm->LockBackMove();
                    pFtnFrm->Calc();
                    pCnt->Calc();
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pCnt = pFtn->ContainsAny();
                            continue;
                        }
                    }
                    if( bUnlock )
                    {
                        pFtnFrm->UnlockBackMove();
                        if( !pFtnFrm->Lower() &&
                            !pFtnFrm->IsColLocked() )
                        {
                            // #i49383#
                            OSL_ENSURE( pLastFtnFrm == pFtnFrm,
                                    "<SwFtnBossFrm::RearrangeFtns(..)> - <pLastFtnFrm> != <pFtnFrm>" );
                            pLastFtnFrm = 0L;
                            pFtnFrm->Cut();
                            SwFrm::DestroyFrm(pFtnFrm);
                        }
                    }
                }
                else
                {
                    pFtnFrm->Calc();
                    pCnt->Calc();
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pCnt = pFtn->ContainsAny();
                            continue;
                        }
                    }
                }
            }
            SwSectionFrm *pDel = NULL;
            if( pCnt->IsSctFrm() )
            {
                SwFrm* pTmp = static_cast<SwSectionFrm*>(pCnt)->ContainsAny();
                if( pTmp )
                {
                    pCnt = pTmp;
                    continue;
                }
                pDel = static_cast<SwSectionFrm*>(pCnt);
            }
            if ( pCnt->GetNext() )
                pCnt = pCnt->GetNext();
            else
            {
                pCnt = pCnt->FindNext();
                if ( pCnt )
                {
                    SwFtnFrm* pFtnFrm = pCnt->FindFtnFrm();
                    if( pFtnFrm->GetRef()->FindFtnBossFrm(
                        pFtnFrm->GetAttr()->GetFtn().IsEndNote() ) != this )
                        bMore = false;
                }
                else
                    bMore = false;
            }
            if( pDel )
            {
                bool bUnlockLastFtnFrmGuard = pLastFtnFrm && !pLastFtnFrm->IsColLocked();
                if (bUnlockLastFtnFrmGuard)
                    pLastFtnFrm->ColLock();
                pDel->Cut();
                if (bUnlockLastFtnFrmGuard)
                    pLastFtnFrm->ColUnlock();
                SwFrm::DestroyFrm(pDel);
            }
            if ( bMore )
            {
                // Go not further than to the provided footnote (if given)
                if ( pAttr &&
                     (::lcl_FindFtnPos( pDoc,
                                    pCnt->FindFtnFrm()->GetAttr()) > nFtnPos ) )
                    bMore = false;
            }
        } while ( bMore );
        // #i49383#
        if ( pLastFtnFrm )
        {
            if ( !bLock && bUnlockLastFtnFrm )
            {
                pLastFtnFrm->ColUnlock();
            }
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pLastFtnFrm->UnlockPosOfLowerObjs();
            pLastFtnFrm->Calc();
            if ( !bLock && bUnlockLastFtnFrm &&
                 !pLastFtnFrm->GetLower() &&
                 !pLastFtnFrm->IsColLocked() &&
                 !pLastFtnFrm->IsBackMoveLocked() )
            {
                pLastFtnFrm->Cut();
                SwFrm::DestroyFrm(pLastFtnFrm);
            }
        }
    }
}

void SwPageFrm::UpdateFtnNum()
{
    // page numbering only if set at the document
    if ( GetFmt()->GetDoc()->GetFtnInfo().eNum != FTNNUM_PAGE )
        return;

    SwLayoutFrm* pBody = FindBodyCont();
    if( !pBody || !pBody->Lower() )
        return;

    SwCntntFrm* pCntnt = pBody->ContainsCntnt();
    sal_uInt16 nNum = 0;

    while( pCntnt && pCntnt->FindPageFrm() == this )
    {
        if( static_cast<SwTxtFrm*>(pCntnt)->HasFtn() )
        {
            SwFtnBossFrm* pBoss = pCntnt->FindFtnBossFrm( true );
            if( pBoss->GetUpper()->IsSctFrm() &&
                static_cast<SwSectionFrm*>(pBoss->GetUpper())->IsOwnFtnNum() )
                pCntnt = static_cast<SwSectionFrm*>(pBoss->GetUpper())->FindLastCntnt();
            else
            {
                SwFtnFrm* pFtn = const_cast<SwFtnFrm*>(pBoss->FindFirstFtn( pCntnt ));
                while( pFtn )
                {
                    SwTxtFtn* pTxtFtn = pFtn->GetAttr();
                    if( !pTxtFtn->GetFtn().IsEndNote() &&
                         pTxtFtn->GetFtn().GetNumStr().isEmpty() &&
                         !pFtn->GetMaster() &&
                         (pTxtFtn->GetFtn().GetNumber() != ++nNum) )
                    {
                        pTxtFtn->SetNumber( nNum, OUString() );
                    }
                    if ( pFtn->GetNext() )
                        pFtn = static_cast<SwFtnFrm*>(pFtn->GetNext());
                    else
                    {
                        SwFtnBossFrm* pTmpBoss = pFtn->FindFtnBossFrm( true );
                        if( pTmpBoss )
                        {
                            SwPageFrm* pPage = pTmpBoss->FindPageFrm();
                            pFtn = NULL;
                            lcl_NextFtnBoss( pTmpBoss, pPage, false );
                            SwFtnContFrm *pCont = pTmpBoss ? pTmpBoss->FindNearestFtnCont() : NULL;
                            if ( pCont )
                                pFtn = static_cast<SwFtnFrm*>(pCont->Lower());
                        }
                    }
                    if( pFtn && pFtn->GetRef() != pCntnt )
                        pFtn = NULL;
                }
            }
        }
        pCntnt = pCntnt->FindNextCnt();
    }
}

void SwFtnBossFrm::SetFtnDeadLine( const SwTwips nDeadLine )
{
    SwFrm *pBody = FindBodyCont();
    pBody->Calc();

    SwFrm *pCont = FindFtnCont();
    const SwTwips nMax = nMaxFtnHeight;// current should exceed MaxHeight
    SWRECTFN( this )
    if ( pCont )
    {
        pCont->Calc();
        nMaxFtnHeight = -(pCont->Frm().*fnRect->fnBottomDist)( nDeadLine );
    }
    else
        nMaxFtnHeight = -(pBody->Frm().*fnRect->fnBottomDist)( nDeadLine );

    const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        nMaxFtnHeight += pBody->Grow( LONG_MAX, true );
    if ( IsInSct() )
        nMaxFtnHeight += FindSctFrm()->Grow( LONG_MAX, true );

    if ( nMaxFtnHeight < 0 )
        nMaxFtnHeight = 0;
    if ( nMax != LONG_MAX && nMaxFtnHeight > nMax )
        nMaxFtnHeight = nMax;
}

SwTwips SwFtnBossFrm::GetVarSpace() const
{
    // To not fall below 20% of the page height
    // (in contrast to MSOffice where footnotes can fill a whole column/page)

    const SwPageFrm* pPg = FindPageFrm();
    OSL_ENSURE( pPg || IsInSct(), "Footnote lost page" );

    const SwFrm *pBody = FindBodyCont();
    SwTwips nRet;
    if( pBody )
    {
        SWRECTFN( this )
        if( IsInSct() )
        {
            nRet = 0;
            SwTwips nTmp = (*fnRect->fnYDiff)( (pBody->*fnRect->fnGetPrtTop)(),
                                               (Frm().*fnRect->fnGetTop)() );
            const SwSectionFrm* pSect = FindSctFrm();
            //  Endnotes in a ftncontainer causes a deadline:
            // the bottom of the last contentfrm
            if( pSect->IsEndnAtEnd() ) // endnotes allowed?
            {
                OSL_ENSURE( !Lower() || !Lower()->GetNext() || Lower()->GetNext()->
                        IsFtnContFrm(), "FtnContainer expected" );
                const SwFtnContFrm* pCont = Lower() ?
                    static_cast<const SwFtnContFrm*>(Lower()->GetNext()) : 0;
                if( pCont )
                {
                    const SwFtnFrm* pFtn = static_cast<const SwFtnFrm*>(pCont->Lower());
                    while( pFtn)
                    {
                        if( pFtn->GetAttr()->GetFtn().IsEndNote() )
                        { // endnote found
                            const SwFrm* pFrm = static_cast<const SwLayoutFrm*>(Lower())->Lower();
                            if( pFrm )
                            {
                                while( pFrm->GetNext() )
                                    pFrm = pFrm->GetNext(); // last cntntfrm
                                nTmp += (*fnRect->fnYDiff)(
                                         (Frm().*fnRect->fnGetTop)(),
                                         (pFrm->Frm().*fnRect->fnGetBottom)() );
                            }
                            break;
                        }
                        pFtn = static_cast<const SwFtnFrm*>(pFtn->GetNext());
                    }
                }
            }
            if( nTmp < nRet )
                nRet = nTmp;
        }
        else
            nRet = - (pPg->Prt().*fnRect->fnGetHeight)()/5;
        nRet += (pBody->Frm().*fnRect->fnGetHeight)();
        if( nRet < 0 )
            nRet = 0;
    }
    else
        nRet = 0;
    if ( IsPageFrm() )
    {
        const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
        if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        nRet += BROWSE_HEIGHT - Frm().Height();
    }
    return nRet;
}

/** Obtain if pFrm's size adjustment should be processed
 *
 * For a page frame of columns directly below the page AdjustNeighbourhood() needs
 * to be called, or Grow()/ Shrink() for frame columns respectively.
 *
 * A column section is special, since if there is a footnote container in a column
 * and those footnotes are not collected, it is handled like a page frame.
 *
 * @see AdjustNeighbourhood()
 * @see Grow()
 * @see Shrink()
 */
sal_uInt8 SwFtnBossFrm::_NeighbourhoodAdjustment( const SwFrm* ) const
{
    sal_uInt8 nRet = NA_ONLY_ADJUST;
    if( GetUpper() && !GetUpper()->IsPageBodyFrm() )
    {
        // column sections need grow/shrink
        if( GetUpper()->IsFlyFrm() )
            nRet = NA_GROW_SHRINK;
        else
        {
            OSL_ENSURE( GetUpper()->IsSctFrm(), "NeighbourhoodAdjustment: Unexpected Upper" );
            if( !GetNext() && !GetPrev() )
                nRet = NA_GROW_ADJUST; // section with a single column (FtnAtEnd)
            else
            {
                const SwFrm* pTmp = Lower();
                OSL_ENSURE( pTmp, "NeighbourhoodAdjustment: Missing Lower()" );
                if( !pTmp->GetNext() )
                    nRet = NA_GROW_SHRINK;
                else if( !GetUpper()->IsColLocked() )
                    nRet = NA_ADJUST_GROW;
                OSL_ENSURE( !pTmp->GetNext() || pTmp->GetNext()->IsFtnContFrm(),
                        "NeighbourhoodAdjustment: Who's that guy?" );
            }
        }
    }
    return nRet;
}

void SwPageFrm::SetColMaxFtnHeight()
{
    SwLayoutFrm *pBody = FindBodyCont();
    if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
    {
        SwColumnFrm* pCol = static_cast<SwColumnFrm*>(pBody->Lower());
        do
        {
            pCol->SetMaxFtnHeight( GetMaxFtnHeight() );
            pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
        } while ( pCol );
    }
}

bool SwLayoutFrm::MoveLowerFtns( SwCntntFrm *pStart, SwFtnBossFrm *pOldBoss,
                                 SwFtnBossFrm *pNewBoss, const bool bFtnNums )
{
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( pDoc->GetFtnIdxs().empty() )
        return false;
    if( pDoc->GetFtnInfo().ePos == FTNPOS_CHAPTER &&
        ( !IsInSct() || !FindSctFrm()->IsFtnAtEnd() ) )
        return true;

    if ( !pNewBoss )
        pNewBoss = FindFtnBossFrm( true );
    if ( pNewBoss == pOldBoss )
        return false;

    bool bMoved = false;
    if( !pStart )
        pStart = ContainsCntnt();

    SwFtnFrms aFtnArr;

    while ( IsAnLower( pStart ) )
    {
        if ( static_cast<SwTxtFrm*>(pStart)->HasFtn() )
        {
            // OD 03.04.2003 #108446# - To avoid unnecessary moves of footnotes
            // use new parameter <_bCollectOnlyPreviousFtn> (4th parameter of
            // method <SwFtnBossFrm::CollectFtn(..)>) to control, that only
            // footnotes have to be collected, that are positioned before the
            // new dedicated footnote boss frame.
            pNewBoss->CollectFtns( pStart, pOldBoss, aFtnArr, true );
        }
        pStart = pStart->GetNextCntntFrm();
    }

    OSL_ENSURE( pOldBoss->IsInSct() == pNewBoss->IsInSct(),
            "MoveLowerFtns: Section confusion" );
    SwFtnFrms *pFtnArr;
    SwLayoutFrm* pNewChief = 0;
    SwLayoutFrm* pOldChief = 0;

    bool bFoundCandidate = false;
    if (pStart && pOldBoss->IsInSct())
    {
        pOldChief = pOldBoss->FindSctFrm();
        pNewChief = pNewBoss->FindSctFrm();
        bFoundCandidate = pOldChief != pNewChief;
    }

    if (bFoundCandidate)
    {
        pFtnArr = new SwFtnFrms;
        pOldChief = pOldBoss->FindFtnBossFrm( true );
        pNewChief = pNewBoss->FindFtnBossFrm( true );
        while( pOldChief->IsAnLower( pStart ) )
        {
            if ( static_cast<SwTxtFrm*>(pStart)->HasFtn() )
                static_cast<SwFtnBossFrm*>(pNewChief)->CollectFtns( pStart,
                                        pOldBoss, *pFtnArr );
            pStart = pStart->GetNextCntntFrm();
        }
        if( pFtnArr->empty() )
        {
            delete pFtnArr;
            pFtnArr = NULL;
        }
    }
    else
        pFtnArr = NULL;

    if ( !aFtnArr.empty() || pFtnArr )
    {
        if( !aFtnArr.empty() )
            pNewBoss->_MoveFtns( aFtnArr, true );
        if( pFtnArr )
        {
            static_cast<SwFtnBossFrm*>(pNewChief)->_MoveFtns( *pFtnArr, true );
            delete pFtnArr;
        }
        bMoved = true;

        // update FtnNum only at page change
        if ( bFtnNums )
        {
            SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
            SwPageFrm* pNewPage =pNewBoss->FindPageFrm();
            if( pOldPage != pNewPage )
            {
                pOldPage->UpdateFtnNum();
                pNewPage->UpdateFtnNum();
            }
        }
    }
    return bMoved;
}

bool SwCntntFrm::MoveFtnCntFwd( bool bMakePage, SwFtnBossFrm *pOldBoss )
{
    OSL_ENSURE( IsInFtn(), "no footnote." );
    SwLayoutFrm *pFtn = FindFtnFrm();

    // The first paragraph in the first footnote in the first column in the
    // sectionfrm at the top of the page has not to move forward, if the
    // columnbody is empty.
    if( pOldBoss->IsInSct() && !pOldBoss->GetIndPrev() && !GetIndPrev() &&
        !pFtn->GetPrev() )
    {
        SwLayoutFrm* pBody = pOldBoss->FindBodyCont();
        if( !pBody || !pBody->Lower() )
            return true;
    }

    //fix(9538): if the footnote has neighbors behind itself, remove them temporarily
    SwLayoutFrm *pNxt = static_cast<SwLayoutFrm*>(pFtn->GetNext());
    SwLayoutFrm *pLst = 0;
    while ( pNxt )
    {
        while ( pNxt->GetNext() )
            pNxt = static_cast<SwLayoutFrm*>(pNxt->GetNext());
        if ( pNxt == pLst )
            pNxt = 0;
        else
        {   pLst = pNxt;
            SwCntntFrm *pCnt = pNxt->ContainsCntnt();
            if( pCnt )
                pCnt->MoveFtnCntFwd( true, pOldBoss );
            pNxt = static_cast<SwLayoutFrm*>(pFtn->GetNext());
        }
    }

    bool bSamePage = true;
    SwLayoutFrm *pNewUpper =
                GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, true );

    if ( pNewUpper )
    {
        bool bSameBoss = true;
        SwFtnBossFrm * const pNewBoss = pNewUpper->FindFtnBossFrm();
        // Are we changing the column/page?
        if ( !( bSameBoss = (pNewBoss == pOldBoss) ) )
        {
            bSamePage = pOldBoss->FindPageFrm() == pNewBoss->FindPageFrm(); // page change?
            pNewUpper->Calc();
        }

        // The layout leaf of the footnote is either a footnote container or a footnote.
        // If it is a footnote and it has the same footnote reference like the old Upper,
        // then move the content inside of it.
        // If it is a container or the reference differs, create a new footnote and add
        // it into the container.
        // Create also a SectionFrame if currently in a area inside a footnote.
        SwFtnFrm* pTmpFtn = pNewUpper->IsFtnFrm() ? static_cast<SwFtnFrm*>(pNewUpper) : 0;
        if( !pTmpFtn )
        {
            OSL_ENSURE( pNewUpper->IsFtnContFrm(), "New Upper not a FtnCont.");
            SwFtnContFrm *pCont = static_cast<SwFtnContFrm*>(pNewUpper);

            // create footnote
            SwFtnFrm *pOld = FindFtnFrm();
            pTmpFtn = new SwFtnFrm( pOld->GetFmt()->GetDoc()->GetDfltFrmFmt(),
                                    pOld, pOld->GetRef(), pOld->GetAttr() );
            // chaining of footnotes
            if ( pOld->GetFollow() )
            {
                pTmpFtn->SetFollow( pOld->GetFollow() );
                pOld->GetFollow()->SetMaster( pTmpFtn );
            }
            pOld->SetFollow( pTmpFtn );
            pTmpFtn->SetMaster( pOld );
            SwFrm* pNx = pCont->Lower();
            if( pNx && pTmpFtn->GetAttr()->GetFtn().IsEndNote() )
                while(pNx && !static_cast<SwFtnFrm*>(pNx)->GetAttr()->GetFtn().IsEndNote())
                    pNx = pNx->GetNext();
            pTmpFtn->Paste( pCont, pNx );
            pTmpFtn->Calc();
        }
        OSL_ENSURE( pTmpFtn->GetAttr() == FindFtnFrm()->GetAttr(), "Wrong Footnote!" );
        // areas inside of footnotes get a special treatment
        SwLayoutFrm *pNewUp = pTmpFtn;
        if( IsInSct() )
        {
            SwSectionFrm* pSect = FindSctFrm();
            // area inside of a footnote (or only footnote in an area)?
            if( pSect->IsInFtn() )
            {
                if( pTmpFtn->Lower() && pTmpFtn->Lower()->IsSctFrm() &&
                    pSect->GetFollow() == static_cast<SwSectionFrm*>(pTmpFtn->Lower()) )
                    pNewUp = static_cast<SwSectionFrm*>(pTmpFtn->Lower());
                else
                {
                    pNewUp = new SwSectionFrm( *pSect, false );
                    pNewUp->InsertBefore( pTmpFtn, pTmpFtn->Lower() );
                    static_cast<SwSectionFrm*>(pNewUp)->Init();
                    pNewUp->Frm().Pos() = pTmpFtn->Frm().Pos();
                    pNewUp->Frm().Pos().Y() += 1; // for notifications

                    // If the section frame has a successor then the latter needs
                    // to be moved behind the new Follow of the section frame.
                    SwFrm* pTmp = pSect->GetNext();
                    if( pTmp )
                    {
                        SwFlowFrm* pTmpNxt;
                        if( pTmp->IsCntntFrm() )
                            pTmpNxt = static_cast<SwCntntFrm*>(pTmp);
                        else if( pTmp->IsSctFrm() )
                            pTmpNxt = static_cast<SwSectionFrm*>(pTmp);
                        else
                        {
                            OSL_ENSURE( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                            pTmpNxt = static_cast<SwTabFrm*>(pTmp);
                        }
                        pTmpNxt->MoveSubTree( pTmpFtn, pNewUp->GetNext() );
                    }
                }
            }
        }

        MoveSubTree( pNewUp, pNewUp->Lower() );

        if( !bSameBoss )
            Prepare( PREP_BOSS_CHGD );
    }
    return bSamePage;
}

SwSaveFtnHeight::SwSaveFtnHeight( SwFtnBossFrm *pBs, const SwTwips nDeadLine ) :
    pBoss( pBs ),
    nOldHeight( pBs->GetMaxFtnHeight() )
{
    pBoss->SetFtnDeadLine( nDeadLine );
    nNewHeight = pBoss->GetMaxFtnHeight();
}

SwSaveFtnHeight::~SwSaveFtnHeight()
{
    // If somebody tweaked the deadline meanwhile, we let it happen
    if ( nNewHeight == pBoss->GetMaxFtnHeight() )
        pBoss->nMaxFtnHeight = nOldHeight;
}

#ifdef DBG_UTIL
//JP 15.10.2001: in a non pro version test if the attribute has the same
//              meaning which his reference is

// Normally, the pRef member and the GetRefFromAttr() result has to be
// identically. Sometimes footnote will be moved from a master to its follow,
// but the GetRef() is called first, so we have to ignore a master/follow
// mismatch.

const SwCntntFrm* SwFtnFrm::GetRef() const
{
    const SwCntntFrm* pRefAttr = GetRefFromAttr();
    SAL_WARN_IF( pRef != pRefAttr && !pRef->IsAnFollow( pRefAttr )
            && !pRefAttr->IsAnFollow( pRef ),
            "sw", "access to deleted Frame? pRef != pAttr->GetRef()" );
    return pRef;
}

SwCntntFrm* SwFtnFrm::GetRef()
{
    const SwCntntFrm* pRefAttr = GetRefFromAttr();
    SAL_WARN_IF( pRef != pRefAttr && !pRef->IsAnFollow( pRefAttr )
            && !pRefAttr->IsAnFollow( pRef ),
            "sw", "access to deleted Frame? pRef != pAttr->GetRef()" );
    return pRef;
}
#endif

const SwCntntFrm* SwFtnFrm::GetRefFromAttr()  const
{
    SwFtnFrm* pThis = const_cast<SwFtnFrm*>(this);
    return pThis->GetRefFromAttr();
}

SwCntntFrm* SwFtnFrm::GetRefFromAttr()
{
    assert(pAttr && "invalid Attribute");
    SwTxtNode& rTNd = (SwTxtNode&)pAttr->GetTxtNode();
    SwPosition aPos( rTNd, SwIndex( &rTNd, pAttr->GetStart() ));
    SwCntntFrm* pCFrm = rTNd.getLayoutFrm( getRootFrm(), 0, &aPos, false );
    return pCFrm;
}

/** search for last content in the current footnote frame

    OD 2005-12-02 #i27138#
*/
SwCntntFrm* SwFtnFrm::FindLastCntnt()
{
    SwCntntFrm* pLastCntntFrm( 0L );

    // find last lower, which is a content frame or contains content.
    // hidden text frames, empty sections and empty tables have to be skipped.
    SwFrm* pLastLowerOfFtn( GetLower() );
    SwFrm* pTmpLastLower( pLastLowerOfFtn );
    while ( pTmpLastLower && pTmpLastLower->GetNext() )
    {
        pTmpLastLower = pTmpLastLower->GetNext();
        if ( ( pTmpLastLower->IsTxtFrm() &&
               !static_cast<SwTxtFrm*>(pTmpLastLower)->IsHiddenNow() ) ||
             ( pTmpLastLower->IsSctFrm() &&
               static_cast<SwSectionFrm*>(pTmpLastLower)->GetSection() &&
               static_cast<SwSectionFrm*>(pTmpLastLower)->ContainsCntnt() ) ||
             ( pTmpLastLower->IsTabFrm() &&
               static_cast<SwTabFrm*>(pTmpLastLower)->ContainsCntnt() ) )
        {
            pLastLowerOfFtn = pTmpLastLower;
        }
    }

    // determine last content frame depending on type of found last lower.
    if ( pLastLowerOfFtn && pLastLowerOfFtn->IsTabFrm() )
    {
        pLastCntntFrm = static_cast<SwTabFrm*>(pLastLowerOfFtn)->FindLastCntnt();
    }
    else if ( pLastLowerOfFtn && pLastLowerOfFtn->IsSctFrm() )
    {
        pLastCntntFrm = static_cast<SwSectionFrm*>(pLastLowerOfFtn)->FindLastCntnt();
    }
    else
    {
        pLastCntntFrm = dynamic_cast<SwCntntFrm*>(pLastLowerOfFtn);
    }

    return pLastCntntFrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
