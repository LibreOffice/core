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

#include "porlay.hxx"
#include "itrform2.hxx"
#include "porglue.hxx"
#include "porexp.hxx"
#include <blink.hxx>
#include "redlnitr.hxx"
#include "porfly.hxx"
#include "porrst.hxx"
#include "pormulti.hxx"
#include "pordrop.hxx"
#include <breakit.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CTLScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <paratr.hxx>
#include <sal/log.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <vcl/outdev.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/blinkitem.hxx>
#include <tools/multisel.hxx>
#include <unotools/charclass.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <section.hxx>
#include <calbck.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <IMark.hxx>

using namespace ::com::sun::star;
using namespace i18n::ScriptType;

#include <unicode/ubidi.h>
#include <i18nutil/scripttypedetector.hxx>
#include <i18nutil/unicode.hxx>

#define IS_JOINING_GROUP(c, g) ( u_getIntPropertyValue( (c), UCHAR_JOINING_GROUP ) == U_JG_##g )
#define isAinChar(c)        IS_JOINING_GROUP((c), AIN)
#define isAlefChar(c)       IS_JOINING_GROUP((c), ALEF)
#define isDalChar(c)        IS_JOINING_GROUP((c), DAL)
#if U_ICU_VERSION_MAJOR_NUM >= 58
#define isFehChar(c)       (IS_JOINING_GROUP((c), FEH) || IS_JOINING_GROUP((c), AFRICAN_FEH))
#else
#define isFehChar(c)        IS_JOINING_GROUP((c), FEH)
#endif
#define isGafChar(c)        IS_JOINING_GROUP((c), GAF)
#define isHehChar(c)        IS_JOINING_GROUP((c), HEH)
#define isKafChar(c)        IS_JOINING_GROUP((c), KAF)
#define isLamChar(c)        IS_JOINING_GROUP((c), LAM)
#if U_ICU_VERSION_MAJOR_NUM >= 58
#define isQafChar(c)       (IS_JOINING_GROUP((c), QAF) || IS_JOINING_GROUP((c), AFRICAN_QAF))
#else
#define isQafChar(c)        IS_JOINING_GROUP((c), QAF)
#endif
#define isRehChar(c)        IS_JOINING_GROUP((c), REH)
#define isTahChar(c)        IS_JOINING_GROUP((c), TAH)
#define isTehMarbutaChar(c) IS_JOINING_GROUP((c), TEH_MARBUTA)
#define isWawChar(c)        IS_JOINING_GROUP((c), WAW)
#define isSeenOrSadChar(c)  (IS_JOINING_GROUP((c), SAD) || IS_JOINING_GROUP((c), SEEN))

// Beh and charters that behave like Beh in medial form.
static bool isBehChar(sal_Unicode cCh)
{
    bool bRet = false;
    switch (u_getIntPropertyValue(cCh, UCHAR_JOINING_GROUP))
    {
    case U_JG_BEH:
    case U_JG_NOON:
#if U_ICU_VERSION_MAJOR_NUM >= 58
    case U_JG_AFRICAN_NOON:
#endif
    case U_JG_NYA:
    case U_JG_YEH:
    case U_JG_FARSI_YEH:
    case U_JG_BURUSHASKI_YEH_BARREE:
        bRet = true;
        break;
    default:
        bRet = false;
        break;
    }

    return bRet;
}

// Yeh and charters that behave like Yeh in final form.
static bool isYehChar(sal_Unicode cCh)
{
    bool bRet = false;
    switch (u_getIntPropertyValue(cCh, UCHAR_JOINING_GROUP))
    {
    case U_JG_YEH:
    case U_JG_FARSI_YEH:
    case U_JG_YEH_BARREE:
    case U_JG_BURUSHASKI_YEH_BARREE:
    case U_JG_YEH_WITH_TAIL:
        bRet = true;
        break;
    default:
        bRet = false;
        break;
    }

    return bRet;
}

static bool isTransparentChar ( sal_Unicode cCh )
{
    return u_getIntPropertyValue( cCh, UCHAR_JOINING_TYPE ) == U_JT_TRANSPARENT;
}

// Checks if cCh + cNectCh builds a ligature (used for Kashidas)
static bool lcl_IsLigature( sal_Unicode cCh, sal_Unicode cNextCh )
{
            // Lam + Alef
    return ( isLamChar ( cCh ) && isAlefChar ( cNextCh ));
}

// Checks if cCh is connectable to cPrevCh (used for Kashidas)
static bool lcl_ConnectToPrev( sal_Unicode cCh, sal_Unicode cPrevCh )
{
    const int32_t nJoiningType = u_getIntPropertyValue( cPrevCh, UCHAR_JOINING_TYPE );
    bool bRet = nJoiningType != U_JT_RIGHT_JOINING && nJoiningType != U_JT_NON_JOINING;

    // check for ligatures cPrevChar + cChar
    if( bRet )
        bRet = !lcl_IsLigature( cPrevCh, cCh );

    return bRet;
}

static  bool lcl_HasStrongLTR ( const OUString& rText, sal_Int32 nStart, sal_Int32 nEnd )
 {
     for( sal_Int32 nCharIdx = nStart; nCharIdx < nEnd; ++nCharIdx )
     {
         const UCharDirection nCharDir = u_charDirection ( rText[ nCharIdx ] );
         if ( nCharDir == U_LEFT_TO_RIGHT ||
              nCharDir == U_LEFT_TO_RIGHT_EMBEDDING ||
              nCharDir == U_LEFT_TO_RIGHT_OVERRIDE )
             return true;
     }
     return false;
 }

// This is (meant to be) functionally equivalent to 'delete m_pNext' where
// deleting a SwLineLayout recursively deletes the owned m_pNext SwLineLayout.
//
// Here, instead of using a potentially deep stack, iterate over all the
// SwLineLayouts that would be deleted recursively and delete them linearly
void SwLineLayout::DeleteNext()
{
    if (!m_pNext)
        return;
    std::vector<SwLineLayout*> aNexts;
    SwLineLayout* pNext = m_pNext;
    do
    {
        aNexts.push_back(pNext);
        SwLineLayout* pLastNext = pNext;
        pNext = pNext->GetNext();
        pLastNext->SetNext(nullptr);
    }
    while (pNext);
    for (auto a : aNexts)
        delete a;
}

// class SwLineLayout: This is the layout of a single line, which is made
// up of its dimension, the character count and the word spacing in the line.
// Line objects are managed in an own pool, in order to store them continuously
// in memory so that they are paged out together and don't fragment memory.
SwLineLayout::~SwLineLayout()
{
    Truncate();
    DeleteNext();
    if( pBlink )
        pBlink->Delete( this );
    m_pLLSpaceAdd.reset();
    m_pKanaComp.reset();
}

SwLinePortion *SwLineLayout::Insert( SwLinePortion *pIns )
{
    // First attribute change: copy mass and length from *pIns into the first
    // text portion
    if( !mpNextPortion )
    {
        if( GetLen() )
        {
            mpNextPortion = SwTextPortion::CopyLinePortion(*this);
            if( IsBlinking() && pBlink )
            {
                SetBlinking( false );
                pBlink->Replace( this, mpNextPortion );
            }
        }
        else
        {
            SetNextPortion( pIns );
            return pIns;
        }
    }
    // Call with scope or we'll end up with recursion!
    return mpNextPortion->SwLinePortion::Insert( pIns );
}

SwLinePortion *SwLineLayout::Append( SwLinePortion *pIns )
{
    // First attribute change: copy mass and length from *pIns into the first
    // text portion
    if( !mpNextPortion )
        mpNextPortion = SwTextPortion::CopyLinePortion(*this);
    // Call with scope or we'll end up with recursion!
    return mpNextPortion->SwLinePortion::Append( pIns );
}

// For special treatment of empty lines

bool SwLineLayout::Format( SwTextFormatInfo &rInf )
{
    if( GetLen() )
        return SwTextPortion::Format( rInf );

    Height( rInf.GetTextHeight() );
    return true;
}

// We collect all FlyPortions at the beginning of the line and make that a
// MarginPortion.
SwMarginPortion *SwLineLayout::CalcLeftMargin()
{
    SwMarginPortion *pLeft = (GetNextPortion() && GetNextPortion()->IsMarginPortion()) ?
        static_cast<SwMarginPortion *>(GetNextPortion()) : nullptr;
    if( !GetNextPortion() )
         SetNextPortion(SwTextPortion::CopyLinePortion(*this));
    if( !pLeft )
    {
        pLeft = new SwMarginPortion;
        pLeft->SetNextPortion( GetNextPortion() );
        SetNextPortion( pLeft );
    }
    else
    {
        pLeft->Height( 0 );
        pLeft->Width( 0 );
        pLeft->SetLen(TextFrameIndex(0));
        pLeft->SetAscent( 0 );
        pLeft->SetNextPortion( nullptr );
        pLeft->SetFixWidth(0);
    }

    SwLinePortion *pPos = pLeft->GetNextPortion();
    while( pPos )
    {
        if( pPos->IsFlyPortion() )
        {
            // The FlyPortion gets sucked out...
            pLeft->Join( static_cast<SwGluePortion*>(pPos) );
            pPos = pLeft->GetNextPortion();
            if( GetpKanaComp() && !GetKanaComp().empty() )
                GetKanaComp().pop_front();
        }
        else
            pPos = nullptr;
    }
    return pLeft;
}

