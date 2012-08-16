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

#include <ctype.h>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <i18npool/mslangid.hxx>
#include <hintids.hxx>     // CH_TXTATR
#include <SwPortionHandler.hxx>
#include <porlay.hxx>
#include <inftxt.hxx>
#include <guess.hxx>    // SwTxtGuess, line break
#include <porglue.hxx>
#include <portab.hxx>       // pLastTab->
#include <porfld.hxx>       // SwFldPortion
#include <wrong.hxx>
#include <viewsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <viewopt.hxx>  // SwViewOptions

#include <IMark.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <xmloff/odffields.hxx>

using namespace ::sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::ScriptType;

/*************************************************************************
 *                          lcl_AddSpace
 * Returns for how many characters an extra space has to be added
 * (for justified alignment).
 *************************************************************************/

sal_uInt16 lcl_AddSpace( const SwTxtSizeInfo &rInf, const XubString* pStr,
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

    sal_uInt16 nCnt = 0;
    sal_uInt8 nScript = 0;

    // If portion consists of Asian characters and language is not
    // Korean, we add extra space to each character.
    // first we get the script type
    if ( pSI )
        nScript = pSI->ScriptType( nPos );
    else if ( pBreakIt->GetBreakIter().is() )
        nScript = (sal_uInt8)pBreakIt->GetBreakIter()->getScriptType( *pStr, nPos );

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

        if (MsLangId::isKorean(aLang))
        {
            const SwLinePortion* pPor = rPor.GetPortion();
            if ( pPor && ( pPor->IsKernPortion() ||
                           pPor->IsControlCharPortion() ||
                           pPor->IsPostItsPortion() ) )
                pPor = pPor->GetPortion();

            nCnt += nEnd - nPos;

            if ( !pPor || pPor->IsHolePortion() || pPor->InFixMargGrp() ||
                  pPor->IsBreakPortion() )
                --nCnt;

            return nCnt;
        }
    }

    // Kashida Justification: Insert Kashidas
    if ( nEnd > nPos && pSI && COMPLEX == nScript )
    {
        if ( SwScriptInfo::IsArabicText( *pStr, nPos, nEnd - nPos ) && pSI->CountKashida() )
        {
            const sal_uInt16 nKashRes = pSI->KashidaJustify( 0, 0, nPos, nEnd - nPos );
            // i60591: need to check result of KashidaJustify
            // determine if kashida justification is applicable
            if( nKashRes != STRING_LEN )
                return nKashRes;
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
            if ( pPor && ( pPor->IsKernPortion() ||
                           pPor->IsControlCharPortion() ||
                           pPor->IsPostItsPortion() ) )
                pPor = pPor->GetPortion();

            if ( nCnt && ( ! pPor || pPor->IsHolePortion() || pPor->InFixMargGrp() ) )
                --nCnt;

            return nCnt;
        }
    }

    // Here starts the good old "Look for blanks and add space to them" part.
    // Note: We do not want to add space to an isolated latin blank in front
    // of some complex characters in RTL environment
    const sal_Bool bDoNotAddSpace =
            LATIN == nScript && ( nEnd == nPos + 1 ) && pSI &&
            ( i18n::ScriptType::COMPLEX ==
              pSI->ScriptType( nPos + 1 ) ) &&
            rInf.GetTxtFrm() && rInf.GetTxtFrm()->IsRightToLeft();

    if ( bDoNotAddSpace )
        return nCnt;

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
        sal_uInt8 nNextScript = 0;
        const SwLinePortion* pPor = rPor.GetPortion();
        if ( pPor && pPor->IsKernPortion() )
            pPor = pPor->GetPortion();

        if ( ! pBreakIt->GetBreakIter().is() || ! pPor || pPor->InFixMargGrp() )
            return nCnt;

        // next character is inside a field?
        if ( CH_TXTATR_BREAKWORD == rInf.GetChar( nPos ) && pPor->InExpGrp() )
        {
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            pPor->GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );

            nNextScript = (sal_uInt8)pBreakIt->GetBreakIter()->getScriptType( aStr, 0 );
        }
        else
            nNextScript = (sal_uInt8)pBreakIt->GetBreakIter()->getScriptType( rInf.GetTxt(), nPos );

        if( ASIAN == nNextScript )
        {
            LanguageType aLang =
                rInf.GetTxtFrm()->GetTxtNode()->GetLang( nPos, 1, nNextScript );

            if (MsLangId::isKorean(aLang))
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
    // The word/char is larger than the line
    // Special case 1: The word is larger than the line
    // We truncate ...
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
                nItalic = Height() / 12;
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
    // 5744: If only the hypen does not fit anymore, we still need to wrap
    // the word, or else return sal_True!
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
            OSL_ENSURE( bFull, "Problem with hyphenation!!!" );
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
            nKern = (short)(rInf.Width() - rInf.X() - Width() - 1);
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
        // - Footnote portions with fake line start (i.e., not at beginning of line)
        //   should keep together with the text portion. (Note: no keep together
        //   with only footnote portions.
        // - TabPortions not at beginning of line should keep together with the
        //   text portion, if they are not followed by a blank
        //   (work around different definition of tab stop character - breaking or
        //   non breaking character - in compatibility mode)
        else if ( ( IsFtnPortion() && rInf.IsFakeLineStart() &&
                    //
                    rInf.IsOtherThanFtnInside() ) ||
                  ( rInf.GetLast() &&
                    rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT) &&
                    rInf.GetLast()->InTabGrp() &&
                    rInf.GetLineStart() + rInf.GetLast()->GetLen() < rInf.GetIdx() &&
                    aGuess.BreakPos() == rInf.GetIdx()  &&
                    CH_BLANK != rInf.GetChar( rInf.GetIdx() ) &&
                    0x3000 != rInf.GetChar( rInf.GetIdx() ) ) )
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

            OSL_ENSURE( aGuess.BreakStart() >= aGuess.FieldDiff(),
                    "Trouble with expanded field portions during line break" );
            const xub_StrLen nRealStart = aGuess.BreakStart() - aGuess.FieldDiff();
            if( aGuess.BreakPos() < nRealStart && !InExpGrp() )
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

    OSL_ENSURE( rInf.RealWidth() || (rInf.X() == rInf.Width()),
        "SwTxtPortion::Format: missing real width" );
    OSL_ENSURE( Height(), "SwTxtPortion::Format: missing height" );

    return _Format( rInf );
}

