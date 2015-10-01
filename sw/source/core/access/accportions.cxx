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

#include "accportions.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/Boundary.hpp>
#include <txttypes.hxx>

// for portion replacement in Special()
#include "access.hrc"
#include <tools/resid.hxx>
#include "viewopt.hxx"

// for GetWordBoundary(...), GetSentenceBoundary(...):
#include <breakit.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <ndtxt.hxx>

// for FillSpecialPos(...)
#include "crstate.hxx"

// for SwAccessibleContext::GetResource()
#include "acccontext.hxx"

// for Post-It replacement text:
#include "txatbase.hxx"
#include "fmtfld.hxx"
#include "fldbas.hxx"
#include "docufld.hxx"

// for in-line graphics replacement:
#include "ndindex.hxx"
#include "ndnotxt.hxx"
#include "fmtflcnt.hxx"
#include "frmfmt.hxx"
#include "fmtcntnt.hxx"

using namespace ::com::sun::star;

using i18n::Boundary;

// 'portion type' for terminating portions
#define POR_TERMINATE 0

// portion attributes
#define PORATTR_SPECIAL     1
#define PORATTR_READONLY    2
#define PORATTR_GRAY        4
#define PORATTR_TERM        128

SwAccessiblePortionData::SwAccessiblePortionData(
    const SwTextNode* pTextNd,
    const SwViewOption* pViewOpt ) :
    SwPortionHandler(),
    pTextNode( pTextNd ),
    aBuffer(),
    nModelPosition( 0 ),
    pViewOptions( pViewOpt ),
    sAccessibleString(),
    aLineBreaks(),
    aModelPositions(),
    aAccessiblePositions(),
    aFieldPosition(),
    aAttrFieldType(),
    aPortionAttrs(),
    pSentences( 0 ),
    nBeforePortions( 0 ),
    bFinished( false ),
    bLastIsSpecial( false )
{
    OSL_ENSURE( pTextNode != NULL, "Text node is needed!" );

    // reserve some space to reduce memory allocations
    aLineBreaks.reserve( 5 );
    aModelPositions.reserve( 10 );
    aAccessiblePositions.reserve( 10 );

    // always include 'first' line-break position
    aLineBreaks.push_back( 0 );
}

SwAccessiblePortionData::~SwAccessiblePortionData()
{
    delete pSentences;
}

void SwAccessiblePortionData::Text(sal_Int32 nLength, sal_uInt16 nType, sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/)
{
    OSL_ENSURE( (nModelPosition + nLength) <= pTextNode->GetText().getLength(),
                "portion exceeds model string!" );

    OSL_ENSURE( !bFinished, "We are already done!" );

    // ignore zero-length portions
    if( nLength == 0 )
        return;

    // store 'old' positions
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    // store portion attributes
    sal_uInt8 nAttr = IsGrayPortionType(nType) ? PORATTR_GRAY : 0;
    aPortionAttrs.push_back( nAttr );

    // update buffer + nModelPosition
    aBuffer.append( pTextNode->GetText().copy(nModelPosition, nLength) );
    nModelPosition += nLength;

    bLastIsSpecial = false;
}

void SwAccessiblePortionData::SetAttrFieldType( sal_uInt16 nAttrFieldType )
{
    aAttrFieldType.push_back(nAttrFieldType);
    return;
}

