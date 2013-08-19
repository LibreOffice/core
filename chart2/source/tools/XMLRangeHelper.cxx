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

#include "XMLRangeHelper.hxx"
#include <unotools/charclass.hxx>
#include <rtl/ustrbuf.hxx>

#include <algorithm>
#include <functional>

namespace
{
/** unary function that escapes backslashes and single quotes in a sal_Unicode
    array (which you can get from an OUString with getStr()) and puts the result
    into the OUStringBuffer given in the CTOR
 */
class lcl_Escape : public ::std::unary_function< sal_Unicode, void >
{
public:
    lcl_Escape( OUStringBuffer & aResultBuffer ) : m_aResultBuffer( aResultBuffer ) {}
    void operator() ( sal_Unicode aChar )
    {
        static const sal_Unicode m_aQuote( '\'' );
        static const sal_Unicode m_aBackslash( '\\' );

        if( aChar == m_aQuote ||
            aChar == m_aBackslash )
            m_aResultBuffer.append( m_aBackslash );
        m_aResultBuffer.append( aChar );
    }

private:
    OUStringBuffer & m_aResultBuffer;
};

/** unary function that removes backslash escapes in a sal_Unicode array (which
    you can get from an OUString with getStr()) and puts the result into the
    OUStringBuffer given in the CTOR
 */
class lcl_UnEscape : public ::std::unary_function< sal_Unicode, void >
{
public:
    lcl_UnEscape( OUStringBuffer & aResultBuffer ) : m_aResultBuffer( aResultBuffer ) {}
    void operator() ( sal_Unicode aChar )
    {
        static const sal_Unicode m_aBackslash( '\\' );

        if( aChar != m_aBackslash )
            m_aResultBuffer.append( aChar );
    }

private:
    OUStringBuffer & m_aResultBuffer;
};

void lcl_getXMLStringForCell( const ::chart::XMLRangeHelper::Cell & rCell, OUStringBuffer * output )
{
    OSL_ASSERT(output != 0);

    if( rCell.empty())
        return;

    sal_Int32 nCol = rCell.nColumn;
    output->append( (sal_Unicode)'.' );
    if( ! rCell.bRelativeColumn )
        output->append( (sal_Unicode)'$' );

    // get A, B, C, ..., AA, AB, ... representation of column number
    if( nCol < 26 )
        output->append( (sal_Unicode)('A' + nCol) );
    else if( nCol < 702 )
    {
        output->append( (sal_Unicode)('A' + nCol / 26 - 1 ));
        output->append( (sal_Unicode)('A' + nCol % 26) );
    }
    else    // works for nCol <= 18,278
    {
        output->append( (sal_Unicode)('A' + nCol / 702 - 1 ));
        output->append( (sal_Unicode)('A' + (nCol % 702) / 26 ));
        output->append( (sal_Unicode)('A' + nCol % 26) );
    }

    // write row number as number
    if( ! rCell.bRelativeRow )
        output->append( (sal_Unicode)'$' );
    output->append( rCell.nRow + (sal_Int32)1 );
}

void lcl_getSingleCellAddressFromXMLString(
    const OUString& rXMLString,
    sal_Int32 nStartPos, sal_Int32 nEndPos,
    ::chart::XMLRangeHelper::Cell & rOutCell )
{
    // expect "\$?[a-zA-Z]+\$?[1-9][0-9]*"
    static const sal_Unicode aDollar( '$' );
    static const sal_Unicode aLetterA( 'A' );

    OUString aCellStr = rXMLString.copy( nStartPos, nEndPos - nStartPos + 1 ).toAsciiUpperCase();
    const sal_Unicode* pStrArray = aCellStr.getStr();
    sal_Int32 nLength = aCellStr.getLength();
    sal_Int32 i = nLength - 1, nColumn = 0;

    // parse number for row
    while( rtl::isAsciiDigit( pStrArray[ i ] ) && i >= 0 )
        i--;
    rOutCell.nRow = (aCellStr.copy( i + 1 )).toInt32() - 1;
    // a dollar in XML means absolute (whereas in UI it means relative)
    if( pStrArray[ i ] == aDollar )
    {
        i--;
        rOutCell.bRelativeRow = false;
    }
    else
        rOutCell.bRelativeRow = true;

    // parse rest for column
    sal_Int32 nPower = 1;
    while( rtl::isAsciiAlpha( pStrArray[ i ] ))
    {
        nColumn += (pStrArray[ i ] - aLetterA + 1) * nPower;
        i--;
        nPower *= 26;
    }
    rOutCell.nColumn = nColumn - 1;

    rOutCell.bRelativeColumn = true;
    if( i >= 0 &&
        pStrArray[ i ] == aDollar )
        rOutCell.bRelativeColumn = false;
    rOutCell.bIsEmpty = false;
}

bool lcl_getCellAddressFromXMLString(
    const OUString& rXMLString,
    sal_Int32 nStartPos, sal_Int32 nEndPos,
    ::chart::XMLRangeHelper::Cell & rOutCell,
    OUString& rOutTableName )
{
    static const sal_Unicode aDot( '.' );
    static const sal_Unicode aQuote( '\'' );
    static const sal_Unicode aBackslash( '\\' );

    sal_Int32 nNextDelimiterPos = nStartPos;

    sal_Int32 nDelimiterPos = nStartPos;
    bool bInQuotation = false;
    // parse table name
    while( nDelimiterPos < nEndPos &&
           ( bInQuotation || rXMLString[ nDelimiterPos ] != aDot ))
    {
        // skip escaped characters (with backslash)
        if( rXMLString[ nDelimiterPos ] == aBackslash )
            ++nDelimiterPos;
        // toggle quotation mode when finding single quotes
        else if( rXMLString[ nDelimiterPos ] == aQuote )
            bInQuotation = ! bInQuotation;

        ++nDelimiterPos;
    }

    if( nDelimiterPos == -1 )
        return false;

    if( nDelimiterPos > nStartPos && nDelimiterPos < nEndPos )
    {
        // there is a table name before the address

        OUStringBuffer aTableNameBuffer;
        const sal_Unicode * pTableName = rXMLString.getStr();

        // remove escapes from table name
        ::std::for_each( pTableName + nStartPos,
                         pTableName + nDelimiterPos,
                         lcl_UnEscape( aTableNameBuffer ));

        // unquote quoted table name
        const sal_Unicode * pBuf = aTableNameBuffer.getStr();
        if( pBuf[ 0 ] == aQuote &&
            pBuf[ aTableNameBuffer.getLength() - 1 ] == aQuote )
        {
            OUString aName = aTableNameBuffer.makeStringAndClear();
            rOutTableName = aName.copy( 1, aName.getLength() - 2 );
        }
        else
            rOutTableName = aTableNameBuffer.makeStringAndClear();
    }
    else
        nDelimiterPos = nStartPos;

    for( sal_Int32 i = 0;
         nNextDelimiterPos < nEndPos;
         nDelimiterPos = nNextDelimiterPos, i++ )
    {
        nNextDelimiterPos = rXMLString.indexOf( aDot, nDelimiterPos + 1 );
        if( nNextDelimiterPos == -1 ||
            nNextDelimiterPos > nEndPos )
            nNextDelimiterPos = nEndPos + 1;

        if( i==0 )
            // only take first cell
            lcl_getSingleCellAddressFromXMLString(
                rXMLString, nDelimiterPos + 1, nNextDelimiterPos - 1, rOutCell );
    }

    return true;
}

bool lcl_getCellRangeAddressFromXMLString(
    const OUString& rXMLString,
    sal_Int32 nStartPos, sal_Int32 nEndPos,
    ::chart::XMLRangeHelper::CellRange & rOutRange )
{
    bool bResult = true;
    static const sal_Unicode aColon( ':' );
    static const sal_Unicode aQuote( '\'' );
    static const sal_Unicode aBackslash( '\\' );

    sal_Int32 nDelimiterPos = nStartPos;
    bool bInQuotation = false;
    // parse table name
    while( nDelimiterPos < nEndPos &&
           ( bInQuotation || rXMLString[ nDelimiterPos ] != aColon ))
    {
        // skip escaped characters (with backslash)
        if( rXMLString[ nDelimiterPos ] == aBackslash )
            ++nDelimiterPos;
        // toggle quotation mode when finding single quotes
        else if( rXMLString[ nDelimiterPos ] == aQuote )
            bInQuotation = ! bInQuotation;

        ++nDelimiterPos;
    }

    if( nDelimiterPos == nEndPos )
    {
        // only one cell
        bResult = lcl_getCellAddressFromXMLString( rXMLString, nStartPos, nEndPos,
                                                   rOutRange.aUpperLeft,
                                                   rOutRange.aTableName );
        if( rOutRange.aTableName.isEmpty() )
            bResult = false;
    }
    else
    {
        // range (separated by a colon)
        bResult = lcl_getCellAddressFromXMLString( rXMLString, nStartPos, nDelimiterPos - 1,
                                                   rOutRange.aUpperLeft,
                                                   rOutRange.aTableName );
        if( rOutRange.aTableName.isEmpty() )
            bResult = false;

        OUString sTableSecondName;
        if( bResult )
        {
            bResult = lcl_getCellAddressFromXMLString( rXMLString, nDelimiterPos + 1, nEndPos,
                                                       rOutRange.aLowerRight,
                                                       sTableSecondName );
        }
        if( bResult &&
            !sTableSecondName.isEmpty() &&
            ! sTableSecondName.equals( rOutRange.aTableName ))
            bResult = false;
    }

    return bResult;
}

} // anonymous namespace

