/*************************************************************************
 *
 *  $RCSfile: portxt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-21 11:28:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <ctype.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>     // CH_TXTATR
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   // ASSERT
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _PORLAY_HXX
#include <porlay.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>    // SwTxtGuess, Zeilenumbruch
#endif
#ifndef _PORGLUE_HXX
#include <porglue.hxx>
#endif
#ifndef _POREXP_HXX
#include <porexp.hxx>   // HyphPortion
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>   // SwKernPortion
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>
#endif

#ifdef DEBUG
const sal_Char *GetLangName( const MSHORT nLang );
#endif

/*************************************************************************
 *                      class SwTxtPortion
 *************************************************************************/



SwTxtPortion::SwTxtPortion( const SwLinePortion &rPortion )
  : SwLinePortion( rPortion )
{
    SetWhichPor( POR_TXT );
}



short SwTxtPortion::GetEscapement() const  { return 0; }

/*************************************************************************
 *                      SwTxtPortion::BreakCut()
 *************************************************************************/



// 4846: IsPunctUnderFlow() loest UnderFlow aus.

sal_Bool SwTxtPortion::IsPunctUnderFlow( const SwTxtSizeInfo &rInf ) const
{
    const xub_Unicode cCh = rInf.GetChar( rInf.GetIdx() );
    //JP 15.08.00: ispunct can't handle characters greater 255 !!! Bug: 77527
    //              This Code must be changed to the new
    //              character classification class!
    return (cCh < 0xff && ispunct( cCh ))
            && '-' != cCh;
}

/*************************************************************************
 *                      SwTxtPortion::BreakCut()
 *************************************************************************/



void SwTxtPortion::BreakCut( SwTxtFormatInfo &rInf, const SwTxtGuess &rGuess )
{
    // Das Wort/Zeichen ist groesser als die Zeile
    // Sonderfall Nr.1: Das Wort ist groesser als die Zeile
    // Wir kappen...
    const KSHORT nLineWidth = rInf.Width() - rInf.X();
    xub_StrLen nLen = xub_StrLen( Max( 1L, long( rGuess.RightPos() + 1 - rInf.GetIdx() ) ) );
    Width( KSHRT_MAX );

    // Wer zu breit ist, den straft das Leben: auf aufwendige
    // Optimierung wird hier verzichtet.
    while( nLen && Width() > nLineWidth )
    {
        DBG_LOOP;
        rInf.SetLen( nLen );
        SetLen( nLen );
        CalcTxtSize( rInf );
        --nLen;
    }

    if( Width() > nLineWidth )
    {
        // Sonderfall Nr.2, Zeichen breiter als Zeile
        // 5057: Am Zeilenanfang wir abgeschnitten, ansonsten
        // rutscht die Portion in die naechste Zeile
        // 6721: !rInf.GetIdx() fuer mehrzeilige Felder.
        if( !rInf.GetIdx() || rInf.GetIdx() == rInf.GetLineStart() )
            Width( nLineWidth );
        else
        {
            Height( 0 );
            Width( 0 );
            SetLen( 0 );
            SetAscent( 0 );
            SetPortion( NULL );  // ????
        }
    }
}

/*************************************************************************
 *                      SwTxtPortion::BreakUnderflow()
 *************************************************************************/



