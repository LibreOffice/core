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
#include <inftxt.hxx>
#include <porexp.hxx>

sal_Int32 SwExpandPortion::GetCrsrOfst( const sal_uInt16 nOfst ) const
{ return SwLinePortion::GetCrsrOfst( nOfst ); }

bool SwExpandPortion::GetExpTxt( const SwTxtSizeInfo&, OUString &rTxt ) const
{
    rTxt.clear();
    // Do not do: return 0 != rTxt.Len();
    // Reason being: empty fields replace CH_TXTATR with an empty string
    return true;
}

void SwExpandPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString(), GetWhichPor() );
}

SwPosSize SwExpandPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    SwTxtSlot aDiffTxt( &rInf, this, false, false );
    return rInf.GetTxtSize();
}

bool SwExpandPortion::Format( SwTxtFormatInfo &rInf )
{
    SwTxtSlot aDiffTxt( &rInf, this, true, false );
    const sal_Int32 nFullLen = rInf.GetLen();

    // As odd as it may seem: the query for GetLen() must return
    // false due to the ExpandPortions _after_ the aDiffTxt (see SoftHyphs)
    // caused by the SetFull ...
    if( !nFullLen )
    {
        // Do not Init(), because we need height and ascent
        Width(0);
        return false;
    }
    return SwTxtPortion::Format( rInf );
}

void SwExpandPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    SwTxtSlot aDiffTxt( &rInf, this, true, true );
    const SwFont aOldFont = *rInf.GetFont();
    if( GetJoinBorderWithPrev() )
        const_cast<SwTxtPaintInfo&>(rInf).GetFont()->SetLeftBorder(0);
    if( GetJoinBorderWithNext() )
        const_cast<SwTxtPaintInfo&>(rInf).GetFont()->SetRightBorder(0);

    rInf.DrawBackBrush( *this );
    rInf.DrawBorder( *this );

    // Do we have to repaint a post it portion?
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    // The contents of field portions is not considered during the
    // calculation of the directions. Therefore we let vcl handle
    // the calculation by removing the BIDI_STRONG_FLAG temporarily.
    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    // ST2
    if ( rInf.GetSmartTags() || rInf.GetGrammarCheckList() )
        rInf.DrawMarkedText( *this, rInf.GetLen(), false, false,
            0 != rInf.GetSmartTags(), 0 != rInf.GetGrammarCheckList() );
    else
        rInf.DrawText( *this, rInf.GetLen(), false );

    if( GetJoinBorderWithPrev() || GetJoinBorderWithNext() )
        *const_cast<SwTxtPaintInfo&>(rInf).GetFont() = aOldFont;
}

SwLinePortion *SwBlankPortion::Compress() { return this; }

/**
 * If a Line is full of HardBlanks and overflows, we must not generate
 * underflows!
 * Causes problems with Fly
 */
sal_uInt16 SwBlankPortion::MayUnderflow( const SwTxtFormatInfo &rInf,
    sal_Int32 nIdx, bool bUnderflow )
{
    if( rInf.StopUnderflow() )
        return 0;
    const SwLinePortion *pPos = rInf.GetRoot();
    if( pPos->GetPortion() )
        pPos = pPos->GetPortion();
    while( pPos && pPos->IsBlankPortion() )
        pPos = pPos->GetPortion();
    if( !pPos || !rInf.GetIdx() || ( !pPos->GetLen() && pPos == rInf.GetRoot() ) )
        return 0; // There are just BlankPortions left

    // If a Blank is preceding us, we do not need to trigger underflow
    // If a Blank is succeeding us, we do not need to pass on the underflow
    if (bUnderflow && nIdx + 1 < rInf.GetTxt().getLength() && CH_BLANK == rInf.GetTxt()[nIdx + 1])
        return 0;
    if( nIdx && !const_cast<SwTxtFormatInfo&>(rInf).GetFly() )
    {
        while( pPos && !pPos->IsFlyPortion() )
            pPos = pPos->GetPortion();
        if( !pPos )
        {
        // We check to see if there are useful line breaks, blanks or fields etc. left
        // In case there still are some, no underflow
        // If there are Flys, we still allow the underflow
            sal_Int32 nBlank = nIdx;
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
    sal_Unicode cCh;
    if( nIdx < 2 || CH_BLANK == (cCh = rInf.GetChar( nIdx - 1 )) )
        return 1;
    if( CH_BREAK == cCh )
        return 0;
    return 2;
}

/**
 * Format End of Line
 */
void SwBlankPortion::FormatEOL( SwTxtFormatInfo &rInf )
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
bool SwBlankPortion::Format( SwTxtFormatInfo &rInf )
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

void SwBlankPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( !bMulti ) // No gray background for multiportion brackets
        rInf.DrawViewOpt( *this, POR_BLANK );
    SwExpandPortion::Paint( rInf );
}

bool SwBlankPortion::GetExpTxt( const SwTxtSizeInfo&, OUString &rTxt ) const
{
    rTxt = OUString(cChar);
    return true;
}

void SwBlankPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString( cChar ), GetWhichPor() );
}

SwPostItsPortion::SwPostItsPortion( bool bScrpt )
    : bScript( bScrpt )
{
    nLineLength = 1;
    SetWhichPor( POR_POSTITS );
}

void SwPostItsPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( rInf.OnWin() && Width() )
        rInf.DrawPostIts( *this, IsScript() );
}

sal_uInt16 SwPostItsPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Unbelievable: PostIts are always visible
    return rInf.OnWin() ? SwViewOption::GetPostItsWidth( rInf.GetOut() ) : 0;
}

bool SwPostItsPortion::Format( SwTxtFormatInfo &rInf )
{
    const bool bRet = SwLinePortion::Format( rInf );
    // PostIts should not have an effect on line height etc.
    SetAscent( 1 );
    Height( 1 );
    return bRet;
}

bool SwPostItsPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsPostIts() )
        rTxt = " ";
    else
        rTxt.clear();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
