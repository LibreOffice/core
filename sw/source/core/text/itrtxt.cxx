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

void SwTextIter::CtorInitTextIter( SwTextFrame *pNewFrame, SwTextInfo *pNewInf )
{
    SwTextNode *pNode = pNewFrame->GetTextNode();

    OSL_ENSURE( pNewFrame->GetPara(), "No paragraph" );

    CtorInitAttrIter( *pNode, pNewFrame->GetPara()->GetScriptInfo(), pNewFrame );

    m_pFrame = pNewFrame;
    m_pInf = pNewInf;
    m_aLineInf.CtorInitLineInfo( pNode->GetSwAttrSet(), *pNode );
    m_nFrameStart = m_pFrame->Frame().Pos().Y() + m_pFrame->Prt().Pos().Y();
    SwTextIter::Init();

    // Order is important: only execute FillRegister if GetValue!=0
    m_bRegisterOn = pNode->GetSwAttrSet().GetRegister().GetValue()
        && m_pFrame->FillRegister( m_nRegStart, m_nRegDiff );
}

void SwTextIter::Init()
{
    m_pCurr = m_pInf->GetParaPortion();
    m_nStart = m_pInf->GetTextStart();
    m_nY = m_nFrameStart;
    m_bPrev = true;
    m_pPrev = nullptr;
    m_nLineNr = 1;
}

void SwTextIter::CalcAscentAndHeight( sal_uInt16 &rAscent, sal_uInt16 &rHeight ) const
{
    rHeight = GetLineHeight();
    rAscent = m_pCurr->GetAscent() + rHeight - m_pCurr->Height();
}

SwLineLayout *SwTextIter::_GetPrev()
{
    m_pPrev = nullptr;
    m_bPrev = true;
    SwLineLayout *pLay = m_pInf->GetParaPortion();
    if( m_pCurr == pLay )
        return nullptr;
    while( pLay->GetNext() != m_pCurr )
        pLay = pLay->GetNext();
    return m_pPrev = pLay;
}

const SwLineLayout *SwTextIter::GetPrev()
{
    if(! m_bPrev)
        _GetPrev();
    return m_pPrev;
}

const SwLineLayout *SwTextIter::Prev()
{
    if( !m_bPrev )
        _GetPrev();
    if( m_pPrev )
    {
        m_bPrev = false;
        m_pCurr = m_pPrev;
        m_nStart = m_nStart - m_pCurr->GetLen();
        m_nY = m_nY - GetLineHeight();
        if( !m_pCurr->IsDummy() && !(--m_nLineNr) )
            ++m_nLineNr;
        return m_pCurr;
    }
    else
        return nullptr;
}

const SwLineLayout *SwTextIter::Next()
{
    if(m_pCurr->GetNext())
    {
        m_pPrev = m_pCurr;
        m_bPrev = true;
        m_nStart = m_nStart + m_pCurr->GetLen();
        m_nY += GetLineHeight();
        if( m_pCurr->GetLen() || ( m_nLineNr>1 && !m_pCurr->IsDummy() ) )
            ++m_nLineNr;
        return m_pCurr = m_pCurr->GetNext();
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
    const SwLineLayout *pNext = m_pCurr->GetNext();
    while( pNext && pNext->IsDummy() && pNext->GetNext() )
    {
        pNext = pNext->GetNext();
    }
    return pNext;
}

const SwLineLayout *SwTextIter::GetPrevLine()
{
    const SwLineLayout *pRoot = m_pInf->GetParaPortion();
    if( pRoot == m_pCurr )
        return nullptr;
    const SwLineLayout *pLay = pRoot;

    while( pLay->GetNext() != m_pCurr )
        pLay = pLay->GetNext();

    if( pLay->IsDummy() )
    {
        const SwLineLayout *pTmp = pRoot;
        pLay = pRoot->IsDummy() ? nullptr : pRoot;
        while( pTmp->GetNext() != m_pCurr )
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
    while( m_nStart + m_pCurr->GetLen() <= nChar && Next() )
        ;
    while( m_nStart > nChar && Prev() )
        ;
}

// 1170: beruecksichtigt Mehrdeutigkeiten:
const SwLineLayout *SwTextCursor::CharCursorToLine( const sal_Int32 nPosition )
{
    CharToLine( nPosition );
    if( nPosition != m_nStart )
        bRightMargin = false;
    bool bPrevious = bRightMargin && m_pCurr->GetLen() && GetPrev() &&
        GetPrev()->GetLen();
    if( bPrevious && nPosition && CH_BREAK == GetInfo().GetChar( nPosition-1 ) )
        bPrevious = false;
    return bPrevious ? PrevLine() : m_pCurr;
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

    SwTextGridItem const*const pGrid(GetGridItem(m_pFrame->FindPageFrame()));

    if ( pGrid && GetInfo().SnapToGrid() )
    {
        const sal_uInt16 nRubyHeight = pGrid->GetRubyHeight();
        const bool bRubyTop = ! pGrid->GetRubyTextBelow();

        if ( GetInfo().IsMulti() )
            // we are inside the GetCharRect recursion for multi portions
            // we center the portion in its surrounding line
            nOfst = ( m_pCurr->Height() - nPorHeight ) / 2 + nPorAscent;
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
                if ( bAutoToCentered || GetInfo().GetTextFrame()->IsVertical() )
                {
                    if( GetInfo().GetTextFrame()->IsVertLR() )
                            nOfst += rLine.Height() - ( rLine.Height() - nPorHeight ) / 2 - nPorAscent;
                    else
                            nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
                    break;
                }
                SAL_FALLTHROUGH;
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
    while( m_nY + GetLineHeight() <= y && Next() )
        ;
    while( m_nY > y && Prev() )
        ;
    return m_pCurr;
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
    SwLineLayout *pDel = m_pCurr->GetNext();
    const sal_Int32 nEnd = m_nStart + m_pCurr->GetLen();

    if( pDel )
    {
        m_pCurr->SetNext( nullptr );
        if( GetHints() && bNoteFollow )
        {
            GetInfo().GetParaPortion()->SetFollowField( pDel->IsRest() ||
                                                        lcl_NeedsFieldRest( m_pCurr ) );

            // bug 88534: wrong positioning of flys
            SwTextFrame* pFollow = GetTextFrame()->GetFollow();
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

                SwpHints* pTmpHints = GetTextFrame()->GetTextNode()->GetpSwpHints();

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
    if( m_pCurr->IsDummy() &&
        !m_pCurr->GetLen() &&
         m_nStart < GetTextFrame()->GetText().getLength() )
        m_pCurr->SetRealHeight( 1 );
    if( GetHints() )
        m_pFrame->RemoveFootnote( nEnd );
}

void SwTextIter::CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const
{
    nEndCnt = 0;
    nMidCnt = 0;
    if ( m_bPrev && m_pPrev && !m_pPrev->IsEndHyph() && !m_pPrev->IsMidHyph() )
         return;
    SwLineLayout *pLay = m_pInf->GetParaPortion();
    if( m_pCurr == pLay )
        return;
    while( pLay != m_pCurr )
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