void SwTxtPortion::BreakUnderflow( SwTxtFormatInfo &rInf,
                                   const SwTxtGuess &rGuess,
                                   const sal_Bool bPunct )
{
    // 4846: FtnPortions nehmen das Wort davor mit
    sal_Bool bUnderFlow = bPunct || rGuess.LeftPos() < rInf.GetIdx();
    sal_Bool bFtn = sal_False;
    sal_Bool bNullWidth = sal_False;
    if( !bUnderFlow && rGuess.LeftPos() == rInf.GetIdx() && IsFtnPortion() )
        bUnderFlow = bFtn = sal_True;
    sal_Bool bSoftHyph = sal_False;
    if( !bUnderFlow && rInf.GetIdx() && !rInf.GetLast()->IsFlyPortion() )
    {
        // SonderFall: Durch recycled Portions ist auch folgender
        // Fall denkbar: {old}{ new}
        // Bei SoftHyphens laufen wir ins Underflow, um die deutschen
        // Sondertrennungen zu pruefen.
        const xub_Unicode cCh = rInf.GetChar( rInf.GetIdx() );
        bSoftHyph =  ' ' != cCh && rInf.GetLast()->IsSoftHyphPortion();
        if( bSoftHyph )
            bUnderFlow = sal_True;
        else
        {
            xub_StrLen nIdx = rInf.GetIdx() - 1;
            const xub_Unicode cLastCh = rInf.GetChar( nIdx );
            bUnderFlow = (' ' != cCh && ' ' != cLastCh && CH_BREAK != cLastCh &&
                            ( '-' != cLastCh ||
                            !SwTxtGuess::IsWordEnd( rInf, nIdx ) ) );
            if( bUnderFlow && ( CH_TXTATR_BREAKWORD == cLastCh ||
                CH_TXTATR_INWORD == cLastCh ) && rInf.HasHint(nIdx) )
                bUnderFlow = bNullWidth = !rInf.GetLast()->Width();
        }
    }

    Truncate();

    // Das ganze Wort passt nicht mehr auf die Zeile
    if( !bUnderFlow )
    {
        // Wir rufen das FormatEOL der letzten Portion.
        // Damit kommen HardBlanks, SoftHyphs und Tabs zum Zuge.
        Height( 0 );
        Width( 0 );
        SetLen( 0 );
        SetAscent( 0 );
        SetPortion( NULL );  // ????
        rInf.GetLast()->FormatEOL( rInf );
    }
    else
    {
        // Dieser Teil wird nur betreten, wenn eine Unterlaufsituation
        // bei Attributen aufgetreten ist: in dem Wort, das nicht mehr
        // passt, liegt ein Attributwechsel und damit Portionwechsel vor.
        // Der Umbruch liegt demnach also vor der aktuellen Portion.
        // Wir muessen uns in die Kette mit aufnehmen, weil
        // SwTxtFormatter::UnderFlow() wissen muss, wo wieder neu
        // aufgesetzt werden soll (naemlich vor uns).

        // Wieder mal ein Sonderfall: wenn der Wordbeginn vor unserer Zeile
        // oder genau am Zeilenbeginn ist, dann schneiden wir ab.
        const xub_StrLen nWordStart = rGuess.GetPrevEnd(rInf, rGuess.LeftPos());
        if( !bSoftHyph && (!bFtn || rInf.StopUnderFlow() )
            && nWordStart <= rInf.GetLineStart() )
        {
            if( rInf.GetFly() || rInf.GetLast()->IsFlyPortion()
                || rInf.IsDropInit() ||
                ( rInf.StopUnderFlow() && rInf.GetLast()->IsFlyCntPortion() ) )
            {
                Height( 0 );
                Width( 0 );
                SetLen( 0 );
                SetAscent( 0 );
                SetPortion( NULL );  // ????
                // 3983: {N}ikolaus, Fly rechts drueber haengen
                if( rInf.GetFly() && nWordStart < rInf.GetIdx() )
                    Underflow( rInf );
                else
                    rInf.GetLast()->FormatEOL( rInf );
            }
            else
                BreakCut( rInf, rGuess );
        }
        else
        {
            Height( 0 );
            Width( 0 );
            SetLen( 0 );
            SetAscent( 0 );
            SetPortion( NULL );  // ????
            if ( bNullWidth )
            {
                SwLinePortion *pPor = rInf.GetLast();
                pPor->Truncate();
                rInf.SetUnderFlow( pPor );
            }
            else
                Underflow( rInf );
        }
    }
}

/*************************************************************************
 *                      SwTxtPortion::BreakLine()
 *************************************************************************/



