/*************************************************************************
 *
 *  $RCSfile: accportions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2002-02-19 19:11:48 $
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

// for GetWordBoundary, GetSentenceBoundary:
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



using rtl::OUString;
using com::sun::star::i18n::Boundary;

// for GetWordBoundary
using com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES;

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
    pWords( NULL ),
    pSentences( NULL )
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
    delete pWords;
    delete pSentences;
}

void SwAccessiblePortionData::Text(USHORT nLength)
{
    DBG_ASSERT( nLength >= 0, "illegal length" );
    DBG_ASSERT( (nModelPosition + nLength) <= sModelString.getLength(),
                "portion exceeds model string!" )

    DBG_ASSERT( !bFinished, "We are already done!" );

    // store 'old' positions
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    // update buffer + nModelPosition
    aBuffer.append( sModelString.copy(nModelPosition, nLength) );
    nModelPosition += nLength;
}

void SwAccessiblePortionData::Special(
    USHORT nLength, const String& rText, USHORT nType)
{
    DBG_ASSERT( nLength >= 0, "illegal length" );
    DBG_ASSERT( (nModelPosition + nLength) <= sModelString.getLength(),
                "portion exceeds model string!" )

    DBG_ASSERT( !bFinished, "We are already done!" );

    // for now, ignore the nType variable

    // store 'old' positions
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    // update buffer + nModelPosition
    aBuffer.append( OUString(rText) );
    nModelPosition += nLength;
}

void SwAccessiblePortionData::LineBreak()
{
    DBG_ASSERT( !bFinished, "We are already done!" );

    aLineBreaks.push_back( aBuffer.getLength() );
}

void SwAccessiblePortionData::Finish()
{
    DBG_ASSERT( !bFinished, "We are already done!" );

    bFinished = sal_True;
    sAccessibleString = aBuffer.makeStringAndClear();

    // include 'final' positions in positions array(s)
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( sAccessibleString.getLength() );
    aLineBreaks.push_back( sAccessibleString.getLength() );
}

const OUString& SwAccessiblePortionData::GetAccesibleString()
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

sal_Int32 SwAccessiblePortionData::GetModelPosition( sal_Int32 nPos )
{
    DBG_ASSERT( nPos >= 0, "illegal position" );
    DBG_ASSERT( nPos < sAccessibleString.getLength(), "illegal position" );

    // find the portion number
    sal_Int32 nPortionNo = FindBreak( aAccessiblePositions, nPos );

    // get model portion size
    sal_Int32 nStartPos = aModelPositions[nPortionNo];
    sal_Int32 nEndPos = aModelPositions[nPortionNo+1];

    // if the model portion has more than one position, go into it;
    // else return that position
    if( (nEndPos - nStartPos) > 1 )
    {
        // 'wide' portions have to be of the same with
        DBG_ASSERT( ( nEndPos - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accesability portion disagrees with text model" );

        sal_Int32 nWithinPortion = nPos - aAccessiblePositions[nPortionNo];
        nStartPos += nWithinPortion;
    }
    // else: return startPos unmodified

    return nStartPos;
}

void SwAccessiblePortionData::FillBoundary(
    Boundary& rBound,
    const Positions_t& rPositions,
    sal_Int32 nPos )
{
    rBound.startPos = rPositions[nPos];
    rBound.endPos = rPositions[nPos+1];
}


sal_Int32 SwAccessiblePortionData::FindBreak(
    const Positions_t& rPositions,
    sal_Int32 nValue )
{
    DBG_ASSERT( rPositions.size() >= 2, "need min + may value" );
    DBG_ASSERT( rPositions[0] == 0, "need min value" );
    DBG_ASSERT( rPositions[rPositions.size()-1] ==
                sAccessibleString.getLength(), "need max value" );

    sal_Int32 nMin = 0;

    // early out if first position is first value
    if( ! (rPositions[nMin] == nValue) )
    {
        sal_Int32 nMax = rPositions.size()-2;

        while( nMin < nMax )
        {
            // check loop invariants
            DBG_ASSERT( rPositions[nMin] < nValue, "search failed" );
            DBG_ASSERT( nValue <= rPositions[nMax+1], "search failed" );

            // get middle (and ensure progress)
            sal_Int32 nMiddle = (nMin + nMax + 1)/2;

            // check array
            DBG_ASSERT( rPositions[nMin] <= rPositions[nMiddle],
                        "garbled positions array" );
            DBG_ASSERT( rPositions[nMiddle] <= rPositions[nMax],
                        "garbled positions array" );

            sal_Int32 nMiddleValue = rPositions[nMiddle];
            if( nValue > nMiddleValue )
                nMin = nMiddle;
            else
                nMax = nMiddle - 1;
        }
    }

    return nMin;
}


void SwAccessiblePortionData::GetWordBoundary(
    Boundary& rBound,
    sal_Int32 nPos,
    const SwTxtNode* pNode )
{
     DBG_ASSERT( nPos >= 0, "illegal position; check before" );
     DBG_ASSERT( nPos < sAccessibleString.getLength(), "illegal position" );

     if( pWords == NULL )
     {
         DBG_ASSERT( pBreakIt != NULL, "We always need a break." );
         DBG_ASSERT( pBreakIt->xBreak.is(), "No break-iterator." );
         if( pBreakIt->xBreak.is() )
         {
             pWords = new Positions_t();
             pWords->reserve(100);

             // use xBreak->nextWord to iterate over all words; store
             // positions in pWords
             sal_Int32 nCurrent = 0;
             sal_Int32 nLength = sAccessibleString.getLength();
             const USHORT nWordType = ANYWORD_IGNOREWHITESPACES;
             do
             {
                 pWords->push_back( nCurrent );

                 sal_uInt32 nModelPos = static_cast<sal_uInt32>(
                     GetModelPosition( nCurrent ) );

                 nCurrent = pBreakIt->xBreak->nextWord(
                     sAccessibleString, nCurrent,
                     pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ),
                     nWordType ).startPos;

                 if( (nCurrent < 0) && (nCurrent > nLength) )
                     nCurrent = nLength;
             }
             while (nCurrent < nLength);
             pWords->push_back( nLength );
         }
         else
         {
             // no break iterator -> empty word
             rBound.startPos = 0;
             rBound.endPos = 0;
             return;
         }
     }

     FillBoundary( rBound, *pWords, FindBreak( *pWords, nPos ) );
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

                 sal_uInt32 nModelPos = static_cast<sal_uInt32>(
                     GetModelPosition( nCurrent ) );

                 nCurrent = pBreakIt->xBreak->endOfSentence(
                     sAccessibleString, nCurrent,
                     pBreakIt->GetLocale( pNode->GetLang( nModelPos ) ) ) + 1;

                 if( (nCurrent < 0) && (nCurrent > nLength) )
                     nCurrent = nLength;
             }
             while (nCurrent < nLength);
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
