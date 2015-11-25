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

#include "viewsh.hxx"
#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "ndtxt.hxx"
#include "txtatr.hxx"
#include <SwPortionHandler.hxx>
#include <txtftn.hxx>
#include <flyfrm.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <dflyobj.hxx>
#include <rowfrm.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <tabfrm.hxx>
#include <sortedobjs.hxx>

#include "swfont.hxx"
#include "porftn.hxx"
#include "porfly.hxx"
#include "porlay.hxx"
#include "txtfrm.hxx"
#include "itrform2.hxx"
#include "ftnfrm.hxx"
#include "pagedesc.hxx"
#include "redlnitr.hxx"
#include "sectfrm.hxx"
#include "layouter.hxx"
#include "frmtool.hxx"
#include "ndindex.hxx"

using namespace ::com::sun::star;

bool SwTextFrame::_IsFootnoteNumFrame() const
{
    const SwFootnoteFrame* pFootnote = FindFootnoteFrame()->GetMaster();
    while( pFootnote && !pFootnote->ContainsContent() )
        pFootnote = pFootnote->GetMaster();
    return !pFootnote;
}

/**
 * Looks for the TextFrame matching the SwTextFootnote within a master-follow chain
 */
SwTextFrame *SwTextFrame::FindFootnoteRef( const SwTextFootnote *pFootnote )
{
    SwTextFrame *pFrame = this;
    const bool bFwd = pFootnote->GetStart() >= GetOfst();
    while( pFrame )
    {
        if( SwFootnoteBossFrame::FindFootnote( pFrame, pFootnote ) )
            return pFrame;
        pFrame = bFwd ? pFrame->GetFollow() :
                      pFrame->IsFollow() ? pFrame->FindMaster() : nullptr;
    }
    return pFrame;
}

#ifdef DBG_UTIL
void SwTextFrame::CalcFootnoteFlag( sal_Int32 nStop )// For testing the SplitFrame
#else
void SwTextFrame::CalcFootnoteFlag()
#endif
{
    mbFootnote = false;

    const SwpHints *pHints = GetTextNode()->GetpSwpHints();
    if( !pHints )
        return;

    const size_t nSize = pHints->Count();

#ifdef DBG_UTIL
    const sal_Int32 nEnd = nStop != COMPLETE_STRING ? nStop
                        : GetFollow() ? GetFollow()->GetOfst() : COMPLETE_STRING;
#else
    const sal_Int32 nEnd = GetFollow() ? GetFollow()->GetOfst() : COMPLETE_STRING;
#endif

    for ( size_t i = 0; i < nSize; ++i )
    {
        const SwTextAttr *pHt = pHints->Get(i);
        if ( pHt->Which() == RES_TXTATR_FTN )
        {
            const sal_Int32 nIdx = pHt->GetStart();
            if ( nEnd < nIdx )
                break;
            if( GetOfst() <= nIdx )
            {
                mbFootnote = true;
                break;
            }
        }
    }
}

bool SwTextFrame::CalcPrepFootnoteAdjust()
{
    OSL_ENSURE( HasFootnote(), "Who´s calling me?" );
    SwFootnoteBossFrame *pBoss = FindFootnoteBossFrame( true );
    const SwFootnoteFrame *pFootnote = pBoss->FindFirstFootnote( this );
    if( pFootnote && FTNPOS_CHAPTER != GetNode()->GetDoc()->GetFootnoteInfo().ePos &&
        ( !pBoss->GetUpper()->IsSctFrame() ||
        !static_cast<SwSectionFrame*>(pBoss->GetUpper())->IsFootnoteAtEnd() ) )
    {
        const SwFootnoteContFrame *pCont = pBoss->FindFootnoteCont();
        bool bReArrange = true;

        SWRECTFN( this )
        if ( pCont && (*fnRect->fnYDiff)( (pCont->Frame().*fnRect->fnGetTop)(),
                                          (Frame().*fnRect->fnGetBottom)() ) > 0 )
        {
            pBoss->RearrangeFootnotes( (Frame().*fnRect->fnGetBottom)(), false,
                                  pFootnote->GetAttr() );
            ValidateBodyFrame();
            ValidateFrame();
            pFootnote = pBoss->FindFirstFootnote( this );
        }
        else
            bReArrange = false;
        if( !pCont || !pFootnote || bReArrange != (pFootnote->FindFootnoteBossFrame() == pBoss) )
        {
            SwTextFormatInfo aInf( getRootFrame()->GetCurrShell()->GetOut(), this );
            SwTextFormatter aLine( this, &aInf );
            aLine.TruncLines();
            SetPara( nullptr ); // May be deleted!
            ResetPreps();
            return false;
        }
    }
    return true;
}

/**
 * Local helper function. Checks if nLower should be taken as the boundary
 * for the footnote.
 */
static SwTwips lcl_GetFootnoteLower( const SwTextFrame* pFrame, SwTwips nLower )
{
    // nLower is an absolute value. It denotes the bottom of the line
    // containing the footnote.
    SWRECTFN( pFrame )

    OSL_ENSURE( !pFrame->IsVertical() || !pFrame->IsSwapped(),
            "lcl_GetFootnoteLower with swapped frame" );

    SwTwips nAdd;
    SwTwips nRet = nLower;

    // Check if text is inside a table.
    if ( pFrame->IsInTab() )
    {
        // If pFrame is inside a table, we have to check if
        // a) The table is not allowed to split or
        // b) The table row is not allowed to split

        // Inside a table, there are no footnotes,
        // see SwFrame::FindFootnoteBossFrame. So we don't have to check
        // the case that pFrame is inside a (footnote collecting) section
        // within the table.
        const SwFrame* pRow = pFrame;
        while( !pRow->IsRowFrame() || !pRow->GetUpper()->IsTabFrame() )
            pRow = pRow->GetUpper();
        const SwTabFrame* pTabFrame = static_cast<const SwTabFrame*>(pRow->GetUpper());

        OSL_ENSURE( pTabFrame && pRow &&
                pRow->GetUpper()->IsTabFrame(), "Upper of row should be tab" );

        const bool bDontSplit = !pTabFrame->IsFollow() &&
                                !pTabFrame->IsLayoutSplitAllowed();

        SwTwips nMin = 0;
        if ( bDontSplit )
            nMin = (pTabFrame->Frame().*fnRect->fnGetBottom)();
        else if ( !static_cast<const SwRowFrame*>(pRow)->IsRowSplitAllowed() )
            nMin = (pRow->Frame().*fnRect->fnGetBottom)();

        if ( nMin && (*fnRect->fnYDiff)( nMin, nLower ) > 0 )
            nRet = nMin;

        nAdd = (pRow->GetUpper()->*fnRect->fnGetBottomMargin)();
    }
    else
        nAdd = (pFrame->*fnRect->fnGetBottomMargin)();

    if( nAdd > 0 )
    {
        if ( bVert )
            nRet -= nAdd;
        else
            nRet += nAdd;
    }

    // #i10770#: If there are fly frames anchored at previous paragraphs,
    // the deadline should consider their lower borders.
    const SwFrame* pStartFrame = pFrame->GetUpper()->GetLower();
    OSL_ENSURE( pStartFrame, "Upper has no lower" );
    SwTwips nFlyLower = bVert ? LONG_MAX : 0;
    while ( pStartFrame != pFrame )
    {
        OSL_ENSURE( pStartFrame, "Frame chain is broken" );
        if ( pStartFrame->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pStartFrame->GetDrawObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                SwRect aRect( pAnchoredObj->GetObjRect() );

                if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) ==  nullptr ||
                     static_cast<SwFlyFrame*>(pAnchoredObj)->IsValid() )
                {
                    const SwTwips nBottom = (aRect.*fnRect->fnGetBottom)();
                    if ( (*fnRect->fnYDiff)( nBottom, nFlyLower ) > 0 )
                        nFlyLower = nBottom;
                }
            }
        }

        pStartFrame = pStartFrame->GetNext();
    }

    if ( bVert )
        nRet = std::min( nRet, nFlyLower );
    else
        nRet = std::max( nRet, nFlyLower );

    return nRet;
}

