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

#include "accportions.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/Boundary.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <txttypes.hxx>

// for portion replacement in Special()
#include <viewopt.hxx>

// for GetWordBoundary(...), GetSentenceBoundary(...):
#include <breakit.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>

// for FillSpecialPos(...)
#include <crstate.hxx>

// for SwAccessibleContext::GetResource()
#include "acccontext.hxx"

// for Post-It replacement text:
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>

// for in-line graphics replacement:
#include <ndindex.hxx>
#include <ndnotxt.hxx>
#include <fmtflcnt.hxx>
#include <frmfmt.hxx>
#include <fmtcntnt.hxx>

using namespace ::com::sun::star;

using i18n::Boundary;

// 'portion type' for terminating portions
#define POR_TERMINATE PortionType::NONE

// portion attributes
#define PORATTR_SPECIAL     1
#define PORATTR_READONLY    2
#define PORATTR_GRAY        4
#define PORATTR_TERM        128

/// returns the index of the first position whose value is smaller
/// or equal, and whose following value is equal or larger
template<typename T>
static size_t FindBreak(const std::vector<T>& rPositions, T nValue);

/// like FindBreak, but finds the last equal or larger position
template<typename T>
static size_t FindLastBreak(const std::vector<T>& rPositions, T nValue);


SwAccessiblePortionData::SwAccessiblePortionData(
    const SwTextFrame *const pTextFrame,
    const SwViewOption* pViewOpt ) :
    SwPortionHandler(),
    m_pTextFrame(pTextFrame),
    m_aBuffer(),
    m_nViewPosition( 0 ),
    m_pViewOptions( pViewOpt ),
    m_sAccessibleString(),
    m_aLineBreaks(),
    m_aAccessiblePositions(),
    m_aFieldPosition(),
    m_aPortionAttrs(),
    m_nBeforePortions( 0 ),
    m_bFinished( false )
{
    OSL_ENSURE( m_pTextFrame != nullptr, "Need SwTextFrame!" );

    // reserve some space to reduce memory allocations
    m_aLineBreaks.reserve( 5 );
    m_ViewPositions.reserve( 10 );
    m_aAccessiblePositions.reserve( 10 );

    // always include 'first' line-break position
    m_aLineBreaks.push_back( 0 );
}

SwAccessiblePortionData::~SwAccessiblePortionData()
{
}

void SwAccessiblePortionData::Text(TextFrameIndex const nLength,
        PortionType nType, sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/)
{
    OSL_ENSURE((m_nViewPosition + nLength) <= TextFrameIndex(m_pTextFrame->GetText().getLength()),
                "portion exceeds model string!" );

    OSL_ENSURE( !m_bFinished, "We are already done!" );

    // ignore zero-length portions
    if (nLength == TextFrameIndex(0))
        return;

    // store 'old' positions
    m_ViewPositions.push_back( m_nViewPosition );
    m_aAccessiblePositions.push_back( m_aBuffer.getLength() );

    // store portion attributes
    sal_uInt8 nAttr = IsGrayPortionType(nType) ? PORATTR_GRAY : 0;
    m_aPortionAttrs.push_back( nAttr );

    // update buffer + nViewPosition
    m_aBuffer.append(std::u16string_view(m_pTextFrame->GetText()).substr(sal_Int32(m_nViewPosition), sal_Int32(nLength)));
    m_nViewPosition += nLength;
}

