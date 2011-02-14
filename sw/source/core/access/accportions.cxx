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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include "accportions.hxx"
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/Boundary.hpp>
#include <txttypes.hxx>

// for portion replacement in Special()
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
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

using rtl::OUString;
using rtl::OUStringBuffer;
using i18n::Boundary;


// 'portion type' for terminating portions
#define POR_TERMINATE 0


// portion attributes
#define PORATTR_SPECIAL     1
#define PORATTR_READONLY    2
#define PORATTR_GRAY        4
#define PORATTR_TERM        128

SwAccessiblePortionData::SwAccessiblePortionData(
    const SwTxtNode* pTxtNd,
    const SwViewOption* pViewOpt ) :
    SwPortionHandler(),
    pTxtNode( pTxtNd ),
    aBuffer(),
    nModelPosition( 0 ),
    bFinished( sal_False ),
    pViewOptions( pViewOpt ),
    sAccessibleString(),
    aLineBreaks(),
    aModelPositions(),
    aAccessiblePositions(),
    pSentences( 0 ),
    nBeforePortions( 0 ),
    bLastIsSpecial( sal_False )
{
    DBG_ASSERT( pTxtNode != NULL, "Text node is needed!" );

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

void SwAccessiblePortionData::Text(sal_uInt16 nLength, sal_uInt16 nType)
{
    DBG_ASSERT( (nModelPosition + nLength) <= pTxtNode->GetTxt().Len(),
                "portion exceeds model string!" );

    DBG_ASSERT( !bFinished, "We are already done!" );

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
    aBuffer.append( OUString(
        pTxtNode->GetTxt().Copy(
            static_cast<sal_uInt16>( nModelPosition ),
            nLength ) ) );
    nModelPosition += nLength;

    bLastIsSpecial = sal_False;
}

void SwAccessiblePortionData::Special(
    sal_uInt16 nLength, const String& rText, sal_uInt16 nType)
{
    DBG_ASSERT( nModelPosition >= 0, "illegal position" );
    DBG_ASSERT( (nModelPosition + nLength) <= pTxtNode->GetTxt().Len(),
                "portion exceeds model string!" );

    DBG_ASSERT( !bFinished, "We are already done!" );

    // construct string with representation; either directly from
    // rText, or use resources for special case portions
    String sDisplay;
    switch( nType )
    {
        case POR_POSTITS:
        case POR_FLYCNT:
        case POR_GRFNUM:
            sDisplay = String(sal_Unicode(0xfffc));

            break;
        case POR_NUMBER:
        {
            OUStringBuffer aTmpBuffer( rText.Len() + 1 );
            aTmpBuffer.append( rText );
            aTmpBuffer.append( sal_Unicode(' ') );
            sDisplay = aTmpBuffer.makeStringAndClear();
            break;
        }
        // --> OD 2010-06-04 #i111768# - apply patch from kstribley:
        // Include the control characters.
        case POR_CONTROLCHAR:
        {
            OUStringBuffer aTmpBuffer( rText.Len() + 1 );
            aTmpBuffer.append( rText );
            aTmpBuffer.append( pTxtNode->GetTxt().GetChar(nModelPosition) );
            sDisplay = aTmpBuffer.makeStringAndClear();
            break;
        }
        // <--
        default:
            sDisplay = rText;
            break;
    }

    // ignore zero/zero portions (except for terminators)
    if( (nLength == 0) && (sDisplay.Len() == 0) && (nType != POR_TERMINATE) )
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
    aBuffer.append( OUString(sDisplay) );
    nModelPosition += nLength;

    // remember 'last' special portion (unless it's our own 'closing'
    // portions from 'Finish()'
    if( nType != POR_TERMINATE )
        bLastIsSpecial = sal_True;
}

void SwAccessiblePortionData::LineBreak()
{
    DBG_ASSERT( !bFinished, "We are already done!" );

    aLineBreaks.push_back( aBuffer.getLength() );
}

void SwAccessiblePortionData::Skip(sal_uInt16 nLength)
{
    DBG_ASSERT( !bFinished, "We are already done!" );
    DBG_ASSERT( aModelPositions.size() == 0, "Never Skip() after portions" );
    DBG_ASSERT( nLength <= pTxtNode->GetTxt().Len(), "skip exceeds model string!" );

    nModelPosition += nLength;
}

void SwAccessiblePortionData::Finish()
{
    DBG_ASSERT( !bFinished, "We are already done!" );

    // include terminator values: always include two 'last character'
    // markers in the position arrays to make sure we always find one
    // position before the end
    Special( 0, String(), POR_TERMINATE );
    Special( 0, String(), POR_TERMINATE );
    LineBreak();
    LineBreak();

    sAccessibleString = aBuffer.makeStringAndClear();
    bFinished = sal_True;
}


sal_Bool SwAccessiblePortionData::IsPortionAttrSet(
    size_t nPortionNo, sal_uInt8 nAttr ) const
{
    DBG_ASSERT( nPortionNo < aPortionAttrs.size(),
                "Illegal portion number" );
    return (aPortionAttrs[nPortionNo] & nAttr) != 0;
}

sal_Bool SwAccessiblePortionData::IsSpecialPortion( size_t nPortionNo ) const
{
    return IsPortionAttrSet(nPortionNo, PORATTR_SPECIAL);
}

sal_Bool SwAccessiblePortionData::IsReadOnlyPortion( size_t nPortionNo ) const
{
    return IsPortionAttrSet(nPortionNo, PORATTR_READONLY);
}

sal_Bool SwAccessiblePortionData::IsGrayPortionType( sal_uInt16 nType ) const
{
    // gray portions?
    // Compare with: inftxt.cxx, SwTxtPaintInfo::DrawViewOpt(...)
    sal_Bool bGray = sal_False;
    switch( nType )
    {
        case POR_FTN:
        case POR_ISOREF:
        case POR_REF:
        case POR_QUOVADIS:
        case POR_NUMBER:
        case POR_FLD:
        case POR_URL:
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
    DBG_ASSERT( bFinished, "Shouldn't call this before we are done!" );

    return sAccessibleString;
}


void SwAccessiblePortionData::GetLineBoundary(
    Boundary& rBound,
    sal_Int32 nPos ) const
{
    FillBoundary( rBound, aLineBreaks,
                  FindBreak( aLineBreaks, nPos ) );
}

// --> OD 2008-05-30 #i89175#
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
// <--

void SwAccessiblePortionData::GetLastLineBoundary(
    Boundary& rBound ) const
{
    DBG_ASSERT( aLineBreaks.size() >= 2, "need min + max value" );

    // The last two positions except the two deleimiters are the ones
    // we are looking for, except for empty paragraphs (nBreaks==3)
    size_t nBreaks = aLineBreaks.size();
    FillBoundary( rBound, aLineBreaks, nBreaks <= 3 ? 0 : nBreaks-4 );
}

sal_uInt16 SwAccessiblePortionData::GetModelPosition( sal_Int32 nPos ) const
{
    DBG_ASSERT( nPos >= 0, "illegal position" );
    DBG_ASSERT( nPos <= sAccessibleString.getLength(), "illegal position" );

    // find the portion number
    size_t nPortionNo = FindBreak( aAccessiblePositions, nPos );

    // get model portion size
    sal_Int32 nStartPos = aModelPositions[nPortionNo];

    // if it's a non-special portion, move into the portion, else
    // return the portion start
    if( ! IsSpecialPortion( nPortionNo ) )
    {
        // 'wide' portions have to be of the same width
        DBG_ASSERT( ( aModelPositions[nPortionNo+1] - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accesability portion disagrees with text model" );

        sal_Int32 nWithinPortion = nPos - aAccessiblePositions[nPortionNo];
        nStartPos += nWithinPortion;
    }
    // else: return nStartPos unmodified

    DBG_ASSERT( (nStartPos >= 0) && (nStartPos < USHRT_MAX),
                "How can the SwTxtNode have so many characters?" );
    return static_cast<sal_uInt16>(nStartPos);
}

void SwAccessiblePortionData::FillBoundary(
    Boundary& rBound,
    const Positions_t& rPositions,
    size_t nPos ) const
{
    rBound.startPos = rPositions[nPos];
    rBound.endPos = rPositions[nPos+1];
}


size_t SwAccessiblePortionData::FindBreak(
    const Positions_t& rPositions,
    sal_Int32 nValue ) const
{
    DBG_ASSERT( rPositions.size() >= 2, "need min + max value" );
    DBG_ASSERT( rPositions[0] <= nValue, "need min value" );
    DBG_ASSERT( rPositions[rPositions.size()-1] >= nValue,
                "need first terminator value" );
    DBG_ASSERT( rPositions[rPositions.size()-2] >= nValue,
                "need second terminator value" );

    size_t nMin = 0;
    size_t nMax = rPositions.size()-2;

    // loop until no more than two candidates are left
    while( nMin+1 < nMax )
    {
        // check loop invariants
        DBG_ASSERT( ( (nMin == 0) && (rPositions[nMin] <= nValue) ) ||
                    ( (nMin != 0) && (rPositions[nMin] < nValue) ),
                    "minvalue not minimal" );
        DBG_ASSERT( nValue <= rPositions[nMax], "max value not maximal" );

        // get middle (and ensure progress)
        size_t nMiddle = (nMin + nMax)/2;
        DBG_ASSERT( nMin < nMiddle, "progress?" );
        DBG_ASSERT( nMiddle < nMax, "progress?" );

        // check array
        DBG_ASSERT( rPositions[nMin] <= rPositions[nMiddle],
                    "garbled positions array" );
        DBG_ASSERT( rPositions[nMiddle] <= rPositions[nMax],
                    "garbled positions array" );

        if( nValue > rPositions[nMiddle] )
            nMin = nMiddle;
        else
            nMax = nMiddle;
    }

    // only two are left; we only need to check which one is the winner
    DBG_ASSERT( (nMax == nMin) || (nMax == nMin+1), "only two left" );
    if( (rPositions[nMin] < nValue) && (rPositions[nMin+1] <= nValue) )
        nMin = nMin+1;

    // finally, check to see whether the returned value is the 'right' position
    DBG_ASSERT( rPositions[nMin] <= nValue, "not smaller or equal" );
    DBG_ASSERT( nValue <= rPositions[nMin+1], "not equal or larger" );
    DBG_ASSERT( (nMin == 0) || (rPositions[nMin-1] <= nValue),
                "earlier value should have been returned" );

    DBG_ASSERT( nMin < rPositions.size()-1,
                "shouldn't return last position (due to termintator values)" );

    return nMin;
}

size_t SwAccessiblePortionData::FindLastBreak(
    const Positions_t& rPositions,
    sal_Int32 nValue ) const
{
    size_t nResult = FindBreak( rPositions, nValue );

    // skip 'zero-length' portions
    // --> OD 2006-10-19 #i70538#
    // consider size of <rPosition> and ignore last entry
//    while( rPositions[nResult+1] <= nValue )
    while ( nResult < rPositions.size() - 2 &&
            rPositions[nResult+1] <= nValue )
    {
        nResult++;
    }
    // <--

    return nResult;
}


void SwAccessiblePortionData::GetSentenceBoundary(
    Boundary& rBound,
    sal_Int32 nPos )
{
    DBG_ASSERT( nPos >= 0, "illegal position; check before" );
    DBG_ASSERT( nPos < sAccessibleString.getLength(), "illegal position" );

    if( pSentences == NULL )
    {
         DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
         DBG_ASSERT( pBreakIt->GetBreakIter().is(), "No break-iterator." );
         if( pBreakIt->GetBreakIter().is() )
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

                 sal_uInt16 nModelPos = GetModelPosition( nCurrent );

                 sal_Int32 nNew = pBreakIt->GetBreakIter()->endOfSentence(
                     sAccessibleString, nCurrent,
                     pBreakIt->GetLocale(pTxtNode->GetLang(nModelPos)) ) + 1;

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
    DBG_ASSERT( pTxtNode != NULL, "Need SwTxtNode!" );

    // attribute boundaries can only occur on portion boundaries
    FillBoundary( rBound, aAccessiblePositions,
                  FindBreak( aAccessiblePositions, nPos ) );
}


sal_Int32 SwAccessiblePortionData::GetAccessiblePosition( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= pTxtNode->GetTxt().Len(), "illegal position" );

    // find the portion number
    // --> OD 2006-10-19 #i70538#
    // consider "empty" model portions - e.g. number portion
    size_t nPortionNo = FindLastBreak( aModelPositions,
                                       static_cast<sal_Int32>(nPos) );
    // <--

    sal_Int32 nRet = aAccessiblePositions[nPortionNo];

    // if the model portion has more than one position, go into it;
    // else return that position
    sal_Int32 nStartPos = aModelPositions[nPortionNo];
    sal_Int32 nEndPos = aModelPositions[nPortionNo+1];
    if( (nEndPos - nStartPos) > 1 )
    {
        // 'wide' portions have to be of the same width
        DBG_ASSERT( ( nEndPos - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accesability portion disagrees with text model" );

        sal_Int32 nWithinPortion = nPos - aModelPositions[nPortionNo];
        nRet += nWithinPortion;
    }
    // else: return nRet unmodified

    DBG_ASSERT( (nRet >= 0) && (nRet <= sAccessibleString.getLength()),
                "too long!" );
    return nRet;
}

sal_uInt16 SwAccessiblePortionData::FillSpecialPos(
    sal_Int32 nPos,
    SwSpecialPos& rPos,
    SwSpecialPos*& rpPos ) const
{
    size_t nPortionNo = FindLastBreak( aAccessiblePositions, nPos );

    sal_uInt8 nExtend(SP_EXTEND_RANGE_NONE);
    sal_Int32 nRefPos(0);
    sal_Int32 nModelPos(0);

    if( nPortionNo < nBeforePortions )
    {
        nExtend = SP_EXTEND_RANGE_BEFORE;
        rpPos = &rPos;
    }
    else
    {
        sal_Int32 nModelEndPos = aModelPositions[nPortionNo+1];
        nModelPos = aModelPositions[nPortionNo];

        // skip backwards over zero-length portions, since GetCharRect()
        // counts all model-zero-length portions as belonging to the
        // previus portion
        size_t nCorePortionNo = nPortionNo;
        while( nModelPos == nModelEndPos )
        {
            nCorePortionNo--;
            nModelEndPos = nModelPos;
            nModelPos = aModelPositions[nCorePortionNo];

            DBG_ASSERT( nModelPos >= 0, "Can't happen." );
            DBG_ASSERT( nCorePortionNo >= nBeforePortions, "Can't happen." );
        }
        DBG_ASSERT( nModelPos != nModelEndPos,
                    "portion with core-representation expected" );

        // if we have anything except plain text, compute nExtend + nRefPos
        if( (nModelEndPos - nModelPos == 1) &&
            (pTxtNode->GetTxt().GetChar(static_cast<sal_uInt16>(nModelPos)) !=
             sAccessibleString.getStr()[nPos]) )
        {
            // case 1: a one-character, non-text portion
            // reference position is the first accessibilty for our
            // core portion
            nRefPos = aAccessiblePositions[ nCorePortionNo ];
            nExtend = SP_EXTEND_RANGE_NONE;
            rpPos = &rPos;
        }
        else if(nPortionNo != nCorePortionNo)
        {
            // case 2: a multi-character (text!) portion, followed by
            // zero-length portions
            // reference position is the first character of the next
            // portion, and we are 'behind'
            nRefPos = aAccessiblePositions[ nCorePortionNo+1 ];
            nExtend = SP_EXTEND_RANGE_BEHIND;
            rpPos = &rPos;
        }
        else
        {
            // case 3: regular text portion
            DBG_ASSERT( ( nModelEndPos - nModelPos ) ==
                        ( aAccessiblePositions[nPortionNo+1] -
                          aAccessiblePositions[nPortionNo] ),
                        "text portion expected" );

            nModelPos += nPos - aAccessiblePositions[ nPortionNo ];
            rpPos = NULL;
        }
    }
    if( rpPos != NULL )
    {
        DBG_ASSERT( rpPos == &rPos, "Yes!" );
        DBG_ASSERT( nRefPos <= nPos, "wrong reference" );
        DBG_ASSERT( (nExtend == SP_EXTEND_RANGE_NONE) ||
                    (nExtend == SP_EXTEND_RANGE_BEFORE) ||
                    (nExtend == SP_EXTEND_RANGE_BEHIND), "need extend" );

        // get the line number, and adjust nRefPos for the line
        // (if necessary)
        size_t nRefLine = FindBreak( aLineBreaks, nRefPos );
        size_t nMyLine  = FindBreak( aLineBreaks, nPos );
        sal_uInt16 nLineOffset = static_cast<sal_uInt16>( nMyLine - nRefLine );
        if( nLineOffset != 0 )
            nRefPos = aLineBreaks[ nMyLine ];

        // fill char offset and 'special position'
        rPos.nCharOfst = static_cast<sal_uInt16>( nPos - nRefPos );
        rPos.nExtendRange = nExtend;
        rPos.nLineOfst = nLineOffset;
    }

    return static_cast<sal_uInt16>( nModelPos );
}

void SwAccessiblePortionData::AdjustAndCheck(
    sal_Int32 nPos,
    size_t& nPortionNo,
    sal_uInt16& nCorePos,
    sal_Bool& bEdit) const
{
    // find portion and get mode position
    nPortionNo = FindBreak( aAccessiblePositions, nPos );
    nCorePos = static_cast<sal_uInt16>( aModelPositions[ nPortionNo ] );

    // for special portions, make sure we're on a portion boundary
    // for text portions, add the in-portion offset
    if( IsSpecialPortion( nPortionNo ) )
        bEdit &= nPos == aAccessiblePositions[nPortionNo];
    else
        nCorePos = static_cast<sal_uInt16>( nCorePos +
            nPos - aAccessiblePositions[nPortionNo] );
}

sal_Bool SwAccessiblePortionData::GetEditableRange(
    sal_Int32 nStart, sal_Int32 nEnd,
    sal_uInt16& nCoreStart, sal_uInt16& nCoreEnd ) const
{
    sal_Bool bIsEditable = sal_True;

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
            // special case and just increae the start portion beyond the last
            // portion to make sure the loop below will have zero iteration.
            nStartPortion = nLastPortion + 1;
    }

    for( size_t nPor = nStartPortion; nPor <= nLastPortion; nPor ++ )
    {
        bIsEditable &= ! IsReadOnlyPortion( nPor );
    }

    return bIsEditable;
}

sal_Bool SwAccessiblePortionData::IsValidCorePosition( sal_uInt16 nPos ) const
{
    // a position is valid its within the model positions that we know
    return ( aModelPositions[0] <= nPos ) &&
           ( nPos <= aModelPositions[ aModelPositions.size()-1 ] );
}

sal_uInt16 SwAccessiblePortionData::GetFirstValidCorePosition() const
{
    return static_cast<sal_uInt16>( aModelPositions[0] );
}

sal_uInt16 SwAccessiblePortionData::GetLastValidCorePosition() const
{
    return static_cast<sal_uInt16>( aModelPositions[ aModelPositions.size()-1 ] );
}