SwTwips SwTextFrame::GetFootnoteLine( const SwTextFootnote *pFootnote ) const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTextFrame::GetFootnoteLine with swapped frame" );

    SwTextFrame *pThis = const_cast<SwTextFrame*>(this);

    if( !HasPara() )
    {
        // #109071# GetFormatted() does not work here, because most probably
        // the frame is currently locked. We return the previous value.
        return pThis->mnFootnoteLine > 0 ?
               pThis->mnFootnoteLine :
               IsVertical() ? Frame().Left() : Frame().Bottom();
    }

    SwTwips nRet;
    {
        SwSwapIfNotSwapped swap(const_cast<SwTextFrame *>(this));

        SwTextInfo aInf( pThis );
        SwTextIter aLine( pThis, &aInf );
        const sal_Int32 nPos = pFootnote->GetStart();
        aLine.CharToLine( nPos );

        nRet = aLine.Y() + SwTwips(aLine.GetLineHeight());
        if( IsVertical() )
            nRet = SwitchHorizontalToVertical( nRet );
    }

    nRet = lcl_GetFootnoteLower( pThis, nRet );

    pThis->mnFootnoteLine = nRet;
    return nRet;
}

/**
 * Calculates the maximum reachable height for the TextFrame in the Footnote Area.
 * The cell's bottom margin with the Footnote Reference limit's this height.
 */
SwTwips SwTextFrame::_GetFootnoteFrameHeight() const
{
    OSL_ENSURE( !IsFollow() && IsInFootnote(), "SwTextFrame::SetFootnoteLine: moon walk" );

    const SwFootnoteFrame *pFootnoteFrame = FindFootnoteFrame();
    const SwTextFrame *pRef = static_cast<const SwTextFrame *>(pFootnoteFrame->GetRef());
    const SwFootnoteBossFrame *pBoss = FindFootnoteBossFrame();
    if( pBoss != pRef->FindFootnoteBossFrame( !pFootnoteFrame->GetAttr()->
                                        GetFootnote().IsEndNote() ) )
        return 0;

    SwSwapIfSwapped swap(const_cast<SwTextFrame *>(this));

    SwTwips nHeight = pRef->IsInFootnoteConnect() ?
                            1 : pRef->GetFootnoteLine( pFootnoteFrame->GetAttr() );
    if( nHeight )
    {
        // As odd as it may seem: the first Footnote on the page may not touch the
        // Footnote Reference, when entering text in the Footnote Area.
        const SwFrame *pCont = pFootnoteFrame->GetUpper();

        // Height within the Container which we're allowed to consume anyways
        SWRECTFN( pCont )
        SwTwips nTmp = (*fnRect->fnYDiff)( (pCont->*fnRect->fnGetPrtBottom)(),
                                           (Frame().*fnRect->fnGetTop)() );

#if OSL_DEBUG_LEVEL > 0
        if( nTmp < 0 )
        {
            bool bInvalidPos = false;
            const SwLayoutFrame* pTmp = GetUpper();
            while( !bInvalidPos && pTmp )
            {
                bInvalidPos = !pTmp->GetValidPosFlag() ||
                               !pTmp->Lower()->GetValidPosFlag();
                if( pTmp == pCont )
                    break;
                pTmp = pTmp->GetUpper();
            }
            OSL_ENSURE( bInvalidPos, "Hanging below FootnoteCont" );
        }
#endif

        if ( (*fnRect->fnYDiff)( (pCont->Frame().*fnRect->fnGetTop)(), nHeight) > 0 )
        {
            // Growth potential of the container
            if ( !pRef->IsInFootnoteConnect() )
            {
                SwSaveFootnoteHeight aSave( const_cast<SwFootnoteBossFrame*>(pBoss), nHeight  );
                nHeight = const_cast<SwFootnoteContFrame*>(static_cast<const SwFootnoteContFrame*>(pCont))->Grow( LONG_MAX, true );
            }
            else
                nHeight = const_cast<SwFootnoteContFrame*>(static_cast<const SwFootnoteContFrame*>(pCont))->Grow( LONG_MAX, true );

            nHeight += nTmp;
            if( nHeight < 0 )
                nHeight = 0;
        }
        else
        {   // The container has to shrink
            nTmp += (*fnRect->fnYDiff)( (pCont->Frame().*fnRect->fnGetTop)(), nHeight);
            if( nTmp > 0 )
                nHeight = nTmp;
            else
                nHeight = 0;
        }
    }

    return nHeight;
}

SwTextFrame *SwTextFrame::FindQuoVadisFrame()
{
    // Check whether we're in a FootnoteFrame
    if( GetIndPrev() || !IsInFootnote() )
        return nullptr;

    // To the preceding FootnoteFrame
    SwFootnoteFrame *pFootnoteFrame = FindFootnoteFrame()->GetMaster();
    if( !pFootnoteFrame )
        return nullptr;

    // Now the last Content
    SwContentFrame *pCnt = pFootnoteFrame->ContainsContent();
    if( !pCnt )
        return nullptr;
    SwContentFrame *pLast;
    do
    {   pLast = pCnt;
        pCnt = pCnt->GetNextContentFrame();
    } while( pCnt && pFootnoteFrame->IsAnLower( pCnt ) );
    return static_cast<SwTextFrame*>(pLast);
}

