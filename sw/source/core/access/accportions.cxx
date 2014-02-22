/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "accportions.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/Boundary.hpp>
#include <txttypes.hxx>


#include "access.hrc"
#include <tools/resid.hxx>
#include "viewopt.hxx"


#include <breakit.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <ndtxt.hxx>


#include "crstate.hxx"


#include "acccontext.hxx"


#include "txatbase.hxx"
#include "fmtfld.hxx"
#include "fldbas.hxx"
#include "docufld.hxx"


#include "ndindex.hxx"
#include "ndnotxt.hxx"
#include "fmtflcnt.hxx"
#include "frmfmt.hxx"
#include "fmtcntnt.hxx"

using namespace ::com::sun::star;

using i18n::Boundary;


#define POR_TERMINATE 0


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
    OSL_ENSURE( pTxtNode != NULL, "Text node is needed!" );

    
    aLineBreaks.reserve( 5 );
    aModelPositions.reserve( 10 );
    aAccessiblePositions.reserve( 10 );

    
    aLineBreaks.push_back( 0 );
}

SwAccessiblePortionData::~SwAccessiblePortionData()
{
    delete pSentences;
}

void SwAccessiblePortionData::Text(sal_uInt16 nLength, sal_uInt16 nType, sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/)
{
    OSL_ENSURE( (nModelPosition + nLength) <= pTxtNode->GetTxt().getLength(),
                "portion exceeds model string!" );

    OSL_ENSURE( !bFinished, "We are already done!" );

    
    if( nLength == 0 )
        return;

    
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    
    sal_uInt8 nAttr = IsGrayPortionType(nType) ? PORATTR_GRAY : 0;
    aPortionAttrs.push_back( nAttr );

    
    aBuffer.append( OUString(
        pTxtNode->GetTxt().copy(nModelPosition, nLength)) );
    nModelPosition += nLength;

    bLastIsSpecial = sal_False;
}

void SwAccessiblePortionData::SetAttrFieldType( sal_uInt16 nAttrFldType )
{
    aAttrFieldType.push_back(nAttrFldType);
    return;
}

void SwAccessiblePortionData::Special(
    sal_uInt16 nLength, const OUString& rText, sal_uInt16 nType, sal_Int32 /*nHeight*/, sal_Int32 /*nWidth*/)
{
    OSL_ENSURE( nModelPosition >= 0, "illegal position" );
    OSL_ENSURE( (nModelPosition + nLength) <= pTxtNode->GetTxt().getLength(),
                "portion exceeds model string!" );

    OSL_ENSURE( !bFinished, "We are already done!" );

    
    
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
        case POR_BULLET:
                break;
        case POR_FLD:
        case POR_HIDDEN:
        case POR_COMBINED:
        case POR_ISOREF:
            {
                
                if (rText.isEmpty())
                    sDisplay = OUString(sal_Unicode(0xfffc));
                else
                    sDisplay = rText;
                aFieldPosition.push_back(aBuffer.getLength());
                aFieldPosition.push_back(aBuffer.getLength() + rText.getLength());
                break;
            }
        case POR_FTNNUM:
            {
                break;
            }
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
        {
            sDisplay = OUString( rText ) + " ";
            break;
        }
        
        
        case POR_CONTROLCHAR:
        {
            sDisplay = OUString( rText ) + OUString( pTxtNode->GetTxt()[nModelPosition] );
            break;
        }
        default:
            sDisplay = OUString( rText );
            break;
    }

    
    if( (nLength == 0) && (sDisplay.getLength() == 0) && (nType != POR_TERMINATE) )
        return;

    
    
    if( ( nLength == 0 ) && ( nModelPosition == 0 ) )
        nBeforePortions++;

    
    aModelPositions.push_back( nModelPosition );
    aAccessiblePositions.push_back( aBuffer.getLength() );

    
    sal_uInt8 nAttr = PORATTR_SPECIAL;
    if( IsGrayPortionType(nType) )      nAttr |= PORATTR_GRAY;
    if( nLength == 0 )                  nAttr |= PORATTR_READONLY;
    if( nType == POR_TERMINATE )        nAttr |= PORATTR_TERM;
    aPortionAttrs.push_back( nAttr );

    
    aBuffer.append( sDisplay );
    nModelPosition += nLength;

    
    
    if( nType != POR_TERMINATE )
        bLastIsSpecial = sal_True;
}

