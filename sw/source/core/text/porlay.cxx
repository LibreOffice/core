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


#include "porlay.hxx"
#include "itrform2.hxx"
#include "porglue.hxx"
#include "porexp.hxx"   // SwQuoVadisPortion
#include "blink.hxx"    // pBlink
#include "redlnitr.hxx" // SwRedlineItr
#include "porfly.hxx"   // SwFlyCntPortion
#include <porrst.hxx>   // SwHangingPortion
#include <pormulti.hxx> // SwMultiPortion
#include <breakit.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CTLScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <paratr.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <vcl/outdev.hxx>
#include <editeng/blnkitem.hxx>
#include <tools/multisel.hxx>
#include <unotools/charclass.hxx>
#include <i18npool/mslangid.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <docary.hxx>   // SwRedlineTbl
#include <redline.hxx>  // SwRedline
#include <section.hxx>
#include <switerator.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentContentOperations.hxx>

using namespace ::com::sun::star;
using namespace i18n::ScriptType;

#include <unicode/ubidi.h>
#include <i18nutil/scripttypedetector.hxx>
#include <i18nutil/unicode.hxx>

#define IS_JOINING_GROUP(c, g) ( u_getIntPropertyValue( (c), UCHAR_JOINING_GROUP ) == U_JG_##g )
#define isAinChar(c)        IS_JOINING_GROUP((c), AIN)
#define isAlefChar(c)       IS_JOINING_GROUP((c), ALEF)
#define isBaaChar(c)        IS_JOINING_GROUP((c), BEH)
#define isDalChar(c)        IS_JOINING_GROUP((c), DAL)
#define isFehChar(c)        IS_JOINING_GROUP((c), FEH)
#define isGafChar(c)        IS_JOINING_GROUP((c), GAF)
#define isHahChar(c)        IS_JOINING_GROUP((c), HAH)
#define isKafChar(c)        IS_JOINING_GROUP((c), KAF)
#define isLamChar(c)        IS_JOINING_GROUP((c), LAM)
#define isQafChar(c)        IS_JOINING_GROUP((c), QAF)
#define isRehChar(c)        IS_JOINING_GROUP((c), REH)
#define isTehMarbutaChar(c) IS_JOINING_GROUP((c), TEH_MARBUTA)
#define isWawChar(c)        IS_JOINING_GROUP((c), WAW)
#if ( (U_ICU_VERSION_MAJOR_NUM > 4 ) || ((U_ICU_VERSION_MAJOR_NUM == 4) && (U_ICU_VERSION_MINOR_NUM >= 4)) )
#define isYehChar(c)        (IS_JOINING_GROUP((c), YEH) || IS_JOINING_GROUP((c), FARSI_YEH))
#else
#define isYehChar(c)        (c == 0x626 || c == 0x649 || c == 0x64A || c == 0x678 || c == 0x6CC || c == 0x6CE || c == 0x6D0 || c == 0x6D1)
#endif
#define isSeenOrSadChar(c)  (IS_JOINING_GROUP((c), SAD) || IS_JOINING_GROUP((c), SEEN))

sal_Bool isTransparentChar ( sal_Unicode cCh )
{
    return u_getIntPropertyValue( cCh, UCHAR_JOINING_TYPE ) == U_JT_TRANSPARENT;
}

/*************************************************************************
 * lcl_IsLigature
 *
 * Checks if cCh + cNectCh builds a ligature (used for Kashidas)
 *************************************************************************/

static sal_Bool lcl_IsLigature( sal_Unicode cCh, sal_Unicode cNextCh )
{
            // Lam + Alef
    return ( isLamChar ( cCh ) && isAlefChar ( cNextCh ));
}

/*************************************************************************
 * lcl_ConnectToPrev
 *
 * Checks if cCh is connectable to cPrevCh (used for Kashidas)
 *************************************************************************/

static sal_Bool lcl_ConnectToPrev( sal_Unicode cCh, sal_Unicode cPrevCh )
{
    const int32_t nJoiningType = u_getIntPropertyValue( cPrevCh, UCHAR_JOINING_TYPE );
    sal_Bool bRet = nJoiningType != U_JT_RIGHT_JOINING && nJoiningType != U_JT_NON_JOINING;

    // check for ligatures cPrevChar + cChar
    if( bRet )
        bRet = !lcl_IsLigature( cPrevCh, cCh );

    return bRet;
}

/*************************************************************************
 * lcl_HasStrongLTR
 *************************************************************************/
static  bool lcl_HasStrongLTR ( const String& rTxt, xub_StrLen nStart, xub_StrLen nEnd )
 {
     for ( xub_StrLen nCharIdx = nStart; nCharIdx < nEnd; ++nCharIdx )
     {
         const UCharDirection nCharDir = u_charDirection ( rTxt.GetChar ( nCharIdx ));
         if ( nCharDir == U_LEFT_TO_RIGHT ||
              nCharDir == U_LEFT_TO_RIGHT_EMBEDDING ||
              nCharDir == U_LEFT_TO_RIGHT_OVERRIDE )
             return true;
     }
     return false;
 }

/*************************************************************************
 * SwLineLayout::~SwLineLayout()
 *
 * class SwLineLayout: This is the layout of a single line, which is made
 * up of it's dimension, the character count and the word spacing in the
 * line.
 * Line objects are managed in an own pool, in order to store them continuously
 * in memory so that they are paged out together and don't fragment memory.
 *************************************************************************/

SwLineLayout::~SwLineLayout()
{
    Truncate();
    delete pNext;
    if( pBlink )
        pBlink->Delete( this );
    delete pLLSpaceAdd;
    delete pKanaComp;
}

SwLinePortion *SwLineLayout::Insert( SwLinePortion *pIns )
{
   // First attribute change: copy mass and length from *pIns into the first
   // text portion
    if( !pPortion )
    {
        if( GetLen() )
        {
            pPortion = new SwTxtPortion( *(SwLinePortion*)this );
            if( IsBlinking() && pBlink )
            {
                SetBlinking( sal_False );
                pBlink->Replace( this, pPortion );
            }
        }
        else
        {
            SetPortion( pIns );
            return pIns;
        }
    }
    // Call with scope or we'll end up with recursion!
    return pPortion->SwLinePortion::Insert( pIns );
}

SwLinePortion *SwLineLayout::Append( SwLinePortion *pIns )
{
    // First attribute change: copy mass and length from *pIns into the first
    // text portion
    if( !pPortion )
        pPortion = new SwTxtPortion( *(SwLinePortion*)this );
    // Call with scope or we'll end up with recursion!
    return pPortion->SwLinePortion::Append( pIns );
}

// For special treatment of empty lines

sal_Bool SwLineLayout::Format( SwTxtFormatInfo &rInf )
{
    if( GetLen() )
        return SwTxtPortion::Format( rInf );
    else
    {
        Height( rInf.GetTxtHeight() );
        return sal_True;
    }
}

/*************************************************************************
 * SwLineLayout::CalcLeftMargin()
 *
 * We collect all FlyPortions at the beginning of the line and make that a
 * MarginPortion.
 *************************************************************************/

SwMarginPortion *SwLineLayout::CalcLeftMargin()
{
    SwMarginPortion *pLeft = (GetPortion() && GetPortion()->IsMarginPortion()) ?
        (SwMarginPortion *)GetPortion() : 0;
    if( !GetPortion() )
         SetPortion( new SwTxtPortion( *(SwLinePortion*)this ) );
    if( !pLeft )
    {
        pLeft = new SwMarginPortion( 0 );
        pLeft->SetPortion( GetPortion() );
        SetPortion( pLeft );
    }
    else
    {
        pLeft->Height( 0 );
        pLeft->Width( 0 );
        pLeft->SetLen( 0 );
        pLeft->SetAscent( 0 );
        pLeft->SetPortion( NULL );
        pLeft->SetFixWidth(0);
    }

    SwLinePortion *pPos = pLeft->GetPortion();
    while( pPos )
    {
        if( pPos->IsFlyPortion() )
        {
            // The FlyPortion get's sucked out ...
            pLeft->Join( (SwGluePortion*)pPos );
            pPos = pLeft->GetPortion();
            if( GetpKanaComp() && !GetKanaComp().empty() )
                GetKanaComp().pop_front();
        }
        else
            pPos = 0;
    }
    return pLeft;
}

void SwLineLayout::InitSpaceAdd()
{
    if ( !pLLSpaceAdd )
        CreateSpaceAdd();
    else
        SetLLSpaceAdd( 0, 0 );
}

void SwLineLayout::CreateSpaceAdd( const long nInit )
{
    pLLSpaceAdd = new std::vector<long>;
    SetLLSpaceAdd( nInit, 0 );
}

/*************************************************************************
 * Local helper function. Returns true if there are only blanks
 * in [nStt, nEnd[
 *************************************************************************/

static bool lcl_HasOnlyBlanks( const XubString& rTxt, xub_StrLen nStt, xub_StrLen nEnd )
{
    bool bBlankOnly = true;
    while ( nStt < nEnd )
    {
        const sal_Unicode cChar = rTxt.GetChar( nStt++ );
        if ( ' ' != cChar && 0x3000 != cChar )
        {
            bBlankOnly = false;
            break;
        }
    }
    return bBlankOnly;
}

/*************************************************************************
 * SwLineLayout::CalcLine()
 *
 * Swapped out from FormatLine()
 *************************************************************************/