void SwAccessiblePortionData::Special(
    sal_Int32 nLength, const OUString& rText, sal_uInt16 nType, sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/, const SwFont* /*pFont*/)
{
    OSL_ENSURE( nModelPosition >= 0, "illegal position" );
    OSL_ENSURE( (nModelPosition + nLength) <= pTextNode->GetText().getLength(),
                "portion exceeds model string!" );

    OSL_ENSURE( !bFinished, "We are already done!" );

    // construct string with representation; either directly from
    // rText, or use resources for special case portions
    OUString sDisplay;
    switch( nType )
    {
        case POR_POSTITS:
            sDisplay = OUString(sal_Unicode(0xfffc));
            break;
        case POR_FLYCNT:
            sDisplay = OUString(sal_Unicode(0xfffc));
            break;
        case POR_GRFNUM:
            break;
        case POR_FLD:
        case POR_HIDDEN:
        case POR_COMBINED:
        case POR_ISOREF:
            // When the filed content is empty, input a special character.
            if (rText.isEmpty())
                sDisplay = OUString(sal_Unicode(0xfffc));
            else
                sDisplay = rText;
            aFieldPosition.push_back(aBuffer.getLength());
            aFieldPosition.push_back(aBuffer.getLength() + rText.getLength());
            break;
        case POR_FTNNUM:
            break;
        case POR_FTN:
            {
                sDisplay = rText;
                sal_Int32 nStart=aBuffer.getLength();
                sal_Int32 nEnd=nStart + rText.getLength();
                m_vecPairPos.push_back(std::make_pair(nStart,nEnd));
                break;
            }
            break;
        case POR_NUMBER:
        case POR_BULLET:
        {
            sDisplay = rText + " ";
            break;
        }
        // #i111768# - apply patch from kstribley:
        // Include the control characters.
        case POR_CONTROLCHAR:
            sDisplay = rText + OUString( pTextNode->GetText()[nModelPosition] );
            break;
        default:
            sDisplay = rText;
            break;
    }

    // ignore zero/zero portions (except for terminators)
    if( (nLength == 0) && (sDisplay.getLength() == 0) && (nType != POR_TERMINATE) )
        return;

    // special treatment for zero length portion at the beginning:
    // count as 'before' portion
    if( ( nLength == 0 ) && ( nModelPosition == 0 ) )
        nBeforePortions++;

    // store the 'old' positions
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    // store portion attributes
    sal_uInt8 nAttr = PORATTR_SPECIAL;
    if( IsGrayPortionType(nType) )      nAttr |= PORATTR_GRAY;
    if( nLength == 0 )                  nAttr |= PORATTR_READONLY;
    if( nType == POR_TERMINATE )        nAttr |= PORATTR_TERM;
    aPortionAttrs.push_back( nAttr );

    // update buffer + nModelPosition
    aBuffer.append( sDisplay );
    nModelPosition += nLength;

    // remember 'last' special portion (unless it's our own 'closing'
    // portions from 'Finish()'
    if( nType != POR_TERMINATE )
        bLastIsSpecial = true;
}

void SwAccessiblePortionData::LineBreak(sal_Int32 /*nWidth*/)
{
    OSL_ENSURE( !bFinished, "We are already done!" );

    aLineBreaks.push_back( aBuffer.getLength() );
}

void SwAccessiblePortionData::Skip(sal_Int32 nLength)
{
    OSL_ENSURE( !bFinished, "We are already done!" );
    OSL_ENSURE( aModelPositions.empty(), "Never Skip() after portions" );
    OSL_ENSURE( nLength <= pTextNode->GetText().getLength(),
            "skip exceeds model string!" );

    nModelPosition += nLength;
}

void SwAccessiblePortionData::Finish()
{
    OSL_ENSURE( !bFinished, "We are already done!" );

    // include terminator values: always include two 'last character'
    // markers in the position arrays to make sure we always find one
    // position before the end
    Special( 0, OUString(), POR_TERMINATE );
    Special( 0, OUString(), POR_TERMINATE );
    LineBreak(0);
    LineBreak(0);

    sAccessibleString = aBuffer.makeStringAndClear();
    bFinished = true;
}

bool SwAccessiblePortionData::IsPortionAttrSet(
    size_t nPortionNo, sal_uInt8 nAttr ) const
{
    OSL_ENSURE( nPortionNo < aPortionAttrs.size(),
                "Illegal portion number" );
    return (aPortionAttrs[nPortionNo] & nAttr) != 0;
}

bool SwAccessiblePortionData::IsSpecialPortion( size_t nPortionNo ) const
{
    return IsPortionAttrSet(nPortionNo, PORATTR_SPECIAL);
}

bool SwAccessiblePortionData::IsReadOnlyPortion( size_t nPortionNo ) const
{
    return IsPortionAttrSet(nPortionNo, PORATTR_READONLY);
}

bool SwAccessiblePortionData::IsGrayPortionType( sal_uInt16 nType ) const
{
    // gray portions?
    // Compare with: inftxt.cxx, SwTextPaintInfo::DrawViewOpt(...)
    bool bGray = false;
    switch( nType )
    {
        case POR_FTN:
        case POR_ISOREF:
        case POR_REF:
        case POR_QUOVADIS:
        case POR_NUMBER:
        case POR_FLD:
        case POR_URL:
        case POR_INPUTFLD:
        case POR_ISOTOX:
        case POR_TOX:
        case POR_HIDDEN:
            bGray = !pViewOptions->IsPagePreview() &&
                !pViewOptions->IsReadonly() && SwViewOption::IsFieldShadings();
            break;
        case POR_TAB:       bGray = pViewOptions->IsTab();          break;
        case POR_SOFTHYPH:  bGray = pViewOptions->IsSoftHyph();     break;
        case POR_BLANK:     bGray = pViewOptions->IsHardBlank();    break;
        default:
            break; // bGray is false
    }
    return bGray;
}

