/*************************************************************************
 *
 *  $RCSfile: portxt.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:55:18 $
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

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>     // CH_TXTATR
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   // ASSERT
#endif
#ifndef _SW_PORTIONHANDLER_HXX
#include <SwPortionHandler.hxx>
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
#ifndef _PORFLD_HXX
#include <porfld.hxx>       // SwFldPortion
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
const sal_Char *GetLangName( const MSHORT nLang );
#endif

using namespace ::com::sun::star::i18n::ScriptType;

/*************************************************************************
 *                          lcl_AddSpace
 * Returns for how many characters an extra space has to be added
 * (for justified alignment).
 *************************************************************************/

USHORT lcl_AddSpace( const SwTxtSizeInfo &rInf, const XubString* pStr,
                     const SwLinePortion& rPor )
{
    xub_StrLen nPos, nEnd;
    const SwScriptInfo* pSI = 0;

    if ( pStr )
    {
        // passing a string means we are inside a field
        nPos = 0;
        nEnd = pStr->Len();
    }
    else
    {
        nPos = rInf.GetIdx();
        nEnd = rInf.GetIdx() + rPor.GetLen();
        pStr = &rInf.GetTxt();
        pSI = &((SwParaPortion*)rInf.GetParaPortion())->GetScriptInfo();
    }

    USHORT nCnt = 0;
    BYTE nScript = 0;

    // If portion consists of Asian characters and language is not
    // Korean, we add extra space to each character.
    // first we get the script type
    if ( pSI )
        nScript = pSI->ScriptType( nPos );
    else if ( pBreakIt->xBreak.is() )
        nScript = (BYTE)pBreakIt->xBreak->getScriptType( *pStr, nPos );

    // Note: rInf.GetIdx() can differ from nPos,
    // e.g., when rPor is a field portion. nPos referes to the string passed
    // to the function, rInf.GetIdx() referes to the original string.

    // We try to find out which justification mode is required. This is done by
    // evaluating the script type and the language attribute set for this portion

    // Asian Justification: Each character get some extra space
    if ( nEnd > nPos && ASIAN == nScript )
    {
        LanguageType aLang =
            rInf.GetTxtFrm()->GetTxtNode()->GetLang( rInf.GetIdx(), 1, nScript );

        if ( LANGUAGE_KOREAN != aLang && LANGUAGE_KOREAN_JOHAB != aLang )
        {
            const SwLinePortion* pPor = rPor.GetPortion();
            if ( pPor && pPor->IsKernPortion() )
                pPor = pPor->GetPortion();

            nCnt += nEnd - nPos;

            if ( ! pPor || pPor->IsHolePortion() || pPor->InFixMargGrp() )
                --nCnt;

            return nCnt;
        }
    }

    // Thai Justification: Each character cell gets some extra space
    if ( nEnd > nPos && COMPLEX == nScript )
    {
        LanguageType aLang =
            rInf.GetTxtFrm()->GetTxtNode()->GetLang( rInf.GetIdx(), 1, nScript );

        if ( LANGUAGE_THAI == aLang )
        {
            nCnt = SwScriptInfo::ThaiJustify( *pStr, 0, 0, nPos, nEnd - nPos );

            const SwLinePortion* pPor = rPor.GetPortion();
            if ( pPor && pPor->IsKernPortion() )
                pPor = pPor->GetPortion();

            if ( nCnt && ( ! pPor || pPor->IsHolePortion() || pPor->InFixMargGrp() ) )
                --nCnt;

            return nCnt;
        }
    }

#ifdef BIDI
    // Kashida Justification: Insert Kashidas
    if ( nEnd > nPos && pSI && COMPLEX == nScript )
    {
        LanguageType aLang =
            rInf.GetTxtFrm()->GetTxtNode()->GetLang( rInf.GetIdx(), 1, nScript );

        if ( SwScriptInfo::IsArabicLanguage( aLang ) )
            return pSI->KashidaJustify( 0, 0, nPos, nEnd - nPos );
    }
#endif

    // Here starts the good old "Look for blanks and add space to them" part.
    for ( ; nPos < nEnd; ++nPos )
    {
        if( CH_BLANK == pStr->GetChar( nPos ) )
            ++nCnt;
    }

    // We still have to examine the next character:
    // If the next character is ASIAN and not KOREAN we have
    // to add an extra space
    // nPos referes to the original string, even if a field string has
    // been passed to this function
    nPos = rInf.GetIdx() + rPor.GetLen();
    if ( nPos < rInf.GetTxt().Len() )
    {
        BYTE nNextScript = 0;
        const SwLinePortion* pPor = rPor.GetPortion();
        if ( pPor && pPor->IsKernPortion() )
            pPor = pPor->GetPortion();

        if ( ! pBreakIt->xBreak.is() || ! pPor || pPor->InFixMargGrp() )
            return nCnt;

        // next character is inside a field?
        if ( CH_TXTATR_BREAKWORD == rInf.GetChar( nPos ) && pPor->InExpGrp() )
        {
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            pPor->GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );

            nNextScript = (BYTE)pBreakIt->xBreak->getScriptType( aStr, 0 );
        }
        else
            nNextScript = (BYTE)pBreakIt->xBreak->getScriptType( rInf.GetTxt(), nPos );

        if( ASIAN == nNextScript )
        {
            LanguageType aLang =
                rInf.GetTxtFrm()->GetTxtNode()->GetLang( nPos, 1, nNextScript );

            if ( LANGUAGE_KOREAN != aLang && LANGUAGE_KOREAN_JOHAB != aLang )
                ++nCnt;
        }
    }

    return nCnt;
}