void SwLineLayout::CalcLine( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
{
    const KSHORT nLineWidth = rInf.RealWidth();

    KSHORT nFlyAscent = 0;
    KSHORT nFlyHeight = 0;
    KSHORT nFlyDescent = 0;
    sal_Bool bOnlyPostIts = sal_True;
    SetHanging( sal_False );

    sal_Bool bTmpDummy = ( 0 == GetLen() );
    SwFlyCntPortion* pFlyCnt = 0;
    if( bTmpDummy )
    {
        nFlyAscent = 0;
        nFlyHeight = 0;
        nFlyDescent = 0;
    }

    // #i3952#
    const bool bIgnoreBlanksAndTabsForLineHeightCalculation =
            rInf.GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION);

    bool bHasBlankPortion = false;
    bool bHasOnlyBlankPortions = true;

    if( pPortion )
    {
        SetCntnt( sal_False );
        if( pPortion->IsBreakPortion() )
        {
            SetLen( pPortion->GetLen() );
            if( GetLen() )
                bTmpDummy = sal_False;
        }
        else
        {
            KSHORT nLineHeight = Height();
            Init( GetPortion() );
            SwLinePortion *pPos = pPortion;
            SwLinePortion *pLast = this;
            KSHORT nMaxDescent = 0;

            // A group is a segment in the portion chain of pCurr or a fixed
            // portion spanning to the end or the next fixed portion
            while( pPos )
            {
                SAL_WARN_IF( POR_LIN == pPos->GetWhichPor(),
                        "sw.core", "SwLineLayout::CalcLine: don't use SwLinePortions !" );

                // Null portions are eliminated. They can form if two FlyFrms
                // overlap.
                if( !pPos->Compress() )
                {
                    // Only take over Height and Ascent if the rest of the line
                    // is empty.
                    if( !pPos->GetPortion() )
                    {
                        if( !Height() )
                            Height( pPos->Height() );
                        if( !GetAscent() )
                            SetAscent( pPos->GetAscent() );
                    }
                    delete pLast->Cut( pPos );
                    pPos = pLast->GetPortion();
                    continue;
                }

                const xub_StrLen nPorSttIdx = rInf.GetLineStart() + nLineLength;
                nLineLength = nLineLength + pPos->GetLen();
                AddPrtWidth( pPos->Width() );

                // #i3952#
                if ( bIgnoreBlanksAndTabsForLineHeightCalculation )
                {
                    if ( pPos->InTabGrp() || pPos->IsHolePortion() ||
                            ( pPos->IsTextPortion() &&
                              lcl_HasOnlyBlanks( rInf.GetTxt(), nPorSttIdx, nPorSttIdx + pPos->GetLen() ) ) )
                    {
                        pLast = pPos;
                        pPos = pPos->GetPortion();
                        bHasBlankPortion = true;
                        continue;
                    }
                }

                bHasOnlyBlankPortions = false;

                // We had an attribute change: Sum up/build maxima of length and mass

                KSHORT nPosHeight = pPos->Height();
                KSHORT nPosAscent = pPos->GetAscent();

                SAL_WARN_IF( nPosHeight < nPosAscent,
                        "sw.core", "SwLineLayout::CalcLine: bad ascent or height" );

                if( pPos->IsHangingPortion() )
                {
                    SetHanging( sal_True );
                    rInf.GetParaPortion()->SetMargin( sal_True );
                }

                // To prevent that a paragraph-end-character does not change
                // the line height through a Descent and thus causing the line
                // to reformat.
                if ( !pPos->IsBreakPortion() || !Height() )
                {
                    bOnlyPostIts &= pPos->IsPostItsPortion();

                    if( bTmpDummy && !nLineLength )
                    {
                        if( pPos->IsFlyPortion() )
                        {
                            if( nFlyHeight < nPosHeight )
                                nFlyHeight = nPosHeight;
                            if( nFlyAscent < nPosAscent )
                                nFlyAscent = nPosAscent;
                            if( nFlyDescent < nPosHeight - nPosAscent )
                                nFlyDescent = nPosHeight - nPosAscent;
                        }
                        else
                        {
                            if( pPos->InNumberGrp() )
                            {
                                KSHORT nTmp = rInf.GetFont()->GetAscent(
                                                rInf.GetVsh(), *rInf.GetOut() );
                                if( nTmp > nPosAscent )
                                {
                                    nPosHeight += nTmp - nPosAscent;
                                    nPosAscent = nTmp;
                                }
                                nTmp = rInf.GetFont()->GetHeight( rInf.GetVsh(),
                                                                 *rInf.GetOut() );
                                if( nTmp > nPosHeight )
                                    nPosHeight = nTmp;
                            }
                            Height( nPosHeight );
                            nAscent = nPosAscent;
                            nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                    else if( !pPos->IsFlyPortion() )
                    {
                        if( Height() < nPosHeight )
                        {
                            // Height is set to 0 when Init() is called.
                            if (bIgnoreBlanksAndTabsForLineHeightCalculation && pPos->GetWhichPor() == POR_FLYCNT)
                                // Compat flag set: take the line height, if it's larger.
                                Height(std::max(nPosHeight, nLineHeight));
                            else
                                // Just care about the portion height.
                                Height(nPosHeight);
                        }
                        if( pPos->IsFlyCntPortion() || ( pPos->IsMultiPortion()
                            && ((SwMultiPortion*)pPos)->HasFlyInCntnt() ) )
                            rLine.SetFlyInCntBase();
                        if( pPos->IsFlyCntPortion() &&
                            ((SwFlyCntPortion*)pPos)->GetAlign() )
                        {
                            ((SwFlyCntPortion*)pPos)->SetMax( sal_False );
                            if( !pFlyCnt || pPos->Height() > pFlyCnt->Height() )
                                pFlyCnt = (SwFlyCntPortion*)pPos;
                        }
                        else
                        {
                            if( nAscent < nPosAscent )
                                nAscent = nPosAscent;
                            if( nMaxDescent < nPosHeight - nPosAscent )
                                nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                }
                else if( pPos->GetLen() )
                    bTmpDummy = sal_False;

                if( !HasCntnt() && !pPos->InNumberGrp() )
                {
                    if ( pPos->InExpGrp() )
                    {
                        XubString aTxt;
                        if( pPos->GetExpTxt( rInf, aTxt ) && aTxt.Len() )
                            SetCntnt( sal_True );
                    }
                    else if( ( pPos->InTxtGrp() || pPos->IsMultiPortion() ) &&
                             pPos->GetLen() )
                        SetCntnt( sal_True );
                }

                bTmpDummy = bTmpDummy && !HasCntnt() &&
                            ( !pPos->Width() || pPos->IsFlyPortion() );

                pLast = pPos;
                pPos = pPos->GetPortion();
            }

            if( pFlyCnt )
            {
                if( pFlyCnt->Height() == Height() )
                {
                    pFlyCnt->SetMax( sal_True );
                    if( Height() > nMaxDescent + nAscent )
                    {
                        if( 3 == pFlyCnt->GetAlign() ) // Bottom
                            nAscent = Height() - nMaxDescent;
                        else if( 2 == pFlyCnt->GetAlign() ) // Center
                            nAscent = ( Height() + nAscent - nMaxDescent ) / 2;
                    }
                    pFlyCnt->SetAscent( nAscent );
                }
            }

            if( bTmpDummy && nFlyHeight )
            {
                nAscent = nFlyAscent;
                if( nFlyDescent > nFlyHeight - nFlyAscent )
                    Height( nFlyHeight + nFlyDescent );
                else
                    Height( nFlyHeight );
            }
            else if( nMaxDescent > Height() - nAscent )
                Height( nMaxDescent + nAscent );

            if( bOnlyPostIts && !( bHasBlankPortion && bHasOnlyBlankPortions ) )
            {
                Height( rInf.GetFont()->GetHeight( rInf.GetVsh(), *rInf.GetOut() ) );
                nAscent = rInf.GetFont()->GetAscent( rInf.GetVsh(), *rInf.GetOut() );
            }
        }
    }
    else
    {
        SetCntnt( !bTmpDummy );

        // #i3952#
        if ( bIgnoreBlanksAndTabsForLineHeightCalculation &&
             lcl_HasOnlyBlanks( rInf.GetTxt(), rInf.GetLineStart(), rInf.GetLineStart() + GetLen() ) )
        {
            bHasBlankPortion = true;
        }
    }

    // #i3952#
    if ( bHasBlankPortion && bHasOnlyBlankPortions )
    {
        sal_uInt16 nTmpAscent = GetAscent();
        sal_uInt16 nTmpHeight = Height();
        rLine.GetAttrHandler().GetDefaultAscentAndHeight( rInf.GetVsh(), *rInf.GetOut(), nTmpAscent, nTmpHeight );
        SetAscent( nTmpAscent );
        Height( nTmpHeight );
    }

    // Robust:
    if( nLineWidth < Width() )
        Width( nLineWidth );
    SAL_WARN_IF( nLineWidth < Width(), "sw.core", "SwLineLayout::CalcLine: line is bursting" );
    SetDummy( bTmpDummy );
    SetRedline( rLine.GetRedln() &&
        rLine.GetRedln()->CheckLine( rLine.GetStart(), rLine.GetEnd() ) );
}

// #i47162# - add optional parameter <_bNoFlyCntPorAndLinePor>
// to control, if the fly content portions and line portion are considered.
void SwLineLayout::MaxAscentDescent( SwTwips& _orAscent,
                                     SwTwips& _orDescent,
                                     SwTwips& _orObjAscent,
                                     SwTwips& _orObjDescent,
                                     const SwLinePortion* _pDontConsiderPortion,
                                     const bool _bNoFlyCntPorAndLinePor ) const
{
    _orAscent = 0;
    _orDescent = 0;
    _orObjAscent = 0;
    _orObjDescent = 0;

    const SwLinePortion* pTmpPortion = this;
    if ( !pTmpPortion->GetLen() && pTmpPortion->GetPortion() )
    {
        pTmpPortion = pTmpPortion->GetPortion();
    }

    while ( pTmpPortion )
    {
        if ( !pTmpPortion->IsBreakPortion() && !pTmpPortion->IsFlyPortion() &&
             ( !_bNoFlyCntPorAndLinePor ||
               ( !pTmpPortion->IsFlyCntPortion() &&
                 !(pTmpPortion == this && pTmpPortion->GetPortion() ) ) ) )
        {
            SwTwips nPortionAsc = static_cast<SwTwips>(pTmpPortion->GetAscent());
            SwTwips nPortionDesc = static_cast<SwTwips>(pTmpPortion->Height()) -
                                   nPortionAsc;

            const sal_Bool bFlyCmp = pTmpPortion->IsFlyCntPortion() ?
                                     static_cast<const SwFlyCntPortion*>(pTmpPortion)->IsMax() :
                                     !( pTmpPortion == _pDontConsiderPortion );

            if ( bFlyCmp )
            {
                _orObjAscent = Max( _orObjAscent, nPortionAsc );
                _orObjDescent = Max( _orObjDescent, nPortionDesc );
            }

            if ( !pTmpPortion->IsFlyCntPortion() && !pTmpPortion->IsGrfNumPortion() )
            {
                _orAscent = Max( _orAscent, nPortionAsc );
                _orDescent = Max( _orDescent, nPortionDesc );
            }
        }
        pTmpPortion = pTmpPortion->GetPortion();
    }
}

SwCharRange &SwCharRange::operator+=(const SwCharRange &rRange)
{
    if(0 != rRange.nLen ) {
        if(0 == nLen) {
            nStart = rRange.nStart;
            nLen = rRange.nLen ;
        }
        else {
            if(rRange.nStart + rRange.nLen > nStart + nLen) {
                nLen = rRange.nStart + rRange.nLen - nStart;
            }
            if(rRange.nStart < nStart) {
                nLen += nStart - rRange.nStart;
                nStart = rRange.nStart;
            }
        }
    }
    return *this;
}

SwScriptInfo::SwScriptInfo() :
    nInvalidityPos( 0 ),
    nDefaultDir( 0 )
{
};

SwScriptInfo::~SwScriptInfo()
{
}

/*************************************************************************
 * SwScriptInfo::WhichFont()
 *
 * Converts i18n Script Type (LATIN, ASIAN, COMPLEX, WEAK) to
 * Sw Script Types (SW_LATIN, SW_CJK, SW_CTL), used to identify the font
*************************************************************************/

sal_uInt8 SwScriptInfo::WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI )
{
    SAL_WARN_IF( !pTxt && !pSI, "sw.core", "How should I determine the script type?" );
    sal_uInt16 nScript;

    // First we try to use our SwScriptInfo
    if ( pSI )
        nScript = pSI->ScriptType( nIdx );
    else
        // Ok, we have to ask the break iterator
        nScript = pBreakIt->GetRealScriptOfText( *pTxt, nIdx );

    switch ( nScript ) {
        case i18n::ScriptType::LATIN : return SW_LATIN;
        case i18n::ScriptType::ASIAN : return SW_CJK;
        case i18n::ScriptType::COMPLEX : return SW_CTL;
    }

    OSL_FAIL( "Somebody tells lies about the script type!" );
    return SW_LATIN;
}

/*************************************************************************
 * SwScriptInfo::InitScriptInfo()
 *
 * searches for script changes in rTxt and stores them
 *************************************************************************/

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode )
{
    InitScriptInfo( rNode, nDefaultDir == UBIDI_RTL );
}

void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL )
{
    if( !pBreakIt->GetBreakIter().is() )
        return;

    const String& rTxt = rNode.GetTxt();

    //
    // HIDDEN TEXT INFORMATION
    //
    Range aRange( 0, rTxt.Len() ? rTxt.Len() - 1 : 0 );
    MultiSelection aHiddenMulti( aRange );
    CalcHiddenRanges( rNode, aHiddenMulti );

    aHiddenChg.clear();
    sal_uInt16 i = 0;
    for( i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
    {
        const Range& rRange = aHiddenMulti.GetRange( i );
        const xub_StrLen nStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nEnd = (xub_StrLen)rRange.Max() + 1;

        aHiddenChg.push_back( nStart );
        aHiddenChg.push_back( nEnd );
    }

    //
    // SCRIPT AND SCRIPT RELATED INFORMATION
    //

    xub_StrLen nChg = nInvalidityPos;

    // STRING_LEN means the data structure is up to date
    nInvalidityPos = STRING_LEN;

    // this is the default direction
    nDefaultDir = static_cast<sal_uInt8>(bRTL ? UBIDI_RTL : UBIDI_LTR);

    // counter for script info arrays
    sal_uInt16 nCnt = 0;
    // counter for compression information arrays
    sal_uInt16 nCntComp = 0;
    // counter for kashida array
    sal_uInt16 nCntKash = 0;

    sal_uInt8 nScript = i18n::ScriptType::LATIN;

    // compression type
    const SwCharCompressType aCompEnum = rNode.getIDocumentSettingAccess()->getCharacterCompressionType();

    // justification type
    const sal_Bool bAdjustBlock = SVX_ADJUST_BLOCK ==
                                  rNode.GetSwAttrSet().GetAdjust().GetAdjust();

    //
    // FIND INVALID RANGES IN SCRIPT INFO ARRAYS:
    //

    if( nChg )
    {
        // if change position = 0 we do not use any data from the arrays
        // because by deleting all characters of the first group at the beginning
        // of a paragraph nScript is set to a wrong value
        SAL_WARN_IF( !CountScriptChg(), "sw.core", "Where're my changes of script?" );
        while( nCnt < CountScriptChg() )
        {
            if ( nChg > GetScriptChg( nCnt ) )
                nCnt++;
            else
            {
                nScript = GetScriptType( nCnt );
                break;
            }
        }
        if( CHARCOMPRESS_NONE != aCompEnum )
        {
            while( nCntComp < CountCompChg() )
            {
                if ( nChg > GetCompStart( nCntComp ) )
                    nCntComp++;
                else
                    break;
            }
        }
        if ( bAdjustBlock )
        {
            while( nCntKash < CountKashida() )
            {
                if ( nChg > GetKashida( nCntKash ) )
                    nCntKash++;
                else
                    break;
            }
        }
    }

    //
    // ADJUST nChg VALUE:
    //

    // by stepping back one position we know that we are inside a group
    // declared as an nScript group
    if ( nChg )
        --nChg;

    const xub_StrLen nGrpStart = nCnt ? GetScriptChg( nCnt - 1 ) : 0;

    // we go back in our group until we reach the first character of
    // type nScript
    while ( nChg > nGrpStart &&
            nScript != pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg ) )
        --nChg;

    // If we are at the start of a group, we do not trust nScript,
    // we better get nScript from the breakiterator:
    if ( nChg == nGrpStart )
        nScript = (sal_uInt8)pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg );

    //
    // INVALID DATA FROM THE SCRIPT INFO ARRAYS HAS TO BE DELETED:
    //

    // remove invalid entries from script information arrays
    aScriptChanges.erase( aScriptChanges.begin() + nCnt, aScriptChanges.end() );

    // get the start of the last compression group
    sal_uInt16 nLastCompression = nChg;
    if( nCntComp )
    {
        --nCntComp;
        nLastCompression = GetCompStart( nCntComp );
        if( nChg >= nLastCompression + GetCompLen( nCntComp ) )
        {
            nLastCompression = nChg;
            ++nCntComp;
        }
    }

    // remove invalid entries from compression information arrays
    aCompressionChanges.erase(aCompressionChanges.begin() + nCntComp, aCompressionChanges.end() );

    // get the start of the last kashida group
    sal_uInt16 nLastKashida = nChg;
    if( nCntKash && i18n::ScriptType::COMPLEX == nScript )
    {
        --nCntKash;
        nLastKashida = GetKashida( nCntKash );
    }

    // remove invalid entries from kashida array
    aKashida.erase( aKashida.begin() + nCntKash, aKashida.end() );

    //
    // TAKE CARE OF WEAK CHARACTERS: WE MUST FIND AN APPROPRIATE
    // SCRIPT FOR WEAK CHARACTERS AT THE BEGINNING OF A PARAGRAPH
    //

    if( WEAK == pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg ) )
    {
        // If the beginning of the current group is weak, this means that
        // all of the characters in this grounp are weak. We have to assign
        // the scripts to these characters depending on the fonts which are
        // set for these characters to display them.
        xub_StrLen nEnd =
                (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript( rTxt, nChg, WEAK );

        if( nEnd > rTxt.Len() )
            nEnd = rTxt.Len();

        nScript = (sal_uInt8)GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );

        SAL_WARN_IF( i18n::ScriptType::LATIN != nScript &&
                i18n::ScriptType::ASIAN != nScript &&
                i18n::ScriptType::COMPLEX != nScript, "sw.core", "Wrong default language" );

        nChg = nEnd;

        // Get next script type or set to weak in order to exit
        sal_uInt8 nNextScript = ( nEnd < rTxt.Len() ) ?
           (sal_uInt8)pBreakIt->GetBreakIter()->getScriptType( rTxt, nEnd ) :
           (sal_uInt8)WEAK;

        if ( nScript != nNextScript )
        {
            aScriptChanges.push_back( ScriptChangeInfo(nEnd, nScript) );
            nCnt++;
            nScript = nNextScript;
        }
    }

    //
    // UPDATE THE SCRIPT INFO ARRAYS:
    //

    while ( nChg < rTxt.Len() || ( aScriptChanges.empty() && !rTxt.Len() ) )
    {
        SAL_WARN_IF( i18n::ScriptType::WEAK == nScript,
                "sw.core", "Inserting WEAK into SwScriptInfo structure" );
        SAL_WARN_IF( STRING_LEN == nChg, "sw.core", "65K? Strange length of script section" );

        xub_StrLen nSearchStt = nChg;
        nChg = (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript( rTxt, nSearchStt, nScript );

        if ( nChg > rTxt.Len() )
            nChg = rTxt.Len();

        // #i28203#
        // for 'complex' portions, we make sure that a portion does not contain more
        // than one script:
        if( i18n::ScriptType::COMPLEX == nScript )
        {
            const short nScriptType = ScriptTypeDetector::getCTLScriptType( rTxt, nSearchStt );
            xub_StrLen nNextCTLScriptStart = nSearchStt;
            short nCurrentScriptType = nScriptType;
            while( com::sun::star::i18n::CTLScriptType::CTL_UNKNOWN == nCurrentScriptType || nScriptType == nCurrentScriptType )
            {
                nNextCTLScriptStart = (xub_StrLen)ScriptTypeDetector::endOfCTLScriptType( rTxt, nNextCTLScriptStart );
                if( nNextCTLScriptStart < rTxt.Len() && nNextCTLScriptStart < nChg )
                    nCurrentScriptType = ScriptTypeDetector::getCTLScriptType( rTxt, nNextCTLScriptStart );
                else
                    break;
            }
            nChg = Min( nChg, nNextCTLScriptStart );
        }

        // special case for dotted circle since it can be used with complex
        // before a mark, so we want it associated with the mark's script
        if (nChg < rTxt.Len() && nChg > 0 && (i18n::ScriptType::WEAK ==
            pBreakIt->GetBreakIter()->getScriptType(rTxt,nChg - 1)))
        {
            int8_t nType = u_charType(rTxt.GetChar(nChg) );
            if (nType == U_NON_SPACING_MARK || nType == U_ENCLOSING_MARK ||
                nType == U_COMBINING_SPACING_MARK )
            {
                aScriptChanges.push_back( ScriptChangeInfo(nChg-1, nScript) );
            }
            else
            {
                aScriptChanges.push_back( ScriptChangeInfo(nChg, nScript) );
            }
        }
        else
        {
            aScriptChanges.push_back( ScriptChangeInfo(nChg, nScript) );
        }
        ++nCnt;

        // if current script is asian, we search for compressable characters
        // in this range
        if ( CHARCOMPRESS_NONE != aCompEnum &&
             i18n::ScriptType::ASIAN == nScript )
        {
            sal_uInt8 ePrevState = NONE;
            sal_uInt8 eState;
            sal_uInt16 nPrevChg = nLastCompression;

            while ( nLastCompression < nChg )
            {
                sal_Unicode cChar = rTxt.GetChar( nLastCompression );

                // examine current character
                switch ( cChar )
                {
                // Left punctuation found
                case 0x3008: case 0x300A: case 0x300C: case 0x300E:
                case 0x3010: case 0x3014: case 0x3016: case 0x3018:
                case 0x301A: case 0x301D:
                    eState = SPECIAL_LEFT;
                    break;
                // Right punctuation found
                case 0x3001: case 0x3002: case 0x3009: case 0x300B:
                case 0x300D: case 0x300F: case 0x3011: case 0x3015:
                case 0x3017: case 0x3019: case 0x301B: case 0x301E:
                case 0x301F:
                    eState = SPECIAL_RIGHT;
                    break;
                default:
                    eState = static_cast<sal_uInt8>( ( 0x3040 <= cChar && 0x3100 > cChar ) ? KANA : NONE );
                }

                // insert range of compressable characters
                if( ePrevState != eState )
                {
                    if ( ePrevState != NONE )
                    {
                        // insert start and type
                        if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                             ePrevState != KANA )
                        {
                            aCompressionChanges.push_back( CompressionChangeInfo(nPrevChg, nLastCompression - nPrevChg, ePrevState) );
                        }
                    }

                    ePrevState = eState;
                    nPrevChg = nLastCompression;
                }

                nLastCompression++;
            }

            // we still have to examine last entry
            if ( ePrevState != NONE )
            {
                // insert start and type
                if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
                     ePrevState != KANA )
                {
                    aCompressionChanges.push_back( CompressionChangeInfo(nPrevChg, nLastCompression - nPrevChg, ePrevState) );
                }
            }
        }

        // we search for connecting opportunities (kashida)
        else if ( bAdjustBlock && i18n::ScriptType::COMPLEX == nScript )
        {
            SwScanner aScanner( rNode, rNode.GetTxt(), 0, ModelToViewHelper(),
                                i18n::WordType::DICTIONARY_WORD,
                                nLastKashida, nChg );

            // the search has to be performed on a per word base
            while ( aScanner.NextWord() )
            {
                const XubString& rWord = aScanner.GetWord();

                xub_StrLen nIdx = 0;
                xub_StrLen nKashidaPos = STRING_LEN;
                sal_Unicode cCh;
                sal_Unicode cPrevCh = 0;

                sal_uInt16 nPriorityLevel = 7; // 0..6 = level found
                                           // 7 not found

                xub_StrLen nWordLen = rWord.Len();

                // ignore trailing vowel chars
                while( nWordLen && isTransparentChar( rWord.GetChar( nWordLen - 1 )))
                    --nWordLen;

                while (nIdx < nWordLen)
                {
                    cCh = rWord.GetChar( nIdx );

                    // 1. Priority:
                    // after user inserted kashida
                    if ( 0x640 == cCh )
                    {
                        nKashidaPos = aScanner.GetBegin() + nIdx;
                        nPriorityLevel = 0;
                    }

                    // 2. Priority:
                    // after a Seen or Sad
                    if (nPriorityLevel >= 1 && nIdx < nWordLen - 1)
                    {
                        if( isSeenOrSadChar( cCh )
                         && (rWord.GetChar( nIdx+1 ) != 0x200C) ) // #i98410#: prevent ZWNJ expansion
                        {
                            nKashidaPos  = aScanner.GetBegin() + nIdx;
                            nPriorityLevel = 1;
                        }
                    }

                    // 3. Priority:
                    // before final form of Teh Marbuta, Hah, Dal
                    if ( nPriorityLevel >= 2 && nIdx > 0 )
                    {
                        if ( isTehMarbutaChar ( cCh ) || // Teh Marbuta (right joining)
                             isDalChar ( cCh ) ||        // Dal (right joining) final form may appear in the middle of word
                             ( isHahChar ( cCh ) && nIdx == nWordLen - 1))  // Hah (dual joining) only at end of word
                        {

                            SAL_WARN_IF( 0 == cPrevCh, "sw.core", "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 2;
                            }
                        }
                    }

                    // 4. Priority:
                    // before final form of Alef, Lam or Kaf
                    if ( nPriorityLevel >= 3 && nIdx > 0 )
                    {
                        if ( isAlefChar ( cCh ) ||   // Alef (right joining) final form may appear in the middle of word
                             (( isLamChar ( cCh ) || // Lam
                              isKafChar ( cCh )   || // Kaf (both dual joining)
                              isGafChar ( cCh ) )
                              && nIdx == nWordLen - 1))  // only at end of word
                        {
                            SAL_WARN_IF( 0 == cPrevCh, "sw.core", "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 3;
                            }
                        }
                    }

                    // 5. Priority:
                    // before media Bah
                    if ( nPriorityLevel >= 4 && nIdx > 0 && nIdx < nWordLen - 1 )
                    {
                        if ( isBaaChar ( cCh )) // Bah
                        {
                            // check if next character is Reh, Yeh or Alef Maksura
                            sal_Unicode cNextCh = rWord.GetChar( nIdx + 1 );
                            if ( isRehChar ( cNextCh ) || isYehChar ( cNextCh ))
                           {
                                SAL_WARN_IF( 0 == cPrevCh, "sw.core", "No previous character" );
                                // check if character is connectable to previous character,
                                if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                                {
                                    nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                    nPriorityLevel = 4;
                                }
                            }
                        }
                    }

                    // 6. Priority:
                    // before the final form of Waw, Ain, Qaf and Fa
                    if ( nPriorityLevel >= 5 && nIdx > 0 )
                    {
                        if ( isWawChar ( cCh )   || // Wav (right joining)
                                                    // final form may appear in the middle of word
                             (( isAinChar ( cCh ) ||  // Ain (dual joining)
                                isQafChar ( cCh ) ||  // Qaf (dual joining)
                                isFehChar ( cCh ) )   // Feh (dual joining)
                                && nIdx == nWordLen - 1))  // only at end of word
                        {
                            SAL_WARN_IF( 0 == cPrevCh, "sw.core", "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 5;
                            }
                        }
                    }

                    // other connecting possibilities
                    if ( nPriorityLevel >= 6 && nIdx > 0 )
                    {
                        // remaining right joiners
                        // Reh, Zain, Thal,
                        if ( isRehChar ( cCh ) ||   // Reh Zain (right joining)
                                                    // final form may appear in the middle of word
                             ( 0x60C <= cCh && 0x6FE >= cCh // all others
                              && nIdx == nWordLen - 1))   // only at end of word
                        {
                            SAL_WARN_IF( 0 == cPrevCh, "sw.core", "No previous character" );
                            // check if character is connectable to previous character,
                            if ( lcl_ConnectToPrev( cCh, cPrevCh ) )
                            {
                                nKashidaPos = aScanner.GetBegin() + nIdx - 1;
                                nPriorityLevel = 6;
                            }
                        }
                    }

                    // Do not consider Fathatan, Dammatan, Kasratan, Fatha,
                    // Damma, Kasra, Shadda and Sukun when checking if
                    // a character can be connected to previous character.
                    if ( !isTransparentChar ( cCh) )
                        cPrevCh = cCh;

                   ++nIdx;
                } // end of current word

                if ( STRING_LEN != nKashidaPos )
                {
                    aKashida.insert( aKashida.begin() + nCntKash, nKashidaPos);
                    nCntKash++;
                }
            } // end of kashida search
        }

        if ( nChg < rTxt.Len() )
            nScript = (sal_uInt8)pBreakIt->GetBreakIter()->getScriptType( rTxt, nChg );

        nLastCompression = nChg;
        nLastKashida = nChg;
    }

