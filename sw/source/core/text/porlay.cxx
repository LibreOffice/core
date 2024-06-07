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

#include <sal/config.h>

#include <string_view>

#include "porlay.hxx"
#include "itrform2.hxx"
#include "porglue.hxx"
#include "redlnitr.hxx"
#include "porfly.hxx"
#include "porrst.hxx"
#include "pormulti.hxx"
#include "pordrop.hxx"
#include <breakit.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <paratr.hxx>
#include <sal/log.hxx>
#include <optional>
#include <editeng/adjustitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/escapementitem.hxx>
#include <svl/asiancfg.hxx>
#include <svl/languageoptions.hxx>
#include <tools/multisel.hxx>
#include <unotools/charclass.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <redline.hxx>
#include <calbck.hxx>
#include <doc.hxx>
#include <swscanner.hxx>
#include <txatbase.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <IMark.hxx>
#include <sortedobjs.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <docsh.hxx>
#include <unobookmark.hxx>
#include <unocrsrhelper.hxx>
#include <frmatr.hxx>
#include <vcl/kernarray.hxx>
#include <editeng/ulspitem.hxx>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/text/XTextContent.hpp>

#include <unicode/ubidi.h>
#include <i18nutil/scripttypedetector.hxx>
#include <i18nutil/unicode.hxx>

using namespace ::com::sun::star;
using namespace i18n::ScriptType;

/*
   https://www.khtt.net/en/page/1821/the-big-kashida-secret

   the rules of priorities that govern the addition of kashidas in Arabic text
   made ... for ... Explorer 5.5 browser.

   The kashida justification is based on a connection priority scheme that
   decides where kashidas are put automatically.

   This is how the software decides on kashida-inserting priorities:
   1. First it looks for characters with the highest priority in each word,
   which means kashida-extensions will only been used in one position in each
   word. Not more.
   2. The kashida will be connected to the character with the highest priority.
   3. If kashida connection opportunities are found with an equal level of
   priority in one word, the kashida will be placed towards the end of the
   word.

   The priority list of characters and the positioning is as follows:
   1. after a kashida that is manually placed in the text by the user,
   2. after a Seen or Sad (initial and medial form),
   3. before the final form of Taa Marbutah, Haa, Dal,
   4. before the final form of Alef, Tah Lam, Kaf and Gaf,
   5. before the preceding medial Baa of Ra, Ya and Alef Maqsurah,
   6. before the final form of Waw, Ain, Qaf and Fa,
   7. before the final form of other characters that can be connected.
*/

#define IS_JOINING_GROUP(c, g) ( u_getIntPropertyValue( (c), UCHAR_JOINING_GROUP ) == U_JG_##g )
#define isAinChar(c)        IS_JOINING_GROUP((c), AIN)
#define isAlefChar(c)       IS_JOINING_GROUP((c), ALEF)
#define isDalChar(c)        IS_JOINING_GROUP((c), DAL)
#define isFehChar(c)       (IS_JOINING_GROUP((c), FEH) || IS_JOINING_GROUP((c), AFRICAN_FEH))
#define isGafChar(c)        IS_JOINING_GROUP((c), GAF)
#define isHehChar(c)        IS_JOINING_GROUP((c), HEH)
#define isKafChar(c)        IS_JOINING_GROUP((c), KAF)
#define isLamChar(c)        IS_JOINING_GROUP((c), LAM)
#define isQafChar(c)       (IS_JOINING_GROUP((c), QAF) || IS_JOINING_GROUP((c), AFRICAN_QAF))
#define isRehChar(c)        IS_JOINING_GROUP((c), REH)
#define isTahChar(c)        IS_JOINING_GROUP((c), TAH)
#define isTehMarbutaChar(c) IS_JOINING_GROUP((c), TEH_MARBUTA)
#define isWawChar(c)        IS_JOINING_GROUP((c), WAW)
#define isSeenOrSadChar(c)  (IS_JOINING_GROUP((c), SAD) || IS_JOINING_GROUP((c), SEEN))

