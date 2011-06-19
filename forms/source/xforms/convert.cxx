/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_forms.hxx"

#include "convert.hxx"

#include "unohelper.hxx"
#include <memory>
#include <algorithm>
#include <functional>
#include <o3tl/compat_functional.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/date.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>

using xforms::Convert;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::util::Time;
using namespace std;
using namespace o3tl;

typedef com::sun::star::util::Date UNODate;
typedef com::sun::star::util::Time UNOTime;
typedef com::sun::star::util::DateTime UNODateTime;

Convert::Convert()
    : maMap()
{
    init();
}

#define ADD_ENTRY(XCONVERT,TYPE) XCONVERT->maMap[ getCppuType( static_cast<TYPE*>( NULL ) ) ] = Convert_t( &lcl_toXSD_##TYPE, &lcl_toAny_##TYPE )

namespace
{
    // ========================================================================
    struct StringToken
    {
    private:
        ::rtl::OUString m_sString;
        sal_Int32       m_nTokenStart;
        sal_Int32       m_nTokenEnd;

    public:
        StringToken() : m_sString(), m_nTokenStart( 0 ), m_nTokenEnd( 0 ) { }
        StringToken( const ::rtl::OUString& _rString, sal_Int32 _nTokenStart, sal_Int32 _nTokenEnd );
        StringToken( const StringToken& );
        StringToken& operator=( const StringToken& );

        inline  bool                isEmpty() const { return m_nTokenEnd <= m_nTokenStart; }
        inline  sal_Int32           getLength() const { return isEmpty() ? 0 : m_nTokenEnd - m_nTokenStart - 1; }
        inline  const sal_Unicode*  begin() const { return m_sString.getStr() + m_nTokenStart; }
        inline  const sal_Unicode*  end() const { return m_sString.getStr() + m_nTokenEnd; }

        bool    toInt32( sal_Int32& _rValue ) const;
    };

    // ------------------------------------------------------------------------
    StringToken::StringToken( const ::rtl::OUString& _rString, sal_Int32 _nTokenStart, sal_Int32 _nTokenEnd )
        :m_sString( _rString )
        ,m_nTokenStart( _nTokenStart )
        ,m_nTokenEnd( _nTokenEnd )
    {
        OSL_ENSURE( ( m_nTokenStart >= 0 ) && ( m_nTokenStart <= m_sString.getLength() ), "StringToken::StringToken: invalid token start!" );
        OSL_ENSURE( ( m_nTokenEnd >= 0 ) && ( m_nTokenEnd <= m_sString.getLength() ), "StringToken::StringToken: invalid token end!" );
    }

    // ------------------------------------------------------------------------
    StringToken::StringToken( const StringToken& _rRHS )
    {
        *this = _rRHS;
    }

    // ------------------------------------------------------------------------
    StringToken& StringToken::operator=( const StringToken& _rRHS )
    {
        if ( this == &_rRHS )
            return *this;

        m_sString = _rRHS.m_sString;
        m_nTokenStart = _rRHS.m_nTokenStart;
        m_nTokenEnd = _rRHS.m_nTokenEnd;

        return *this;
    }

    // ------------------------------------------------------------------------
    bool StringToken::toInt32( sal_Int32& _rValue ) const
    {
        if ( isEmpty() )
            return false;

        _rValue = 0;
        const sal_Unicode* pStr = begin();
        while ( pStr < end() )
        {
            if ( ( *pStr < '0' ) || ( *pStr > '9' ) )
                return false;

            _rValue *= 10;
            _rValue += ( *pStr - '0' );

            ++pStr;
        }

        return true;
    }

    // ========================================================================
    class StringTokenizer
    {
    private:
        ::rtl::OUString     m_sString;
        const sal_Unicode   m_nTokenSeparator;
        sal_Int32           m_nTokenStart;

    public:
        /** constructs a tokenizer
            @param _rString             the string to tokenize
            @param _nTokenSeparator     the token value. May be 0, in this case the tokenizer
                                        will recognize exactly one token, being the whole string.
                                        This may make sense if you want to apply <type>StringToken</type>
                                        methods to a whole string.
        */
        StringTokenizer( const ::rtl::OUString& _rString, sal_Unicode _nTokenSeparator = ';' );