#if OSL_DEBUG_LEVEL > 0
    // check kashida data
    long nTmpKashidaPos = -1;
    sal_Bool bWrongKash = sal_False;
    for (i = 0; i < aKashida.size(); ++i )
    {
        long nCurrKashidaPos = GetKashida( i );
        if ( nCurrKashidaPos <= nTmpKashidaPos )
        {
            bWrongKash = sal_True;
            break;
        }
        nTmpKashidaPos = nCurrKashidaPos;
    }
    SAL_WARN_IF( bWrongKash, "sw.core", "Kashida array contains wrong data" );
#endif

    // remove invalid entries from direction information arrays
    aDirectionChanges.clear();

    // Perform Unicode Bidi Algorithm for text direction information
    bool bPerformUBA = UBIDI_LTR != nDefaultDir;
    nCnt = 0;
    while( !bPerformUBA && nCnt < CountScriptChg() )
    {
        if ( i18n::ScriptType::COMPLEX == GetScriptType( nCnt++ ) )
            bPerformUBA = true;
    }

    // do not call the unicode bidi algorithm if not required
    if ( bPerformUBA )
    {
        UpdateBidiInfo( rTxt );

        // #i16354# Change script type for RTL text to CTL:
        // 1. All text in RTL runs will use the CTL font
        // #i89825# change the script type also to CTL (hennerdrewes)
        // 2. Text in embedded LTR runs that does not have any strong LTR characters (numbers!)
        for ( sal_uInt32 nDirIdx = 0; nDirIdx < aDirectionChanges.size(); ++nDirIdx )
        {
            const sal_uInt8 nCurrDirType = GetDirType( nDirIdx );
                // nStart ist start of RTL run:
                const xub_StrLen nStart = nDirIdx > 0 ? GetDirChg( nDirIdx - 1 ) : 0;
                // nEnd is end of RTL run:
                const xub_StrLen nEnd = GetDirChg( nDirIdx );

            if ( nCurrDirType % 2 == UBIDI_RTL  || // text in RTL run
                ( nCurrDirType > UBIDI_LTR && !lcl_HasStrongLTR( rTxt, nStart, nEnd ) ) ) // non-strong text in embedded LTR run
            {
                // nScriptIdx points into the ScriptArrays:
                size_t nScriptIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                // Make nScriptIdx become the index of the script group with
                // 1. nStartPosOfGroup <= nStart and
                // 2. nEndPosOfGroup > nStart
                while ( GetScriptChg( nScriptIdx ) <= nStart )
                    ++nScriptIdx;

                const xub_StrLen nStartPosOfGroup = nScriptIdx ? GetScriptChg( nScriptIdx - 1 ) : 0;
                const sal_uInt8 nScriptTypeOfGroup = GetScriptType( nScriptIdx );

                SAL_WARN_IF( nStartPosOfGroup > nStart || GetScriptChg( nScriptIdx ) <= nStart,
                        "sw.core", "Script override with CTL font trouble" );

                // Check if we have to insert a new script change at
                // position nStart. If nStartPosOfGroup < nStart,
                // we have to insert a new script change:
                if ( nStart > 0 && nStartPosOfGroup < nStart )
                {
                    aScriptChanges.insert(aScriptChanges.begin() + nScriptIdx,
                                          ScriptChangeInfo(nStart, nScriptTypeOfGroup) );
                    ++nScriptIdx;
                }

                // Remove entries in ScriptArray which end inside the RTL run:
                while ( nScriptIdx < aScriptChanges.size() && GetScriptChg( nScriptIdx ) <= nEnd )
                {
                    aScriptChanges.erase(aScriptChanges.begin() + nScriptIdx);
                }

                // Insert a new entry in ScriptArray for the end of the RTL run:
                aScriptChanges.insert(aScriptChanges.begin() + nScriptIdx,
                                      ScriptChangeInfo(nEnd, i18n::ScriptType::COMPLEX) );

#if OSL_DEBUG_LEVEL > 0
                // Check that ScriptChangeInfos are in increasing order of
                // position and that we don't have "empty" changes.
                sal_uInt8 nLastTyp = i18n::ScriptType::WEAK;
                xub_StrLen nLastPos = 0;
                for (std::vector<ScriptChangeInfo>::const_iterator i2 = aScriptChanges.begin(); i2 < aScriptChanges.end(); ++i2)
                {
                    SAL_WARN_IF( nLastTyp == i2->type ||
                            nLastPos >= i2->position,
                            "sw.core", "Heavy InitScriptType() confusion" );
                    nLastPos = i2->position;
                    nLastTyp = i2->type;
                }
#endif
            }
        }
    }
}