// Beh and characters that behave like Beh in medial form.
static bool isBehChar(sal_Unicode cCh)
{
    bool bRet = false;
    switch (u_getIntPropertyValue(cCh, UCHAR_JOINING_GROUP))
    {
    case U_JG_BEH:
    case U_JG_NOON:
    case U_JG_AFRICAN_NOON:
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

// Yeh and characters that behave like Yeh in final form.
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

static  bool lcl_HasStrongLTR ( std::u16string_view rText, sal_Int32 nStart, sal_Int32 nEnd )
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
    SwLineLayout* pNext = m_pNext;
    do
    {
        SwLineLayout* pLastNext = pNext;
        pNext = pNext->GetNext();
        pLastNext->SetNext(nullptr);
        delete pLastNext;
    }
    while (pNext);
}

void SwLineLayout::Height(const SwTwips nNew, const bool bText)
{
    SwPosSize::Height(nNew);
    if (bText)
        m_nTextHeight = nNew;
}

// class SwLineLayout: This is the layout of a single line, which is made
// up of its dimension, the character count and the word spacing in the line.
// Line objects are managed in an own pool, in order to store them continuously
// in memory so that they are paged out together and don't fragment memory.
SwLineLayout::~SwLineLayout()
{
    Truncate();
    DeleteNext();
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
            if( IsBlinking() )
            {
                SetBlinking( false );
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

void SwLineLayout::CreateSpaceAdd( const tools::Long nInit )
{
    m_pLLSpaceAdd.reset( new std::vector<tools::Long> );
    SetLLSpaceAdd( nInit, 0 );
}

// #i3952# Returns true if there are only blanks in [nStt, nEnd[
// Used to implement IgnoreTabsAndBlanksForLineCalculation compat flag
static bool lcl_HasOnlyBlanks(std::u16string_view rText, TextFrameIndex nStt, TextFrameIndex nEnd)
{
    while ( nStt < nEnd )
    {
        switch (rText[sal_Int32(nStt++)])
        {
        case 0x0020: // SPACE
        case 0x2002: // EN SPACE
        case 0x2003: // EM SPACE
        case 0x2005: // FOUR-PER-EM SPACE
        case 0x3000: // IDEOGRAPHIC SPACE
            continue;
        default:
            return false;
        }
    }
    return true;
}

// Swapped out from FormatLine()
void SwLineLayout::CalcLine( SwTextFormatter &rLine, SwTextFormatInfo &rInf )
{
    const SwTwips nLineWidth = rInf.RealWidth();

    sal_uInt16 nFlyAscent = 0;
    sal_uInt16 nFlyHeight = 0;
    sal_uInt16 nFlyDescent = 0;

    // If this line has a clearing break, then this is the portion's height.
    sal_uInt16 nBreakHeight = 0;

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
    bool bHasFlyPortion = false;

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
            const SwTwips nLineHeight = Height();
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
                // coverity[deref_arg] - "Cut" means next "GetNextPortion" returns a different Portion
                if( !pPos->Compress() )
                {
                    // Only take over Height and Ascent if the rest of the line
                    // is empty.
                    if( !pPos->GetNextPortion() )
                    {
                        if( !Height() )
                            Height( pPos->Height(), false );
                        if( !GetAscent() )
                            SetAscent( pPos->GetAscent() );
                    }
                    SwLinePortion* pPortion = pLast->Cut( pPos );
                    rLine.ClearIfIsFirstOfBorderMerge(pPortion);
                    delete pPortion;
                    pPos = pLast->GetNextPortion();
                    continue;
                }

                TextFrameIndex const nPorSttIdx = rInf.GetLineStart() + mnLineLength;
                mnLineLength += pPos->GetLen();
                AddPrtWidth( pPos->Width() );

                // #i3952#
                if (bIgnoreBlanksAndTabsForLineHeightCalculation && !rInf.GetLineStart())
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
                // tdf#130804 ... and bookmark portions
                if ((pPos->IsDropPortion() && static_cast<SwDropPortion*>(pPos)->GetLines() > 1)
                    || pPos->GetWhichPor() == PortionType::Bookmark)
                {
                    pLast = pPos;
                    pPos = pPos->GetNextPortion();
                    continue;
                }

                bHasOnlyBlankPortions = false;

                // We had an attribute change: Sum up/build maxima of length and mass

                SwTwips nPosHeight = pPos->Height();
                SwTwips nPosAscent = pPos->GetAscent();

                SAL_WARN_IF( nPosHeight < nPosAscent,
                        "sw.core", "SwLineLayout::CalcLine: bad ascent or height" );

                if( pPos->IsHangingPortion() )
                {
                    SetHanging(true);
                    rInf.GetParaPortion()->SetMargin();
                }
                else if( !bHasFlyPortion && ( pPos->IsFlyCntPortion() || pPos->IsFlyPortion() ) )
                     bHasFlyPortion = true;

                // A line break portion only influences the height of the line in case it's the only
                // portion in the line, except when it's a clearing break.
                bool bClearingBreak = false;
                if (pPos->IsBreakPortion())
                {
                    auto pBreakPortion = static_cast<SwBreakPortion*>(pPos);
                    bClearingBreak = pBreakPortion->GetClear() != SwLineBreakClear::NONE;
                    nBreakHeight = nPosHeight;
                }
                if (!(pPos->IsBreakPortion() && !bClearingBreak) || !Height())
                {
                    if (!pPos->IsPostItsPortion()) bOnlyPostIts = false;

                    if( bTmpDummy && !mnLineLength )
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
                            Height( nPosHeight, false );
                            mnAscent = nPosAscent;
                            nMaxDescent = nPosHeight - nPosAscent;
                        }
                    }
                    else if( !pPos->IsFlyPortion() )
                    {
                        if( Height() < nPosHeight )
                        {
                            // Height is set to 0 when Init() is called.
                            if (bIgnoreBlanksAndTabsForLineHeightCalculation && pPos->IsFlyCntPortion())
                                // Compat flag set: take the line height, if it's larger.
                                Height(std::max(nPosHeight, nLineHeight), false);
                            else
                                // Just care about the portion height.
                                Height(nPosHeight, pPos->IsTextPortion());
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
                            if( mnAscent < nPosAscent )
                                mnAscent = nPosAscent;
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
                    if( Height() > nMaxDescent + mnAscent )
                    {
                        if( sw::LineAlign::BOTTOM == pFlyCnt->GetAlign() )
                            mnAscent = Height() - nMaxDescent;
                        else if( sw::LineAlign::CENTER == pFlyCnt->GetAlign() )
                            mnAscent = ( Height() + mnAscent - nMaxDescent ) / 2;
                    }
                    pFlyCnt->SetAscent( mnAscent );
                }
            }

            if( bTmpDummy && nFlyHeight )
            {
                mnAscent = nFlyAscent;
                if( nFlyDescent > nFlyHeight - nFlyAscent )
                    Height( nFlyHeight + nFlyDescent, false );
                else
                {
                    if (nBreakHeight > nFlyHeight)
                    {
                        // The line has no content, but it has a clearing break: then the line
                        // height is not only the intersection of the fly and line's rectangle, but
                        // also includes the clearing break's height.
                        Height(nBreakHeight, false);
                    }
                    else
                    {
                        Height(nFlyHeight, false);
                    }
                }
            }
            else if( nMaxDescent > Height() - mnAscent )
                Height( nMaxDescent + mnAscent, false );

            if( bOnlyPostIts && !( bHasBlankPortion && bHasOnlyBlankPortions ) )
            {
                Height( rInf.GetFont()->GetHeight( rInf.GetVsh(), *rInf.GetOut() ) );
                mnAscent = rInf.GetFont()->GetAscent( rInf.GetVsh(), *rInf.GetOut() );
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

    // #i3952# Whitespace does not increase line height
    if ( bHasBlankPortion && bHasOnlyBlankPortions )
    {
        sal_uInt16 nTmpAscent = GetAscent();
        sal_uInt16 nTmpHeight = Height();
        rLine.GetAttrHandler().GetDefaultAscentAndHeight( rInf.GetVsh(), *rInf.GetOut(), nTmpAscent, nTmpHeight );

        short nEscapement = rLine.GetAttrHandler().GetFont()->GetEscapement();
        if (GetAscent() && Height() && !nTmpAscent && !nTmpHeight
            && (nEscapement == DFLT_ESC_AUTO_SUPER || nEscapement == DFLT_ESC_AUTO_SUB))
        {
            // We already had a calculated ascent + height, it would be cleared, automatic
            // sub/superscript is set and we have no content. In this case it makes no sense to
            // clear the old, correct ascent/height.
            nTmpAscent = GetAscent();
            nTmpHeight = Height();
        }

        if (nTmpAscent < GetAscent() || GetAscent() <= 0)
            SetAscent(nTmpAscent);
        if (nTmpHeight < Height() || Height() <= 0)
            Height(nTmpHeight, false);
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
    bool bHasRedline = rLine.GetRedln();
    if( bHasRedline )
    {
        OUString sRedlineText;
        bool bHasRedlineEnd;
        enum RedlineType eRedlineEnd;
        bHasRedline = rLine.GetRedln()->CheckLine(start.first->GetIndex(), start.second,
            end.first->GetIndex(), end.second, sRedlineText, bHasRedlineEnd, eRedlineEnd);
        if( bHasRedline )
        {
            SetRedlineText( sRedlineText );
            if( bHasRedlineEnd )
                SetRedlineEnd( bHasRedlineEnd );
            if( eRedlineEnd != RedlineType::None )
                SetRedlineEndType( eRedlineEnd );
        }
    }
    SetRedline( bHasRedline );

    // redlining: set crossing out for deleted anchored objects
    if ( !bHasFlyPortion )
        return;

    SwLinePortion *pPos = mpNextPortion;
    TextFrameIndex nLineLength;
    while ( pPos )
    {
        TextFrameIndex const nPorSttIdx = rInf.GetLineStart() + nLineLength;
        nLineLength += pPos->GetLen();
        // anchored as characters
        if( pPos->IsFlyCntPortion() )
        {
            bool bDeleted = false;
            size_t nAuthor = std::string::npos;
            if ( bHasRedline )
            {
                OUString sRedlineText;
                bool bHasRedlineEnd;
                enum RedlineType eRedlineEnd;
                std::pair<SwTextNode const*, sal_Int32> const flyStart(
                    rInf.GetTextFrame()->MapViewToModel(nPorSttIdx));
                bool bHasFlyRedline = rLine.GetRedln()->CheckLine(flyStart.first->GetIndex(),
                    flyStart.second, flyStart.first->GetIndex(), flyStart.second, sRedlineText,
                    bHasRedlineEnd, eRedlineEnd, /*pAuthorAtPos=*/&nAuthor);
                bDeleted = bHasFlyRedline && eRedlineEnd == RedlineType::Delete;
            }
            static_cast<SwFlyCntPortion*>(pPos)->SetDeleted(bDeleted);
            static_cast<SwFlyCntPortion*>(pPos)->SetAuthor(nAuthor);
        }
        // anchored to characters
        else if ( pPos->IsFlyPortion() )
        {
            const IDocumentRedlineAccess& rIDRA =
                    rInf.GetTextFrame()->GetDoc().getIDocumentRedlineAccess();
            SwSortedObjs *pObjs = rInf.GetTextFrame()->GetDrawObjs();
            if ( pObjs && IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineFlags() ) )
            {
                for ( size_t i = 0; rInf.GetTextFrame()->GetDrawObjs() && i < pObjs->size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*rInf.GetTextFrame()->GetDrawObjs())[i];
                    if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
                    {
                        bool bDeleted = false;
                        size_t nAuthor = std::string::npos;
                        const SwFormatAnchor& rAnchor = pAnchoredObj->GetFrameFormat()->GetAnchor();
                        if ( rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR )
                        {
                            SwPosition aAnchor = *rAnchor.GetContentAnchor();
                            SwRedlineTable::size_type n = 0;
                            const SwRangeRedline* pFnd =
                                    rIDRA.GetRedlineTable().FindAtPosition( aAnchor, n );
                            if ( pFnd && RedlineType::Delete == pFnd->GetType() )
                            {
                                bDeleted = true;
                                nAuthor = pFnd->GetAuthor();
                            }
                        }
                        pFly->SetDeleted(bDeleted);
                        pFly->SetAuthor(nAuthor);
                    }
                }
            }
        }
        pPos = pPos->GetNextPortion();
    }
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
            // tdf#130804 ignore bookmark portions
             pTmpPortion->GetWhichPor() != PortionType::Bookmark &&
             ( !_bNoFlyCntPorAndLinePor ||
               ( !pTmpPortion->IsFlyCntPortion() &&
                 !(pTmpPortion == this && pTmpPortion->GetNextPortion() ) ) ) )
        {
            SwTwips nPortionAsc = pTmpPortion->GetAscent();
            SwTwips nPortionDesc = pTmpPortion->Height() - nPortionAsc;

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

void SwLineLayout::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText,
                             TextFrameIndex& nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwLineLayout"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    (void)xmlTextWriterEndElement(pWriter);
}

void SwLineLayout::ResetFlags()
{
    m_bFormatAdj = m_bDummy = m_bEndHyph = m_bMidHyph = m_bLastHyph = m_bFly
    = m_bRest = m_bBlinking = m_bClipping = m_bContent = m_bRedline
    = m_bRedlineEnd = m_bForcedLeftMargin = m_bHanging = false;
    m_eRedlineEnd = RedlineType::None;
}

SwLineLayout::SwLineLayout()
    : m_pNext( nullptr ),
      m_nRealHeight( 0 ),
      m_nTextHeight( 0 )
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
    if (TextFrameIndex(0) != rRange.m_nLen)
    {
        if (TextFrameIndex(0) == m_nLen) {
            m_nStart = rRange.m_nStart;
            m_nLen = rRange.m_nLen ;
        }
        else {
            if(rRange.m_nStart + rRange.m_nLen > m_nStart + m_nLen) {
                m_nLen = rRange.m_nStart + rRange.m_nLen - m_nStart;
            }
            if(rRange.m_nStart < m_nStart) {
                m_nLen += m_nStart - rRange.m_nStart;
                m_nStart = rRange.m_nStart;
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

static Color getBookmarkColor(const SwTextNode& rNode, const sw::mark::IBookmark* pBookmark)
{
    // search custom color in metadata, otherwise use COL_TRANSPARENT;
    Color c = COL_TRANSPARENT;

    try
    {
        SwDoc& rDoc = const_cast<SwDoc&>(rNode.GetDoc());
        const rtl::Reference< SwXBookmark > xRef = SwXBookmark::CreateXBookmark(rDoc,
                const_cast<sw::mark::IMark*>(static_cast<const sw::mark::IMark*>(pBookmark)));
        const css::uno::Reference<css::rdf::XResource> xSubject(xRef);
        uno::Reference<frame::XModel> xModel = rDoc.GetDocShell()->GetBaseModel();

        static uno::Reference< uno::XComponentContext > xContext(
            ::comphelper::getProcessComponentContext());

        static uno::Reference< rdf::XURI > xODF_SHADING(
            rdf::URI::createKnown(xContext, rdf::URIs::LO_EXT_SHADING), uno::UNO_SET_THROW);

        uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(
            rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
        const uno::Reference<rdf::XRepository>& xRepository =
            xDocumentMetadataAccess->getRDFRepository();
        const uno::Reference<container::XEnumeration> xEnum(
            xRepository->getStatements(xSubject, xODF_SHADING, nullptr), uno::UNO_SET_THROW);

        rdf::Statement stmt;
        if ( xEnum->hasMoreElements() && (xEnum->nextElement() >>= stmt) )
        {
            const uno::Reference<rdf::XLiteral> xObject(stmt.Object, uno::UNO_QUERY);
            if ( xObject.is() )
                c = Color::STRtoRGB(xObject->getValue());
        }
    }
    catch (const lang::IllegalArgumentException&)
    {
    }

    return c;
}

static void InitBookmarks(
    std::optional<std::vector<sw::Extent>::const_iterator> oPrevIter,
    std::vector<sw::Extent>::const_iterator iter,
    std::vector<sw::Extent>::const_iterator const end,
    TextFrameIndex nOffset,
    std::vector<std::pair<sw::mark::IBookmark const*, SwScriptInfo::MarkKind>> & rBookmarks,
    std::vector<std::tuple<TextFrameIndex, SwScriptInfo::MarkKind, Color, OUString>> & o_rBookmarks)
{
    SwTextNode const*const pNode(iter->pNode);
    for (auto const& it : rBookmarks)
    {
        assert(iter->pNode == pNode || pNode->GetIndex() < iter->pNode->GetIndex());
        assert(!oPrevIter || (*oPrevIter)->pNode->GetIndex() <= pNode->GetIndex());

        // search for custom bookmark boundary mark color
        Color c = getBookmarkColor(*pNode, it.first);

        switch (it.second)
        {
            case SwScriptInfo::MarkKind::Start:
            {
                // SwUndoSaveContent::DelContentIndex() is rather messy but
                // apparently bookmarks "on the edge" are deleted if
                // * point: equals start-of-selection (not end-of-selection)
                // * expanded: one position equals edge of selection
                //             and other does not (is inside)
                // interesting case: if end[/start] of the mark is on the
                // start of first[/end of last] extent, and the other one
                // is outside this merged paragraph, is it deleted or not?
                // assume "no" because the line break it contains isn't deleted.
                SwPosition const& rStart(it.first->GetMarkStart());
                SwPosition const& rEnd(it.first->GetMarkEnd());
                assert(&rStart.GetNode() == pNode);
                while (iter != end)
                {
                    if (&rStart.GetNode() != iter->pNode // iter moved to next node
                        || rStart.GetContentIndex() < iter->nStart)
                    {
                        if (rEnd.GetNodeIndex() < iter->pNode->GetIndex()
                            || (&rEnd.GetNode() == iter->pNode && rEnd.GetContentIndex() <= iter->nStart))
                        {
                            break; // deleted - skip it
                        }
                        else
                        {
                            o_rBookmarks.emplace_back(nOffset, it.second, c, it.first->GetName());
                            break;
                        }
                    }
                    else if (rStart.GetContentIndex() <= iter->nEnd)
                    {
                        auto const iterNext(iter + 1);
                        if (rStart.GetContentIndex() == iter->nEnd
                            && (iterNext == end
                                ?   &rEnd.GetNode() == iter->pNode
                                :  (rEnd.GetNodeIndex() < iterNext->pNode->GetIndex()
                                    || (&rEnd.GetNode() == iterNext->pNode && rEnd.GetContentIndex() < iterNext->nStart))))
                        {
                            break; // deleted - skip it
                        }
                        else
                        {
                            o_rBookmarks.emplace_back(
                                nOffset + TextFrameIndex(rStart.GetContentIndex() - iter->nStart),
                                it.second, c, it.first->GetName());
                            break;
                        }
                    }
                    else
                    {
                        nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                        oPrevIter = iter;
                        ++iter; // bookmarks are sorted...
                    }
                }
                if (iter == end)
                {
                    if (pNode->GetIndex() < rEnd.GetNodeIndex()) // pNode is last node of merged
                    {
                        break; // deleted - skip it
                    }
                    else
                    {
                        o_rBookmarks.emplace_back(nOffset, it.second, c, it.first->GetName());
                    }
                }
                break;
            }
            case SwScriptInfo::MarkKind::End:
            {
                SwPosition const& rEnd(it.first->GetMarkEnd());
                assert(&rEnd.GetNode() == pNode);
                while (true)
                {
                    if (iter == end
                        || &rEnd.GetNode() != iter->pNode // iter moved to next node
                        || rEnd.GetContentIndex() <= iter->nStart)
                    {
                        SwPosition const& rStart(it.first->GetMarkStart());
                        // oPrevIter may point to pNode or a preceding node
                        if (oPrevIter
                            ? ((*oPrevIter)->pNode->GetIndex() < rStart.GetNodeIndex()
                                || ((*oPrevIter)->pNode == &rStart.GetNode()
                                    && ((iter != end && &rEnd.GetNode() == iter->pNode && rEnd.GetContentIndex() == iter->nStart)
                                        ? (*oPrevIter)->nEnd < rStart.GetContentIndex()
                                        : (*oPrevIter)->nEnd <= rStart.GetContentIndex())))
                            : rStart.GetNode() == rEnd.GetNode())
                        {
                            break; // deleted - skip it
                        }
                        else
                        {
                            o_rBookmarks.emplace_back(nOffset, it.second, c, it.first->GetName());
                            break;
                        }
                    }
                    else if (rEnd.GetContentIndex() <= iter->nEnd)
                    {
                        o_rBookmarks.emplace_back(
                            nOffset + TextFrameIndex(rEnd.GetContentIndex() - iter->nStart),
                            it.second, c, it.first->GetName());
                        break;
                    }
                    else
                    {
                        nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                        oPrevIter = iter;
                        ++iter;
                    }
                }
                break;
            }
            case SwScriptInfo::MarkKind::Point:
            {
                SwPosition const& rPos(it.first->GetMarkPos());
                assert(&rPos.GetNode() == pNode);
                while (iter != end)
                {
                    if (&rPos.GetNode() != iter->pNode // iter moved to next node
                        || rPos.GetContentIndex() < iter->nStart)
                    {
                        break; // deleted - skip it
                    }
                    else if (rPos.GetContentIndex() <= iter->nEnd)
                    {
                        if (rPos.GetContentIndex() == iter->nEnd
                            && rPos.GetContentIndex() != iter->pNode->Len())
                        {
                            break; // deleted - skip it
                        }
                        else
                        {
                            o_rBookmarks.emplace_back(
                                nOffset + TextFrameIndex(rPos.GetContentIndex() - iter->nStart),
                                it.second, c, it.first->GetName());
                        }
                        break;
                    }
                    else
                    {
                        nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                        oPrevIter = iter;
                        ++iter;
                    }
                }
                break;
            }
        }
        if (iter == end)
        {
            break; // remaining marks are hidden
        }
    }
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

    m_Bookmarks.clear();
    m_HiddenChg.clear();
    if (pMerged)
    {
        SwTextNode const* pNode(nullptr);
        TextFrameIndex nOffset(0);
        std::optional<std::vector<sw::Extent>::const_iterator> oPrevIter;
        for (auto iter = pMerged->extents.begin(); iter != pMerged->extents.end();
             oPrevIter = iter)
        {
            if (iter->pNode == pNode)
            {
                nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                ++iter;
                continue; // skip extents at end of previous node
            }
            pNode = iter->pNode;
            Range aRange( 0, pNode->Len() > 0 ? pNode->Len() - 1 : 0 );
            MultiSelection aHiddenMulti( aRange );
            std::vector<std::pair<sw::mark::IBookmark const*, MarkKind>> bookmarks;
            CalcHiddenRanges(*pNode, aHiddenMulti, &bookmarks);

            InitBookmarks(oPrevIter, iter, pMerged->extents.end(), nOffset, bookmarks, m_Bookmarks);

            for (sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
            {
                const Range& rRange = aHiddenMulti.GetRange( i );
                const sal_Int32 nStart = rRange.Min();
                const sal_Int32 nEnd = rRange.Max() + 1;
                bool isStartHandled(false);
                ::std::optional<sal_Int32> oExtend;

                if (nEnd <= iter->nStart)
                {   // entirely in gap, skip this hidden range
                    continue;
                }

                do
                {
                    if (!isStartHandled && nStart <= iter->nEnd)
                    {
                        isStartHandled = true;
                        if (nStart <= iter->nStart && !m_HiddenChg.empty()
                            && m_HiddenChg.back() == nOffset)
                        {
                            // previous one went until end of extent, extend it
                            oExtend.emplace(::std::min(iter->nEnd, nEnd) - ::std::max(iter->nStart, nStart));
                        }
                        else
                        {
                            m_HiddenChg.push_back(nOffset + TextFrameIndex(::std::max(nStart - iter->nStart, sal_Int32(0))));
                        }
                    }
                    else if (oExtend)
                    {
                        *oExtend += ::std::min(iter->nEnd, nEnd) - iter->nStart;
                    }
                    if (nEnd <= iter->nEnd)
                    {
                        if (oExtend)
                        {
                            m_HiddenChg.back() += TextFrameIndex(*oExtend);
                        }
                        else
                        {
                            m_HiddenChg.push_back(nOffset + TextFrameIndex(::std::max(nEnd - iter->nStart, sal_Int32(0))));
                        }
                        break; // iterate to next hidden range
                    }
                    nOffset += TextFrameIndex(iter->nEnd - iter->nStart);
                    ++iter;
                }
                while (iter != pMerged->extents.end() && iter->pNode == pNode);
                if (iter == pMerged->extents.end() || iter->pNode != pNode)
                {
                    if (isStartHandled)
                    {   // dangling end
                        if (oExtend)
                        {
                            m_HiddenChg.back() += TextFrameIndex(*oExtend);
                        }
                        else
                        {
                            m_HiddenChg.push_back(nOffset);
                        }
                    } // else: beyond last extent in node, ignore
                    break; // skip hidden ranges beyond last extent in node
                }
            }
        }
    }
    else
    {
        Range aRange( 0, !rText.isEmpty() ? rText.getLength() - 1 : 0 );
        MultiSelection aHiddenMulti( aRange );
        std::vector<std::pair<sw::mark::IBookmark const*, MarkKind>> bookmarks;
        CalcHiddenRanges(rNode, aHiddenMulti, &bookmarks);

        for (auto const& it : bookmarks)
        {
            // don't show __RefHeading__ bookmarks, which are hidden in Navigator, too
            // (They are inserted automatically e.g. with the ToC at the beginning of
            // the headings)
            if (it.first->GetName().startsWith(
                                    IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix()))
            {
                continue;
            }

            // search for custom bookmark boundary mark color
            Color c = getBookmarkColor(rNode, it.first);

            switch (it.second)
            {
                case MarkKind::Start:
                    m_Bookmarks.emplace_back(TextFrameIndex(it.first->GetMarkStart().GetContentIndex()), it.second, c, it.first->GetName());
                    break;
                case MarkKind::End:
                    m_Bookmarks.emplace_back(TextFrameIndex(it.first->GetMarkEnd().GetContentIndex()), it.second, c, it.first->GetName());
                    break;
                case MarkKind::Point:
                    m_Bookmarks.emplace_back(TextFrameIndex(it.first->GetMarkPos().GetContentIndex()), it.second, c, it.first->GetName());
                    break;
            }
        }

        m_HiddenChg.reserve( aHiddenMulti.GetRangeCount() * 2 );
        for (sal_Int32 i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
        {
            const Range& rRange = aHiddenMulti.GetRange( i );
            const sal_Int32 nStart = rRange.Min();
            const sal_Int32 nEnd = rRange.Max() + (rText.isEmpty() ? 0 : 1);

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
        TextFrameIndex nEnd(
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

        // special case for dotted circle since it can be used with complex
        // before a mark, so we want it associated with the mark's script
        // tdf#112594: another special case for NNBSP followed by a Mongolian
        // character, since NNBSP has special uses in Mongolian (tdf#112594)
        auto nPos = sal_Int32(nChg);
        auto nPrevPos = nPos;
        auto nPrevChar = rText.iterateCodePoints(&nPrevPos, -1);
        if (nChg < TextFrameIndex(rText.getLength()) && nChg > TextFrameIndex(0) &&
            i18n::ScriptType::WEAK == g_pBreakIt->GetBreakIter()->getScriptType(rText, nPrevPos))
        {
            auto nChar = rText.iterateCodePoints(&nPos, 0);
            auto nType = unicode::getUnicodeType(nChar);
            if (nType == css::i18n::UnicodeType::NON_SPACING_MARK ||
                nType == css::i18n::UnicodeType::ENCLOSING_MARK ||
                nType == css::i18n::UnicodeType::COMBINING_SPACING_MARK ||
                (nPrevChar == CHAR_NNBSP &&
                 u_getIntPropertyValue(nChar, UCHAR_SCRIPT) == USCRIPT_MONGOLIAN))
            {
                nPos = nPrevPos;
            }
        }
        m_ScriptChanges.emplace_back(TextFrameIndex(nPos), nScript);
        ++nCnt;

        // if current script is asian, we search for compressible characters
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
                case 0xFF08: case 0xFF3B: case 0xFF5B:
                    eState = SPECIAL_LEFT;
                    break;
                // Right punctuation found
                case 0x3009: case 0x300B:
                case 0x300D: case 0x300F: case 0x3011: case 0x3015:
                case 0x3017: case 0x3019: case 0x301B: case 0x301E:
                case 0x301F:
                case 0xFF09: case 0xFF3D: case 0xFF5D:
                    eState = SPECIAL_RIGHT;
                    break;
                case 0x3001: case 0x3002:   // Fullstop or comma
                case 0xFF0C: case 0xFF0E: case 0xFF1A: case 0xFF1B:
                    eState = SPECIAL_MIDDLE ;
                    break;
                default:
                    eState = ( 0x3040 <= cChar && 0x3100 > cChar ) ? KANA : NONE;
                }

                // insert range of compressible characters
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
            SwScanner aScanner( std::move(pGetLangOfChar), rText, nullptr, ModelToViewHelper(),
                                i18n::WordType::DICTIONARY_WORD,
                                sal_Int32(nLastKashida), sal_Int32(nChg));

            // the search has to be performed on a per word base
            while ( aScanner.NextWord() )
            {
                const OUString& rWord = aScanner.GetWord();

                sal_Int32 nIdx = 0, nPrevIdx = 0;
                sal_Int32 nKashidaPos = -1;
                sal_Unicode cCh, cPrevCh = 0;

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
                                nKashidaPos = aScanner.GetBegin() + nPrevIdx;
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
                                nKashidaPos = aScanner.GetBegin() + nPrevIdx;
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
                                    nKashidaPos = aScanner.GetBegin() + nPrevIdx;
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
                                nKashidaPos = aScanner.GetBegin() + nPrevIdx;
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
                                nKashidaPos = aScanner.GetBegin() + nPrevIdx;
                                nPriorityLevel = 6;
                            }
                        }
                    }

                    // Do not consider vowel marks when checking if a character
                    // can be connected to previous character.
                    if ( !isTransparentChar ( cCh) )
                    {
                        cPrevCh = cCh;
                        nPrevIdx = nIdx;
                    }

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

TextFrameIndex SwScriptInfo::NextBookmark(TextFrameIndex const nPos) const
{
    for (auto const& it : m_Bookmarks)
    {
        if (nPos < std::get<0>(it))
        {
            return std::get<0>(it);
        }
    }
    return TextFrameIndex(COMPLETE_STRING);
}

std::vector<std::tuple<SwScriptInfo::MarkKind, Color, OUString>>
                                    SwScriptInfo::GetBookmarks(TextFrameIndex const nPos)
{
    std::vector<std::tuple<SwScriptInfo::MarkKind, Color, OUString>> aColors;
    for (auto const& it : m_Bookmarks)
    {
        if (nPos == std::get<0>(it))
        {
            const OUString& sName = std::get<3>(it);
            // filter hidden bookmarks imported from OOXML
            // TODO import them as hidden bookmarks
            if ( !( sName.startsWith("_Toc") || sName.startsWith("_Ref") ) )
                aColors.push_back(std::tuple<MarkKind, Color,
                                    OUString>(std::get<1>(it), std::get<2>(it), std::get<3>(it)));
        }
        else if (nPos < std::get<0>(it))
        {
            break;
        }
    }

    // sort bookmark boundary marks at the same position
    // mark order: ] | [
    // color order: [c1 [c2 [c3 ... c3] c2] c1]
    sort(aColors.begin(), aColors.end(),
                 [](std::tuple<MarkKind, Color, OUString> const a, std::tuple<MarkKind, Color, OUString> const b) {
         return (MarkKind::End == std::get<0>(a) && MarkKind::End != std::get<0>(b)) ||
             (MarkKind::Point == std::get<0>(a) && MarkKind::Start == std::get<0>(b)) ||
             // if both are end or start, order by color
             (MarkKind::End == std::get<0>(a) && MarkKind::End == std::get<0>(b) && std::get<1>(a) < std::get<1>(b)) ||
             (MarkKind::Start == std::get<0>(a) && MarkKind::Start == std::get<0>(b) && std::get<1>(b) < std::get<1>(a));});

    return aColors;
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
        SwScriptInfo::CalcHiddenRanges(rNode, aHiddenMulti, nullptr);
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

// returns, if there are compressible kanas or specials
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

tools::Long SwScriptInfo::Compress(KernArray& rKernArray, TextFrameIndex nIdx, TextFrameIndex nLen,
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
    const tools::Long nMinWidth = ( 3 * nFontHeight ) / 4;

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

    tools::Long nSub = 0;
    tools::Long nLast = nI ? rKernArray[ nI - 1 ] : 0;
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
        if ( rKernArray[ nI ] - nLast < nMinWidth )
        {
            nIdx++; nI++;
        }
        else
        {
            while( nIdx < nCompLen )
            {
                SAL_WARN_IF( SwScriptInfo::NONE == nType, "sw.core", "None compression?!" );

                // nLast is width of current character
                nLast -= rKernArray[ nI ];

                nLast *= nCompress;
                tools::Long nMove = 0;
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
                nLast = rKernArray[ nI ];
                if( nI && nMove )
                    rKernArray.adjust(nI - 1, nMove);
                rKernArray.adjust(nI, -nSub);
                ++nI;
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
            nLast = rKernArray[ nI ];
            rKernArray.adjust(nI, -nSub);
            ++nI;
            ++nIdx;
        }
    } while( nIdx < nLen );
    return nSub;
}

// Note on calling KashidaJustify():
// Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
// total number of kashida positions, or the number of kashida positions after some positions
// have been dropped, depending on the state of the m_KashidaInvalid set.

sal_Int32 SwScriptInfo::KashidaJustify( KernArray* pKernArray,
                                        sal_Bool* pKashidaArray,
                                        TextFrameIndex const nStt,
                                        TextFrameIndex const nLen,
                                        tools::Long nSpaceAdd ) const
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
        tools::Long nKashAdd = nSpaceAdd;

        while ( nIdx < nEnd )
        {
            TextFrameIndex nArrayPos = nIdx - nStt;

            // mark Kashida insertion positions, code in VCL will use this
            // array to know where to insert Kashida.
            if (pKashidaArray)
                pKashidaArray[sal_Int32(nArrayPos)] = true;

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
                pKernArray->adjust(sal_Int32(nArrayPos), nKashAdd);
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
        assert(pKashidaPositions && "Where are kashidas?");

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

TextFrameIndex SwScriptInfo::ThaiJustify( std::u16string_view aText, KernArray* pKernArray,
                                     TextFrameIndex const nStt,
                                     TextFrameIndex const nLen,
                                     TextFrameIndex nNumberOfBlanks,
                                     tools::Long nSpaceAdd )
{
    SAL_WARN_IF( nStt + nLen > TextFrameIndex(aText.size()), "sw.core", "String in ThaiJustify too small" );

    SwTwips nNumOfTwipsToDistribute = nSpaceAdd * sal_Int32(nNumberOfBlanks) /
                                      SPACING_PRECISION_FACTOR;

    tools::Long nSpaceSum = 0;
    TextFrameIndex nCnt(0);

    for (sal_Int32 nI = 0; nI < sal_Int32(nLen); ++nI)
    {
        const sal_Unicode cCh = aText[sal_Int32(nStt) + nI];

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

        if (pKernArray)
            pKernArray->adjust(nI, nSpaceSum);
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

bool SwParaPortion::HasNumberingPortion(FootnoteOrNot const eFootnote) const
{
    SwLinePortion const* pPortion(nullptr);
    // the first line may contain only fly portion...
    for (SwLineLayout const* pLine = this; pLine && !pPortion; pLine = pLine->GetNext())
    {
        pPortion = pLine->GetFirstPortion();
        while (pPortion && (pPortion->InGlueGrp() || pPortion->IsKernPortion() || pPortion->IsFlyPortion()))
        {   // skip margins and fly spacers - numbering should be first then
            pPortion = pPortion->GetNextPortion();
        }
    }
    if (pPortion && pPortion->InHyphGrp())
    {   // weird special case, bullet with soft hyphen
        pPortion = pPortion->GetNextPortion();
    }
    return pPortion && pPortion->InNumberGrp()
        && (eFootnote == SwParaPortion::FootnoteToo || !pPortion->IsFootnoteNumPortion());
}

bool SwParaPortion::HasContentPortions() const
{
    SwLinePortion const* pPortion(nullptr);
    for (SwLineLayout const* pLine = this; pLine && !pPortion; pLine = pLine->GetNext())
    {
        pPortion = pLine->GetFirstPortion();
        while (pPortion && (pPortion->InGlueGrp() || pPortion->IsKernPortion() || pPortion->IsFlyPortion()))
        {   // skip margins and fly spacers
            pPortion = pPortion->GetNextPortion();
        }
    }
    return pPortion != nullptr;
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

void SwParaPortion::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText,
                              TextFrameIndex& nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwParaPortion"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    (void)xmlTextWriterEndElement(pWriter);
}

void SwLineLayout::Init( SwLinePortion* pNextPortion )
{
    Height( 0, false );
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
            nDiff = mnAscent;

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

SwTwips SwTextFrame::GetLowerMarginForFlyIntersect() const
{
    const IDocumentSettingAccess& rIDSA = GetDoc().getIDocumentSettingAccess();
    if (!rIDSA.get(DocumentSettingId::TAB_OVER_MARGIN))
    {
        // Word >= 2013 style or Writer style: lower margin is ignored when determining the text
        // frame height.
        return 0;
    }

    const SwAttrSet* pAttrSet = GetTextNodeForParaProps()->GetpSwAttrSet();
    if (!pAttrSet)
    {
        return 0;
    }

    // If it has multiple lines, then probably it already has the needed fly portion.
    // Limit this to empty paragraphs for now.
    if ((GetPara() && GetPara()->GetNext()) || !GetText().isEmpty())
    {
        return 0;
    }

    return pAttrSet->GetULSpace().GetLower();
}

void SwScriptInfo::selectHiddenTextProperty(const SwTextNode& rNode,
    MultiSelection & rHiddenMulti,
    std::vector<std::pair<sw::mark::IBookmark const*, MarkKind>> *const pBookmarks)
{
    assert((rNode.GetText().isEmpty() && rHiddenMulti.GetTotalRange().Len() == 1)
        || (rNode.GetText().getLength() == rHiddenMulti.GetTotalRange().Len()));

    const SvxCharHiddenItem* pItem = rNode.GetSwAttrSet().GetItemIfSet( RES_CHRATR_HIDDEN );
    if( pItem && pItem->GetValue() )
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

    for (const SwContentIndex* pIndex = rNode.GetFirstIndex(); pIndex; pIndex = pIndex->GetNext())
    {
        const sw::mark::IMark* pMark = pIndex->GetMark();
        const sw::mark::IBookmark* pBookmark = dynamic_cast<const sw::mark::IBookmark*>(pMark);
        if (pBookmarks && pBookmark)
        {
            if (!pBookmark->IsExpanded())
            {
                pBookmarks->emplace_back(pBookmark, MarkKind::Point);
            }
            else if (pIndex == &pBookmark->GetMarkStart().nContent)
            {
                pBookmarks->emplace_back(pBookmark, MarkKind::Start);
            }
            else
            {
                assert(pIndex == &pBookmark->GetMarkEnd().nContent);
                pBookmarks->emplace_back(pBookmark, MarkKind::End);
            }
        }

        // condition is evaluated in DocumentFieldsManager::UpdateExpFields()
        if (pBookmark && pBookmark->IsHidden())
        {
            // intersect bookmark range with textnode range and add the intersection to rHiddenMulti

            const sal_Int32 nSt =  pBookmark->GetMarkStart().GetContentIndex();
            const sal_Int32 nEnd = pBookmark->GetMarkEnd().GetContentIndex();

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
    if ( !IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineFlags() ) )
        return;

    SwRedlineTable::size_type nAct = rIDRA.GetRedlinePos( rNode, RedlineType::Any );

    for ( ; nAct < rIDRA.GetRedlineTable().size(); nAct++ )
    {
        const SwRangeRedline* pRed = rIDRA.GetRedlineTable()[ nAct ];

        if (pRed->Start()->GetNode() > rNode)
            break;

        if (pRed->GetType() != RedlineType::Delete)
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

// Returns a MultiSection indicating the hidden ranges.
void SwScriptInfo::CalcHiddenRanges( const SwTextNode& rNode,
    MultiSelection & rHiddenMulti,
    std::vector<std::pair<sw::mark::IBookmark const*, MarkKind>> *const pBookmarks)
{
    selectHiddenTextProperty(rNode, rHiddenMulti, pBookmarks);

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

void SwScriptInfo::CJKJustify( const OUString& rText, KernArray& rKernArray,
                                     TextFrameIndex const nStt,
                                     TextFrameIndex const nLen, LanguageType aLang,
                                     tools::Long nSpaceAdd, bool bIsSpaceStop )
{
    assert( sal_Int32(nStt) >= 0 );
    if (sal_Int32(nLen) <= 0)
        return;

    tools::Long nSpaceSum = 0;
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
        rKernArray.adjust(nI, nSpaceSum);
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
