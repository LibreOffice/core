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
#include <editeng/scripthintitem.hxx>
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
#include <i18nutil/kashida.hxx>
#include <i18nutil/scriptchangescanner.hxx>
#include <unotxdoc.hxx>

using namespace ::com::sun::star;
using namespace i18n::ScriptType;

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
    SwPositiveSize::Height(nNew);
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
static bool lcl_HasOnlyBlanks(std::u16string_view rText, TextFrameIndex nStt, TextFrameIndex nEnd,
    bool isFieldMarkPortion)
{
    while ( nStt < nEnd )
    {
        switch (rText[sal_Int32(nStt++)])
        {
        case 0x0020: // SPACE
        case 0x2003: // EM SPACE
        case 0x2005: // FOUR-PER-EM SPACE
        case 0x3000: // IDEOGRAPHIC SPACE
            continue;
        case 0x2002: // EN SPACE :
            if (isFieldMarkPortion)
                return false;
            else
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
    bool bHasTabPortions = false;
    bool bHasNonBlankPortions = false;
    SwTwips nTabPortionAscent = 0;
    SwTwips nTabPortionHeight = 0;
    SwTwips nSpacePortionAscent = 0;
    SwTwips nSpacePortionHeight = 0;
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
                if (bIgnoreBlanksAndTabsForLineHeightCalculation)
                {
                    bHasTabPortions |= pPos->InTabGrp();
                    bool isSpacePortion = false;
                    if ( pPos->InTabGrp() || pPos->IsHolePortion() ||
                            ( pPos->IsTextPortion() &&
                              (isSpacePortion = lcl_HasOnlyBlanks( rInf.GetText(), nPorSttIdx, nPorSttIdx + pPos->GetLen(),
                                  pPos->IsFieldmarkText() ) ) ) )
                    {
                        pLast = pPos;
                        if (pPos->InTabGrp())
                        {
                            if (nTabPortionAscent < pPos->GetAscent())
                            {
                                nTabPortionAscent = pPos->GetAscent();
                            }
                            if (nTabPortionHeight < pPos->Height())
                            {
                                nTabPortionHeight = pPos->Height();
                            }
                        }
                        else if (isSpacePortion)
                        {
                            if (nSpacePortionAscent < pPos->GetAscent())
                            {
                                nSpacePortionAscent = pPos->GetAscent();
                            }
                            if (nSpacePortionHeight < pPos->Height())
                            {
                                nSpacePortionHeight = pPos->Height();
                            }
                            bHasBlankPortion = true;
                        }
                        bTmpDummy &= !pPos->InTabGrp();
                        pPos = pPos->GetNextPortion();
                        continue;
                    }
                }

                // Ignore drop portion height
                // tdf#130804 ... and bookmark portions
                if ((pPos->IsDropPortion() && static_cast<SwDropPortion*>(pPos)->GetLines() > 1)
                    || pPos->GetWhichPor() == PortionType::Bookmark
                    || (pPos->GetWhichPor() == PortionType::HiddenText
                        && rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
                            DocumentSettingId::IGNORE_HIDDEN_CHARS_FOR_LINE_CALCULATION)))
                {
                    pLast = pPos;
                    pPos = pPos->GetNextPortion();
                    continue;
                }

                bHasOnlyBlankPortions = false;
                bHasNonBlankPortions = true;

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
            lcl_HasOnlyBlanks(rInf.GetText(), rInf.GetLineStart(),
                                 rInf.GetLineStart() + GetLen(),
                                 false))
        {
            bHasBlankPortion = true;
        }
        else
        {
            bHasOnlyBlankPortions = false;
            bHasNonBlankPortions = true;
        }
    }

    if (!rInf.IsNewLine()
        && TextFrameIndex(rInf.GetText().getLength()) <= rInf.GetIdx()
        && !bHasNonBlankPortions
        && rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
            DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH))
    {
        // Word: for empty last line, line height is based on paragraph marker
        // formatting, ignoring blanks/tabs
        rLine.SeekAndChg(rInf);
        SetAscent(rInf.GetAscent());
        Height(rInf.GetTextHeight());
    }
    else if (bIgnoreBlanksAndTabsForLineHeightCalculation && !bHasNonBlankPortions &&
        (bHasTabPortions || (bHasBlankPortion && (nSpacePortionAscent > 0 || nSpacePortionHeight > 0))))
    {
        //Word increases line height if _only_ spaces and|or tabstops are in a line
        if (bHasTabPortions)
        {
            mnAscent = nTabPortionAscent;
            Height(nTabPortionHeight, true);
        }
        else if (bHasBlankPortion)
        {
            if(  mnAscent < nSpacePortionAscent )
                mnAscent = nSpacePortionAscent;
            if (Height() < nSpacePortionHeight)
                Height(nSpacePortionHeight, true);
        }
    }
    // #i3952# Whitespace does not increase line height
    else if (bHasBlankPortion && bHasOnlyBlankPortions)
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
    m_bFormatAdj = m_bDummy = m_bEndHyph = m_bMidHyph = m_bLastHyph = m_bFly = m_bRest = m_bBlinking
        = m_bClipping = m_bContent = m_bRedline = m_bRedlineEnd = m_bForcedLeftMargin = m_bHanging
        = false;
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