void SwTextFrame::RemoveFootnote( const sal_Int32 nStart, const sal_Int32 nLen )
{
    if ( !IsFootnoteAllowed() )
        return;

    SwpHints *pHints = GetTextNode()->GetpSwpHints();
    if( !pHints )
        return;

    bool bRollBack = nLen != COMPLETE_STRING;
    const size_t nSize = pHints->Count();
    sal_Int32 nEnd;
    SwTextFrame* pSource;
    if( bRollBack )
    {
        nEnd = nStart + nLen;
        pSource = GetFollow();
        if( !pSource )
            return;
    }
    else
    {
        nEnd = COMPLETE_STRING;
        pSource = this;
    }

    if( nSize )
    {
        SwPageFrame* pUpdate = nullptr;
        bool bRemove = false;
        SwFootnoteBossFrame *pFootnoteBoss = nullptr;
        SwFootnoteBossFrame *pEndBoss = nullptr;
        bool bFootnoteEndDoc
            = FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFootnoteInfo().ePos;
        for ( size_t i = nSize; i; )
        {
            SwTextAttr *pHt = pHints->Get(--i);
            if ( RES_TXTATR_FTN != pHt->Which() )
                continue;

            const sal_Int32 nIdx = pHt->GetStart();
            if( nStart > nIdx )
                break;

            if( nEnd >= nIdx )
            {
                SwTextFootnote *pFootnote = static_cast<SwTextFootnote*>(pHt);
                const bool bEndn = pFootnote->GetFootnote().IsEndNote();

                if( bEndn )
                {
                    if( !pEndBoss )
                        pEndBoss = pSource->FindFootnoteBossFrame();
                }
                else
                {
                    if( !pFootnoteBoss )
                    {
                        pFootnoteBoss = pSource->FindFootnoteBossFrame( true );
                        if( pFootnoteBoss->GetUpper()->IsSctFrame() )
                        {
                            SwSectionFrame* pSect = static_cast<SwSectionFrame*>(
                                                  pFootnoteBoss->GetUpper());
                            if( pSect->IsFootnoteAtEnd() )
                                bFootnoteEndDoc = false;
                        }
                    }
                }

                // We don't delete, but move instead.
                // Three cases are to be considered:
                // 1) There's neither Follow nor PrevFollow:
                //    -> RemoveFootnote() (maybe even a OSL_ENSURE(value))
                //
                // 2) nStart > GetOfst, I have a Follow
                //    -> Footnote moves into Follow
                //
                // 3) nStart < GetOfst, I am a Follow
                //    -> Footnote moves into the PrevFollow
                //
                // Both need to be on one Page/in one Column
                SwFootnoteFrame *pFootnoteFrame = SwFootnoteBossFrame::FindFootnote(pSource, pFootnote);

                if( pFootnoteFrame )
                {
                    const bool bEndDoc = bEndn || bFootnoteEndDoc;
                    if( bRollBack )
                    {
                        while ( pFootnoteFrame )
                        {
                            pFootnoteFrame->SetRef( this );
                            pFootnoteFrame = pFootnoteFrame->GetFollow();
                            SetFootnote( true );
                        }
                    }
                    else if( GetFollow() )
                    {
                        SwContentFrame *pDest = GetFollow();
                        while( pDest->GetFollow() && static_cast<SwTextFrame*>(pDest->
                               GetFollow())->GetOfst() <= nIdx )
                            pDest = pDest->GetFollow();
                        OSL_ENSURE( !SwFootnoteBossFrame::FindFootnote(
                            pDest,pFootnote),"SwTextFrame::RemoveFootnote: footnote exists");

                        // Never deregister; always move
                        if ( bEndDoc ||
                             !pFootnoteFrame->FindFootnoteBossFrame()->IsBefore( pDest->FindFootnoteBossFrame( !bEndn ) )
                           )
                        {
                            SwPageFrame* pTmp = pFootnoteFrame->FindPageFrame();
                            if( pUpdate && pUpdate != pTmp )
                                pUpdate->UpdateFootnoteNum();
                            pUpdate = pTmp;
                            while ( pFootnoteFrame )
                            {
                                pFootnoteFrame->SetRef( pDest );
                                pFootnoteFrame = pFootnoteFrame->GetFollow();
                            }
                        }
                        else
                        {
                            pFootnoteBoss->MoveFootnotes( this, pDest, pFootnote );
                            bRemove = true;
                        }
                        static_cast<SwTextFrame*>(pDest)->SetFootnote( true );

                        OSL_ENSURE( SwFootnoteBossFrame::FindFootnote( pDest,
                           pFootnote),"SwTextFrame::RemoveFootnote: footnote ChgRef failed");
                    }
                    else
                    {
                        if( !bEndDoc || ( bEndn && pEndBoss->IsInSct() &&
                            !SwLayouter::Collecting( GetNode()->GetDoc(),
                            pEndBoss->FindSctFrame(), nullptr ) ) )
                        {
                            if( bEndn )
                                pEndBoss->RemoveFootnote( this, pFootnote );
                            else
                                pFootnoteBoss->RemoveFootnote( this, pFootnote );
                            bRemove = bRemove || !bEndDoc;
                            OSL_ENSURE( !SwFootnoteBossFrame::FindFootnote( this, pFootnote ),
                            "SwTextFrame::RemoveFootnote: can't get off that footnote" );
                        }
                    }
                }
            }
        }
        if( pUpdate )
            pUpdate->UpdateFootnoteNum();

        // We brake the oscillation
        if( bRemove && !bFootnoteEndDoc && HasPara() )
        {
            ValidateBodyFrame();
            ValidateFrame();
        }
    }

    // We call the RemoveFootnote from within the FindBreak, because the last line is
    // to be passed to the Follow. The Offset of the Follow is, however, outdated;
    // it'll be set soon. CalcFntFlag depends on a correctly set Follow Offset.
    // Therefore we temporarily calculate the Follow Offset here
    sal_Int32 nOldOfst = COMPLETE_STRING;
    if( HasFollow() && nStart > GetOfst() )
    {
        nOldOfst = GetFollow()->GetOfst();
        GetFollow()->ManipOfst( nStart + ( bRollBack ? nLen : 0 ) );
    }
    pSource->CalcFootnoteFlag();
    if( nOldOfst < COMPLETE_STRING )
        GetFollow()->ManipOfst( nOldOfst );
}


/**
 * We basically only have two possibilities:
 *
 * a) The Footnote is already present
 *    => we move it, if another pSrcFrame has been found
 *
 * b) The Footnote is not present
 *    => we have it created for us
 *
 * Whether the Footnote ends up on our Page/Column, doesn't matter in this
 * context.
 *
 * Optimization for Endnotes.
 *
 * Another problem: if the Deadline falls within the Footnote Area, we need
 * to move the Footnote.
 *
 * @returns false on any type of error
 */
