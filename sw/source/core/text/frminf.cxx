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

#include <pam.hxx>
#include <frminf.hxx>
#include <itrtxt.hxx>

sal_Int32 SwTextMargin::GetTextStart() const
{
    const OUString &rText = GetInfo().GetText();
    const sal_Int32 nEnd = m_nStart + m_pCurr->GetLen();

    for( sal_Int32 i = m_nStart; i < nEnd; ++i )
    {
        const sal_Unicode aChar = rText[i];
        if( CH_TAB != aChar && ' ' != aChar )
            return i;
    }
    return nEnd;
}

sal_Int32 SwTextMargin::GetTextEnd() const
{
    const OUString &rText = GetInfo().GetText();
    const sal_Int32 nEnd = m_nStart + m_pCurr->GetLen();
    for( sal_Int32 i = nEnd - 1; i >= m_nStart; --i )
    {
        const sal_Unicode aChar = rText[i];
        if( CH_TAB != aChar && CH_BREAK != aChar && ' ' != aChar )
            return i + 1;
    }
    return m_nStart;
}

// Does the paragraph fit into one line?
bool SwTextFrameInfo::IsOneLine() const
{
    const SwLineLayout *pLay = pFrame->GetPara();
    if( !pLay )
        return false;

    // For follows false of course
    if( pFrame->GetFollow() )
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
    const SwLineLayout *pLay = pFrame->GetPara();
    if( !pLay )
        return false;

    long nWidth = pFrame->Prt().Width();
    nWidth *= nPercent;
    nWidth /= 100;
    return sal_uInt16(nWidth) <= pLay->Width();
}

// Where does the text start (without whitespace)? (document global)
SwTwips SwTextFrameInfo::GetLineStart( const SwTextCursor &rLine )
{
    const sal_Int32 nTextStart = rLine.GetTextStart();
    if( rLine.GetStart() == nTextStart )
        return rLine.GetLineStart();

    SwRect aRect;
    if( const_cast<SwTextCursor&>(rLine).GetCharRect( &aRect, nTextStart ) )
        return aRect.Left();

    return rLine.GetLineStart();
}

// Where does the text start (without whitespace)? (relative in the Frame)
SwTwips SwTextFrameInfo::GetLineStart() const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(pFrame), &aInf );
    return GetLineStart( aLine ) - pFrame->Frame().Left() - pFrame->Prt().Left();
}

// Calculates the character's position and returns the middle position
SwTwips SwTextFrameInfo::GetCharPos( sal_Int32 nChar, bool bCenter ) const
{
    SWRECTFN( pFrame )
    SwFrameSwapper aSwapper( pFrame, true );

    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(pFrame), &aInf );

    SwTwips nStt, nNext;
    SwRect aRect;
    if( static_cast<SwTextCursor&>(aLine).GetCharRect( &aRect, nChar ) )
    {
        if ( bVert )
            pFrame->SwitchHorizontalToVertical( aRect );

        nStt = (aRect.*fnRect->fnGetLeft)();
    }
    else
        nStt = aLine.GetLineStart();

    if( !bCenter )
        return nStt - (pFrame->Frame().*fnRect->fnGetLeft)();

    if( static_cast<SwTextCursor&>(aLine).GetCharRect( &aRect, nChar+1 ) )
    {
        if ( bVert )
            pFrame->SwitchHorizontalToVertical( aRect );

        nNext = (aRect.*fnRect->fnGetLeft)();
    }
    else
        nNext = aLine.GetLineStart();

    return (( nNext + nStt ) / 2 ) - (pFrame->Frame().*fnRect->fnGetLeft)();
}

SwPaM *AddPam( SwPaM *pPam, const SwTextFrame* pTextFrame,
                const sal_Int32 nPos, const sal_Int32 nLen )
{
    if( nLen )
    {
        // It could be the first
        if( pPam->HasMark() )
        {
            // If the new position is right after the current one, then
            // simply extend the Pam
            if( nPos == pPam->GetPoint()->nContent.GetIndex() )
            {
                pPam->GetPoint()->nContent += nLen;
                return pPam;
            }
            pPam = new SwPaM(*pPam, pPam);
        }

        SwIndex &rContent = pPam->GetPoint()->nContent;
        rContent.Assign( const_cast<SwTextNode*>(pTextFrame->GetTextNode()), nPos );
        pPam->SetMark();
        rContent += nLen;
    }
    return pPam;
}