const OUString& SwAccessiblePortionData::GetAccessibleString() const
{
    OSL_ENSURE( bFinished, "Shouldn't call this before we are done!" );

    return sAccessibleString;
}

void SwAccessiblePortionData::GetLineBoundary(
    Boundary& rBound,
    sal_Int32 nPos ) const
{
    FillBoundary( rBound, aLineBreaks,
                  FindBreak( aLineBreaks, nPos ) );
}

// #i89175#
sal_Int32 SwAccessiblePortionData::GetLineCount() const
{
    size_t nBreaks = aLineBreaks.size();
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
    sal_Int32 nLineNo = FindBreak( aLineBreaks, nPos );

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
    FillBoundary( rLineBound, aLineBreaks, nLineNo );
}

void SwAccessiblePortionData::GetLastLineBoundary(
    Boundary& rBound ) const
{
    OSL_ENSURE( aLineBreaks.size() >= 2, "need min + max value" );

    // The last two positions except the two delimiters are the ones
    // we are looking for, except for empty paragraphs (nBreaks==3)
    size_t nBreaks = aLineBreaks.size();
    FillBoundary( rBound, aLineBreaks, nBreaks <= 3 ? 0 : nBreaks-4 );
}

sal_Int32 SwAccessiblePortionData::GetModelPosition( sal_Int32 nPos ) const
{
    OSL_ENSURE( nPos >= 0, "illegal position" );
    OSL_ENSURE( nPos <= sAccessibleString.getLength(), "illegal position" );

    // find the portion number
    size_t nPortionNo = FindBreak( aAccessiblePositions, nPos );

    // get model portion size
    sal_Int32 nStartPos = aModelPositions[nPortionNo];

    // if it's a non-special portion, move into the portion, else
    // return the portion start
    if( ! IsSpecialPortion( nPortionNo ) )
    {
        // 'wide' portions have to be of the same width
        OSL_ENSURE( ( aModelPositions[nPortionNo+1] - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accessibility portion disagrees with text model" );

        nStartPos += nPos - aAccessiblePositions[nPortionNo];
    }
    // else: return nStartPos unmodified

    OSL_ENSURE( nStartPos >= 0, "There's something weird in number of characters of SwTextNode" );
    return nStartPos;
}

void SwAccessiblePortionData::FillBoundary(
    Boundary& rBound,
    const Positions_t& rPositions,
    size_t nPos )
{
    rBound.startPos = rPositions[nPos];
    rBound.endPos = rPositions[nPos+1];
}

size_t SwAccessiblePortionData::FindBreak(
    const Positions_t& rPositions,
    sal_Int32 nValue )
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

size_t SwAccessiblePortionData::FindLastBreak(
    const Positions_t& rPositions,
    sal_Int32 nValue )
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
    OSL_ENSURE( nPos < sAccessibleString.getLength(), "illegal position" );

    if( pSentences == NULL )
    {
        OSL_ENSURE( g_pBreakIt != NULL, "We always need a break." );
        OSL_ENSURE( g_pBreakIt->GetBreakIter().is(), "No break-iterator." );
        if( g_pBreakIt->GetBreakIter().is() )
        {
            pSentences = new Positions_t();
            pSentences->reserve(10);

            // use xBreak->endOfSentence to iterate over all words; store
            // positions in pSentences
            sal_Int32 nCurrent = 0;
            sal_Int32 nLength = sAccessibleString.getLength();
            do
            {
                pSentences->push_back( nCurrent );

                const sal_Int32 nModelPos = GetModelPosition( nCurrent );

                sal_Int32 nNew = g_pBreakIt->GetBreakIter()->endOfSentence(
                    sAccessibleString, nCurrent,
                    g_pBreakIt->GetLocale(pTextNode->GetLang(nModelPos)) ) + 1;

                if( (nNew < 0) && (nNew > nLength) )
                    nNew = nLength;
                else if (nNew <= nCurrent)
                    nNew = nCurrent + 1;   // ensure forward progress

                nCurrent = nNew;
            }
            while (nCurrent < nLength);

            // finish with two terminators
            pSentences->push_back( nLength );
            pSentences->push_back( nLength );
        }
        else
        {
            // no break iterator -> empty word
            rBound.startPos = 0;
            rBound.endPos = 0;
            return;
        }
    }

    FillBoundary( rBound, *pSentences, FindBreak( *pSentences, nPos ) );
}

void SwAccessiblePortionData::GetAttributeBoundary(
    Boundary& rBound,
    sal_Int32 nPos) const
{
    OSL_ENSURE( pTextNode != NULL, "Need SwTextNode!" );

    // attribute boundaries can only occur on portion boundaries
    FillBoundary( rBound, aAccessiblePositions,
                  FindBreak( aAccessiblePositions, nPos ) );
}

sal_Int32 SwAccessiblePortionData::GetAccessiblePosition( sal_Int32 nPos ) const
{
    OSL_ENSURE( nPos <= pTextNode->GetText().getLength(), "illegal position" );

    // find the portion number
    // #i70538# - consider "empty" model portions - e.g. number portion
    size_t nPortionNo = FindLastBreak( aModelPositions, nPos );

    sal_Int32 nRet = aAccessiblePositions[nPortionNo];

    // if the model portion has more than one position, go into it;
    // else return that position
    sal_Int32 nStartPos = aModelPositions[nPortionNo];
    sal_Int32 nEndPos = aModelPositions[nPortionNo+1];
    if( (nEndPos - nStartPos) > 1 )
    {
        // 'wide' portions have to be of the same width
        OSL_ENSURE( ( nEndPos - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accessibility portion disagrees with text model" );

        sal_Int32 nWithinPortion = nPos - aModelPositions[nPortionNo];
        nRet += nWithinPortion;
    }
    // else: return nRet unmodified

    OSL_ENSURE( (nRet >= 0) && (nRet <= sAccessibleString.getLength()),
                "too long!" );
    return nRet;
}

sal_Int32 SwAccessiblePortionData::FillSpecialPos(
    sal_Int32 nPos,
    SwSpecialPos& rPos,
    SwSpecialPos*& rpPos ) const
{
    size_t nPortionNo = FindLastBreak( aAccessiblePositions, nPos );

    SwSPExtendRange nExtend(SwSPExtendRange::NONE);
    sal_Int32 nRefPos(0);
    sal_Int32 nModelPos(0);

    if( nPortionNo < nBeforePortions )
    {
        nExtend = SwSPExtendRange::BEFORE;
        rpPos = &rPos;
    }
    else
    {
        sal_Int32 nModelEndPos = aModelPositions[nPortionNo+1];
        nModelPos = aModelPositions[nPortionNo];

        // skip backwards over zero-length portions, since GetCharRect()
        // counts all model-zero-length portions as belonging to the
        // previous portion
        size_t nCorePortionNo = nPortionNo;
        while( nModelPos == nModelEndPos )
        {
            nCorePortionNo--;
            nModelEndPos = nModelPos;
            nModelPos = aModelPositions[nCorePortionNo];

            OSL_ENSURE( nModelPos >= 0, "Can't happen." );
            OSL_ENSURE( nCorePortionNo >= nBeforePortions, "Can't happen." );
        }
        OSL_ENSURE( nModelPos != nModelEndPos,
                    "portion with core-representation expected" );

        // if we have anything except plain text, compute nExtend + nRefPos
        if( (nModelEndPos - nModelPos == 1) &&
            (pTextNode->GetText()[nModelPos] != sAccessibleString[nPos]))
        {
            // case 1: a one-character, non-text portion
            // reference position is the first accessibilty for our
            // core portion
            nRefPos = aAccessiblePositions[ nCorePortionNo ];
            nExtend = SwSPExtendRange::NONE;
            rpPos = &rPos;
        }
        else if(nPortionNo != nCorePortionNo)
        {
            // case 2: a multi-character (text!) portion, followed by
            // zero-length portions
            // reference position is the first character of the next
            // portion, and we are 'behind'
            nRefPos = aAccessiblePositions[ nCorePortionNo+1 ];
            nExtend = SwSPExtendRange::BEHIND;
            rpPos = &rPos;
        }
        else
        {
            // case 3: regular text portion
            OSL_ENSURE( ( nModelEndPos - nModelPos ) ==
                        ( aAccessiblePositions[nPortionNo+1] -
                          aAccessiblePositions[nPortionNo] ),
                        "text portion expected" );

            nModelPos += nPos - aAccessiblePositions[ nPortionNo ];
            rpPos = NULL;
        }
    }
    if( rpPos != NULL )
    {
        OSL_ENSURE( rpPos == &rPos, "Yes!" );
        OSL_ENSURE( nRefPos <= nPos, "wrong reference" );

        // get the line number, and adjust nRefPos for the line
        // (if necessary)
        size_t nRefLine = FindBreak( aLineBreaks, nRefPos );
        size_t nMyLine  = FindBreak( aLineBreaks, nPos );
        sal_uInt16 nLineOffset = static_cast<sal_uInt16>( nMyLine - nRefLine );
        if( nLineOffset != 0 )
            nRefPos = aLineBreaks[ nMyLine ];

        // fill char offset and 'special position'
        rPos.nCharOfst = nPos - nRefPos;
        rPos.nExtendRange = nExtend;
        rPos.nLineOfst = nLineOffset;
    }

    return nModelPos;
}

bool SwAccessiblePortionData::FillBoundaryIFDateField( com::sun::star::i18n::Boundary& rBound, const sal_Int32 nPos )
{
    if( aFieldPosition.size() < 2 )
        return false;
    for( size_t i = 0; i < aFieldPosition.size() - 1; i += 2 )
    {
        if( nPos < aFieldPosition[ i + 1 ]  &&  nPos >= aFieldPosition[ i ] )
        {
            rBound.startPos = aFieldPosition[i];
            rBound.endPos =  aFieldPosition[i + 1];
            return true;
        }
    }
    return false;
}

void SwAccessiblePortionData::AdjustAndCheck(
    sal_Int32 nPos,
    size_t& nPortionNo,
    sal_Int32& nCorePos,
    bool& bEdit) const
{
    // find portion and get mode position
    nPortionNo = FindBreak( aAccessiblePositions, nPos );
    nCorePos = aModelPositions[ nPortionNo ];

    // for special portions, make sure we're on a portion boundary
    // for text portions, add the in-portion offset
    if( IsSpecialPortion( nPortionNo ) )
        bEdit &= nPos == aAccessiblePositions[nPortionNo];
    else
        nCorePos += nPos - aAccessiblePositions[nPortionNo];
}

bool SwAccessiblePortionData::GetEditableRange(
    sal_Int32 nStart, sal_Int32 nEnd,
    sal_Int32& nCoreStart, sal_Int32& nCoreEnd ) const
{
    bool bIsEditable = true;

    // get start and end portions
    size_t nStartPortion, nEndPortion;
    AdjustAndCheck( nStart, nStartPortion, nCoreStart, bIsEditable );
    AdjustAndCheck( nEnd,   nEndPortion,   nCoreEnd,   bIsEditable );

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
        bIsEditable &= ! IsReadOnlyPortion( nPor );
    }

    return bIsEditable;
}

bool SwAccessiblePortionData::IsValidCorePosition( sal_Int32 nPos ) const
{
    // a position is valid its within the model positions that we know
    return ( aModelPositions[0] <= nPos ) &&
           ( nPos <= aModelPositions[ aModelPositions.size()-1 ] );
}

bool SwAccessiblePortionData::IsZeroCorePositionData()
{
    if( aModelPositions.size() < 1  ) return true;
    return aModelPositions[0] == 0 &&  aModelPositions[aModelPositions.size()-1] == 0;
}

bool SwAccessiblePortionData::IsIndexInFootnode(sal_Int32 nIndex)
{
    VEC_PAIR_POS::iterator vi =m_vecPairPos.begin();
    for (;vi != m_vecPairPos.end() ; ++vi)
    {
        const PAIR_POS &pairPos = *vi;
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
    return IsPortionAttrSet( FindBreak( aAccessiblePositions, nPos ),
                             PORATTR_GRAY );
}

sal_Int32 SwAccessiblePortionData::GetFieldIndex(sal_Int32 nPos)
{
    sal_Int32 nIndex = -1;
    if( aFieldPosition.size() >= 2 )
    {
        for( size_t i = 0; i < aFieldPosition.size() - 1; i += 2 )
        {
            if( nPos <= aFieldPosition[ i + 1 ]  &&  nPos >= aFieldPosition[ i ] )
            {
                nIndex = i/2;
                break;
            }
        }
    }
    return nIndex;
}

sal_Int32 SwAccessiblePortionData::GetFirstValidCorePosition() const
{
    return aModelPositions[0];
}

sal_Int32 SwAccessiblePortionData::GetLastValidCorePosition() const
{
    return aModelPositions[ aModelPositions.size()-1 ];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