void SwTextFrame::ConnectFootnote( SwTextFootnote *pFootnote, const SwTwips nDeadLine )
{
    OSL_ENSURE( !IsVertical() || !IsSwapped(),
            "SwTextFrame::ConnectFootnote with swapped frame" );

    mbFootnote = true;
    mbInFootnoteConnect = true; // Just reset!
    const bool bEnd = pFootnote->GetFootnote().IsEndNote();

    // We want to store this value, because it is needed as a fallback
    // in GetFootnoteLine(), if there is no paragraph information available
    mnFootnoteLine = nDeadLine;

    // We always need a parent (Page/Column)
    SwSectionFrame *pSect;
    SwContentFrame *pContent = this;
    if( bEnd && IsInSct() )
    {
        pSect = FindSctFrame();
        if( pSect->IsEndnAtEnd() )
            pContent = pSect->FindLastContent( FINDMODE_ENDNOTE );
        if( !pContent )
            pContent = this;
    }

    SwFootnoteBossFrame *pBoss = pContent->FindFootnoteBossFrame( !bEnd );

#if OSL_DEBUG_LEVEL > 1
    SwTwips nRstHeight = GetRstHeight();
#endif

    pSect = pBoss->FindSctFrame();
    bool bDocEnd = bEnd ? !( pSect && pSect->IsEndnAtEnd() ) :
                   ( !( pSect && pSect->IsFootnoteAtEnd() ) &&
                       FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFootnoteInfo().ePos );

    // Footnote can be registered with the Follow
    SwContentFrame *pSrcFrame = FindFootnoteRef( pFootnote );

    if( bDocEnd )
    {
        if( pSect && pSrcFrame )
        {
            SwFootnoteFrame *pFootnoteFrame = SwFootnoteBossFrame::FindFootnote( pSrcFrame, pFootnote );
            if( pFootnoteFrame && pFootnoteFrame->IsInSct() )
            {
                pBoss->RemoveFootnote( pSrcFrame, pFootnote );
                pSrcFrame = nullptr;
            }
        }
    }
    else if( bEnd && pSect )
    {
        SwFootnoteFrame *pFootnoteFrame = pSrcFrame ? SwFootnoteBossFrame::FindFootnote( pSrcFrame, pFootnote ) : nullptr;
        if( pFootnoteFrame && !pFootnoteFrame->GetUpper() )
            pFootnoteFrame = nullptr;
        SwDoc *pDoc = GetNode()->GetDoc();
        if( SwLayouter::Collecting( pDoc, pSect, pFootnoteFrame ) )
        {
            if( !pSrcFrame )
            {
                SwFootnoteFrame *pNew = new SwFootnoteFrame(pDoc->GetDfltFrameFormat(),this,this,pFootnote);
                 SwNodeIndex aIdx( *pFootnote->GetStartNode(), 1 );
                 ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
                GetNode()->getIDocumentLayoutAccess().GetLayouter()->CollectEndnote( pNew );
            }
            else if( pSrcFrame != this )
                SwFootnoteBossFrame::ChangeFootnoteRef( pSrcFrame, pFootnote, this );
            mbInFootnoteConnect = false;
            return;
        }
        else if( pSrcFrame )
        {
            SwFootnoteBossFrame *pFootnoteBoss = pFootnoteFrame->FindFootnoteBossFrame();
            if( !pFootnoteBoss->IsInSct() ||
                pFootnoteBoss->ImplFindSctFrame()->GetSection()!=pSect->GetSection() )
            {
                pBoss->RemoveFootnote( pSrcFrame, pFootnote );
                pSrcFrame = nullptr;
            }
        }
    }

    if( bDocEnd || bEnd )
    {
        if( !pSrcFrame )
            pBoss->AppendFootnote( this, pFootnote );
        else if( pSrcFrame != this )
            SwFootnoteBossFrame::ChangeFootnoteRef( pSrcFrame, pFootnote, this );
        mbInFootnoteConnect = false;
        return;
    }

    SwSaveFootnoteHeight aHeight( pBoss, nDeadLine );

    if( !pSrcFrame ) // No Footnote was found at all
        pBoss->AppendFootnote( this, pFootnote );
    else
    {
        SwFootnoteFrame *pFootnoteFrame = SwFootnoteBossFrame::FindFootnote( pSrcFrame, pFootnote );
        SwFootnoteBossFrame *pFootnoteBoss = pFootnoteFrame->FindFootnoteBossFrame();

        bool bBrutal = false;

        if( pFootnoteBoss == pBoss ) // Ref and Footnote are on the same Page/Column
        {
            SwFrame *pCont = pFootnoteFrame->GetUpper();

            SWRECTFN ( pCont )
            long nDiff = (*fnRect->fnYDiff)( (pCont->Frame().*fnRect->fnGetTop)(),
                                             nDeadLine );

            if( nDiff >= 0 )
            {
                // If the Footnote has been registered to a Follow, we need to
                // rewire it now too
                if ( pSrcFrame != this )
                    SwFootnoteBossFrame::ChangeFootnoteRef( pSrcFrame, pFootnote, this );

                // We have some room left, so the Footnote can grow
                if ( pFootnoteFrame->GetFollow() && nDiff > 0 )
                {
                    SwTwips nHeight = (pCont->Frame().*fnRect->fnGetHeight)();
                    pBoss->RearrangeFootnotes( nDeadLine, false, pFootnote );
                    ValidateBodyFrame();
                    ValidateFrame();
                    SwViewShell *pSh = getRootFrame()->GetCurrShell();
                    if ( pSh && nHeight == (pCont->Frame().*fnRect->fnGetHeight)() )
                        // So that we don't miss anything
                        pSh->InvalidateWindows( pCont->Frame() );
                }
                mbInFootnoteConnect = false;
                return;
            }
            else
                bBrutal = true;
        }
        else
        {
            // Ref and Footnote are not on one Page; attempt to move is necessary
            SwFrame* pTmp = this;
            while( pTmp->GetNext() && pSrcFrame != pTmp )
                pTmp = pTmp->GetNext();
            if( pSrcFrame == pTmp )
                bBrutal = true;
            else
            {   // If our Parent is in a column Area, but the Page already has a
                // FootnoteContainer, we can only brute force it
                if( pSect && pSect->FindFootnoteBossFrame( !bEnd )->FindFootnoteCont() )
                    bBrutal = true;

                else if ( !pFootnoteFrame->GetPrev() ||
                          pFootnoteBoss->IsBefore( pBoss )
                        )
                {
                    SwFootnoteBossFrame *pSrcBoss = pSrcFrame->FindFootnoteBossFrame( !bEnd );
                    pSrcBoss->MoveFootnotes( pSrcFrame, this, pFootnote );
                }
                else
                    SwFootnoteBossFrame::ChangeFootnoteRef( pSrcFrame, pFootnote, this );
            }
        }

        // The brute force method: Remove Footnote and append.
        // We need to call SetFootnoteDeadLine(), as we can more easily adapt the
        // nMaxFootnoteHeight after RemoveFootnote
        if( bBrutal )
        {
            pBoss->RemoveFootnote( pSrcFrame, pFootnote, false );
            SwSaveFootnoteHeight *pHeight = bEnd ? nullptr : new SwSaveFootnoteHeight( pBoss, nDeadLine );
            pBoss->AppendFootnote( this, pFootnote );
            delete pHeight;
        }
    }

    // In column Areas, that not yet reach the Page's border a RearrangeFootnotes is not
    // useful yet, as the Footnote container has not yet been calculated
    if( !pSect || !pSect->Growable() )
    {
        // Validate environment, to avoid oscillation
        SwSaveFootnoteHeight aNochmal( pBoss, nDeadLine );
        ValidateBodyFrame();
        pBoss->RearrangeFootnotes( nDeadLine, true );
        ValidateFrame();
    }
    else if( pSect->IsFootnoteAtEnd() )
    {
        ValidateBodyFrame();
        ValidateFrame();
    }

#if OSL_DEBUG_LEVEL > 1
    // pFootnoteFrame may have changed due to Calc ...
    SwFootnoteFrame *pFootnoteFrame = pBoss->FindFootnote( this, pFootnote );
    if( pFootnoteFrame && pBoss != pFootnoteFrame->FindFootnoteBossFrame( !bEnd ) )
    {
        int bla = 5;
        (void)bla;
    }
    nRstHeight = GetRstHeight();
    (void)nRstHeight;
#endif
    mbInFootnoteConnect = false;
    return;
}

/**
 * The portion for the Footnote Reference in the Text
 */