        /// resets the tokenizer to the beginning of the string
        void    reset();

        /// determines whether there is a next token
        bool    hasNextToken() const;

        /// retrieves the next token
        StringToken
                getNextToken();
    };

    // ------------------------------------------------------------------------
    StringTokenizer::StringTokenizer( const ::rtl::OUString& _rString, sal_Unicode _nTokenSeparator )
        :m_sString( _rString )
        ,m_nTokenSeparator( _nTokenSeparator )
    {
        reset();
    }

    // ------------------------------------------------------------------------
    void StringTokenizer::reset()
    {
        m_nTokenStart = 0;
    }

    // ------------------------------------------------------------------------
    bool StringTokenizer::hasNextToken() const
    {
        return ( m_nTokenStart < m_sString.getLength() );
    }

    // ------------------------------------------------------------------------
    StringToken StringTokenizer::getNextToken()
    {
        OSL_PRECOND( hasNextToken(), "StringTokenizer::getNextToken: there is no next token!" );
        if ( !hasNextToken() )
            return StringToken();

        // determine the end of the current token
        sal_Int32 nTokenEnd = m_nTokenSeparator ? m_sString.indexOf( m_nTokenSeparator, m_nTokenStart ) : m_sString.getLength();
        bool bLastToken = !m_nTokenSeparator || ( nTokenEnd == -1 );

        // construct a new token
        StringToken aToken( m_sString, m_nTokenStart, bLastToken ? m_sString.getLength() : nTokenEnd );
        // advance
        m_nTokenStart = bLastToken ? m_sString.getLength() : nTokenEnd + 1;
        // outta here
        return aToken;
    }

    // ========================================================================
    // ------------------------------------------------------------------------
    OUString lcl_toXSD_OUString( const Any& rAny )
    { OUString sStr; rAny >>= sStr; return sStr; }

    // ------------------------------------------------------------------------
    Any lcl_toAny_OUString( const OUString& rStr )
    { Any aAny; aAny <<= rStr; return aAny; }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_bool( const Any& rAny )
    { bool b = false; rAny >>= b; return b ? OUSTRING("true") : OUSTRING("false"); }

    // ------------------------------------------------------------------------
    Any lcl_toAny_bool( const OUString& rStr )
    {
        bool b = ( rStr == OUSTRING("true")  ||  rStr == OUSTRING("1") );
        return makeAny( b );
    }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_double( const Any& rAny )
    {
        double f = 0.0;
        rAny >>= f;

        return rtl::math::isFinite( f )
            ? rtl::math::doubleToUString( f, rtl_math_StringFormat_Automatic,
                                        rtl_math_DecimalPlaces_Max, '.',
                                        sal_True )
            : OUString();
    }

    // ------------------------------------------------------------------------
    Any lcl_toAny_double( const OUString& rString )
    {
        rtl_math_ConversionStatus eStatus;
        double f = rtl::math::stringToDouble(
            rString, sal_Unicode('.'), sal_Unicode(','), &eStatus, NULL );
        return ( eStatus == rtl_math_ConversionStatus_Ok ) ? makeAny( f ) : Any();
    }

    // ------------------------------------------------------------------------
    void lcl_appendInt32ToBuffer( const sal_Int32 _nValue, ::rtl::OUStringBuffer& _rBuffer, sal_Int16 _nMinDigits )
    {
        if ( ( _nMinDigits >= 4 ) && ( _nValue < 1000 ) )
            _rBuffer.append( (sal_Unicode)'0' );
        if ( ( _nMinDigits >= 3 ) && ( _nValue < 100 ) )
            _rBuffer.append( (sal_Unicode)'0' );
        if ( ( _nMinDigits >= 2 ) && ( _nValue < 10 ) )
            _rBuffer.append( (sal_Unicode)'0' );
        _rBuffer.append( _nValue );
    }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_UNODate_typed( const UNODate& rDate )
    {

        ::rtl::OUStringBuffer sInfo;
        lcl_appendInt32ToBuffer( rDate.Year, sInfo, 4 );
        sInfo.appendAscii( "-" );
        lcl_appendInt32ToBuffer( rDate.Month, sInfo, 2 );
        sInfo.appendAscii( "-" );
        lcl_appendInt32ToBuffer( rDate.Day, sInfo, 2 );

        return sInfo.makeStringAndClear();
    }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_UNODate( const Any& rAny )
    {
        UNODate aDate;
        OSL_VERIFY( rAny >>= aDate );
        return lcl_toXSD_UNODate_typed( aDate );
    }