void SwAccessiblePortionData::LineBreak(KSHORT /*nWidth*/)
{
    OSL_ENSURE( !bFinished, "We are already done!" );

    aLineBreaks.push_back( aBuffer.getLength() );
}

void SwAccessiblePortionData::Skip(sal_uInt16 nLength)
{
    OSL_ENSURE( !bFinished, "We are already done!" );
    OSL_ENSURE( aModelPositions.empty(), "Never Skip() after portions" );
    OSL_ENSURE( nLength <= pTxtNode->GetTxt().getLength(),
            "skip exceeds model string!" );

    nModelPosition += nLength;
}

void SwAccessiblePortionData::Finish()
{
    OSL_ENSURE( !bFinished, "We are already done!" );

    
    
    
    Special( 0, OUString(), POR_TERMINATE );
    Special( 0, OUString(), POR_TERMINATE );
    LineBreak(0);
    LineBreak(0);

    sAccessibleString = aBuffer.makeStringAndClear();
    bFinished = sal_True;
}

sal_Bool SwAccessiblePortionData::IsPortionAttrSet(
    size_t nPortionNo, sal_uInt8 nAttr ) const
{
    OSL_ENSURE( nPortionNo < aPortionAttrs.size(),
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
            break; 
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


sal_Int32 SwAccessiblePortionData::GetLineCount() const
{
    size_t nBreaks = aLineBreaks.size();
    
    
    
    
    sal_Int32 nLineCount = ( nBreaks > 3 )
                           ? nBreaks - 3
                           : ( ( nBreaks == 3 ) ? 1 : 0 );
    return nLineCount;
}

sal_Int32 SwAccessiblePortionData::GetLineNo( const sal_Int32 nPos ) const
{
    sal_Int32 nLineNo = FindBreak( aLineBreaks, nPos );

    
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

    
    
    size_t nBreaks = aLineBreaks.size();
    FillBoundary( rBound, aLineBreaks, nBreaks <= 3 ? 0 : nBreaks-4 );
}

sal_Int32 SwAccessiblePortionData::GetModelPosition( sal_Int32 nPos ) const
{
    OSL_ENSURE( nPos >= 0, "illegal position" );
    OSL_ENSURE( nPos <= sAccessibleString.getLength(), "illegal position" );

    
    size_t nPortionNo = FindBreak( aAccessiblePositions, nPos );

    
    sal_Int32 nStartPos = aModelPositions[nPortionNo];

    
    
    if( ! IsSpecialPortion( nPortionNo ) )
    {
        
        OSL_ENSURE( ( aModelPositions[nPortionNo+1] - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accesability portion disagrees with text model" );

        nStartPos += nPos - aAccessiblePositions[nPortionNo];
    }
    

    OSL_ENSURE( nStartPos >= 0, "There's something weird in number of characters of SwTxtNode" );
    return nStartPos;
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
    OSL_ENSURE( rPositions.size() >= 2, "need min + max value" );
    OSL_ENSURE( rPositions[0] <= nValue, "need min value" );
    OSL_ENSURE( rPositions[rPositions.size()-1] >= nValue,
                "need first terminator value" );
    OSL_ENSURE( rPositions[rPositions.size()-2] >= nValue,
                "need second terminator value" );

    size_t nMin = 0;
    size_t nMax = rPositions.size()-2;

    
    while( nMin+1 < nMax )
    {
        
        OSL_ENSURE( ( (nMin == 0) && (rPositions[nMin] <= nValue) ) ||
                    ( (nMin != 0) && (rPositions[nMin] < nValue) ),
                    "minvalue not minimal" );
        OSL_ENSURE( nValue <= rPositions[nMax], "max value not maximal" );

        
        size_t nMiddle = (nMin + nMax)/2;
        OSL_ENSURE( nMin < nMiddle, "progress?" );
        OSL_ENSURE( nMiddle < nMax, "progress?" );

        
        OSL_ENSURE( rPositions[nMin] <= rPositions[nMiddle],
                    "garbled positions array" );
        OSL_ENSURE( rPositions[nMiddle] <= rPositions[nMax],
                    "garbled positions array" );

        if( nValue > rPositions[nMiddle] )
            nMin = nMiddle;
        else
            nMax = nMiddle;
    }

    
    OSL_ENSURE( (nMax == nMin) || (nMax == nMin+1), "only two left" );
    if( (rPositions[nMin] < nValue) && (rPositions[nMin+1] <= nValue) )
        nMin = nMin+1;

    
    OSL_ENSURE( rPositions[nMin] <= nValue, "not smaller or equal" );
    OSL_ENSURE( nValue <= rPositions[nMin+1], "not equal or larger" );
    OSL_ENSURE( (nMin == 0) || (rPositions[nMin-1] <= nValue),
                "earlier value should have been returned" );

    OSL_ENSURE( nMin < rPositions.size()-1,
                "shouldn't return last position (due to termintator values)" );

    return nMin;
}

size_t SwAccessiblePortionData::FindLastBreak(
    const Positions_t& rPositions,
    sal_Int32 nValue ) const
{
    size_t nResult = FindBreak( rPositions, nValue );

    
    
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

             
             
             sal_Int32 nCurrent = 0;
             sal_Int32 nLength = sAccessibleString.getLength();
             do
             {
                 pSentences->push_back( nCurrent );

                 sal_uInt16 nModelPos = GetModelPosition( nCurrent );

                 sal_Int32 nNew = g_pBreakIt->GetBreakIter()->endOfSentence(
                     sAccessibleString, nCurrent,
                     g_pBreakIt->GetLocale(pTxtNode->GetLang(nModelPos)) ) + 1;

                 if( (nNew < 0) && (nNew > nLength) )
                     nNew = nLength;
                 else if (nNew <= nCurrent)
                     nNew = nCurrent + 1;   

                 nCurrent = nNew;
             }
             while (nCurrent < nLength);

             
             pSentences->push_back( nLength );
             pSentences->push_back( nLength );
         }
         else
         {
             
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
    OSL_ENSURE( pTxtNode != NULL, "Need SwTxtNode!" );

    
    FillBoundary( rBound, aAccessiblePositions,
                  FindBreak( aAccessiblePositions, nPos ) );
}

sal_Int32 SwAccessiblePortionData::GetAccessiblePosition( sal_Int32 nPos ) const
{
    OSL_ENSURE( nPos <= pTxtNode->GetTxt().getLength(), "illegal position" );

    
    
    size_t nPortionNo = FindLastBreak( aModelPositions, nPos );

    sal_Int32 nRet = aAccessiblePositions[nPortionNo];

    
    
    sal_Int32 nStartPos = aModelPositions[nPortionNo];
    sal_Int32 nEndPos = aModelPositions[nPortionNo+1];
    if( (nEndPos - nStartPos) > 1 )
    {
        
        OSL_ENSURE( ( nEndPos - nStartPos ) ==
                    ( aAccessiblePositions[nPortionNo+1] -
                      aAccessiblePositions[nPortionNo] ),
                    "accesability portion disagrees with text model" );

        sal_Int32 nWithinPortion = nPos - aModelPositions[nPortionNo];
        nRet += nWithinPortion;
    }
    

    OSL_ENSURE( (nRet >= 0) && (nRet <= sAccessibleString.getLength()),
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

        
        if( (nModelEndPos - nModelPos == 1) &&
            (pTxtNode->GetTxt()[nModelPos] != sAccessibleString[nPos]))
        {
            
            
            
            nRefPos = aAccessiblePositions[ nCorePortionNo ];
            nExtend = SP_EXTEND_RANGE_NONE;
            rpPos = &rPos;
        }
        else if(nPortionNo != nCorePortionNo)
        {
            
            
            
            
            nRefPos = aAccessiblePositions[ nCorePortionNo+1 ];
            nExtend = SP_EXTEND_RANGE_BEHIND;
            rpPos = &rPos;
        }
        else
        {
            
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
        OSL_ENSURE( (nExtend == SP_EXTEND_RANGE_NONE) ||
                    (nExtend == SP_EXTEND_RANGE_BEFORE) ||
                    (nExtend == SP_EXTEND_RANGE_BEHIND), "need extend" );

        
        
        size_t nRefLine = FindBreak( aLineBreaks, nRefPos );
        size_t nMyLine  = FindBreak( aLineBreaks, nPos );
        sal_uInt16 nLineOffset = static_cast<sal_uInt16>( nMyLine - nRefLine );
        if( nLineOffset != 0 )
            nRefPos = aLineBreaks[ nMyLine ];

        
        rPos.nCharOfst = nPos - nRefPos;
        rPos.nExtendRange = nExtend;
        rPos.nLineOfst = nLineOffset;
    }

    return static_cast<sal_uInt16>( nModelPos );
}

sal_uInt16 SwAccessiblePortionData::GetAttrFldType( sal_Int32 nPos )
{
    if( aFieldPosition.size() < 2 ) return sal_False;
    sal_Int32 nFieldIndex = 0;
    for( size_t i = 0; i < aFieldPosition.size() - 1; i += 2 )
    {
        if( nPos < aFieldPosition[ i + 1 ]  &&  nPos >= aFieldPosition[ i ] )
        {
            return aAttrFieldType[nFieldIndex];
        }
        nFieldIndex++ ;
    }
    return 0;
}

sal_Bool SwAccessiblePortionData::FillBoundaryIFDateField( com::sun::star::i18n::Boundary& rBound, const sal_Int32 nPos )
{
    if( aFieldPosition.size() < 2 ) return sal_False;
    for( size_t i = 0; i < aFieldPosition.size() - 1; i += 2 )
    {
        if( nPos < aFieldPosition[ i + 1 ]  &&  nPos >= aFieldPosition[ i ] )
        {
            rBound.startPos = aFieldPosition[i];
            rBound.endPos =  aFieldPosition[i + 1];
            return sal_True;
        }
    }
    return sal_False;
}

void SwAccessiblePortionData::AdjustAndCheck(
    sal_Int32 nPos,
    size_t& nPortionNo,
    sal_Int32& nCorePos,
    bool& bEdit) const
{
    
    nPortionNo = FindBreak( aAccessiblePositions, nPos );
    nCorePos = aModelPositions[ nPortionNo ];

    
    
    if( IsSpecialPortion( nPortionNo ) )
        bEdit &= nPos == aAccessiblePositions[nPortionNo];
    else
        nCorePos += nPos - aAccessiblePositions[nPortionNo];
}

sal_Bool SwAccessiblePortionData::GetEditableRange(
    sal_Int32 nStart, sal_Int32 nEnd,
    sal_Int32& nCoreStart, sal_Int32& nCoreEnd ) const
{
    bool bIsEditable = true;

    
    size_t nStartPortion, nEndPortion;
    AdjustAndCheck( nStart, nStartPortion, nCoreStart, bIsEditable );
    AdjustAndCheck( nEnd,   nEndPortion,   nCoreEnd,   bIsEditable );

    
    
    size_t nLastPortion = nEndPortion;

    
    if( IsSpecialPortion(nLastPortion) )
    {
        if (nLastPortion > 0)
            nLastPortion--;
        else
            
            
            
            
            
            nStartPortion = nLastPortion + 1;
    }

    for( size_t nPor = nStartPortion; nPor <= nLastPortion; nPor ++ )
    {
        bIsEditable &= ! IsReadOnlyPortion( nPor );
    }

    return bIsEditable;
}

sal_Bool SwAccessiblePortionData::IsValidCorePosition( sal_Int32 nPos ) const
{
    
    return ( aModelPositions[0] <= nPos ) &&
           ( nPos <= aModelPositions[ aModelPositions.size()-1 ] );
}

sal_Bool SwAccessiblePortionData::IsZeroCorePositionData()
{
    if( aModelPositions.size() < 1  ) return sal_True;
    return aModelPositions[0] == 0 &&  aModelPositions[aModelPositions.size()-1] == 0;
}

sal_Bool SwAccessiblePortionData::IsIndexInFootnode(sal_Int32 nIndex)
{
    VEC_PAIR_POS::iterator vi =m_vecPairPos.begin();
    for (;vi != m_vecPairPos.end() ; ++vi)
    {
        const PAIR_POS &pairPos = *vi;
        if(nIndex >= pairPos.first && nIndex < pairPos.second )
        {
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool SwAccessiblePortionData::IsInGrayPortion( sal_Int32 nPos )
{

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