void SwAccessiblePortionData::Special(
    TextFrameIndex const nLength, const OUString& rText, PortionType nType,
    sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/, const SwFont* /*pFont*/)
{
    OSL_ENSURE(m_nViewPosition >= TextFrameIndex(0), "illegal position");
    OSL_ENSURE((m_nViewPosition + nLength) <= TextFrameIndex(m_pTextFrame->GetText().getLength()),
                "portion exceeds model string!" );

    OSL_ENSURE( !m_bFinished, "We are already done!" );

    // construct string with representation; either directly from
    // rText, or use resources for special case portions
    OUString sDisplay;
    switch( nType )
    {
        case PortionType::PostIts:
        case PortionType::FlyCnt:
            sDisplay = OUString(u'\xfffc');
            break;
        case PortionType::Field:
        case PortionType::Hidden:
        case PortionType::Combined:
        case PortionType::IsoRef:
            // When the field content is empty, input a special character.
            if (rText.isEmpty())
                sDisplay = OUString(u'\xfffc');
            else
                sDisplay = rText;
            m_aFieldPosition.push_back(m_aBuffer.getLength());
            m_aFieldPosition.push_back(m_aBuffer.getLength() + rText.getLength());
            break;
        case PortionType::FootnoteNum:
            break;
        case PortionType::Footnote:
            {
                sDisplay = rText;
                sal_Int32 nStart=m_aBuffer.getLength();
                sal_Int32 nEnd=nStart + rText.getLength();
                m_vecPairPos.emplace_back(nStart,nEnd);
                break;
            }
            break;
        case PortionType::Number:
        case PortionType::Bullet:
            sDisplay = rText + " ";
            break;
        // There should probably be some special treatment to graphical bullets
        case PortionType::GrfNum:
            break;
        // #i111768# - apply patch from kstribley:
        // Include the control characters.
        case PortionType::ControlChar:
            sDisplay = rText + OUStringLiteral1(m_pTextFrame->GetText()[sal_Int32(m_nViewPosition)]);
            break;
        default:
            sDisplay = rText;
            break;
    }

    // ignore zero/zero portions (except for terminators)
    if ((nLength == TextFrameIndex(0)) && (sDisplay.getLength() == 0) && (nType != POR_TERMINATE))
        return;

    // special treatment for zero length portion at the beginning:
    // count as 'before' portion
    if ((nLength == TextFrameIndex(0)) && (m_nViewPosition == TextFrameIndex(0)))
        m_nBeforePortions++;

    // store the 'old' positions
    m_ViewPositions.push_back( m_nViewPosition );
    m_aAccessiblePositions.push_back( m_aBuffer.getLength() );

    // store portion attributes
    sal_uInt8 nAttr = PORATTR_SPECIAL;
    if( IsGrayPortionType(nType) )      nAttr |= PORATTR_GRAY;
    if (nLength == TextFrameIndex(0))   nAttr |= PORATTR_READONLY;
    if( nType == POR_TERMINATE )        nAttr |= PORATTR_TERM;
    m_aPortionAttrs.push_back( nAttr );

    // update buffer + nViewPosition
    m_aBuffer.append( sDisplay );
    m_nViewPosition += nLength;
}

void SwAccessiblePortionData::LineBreak(sal_Int32 /*nWidth*/)
{
    OSL_ENSURE( !m_bFinished, "We are already done!" );

    m_aLineBreaks.push_back( m_aBuffer.getLength() );
}

void SwAccessiblePortionData::Skip(TextFrameIndex const nLength)
{
    OSL_ENSURE( !m_bFinished, "We are already done!" );
    OSL_ENSURE( m_ViewPositions.empty(), "Never Skip() after portions" );
    OSL_ENSURE(nLength <= TextFrameIndex(m_pTextFrame->GetText().getLength()),
            "skip exceeds model string!" );

    m_nViewPosition += nLength;
}

void SwAccessiblePortionData::Finish()
{
    OSL_ENSURE( !m_bFinished, "We are already done!" );

    // include terminator values: always include two 'last character'
    // markers in the position arrays to make sure we always find one
    // position before the end
    Special( TextFrameIndex(0), OUString(), POR_TERMINATE );
    Special( TextFrameIndex(0), OUString(), POR_TERMINATE );
    LineBreak(0);
    LineBreak(0);

    m_sAccessibleString = m_aBuffer.makeStringAndClear();
    m_bFinished = true;
}