    // ------------------------------------------------------------------------
    UNODate lcl_toUNODate( const OUString& rString )
    {
        bool bWellformed = true;

        UNODate aDate( 1, 1, 1900 );

        sal_Int32 nToken = 0;
        StringTokenizer aTokenizer( rString, '-' );
        while ( aTokenizer.hasNextToken() )
        {
            sal_Int32 nTokenValue = 0;
            if ( !aTokenizer.getNextToken().toInt32( nTokenValue ) )
            {
                bWellformed = false;
                break;
            }

            if ( nToken == 0 )
                aDate.Year = (sal_uInt16)nTokenValue;
            else if ( nToken == 1 )
                aDate.Month = (sal_uInt16)nTokenValue;
            else if ( nToken == 2 )
                aDate.Day = (sal_uInt16)nTokenValue;
            else
            {
                bWellformed = false;
                break;
            }
            ++nToken;
        }

        // sanity checks
        if ( ( aDate.Year > 9999 ) || ( aDate.Month < 1 ) || ( aDate.Month > 12 ) || ( aDate.Day < 1 ) || ( aDate.Day > 31 ) )
            bWellformed = false;
        else
        {
            ::Date aDateCheck( 1, aDate.Month, aDate.Year );
            if ( aDate.Day > aDateCheck.GetDaysInMonth() )
                bWellformed = false;
        }

        // all okay?
        if ( !bWellformed )
            return UNODate( 1, 1, 1900 );

        return aDate;
    }

    // ------------------------------------------------------------------------
    Any lcl_toAny_UNODate( const OUString& rString )
    {
        return makeAny( lcl_toUNODate( rString ) );
    }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_UNOTime_typed( const UNOTime& rTime )
    {

        ::rtl::OUStringBuffer sInfo;
        lcl_appendInt32ToBuffer( rTime.Hours, sInfo, 2 );
        sInfo.appendAscii( ":" );
        lcl_appendInt32ToBuffer( rTime.Minutes, sInfo, 2 );
        sInfo.appendAscii( ":" );
        lcl_appendInt32ToBuffer( rTime.Seconds, sInfo, 2 );
        if ( rTime.HundredthSeconds )
        {
            sInfo.appendAscii( "." );
            lcl_appendInt32ToBuffer( rTime.HundredthSeconds, sInfo, 2 );
        }

        return sInfo.makeStringAndClear();
    }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_UNOTime( const Any& rAny )
    {
        UNOTime aTime;
        OSL_VERIFY( rAny >>= aTime );
        return lcl_toXSD_UNOTime_typed( aTime );
    }

