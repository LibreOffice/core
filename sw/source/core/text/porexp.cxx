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

#include <viewopt.hxx>
#include <SwPortionHandler.hxx>
#include "inftxt.hxx"
#include "porexp.hxx"

TextFrameIndex SwExpandPortion::GetCursorOfst(const sal_uInt16 nOfst) const
{ return SwLinePortion::GetCursorOfst( nOfst ); }

bool SwExpandPortion::GetExpText( const SwTextSizeInfo&, OUString &rText ) const
{
    rText.clear();
    // Do not do: return 0 != rText.Len();
    // Reason being: empty fields replace CH_TXTATR with an empty string
    return true;
}

void SwExpandPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString(), GetWhichPor() );
}

SwPosSize SwExpandPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwTextSlot aDiffText( &rInf, this, false, false );
    return rInf.GetTextSize();
}

bool SwExpandPortion::Format( SwTextFormatInfo &rInf )
{
    SwTextSlot aDiffText( &rInf, this, true, false );
    TextFrameIndex const nFullLen = rInf.GetLen();

    // As odd as it may seem: the query for GetLen() must return
    // false due to the ExpandPortions _after_ the aDiffText (see SoftHyphs)
    // caused by the SetFull ...
    if( !nFullLen )
    {
        // Do not Init(), because we need height and ascent
        Width(0);
        return false;
    }
    return SwTextPortion::Format( rInf );
}

void SwExpandPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    SwTextSlot aDiffText( &rInf, this, true, true );
    const SwFont aOldFont = *rInf.GetFont();
    if( GetJoinBorderWithPrev() )
        const_cast<SwTextPaintInfo&>(rInf).GetFont()->SetLeftBorder(nullptr);
    if( GetJoinBorderWithNext() )
        const_cast<SwTextPaintInfo&>(rInf).GetFont()->SetRightBorder(nullptr);

    rInf.DrawBackBrush( *this );
    rInf.DrawBorder( *this );

    // Do we have to repaint a post it portion?
    if( rInf.OnWin() && mpNextPortion && !mpNextPortion->Width() )
        mpNextPortion->PrePaint( rInf, this );

    // The contents of field portions is not considered during the
    // calculation of the directions. Therefore we let vcl handle
    // the calculation by removing the BIDI_STRONG_FLAG temporarily.
    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    // ST2
    if ( rInf.GetSmartTags() || rInf.GetGrammarCheckList() )
        rInf.DrawMarkedText( *this, rInf.GetLen(), false,
            nullptr != rInf.GetSmartTags(), nullptr != rInf.GetGrammarCheckList() );
    else
        rInf.DrawText( *this, rInf.GetLen() );

    if( GetJoinBorderWithPrev() || GetJoinBorderWithNext() )
        *const_cast<SwTextPaintInfo&>(rInf).GetFont() = aOldFont;
}

SwLinePortion *SwBlankPortion::Compress() { return this; }

/**
 * If a Line is full of HardBlanks and overflows, we must not generate
 * underflows!
 * Causes problems with Fly
 */
