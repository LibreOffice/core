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

#include <viewopt.hxx>  // SwViewOptions
#include <SwPortionHandler.hxx>
#include <inftxt.hxx>
#include <porexp.hxx>

/*************************************************************************
 *                      class SwExpandPortion
 *************************************************************************/

xub_StrLen SwExpandPortion::GetCrsrOfst( const MSHORT nOfst ) const
{ return SwLinePortion::GetCrsrOfst( nOfst ); }

/*************************************************************************
 *              virtual SwExpandPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwExpandPortion::GetExpTxt( const SwTxtSizeInfo&,
                                 XubString &rTxt ) const
{
    rTxt.Erase();
    // Nicht etwa: return 0 != rTxt.Len();
    // Weil: leere Felder ersetzen CH_TXTATR gegen einen Leerstring
    return sal_True;
}

/*************************************************************************
 *              virtual SwExpandPortion::HandlePortion()
 *************************************************************************/

void SwExpandPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString;
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

// 5010: Exp und Tabs

sal_Bool SwExpandPortion::Format( SwTxtFormatInfo &rInf )
{
    SwTxtSlot aDiffTxt( &rInf, this, true, false );
    const xub_StrLen nFullLen = rInf.GetLen();

    // So komisch es aussieht, die Abfrage auf GetLen() muss wegen der
    // ExpandPortions _hinter_ aDiffTxt (vgl. SoftHyphs)
    // sal_False returnen wegen SetFull ...
    if( !nFullLen )
    {
        // nicht Init(), weil wir Hoehe und Ascent brauchen
        Width(0);
        return sal_False;
    }
    return SwTxtPortion::Format( rInf );
}

/*************************************************************************
 *              virtual SwExpandPortion::Paint()
 *************************************************************************/

void SwExpandPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    SwTxtSlot aDiffTxt( &rInf, this, true, true );

    rInf.DrawBackBrush( *this );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    // The contents of field portions is not considered during the
    // calculation of the directions. Therefore we let vcl handle
    // the calculation by removing the BIDI_STRONG_FLAG temporarily.
    SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
    aLayoutModeModifier.SetAuto();

    // ST2
    if ( rInf.GetSmartTags() || rInf.GetGrammarCheckList() )
        rInf.DrawMarkedText( *this, rInf.GetLen(), sal_False, sal_False,
            0 != rInf.GetSmartTags(), 0 != rInf.GetGrammarCheckList() );
    else
        rInf.DrawText( *this, rInf.GetLen(), sal_False );
}

/*************************************************************************
 *                      class SwBlankPortion
 *************************************************************************/

SwLinePortion *SwBlankPortion::Compress() { return this; }

/*************************************************************************
 *                 SwBlankPortion::MayUnderFlow()
 *************************************************************************/

// 5497: Es gibt schon Gemeinheiten auf der Welt...
// Wenn eine Zeile voll mit HardBlanks ist und diese ueberlaeuft,
// dann duerfen keine Underflows generiert werden!
// Komplikationen bei Flys...

MSHORT SwBlankPortion::MayUnderFlow( const SwTxtFormatInfo &rInf,
    xub_StrLen nIdx, sal_Bool bUnderFlow ) const
{
    if( rInf.StopUnderFlow() )
        return 0;
    const SwLinePortion *pPos = rInf.GetRoot();
    if( pPos->GetPortion() )
        pPos = pPos->GetPortion();
    while( pPos && pPos->IsBlankPortion() )
        pPos = pPos->GetPortion();
    if( !pPos || !rInf.GetIdx() || ( !pPos->GetLen() && pPos == rInf.GetRoot() ) )
        return 0; // Nur noch BlankPortions unterwegs
    // Wenn vor uns ein Blank ist, brauchen wir kein Underflow ausloesen,
    // wenn hinter uns ein Blank ist, brauchen wir kein Underflow weiterreichen
    if( bUnderFlow && CH_BLANK == rInf.GetTxt().GetChar( nIdx + 1) )
        return 0;
    if( nIdx && !((SwTxtFormatInfo&)rInf).GetFly() )
    {
        while( pPos && !pPos->IsFlyPortion() )
            pPos = pPos->GetPortion();
        if( !pPos )
        {
        //Hier wird ueberprueft, ob es in dieser Zeile noch sinnvolle Umbrueche
        //gibt, Blanks oder Felder etc., wenn nicht, kein Underflow.
        //Wenn Flys im Spiel sind, lassen wir das Underflow trotzdem zu.
            xub_StrLen nBlank = nIdx;
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
// Format end of Line

void SwBlankPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    MSHORT nMay = MayUnderFlow( rInf, rInf.GetIdx() - nLineLength, sal_True );
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

// 7771: UnderFlows weiterreichen und selbst ausloesen!
sal_Bool SwBlankPortion::Format( SwTxtFormatInfo &rInf )
{
    const sal_Bool bFull = rInf.IsUnderFlow() || SwExpandPortion::Format( rInf );
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
    if( !bMulti ) // No gray background for multiportion brackets
        rInf.DrawViewOpt( *this, POR_BLANK );
    SwExpandPortion::Paint( rInf );
}

/*************************************************************************
 *              virtual SwBlankPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwBlankPortion::GetExpTxt( const SwTxtSizeInfo&, XubString &rTxt ) const
{
    rTxt = cChar;
    return sal_True;
}

/*************************************************************************
 *              virtual SwBlankPortion::HandlePortion()
 *************************************************************************/

void SwBlankPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rtl::OUString aString( cChar );
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

/*************************************************************************
 *                      class SwPostItsPortion
 *************************************************************************/

SwPostItsPortion::SwPostItsPortion( sal_Bool bScrpt )
    : nViewWidth(0), bScript( bScrpt )
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
    // Nicht zu fassen: PostIts sind immer zu sehen.
    return rInf.OnWin() ?
                (KSHORT)rInf.GetOpt().GetPostItsWidth( rInf.GetOut() ) : 0;
}

/*************************************************************************
 *                 virtual SwPostItsPortion::Format()
 *************************************************************************/

sal_Bool SwPostItsPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Bool bRet = SwLinePortion::Format( rInf );
    // 32749: PostIts sollen keine Auswirkung auf Zeilenhoehe etc. haben
    SetAscent( 1 );
    Height( 1 );
    return bRet;
}

/*************************************************************************
 *              virtual SwPostItsPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwPostItsPortion::GetExpTxt( const SwTxtSizeInfo &rInf,
                                  XubString &rTxt ) const
{
    if( rInf.OnWin() && rInf.GetOpt().IsPostIts() )
        rTxt = ' ';
    else
        rTxt.Erase();
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