    // ------------------------------------------------------------------------
    UNOTime lcl_toUNOTime( const OUString& rString )
    {
        bool bWellformed = true;

        UNOTime aTime( 0, 0, 0, 0 );

        ::rtl::OUString sString( rString );
        // see if there's a decimal separator for the seconds,
        // and if so, handle it separately
        sal_Int32 nDecimalSepPos = rString.indexOf( '.' );
        if ( nDecimalSepPos == -1 )
            // ISO 8601 allows for both a comma and a dot
            nDecimalSepPos = rString.indexOf( ',' );
        if ( nDecimalSepPos != -1 )
        {
            // handle fractional seconds
            ::rtl::OUString sFractional = sString.copy( nDecimalSepPos + 1 );
            if ( sFractional.getLength() > 2 )
                // our precision is HundrethSeconds - it's all a css.util.Time can hold
                sFractional = sFractional.copy( 0, 2 );
            sal_Int32 nFractional = 0;
            if ( sFractional.getLength() )
            {
                if ( StringTokenizer( sFractional, 0 ).getNextToken().toInt32( nFractional ) )
                {
                    aTime.HundredthSeconds = (sal_uInt16)nFractional;
                    if ( nFractional < 10 )
                        aTime.HundredthSeconds *= 10;
                }
                else
                    bWellformed = false;
            }

            // strip the fraction before further processing
            sString = sString.copy( 0, nDecimalSepPos );
        }

        // split into the tokens which are separated by colon
        sal_Int32 nToken = 0;
        StringTokenizer aTokenizer( sString, ':' );
        while ( aTokenizer.hasNextToken() )
        {
            sal_Int32 nTokenValue = 0;
            if ( !aTokenizer.getNextToken().toInt32( nTokenValue ) )
            {
                bWellformed = false;
                break;
            }

            if ( nToken == 0 )
                aTime.Hours = (sal_uInt16)nTokenValue;
            else if ( nToken == 1 )
                aTime.Minutes = (sal_uInt16)nTokenValue;
            else if ( nToken == 2 )
                aTime.Seconds = (sal_uInt16)nTokenValue;
            else
            {
                bWellformed = false;
                break;
            }
            ++nToken;
        }

        // sanity checks
        // note that Seconds == 60 denotes leap seconds. Normally, they're not allowed everywhere,
        // but we accept them all the time for simplicity reasons
        if  (  ( aTime.Hours > 24 )
            || ( aTime.Minutes > 59 )
            || ( aTime.Seconds > 60 )
            )
            bWellformed = false;

        if  (   bWellformed
            &&  ( aTime.Hours == 24 )
            &&  (   ( aTime.Minutes != 0 )
                ||  ( aTime.Seconds != 0 )
                ||  ( aTime.HundredthSeconds != 0 )
                )
            )
            bWellformed = false;

        // all okay?
        if ( !bWellformed )
            return UNOTime( 0, 0, 0, 0 );

        return aTime;
    }

    // ------------------------------------------------------------------------
    Any lcl_toAny_UNOTime( const OUString& rString )
    {
        return makeAny( lcl_toUNOTime( rString ) );
    }

    // ------------------------------------------------------------------------
    OUString lcl_toXSD_UNODateTime( const Any& rAny )
    {
        UNODateTime aDateTime;
        OSL_VERIFY( rAny >>= aDateTime );

        UNODate aDate( aDateTime.Day, aDateTime.Month, aDateTime.Year );
        ::rtl::OUString sDate = lcl_toXSD_UNODate_typed( aDate );

        UNOTime aTime( aDateTime.HundredthSeconds, aDateTime.Seconds, aDateTime.Minutes, aDateTime.Hours );
        ::rtl::OUString sTime = lcl_toXSD_UNOTime_typed( aTime );

        ::rtl::OUStringBuffer sInfo;
        sInfo.append( sDate );
        sInfo.append( (sal_Unicode) 'T' );
        sInfo.append( sTime );
        return sInfo.makeStringAndClear();
    }

    // ------------------------------------------------------------------------
    Any lcl_toAny_UNODateTime( const OUString& rString )
    {
        // separate the date from the time part
        sal_Int32 nDateTimeSep = rString.indexOf( 'T' );
        if ( nDateTimeSep == -1 )
            nDateTimeSep = rString.indexOf( 't' );

        UNODate aDate;
        UNOTime aTime;
        if ( nDateTimeSep == -1 )
        {   // no time part
            aDate = lcl_toUNODate( rString );
            aTime = UNOTime( 0, 0, 0, 0 );
        }
        else
        {
            aDate = lcl_toUNODate( rString.copy( 0, nDateTimeSep ) );
            aTime = lcl_toUNOTime( rString.copy( nDateTimeSep + 1 ) );
        }
        UNODateTime aDateTime(
            aTime.HundredthSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
            aDate.Day, aDate.Month, aDate.Year
        );
        return makeAny( aDateTime );
    }
}