SwFootnotePortion *SwTextFormatter::NewFootnotePortion( SwTextFormatInfo &rInf,
                                             SwTextAttr *pHint )
{
    OSL_ENSURE( ! m_pFrame->IsVertical() || m_pFrame->IsSwapped(),
            "NewFootnotePortion with unswapped frame" );

    if( !m_pFrame->IsFootnoteAllowed() )
        return nullptr;

    SwTextFootnote  *pFootnote = static_cast<SwTextFootnote*>(pHint);
    const SwFormatFootnote& rFootnote = static_cast<const SwFormatFootnote&>(pFootnote->GetFootnote());
    SwDoc *pDoc = m_pFrame->GetNode()->GetDoc();

    if( rInf.IsTest() )
        return new SwFootnotePortion( rFootnote.GetViewNumStr( *pDoc ), pFootnote );

    SwSwapIfSwapped swap(m_pFrame);

    sal_uInt16 nReal;
    {
        sal_uInt16 nOldReal = m_pCurr->GetRealHeight();
        sal_uInt16 nOldAscent = m_pCurr->GetAscent();
        sal_uInt16 nOldHeight = m_pCurr->Height();
        CalcRealHeight();
        nReal = m_pCurr->GetRealHeight();
        if( nReal < nOldReal )
            nReal = nOldReal;
        m_pCurr->SetRealHeight( nOldReal );
        m_pCurr->Height( nOldHeight );
        m_pCurr->SetAscent( nOldAscent );
    }

    SwTwips nLower = Y() + nReal;

    const bool bVertical = m_pFrame->IsVertical();
    if( bVertical )
        nLower = m_pFrame->SwitchHorizontalToVertical( nLower );

    nLower = lcl_GetFootnoteLower( m_pFrame, nLower );

    // We just refresh.
    // The Connect does not do anything useful in this case, but will
    // mostly throw away the Footnote and create it anew.
    if( !rInf.IsQuick() )
        m_pFrame->ConnectFootnote( pFootnote, nLower );

    SwTextFrame *pScrFrame = m_pFrame->FindFootnoteRef( pFootnote );
    SwFootnoteBossFrame *pBoss = m_pFrame->FindFootnoteBossFrame( !rFootnote.IsEndNote() );
    SwFootnoteFrame *pFootnoteFrame = nullptr;
    if( pScrFrame )
        pFootnoteFrame = SwFootnoteBossFrame::FindFootnote( pScrFrame, pFootnote );

    // We see whether our Append has caused some Footnote to
    // still be on the Page/Column. If not, our line disappears too,
    // which will lead to the following undesired behaviour:
    // Footnote1 still fits onto the Page/Column, but Footnote2 doesn't.
    // The Footnote2 Reference remains on the Page/Column. The Footnote itself
    // is on the next Page/Column.
    //
    // Exception: If the Page/Column cannot accommodate another line,
    // the Footnote Reference should be moved to the next one.
    if( !rFootnote.IsEndNote() )
    {
        SwSectionFrame *pSct = pBoss->FindSctFrame();
        bool bAtSctEnd = pSct && pSct->IsFootnoteAtEnd();
        if( FTNPOS_CHAPTER != pDoc->GetFootnoteInfo().ePos || bAtSctEnd )
        {
            SwFrame* pFootnoteCont = pBoss->FindFootnoteCont();
            // If the Parent is within an Area, it can only be a Column of this
            // Area. If this one is not the first Column, we can avoid it.
            if( !m_pFrame->IsInTab() && ( GetLineNr() > 1 || m_pFrame->GetPrev() ||
                ( !bAtSctEnd && m_pFrame->GetIndPrev() ) ||
                ( pSct && pBoss->GetPrev() ) ) )
            {
                if( !pFootnoteCont )
                {
                    rInf.SetStop( true );
                    return nullptr;
                }
                else
                {
                    // There must not be any Footnote Containers in column Areas and at the same time on the
                    // Page/Page column
                    if( pSct && !bAtSctEnd ) // Is the Container in a (column) Area?
                    {
                        SwFootnoteBossFrame* pTmp = pBoss->FindSctFrame()->FindFootnoteBossFrame( true );
                        SwFootnoteContFrame* pFootnoteC = pTmp->FindFootnoteCont();
                        if( pFootnoteC )
                        {
                            SwFootnoteFrame* pTmpFrame = static_cast<SwFootnoteFrame*>(pFootnoteC->Lower());
                            if( pTmpFrame && *pTmpFrame < pFootnote )
                            {
                                rInf.SetStop( true );
                                return nullptr;
                            }
                        }
                    }
                    // Is this the last Line that fits?
                    SwTwips nTmpBot = Y() + nReal * 2;

                    if( bVertical )
                        nTmpBot = m_pFrame->SwitchHorizontalToVertical( nTmpBot );

                    SWRECTFN( pFootnoteCont )

                    const long nDiff = (*fnRect->fnYDiff)(
                                            (pFootnoteCont->Frame().*fnRect->fnGetTop)(),
                                             nTmpBot );

                    if( pScrFrame && nDiff < 0 )
                    {
                        if( pFootnoteFrame )
                        {
                            SwFootnoteBossFrame *pFootnoteBoss = pFootnoteFrame->FindFootnoteBossFrame();
                            if( pFootnoteBoss != pBoss )
                            {
                                // We're in the last Line and the Footnote has moved
                                // to another Page. We also want to be on that Page!
                                rInf.SetStop( true );
                                return nullptr;
                            }
                        }
                    }
                }
            }
        }
    }
    // Finally: Create FootnotePortion and exit ...
    SwFootnotePortion *pRet = new SwFootnotePortion( rFootnote.GetViewNumStr( *pDoc ),
                                           pFootnote, nReal );
    rInf.SetFootnoteInside( true );

    return pRet;
 }

/**
 * The portion for the Footnote Numbering in the Footnote Area
 */
SwNumberPortion *SwTextFormatter::NewFootnoteNumPortion( SwTextFormatInfo &rInf ) const
{
    OSL_ENSURE( m_pFrame->IsInFootnote() && !m_pFrame->GetIndPrev() && !rInf.IsFootnoteDone(),
            "This is the wrong place for a ftnnumber" );
    if( rInf.GetTextStart() != m_nStart ||
        rInf.GetTextStart() != rInf.GetIdx() )
        return nullptr;

    const SwFootnoteFrame* pFootnoteFrame = m_pFrame->FindFootnoteFrame();
    const SwTextFootnote* pFootnote = pFootnoteFrame->GetAttr();

    // Aha! So we're in the Footnote Area!
    SwFormatFootnote& rFootnote = (SwFormatFootnote&)pFootnote->GetFootnote();

    SwDoc *pDoc = m_pFrame->GetNode()->GetDoc();
    OUString aFootnoteText( rFootnote.GetViewNumStr( *pDoc, true ));

    const SwEndNoteInfo* pInfo;
    if( rFootnote.IsEndNote() )
        pInfo = &pDoc->GetEndNoteInfo();
    else
        pInfo = &pDoc->GetFootnoteInfo();
    const SwAttrSet& rSet = pInfo->GetCharFormat(*pDoc)->GetAttrSet();

    const SwAttrSet* pParSet = &rInf.GetCharAttr();
    const IDocumentSettingAccess* pIDSA = m_pFrame->GetTextNode()->getIDocumentSettingAccess();
    SwFont *pNumFnt = new SwFont( pParSet, pIDSA );

    // #i37142#
    // Underline style of paragraph font should not be considered
    // Overline style of paragraph font should not be considered
    // Weight style of paragraph font should not be considered
    // Posture style of paragraph font should not be considered
    // See also #i18463# and SwTextFormatter::NewNumberPortion()
    pNumFnt->SetUnderline( UNDERLINE_NONE );
    pNumFnt->SetOverline( UNDERLINE_NONE );
    pNumFnt->SetItalic( ITALIC_NONE, SW_LATIN );
    pNumFnt->SetItalic( ITALIC_NONE, SW_CJK );
    pNumFnt->SetItalic( ITALIC_NONE, SW_CTL );
    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_LATIN );
    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_CJK );
    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_CTL );

    pNumFnt->SetDiffFnt(&rSet, pIDSA );
    pNumFnt->SetVertical( pNumFnt->GetOrientation(), m_pFrame->IsVertical() );

    SwFootnoteNumPortion* pNewPor = new SwFootnoteNumPortion( aFootnoteText, pNumFnt );
    pNewPor->SetLeft( !m_pFrame->IsRightToLeft() );
    return pNewPor;
}

