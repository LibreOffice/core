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
#include <viewopt.hxx>
#include <calbck.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <sal/log.hxx>

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

bool SwFootnoteFrame::operator<( const SwTextFootnote* pTextFootnote ) const
{
    const SwDoc* pDoc = GetFormat()->GetDoc();
    OSL_ENSURE( pDoc, "SwFootnoteFrame: Missing doc!" );
    return lcl_FindFootnotePos( pDoc, GetAttr() ) <
           lcl_FindFootnotePos( pDoc, pTextFootnote );
}

/*
|*
|*  bool lcl_NextFootnoteBoss( SwFootnoteBossFrame* pBoss, SwPageFrame* pPage)
|*  sets pBoss on the next SwFootnoteBossFrame, which can either be a column
|*  or a page (without columns). If the page changes meanwhile,
|*  pPage contains the new page and this function returns true.
|*
|*/

static bool lcl_NextFootnoteBoss( SwFootnoteBossFrame* &rpBoss, SwPageFrame* &rpPage,
    bool bDontLeave )
{
    if( rpBoss->IsColumnFrame() )
    {
        if( rpBoss->GetNext() )
        {
            rpBoss = static_cast<SwFootnoteBossFrame*>(rpBoss->GetNext()); //next column
            return false;
        }
        if( rpBoss->IsInSct() )
        {
            SwSectionFrame* pSct = rpBoss->FindSctFrame()->GetFollow();
            if( pSct )
            {
                OSL_ENSURE( pSct->Lower() && pSct->Lower()->IsColumnFrame(),
                        "Where's the column?" );
                rpBoss = static_cast<SwColumnFrame*>(pSct->Lower());
                SwPageFrame* pOld = rpPage;
                rpPage = pSct->FindPageFrame();
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
    rpPage = static_cast<SwPageFrame*>(rpPage->GetNext()); // next page
    rpBoss = rpPage;
    if( rpPage )
    {
        SwLayoutFrame* pBody = rpPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrame() )
            rpBoss = static_cast<SwFootnoteBossFrame*>(pBody->Lower()); // first column
    }
    return true;
}

/// @returns column number if pBoss is a column, otherwise 0.
static sal_uInt16 lcl_ColumnNum( const SwFrame* pBoss )
{
    sal_uInt16 nRet = 0;
    if( !pBoss->IsColumnFrame() )
        return 0;
    const SwFrame* pCol;
    if( pBoss->IsInSct() )
    {
        pCol = pBoss->GetUpper()->FindColFrame();
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

SwFootnoteContFrame::SwFootnoteContFrame( SwFrameFormat *pFormat, SwFrame* pSib ):
    SwLayoutFrame( pFormat, pSib )
{
    mnFrameType = SwFrameType::FtnCont;
}


// lcl_Undersize(..) walks over a SwFrame and its contents
// and returns the sum of all requested TextFrame magnifications.

static long lcl_Undersize( const SwFrame* pFrame )
{
    long nRet = 0;
    SwRectFnSet aRectFnSet(pFrame);
    if( pFrame->IsTextFrame() )
    {
        if( static_cast<const SwTextFrame*>(pFrame)->IsUndersized() )
        {
            // Does this TextFrame would like to be a little bit bigger?
            nRet = static_cast<const SwTextFrame*>(pFrame)->GetParHeight() -
                    aRectFnSet.GetHeight(pFrame->getFramePrintArea());
            if( nRet < 0 )
                nRet = 0;
        }
    }
    else if( pFrame->IsLayoutFrame() )
    {
        const SwFrame* pNxt = static_cast<const SwLayoutFrame*>(pFrame)->Lower();
        while( pNxt )
        {
            nRet += lcl_Undersize( pNxt );
            pNxt = pNxt->GetNext();
        }
    }
    return nRet;
}

/// "format" the frame (Fixsize is not set here).
void SwFootnoteContFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    // calculate total border, only one distance to the top
    const SwPageFrame* pPage = FindPageFrame();
    const SwPageFootnoteInfo &rInf = pPage->GetPageDesc()->GetFootnoteInfo();
    const SwTwips nBorder = rInf.GetTopDist() + rInf.GetBottomDist() +
                            rInf.GetLineWidth();
    SwRectFnSet aRectFnSet(this);

    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);
        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);

        aRectFnSet.SetTop( aPrt, nBorder );
        aRectFnSet.SetWidth( aPrt, aRectFnSet.GetWidth(getFrameArea()) );
        aRectFnSet.SetHeight(aPrt, aRectFnSet.GetHeight(getFrameArea()) - nBorder );

        if( aRectFnSet.GetHeight(aPrt) < 0 && !pPage->IsFootnotePage() )
        {
            setFrameAreaSizeValid(false);
        }
    }

    if ( !isFrameAreaSizeValid() )
    {
        bool bGrow = pPage->IsFootnotePage();
        if( bGrow )
        {
            const SwViewShell *pSh = getRootFrame() ? getRootFrame()->GetCurrShell() : nullptr;
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bGrow = false;
        }
        if( bGrow )
                Grow( LONG_MAX );
        else
        {
            // VarSize is determined based on the content plus the borders
            SwTwips nRemaining = 0;
            SwFrame *pFrame = m_pLower;
            while ( pFrame )
            {   // lcl_Undersize(..) respects (recursively) TextFrames, which
                // would like to be bigger. They are created especially in
                // columnized borders, if these do not have their maximum
                // size yet.
                nRemaining += aRectFnSet.GetHeight(pFrame->getFrameArea()) + lcl_Undersize( pFrame );
                pFrame = pFrame->GetNext();
            }
            // add the own border
            nRemaining += nBorder;

            SwTwips nDiff;
            if( IsInSct() )
            {
                nDiff = -aRectFnSet.BottomDist( getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()) );
                if( nDiff > 0 )
                {
                    if( nDiff > aRectFnSet.GetHeight(getFrameArea()) )
                    {
                        nDiff = aRectFnSet.GetHeight(getFrameArea());
                    }

                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                    aRectFnSet.AddBottom( aFrm, -nDiff );
                    aRectFnSet.AddHeight( aFrm, -nDiff );
                }
            }
            nDiff = aRectFnSet.GetHeight(getFrameArea()) - nRemaining;
            if ( nDiff > 0 )
                Shrink( nDiff );
            else if ( nDiff < 0 )
            {
                Grow( -nDiff );
                // It may happen that there is less space available,
                // than what the border needs - the size of the PrtArea
                // will then be negative.
                SwTwips nPrtHeight = aRectFnSet.GetHeight(getFramePrintArea());
                if( nPrtHeight < 0 )
                {
                    const SwTwips nTmpDiff = std::max( aRectFnSet.GetTop(getFramePrintArea()), -nPrtHeight );
                    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                    aRectFnSet.SubTop( aPrt, nTmpDiff );
                }
            }
        }

        setFrameAreaSizeValid(true);
    }
}

SwTwips SwFootnoteContFrame::GrowFrame( SwTwips nDist, bool bTst, bool )
{
    // No check if FixSize since FootnoteContainer are variable up to their max. height.
    // If the max. height is LONG_MAX, take as much space as needed.
    // If the page is a special footnote page, take also as much as possible.
    assert(GetUpper() && GetUpper()->IsFootnoteBossFrame());

    SwRectFnSet aRectFnSet(this);
    if( aRectFnSet.GetHeight(getFrameArea()) > 0 &&
         nDist > ( LONG_MAX - aRectFnSet.GetHeight(getFrameArea()) ) )
        nDist = LONG_MAX - aRectFnSet.GetHeight(getFrameArea());

    SwFootnoteBossFrame *pBoss = static_cast<SwFootnoteBossFrame*>(GetUpper());
    if( IsInSct() )
    {
        SwSectionFrame* pSect = FindSctFrame();
        OSL_ENSURE( pSect, "GrowFrame: Missing SectFrame" );
        // In a section, which has to maximize, a footnotecontainer is allowed
        // to grow, when the section can't grow anymore.
        if( !bTst && !pSect->IsColLocked() &&
            pSect->ToMaximize( false ) && pSect->Growable() )
        {
            pSect->InvalidateSize();
            return 0;
        }
    }
    const SwViewShell *pSh = getRootFrame() ? getRootFrame()->GetCurrShell() : nullptr;
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    SwPageFrame *pPage = pBoss->FindPageFrame();
    if ( bBrowseMode || !pPage->IsFootnotePage() )
    {
        if ( pBoss->GetMaxFootnoteHeight() != LONG_MAX )
        {
            nDist = std::min( nDist, pBoss->GetMaxFootnoteHeight()
                         - aRectFnSet.GetHeight(getFrameArea()) );
            if ( nDist <= 0 )
                return 0;
        }
        // FootnoteBoss also influences the max value
        if( !IsInSct() )
        {
            const SwTwips nMax = pBoss->GetVarSpace();
            if ( nDist > nMax )
                nDist = nMax;
            if ( nDist <= 0 )
                return 0;
        }
    }
    else if( nDist > aRectFnSet.GetHeight(GetPrev()->getFrameArea()) )
        // do not use more space than the body has
        nDist = aRectFnSet.GetHeight(GetPrev()->getFrameArea());

    long nAvail = 0;
    if ( bBrowseMode )
    {
        nAvail = GetUpper()->getFramePrintArea().Height();
        const SwFrame *pAvail = GetUpper()->Lower();
        do
        {   nAvail -= pAvail->getFrameArea().Height();
            pAvail = pAvail->GetNext();
        } while ( pAvail );
        if ( nAvail > nDist )
            nAvail = nDist;
    }

    if ( !bTst )
    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aRectFnSet.SetHeight( aFrm, aRectFnSet.GetHeight(aFrm) + nDist );

        if( IsVertical() && !IsVertLR() )
        {
            aFrm.Pos().AdjustX( -nDist );
        }
    }
    long nGrow = nDist - nAvail,
         nReal = 0;
    if ( nGrow > 0 )
    {
        SwNeighbourAdjust nAdjust = pBoss->NeighbourhoodAdjustment();
        if( SwNeighbourAdjust::OnlyAdjust == nAdjust )
            nReal = AdjustNeighbourhood( nGrow, bTst );
        else
        {
            if( SwNeighbourAdjust::GrowAdjust == nAdjust )
            {
                SwFrame* pFootnote = Lower();
                if( pFootnote )
                {
                    while( pFootnote->GetNext() )
                        pFootnote = pFootnote->GetNext();
                    if( static_cast<SwFootnoteFrame*>(pFootnote)->GetAttr()->GetFootnote().IsEndNote() )
                    {
                        nReal = AdjustNeighbourhood( nGrow, bTst );
                        nAdjust = SwNeighbourAdjust::GrowShrink; // no more AdjustNeighbourhood
                    }
                }
            }
            nReal += pBoss->Grow( nGrow - nReal, bTst );
            if( ( SwNeighbourAdjust::GrowAdjust == nAdjust || SwNeighbourAdjust::AdjustGrow == nAdjust )
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
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aFrm.SSize().AdjustHeight( -nDist );

            if( IsVertical() && !IsVertLR() )
            {
                aFrm.Pos().AdjustX(nDist );
            }
        }

        // growing happens upwards, so successors to not need to be invalidated
        if( nReal )
        {
            InvalidateSize_();
            InvalidatePos_();
            InvalidatePage( pPage );
        }
    }
    return nReal;
}

SwTwips SwFootnoteContFrame::ShrinkFrame( SwTwips nDiff, bool bTst, bool bInfo )
{
    SwPageFrame *pPage = FindPageFrame();
    bool bShrink = false;
    if ( pPage )
    {
        if( !pPage->IsFootnotePage() )
            bShrink = true;
        else
        {
            const SwViewShell *pSh = getRootFrame()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                bShrink = true;
        }
    }
    if( bShrink )
    {
        SwTwips nRet = SwLayoutFrame::ShrinkFrame( nDiff, bTst, bInfo );
        if( IsInSct() && !bTst )
            FindSctFrame()->InvalidateNextPos();
        if ( !bTst && nRet )
        {
            InvalidatePos_();
            InvalidatePage( pPage );
        }
        return nRet;
    }
    return 0;
}