void SwTxtPortion::BreakLine( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess )
{
    // Dieses Wort passt nicht mehr
    // Hyphenator ...
    // Wenn das Wort ueber den Rand hinausragt.

    if( rGuess.HyphWord().is() && IsHyphenate( rInf, rGuess ) )
    {
        if ( rInf.GetFly() )
            rInf.GetRoot()->SetMidHyph( sal_True );
        else
            rInf.GetRoot()->SetEndHyph( sal_True );
        return;
    }

    // Drei Faelle muessen beachtet werden:
    // "012 456"    Guess: "2 456"  I:0, L:2 -> " 456" in die naechste Zeile
    // "{01}2 456"  Guess: "2 456", I:2, L:2 -> " 456" in die naechste Zeile
    // "{012} 456"  Guess: " 456",  I:3, L:3 -> Underflow
    // Wer weiss, was jetzt alles schief geht:
    // old: win090: if( rInf.GetIdx() < rGuess.LeftPos() )

    // Und schon ist es passiert:
    // 4801: "{Drucken}, das" : "{Drucken}" passt "," ist zuviel ...
    if( rInf.X() + rGuess.LeftWidth() <= rInf.Width() )
    {
        if( rInf.GetIdx() <  rGuess.LeftPos() ||
           ( rInf.GetIdx() == rGuess.LeftPos() &&
             rInf.GetIdx() > rInf.GetLineStart() &&
            rGuess.IsWordEnd( rInf, rInf.GetIdx() ) ) )
        {
            // Es wurde noch eine Trennstelle gefunden, oder das Wort
            // soll komplett in die naechste Zeile
            Width( rGuess.LeftWidth() );
            SetLen( rGuess.LeftPos() - rInf.GetIdx() );
// OLD_ITERATOR
//          SetLen( rGuess.LeftPos() - rInf.GetIdx() + 1 );
            return;
        }
    }

    // nTxtOfst ist der Original-GetIdx (s.Kommentar vom _Format())
    // Nur wenn die Portion am Anfang der Zeile steht, dann liegt
    // u.U. ein Sonderfall vor, der im else-Zweig bearbeitet wird.
    // Wenn ein Zeilenumbruch emuliert wird (derzeit nur durch Flys),
    // dann wollen wir aus kosmetischen Gruenden das Wort nicht kappen.

    // Es gibt nur zwei Moeglichkeiten:
    // BreakUnderFlow:  wir formatieren rueckwaerts.
    // BreakCut: wir schneiden gnadenlos ab.


    // 5010: Exp und Tabs
    sal_Bool bFirstPor = sal_True;

    // Nur bei Feldern muss noch einmal geprueft werden,
    // ob wir pFirstPor sind.
    if( InExpGrp() )
    {
        const SwLinePortion *pPor = rInf.GetRoot();
        while( pPor && bFirstPor )
        {
            bFirstPor = 0 == pPor->GetLen();
            pPor = pPor->GetPortion();
        }
    }
    else
        bFirstPor = rInf.GetLineStart() == rInf.GetIdx();

    // 8790: erste Portion hat Vorrang vor bPunct, sonst wird
    // fuer die ein "(xxx)" am Zeilenanfang ein Dauer-Underflow erzeugt.
    // 8790: Flys emulieren Zeilenumbrueche, das bricht uns das Genick
    // Robust:
    if ( !rInf.GetLast() )
        rInf.SetLast( rInf.GetRoot() );

    // During multi-portion formatting the bFirstPor-flag is set only
    // if the multi-portion is the first portion in the line.
    bFirstPor = bFirstPor && !rInf.IsDropInit() &&
                ( !rInf.IsMulti() || rInf.IsFirstMulti() );

    if( ( !bFirstPor || rInf.GetFly() || rInf.GetLast()->IsFlyPortion() ) &&
        ( !rInf.GetLast()->IsBlankPortion() ||  ((SwBlankPortion*)
          rInf.GetLast())->MayUnderFlow( rInf, rInf.GetIdx()-1, sal_True ) ) )
    {
        sal_Bool bPunct = !bFirstPor && IsPunctUnderFlow( rInf ) &&
                      !rInf.GetLast()->IsFlyPortion();
        BreakUnderflow( rInf, rGuess, bPunct );
    }
    else
        BreakCut( rInf, rGuess );
}