OUString lcl_GetPageNumber( const SwPageFrame* pPage )
{
    OSL_ENSURE( pPage, "GetPageNumber: Homeless TextFrame" );
    const sal_uInt16 nVirtNum = pPage->GetVirtPageNum();
    const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
    return rNum.GetNumStr( nVirtNum );
}

SwErgoSumPortion *SwTextFormatter::NewErgoSumPortion( SwTextFormatInfo &rInf ) const
{
    // We cannot assume we're a Follow
    if( !m_pFrame->IsInFootnote()  || m_pFrame->GetPrev() ||
        rInf.IsErgoDone() || rInf.GetIdx() != m_pFrame->GetOfst() ||
        m_pFrame->ImplFindFootnoteFrame()->GetAttr()->GetFootnote().IsEndNote() )
        return nullptr;

    // Aha, wir sind also im Fussnotenbereich
    const SwFootnoteInfo &rFootnoteInfo = m_pFrame->GetNode()->GetDoc()->GetFootnoteInfo();
    SwTextFrame *pQuoFrame = m_pFrame->FindQuoVadisFrame();
    if( !pQuoFrame )
        return nullptr;
    const SwPageFrame* pPage = m_pFrame->FindPageFrame();
    const SwPageFrame* pQuoPage = pQuoFrame->FindPageFrame();
    if( pPage == pQuoFrame->FindPageFrame() )
        return nullptr; // If the QuoVadis is on the same Column/Page
    const OUString aPage = lcl_GetPageNumber( pPage );
    SwParaPortion *pPara = pQuoFrame->GetPara();
    if( pPara )
        pPara->SetErgoSumNum( aPage );
    if( rFootnoteInfo.aErgoSum.isEmpty() )
        return nullptr;
    SwErgoSumPortion *pErgo = new SwErgoSumPortion( rFootnoteInfo.aErgoSum,
                                lcl_GetPageNumber( pQuoPage ) );
    return pErgo;
}

sal_Int32 SwTextFormatter::FormatQuoVadis( const sal_Int32 nOffset )
{
    OSL_ENSURE( ! m_pFrame->IsVertical() || ! m_pFrame->IsSwapped(),
            "SwTextFormatter::FormatQuoVadis with swapped frame" );

    if( !m_pFrame->IsInFootnote() || m_pFrame->ImplFindFootnoteFrame()->GetAttr()->GetFootnote().IsEndNote() )
        return nOffset;

    const SwFrame* pErgoFrame = m_pFrame->FindFootnoteFrame()->GetFollow();
    if( !pErgoFrame && m_pFrame->HasFollow() )
        pErgoFrame = m_pFrame->GetFollow();
    if( !pErgoFrame )
        return nOffset;

    if( pErgoFrame == m_pFrame->GetNext() )
    {
        SwFrame *pCol = m_pFrame->FindColFrame();
        while( pCol && !pCol->GetNext() )
            pCol = pCol->GetUpper()->FindColFrame();
        if( pCol )
            return nOffset;
    }
    else
    {
        const SwPageFrame* pPage = m_pFrame->FindPageFrame();
        const SwPageFrame* pErgoPage = pErgoFrame->FindPageFrame();
        if( pPage == pErgoPage )
            return nOffset; // If the ErgoSum is on the same Page
    }

    SwTextFormatInfo &rInf = GetInfo();
    const SwFootnoteInfo &rFootnoteInfo = m_pFrame->GetNode()->GetDoc()->GetFootnoteInfo();
    if( rFootnoteInfo.aQuoVadis.isEmpty() )
        return nOffset;

    // A remark on QuoVadis/ErgoSum:
    // We use the Font set for the Paragraph for these texts.
    // Thus, we initialze:
    // TODO: ResetFont();
    FeedInf( rInf );
    SeekStartAndChg( rInf, true );
    if( GetRedln() && m_pCurr->HasRedline() )
        GetRedln()->Seek( *pFnt, nOffset, 0 );

    // A tricky special case: Flyfrms extend into the Line and are at the
    // position we want to insert the Quovadis text
    // Let's see if it is that bad indeed:
    SwLinePortion *pPor = m_pCurr->GetFirstPortion();
    sal_uInt16 nLastLeft = 0;
    while( pPor )
    {
        if ( pPor->IsFlyPortion() )
            nLastLeft = static_cast<SwFlyPortion*>(pPor)->Fix() +
                        static_cast<SwFlyPortion*>(pPor)->Width();
        pPor = pPor->GetPortion();
    }

    // The old game all over again: we want the Line to wrap around
    // at a certain point, so we adjust the width.
    // nLastLeft is now basically the right margin
    const sal_uInt16 nOldRealWidth = rInf.RealWidth();
    rInf.RealWidth( nOldRealWidth - nLastLeft );

    OUString aErgo = lcl_GetPageNumber( pErgoFrame->FindPageFrame() );
    SwQuoVadisPortion *pQuo = new SwQuoVadisPortion(rFootnoteInfo.aQuoVadis, aErgo );
    pQuo->SetAscent( rInf.GetAscent()  );
    pQuo->Height( rInf.GetTextHeight() );
    pQuo->Format( rInf );
    sal_uInt16 nQuoWidth = pQuo->Width();
    SwLinePortion* pCurrPor = pQuo;

    while ( rInf.GetRest() )
    {
        SwLinePortion* pFollow = rInf.GetRest();
        rInf.SetRest( nullptr );
        pCurrPor->Move( rInf );

        OSL_ENSURE( pFollow->IsQuoVadisPortion(),
                "Quo Vadis, rest of QuoVadisPortion" );

        // format the rest and append it to the other QuoVadis parts
        pFollow->Format( rInf );
        nQuoWidth = nQuoWidth + pFollow->Width();

        pCurrPor->Append( pFollow );
        pCurrPor = pFollow;
    }

    Right( Right() - nQuoWidth );

    sal_Int32 nRet;
    {
        SwSwapIfNotSwapped swap(m_pFrame);

        nRet = FormatLine( m_nStart );
    }

    Right( rInf.Left() + nOldRealWidth - 1 );

    nLastLeft = nOldRealWidth - m_pCurr->Width();
    FeedInf( rInf );

    // It's possible that there's a Margin Portion at the end, which would
    // just cause a lot of trouble, when respanning
    pPor = m_pCurr->FindLastPortion();
    SwGluePortion *pGlue = pPor->IsMarginPortion() ? static_cast<SwMarginPortion*>(pPor) : nullptr;
    if( pGlue )
    {
        pGlue->Height( 0 );
        pGlue->Width( 0 );
        pGlue->SetLen( 0 );
        pGlue->SetAscent( 0 );
        pGlue->SetPortion( nullptr );
        pGlue->SetFixWidth(0);
    }

    // Luxury: We make sure the QuoVadis text appears on the right, by
    // using Glues.
    nLastLeft = nLastLeft - nQuoWidth;
    if( nLastLeft )
    {
        if( nLastLeft > pQuo->GetAscent() ) // Minimum distance
        {
            switch( GetAdjust() )
            {
                case SVX_ADJUST_BLOCK:
                {
                    if( !m_pCurr->GetLen() ||
                        CH_BREAK != GetInfo().GetChar(m_nStart+m_pCurr->GetLen()-1))
                        nLastLeft = pQuo->GetAscent();
                    nQuoWidth = nQuoWidth + nLastLeft;
                    break;
                }
                case SVX_ADJUST_RIGHT:
                {
                    nLastLeft = pQuo->GetAscent();
                    nQuoWidth = nQuoWidth + nLastLeft;
                    break;
                }
                case SVX_ADJUST_CENTER:
                {
                    nQuoWidth = nQuoWidth + pQuo->GetAscent();
                    long nDiff = nLastLeft - nQuoWidth;
                    if( nDiff < 0 )
                    {
                        nLastLeft = pQuo->GetAscent();
                        nQuoWidth = (sal_uInt16)(-nDiff + nLastLeft);
                    }
                    else
                    {
                        nQuoWidth = 0;
                        nLastLeft = sal_uInt16(( pQuo->GetAscent() + nDiff ) / 2);
                    }
                    break;
                }
                default:
                    nQuoWidth = nQuoWidth + nLastLeft;
            }
        }
        else
            nQuoWidth = nQuoWidth + nLastLeft;
        if( nLastLeft )
        {
            pGlue = new SwGluePortion(0);
            pGlue->Width( nLastLeft );
            pPor->Append( pGlue );
            pPor = pPor->GetPortion();
        }
    }

    // Finally: we insert the QuoVadis Portion
    pCurrPor = pQuo;
    while ( pCurrPor )
    {
        // pPor->Append deletes the pPortion pointer of pPor.
        // Therefore we have to keep a pointer to the next portion
        pQuo = static_cast<SwQuoVadisPortion*>(pCurrPor->GetPortion());
        pPor->Append( pCurrPor );
        pPor = pPor->GetPortion();
        pCurrPor = pQuo;
    }

    m_pCurr->Width( m_pCurr->Width() + nQuoWidth );

    // And adjust again, due to the adjustment and due to the following special
    // case:
    // The DummyUser has set a smaller Font in the Line than the one used
    // by the QuoVadis text ...
    CalcAdjustLine( m_pCurr );

    return nRet;
}

