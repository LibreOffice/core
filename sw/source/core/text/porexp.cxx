/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <viewopt.hxx>
#include <SwPortionHandler.hxx>
#include <inftxt.hxx>
#include <porexp.hxx>

/*************************************************************************
 *                      class SwExpandPortion
 *************************************************************************/

sal_Int32 SwExpandPortion::GetCrsrOfst( const MSHORT nOfst ) const
{ return SwLinePortion::GetCrsrOfst( nOfst ); }

/*************************************************************************
 *              virtual SwExpandPortion::GetExpTxt()
 *************************************************************************/

bool SwExpandPortion::GetExpTxt( const SwTxtSizeInfo&, OUString &rTxt ) const
{
    rTxt = OUString();
    
    
    return true;
}

/*************************************************************************
 *              virtual SwExpandPortion::HandlePortion()
 *************************************************************************/

void SwExpandPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    OUString aString;
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

/*************************************************************************
 *              virtual SwExpandPortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwExpandPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    SwTxtSlot aDiffTxt( &rInf, this, false, false );
    return rInf.GetTxtSize();
}

/*************************************************************************
 *                 virtual SwExpandPortion::Format()
 *************************************************************************/



bool SwExpandPortion::Format( SwTxtFormatInfo &rInf )
{
    SwTxtSlot aDiffTxt( &rInf, this, true, false );
    const sal_Int32 nFullLen = rInf.GetLen();

    
    
    
    if( !nFullLen )
    {
        
        Width(0);
        return false;
    }
    return SwTxtPortion::Format( rInf );
}

/*************************************************************************
 *              virtual SwExpandPortion::Paint()
 *************************************************************************/

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

    
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    
    
    
    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    
    if ( rInf.GetSmartTags() || rInf.GetGrammarCheckList() )
        rInf.DrawMarkedText( *this, rInf.GetLen(), false, false,
            0 != rInf.GetSmartTags(), 0 != rInf.GetGrammarCheckList() );
    else
        rInf.DrawText( *this, rInf.GetLen(), false );

    if( GetJoinBorderWithPrev() || GetJoinBorderWithNext() )
        *const_cast<SwTxtPaintInfo&>(rInf).GetFont() = aOldFont;
}

/*************************************************************************
 *                      class SwBlankPortion
 *************************************************************************/

SwLinePortion *SwBlankPortion::Compress() { return this; }

/*************************************************************************
 *                 SwBlankPortion::MayUnderFlow()
 *************************************************************************/






MSHORT SwBlankPortion::MayUnderFlow( const SwTxtFormatInfo &rInf,
    sal_Int32 nIdx, bool bUnderFlow ) const
{
    if( rInf.StopUnderFlow() )
        return 0;
    const SwLinePortion *pPos = rInf.GetRoot();
    if( pPos->GetPortion() )
        pPos = pPos->GetPortion();
    while( pPos && pPos->IsBlankPortion() )
        pPos = pPos->GetPortion();
    if( !pPos || !rInf.GetIdx() || ( !pPos->GetLen() && pPos == rInf.GetRoot() ) )
        return 0; 
    
    
    if (bUnderFlow && nIdx + 1 < rInf.GetTxt().getLength() && CH_BLANK == rInf.GetTxt()[nIdx + 1])
        return 0;
    if( nIdx && !((SwTxtFormatInfo&)rInf).GetFly() )
    {
        while( pPos && !pPos->IsFlyPortion() )
            pPos = pPos->GetPortion();
        if( !pPos )
        {
        
        
        
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

/*************************************************************************
 *                 virtual SwBlankPortion::FormatEOL()
 *************************************************************************/


void SwBlankPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    MSHORT nMay = MayUnderFlow( rInf, rInf.GetIdx() - nLineLength, true );
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
        rInf.SetUnderFlow( this );
        if( rInf.GetLast()->IsKernPortion() )
            rInf.SetUnderFlow( rInf.GetLast() );
    }
}

/*************************************************************************
 *                 virtual SwBlankPortion::Format()
 *************************************************************************/


bool SwBlankPortion::Format( SwTxtFormatInfo &rInf )
{
    const bool bFull = rInf.IsUnderFlow() || SwExpandPortion::Format( rInf );
    if( bFull && MayUnderFlow( rInf, rInf.GetIdx(), rInf.IsUnderFlow() ) )
    {
        Truncate();
        rInf.SetUnderFlow( this );
        if( rInf.GetLast()->IsKernPortion() )
            rInf.SetUnderFlow( rInf.GetLast() );
    }
    return bFull;
}

/*************************************************************************
 *                 virtual SwBlankPortion::Paint()
 *************************************************************************/

void SwBlankPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( !bMulti ) 
        rInf.DrawViewOpt( *this, POR_BLANK );
    SwExpandPortion::Paint( rInf );
}

/*************************************************************************
 *              virtual SwBlankPortion::GetExpTxt()
 *************************************************************************/

bool SwBlankPortion::GetExpTxt( const SwTxtSizeInfo&, OUString &rTxt ) const
{
    rTxt = OUString(cChar);
    return true;
}

/*************************************************************************
 *              virtual SwBlankPortion::HandlePortion()
 *************************************************************************/

void SwBlankPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    OUString aString( cChar );
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

/*************************************************************************
 *                      class SwPostItsPortion
 *************************************************************************/

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

KSHORT SwPostItsPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    
    return rInf.OnWin() ?
                (KSHORT)rInf.GetOpt().GetPostItsWidth( rInf.GetOut() ) : 0;
}

/*************************************************************************
 *                 virtual SwPostItsPortion::Format()
 *************************************************************************/

bool SwPostItsPortion::Format( SwTxtFormatInfo &rInf )
{
    const bool bRet = SwLinePortion::Format( rInf );
    
    SetAscent( 1 );
    Height( 1 );
    return bRet;
}

/*************************************************************************
 *              virtual SwPostItsPortion::GetExpTxt()
 *************************************************************************/

bool SwPostItsPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsPostIts() )
        rTxt = OUString(' ');
    else
        rTxt = OUString();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
