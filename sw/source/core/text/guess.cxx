/*************************************************************************
 *
 *  $RCSfile: guess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 12:08:48 $
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

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   // ASSERTs
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>   // SwTxtSizeInfo, SwTxtFormatInfo
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_BREAKTYPE_HPP_
#include <com/sun/star/text/BreakType.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::linguistic2;

inline sal_Bool IsDelim( const xub_Unicode cCh )
{
    return ' ' == cCh;
}

/*************************************************************************
 *                      SwTxtGuess::IsWordEnd
 *************************************************************************/


sal_Bool SwTxtGuess::IsWordEnd( const SwTxtSizeInfo &rInf, const xub_StrLen nPos )
{
    const xub_StrLen nEnd = rInf.GetIdx() + rInf.GetLen();

    ASSERT( nPos <= nEnd, "WordEnd hinter Text?" );

    // Das letzte Zeichen gilt immer als Wortende
    if ( nPos >= nEnd )
        return sal_True;

    const XubString& rTxt = rInf.GetTxt();
    xub_Unicode aChr = rTxt.GetChar( nPos );
    xub_Unicode aChr1 = rTxt.GetChar( nPos + 1 );

    // Bindestriche sind potentielle Wortenden mit folgenden Ausnahmen:
    // es folgt ein weiterer Bindestrich
    // es folgt Textmasse, vor dem Bindestrich sind nur Bindestriche und
    // davor ein Blank ...
    if( '-' == aChr )
    {
        if ( IsDelim( aChr1 ) )
            return sal_True;
        if ( !nPos || '-' == aChr1 )
            return sal_False;
        for ( xub_StrLen nCnt = nPos; nCnt && '-' == aChr; )
            aChr = rTxt.GetChar( --nCnt );
        return ( !IsDelim( aChr ) && '-' != aChr );
    }

    // sal_True, wenn nPos kein Blank ist, aber der darauffolgende
    if( !IsDelim( aChr ) && IsDelim( aChr1 ) )
        return sal_True;
    else
        return sal_False;
}


/*************************************************************************
 *                      _GetFwdWordEnd()
 *
 * Es wird die letzte Position im Wort zurueckgeliefert:
 * Aus "123 456" wird "123" nicht "123 " !!!
 * Sonderfaelle bei "-"
 *************************************************************************/

xub_StrLen _GetFwdWordEnd( const XubString& rTxt, xub_StrLen nFound,
                           const xub_StrLen nMaxPos )
{
    if( nFound == nMaxPos )
        return nMaxPos;

    // Suche von links nach rechts
    // Die Spaces vor einem Wort gehoeren dazu:
    for( ++nFound; nFound < nMaxPos && IsDelim( rTxt.GetChar( nFound ) ); ++nFound )
        ;

    if ( nFound == nMaxPos )
        return nMaxPos;

    // Sonderfall: Bindestrich
    if ( '-' == rTxt.GetChar( nFound ) )
    {
        // Mal gucken, was vor den Bindestrichen los ist
        for ( ; nFound && '-' == rTxt.GetChar( nFound ); --nFound )
            ;
        // Ist vor uns ein Wortteil oder Nichts?
        const sal_Bool bPartOfWord = nFound && !IsDelim( rTxt.GetChar( nFound ) );
        // Und wieder hinter die Bindestriche
        for ( ++nFound; nFound && '-' == rTxt.GetChar( nFound ); ++nFound )
            ;
        if ( nFound > nMaxPos )
            return nMaxPos;
        if ( bPartOfWord || IsDelim( rTxt.GetChar( nFound ) ) )
            return nFound - 1;
    }

    // Beim naechsten Space ist Schluss
    for( ; nFound < nMaxPos; ++nFound )
    {
         if( IsDelim( rTxt.GetChar( nFound ) ) )
            return nFound - 1;
         if( '-' == rTxt.GetChar( nFound ) )
         {
            while ( '-' == rTxt.GetChar( nFound ) )
                ++nFound;
            if ( nFound > nMaxPos )
                return nMaxPos;
            else
                return nFound - 1;
        }
    }
    return nFound;
}

/*************************************************************************
 *                      _GetBwdWordEnd()
 *
 * Es wird die letzte Position im Wort zurueckgeliefert:
 * Aus "123 456" wird "123" nicht "123 " !!!
 * Sonderfaelle bei "-"
 *************************************************************************/