/*************************************************************************
 *                      class SwTxtPortion
 *************************************************************************/

SwTxtPortion::SwTxtPortion( const SwLinePortion &rPortion )
  : SwLinePortion( rPortion )
{
    SetWhichPor( POR_TXT );
}

/*************************************************************************
 *                      SwTxtPortion::BreakCut()
 *************************************************************************/

void SwTxtPortion::BreakCut( SwTxtFormatInfo &rInf, const SwTxtGuess &rGuess )
{
    // Das Wort/Zeichen ist groesser als die Zeile
    // Sonderfall Nr.1: Das Wort ist groesser als die Zeile
    // Wir kappen...
    const KSHORT nLineWidth = (KSHORT)(rInf.Width() - rInf.X());
    xub_StrLen nLen = rGuess.CutPos() - rInf.GetIdx();
    if( nLen )
    {
        // special case: guess does not always provide the correct
        // width, only in common cases.
        if ( !rGuess.BreakWidth() )
        {
            rInf.SetLen( nLen );
            SetLen( nLen );
            CalcTxtSize( rInf );

            // changing these values requires also changing them in
            // guess.cxx
            KSHORT nItalic = 0;
            if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
            {
#ifdef MAC
                nItalic = Height() / 4;
#else
                nItalic = Height() / 12;
#endif
            }
            Width( Width() + nItalic );
        }
        else
        {
            Width( rGuess.BreakWidth() );
            SetLen( nLen );
        }
    }
    // special case: first character does not fit to line
    else if ( rGuess.CutPos() == rInf.GetLineStart() )
    {
        SetLen( 1 );
        Width( nLineWidth );
    }
    else
    {
        SetLen( 0 );
        Width( 0 );
    }
}

/*************************************************************************
 *                      SwTxtPortion::BreakUnderflow()
 *************************************************************************/

void SwTxtPortion::BreakUnderflow( SwTxtFormatInfo &rInf )
{
    Truncate();
    Height( 0 );
    Width( 0 );
    SetLen( 0 );
    SetAscent( 0 );
    rInf.SetUnderFlow( this );
}

 /*************************************************************************
 *                      SwTxtPortion::_Format()
 *************************************************************************/

sal_Bool lcl_HasContent( const SwFldPortion& rFld, SwTxtFormatInfo &rInf )
{
    String aTxt;
    return rFld.GetExpTxt( rInf, aTxt ) && aTxt.Len();
}