/*************************************************************************
 *                      SwTxtPortion::_Format()
 *************************************************************************/
// nTxtOfst ist das Original-GetIdx(). Wenn der Text ersetzt wurde, dann
// sind nIdx und Len an den Ersatzstring angepasst. In BreakLine() muessen
// wir jedoch entscheiden, ob ein Wort komplett in die naechste Zeile soll,
// oder ob ein Sonderfall vorliegt.



sal_Bool SwTxtPortion::_Format( SwTxtFormatInfo &rInf )
{
    // 5744: wenn nur der Trennstrich nicht mehr passt,
    // muss trotzdem das Wort umgebrochen werden, ansonsten return sal_True!
    if( rInf.IsUnderFlow() && rInf.GetSoftHyphPos() )
        if( FormatHyph( rInf ) )
            return sal_True;

    SwTxtGuess aGuess;
    const sal_Bool bFull = !aGuess.Guess( rInf, Height() );
    if( !InExpGrp() || IsFtnPortion() )
        Height( aGuess.Height() );
    if( bFull && !aGuess.GetHangingPortion() )
        BreakLine( rInf, aGuess );
    else
    {
        Width( aGuess.LeftWidth() );
        // Vorsicht !
        if( !InExpGrp() || InFldGrp() )
            SetLen( aGuess.LeftPos() - rInf.GetIdx() + 1 );
#ifdef DBGTXT
        const SwPosSize aSize( GetTxtSize( rInf ) );
        if( aSize.Width() != Width() || aSize.Height() != Height() )
        {
            aDbstream << "Format: diff size: "
                      << aSize.Width()  << "!=" << Width()  << ' '
                      << aSize.Height() << "!=" << Height() << endl;
        }
#endif
        if( aGuess.GetHangingPortion() )
        {
            Insert( aGuess.GetHangingPortion() );
            SwTwips nTmpW = rInf.Width() - rInf.X() - Width();
            if( nTmpW > 0 )
                aGuess.GetHangingPortion()->Width( nTmpW );
            aGuess.GetHangingPortion()->SetAscent( GetAscent() );
            aGuess.ClearHangingPortion();
        }
        else
        {
            short nKern = rInf.GetFont()->CheckKerning();
            if( nKern > 0 && rInf.Width() < rInf.X() + Width() + nKern )
            {
                nKern = rInf.Width() - rInf.X() - Width();
                if( nKern < 0 )
                    nKern = 0;
            }
            if( nKern )
                new SwKernPortion( *this, nKern );
        }
    }
    return bFull;
}

/*************************************************************************
 *                 virtual SwTxtPortion::Format()
 *************************************************************************/



sal_Bool SwTxtPortion::Format( SwTxtFormatInfo &rInf )
{
#ifdef DEBUG
    const XubString aDbgTxt( rInf.GetTxt().Copy( rInf.GetIdx(), rInf.GetLen() ) );
#endif

    if( rInf.X() > rInf.Width() || (!GetLen() && !InExpGrp()) )
    {
        Height( 0 );
        Width( 0 );
        SetLen( 0 );
        SetAscent( 0 );
        SetPortion( NULL );  // ????
        return sal_True;
    }

    ASSERT( rInf.RealWidth() || (rInf.X() == rInf.Width()),
        "SwTxtPortion::Format: missing real width" );
    ASSERT( Height(), "SwTxtPortion::Format: missing height" );

    return _Format( rInf );
}

/*************************************************************************
 *                 virtual SwTxtPortion::FormatEOL()
 *************************************************************************/