xub_StrLen _GetBwdWordEnd( const XubString& rTxt, xub_StrLen nFound,
                           const xub_StrLen nMin )
{
    const xub_StrLen nStart = nFound;
    if( !nFound )
        return 0;

    // Suche von rechts nach links
    // Bloede Striche
    for( ; nFound && '-' == rTxt.GetChar( nFound ); --nFound )
        ;

    // Bis zum Anfang der Textmasse.
    for( ; nFound && !IsDelim( rTxt.GetChar( nFound ) ); --nFound )
    {
        if( '-' == rTxt.GetChar(nFound) && !ispunct( rTxt.GetChar(nFound+1) ) )
            break;
    }

    if ( '-' == rTxt.GetChar( nFound ) )
    {
        xub_StrLen nOldFound = nFound;
        // Mal gucken, was vor den Bindestrichen los ist
        for ( ; nFound && '-' == rTxt.GetChar( nFound ); --nFound )
            ;
        // Ist vor uns ein Wortteil?
        if( nFound && !IsDelim( rTxt.GetChar( nFound ) ) )
            return nOldFound;
    }

    // Die Spaces hinter einem Wort gehoeren nicht dazu:
    for( ; nFound > nMin; --nFound )
    {
        if( !IsDelim( rTxt.GetChar( nFound ) ) )
            break;
    }
    if( nMin == nFound )
    {
        if( IsDelim( rTxt.GetChar( nFound ) ) )
        {
            while( ++nFound < nStart && IsDelim( rTxt.GetChar( nFound ) ) )
                ;
            --nFound;
        }
        else if( nMin )
            --nFound;
    }

    return nFound;
}

/*************************************************************************
 *                      SwTxtGuess::GetWordEnd
 *************************************************************************/

// Liefert die Wortgrenze je nach Suchrichtung zurueck.
// Wortdelimitter sind (nach dem letzten Projekt-Parteitag) nur
// noch Blanks.
// Fuehrende Blanks gehoeren zum Wort, aus "XXX   YYY" wird
// "XXX" und "   YYY".

xub_StrLen SwTxtGuess::GetWordEnd( const SwTxtFormatInfo &rInf,
                                const xub_StrLen nPos, const sal_Bool bFwd ) const
{
    const xub_StrLen nMaxPos = rInf.GetIdx() + rInf.GetLen() - 1;
    const xub_StrLen nIdx = nPos < nMaxPos ? nPos : nMaxPos;
    return  bFwd ? _GetFwdWordEnd( rInf.GetTxt(), nIdx, nMaxPos )
                 : _GetBwdWordEnd( rInf.GetTxt(), nIdx, rInf.GetLineStart() );
}

/*************************************************************************
 *                      SwTxtGuess::Guess
 *
 * Nach Guess muessen folgende Verhaeltnisse vorliegen:
 * Bsp: "XXX   YYY", Umbruch beim 2.Space
 * GetLeftPos() liefert das Ende des noch passenden Wortes: "XXX", 3
 * GetRightPos() liefert den Beginn des nicht mehr passenden Wortes: " YYY"
 * Wenn das Wort ganz passte, dann ist GetLeftPos() == GetRightPos().
 * Etwas Verwirrung stiftet der Unterschied zwischen Position und Laenge:
 * GetWord() arbeitet ausschliesslich mit String-Positionen, waehrend
 * die Breiten per GetTxtSize() und damit ueber Laengen ermittelt werden.
 * Die Laenge ist (nach Abzug des Offsets) immer genau um 1 groesser
 * als die Position.
 * Liefert zurueck, ob es noch passte...
 *************************************************************************/