void SwScriptInfo::UpdateBidiInfo( const String& rTxt )
{
    // remove invalid entries from direction information arrays
    aDirectionChanges.clear();
    //
    // Bidi functions from icu 2.0
    //
    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( rTxt.Len(), 0, &nError );
    nError = U_ZERO_ERROR;

    ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(rTxt.GetBuffer()), rTxt.Len(),    // UChar != sal_Unicode in MinGW
                   nDefaultDir, NULL, &nError );
    nError = U_ZERO_ERROR;
    long nCount = ubidi_countRuns( pBidi, &nError );
    int32_t nStart = 0;
    int32_t nEnd;
    UBiDiLevel nCurrDir;
    for ( sal_uInt16 nIdx = 0; nIdx < nCount; ++nIdx )
    {
        ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
        aDirectionChanges.push_back( DirectionChangeInfo(nEnd, nCurrDir) );
        nStart = nEnd;
    }

    ubidi_close( pBidi );
}


/*************************************************************************
 * SwScriptInfo::NextScriptChg(..)
 * returns the position of the next character which belongs to another script
 * than the character of the actual (input) position.
 * If there's no script change until the end of the paragraph, it will return
 * STRING_LEN.
 * Scripts are Asian (Chinese, Japanese, Korean),
 *             Latin ( English etc.)
 *         and Complex ( Hebrew, Arabian )
 *************************************************************************/