SwFootnoteFrame::SwFootnoteFrame( SwFrameFormat *pFormat, SwFrame* pSib, SwContentFrame *pCnt, SwTextFootnote *pAt ):
    SwLayoutFrame( pFormat, pSib ),
    mpFollow( nullptr ),
    mpMaster( nullptr ),
    mpReference( pCnt ),
    mpAttribute( pAt ),
    mbBackMoveLocked( false ),
    // #i49383#
    mbUnlockPosOfLowerObjs( true )
{
    mnFrameType = SwFrameType::Ftn;
}

void SwFootnoteFrame::InvalidateNxtFootnoteCnts( SwPageFrame const *pPage )
{
    if ( GetNext() )
    {
        SwFrame *pCnt = static_cast<SwLayoutFrame*>(GetNext())->ContainsAny();
        if( pCnt )
        {
            pCnt->InvalidatePage( pPage );
            pCnt->InvalidatePrt_();
            do
            {   pCnt->InvalidatePos_();
                if( pCnt->IsSctFrame() )
                {
                    SwFrame* pTmp = static_cast<SwSectionFrame*>(pCnt)->ContainsAny();
                    if( pTmp )
                        pTmp->InvalidatePos_();
                }
                pCnt->GetUpper()->InvalidateSize_();
                pCnt = pCnt->FindNext();
            } while ( pCnt && GetUpper()->IsAnLower( pCnt ) );
        }
    }
}

bool SwFootnoteFrame::IsDeleteForbidden() const
{
    if (SwLayoutFrame::IsDeleteForbidden())
        return true;
    // needs to be in sync with the ::Cut logic
    const SwLayoutFrame *pUp = GetUpper();
    if (pUp)
    {
        if (GetPrev())
            return false;

        // The last footnote takes its container along if it
        // is deleted. Cut would put pUp->Lower() to the value
        // of GetNext(), so if there is no GetNext then
        // Cut would delete pUp. If that condition is true
        // here then check if the container is delete-forbidden
        return !GetNext() && pUp->IsDeleteForbidden();
    }
    return false;
}

void SwFootnoteFrame::Cut()
{
    if ( GetNext() )
        GetNext()->InvalidatePos();
    else if ( GetPrev() )
        GetPrev()->SetRetouche();

    // first move then shrink Upper
    SwLayoutFrame *pUp = GetUpper();

    // correct chaining
    SwFootnoteFrame *pFootnote = this;
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
        if (!pUp->Lower())
        {
            SwPageFrame *pPage = pUp->FindPageFrame();
            if ( pPage )
            {
                SwLayoutFrame *pBody = pPage->FindBodyCont();
                if( pBody && !pBody->ContainsContent() )
                    pPage->getRootFrame()->SetSuperfluous();
            }
            SwSectionFrame* pSect = pUp->FindSctFrame();
            pUp->Cut();
            SwFrame::DestroyFrame(pUp);
            // If the last footnote container was removed from a column
            // section without a Follow, then this section can be shrunk.
            if( pSect && !pSect->ToMaximize( false ) && !pSect->IsColLocked() )
                pSect->InvalidateSize_();
        }
        else
        {   if ( getFrameArea().Height() )
                pUp->Shrink( getFrameArea().Height() );
            pUp->SetCompletePaint();
            pUp->InvalidatePage();
        }
    }
}

void SwFootnoteFrame::Paste(  SwFrame* pParent, SwFrame* pSibling )
{
    OSL_ENSURE( pParent, "no parent in Paste." );
    OSL_ENSURE( pParent->IsLayoutFrame(), "Parent is ContentFrame." );
    OSL_ENSURE( pParent != this, "I am my own parent." );
    OSL_ENSURE( pSibling != this, "I am my own sibling." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I am still somewhere registered." );

    // insert into tree structure
    InsertBefore( static_cast<SwLayoutFrame*>(pParent), pSibling );

    SwRectFnSet aRectFnSet(this);
    if( aRectFnSet.GetWidth(getFrameArea())!=aRectFnSet.GetWidth(pParent->getFramePrintArea()) )
        InvalidateSize_();
    InvalidatePos_();
    SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );
    if ( GetNext() )
        GetNext()->InvalidatePos_();
    if( aRectFnSet.GetHeight(getFrameArea()) )
        pParent->Grow( aRectFnSet.GetHeight(getFrameArea()) );

    // If the predecessor is the master and/or the successor is the Follow,
    // then take their content and destroy them.
    if ( GetPrev() && GetPrev() == GetMaster() )
    {
        OSL_ENSURE( SwFlowFrame::CastFlowFrame( GetPrev()->GetLower() ),
                "Footnote without content?" );
        SwFlowFrame::CastFlowFrame( GetPrev()->GetLower())->
            MoveSubTree( this, GetLower() );
        SwFrame *pDel = GetPrev();
        pDel->Cut();
        SwFrame::DestroyFrame(pDel);
    }
    if ( GetNext() && GetNext() == GetFollow() )
    {
        OSL_ENSURE( SwFlowFrame::CastFlowFrame( GetNext()->GetLower() ),
                "Footnote without content?" );
        SwFlowFrame::CastFlowFrame( GetNext()->GetLower() )->MoveSubTree( this );
        SwFrame *pDel = GetNext();
        pDel->Cut();
        SwFrame::DestroyFrame(pDel);
    }
#if OSL_DEBUG_LEVEL > 0
    SwDoc *pDoc = GetFormat()->GetDoc();
    if ( GetPrev() )
    {
        OSL_ENSURE( lcl_FindFootnotePos( pDoc, static_cast<SwFootnoteFrame*>(GetPrev())->GetAttr() ) <=
                lcl_FindFootnotePos( pDoc, GetAttr() ), "Prev is not FootnotePrev" );
    }
    if ( GetNext() )
    {
        OSL_ENSURE( lcl_FindFootnotePos( pDoc, GetAttr() ) <=
                lcl_FindFootnotePos( pDoc, static_cast<SwFootnoteFrame*>(GetNext())->GetAttr() ),
                "Next is not FootnoteNext" );
    }
#endif
    InvalidateNxtFootnoteCnts( pPage );
}