sal_Bool SwTxtGuess::Guess( const SwTxtFormatInfo &rInf, const KSHORT nPorHeight )
{
    nLeftPos = nRightPos = rInf.GetIdx();
    // Leere Strings sind immer 0
    if( !rInf.GetLen() || !rInf.GetTxt().Len() )
    {
        nHeight = rInf.GetTxtHeight();
        return sal_False;
    }
    ASSERT( rInf.GetIdx() < rInf.GetTxt().Len(),
            "+SwTxtGuess::Guess: invalid SwTxtFormatInfo" );

    nHeight = nPorHeight;

    KSHORT nLineWidth = rInf.Width() - rInf.X();
    const xub_StrLen nMaxLen = Min( xub_StrLen(rInf.GetTxt().Len() - rInf.GetIdx()),
                                rInf.GetLen() );
    // Sonderfall: Zeichen breiter als Zeile
    if( !nMaxLen || !nLineWidth )
    {
        nHeight = rInf.GetTxtHeight();
        return sal_False;
    }

    if( !nHeight )
    {
        ASSERT( nHeight, "+SwTxtGuess::Guess: no height" );
        nHeight = rInf.GetTxtHeight();
        if( !nHeight )
            nHeight = 1;
    }

    KSHORT nItalic = 0;
    if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
    {
#ifdef DEBUG
        static MSHORT nDiv = 12;
        nItalic = nHeight / nDiv;
#else
#ifdef MAC
        nItalic = nHeight / 4;
#else
        nItalic = nHeight / 12;
#endif
#endif
        if( nItalic >= nLineWidth )
        {
            nLeftWidth = nRightWidth = nItalic;
            nLeftPos = nRightPos = rInf.GetIdx();
            return sal_False;
        }
        else
            nLineWidth -= nItalic;
    }

    // Kein GetTextBreak, wenn vermutlich alles passt
    if ( long ( nLineWidth ) * 2 > long ( nMaxLen ) * nHeight )
    {
        nLeftWidth = rInf.GetTxtSize( rInf.GetIdx(), nMaxLen ).Width();

        if ( nLeftWidth <= nLineWidth )
        {
            // Die Vermutung hat sich bewahrheitet
            nLeftPos = nRightPos = rInf.GetIdx() + nMaxLen - 1;
            nHeight = rInf.GetTxtHeight();
            if( nItalic && ( nLeftPos + 1 ) >= rInf.GetTxt().Len() )
                nLeftWidth += nItalic;
            nRightWidth = nLeftWidth;
            return sal_True;
        }
#ifdef DEBUG
        nRightWidth = nLeftWidth; // nur zum Breakpoint setzen,
                                  // Vermutung schlug fehl, teuer!
#endif
    }

    nHeight = rInf.GetTxtHeight();

    sal_Bool bHyph = rInf.IsHyphenate() && !rInf.IsHyphForbud();
    xub_StrLen nHyphPos = 0;

    if( bHyph )
        nRightPos = rInf.GetTxtBreak( nLineWidth, rInf.GetIdx(), nMaxLen,
                                       nHyphPos );
    else
        nRightPos = rInf.GetTxtBreak( nLineWidth, rInf.GetIdx(), nMaxLen );

    if( nRightPos > rInf.GetIdx() + nMaxLen )
    {
        // passt noch auf die Zeile
        nLeftPos = nRightPos = rInf.GetIdx() + nMaxLen - 1;
        if( bHyph )
            nHyphPos = nLeftPos;
        nLeftWidth = nRightWidth =
            rInf.GetTxtSize( rInf.GetIdx(), nMaxLen ).Width();
        // Der folgende Vergleich sollte eigenlich immer sal_True ergeben, sonst
        // hat es wohl bei GetTxtBreak einen Pixel-Rundungsfehler gegeben...
        if ( nLeftWidth <= nLineWidth )
        {
            if( nItalic && ( nLeftPos + 1 ) >= rInf.GetTxt().Len() )
            {
                nLeftWidth += nItalic;
                nRightWidth = nLeftWidth;
            }
            return sal_True;
        }
    }

    if( IsDelim( rInf.GetChar( nRightPos ) ) )
    {
        nLeftPos = nRightPos;
        while( nRightPos && IsDelim( rInf.GetChar( --nRightPos ) ) )
            --nLeftPos;
    }
    else
    {
        nLeftPos = nRightPos;
        if( pBreakIt->xBreak.is() )
        {
            LineBreakHyphenationOptions aHyphOpt;
            Reference< XHyphenator >  xHyph;
            if( bHyph )
            {
                xHyph = ::GetHyphenator();
                aHyphOpt = LineBreakHyphenationOptions( xHyph, nHyphPos );
            }
            LineBreakResults aResult = pBreakIt->xBreak->getLineBreak( rInf.GetTxt(),
                nRightPos, pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
                rInf.GetIdx(), aHyphOpt, LineBreakUserOptions() );
            nLeftPos = aResult.breakIndex;
            if( nLeftPos == STRING_LEN )
                nLeftPos = 0;
            bHyph = aResult.breakType == BreakType::HYPHENATION;
            if( bHyph )
                xHyphWord = aResult.rHyphenatedWord;
            else
                xHyphWord = NULL;

            if( nLeftPos )
            {
                xub_StrLen nX = nLeftPos;
                while( nX && IsDelim( rInf.GetChar( --nX ) ) )
                    nLeftPos = nX;
            }
        }
    }

    if ( nLeftPos < rInf.GetIdx() )
        nLeftPos = rInf.GetIdx();

    if( !SwTxtGuess::IsWordEnd( rInf, nRightPos ) )
        nRightPos = GetNextEnd( rInf, nRightPos );

    nLeftWidth =
    rInf.GetTxtSize( rInf.GetIdx(), nLeftPos - rInf.GetIdx() ).Width();

    bHyph = bHyph && ( nHyphPos > nLeftPos );

    // Sicher ist sicher, robust gg. Rundungfehler in GetTxtBreak ...
    if ( nLeftWidth > nLineWidth )
    {
        rInf.GetTxtSize( rInf.GetIdx(), nLeftPos - rInf.GetIdx() + 1 ).Width();
        bHyph = sal_False;
        nLeftPos = GetPrevEnd( rInf, nLeftPos );
        if ( nLeftPos < rInf.GetIdx() )
            nLeftPos = rInf.GetIdx();
        nRightPos = GetNextEnd( rInf, nLeftPos );
        nLeftWidth =
            rInf.GetTxtSize( rInf.GetIdx(), nLeftPos - rInf.GetIdx() + 1 ).Width();
    }
    nLeftWidth += nItalic;
    nRightWidth = nLeftWidth;
    return sal_False;
}