xub_StrLen SwScriptInfo::NextScriptChg( const xub_StrLen nPos )  const
{
    sal_uInt16 nEnd = CountScriptChg();
    for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptChg( nX );
    }

    return STRING_LEN;
}

/*************************************************************************
 * SwScriptInfo::ScriptType(..)
 * returns the script of the character at the input position
 *************************************************************************/

sal_uInt8 SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
{
    sal_uInt16 nEnd = CountScriptChg();
    for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    }

    // the default is the application language script
    return (sal_uInt8)GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
}

xub_StrLen SwScriptInfo::NextDirChg( const xub_StrLen nPos,
                                     const sal_uInt8* pLevel )  const
{
    sal_uInt8 nCurrDir = pLevel ? *pLevel : 62;
    sal_uInt16 nEnd = CountDirChg();
    for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) &&
            ( nX + 1 == nEnd || GetDirType( nX + 1 ) <= nCurrDir ) )
            return GetDirChg( nX );
    }

    return STRING_LEN;
}

sal_uInt8 SwScriptInfo::DirType( const xub_StrLen nPos ) const
{
    sal_uInt16 nEnd = CountDirChg();
    for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) )
            return GetDirType( nX );
    }

    return 0;
}

/*************************************************************************
 * SwScriptInfo::MaskHiddenRanges(..)
 * Takes a string and replaced the hidden ranges with cChar.
 **************************************************************************/

