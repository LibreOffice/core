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

#include "ndtxt.hxx"
#include "flyfrm.hxx"
#include "paratr.hxx"
#include <vcl/outdev.hxx>
#include <editeng/paravertalignitem.hxx>

#include "pormulti.hxx"
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <porfld.hxx>

#include "itrtxt.hxx"
#include "txtfrm.hxx"
#include "porfly.hxx"

void SwTextIter::CtorInitTextIter( SwTextFrm *pNewFrm, SwTextInfo *pNewInf )
{
    SwTextNode *pNode = pNewFrm->GetTextNode();

    OSL_ENSURE( pNewFrm->GetPara(), "No paragraph" );

    CtorInitAttrIter( *pNode, pNewFrm->GetPara()->GetScriptInfo(), pNewFrm );

    pFrm = pNewFrm;
    pInf = pNewInf;
    aLineInf.CtorInitLineInfo( pNode->GetSwAttrSet(), *pNode );
    nFrameStart = pFrm->Frm().Pos().Y() + pFrm->Prt().Pos().Y();
    SwTextIter::Init();

    // Order is important: only execute FillRegister if GetValue!=0
    bRegisterOn = pNode->GetSwAttrSet().GetRegister().GetValue()
        && pFrm->FillRegister( nRegStart, nRegDiff );
}

void SwTextIter::Init()
{
    pCurr = pInf->GetParaPortion();
    nStart = pInf->GetTextStart();
    nY = nFrameStart;
    bPrev = true;
    pPrev = nullptr;
    nLineNr = 1;
}

void SwTextIter::CalcAscentAndHeight( sal_uInt16 &rAscent, sal_uInt16 &rHeight ) const
{
    rHeight = GetLineHeight();
    rAscent = pCurr->GetAscent() + rHeight - pCurr->Height();
}

SwLineLayout *SwTextIter::_GetPrev()
{
    pPrev = nullptr;
    bPrev = true;
    SwLineLayout *pLay = pInf->GetParaPortion();
    if( pCurr == pLay )
        return nullptr;
    while( pLay->GetNext() != pCurr )
        pLay = pLay->GetNext();
    return pPrev = pLay;
}

const SwLineLayout *SwTextIter::GetPrev()
{
    if(! bPrev)
        _GetPrev();
    return pPrev;
}

const SwLineLayout *SwTextIter::Prev()
{
    if( !bPrev )
        _GetPrev();
    if( pPrev )
    {
        bPrev = false;
        pCurr = pPrev;
        nStart = nStart - pCurr->GetLen();
        nY = nY - GetLineHeight();
        if( !pCurr->IsDummy() && !(--nLineNr) )
            ++nLineNr;
        return pCurr;
    }
    else
        return nullptr;
}

const SwLineLayout *SwTextIter::Next()
{
    if(pCurr->GetNext())
    {
        pPrev = pCurr;
        bPrev = true;
        nStart = nStart + pCurr->GetLen();
        nY += GetLineHeight();
        if( pCurr->GetLen() || ( nLineNr>1 && !pCurr->IsDummy() ) )
            ++nLineNr;
        return pCurr = pCurr->GetNext();
    }
    else
        return nullptr;
}

const SwLineLayout *SwTextIter::NextLine()
{
    const SwLineLayout *pNext = Next();
    while( pNext && pNext->IsDummy() && pNext->GetNext() )
    {
        pNext = Next();
    }
    return pNext;
}

const SwLineLayout *SwTextIter::GetNextLine() const
{
    const SwLineLayout *pNext = pCurr->GetNext();
    while( pNext && pNext->IsDummy() && pNext->GetNext() )
    {
        pNext = pNext->GetNext();
    }
    return pNext;
}

const SwLineLayout *SwTextIter::GetPrevLine()
{
    const SwLineLayout *pRoot = pInf->GetParaPortion();
    if( pRoot == pCurr )
        return nullptr;
    const SwLineLayout *pLay = pRoot;

    while( pLay->GetNext() != pCurr )
        pLay = pLay->GetNext();

    if( pLay->IsDummy() )
    {
        const SwLineLayout *pTmp = pRoot;
        pLay = pRoot->IsDummy() ? nullptr : pRoot;
        while( pTmp->GetNext() != pCurr )
        {
            if( !pTmp->IsDummy() )
                pLay = pTmp;
            pTmp = pTmp->GetNext();
        }
    }

    // Wenn sich nichts getan hat, dann gibt es nur noch Dummys
    return pLay;
}