/**
 * This function creates a Line that reaches to the other Page Margin.
 * DummyLines or DummyPortions make sure, that osicllations stop, because
 * there's no way to flow back.
 * They are used for Footnotes in paragraph-bound Frames and for Footnote
 * oscillations
 */
void SwTextFormatter::MakeDummyLine()
{
    sal_uInt16 nRstHeight = GetFrameRstHeight();
    if( m_pCurr && nRstHeight > m_pCurr->Height() )
    {
        SwLineLayout *pLay = new SwLineLayout;
        nRstHeight = nRstHeight - m_pCurr->Height();
        pLay->Height( nRstHeight );
        pLay->SetAscent( nRstHeight );
        Insert( pLay );
        Next();
    }
}

class SwFootnoteSave
{
    SwTextSizeInfo *pInf;
    SwFont       *pFnt;
    SwFont       *pOld;
public:
    SwFootnoteSave( const SwTextSizeInfo &rInf,
               const SwTextFootnote *pTextFootnote,
               const bool bApplyGivenScriptType,
               const sal_uInt8 nGivenScriptType );
   ~SwFootnoteSave();
};

SwFootnoteSave::SwFootnoteSave( const SwTextSizeInfo &rInf,
                      const SwTextFootnote* pTextFootnote,
                      const bool bApplyGivenScriptType,
                      const sal_uInt8 nGivenScriptType )
    : pInf( &((SwTextSizeInfo&)rInf) )
    , pFnt( nullptr )
    , pOld( nullptr )
{
    if( pTextFootnote && rInf.GetTextFrame() )
    {
        pFnt = ((SwTextSizeInfo&)rInf).GetFont();
          pOld = new SwFont( *pFnt );
        pOld->GetTox() = pFnt->GetTox();
        pFnt->GetTox() = 0;
        SwFormatFootnote& rFootnote = (SwFormatFootnote&)pTextFootnote->GetFootnote();
        const SwDoc *pDoc = rInf.GetTextFrame()->GetNode()->GetDoc();

        // #i98418#
        if ( bApplyGivenScriptType )
        {
            pFnt->SetActual( nGivenScriptType );
        }
        else
        {
            // examine text and set script
            OUString aTmpStr( rFootnote.GetViewNumStr( *pDoc ) );
            pFnt->SetActual( SwScriptInfo::WhichFont( 0, &aTmpStr, nullptr ) );
        }

        const SwEndNoteInfo* pInfo;
        if( rFootnote.IsEndNote() )
            pInfo = &pDoc->GetEndNoteInfo();
        else
            pInfo = &pDoc->GetFootnoteInfo();
        const SwAttrSet& rSet = pInfo->GetAnchorCharFormat((SwDoc&)*pDoc)->GetAttrSet();
        pFnt->SetDiffFnt( &rSet, rInf.GetTextFrame()->GetNode()->getIDocumentSettingAccess() );

        // we reduce footnote size, if we are inside a double line portion
        if ( ! pOld->GetEscapement() && 50 == pOld->GetPropr() )
        {
            Size aSize = pFnt->GetSize( pFnt->GetActual() );
            pFnt->SetSize( Size( (long)aSize.Width() / 2,
                                 (long)aSize.Height() / 2 ),
                           pFnt->GetActual() );
        }

        // set the correct rotation at the footnote font
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == rSet.GetItemState( RES_CHRATR_ROTATE,
            true, &pItem ))
            pFnt->SetVertical( static_cast<const SvxCharRotateItem*>(pItem)->GetValue(),
                                rInf.GetTextFrame()->IsVertical() );

        pFnt->ChgPhysFnt( pInf->GetVsh(), *pInf->GetOut() );

        if( SfxItemState::SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
            true, &pItem ))
            pFnt->SetBackColor( new Color( static_cast<const SvxBrushItem*>(pItem)->GetColor() ) );
    }
    else
        pFnt = nullptr;
}

