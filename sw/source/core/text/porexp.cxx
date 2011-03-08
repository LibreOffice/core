/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
                const xub_Unicode cCh = rInf.GetChar( nBlank );
                if( CH_BLANK == cCh ||
                    (( CH_TXTATR_BREAKWORD == cCh || CH_TXTATR_INWORD == cCh )
                        && rInf.HasHint( nBlank ) ) )
                    break;
            }
            if( nBlank <= rInf.GetLineStart() )
                return 0;
        }
    }
    xub_Unicode cCh;
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
    String aString( cChar );
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