sal_uInt16 SwScriptInfo::MaskHiddenRanges( const SwTxtNode& rNode, XubString& rText,
                                       const xub_StrLen nStt, const xub_StrLen nEnd,
                                       const sal_Unicode cChar )
{
    assert(rNode.GetTxt().Len() == rText.Len());

    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    sal_uInt16 nNumOfHiddenChars = 0;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        if ( nHiddenEnd < nStt || nHiddenStart > nEnd )
            continue;

        while ( nHiddenStart < nHiddenEnd && nHiddenStart < nEnd )
        {
            if ( nHiddenStart >= nStt && nHiddenStart < nEnd )
            {
                rText.SetChar( nHiddenStart, cChar );
                ++nNumOfHiddenChars;
            }
            ++nHiddenStart;
        }
    }

    return nNumOfHiddenChars;
}

/*************************************************************************
 * SwScriptInfo::DeleteHiddenRanges(..)
 * Takes a SwTxtNode and deletes the hidden ranges from the node.
 **************************************************************************/

void SwScriptInfo::DeleteHiddenRanges( SwTxtNode& rNode )
{
    PositionList aList;
    xub_StrLen nHiddenStart;
    xub_StrLen nHiddenEnd;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    PositionList::const_reverse_iterator rFirst( aList.end() );
    PositionList::const_reverse_iterator rLast( aList.begin() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        SwPaM aPam( rNode, nHiddenStart, rNode, nHiddenEnd );
        rNode.getIDocumentContentOperations()->DeleteRange( aPam );
    }
}

/*************************************************************************
 * SwScriptInfo::GetBoundsOfHiddenRange(..)
 * static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos,
                                           xub_StrLen& rnStartPos, xub_StrLen& rnEndPos,
                                           PositionList* pList )
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    bool bNewContainsHiddenChars = false;

    //
    // Optimization: First examine the flags at the text node:
    //
    if ( !rNode.IsCalcHiddenCharFlags() )
    {
        bool bWholePara = rNode.HasHiddenCharAttribute( true );
        bool bContainsHiddenChars = rNode.HasHiddenCharAttribute( false );
        if ( !bContainsHiddenChars )
            return false;

        if ( bWholePara )
        {
            if ( pList )
            {
                pList->push_back( 0 );
                pList->push_back( rNode.GetTxt().Len() );
            }

            rnStartPos = 0;
            rnEndPos = rNode.GetTxt().Len();
            return true;
        }
    }

    const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rNode );
    if ( pSI )
    {
        //
        // Check first, if we have a valid SwScriptInfo object for this text node:
        //
        bNewContainsHiddenChars = pSI->GetBoundsOfHiddenRange( nPos, rnStartPos, rnEndPos, pList );
        const bool bNewHiddenCharsHidePara = ( rnStartPos == 0 && rnEndPos >= rNode.GetTxt().Len() );
        rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
    }
    else
    {
        //
        // No valid SwScriptInfo Object, we have to do it the hard way:
        //
        Range aRange( 0, rNode.GetTxt().Len() ? rNode.GetTxt().Len() - 1 : 0 );
        MultiSelection aHiddenMulti( aRange );
        SwScriptInfo::CalcHiddenRanges( rNode, aHiddenMulti );
        for( sal_uInt16 i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
            const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;

            if ( nHiddenStart > nPos )
                break;
            else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
            {
                rnStartPos = nHiddenStart;
                rnEndPos   = Min( nHiddenEnd, rNode.GetTxt().Len() );
                break;
            }
        }

        if ( pList )
        {
            for( sal_uInt16 i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                pList->push_back( (xub_StrLen)rRange.Min() );
                pList->push_back( (xub_StrLen)rRange.Max() + 1 );
            }
        }

        bNewContainsHiddenChars = aHiddenMulti.GetRangeCount() > 0;
    }

    return bNewContainsHiddenChars;
}

/*************************************************************************
 * SwScriptInfo::GetBoundsOfHiddenRange(..)
 * non-static version
 **************************************************************************/

bool SwScriptInfo::GetBoundsOfHiddenRange( xub_StrLen nPos, xub_StrLen& rnStartPos,
                                           xub_StrLen& rnEndPos, PositionList* pList ) const
{
    rnStartPos = STRING_LEN;
    rnEndPos = 0;

    sal_uInt16 nEnd = CountHiddenChg();
    for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
    {
        const xub_StrLen nHiddenStart = GetHiddenChg( nX++ );
        const xub_StrLen nHiddenEnd = GetHiddenChg( nX );

        if ( nHiddenStart > nPos )
            break;
        else if ( nHiddenStart <= nPos && nPos < nHiddenEnd )
        {
            rnStartPos = nHiddenStart;
            rnEndPos   = nHiddenEnd;
            break;
        }
    }

    if ( pList )
    {
        for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
        {
            pList->push_back( GetHiddenChg( nX++ ) );
            pList->push_back( GetHiddenChg( nX ) );
        }
    }

    return CountHiddenChg() > 0;
}

bool SwScriptInfo::IsInHiddenRange( const SwTxtNode& rNode, xub_StrLen nPos )
{
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nPos, nStartPos, nEndPos );
    return nStartPos != STRING_LEN;
}


#ifdef DBG_UTIL
/*************************************************************************
 * SwScriptInfo::CompType(..)
 * returns the type of the compressed character
 *************************************************************************/

sal_uInt8 SwScriptInfo::CompType( const xub_StrLen nPos ) const
{
    sal_uInt16 nEnd = CountCompChg();
    for( sal_uInt16 nX = 0; nX < nEnd; ++nX )
    {
        xub_StrLen nChg = GetCompStart( nX );

        if ( nPos < nChg )
            return NONE;

        if( nPos < nChg + GetCompLen( nX ) )
            return GetCompType( nX );
    }
    return NONE;
}
#endif

/*************************************************************************
 * SwScriptInfo::HasKana()
 * returns, if there are compressable kanas or specials
 * between nStart and nEnd
 *************************************************************************/

sal_uInt16 SwScriptInfo::HasKana( xub_StrLen nStart, const xub_StrLen nLen ) const
{
    sal_uInt16 nCnt = CountCompChg();
    xub_StrLen nEnd = nStart + nLen;

    for( sal_uInt16 nX = 0; nX < nCnt; ++nX )
    {
        xub_StrLen nKanaStart  = GetCompStart( nX );
        xub_StrLen nKanaEnd = nKanaStart + GetCompLen( nX );

        if ( nKanaStart >= nEnd )
            return USHRT_MAX;

        if ( nStart < nKanaEnd )
            return nX;
    }

    return USHRT_MAX;
}

