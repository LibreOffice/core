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

bool SwTextFrm::_IsFootnoteNumFrm() const
{
    const SwFootnoteFrm* pFootnote = FindFootnoteFrm()->GetMaster();
    while( pFootnote && !pFootnote->ContainsContent() )
        pFootnote = pFootnote->GetMaster();
    return !pFootnote;
}

/**
 * Looks for the TextFrm matching the SwTextFootnote within a master-follow chain
 */
SwTextFrm *SwTextFrm::FindFootnoteRef( const SwTextFootnote *pFootnote )
{
    SwTextFrm *pFrm = this;
    const bool bFwd = pFootnote->GetStart() >= GetOfst();
    while( pFrm )
    {
        if( SwFootnoteBossFrm::FindFootnote( pFrm, pFootnote ) )
            return pFrm;
        pFrm = bFwd ? pFrm->GetFollow() :
                      pFrm->IsFollow() ? pFrm->FindMaster() : nullptr;
    }
    return pFrm;
}

#ifdef DBG_UTIL
void SwTextFrm::CalcFootnoteFlag( sal_Int32 nStop )// For testing the SplitFrm
#else
void SwTextFrm::CalcFootnoteFlag()
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

bool SwTextFrm::CalcPrepFootnoteAdjust()
{
    OSL_ENSURE( HasFootnote(), "Who´s calling me?" );
    SwFootnoteBossFrm *pBoss = FindFootnoteBossFrm( true );
    const SwFootnoteFrm *pFootnote = pBoss->FindFirstFootnote( this );
    if( pFootnote && FTNPOS_CHAPTER != GetNode()->GetDoc()->GetFootnoteInfo().ePos &&
        ( !pBoss->GetUpper()->IsSctFrm() ||
        !static_cast<SwSectionFrm*>(pBoss->GetUpper())->IsFootnoteAtEnd() ) )
    {
        const SwFootnoteContFrm *pCont = pBoss->FindFootnoteCont();
        bool bReArrange = true;

        SWRECTFN( this )
        if ( pCont && (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(),
                                          (Frm().*fnRect->fnGetBottom)() ) > 0 )
        {
            pBoss->RearrangeFootnotes( (Frm().*fnRect->fnGetBottom)(), false,
                                  pFootnote->GetAttr() );
            ValidateBodyFrm();
            ValidateFrm();
            pFootnote = pBoss->FindFirstFootnote( this );
        }
        else
            bReArrange = false;
        if( !pCont || !pFootnote || bReArrange != (pFootnote->FindFootnoteBossFrm() == pBoss) )
        {
            SwTextFormatInfo aInf( getRootFrm()->GetCurrShell()->GetOut(), this );
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
static SwTwips lcl_GetFootnoteLower( const SwTextFrm* pFrm, SwTwips nLower )
{
    // nLower is an absolute value. It denotes the bottom of the line
    // containing the footnote.
    SWRECTFN( pFrm )

    OSL_ENSURE( !pFrm->IsVertical() || !pFrm->IsSwapped(),
            "lcl_GetFootnoteLower with swapped frame" );

    SwTwips nAdd;
    SwTwips nRet = nLower;

    // Check if text is inside a table.
    if ( pFrm->IsInTab() )
    {
        // If pFrm is inside a table, we have to check if
        // a) The table is not allowed to split or
        // b) The table row is not allowed to split

        // Inside a table, there are no footnotes,
        // see SwFrm::FindFootnoteBossFrm. So we don't have to check
        // the case that pFrm is inside a (footnote collecting) section
        // within the table.
        const SwFrm* pRow = pFrm;
        while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
            pRow = pRow->GetUpper();
        const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>(pRow->GetUpper());

        OSL_ENSURE( pTabFrm && pRow &&
                pRow->GetUpper()->IsTabFrm(), "Upper of row should be tab" );

        const bool bDontSplit = !pTabFrm->IsFollow() &&
                                !pTabFrm->IsLayoutSplitAllowed();

        SwTwips nMin = 0;
        if ( bDontSplit )
            nMin = (pTabFrm->Frm().*fnRect->fnGetBottom)();
        else if ( !static_cast<const SwRowFrm*>(pRow)->IsRowSplitAllowed() )
            nMin = (pRow->Frm().*fnRect->fnGetBottom)();

        if ( nMin && (*fnRect->fnYDiff)( nMin, nLower ) > 0 )
            nRet = nMin;

        nAdd = (pRow->GetUpper()->*fnRect->fnGetBottomMargin)();
    }
    else
        nAdd = (pFrm->*fnRect->fnGetBottomMargin)();

    if( nAdd > 0 )
    {
        if ( bVert )
            nRet -= nAdd;
        else
            nRet += nAdd;
    }

    // #i10770#: If there are fly frames anchored at previous paragraphs,
    // the deadline should consider their lower borders.
    const SwFrm* pStartFrm = pFrm->GetUpper()->GetLower();
    OSL_ENSURE( pStartFrm, "Upper has no lower" );
    SwTwips nFlyLower = bVert ? LONG_MAX : 0;
    while ( pStartFrm != pFrm )
    {
        OSL_ENSURE( pStartFrm, "Frame chain is broken" );
        if ( pStartFrm->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pStartFrm->GetDrawObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                SwRect aRect( pAnchoredObj->GetObjRect() );

                if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) ==  nullptr ||
                     static_cast<SwFlyFrm*>(pAnchoredObj)->IsValid() )
                {
                    const SwTwips nBottom = (aRect.*fnRect->fnGetBottom)();
                    if ( (*fnRect->fnYDiff)( nBottom, nFlyLower ) > 0 )
                        nFlyLower = nBottom;
                }
            }
        }

        pStartFrm = pStartFrm->GetNext();
    }

    if ( bVert )
        nRet = std::min( nRet, nFlyLower );
    else
        nRet = std::max( nRet, nFlyLower );

    return nRet;
}

