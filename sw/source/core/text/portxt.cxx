/*************************************************************************
 *
 *  $RCSfile: portxt.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-13 09:56:21 $
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
#ifndef _PORFLD_HXX
#include <porfld.hxx>       // SwFldPortion
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

/*************************************************************************
 *                      SwTxtPortion::BreakCut()
 *************************************************************************/

void SwTxtPortion::BreakCut( SwTxtFormatInfo &rInf, const SwTxtGuess &rGuess )
{
    // Das Wort/Zeichen ist groesser als die Zeile
    // Sonderfall Nr.1: Das Wort ist groesser als die Zeile
    // Wir kappen...
    const KSHORT nLineWidth = rInf.Width() - rInf.X();
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
    else if (  rGuess.CutPos() == rInf.GetLineStart() )
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
        }
        rInf.ChgHyph( bHyph );
        rInf.SetSoftHyphPos( 0 );
        return bFull;
    }

    SwTxtGuess aGuess;
    const sal_Bool bFull = !aGuess.Guess( rInf, Height() );

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
            nKern = rInf.Width() - rInf.X() - Width();
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
        if( aGuess.HyphWord().is() && ( aGuess.BreakPos() > rInf.GetIdx() ||
            !rInf.GetLast()->IsFlyPortion() ) )
        {
            CreateHyphen( rInf, aGuess );
            if ( rInf.GetFly() )
                rInf.GetRoot()->SetMidHyph( sal_True );
            else
                rInf.GetRoot()->SetEndHyph( sal_True );
        }
        // case C1
        else if ( IsFtnPortion() )
            BreakUnderflow( rInf );
        // case B2
        else if( rInf.GetIdx() > rInf.GetLineStart() ||
                 aGuess.BreakPos() > rInf.GetIdx() ||
                    rInf.GetFly() ||
                 rInf.GetLast()->IsFlyPortion() || rInf.IsFirstMulti() ||
                  (rInf.GetLast()->InFldGrp() && !rInf.GetLast()->InNumberGrp()
                      && lcl_HasContent(*((SwFldPortion*)rInf.GetLast()),rInf) )
                 )
        {
            ASSERT( rInf.X() + aGuess.BreakWidth() <= rInf.Width(),
                    "What a guess?!" );
            Width( aGuess.BreakWidth() );
            SetLen( aGuess.BreakPos() - rInf.GetIdx() );
            if( aGuess.BreakPos() < aGuess.BreakStart() && !InFldGrp() )
            {
                SwHolePortion *pNew = new SwHolePortion( *this );
                pNew->SetLen( aGuess.BreakStart() - aGuess.BreakPos() );
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
        if( !IsBlankPortion() && !InNumberGrp() && !IsCombinedPortion() )
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