bool SwAccessiblePortionData::IsPortionAttrSet(
    size_t nPortionNo, sal_uInt8 nAttr ) const
{
    OSL_ENSURE( nPortionNo < m_aPortionAttrs.size(),
                "Illegal portion number" );
    return (m_aPortionAttrs[nPortionNo] & nAttr) != 0;
}

bool SwAccessiblePortionData::IsSpecialPortion( size_t nPortionNo ) const
{
    return IsPortionAttrSet(nPortionNo, PORATTR_SPECIAL);
}

bool SwAccessiblePortionData::IsGrayPortionType( PortionType nType ) const
{
    // gray portions?
    // Compare with: inftxt.cxx, SwTextPaintInfo::DrawViewOpt(...)
    bool bGray = false;
    switch( nType )
    {
        case PortionType::Footnote:
        case PortionType::IsoRef:
        case PortionType::Ref:
        case PortionType::QuoVadis:
        case PortionType::Number:
        case PortionType::Field:
        case PortionType::InputField:
        case PortionType::IsoTox:
        case PortionType::Tox:
        case PortionType::Hidden:
            bGray = !m_pViewOptions->IsPagePreview() &&
                !m_pViewOptions->IsReadonly() && SwViewOption::IsFieldShadings();
            break;
        case PortionType::Table:       bGray = m_pViewOptions->IsTab();          break;
        case PortionType::SoftHyphen:  bGray = m_pViewOptions->IsSoftHyph();     break;
        case PortionType::Blank:     bGray = m_pViewOptions->IsHardBlank();    break;
        default:
            break; // bGray is false
    }
    return bGray;
}

const OUString& SwAccessiblePortionData::GetAccessibleString() const
{
    OSL_ENSURE( m_bFinished, "Shouldn't call this before we are done!" );

    return m_sAccessibleString;
}

void SwAccessiblePortionData::GetLineBoundary(
    Boundary& rBound,
    sal_Int32 nPos ) const
{
    FillBoundary( rBound, m_aLineBreaks,
                  FindBreak( m_aLineBreaks, nPos ) );
}

// #i89175#
sal_Int32 SwAccessiblePortionData::GetLineCount() const
{
    size_t nBreaks = m_aLineBreaks.size();
    // A non-empty paragraph has at least 4 breaks: one for each line3 and
    // 3 additional ones.
    // An empty paragraph has 3 breaks.
    // Less than 3 breaks is an error case.
    sal_Int32 nLineCount = ( nBreaks > 3 )
                           ? nBreaks - 3
                           : ( ( nBreaks == 3 ) ? 1 : 0 );
    return nLineCount;
}

sal_Int32 SwAccessiblePortionData::GetLineNo( const sal_Int32 nPos ) const
{
    sal_Int32 nLineNo = FindBreak( m_aLineBreaks, nPos );

    // handling of position after last character
    const sal_Int32 nLineCount( GetLineCount() );
    if ( nLineNo >= nLineCount )
    {
        nLineNo = nLineCount - 1;
    }

    return nLineNo;
}

void SwAccessiblePortionData::GetBoundaryOfLine( const sal_Int32 nLineNo,
                                                 i18n::Boundary& rLineBound )
{
    FillBoundary( rLineBound, m_aLineBreaks, nLineNo );
}

void SwAccessiblePortionData::GetLastLineBoundary(
    Boundary& rBound ) const
{
    OSL_ENSURE( m_aLineBreaks.size() >= 2, "need min + max value" );

    // The last two positions except the two delimiters are the ones
    // we are looking for, except for empty paragraphs (nBreaks==3)
    size_t nBreaks = m_aLineBreaks.size();
    FillBoundary( rBound, m_aLineBreaks, nBreaks <= 3 ? 0 : nBreaks-4 );
}