long SwScriptInfo::Compress( sal_Int32* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                             const sal_uInt16 nCompress, const sal_uInt16 nFontHeight,
                             Point* pPoint ) const
{
    SAL_WARN_IF( !nCompress, "sw.core", "Compression without compression?!" );
    SAL_WARN_IF( !nLen, "sw.core", "Compression without text?!" );
    sal_uInt16 nCompCount = CountCompChg();

    // In asian typography, there are full width and half width characters.
    // Full width punctuation characters can be compressed by 50 %
    // to determine this, we compare the font width with 75 % of its height
    sal_uInt16 nMinWidth = ( 3 * nFontHeight ) / 4;

    sal_uInt16 nCompIdx = HasKana( nIdx, nLen );

    if ( USHRT_MAX == nCompIdx )
        return 0;

    xub_StrLen nChg = GetCompStart( nCompIdx );
    xub_StrLen nCompLen = GetCompLen( nCompIdx );
    sal_uInt16 nI = 0;
    nLen = nLen + nIdx;

    if( nChg > nIdx )
    {
        nI = nChg - nIdx;
        nIdx = nChg;
    }
    else if( nIdx < nChg + nCompLen )
        nCompLen -= nIdx - nChg;

    if( nIdx > nLen || nCompIdx >= nCompCount )
        return 0;

    long nSub = 0;
    long nLast = nI ? pKernArray[ nI - 1 ] : 0;
    do
    {
        sal_uInt16 nType = GetCompType( nCompIdx );
#ifdef DBG_UTIL
        SAL_WARN_IF( nType != CompType( nIdx ), "sw.core", "Gimme the right type!" );
#endif
        nCompLen = nCompLen + nIdx;
        if( nCompLen > nLen )
            nCompLen = nLen;

        // are we allowed to compress the character?
        if ( pKernArray[ nI ] - nLast < nMinWidth )
        {
            nIdx++; nI++;
        }
        else
        {
            while( nIdx < nCompLen )
            {
                SAL_WARN_IF( SwScriptInfo::NONE == nType, "sw.core", "None compression?!" );

                // nLast is width of current character
                nLast -= pKernArray[ nI ];

                nLast *= nCompress;
                long nMove = 0;
                if( SwScriptInfo::KANA != nType )
                {
                    nLast /= 20000;
                    if( pPoint && SwScriptInfo::SPECIAL_LEFT == nType )
                    {
                        if( nI )
                            nMove = nLast;
                        else
                        {
                            pPoint->X() += nLast;
                            nLast = 0;
                        }
                    }
                }
                else
                    nLast /= 100000;
                nSub -= nLast;
                nLast = pKernArray[ nI ];
                if( nMove )
                    pKernArray[ nI - 1 ] += nMove;
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }

        if( nIdx < nLen )
        {
            xub_StrLen nTmpChg;
            if( ++nCompIdx < nCompCount )
            {
                nTmpChg = GetCompStart( nCompIdx );
                if( nTmpChg > nLen )
                    nTmpChg = nLen;
                nCompLen = GetCompLen( nCompIdx );
            }
            else
                nTmpChg = nLen;
            while( nIdx < nTmpChg )
            {
                nLast = pKernArray[ nI ];
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }
        else
            break;
    } while( nIdx < nLen );
    return nSub;
}

/*************************************************************************
 * SwScriptInfo::KashidaJustify()
 *************************************************************************/

// Note on calling KashidaJustify():
// Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
// total number of kashida positions, or the number of kashida positions after some positions
// have been dropped, depending on the state of the aKashidaInvalid array.

sal_uInt16 SwScriptInfo::KashidaJustify( sal_Int32* pKernArray,
                                    sal_Int32* pScrArray,
                                    xub_StrLen nStt,
                                    xub_StrLen nLen,
                                    long nSpaceAdd ) const
{
    SAL_WARN_IF( !nLen, "sw.core", "Kashida justification without text?!" );

    if( !IsKashidaLine(nStt))
        return STRING_LEN;

    // evaluate kashida information in collected in SwScriptInfo

    sal_uInt16 nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    sal_uInt16 nCntKashEnd = nCntKash;
    while ( nCntKashEnd < CountKashida() )
    {
       if ( nEnd <= GetKashida( nCntKashEnd ) )
            break;
        else
            nCntKashEnd++;
    }

    sal_uInt16 nActualKashCount = nCntKashEnd - nCntKash;
    for ( sal_uInt16 i = nCntKash; i < nCntKashEnd; ++i )
    {
        if ( nActualKashCount && !IsKashidaValid ( i ) )
            --nActualKashCount;
    }

    if ( !pKernArray )
        return nActualKashCount;

    // do nothing if there is no more kashida
    if ( nCntKash < CountKashida() )
    {
        // skip any invalid kashidas
        while ( ! IsKashidaValid ( nCntKash ) && nCntKash < nCntKashEnd )
            ++nCntKash;

        xub_StrLen nKashidaPos = GetKashida( nCntKash );
        xub_StrLen nIdx = nKashidaPos;
        long nKashAdd = nSpaceAdd;

        while ( nIdx < nEnd )
        {
            sal_uInt16 nArrayPos = nIdx - nStt;

            // next kashida position
            ++nCntKash;
            while ( ! IsKashidaValid ( nCntKash ) && nCntKash < nCntKashEnd )
                ++nCntKash;

            nIdx = nCntKash < CountKashida() && IsKashidaValid ( nCntKash ) ? GetKashida( nCntKash ) : nEnd;
            if ( nIdx > nEnd )
                nIdx = nEnd;

            const sal_uInt16 nArrayEnd = nIdx - nStt;

            while ( nArrayPos < nArrayEnd )
            {
                pKernArray[ nArrayPos ] += nKashAdd;
                if ( pScrArray )
                    pScrArray[ nArrayPos ] += nKashAdd;
                ++nArrayPos;
            }
            nKashAdd += nSpaceAdd;
        }
    }

    return 0;
}

/*************************************************************************
 * SwScriptInfo::IsArabicText()
 *
 * Checks if the current text is 'Arabic' text. Note that only the first
 * character has to be checked because a ctl portion only contains one
 * script, see NewTxtPortion
 *************************************************************************/
bool SwScriptInfo::IsArabicText( const OUString& rTxt, sal_Int32 nStt, sal_Int32 nLen )
{
    using namespace ::com::sun::star::i18n;
    static ScriptTypeList typeList[] = {
        { UnicodeScript_kArabic, UnicodeScript_kArabic, UnicodeScript_kArabic },        // 11,
        { UnicodeScript_kScriptCount, UnicodeScript_kScriptCount, UnicodeScript_kScriptCount }    // 88
    };

    // go forward if current position does not hold a regular character:
    const CharClass& rCC = GetAppCharClass();
    sal_Int32 nIdx = nStt;
    const sal_Int32 nEnd = nStt + nLen;
    while ( nIdx < nEnd && !rCC.isLetterNumeric( rTxt, (xub_StrLen)nIdx ) )
    {
        ++nIdx;
    }

    if( nIdx == nEnd )
    {
        // no regular character found in this portion. Go backward:
        --nIdx;
        while ( nIdx >= 0 && !rCC.isLetterNumeric( rTxt, (xub_StrLen)nIdx ) )
        {
            --nIdx;
        }
    }

    if( nIdx >= 0 )
    {
        const sal_Unicode cCh = rTxt[nIdx];
        const sal_Int16 type = unicode::getUnicodeScriptType( cCh, typeList, UnicodeScript_kScriptCount );
        return type == UnicodeScript_kArabic;
    }
    return false;
}

sal_Bool SwScriptInfo::IsKashidaValid ( xub_StrLen nKashPos ) const
{
    for ( size_t i = 0; i < aKashidaInvalid.size(); ++i )
    {
        if ( aKashidaInvalid [ i ] == nKashPos )
            return false;
    }
    return true;
}

void SwScriptInfo::ClearKashidaInvalid ( xub_StrLen nKashPos )
{
    for ( size_t i = 0; i < aKashidaInvalid.size(); ++i )
    {
        if ( aKashidaInvalid [ i ] == nKashPos )
        {
           aKashidaInvalid.erase ( aKashidaInvalid.begin() + i );
           return;
        }
    }
}

/*************************************************************************
 * SwScriptInfo::MarkOrClearKashidaInvalid()
 *
 * bMark == true:
 * marks the first valid kashida in the given text range as invalid
 *
 * bMark == false:
 * clears all kashida invalid flags in the given text range
*************************************************************************/

bool SwScriptInfo::MarkOrClearKashidaInvalid ( xub_StrLen nStt, xub_StrLen nLen, bool bMark, xub_StrLen nMarkCount )
{
    sal_uInt16 nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    while ( nCntKash < CountKashida() )
    {
        if ( nEnd <= GetKashida( nCntKash ) )
            break;
        else
        {
            if(bMark)
            {
                if ( IsKashidaValid ( nCntKash ) )
                {
                    MarkKashidaInvalid ( nCntKash );
                    --nMarkCount;
                    if(!nMarkCount)
                       return true;
                }
            }
            else
            {
                ClearKashidaInvalid ( nCntKash );
            }
            nCntKash++;
        }
    }
    return false;
}

void SwScriptInfo::MarkKashidaInvalid ( xub_StrLen nKashPos )
{
    aKashidaInvalid.push_back( nKashPos );
}

/*************************************************************************
 * SwScriptInfo::GetKashidaPositions()
 * retrieve the kashida positions in the given text range
*************************************************************************/

sal_uInt16 SwScriptInfo::GetKashidaPositions ( xub_StrLen nStt, xub_StrLen nLen,
                                           xub_StrLen* pKashidaPosition )
{
    sal_uInt16 nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        else
            nCntKash++;
    }

    const xub_StrLen nEnd = nStt + nLen;

    sal_uInt16 nCntKashEnd = nCntKash;
    while ( nCntKashEnd < CountKashida() )
    {
       if ( nEnd <= GetKashida( nCntKashEnd ) )
            break;
        else
        {
            pKashidaPosition [ nCntKashEnd - nCntKash ] = GetKashida ( nCntKashEnd );
            nCntKashEnd++;
        }
    }
    return nCntKashEnd - nCntKash;
}

void SwScriptInfo::SetNoKashidaLine ( xub_StrLen nStt, xub_StrLen nLen )
{
    aNoKashidaLine.push_back( nStt );
    aNoKashidaLineEnd.push_back( nStt+nLen );
}

/*************************************************************************
 * SwScriptInfo::IsKashidaLine()
 * determines if the line uses kashida justification
*************************************************************************/

bool SwScriptInfo::IsKashidaLine ( xub_StrLen nCharIdx ) const
{
   for( size_t i = 0; i < aNoKashidaLine.size(); ++i )
    {
       if( nCharIdx >= aNoKashidaLine[ i ] && nCharIdx < aNoKashidaLineEnd[ i ])
           return false;
    }
   return true;
}

void SwScriptInfo::ClearNoKashidaLine ( xub_StrLen nStt, xub_StrLen nLen )
{
   size_t i = 0;
   while( i < aNoKashidaLine.size())
   {
       if( nStt + nLen >= aNoKashidaLine[ i ] && nStt < aNoKashidaLineEnd [ i ] )
       {
           aNoKashidaLine.erase(aNoKashidaLine.begin() + i);
           aNoKashidaLineEnd.erase(aNoKashidaLineEnd.begin() + i);
       }
       else
           ++i;
   }
}

/*************************************************************************
 * SwScriptInfo::MarkKashidasInvalid()
 *
 * mark the given character indices as invalid kashida positions
************************************************************************/

bool SwScriptInfo::MarkKashidasInvalid ( xub_StrLen nCnt, xub_StrLen* pKashidaPositions )
{
   SAL_WARN_IF( !pKashidaPositions || nCnt == 0, "sw.core", "Where are kashidas?" );

   sal_uInt16 nCntKash = 0;
   xub_StrLen nKashidaPosIdx = 0;

    while ( nCntKash < CountKashida() && nKashidaPosIdx < nCnt )
    {
       if ( pKashidaPositions [nKashidaPosIdx] > GetKashida( nCntKash ) )
       {
           nCntKash++;
           continue;
       }

        if ( pKashidaPositions [nKashidaPosIdx] == GetKashida( nCntKash ) && IsKashidaValid ( nCntKash ) )
       {
            MarkKashidaInvalid ( nCntKash );
       }
       else
           return false; // something is wrong
       nKashidaPosIdx++;
   }
   return true;
}

sal_Int32 SwScriptInfo::ThaiJustify( const OUString& rTxt, sal_Int32* pKernArray,
                                  sal_Int32* pScrArray, sal_Int32 nStt,
                                  sal_Int32 nLen, sal_Int32 nNumberOfBlanks,
                                  long nSpaceAdd )
{
    SAL_WARN_IF( nStt + nLen > rTxt.getLength(), "sw.core", "String in ThaiJustify too small" );

    SwTwips nNumOfTwipsToDistribute = nSpaceAdd * nNumberOfBlanks /
                                      SPACING_PRECISION_FACTOR;

    long nSpaceSum = 0;
    sal_Int32 nCnt = 0;

    for (sal_Int32 nI = 0; nI < nLen; ++nI)
    {
        const sal_Unicode cCh = rTxt[nStt + nI];

        // check if character is not above or below base
        if ( ( 0xE34 > cCh || cCh > 0xE3A ) &&
             ( 0xE47 > cCh || cCh > 0xE4E ) && cCh != 0xE31 )
        {
            if ( nNumberOfBlanks > 0 )
            {
                nSpaceAdd = nNumOfTwipsToDistribute / nNumberOfBlanks;
                --nNumberOfBlanks;
                nNumOfTwipsToDistribute -= nSpaceAdd;
            }
            nSpaceSum += nSpaceAdd;
            ++nCnt;
        }

        if ( pKernArray ) pKernArray[ nI ] += nSpaceSum;
        if ( pScrArray ) pScrArray[ nI ] += nSpaceSum;
    }

    return nCnt;
}

SwScriptInfo* SwScriptInfo::GetScriptInfo( const SwTxtNode& rTNd,
                                           sal_Bool bAllowInvalid )
{
    SwIterator<SwTxtFrm,SwTxtNode> aIter( rTNd );
    SwScriptInfo* pScriptInfo = 0;

    for( SwTxtFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
            pScriptInfo = (SwScriptInfo*)pLast->GetScriptInfo();
            if ( pScriptInfo )
            {
                if ( !bAllowInvalid && STRING_LEN != pScriptInfo->GetInvalidity() )
                    pScriptInfo = 0;
                else break;
            }
        }

    return pScriptInfo;
}

SwParaPortion::SwParaPortion()
{
    FormatReset();
    bFlys = bFtnNum = bMargin = sal_False;
    SetWhichPor( POR_PARA );
}

SwParaPortion::~SwParaPortion()
{
}

xub_StrLen SwParaPortion::GetParLen() const
{
    xub_StrLen nLen = 0;
    const SwLineLayout *pLay = this;
    while( pLay )
    {
        nLen = nLen + pLay->GetLen();
        pLay = pLay->GetNext();
    }
    return nLen;
}

const SwDropPortion *SwParaPortion::FindDropPortion() const
{
    const SwLineLayout *pLay = this;
    while( pLay && pLay->IsDummy() )
        pLay = pLay->GetNext();
    while( pLay )
    {
        const SwLinePortion *pPos = pLay->GetPortion();
        while ( pPos && !pPos->GetLen() )
            pPos = pPos->GetPortion();
        if( pPos && pPos->IsDropPortion() )
            return (SwDropPortion *)pPos;
        pLay = pLay->GetLen() ? NULL : pLay->GetNext();
    }
    return NULL;
}

void SwLineLayout::Init( SwLinePortion* pNextPortion )
{
    Height( 0 );
    Width( 0 );
    SetLen( 0 );
    SetAscent( 0 );
    SetRealHeight( 0 );
    SetPortion( pNextPortion );
}

/*--------------------------------------------------
 * HangingMargin()
 * looks for hanging punctuation portions in the paragraph
 * and return the maximum right offset of them.
 * If no such portion is found, the Margin/Hanging-flags will be updated.
 * --------------------------------------------------*/

SwTwips SwLineLayout::_GetHangingMargin() const
{
    SwLinePortion* pPor = GetPortion();
    sal_Bool bFound = sal_False;
    SwTwips nDiff = 0;
    while( pPor)
    {
        if( pPor->IsHangingPortion() )
        {
            nDiff = ((SwHangingPortion*)pPor)->GetInnerWidth() - pPor->Width();
            if( nDiff )
                bFound = sal_True;
        }
        // the last post its portion
        else if ( pPor->IsPostItsPortion() && ! pPor->GetPortion() )
            nDiff = nAscent;

        pPor = pPor->GetPortion();
    }
    if( !bFound ) // update the hanging-flag
        ((SwLineLayout*)this)->SetHanging( sal_False );
    return nDiff;
}

SwTwips SwTxtFrm::HangingMargin() const
{
    SAL_WARN_IF( !HasPara(), "sw.core", "Don't call me without a paraportion" );
    if( !GetPara()->IsMargin() )
        return 0;
    const SwLineLayout* pLine = GetPara();
    SwTwips nRet = 0;
    do
    {
        SwTwips nDiff = pLine->GetHangingMargin();
        if( nDiff > nRet )
            nRet = nDiff;
        pLine = pLine->GetNext();
    } while ( pLine );
    if( !nRet ) // update the margin-flag
        ((SwParaPortion*)GetPara())->SetMargin( sal_False );
    return nRet;
}

void SwScriptInfo::selectHiddenTextProperty(const SwTxtNode& rNode, MultiSelection &rHiddenMulti)
{
    assert((!rNode.GetTxt().Len() && rHiddenMulti.GetTotalRange().Len() == 1) ||
           (rNode.GetTxt().Len() == rHiddenMulti.GetTotalRange().Len()));

    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState( RES_CHRATR_HIDDEN, sal_True, &pItem ) &&
        ((SvxCharHiddenItem*)pItem)->GetValue() )
    {
        rHiddenMulti.SelectAll();
    }

    const SwpHints* pHints = rNode.GetpSwpHints();

    if( pHints )
    {
        MSHORT nTmp = 0;

        while( nTmp < pHints->GetStartCount() )
        {
            const SwTxtAttr* pTxtAttr = pHints->GetStart( nTmp++ );
            const SvxCharHiddenItem* pHiddenItem =
                static_cast<const SvxCharHiddenItem*>( CharFmt::GetItem( *pTxtAttr, RES_CHRATR_HIDDEN ) );
            if( pHiddenItem )
            {
                xub_StrLen nSt = *pTxtAttr->GetStart();
                xub_StrLen nEnd = *pTxtAttr->GetEnd();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    rHiddenMulti.Select( aTmp, pHiddenItem->GetValue() );
                }
            }
        }
    }
}