const SwLineLayout *SwTextIter::PrevLine()
{
    const SwLineLayout *pMyPrev = Prev();
    if( !pMyPrev )
        return nullptr;

    const SwLineLayout *pLast = pMyPrev;
    while( pMyPrev && pMyPrev->IsDummy() )
    {
        pLast = pMyPrev;
        pMyPrev = Prev();
    }
    return pMyPrev ? pMyPrev : pLast;
}

void SwTextIter::Bottom()
{
    while( Next() )
    {
        // nothing
    }
}

void SwTextIter::CharToLine(const sal_Int32 nChar)
{
    while( nStart + pCurr->GetLen() <= nChar && Next() )
        ;
    while( nStart > nChar && Prev() )
        ;
}

// 1170: beruecksichtigt Mehrdeutigkeiten:
const SwLineLayout *SwTextCursor::CharCrsrToLine( const sal_Int32 nPosition )
{
    CharToLine( nPosition );
    if( nPosition != nStart )
        bRightMargin = false;
    bool bPrevious = bRightMargin && pCurr->GetLen() && GetPrev() &&
        GetPrev()->GetLen();
    if( bPrevious && nPosition && CH_BREAK == GetInfo().GetChar( nPosition-1 ) )
        bPrevious = false;
    return bPrevious ? PrevLine() : pCurr;
}

sal_uInt16 SwTextCursor::AdjustBaseLine( const SwLineLayout& rLine,
                                    const SwLinePortion* pPor,
                                    sal_uInt16 nPorHeight, sal_uInt16 nPorAscent,
                                    const bool bAutoToCentered ) const
{
    if ( pPor )
    {
        nPorHeight = pPor->Height();
        nPorAscent = pPor->GetAscent();
    }

    sal_uInt16 nOfst = rLine.GetRealHeight() - rLine.Height();

    SwTextGridItem const*const pGrid(GetGridItem(pFrm->FindPageFrm()));

    if ( pGrid && GetInfo().SnapToGrid() )
    {
        const sal_uInt16 nRubyHeight = pGrid->GetRubyHeight();
        const bool bRubyTop = ! pGrid->GetRubyTextBelow();

        if ( GetInfo().IsMulti() )
            // we are inside the GetCharRect recursion for multi portions
            // we center the portion in its surrounding line
            nOfst = ( pCurr->Height() - nPorHeight ) / 2 + nPorAscent;
        else
        {
            // We have to take care for ruby portions.
            // The ruby portion is NOT centered
            nOfst = nOfst + nPorAscent;

            if ( ! pPor || ! pPor->IsMultiPortion() ||
                 ! static_cast<const SwMultiPortion*>(pPor)->IsRuby() )
            {
                // Portions which are bigger than on grid distance are
                // centered inside the whole line.

                //for text refactor
                const sal_uInt16 nLineNet =  rLine.Height() - nRubyHeight;
                //const sal_uInt16 nLineNet = ( nPorHeight > nGridWidth ) ?
                 //                           rLine.Height() - nRubyHeight :
                 //                           nGridWidth;
                nOfst += ( nLineNet - nPorHeight ) / 2;
                if ( bRubyTop )
                    nOfst += nRubyHeight;
            }
        }
    }
    else
    {
        switch ( GetLineInfo().GetVertAlign() ) {
            case SvxParaVertAlignItem::TOP :
                nOfst = nOfst + nPorAscent;
                break;
            case SvxParaVertAlignItem::CENTER :
                OSL_ENSURE( rLine.Height() >= nPorHeight, "Portion height > Line height");
                nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
                break;
            case SvxParaVertAlignItem::BOTTOM :
                nOfst += rLine.Height() - nPorHeight + nPorAscent;
                break;
            case SvxParaVertAlignItem::AUTOMATIC :
                if ( bAutoToCentered || GetInfo().GetTextFrm()->IsVertical() )
                {
                    if( GetInfo().GetTextFrm()->IsVertLR() )
                            nOfst += rLine.Height() - ( rLine.Height() - nPorHeight ) / 2 - nPorAscent;
                    else
                            nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
                    break;
                }
            case SvxParaVertAlignItem::BASELINE :
                // base line
                nOfst = nOfst + rLine.GetAscent();
                break;
        }
    }

    return nOfst;
}

