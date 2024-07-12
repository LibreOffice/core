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

#include <sal/config.h>

#include <frminf.hxx>
#include "itrtxt.hxx"

TextFrameIndex SwTextMargin::GetTextStart() const
{
    const OUString &rText = GetInfo().GetText();
    const TextFrameIndex nEnd = m_nStart + m_pCurr->GetLen();

    for (TextFrameIndex i = m_nStart; i < nEnd; ++i)
    {
        const sal_Unicode aChar = rText[sal_Int32(i)];
        if( CH_TAB != aChar && ' ' != aChar )
            return i;
    }
    return nEnd;
}

TextFrameIndex SwTextMargin::GetTextEnd() const
{
    const OUString &rText = GetInfo().GetText();
    const TextFrameIndex nEnd = m_nStart + m_pCurr->GetLen();
    for (TextFrameIndex i = nEnd - TextFrameIndex(1); i >= m_nStart; --i)
    {
        const sal_Unicode aChar = rText[sal_Int32(i)];
        if( CH_TAB != aChar && CH_BREAK != aChar && ' ' != aChar )
            return i + TextFrameIndex(1);
    }
    return m_nStart;
}

// Does the paragraph fit into one line?
bool SwTextFrameInfo::IsOneLine() const
{
    const SwLineLayout *pLay = m_pFrame->GetPara();
    if( !pLay )
        return false;

    // For follows false of course
    if( m_pFrame->GetFollow() )
        return false;

    pLay = pLay->GetNext();
    while( pLay )
    {
        if( pLay->GetLen() )
            return false;
        pLay = pLay->GetNext();
    }
    return true;
}

// Is the line filled for X percent?
bool SwTextFrameInfo::IsFilled( const sal_uInt8 nPercent ) const
{
    const SwLineLayout *pLay = m_pFrame->GetPara();
    if( !pLay )
        return false;

    tools::Long nWidth = m_pFrame->getFramePrintArea().Width();
    nWidth *= nPercent;
    nWidth /= 100;
    return nWidth <= pLay->Width();
}

// Where does the text start (without whitespace)? (document global)
SwTwips SwTextFrameInfo::GetLineStart( const SwTextCursor &rLine )
{
    const TextFrameIndex nTextStart = rLine.GetTextStart();
    if( rLine.GetStart() == nTextStart )
        return rLine.GetLineStart();

    SwRect aRect;
    const_cast<SwTextCursor&>(rLine).GetCharRect( &aRect, nTextStart );
    return aRect.Left();
}

// Where does the text start (without whitespace)? (relative in the Frame)
SwTwips SwTextFrameInfo::GetLineStart() const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(m_pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(m_pFrame), &aInf );
    return GetLineStart( aLine ) - m_pFrame->getFrameArea().Left() - m_pFrame->getFramePrintArea().Left();
}

// Calculates the character's position and returns the middle position
SwTwips SwTextFrameInfo::GetCharPos(TextFrameIndex const nChar, bool bCenter) const
{
    SwRectFnSet aRectFnSet(m_pFrame);
    SwFrameSwapper aSwapper( m_pFrame, true );

    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(m_pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(m_pFrame), &aInf );

    SwTwips nStt, nNext;
    SwRect aRect;
    aLine.GetCharRect( &aRect, nChar );
    if ( aRectFnSet.IsVert() )
        m_pFrame->SwitchHorizontalToVertical( aRect );

    nStt = aRectFnSet.GetLeft(aRect);

    if( !bCenter )
        return nStt - aRectFnSet.GetLeft(m_pFrame->getFrameArea());

    aLine.GetCharRect( &aRect, nChar + TextFrameIndex(1) );
    if ( aRectFnSet.IsVert() )
        m_pFrame->SwitchHorizontalToVertical( aRect );

    nNext = aRectFnSet.GetLeft(aRect);

    return (( nNext + nStt ) / 2 ) - aRectFnSet.GetLeft(m_pFrame->getFrameArea());
}

static void
AddRange(std::vector<std::pair<TextFrameIndex, TextFrameIndex>> & rRanges,
            TextFrameIndex const nPos, TextFrameIndex const nLen)
{
    assert(rRanges.empty() || rRanges.back().second <= nPos);
    if( nLen )
    {
        if (!rRanges.empty() && nPos == rRanges.back().second)
        {
            rRanges.back().second += nLen;
        }
        else
        {
            rRanges.emplace_back(nPos, nPos + nLen);
        }
    }
}