TextFrameIndex SwAccessiblePortionData::GetCoreViewPosition(sal_Int32 const nPos) const
{
    OSL_ENSURE( nPos >= 0, "illegal position" );
    OSL_ENSURE( nPos <= m_sAccessibleString.getLength(), "illegal position" );

    // find the portion number
    size_t nPortionNo = FindBreak( m_aAccessiblePositions, nPos );

    // get core view portion size
    TextFrameIndex nStartPos = m_ViewPositions[nPortionNo];

    // if it's a non-special portion, move into the portion, else
    // return the portion start
    if( ! IsSpecialPortion( nPortionNo ) )
    {
        // 'wide' portions have to be of the same width
        OSL_ENSURE( sal_Int32(m_ViewPositions[nPortionNo+1] - nStartPos) ==
                    ( m_aAccessiblePositions[nPortionNo+1] -
                      m_aAccessiblePositions[nPortionNo] ),
                    "accessibility portion disagrees with text model" );

        nStartPos += TextFrameIndex(nPos - m_aAccessiblePositions[nPortionNo]);
    }
    // else: return nStartPos unmodified

    OSL_ENSURE(nStartPos >= TextFrameIndex(0), "There's something weird in number of characters of SwTextFrame");
    return nStartPos;
}

void SwAccessiblePortionData::FillBoundary(
    Boundary& rBound,
    const AccessiblePositions& rPositions,
    size_t nPos )
{
    rBound.startPos = rPositions[nPos];
    rBound.endPos = rPositions[nPos+1];
}

template<typename T>
static size_t FindBreak(const std::vector<T>& rPositions, T const nValue)
{
    OSL_ENSURE( rPositions.size() >= 2, "need min + max value" );
    OSL_ENSURE( rPositions[0] <= nValue, "need min value" );
    OSL_ENSURE( rPositions[rPositions.size()-1] >= nValue,
                "need first terminator value" );
    OSL_ENSURE( rPositions[rPositions.size()-2] >= nValue,
                "need second terminator value" );

    size_t nMin = 0;
    size_t nMax = rPositions.size()-2;

    // loop until no more than two candidates are left
    while( nMin+1 < nMax )
    {
        // check loop invariants
        OSL_ENSURE( ( (nMin == 0) && (rPositions[nMin] <= nValue) ) ||
                    ( (nMin != 0) && (rPositions[nMin] < nValue) ),
                    "minvalue not minimal" );
        OSL_ENSURE( nValue <= rPositions[nMax], "max value not maximal" );

        // get middle (and ensure progress)
        size_t nMiddle = (nMin + nMax)/2;
        OSL_ENSURE( nMin < nMiddle, "progress?" );
        OSL_ENSURE( nMiddle < nMax, "progress?" );

        // check array
        OSL_ENSURE( rPositions[nMin] <= rPositions[nMiddle],
                    "garbled positions array" );
        OSL_ENSURE( rPositions[nMiddle] <= rPositions[nMax],
                    "garbled positions array" );

        if( nValue > rPositions[nMiddle] )
            nMin = nMiddle;
        else
            nMax = nMiddle;
    }

    // only two are left; we only need to check which one is the winner
    OSL_ENSURE( (nMax == nMin) || (nMax == nMin+1), "only two left" );
    if( (rPositions[nMin] < nValue) && (rPositions[nMin+1] <= nValue) )
        nMin = nMin+1;

    // finally, check to see whether the returned value is the 'right' position
    OSL_ENSURE( rPositions[nMin] <= nValue, "not smaller or equal" );
    OSL_ENSURE( nValue <= rPositions[nMin+1], "not equal or larger" );
    OSL_ENSURE( (nMin == 0) || (rPositions[nMin-1] <= nValue),
                "earlier value should have been returned" );

    OSL_ENSURE( nMin < rPositions.size()-1,
                "shouldn't return last position (due to terminator values)" );

    return nMin;
}

template<typename T>
static size_t FindLastBreak(const std::vector<T>& rPositions, T const nValue)
{
    size_t nResult = FindBreak( rPositions, nValue );

    // skip 'zero-length' portions
    // #i70538# consider size of <rPosition> and ignore last entry
    while ( nResult < rPositions.size() - 2 &&
            rPositions[nResult+1] <= nValue )
    {
        nResult++;
    }

    return nResult;
}