// Format end of line
// 5083: Es kann schon manchmal unguenstige Faelle geben...
// "vom {Nikolaus}", Nikolaus bricht um "vom " wird im Blocksatz
// zu "vom" und " ", wobei der Glue expandiert wird, statt in die
// MarginPortion aufzugehen.
// rInf.nIdx steht auf dem naechsten Wort, nIdx-1 ist der letzte
// Buchstabe der Portion.



void SwTxtPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
#ifndef USED
    if( ( !GetPortion() || ( GetPortion()->IsKernPortion() &&
        !GetPortion()->GetPortion() ) ) && GetLen() &&
        rInf.GetIdx() < rInf.GetTxt().Len() &&
        1 < rInf.GetIdx() && ' ' == rInf.GetChar( rInf.GetIdx() - 1 )
#else
    if( !GetPortion() && 1 < GetLen() &&
        rInf.GetIdx() < rInf.GetTxt().Len() &&
        1 < rInf.GetIdx() && ' ' == rInf.GetTxt()[xub_StrLen(rInf.GetIdx()-1)]
        && !rInf.GetFly()
#endif

        && !rInf.GetLast()->IsHolePortion() )
    {
        // Erst uns einstellen und dann Inserten, weil wir ja auch ein
        // SwLineLayout sein koennten.
        KSHORT nBlankSize;
        if(1 == GetLen())
            nBlankSize = Width();
        else
            nBlankSize = rInf.GetTxtSize( ' ' ).Width();
        Width( Width() - nBlankSize );
        rInf.X( rInf.X() - nBlankSize );
        SetLen( GetLen() - 1 );
        SwLinePortion *pHole = new SwHolePortion( *this );
        ( (SwHolePortion *)pHole )->SetBlankWidth( nBlankSize );
        Insert( pHole );
    }
}

/*************************************************************************
 *                 virtual SwTxtPortion::Underflow()
 *************************************************************************/
// Underflow() wird innerhalb des Formatierungsprozesses gerufen,
// wenn diese Portion nicht mehr passt und die Vorgaengerportion
// formatiert werden muss (haeufig bei HardBlank, SoftHyph).
// Wir stellen rInf ein und initialisieren uns.



sal_Bool SwTxtPortion::Underflow( SwTxtFormatInfo &rInf )
{
    Truncate();
    rInf.SetUnderFlow( this );
    return sal_True;
}

/*************************************************************************
 *               virtual SwTxtPortion::GetCrsrOfst()
 *************************************************************************/



xub_StrLen SwTxtPortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    ASSERT( !this, "SwTxtPortion::GetCrsrOfst: don't use this method!" );
    return SwLinePortion::GetCrsrOfst( nOfst );
}

/*************************************************************************
 *               SwTxtPortion::GetCrsrOfst()
 *************************************************************************/



xub_StrLen SwTxtPortion::GetCrsrOfst( const KSHORT nOfst,
                                  SwTxtSizeInfo &rSizeInf ) const
{
    rSizeInf.SetLen( rSizeInf.GetTxtBreak( nOfst, rSizeInf.GetIdx(),
        nLineLength ) - rSizeInf.GetIdx() );
    return rSizeInf.GetLen();
}

/*************************************************************************
 *                virtual SwTxtPortion::GetTxtSize()
 *************************************************************************/
// Das GetTxtSize() geht davon aus, dass die eigene Laenge korrekt ist



SwPosSize SwTxtPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    return rInf.GetTxtSize();
}

/*************************************************************************
 *               virtual SwTxtPortion::Paint()
 *************************************************************************/



void SwTxtPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( GetLen() )
    {
        rInf.DrawBackBrush( *this );
        const SwWrongList *pWrongList = rInf.GetpWrongList();
        if ( pWrongList )
            rInf.DrawWrongText( *this, rInf.GetLen(), sal_False );
        else
            rInf.DrawText( *this, rInf.GetLen(), sal_False );
    }
}