const SwLineLayout *SwTextIter::TwipsToLine( const SwTwips y)
{
    while( nY + GetLineHeight() <= y && Next() )
        ;
    while( nY > y && Prev() )
        ;
    return pCurr;
}

// Local helper function to check, if pCurr needs a field rest portion:
static bool lcl_NeedsFieldRest( const SwLineLayout* pCurr )
{
    const SwLinePortion *pPor = pCurr->GetPortion();
    bool bRet = false;
    while( pPor && !bRet )
    {
        bRet = pPor->InFieldGrp() && static_cast<const SwFieldPortion*>(pPor)->HasFollow();
        if( !pPor->GetPortion() || !pPor->GetPortion()->InFieldGrp() )
            break;
        pPor = pPor->GetPortion();
    }
    return bRet;
}

void SwTextIter::TruncLines( bool bNoteFollow )
{
    SwLineLayout *pDel = pCurr->GetNext();
    const sal_Int32 nEnd = nStart + pCurr->GetLen();

    if( pDel )
    {
        pCurr->SetNext( nullptr );
        if( GetHints() && bNoteFollow )
        {
            GetInfo().GetParaPortion()->SetFollowField( pDel->IsRest() ||
                                                        lcl_NeedsFieldRest( pCurr ) );

            // bug 88534: wrong positioning of flys
            SwTextFrm* pFollow = GetTextFrm()->GetFollow();
            if ( pFollow && ! pFollow->IsLocked() &&
                 nEnd == pFollow->GetOfst() )
            {
                sal_Int32 nRangeEnd = nEnd;
                SwLineLayout* pLine = pDel;

                // determine range to be searched for flys anchored as characters
                while ( pLine )
                {
                    nRangeEnd = nRangeEnd + pLine->GetLen();
                    pLine = pLine->GetNext();
                }

                SwpHints* pTmpHints = GetTextFrm()->GetTextNode()->GetpSwpHints();

                // examine hints in range nEnd - (nEnd + nRangeChar)
                for( size_t i = 0; i < pTmpHints->Count(); ++i )
                {
                    const SwTextAttr* pHt = pTmpHints->Get( i );
                    if( RES_TXTATR_FLYCNT == pHt->Which() )
                    {
                        // check, if hint is in our range
                        const sal_uInt16 nTmpPos = pHt->GetStart();
                        if ( nEnd <= nTmpPos && nTmpPos < nRangeEnd )
                            pFollow->_InvalidateRange(
                                SwCharRange( nTmpPos, nTmpPos ) );
                    }
                }
            }
        }
        delete pDel;
    }
    if( pCurr->IsDummy() &&
        !pCurr->GetLen() &&
         nStart < GetTextFrm()->GetText().getLength() )
        pCurr->SetRealHeight( 1 );
    if( GetHints() )
        pFrm->RemoveFootnote( nEnd );
}

void SwTextIter::CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const
{
    nEndCnt = 0;
    nMidCnt = 0;
    if ( bPrev && pPrev && !pPrev->IsEndHyph() && !pPrev->IsMidHyph() )
         return;
    SwLineLayout *pLay = pInf->GetParaPortion();
    if( pCurr == pLay )
        return;
    while( pLay != pCurr )
    {
        if ( pLay->IsEndHyph() )
            nEndCnt++;
        else
            nEndCnt = 0;
        if ( pLay->IsMidHyph() )
            nMidCnt++;
        else
            nMidCnt = 0;
        pLay = pLay->GetNext();
    }
}

// Change current output device to formatting device, this has to be done before
// formatting.
SwHookOut::SwHookOut( SwTextSizeInfo& rInfo ) :
     pInf( &rInfo ),
     pOut( rInfo.GetOut() ),
     bOnWin( rInfo.OnWin() )
{
    OSL_ENSURE( rInfo.GetRefDev(), "No reference device for text formatting" );

    // set new values
    rInfo.SetOut( rInfo.GetRefDev() );
    rInfo.SetOnWin( false );
}

SwHookOut::~SwHookOut()
{
    pInf->SetOut( pOut );
    pInf->SetOnWin( bOnWin );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