void SwAccessiblePortionData::GetSentenceBoundary(
    Boundary& rBound,
    sal_Int32 nPos )
{
    OSL_ENSURE( nPos >= 0, "illegal position; check before" );
    OSL_ENSURE( nPos < m_sAccessibleString.getLength(), "illegal position" );

    if( m_pSentences == nullptr )
    {
        assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

        m_pSentences.reset(new AccessiblePositions);
        m_pSentences->reserve(10);

        // use xBreak->endOfSentence to iterate over all words; store
        // positions in pSentences
        sal_Int32 nCurrent = 0;
        sal_Int32 nLength = m_sAccessibleString.getLength();
        do
        {
            m_pSentences->push_back( nCurrent );

            const TextFrameIndex nFramePos = GetCoreViewPosition(nCurrent);

            sal_Int32 nNew = g_pBreakIt->GetBreakIter()->endOfSentence(
                m_sAccessibleString, nCurrent,
                g_pBreakIt->GetLocale(m_pTextFrame->GetLangOfChar(nFramePos, 0, true))) + 1;

            if( (nNew < 0) && (nNew > nLength) )
                nNew = nLength;
            else if (nNew <= nCurrent)
                nNew = nCurrent + 1;   // ensure forward progress

            nCurrent = nNew;
        }
        while (nCurrent < nLength);

        // finish with two terminators
        m_pSentences->push_back( nLength );
        m_pSentences->push_back( nLength );
    }

    FillBoundary( rBound, *m_pSentences, FindBreak( *m_pSentences, nPos ) );
}

void SwAccessiblePortionData::GetAttributeBoundary(
    Boundary& rBound,
    sal_Int32 nPos) const
{
    OSL_ENSURE( m_pTextFrame != nullptr, "Need SwTextNode!" );

    // attribute boundaries can only occur on portion boundaries
    FillBoundary( rBound, m_aAccessiblePositions,
                  FindBreak( m_aAccessiblePositions, nPos ) );
}

sal_Int32 SwAccessiblePortionData::GetAccessiblePosition(TextFrameIndex const nPos) const
{
    OSL_ENSURE(nPos <= TextFrameIndex(m_pTextFrame->GetText().getLength()), "illegal position");

    // find the portion number
    // #i70538# - consider "empty" model portions - e.g. number portion
    size_t nPortionNo = FindLastBreak( m_ViewPositions, nPos );

    sal_Int32 nRet = m_aAccessiblePositions[nPortionNo];

    // if the view portion has more than one position, go into it;
    // else return that position
    TextFrameIndex nStartPos = m_ViewPositions[nPortionNo];
    TextFrameIndex nEndPos = m_ViewPositions[nPortionNo+1];
    if ((nEndPos - nStartPos) > TextFrameIndex(1))
    {
        // 'wide' portions have to be of the same width
        OSL_ENSURE( sal_Int32(nEndPos - nStartPos) ==
                    ( m_aAccessiblePositions[nPortionNo+1] -
                      m_aAccessiblePositions[nPortionNo] ),
                    "accessibility portion disagrees with text model" );

        TextFrameIndex nWithinPortion = nPos - m_ViewPositions[nPortionNo];
        nRet += sal_Int32(nWithinPortion);
    }
    // else: return nRet unmodified

    OSL_ENSURE( (nRet >= 0) && (nRet <= m_sAccessibleString.getLength()),
                "too long!" );
    return nRet;
}