void SwLineLayout::InitSpaceAdd()
{
    if ( !m_pLLSpaceAdd )
        CreateSpaceAdd();
    else
        SetLLSpaceAdd( 0, 0 );
}

void SwLineLayout::CreateSpaceAdd( const long nInit )
{
    m_pLLSpaceAdd.reset( new std::vector<long> );
    SetLLSpaceAdd( nInit, 0 );
}

// Returns true if there are only blanks in [nStt, nEnd[
static bool lcl_HasOnlyBlanks(const OUString& rText, TextFrameIndex nStt, TextFrameIndex nEnd)
{
    bool bBlankOnly = true;
    while ( nStt < nEnd )
    {
        const sal_Unicode cChar = rText[ sal_Int32(nStt++) ];
        if ( ' ' != cChar && 0x3000 != cChar )
        {
            bBlankOnly = false;
            break;
        }
    }
    return bBlankOnly;
}

// Swapped out from FormatLine()
void SwLineLayout::CalcLine( SwTextFormatter &rLine, SwTextFormatInfo &rInf )
{
    const sal_uInt16 nLineWidth = rInf.RealWidth();

    sal_uInt16 nFlyAscent = 0;
    sal_uInt16 nFlyHeight = 0;
    sal_uInt16 nFlyDescent = 0;
    bool bOnlyPostIts = true;
    SetHanging( false );

    bool bTmpDummy = !GetLen();
    SwFlyCntPortion* pFlyCnt = nullptr;
    if( bTmpDummy )
    {
        nFlyAscent = 0;
        nFlyHeight = 0;
        nFlyDescent = 0;
    }

    // #i3952#
    const bool bIgnoreBlanksAndTabsForLineHeightCalculation =
        rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
            DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION);

    bool bHasBlankPortion = false;
    bool bHasOnlyBlankPortions = true;

    if( mpNextPortion )
    {
        SetContent( false );
        if( mpNextPortion->IsBreakPortion() )
        {
            SetLen( mpNextPortion->GetLen() );
            if( GetLen() )
                bTmpDummy = false;
        }
        else
        {
            const sal_uInt16 nLineHeight = Height();
            Init( GetNextPortion() );
            SwLinePortion *pPos = mpNextPortion;
            SwLinePortion *pLast = this;
            sal_uInt16 nMaxDescent = 0;

            // A group is a segment in the portion chain of pCurr or a fixed
            // portion spanning to the end or the next fixed portion
            while( pPos )
            {
                SAL_WARN_IF( PortionType::NONE == pPos->GetWhichPor(),
                        "sw.core", "SwLineLayout::CalcLine: don't use SwLinePortions !" );

                // Null portions are eliminated. They can form if two FlyFrames
                // overlap.
                if( !pPos->Compress() )
                {
                    // Only take over Height and Ascent if the rest of the line
                    // is empty.
                    if( !pPos->GetNextPortion() )
                    {
                        if( !Height() )
                            Height( pPos->Height() );
                        if( !GetAscent() )
                            SetAscent( pPos->GetAscent() );
                    }
                    delete pLast->Cut( pPos );
                    pPos = pLast->GetNextPortion();
                    continue;
                }

                TextFrameIndex const nPorSttIdx = rInf.GetLineStart() + nLineLength;
                nLineLength += pPos->GetLen();
                AddPrtWidth( pPos->Width() );

                // #i3952#
                if ( bIgnoreBlanksAndTabsForLineHeightCalculation )
                {
                    if ( pPos->InTabGrp() || pPos->IsHolePortion() ||
                            ( pPos->IsTextPortion() &&
                              lcl_HasOnlyBlanks( rInf.GetText(), nPorSttIdx, nPorSttIdx + pPos->GetLen() ) ) )
                    {
                        pLast = pPos;
                        pPos = pPos->GetNextPortion();
                        bHasBlankPortion = true;
                        continue;
                    }
                }

                // Ignore drop portion height
                if( pPos->IsDropPortion() && static_cast<SwDropPortion*>(pPos)->GetLines() > 1)
                {
                    pLast = pPos;
                    pPos = pPos->GetNextPortion();
                    continue;
                }

                bHasOnlyBlankPortions = false;

                // We had an attribute change: Sum up/build maxima of length and mass

                sal_uInt16 nPosHeight = pPos->Height();
                sal_uInt16 nPosAscent = pPos->GetAscent();

                SAL_WARN_IF( nPosHeight < nPosAscent,
                        "sw.core", "SwLineLayout::CalcLine: bad ascent or height" );

                if( pPos->IsHangingPortion() )
                {
                    SetHanging(true);
                    rInf.GetParaPortion()->SetMargin();
                }

                // To prevent that a paragraph-end-character does not change
                // the line height through a Descent and thus causing the line
                // to reformat.
                if ( !pPos->IsBreakPortion() || !Height() )
                {
                    if (!pPos->IsPostItsPortion()) bOnlyPostIts = false;

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
                                sal_uInt16 nTmp = rInf.GetFont()->GetAscent(
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
                            if (bIgnoreBlanksAndTabsForLineHeightCalculation && pPos->GetWhichPor() == PortionType::FlyCnt)
                                // Compat flag set: take the line height, if it's larger.
                                Height(std::max(nPosHeight, nLineHeight));
                            else
                                // Just care about the portion height.
                                Height(nPosHeight);
                        }
                        SwFlyCntPortion* pAsFly(nullptr);
                        if(pPos->IsFlyCntPortion())
                            pAsFly = static_cast<SwFlyCntPortion*>(pPos);
                        if( pAsFly || ( pPos->IsMultiPortion()
                            && static_cast<SwMultiPortion*>(pPos)->HasFlyInContent() ) )
                            rLine.SetFlyInCntBase();
                        if(pAsFly && pAsFly->GetAlign() != sw::LineAlign::NONE)
                        {
                            pAsFly->SetMax(false);
                            if( !pFlyCnt || pPos->Height() > pFlyCnt->Height() )
                                pFlyCnt = pAsFly;
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
                    bTmpDummy = false;

                if( !HasContent() && !pPos->InNumberGrp() )
                {
                    if ( pPos->InExpGrp() )
                    {
                        OUString aText;
                        if( pPos->GetExpText( rInf, aText ) && !aText.isEmpty() )
                            SetContent(true);
                    }
                    else if( ( pPos->InTextGrp() || pPos->IsMultiPortion() ) &&
                             pPos->GetLen() )
                        SetContent(true);
                }

                bTmpDummy &= !HasContent() && ( !pPos->Width() || pPos->IsFlyPortion() );

                pLast = pPos;
                pPos = pPos->GetNextPortion();
            }

            if( pFlyCnt )
            {
                if( pFlyCnt->Height() == Height() )
                {
                    pFlyCnt->SetMax( true );
                    if( Height() > nMaxDescent + nAscent )
                    {
                        if( sw::LineAlign::BOTTOM == pFlyCnt->GetAlign() )
                            nAscent = Height() - nMaxDescent;
                        else if( sw::LineAlign::CENTER == pFlyCnt->GetAlign() )
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
        SetContent( !bTmpDummy );

        // #i3952#
        if ( bIgnoreBlanksAndTabsForLineHeightCalculation &&
             lcl_HasOnlyBlanks( rInf.GetText(), rInf.GetLineStart(), rInf.GetLineStart() + GetLen() ) )
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
    std::pair<SwTextNode const*, sal_Int32> const start(
            rInf.GetTextFrame()->MapViewToModel(rLine.GetStart()));
    std::pair<SwTextNode const*, sal_Int32> const end(
            rInf.GetTextFrame()->MapViewToModel(rLine.GetEnd()));
    SetRedline( rLine.GetRedln() &&
        rLine.GetRedln()->CheckLine(start.first->GetIndex(), start.second,
            end.first->GetIndex(), end.second) );
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
    if ( !pTmpPortion->GetLen() && pTmpPortion->GetNextPortion() )
    {
        pTmpPortion = pTmpPortion->GetNextPortion();
    }

    while ( pTmpPortion )
    {
        if ( !pTmpPortion->IsBreakPortion() && !pTmpPortion->IsFlyPortion() &&
             ( !_bNoFlyCntPorAndLinePor ||
               ( !pTmpPortion->IsFlyCntPortion() &&
                 !(pTmpPortion == this && pTmpPortion->GetNextPortion() ) ) ) )
        {
            SwTwips nPortionAsc = static_cast<SwTwips>(pTmpPortion->GetAscent());
            SwTwips nPortionDesc = static_cast<SwTwips>(pTmpPortion->Height()) -
                                   nPortionAsc;

            const bool bFlyCmp = pTmpPortion->IsFlyCntPortion() ?
                                     static_cast<const SwFlyCntPortion*>(pTmpPortion)->IsMax() :
                                     ( pTmpPortion != _pDontConsiderPortion );

            if ( bFlyCmp )
            {
                _orObjAscent = std::max( _orObjAscent, nPortionAsc );
                _orObjDescent = std::max( _orObjDescent, nPortionDesc );
            }

            if ( !pTmpPortion->IsFlyCntPortion() && !pTmpPortion->IsGrfNumPortion() )
            {
                _orAscent = std::max( _orAscent, nPortionAsc );
                _orDescent = std::max( _orDescent, nPortionDesc );
            }
        }
        pTmpPortion = pTmpPortion->GetNextPortion();
    }
}

void SwLineLayout::ResetFlags()
{
    m_bFormatAdj = m_bDummy = m_bEndHyph = m_bMidHyph = m_bFly
    = m_bRest = m_bBlinking = m_bClipping = m_bContent = m_bRedline
    = m_bForcedLeftMargin = m_bHanging = false;
}

SwLineLayout::SwLineLayout()
    : m_pNext( nullptr ), m_nRealHeight( 0 ),
      m_bUnderscore( false )
{
    ResetFlags();
    SetWhichPor( PortionType::Lay );
}

SwLinePortion *SwLineLayout::GetFirstPortion() const
{
    const SwLinePortion *pRet = mpNextPortion ? mpNextPortion : this;
    return const_cast<SwLinePortion*>(pRet);
}

SwCharRange &SwCharRange::operator+=(const SwCharRange &rRange)
{
    if (TextFrameIndex(0) != rRange.nLen)
    {
        if (TextFrameIndex(0) == nLen) {
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

SwScriptInfo::SwScriptInfo()
    : m_nInvalidityPos(0)
    , m_nDefaultDir(0)
{
};

SwScriptInfo::~SwScriptInfo()
{
}

// Converts i18n Script Type (LATIN, ASIAN, COMPLEX, WEAK) to
// Sw Script Types (SwFontScript::Latin, SwFontScript::CJK, SwFontScript::CTL), used to identify the font
static SwFontScript lcl_ScriptToFont(sal_uInt16 const nScript)
{
    switch ( nScript ) {
        case i18n::ScriptType::LATIN : return SwFontScript::Latin;
        case i18n::ScriptType::ASIAN : return SwFontScript::CJK;
        case i18n::ScriptType::COMPLEX : return SwFontScript::CTL;
    }

    OSL_FAIL( "Somebody tells lies about the script type!" );
    return SwFontScript::Latin;
}

SwFontScript SwScriptInfo::WhichFont(TextFrameIndex const nIdx) const
{
    const sal_uInt16 nScript(ScriptType(nIdx));
    return lcl_ScriptToFont(nScript);
}

SwFontScript SwScriptInfo::WhichFont(sal_Int32 nIdx, OUString const& rText)
{
    const sal_uInt16 nScript(g_pBreakIt->GetRealScriptOfText(rText, nIdx));
    return lcl_ScriptToFont(nScript);
}

// searches for script changes in rText and stores them
void SwScriptInfo::InitScriptInfo(const SwTextNode& rNode,
        sw::MergedPara const*const pMerged)
{
    InitScriptInfo( rNode, pMerged, m_nDefaultDir == UBIDI_RTL );
}

void SwScriptInfo::InitScriptInfo(const SwTextNode& rNode,
        sw::MergedPara const*const pMerged, bool bRTL)
{
    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

    const OUString& rText(pMerged ? pMerged->mergedText : rNode.GetText());

    // HIDDEN TEXT INFORMATION

    m_HiddenChg.clear();
    if (pMerged)
    {
        SwTextNode const* pNode(nullptr);
        TextFrameIndex nOffset(0);
        for (auto iter = pMerged->extents.begin(); iter != pMerged->extents.end(); ++iter)
        {
            if (iter->pNode == pNode)
            {
                nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                continue; // skip extents at end of previous node
            }
            pNode = iter->pNode;
            Range aRange( 0, pNode->Len() > 0 ? pNode->Len() - 1 : 0 );
            MultiSelection aHiddenMulti( aRange );
            CalcHiddenRanges( *pNode, aHiddenMulti );

            for (sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                const sal_Int32 nStart = rRange.Min();
                const sal_Int32 nEnd = rRange.Max() + 1;

                while (true)
                {
                    // because of the selectRedLineDeleted call, never overlaps
                    // extents, must be contained inside one extent
                    assert(!(iter->nStart <= nStart && nStart < iter->nEnd && iter->nEnd < nEnd));
                    assert(!(nStart < iter->nStart && iter->nStart < nEnd && nEnd <= iter->nEnd));
                    if (iter->nStart <= nStart && nEnd <= iter->nEnd)
                    {
                        if (iter->nStart == nStart && !m_HiddenChg.empty()
                            && m_HiddenChg.back() == nOffset)
                        {
                            // previous one went until end of extent, extend it
                            m_HiddenChg.back() += TextFrameIndex(nEnd - iter->nStart);
                        }
                        else // new one
                        {
                            m_HiddenChg.push_back(nOffset + TextFrameIndex(nStart - iter->nStart));
                            m_HiddenChg.push_back(nOffset + TextFrameIndex(nEnd - iter->nStart));
                        }
                        break;
                    }
                    else
                    {
                        nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                        ++iter;
                        // because selectRedLineDeleted, must find it in pNode
                        assert(iter != pMerged->extents.end());
                        assert(iter->pNode == pNode);
                    }
                }
            }
        }
    }
    else
    {
        Range aRange( 0, !rText.isEmpty() ? rText.getLength() - 1 : 0 );
        MultiSelection aHiddenMulti( aRange );
        CalcHiddenRanges( rNode, aHiddenMulti );

        for (sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const sal_Int32 nStart = rRange.Min();
            const sal_Int32 nEnd = rRange.Max() + 1;

            m_HiddenChg.push_back( TextFrameIndex(nStart) );
            m_HiddenChg.push_back( TextFrameIndex(nEnd) );
        }
    }

    // SCRIPT AND SCRIPT RELATED INFORMATION

    TextFrameIndex nChg = m_nInvalidityPos;

    // COMPLETE_STRING means the data structure is up to date
    m_nInvalidityPos = TextFrameIndex(COMPLETE_STRING);

    // this is the default direction
    m_nDefaultDir = static_cast<sal_uInt8>(bRTL ? UBIDI_RTL : UBIDI_LTR);

    // counter for script info arrays
    size_t nCnt = 0;
    // counter for compression information arrays
    size_t nCntComp = 0;
    // counter for kashida array
    size_t nCntKash = 0;

    sal_Int16 nScript = i18n::ScriptType::LATIN;

    // compression type
    const CharCompressType aCompEnum = rNode.getIDocumentSettingAccess()->getCharacterCompressionType();

    auto const& rParaItems((pMerged ? *pMerged->pParaPropsNode : rNode).GetSwAttrSet());
    // justification type
    const bool bAdjustBlock = SvxAdjust::Block == rParaItems.GetAdjust().GetAdjust();

    // FIND INVALID RANGES IN SCRIPT INFO ARRAYS:

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
        if( CharCompressType::NONE != aCompEnum )
        {
            while( nCntComp < CountCompChg() )
            {
                if ( nChg <= GetCompStart( nCntComp ) )
                    break;
                nCntComp++;
            }
        }
        if ( bAdjustBlock )
        {
            while( nCntKash < CountKashida() )
            {
                if ( nChg <= GetKashida( nCntKash ) )
                    break;
                nCntKash++;
            }
        }
    }

    // ADJUST nChg VALUE:

    // by stepping back one position we know that we are inside a group
    // declared as an nScript group
    if ( nChg )
        --nChg;

    const TextFrameIndex nGrpStart = nCnt ? GetScriptChg(nCnt - 1) : TextFrameIndex(0);

    // we go back in our group until we reach the first character of
    // type nScript
    while ( nChg > nGrpStart &&
            nScript != g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nChg)))
        --nChg;

    // If we are at the start of a group, we do not trust nScript,
    // we better get nScript from the breakiterator:
    if ( nChg == nGrpStart )
        nScript = static_cast<sal_uInt8>(g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nChg)));

    // INVALID DATA FROM THE SCRIPT INFO ARRAYS HAS TO BE DELETED:

    // remove invalid entries from script information arrays
    m_ScriptChanges.erase(m_ScriptChanges.begin() + nCnt, m_ScriptChanges.end());

    // get the start of the last compression group
    TextFrameIndex nLastCompression = nChg;
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
    m_CompressionChanges.erase(m_CompressionChanges.begin() + nCntComp,
            m_CompressionChanges.end());

    // get the start of the last kashida group
    TextFrameIndex nLastKashida = nChg;
    if( nCntKash && i18n::ScriptType::COMPLEX == nScript )
    {
        --nCntKash;
        nLastKashida = GetKashida( nCntKash );
    }

    // remove invalid entries from kashida array
    m_Kashida.erase(m_Kashida.begin() + nCntKash, m_Kashida.end());

    // TAKE CARE OF WEAK CHARACTERS: WE MUST FIND AN APPROPRIATE
    // SCRIPT FOR WEAK CHARACTERS AT THE BEGINNING OF A PARAGRAPH

    if (WEAK == g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nChg)))
    {
        // If the beginning of the current group is weak, this means that
        // all of the characters in this group are weak. We have to assign
        // the scripts to these characters depending on the fonts which are
        // set for these characters to display them.
        TextFrameIndex nEnd = TextFrameIndex(
            g_pBreakIt->GetBreakIter()->endOfScript(rText, sal_Int32(nChg), WEAK));

        if (nEnd > TextFrameIndex(rText.getLength()) || nEnd < TextFrameIndex(0))
            nEnd = TextFrameIndex(rText.getLength());

        nScript = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );

        SAL_WARN_IF( i18n::ScriptType::LATIN != nScript &&
                i18n::ScriptType::ASIAN != nScript &&
                i18n::ScriptType::COMPLEX != nScript, "sw.core", "Wrong default language" );

        nChg = nEnd;

        // Get next script type or set to weak in order to exit
        sal_uInt8 nNextScript = (nEnd < TextFrameIndex(rText.getLength()))
            ? static_cast<sal_uInt8>(g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nEnd)))
            : sal_uInt8(WEAK);

        if ( nScript != nNextScript )
        {
            m_ScriptChanges.emplace_back(nEnd, nScript);
            nCnt++;
            nScript = nNextScript;
        }
    }

    // UPDATE THE SCRIPT INFO ARRAYS:

    while (nChg < TextFrameIndex(rText.getLength())
           || (m_ScriptChanges.empty() && rText.isEmpty()))
    {
        SAL_WARN_IF( i18n::ScriptType::WEAK == nScript,
                "sw.core", "Inserting WEAK into SwScriptInfo structure" );

        TextFrameIndex nSearchStt = nChg;
        nChg = TextFrameIndex(g_pBreakIt->GetBreakIter()->endOfScript(
                    rText, sal_Int32(nSearchStt), nScript));

        if (nChg > TextFrameIndex(rText.getLength()) || nChg < TextFrameIndex(0))
            nChg = TextFrameIndex(rText.getLength());

        // #i28203#
        // for 'complex' portions, we make sure that a portion does not contain more
        // than one script:
        if( i18n::ScriptType::COMPLEX == nScript )
        {
            const short nScriptType = ScriptTypeDetector::getCTLScriptType(
                    rText, sal_Int32(nSearchStt) );
            TextFrameIndex nNextCTLScriptStart = nSearchStt;
            short nCurrentScriptType = nScriptType;
            while( css::i18n::CTLScriptType::CTL_UNKNOWN == nCurrentScriptType || nScriptType == nCurrentScriptType )
            {
                nNextCTLScriptStart = TextFrameIndex(
                        ScriptTypeDetector::endOfCTLScriptType(
                            rText, sal_Int32(nNextCTLScriptStart)));
                if (nNextCTLScriptStart >= TextFrameIndex(rText.getLength())
                    || nNextCTLScriptStart >= nChg)
                    break;
                nCurrentScriptType = ScriptTypeDetector::getCTLScriptType(
                                        rText, sal_Int32(nNextCTLScriptStart));
            }
            nChg = std::min( nChg, nNextCTLScriptStart );
        }

        // special case for dotted circle since it can be used with complex
        // before a mark, so we want it associated with the mark's script
        if (nChg < TextFrameIndex(rText.getLength()) && nChg > TextFrameIndex(0)
            && (i18n::ScriptType::WEAK ==
                g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nChg) - 1)))
        {
            int8_t nType = u_charType(rText[sal_Int32(nChg)]);
            if (nType == U_NON_SPACING_MARK || nType == U_ENCLOSING_MARK ||
                nType == U_COMBINING_SPACING_MARK )
            {
                m_ScriptChanges.emplace_back(nChg-TextFrameIndex(1), nScript);
            }
            else
            {
                m_ScriptChanges.emplace_back(nChg, nScript);
            }
        }
        else
        {
            m_ScriptChanges.emplace_back(nChg, nScript);
        }
        ++nCnt;

        // if current script is asian, we search for compressable characters
        // in this range
        if ( CharCompressType::NONE != aCompEnum &&
             i18n::ScriptType::ASIAN == nScript )
        {
            CompType ePrevState = NONE;
            CompType eState = NONE;
            TextFrameIndex nPrevChg = nLastCompression;

            while ( nLastCompression < nChg )
            {
                sal_Unicode cChar = rText[ sal_Int32(nLastCompression) ];

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
                case 0x3009: case 0x300B:
                case 0x300D: case 0x300F: case 0x3011: case 0x3015:
                case 0x3017: case 0x3019: case 0x301B: case 0x301E:
                case 0x301F:
                    eState = SPECIAL_RIGHT;
                    break;
                case 0x3001: case 0x3002:   // Fullstop or comma
                    eState = SPECIAL_MIDDLE ;
                    break;
                default:
                    eState = ( 0x3040 <= cChar && 0x3100 > cChar ) ? KANA : NONE;
                }

                // insert range of compressable characters
                if( ePrevState != eState )
                {
                    if ( ePrevState != NONE )
                    {
                        // insert start and type
                        if ( CharCompressType::PunctuationAndKana == aCompEnum ||
                             ePrevState != KANA )
                        {
                            m_CompressionChanges.emplace_back(nPrevChg,
                                    nLastCompression - nPrevChg, ePrevState);
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
                if ( CharCompressType::PunctuationAndKana == aCompEnum ||
                     ePrevState != KANA )
                {
                    m_CompressionChanges.emplace_back(nPrevChg,
                            nLastCompression - nPrevChg, ePrevState);
                }
            }
        }

        // we search for connecting opportunities (kashida)
        else if ( bAdjustBlock && i18n::ScriptType::COMPLEX == nScript )
        {
            // sw_redlinehide: this is the only place that uses SwScanner with
            // frame text, so we convert to sal_Int32 here
            std::function<LanguageType (sal_Int32, sal_Int32, bool)> const pGetLangOfCharM(
                [&pMerged](sal_Int32 const nBegin, sal_uInt16 const script, bool const bNoChar)
                    {
                        std::pair<SwTextNode const*, sal_Int32> const pos(
                            sw::MapViewToModel(*pMerged, TextFrameIndex(nBegin)));
                        return pos.first->GetLang(pos.second, bNoChar ? 0 : 1, script);
                    });
            std::function<LanguageType (sal_Int32, sal_Int32, bool)> const pGetLangOfChar1(
                [&rNode](sal_Int32 const nBegin, sal_uInt16 const script, bool const bNoChar)
                    { return rNode.GetLang(nBegin, bNoChar ? 0 : 1, script); });
            auto pGetLangOfChar(pMerged ? pGetLangOfCharM : pGetLangOfChar1);
            SwScanner aScanner( pGetLangOfChar, rText, nullptr, ModelToViewHelper(),
                                i18n::WordType::DICTIONARY_WORD,
                                sal_Int32(nLastKashida), sal_Int32(nChg));

            // the search has to be performed on a per word base
            while ( aScanner.NextWord() )
            {
                const OUString& rWord = aScanner.GetWord();

                sal_Int32 nIdx = 0;
                sal_Int32 nKashidaPos = -1;
                sal_Unicode cCh;
                sal_Unicode cPrevCh = 0;

                int nPriorityLevel = 7;    // 0..6 = level found
                                           // 7 not found

                sal_Int32 nWordLen = rWord.getLength();

                // ignore trailing vowel chars
                while( nWordLen && isTransparentChar( rWord[ nWordLen - 1 ] ))
                    --nWordLen;

                while (nIdx < nWordLen)
                {
                    cCh = rWord[ nIdx ];

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
                         && (rWord[ nIdx+1 ] != 0x200C) ) // #i98410#: prevent ZWNJ expansion
                        {
                            nKashidaPos  = aScanner.GetBegin() + nIdx;
                            nPriorityLevel = 1;
                        }
                    }

                    // 3. Priority:
                    // before final form of Teh Marbuta, Heh, Dal
                    if ( nPriorityLevel >= 2 && nIdx > 0 )
                    {
                        if ( isTehMarbutaChar ( cCh ) || // Teh Marbuta (right joining)
                             isDalChar ( cCh ) ||        // Dal (right joining) final form may appear in the middle of word
                             ( isHehChar ( cCh ) && nIdx == nWordLen - 1))  // Heh (dual joining) only at end of word
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
                    // before final form of Alef, Tah, Lam, Kaf or Gaf
                    if ( nPriorityLevel >= 3 && nIdx > 0 )
                    {
                        if ( isAlefChar ( cCh ) ||   // Alef (right joining) final form may appear in the middle of word
                             (( isLamChar ( cCh ) || // Lam,
                              isTahChar ( cCh )   || // Tah,
                              isKafChar ( cCh )   || // Kaf (all dual joining)
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
                    // before medial Beh-like
                    if ( nPriorityLevel >= 4 && nIdx > 0 && nIdx < nWordLen - 1 )
                    {
                        if ( isBehChar ( cCh ) )
                        {
                            // check if next character is Reh or Yeh-like
                            sal_Unicode cNextCh = rWord[ nIdx + 1 ];
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
                    // before the final form of Waw, Ain, Qaf and Feh
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
                        // Reh, Zain
                        if ( isRehChar ( cCh ) )
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

                    // Do not consider vowel marks when checking if a character
                    // can be connected to previous character.
                    if ( !isTransparentChar ( cCh) )
                        cPrevCh = cCh;

                    ++nIdx;
                } // end of current word

                if ( -1 != nKashidaPos )
                {
                    m_Kashida.insert(m_Kashida.begin() + nCntKash, TextFrameIndex(nKashidaPos));
                    nCntKash++;
                }
            } // end of kashida search
        }

        if (nChg < TextFrameIndex(rText.getLength()))
            nScript = static_cast<sal_uInt8>(g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nChg)));

        nLastCompression = nChg;
        nLastKashida = nChg;
    }

#if OSL_DEBUG_LEVEL > 0
    // check kashida data
    TextFrameIndex nTmpKashidaPos(-1);
    bool bWrongKash = false;
    for (size_t i = 0; i < m_Kashida.size(); ++i)
    {
        TextFrameIndex nCurrKashidaPos = GetKashida( i );
        if ( nCurrKashidaPos <= nTmpKashidaPos )
        {
            bWrongKash = true;
            break;
        }
        nTmpKashidaPos = nCurrKashidaPos;
    }
    SAL_WARN_IF( bWrongKash, "sw.core", "Kashida array contains wrong data" );
#endif

    // remove invalid entries from direction information arrays
    m_DirectionChanges.clear();

    // Perform Unicode Bidi Algorithm for text direction information
    {
        UpdateBidiInfo( rText );

        // #i16354# Change script type for RTL text to CTL:
        // 1. All text in RTL runs will use the CTL font
        // #i89825# change the script type also to CTL (hennerdrewes)
        // 2. Text in embedded LTR runs that does not have any strong LTR characters (numbers!)
        for (size_t nDirIdx = 0; nDirIdx < m_DirectionChanges.size(); ++nDirIdx)
        {
            const sal_uInt8 nCurrDirType = GetDirType( nDirIdx );
                // nStart is start of RTL run:
            const TextFrameIndex nStart = nDirIdx > 0 ? GetDirChg(nDirIdx - 1) : TextFrameIndex(0);
                // nEnd is end of RTL run:
            const TextFrameIndex nEnd = GetDirChg( nDirIdx );

            if ( nCurrDirType % 2 == UBIDI_RTL  || // text in RTL run
                (nCurrDirType > UBIDI_LTR && // non-strong text in embedded LTR run
                 !lcl_HasStrongLTR(rText, sal_Int32(nStart), sal_Int32(nEnd))))
            {
                // nScriptIdx points into the ScriptArrays:
                size_t nScriptIdx = 0;

                // Skip entries in ScriptArray which are not inside the RTL run:
                // Make nScriptIdx become the index of the script group with
                // 1. nStartPosOfGroup <= nStart and
                // 2. nEndPosOfGroup > nStart
                while ( GetScriptChg( nScriptIdx ) <= nStart )
                    ++nScriptIdx;

                const TextFrameIndex nStartPosOfGroup = nScriptIdx
                        ? GetScriptChg(nScriptIdx - 1)
                        : TextFrameIndex(0);
                const sal_uInt8 nScriptTypeOfGroup = GetScriptType( nScriptIdx );

                SAL_WARN_IF( nStartPosOfGroup > nStart || GetScriptChg( nScriptIdx ) <= nStart,
                        "sw.core", "Script override with CTL font trouble" );

                // Check if we have to insert a new script change at
                // position nStart. If nStartPosOfGroup < nStart,
                // we have to insert a new script change:
                if (nStart > TextFrameIndex(0) && nStartPosOfGroup < nStart)
                {
                    m_ScriptChanges.insert(m_ScriptChanges.begin() + nScriptIdx,
                                          ScriptChangeInfo(nStart, nScriptTypeOfGroup) );
                    ++nScriptIdx;
                }

                // Remove entries in ScriptArray which end inside the RTL run:
                while (nScriptIdx < m_ScriptChanges.size()
                       && GetScriptChg(nScriptIdx) <= nEnd)
                {
                    m_ScriptChanges.erase(m_ScriptChanges.begin() + nScriptIdx);
                }

                // Insert a new entry in ScriptArray for the end of the RTL run:
                m_ScriptChanges.insert(m_ScriptChanges.begin() + nScriptIdx,
                                      ScriptChangeInfo(nEnd, i18n::ScriptType::COMPLEX) );

#if OSL_DEBUG_LEVEL > 1
                // Check that ScriptChangeInfos are in increasing order of
                // position and that we don't have "empty" changes.
                sal_uInt8 nLastTyp = i18n::ScriptType::WEAK;
                TextFrameIndex nLastPos = TextFrameIndex(0);
                for (const auto& rScriptChange : m_ScriptChanges)
                {
                    SAL_WARN_IF( nLastTyp == rScriptChange.type ||
                            nLastPos >= rScriptChange.position,
                            "sw.core", "Heavy InitScriptType() confusion" );
                    nLastPos = rScriptChange.position;
                    nLastTyp = rScriptChange.type;
                }
#endif
            }
        }
    }
}

void SwScriptInfo::UpdateBidiInfo( const OUString& rText )
{
    // remove invalid entries from direction information arrays
    m_DirectionChanges.clear();

    // Bidi functions from icu 2.0

    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( rText.getLength(), 0, &nError );
    nError = U_ZERO_ERROR;

    ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(rText.getStr()), rText.getLength(),
                   m_nDefaultDir, nullptr, &nError );
    nError = U_ZERO_ERROR;
    int nCount = ubidi_countRuns( pBidi, &nError );
    int32_t nStart = 0;
    int32_t nEnd;
    UBiDiLevel nCurrDir;
    for ( int nIdx = 0; nIdx < nCount; ++nIdx )
    {
        ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
        m_DirectionChanges.emplace_back(TextFrameIndex(nEnd), nCurrDir);
        nStart = nEnd;
    }

    ubidi_close( pBidi );
}

// returns the position of the next character which belongs to another script
// than the character of the actual (input) position.
// If there's no script change until the end of the paragraph, it will return
// COMPLETE_STRING.
// Scripts are Asian (Chinese, Japanese, Korean),
//             Latin ( English etc.)
//         and Complex ( Hebrew, Arabian )
TextFrameIndex SwScriptInfo::NextScriptChg(const TextFrameIndex nPos)  const
{
    const size_t nEnd = CountScriptChg();
    for( size_t nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptChg( nX );
    }

    return TextFrameIndex(COMPLETE_STRING);
}

// returns the script of the character at the input position
sal_Int16 SwScriptInfo::ScriptType(const TextFrameIndex nPos) const
{
    const size_t nEnd = CountScriptChg();
    for( size_t nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetScriptChg( nX ) )
            return GetScriptType( nX );
    }

    // the default is the application language script
    return SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
}

TextFrameIndex SwScriptInfo::NextDirChg(const TextFrameIndex nPos,
                                     const sal_uInt8* pLevel )  const
{
    const sal_uInt8 nCurrDir = pLevel ? *pLevel : 62;
    const size_t nEnd = CountDirChg();
    for( size_t nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) &&
            ( nX + 1 == nEnd || GetDirType( nX + 1 ) <= nCurrDir ) )
            return GetDirChg( nX );
    }

    return TextFrameIndex(COMPLETE_STRING);
}