SwTwips SwTextFrm::GetFootnoteLine( const SwTextFootnote *pFootnote ) const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTextFrm::GetFootnoteLine with swapped frame" );

    SwTextFrm *pThis = const_cast<SwTextFrm*>(this);

    if( !HasPara() )
    {
        // #109071# GetFormatted() does not work here, because most probably
        // the frame is currently locked. We return the previous value.
        return pThis->mnFootnoteLine > 0 ?
               pThis->mnFootnoteLine :
               IsVertical() ? Frm().Left() : Frm().Bottom();
    }

    SwTwips nRet;
    {
        SwSwapIfNotSwapped swap(const_cast<SwTextFrm *>(this));

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
 * Calculates the maximum reachable height for the TextFrm in the Footnote Area.
 * The cell's bottom margin with the Footnote Reference limit's this height.
 */
SwTwips SwTextFrm::_GetFootnoteFrmHeight() const
{
    OSL_ENSURE( !IsFollow() && IsInFootnote(), "SwTextFrm::SetFootnoteLine: moon walk" );

    const SwFootnoteFrm *pFootnoteFrm = FindFootnoteFrm();
    const SwTextFrm *pRef = static_cast<const SwTextFrm *>(pFootnoteFrm->GetRef());
    const SwFootnoteBossFrm *pBoss = FindFootnoteBossFrm();
    if( pBoss != pRef->FindFootnoteBossFrm( !pFootnoteFrm->GetAttr()->
                                        GetFootnote().IsEndNote() ) )
        return 0;

    SwSwapIfSwapped swap(const_cast<SwTextFrm *>(this));

    SwTwips nHeight = pRef->IsInFootnoteConnect() ?
                            1 : pRef->GetFootnoteLine( pFootnoteFrm->GetAttr() );
    if( nHeight )
    {
        // As odd as it may seem: the first Footnote on the page may not touch the
        // Footnote Reference, when entering text in the Footnote Area.
        const SwFrm *pCont = pFootnoteFrm->GetUpper();

        // Height within the Container which we're allowed to consume anyways
        SWRECTFN( pCont )
        SwTwips nTmp = (*fnRect->fnYDiff)( (pCont->*fnRect->fnGetPrtBottom)(),
                                           (Frm().*fnRect->fnGetTop)() );

#if OSL_DEBUG_LEVEL > 0
        if( nTmp < 0 )
        {
            bool bInvalidPos = false;
            const SwLayoutFrm* pTmp = GetUpper();
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

        if ( (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(), nHeight) > 0 )
        {
            // Growth potential of the container
            if ( !pRef->IsInFootnoteConnect() )
            {
                SwSaveFootnoteHeight aSave( const_cast<SwFootnoteBossFrm*>(pBoss), nHeight  );
                nHeight = const_cast<SwFootnoteContFrm*>(static_cast<const SwFootnoteContFrm*>(pCont))->Grow( LONG_MAX, true );
            }
            else
                nHeight = const_cast<SwFootnoteContFrm*>(static_cast<const SwFootnoteContFrm*>(pCont))->Grow( LONG_MAX, true );

            nHeight += nTmp;
            if( nHeight < 0 )
                nHeight = 0;
        }
        else
        {   // The container has to shrink
            nTmp += (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(), nHeight);
            if( nTmp > 0 )
                nHeight = nTmp;
            else
                nHeight = 0;
        }
    }

    return nHeight;
}

SwTextFrm *SwTextFrm::FindQuoVadisFrm()
{
    // Check whether we're in a FootnoteFrm
    if( GetIndPrev() || !IsInFootnote() )
        return nullptr;

    // To the preceding FootnoteFrm
    SwFootnoteFrm *pFootnoteFrm = FindFootnoteFrm()->GetMaster();
    if( !pFootnoteFrm )
        return nullptr;

    // Now the last Content
    SwContentFrm *pCnt = pFootnoteFrm->ContainsContent();
    if( !pCnt )
        return nullptr;
    SwContentFrm *pLast;
    do
    {   pLast = pCnt;
        pCnt = pCnt->GetNextContentFrm();
    } while( pCnt && pFootnoteFrm->IsAnLower( pCnt ) );
    return static_cast<SwTextFrm*>(pLast);
}

void SwTextFrm::RemoveFootnote( const sal_Int32 nStart, const sal_Int32 nLen )
{
    if ( !IsFootnoteAllowed() )
        return;

    SwpHints *pHints = GetTextNode()->GetpSwpHints();
    if( !pHints )
        return;

    bool bRollBack = nLen != COMPLETE_STRING;
    const size_t nSize = pHints->Count();
    sal_Int32 nEnd;
    SwTextFrm* pSource;
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
        SwPageFrm* pUpdate = nullptr;
        bool bRemove = false;
        SwFootnoteBossFrm *pFootnoteBoss = nullptr;
        SwFootnoteBossFrm *pEndBoss = nullptr;
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
                        pEndBoss = pSource->FindFootnoteBossFrm();
                }
                else
                {
                    if( !pFootnoteBoss )
                    {
                        pFootnoteBoss = pSource->FindFootnoteBossFrm( true );
                        if( pFootnoteBoss->GetUpper()->IsSctFrm() )
                        {
                            SwSectionFrm* pSect = static_cast<SwSectionFrm*>(
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
                SwFootnoteFrm *pFootnoteFrm = SwFootnoteBossFrm::FindFootnote(pSource, pFootnote);

                if( pFootnoteFrm )
                {
                    const bool bEndDoc = bEndn || bFootnoteEndDoc;
                    if( bRollBack )
                    {
                        while ( pFootnoteFrm )
                        {
                            pFootnoteFrm->SetRef( this );
                            pFootnoteFrm = pFootnoteFrm->GetFollow();
                            SetFootnote( true );
                        }
                    }
                    else if( GetFollow() )
                    {
                        SwContentFrm *pDest = GetFollow();
                        while( pDest->GetFollow() && static_cast<SwTextFrm*>(pDest->
                               GetFollow())->GetOfst() <= nIdx )
                            pDest = pDest->GetFollow();
                        OSL_ENSURE( !SwFootnoteBossFrm::FindFootnote(
                            pDest,pFootnote),"SwTextFrm::RemoveFootnote: footnote exists");

                        // Never deregister; always move
                        if ( bEndDoc ||
                             !pFootnoteFrm->FindFootnoteBossFrm()->IsBefore( pDest->FindFootnoteBossFrm( !bEndn ) )
                           )
                        {
                            SwPageFrm* pTmp = pFootnoteFrm->FindPageFrm();
                            if( pUpdate && pUpdate != pTmp )
                                pUpdate->UpdateFootnoteNum();
                            pUpdate = pTmp;
                            while ( pFootnoteFrm )
                            {
                                pFootnoteFrm->SetRef( pDest );
                                pFootnoteFrm = pFootnoteFrm->GetFollow();
                            }
                        }
                        else
                        {
                            pFootnoteBoss->MoveFootnotes( this, pDest, pFootnote );
                            bRemove = true;
                        }
                        static_cast<SwTextFrm*>(pDest)->SetFootnote( true );

                        OSL_ENSURE( SwFootnoteBossFrm::FindFootnote( pDest,
                           pFootnote),"SwTextFrm::RemoveFootnote: footnote ChgRef failed");
                    }
                    else
                    {
                        if( !bEndDoc || ( bEndn && pEndBoss->IsInSct() &&
                            !SwLayouter::Collecting( GetNode()->GetDoc(),
                            pEndBoss->FindSctFrm(), nullptr ) ) )
                        {
                            if( bEndn )
                                pEndBoss->RemoveFootnote( this, pFootnote );
                            else
                                pFootnoteBoss->RemoveFootnote( this, pFootnote );
                            bRemove = bRemove || !bEndDoc;
                            OSL_ENSURE( !SwFootnoteBossFrm::FindFootnote( this, pFootnote ),
                            "SwTextFrm::RemoveFootnote: can't get off that footnote" );
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
            ValidateBodyFrm();
            ValidateFrm();
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
 *    => we move it, if another pSrcFrm has been found
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
void SwTextFrm::ConnectFootnote( SwTextFootnote *pFootnote, const SwTwips nDeadLine )
{
    OSL_ENSURE( !IsVertical() || !IsSwapped(),
            "SwTextFrm::ConnectFootnote with swapped frame" );

    mbFootnote = true;
    mbInFootnoteConnect = true; // Just reset!
    const bool bEnd = pFootnote->GetFootnote().IsEndNote();

    // We want to store this value, because it is needed as a fallback
    // in GetFootnoteLine(), if there is no paragraph information available
    mnFootnoteLine = nDeadLine;

    // We always need a parent (Page/Column)
    SwSectionFrm *pSect;
    SwContentFrm *pContent = this;
    if( bEnd && IsInSct() )
    {
        pSect = FindSctFrm();
        if( pSect->IsEndnAtEnd() )
            pContent = pSect->FindLastContent( FINDMODE_ENDNOTE );
        if( !pContent )
            pContent = this;
    }

    SwFootnoteBossFrm *pBoss = pContent->FindFootnoteBossFrm( !bEnd );

#if OSL_DEBUG_LEVEL > 1
    SwTwips nRstHeight = GetRstHeight();
#endif

    pSect = pBoss->FindSctFrm();
    bool bDocEnd = bEnd ? !( pSect && pSect->IsEndnAtEnd() ) :
                   ( !( pSect && pSect->IsFootnoteAtEnd() ) &&
                       FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFootnoteInfo().ePos );

    // Footnote can be registered with the Follow
    SwContentFrm *pSrcFrm = FindFootnoteRef( pFootnote );

    if( bDocEnd )
    {
        if( pSect && pSrcFrm )
        {
            SwFootnoteFrm *pFootnoteFrm = SwFootnoteBossFrm::FindFootnote( pSrcFrm, pFootnote );
            if( pFootnoteFrm && pFootnoteFrm->IsInSct() )
            {
                pBoss->RemoveFootnote( pSrcFrm, pFootnote );
                pSrcFrm = nullptr;
            }
        }
    }
    else if( bEnd && pSect )
    {
        SwFootnoteFrm *pFootnoteFrm = pSrcFrm ? SwFootnoteBossFrm::FindFootnote( pSrcFrm, pFootnote ) : nullptr;
        if( pFootnoteFrm && !pFootnoteFrm->GetUpper() )
            pFootnoteFrm = nullptr;
        SwDoc *pDoc = GetNode()->GetDoc();
        if( SwLayouter::Collecting( pDoc, pSect, pFootnoteFrm ) )
        {
            if( !pSrcFrm )
            {
                SwFootnoteFrm *pNew = new SwFootnoteFrm(pDoc->GetDfltFrameFormat(),this,this,pFootnote);
                 SwNodeIndex aIdx( *pFootnote->GetStartNode(), 1 );
                 ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
                GetNode()->getIDocumentLayoutAccess().GetLayouter()->CollectEndnote( pNew );
            }
            else if( pSrcFrm != this )
                SwFootnoteBossFrm::ChangeFootnoteRef( pSrcFrm, pFootnote, this );
            mbInFootnoteConnect = false;
            return;
        }
        else if( pSrcFrm )
        {
            SwFootnoteBossFrm *pFootnoteBoss = pFootnoteFrm->FindFootnoteBossFrm();
            if( !pFootnoteBoss->IsInSct() ||
                pFootnoteBoss->ImplFindSctFrm()->GetSection()!=pSect->GetSection() )
            {
                pBoss->RemoveFootnote( pSrcFrm, pFootnote );
                pSrcFrm = nullptr;
            }
        }
    }

    if( bDocEnd || bEnd )
    {
        if( !pSrcFrm )
            pBoss->AppendFootnote( this, pFootnote );
        else if( pSrcFrm != this )
            SwFootnoteBossFrm::ChangeFootnoteRef( pSrcFrm, pFootnote, this );
        mbInFootnoteConnect = false;
        return;
    }

    SwSaveFootnoteHeight aHeight( pBoss, nDeadLine );

    if( !pSrcFrm ) // No Footnote was found at all
        pBoss->AppendFootnote( this, pFootnote );
    else
    {
        SwFootnoteFrm *pFootnoteFrm = SwFootnoteBossFrm::FindFootnote( pSrcFrm, pFootnote );
        SwFootnoteBossFrm *pFootnoteBoss = pFootnoteFrm->FindFootnoteBossFrm();

        bool bBrutal = false;

        if( pFootnoteBoss == pBoss ) // Ref and Footnote are on the same Page/Column
        {
            SwFrm *pCont = pFootnoteFrm->GetUpper();

            SWRECTFN ( pCont )
            long nDiff = (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(),
                                             nDeadLine );

            if( nDiff >= 0 )
            {
                // If the Footnote has been registered to a Follow, we need to
                // rewire it now too
                if ( pSrcFrm != this )
                    SwFootnoteBossFrm::ChangeFootnoteRef( pSrcFrm, pFootnote, this );

                // We have some room left, so the Footnote can grow
                if ( pFootnoteFrm->GetFollow() && nDiff > 0 )
                {
                    SwTwips nHeight = (pCont->Frm().*fnRect->fnGetHeight)();
                    pBoss->RearrangeFootnotes( nDeadLine, false, pFootnote );
                    ValidateBodyFrm();
                    ValidateFrm();
                    SwViewShell *pSh = getRootFrm()->GetCurrShell();
                    if ( pSh && nHeight == (pCont->Frm().*fnRect->fnGetHeight)() )
                        // So that we don't miss anything
                        pSh->InvalidateWindows( pCont->Frm() );
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
            SwFrm* pTmp = this;
            while( pTmp->GetNext() && pSrcFrm != pTmp )
                pTmp = pTmp->GetNext();
            if( pSrcFrm == pTmp )
                bBrutal = true;
            else
            {   // If our Parent is in a column Area, but the Page already has a
                // FootnoteContainer, we can only brute force it
                if( pSect && pSect->FindFootnoteBossFrm( !bEnd )->FindFootnoteCont() )
                    bBrutal = true;

                else if ( !pFootnoteFrm->GetPrev() ||
                          pFootnoteBoss->IsBefore( pBoss )
                        )
                {
                    SwFootnoteBossFrm *pSrcBoss = pSrcFrm->FindFootnoteBossFrm( !bEnd );
                    pSrcBoss->MoveFootnotes( pSrcFrm, this, pFootnote );
                }
                else
                    SwFootnoteBossFrm::ChangeFootnoteRef( pSrcFrm, pFootnote, this );
            }
        }

        // The brute force method: Remove Footnote and append.
        // We need to call SetFootnoteDeadLine(), as we can more easily adapt the
        // nMaxFootnoteHeight after RemoveFootnote
        if( bBrutal )
        {
            pBoss->RemoveFootnote( pSrcFrm, pFootnote, false );
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
        ValidateBodyFrm();
        pBoss->RearrangeFootnotes( nDeadLine, true );
        ValidateFrm();
    }
    else if( pSect->IsFootnoteAtEnd() )
    {
        ValidateBodyFrm();
        ValidateFrm();
    }

#if OSL_DEBUG_LEVEL > 1
    // pFootnoteFrm may have changed due to Calc ...
    SwFootnoteFrm *pFootnoteFrm = pBoss->FindFootnote( this, pFootnote );
    if( pFootnoteFrm && pBoss != pFootnoteFrm->FindFootnoteBossFrm( !bEnd ) )
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
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "NewFootnotePortion with unswapped frame" );

    if( !pFrm->IsFootnoteAllowed() )
        return nullptr;

    SwTextFootnote  *pFootnote = static_cast<SwTextFootnote*>(pHint);
    const SwFormatFootnote& rFootnote = static_cast<const SwFormatFootnote&>(pFootnote->GetFootnote());
    SwDoc *pDoc = pFrm->GetNode()->GetDoc();

    if( rInf.IsTest() )
        return new SwFootnotePortion( rFootnote.GetViewNumStr( *pDoc ), pFootnote );

    SwSwapIfSwapped swap(pFrm);

    sal_uInt16 nReal;
    {
        sal_uInt16 nOldReal = pCurr->GetRealHeight();
        sal_uInt16 nOldAscent = pCurr->GetAscent();
        sal_uInt16 nOldHeight = pCurr->Height();
        CalcRealHeight();
        nReal = pCurr->GetRealHeight();
        if( nReal < nOldReal )
            nReal = nOldReal;
        pCurr->SetRealHeight( nOldReal );
        pCurr->Height( nOldHeight );
        pCurr->SetAscent( nOldAscent );
    }

    SwTwips nLower = Y() + nReal;

    const bool bVertical = pFrm->IsVertical();
    if( bVertical )
        nLower = pFrm->SwitchHorizontalToVertical( nLower );

    nLower = lcl_GetFootnoteLower( pFrm, nLower );

    // We just refresh.
    // The Connect does not do anything useful in this case, but will
    // mostly throw away the Footnote and create it anew.
    if( !rInf.IsQuick() )
        pFrm->ConnectFootnote( pFootnote, nLower );

    SwTextFrm *pScrFrm = pFrm->FindFootnoteRef( pFootnote );
    SwFootnoteBossFrm *pBoss = pFrm->FindFootnoteBossFrm( !rFootnote.IsEndNote() );
    SwFootnoteFrm *pFootnoteFrm = nullptr;
    if( pScrFrm )
        pFootnoteFrm = SwFootnoteBossFrm::FindFootnote( pScrFrm, pFootnote );

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
        SwSectionFrm *pSct = pBoss->FindSctFrm();
        bool bAtSctEnd = pSct && pSct->IsFootnoteAtEnd();
        if( FTNPOS_CHAPTER != pDoc->GetFootnoteInfo().ePos || bAtSctEnd )
        {
            SwFrm* pFootnoteCont = pBoss->FindFootnoteCont();
            // If the Parent is within an Area, it can only be a Column of this
            // Area. If this one is not the first Column, we can avoid it.
            if( !pFrm->IsInTab() && ( GetLineNr() > 1 || pFrm->GetPrev() ||
                ( !bAtSctEnd && pFrm->GetIndPrev() ) ||
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
                        SwFootnoteBossFrm* pTmp = pBoss->FindSctFrm()->FindFootnoteBossFrm( true );
                        SwFootnoteContFrm* pFootnoteC = pTmp->FindFootnoteCont();
                        if( pFootnoteC )
                        {
                            SwFootnoteFrm* pTmpFrm = static_cast<SwFootnoteFrm*>(pFootnoteC->Lower());
                            if( pTmpFrm && *pTmpFrm < pFootnote )
                            {
                                rInf.SetStop( true );
                                return nullptr;
                            }
                        }
                    }
                    // Is this the last Line that fits?
                    SwTwips nTmpBot = Y() + nReal * 2;

                    if( bVertical )
                        nTmpBot = pFrm->SwitchHorizontalToVertical( nTmpBot );

                    SWRECTFN( pFootnoteCont )

                    const long nDiff = (*fnRect->fnYDiff)(
                                            (pFootnoteCont->Frm().*fnRect->fnGetTop)(),
                                             nTmpBot );

                    if( pScrFrm && nDiff < 0 )
                    {
                        if( pFootnoteFrm )
                        {
                            SwFootnoteBossFrm *pFootnoteBoss = pFootnoteFrm->FindFootnoteBossFrm();
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
    OSL_ENSURE( pFrm->IsInFootnote() && !pFrm->GetIndPrev() && !rInf.IsFootnoteDone(),
            "This is the wrong place for a ftnnumber" );
    if( rInf.GetTextStart() != nStart ||
        rInf.GetTextStart() != rInf.GetIdx() )
        return nullptr;

    const SwFootnoteFrm* pFootnoteFrm = pFrm->FindFootnoteFrm();
    const SwTextFootnote* pFootnote = pFootnoteFrm->GetAttr();

    // Aha! So we're in the Footnote Area!
    SwFormatFootnote& rFootnote = (SwFormatFootnote&)pFootnote->GetFootnote();

    SwDoc *pDoc = pFrm->GetNode()->GetDoc();
    OUString aFootnoteText( rFootnote.GetViewNumStr( *pDoc, true ));

    const SwEndNoteInfo* pInfo;
    if( rFootnote.IsEndNote() )
        pInfo = &pDoc->GetEndNoteInfo();
    else
        pInfo = &pDoc->GetFootnoteInfo();
    const SwAttrSet& rSet = pInfo->GetCharFormat(*pDoc)->GetAttrSet();

    const SwAttrSet* pParSet = &rInf.GetCharAttr();
    const IDocumentSettingAccess* pIDSA = pFrm->GetTextNode()->getIDocumentSettingAccess();
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
    pNumFnt->SetVertical( pNumFnt->GetOrientation(), pFrm->IsVertical() );

    SwFootnoteNumPortion* pNewPor = new SwFootnoteNumPortion( aFootnoteText, pNumFnt );
    pNewPor->SetLeft( !pFrm->IsRightToLeft() );
    return pNewPor;
}

OUString lcl_GetPageNumber( const SwPageFrm* pPage )
{
    OSL_ENSURE( pPage, "GetPageNumber: Homeless TextFrm" );
    const sal_uInt16 nVirtNum = pPage->GetVirtPageNum();
    const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
    return rNum.GetNumStr( nVirtNum );
}

SwErgoSumPortion *SwTextFormatter::NewErgoSumPortion( SwTextFormatInfo &rInf ) const
{
    // We cannot assume we're a Follow
    if( !pFrm->IsInFootnote()  || pFrm->GetPrev() ||
        rInf.IsErgoDone() || rInf.GetIdx() != pFrm->GetOfst() ||
        pFrm->ImplFindFootnoteFrm()->GetAttr()->GetFootnote().IsEndNote() )
        return nullptr;

    // Aha, wir sind also im Fussnotenbereich
    const SwFootnoteInfo &rFootnoteInfo = pFrm->GetNode()->GetDoc()->GetFootnoteInfo();
    SwTextFrm *pQuoFrm = pFrm->FindQuoVadisFrm();
    if( !pQuoFrm )
        return nullptr;
    const SwPageFrm* pPage = pFrm->FindPageFrm();
    const SwPageFrm* pQuoPage = pQuoFrm->FindPageFrm();
    if( pPage == pQuoFrm->FindPageFrm() )
        return nullptr; // If the QuoVadis is on the same Column/Page
    const OUString aPage = lcl_GetPageNumber( pPage );
    SwParaPortion *pPara = pQuoFrm->GetPara();
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
    OSL_ENSURE( ! pFrm->IsVertical() || ! pFrm->IsSwapped(),
            "SwTextFormatter::FormatQuoVadis with swapped frame" );

    if( !pFrm->IsInFootnote() || pFrm->ImplFindFootnoteFrm()->GetAttr()->GetFootnote().IsEndNote() )
        return nOffset;

    const SwFrm* pErgoFrm = pFrm->FindFootnoteFrm()->GetFollow();
    if( !pErgoFrm && pFrm->HasFollow() )
        pErgoFrm = pFrm->GetFollow();
    if( !pErgoFrm )
        return nOffset;

    if( pErgoFrm == pFrm->GetNext() )
    {
        SwFrm *pCol = pFrm->FindColFrm();
        while( pCol && !pCol->GetNext() )
            pCol = pCol->GetUpper()->FindColFrm();
        if( pCol )
            return nOffset;
    }
    else
    {
        const SwPageFrm* pPage = pFrm->FindPageFrm();
        const SwPageFrm* pErgoPage = pErgoFrm->FindPageFrm();
        if( pPage == pErgoPage )
            return nOffset; // If the ErgoSum is on the same Page
    }

    SwTextFormatInfo &rInf = GetInfo();
    const SwFootnoteInfo &rFootnoteInfo = pFrm->GetNode()->GetDoc()->GetFootnoteInfo();
    if( rFootnoteInfo.aQuoVadis.isEmpty() )
        return nOffset;

    // A remark on QuoVadis/ErgoSum:
    // We use the Font set for the Paragraph for these texts.
    // Thus, we initialze:
    // TODO: ResetFont();
    FeedInf( rInf );
    SeekStartAndChg( rInf, true );
    if( GetRedln() && pCurr->HasRedline() )
        GetRedln()->Seek( *pFnt, nOffset, 0 );

    // A tricky special case: Flyfrms extend into the Line and are at the
    // position we want to insert the Quovadis text
    // Let's see if it is that bad indeed:
    SwLinePortion *pPor = pCurr->GetFirstPortion();
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

    OUString aErgo = lcl_GetPageNumber( pErgoFrm->FindPageFrm() );
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
        SwSwapIfNotSwapped swap(pFrm);

        nRet = FormatLine( nStart );
    }

    Right( rInf.Left() + nOldRealWidth - 1 );

    nLastLeft = nOldRealWidth - pCurr->Width();
    FeedInf( rInf );

    // It's possible that there's a Margin Portion at the end, which would
    // just cause a lot of trouble, when respanning
    pPor = pCurr->FindLastPortion();
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
                    if( !pCurr->GetLen() ||
                        CH_BREAK != GetInfo().GetChar(nStart+pCurr->GetLen()-1))
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

    pCurr->Width( pCurr->Width() + nQuoWidth );

    // And adjust again, due to the adjustment and due to the following special
    // case:
    // The DummyUser has set a smaller Font in the Line than the one used
    // by the QuoVadis text ...
    CalcAdjustLine( pCurr );

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
    sal_uInt16 nRstHeight = GetFrmRstHeight();
    if( pCurr && nRstHeight > pCurr->Height() )
    {
        SwLineLayout *pLay = new SwLineLayout;
        nRstHeight = nRstHeight - pCurr->Height();
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
    if( pTextFootnote && rInf.GetTextFrm() )
    {
        pFnt = ((SwTextSizeInfo&)rInf).GetFont();
          pOld = new SwFont( *pFnt );
        pOld->GetTox() = pFnt->GetTox();
        pFnt->GetTox() = 0;
        SwFormatFootnote& rFootnote = (SwFormatFootnote&)pTextFootnote->GetFootnote();
        const SwDoc *pDoc = rInf.GetTextFrm()->GetNode()->GetDoc();

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
        pFnt->SetDiffFnt( &rSet, rInf.GetTextFrm()->GetNode()->getIDocumentSettingAccess() );

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
                                rInf.GetTextFrm()->IsVertical() );

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

sal_Int32 SwErgoSumPortion::GetCrsrOfst( const sal_uInt16 ) const
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
 * Is called in SwTextFrm::Prepare()
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