TextFrameIndex SwAccessiblePortionData::FillSpecialPos(
    sal_Int32 nPos,
    SwSpecialPos& rPos,
    SwSpecialPos*& rpPos ) const
{
    size_t nPortionNo = FindLastBreak( m_aAccessiblePositions, nPos );

    SwSPExtendRange nExtend(SwSPExtendRange::NONE);
    sal_Int32 nRefPos(0);
    TextFrameIndex nCorePos(0);

    if( nPortionNo < m_nBeforePortions )
    {
        nExtend = SwSPExtendRange::BEFORE;
        rpPos = &rPos;
    }
    else
    {
        TextFrameIndex nCoreEndPos = m_ViewPositions[nPortionNo+1];
        nCorePos = m_ViewPositions[nPortionNo];

        // skip backwards over zero-length portions, since GetCharRect()
        // counts all model-zero-length portions as belonging to the
        // previous portion
        size_t nCorePortionNo = nPortionNo;
        while (nCorePos == nCoreEndPos)
        {
            nCorePortionNo--;
            nCoreEndPos = nCorePos;
            nCorePos = m_ViewPositions[nCorePortionNo];

            OSL_ENSURE( nCorePos >= TextFrameIndex(0), "Can't happen." );
            OSL_ENSURE( nCorePortionNo >= m_nBeforePortions, "Can't happen." );
        }
        OSL_ENSURE( nCorePos != nCoreEndPos,
                    "portion with core-representation expected" );

        // if we have anything except plain text, compute nExtend + nRefPos
        if ((nCoreEndPos - nCorePos == TextFrameIndex(1)) &&
            (m_pTextFrame->GetText()[sal_Int32(nCorePos)] != m_sAccessibleString[nPos]))
        {
            // case 1: a one-character, non-text portion
            // reference position is the first accessibility for our
            // core portion
            nRefPos = m_aAccessiblePositions[ nCorePortionNo ];
            nExtend = SwSPExtendRange::NONE;
            rpPos = &rPos;
        }
        else if(nPortionNo != nCorePortionNo)
        {
            // case 2: a multi-character (text!) portion, followed by
            // zero-length portions
            // reference position is the first character of the next
            // portion, and we are 'behind'
            nRefPos = m_aAccessiblePositions[ nCorePortionNo+1 ];
            nExtend = SwSPExtendRange::BEHIND;
            rpPos = &rPos;
        }
        else
        {
            // case 3: regular text portion
            OSL_ENSURE( sal_Int32(nCoreEndPos - nCorePos) ==
                        ( m_aAccessiblePositions[nPortionNo+1] -
                          m_aAccessiblePositions[nPortionNo] ),
                        "text portion expected" );

            nCorePos += TextFrameIndex(nPos - m_aAccessiblePositions[nPortionNo]);
            rpPos = nullptr;
        }
    }
    if( rpPos != nullptr )
    {
        OSL_ENSURE( rpPos == &rPos, "Yes!" );
        OSL_ENSURE( nRefPos <= nPos, "wrong reference" );

        // get the line number, and adjust nRefPos for the line
        // (if necessary)
        size_t nRefLine = FindBreak( m_aLineBreaks, nRefPos );
        size_t nMyLine  = FindBreak( m_aLineBreaks, nPos );
        sal_uInt16 nLineOffset = static_cast<sal_uInt16>( nMyLine - nRefLine );
        if( nLineOffset != 0 )
            nRefPos = m_aLineBreaks[ nMyLine ];

        // fill char offset and 'special position'
        rPos.nCharOfst = nPos - nRefPos;
        rPos.nExtendRange = nExtend;
        rPos.nLineOfst = nLineOffset;
    }

    return nCorePos;
}

bool SwAccessiblePortionData::FillBoundaryIFDateField( css::i18n::Boundary& rBound, const sal_Int32 nPos )
{
    if( m_aFieldPosition.size() < 2 )
        return false;
    for( size_t i = 0; i < m_aFieldPosition.size() - 1; i += 2 )
    {
        if( nPos < m_aFieldPosition[ i + 1 ]  &&  nPos >= m_aFieldPosition[ i ] )
        {
            rBound.startPos = m_aFieldPosition[i];
            rBound.endPos =  m_aFieldPosition[i + 1];
            return true;
        }
    }
    return false;
}