static Color getBookmarkColor(const SwTextNode& rNode, sw::mark::Bookmark* pBookmark)
{
    // search custom color in metadata, otherwise use COL_TRANSPARENT;
    Color c = COL_TRANSPARENT;

    try
    {
        SwDoc& rDoc = const_cast<SwDoc&>(rNode.GetDoc());
        const rtl::Reference< SwXBookmark > xRef = SwXBookmark::CreateXBookmark(rDoc, pBookmark);
        if (const SwDocShell* pShell = rDoc.GetDocShell())
        {
            rtl::Reference<SwXTextDocument> xModel = pShell->GetBaseModel();

            static uno::Reference< uno::XComponentContext > xContext(
                ::comphelper::getProcessComponentContext());

            static uno::Reference< rdf::XURI > xODF_SHADING(
                rdf::URI::createKnown(xContext, rdf::URIs::LO_EXT_SHADING), uno::UNO_SET_THROW);

            const uno::Reference<rdf::XRepository> xRepository =
                xModel->getRDFRepository();
            const uno::Reference<container::XEnumeration> xEnum(
                xRepository->getStatements(css::uno::Reference<css::rdf::XResource>(xRef), xODF_SHADING, nullptr), uno::UNO_SET_THROW);

            rdf::Statement stmt;
            if ( xEnum->hasMoreElements() && (xEnum->nextElement() >>= stmt) )
            {
                const uno::Reference<rdf::XLiteral> xObject(stmt.Object, uno::UNO_QUERY);
                if ( xObject.is() )
                    c = Color::STRtoRGB(xObject->getValue());
            }
        }
    }
    catch (const lang::IllegalArgumentException&)
    {
    }

    return c;
}

static OUString getBookmarkType(const SwTextNode& rNode, sw::mark::Bookmark* pBookmark)
{
    // search ODF_PREFIX in metadata, otherwise use empty string;
    OUString sRet;

    try
    {
        SwDoc& rDoc = const_cast<SwDoc&>(rNode.GetDoc());
        const rtl::Reference< SwXBookmark > xRef = SwXBookmark::CreateXBookmark(rDoc, pBookmark);
        if (const SwDocShell* pShell = rDoc.GetDocShell())
        {
            rtl::Reference<SwXTextDocument> xModel = pShell->GetBaseModel();

            static uno::Reference< uno::XComponentContext > xContext(
                ::comphelper::getProcessComponentContext());

            static uno::Reference< rdf::XURI > xODF_PREFIX(
                rdf::URI::createKnown(xContext, rdf::URIs::RDF_TYPE), uno::UNO_SET_THROW);

            rtl::Reference<SwXTextDocument> xDocumentMetadataAccess(
                pShell->GetBaseModel());
            const uno::Reference<rdf::XRepository> xRepository =
                xDocumentMetadataAccess->getRDFRepository();
            const uno::Reference<container::XEnumeration> xEnum(
                xRepository->getStatements(css::uno::Reference<css::rdf::XResource>(xRef), xODF_PREFIX, nullptr), uno::UNO_SET_THROW);

            rdf::Statement stmt;
            if ( xEnum->hasMoreElements() && (xEnum->nextElement() >>= stmt) )
            {
                const uno::Reference<rdf::XLiteral> xObject(stmt.Object, uno::UNO_QUERY);
                if ( xObject.is() )
                    sRet = xObject->getValue();
            }
        }
    }
    catch (const lang::IllegalArgumentException&)
    {
    }

    return sRet;
}

