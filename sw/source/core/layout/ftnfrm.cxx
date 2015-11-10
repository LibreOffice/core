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

/// Search the position of an attribute in the FootnoteArray at the document,
/// because all footnotes are located there, ordered by their index.
static sal_uLong lcl_FindFootnotePos( const SwDoc *pDoc, const SwTextFootnote *pAttr )
{
    const SwFootnoteIdxs &rFootnoteIdxs = pDoc->GetFootnoteIdxs();

    SwTextFootnote* pBla = const_cast<SwTextFootnote*>(pAttr);
    SwFootnoteIdxs::const_iterator it = rFootnoteIdxs.find( pBla );
    if ( it != rFootnoteIdxs.end() )
    {
        sal_uLong nRet = it - rFootnoteIdxs.begin();
        if( pAttr->GetFootnote().IsEndNote() )
            return nRet + ENDNOTE;
        return nRet;
    }
    OSL_ENSURE( !pDoc, "FootnotePos not found." );
    return 0;
}

bool SwFootnoteFrm::operator<( const SwTextFootnote* pTextFootnote ) const
{
    const SwDoc* pDoc = GetFormat()->GetDoc();
    OSL_ENSURE( pDoc, "SwFootnoteFrm: Missing doc!" );
    return lcl_FindFootnotePos( pDoc, GetAttr() ) <
           lcl_FindFootnotePos( pDoc, pTextFootnote );
}

/*
|*
|*  bool lcl_NextFootnoteBoss( SwFootnoteBossFrm* pBoss, SwPageFrm* pPage)
|*  sets pBoss on the next SwFootnoteBossFrm, which can either be a column
|*  or a page (without columns). If the page changes meanwhile,
|*  pPage contains the new page and this function returns true.
|*
|*/