// Accumulates the whitespace at line start and end in the Pam
void SwTextFrameInfo::GetSpaces( SwPaM &rPam, bool bWithLineBreak ) const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(pFrame) );
    SwTextMargin aLine( const_cast<SwTextFrame*>(pFrame), &aInf );
    SwPaM *pPam = &rPam;
    bool bFirstLine = true;
    do {

        if( aLine.GetCurr()->GetLen() )
        {
            sal_Int32 nPos = aLine.GetTextStart();
            // Do NOT include the blanks/tabs from the first line
            // in the selection
            if( !bFirstLine && nPos > aLine.GetStart() )
                pPam = AddPam( pPam, pFrame, aLine.GetStart(),
                                nPos - aLine.GetStart() );

            // Do NOT include the blanks/tabs from the last line
            // in the selection
            if( aLine.GetNext() )
            {
                nPos = aLine.GetTextEnd();

                if( nPos < aLine.GetEnd() )
                {
                    sal_uInt16 nOff = !bWithLineBreak && CH_BREAK ==
                                aLine.GetInfo().GetChar( aLine.GetEnd() - 1 )
                                ? 1 : 0;
                    pPam = AddPam( pPam, pFrame, nPos, aLine.GetEnd() - nPos - nOff );
                }
            }
        }
        bFirstLine = false;
    }
    while( aLine.Next() );
}

// Is there a bullet/symbol etc. at the text position?
// Fonts: CharSet, SYMBOL und DONTKNOW
bool SwTextFrameInfo::IsBullet( sal_Int32 nTextStart ) const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(pFrame) );
    SwTextMargin aLine( const_cast<SwTextFrame*>(pFrame), &aInf );
    aInf.SetIdx( nTextStart );
    return aLine.IsSymbol( nTextStart );
}

// Get first line indent
// The precondition for a positive or negative first line indent:
// All lines (except for the first one) have the same left margin.
// We do not want to be so picky and work with a tolerance of TOLERANCE twips.
SwTwips SwTextFrameInfo::GetFirstIndent() const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(pFrame), &aInf );
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

sal_Int32 SwTextFrameInfo::GetBigIndent( sal_Int32& rFndPos,
                                    const SwTextFrame *pNextFrame ) const
{
    SwTextSizeInfo aInf( const_cast<SwTextFrame*>(pFrame) );
    SwTextCursor aLine( const_cast<SwTextFrame*>(pFrame), &aInf );
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
    rFndPos = aLine.GetCursorOfst( nullptr, aPoint, false );
    if( 1 >= rFndPos )
        return 0;

    // Is on front of a non-space
    const OUString& rText = aInf.GetText();
    sal_Unicode aChar = rText[rFndPos];
    if( CH_TAB == aChar || CH_BREAK == aChar || ' ' == aChar ||
        (( CH_TXTATR_BREAKWORD == aChar || CH_TXTATR_INWORD == aChar ) &&
            aInf.HasHint( rFndPos ) ) )
        return 0;

    // and after a space
    aChar = rText[rFndPos - 1];
    if( CH_TAB != aChar && CH_BREAK != aChar &&
        ( ( CH_TXTATR_BREAKWORD != aChar && CH_TXTATR_INWORD != aChar ) ||
            !aInf.HasHint( rFndPos - 1 ) ) &&
        // More than two Blanks!
        ( ' ' != aChar || ' ' != rText[rFndPos - 2] ) )
        return 0;

    SwRect aRect;
    return aLine.GetCharRect( &aRect, rFndPos )
            ? static_cast<sal_Int32>(aRect.Left() - pFrame->Frame().Left() - pFrame->Prt().Left())
            : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