static void InitBookmarks(
    std::optional<std::vector<sw::Extent>::const_iterator> oPrevIter,
    std::vector<sw::Extent>::const_iterator iter,
    std::vector<sw::Extent>::const_iterator const end,
    TextFrameIndex nOffset,
    std::vector<std::pair<sw::mark::Bookmark*, SwScriptInfo::MarkKind>> & rBookmarks,
    std::vector<std::tuple<TextFrameIndex, SwScriptInfo::MarkKind, Color, SwMarkName, OUString>> & o_rBookmarks)
{
    SwTextNode const*const pNode(iter->pNode);
    for (auto const& it : rBookmarks)
    {
        assert(iter->pNode == pNode || pNode->GetIndex() < iter->pNode->GetIndex());
        assert(!oPrevIter || (*oPrevIter)->pNode->GetIndex() <= pNode->GetIndex());

        // search for custom bookmark boundary mark color
        Color c = getBookmarkColor(*pNode, it.first);
        OUString sType = getBookmarkType(*pNode, it.first);
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
                auto [/*const SwPosition&*/ rStart, rEnd] = it.first->GetMarkStartEnd();
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
                            o_rBookmarks.emplace_back(nOffset, it.second, c, it.first->GetName(), sType);
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
                                it.second, c, it.first->GetName(), sType);
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
                        o_rBookmarks.emplace_back(nOffset, it.second, c, it.first->GetName(), sType);
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
                            o_rBookmarks.emplace_back(nOffset, it.second, c, it.first->GetName(), sType);
                            break;
                        }
                    }
                    else if (rEnd.GetContentIndex() <= iter->nEnd)
                    {
                        o_rBookmarks.emplace_back(
                            nOffset + TextFrameIndex(rEnd.GetContentIndex() - iter->nStart),
                            it.second, c, it.first->GetName(), sType);
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
                                it.second, c, it.first->GetName(), sType);
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