static bool lcl_NextFootnoteBoss( SwFootnoteBossFrm* &rpBoss, SwPageFrm* &rpPage,
    bool bDontLeave )
{
    if( rpBoss->IsColumnFrm() )
    {
        if( rpBoss->GetNext() )
        {
            rpBoss = static_cast<SwFootnoteBossFrm*>(rpBoss->GetNext()); //next column
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
                rpPage = nullptr;
                rpBoss = nullptr;
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
            rpBoss = static_cast<SwFootnoteBossFrm*>(pBody->Lower()); // first column
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

SwFootnoteContFrm::SwFootnoteContFrm( SwFrameFormat *pFormat, SwFrm* pSib ):
    SwLayoutFrm( pFormat, pSib )
{
    mnFrmType = FRM_FTNCONT;
}


// lcl_Undersize(..) walks over a SwFrm and its contents
// and returns the sum of all requested TextFrm magnifications.

static long lcl_Undersize( const SwFrm* pFrm )
{
    long nRet = 0;
    SWRECTFN( pFrm )
    if( pFrm->IsTextFrm() )
    {
        if( static_cast<const SwTextFrm*>(pFrm)->IsUndersized() )
        {
            // Does this TextFrm would like to be a little bit bigger?
            nRet = static_cast<const SwTextFrm*>(pFrm)->GetParHeight() -
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
void SwFootnoteContFrm::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    // calculate total border, only one distance to the top
    const SwPageFrm* pPage = FindPageFrm();
    const SwPageFootnoteInfo &rInf = pPage->GetPageDesc()->GetFootnoteInfo();
    const SwTwips nBorder = rInf.GetTopDist() + rInf.GetBottomDist() +
                            rInf.GetLineWidth();
    SWRECTFN( this )
    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = true;
        (Prt().*fnRect->fnSetTop)( nBorder );
        (Prt().*fnRect->fnSetWidth)( (Frm().*fnRect->fnGetWidth)() );
        (Prt().*fnRect->fnSetHeight)((Frm().*fnRect->fnGetHeight)() - nBorder );
        if( (Prt().*fnRect->fnGetHeight)() < 0 && !pPage->IsFootnotePage() )
            mbValidSize = false;
    }

    if ( !mbValidSize )
    {
        bool bGrow = pPage->IsFootnotePage();
        if( bGrow )
        {
            const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : nullptr;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bGrow = false;
        }
        if( bGrow )
                Grow( LONG_MAX );
        else
        {
            // VarSize is determined based on the content plus the borders
            SwTwips nRemaining = 0;
            SwFrm *pFrm = m_pLower;
            while ( pFrm )
            {   // lcl_Undersize(..) respects (recursively) TextFrms, which
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

SwTwips SwFootnoteContFrm::GrowFrm( SwTwips nDist, bool bTst, bool )
{
    // No check if FixSize since FootnoteContainer are variable up to their max. height.
    // If the max. height is LONG_MAX, take as much space as needed.
    // If the page is a special footnote page, take also as much as possible.
    assert(GetUpper() && GetUpper()->IsFootnoteBossFrm());

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetHeight)() > 0 &&
         nDist > ( LONG_MAX - (Frm().*fnRect->fnGetHeight)() ) )
        nDist = LONG_MAX - (Frm().*fnRect->fnGetHeight)();

    SwFootnoteBossFrm *pBoss = static_cast<SwFootnoteBossFrm*>(GetUpper());
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
    const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : nullptr;
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    SwPageFrm *pPage = pBoss->FindPageFrm();
    if ( bBrowseMode || !pPage->IsFootnotePage() )
    {
        if ( pBoss->GetMaxFootnoteHeight() != LONG_MAX )
        {
            nDist = std::min( nDist, pBoss->GetMaxFootnoteHeight()
                         - (Frm().*fnRect->fnGetHeight)() );
            if ( nDist <= 0 )
                return 0L;
        }
        // FootnoteBoss also influences the max value
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
                SwFrm* pFootnote = Lower();
                if( pFootnote )
                {
                    while( pFootnote->GetNext() )
                        pFootnote = pFootnote->GetNext();
                    if( static_cast<SwFootnoteFrm*>(pFootnote)->GetAttr()->GetFootnote().IsEndNote() )
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

SwTwips SwFootnoteContFrm::ShrinkFrm( SwTwips nDiff, bool bTst, bool bInfo )
{
    SwPageFrm *pPage = FindPageFrm();
    bool bShrink = false;
    if ( pPage )
    {
        if( !pPage->IsFootnotePage() )
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

SwFootnoteFrm::SwFootnoteFrm( SwFrameFormat *pFormat, SwFrm* pSib, SwContentFrm *pCnt, SwTextFootnote *pAt ):
    SwLayoutFrm( pFormat, pSib ),
    pFollow( nullptr ),
    pMaster( nullptr ),
    pRef( pCnt ),
    pAttr( pAt ),
    bBackMoveLocked( false ),
    // #i49383#
    mbUnlockPosOfLowerObjs( true )
{
    mnFrmType = FRM_FTN;
}

void SwFootnoteFrm::InvalidateNxtFootnoteCnts( SwPageFrm *pPage )
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
SwTwips SwFootnoteFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    static sal_uInt16 nNum = USHRT_MAX;
    SwTextFootnote* pTextFootnote = GetAttr();
    if ( pTextFootnote->GetFootnote().GetNumber() == nNum )
    {
        int bla = 5;
        (void)bla;

    }
    return SwLayoutFrm::GrowFrm( nDist, bTst, bInfo );
}

SwTwips SwFootnoteFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    static sal_uInt16 nNum = USHRT_MAX;
    if( nNum != USHRT_MAX )
    {
        SwTextFootnote* pTextFootnote = GetAttr();
        if( pTextFootnote->GetFootnote().GetNumber() == nNum )
        {
            int bla = 5;
            (void)bla;
        }
    }
    return SwLayoutFrm::ShrinkFrm( nDist, bTst, bInfo );
}
#endif

void SwFootnoteFrm::Cut()
{
    if ( GetNext() )
        GetNext()->InvalidatePos();
    else if ( GetPrev() )
        GetPrev()->SetRetouche();

    // first move then shrink Upper
    SwLayoutFrm *pUp = GetUpper();

    // correct chaining
    SwFootnoteFrm *pFootnote = this;
    if ( pFootnote->GetFollow() )
        pFootnote->GetFollow()->SetMaster( pFootnote->GetMaster() );
    if ( pFootnote->GetMaster() )
        pFootnote->GetMaster()->SetFollow( pFootnote->GetFollow() );
    pFootnote->SetFollow( nullptr );
    pFootnote->SetMaster( nullptr );

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
                if( pBody && !pBody->ContainsContent() )
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

void SwFootnoteFrm::Paste(  SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "no parent in Paste." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is ContentFrm." );
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
    SwDoc *pDoc = GetFormat()->GetDoc();
    if ( GetPrev() )
    {
        OSL_ENSURE( lcl_FindFootnotePos( pDoc, static_cast<SwFootnoteFrm*>(GetPrev())->GetAttr() ) <=
                lcl_FindFootnotePos( pDoc, GetAttr() ), "Prev is not FootnotePrev" );
    }
    if ( GetNext() )
    {
        OSL_ENSURE( lcl_FindFootnotePos( pDoc, GetAttr() ) <=
                lcl_FindFootnotePos( pDoc, static_cast<SwFootnoteFrm*>(GetNext())->GetAttr() ),
                "Next is not FootnoteNext" );
    }
#endif
    InvalidateNxtFootnoteCnts( pPage );
}

/// Return the next layout leaf in that the frame can be moved.
/// New pages will only be created if specified by the parameter.
SwLayoutFrm *SwFrm::GetNextFootnoteLeaf( MakePageType eMakePage )
{
    SwFootnoteBossFrm *pOldBoss = FindFootnoteBossFrm();
    SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
    SwPageFrm* pPage;
    SwFootnoteBossFrm *pBoss = pOldBoss->IsColumnFrm() ?
        static_cast<SwFootnoteBossFrm*>(pOldBoss->GetNext()) : nullptr; // next column, if existing
    if( pBoss )
        pPage = nullptr;
    else
    {
        if( pOldBoss->GetUpper()->IsSctFrm() )
        {   // this can only be in a column area
            SwLayoutFrm* pNxt = pOldBoss->GetNextSctLeaf( eMakePage );
            if( pNxt )
            {
                OSL_ENSURE( pNxt->IsColBodyFrm(), "GetNextFootnoteLeaf: Funny Leaf" );
                pBoss = static_cast<SwFootnoteBossFrm*>(pNxt->GetUpper());
                pPage = pBoss->FindPageFrm();
            }
            else
                return nullptr;
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
    // However, if there are unwanted empty columns/pages between Footnote and Follow,
    // create another Follow on the next best column/page and the rest will sort itself out.
    SwFootnoteFrm *pFootnote = FindFootnoteFrm();
    if ( pFootnote && pFootnote->GetFollow() )
    {
        SwFootnoteBossFrm* pTmpBoss = pFootnote->GetFollow()->FindFootnoteBossFrm();
        // Following cases will be handled:
        // 1. both "FootnoteBoss"es are neighboring columns/pages
        // 2. the new one is the first column of a neighboring page
        // 3. the new one is the first column in a section of the next page
        while( pTmpBoss != pBoss && pTmpBoss && !pTmpBoss->GetPrev() )
            pTmpBoss = pTmpBoss->GetUpper()->FindFootnoteBossFrm();
        if( pTmpBoss == pBoss )
            return pFootnote->GetFollow();
    }

    // If no pBoss could be found or it is a "wrong" page, we need a new page.
    if ( !pBoss || ( pPage && pPage->IsEndNotePage() && !pOldPage->IsEndNotePage() ) )
    {
        if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
        {
            pBoss = InsertPage( pOldPage, pOldPage->IsFootnotePage() );
            static_cast<SwPageFrm*>(pBoss)->SetEndNotePage( pOldPage->IsEndNotePage() );
        }
        else
            return nullptr;
    }
    if( pBoss->IsPageFrm() )
    {
        // If this page has columns, then go to the first one
        SwLayoutFrm* pLay = pBoss->FindBodyCont();
        if( pLay && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
            pBoss = static_cast<SwFootnoteBossFrm*>(pLay->Lower());
    }
    // found column/page - add myself
    SwFootnoteContFrm *pCont = pBoss->FindFootnoteCont();
    if ( !pCont && pBoss->GetMaxFootnoteHeight() &&
         ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        pCont = pBoss->MakeFootnoteCont();
    return pCont;
}

/// Get the preceding layout leaf in that the frame can be moved.
SwLayoutFrm *SwFrm::GetPrevFootnoteLeaf( MakePageType eMakeFootnote )
{
    // The predecessor of a footnote is (if possible)
    // the master of the chain of the footnote.
    SwFootnoteFrm *pFootnote = FindFootnoteFrm();
    SwLayoutFrm *pRet = pFootnote->GetMaster();

    SwFootnoteBossFrm* pOldBoss = FindFootnoteBossFrm();
    SwPageFrm *pOldPage = pOldBoss->FindPageFrm();

    if ( !pOldBoss->GetPrev() && !pOldPage->GetPrev() )
        return pRet; // there is neither a predecessor column nor page

    if ( !pRet )
    {
        bool bEndn = pFootnote->GetAttr()->GetFootnote().IsEndNote();
        SwFrm* pTmpRef = nullptr;
        if( bEndn && pFootnote->IsInSct() )
        {
            SwSectionFrm* pSect = pFootnote->FindSctFrm();
            if( pSect->IsEndnAtEnd() )
                pTmpRef = pSect->FindLastContent( FINDMODE_LASTCNT );
        }
        if( !pTmpRef )
            pTmpRef = pFootnote->GetRef();
        SwFootnoteBossFrm* pStop = pTmpRef->FindFootnoteBossFrm( !bEndn );

        const sal_uInt16 nNum = pStop->GetPhyPageNum();

        // Do not leave the corresponding page if the footnote should
        // be shown at the document ending or the footnote is an endnote.
        const bool bEndNote = pOldPage->IsEndNotePage();
        const bool bFootnoteEndDoc = pOldPage->IsFootnotePage();
        SwFootnoteBossFrm* pNxtBoss = pOldBoss;
        SwSectionFrm *pSect = pNxtBoss->GetUpper()->IsSctFrm() ?
                              static_cast<SwSectionFrm*>(pNxtBoss->GetUpper()) : nullptr;

        do
        {
            if( pNxtBoss->IsColumnFrm() && pNxtBoss->GetPrev() )
                pNxtBoss = static_cast<SwFootnoteBossFrm*>(pNxtBoss->GetPrev());  // one column backwards
            else // one page backwards
            {
                SwLayoutFrm* pBody = nullptr;
                if( pSect )
                {
                    if( pSect->IsFootnoteLock() )
                    {
                        if( pNxtBoss == pOldBoss )
                            return nullptr;
                        pStop = pNxtBoss;
                    }
                    else
                    {
                        pSect = pSect->FindMaster();
                        if( !pSect || !pSect->Lower() )
                            return nullptr;
                        OSL_ENSURE( pSect->Lower()->IsColumnFrm(),
                                "GetPrevFootnoteLeaf: Where's the column?" );
                        pNxtBoss = static_cast<SwFootnoteBossFrm*>(pSect->Lower());
                        pBody = pSect;
                    }
                }
                else
                {
                    SwPageFrm* pPage = static_cast<SwPageFrm*>(pNxtBoss->FindPageFrm()->GetPrev());
                    if( !pPage || pPage->GetPhyPageNum() < nNum ||
                        bEndNote != pPage->IsEndNotePage() || bFootnoteEndDoc != pPage->IsFootnotePage() )
                        return nullptr; // no further pages found
                    pNxtBoss = pPage;
                    pBody = pPage->FindBodyCont();
                }
                // We have the previous page, we might need to find the last column of it
                if( pBody )
                {
                    if ( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
                    {
                        pNxtBoss = static_cast<SwFootnoteBossFrm*>(pBody->GetLastLower());
                    }
                }
            }
            SwFootnoteContFrm *pCont = pNxtBoss->FindFootnoteCont();
            if ( pCont )
            {
                pRet = pCont;
                break;
            }
            if ( pStop == pNxtBoss )
            {
                // Reached the column/page of the reference.
                // Try to add a container and paste our content.
                if ( eMakeFootnote == MAKEPAGE_FTN && pNxtBoss->GetMaxFootnoteHeight() )
                    pRet = pNxtBoss->MakeFootnoteCont();
                break;
            }
        } while( !pRet );
    }
    if ( pRet )
    {
        const SwFootnoteBossFrm* pNewBoss = pRet->FindFootnoteBossFrm();
        bool bJump = false;
        if( pOldBoss->IsColumnFrm() && pOldBoss->GetPrev() ) // a previous column exists
            bJump = pOldBoss->GetPrev() != static_cast<SwFrm const *>(pNewBoss); // did we chose it?
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

bool SwFrm::IsFootnoteAllowed() const
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

void SwRootFrm::UpdateFootnoteNums()
{
    // page numbering only if set at the document
    if ( GetFormat()->GetDoc()->GetFootnoteInfo().eNum == FTNNUM_PAGE )
    {
        SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
        while ( pPage && !pPage->IsFootnotePage() )
        {
            pPage->UpdateFootnoteNum();
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
        }
    }
}

/// remove all footnotes (not the references) and all footnote pages
void sw_RemoveFootnotes( SwFootnoteBossFrm* pBoss, bool bPageOnly, bool bEndNotes )
{
    do
    {
        SwFootnoteContFrm *pCont = pBoss->FindFootnoteCont();
        if ( pCont )
        {
            SwFootnoteFrm *pFootnote = static_cast<SwFootnoteFrm*>(pCont->Lower());
            OSL_ENSURE( pFootnote, "Footnote content without footnote." );
            if ( bPageOnly )
                while ( pFootnote->GetMaster() )
                    pFootnote = pFootnote->GetMaster();
            do
            {
                SwFootnoteFrm *pNxt = static_cast<SwFootnoteFrm*>(pFootnote->GetNext());
                if ( !pFootnote->GetAttr()->GetFootnote().IsEndNote() ||
                        bEndNotes )
                {
                    pFootnote->GetRef()->Prepare( PREP_FTN, static_cast<void*>(pFootnote->GetAttr()) );
                    if ( bPageOnly && !pNxt )
                        pNxt = pFootnote->GetFollow();
                    pFootnote->Cut();
                    SwFrm::DestroyFrm(pFootnote);
                }
                pFootnote = pNxt;

            } while ( pFootnote );
        }
        if( !pBoss->IsInSct() )
        {
            // A sectionframe with the Footnote/EndnAtEnd-flags may contain
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
                        sw_RemoveFootnotes( static_cast<SwColumnFrm*>(static_cast<SwSectionFrm*>(pLow)->Lower()),
                            bPageOnly, bEndNotes );
                    pLow = pLow->GetNext();
                }
            }
        }
        // is there another column?
        pBoss = pBoss->IsColumnFrm() ? static_cast<SwColumnFrm*>(pBoss->GetNext()) : nullptr;
    } while( pBoss );
}

void SwRootFrm::RemoveFootnotes( SwPageFrm *pPage, bool bPageOnly, bool bEndNotes )
{
    if ( !pPage )
        pPage = static_cast<SwPageFrm*>(Lower());

    do
    {   // On columned pages we have to clean up in all columns
        SwFootnoteBossFrm* pBoss;
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = static_cast<SwFootnoteBossFrm*>(pBody->Lower()); // the first column
        else
            pBoss = pPage; // no columns
        sw_RemoveFootnotes( pBoss, bPageOnly, bEndNotes );
        if ( !bPageOnly )
        {
            if ( pPage->IsFootnotePage() &&
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
void SwRootFrm::CheckFootnotePageDescs( bool bEndNote )
{
    SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
    while ( pPage && !pPage->IsFootnotePage() )
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
 * The frame format (FrameFormat) is always the default frame format.
 *
 * @return footnote container frame
 */
SwFootnoteContFrm *SwFootnoteBossFrm::MakeFootnoteCont()
{
    SAL_WARN_IF(FindFootnoteCont(), "sw.core", "footnote container exists already");

    SwFootnoteContFrm *pNew = new SwFootnoteContFrm( GetFormat()->GetDoc()->GetDfltFrameFormat(), this );
    SwLayoutFrm *pLay = FindBodyCont();
    pNew->Paste( this, pLay->GetNext() );
    return pNew;
}

SwFootnoteContFrm *SwFootnoteBossFrm::FindFootnoteCont()
{
    SwFrm *pFrm = Lower();
    while( pFrm && !pFrm->IsFootnoteContFrm() )
        pFrm = pFrm->GetNext();

#if OSL_DEBUG_LEVEL > 0
    if ( pFrm )
    {
        SwFrm *pFootnote = pFrm->GetLower();
        OSL_ENSURE( pFootnote, "Content without footnote." );
        while ( pFootnote )
        {
            OSL_ENSURE( pFootnote->IsFootnoteFrm(), "Neighbor of footnote is not a footnote." );
            pFootnote = pFootnote->GetNext();
        }
    }
#endif

    return static_cast<SwFootnoteContFrm*>(pFrm);
}

/// Search the next available footnote container.
SwFootnoteContFrm *SwFootnoteBossFrm::FindNearestFootnoteCont( bool bDontLeave )
{
    SwFootnoteContFrm *pCont = nullptr;
    if ( !GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
    {
        pCont = FindFootnoteCont();
        if ( !pCont )
        {
            SwPageFrm *pPage = FindPageFrm();
            SwFootnoteBossFrm* pBoss = this;
            bool bEndNote = pPage->IsEndNotePage();
            do
            {
                bool bChgPage = lcl_NextFootnoteBoss( pBoss, pPage, bDontLeave );
                // Found another boss? When changing pages, also the endnote flag must match.
                if( pBoss && ( !bChgPage || pPage->IsEndNotePage() == bEndNote ) )
                    pCont = pBoss->FindFootnoteCont();
            } while ( !pCont && pPage );
        }
    }
    return pCont;
}

SwFootnoteFrm *SwFootnoteBossFrm::FindFirstFootnote()
{
    // search for the nearest footnote container
    SwFootnoteContFrm *pCont = FindNearestFootnoteCont();
    if ( !pCont )
        return nullptr;

    // Starting from the first footnote, search the first
    // footnote that is referenced by the current column/page

    SwFootnoteFrm *pRet = static_cast<SwFootnoteFrm*>(pCont->Lower());
    const sal_uInt16 nRefNum = FindPageFrm()->GetPhyPageNum();
    const sal_uInt16 nRefCol = lcl_ColumnNum( this );
    sal_uInt16 nPgNum, nColNum; // page number, column number
    SwFootnoteBossFrm* pBoss;
    SwPageFrm* pPage;
    if( pRet )
    {
        pBoss = pRet->GetRef()->FindFootnoteBossFrm();
        OSL_ENSURE( pBoss, "FindFirstFootnote: No boss found" );
        if( !pBoss )
            return nullptr; // ?There must be a bug, but no GPF
        pPage = pBoss->FindPageFrm();
        nPgNum = pPage->GetPhyPageNum();
        if ( nPgNum == nRefNum )
        {
            nColNum = lcl_ColumnNum( pBoss );
            if( nColNum == nRefCol )
                return pRet; // found
            else if( nColNum > nRefCol )
                return nullptr; // at least one column too far
        }
        else if ( nPgNum > nRefNum )
            return nullptr;    // at least one column too far
    }
    else
        return nullptr;
    // Done if Ref is on a subsequent page or on the same page in a subsequent column

    do
    {
        while ( pRet->GetFollow() )
            pRet = pRet->GetFollow();

        SwFootnoteFrm *pNxt = static_cast<SwFootnoteFrm*>(pRet->GetNext());
        if ( !pNxt )
        {
            pBoss = pRet->FindFootnoteBossFrm();
            pPage = pBoss->FindPageFrm();
            lcl_NextFootnoteBoss( pBoss, pPage, false ); // next FootnoteBoss
            pCont = pBoss ? pBoss->FindNearestFootnoteCont() : nullptr;
            if ( pCont )
                pNxt = static_cast<SwFootnoteFrm*>(pCont->Lower());
        }
        if ( pNxt )
        {
            pRet = pNxt;
            pBoss = pRet->GetRef()->FindFootnoteBossFrm();
            pPage = pBoss->FindPageFrm();
            nPgNum = pPage->GetPhyPageNum();
            if ( nPgNum == nRefNum )
            {
                nColNum = lcl_ColumnNum( pBoss );
                if( nColNum == nRefCol )
                    break; // found
                else if( nColNum > nRefCol )
                    pRet = nullptr; // at least one column too far
            }
            else if ( nPgNum > nRefNum )
                pRet = nullptr;   // at least a page too far
        }
        else
            pRet = nullptr;   // there is none
    } while( pRet );
    return pRet;
}

/// Get the first footnote of a given content
const SwFootnoteFrm *SwFootnoteBossFrm::FindFirstFootnote( SwContentFrm *pCnt ) const
{
    const SwFootnoteFrm *pRet = const_cast<SwFootnoteBossFrm*>(this)->FindFirstFootnote();
    if ( pRet )
    {
        const sal_uInt16 nColNum = lcl_ColumnNum( this );
        const sal_uInt16 nPageNum = GetPhyPageNum();
        while ( pRet && (pRet->GetRef() != pCnt) )
        {
            while ( pRet->GetFollow() )
                pRet = pRet->GetFollow();

            if ( pRet->GetNext() )
                pRet = static_cast<const SwFootnoteFrm*>(pRet->GetNext());
            else
            {   SwFootnoteBossFrm *pBoss = const_cast<SwFootnoteBossFrm*>(pRet->FindFootnoteBossFrm());
                SwPageFrm *pPage = pBoss->FindPageFrm();
                lcl_NextFootnoteBoss( pBoss, pPage, false ); // next FootnoteBoss
                SwFootnoteContFrm *pCont = pBoss ? pBoss->FindNearestFootnoteCont() : nullptr;
                pRet = pCont ? static_cast<SwFootnoteFrm*>(pCont->Lower()) : nullptr;
            }
            if ( pRet )
            {
                const SwFootnoteBossFrm* pBoss = pRet->GetRef()->FindFootnoteBossFrm();
                if( pBoss->GetPhyPageNum() != nPageNum ||
                    nColNum != lcl_ColumnNum( pBoss ) )
                pRet = nullptr;
            }
        }
    }
    return pRet;
}

void SwFootnoteBossFrm::ResetFootnote( const SwFootnoteFrm *pCheck )
{
    // Destroy the incarnations of footnotes to an attribute, if they don't
    // belong to pAssumed
    OSL_ENSURE( !pCheck->GetMaster(), "given master is not a Master." );

    SwNodeIndex aIdx( *pCheck->GetAttr()->GetStartNode(), 1 );
    SwContentNode *pNd = aIdx.GetNode().GetContentNode();
    if ( !pNd )
        pNd = pCheck->GetFormat()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    SwIterator<SwFrm,SwContentNode> aIter( *pNd );
    SwFrm* pFrm = aIter.First();
    while( pFrm )
    {
            if( pFrm->getRootFrm() == pCheck->getRootFrm() )
            {
            SwFrm *pTmp = pFrm->GetUpper();
            while ( pTmp && !pTmp->IsFootnoteFrm() )
                pTmp = pTmp->GetUpper();

            SwFootnoteFrm *pFootnote = static_cast<SwFootnoteFrm*>(pTmp);
            while ( pFootnote && pFootnote->GetMaster() )
                pFootnote = pFootnote->GetMaster();
            if ( pFootnote != pCheck )
            {
                while ( pFootnote )
                {
                    SwFootnoteFrm *pNxt = pFootnote->GetFollow();
                    pFootnote->Cut();
                    SwFrm::DestroyFrm(pFootnote);
                    pFootnote = pNxt;
                }
            }
        }

        pFrm = aIter.Next();
    }
}

void SwFootnoteBossFrm::InsertFootnote( SwFootnoteFrm* pNew )
{
    // Place the footnote in front of the footnote whose attribute
    // is in front of the new one (get position via the Doc).
    // If there is no footnote in this footnote-boss yet, create a new container.
    // If there is a container but no footnote for this footnote-boss yet, place
    // the footnote behind the last footnote of the closest previous column/page.

    ResetFootnote( pNew );
    SwFootnoteFrm *pSibling = FindFirstFootnote();
    bool bDontLeave = false;

    // Ok, a sibling has been found, but is the sibling in an acceptable
    // environment?
    if( IsInSct() )
    {
        SwSectionFrm* pMySect = ImplFindSctFrm();
        bool bEndnt = pNew->GetAttr()->GetFootnote().IsEndNote();
        if( bEndnt )
        {
            const SwSectionFormat* pEndFormat = pMySect->GetEndSectFormat();
            bDontLeave = nullptr != pEndFormat;
            if( pSibling )
            {
                if( pEndFormat )
                {
                    if( !pSibling->IsInSct() ||
                        !pSibling->ImplFindSctFrm()->IsDescendantFrom( pEndFormat ) )
                        pSibling = nullptr;
                }
                else if( pSibling->IsInSct() )
                    pSibling = nullptr;
            }
        }
        else
        {
            bDontLeave = pMySect->IsFootnoteAtEnd();
            if( pSibling )
            {
                if( pMySect->IsFootnoteAtEnd() )
                {
                    if( !pSibling->IsInSct() ||
                        !pMySect->IsAnFollow( pSibling->ImplFindSctFrm() ) )
                        pSibling = nullptr;
                }
                else if( pSibling->IsInSct() )
                    pSibling = nullptr;
            }
        }
    }

    if( pSibling && pSibling->FindPageFrm()->IsEndNotePage() !=
        FindPageFrm()->IsEndNotePage() )
        pSibling = nullptr;

    // use the Doc to find out the position
    SwDoc *pDoc = GetFormat()->GetDoc();
    const sal_uLong nStPos = ::lcl_FindFootnotePos( pDoc, pNew->GetAttr() );

    sal_uLong nCmpPos = 0;
    sal_uLong nLastPos = 0;
    SwFootnoteContFrm *pParent = nullptr;
    if( pSibling )
    {
        nCmpPos = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );
        if( nCmpPos > nStPos )
            pSibling = nullptr;
    }

    if ( !pSibling )
    {   pParent = FindFootnoteCont();
        if ( !pParent )
        {
            // There is no footnote container yet. Before creating one, keep in mind that
            // there might exist another following footnote that must be placed before the
            // new inserted one e.g. because it was divided over multiple pages etc.
            pParent = FindNearestFootnoteCont( bDontLeave );
            if ( pParent )
            {
                SwFootnoteFrm *pFootnote = static_cast<SwFootnoteFrm*>(pParent->Lower());
                if ( pFootnote )
                {

                    nCmpPos = ::lcl_FindFootnotePos( pDoc, pFootnote->GetAttr() );
                    if ( nCmpPos > nStPos )
                        pParent = nullptr;
                }
                else
                    pParent = nullptr;
            }
        }
        if ( !pParent )
            // here, we are sure that we can create a footnote container
            pParent = MakeFootnoteCont();
        else
        {
            // Based on the first footnote below the Parent, search for the first footnote whose
            // index is after the index of the newly inserted, to place the new one correctly
            pSibling = static_cast<SwFootnoteFrm*>(pParent->Lower());
            if ( !pSibling )
            { OSL_ENSURE( false, "Could not find space for footnote.");
                return;
            }
            nCmpPos  = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );

            SwFootnoteBossFrm *pNxtB; // remember the last one to not
            SwFootnoteFrm  *pLastSib = nullptr;    // go too far.

            while ( pSibling && nCmpPos <= nStPos )
            {
                pLastSib = pSibling; // potential candidate
                nLastPos = nCmpPos;

                while ( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();

                if ( pSibling->GetNext() )
                {
                    pSibling = static_cast<SwFootnoteFrm*>(pSibling->GetNext());
                    OSL_ENSURE( !pSibling->GetMaster() || ( ENDNOTE > nStPos &&
                            pSibling->GetAttr()->GetFootnote().IsEndNote() ),
                            "InsertFootnote: Master expected I" );
                }
                else
                {
                    pNxtB = pSibling->FindFootnoteBossFrm();
                    SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                    bool bEndNote = pSibPage->IsEndNotePage();
                    bool bChgPage = lcl_NextFootnoteBoss( pNxtB, pSibPage, bDontLeave );
                    // When changing pages, also the endnote flag must match.
                    SwFootnoteContFrm *pCont = pNxtB && ( !bChgPage ||
                        pSibPage->IsEndNotePage() == bEndNote )
                        ? pNxtB->FindNearestFootnoteCont( bDontLeave ) : nullptr;
                    if( pCont )
                        pSibling = static_cast<SwFootnoteFrm*>(pCont->Lower());
                    else // no further FootnoteContainer, insert after pSibling
                        break;
                }
                if ( pSibling )
                {
                    nCmpPos = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );
                    OSL_ENSURE( nCmpPos > nLastPos, "InsertFootnote: Order of FootnoteFrm's buggy" );
                }
            }
            // pLastSib is the last footnote before the new one and
            // pSibling is empty or the first one after the new one
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
                OSL_ENSURE( !pSibling->GetNext(), "InsertFootnote: Who's that guy?" );
            }
        }
    }
    else
    {
        // First footnote of the column/page found. Now search from there for the first one on the
        // same column/page whose index is after the given one. The last one found is the predecessor.
        SwFootnoteBossFrm* pBoss = pNew->GetRef()->FindFootnoteBossFrm(
            !pNew->GetAttr()->GetFootnote().IsEndNote() );
        sal_uInt16 nRefNum = pBoss->GetPhyPageNum();    // page number of the new footnote
        sal_uInt16 nRefCol = lcl_ColumnNum( pBoss );    // column number of the new footnote
        bool bEnd = false;
        SwFootnoteFrm *pLastSib = nullptr;
        while ( pSibling && !bEnd && (nCmpPos <= nStPos) )
        {
            pLastSib = pSibling;
            nLastPos = nCmpPos;

            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();

            SwFootnoteFrm *pFoll = static_cast<SwFootnoteFrm*>(pSibling->GetNext());
            if ( pFoll )
            {
                pBoss = pSibling->GetRef()->FindFootnoteBossFrm( !pSibling->
                                            GetAttr()->GetFootnote().IsEndNote() );
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
                SwFootnoteBossFrm* pNxtB = pSibling->FindFootnoteBossFrm();
                SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                bool bEndNote = pSibPage->IsEndNotePage();
                bool bChgPage = lcl_NextFootnoteBoss( pNxtB, pSibPage, bDontLeave );
                // When changing pages, also the endnote flag must match.
                SwFootnoteContFrm *pCont = pNxtB && ( !bChgPage ||
                    pSibPage->IsEndNotePage() == bEndNote )
                    ? pNxtB->FindNearestFootnoteCont( bDontLeave ) : nullptr;
                if ( pCont )
                    pSibling = static_cast<SwFootnoteFrm*>(pCont->Lower());
                else
                    bEnd = true;
            }
            if ( !bEnd && pSibling )
                nCmpPos = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );
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
        nCmpPos = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );
        if ( nCmpPos < nStPos )
        {
            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();
            pParent = static_cast<SwFootnoteContFrm*>(pSibling->GetUpper());
            pSibling = static_cast<SwFootnoteFrm*>(pSibling->GetNext());
        }
        else
        {
            if( pSibling->GetMaster() )
            {
                if( ENDNOTE > nCmpPos || nStPos >= ENDNOTE )
                {
                    OSL_FAIL( "InsertFootnote: Master expected II" );
                    do
                        pSibling = pSibling->GetMaster();
                    while ( pSibling->GetMaster() );
                }
            }
            pParent = static_cast<SwFootnoteContFrm*>(pSibling->GetUpper());
        }
    }
    OSL_ENSURE( pParent, "paste in space?" );
    pNew->Paste( pParent, pSibling );
}

void SwFootnoteBossFrm::AppendFootnote( SwContentFrm *pRef, SwTextFootnote *pAttr )
{
    // If the footnote already exists, do nothing.
    if ( FindFootnote( pRef, pAttr ) )
        return;

    // If footnotes are inserted at the end of the document,
    // we only need to search from the relevant page on.
    // If there is none yet, we need to create one.
    // If it is an Endnote, we need to search for or create an
    // Endnote page.
    SwDoc *pDoc = GetFormat()->GetDoc();
    SwFootnoteBossFrm *pBoss = this;
    SwPageFrm *pPage = FindPageFrm();
    SwPageFrm *pMyPage = pPage;
    bool bChgPage = false;
    bool bEnd = false;
    if ( pAttr->GetFootnote().IsEndNote() )
    {
        bEnd = true;
        if( GetUpper()->IsSctFrm() &&
            static_cast<SwSectionFrm*>(GetUpper())->IsEndnAtEnd() )
        {
            SwFrm* pLast =
                static_cast<SwSectionFrm*>(GetUpper())->FindLastContent( FINDMODE_ENDNOTE );
            if( pLast )
            {
                pBoss = pLast->FindFootnoteBossFrm();
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
                        !pPage->OnRightPage(), false, false, true, nullptr );
                pPage->SetEndNotePage( true );
                bChgPage = true;
            }
            else
            {
                // At least we can search the approximately correct page.
                // To ensure to be finished in finite time even if hundreds
                // of footnotes exist
                SwPageFrm *pNxt = static_cast<SwPageFrm*>(pPage->GetNext());
                const sal_uLong nStPos = ::lcl_FindFootnotePos( pDoc, pAttr );
                while ( pNxt && pNxt->IsEndNotePage() )
                {
                    SwFootnoteContFrm *pCont = pNxt->FindFootnoteCont();
                    if ( pCont && pCont->Lower() )
                    {
                        OSL_ENSURE( pCont->Lower()->IsFootnoteFrm(), "no footnote in the container" );
                        if ( nStPos > ::lcl_FindFootnotePos( pDoc,
                                        static_cast<SwFootnoteFrm*>(pCont->Lower())->GetAttr()))
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
    else if( FTNPOS_CHAPTER == pDoc->GetFootnoteInfo().ePos && ( !GetUpper()->
             IsSctFrm() || !static_cast<SwSectionFrm*>(GetUpper())->IsFootnoteAtEnd() ) )
    {
        while ( pPage->GetNext() && !pPage->IsFootnotePage() &&
                !static_cast<SwPageFrm*>(pPage->GetNext())->IsEndNotePage() )
        {
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            bChgPage = true;
        }

        if ( !pPage->IsFootnotePage() )
        {
            SwPageDesc *pDesc = pDoc->GetFootnoteInfo().GetPageDesc( *pDoc );
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
            const sal_uLong nStPos = ::lcl_FindFootnotePos( pDoc, pAttr );
            while ( pNxt && pNxt->IsFootnotePage() && !pNxt->IsEndNotePage() )
            {
                SwFootnoteContFrm *pCont = pNxt->FindFootnoteCont();
                if ( pCont && pCont->Lower() )
                {
                    OSL_ENSURE( pCont->Lower()->IsFootnoteFrm(), "no footnote in the container" );
                    if ( nStPos > ::lcl_FindFootnotePos( pDoc,
                                        static_cast<SwFootnoteFrm*>(pCont->Lower())->GetAttr()))
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
    if( pBoss->IsInSct() && pBoss->IsColumnFrm() && !pPage->IsFootnotePage() )
    {
        SwSectionFrm* pSct = pBoss->FindSctFrm();
        if( bEnd ? !pSct->IsEndnAtEnd() : !pSct->IsFootnoteAtEnd() )
        {
            SwFootnoteContFrm* pFootnoteCont = pSct->FindFootnoteBossFrm(!bEnd)->FindFootnoteCont();
            if( pFootnoteCont )
            {
                SwFootnoteFrm* pTmp = static_cast<SwFootnoteFrm*>(pFootnoteCont->Lower());
                if( bEnd )
                    while( pTmp && !pTmp->GetAttr()->GetFootnote().IsEndNote() )
                        pTmp = static_cast<SwFootnoteFrm*>(pTmp->GetNext());
                if( pTmp && *pTmp < pAttr )
                    return;
            }
        }
    }

    SwFootnoteFrm *pNew = new SwFootnoteFrm( pDoc->GetDfltFrameFormat(), this, pRef, pAttr );
    {
        SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
        ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
    }
    // If the page was changed or newly created,
    // we need to place ourselves in the first column
    if( bChgPage )
    {
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        OSL_ENSURE( pBody, "AppendFootnote: NoPageBody?" );
        if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = static_cast<SwFootnoteBossFrm*>(pBody->Lower());
        else
            pBoss = pPage; // page if no columns exist
    }
    pBoss->InsertFootnote( pNew );
    if ( pNew->GetUpper() ) // inserted or not?
    {
        ::RegistFlys( pNew->FindPageFrm(), pNew );
        SwSectionFrm* pSect = FindSctFrm();
        // The content of a FootnoteContainer in a (column) section only need to be calculated
        // if the section stretches already to the bottom edge of the Upper.
        if( pSect && !pSect->IsJoinLocked() && ( bEnd ? !pSect->IsEndnAtEnd() :
            !pSect->IsFootnoteAtEnd() ) && pSect->Growable() )
            pSect->InvalidateSize();
        else
        {
            // #i49383# - disable unlock of position of
            // lower objects during format of footnote content.
            const bool bOldFootnoteFrmLocked( pNew->IsColLocked() );
            pNew->ColLock();
            pNew->KeepLockPosOfLowerObjs();
            // #i57914# - adjust fix #i49383#
            SwContentFrm *pCnt = pNew->ContainsContent();
            while ( pCnt && pCnt->FindFootnoteFrm()->GetAttr() == pAttr )
            {
                pCnt->Calc(getRootFrm()->GetCurrShell()->GetOut());
                // #i49383# - format anchored objects
                if ( pCnt->IsTextFrm() && pCnt->IsValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                              *(pCnt->FindPageFrm()) ) )
                    {
                        // restart format with first content
                        pCnt = pNew->ContainsContent();
                        continue;
                    }
                }
                pCnt = pCnt->FindNextCnt();
            }
            // #i49383#
            if ( !bOldFootnoteFrmLocked )
            {
                pNew->ColUnlock();
            }
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pNew->UnlockPosOfLowerObjs();
            pNew->Calc(getRootFrm()->GetCurrShell()->GetOut());
            // #i57914# - adjust fix #i49383#
            if ( !bOldFootnoteFrmLocked && !pNew->GetLower() &&
                 !pNew->IsColLocked() && !pNew->IsBackMoveLocked() )
            {
                pNew->Cut();
                SwFrm::DestroyFrm(pNew);
            }
        }
        pMyPage->UpdateFootnoteNum();
    }
    else
        SwFrm::DestroyFrm(pNew);
}

SwFootnoteFrm *SwFootnoteBossFrm::FindFootnote( const SwContentFrm *pRef, const SwTextFootnote *pAttr )
{
    // the easiest and savest way goes via the attribute
    OSL_ENSURE( pAttr->GetStartNode(), "FootnoteAtr without StartNode." );
    SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
    SwContentNode *pNd = aIdx.GetNode().GetContentNode();
    if ( !pNd )
        pNd = pRef->GetAttrSet()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    if ( !pNd )
        return nullptr;
    SwIterator<SwFrm,SwContentNode> aIter( *pNd );
    SwFrm* pFrm = aIter.First();
    if( pFrm )
        do
        {
                pFrm = pFrm->GetUpper();
                // #i28500#, #i27243# Due to the endnode collector, there are
                // SwFootnoteFrms, which are not in the layout. Therefore the
                // bInfFootnote flags are not set correctly, and a cell of FindFootnoteFrm
                // would return 0. Therefore we better call ImplFindFootnoteFrm().
                SwFootnoteFrm *pFootnote = pFrm->ImplFindFootnoteFrm();
                if ( pFootnote && pFootnote->GetRef() == pRef )
                {
                    // The following condition becomes true, if the whole
                    // footnotecontent is a section. While no frames exist,
                    // the HiddenFlag of the section is set, this causes
                    // the GoNextSection-function leaves the footnote.
                    if( pFootnote->GetAttr() != pAttr )
                        return nullptr;
                    while ( pFootnote && pFootnote->GetMaster() )
                        pFootnote = pFootnote->GetMaster();
                    return pFootnote;
                }

        } while ( nullptr != (pFrm = aIter.Next()) );

    return nullptr;
}

void SwFootnoteBossFrm::RemoveFootnote( const SwContentFrm *pRef, const SwTextFootnote *pAttr,
                              bool bPrep )
{
    SwFootnoteFrm *pFootnote = FindFootnote( pRef, pAttr );
    if( pFootnote )
    {
        do
        {
            SwFootnoteFrm *pFoll = pFootnote->GetFollow();
            pFootnote->Cut();
            SwFrm::DestroyFrm(pFootnote);
            pFootnote = pFoll;
        } while ( pFootnote );
        if( bPrep && pRef->IsFollow() )
        {
            OSL_ENSURE( pRef->IsTextFrm(), "NoTextFrm has Footnote?" );
            SwTextFrm* pMaster = pRef->FindMaster();
            if( !pMaster->IsLocked() )
                pMaster->Prepare( PREP_FTN_GONE );
        }
    }
    FindPageFrm()->UpdateFootnoteNum();
}

void SwFootnoteBossFrm::ChangeFootnoteRef( const SwContentFrm *pOld, const SwTextFootnote *pAttr,
                                 SwContentFrm *pNew )
{
    SwFootnoteFrm *pFootnote = FindFootnote( pOld, pAttr );
    while ( pFootnote )
    {
        pFootnote->SetRef( pNew );
        pFootnote = pFootnote->GetFollow();
    }
}

/// OD 03.04.2003 #108446# - add parameter <_bCollectOnlyPreviousFootnotes> in
/// order to control, if only footnotes, which are positioned before the
/// footnote boss frame <this> have to be collected.
void SwFootnoteBossFrm::CollectFootnotes( const SwContentFrm* _pRef,
                                SwFootnoteBossFrm*     _pOld,
                                SwFootnoteFrms&        _rFootnoteArr,
                                const bool    _bCollectOnlyPreviousFootnotes )
{
    SwFootnoteFrm *pFootnote = _pOld->FindFirstFootnote();
    while( !pFootnote )
    {
        if( _pOld->IsColumnFrm() )
        {
            // visit columns
            while ( !pFootnote && _pOld->GetPrev() )
            {
                // Still no problem if no footnote was found yet. The loop is needed to pick up
                // following rows in tables. In all other cases it might correct bad contexts.
                _pOld = static_cast<SwFootnoteBossFrm*>(_pOld->GetPrev());
                pFootnote = _pOld->FindFirstFootnote();
            }
        }
        if( !pFootnote )
        {
            // previous page
            SwPageFrm* pPg;
            for ( SwFrm* pTmp = _pOld;
                  nullptr != ( pPg = static_cast<SwPageFrm*>(pTmp->FindPageFrm()->GetPrev()))
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
                _pOld = static_cast<SwFootnoteBossFrm*>(pBody->GetLastLower());
            }
            else
                _pOld = pPg; // single column page
            pFootnote = _pOld->FindFirstFootnote();
        }
    }
    // OD 03.04.2003 #108446# - consider new parameter <_bCollectOnlyPreviousFootnotes>
    SwFootnoteBossFrm* pRefBossFrm = nullptr;
    if ( _bCollectOnlyPreviousFootnotes )
    {
        pRefBossFrm = this;
    }
    _CollectFootnotes( _pRef, pFootnote, _rFootnoteArr, _bCollectOnlyPreviousFootnotes, pRefBossFrm );
}

inline void FootnoteInArr( SwFootnoteFrms& rFootnoteArr, SwFootnoteFrm* pFootnote )
{
    if ( rFootnoteArr.end() == std::find( rFootnoteArr.begin(), rFootnoteArr.end(), pFootnote ) )
        rFootnoteArr.push_back( pFootnote );
}

/// OD 03.04.2003 #108446# - add parameters <_bCollectOnlyPreviousFootnotes> and
/// <_pRefFootnoteBossFrm> in order to control, if only footnotes, which are positioned
/// before the given reference footnote boss frame have to be collected.
/// Note: if parameter <_bCollectOnlyPreviousFootnotes> is true, then parameter
/// <_pRefFootnoteBossFrm> have to be referenced to an object.
/// Adjust parameter names.
void SwFootnoteBossFrm::_CollectFootnotes( const SwContentFrm*   _pRef,
                                 SwFootnoteFrm*           _pFootnote,
                                 SwFootnoteFrms&          _rFootnoteArr,
                                 bool                _bCollectOnlyPreviousFootnotes,
                                 const SwFootnoteBossFrm* _pRefFootnoteBossFrm)
{
    // OD 03.04.2003 #108446# - assert, that no reference footnote boss frame
    // is set, in spite of the order, that only previous footnotes has to be
    // collected.
    OSL_ENSURE( !_bCollectOnlyPreviousFootnotes || _pRefFootnoteBossFrm,
            "<SwFootnoteBossFrm::_CollectFootnotes(..)> - No reference footnote boss frame for collecting only previous footnotes set.\nCrash will be caused!" );

    // Collect all footnotes referenced by pRef (attribute by attribute), combine them
    // (the content might be divided over multiple pages) and cut them.

    // For robustness, we do not log the corresponding footnotes here. If a footnote
    // is touched twice, there might be a crash. This allows this function here to
    // also handle corrupt layouts in some degrees (without loops or even crashes).
    SwFootnoteFrms aNotFootnoteArr;

    // here we have a footnote placed in front of the first one of the reference
    OSL_ENSURE( !_pFootnote->GetMaster() || _pFootnote->GetRef() != _pRef, "move FollowFootnote?" );
    while ( _pFootnote->GetMaster() )
        _pFootnote = _pFootnote->GetMaster();

    bool bFound = false;

    do
    {
        // Search for the next footnote in this column/page so that
        // we do not start from zero again after cutting one footnote.
        SwFootnoteFrm *pNxtFootnote = _pFootnote;
        while ( pNxtFootnote->GetFollow() )
            pNxtFootnote = pNxtFootnote->GetFollow();
        pNxtFootnote = static_cast<SwFootnoteFrm*>(pNxtFootnote->GetNext());

        if ( !pNxtFootnote )
        {
            SwFootnoteBossFrm* pBoss = _pFootnote->FindFootnoteBossFrm();
            SwPageFrm* pPage = pBoss->FindPageFrm();
            do
            {
                lcl_NextFootnoteBoss( pBoss, pPage, false );
                if( pBoss )
                {
                    SwLayoutFrm* pCont = pBoss->FindFootnoteCont();
                    if( pCont )
                    {
                        pNxtFootnote = static_cast<SwFootnoteFrm*>(pCont->Lower());
                        if( pNxtFootnote )
                        {
                            while( pNxtFootnote->GetMaster() )
                                pNxtFootnote = pNxtFootnote->GetMaster();
                            if( pNxtFootnote == _pFootnote )
                                pNxtFootnote = nullptr;
                        }
                    }
                }
            } while( !pNxtFootnote && pBoss );
        }
        else if( !pNxtFootnote->GetAttr()->GetFootnote().IsEndNote() )
        { OSL_ENSURE( !pNxtFootnote->GetMaster(), "_CollectFootnote: Master expected" );
            while ( pNxtFootnote->GetMaster() )
                pNxtFootnote = pNxtFootnote->GetMaster();
        }
        if ( pNxtFootnote == _pFootnote )
        {
            OSL_FAIL(   "_CollectFootnote: Vicious circle" );
            pNxtFootnote = nullptr;
        }

        // OD 03.04.2003 #108446# - determine, if found footnote has to be collected.
        bool bCollectFoundFootnote = false;
        if ( _pFootnote->GetRef() == _pRef && !_pFootnote->GetAttr()->GetFootnote().IsEndNote() )
        {
            if ( _bCollectOnlyPreviousFootnotes )
            {
                SwFootnoteBossFrm* pBossOfFoundFootnote = _pFootnote->FindFootnoteBossFrm( true );
                OSL_ENSURE( pBossOfFoundFootnote,
                        "<SwFootnoteBossFrm::_CollectFootnotes(..)> - footnote boss frame of found footnote frame missing.\nWrong layout!" );
                if ( !pBossOfFoundFootnote ||    // don't crash, if no footnote boss is found.
                     pBossOfFoundFootnote->IsBefore( _pRefFootnoteBossFrm )
                   )
                {
                    bCollectFoundFootnote = true;
                }
            }
            else
            {
                bCollectFoundFootnote = true;
            }
        }

        if ( bCollectFoundFootnote )
        {
            OSL_ENSURE( !_pFootnote->GetMaster(), "move FollowFootnote?" );
            SwFootnoteFrm *pNxt = _pFootnote->GetFollow();
            while ( pNxt )
            {
                SwFrm *pCnt = pNxt->ContainsAny();
                if ( pCnt )
                {
                    // destroy the follow on the way as it is empty
                    do
                    {   SwFrm *pNxtCnt = pCnt->GetNext();
                        pCnt->Cut();
                        pCnt->Paste( _pFootnote );
                        pCnt = pNxtCnt;
                    } while ( pCnt );
                }
                else
                {
                    OSL_ENSURE( !pNxt, "footnote without content?" );
                    pNxt->Cut();
                    SwFrm::DestroyFrm(pNxt);
                }
                pNxt = _pFootnote->GetFollow();
            }
            _pFootnote->Cut();
            FootnoteInArr( _rFootnoteArr, _pFootnote );
            bFound = true;
        }
        else
        {
            FootnoteInArr( aNotFootnoteArr, _pFootnote );
            if( bFound )
                break;
        }
        if ( pNxtFootnote &&
             _rFootnoteArr.end() == std::find( _rFootnoteArr.begin(), _rFootnoteArr.end(), pNxtFootnote ) &&
             aNotFootnoteArr.end() == std::find( aNotFootnoteArr.begin(), aNotFootnoteArr.end(), pNxtFootnote ) )
            _pFootnote = pNxtFootnote;
        else
            break;
    }
    while ( _pFootnote );
}

void SwFootnoteBossFrm::_MoveFootnotes( SwFootnoteFrms &rFootnoteArr, bool bCalc )
{
    // All footnotes referenced by pRef need to be moved
    // to a new position (based on the new column/page)
    const sal_uInt16 nMyNum = FindPageFrm()->GetPhyPageNum();
    const sal_uInt16 nMyCol = lcl_ColumnNum( this );
    SWRECTFN( this )

    // #i21478# - keep last inserted footnote in order to
    // format the content of the following one.
    SwFootnoteFrm* pLastInsertedFootnote = nullptr;
    for ( size_t i = 0; i < rFootnoteArr.size(); ++i )
    {
        SwFootnoteFrm *pFootnote = rFootnoteArr[i];

        SwFootnoteBossFrm* pRefBoss = pFootnote->GetRef()->FindFootnoteBossFrm( true );
        if( pRefBoss != this )
        {
            const sal_uInt16 nRefNum = pRefBoss->FindPageFrm()->GetPhyPageNum();
            const sal_uInt16 nRefCol = lcl_ColumnNum( this );
            if( nRefNum < nMyNum || ( nRefNum == nMyNum && nRefCol <= nMyCol ) )
                pRefBoss = this;
        }
        pRefBoss->InsertFootnote( pFootnote );

        if ( pFootnote->GetUpper() ) // robust, e.g. with duplicates
        {
            // First condense the content so that footnote frames that do not fit on the page
            // do not do too much harm (Loop 66312). So, the footnote content first grows as
            // soon as the content gets formatted and it is sure that it fits on the page.
            SwFrm *pCnt = pFootnote->ContainsAny();
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
            (pFootnote->Frm().*fnRect->fnSetHeight)(0);
            (pFootnote->Prt().*fnRect->fnSetHeight)(0);
            pFootnote->Calc(getRootFrm()->GetCurrShell()->GetOut());
            pFootnote->GetUpper()->Calc(getRootFrm()->GetCurrShell()->GetOut());

            if( bCalc )
            {
                SwTextFootnote *pAttr = pFootnote->GetAttr();
                pCnt = pFootnote->ContainsAny();
                bool bUnlock = !pFootnote->IsBackMoveLocked();
                pFootnote->LockBackMove();

                // #i49383# - disable unlock of position of
                // lower objects during format of footnote content.
                pFootnote->KeepLockPosOfLowerObjs();
                // #i57914# - adjust fix #i49383#

                while ( pCnt && pCnt->FindFootnoteFrm()->GetAttr() == pAttr )
                {
                    pCnt->_InvalidatePos();
                    pCnt->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTextFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pCnt = pFootnote->ContainsAny();
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
                    pFootnote->UnlockBackMove();
                    if( !pFootnote->ContainsAny() && !pFootnote->IsColLocked() )
                    {
                        pFootnote->Cut();
                        SwFrm::DestroyFrm(pFootnote);
                        // #i21478#
                        pFootnote = nullptr;
                    }
                }
                // #i49383#
                if ( pFootnote )
                {
                    // #i57914# - adjust fix #i49383#
                    // enable lock of lower object position before format of footnote frame.
                    pFootnote->UnlockPosOfLowerObjs();
                    pFootnote->Calc(getRootFrm()->GetCurrShell()->GetOut());
                }
            }
        }
        else
        { OSL_ENSURE( !pFootnote->GetMaster() && !pFootnote->GetFollow(),
                    "DelFootnote and Master/Follow?" );
            SwFrm::DestroyFrm(pFootnote);
            // #i21478#
            pFootnote = nullptr;
        }

        // #i21478#
        if ( pFootnote )
        {
            pLastInsertedFootnote = pFootnote;
        }
    }

    // #i21478# - format content of footnote following
    // the new inserted ones.
    if ( bCalc && pLastInsertedFootnote )
    {
        if ( pLastInsertedFootnote->GetNext() )
        {
            SwFootnoteFrm* pNextFootnote = static_cast<SwFootnoteFrm*>(pLastInsertedFootnote->GetNext());
            SwTextFootnote* pAttr = pNextFootnote->GetAttr();
            SwFrm* pCnt = pNextFootnote->ContainsAny();

            bool bUnlock = !pNextFootnote->IsBackMoveLocked();
            pNextFootnote->LockBackMove();
            // #i49383# - disable unlock of position of
            // lower objects during format of footnote content.
            pNextFootnote->KeepLockPosOfLowerObjs();
            // #i57914# - adjust fix #i49383#

            while ( pCnt && pCnt->FindFootnoteFrm()->GetAttr() == pAttr )
            {
                pCnt->_InvalidatePos();
                pCnt->Calc(getRootFrm()->GetCurrShell()->GetOut());
                // #i49383# - format anchored objects
                if ( pCnt->IsTextFrm() && pCnt->IsValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                              *(pCnt->FindPageFrm()) ) )
                    {
                        // restart format with first content
                        pCnt = pNextFootnote->ContainsAny();
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
                pNextFootnote->UnlockBackMove();
            }
            // #i49383#
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pNextFootnote->UnlockPosOfLowerObjs();
            pNextFootnote->Calc(getRootFrm()->GetCurrShell()->GetOut());
        }
    }
}

void SwFootnoteBossFrm::MoveFootnotes( const SwContentFrm *pSrc, SwContentFrm *pDest,
                             SwTextFootnote *pAttr )
{
    if( ( GetFormat()->GetDoc()->GetFootnoteInfo().ePos == FTNPOS_CHAPTER &&
        (!GetUpper()->IsSctFrm() || !static_cast<SwSectionFrm*>(GetUpper())->IsFootnoteAtEnd()))
        || pAttr->GetFootnote().IsEndNote() )
        return;

    OSL_ENSURE( this == pSrc->FindFootnoteBossFrm( true ),
            "SwPageFrm::MoveFootnotes: source frame isn't on that FootnoteBoss" );

    SwFootnoteFrm *pFootnote = FindFirstFootnote();
    if( pFootnote )
    {
        ChangeFootnoteRef( pSrc, pAttr, pDest );
        SwFootnoteBossFrm *pDestBoss = pDest->FindFootnoteBossFrm( true );
        OSL_ENSURE( pDestBoss, "+SwPageFrm::MoveFootnotes: no destination boss" );
        if( pDestBoss )     // robust
        {
            SwFootnoteFrms aFootnoteArr;
            SwFootnoteBossFrm::_CollectFootnotes( pDest, pFootnote, aFootnoteArr );
            if ( !aFootnoteArr.empty() )
            {
                pDestBoss->_MoveFootnotes( aFootnoteArr, true );
                SwPageFrm* pSrcPage = FindPageFrm();
                SwPageFrm* pDestPage = pDestBoss->FindPageFrm();
                // update FootnoteNum only at page change
                if( pSrcPage != pDestPage )
                {
                    if( pSrcPage->GetPhyPageNum() > pDestPage->GetPhyPageNum() )
                        pSrcPage->UpdateFootnoteNum();
                    pDestPage->UpdateFootnoteNum();
                }
            }
        }
    }
}

void SwFootnoteBossFrm::RearrangeFootnotes( const SwTwips nDeadLine, const bool bLock,
                                  const SwTextFootnote *pAttr )
{
    // Format all footnotes of a column/page so that they might change the column/page.

    SwSaveFootnoteHeight aSave( this, nDeadLine );
    SwFootnoteFrm *pFootnote = FindFirstFootnote();
    if( pFootnote && pFootnote->GetPrev() && bLock )
    {
        SwFootnoteFrm* pFirst = static_cast<SwFootnoteFrm*>(pFootnote->GetUpper()->Lower());
        SwFrm* pContent = pFirst->ContainsAny();
        if( pContent )
        {
            bool bUnlock = !pFirst->IsBackMoveLocked();
            pFirst->LockBackMove();
            pFirst->Calc(getRootFrm()->GetCurrShell()->GetOut());
            pContent->Calc(getRootFrm()->GetCurrShell()->GetOut());
            // #i49383# - format anchored objects
            if ( pContent->IsTextFrm() && pContent->IsValid() )
            {
                SwObjectFormatter::FormatObjsAtFrm( *pContent,
                                                    *(pContent->FindPageFrm()) );
            }
            if( bUnlock )
                pFirst->UnlockBackMove();
        }
        pFootnote = FindFirstFootnote();
    }
    SwDoc *pDoc = GetFormat()->GetDoc();
    const sal_uLong nFootnotePos = pAttr ? ::lcl_FindFootnotePos( pDoc, pAttr ) : 0;
    SwFrm *pCnt = pFootnote ? pFootnote->ContainsAny() : nullptr;
    if ( pCnt )
    {
        bool bMore = true;
        bool bStart = pAttr == nullptr; // If no attribute is given, process all
        // #i49383# - disable unlock of position of
        // lower objects during format of footnote and footnote content.
        SwFootnoteFrm* pLastFootnoteFrm( nullptr );
        // footnote frame needs to be locked, if <bLock> isn't set.
        bool bUnlockLastFootnoteFrm( false );
        do
        {
            if( !bStart )
                bStart = ::lcl_FindFootnotePos( pDoc, pCnt->FindFootnoteFrm()->GetAttr() )
                         == nFootnotePos;
            if( bStart )
            {
                pCnt->_InvalidatePos();
                pCnt->_InvalidateSize();
                pCnt->Prepare( PREP_ADJUST_FRM );
                SwFootnoteFrm* pFootnoteFrm = pCnt->FindFootnoteFrm();
                // #i49383#
                if ( pFootnoteFrm != pLastFootnoteFrm )
                {
                    if ( pLastFootnoteFrm )
                    {
                        if ( !bLock && bUnlockLastFootnoteFrm )
                        {
                            pLastFootnoteFrm->ColUnlock();
                        }
                        // #i57914# - adjust fix #i49383#
                        // enable lock of lower object position before format of footnote frame.
                        pLastFootnoteFrm->UnlockPosOfLowerObjs();
                        pLastFootnoteFrm->Calc(getRootFrm()->GetCurrShell()->GetOut());
                        if ( !bLock && bUnlockLastFootnoteFrm &&
                             !pLastFootnoteFrm->GetLower() &&
                             !pLastFootnoteFrm->IsColLocked() &&
                             !pLastFootnoteFrm->IsBackMoveLocked() )
                        {
                            pLastFootnoteFrm->Cut();
                            SwFrm::DestroyFrm(pLastFootnoteFrm);
                            pLastFootnoteFrm = nullptr;
                        }
                    }
                    if ( !bLock )
                    {
                        bUnlockLastFootnoteFrm = !pFootnoteFrm->IsColLocked();
                        pFootnoteFrm->ColLock();
                    }
                    pFootnoteFrm->KeepLockPosOfLowerObjs();
                    pLastFootnoteFrm = pFootnoteFrm;
                }
                // OD 30.10.2002 #97265# - invalidate position of footnote
                // frame, if it's below its footnote container, in order to
                // assure its correct position, probably calculating its previous
                // footnote frames.
                {
                    SWRECTFN( this );
                    SwFrm* aFootnoteContFrm = pFootnoteFrm->GetUpper();
                    if ( (pFootnoteFrm->Frm().*fnRect->fnTopDist)((aFootnoteContFrm->*fnRect->fnGetPrtBottom)()) > 0 )
                    {
                        pFootnoteFrm->_InvalidatePos();
                    }
                }
                if ( bLock )
                {
                    bool bUnlock = !pFootnoteFrm->IsBackMoveLocked();
                    pFootnoteFrm->LockBackMove();
                    pFootnoteFrm->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    pCnt->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTextFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pCnt = pFootnote->ContainsAny();
                            continue;
                        }
                    }
                    if( bUnlock )
                    {
                        pFootnoteFrm->UnlockBackMove();
                        if( !pFootnoteFrm->Lower() &&
                            !pFootnoteFrm->IsColLocked() )
                        {
                            // #i49383#
                            OSL_ENSURE( pLastFootnoteFrm == pFootnoteFrm,
                                    "<SwFootnoteBossFrm::RearrangeFootnotes(..)> - <pLastFootnoteFrm> != <pFootnoteFrm>" );
                            pLastFootnoteFrm = nullptr;
                            pFootnoteFrm->Cut();
                            SwFrm::DestroyFrm(pFootnoteFrm);
                        }
                    }
                }
                else
                {
                    pFootnoteFrm->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    pCnt->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTextFrm() && pCnt->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                                  *(pCnt->FindPageFrm()) ) )
                        {
                            // restart format with first content
                            pCnt = pFootnote->ContainsAny();
                            continue;
                        }
                    }
                }
            }
            SwSectionFrm *pDel = nullptr;
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
                    SwFootnoteFrm* pFootnoteFrm = pCnt->FindFootnoteFrm();
                    if( pFootnoteFrm->GetRef()->FindFootnoteBossFrm(
                        pFootnoteFrm->GetAttr()->GetFootnote().IsEndNote() ) != this )
                        bMore = false;
                }
                else
                    bMore = false;
            }
            if( pDel )
            {
                bool bUnlockLastFootnoteFrmGuard = pLastFootnoteFrm && !pLastFootnoteFrm->IsColLocked();
                if (bUnlockLastFootnoteFrmGuard)
                    pLastFootnoteFrm->ColLock();
                pDel->Cut();
                if (bUnlockLastFootnoteFrmGuard)
                    pLastFootnoteFrm->ColUnlock();
                SwFrm::DestroyFrm(pDel);
            }
            if ( bMore )
            {
                // Go not further than to the provided footnote (if given)
                if ( pAttr &&
                     (::lcl_FindFootnotePos( pDoc,
                                    pCnt->FindFootnoteFrm()->GetAttr()) > nFootnotePos ) )
                    bMore = false;
            }
        } while ( bMore );
        // #i49383#
        if ( pLastFootnoteFrm )
        {
            if ( !bLock && bUnlockLastFootnoteFrm )
            {
                pLastFootnoteFrm->ColUnlock();
            }
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pLastFootnoteFrm->UnlockPosOfLowerObjs();
            pLastFootnoteFrm->Calc(getRootFrm()->GetCurrShell()->GetOut());
            if ( !bLock && bUnlockLastFootnoteFrm &&
                 !pLastFootnoteFrm->GetLower() &&
                 !pLastFootnoteFrm->IsColLocked() &&
                 !pLastFootnoteFrm->IsBackMoveLocked() )
            {
                pLastFootnoteFrm->Cut();
                SwFrm::DestroyFrm(pLastFootnoteFrm);
            }
        }
    }
}

void SwPageFrm::UpdateFootnoteNum()
{
    // page numbering only if set at the document
    if ( GetFormat()->GetDoc()->GetFootnoteInfo().eNum != FTNNUM_PAGE )
        return;

    SwLayoutFrm* pBody = FindBodyCont();
    if( !pBody || !pBody->Lower() )
        return;

    SwContentFrm* pContent = pBody->ContainsContent();
    sal_uInt16 nNum = 0;

    while( pContent && pContent->FindPageFrm() == this )
    {
        if( static_cast<SwTextFrm*>(pContent)->HasFootnote() )
        {
            SwFootnoteBossFrm* pBoss = pContent->FindFootnoteBossFrm( true );
            if( pBoss->GetUpper()->IsSctFrm() &&
                static_cast<SwSectionFrm*>(pBoss->GetUpper())->IsOwnFootnoteNum() )
                pContent = static_cast<SwSectionFrm*>(pBoss->GetUpper())->FindLastContent();
            else
            {
                SwFootnoteFrm* pFootnote = const_cast<SwFootnoteFrm*>(pBoss->FindFirstFootnote( pContent ));
                while( pFootnote )
                {
                    SwTextFootnote* pTextFootnote = pFootnote->GetAttr();
                    if( !pTextFootnote->GetFootnote().IsEndNote() &&
                         pTextFootnote->GetFootnote().GetNumStr().isEmpty() &&
                         !pFootnote->GetMaster() &&
                         (pTextFootnote->GetFootnote().GetNumber() != ++nNum) )
                    {
                        pTextFootnote->SetNumber( nNum, OUString() );
                    }
                    if ( pFootnote->GetNext() )
                        pFootnote = static_cast<SwFootnoteFrm*>(pFootnote->GetNext());
                    else
                    {
                        SwFootnoteBossFrm* pTmpBoss = pFootnote->FindFootnoteBossFrm( true );
                        if( pTmpBoss )
                        {
                            SwPageFrm* pPage = pTmpBoss->FindPageFrm();
                            pFootnote = nullptr;
                            lcl_NextFootnoteBoss( pTmpBoss, pPage, false );
                            SwFootnoteContFrm *pCont = pTmpBoss ? pTmpBoss->FindNearestFootnoteCont() : nullptr;
                            if ( pCont )
                                pFootnote = static_cast<SwFootnoteFrm*>(pCont->Lower());
                        }
                    }
                    if( pFootnote && pFootnote->GetRef() != pContent )
                        pFootnote = nullptr;
                }
            }
        }
        pContent = pContent->FindNextCnt();
    }
}

void SwFootnoteBossFrm::SetFootnoteDeadLine( const SwTwips nDeadLine )
{
    SwFrm *pBody = FindBodyCont();
    pBody->Calc(getRootFrm()->GetCurrShell()->GetOut());

    SwFrm *pCont = FindFootnoteCont();
    const SwTwips nMax = nMaxFootnoteHeight;// current should exceed MaxHeight
    SWRECTFN( this )
    if ( pCont )
    {
        pCont->Calc(getRootFrm()->GetCurrShell()->GetOut());
        nMaxFootnoteHeight = -(pCont->Frm().*fnRect->fnBottomDist)( nDeadLine );
    }
    else
        nMaxFootnoteHeight = -(pBody->Frm().*fnRect->fnBottomDist)( nDeadLine );

    const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : nullptr;
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        nMaxFootnoteHeight += pBody->Grow( LONG_MAX, true );
    if ( IsInSct() )
        nMaxFootnoteHeight += FindSctFrm()->Grow( LONG_MAX, true );

    if ( nMaxFootnoteHeight < 0 )
        nMaxFootnoteHeight = 0;
    if ( nMax != LONG_MAX && nMaxFootnoteHeight > nMax )
        nMaxFootnoteHeight = nMax;
}

SwTwips SwFootnoteBossFrm::GetVarSpace() const
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
                        IsFootnoteContFrm(), "FootnoteContainer expected" );
                const SwFootnoteContFrm* pCont = Lower() ?
                    static_cast<const SwFootnoteContFrm*>(Lower()->GetNext()) : nullptr;
                if( pCont )
                {
                    const SwFootnoteFrm* pFootnote = static_cast<const SwFootnoteFrm*>(pCont->Lower());
                    while( pFootnote)
                    {
                        if( pFootnote->GetAttr()->GetFootnote().IsEndNote() )
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
                        pFootnote = static_cast<const SwFootnoteFrm*>(pFootnote->GetNext());
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
        const SwViewShell *pSh = getRootFrm() ? getRootFrm()->GetCurrShell() : nullptr;
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
sal_uInt8 SwFootnoteBossFrm::_NeighbourhoodAdjustment( const SwFrm* ) const
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
                nRet = NA_GROW_ADJUST; // section with a single column (FootnoteAtEnd)
            else
            {
                const SwFrm* pTmp = Lower();
                OSL_ENSURE( pTmp, "NeighbourhoodAdjustment: Missing Lower()" );
                if( !pTmp->GetNext() )
                    nRet = NA_GROW_SHRINK;
                else if( !GetUpper()->IsColLocked() )
                    nRet = NA_ADJUST_GROW;
                OSL_ENSURE( !pTmp->GetNext() || pTmp->GetNext()->IsFootnoteContFrm(),
                        "NeighbourhoodAdjustment: Who's that guy?" );
            }
        }
    }
    return nRet;
}

void SwPageFrm::SetColMaxFootnoteHeight()
{
    SwLayoutFrm *pBody = FindBodyCont();
    if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
    {
        SwColumnFrm* pCol = static_cast<SwColumnFrm*>(pBody->Lower());
        do
        {
            pCol->SetMaxFootnoteHeight( GetMaxFootnoteHeight() );
            pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
        } while ( pCol );
    }
}

bool SwLayoutFrm::MoveLowerFootnotes( SwContentFrm *pStart, SwFootnoteBossFrm *pOldBoss,
                                 SwFootnoteBossFrm *pNewBoss, const bool bFootnoteNums )
{
    SwDoc *pDoc = GetFormat()->GetDoc();
    if ( pDoc->GetFootnoteIdxs().empty() )
        return false;
    if( pDoc->GetFootnoteInfo().ePos == FTNPOS_CHAPTER &&
        ( !IsInSct() || !FindSctFrm()->IsFootnoteAtEnd() ) )
        return true;

    if ( !pNewBoss )
        pNewBoss = FindFootnoteBossFrm( true );
    if ( pNewBoss == pOldBoss )
        return false;

    bool bMoved = false;
    if( !pStart )
        pStart = ContainsContent();

    SwFootnoteFrms aFootnoteArr;

    while ( IsAnLower( pStart ) )
    {
        if ( static_cast<SwTextFrm*>(pStart)->HasFootnote() )
        {
            // OD 03.04.2003 #108446# - To avoid unnecessary moves of footnotes
            // use new parameter <_bCollectOnlyPreviousFootnote> (4th parameter of
            // method <SwFootnoteBossFrm::CollectFootnote(..)>) to control, that only
            // footnotes have to be collected, that are positioned before the
            // new dedicated footnote boss frame.
            pNewBoss->CollectFootnotes( pStart, pOldBoss, aFootnoteArr, true );
        }
        pStart = pStart->GetNextContentFrm();
    }

    OSL_ENSURE( pOldBoss->IsInSct() == pNewBoss->IsInSct(),
            "MoveLowerFootnotes: Section confusion" );
    SwFootnoteFrms *pFootnoteArr;
    SwLayoutFrm* pNewChief = nullptr;
    SwLayoutFrm* pOldChief = nullptr;

    bool bFoundCandidate = false;
    if (pStart && pOldBoss->IsInSct())
    {
        pOldChief = pOldBoss->FindSctFrm();
        pNewChief = pNewBoss->FindSctFrm();
        bFoundCandidate = pOldChief != pNewChief;
    }

    if (bFoundCandidate)
    {
        pFootnoteArr = new SwFootnoteFrms;
        pOldChief = pOldBoss->FindFootnoteBossFrm( true );
        pNewChief = pNewBoss->FindFootnoteBossFrm( true );
        while( pOldChief->IsAnLower( pStart ) )
        {
            if ( static_cast<SwTextFrm*>(pStart)->HasFootnote() )
                static_cast<SwFootnoteBossFrm*>(pNewChief)->CollectFootnotes( pStart,
                                        pOldBoss, *pFootnoteArr );
            pStart = pStart->GetNextContentFrm();
        }
        if( pFootnoteArr->empty() )
        {
            delete pFootnoteArr;
            pFootnoteArr = nullptr;
        }
    }
    else
        pFootnoteArr = nullptr;

    if ( !aFootnoteArr.empty() || pFootnoteArr )
    {
        if( !aFootnoteArr.empty() )
            pNewBoss->_MoveFootnotes( aFootnoteArr, true );
        if( pFootnoteArr )
        {
            static_cast<SwFootnoteBossFrm*>(pNewChief)->_MoveFootnotes( *pFootnoteArr, true );
            delete pFootnoteArr;
        }
        bMoved = true;

        // update FootnoteNum only at page change
        if ( bFootnoteNums )
        {
            SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
            SwPageFrm* pNewPage =pNewBoss->FindPageFrm();
            if( pOldPage != pNewPage )
            {
                pOldPage->UpdateFootnoteNum();
                pNewPage->UpdateFootnoteNum();
            }
        }
    }
    return bMoved;
}

bool SwContentFrm::MoveFootnoteCntFwd( bool bMakePage, SwFootnoteBossFrm *pOldBoss )
{
    OSL_ENSURE( IsInFootnote(), "no footnote." );
    SwLayoutFrm *pFootnote = FindFootnoteFrm();

    // The first paragraph in the first footnote in the first column in the
    // sectionfrm at the top of the page has not to move forward, if the
    // columnbody is empty.
    if( pOldBoss->IsInSct() && !pOldBoss->GetIndPrev() && !GetIndPrev() &&
        !pFootnote->GetPrev() )
    {
        SwLayoutFrm* pBody = pOldBoss->FindBodyCont();
        if( !pBody || !pBody->Lower() )
            return true;
    }

    //fix(9538): if the footnote has neighbors behind itself, remove them temporarily
    SwLayoutFrm *pNxt = static_cast<SwLayoutFrm*>(pFootnote->GetNext());
    SwLayoutFrm *pLst = nullptr;
    while ( pNxt )
    {
        while ( pNxt->GetNext() )
            pNxt = static_cast<SwLayoutFrm*>(pNxt->GetNext());
        if ( pNxt == pLst )
            pNxt = nullptr;
        else
        {   pLst = pNxt;
            SwContentFrm *pCnt = pNxt->ContainsContent();
            if( pCnt )
                pCnt->MoveFootnoteCntFwd( true, pOldBoss );
            pNxt = static_cast<SwLayoutFrm*>(pFootnote->GetNext());
        }
    }

    bool bSamePage = true;
    SwLayoutFrm *pNewUpper =
                GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, true );

    if ( pNewUpper )
    {
        bool bSameBoss = true;
        SwFootnoteBossFrm * const pNewBoss = pNewUpper->FindFootnoteBossFrm();
        // Are we changing the column/page?
        if ( !( bSameBoss = (pNewBoss == pOldBoss) ) )
        {
            bSamePage = pOldBoss->FindPageFrm() == pNewBoss->FindPageFrm(); // page change?
            pNewUpper->Calc(getRootFrm()->GetCurrShell()->GetOut());
        }

        // The layout leaf of the footnote is either a footnote container or a footnote.
        // If it is a footnote and it has the same footnote reference like the old Upper,
        // then move the content inside of it.
        // If it is a container or the reference differs, create a new footnote and add
        // it into the container.
        // Create also a SectionFrame if currently in a area inside a footnote.
        SwFootnoteFrm* pTmpFootnote = pNewUpper->IsFootnoteFrm() ? static_cast<SwFootnoteFrm*>(pNewUpper) : nullptr;
        if( !pTmpFootnote )
        {
            OSL_ENSURE( pNewUpper->IsFootnoteContFrm(), "New Upper not a FootnoteCont.");
            SwFootnoteContFrm *pCont = static_cast<SwFootnoteContFrm*>(pNewUpper);

            // create footnote
            SwFootnoteFrm *pOld = FindFootnoteFrm();
            pTmpFootnote = new SwFootnoteFrm( pOld->GetFormat()->GetDoc()->GetDfltFrameFormat(),
                                    pOld, pOld->GetRef(), pOld->GetAttr() );
            // chaining of footnotes
            if ( pOld->GetFollow() )
            {
                pTmpFootnote->SetFollow( pOld->GetFollow() );
                pOld->GetFollow()->SetMaster( pTmpFootnote );
            }
            pOld->SetFollow( pTmpFootnote );
            pTmpFootnote->SetMaster( pOld );
            SwFrm* pNx = pCont->Lower();
            if( pNx && pTmpFootnote->GetAttr()->GetFootnote().IsEndNote() )
                while(pNx && !static_cast<SwFootnoteFrm*>(pNx)->GetAttr()->GetFootnote().IsEndNote())
                    pNx = pNx->GetNext();
            pTmpFootnote->Paste( pCont, pNx );
            pTmpFootnote->Calc(getRootFrm()->GetCurrShell()->GetOut());
        }
        OSL_ENSURE( pTmpFootnote->GetAttr() == FindFootnoteFrm()->GetAttr(), "Wrong Footnote!" );
        // areas inside of footnotes get a special treatment
        SwLayoutFrm *pNewUp = pTmpFootnote;
        if( IsInSct() )
        {
            SwSectionFrm* pSect = FindSctFrm();
            // area inside of a footnote (or only footnote in an area)?
            if( pSect->IsInFootnote() )
            {
                if( pTmpFootnote->Lower() && pTmpFootnote->Lower()->IsSctFrm() &&
                    pSect->GetFollow() == static_cast<SwSectionFrm*>(pTmpFootnote->Lower()) )
                    pNewUp = static_cast<SwSectionFrm*>(pTmpFootnote->Lower());
                else
                {
                    pNewUp = new SwSectionFrm( *pSect, false );
                    pNewUp->InsertBefore( pTmpFootnote, pTmpFootnote->Lower() );
                    static_cast<SwSectionFrm*>(pNewUp)->Init();
                    pNewUp->Frm().Pos() = pTmpFootnote->Frm().Pos();
                    pNewUp->Frm().Pos().Y() += 1; // for notifications

                    // If the section frame has a successor then the latter needs
                    // to be moved behind the new Follow of the section frame.
                    SwFrm* pTmp = pSect->GetNext();
                    if( pTmp )
                    {
                        SwFlowFrm* pTmpNxt;
                        if( pTmp->IsContentFrm() )
                            pTmpNxt = static_cast<SwContentFrm*>(pTmp);
                        else if( pTmp->IsSctFrm() )
                            pTmpNxt = static_cast<SwSectionFrm*>(pTmp);
                        else
                        {
                            OSL_ENSURE( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                            pTmpNxt = static_cast<SwTabFrm*>(pTmp);
                        }
                        pTmpNxt->MoveSubTree( pTmpFootnote, pNewUp->GetNext() );
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

SwSaveFootnoteHeight::SwSaveFootnoteHeight( SwFootnoteBossFrm *pBs, const SwTwips nDeadLine ) :
    pBoss( pBs ),
    nOldHeight( pBs->GetMaxFootnoteHeight() )
{
    pBoss->SetFootnoteDeadLine( nDeadLine );
    nNewHeight = pBoss->GetMaxFootnoteHeight();
}

SwSaveFootnoteHeight::~SwSaveFootnoteHeight()
{
    // If somebody tweaked the deadline meanwhile, we let it happen
    if ( nNewHeight == pBoss->GetMaxFootnoteHeight() )
        pBoss->nMaxFootnoteHeight = nOldHeight;
}

#ifdef DBG_UTIL
//JP 15.10.2001: in a non pro version test if the attribute has the same
//              meaning which his reference is

// Normally, the pRef member and the GetRefFromAttr() result has to be
// identically. Sometimes footnote will be moved from a master to its follow,
// but the GetRef() is called first, so we have to ignore a master/follow
// mismatch.

const SwContentFrm* SwFootnoteFrm::GetRef() const
{
    const SwContentFrm* pRefAttr = GetRefFromAttr();
    SAL_WARN_IF( pRef != pRefAttr && !pRef->IsAnFollow( pRefAttr )
            && !pRefAttr->IsAnFollow( pRef ),
            "sw", "access to deleted Frame? pRef != pAttr->GetRef()" );
    return pRef;
}

SwContentFrm* SwFootnoteFrm::GetRef()
{
    const SwContentFrm* pRefAttr = GetRefFromAttr();
    SAL_WARN_IF( pRef != pRefAttr && !pRef->IsAnFollow( pRefAttr )
            && !pRefAttr->IsAnFollow( pRef ),
            "sw", "access to deleted Frame? pRef != pAttr->GetRef()" );
    return pRef;
}
#endif

const SwContentFrm* SwFootnoteFrm::GetRefFromAttr()  const
{
    SwFootnoteFrm* pThis = const_cast<SwFootnoteFrm*>(this);
    return pThis->GetRefFromAttr();
}

SwContentFrm* SwFootnoteFrm::GetRefFromAttr()
{
    assert(pAttr && "invalid Attribute");
    SwTextNode& rTNd = (SwTextNode&)pAttr->GetTextNode();
    SwPosition aPos( rTNd, SwIndex( &rTNd, pAttr->GetStart() ));
    SwContentFrm* pCFrm = rTNd.getLayoutFrm( getRootFrm(), nullptr, &aPos, false );
    return pCFrm;
}

/** search for last content in the current footnote frame

    OD 2005-12-02 #i27138#
*/
SwContentFrm* SwFootnoteFrm::FindLastContent()
{
    SwContentFrm* pLastContentFrm( nullptr );

    // find last lower, which is a content frame or contains content.
    // hidden text frames, empty sections and empty tables have to be skipped.
    SwFrm* pLastLowerOfFootnote( GetLower() );
    SwFrm* pTmpLastLower( pLastLowerOfFootnote );
    while ( pTmpLastLower && pTmpLastLower->GetNext() )
    {
        pTmpLastLower = pTmpLastLower->GetNext();
        if ( ( pTmpLastLower->IsTextFrm() &&
               !static_cast<SwTextFrm*>(pTmpLastLower)->IsHiddenNow() ) ||
             ( pTmpLastLower->IsSctFrm() &&
               static_cast<SwSectionFrm*>(pTmpLastLower)->GetSection() &&
               static_cast<SwSectionFrm*>(pTmpLastLower)->ContainsContent() ) ||
             ( pTmpLastLower->IsTabFrm() &&
               static_cast<SwTabFrm*>(pTmpLastLower)->ContainsContent() ) )
        {
            pLastLowerOfFootnote = pTmpLastLower;
        }
    }

    // determine last content frame depending on type of found last lower.
    if ( pLastLowerOfFootnote && pLastLowerOfFootnote->IsTabFrm() )
    {
        pLastContentFrm = static_cast<SwTabFrm*>(pLastLowerOfFootnote)->FindLastContent();
    }
    else if ( pLastLowerOfFootnote && pLastLowerOfFootnote->IsSctFrm() )
    {
        pLastContentFrm = static_cast<SwSectionFrm*>(pLastLowerOfFootnote)->FindLastContent();
    }
    else
    {
        pLastContentFrm = dynamic_cast<SwContentFrm*>(pLastLowerOfFootnote);
    }

    return pLastContentFrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