/*************************************************************************
 *                 virtual SwTxtPortion::FormatEOL()
 *************************************************************************/

// Format end of line
// 5083: We can have awkward cases e.g.:
// "from {Santa}"
// Santa wraps, "from " turns into "from" and " " in a justified
// paragraph, in which the glue gets expanded instead of merged
// with the MarginPortion.
//
// rInf.nIdx points to the next word, nIdx-1 is the portion's last char

void SwTxtPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if( ( !GetPortion() || ( GetPortion()->IsKernPortion() &&
        !GetPortion()->GetPortion() ) ) && GetLen() &&
        rInf.GetIdx() < rInf.GetTxt().Len() &&
        1 < rInf.GetIdx() && ' ' == rInf.GetChar( rInf.GetIdx() - 1 )
        && !rInf.GetLast()->IsHolePortion() )
    {
        // calculate number of blanks
        xub_StrLen nX = rInf.GetIdx() - 1;
        sal_uInt16 nHoleLen = 1;
        while( nX && nHoleLen < GetLen() && CH_BLANK == rInf.GetChar( --nX ) )
            nHoleLen++;

        // First set ourselves and the insert, because there could be
        // a SwLineLayout
        KSHORT nBlankSize;
        if( nHoleLen == GetLen() )
            nBlankSize = Width();
        else
            nBlankSize = nHoleLen * rInf.GetTxtSize(rtl::OUString(' ')).Width();
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
    OSL_ENSURE( !this, "SwTxtPortion::GetCrsrOfst: don't use this method!" );
    return SwLinePortion::GetCrsrOfst( nOfst );
}

/*************************************************************************
 *                virtual SwTxtPortion::GetTxtSize()
 *************************************************************************/
// The GetTxtSize() assumes that the own length is correct

SwPosSize SwTxtPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    return rInf.GetTxtSize();
}

/*************************************************************************
 *               virtual SwTxtPortion::Paint()
 *************************************************************************/
void SwTxtPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if (rInf.OnWin() && 1==rInf.GetLen() && CH_TXT_ATR_FIELDEND==rInf.GetTxt().GetChar(rInf.GetIdx()))
    {
        rInf.DrawBackBrush( *this );
        const rtl::OUString aTxt(CH_TXT_ATR_SUBST_FIELDEND);
        rInf.DrawText( aTxt, *this, 0, aTxt.getLength(), false );
    }
    else if (rInf.OnWin() && 1==rInf.GetLen() && CH_TXT_ATR_FIELDSTART==rInf.GetTxt().GetChar(rInf.GetIdx()))
    {
        rInf.DrawBackBrush( *this );
        const rtl::OUString aTxt(CH_TXT_ATR_SUBST_FIELDSTART);
        rInf.DrawText( aTxt, *this, 0, aTxt.getLength(), false );
    }
    else if( GetLen() )
    {
        rInf.DrawBackBrush( *this );

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && pPortion && !pPortion->Width() )
            pPortion->PrePaint( rInf, this );

        const SwWrongList *pWrongList = rInf.GetpWrongList();
        const SwWrongList *pGrammarCheckList = rInf.GetGrammarCheckList();
        // SMARTTAGS
        const SwWrongList *pSmarttags = rInf.GetSmartTags();

        const bool bWrong = 0 != pWrongList;
        const bool bGrammarCheck = 0 != pGrammarCheckList;
        const bool bSmartTags = 0 != pSmarttags;

        if ( bWrong || bSmartTags || bGrammarCheck )
            rInf.DrawMarkedText( *this, rInf.GetLen(), sal_False, bWrong, bSmartTags, bGrammarCheck );
        else
            rInf.DrawText( *this, rInf.GetLen(), sal_False );
    }
}

/*************************************************************************
 *              virtual SwTxtPortion::GetExpTxt()
 *************************************************************************/



sal_Bool SwTxtPortion::GetExpTxt( const SwTxtSizeInfo &, XubString & ) const
{
    return sal_False;
}

/*************************************************************************
 *        xub_StrLen SwTxtPortion::GetSpaceCnt()
 *              long SwTxtPortion::CalcSpacing()
 * Are responsible for the justified paragraph. They calculate the blank
 * count and the resulting added space.
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
            // OnWin() likes to return a blank instead of an empty string from
            // time to time. We cannot use that here at all, however.
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );

            nCnt = nCnt + lcl_AddSpace( rInf, &aStr, *this );
            nPos = aStr.Len();
        }
    }
    else if( !IsDropPortion() )
    {
        nCnt = nCnt + lcl_AddSpace( rInf, 0, *this );
        nPos = GetLen();
    }
    rCharCnt = rCharCnt + nPos;
    return nCnt;
}

long SwTxtPortion::CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const
{
    xub_StrLen nCnt = 0;

    if ( InExpGrp() )
    {
        if( !IsBlankPortion() && !InNumberGrp() && !IsCombinedPortion() )
        {
            // OnWin() likes to return a blank instead of an empty string from
            // time to time. We cannot use that here at all, however.
            sal_Bool bOldOnWin = rInf.OnWin();
            ((SwTxtSizeInfo &)rInf).SetOnWin( sal_False );

            XubString aStr( aEmptyStr );
            GetExpTxt( rInf, aStr );
            ((SwTxtSizeInfo &)rInf).SetOnWin( bOldOnWin );
            if( nSpaceAdd > 0 )
                nCnt = nCnt + lcl_AddSpace( rInf, &aStr, *this );
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
            nCnt = nCnt + lcl_AddSpace( rInf, 0, *this );
        else
        {
            nSpaceAdd = -nSpaceAdd;
            nCnt = GetLen();
            SwLinePortion* pPor = GetPortion();

            // we do not want an extra space in front of margin portions
            if ( nCnt )
            {
                while ( pPor && !pPor->Width() && ! pPor->IsHolePortion() )
                    pPor = pPor->GetPortion();

                if ( !pPor || pPor->InFixMargGrp() || pPor->IsHolePortion() )
                    --nCnt;
            }
        }
    }

    return nCnt * nSpaceAdd / SPACING_PRECISION_FACTOR;
}

/*************************************************************************
 *              virtual SwTxtPortion::HandlePortion()
 *************************************************************************/

void SwTxtPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor(), Height(), Width() );
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
    // #i16816# tagged pdf support
    if( rInf.GetVsh() && rInf.GetVsh()->GetViewOptions()->IsPDFExport() )
    {
        const rtl::OUString aTxt( ' ' );
        rInf.DrawText( aTxt, *this, 0, 1, false );
    }
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