sal_Bool SwTxtPortion::_Format( SwTxtFormatInfo &rInf )
{
    // 5744: wenn nur der Trennstrich nicht mehr passt,
    // muss trotzdem das Wort umgebrochen werden, ansonsten return sal_True!
    if( rInf.IsUnderFlow() && rInf.GetSoftHyphPos() )
    {
        // soft hyphen portion has triggered an underflow event because
        // of an alternative spelling position
        sal_Bool bFull = sal_False;
        const sal_Bool bHyph = rInf.ChgHyph( sal_True );
        if( rInf.IsHyphenate() )
        {
            SwTxtGuess aGuess;
            // check for alternative spelling left from the soft hyphen
            // this should usually be true but
            aGuess.AlternativeSpelling( rInf, rInf.GetSoftHyphPos() - 1 );
            bFull = CreateHyphen( rInf, aGuess );
            ASSERT( bFull, "Problem with hyphenation!!!" );
        }
        rInf.ChgHyph( bHyph );
        rInf.SetSoftHyphPos( 0 );
        return bFull;
    }

    SwTxtGuess aGuess;
    const sal_Bool bFull = !aGuess.Guess( *this, rInf, Height() );

    // these are the possible cases:
    // A Portion fits to current line
    // B Portion does not fit to current line but a possible line break
    //   within the portion has been found by the break iterator, 2 subcases
    //   B1 break is hyphen
    //   B2 break is word end
    // C Portion does not fit to current line and no possible line break
    //   has been found by break iterator, 2 subcases:
    //   C1 break iterator found a possible line break in portion before us
    //      ==> this break is used (underflow)
    //   C2 break iterator does not found a possible line break at all:
    //      ==> line break

      // case A: line not yet full
    if ( !bFull )
    {
        Width( aGuess.BreakWidth() );
        // Vorsicht !
        if( !InExpGrp() || InFldGrp() )
            SetLen( rInf.GetLen() );

        short nKern = rInf.GetFont()->CheckKerning();
        if( nKern > 0 && rInf.Width() < rInf.X() + Width() + nKern )
        {
            nKern = rInf.Width() - rInf.X() - Width() - 1;
            if( nKern < 0 )
                nKern = 0;
        }
        if( nKern )
            new SwKernPortion( *this, nKern );
    }
    // special case: hanging portion
    else if( bFull && aGuess.GetHangingPortion() )
    {
        Width( aGuess.BreakWidth() );
        SetLen( aGuess.BreakPos() - rInf.GetIdx() );
        Insert( aGuess.GetHangingPortion() );
        aGuess.GetHangingPortion()->SetAscent( GetAscent() );
        aGuess.ClearHangingPortion();
    }
    // breakPos >= index
    else if ( aGuess.BreakPos() >= rInf.GetIdx() && aGuess.BreakPos() != STRING_LEN )
    {
        // case B1
        if( aGuess.HyphWord().is() && aGuess.BreakPos() > rInf.GetLineStart()
            && ( aGuess.BreakPos() > rInf.GetIdx() ||
               ( rInf.GetLast() && ! rInf.GetLast()->IsFlyPortion() ) ) )
        {
            CreateHyphen( rInf, aGuess );
            if ( rInf.GetFly() )
                rInf.GetRoot()->SetMidHyph( sal_True );
            else
                rInf.GetRoot()->SetEndHyph( sal_True );
        }
        // case C1
        else if ( IsFtnPortion() && rInf.IsFakeLineStart() )
            BreakUnderflow( rInf );
        // case B2
        else if( rInf.GetIdx() > rInf.GetLineStart() ||
                 aGuess.BreakPos() > rInf.GetIdx() ||
                 // this is weird: during formatting the follow of a field
                 // the values rInf.GetIdx and rInf.GetLineStart are replaced
                 // IsFakeLineStart indicates GetIdx > GetLineStart
                 rInf.IsFakeLineStart() ||
                 rInf.GetFly() ||
                 rInf.IsFirstMulti() ||
                 ( rInf.GetLast() &&
                    ( rInf.GetLast()->IsFlyPortion() ||
                        ( rInf.GetLast()->InFldGrp() &&
                          ! rInf.GetLast()->InNumberGrp() &&
                          ! rInf.GetLast()->IsErgoSumPortion() &&
                          lcl_HasContent(*((SwFldPortion*)rInf.GetLast()),rInf ) ) ) ) )
        {
            if ( rInf.X() + aGuess.BreakWidth() <= rInf.Width() )
                Width( aGuess.BreakWidth() );
            else
                // this actually should not happen
                Width( KSHORT(rInf.Width() - rInf.X()) );

            SetLen( aGuess.BreakPos() - rInf.GetIdx() );

            ASSERT( aGuess.BreakStart() >= aGuess.FieldDiff(),
                    "Trouble with expanded field portions during line break" );
            const xub_StrLen nRealStart = aGuess.BreakStart() - aGuess.FieldDiff();
            if( aGuess.BreakPos() < nRealStart && !InFldGrp() )
            {
                SwHolePortion *pNew = new SwHolePortion( *this );
                pNew->SetLen( nRealStart - aGuess.BreakPos() );
                Insert( pNew );
            }
        }
        else    // case C2, last exit
            BreakCut( rInf, aGuess );
    }
    // breakPos < index or no breakpos at all
    else
    {
        sal_Bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if( aGuess.BreakPos() != STRING_LEN &&
            aGuess.BreakPos() != rInf.GetLineStart() &&
            ( !bFirstPor || rInf.GetFly() || rInf.GetLast()->IsFlyPortion() ||
              rInf.IsFirstMulti() ) &&
            ( !rInf.GetLast()->IsBlankPortion() ||  ((SwBlankPortion*)
              rInf.GetLast())->MayUnderFlow( rInf, rInf.GetIdx()-1, sal_True )))
        {       // case C1 (former BreakUnderflow())
            BreakUnderflow( rInf );
        }
        else
             // case C2, last exit
            BreakCut( rInf, aGuess );
    }

    return bFull;
}

