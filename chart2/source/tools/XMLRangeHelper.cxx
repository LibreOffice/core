/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "XMLRangeHelper.hxx"
#include <unotools/charclass.hxx>
#include <rtl/ustrbuf.hxx>

#include <algorithm>
#include <functional>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ================================================================================

namespace
{
/** unary function that escapes backslashes and single quotes in a sal_Unicode
    array (which you can get from an OUString with getStr()) and puts the result
    into the OUStringBuffer given in the CTOR
 */
class lcl_Escape : public ::std::unary_function< sal_Unicode, void >
{
public:
    lcl_Escape( ::rtl::OUStringBuffer & aResultBuffer ) : m_aResultBuffer( aResultBuffer ) {}
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
    ::rtl::OUStringBuffer & m_aResultBuffer;
};

// ----------------------------------------

/** unary function that removes backslash escapes in a sal_Unicode array (which
    you can get from an OUString with getStr()) and puts the result into the
    OUStringBuffer given in the CTOR
 */
class lcl_UnEscape : public ::std::unary_function< sal_Unicode, void >
{
public:
    lcl_UnEscape( ::rtl::OUStringBuffer & aResultBuffer ) : m_aResultBuffer( aResultBuffer ) {}
    void operator() ( sal_Unicode aChar )
    {
        static const sal_Unicode m_aBackslash( '\\' );

        if( aChar != m_aBackslash )
            m_aResultBuffer.append( aChar );
    }

private:
    ::rtl::OUStringBuffer & m_aResultBuffer;
};

// ----------------------------------------

OUStringBuffer lcl_getXMLStringForCell( const ::chart::XMLRangeHelper::Cell & rCell )
{
    ::rtl::OUStringBuffer aBuffer;
    if( rCell.empty())
        return aBuffer;

    sal_Int32 nCol = rCell.nColumn;
    aBuffer.append( (sal_Unicode)'.' );
    if( ! rCell.bRelativeColumn )
        aBuffer.append( (sal_Unicode)'$' );

    // get A, B, C, ..., AA, AB, ... representation of column number
    if( nCol < 26 )
        aBuffer.append( (sal_Unicode)('A' + nCol) );
    else if( nCol < 702 )
    {
        aBuffer.append( (sal_Unicode)('A' + nCol / 26 - 1 ));
        aBuffer.append( (sal_Unicode)('A' + nCol % 26) );
    }
    else    // works for nCol <= 18,278
    {
        aBuffer.append( (sal_Unicode)('A' + nCol / 702 - 1 ));
        aBuffer.append( (sal_Unicode)('A' + (nCol % 702) / 26 ));
        aBuffer.append( (sal_Unicode)('A' + nCol % 26) );
    }

    // write row number as number
    if( ! rCell.bRelativeRow )
        aBuffer.append( (sal_Unicode)'$' );
    aBuffer.append( rCell.nRow + (sal_Int32)1 );

    return aBuffer;
}

void lcl_getSingleCellAddressFromXMLString(
    const ::rtl::OUString& rXMLString,
    sal_Int32 nStartPos, sal_Int32 nEndPos,
    ::chart::XMLRangeHelper::Cell & rOutCell )
{
    // expect "\$?[a-zA-Z]+\$?[1-9][0-9]*"
    static const sal_Unicode aDollar( '$' );
    static const sal_Unicode aLetterA( 'A' );

    ::rtl::OUString aCellStr = rXMLString.copy( nStartPos, nEndPos - nStartPos + 1 ).toAsciiUpperCase();
    const sal_Unicode* pStrArray = aCellStr.getStr();
    sal_Int32 nLength = aCellStr.getLength();
    sal_Int32 i = nLength - 1, nColumn = 0;

    // parse number for row
    while( CharClass::isAsciiDigit( pStrArray[ i ] ) && i >= 0 )
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
    while( CharClass::isAsciiAlpha( pStrArray[ i ] ))
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
    const ::rtl::OUString& rXMLString,
    sal_Int32 nStartPos, sal_Int32 nEndPos,
    ::chart::XMLRangeHelper::Cell & rOutCell,
    ::rtl::OUString& rOutTableName )
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

        ::rtl::OUStringBuffer aTableNameBuffer;
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
            ::rtl::OUString aName = aTableNameBuffer.makeStringAndClear();
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
    const ::rtl::OUString& rXMLString,
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

        ::rtl::OUString sTableSecondName;
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

// ================================================================================

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
            // if an error occured, bail out
            return CellRange();
        }
    }

    return aResult;
}

OUString getXMLStringFromCellRange( const CellRange & rRange )
{
    static const sal_Unicode aSpace( ' ' );
    static const sal_Unicode aQuote( '\'' );

    ::rtl::OUStringBuffer aBuffer;

    if( !rRange.aTableName.isEmpty())
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
    aBuffer.append( lcl_getXMLStringForCell( rRange.aUpperLeft ));

    if( ! rRange.aLowerRight.empty())
    {
        // we have a range (not a single cell)
        aBuffer.append( sal_Unicode( ':' ));
        aBuffer.append( lcl_getXMLStringForCell( rRange.aLowerRight ));
    }

    return aBuffer.makeStringAndClear();
}

} //  namespace XMLRangeHelper
} //  namespace chart