// ============================================================================
void Convert::init()
{
    ADD_ENTRY( this, OUString );
    ADD_ENTRY( this, bool );
    ADD_ENTRY( this, double );
    ADD_ENTRY( this, UNODate );
    ADD_ENTRY( this, UNOTime );
    ADD_ENTRY( this, UNODateTime );
}


Convert& Convert::get()
{
    // create our Singleton instance on demand
    static Convert* pConvert = NULL;
    if( pConvert == NULL )
        pConvert = new Convert();

    OSL_ENSURE( pConvert != NULL, "no converter?" );
    return *pConvert;
}

bool Convert::hasType( const Type_t& rType )
{
    return maMap.find( rType ) != maMap.end();
}

Convert::Types_t Convert::getTypes()
{
    Types_t aTypes( maMap.size() );
    transform( maMap.begin(), maMap.end(), aTypes.getArray(),
               o3tl::select1st<Map_t::value_type>() );
    return aTypes;
}

rtl::OUString Convert::toXSD( const Any_t& rAny )
{
    Map_t::iterator aIter = maMap.find( rAny.getValueType() );
    return aIter != maMap.end() ? aIter->second.first( rAny ) : OUString();
}

Convert::Any_t Convert::toAny( const rtl::OUString& rValue,
                               const Type_t& rType )
{
    Map_t::iterator aIter = maMap.find( rType );
    return aIter != maMap.end() ? aIter->second.second( rValue ) : Any_t();
}

//------------------------------------------------------------------------
::rtl::OUString Convert::convertWhitespace( const ::rtl::OUString& _rString, sal_Int16 _nWhitespaceTreatment )
{
    ::rtl::OUString sConverted;
    switch( _nWhitespaceTreatment )
    {
    default:
        OSL_FAIL( "Convert::convertWhitespace: invalid whitespace treatment constant!" );
        // NO break
    case com::sun::star::xsd::WhiteSpaceTreatment::Preserve:
        sConverted = _rString;
        break;
    case com::sun::star::xsd::WhiteSpaceTreatment::Replace:
        sConverted = replaceWhitespace( _rString );
        break;
    case com::sun::star::xsd::WhiteSpaceTreatment::Collapse:
        sConverted = collapseWhitespace( _rString );
        break;
    }
    return sConverted;
}

//------------------------------------------------------------------------
::rtl::OUString Convert::replaceWhitespace( const ::rtl::OUString& _rString )
{
    OUStringBuffer aBuffer( _rString );
    sal_Int32 nLength = aBuffer.getLength();
    const sal_Unicode* pBuffer = aBuffer.getStr();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        sal_Unicode c = pBuffer[i];
        if( c == sal_Unicode(0x08) ||
            c == sal_Unicode(0x0A) ||
            c == sal_Unicode(0x0D) )
            aBuffer.setCharAt( i, sal_Unicode(0x20) );
    }
    return aBuffer.makeStringAndClear();
}

//------------------------------------------------------------------------
::rtl::OUString Convert::collapseWhitespace( const ::rtl::OUString& _rString )
{
    sal_Int32 nLength = _rString.getLength();
    OUStringBuffer aBuffer( nLength );
    const sal_Unicode* pStr = _rString.getStr();
    bool bStrip = true;
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        sal_Unicode c = pStr[i];
        if( c == sal_Unicode(0x08) ||
            c == sal_Unicode(0x0A) ||
            c == sal_Unicode(0x0D) ||
            c == sal_Unicode(0x20) )
        {
            if( ! bStrip )
            {
                aBuffer.append( sal_Unicode(0x20) );
                bStrip = true;
            }
        }
        else
        {
            bStrip = false;
            aBuffer.append( c );
        }
    }
    if( aBuffer[ aBuffer.getLength() - 1 ] == sal_Unicode( 0x20 ) )
        aBuffer.setLength( aBuffer.getLength() - 1 );
    return aBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