/// Return the next layout leaf in that the frame can be moved.
/// New pages will only be created if specified by the parameter.
SwLayoutFrame *SwFrame::GetNextFootnoteLeaf( MakePageType eMakePage )
{
    SwFootnoteBossFrame *pOldBoss = FindFootnoteBossFrame();
    SwPageFrame* pOldPage = pOldBoss->FindPageFrame();
    SwPageFrame* pPage;
    SwFootnoteBossFrame *pBoss = pOldBoss->IsColumnFrame() ?
        static_cast<SwFootnoteBossFrame*>(pOldBoss->GetNext()) : nullptr; // next column, if existing
    if( pBoss )
        pPage = nullptr;
    else
    {
        if( pOldBoss->GetUpper()->IsSctFrame() )
        {   // this can only be in a column area
            SwLayoutFrame* pNxt = pOldBoss->GetNextSctLeaf( eMakePage );
            if( pNxt )
            {
                OSL_ENSURE( pNxt->IsColBodyFrame(), "GetNextFootnoteLeaf: Funny Leaf" );
                pBoss = static_cast<SwFootnoteBossFrame*>(pNxt->GetUpper());
                pPage = pBoss->FindPageFrame();
            }
            else
                return nullptr;
        }
        else
        {
            // next page
            pPage = static_cast<SwPageFrame*>(pOldPage->GetNext());
            // skip empty pages
            if( pPage && pPage->IsEmptyPage() )
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
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
    SwFootnoteFrame *pFootnote = FindFootnoteFrame();
    if ( pFootnote && pFootnote->GetFollow() )
    {
        SwFootnoteBossFrame* pTmpBoss = pFootnote->GetFollow()->FindFootnoteBossFrame();
        // Following cases will be handled:
        // 1. both "FootnoteBoss"es are neighboring columns/pages
        // 2. the new one is the first column of a neighboring page
        // 3. the new one is the first column in a section of the next page
        while( pTmpBoss != pBoss && pTmpBoss && !pTmpBoss->GetPrev() )
            pTmpBoss = pTmpBoss->GetUpper()->FindFootnoteBossFrame();
        if( pTmpBoss == pBoss )
            return pFootnote->GetFollow();
    }

    // If no pBoss could be found or it is a "wrong" page, we need a new page.
    if ( !pBoss || ( pPage && pPage->IsEndNotePage() && !pOldPage->IsEndNotePage() ) )
    {
        if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
        {
            pBoss = InsertPage( pOldPage, pOldPage->IsFootnotePage() );
            static_cast<SwPageFrame*>(pBoss)->SetEndNotePage( pOldPage->IsEndNotePage() );
        }
        else
            return nullptr;
    }
    if( pBoss->IsPageFrame() )
    {
        // If this page has columns, then go to the first one
        SwLayoutFrame* pLay = pBoss->FindBodyCont();
        if( pLay && pLay->Lower() && pLay->Lower()->IsColumnFrame() )
            pBoss = static_cast<SwFootnoteBossFrame*>(pLay->Lower());
    }
    // found column/page - add myself
    SwFootnoteContFrame *pCont = pBoss->FindFootnoteCont();
    if ( !pCont && pBoss->GetMaxFootnoteHeight() &&
         ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        pCont = pBoss->MakeFootnoteCont();
    return pCont;
}

/// Get the preceding layout leaf in that the frame can be moved.
SwLayoutFrame *SwFrame::GetPrevFootnoteLeaf( MakePageType eMakeFootnote )
{
    // The predecessor of a footnote is (if possible)
    // the master of the chain of the footnote.
    SwFootnoteFrame *pFootnote = FindFootnoteFrame();
    SwLayoutFrame *pRet = pFootnote->GetMaster();

    SwFootnoteBossFrame* pOldBoss = FindFootnoteBossFrame();
    SwPageFrame *pOldPage = pOldBoss->FindPageFrame();

    if ( !pOldBoss->GetPrev() && !pOldPage->GetPrev() )
        return pRet; // there is neither a predecessor column nor page

    if ( !pRet )
    {
        bool bEndn = pFootnote->GetAttr()->GetFootnote().IsEndNote();
        SwFrame* pTmpRef = nullptr;
        if( bEndn && pFootnote->IsInSct() )
        {
            SwSectionFrame* pSect = pFootnote->FindSctFrame();
            if( pSect->IsEndnAtEnd() )
                pTmpRef = pSect->FindLastContent( SwFindMode::LastCnt );
        }
        if( !pTmpRef )
            pTmpRef = pFootnote->GetRef();
        SwFootnoteBossFrame* pStop = pTmpRef->FindFootnoteBossFrame( !bEndn );

        const sal_uInt16 nNum = pStop->GetPhyPageNum();

        // Do not leave the corresponding page if the footnote should
        // be shown at the document ending or the footnote is an endnote.
        const bool bEndNote = pOldPage->IsEndNotePage();
        const bool bFootnoteEndDoc = pOldPage->IsFootnotePage();
        SwFootnoteBossFrame* pNxtBoss = pOldBoss;
        SwSectionFrame *pSect = pNxtBoss->GetUpper()->IsSctFrame() ?
                              static_cast<SwSectionFrame*>(pNxtBoss->GetUpper()) : nullptr;

        do
        {
            if( pNxtBoss->IsColumnFrame() && pNxtBoss->GetPrev() )
                pNxtBoss = static_cast<SwFootnoteBossFrame*>(pNxtBoss->GetPrev());  // one column backwards
            else // one page backwards
            {
                SwLayoutFrame* pBody = nullptr;
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
                        OSL_ENSURE( pSect->Lower()->IsColumnFrame(),
                                "GetPrevFootnoteLeaf: Where's the column?" );
                        pNxtBoss = static_cast<SwFootnoteBossFrame*>(pSect->Lower());
                        pBody = pSect;
                    }
                }
                else
                {
                    SwPageFrame* pPage = static_cast<SwPageFrame*>(pNxtBoss->FindPageFrame()->GetPrev());
                    if( !pPage || pPage->GetPhyPageNum() < nNum ||
                        bEndNote != pPage->IsEndNotePage() || bFootnoteEndDoc != pPage->IsFootnotePage() )
                        return nullptr; // no further pages found
                    pNxtBoss = pPage;
                    pBody = pPage->FindBodyCont();
                }
                // We have the previous page, we might need to find the last column of it
                if( pBody )
                {
                    if ( pBody->Lower() && pBody->Lower()->IsColumnFrame() )
                    {
                        pNxtBoss = static_cast<SwFootnoteBossFrame*>(pBody->GetLastLower());
                    }
                }
            }
            SwFootnoteContFrame *pCont = pNxtBoss->FindFootnoteCont();
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
        const SwFootnoteBossFrame* pNewBoss = pRet->FindFootnoteBossFrame();
        bool bJump = false;
        if( pOldBoss->IsColumnFrame() && pOldBoss->GetPrev() ) // a previous column exists
            bJump = pOldBoss->GetPrev() != static_cast<SwFrame const *>(pNewBoss); // did we chose it?
        else if( pNewBoss->IsColumnFrame() && pNewBoss->GetNext() )
            bJump = true; // there is another column after the boss (not the old boss)
        else
        {
            // Will be reached only if old and new boss are both either pages or the last (new)
            // or first (old) column of a page. In this case, check if pages were skipped.
            const sal_uInt16 nDiff = pOldPage->GetPhyPageNum() - pRet->FindPageFrame()->GetPhyPageNum();
            if ( nDiff > 2 ||
                 (nDiff > 1 && !static_cast<SwPageFrame*>(pOldPage->GetPrev())->IsEmptyPage()) )
                bJump = true;
        }
        if( bJump )
            SwFlowFrame::SetMoveBwdJump( true );
    }
    return pRet;
}

bool SwFrame::IsFootnoteAllowed() const
{
    if ( !IsInDocBody() )
        return false;

    if ( IsInTab() )
    {
        // no footnotes in repeated headlines
        const SwTabFrame *pTab = const_cast<SwFrame*>(this)->ImplFindTabFrame();
        assert(pTab);
        if ( pTab->IsFollow() )
            return !pTab->IsInHeadline( *this );
    }
    return true;
}

void SwRootFrame::UpdateFootnoteNums()
{
    // page numbering only if set at the document
    if ( GetFormat()->GetDoc()->GetFootnoteInfo().eNum == FTNNUM_PAGE )
    {
        SwPageFrame *pPage = static_cast<SwPageFrame*>(Lower());
        while ( pPage && !pPage->IsFootnotePage() )
        {
            pPage->UpdateFootnoteNum();
            pPage = static_cast<SwPageFrame*>(pPage->GetNext());
        }
    }
}

/// remove all footnotes (not the references) and all footnote pages
void sw_RemoveFootnotes( SwFootnoteBossFrame* pBoss, bool bPageOnly, bool bEndNotes )
{
    do
    {
        SwFootnoteContFrame *pCont = pBoss->FindFootnoteCont();
        if ( pCont )
        {
            SwFootnoteFrame *pFootnote = static_cast<SwFootnoteFrame*>(pCont->Lower());
            assert(pFootnote);
            if ( bPageOnly )
                while ( pFootnote->GetMaster() )
                    pFootnote = pFootnote->GetMaster();
            do
            {
                SwFootnoteFrame *pNxt = static_cast<SwFootnoteFrame*>(pFootnote->GetNext());
                if ( !pFootnote->GetAttr()->GetFootnote().IsEndNote() ||
                        bEndNotes )
                {
                    pFootnote->GetRef()->Prepare( PREP_FTN, static_cast<void*>(pFootnote->GetAttr()) );
                    if ( bPageOnly && !pNxt )
                        pNxt = pFootnote->GetFollow();
                    pFootnote->Cut();
                    SwFrame::DestroyFrame(pFootnote);
                }
                pFootnote = pNxt;

            } while ( pFootnote );
        }
        if( !pBoss->IsInSct() )
        {
            // A sectionframe with the Footnote/EndnAtEnd-flags may contain
            // foot/endnotes. If the last lower frame of the bodyframe is
            // a multicolumned sectionframe, it may contain footnotes, too.
            SwLayoutFrame* pBody = pBoss->FindBodyCont();
            if( pBody && pBody->Lower() )
            {
                SwFrame* pLow = pBody->Lower();
                while (pLow)
                {
                    if( pLow->IsSctFrame() && ( !pLow->GetNext() ||
                        static_cast<SwSectionFrame*>(pLow)->IsAnyNoteAtEnd() ) &&
                        static_cast<SwSectionFrame*>(pLow)->Lower() &&
                        static_cast<SwSectionFrame*>(pLow)->Lower()->IsColumnFrame() )
                        sw_RemoveFootnotes( static_cast<SwColumnFrame*>(static_cast<SwSectionFrame*>(pLow)->Lower()),
                            bPageOnly, bEndNotes );
                    pLow = pLow->GetNext();
                }
            }
        }
        // is there another column?
        pBoss = pBoss->IsColumnFrame() ? static_cast<SwColumnFrame*>(pBoss->GetNext()) : nullptr;
    } while( pBoss );
}

void SwRootFrame::RemoveFootnotes( SwPageFrame *pPage, bool bPageOnly, bool bEndNotes )
{
    if ( !pPage )
        pPage = static_cast<SwPageFrame*>(Lower());

    do
    {   // On columned pages we have to clean up in all columns
        SwFootnoteBossFrame* pBoss;
        SwLayoutFrame* pBody = pPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrame() )
            pBoss = static_cast<SwFootnoteBossFrame*>(pBody->Lower()); // the first column
        else
            pBoss = pPage; // no columns
        sw_RemoveFootnotes( pBoss, bPageOnly, bEndNotes );
        if ( !bPageOnly )
        {
            if ( pPage->IsFootnotePage() &&
                 (!pPage->IsEndNotePage() || bEndNotes) )
            {
                SwFrame *pDel = pPage;
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
                pDel->Cut();
                SwFrame::DestroyFrame(pDel);
            }
            else
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
        }
        else
            break;

    } while ( pPage );
}

/// Change the page template of the footnote pages
void SwRootFrame::CheckFootnotePageDescs( bool bEndNote )
{
    SwPageFrame *pPage = static_cast<SwPageFrame*>(Lower());
    while ( pPage && !pPage->IsFootnotePage() )
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    while ( pPage && pPage->IsEndNotePage() != bEndNote )
        pPage = static_cast<SwPageFrame*>(pPage->GetNext());

    if ( pPage )
        SwFrame::CheckPageDescs( pPage, false );
}

/** Insert a footnote container
 *
 * A footnote container is always placed directly behind the body text.
 *
 * The frame format (FrameFormat) is always the default frame format.
 *
 * @return footnote container frame
 */
SwFootnoteContFrame *SwFootnoteBossFrame::MakeFootnoteCont()
{
    SAL_WARN_IF(FindFootnoteCont(), "sw.core", "footnote container exists already");

    SwFootnoteContFrame *pNew = new SwFootnoteContFrame( GetFormat()->GetDoc()->GetDfltFrameFormat(), this );
    SwLayoutFrame *pLay = FindBodyCont();
    pNew->Paste( this, pLay->GetNext() );
    return pNew;
}

SwFootnoteContFrame *SwFootnoteBossFrame::FindFootnoteCont()
{
    SwFrame *pFrame = Lower();
    while( pFrame && !pFrame->IsFootnoteContFrame() )
        pFrame = pFrame->GetNext();

#if OSL_DEBUG_LEVEL > 0
    if ( pFrame )
    {
        SwFrame *pFootnote = pFrame->GetLower();
        assert(pFootnote);
        while ( pFootnote )
        {
            assert(pFootnote->IsFootnoteFrame() && "Neighbor of footnote must be a footnote");
            pFootnote = pFootnote->GetNext();
        }
    }
#endif

    return static_cast<SwFootnoteContFrame*>(pFrame);
}

/// Search the next available footnote container.
SwFootnoteContFrame *SwFootnoteBossFrame::FindNearestFootnoteCont( bool bDontLeave )
{
    SwFootnoteContFrame *pCont = nullptr;
    if ( !GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
    {
        pCont = FindFootnoteCont();
        if ( !pCont )
        {
            SwPageFrame *pPage = FindPageFrame();
            SwFootnoteBossFrame* pBoss = this;
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

SwFootnoteFrame *SwFootnoteBossFrame::FindFirstFootnote()
{
    // search for the nearest footnote container
    SwFootnoteContFrame *pCont = FindNearestFootnoteCont();
    if ( !pCont )
        return nullptr;

    // Starting from the first footnote, search the first
    // footnote that is referenced by the current column/page

    SwFootnoteFrame *pRet = static_cast<SwFootnoteFrame*>(pCont->Lower());
    const sal_uInt16 nRefNum = FindPageFrame()->GetPhyPageNum();
    const sal_uInt16 nRefCol = lcl_ColumnNum( this );
    sal_uInt16 nPgNum, nColNum; // page number, column number
    SwFootnoteBossFrame* pBoss;
    SwPageFrame* pPage;
    if( pRet )
    {
        pBoss = pRet->GetRef()->FindFootnoteBossFrame();
        OSL_ENSURE( pBoss, "FindFirstFootnote: No boss found" );
        if( !pBoss )
            return nullptr; // ?There must be a bug, but no GPF
        pPage = pBoss->FindPageFrame();
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

        SwFootnoteFrame *pNxt = static_cast<SwFootnoteFrame*>(pRet->GetNext());
        if ( !pNxt )
        {
            pBoss = pRet->FindFootnoteBossFrame();
            pPage = pBoss->FindPageFrame();
            lcl_NextFootnoteBoss( pBoss, pPage, false ); // next FootnoteBoss
            pCont = pBoss ? pBoss->FindNearestFootnoteCont() : nullptr;
            if ( pCont )
                pNxt = static_cast<SwFootnoteFrame*>(pCont->Lower());
        }
        if ( pNxt )
        {
            pRet = pNxt;
            pBoss = pRet->GetRef()->FindFootnoteBossFrame();
            pPage = pBoss->FindPageFrame();
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
const SwFootnoteFrame *SwFootnoteBossFrame::FindFirstFootnote( SwContentFrame const *pCnt ) const
{
    const SwFootnoteFrame *pRet = const_cast<SwFootnoteBossFrame*>(this)->FindFirstFootnote();
    if ( pRet )
    {
        const sal_uInt16 nColNum = lcl_ColumnNum( this );
        const sal_uInt16 nPageNum = GetPhyPageNum();
        while ( pRet && (pRet->GetRef() != pCnt) )
        {
            while ( pRet->GetFollow() )
                pRet = pRet->GetFollow();

            if ( pRet->GetNext() )
                pRet = static_cast<const SwFootnoteFrame*>(pRet->GetNext());
            else
            {   SwFootnoteBossFrame *pBoss = const_cast<SwFootnoteBossFrame*>(pRet->FindFootnoteBossFrame());
                SwPageFrame *pPage = pBoss->FindPageFrame();
                lcl_NextFootnoteBoss( pBoss, pPage, false ); // next FootnoteBoss
                SwFootnoteContFrame *pCont = pBoss ? pBoss->FindNearestFootnoteCont() : nullptr;
                pRet = pCont ? static_cast<SwFootnoteFrame*>(pCont->Lower()) : nullptr;
            }
            if ( pRet )
            {
                const SwFootnoteBossFrame* pBoss = pRet->GetRef()->FindFootnoteBossFrame();
                if( pBoss->GetPhyPageNum() != nPageNum ||
                    nColNum != lcl_ColumnNum( pBoss ) )
                pRet = nullptr;
            }
        }
    }
    return pRet;
}

void SwFootnoteBossFrame::ResetFootnote( const SwFootnoteFrame *pCheck )
{
    // Destroy the incarnations of footnotes to an attribute, if they don't
    // belong to pAssumed
    OSL_ENSURE( !pCheck->GetMaster(), "given master is not a Master." );

    SwNodeIndex aIdx( *pCheck->GetAttr()->GetStartNode(), 1 );
    SwContentNode *pNd = aIdx.GetNode().GetContentNode();
    if ( !pNd )
        pNd = pCheck->GetFormat()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, true, false );
    SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*pNd);
    SwFrame* pFrame = aIter.First();
    while( pFrame )
    {
        if( pFrame->getRootFrame() == pCheck->getRootFrame() )
        {
            SwFrame *pTmp = pFrame->GetUpper();
            while ( pTmp && !pTmp->IsFootnoteFrame() )
                pTmp = pTmp->GetUpper();

            SwFootnoteFrame *pFootnote = static_cast<SwFootnoteFrame*>(pTmp);
            while ( pFootnote && pFootnote->GetMaster() )
                pFootnote = pFootnote->GetMaster();
            if ( pFootnote != pCheck )
            {
                while ( pFootnote )
                {
                    SwFootnoteFrame *pNxt = pFootnote->GetFollow();
                    pFootnote->Cut();
                    SwFrame::DestroyFrame(pFootnote);
                    pFootnote = pNxt;
                }
            }
        }

        pFrame = aIter.Next();
    }
}

void SwFootnoteBossFrame::InsertFootnote( SwFootnoteFrame* pNew )
{
    // Place the footnote in front of the footnote whose attribute
    // is in front of the new one (get position via the Doc).
    // If there is no footnote in this footnote-boss yet, create a new container.
    // If there is a container but no footnote for this footnote-boss yet, place
    // the footnote behind the last footnote of the closest previous column/page.

    ResetFootnote( pNew );
    SwFootnoteFrame *pSibling = FindFirstFootnote();
    bool bDontLeave = false;

    // Ok, a sibling has been found, but is the sibling in an acceptable
    // environment?
    if( IsInSct() )
    {
        SwSectionFrame* pMySect = ImplFindSctFrame();
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
                        !pSibling->ImplFindSctFrame()->IsDescendantFrom( pEndFormat ) )
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
                        !pMySect->IsAnFollow( pSibling->ImplFindSctFrame() ) )
                        pSibling = nullptr;
                }
                else if( pSibling->IsInSct() )
                    pSibling = nullptr;
            }
        }
    }

    if( pSibling && pSibling->FindPageFrame()->IsEndNotePage() !=
        FindPageFrame()->IsEndNotePage() )
        pSibling = nullptr;

    // use the Doc to find out the position
    SwDoc *pDoc = GetFormat()->GetDoc();
    const sal_uLong nStPos = ::lcl_FindFootnotePos( pDoc, pNew->GetAttr() );

    sal_uLong nCmpPos = 0;
    sal_uLong nLastPos = 0;
    SwFootnoteContFrame *pParent = nullptr;
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
                SwFootnoteFrame *pFootnote = static_cast<SwFootnoteFrame*>(pParent->Lower());
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
            pSibling = static_cast<SwFootnoteFrame*>(pParent->Lower());
            if ( !pSibling )
            {
                OSL_ENSURE( false, "Could not find space for footnote.");
                return;
            }
            nCmpPos  = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );

            SwFootnoteBossFrame *pNxtB; // remember the last one to not
            SwFootnoteFrame  *pLastSib = nullptr;    // go too far.

            while ( pSibling && nCmpPos <= nStPos )
            {
                pLastSib = pSibling; // potential candidate
                nLastPos = nCmpPos;

                while ( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();

                if ( pSibling->GetNext() )
                {
                    pSibling = static_cast<SwFootnoteFrame*>(pSibling->GetNext());
                    OSL_ENSURE( !pSibling->GetMaster() || ( ENDNOTE > nStPos &&
                            pSibling->GetAttr()->GetFootnote().IsEndNote() ),
                            "InsertFootnote: Master expected I" );
                }
                else
                {
                    pNxtB = pSibling->FindFootnoteBossFrame();
                    SwPageFrame *pSibPage = pNxtB->FindPageFrame();
                    bool bEndNote = pSibPage->IsEndNotePage();
                    bool bChgPage = lcl_NextFootnoteBoss( pNxtB, pSibPage, bDontLeave );
                    // When changing pages, also the endnote flag must match.
                    SwFootnoteContFrame *pCont = pNxtB && ( !bChgPage ||
                        pSibPage->IsEndNotePage() == bEndNote )
                        ? pNxtB->FindNearestFootnoteCont( bDontLeave ) : nullptr;
                    if( pCont )
                        pSibling = static_cast<SwFootnoteFrame*>(pCont->Lower());
                    else // no further FootnoteContainer, insert after pSibling
                        break;
                }
                if ( pSibling )
                {
                    nCmpPos = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );
                    OSL_ENSURE( nCmpPos > nLastPos, "InsertFootnote: Order of FootnoteFrame's buggy" );
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
        SwFootnoteBossFrame* pBoss = pNew->GetRef()->FindFootnoteBossFrame(
            !pNew->GetAttr()->GetFootnote().IsEndNote() );
        sal_uInt16 nRefNum = pBoss->GetPhyPageNum();    // page number of the new footnote
        sal_uInt16 nRefCol = lcl_ColumnNum( pBoss );    // column number of the new footnote
        bool bEnd = false;
        SwFootnoteFrame *pLastSib = nullptr;
        while ( pSibling && !bEnd && (nCmpPos <= nStPos) )
        {
            pLastSib = pSibling;
            nLastPos = nCmpPos;

            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();

            SwFootnoteFrame *pFoll = static_cast<SwFootnoteFrame*>(pSibling->GetNext());
            if ( pFoll )
            {
                pBoss = pSibling->GetRef()->FindFootnoteBossFrame( !pSibling->
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
                SwFootnoteBossFrame* pNxtB = pSibling->FindFootnoteBossFrame();
                SwPageFrame *pSibPage = pNxtB->FindPageFrame();
                bool bEndNote = pSibPage->IsEndNotePage();
                bool bChgPage = lcl_NextFootnoteBoss( pNxtB, pSibPage, bDontLeave );
                // When changing pages, also the endnote flag must match.
                SwFootnoteContFrame *pCont = pNxtB && ( !bChgPage ||
                    pSibPage->IsEndNotePage() == bEndNote )
                    ? pNxtB->FindNearestFootnoteCont( bDontLeave ) : nullptr;
                if ( pCont )
                    pSibling = static_cast<SwFootnoteFrame*>(pCont->Lower());
                else
                    bEnd = true;
            }
            if ( !bEnd && pSibling )
                nCmpPos = ::lcl_FindFootnotePos( pDoc, pSibling->GetAttr() );
            if (pSibling && (pSibling != pLastSib))
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
            pParent = static_cast<SwFootnoteContFrame*>(pSibling->GetUpper());
            pSibling = static_cast<SwFootnoteFrame*>(pSibling->GetNext());
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
            pParent = static_cast<SwFootnoteContFrame*>(pSibling->GetUpper());
        }
    }
    OSL_ENSURE( pParent, "paste in space?" );
    pNew->Paste( pParent, pSibling );
}

void SwFootnoteBossFrame::AppendFootnote( SwContentFrame *pRef, SwTextFootnote *pAttr )
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
    SwFootnoteBossFrame *pBoss = this;
    SwPageFrame *pPage = FindPageFrame();
    SwPageFrame *pMyPage = pPage;
    bool bChgPage = false;
    bool bEnd = false;
    if ( pAttr->GetFootnote().IsEndNote() )
    {
        bEnd = true;
        if( GetUpper()->IsSctFrame() &&
            static_cast<SwSectionFrame*>(GetUpper())->IsEndnAtEnd() )
        {
            SwFrame* pLast =
                static_cast<SwSectionFrame*>(GetUpper())->FindLastContent( SwFindMode::EndNote );
            if( pLast )
            {
                pBoss = pLast->FindFootnoteBossFrame();
                pPage = pBoss->FindPageFrame();
            }
        }
        else
        {
            while ( pPage->GetNext() && !pPage->IsEndNotePage() )
            {
                pPage = static_cast<SwPageFrame*>(pPage->GetNext());
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
                SwPageFrame *pNxt = static_cast<SwPageFrame*>(pPage->GetNext());
                const sal_uLong nStPos = ::lcl_FindFootnotePos( pDoc, pAttr );
                while ( pNxt && pNxt->IsEndNotePage() )
                {
                    SwFootnoteContFrame *pCont = pNxt->FindFootnoteCont();
                    if ( pCont && pCont->Lower() )
                    {
                        OSL_ENSURE( pCont->Lower()->IsFootnoteFrame(), "no footnote in the container" );
                        if ( nStPos > ::lcl_FindFootnotePos( pDoc,
                                        static_cast<SwFootnoteFrame*>(pCont->Lower())->GetAttr()))
                        {
                            pPage = pNxt;
                            pNxt = static_cast<SwPageFrame*>(pPage->GetNext());
                            continue;
                        }
                    }
                    break;
                }
            }
        }
    }
    else if( FTNPOS_CHAPTER == pDoc->GetFootnoteInfo().ePos && ( !GetUpper()->
             IsSctFrame() || !static_cast<SwSectionFrame*>(GetUpper())->IsFootnoteAtEnd() ) )
    {
        while ( pPage->GetNext() && !pPage->IsFootnotePage() &&
                !static_cast<SwPageFrame*>(pPage->GetNext())->IsEndNotePage() )
        {
            pPage = static_cast<SwPageFrame*>(pPage->GetNext());
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
            SwPageFrame *pNxt = static_cast<SwPageFrame*>(pPage->GetNext());
            const sal_uLong nStPos = ::lcl_FindFootnotePos( pDoc, pAttr );
            while ( pNxt && pNxt->IsFootnotePage() && !pNxt->IsEndNotePage() )
            {
                SwFootnoteContFrame *pCont = pNxt->FindFootnoteCont();
                if ( pCont && pCont->Lower() )
                {
                    OSL_ENSURE( pCont->Lower()->IsFootnoteFrame(), "no footnote in the container" );
                    if ( nStPos > ::lcl_FindFootnotePos( pDoc,
                                        static_cast<SwFootnoteFrame*>(pCont->Lower())->GetAttr()))
                    {
                        pPage = pNxt;
                        pNxt = static_cast<SwPageFrame*>(pPage->GetNext());
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
    if( pBoss->IsInSct() && pBoss->IsColumnFrame() && !pPage->IsFootnotePage() )
    {
        SwSectionFrame* pSct = pBoss->FindSctFrame();
        if( bEnd ? !pSct->IsEndnAtEnd() : !pSct->IsFootnoteAtEnd() )
        {
            SwFootnoteContFrame* pFootnoteCont = pSct->FindFootnoteBossFrame(!bEnd)->FindFootnoteCont();
            if( pFootnoteCont )
            {
                SwFootnoteFrame* pTmp = static_cast<SwFootnoteFrame*>(pFootnoteCont->Lower());
                if( bEnd )
                    while( pTmp && !pTmp->GetAttr()->GetFootnote().IsEndNote() )
                        pTmp = static_cast<SwFootnoteFrame*>(pTmp->GetNext());
                if( pTmp && *pTmp < pAttr )
                    return;
            }
        }
    }

    SwFootnoteFrame *pNew = new SwFootnoteFrame( pDoc->GetDfltFrameFormat(), this, pRef, pAttr );
    {
        SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
        ::InsertCnt_( pNew, pDoc, aIdx.GetIndex() );
    }
    // If the page was changed or newly created,
    // we need to place ourselves in the first column
    if( bChgPage )
    {
        SwLayoutFrame* pBody = pPage->FindBodyCont();
        OSL_ENSURE( pBody, "AppendFootnote: NoPageBody?" );
        if( pBody->Lower() && pBody->Lower()->IsColumnFrame() )
            pBoss = static_cast<SwFootnoteBossFrame*>(pBody->Lower());
        else
            pBoss = pPage; // page if no columns exist
    }
    pBoss->InsertFootnote( pNew );
    if ( pNew->GetUpper() ) // inserted or not?
    {
        ::RegistFlys( pNew->FindPageFrame(), pNew );
        SwSectionFrame* pSect = FindSctFrame();
        // The content of a FootnoteContainer in a (column) section only need to be calculated
        // if the section stretches already to the bottom edge of the Upper.
        if( pSect && !pSect->IsJoinLocked() && ( bEnd ? !pSect->IsEndnAtEnd() :
            !pSect->IsFootnoteAtEnd() ) && pSect->Growable() )
            pSect->InvalidateSize();
        else
        {
            // #i49383# - disable unlock of position of
            // lower objects during format of footnote content.
            const bool bOldFootnoteFrameLocked( pNew->IsColLocked() );
            pNew->ColLock();
            pNew->KeepLockPosOfLowerObjs();
            // #i57914# - adjust fix #i49383#
            SwContentFrame *pCnt = pNew->ContainsContent();
            while ( pCnt && pCnt->FindFootnoteFrame()->GetAttr() == pAttr )
            {
                pCnt->Calc(getRootFrame()->GetCurrShell()->GetOut());
                // #i49383# - format anchored objects
                if ( pCnt->IsTextFrame() && pCnt->isFrameAreaDefinitionValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrame( *pCnt,
                                                              *(pCnt->FindPageFrame()) ) )
                    {
                        // restart format with first content
                        pCnt = pNew->ContainsContent();
                        continue;
                    }
                }
                pCnt = pCnt->FindNextCnt();
            }
            // #i49383#
            if ( !bOldFootnoteFrameLocked )
            {
                pNew->ColUnlock();
            }
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pNew->UnlockPosOfLowerObjs();
            pNew->Calc(getRootFrame()->GetCurrShell()->GetOut());
            // #i57914# - adjust fix #i49383#
            if ( !bOldFootnoteFrameLocked && !pNew->GetLower() &&
                 !pNew->IsColLocked() && !pNew->IsBackMoveLocked() &&
                 !pNew->IsDeleteForbidden() )
            {
                pNew->Cut();
                SwFrame::DestroyFrame(pNew);
            }
        }
        pMyPage->UpdateFootnoteNum();
    }
    else
        SwFrame::DestroyFrame(pNew);
}

SwFootnoteFrame *SwFootnoteBossFrame::FindFootnote( const SwContentFrame *pRef, const SwTextFootnote *pAttr )
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
    SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*pNd);
    SwFrame* pFrame = aIter.First();
    if( pFrame )
        do
        {
                pFrame = pFrame->GetUpper();
                // #i28500#, #i27243# Due to the endnode collector, there are
                // SwFootnoteFrames, which are not in the layout. Therefore the
                // bInfFootnote flags are not set correctly, and a cell of FindFootnoteFrame
                // would return 0. Therefore we better call ImplFindFootnoteFrame().
                SwFootnoteFrame *pFootnote = pFrame->ImplFindFootnoteFrame();
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

        } while ( nullptr != (pFrame = aIter.Next()) );

    return nullptr;
}

bool SwFootnoteBossFrame::RemoveFootnote(
        const SwContentFrame *const pRef, const SwTextFootnote *const pAttr,
                              bool bPrep )
{
    bool ret(false);
    SwFootnoteFrame *pFootnote = FindFootnote( pRef, pAttr );
    if( pFootnote )
    {
        ret = true;
        do
        {
            SwFootnoteFrame *pFoll = pFootnote->GetFollow();
            pFootnote->Cut();
            SwFrame::DestroyFrame(pFootnote);
            pFootnote = pFoll;
        } while ( pFootnote );
        if( bPrep && pRef->IsFollow() )
        {
            OSL_ENSURE( pRef->IsTextFrame(), "NoTextFrame has Footnote?" );
            SwTextFrame* pMaster = pRef->FindMaster();
            if( !pMaster->IsLocked() )
                pMaster->Prepare( PREP_FTN_GONE );
        }
    }
    FindPageFrame()->UpdateFootnoteNum();
    return ret;
}

void SwFootnoteBossFrame::ChangeFootnoteRef( const SwContentFrame *pOld, const SwTextFootnote *pAttr,
                                 SwContentFrame *pNew )
{
    SwFootnoteFrame *pFootnote = FindFootnote( pOld, pAttr );
    while ( pFootnote )
    {
        pFootnote->SetRef( pNew );
        pFootnote = pFootnote->GetFollow();
    }
}

/// OD 03.04.2003 #108446# - add parameter <_bCollectOnlyPreviousFootnotes> in
/// order to control, if only footnotes, which are positioned before the
/// footnote boss frame <this> have to be collected.
void SwFootnoteBossFrame::CollectFootnotes( const SwContentFrame* _pRef,
                                SwFootnoteBossFrame*     _pOld,
                                SwFootnoteFrames&        _rFootnoteArr,
                                const bool    _bCollectOnlyPreviousFootnotes )
{
    SwFootnoteFrame *pFootnote = _pOld->FindFirstFootnote();
    while( !pFootnote )
    {
        if( _pOld->IsColumnFrame() )
        {
            // visit columns
            while ( !pFootnote && _pOld->GetPrev() )
            {
                // Still no problem if no footnote was found yet. The loop is needed to pick up
                // following rows in tables. In all other cases it might correct bad contexts.
                _pOld = static_cast<SwFootnoteBossFrame*>(_pOld->GetPrev());
                pFootnote = _pOld->FindFirstFootnote();
            }
        }
        if( !pFootnote )
        {
            // previous page
            SwPageFrame* pPg;
            for ( SwFrame* pTmp = _pOld;
                  nullptr != ( pPg = static_cast<SwPageFrame*>(pTmp->FindPageFrame()->GetPrev()))
                    && pPg->IsEmptyPage() ;
                )
            {
                pTmp = pPg;
            }
            if( !pPg )
                return;

            SwLayoutFrame* pBody = pPg->FindBodyCont();
            if( pBody->Lower() && pBody->Lower()->IsColumnFrame() )
            {
                // multiple columns on one page => search last column
                _pOld = static_cast<SwFootnoteBossFrame*>(pBody->GetLastLower());
            }
            else
                _pOld = pPg; // single column page
            pFootnote = _pOld->FindFirstFootnote();
        }
    }
    // OD 03.04.2003 #108446# - consider new parameter <_bCollectOnlyPreviousFootnotes>
    SwFootnoteBossFrame* pRefBossFrame = nullptr;
    if ( _bCollectOnlyPreviousFootnotes )
    {
        pRefBossFrame = this;
    }
    CollectFootnotes_( _pRef, pFootnote, _rFootnoteArr, _bCollectOnlyPreviousFootnotes, pRefBossFrame );
}

static void FootnoteInArr( SwFootnoteFrames& rFootnoteArr, SwFootnoteFrame* pFootnote )
{
    if ( rFootnoteArr.end() == std::find( rFootnoteArr.begin(), rFootnoteArr.end(), pFootnote ) )
        rFootnoteArr.push_back( pFootnote );
}

/// OD 03.04.2003 #108446# - add parameters <_bCollectOnlyPreviousFootnotes> and
/// <_pRefFootnoteBossFrame> in order to control, if only footnotes, which are positioned
/// before the given reference footnote boss frame have to be collected.
/// Note: if parameter <_bCollectOnlyPreviousFootnotes> is true, then parameter
/// <_pRefFootnoteBossFrame> have to be referenced to an object.
/// Adjust parameter names.
void SwFootnoteBossFrame::CollectFootnotes_( const SwContentFrame*   _pRef,
                                 SwFootnoteFrame*           _pFootnote,
                                 SwFootnoteFrames&          _rFootnoteArr,
                                 bool                _bCollectOnlyPreviousFootnotes,
                                 const SwFootnoteBossFrame* _pRefFootnoteBossFrame)
{
    // OD 03.04.2003 #108446# - assert, that no reference footnote boss frame
    // is set, in spite of the order, that only previous footnotes has to be
    // collected.
    OSL_ENSURE( !_bCollectOnlyPreviousFootnotes || _pRefFootnoteBossFrame,
            "<SwFootnoteBossFrame::CollectFootnotes_(..)> - No reference footnote boss frame for collecting only previous footnotes set.\nCrash will be caused!" );

    // Collect all footnotes referenced by pRef (attribute by attribute), combine them
    // (the content might be divided over multiple pages) and cut them.

    // For robustness, we do not log the corresponding footnotes here. If a footnote
    // is touched twice, there might be a crash. This allows this function here to
    // also handle corrupt layouts in some degrees (without loops or even crashes).
    SwFootnoteFrames aNotFootnoteArr;

    // here we have a footnote placed in front of the first one of the reference
    OSL_ENSURE( !_pFootnote->GetMaster() || _pFootnote->GetRef() != _pRef, "move FollowFootnote?" );
    while ( _pFootnote->GetMaster() )
        _pFootnote = _pFootnote->GetMaster();

    bool bFound = false;

    do
    {
        // Search for the next footnote in this column/page so that
        // we do not start from zero again after cutting one footnote.
        SwFootnoteFrame *pNxtFootnote = _pFootnote;
        while ( pNxtFootnote->GetFollow() )
            pNxtFootnote = pNxtFootnote->GetFollow();
        pNxtFootnote = static_cast<SwFootnoteFrame*>(pNxtFootnote->GetNext());

        if ( !pNxtFootnote )
        {
            SwFootnoteBossFrame* pBoss = _pFootnote->FindFootnoteBossFrame();
            SwPageFrame* pPage = pBoss->FindPageFrame();
            do
            {
                lcl_NextFootnoteBoss( pBoss, pPage, false );
                if( pBoss )
                {
                    SwLayoutFrame* pCont = pBoss->FindFootnoteCont();
                    if( pCont )
                    {
                        pNxtFootnote = static_cast<SwFootnoteFrame*>(pCont->Lower());
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
        {
            OSL_ENSURE( !pNxtFootnote->GetMaster(), "_CollectFootnote: Master expected" );
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
                SwFootnoteBossFrame* pBossOfFoundFootnote = _pFootnote->FindFootnoteBossFrame( true );
                OSL_ENSURE( pBossOfFoundFootnote,
                        "<SwFootnoteBossFrame::CollectFootnotes_(..)> - footnote boss frame of found footnote frame missing.\nWrong layout!" );
                if ( !pBossOfFoundFootnote ||    // don't crash, if no footnote boss is found.
                     pBossOfFoundFootnote->IsBefore( _pRefFootnoteBossFrame )
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
            SwFootnoteFrame *pNxt = _pFootnote->GetFollow();
            while ( pNxt )
            {
                SwFrame *pCnt = pNxt->ContainsAny();
                if ( pCnt )
                {
                    // destroy the follow on the way as it is empty
                    do
                    {   SwFrame *pNxtCnt = pCnt->GetNext();
                        pCnt->Cut();
                        pCnt->Paste( _pFootnote );
                        pCnt = pNxtCnt;
                    } while ( pCnt );
                }
                else
                {
                    OSL_ENSURE( !pNxt, "footnote without content?" );
                    pNxt->Cut();
                    SwFrame::DestroyFrame(pNxt);
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

void SwFootnoteBossFrame::MoveFootnotes_( SwFootnoteFrames &rFootnoteArr, bool bCalc )
{
    // All footnotes referenced by pRef need to be moved
    // to a new position (based on the new column/page)
    const sal_uInt16 nMyNum = FindPageFrame()->GetPhyPageNum();
    const sal_uInt16 nMyCol = lcl_ColumnNum( this );
    SwRectFnSet aRectFnSet(this);

    // #i21478# - keep last inserted footnote in order to
    // format the content of the following one.
    SwFootnoteFrame* pLastInsertedFootnote = nullptr;
    for (SwFootnoteFrame* pFootnote : rFootnoteArr)
    {
        SwFootnoteBossFrame* pRefBoss(pFootnote->GetRef()->FindFootnoteBossFrame(
                !pFootnote->GetAttr()->GetFootnote().IsEndNote()));
        if( pRefBoss != this )
        {
            const sal_uInt16 nRefNum = pRefBoss->FindPageFrame()->GetPhyPageNum();
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
            SwFrame *pCnt = pFootnote->ContainsAny();
            while( pCnt )
            {
                if( pCnt->IsLayoutFrame() )
                {
                    SwFrame* pTmp = static_cast<SwLayoutFrame*>(pCnt)->ContainsAny();
                    while( pTmp && static_cast<SwLayoutFrame*>(pCnt)->IsAnLower( pTmp ) )
                    {
                        pTmp->Prepare( PREP_MOVEFTN );

                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pTmp);
                        aRectFnSet.SetHeight(aFrm, 0);

                        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pTmp);
                        aRectFnSet.SetHeight(aPrt, 0);

                        pTmp = pTmp->FindNext();
                    }
                }
                else
                {
                    pCnt->Prepare( PREP_MOVEFTN );
                }

                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pCnt);
                aRectFnSet.SetHeight(aFrm, 0);

                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pCnt);
                aRectFnSet.SetHeight(aPrt, 0);

                pCnt = pCnt->GetNext();
            }

            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFootnote);
                aRectFnSet.SetHeight(aFrm, 0);
            }

            {
                SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pFootnote);
                aRectFnSet.SetHeight(aPrt, 0);
            }

            pFootnote->Calc(getRootFrame()->GetCurrShell()->GetOut());
            pFootnote->GetUpper()->Calc(getRootFrame()->GetCurrShell()->GetOut());

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

                while ( pCnt && pCnt->FindFootnoteFrame()->GetAttr() == pAttr )
                {
                    pCnt->InvalidatePos_();
                    pCnt->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTextFrame() && pCnt->isFrameAreaDefinitionValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrame( *pCnt,
                                                                  *(pCnt->FindPageFrame()) ) )
                        {
                            // restart format with first content
                            pCnt = pFootnote->ContainsAny();
                            continue;
                        }
                    }
                    if( pCnt->IsSctFrame() )
                    {
                        // If the area is not empty, iterate also over the content
                        SwFrame* pTmp = static_cast<SwSectionFrame*>(pCnt)->ContainsAny();
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
                        SwFrame::DestroyFrame(pFootnote);
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
                    pFootnote->Calc(getRootFrame()->GetCurrShell()->GetOut());
                }
            }
        }
        else
        {
            OSL_ENSURE( !pFootnote->GetMaster() && !pFootnote->GetFollow(),
                    "DelFootnote and Master/Follow?" );
            SwFrame::DestroyFrame(pFootnote);
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
            SwFootnoteFrame* pNextFootnote = static_cast<SwFootnoteFrame*>(pLastInsertedFootnote->GetNext());
            SwTextFootnote* pAttr = pNextFootnote->GetAttr();
            SwFrame* pCnt = pNextFootnote->ContainsAny();

            bool bUnlock = !pNextFootnote->IsBackMoveLocked();
            pNextFootnote->LockBackMove();
            // #i49383# - disable unlock of position of
            // lower objects during format of footnote content.
            pNextFootnote->KeepLockPosOfLowerObjs();
            // #i57914# - adjust fix #i49383#

            while ( pCnt && pCnt->FindFootnoteFrame()->GetAttr() == pAttr )
            {
                pCnt->InvalidatePos_();
                pCnt->Calc(getRootFrame()->GetCurrShell()->GetOut());
                // #i49383# - format anchored objects
                if ( pCnt->IsTextFrame() && pCnt->isFrameAreaDefinitionValid() )
                {
                    if ( !SwObjectFormatter::FormatObjsAtFrame( *pCnt,
                                                              *(pCnt->FindPageFrame()) ) )
                    {
                        // restart format with first content
                        pCnt = pNextFootnote->ContainsAny();
                        continue;
                    }
                }
                if( pCnt->IsSctFrame() )
                {
                    // If the area is not empty, iterate also over the content
                    SwFrame* pTmp = static_cast<SwSectionFrame*>(pCnt)->ContainsAny();
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
            pNextFootnote->Calc(getRootFrame()->GetCurrShell()->GetOut());
        }
    }
}

void SwFootnoteBossFrame::MoveFootnotes( const SwContentFrame *pSrc, SwContentFrame *pDest,
                             SwTextFootnote const *pAttr )
{
    if( ( GetFormat()->GetDoc()->GetFootnoteInfo().ePos == FTNPOS_CHAPTER &&
        (!GetUpper()->IsSctFrame() || !static_cast<SwSectionFrame*>(GetUpper())->IsFootnoteAtEnd()))
        || pAttr->GetFootnote().IsEndNote() )
        return;

    OSL_ENSURE( this == pSrc->FindFootnoteBossFrame( true ),
            "SwPageFrame::MoveFootnotes: source frame isn't on that FootnoteBoss" );

    SwFootnoteFrame *pFootnote = FindFirstFootnote();
    if( pFootnote )
    {
        ChangeFootnoteRef( pSrc, pAttr, pDest );
        SwFootnoteBossFrame *pDestBoss = pDest->FindFootnoteBossFrame( true );
        OSL_ENSURE( pDestBoss, "+SwPageFrame::MoveFootnotes: no destination boss" );
        if( pDestBoss )     // robust
        {
            SwFootnoteFrames aFootnoteArr;
            SwFootnoteBossFrame::CollectFootnotes_( pDest, pFootnote, aFootnoteArr );
            if ( !aFootnoteArr.empty() )
            {
                pDestBoss->MoveFootnotes_( aFootnoteArr, true );
                SwPageFrame* pSrcPage = FindPageFrame();
                SwPageFrame* pDestPage = pDestBoss->FindPageFrame();
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

void SwFootnoteBossFrame::RearrangeFootnotes( const SwTwips nDeadLine, const bool bLock,
                                  const SwTextFootnote *pAttr )
{
    // Format all footnotes of a column/page so that they might change the column/page.

    SwSaveFootnoteHeight aSave( this, nDeadLine );
    SwFootnoteFrame *pFootnote = FindFirstFootnote();
    if( pFootnote && pFootnote->GetPrev() && bLock )
    {
        SwFootnoteFrame* pFirst = static_cast<SwFootnoteFrame*>(pFootnote->GetUpper()->Lower());
        SwFrame* pContent = pFirst->ContainsAny();
        if( pContent )
        {
            bool bUnlock = !pFirst->IsBackMoveLocked();
            pFirst->LockBackMove();
            pFirst->Calc(getRootFrame()->GetCurrShell()->GetOut());
            pContent->Calc(getRootFrame()->GetCurrShell()->GetOut());
            // #i49383# - format anchored objects
            if ( pContent->IsTextFrame() && pContent->isFrameAreaDefinitionValid() )
            {
                SwObjectFormatter::FormatObjsAtFrame( *pContent,
                                                    *(pContent->FindPageFrame()) );
            }
            if( bUnlock )
                pFirst->UnlockBackMove();
        }
        pFootnote = FindFirstFootnote();
    }
    SwDoc *pDoc = GetFormat()->GetDoc();
    const sal_uLong nFootnotePos = pAttr ? ::lcl_FindFootnotePos( pDoc, pAttr ) : 0;
    SwFrame *pCnt = pFootnote ? pFootnote->ContainsAny() : nullptr;
    if ( pCnt )
    {
        bool bMore = true;
        bool bStart = pAttr == nullptr; // If no attribute is given, process all
        // #i49383# - disable unlock of position of
        // lower objects during format of footnote and footnote content.
        SwFootnoteFrame* pLastFootnoteFrame( nullptr );
        // footnote frame needs to be locked, if <bLock> isn't set.
        bool bUnlockLastFootnoteFrame( false );
        do
        {
            if( !bStart )
                bStart = ::lcl_FindFootnotePos( pDoc, pCnt->FindFootnoteFrame()->GetAttr() )
                         == nFootnotePos;
            if( bStart )
            {
                pCnt->InvalidatePos_();
                pCnt->InvalidateSize_();
                pCnt->Prepare( PREP_ADJUST_FRM );
                SwFootnoteFrame* pFootnoteFrame = pCnt->FindFootnoteFrame();
                // #i49383#
                if ( pFootnoteFrame != pLastFootnoteFrame )
                {
                    if ( pLastFootnoteFrame )
                    {
                        if ( !bLock && bUnlockLastFootnoteFrame )
                        {
                            pLastFootnoteFrame->ColUnlock();
                        }
                        // #i57914# - adjust fix #i49383#
                        // enable lock of lower object position before format of footnote frame.
                        pLastFootnoteFrame->UnlockPosOfLowerObjs();
                        pLastFootnoteFrame->Calc(getRootFrame()->GetCurrShell()->GetOut());
                        if ( !bLock && bUnlockLastFootnoteFrame &&
                             !pLastFootnoteFrame->GetLower() &&
                             !pLastFootnoteFrame->IsColLocked() &&
                             !pLastFootnoteFrame->IsBackMoveLocked() &&
                             !pLastFootnoteFrame->IsDeleteForbidden() )
                        {
                            pLastFootnoteFrame->Cut();
                            SwFrame::DestroyFrame(pLastFootnoteFrame);
                            pLastFootnoteFrame = nullptr;
                        }
                    }
                    if ( !bLock )
                    {
                        bUnlockLastFootnoteFrame = !pFootnoteFrame->IsColLocked();
                        pFootnoteFrame->ColLock();
                    }
                    pFootnoteFrame->KeepLockPosOfLowerObjs();
                    pLastFootnoteFrame = pFootnoteFrame;
                }
                // OD 30.10.2002 #97265# - invalidate position of footnote
                // frame, if it's below its footnote container, in order to
                // assure its correct position, probably calculating its previous
                // footnote frames.
                {
                    SwRectFnSet aRectFnSet(this);
                    SwFrame* pFootnoteContFrame = pFootnoteFrame->GetUpper();
                    if ( aRectFnSet.TopDist(pFootnoteFrame->getFrameArea(), aRectFnSet.GetPrtBottom(*pFootnoteContFrame)) > 0 )
                    {
                        pFootnoteFrame->InvalidatePos_();
                    }
                }
                if ( bLock )
                {
                    bool bUnlock = !pFootnoteFrame->IsBackMoveLocked();
                    pFootnoteFrame->LockBackMove();
                    pFootnoteFrame->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    pCnt->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTextFrame() && pCnt->isFrameAreaDefinitionValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrame( *pCnt,
                                                                  *(pCnt->FindPageFrame()) ) )
                        {
                            // restart format with first content
                            pCnt = pFootnote->ContainsAny();
                            continue;
                        }
                    }
                    if( bUnlock )
                    {
                        pFootnoteFrame->UnlockBackMove();
                        if( !pFootnoteFrame->Lower() &&
                            !pFootnoteFrame->IsColLocked() )
                        {
                            // #i49383#
                            OSL_ENSURE( pLastFootnoteFrame == pFootnoteFrame,
                                    "<SwFootnoteBossFrame::RearrangeFootnotes(..)> - <pLastFootnoteFrame> != <pFootnoteFrame>" );
                            pLastFootnoteFrame = nullptr;
                            pFootnoteFrame->Cut();
                            SwFrame::DestroyFrame(pFootnoteFrame);
                        }
                    }
                }
                else
                {
                    pFootnoteFrame->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    pCnt->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    // #i49383# - format anchored objects
                    if ( pCnt->IsTextFrame() && pCnt->isFrameAreaDefinitionValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrame( *pCnt,
                                                                  *(pCnt->FindPageFrame()) ) )
                        {
                            // restart format with first content
                            pCnt = pFootnote->ContainsAny();
                            continue;
                        }
                    }
                }
            }
            SwSectionFrame *pDel = nullptr;
            if( pCnt->IsSctFrame() )
            {
                SwFrame* pTmp = static_cast<SwSectionFrame*>(pCnt)->ContainsAny();
                if( pTmp )
                {
                    pCnt = pTmp;
                    continue;
                }
                pDel = static_cast<SwSectionFrame*>(pCnt);
            }
            if ( pCnt->GetNext() )
                pCnt = pCnt->GetNext();
            else
            {
                pCnt = pCnt->FindNext();
                if ( pCnt )
                {
                    SwFootnoteFrame* pFootnoteFrame = pCnt->FindFootnoteFrame();
                    if( pFootnoteFrame->GetRef()->FindFootnoteBossFrame(
                        pFootnoteFrame->GetAttr()->GetFootnote().IsEndNote() ) != this )
                        bMore = false;
                }
                else
                    bMore = false;
            }
            if( pDel )
            {
                bool bUnlockLastFootnoteFrameGuard = pLastFootnoteFrame && !pLastFootnoteFrame->IsColLocked();
                if (bUnlockLastFootnoteFrameGuard)
                    pLastFootnoteFrame->ColLock();
                pDel->Cut();
                if (bUnlockLastFootnoteFrameGuard)
                    pLastFootnoteFrame->ColUnlock();
                SwFrame::DestroyFrame(pDel);
            }
            if ( bMore )
            {
                // Go not further than to the provided footnote (if given)
                if ( pAttr &&
                     (::lcl_FindFootnotePos( pDoc,
                                    pCnt->FindFootnoteFrame()->GetAttr()) > nFootnotePos ) )
                    bMore = false;
            }
        } while ( bMore );
        // #i49383#
        if ( pLastFootnoteFrame )
        {
            if ( !bLock && bUnlockLastFootnoteFrame )
            {
                pLastFootnoteFrame->ColUnlock();
            }
            // #i57914# - adjust fix #i49383#
            // enable lock of lower object position before format of footnote frame.
            pLastFootnoteFrame->UnlockPosOfLowerObjs();
            pLastFootnoteFrame->Calc(getRootFrame()->GetCurrShell()->GetOut());
            if ( !bLock && bUnlockLastFootnoteFrame &&
                 !pLastFootnoteFrame->GetLower() &&
                 !pLastFootnoteFrame->IsColLocked() &&
                 !pLastFootnoteFrame->IsBackMoveLocked() )
            {
                pLastFootnoteFrame->Cut();
                SwFrame::DestroyFrame(pLastFootnoteFrame);
            }
        }
    }
}

void SwPageFrame::UpdateFootnoteNum()
{
    // page numbering only if set at the document
    if ( GetFormat()->GetDoc()->GetFootnoteInfo().eNum != FTNNUM_PAGE )
        return;

    SwLayoutFrame* pBody = FindBodyCont();
    if( !pBody || !pBody->Lower() )
        return;

    SwContentFrame* pContent = pBody->ContainsContent();
    sal_uInt16 nNum = 0;

    while( pContent && pContent->FindPageFrame() == this )
    {
        if( static_cast<SwTextFrame*>(pContent)->HasFootnote() )
        {
            SwFootnoteBossFrame* pBoss = pContent->FindFootnoteBossFrame( true );
            if( pBoss->GetUpper()->IsSctFrame() &&
                static_cast<SwSectionFrame*>(pBoss->GetUpper())->IsOwnFootnoteNum() )
                pContent = static_cast<SwSectionFrame*>(pBoss->GetUpper())->FindLastContent();
            else
            {
                SwFootnoteFrame* pFootnote = const_cast<SwFootnoteFrame*>(pBoss->FindFirstFootnote( pContent ));
                while( pFootnote )
                {
                    SwTextFootnote* pTextFootnote = pFootnote->GetAttr();
                    if( !pTextFootnote->GetFootnote().IsEndNote() &&
                         pTextFootnote->GetFootnote().GetNumStr().isEmpty() &&
                         !pFootnote->GetMaster())
                    {
                        // sw_redlinehide: the layout can only keep one number
                        // up to date; depending on its setting, this is either
                        // the non-hidden or the hidden number; the other
                        // number will simply be preserved as-is (so in case
                        // there are 2 layouts, maybe both can be updated...)
                        ++nNum;
                        sal_uInt16 const nOldNum(pTextFootnote->GetFootnote().GetNumber());
                        sal_uInt16 const nOldNumRLHidden(pTextFootnote->GetFootnote().GetNumberRLHidden());
                        if (getRootFrame()->IsHideRedlines())
                        {
                            if (nNum != nOldNumRLHidden)
                            {
                                pTextFootnote->SetNumber(nOldNum, nNum, OUString());
                            }
                        }
                        else
                        {
                            if (nNum != nOldNum)
                            {
                                pTextFootnote->SetNumber(nNum, nOldNumRLHidden, OUString());
                            }
                        }
                    }
                    if ( pFootnote->GetNext() )
                        pFootnote = static_cast<SwFootnoteFrame*>(pFootnote->GetNext());
                    else
                    {
                        SwFootnoteBossFrame* pTmpBoss = pFootnote->FindFootnoteBossFrame( true );
                        if( pTmpBoss )
                        {
                            SwPageFrame* pPage = pTmpBoss->FindPageFrame();
                            pFootnote = nullptr;
                            lcl_NextFootnoteBoss( pTmpBoss, pPage, false );
                            SwFootnoteContFrame *pCont = pTmpBoss ? pTmpBoss->FindNearestFootnoteCont() : nullptr;
                            if ( pCont )
                                pFootnote = static_cast<SwFootnoteFrame*>(pCont->Lower());
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

void SwFootnoteBossFrame::SetFootnoteDeadLine( const SwTwips nDeadLine )
{
    SwFrame *pBody = FindBodyCont();
    pBody->Calc(getRootFrame()->GetCurrShell()->GetOut());

    SwFrame *pCont = FindFootnoteCont();
    const SwTwips nMax = m_nMaxFootnoteHeight;// current should exceed MaxHeight
    SwRectFnSet aRectFnSet(this);
    if ( pCont )
    {
        pCont->Calc(getRootFrame()->GetCurrShell()->GetOut());
        m_nMaxFootnoteHeight = -aRectFnSet.BottomDist( pCont->getFrameArea(), nDeadLine );
    }
    else
        m_nMaxFootnoteHeight = -aRectFnSet.BottomDist( pBody->getFrameArea(), nDeadLine );

    const SwViewShell *pSh = getRootFrame() ? getRootFrame()->GetCurrShell() : nullptr;
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        m_nMaxFootnoteHeight += pBody->Grow( LONG_MAX, true );
    if ( IsInSct() )
        m_nMaxFootnoteHeight += FindSctFrame()->Grow( LONG_MAX, true );

    if ( m_nMaxFootnoteHeight < 0 )
        m_nMaxFootnoteHeight = 0;
    if ( nMax != LONG_MAX && m_nMaxFootnoteHeight > nMax )
        m_nMaxFootnoteHeight = nMax;
}

SwTwips SwFootnoteBossFrame::GetVarSpace() const
{
    // To not fall below 20% of the page height
    // (in contrast to MSOffice where footnotes can fill a whole column/page)

    const SwPageFrame* pPg = FindPageFrame();
    OSL_ENSURE( pPg || IsInSct(), "Footnote lost page" );

    const SwFrame *pBody = FindBodyCont();
    SwTwips nRet;
    if( pBody )
    {
        SwRectFnSet aRectFnSet(this);
        if( IsInSct() )
        {
            nRet = 0;
            SwTwips nTmp = aRectFnSet.YDiff( aRectFnSet.GetPrtTop(*pBody),
                                               aRectFnSet.GetTop(getFrameArea()) );
            const SwSectionFrame* pSect = FindSctFrame();
            //  Endnotes in a ftncontainer causes a deadline:
            // the bottom of the last contentfrm
            if( pSect->IsEndnAtEnd() ) // endnotes allowed?
            {
                OSL_ENSURE( !Lower() || !Lower()->GetNext() || Lower()->GetNext()->
                        IsFootnoteContFrame(), "FootnoteContainer expected" );
                const SwFootnoteContFrame* pCont = Lower() ?
                    static_cast<const SwFootnoteContFrame*>(Lower()->GetNext()) : nullptr;
                if( pCont )
                {
                    const SwFootnoteFrame* pFootnote = static_cast<const SwFootnoteFrame*>(pCont->Lower());
                    while( pFootnote)
                    {
                        if( pFootnote->GetAttr()->GetFootnote().IsEndNote() )
                        { // endnote found
                            const SwFrame* pFrame = static_cast<const SwLayoutFrame*>(Lower())->Lower();
                            if( pFrame )
                            {
                                while( pFrame->GetNext() )
                                    pFrame = pFrame->GetNext(); // last cntntfrm
                                nTmp += aRectFnSet.YDiff(
                                         aRectFnSet.GetTop(getFrameArea()),
                                         aRectFnSet.GetBottom(pFrame->getFrameArea()) );
                            }
                            break;
                        }
                        pFootnote = static_cast<const SwFootnoteFrame*>(pFootnote->GetNext());
                    }
                }
            }
            if( nTmp < nRet )
                nRet = nTmp;
        }
        else
            nRet = - aRectFnSet.GetHeight(pPg->getFramePrintArea())/5;
        nRet += aRectFnSet.GetHeight(pBody->getFrameArea());
        if( nRet < 0 )
            nRet = 0;
    }
    else
        nRet = 0;
    if ( IsPageFrame() )
    {
        const SwViewShell *pSh = getRootFrame() ? getRootFrame()->GetCurrShell() : nullptr;
        if( pSh && pSh->GetViewOptions()->getBrowseMode() )
        nRet += BROWSE_HEIGHT - getFrameArea().Height();
    }
    return nRet;
}

/** Obtain if pFrame's size adjustment should be processed
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
SwNeighbourAdjust SwFootnoteBossFrame::NeighbourhoodAdjustment_() const
{
    SwNeighbourAdjust nRet = SwNeighbourAdjust::OnlyAdjust;
    if( GetUpper() && !GetUpper()->IsPageBodyFrame() )
    {
        // column sections need grow/shrink
        if( GetUpper()->IsFlyFrame() )
            nRet = SwNeighbourAdjust::GrowShrink;
        else
        {
            OSL_ENSURE( GetUpper()->IsSctFrame(), "NeighbourhoodAdjustment: Unexpected Upper" );
            if( !GetNext() && !GetPrev() )
                nRet = SwNeighbourAdjust::GrowAdjust; // section with a single column (FootnoteAtEnd)
            else
            {
                const SwFrame* pTmp = Lower();
                OSL_ENSURE( pTmp, "NeighbourhoodAdjustment: Missing Lower()" );
                if( !pTmp->GetNext() )
                    nRet = SwNeighbourAdjust::GrowShrink;
                else if( !GetUpper()->IsColLocked() )
                    nRet = SwNeighbourAdjust::AdjustGrow;
                OSL_ENSURE( !pTmp->GetNext() || pTmp->GetNext()->IsFootnoteContFrame(),
                        "NeighbourhoodAdjustment: Who's that guy?" );
            }
        }
    }
    return nRet;
}

void SwPageFrame::SetColMaxFootnoteHeight()
{
    SwLayoutFrame *pBody = FindBodyCont();
    if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrame() )
    {
        SwColumnFrame* pCol = static_cast<SwColumnFrame*>(pBody->Lower());
        do
        {
            pCol->SetMaxFootnoteHeight( GetMaxFootnoteHeight() );
            pCol = static_cast<SwColumnFrame*>(pCol->GetNext());
        } while ( pCol );
    }
}

bool SwLayoutFrame::MoveLowerFootnotes( SwContentFrame *pStart, SwFootnoteBossFrame *pOldBoss,
                                 SwFootnoteBossFrame *pNewBoss, const bool bFootnoteNums )
{
    SwDoc *pDoc = GetFormat()->GetDoc();
    if ( pDoc->GetFootnoteIdxs().empty() )
        return false;
    if( pDoc->GetFootnoteInfo().ePos == FTNPOS_CHAPTER &&
        ( !IsInSct() || !FindSctFrame()->IsFootnoteAtEnd() ) )
        return true;

    if ( !pNewBoss )
        pNewBoss = FindFootnoteBossFrame( true );
    if ( pNewBoss == pOldBoss )
        return false;

    bool bMoved = false;
    if( !pStart )
        pStart = ContainsContent();

    SwFootnoteFrames aFootnoteArr;

    while ( IsAnLower( pStart ) )
    {
        if ( static_cast<SwTextFrame*>(pStart)->HasFootnote() )
        {
            // OD 03.04.2003 #108446# - To avoid unnecessary moves of footnotes
            // use new parameter <_bCollectOnlyPreviousFootnote> (4th parameter of
            // method <SwFootnoteBossFrame::CollectFootnote(..)>) to control, that only
            // footnotes have to be collected, that are positioned before the
            // new dedicated footnote boss frame.
            pNewBoss->CollectFootnotes( pStart, pOldBoss, aFootnoteArr, true );
        }
        pStart = pStart->GetNextContentFrame();
    }

    OSL_ENSURE( pOldBoss->IsInSct() == pNewBoss->IsInSct(),
            "MoveLowerFootnotes: Section confusion" );
    std::unique_ptr<SwFootnoteFrames> pFootnoteArr;
    SwLayoutFrame* pNewChief = nullptr;
    SwLayoutFrame* pOldChief = nullptr;

    bool bFoundCandidate = false;
    if (pStart && pOldBoss->IsInSct())
    {
        pOldChief = pOldBoss->FindSctFrame();
        pNewChief = pNewBoss->FindSctFrame();
        bFoundCandidate = pOldChief != pNewChief;
    }

    if (bFoundCandidate)
    {
        pFootnoteArr.reset(new SwFootnoteFrames);
        pOldChief = pOldBoss->FindFootnoteBossFrame( true );
        pNewChief = pNewBoss->FindFootnoteBossFrame( true );
        while( pOldChief->IsAnLower( pStart ) )
        {
            if ( static_cast<SwTextFrame*>(pStart)->HasFootnote() )
                static_cast<SwFootnoteBossFrame*>(pNewChief)->CollectFootnotes( pStart,
                                        pOldBoss, *pFootnoteArr );
            pStart = pStart->GetNextContentFrame();
        }
        if( pFootnoteArr->empty() )
        {
            pFootnoteArr.reset();
        }
    }
    else
        pFootnoteArr = nullptr;

    if ( !aFootnoteArr.empty() || pFootnoteArr )
    {
        if( !aFootnoteArr.empty() )
            pNewBoss->MoveFootnotes_( aFootnoteArr, true );
        if( pFootnoteArr )
        {
            static_cast<SwFootnoteBossFrame*>(pNewChief)->MoveFootnotes_( *pFootnoteArr, true );
            pFootnoteArr.reset();
        }
        bMoved = true;

        // update FootnoteNum only at page change
        if ( bFootnoteNums )
        {
            SwPageFrame* pOldPage = pOldBoss->FindPageFrame();
            SwPageFrame* pNewPage =pNewBoss->FindPageFrame();
            if( pOldPage != pNewPage )
            {
                pOldPage->UpdateFootnoteNum();
                pNewPage->UpdateFootnoteNum();
            }
        }
    }
    return bMoved;
}

bool SwContentFrame::MoveFootnoteCntFwd( bool bMakePage, SwFootnoteBossFrame *pOldBoss )
{
    OSL_ENSURE( IsInFootnote(), "no footnote." );
    SwLayoutFrame *pFootnote = FindFootnoteFrame();

    // The first paragraph in the first footnote in the first column in the
    // sectionfrm at the top of the page has not to move forward, if the
    // columnbody is empty.
    if( pOldBoss->IsInSct() && !pOldBoss->GetIndPrev() && !GetIndPrev() &&
        !pFootnote->GetPrev() )
    {
        SwLayoutFrame* pBody = pOldBoss->FindBodyCont();
        if( !pBody || !pBody->Lower() )
            return true;
    }

    //fix(9538): if the footnote has neighbors behind itself, remove them temporarily
    SwLayoutFrame *pNxt = static_cast<SwLayoutFrame*>(pFootnote->GetNext());
    SwLayoutFrame *pLst = nullptr;
    while ( pNxt )
    {
        while ( pNxt->GetNext() )
            pNxt = static_cast<SwLayoutFrame*>(pNxt->GetNext());
        if ( pNxt == pLst )
            pNxt = nullptr;
        else
        {   pLst = pNxt;
            SwContentFrame *pCnt = pNxt->ContainsContent();
            if( pCnt )
                pCnt->MoveFootnoteCntFwd( true, pOldBoss );
            pNxt = static_cast<SwLayoutFrame*>(pFootnote->GetNext());
        }
    }

    bool bSamePage = true;
    SwLayoutFrame *pNewUpper =
                GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, true );

    if ( pNewUpper )
    {
        SwFootnoteBossFrame * const pNewBoss = pNewUpper->FindFootnoteBossFrame();
        // Are we changing the column/page?
        bool bSameBoss = pNewBoss == pOldBoss;
        if ( !bSameBoss )
        {
            bSamePage = pOldBoss->FindPageFrame() == pNewBoss->FindPageFrame(); // page change?
            pNewUpper->Calc(getRootFrame()->GetCurrShell()->GetOut());
        }

        // The layout leaf of the footnote is either a footnote container or a footnote.
        // If it is a footnote and it has the same footnote reference like the old Upper,
        // then move the content inside of it.
        // If it is a container or the reference differs, create a new footnote and add
        // it into the container.
        // Create also a SectionFrame if currently in a area inside a footnote.
        SwFootnoteFrame* pTmpFootnote = pNewUpper->IsFootnoteFrame() ? static_cast<SwFootnoteFrame*>(pNewUpper) : nullptr;
        if( !pTmpFootnote )
        {
            OSL_ENSURE( pNewUpper->IsFootnoteContFrame(), "New Upper not a FootnoteCont.");
            SwFootnoteContFrame *pCont = static_cast<SwFootnoteContFrame*>(pNewUpper);

            // create footnote
            SwFootnoteFrame *pOld = FindFootnoteFrame();
            pTmpFootnote = new SwFootnoteFrame( pOld->GetFormat()->GetDoc()->GetDfltFrameFormat(),
                                    pOld, pOld->GetRef(), pOld->GetAttr() );
            // chaining of footnotes
            if ( pOld->GetFollow() )
            {
                pTmpFootnote->SetFollow( pOld->GetFollow() );
                pOld->GetFollow()->SetMaster( pTmpFootnote );
            }
            pOld->SetFollow( pTmpFootnote );
            pTmpFootnote->SetMaster( pOld );
            SwFrame* pNx = pCont->Lower();
            if( pNx && pTmpFootnote->GetAttr()->GetFootnote().IsEndNote() )
                while(pNx && !static_cast<SwFootnoteFrame*>(pNx)->GetAttr()->GetFootnote().IsEndNote())
                    pNx = pNx->GetNext();
            pTmpFootnote->Paste( pCont, pNx );
            pTmpFootnote->Calc(getRootFrame()->GetCurrShell()->GetOut());
        }
        OSL_ENSURE( pTmpFootnote->GetAttr() == FindFootnoteFrame()->GetAttr(), "Wrong Footnote!" );
        // areas inside of footnotes get a special treatment
        SwLayoutFrame *pNewUp = pTmpFootnote;
        if( IsInSct() )
        {
            SwSectionFrame* pSect = FindSctFrame();
            // area inside of a footnote (or only footnote in an area)?
            if( pSect->IsInFootnote() )
            {
                if( pTmpFootnote->Lower() && pTmpFootnote->Lower()->IsSctFrame() &&
                    pSect->GetFollow() == static_cast<SwSectionFrame*>(pTmpFootnote->Lower()) )
                    pNewUp = static_cast<SwSectionFrame*>(pTmpFootnote->Lower());
                else
                {
                    pNewUp = new SwSectionFrame( *pSect, false );
                    pNewUp->InsertBefore( pTmpFootnote, pTmpFootnote->Lower() );
                    static_cast<SwSectionFrame*>(pNewUp)->Init();

                    {
                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pNewUp);
                        aFrm.Pos() = pTmpFootnote->getFrameArea().Pos();
                        aFrm.Pos().AdjustY(1 ); // for notifications
                    }

                    // If the section frame has a successor then the latter needs
                    // to be moved behind the new Follow of the section frame.
                    SwFrame* pTmp = pSect->GetNext();
                    if( pTmp )
                    {
                        SwFlowFrame* pTmpNxt;
                        if( pTmp->IsContentFrame() )
                            pTmpNxt = static_cast<SwContentFrame*>(pTmp);
                        else if( pTmp->IsSctFrame() )
                            pTmpNxt = static_cast<SwSectionFrame*>(pTmp);
                        else
                        {
                            OSL_ENSURE( pTmp->IsTabFrame(), "GetNextSctLeaf: Wrong Type" );
                            pTmpNxt = static_cast<SwTabFrame*>(pTmp);
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

SwSaveFootnoteHeight::SwSaveFootnoteHeight( SwFootnoteBossFrame *pBs, const SwTwips nDeadLine ) :
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
        pBoss->m_nMaxFootnoteHeight = nOldHeight;
}

#ifdef DBG_UTIL
//JP 15.10.2001: in a non pro version test if the attribute has the same
//              meaning which his reference is

// Normally, the pRef member and the GetRefFromAttr() result has to be
// identically. Sometimes footnote will be moved from a master to its follow,
// but the GetRef() is called first, so we have to ignore a master/follow
// mismatch.

const SwContentFrame* SwFootnoteFrame::GetRef() const
{
    const SwContentFrame* pRefAttr = GetRefFromAttr();
    // check consistency: access to deleted frame?
    assert(mpReference == pRefAttr || mpReference->IsAnFollow(pRefAttr)
            || pRefAttr->IsAnFollow(mpReference));
    (void) pRefAttr;
    return mpReference;
}

SwContentFrame* SwFootnoteFrame::GetRef()
{
    const SwContentFrame* pRefAttr = GetRefFromAttr();
    // check consistency: access to deleted frame?
    assert(mpReference == pRefAttr || mpReference->IsAnFollow(pRefAttr)
            || pRefAttr->IsAnFollow(mpReference));
    (void) pRefAttr;
    return mpReference;
}
#endif

const SwContentFrame* SwFootnoteFrame::GetRefFromAttr()  const
{
    SwFootnoteFrame* pThis = const_cast<SwFootnoteFrame*>(this);
    return pThis->GetRefFromAttr();
}

SwContentFrame* SwFootnoteFrame::GetRefFromAttr()
{
    assert(mpAttribute && "invalid Attribute");
    SwTextNode& rTNd = const_cast<SwTextNode&>(mpAttribute->GetTextNode());
    SwPosition aPos( rTNd, SwIndex( &rTNd, mpAttribute->GetStart() ));
    SwContentFrame* pCFrame = rTNd.getLayoutFrame(getRootFrame(), &aPos);
    return pCFrame;
}

/** search for last content in the current footnote frame

    OD 2005-12-02 #i27138#
*/
SwContentFrame* SwFootnoteFrame::FindLastContent()
{
    SwContentFrame* pLastContentFrame( nullptr );

    // find last lower, which is a content frame or contains content.
    // hidden text frames, empty sections and empty tables have to be skipped.
    SwFrame* pLastLowerOfFootnote( GetLower() );
    SwFrame* pTmpLastLower( pLastLowerOfFootnote );
    while ( pTmpLastLower && pTmpLastLower->GetNext() )
    {
        pTmpLastLower = pTmpLastLower->GetNext();
        if ( ( pTmpLastLower->IsTextFrame() &&
               !static_cast<SwTextFrame*>(pTmpLastLower)->IsHiddenNow() ) ||
             ( pTmpLastLower->IsSctFrame() &&
               static_cast<SwSectionFrame*>(pTmpLastLower)->GetSection() &&
               static_cast<SwSectionFrame*>(pTmpLastLower)->ContainsContent() ) ||
             ( pTmpLastLower->IsTabFrame() &&
               static_cast<SwTabFrame*>(pTmpLastLower)->ContainsContent() ) )
        {
            pLastLowerOfFootnote = pTmpLastLower;
        }
    }

    // determine last content frame depending on type of found last lower.
    if ( pLastLowerOfFootnote && pLastLowerOfFootnote->IsTabFrame() )
    {
        pLastContentFrame = static_cast<SwTabFrame*>(pLastLowerOfFootnote)->FindLastContent();
    }
    else if ( pLastLowerOfFootnote && pLastLowerOfFootnote->IsSctFrame() )
    {
        pLastContentFrame = static_cast<SwSectionFrame*>(pLastLowerOfFootnote)->FindLastContent();
    }
    else
    {
        pLastContentFrame = dynamic_cast<SwContentFrame*>(pLastLowerOfFootnote);
    }

    return pLastContentFrame;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