sal_uInt8 SwScriptInfo::DirType(const TextFrameIndex nPos) const
{
    const size_t nEnd = CountDirChg();
    for( size_t nX = 0; nX < nEnd; ++nX )
    {
        if( nPos < GetDirChg( nX ) )
            return GetDirType( nX );
    }

    return 0;
}

TextFrameIndex SwScriptInfo::NextHiddenChg(TextFrameIndex const nPos) const
{
    for (auto const& it : m_HiddenChg)
    {
        if (nPos < it)
        {
            return it;
        }
    }
    return TextFrameIndex(COMPLETE_STRING);
}

// Takes a string and replaced the hidden ranges with cChar.
sal_Int32 SwScriptInfo::MaskHiddenRanges( const SwTextNode& rNode, OUStringBuffer & rText,
                                       const sal_Int32 nStt, const sal_Int32 nEnd,
                                       const sal_Unicode cChar )
{
    assert(rNode.GetText().getLength() == rText.getLength());

    std::vector<sal_Int32> aList;
    sal_Int32 nHiddenStart;
    sal_Int32 nHiddenEnd;
    sal_Int32 nNumOfHiddenChars = 0;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    auto rFirst( aList.crbegin() );
    auto rLast( aList.crend() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        if ( nHiddenEnd < nStt || nHiddenStart > nEnd )
            continue;

        while ( nHiddenStart < nHiddenEnd && nHiddenStart < nEnd )
        {
            if (nHiddenStart >= nStt)
            {
                rText[nHiddenStart] = cChar;
                ++nNumOfHiddenChars;
            }
            ++nHiddenStart;
        }
    }

    return nNumOfHiddenChars;
}