namespace chart
{
namespace XMLRangeHelper
{

CellRange getCellRangeFromXMLString( const OUString & rXMLString )
{
    static const sal_Unicode aSpace( ' ' );
    static const sal_Unicode aQuote( '\'' );
//     static const sal_Unicode aDoubleQuote( '\"' );
    static const sal_Unicode aDollar( '$' );
    static const sal_Unicode aBackslash( '\\' );

    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPos = nStartPos;
    const sal_Int32 nLength = rXMLString.getLength();

    // reset
    CellRange aResult;

    // iterate over different ranges
    for( sal_Int32 i = 0;
         nEndPos < nLength;
         nStartPos = ++nEndPos, i++ )
    {
        // find start point of next range

        // ignore leading '$'
        if( rXMLString[ nEndPos ] == aDollar)
            nEndPos++;

        bool bInQuotation = false;
        // parse range
        while( nEndPos < nLength &&
               ( bInQuotation || rXMLString[ nEndPos ] != aSpace ))
        {
            // skip escaped characters (with backslash)
            if( rXMLString[ nEndPos ] == aBackslash )
                ++nEndPos;
            // toggle quotation mode when finding single quotes
            else if( rXMLString[ nEndPos ] == aQuote )
                bInQuotation = ! bInQuotation;

            ++nEndPos;
        }

        if( ! lcl_getCellRangeAddressFromXMLString(
                rXMLString,
                nStartPos, nEndPos - 1,
                aResult ))
        {
            // if an error occurred, bail out
            return CellRange();
        }
    }

    return aResult;
}

OUString getXMLStringFromCellRange( const CellRange & rRange )
{
    static const sal_Unicode aSpace( ' ' );
    static const sal_Unicode aQuote( '\'' );

    OUStringBuffer aBuffer;

    if( !(rRange.aTableName).isEmpty())
    {
        bool bNeedsEscaping = ( rRange.aTableName.indexOf( aQuote ) > -1 );
        bool bNeedsQuoting = bNeedsEscaping || ( rRange.aTableName.indexOf( aSpace ) > -1 );

        // quote table name if it contains spaces or quotes
        if( bNeedsQuoting )
        {
            // leading quote
            aBuffer.append( aQuote );

            // escape existing quotes
            if( bNeedsEscaping )
            {
                const sal_Unicode * pTableNameBeg = rRange.aTableName.getStr();

                // append the quoted string at the buffer
                ::std::for_each( pTableNameBeg,
                                 pTableNameBeg + rRange.aTableName.getLength(),
                                 lcl_Escape( aBuffer ) );
            }
            else
                aBuffer.append( rRange.aTableName );

            // final quote
            aBuffer.append( aQuote );
        }
        else
            aBuffer.append( rRange.aTableName );
    }
    lcl_getXMLStringForCell( rRange.aUpperLeft, &aBuffer );

    if( ! rRange.aLowerRight.empty())
    {
        // we have a range (not a single cell)
        aBuffer.append( sal_Unicode( ':' ));
        lcl_getXMLStringForCell( rRange.aLowerRight, &aBuffer );
    }

    return aBuffer.makeStringAndClear();
}

} //  namespace XMLRangeHelper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