void SwFieldMarkPortion::Paint( const SwTxtPaintInfo & /*rInf*/) const
{
    // These shouldn't be painted!
    // SwTxtPortion::Paint(rInf);
}

sal_Bool SwFieldMarkPortion::Format( SwTxtFormatInfo & )
{
    sal_Bool ret=0;
    Width(0);
    return ret;
}

namespace {
    static sal_Int32 getCurrentListIndex( IFieldmark* pBM,
            ::rtl::OUString* io_pCurrentText = NULL )
    {
        const IFieldmark::parameter_map_t* const pParameters = pBM->GetParameters();
        sal_Int32 nCurrentIdx = 0;
        const IFieldmark::parameter_map_t::const_iterator pResult = pParameters->find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODF_FORMDROPDOWN_RESULT)));
        if(pResult != pParameters->end())
            pResult->second >>= nCurrentIdx;
        if(io_pCurrentText)
        {
            const IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODF_FORMDROPDOWN_LISTENTRY)));
            if(pListEntries != pParameters->end())
            {
                uno::Sequence< ::rtl::OUString > vListEntries;
                pListEntries->second >>= vListEntries;
                if(nCurrentIdx < vListEntries.getLength())
                    *io_pCurrentText = vListEntries[nCurrentIdx];
            }
        }
        return nCurrentIdx;
    }
}

//FIXME Fieldbk
void SwFieldFormPortion::Paint( const SwTxtPaintInfo& rInf ) const
{
    SwTxtNode* pNd = const_cast<SwTxtNode*>(rInf.GetTxtFrm()->GetTxtNode());
    const SwDoc *doc=pNd->GetDoc();
    SwIndex aIndex( pNd, rInf.GetIdx() );
    SwPosition aPosition(*pNd, aIndex);

    IFieldmark* pBM = doc->getIDocumentMarkAccess( )->getFieldmarkFor( aPosition );

    OSL_ENSURE( pBM,
        "SwFieldFormPortion::Paint(..)"
        " - Where is my form field bookmark???");

    if ( pBM != NULL )
    {
        if ( pBM->GetFieldname( ) == ODF_FORMCHECKBOX )
        { // a checkbox...
            ICheckboxFieldmark* pCheckboxFm = dynamic_cast< ICheckboxFieldmark* >(pBM);
            bool checked = pCheckboxFm->IsChecked();
            rInf.DrawCheckBox(*this, checked);
        }
        else if ( pBM->GetFieldname( ) == ODF_FORMDROPDOWN )
        { // a list...
            rtl::OUString aTxt;
            getCurrentListIndex( pBM, &aTxt );
            rInf.DrawViewOpt( *this, POR_FLD );
            rInf.DrawText( aTxt, *this, 0, aTxt.getLength(), false );
        }
        else
        {
            assert(0); // unknown type...
        }
    }
}

sal_Bool SwFieldFormPortion::Format( SwTxtFormatInfo & rInf )
{
    sal_Bool ret = 0;
    SwTxtNode *pNd = const_cast < SwTxtNode * >( rInf.GetTxtFrm(  )->GetTxtNode(  ) );
    const SwDoc *doc = pNd->GetDoc(  );
    SwIndex aIndex( pNd, rInf.GetIdx(  ) );
    SwPosition aPosition( *pNd, aIndex );
    IFieldmark *pBM = doc->getIDocumentMarkAccess( )->getFieldmarkFor( aPosition );
    OSL_ENSURE( pBM != NULL, "Where is my form field bookmark???" );
    if ( pBM != NULL )
    {
        if ( pBM->GetFieldname( ) == ODF_FORMCHECKBOX )
        {
            Width( rInf.GetTxtHeight(  ) );
            Height( rInf.GetTxtHeight(  ) );
            SetAscent( rInf.GetAscent(  ) );
        }
        else if ( pBM->GetFieldname( ) == ODF_FORMDROPDOWN )
        {
            ::rtl::OUString aTxt;
            getCurrentListIndex( pBM, &aTxt );
            SwPosSize aPosSize = rInf.GetTxtSize( aTxt );
            Width( aPosSize.Width(  ) );
            Height( aPosSize.Height(  ) );
            SetAscent( rInf.GetAscent(  ) );
        }
        else
        {
            assert( 0 );        // unknown type...
        }
    }
    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