// Takes a SwTextNode and deletes the hidden ranges from the node.
void SwScriptInfo::DeleteHiddenRanges( SwTextNode& rNode )
{
    std::vector<sal_Int32> aList;
    sal_Int32 nHiddenStart;
    sal_Int32 nHiddenEnd;
    GetBoundsOfHiddenRange( rNode, 0, nHiddenStart, nHiddenEnd, &aList );
    auto rFirst( aList.crbegin() );
    auto rLast( aList.crend() );
    while ( rFirst != rLast )
    {
        nHiddenEnd = *(rFirst++);
        nHiddenStart = *(rFirst++);

        SwPaM aPam( rNode, nHiddenStart, rNode, nHiddenEnd );
        rNode.getIDocumentContentOperations().DeleteRange( aPam );
    }
}

bool SwScriptInfo::GetBoundsOfHiddenRange( const SwTextNode& rNode, sal_Int32 nPos,
                                           sal_Int32& rnStartPos, sal_Int32& rnEndPos,
                                           std::vector<sal_Int32>* pList )
{
    rnStartPos = COMPLETE_STRING;
    rnEndPos = 0;

    bool bNewContainsHiddenChars = false;

    // Optimization: First examine the flags at the text node:

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
                pList->push_back(rNode.GetText().getLength());
            }

            rnStartPos = 0;
            rnEndPos = rNode.GetText().getLength();
            return true;
        }
    }

    // sw_redlinehide: this won't work if it's merged