// Accumulates the whitespace at line start and end in the vector
void SwTextFrameInfo::GetSpaces(
    std::vector<std::pair<TextFrameIndex, TextFrameIndex>> & rRanges,
    bool const bWithLineBreak) const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(m_pFrame) );
    SwTextMargin aLine( const_cast<SwTextFrame*>(m_pFrame), &aInf );
    bool bFirstLine = true;
    do {

        if( aLine.GetCurr()->GetLen() )
        {
            TextFrameIndex nPos = aLine.GetTextStart();
            // Do NOT include the blanks/tabs from the first line
            // in the selection
            if( !bFirstLine && nPos > aLine.GetStart() )
                AddRange( rRanges, aLine.GetStart(), nPos - aLine.GetStart() );

            // Do NOT include the blanks/tabs from the last line
            // in the selection
            if( aLine.GetNext() )
            {
                TextFrameIndex const nEndPos{aLine.GetTextEnd()};

                // if only whitespace in line, nEndPos < nPos !
                if (nPos < nEndPos && nEndPos < aLine.GetEnd())
                {
                    TextFrameIndex const nOff( !bWithLineBreak && CH_BREAK ==
                        aLine.GetInfo().GetChar(aLine.GetEnd() - TextFrameIndex(1))
                                ? 1 : 0 );
                    AddRange(rRanges, nEndPos, aLine.GetEnd() - nEndPos - nOff);
                }
            }
        }
        bFirstLine = false;
    }
    while( aLine.Next() );
}

// Is there a bullet/symbol etc. at the text position?
// Fonts: CharSet, SYMBOL and DONTKNOW
bool SwTextFrameInfo::IsBullet(TextFrameIndex const nTextStart) const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(m_pFrame) );
    SwTextMargin aLine( const_cast<SwTextFrame*>(m_pFrame), &aInf );
    aInf.SetIdx( nTextStart );
    return aLine.IsSymbol( nTextStart );
}

// Get first line indent
// The precondition for a positive or negative first line indent:
// All lines (except for the first one) have the same left margin.
// We do not want to be so picky and work with a tolerance of TOLERANCE twips.
SwTwips SwTextFrameInfo::GetFirstIndent() const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(m_pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(m_pFrame), &aInf );
    const SwTwips nFirst = GetLineStart( aLine );
    const SwTwips TOLERANCE = 20;

    if( !aLine.Next() )
        return 0;

    SwTwips nLeft = GetLineStart( aLine );
    while( aLine.Next() )
    {
        if( aLine.GetCurr()->GetLen() )
        {
            const SwTwips nCurrLeft = GetLineStart( aLine );
            if( nLeft + TOLERANCE < nCurrLeft ||
                nLeft - TOLERANCE > nCurrLeft )
                return 0;
        }
    }

    // At first we only return +1, -1 and 0
    if( nLeft == nFirst )
        return 0;

    if( nLeft > nFirst )
        return -1;

    return 1;
}

sal_Int32 SwTextFrameInfo::GetBigIndent(TextFrameIndex& rFndPos,
                                    const SwTextFrame *pNextFrame ) const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(m_pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(m_pFrame), &aInf );
    SwTwips nNextIndent = 0;

    if( pNextFrame )
    {
        // I'm a single line
        SwTextSizeInfo aNxtInf( const_cast<SwTextFrame*>(pNextFrame) );
        SwTextCursor aNxtLine( const_cast<SwTextFrame*>(pNextFrame), &aNxtInf );
        nNextIndent = GetLineStart( aNxtLine );
    }
    else
    {
        // I'm multi-line
        if( aLine.Next() )
        {
            nNextIndent = GetLineStart( aLine );
            aLine.Prev();
        }
    }

    if( nNextIndent <= GetLineStart( aLine ) )
        return 0;

    const Point aPoint( nNextIndent, aLine.Y() );
    rFndPos = aLine.GetModelPositionForViewPoint( nullptr, aPoint, false );
    if (TextFrameIndex(1) >= rFndPos)
        return 0;

    // Is on front of a non-space
    const OUString& rText = aInf.GetText();
    sal_Unicode aChar = rText[sal_Int32(rFndPos)];
    if( CH_TAB == aChar || CH_BREAK == aChar || ' ' == aChar ||
        (( CH_TXTATR_BREAKWORD == aChar || CH_TXTATR_INWORD == aChar ) &&
            aInf.HasHint( rFndPos ) ) )
        return 0;

    // and after a space
    aChar = rText[sal_Int32(rFndPos) - 1];
    if( CH_TAB != aChar && CH_BREAK != aChar &&
        ( ( CH_TXTATR_BREAKWORD != aChar && CH_TXTATR_INWORD != aChar ) ||
            !aInf.HasHint(rFndPos - TextFrameIndex(1))) &&
        // More than two Blanks!
        (' ' != aChar || ' ' != rText[sal_Int32(rFndPos) - 2]))
        return 0;

    SwRect aRect;
    aLine.GetCharRect( &aRect, rFndPos );
    return static_cast<sal_Int32>(aRect.Left() - m_pFrame->getFrameArea().Left() - m_pFrame->getFramePrintArea().Left());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