/*************************************************************************
 *                 virtual SwTxtPortion::Format()
 *************************************************************************/



sal_Bool SwTxtPortion::Format( SwTxtFormatInfo &rInf )
{
#if OSL_DEBUG_LEVEL > 1
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
        // calculate number of blanks
        xub_StrLen nX = rInf.GetIdx() - 1;
        USHORT nHoleLen = 1;
        while( nX && nHoleLen < GetLen() && CH_BLANK == rInf.GetChar( --nX ) )
            nHoleLen++;

        // Erst uns einstellen und dann Inserten, weil wir ja auch ein
        // SwLineLayout sein koennten.
        KSHORT nBlankSize;
        if( nHoleLen == GetLen() )
            nBlankSize = Width();
        else
            nBlankSize = nHoleLen * rInf.GetTxtSize( ' ' ).Width();
        Width( Width() - nBlankSize );
        rInf.X( rInf.X() - nBlankSize );
        SetLen( GetLen() - nHoleLen );
        SwLinePortion *pHole = new SwHolePortion( *this );
        ( (SwHolePortion *)pHole )->SetBlankWidth( nBlankSize );
        ( (SwHolePortion *)pHole )->SetLen( nHoleLen );
        Insert( pHole );
    }
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
    rSizeInf.SetLen( rSizeInf.GetTxtBreak( nOfst, nLineLength )
                   - rSizeInf.GetIdx() );
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

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && pPortion && !pPortion->Width() )
            pPortion->PrePaint( rInf, this );

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
        if( !IsBlankPortion() && !InNumberGrp() && !IsCombinedPortion() )
        {
            // Bei OnWin() wird anstatt eines Leerstrings gern mal ein Blank
            // zurueckgeliefert, das koennen wir hier aber gar nicht gebrauchen
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );

            nCnt += lcl_AddSpace( rInf, &aStr, *this );
            nPos = aStr.Len();
        }
    }
    else if( !IsDropPortion() )
    {
        nCnt += lcl_AddSpace( rInf, 0, *this );
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
                nCnt += lcl_AddSpace( rInf, &aStr, *this );
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
            nCnt += lcl_AddSpace( rInf, 0, *this );
        else
        {
            nSpaceAdd = -nSpaceAdd;
            nCnt = GetLen();
            SwLinePortion* pPor = GetPortion();

            // we do not want an extra space in front of margin portions
            if ( ! nCnt )
                return 0;

            while ( pPor && !pPor->Width() && ! pPor->IsHolePortion() )
                pPor = pPor->GetPortion();

            if ( !pPor || pPor->InFixMargGrp() || pPor->IsHolePortion() )
                return ( nCnt - 1 ) * nSpaceAdd;
        }
    }
    return nCnt * nSpaceAdd;
}

/*************************************************************************
 *              virtual SwTxtPortion::HandlePortion()
 *************************************************************************/

void SwTxtPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
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

/*************************************************************************
 *              virtual SwHolePortion::HandlePortion()
 *************************************************************************/

void SwHolePortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