// note: must not use pMerged->pParaPropsNode to avoid circular dependency
void SwScriptInfo::InitScriptInfoHidden(const SwTextNode& rNode,
        sw::MergedPara const*const pMerged)
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
        if (pMerged->extents.empty())
        {
            Range aRange(0, pMerged->pLastNode->Len() > 0 ? pMerged->pLastNode->Len() - 1 : 0);
            MultiSelection aHiddenMulti(aRange);
            CalcHiddenRanges(*pMerged->pLastNode, aHiddenMulti, nullptr);
            if (aHiddenMulti.GetRangeCount() != 0)
            {
                m_HiddenChg.push_back(TextFrameIndex(0));
                m_HiddenChg.push_back(TextFrameIndex(0));
            }
        }
        else for (auto iter = pMerged->extents.begin();
                    iter != pMerged->extents.end(); oPrevIter = iter)
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
            std::vector<std::pair<sw::mark::Bookmark*, MarkKind>> bookmarks;
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
        std::vector<std::pair<sw::mark::Bookmark*, MarkKind>> bookmarks;
        CalcHiddenRanges(rNode, aHiddenMulti, &bookmarks);

        for (auto const& it : bookmarks)
        {
            // don't show __RefHeading__ bookmarks, which are hidden in Navigator, too
            // (They are inserted automatically e.g. with the ToC at the beginning of
            // the headings)
            if (it.first->GetName().toString().startsWith(
                                    IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix()))
            {
                continue;
            }

            // search for custom bookmark boundary mark color
            Color c = getBookmarkColor(rNode, it.first);
            OUString sType = getBookmarkType(rNode, it.first);

            switch (it.second)
            {
                case MarkKind::Start:
                    m_Bookmarks.emplace_back(TextFrameIndex(it.first->GetMarkStart().GetContentIndex()), it.second, c, it.first->GetName(), sType);
                    break;
                case MarkKind::End:
                    m_Bookmarks.emplace_back(TextFrameIndex(it.first->GetMarkEnd().GetContentIndex()), it.second, c, it.first->GetName(), sType);
                    break;
                case MarkKind::Point:
                    m_Bookmarks.emplace_back(TextFrameIndex(it.first->GetMarkPos().GetContentIndex()), it.second, c, it.first->GetName(), sType);
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
}

namespace
{
i18nutil::ScriptHintProvider lcl_FindScriptTypeHintSpans(const SwTextNode& rNode)
{
    i18nutil::ScriptHintProvider stProvider;

    const SvxScriptHintItem* pItem = rNode.GetSwAttrSet().GetItemIfSet(RES_CHRATR_SCRIPT_HINT);
    if (pItem)
    {
        stProvider.SetParagraphLevelHint(pItem->GetValue());
    }

    const SwpHints* pHints = rNode.GetpSwpHints();
    if (pHints)
    {
        for (size_t nTmp = 0; nTmp < pHints->Count(); ++nTmp)
        {
            const SwTextAttr* pTextAttr = pHints->Get(nTmp);
            const SvxScriptHintItem* pCharItem
                = CharFormat::GetItem(*pTextAttr, RES_CHRATR_SCRIPT_HINT);
            if (pCharItem)
            {
                const sal_Int32 nSt = pTextAttr->GetStart();
                const sal_Int32 nEnd = *pTextAttr->End();
                if (nEnd > nSt)
                {
                    stProvider.AddHint(pCharItem->GetValue(), nSt, nEnd);
                }
            }
        }
    }

    return stProvider;
}
}

void SwScriptInfo::InitScriptInfo(const SwTextNode& rNode,
        sw::MergedPara const*const pMerged, bool bRTL)
{
    InitScriptInfoHidden(rNode, pMerged);

    const OUString& rText(pMerged ? pMerged->mergedText : rNode.GetText());

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

    sal_Int16 nScript = i18n::ScriptType::LATIN;

    // compression type
    const CharCompressType aCompEnum = rNode.getIDocumentSettingAccess()->getCharacterCompressionType();

    auto const& rParaItems((pMerged ? *pMerged->pParaPropsNode : rNode).GetSwAttrSet());
    // justification type
    m_bAdjustBlock = (SvxAdjust::Block == rParaItems.GetAdjust().GetAdjust());

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

    // Construct the script change scanner and advance it to the change range
    auto stScriptHints = lcl_FindScriptTypeHintSpans(rNode);
    auto pDirScanner = i18nutil::MakeDirectionChangeScanner(rText, m_nDefaultDir);
    auto pScriptScanner = i18nutil::MakeScriptChangeScanner(
        rText, SvtLanguageOptions::GetI18NScriptTypeOfLanguage(GetAppLanguage()), *pDirScanner,
        stScriptHints);
    while (!pScriptScanner->AtEnd())
    {
        if (static_cast<sal_Int32>(nChg) < pScriptScanner->Peek().m_nEndIndex)
        {
            break;
        }

        pScriptScanner->Advance();
    }

    // UPDATE THE SCRIPT INFO ARRAYS:

    while (nChg < TextFrameIndex(rText.getLength())
           || (m_ScriptChanges.empty() && rText.isEmpty()))
    {
        auto stChange = pScriptScanner->Peek();
        pScriptScanner->Advance();

        nScript = stChange.m_nScriptType;
        nChg = TextFrameIndex{ stChange.m_nEndIndex };
        m_ScriptChanges.emplace_back(nChg, nScript);
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
        else if (m_bAdjustBlock && i18n::ScriptType::COMPLEX == nScript)
        {
            if (SwScriptInfo::IsKashidaScriptText(
                    rText, TextFrameIndex{ stChange.m_nStartIndex },
                    TextFrameIndex{ stChange.m_nEndIndex - stChange.m_nStartIndex }))
            {
                m_bParagraphContainsKashidaScript = true;
            }
        }

        if (nChg < TextFrameIndex(rText.getLength()))
            nScript = static_cast<sal_uInt8>(g_pBreakIt->GetBreakIter()->getScriptType(rText, sal_Int32(nChg)));

        nLastCompression = nChg;
    }

    // remove invalid entries from direction information arrays
    m_DirectionChanges.clear();

    // Perform Unicode Bidi Algorithm for text direction information
    pDirScanner->Reset();
    while (!pDirScanner->AtEnd())
    {
        auto stDirChange = pDirScanner->Peek();
        m_DirectionChanges.emplace_back(TextFrameIndex{ stDirChange.m_nEndIndex },
                                        stDirChange.m_nLevel);

        pDirScanner->Advance();
    }
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

std::vector<std::tuple<SwScriptInfo::MarkKind, Color, SwMarkName, OUString>>
                                    SwScriptInfo::GetBookmarks(TextFrameIndex const nPos)
{
    std::vector<std::tuple<SwScriptInfo::MarkKind, Color, SwMarkName, OUString>> aColors;
    for (auto const& it : m_Bookmarks)
    {
        if (nPos == std::get<0>(it))
        {
            const SwMarkName& sName = std::get<3>(it);
            // filter hidden bookmarks imported from OOXML
            // TODO import them as hidden bookmarks
            if ( !( sName.toString().startsWith("_Toc") || sName.toString().startsWith("_Ref") ) )
                aColors.push_back(std::tuple<MarkKind, Color, SwMarkName,
                                    OUString>(std::get<1>(it), std::get<2>(it), std::get<3>(it), std::get<4>(it)));
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
                 [](std::tuple<MarkKind, Color, SwMarkName, OUString> const a,
                    std::tuple<MarkKind, Color, SwMarkName, OUString> const b) {
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

    double nSub = 0;
    double nLast = nI ? rKernArray[ nI - 1 ] : 0;
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
                double nMove = 0;
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
                    rKernArray[nI - 1] += nMove;
                rKernArray[nI] += -nSub;
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
            rKernArray[nI] += -nSub;
            ++nI;
            ++nIdx;
        }
    } while( nIdx < nLen );
    return nSub;
}

// Checks if the text is in Arabic or Syriac. Note that only the first
// character has to be checked because a ctl portion only contains one
// script, see NewTextPortion
bool SwScriptInfo::IsKashidaScriptText(const OUString& rText,
        TextFrameIndex const nStt, TextFrameIndex const nLen)
{
    using namespace ::com::sun::star::i18n;
    static const ScriptTypeList typeList[] = {
        { UnicodeScript_kArabic, UnicodeScript_kArabic, sal_Int16(UnicodeScript_kArabic) }, // 11,
        { UnicodeScript_kSyriac, UnicodeScript_kSyriac, sal_Int16(UnicodeScript_kSyriac) }, // 12,
        { UnicodeScript_kScriptCount, UnicodeScript_kScriptCount,
          sal_Int16(UnicodeScript_kScriptCount) } // 88
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
        return type == sal_Int16(UnicodeScript_kArabic) || type == sal_Int16(UnicodeScript_kSyriac);
    }
    return false;
}

tools::Long SwScriptInfo::CountKashidaPositions(TextFrameIndex nIdx, TextFrameIndex nEnd) const
{
    tools::Long nCount = 0;
    for (const auto& nPos : m_Kashida)
    {
        if (nPos >= nEnd)
            break;

        if (nPos >= nIdx)
            ++nCount;
    }

    return nCount;
}

void SwScriptInfo::ReplaceKashidaPositions(std::vector<TextFrameIndex> aKashidaPositions)
{
    m_Kashida = std::move(aKashidaPositions);
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
            (*pKernArray)[nI] += nSpaceSum;
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
        pScriptInfo = pLast->GetScriptInfo();
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
    std::vector<std::pair<sw::mark::Bookmark*, MarkKind>> *const pBookmarks)
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
        if (!pIndex->GetOwner() || pIndex->GetOwner()->GetOwnerType() != SwContentIndexOwnerType::Mark)
            continue;
        auto pMark = static_cast<sw::mark::MarkBase*>(pIndex->GetOwner());
        sw::mark::Bookmark* pBookmark = dynamic_cast<sw::mark::Bookmark*>(pMark);
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

            auto [/*const SwPosition&*/ rMarkStartPos, rMarkEndPos] = pBookmark->GetMarkStartEnd();
            const sal_Int32 nSt =  rMarkStartPos.GetContentIndex();
            const sal_Int32 nEnd = rMarkEndPos.GetContentIndex();

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
    std::vector<std::pair<sw::mark::Bookmark*, MarkKind>> *const pBookmarks)
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
        rKernArray[nI] += nSpaceSum;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