SwFootnoteSave::~SwFootnoteSave()
{
    if( pFnt )
    {
        // Put back SwFont
        *pFnt = *pOld;
        pFnt->GetTox() = pOld->GetTox();
        pFnt->ChgPhysFnt( pInf->GetVsh(), *pInf->GetOut() );
        delete pOld;
    }
}

SwFootnotePortion::SwFootnotePortion( const OUString &rExpand,
                            SwTextFootnote *pFootn, sal_uInt16 nReal )
        : SwFieldPortion( rExpand, nullptr )
        , pFootnote(pFootn)
        , nOrigHeight( nReal )
        // #i98418#
        , mbPreferredScriptTypeSet( false )
        , mnPreferredScriptType( SW_LATIN )
{
    SetLen(1);
    SetWhichPor( POR_FTN );
}

bool SwFootnotePortion::GetExpText( const SwTextSizeInfo &, OUString &rText ) const
{
    rText = aExpand;
    return true;
}

bool SwFootnotePortion::Format( SwTextFormatInfo &rInf )
{
    // #i98418#
//    SwFootnoteSave aFootnoteSave( rInf, pFootnote );
    SwFootnoteSave aFootnoteSave( rInf, pFootnote, mbPreferredScriptTypeSet, mnPreferredScriptType );
    // the idx is manipulated in SwExpandPortion::Format
    // this flag indicates, that a footnote is allowed to trigger
    // an underflow during SwTextGuess::Guess
    rInf.SetFakeLineStart( rInf.GetIdx() > rInf.GetLineStart() );
    const bool bFull = SwFieldPortion::Format( rInf );
    rInf.SetFakeLineStart( false );
    SetAscent( rInf.GetAscent() );
    Height( rInf.GetTextHeight() );
    rInf.SetFootnoteDone( !bFull );
    if( !bFull )
        rInf.SetParaFootnote();
    return bFull;
}

void SwFootnotePortion::Paint( const SwTextPaintInfo &rInf ) const
{
    // #i98418#
//    SwFootnoteSave aFootnoteSave( rInf, pFootnote );
    SwFootnoteSave aFootnoteSave( rInf, pFootnote, mbPreferredScriptTypeSet, mnPreferredScriptType );
    rInf.DrawViewOpt( *this, POR_FTN );
    SwExpandPortion::Paint( rInf );
}

SwPosSize SwFootnotePortion::GetTextSize( const SwTextSizeInfo &rInfo ) const
{
    // #i98418#
//    SwFootnoteSave aFootnoteSave( rInfo, pFootnote );
    SwFootnoteSave aFootnoteSave( rInfo, pFootnote, mbPreferredScriptTypeSet, mnPreferredScriptType );
    return SwExpandPortion::GetTextSize( rInfo );
}

// #i98418#
void SwFootnotePortion::SetPreferredScriptType( sal_uInt8 nPreferredScriptType )
{
    mbPreferredScriptTypeSet = true;
    mnPreferredScriptType = nPreferredScriptType;
}

SwFieldPortion *SwQuoVadisPortion::Clone( const OUString &rExpand ) const
{
    return new SwQuoVadisPortion( rExpand, aErgo );
}

SwQuoVadisPortion::SwQuoVadisPortion( const OUString &rExp, const OUString& rStr )
    : SwFieldPortion( rExp ), aErgo(rStr)
{
    SetLen(0);
    SetWhichPor( POR_QUOVADIS );
}

bool SwQuoVadisPortion::Format( SwTextFormatInfo &rInf )
{
    // First try; maybe the Text fits
    CheckScript( rInf );
    bool bFull = SwFieldPortion::Format( rInf );
    SetLen( 0 );

    if( bFull )
    {
        // Second try; we make the String shorter
        aExpand = "...";
        bFull = SwFieldPortion::Format( rInf );
        SetLen( 0 );
        if( bFull  )
            // Third try; we're done: we crush
            Width( sal_uInt16(rInf.Width() - rInf.X()) );

        // No multiline Fields for QuoVadis and ErgoSum
        if( rInf.GetRest() )
        {
            delete rInf.GetRest();
            rInf.SetRest( nullptr );
        }
    }
    return bFull;
}

bool SwQuoVadisPortion::GetExpText( const SwTextSizeInfo &, OUString &rText ) const
{
    rText = aExpand;
    // if this QuoVadisPortion has a follow, the follow is responsible for
    // the ergo text.
    if ( ! HasFollow() )
        rText += aErgo;
    return true;
}

void SwQuoVadisPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), aExpand + aErgo, GetWhichPor() );
}

void SwQuoVadisPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    // We _always_ want to ouput per DrawStretchText, because nErgo
    // can quickly switch
    if( PrtWidth() )
    {
        rInf.DrawViewOpt( *this, POR_QUOVADIS );
        SwTextSlot aDiffText( &rInf, this, true, false );
        SwFontSave aSave( rInf, pFnt );
        rInf.DrawText( *this, rInf.GetLen(), true );
    }
}

SwFieldPortion *SwErgoSumPortion::Clone( const OUString &rExpand ) const
{
    return new SwErgoSumPortion( rExpand, OUString() );
}

SwErgoSumPortion::SwErgoSumPortion(const OUString &rExp, const OUString& rStr)
    : SwFieldPortion( rExp )
{
    SetLen(0);
    aExpand += rStr;

    // One blank distance to the text
    aExpand += " ";
    SetWhichPor( POR_ERGOSUM );
}

sal_Int32 SwErgoSumPortion::GetCursorOfst( const sal_uInt16 ) const
{
    return 0;
}

bool SwErgoSumPortion::Format( SwTextFormatInfo &rInf )
{
    const bool bFull = SwFieldPortion::Format( rInf );
    SetLen( 0 );
    rInf.SetErgoDone( true );

    // No multiline Fields for QuoVadis and ErgoSum
    if( bFull && rInf.GetRest() )
    {
        delete rInf.GetRest();
        rInf.SetRest( nullptr );
    }

    // We return false in order to get some text into the current line,
    // even if it's full (better than looping)
    return false;
}

void SwParaPortion::SetErgoSumNum( const OUString& rErgo )
{
    SwLineLayout *pLay = this;
    while( pLay->GetNext() )
    {
        pLay = pLay->GetNext();
    }
    SwLinePortion     *pPor = pLay;
    SwQuoVadisPortion *pQuo = nullptr;
    while( pPor && !pQuo )
    {
        if ( pPor->IsQuoVadisPortion() )
            pQuo = static_cast<SwQuoVadisPortion*>(pPor);
        pPor = pPor->GetPortion();
    }
    if( pQuo )
        pQuo->SetNumber( rErgo );
}

/**
 * Is called in SwTextFrame::Prepare()
 */
bool SwParaPortion::UpdateQuoVadis( const OUString &rQuo )
{
    SwLineLayout *pLay = this;
    while( pLay->GetNext() )
    {
        pLay = pLay->GetNext();
    }
    SwLinePortion     *pPor = pLay;
    SwQuoVadisPortion *pQuo = nullptr;
    while( pPor && !pQuo )
    {
        if ( pPor->IsQuoVadisPortion() )
            pQuo = static_cast<SwQuoVadisPortion*>(pPor);
        pPor = pPor->GetPortion();
    }

    if( !pQuo )
        return false;

    return pQuo->GetQuoText() == rQuo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