#if 0
    const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rNode );
    if ( pSI )
    {

        // Check first, if we have a valid SwScriptInfo object for this text node:

        bNewContainsHiddenChars = pSI->GetBoundsOfHiddenRange( nPos, rnStartPos, rnEndPos, pList );
        const bool bNewHiddenCharsHidePara =
            rnStartPos == 0 && rnEndPos >= rNode.GetText().getLength();
        rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
    }
    else
#endif
    {

        // No valid SwScriptInfo Object, we have to do it the hard way:

        Range aRange(0, (!rNode.GetText().isEmpty())
                            ? rNode.GetText().getLength() - 1
                            : 0);
        MultiSelection aHiddenMulti( aRange );
        SwScriptInfo::CalcHiddenRanges( rNode, aHiddenMulti );
        for( sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const sal_Int32 nHiddenStart = rRange.Min();
            const sal_Int32 nHiddenEnd = rRange.Max() + 1;

            if ( nHiddenStart > nPos )
                break;
            if (nPos < nHiddenEnd)
            {
                rnStartPos = nHiddenStart;
                rnEndPos   = std::min<sal_Int32>(nHiddenEnd,
                                                 rNode.GetText().getLength());
                break;
            }
        }

        if ( pList )
        {
            for( sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i )
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                pList->push_back( rRange.Min() );
                pList->push_back( rRange.Max() + 1 );
            }
        }

        bNewContainsHiddenChars = aHiddenMulti.GetRangeCount() > 0;
    }

    return bNewContainsHiddenChars;
}