sal_uInt16 SwBlankPortion::MayUnderflow( const SwTextFormatInfo &rInf,
    TextFrameIndex const nIdx, bool bUnderflow)
{
    if( rInf.StopUnderflow() )
        return 0;
    const SwLinePortion *pPos = rInf.GetRoot();
    if( pPos->GetNextPortion() )
        pPos = pPos->GetNextPortion();
    while( pPos && pPos->IsBlankPortion() )
        pPos = pPos->GetNextPortion();
    if( !pPos || !rInf.GetIdx() || ( !pPos->GetLen() && pPos == rInf.GetRoot() ) )
        return 0; // There are just BlankPortions left

    // If a Blank is preceding us, we do not need to trigger underflow
    // If a Blank is succeeding us, we do not need to pass on the underflow
    if (bUnderflow
        && nIdx + TextFrameIndex(1) < TextFrameIndex(rInf.GetText().getLength())
        && CH_BLANK == rInf.GetText()[sal_Int32(nIdx) + 1])
    {
        return 0;
    }
    if( nIdx && !const_cast<SwTextFormatInfo&>(rInf).GetFly() )
    {
        while( pPos && !pPos->IsFlyPortion() )
            pPos = pPos->GetNextPortion();
        if( !pPos )
        {
        // We check to see if there are useful line breaks, blanks or fields etc. left
        // In case there still are some, no underflow
        // If there are Flys, we still allow the underflow
            TextFrameIndex nBlank = nIdx;
            while( --nBlank > rInf.GetLineStart() )
            {
                const sal_Unicode cCh = rInf.GetChar( nBlank );
                if( CH_BLANK == cCh ||
                    (( CH_TXTATR_BREAKWORD == cCh || CH_TXTATR_INWORD == cCh )
                        && rInf.HasHint( nBlank ) ) )
                    break;
            }
            if( nBlank <= rInf.GetLineStart() )
                return 0;
        }
    }
    if (nIdx < TextFrameIndex(2))
        return 1;
    sal_Unicode const cCh(rInf.GetChar(nIdx - TextFrameIndex(1)));
    if (CH_BLANK == cCh)
        return 1;
    if( CH_BREAK == cCh )
        return 0;
    return 2;
}

/**
 * Format End of Line
 */
void SwBlankPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    sal_uInt16 nMay = MayUnderflow( rInf, rInf.GetIdx() - nLineLength, true );
    if( nMay )
    {
        if( nMay > 1 )
        {
            if( rInf.GetLast() == this )
               rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );
            rInf.X( rInf.X() - PrtWidth() );
            rInf.SetIdx( rInf.GetIdx() - GetLen() );
        }
        Truncate();
        rInf.SetUnderflow( this );
        if( rInf.GetLast()->IsKernPortion() )
            rInf.SetUnderflow( rInf.GetLast() );
    }
}

/**
 * Pass on the underflows and trigger them ourselves!
 */
bool SwBlankPortion::Format( SwTextFormatInfo &rInf )
{
    const bool bFull = rInf.IsUnderflow() || SwExpandPortion::Format( rInf );
    if( bFull && MayUnderflow( rInf, rInf.GetIdx(), rInf.IsUnderflow() ) )
    {
        Truncate();
        rInf.SetUnderflow( this );
        if( rInf.GetLast()->IsKernPortion() )
            rInf.SetUnderflow( rInf.GetLast() );
    }
    return bFull;
}

void SwBlankPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( !bMulti ) // No gray background for multiportion brackets
        rInf.DrawViewOpt( *this, PortionType::Blank );
    SwExpandPortion::Paint( rInf );
}

bool SwBlankPortion::GetExpText( const SwTextSizeInfo&, OUString &rText ) const
{
    rText = OUString(cChar);
    return true;
}

void SwBlankPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString( cChar ), GetWhichPor() );
}

SwPostItsPortion::SwPostItsPortion( bool bScrpt )
    : bScript( bScrpt )
{
    nLineLength = TextFrameIndex(1);
    SetWhichPor( PortionType::PostIts );
}

void SwPostItsPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( rInf.OnWin() && Width() )
        rInf.DrawPostIts( IsScript() );
}

sal_uInt16 SwPostItsPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    // Unbelievable: PostIts are always visible
    return rInf.OnWin() ? SwViewOption::GetPostItsWidth( rInf.GetOut() ) : 0;
}

bool SwPostItsPortion::Format( SwTextFormatInfo &rInf )
{
    const bool bRet = SwLinePortion::Format( rInf );
    // PostIts should not have an effect on line height etc.
    SetAscent( 1 );
    Height( 1 );
    return bRet;
}

bool SwPostItsPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsPostIts() )
        rText = " ";
    else
        rText.clear();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