/*************************************************************************
 *              virtual SwTxtPortion::GetExpTxt()
 *************************************************************************/



sal_Bool SwTxtPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    return sal_False;
}

/*************************************************************************
 *        xub_StrLen SwTxtPortion::GetSpaceCnt()
 *              long SwTxtPortion::CalcSpacing()
 * sind fuer den Blocksatz zustaendig und ermitteln die Anzahl der Blanks
 * und den daraus resultierenden zusaetzlichen Zwischenraum
 *************************************************************************/



xub_StrLen SwTxtPortion::GetSpaceCnt( const SwTxtSizeInfo &rInf,
                                      xub_StrLen& rCharCnt ) const
{
    xub_StrLen nCnt = 0;
    xub_StrLen nPos = 0;
    if ( InExpGrp() )
    {
        if( !IsBlankPortion() && !InNumberGrp() )
        {
            // Bei OnWin() wird anstatt eines Leerstrings gern mal ein Blank
            // zurueckgeliefert, das koennen wir hier aber gar nicht gebrauchen
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );
            for ( nPos = 0; nPos < aStr.Len(); ++nPos )
            {
                if( CH_BLANK == aStr.GetChar( nPos ) )
                    ++nCnt;
            }
        }
    }
    else if( !IsDropPortion() )
    {
        xub_StrLen nEndPos = rInf.GetIdx() + GetLen();
        for ( nPos = rInf.GetIdx(); nPos < nEndPos; ++nPos )
        {
            if( CH_BLANK == rInf.GetChar( nPos ) )
                ++nCnt;
        }
        nPos = GetLen();
    }
    rCharCnt += nPos;
    return nCnt;
}



long SwTxtPortion::CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const
{
    xub_StrLen nCnt = 0;
    if ( InExpGrp() )
    {
        if( !IsBlankPortion() && !InNumberGrp() )
        {
            // Bei OnWin() wird anstatt eines Leerstrings gern mal ein Blank
            // zurueckgeliefert, das koennen wir hier aber gar nicht gebrauchen
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );
            if( nSpaceAdd > 0 )
            {
                for ( xub_StrLen nPos = 0; nPos < aStr.Len(); ++nPos )
                {
                    if( CH_BLANK == aStr.GetChar( nPos ) )
                        ++nCnt;
                }
            }
            else
            {
                nSpaceAdd = -nSpaceAdd;
                nCnt = aStr.Len();
            }
        }
    }
    else if( !IsDropPortion() )
    {
        if( nSpaceAdd > 0 )
        {
            xub_StrLen nEndPos = rInf.GetIdx() + GetLen();
            for ( xub_StrLen nPos = rInf.GetIdx(); nPos < nEndPos; ++nPos )
            {
                if( CH_BLANK == rInf.GetChar( nPos ) )
                    ++nCnt;
            }
        }
        else
        {
            nSpaceAdd = -nSpaceAdd;
            nCnt = GetLen();
        }
    }
    return nCnt * nSpaceAdd;
}

/*************************************************************************
 *                      class SwHolePortion
 *************************************************************************/



SwHolePortion::SwHolePortion( const SwTxtPortion &rPor )
    : nBlankWidth( 0 )
{
    SetLen( 1 );
    Height( rPor.Height() );
    SetAscent( rPor.GetAscent() );
    SetWhichPor( POR_HOLE );
}

#ifdef OLDRECYCLE



sal_Bool SwHolePortion::MayRecycle() const { return sal_False; }

#endif



SwLinePortion *SwHolePortion::Compress() { return this; }

/*************************************************************************
 *               virtual SwHolePortion::Paint()
 *************************************************************************/



void SwHolePortion::Paint( const SwTxtPaintInfo &rInf ) const
{
}

/*************************************************************************
 *                 virtual SwHolePortion::Format()
 *************************************************************************/



sal_Bool SwHolePortion::Format( SwTxtFormatInfo &rInf )
{
    return rInf.IsFull() || rInf.X() >= rInf.Width();
}

