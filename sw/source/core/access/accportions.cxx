/*************************************************************************
 *
 *  $RCSfile: accportions.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dvo $ $Date: 2002-03-20 10:24:33 $
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

#ifndef _ACCPORTIONS_HXX
#include "accportions.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_BOUNDARY_HPP_
#include <com/sun/star/i18n/Boundary.hpp>
#endif

#ifndef _TXTTYPES_HXX
#include <txttypes.hxx>
#endif

// for portion replacement in Special()
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif

// for GetWordBoundary(...), GetSentenceBoundary(...):
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif

// for FillSpecialPos(...)
#ifndef _CRSTATE_HXX
#include "crstate.hxx"
#endif


using rtl::OUString;
using com::sun::star::i18n::Boundary;


// 'portion type' for terminating portions
#define POR_TERMINATE 0


SwAccessiblePortionData::SwAccessiblePortionData(
    const String& rCoreString ) :
    SwPortionHandler(),
    aBuffer(),
    nModelPosition( 0 ),
    bFinished( sal_False ),
    sModelString( rCoreString ),
    sAccessibleString(),
    aLineBreaks(),
    aModelPositions(),
    aAccessiblePositions(),
    pSentences( NULL ),
    nBeforePortions( 0 ),
    bLastIsSpecial( sal_False )
{
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

void SwAccessiblePortionData::Text(USHORT nLength, USHORT nType)
{
    DBG_ASSERT( nLength >= 0, "illegal length" );
    DBG_ASSERT( (nModelPosition + nLength) <= sModelString.getLength(),
                "portion exceeds model string!" )

    DBG_ASSERT( !bFinished, "We are already done!" );

    // ignore zero-length portions
    if( nLength == 0 )
        return;

    // store 'old' positions
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    // update buffer + nModelPosition
    aBuffer.append( sModelString.copy(nModelPosition, nLength) );
    nModelPosition += nLength;

    bLastIsSpecial = sal_False;
}

void SwAccessiblePortionData::Special(
    USHORT nLength, const String& rText, USHORT nType)
{
    DBG_ASSERT( nLength >= 0, "illegal length" );
    DBG_ASSERT( nModelPosition >= 0, "illegal position" );
    DBG_ASSERT( (nModelPosition + nLength) <= sModelString.getLength(),
                "portion exceeds model string!" )

    DBG_ASSERT( !bFinished, "We are already done!" );

    // ignore zero/zero portions (except for terminators)
    if( (nLength == 0) && (rText.Len() == 0) && (nType != POR_TERMINATE) )
        return;

    // construct string with representation; either directly from
    // rText, or use resources for special case portions
    String sDisplay;
    switch( nType )
    {
        case POR_POSTITS:
            sDisplay = SW_RES(STR_ACCESS_REPLACEMENT_POSTIT);
            break;
        case POR_FLYCNT:
            sDisplay = SW_RES(STR_ACCESS_REPLACEMENT_FRAME);
            break;
        default:
            sDisplay = rText;
            break;
    }

    // special treatment for zero length portion at the beginning:
    // count as 'before' portion
    if( ( nLength == 0 ) && ( nModelPosition == 0 ) )
        nBeforePortions++;

    // the default case: store the 'old' positions (and previous for
    // zero-length portions)
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

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

void SwAccessiblePortionData::Skip(USHORT nLength)
{
    DBG_ASSERT( !bFinished, "We are already done!" );
    DBG_ASSERT( aModelPositions.size() == 0, "Never Skip() after portions" );

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


void SwAccessiblePortionData::AddAutoSpellPortions( const SwTxtNode* pNode )
{
    // not implemented!
}


const OUString& SwAccessiblePortionData::GetAccessibleString()
{
    DBG_ASSERT( bFinished, "Shouldn't call this before we are done!" );

    return sAccessibleString;
}


void SwAccessiblePortionData::GetLineBoundary(
    Boundary& rBound,
    sal_Int32 nPos )
{
    FillBoundary( rBound, aLineBreaks,
                  FindBreak( aLineBreaks, nPos ) );
}

USHORT SwAccessiblePortionData::GetModelPosition( sal_Int32 nPos )
{
    DBG_ASSERT( nPos >= 0, "illegal position" );
    DBG_ASSERT( nPos <= sAccessibleString.getLength(), "illegal position" );

    // find the portion number
    size_t nPortionNo = FindBreak( aAccessiblePositions, nPos );

    // get model portion size
    sal_Int32 nStartPos = aModelPositions[nPortionNo];
    sal_Int32 nEndPos = aModelPositions[nPortionNo+1];

    // if the model portion has more than one position, go into it;
    // else return that position
    if( (nEndPos - nStartPos) > 1 )
    {
        // 'wide' portions have to be of the same width
        DBG_ASSERT( ( nEndPos - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accesability portion disagrees with text model" );

        sal_Int32 nWithinPortion = nPos - aAccessiblePositions[nPortionNo];
        nStartPos += nWithinPortion;
    }
    // else: return startPos unmodified

    DBG_ASSERT( (nStartPos >= 0) && (nStartPos < USHRT_MAX),
                "How can the SwTxtNode have so many characters?" );
    return static_cast<USHORT>(nStartPos);
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
    sal_Int32 nValue )
{
    size_t nResult = FindBreak( rPositions, nValue );

    // skip 'zero-length' portions
    while( rPositions[nResult+1] <= nValue )
        nResult++;

    return nResult;
}


void SwAccessiblePortionData::GetSentenceBoundary(
    Boundary& rBound,
    sal_Int32 nPos,
    const SwTxtNode* pNode )
{
    DBG_ASSERT( nPos >= 0, "illegal position; check before" );
    DBG_ASSERT( nPos < sAccessibleString.getLength(), "illegal position" );

    if( pSentences == NULL )
    {
         DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
         DBG_ASSERT( pBreakIt->xBreak.is(), "No break-iterator." );
         if( pBreakIt->xBreak.is() )
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

                 USHORT nModelPos = GetModelPosition( nCurrent );

                 sal_Int32 nNew = pBreakIt->xBreak->endOfSentence(
                     sAccessibleString, nCurrent,
                     pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ) ) + 1;

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
    sal_Int32 nPos,
    const SwTxtNode* pNode)
{
    DBG_ASSERT( pNode != NULL, "Need SwTxtNode!" );

    // include auto spell portions, if not already available
    if( ! HasAutoSpellPortions() )
        AddAutoSpellPortions( pNode );

    // attribute boundaries can only occur on portion boundaries
    // (if autospell pseudo-portion are included)
    FillBoundary( rBound, aAccessiblePositions,
                  FindBreak( aAccessiblePositions, nPos ) );
}


sal_Int32 SwAccessiblePortionData::GetAccessiblePosition( USHORT nPos )
{
    DBG_ASSERT( nPos <= sModelString.getLength(), "illegal position" );

    // find the portion number
    size_t nPortionNo = FindBreak( aModelPositions,
                                   static_cast<sal_Int32>(nPos) );

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

sal_Int32 SwAccessiblePortionData::GetLineNumber( sal_Int32 nPos )
{
    size_t nPortionNo = FindBreak( aLineBreaks, nPos );
    return nPortionNo;
}



USHORT SwAccessiblePortionData::FillSpecialPos(
    sal_Int32 nPos,
    SwSpecialPos& rPos,
    SwSpecialPos*& rpPos )
{
    size_t nPortionNo = FindLastBreak( aAccessiblePositions, nPos );

    BYTE nExtend;
    sal_Int32 nRefPos;
    sal_Int32 nModelPos;

    if( nPortionNo < nBeforePortions )
    {
        nExtend = SP_EXTEND_RANGE_BEFORE;
        nModelPos = 0;
        nRefPos = 0;
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
            (sModelString.getStr()[nModelPos] !=
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
        USHORT nLineOffset = static_cast<USHORT>( nMyLine - nRefLine );
        if( nLineOffset != 0 )
            nRefPos = aLineBreaks[ nMyLine ];

        // fill char offset and 'special position'
        rPos.nCharOfst = static_cast<USHORT>( nPos - nRefPos );
        rPos.nExtendRange = nExtend;
        rPos.nLineOfst = nLineOffset;
    }

    return static_cast<USHORT>( nModelPos );
}