bool SwScriptInfo::GetBoundsOfHiddenRange(TextFrameIndex nPos,
        TextFrameIndex & rnStartPos, TextFrameIndex & rnEndPos) const
{
    rnStartPos = TextFrameIndex(COMPLETE_STRING);
    rnEndPos = TextFrameIndex(0);

    const size_t nEnd = CountHiddenChg();
    for( size_t nX = 0; nX < nEnd; ++nX )
    {
        const TextFrameIndex nHiddenStart = GetHiddenChg( nX++ );
        const TextFrameIndex nHiddenEnd = GetHiddenChg( nX );

        if ( nHiddenStart > nPos )
            break;
        if (nPos < nHiddenEnd)
        {
            rnStartPos = nHiddenStart;
            rnEndPos   = nHiddenEnd;
            break;
        }
    }

    return CountHiddenChg() > 0;
}

bool SwScriptInfo::IsInHiddenRange( const SwTextNode& rNode, sal_Int32 nPos )
{
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;
    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nPos, nStartPos, nEndPos );
    return nStartPos != COMPLETE_STRING;
}

#ifdef DBG_UTIL
// returns the type of the compressed character
SwScriptInfo::CompType SwScriptInfo::DbgCompType(const TextFrameIndex nPos) const
{
    const size_t nEnd = CountCompChg();
    for( size_t nX = 0; nX < nEnd; ++nX )
    {
        const TextFrameIndex nChg = GetCompStart(nX);

        if ( nPos < nChg )
            return NONE;

        if( nPos < nChg + GetCompLen( nX ) )
            return GetCompType( nX );
    }
    return NONE;
}
#endif

// returns, if there are compressable kanas or specials
// between nStart and nEnd
size_t SwScriptInfo::HasKana(TextFrameIndex const nStart, TextFrameIndex const nLen) const
{
    const size_t nCnt = CountCompChg();
    TextFrameIndex nEnd = nStart + nLen;

    for( size_t nX = 0; nX < nCnt; ++nX )
    {
        TextFrameIndex nKanaStart  = GetCompStart(nX);
        TextFrameIndex nKanaEnd = nKanaStart + GetCompLen(nX);

        if ( nKanaStart >= nEnd )
            return SAL_MAX_SIZE;

        if ( nStart < nKanaEnd )
            return nX;
    }

    return SAL_MAX_SIZE;
}

long SwScriptInfo::Compress(long* pKernArray, TextFrameIndex nIdx, TextFrameIndex nLen,
                             const sal_uInt16 nCompress, const sal_uInt16 nFontHeight,
                             bool bCenter,
                             Point* pPoint ) const
{
    SAL_WARN_IF( !nCompress, "sw.core", "Compression without compression?!" );
    SAL_WARN_IF( !nLen, "sw.core", "Compression without text?!" );
    const size_t nCompCount = CountCompChg();

    // In asian typography, there are full width and half width characters.
    // Full width punctuation characters can be compressed by 50%
    // to determine this, we compare the font width with 75% of its height
    const long nMinWidth = ( 3 * nFontHeight ) / 4;

    size_t nCompIdx = HasKana( nIdx, nLen );

    if ( SAL_MAX_SIZE == nCompIdx )
        return 0;

    TextFrameIndex nChg = GetCompStart( nCompIdx );
    TextFrameIndex nCompLen = GetCompLen( nCompIdx );
    sal_Int32 nI = 0;
    nLen += nIdx;

    if( nChg > nIdx )
    {
        nI = sal_Int32(nChg - nIdx);
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
        const CompType nType = GetCompType( nCompIdx );
#ifdef DBG_UTIL
        SAL_WARN_IF( nType != DbgCompType( nIdx ), "sw.core", "Gimme the right type!" );
#endif
        nCompLen += nIdx;
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
                    nLast /= 24000;
                    if( pPoint && SwScriptInfo::SPECIAL_LEFT == nType )
                    {
                        if( nI )
                            nMove = nLast;
                        else
                        {
                            pPoint->AdjustX(nLast );
                            nLast = 0;
                        }
                    }
                    else if( bCenter && SwScriptInfo::SPECIAL_MIDDLE == nType )
                        nMove = nLast / 2;
                }
                else
                    nLast /= 100000;
                nSub -= nLast;
                nLast = pKernArray[ nI ];
                if( nI && nMove )
                    pKernArray[ nI - 1 ] += nMove;
                pKernArray[ nI++ ] -= nSub;
                ++nIdx;
            }
        }

        if( nIdx >= nLen )
            break;

        TextFrameIndex nTmpChg = nLen;
        if( ++nCompIdx < nCompCount )
        {
            nTmpChg = GetCompStart( nCompIdx );
            if( nTmpChg > nLen )
                nTmpChg = nLen;
            nCompLen = GetCompLen( nCompIdx );
        }

        while( nIdx < nTmpChg )
        {
            nLast = pKernArray[ nI ];
            pKernArray[ nI++ ] -= nSub;
            ++nIdx;
        }
    } while( nIdx < nLen );
    return nSub;
}

// Note on calling KashidaJustify():
// Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
// total number of kashida positions, or the number of kashida positions after some positions
// have been dropped, depending on the state of the m_KashidaInvalid set.