void SwAccessiblePortionData::AdjustAndCheck(
    sal_Int32 nPos,
    size_t& nPortionNo,
    TextFrameIndex& rCorePos,
    bool& bEdit) const
{
    // find portion and get mode position
    nPortionNo = FindBreak( m_aAccessiblePositions, nPos );
    rCorePos = m_ViewPositions[ nPortionNo ];

    // for special portions, make sure we're on a portion boundary
    // for text portions, add the in-portion offset
    if( IsSpecialPortion( nPortionNo ) )
        bEdit &= nPos == m_aAccessiblePositions[nPortionNo];
    else
        rCorePos += TextFrameIndex(nPos - m_aAccessiblePositions[nPortionNo]);
}

bool SwAccessiblePortionData::GetEditableRange(
    sal_Int32 nStart, sal_Int32 nEnd,
    TextFrameIndex& rCoreStart, TextFrameIndex& rCoreEnd) const
{
    bool bIsEditable = true;

    // get start and end portions
    size_t nStartPortion, nEndPortion;
    AdjustAndCheck( nStart, nStartPortion, rCoreStart, bIsEditable );
    AdjustAndCheck( nEnd,   nEndPortion,   rCoreEnd,   bIsEditable );

    // iterate over portions, and make sure there is no read-only portion
    // in-between
    size_t nLastPortion = nEndPortion;

    // don't count last portion if we're in front of a special portion
    if( IsSpecialPortion(nLastPortion) )
    {
        if (nLastPortion > 0)
            nLastPortion--;
        else
            // special case: because size_t is usually unsigned, we can't just
            // decrease nLastPortion to -1 (which would normally do the job, so
            // this whole if wouldn't be needed). Instead, we'll do this
            // special case and just increase the start portion beyond the last
            // portion to make sure the loop below will have zero iteration.
            nStartPortion = nLastPortion + 1;
    }

    for( size_t nPor = nStartPortion; nPor <= nLastPortion; nPor++ )
    {
        bIsEditable &= ! IsPortionAttrSet(nPor, PORATTR_READONLY);
    }

    return bIsEditable;
}

bool SwAccessiblePortionData::IsValidCorePosition(TextFrameIndex const nPos) const
{
    // a position is valid if it's within the core view positions that we know
    return (m_ViewPositions[0] <= nPos) && (nPos <= m_ViewPositions.back());
}

bool SwAccessiblePortionData::IsZeroCorePositionData()
{
    if (m_ViewPositions.empty()) return true;
    return m_ViewPositions[0] == TextFrameIndex(0)
        && m_ViewPositions.back() == TextFrameIndex(0);
}

bool SwAccessiblePortionData::IsIndexInFootnode(sal_Int32 nIndex)
{
    for (const auto & pairPos : m_vecPairPos)
    {
        if(nIndex >= pairPos.first && nIndex < pairPos.second )
        {
            return true;
        }
    }
    return false;
}

bool SwAccessiblePortionData::IsInGrayPortion( sal_Int32 nPos )
{
//    return IsGrayPortion( FindBreak( aAccessiblePositions, nPos ) );
    return IsPortionAttrSet( FindBreak( m_aAccessiblePositions, nPos ),
                             PORATTR_GRAY );
}

sal_Int32 SwAccessiblePortionData::GetFieldIndex(sal_Int32 nPos)
{
    sal_Int32 nIndex = -1;
    if( m_aFieldPosition.size() >= 2 )
    {
        for( size_t i = 0; i < m_aFieldPosition.size() - 1; i += 2 )
        {
            if( nPos <= m_aFieldPosition[ i + 1 ]  &&  nPos >= m_aFieldPosition[ i ] )
            {
                nIndex = i/2;
                break;
            }
        }
    }
    return nIndex;
}

TextFrameIndex SwAccessiblePortionData::GetFirstValidCorePosition() const
{
    return m_ViewPositions[0];
}

TextFrameIndex SwAccessiblePortionData::GetLastValidCorePosition() const
{
    return m_ViewPositions.back();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