void SwScriptInfo::selectRedLineDeleted(const SwTxtNode& rNode, MultiSelection &rHiddenMulti, bool bSelect)
{
    assert((!rNode.GetTxt().Len() && rHiddenMulti.GetTotalRange().Len() == 1) ||
           (rNode.GetTxt().Len() == rHiddenMulti.GetTotalRange().Len()));

    const IDocumentRedlineAccess& rIDRA = *rNode.getIDocumentRedlineAccess();
    if ( IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineMode() ) )
    {
        sal_uInt16 nAct = rIDRA.GetRedlinePos( rNode, USHRT_MAX );

        for ( ; nAct < rIDRA.GetRedlineTbl().size(); nAct++ )
        {
            const SwRedline* pRed = rIDRA.GetRedlineTbl()[ nAct ];

            if ( pRed->Start()->nNode > rNode.GetIndex() )
                break;

            xub_StrLen nRedlStart;
            xub_StrLen nRedlnEnd;
            pRed->CalcStartEnd( rNode.GetIndex(), nRedlStart, nRedlnEnd );
            //clip it if the redline extends past the end of the nodes text
            nRedlnEnd = std::min(nRedlnEnd, rNode.GetTxt().Len());
            if ( nRedlnEnd > nRedlStart )
            {
                Range aTmp( nRedlStart, nRedlnEnd - 1 );
                rHiddenMulti.Select( aTmp, bSelect );
            }
        }
    }
}

/*************************************************************************
 * SwScriptInfo::CalcHiddenRanges()
 *
 * Returns a MultiSection indicating the hidden ranges.
 *************************************************************************/

void SwScriptInfo::CalcHiddenRanges( const SwTxtNode& rNode, MultiSelection& rHiddenMulti )
{
    selectHiddenTextProperty(rNode, rHiddenMulti);

    // If there are any hidden ranges in the current text node, we have
    // to unhide the redlining ranges:
    selectRedLineDeleted(rNode, rHiddenMulti, false);

    //
    // We calculated a lot of stuff. Finally we can update the flags at the text node.
    //
    const bool bNewContainsHiddenChars = rHiddenMulti.GetRangeCount() > 0;
    bool bNewHiddenCharsHidePara = false;
    if ( bNewContainsHiddenChars )
    {
        const Range& rRange = rHiddenMulti.GetRange( 0 );
        const xub_StrLen nHiddenStart = (xub_StrLen)rRange.Min();
        const xub_StrLen nHiddenEnd = (xub_StrLen)rRange.Max() + 1;
        bNewHiddenCharsHidePara = ( nHiddenStart == 0 && nHiddenEnd >= rNode.GetTxt().Len() );
    }
    rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