sal_Int32 SwScriptInfo::KashidaJustify( long* pKernArray,
                                        long* pScrArray,
                                        TextFrameIndex const nStt,
                                        TextFrameIndex const nLen,
                                        long nSpaceAdd ) const
{
    SAL_WARN_IF( !nLen, "sw.core", "Kashida justification without text?!" );

    if( !IsKashidaLine(nStt))
        return -1;

    // evaluate kashida information in collected in SwScriptInfo

    size_t nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        ++nCntKash;
    }

    const TextFrameIndex nEnd = nStt + nLen;

    size_t nCntKashEnd = nCntKash;
    while ( nCntKashEnd < CountKashida() )
    {
        if ( nEnd <= GetKashida( nCntKashEnd ) )
            break;
        ++nCntKashEnd;
    }

    size_t nActualKashCount = nCntKashEnd - nCntKash;
    for (size_t i = nCntKash; i < nCntKashEnd; ++i)
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
        while (nCntKash < nCntKashEnd && !IsKashidaValid(nCntKash))
            ++nCntKash;

        TextFrameIndex nIdx = nCntKash < nCntKashEnd && IsKashidaValid(nCntKash)
            ? GetKashida(nCntKash)
            : nEnd;
        long nKashAdd = nSpaceAdd;

        while ( nIdx < nEnd )
        {
            TextFrameIndex nArrayPos = nIdx - nStt;

            // next kashida position
            ++nCntKash;
            while (nCntKash < nCntKashEnd && !IsKashidaValid(nCntKash))
                ++nCntKash;

            nIdx = nCntKash < nCntKashEnd && IsKashidaValid(nCntKash) ? GetKashida(nCntKash) : nEnd;
            if ( nIdx > nEnd )
                nIdx = nEnd;

            const TextFrameIndex nArrayEnd = nIdx - nStt;

            while ( nArrayPos < nArrayEnd )
            {
                pKernArray[ sal_Int32(nArrayPos) ] += nKashAdd;
                if ( pScrArray )
                    pScrArray[ sal_Int32(nArrayPos) ] += nKashAdd;
                ++nArrayPos;
            }
            nKashAdd += nSpaceAdd;
        }
    }

    return 0;
}

// Checks if the current text is 'Arabic' text. Note that only the first
// character has to be checked because a ctl portion only contains one
// script, see NewTextPortion
bool SwScriptInfo::IsArabicText(const OUString& rText,
        TextFrameIndex const nStt, TextFrameIndex const nLen)
{
    using namespace ::com::sun::star::i18n;
    static const ScriptTypeList typeList[] = {
        { UnicodeScript_kArabic, UnicodeScript_kArabic, sal_Int16(UnicodeScript_kArabic) },        // 11,
        { UnicodeScript_kScriptCount, UnicodeScript_kScriptCount, sal_Int16(UnicodeScript_kScriptCount) }    // 88
    };

    // go forward if current position does not hold a regular character:
    const CharClass& rCC = GetAppCharClass();
    sal_Int32 nIdx = sal_Int32(nStt);
    const sal_Int32 nEnd = sal_Int32(nStt + nLen);
    while ( nIdx < nEnd && !rCC.isLetterNumeric( rText, nIdx ) )
    {
        ++nIdx;
    }

    if( nIdx == nEnd )
    {
        // no regular character found in this portion. Go backward:
        --nIdx;
        while ( nIdx >= 0 && !rCC.isLetterNumeric( rText, nIdx ) )
        {
            --nIdx;
        }
    }

    if( nIdx >= 0 )
    {
        const sal_Unicode cCh = rText[nIdx];
        const sal_Int16 type = unicode::getUnicodeScriptType( cCh, typeList, sal_Int16(UnicodeScript_kScriptCount) );
        return type == sal_Int16(UnicodeScript_kArabic);
    }
    return false;
}

bool SwScriptInfo::IsKashidaValid(size_t const nKashPos) const
{
    return m_KashidaInvalid.find(nKashPos) == m_KashidaInvalid.end();
}

void SwScriptInfo::ClearKashidaInvalid(size_t const nKashPos)
{
    m_KashidaInvalid.erase(nKashPos);
}

// bMark == true:
// marks the first valid kashida in the given text range as invalid
// bMark == false:
// clears all kashida invalid flags in the given text range
bool SwScriptInfo::MarkOrClearKashidaInvalid(
    TextFrameIndex const nStt, TextFrameIndex const nLen,
    bool bMark, sal_Int32 nMarkCount)
{
    size_t nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        nCntKash++;
    }

    const TextFrameIndex nEnd = nStt + nLen;

    while ( nCntKash < CountKashida() )
    {
        if ( nEnd <= GetKashida( nCntKash ) )
            break;
        if(bMark)
        {
            if ( MarkKashidaInvalid ( nCntKash ) )
            {
                --nMarkCount;
                if (!nMarkCount)
                    return true;
            }
        }
        else
        {
            ClearKashidaInvalid ( nCntKash );
        }
        nCntKash++;
    }
    return false;
}

bool SwScriptInfo::MarkKashidaInvalid(size_t const nKashPos)
{
    return m_KashidaInvalid.insert(nKashPos).second;
}

// retrieve the kashida positions in the given text range
void SwScriptInfo::GetKashidaPositions(
    TextFrameIndex const nStt, TextFrameIndex const nLen,
    std::vector<TextFrameIndex>& rKashidaPosition)
{
    size_t nCntKash = 0;
    while( nCntKash < CountKashida() )
    {
        if ( nStt <= GetKashida( nCntKash ) )
            break;
        nCntKash++;
    }

    const TextFrameIndex nEnd = nStt + nLen;

    size_t nCntKashEnd = nCntKash;
    while ( nCntKashEnd < CountKashida() )
    {
        if ( nEnd <= GetKashida( nCntKashEnd ) )
            break;
        rKashidaPosition.push_back(GetKashida(nCntKashEnd));
        nCntKashEnd++;
    }
}

void SwScriptInfo::SetNoKashidaLine(TextFrameIndex const nStt, TextFrameIndex const nLen)
{
    m_NoKashidaLine.push_back( nStt );
    m_NoKashidaLineEnd.push_back( nStt + nLen );
}

// determines if the line uses kashida justification
bool SwScriptInfo::IsKashidaLine(TextFrameIndex const nCharIdx) const
{
    for (size_t i = 0; i < m_NoKashidaLine.size(); ++i)
    {
        if (nCharIdx >= m_NoKashidaLine[i] && nCharIdx < m_NoKashidaLineEnd[i])
            return false;
    }
    return true;
}

void SwScriptInfo::ClearNoKashidaLine(TextFrameIndex const nStt, TextFrameIndex const nLen)
{
    size_t i = 0;
    while (i < m_NoKashidaLine.size())
    {
        if (nStt + nLen >= m_NoKashidaLine[i] && nStt < m_NoKashidaLineEnd[i])
        {
            m_NoKashidaLine.erase(m_NoKashidaLine.begin() + i);
            m_NoKashidaLineEnd.erase(m_NoKashidaLineEnd.begin() + i);
        }
        else
            ++i;
    }
}

// mark the given character indices as invalid kashida positions
void SwScriptInfo::MarkKashidasInvalid(sal_Int32 const nCnt,
        const TextFrameIndex* pKashidaPositions)
{
    SAL_WARN_IF( !pKashidaPositions || nCnt == 0, "sw.core", "Where are kashidas?" );

    size_t nCntKash = 0;
    sal_Int32 nKashidaPosIdx = 0;

    while (nCntKash < CountKashida() && nKashidaPosIdx < nCnt)
    {
        if ( pKashidaPositions [nKashidaPosIdx] > GetKashida( nCntKash ) )
        {
            ++nCntKash;
            continue;
        }

        if ( pKashidaPositions [nKashidaPosIdx] != GetKashida( nCntKash ) || !IsKashidaValid ( nCntKash ) )
            return; // something is wrong

        MarkKashidaInvalid ( nCntKash );
        nKashidaPosIdx++;
    }
}

TextFrameIndex SwScriptInfo::ThaiJustify( const OUString& rText, long* pKernArray,
                                     long* pScrArray, TextFrameIndex const nStt,
                                     TextFrameIndex const nLen,
                                     TextFrameIndex nNumberOfBlanks,
                                     long nSpaceAdd )
{
    SAL_WARN_IF( nStt + nLen > TextFrameIndex(rText.getLength()), "sw.core", "String in ThaiJustify too small" );

    SwTwips nNumOfTwipsToDistribute = nSpaceAdd * sal_Int32(nNumberOfBlanks) /
                                      SPACING_PRECISION_FACTOR;

    long nSpaceSum = 0;
    TextFrameIndex nCnt(0);

    for (sal_Int32 nI = 0; nI < sal_Int32(nLen); ++nI)
    {
        const sal_Unicode cCh = rText[sal_Int32(nStt) + nI];

        // check if character is not above or below base
        if ( ( 0xE34 > cCh || cCh > 0xE3A ) &&
             ( 0xE47 > cCh || cCh > 0xE4E ) && cCh != 0xE31 )
        {
            if (nNumberOfBlanks > TextFrameIndex(0))
            {
                nSpaceAdd = nNumOfTwipsToDistribute / sal_Int32(nNumberOfBlanks);
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

SwScriptInfo* SwScriptInfo::GetScriptInfo( const SwTextNode& rTNd,
                                           SwTextFrame const**const o_ppFrame,
                                           bool const bAllowInvalid)
{
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(rTNd);
    SwScriptInfo* pScriptInfo = nullptr;

    for( SwTextFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        pScriptInfo = const_cast<SwScriptInfo*>(pLast->GetScriptInfo());
        if ( pScriptInfo )
        {
            if (bAllowInvalid ||
                TextFrameIndex(COMPLETE_STRING) == pScriptInfo->GetInvalidityA())
            {
                if (o_ppFrame)
                {
                    *o_ppFrame = pLast;
                }
                break;
            }
            pScriptInfo = nullptr;
        }
    }

    return pScriptInfo;
}

SwParaPortion::SwParaPortion()
{
    FormatReset();
    m_bFlys = m_bFootnoteNum = m_bMargin = false;
    SetWhichPor( PortionType::Para );
}

SwParaPortion::~SwParaPortion()
{
}

TextFrameIndex SwParaPortion::GetParLen() const
{
    TextFrameIndex nLen(0);
    const SwLineLayout *pLay = this;
    while( pLay )
    {
        nLen += pLay->GetLen();
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
        const SwLinePortion *pPos = pLay->GetNextPortion();
        while ( pPos && !pPos->GetLen() )
            pPos = pPos->GetNextPortion();
        if( pPos && pPos->IsDropPortion() )
            return static_cast<const SwDropPortion *>(pPos);
        pLay = pLay->GetLen() ? nullptr : pLay->GetNext();
    }
    return nullptr;
}

void SwLineLayout::Init( SwLinePortion* pNextPortion )
{
    Height( 0 );
    Width( 0 );
    SetLen(TextFrameIndex(0));
    SetAscent( 0 );
    SetRealHeight( 0 );
    SetNextPortion( pNextPortion );
}

// looks for hanging punctuation portions in the paragraph
// and return the maximum right offset of them.
// If no such portion is found, the Margin/Hanging-flags will be updated.
SwTwips SwLineLayout::GetHangingMargin_() const
{
    SwLinePortion* pPor = GetNextPortion();
    bool bFound = false;
    SwTwips nDiff = 0;
    while( pPor)
    {
        if( pPor->IsHangingPortion() )
        {
            nDiff = static_cast<SwHangingPortion*>(pPor)->GetInnerWidth() - pPor->Width();
            if( nDiff )
                bFound = true;
        }
        // the last post its portion
        else if ( pPor->IsPostItsPortion() && ! pPor->GetNextPortion() )
            nDiff = nAscent;

        pPor = pPor->GetNextPortion();
    }
    if( !bFound ) // update the hanging-flag
        const_cast<SwLineLayout*>(this)->SetHanging( false );
    return nDiff;
}

SwTwips SwTextFrame::HangingMargin() const
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
        const_cast<SwParaPortion*>(GetPara())->SetMargin( false );
    return nRet;
}

void SwScriptInfo::selectHiddenTextProperty(const SwTextNode& rNode, MultiSelection &rHiddenMulti)
{
    assert((rNode.GetText().isEmpty() && rHiddenMulti.GetTotalRange().Len() == 1)
        || (rNode.GetText().getLength() == rHiddenMulti.GetTotalRange().Len()));

    const SfxPoolItem* pItem = nullptr;
    if( SfxItemState::SET == rNode.GetSwAttrSet().GetItemState( RES_CHRATR_HIDDEN, true, &pItem ) &&
        static_cast<const SvxCharHiddenItem*>(pItem)->GetValue() )
    {
        rHiddenMulti.SelectAll();
    }

    const SwpHints* pHints = rNode.GetpSwpHints();

    if( pHints )
    {
        for( size_t nTmp = 0; nTmp < pHints->Count(); ++nTmp )
        {
            const SwTextAttr* pTextAttr = pHints->Get( nTmp );
            const SvxCharHiddenItem* pHiddenItem = CharFormat::GetItem( *pTextAttr, RES_CHRATR_HIDDEN );
            if( pHiddenItem )
            {
                const sal_Int32 nSt = pTextAttr->GetStart();
                const sal_Int32 nEnd = *pTextAttr->End();
                if( nEnd > nSt )
                {
                    Range aTmp( nSt, nEnd - 1 );
                    rHiddenMulti.Select( aTmp, pHiddenItem->GetValue() );
                }
            }
        }
    }

    for (const SwIndex* pIndex = rNode.GetFirstIndex(); pIndex; pIndex = pIndex->GetNext())
    {
        const sw::mark::IMark* pMark = pIndex->GetMark();
        const sw::mark::IBookmark* pBookmark = dynamic_cast<const sw::mark::IBookmark*>(pMark);
        if (pBookmark && pBookmark->IsHidden())
        {
            // intersect bookmark range with textnode range and add the intersection to rHiddenMulti

            const sal_Int32 nSt =  pBookmark->GetMarkStart().nContent.GetIndex();
            const sal_Int32 nEnd = pBookmark->GetMarkEnd().nContent.GetIndex();

            if( nEnd > nSt )
            {
                Range aTmp( nSt, nEnd - 1 );
                rHiddenMulti.Select(aTmp, true);
            }
        }
    }
}

void SwScriptInfo::selectRedLineDeleted(const SwTextNode& rNode, MultiSelection &rHiddenMulti, bool bSelect)
{
    assert((rNode.GetText().isEmpty() && rHiddenMulti.GetTotalRange().Len() == 1)
        || (rNode.GetText().getLength() == rHiddenMulti.GetTotalRange().Len()));

    const IDocumentRedlineAccess& rIDRA = rNode.getIDocumentRedlineAccess();
    if ( IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineFlags() ) )
    {
        SwRedlineTable::size_type nAct = rIDRA.GetRedlinePos( rNode, USHRT_MAX );

        for ( ; nAct < rIDRA.GetRedlineTable().size(); nAct++ )
        {
            const SwRangeRedline* pRed = rIDRA.GetRedlineTable()[ nAct ];

            if (pRed->Start()->nNode > rNode.GetIndex())
                break;

            if (pRed->GetType() != nsRedlineType_t::REDLINE_DELETE)
                continue;

            sal_Int32 nRedlStart;
            sal_Int32 nRedlnEnd;
            pRed->CalcStartEnd( rNode.GetIndex(), nRedlStart, nRedlnEnd );
            //clip it if the redline extends past the end of the nodes text
            nRedlnEnd = std::min<sal_Int32>(nRedlnEnd, rNode.GetText().getLength());
            if ( nRedlnEnd > nRedlStart )
            {
                Range aTmp( nRedlStart, nRedlnEnd - 1 );
                rHiddenMulti.Select( aTmp, bSelect );
            }
        }
    }
}

// Returns a MultiSection indicating the hidden ranges.
void SwScriptInfo::CalcHiddenRanges( const SwTextNode& rNode, MultiSelection& rHiddenMulti )
{
    selectHiddenTextProperty(rNode, rHiddenMulti);

    // If there are any hidden ranges in the current text node, we have
    // to unhide the redlining ranges:
    selectRedLineDeleted(rNode, rHiddenMulti, false);

    // We calculated a lot of stuff. Finally we can update the flags at the text node.

    const bool bNewContainsHiddenChars = rHiddenMulti.GetRangeCount() > 0;
    bool bNewHiddenCharsHidePara = false;
    if ( bNewContainsHiddenChars )
    {
        const Range& rRange = rHiddenMulti.GetRange( 0 );
        const sal_Int32 nHiddenStart = rRange.Min();
        const sal_Int32 nHiddenEnd = rRange.Max() + 1;
        bNewHiddenCharsHidePara =
            (nHiddenStart == 0 && nHiddenEnd >= rNode.GetText().getLength());
    }
    rNode.SetHiddenCharAttribute( bNewHiddenCharsHidePara, bNewContainsHiddenChars );
}

TextFrameIndex SwScriptInfo::CountCJKCharacters(const OUString &rText,
    TextFrameIndex nPos, TextFrameIndex const nEnd, LanguageType aLang)
{
    TextFrameIndex nCount(0);
    if (nEnd > nPos)
    {
        sal_Int32 nDone = 0;
        const lang::Locale &rLocale = g_pBreakIt->GetLocale( aLang );
        while ( nPos < nEnd )
        {
            nPos = TextFrameIndex(g_pBreakIt->GetBreakIter()->nextCharacters(
                    rText, sal_Int32(nPos),
                    rLocale,
                    i18n::CharacterIteratorMode::SKIPCELL, 1, nDone));
            nCount++;
        }
    }
    else
        nCount = nEnd - nPos ;

    return nCount;
}

void SwScriptInfo::CJKJustify( const OUString& rText, long* pKernArray,
                                     long* pScrArray, TextFrameIndex const nStt,
                                     TextFrameIndex const nLen, LanguageType aLang,
                                     long nSpaceAdd, bool bIsSpaceStop )
{
    assert( pKernArray != nullptr && sal_Int32(nStt) >= 0 );
    if (sal_Int32(nLen) > 0)
    {
        long nSpaceSum = 0;
        const lang::Locale &rLocale = g_pBreakIt->GetLocale( aLang );
        sal_Int32 nDone = 0;
        sal_Int32 nNext(nStt);
        for ( sal_Int32 nI = 0; nI < sal_Int32(nLen); ++nI )
        {
            if (nI + sal_Int32(nStt) == nNext)
            {
                nNext = g_pBreakIt->GetBreakIter()->nextCharacters( rText, nNext,
                        rLocale,
                        i18n::CharacterIteratorMode::SKIPCELL, 1, nDone );
                if (nNext < sal_Int32(nStt + nLen) || !bIsSpaceStop)
                    nSpaceSum += nSpaceAdd;
            }
            pKernArray[ nI ] += nSpaceSum;
            if ( pScrArray )
                pScrArray[ nI ] += nSpaceSum;
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
