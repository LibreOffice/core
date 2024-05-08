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


#include "convert.hxx"

#include <sstream>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <tools/date.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>
#include <comphelper/sequence.hxx>
#include <unotools/datetime.hxx>

using xforms::Convert;
using com::sun::star::uno::Any;
using namespace utl;

Convert::Convert()
{
    init();
}

namespace
{

    OUString lcl_toXSD_OUString( const Any& rAny )
    { OUString sStr; rAny >>= sStr; return sStr; }


    Any lcl_toAny_OUString( const OUString& rStr )
    { return Any(rStr); }

    OUString lcl_toXSD_bool( const Any& rAny )
    { bool b = false; rAny >>= b; return b ? u"true"_ustr : u"false"_ustr; }

    Any lcl_toAny_bool( const OUString& rStr )
    {
        bool b = ( rStr == "true"  ||  rStr == "1" );
        return Any( b );
    }

    OUString lcl_toXSD_double( const Any& rAny )
    {
        double f = 0.0;
        rAny >>= f;

        return std::isfinite( f )
            ? rtl::math::doubleToUString( f, rtl_math_StringFormat_Automatic,
                                        rtl_math_DecimalPlaces_Max, '.',
                                        true )
            : OUString();
    }


    Any lcl_toAny_double( const OUString& rString )
    {
        rtl_math_ConversionStatus eStatus;
        double f = rtl::math::stringToDouble(
            rString.replace(',','.'), '.', ',', &eStatus );
        return ( eStatus == rtl_math_ConversionStatus_Ok ) ? Any( f ) : Any();
    }

    void lcl_appendInt32ToBuffer( const sal_Int32 _nValue, OUStringBuffer& _rBuffer, sal_Int16 _nMinDigits )
    {
        if ( ( _nMinDigits >= 4 ) && ( _nValue < 1000 ) )
            _rBuffer.append( '0' );
        if ( ( _nMinDigits >= 3 ) && ( _nValue < 100 ) )
            _rBuffer.append( '0' );
        if ( ( _nMinDigits >= 2 ) && ( _nValue < 10 ) )
            _rBuffer.append( '0' );
        _rBuffer.append( _nValue );
    }


    OUString lcl_toXSD_UNODate_typed( const css::util::Date& rDate )
    {

        OUStringBuffer sInfo;
        lcl_appendInt32ToBuffer( rDate.Year, sInfo, 4 );
        sInfo.append( "-" );
        lcl_appendInt32ToBuffer( rDate.Month, sInfo, 2 );
        sInfo.append( "-" );
        lcl_appendInt32ToBuffer( rDate.Day, sInfo, 2 );

        return sInfo.makeStringAndClear();
    }


    OUString lcl_toXSD_UNODate( const Any& rAny )
    {
        css::util::Date aDate;
        OSL_VERIFY( rAny >>= aDate );
        return lcl_toXSD_UNODate_typed( aDate );
    }


    css::util::Date lcl_toUNODate( std::u16string_view rString )
    {
        css::util::Date aDate( 1, 1, 1900 );

        bool bWellformed = ISO8601parseDate(rString, aDate);

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
            throw com::sun::star::lang::IllegalArgumentException();

        return aDate;
    }


    Any lcl_toAny_UNODate( const OUString& rString )
    {
        return Any( lcl_toUNODate( rString ) );
    }


    OUString lcl_toXSD_UNOTime_typed( const css::util::Time& rTime )
    {

        OUStringBuffer sInfo;
        lcl_appendInt32ToBuffer( rTime.Hours, sInfo, 2 );
        sInfo.append( ":" );
        lcl_appendInt32ToBuffer( rTime.Minutes, sInfo, 2 );
        sInfo.append( ":" );
        lcl_appendInt32ToBuffer( rTime.Seconds, sInfo, 2 );
        if ( rTime.NanoSeconds != 0 )
        {
            OSL_ENSURE(rTime.NanoSeconds < 1000000000,"NanoSeconds cannot be more than 999 999 999");
            sInfo.append('.');
            std::ostringstream ostr;
            ostr.fill('0');
            ostr.width(9);
            ostr << rTime.NanoSeconds;
            sInfo.appendAscii(ostr.str().c_str());
        }

        return sInfo.makeStringAndClear();
    }


    OUString lcl_toXSD_UNOTime( const Any& rAny )
    {
        css::util::Time aTime;
        OSL_VERIFY( rAny >>= aTime );
        return lcl_toXSD_UNOTime_typed( aTime );
    }


    css::util::Time lcl_toUNOTime( std::u16string_view rString )
    {
        css::util::Time aTime;

        bool bWellformed = ISO8601parseTime(rString, aTime);

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
                ||  ( aTime.NanoSeconds != 0 )
                )
            )
            bWellformed = false;

        // all okay?
        if ( !bWellformed )
            throw com::sun::star::lang::IllegalArgumentException();

        return aTime;
    }


    Any lcl_toAny_UNOTime( const OUString& rString )
    {
        return Any( lcl_toUNOTime( rString ) );
    }


    OUString lcl_toXSD_UNODateTime( const Any& rAny )
    {
        css::util::DateTime aDateTime;
        OSL_VERIFY( rAny >>= aDateTime );

        css::util::Date aDate( aDateTime.Day, aDateTime.Month, aDateTime.Year );
        OUString sDate = lcl_toXSD_UNODate_typed( aDate );

        css::util::Time const aTime( aDateTime.NanoSeconds, aDateTime.Seconds,
                    aDateTime.Minutes, aDateTime.Hours, aDateTime.IsUTC);
        OUString sTime = lcl_toXSD_UNOTime_typed( aTime );

        OUString sRet = sDate + "T" + sTime;
        return sRet;
    }


    Any lcl_toAny_UNODateTime( const OUString& rString )
    {
        // separate the date from the time part
        sal_Int32 nDateTimeSep = rString.indexOf( 'T' );
        if ( nDateTimeSep == -1 )
            nDateTimeSep = rString.indexOf( 't' );

        css::util::Date aDate;
        css::util::Time aTime;
        if ( nDateTimeSep == -1 )
        {   // no time part
            aDate = lcl_toUNODate( rString );
        }
        else
        {
            aDate = lcl_toUNODate( rString.subView( 0, nDateTimeSep ) );
            aTime = lcl_toUNOTime( rString.subView( nDateTimeSep + 1 ) );
        }
        css::util::DateTime aDateTime(
            aTime.NanoSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
            aDate.Day, aDate.Month, aDate.Year, aTime.IsUTC
        );
        return Any( aDateTime );
    }
}


void Convert::init()
{
    maMap[ cppu::UnoType<OUString>::get() ] = Convert_t(&lcl_toXSD_OUString, &lcl_toAny_OUString);
    maMap[ cppu::UnoType<bool>::get() ] = Convert_t(&lcl_toXSD_bool, &lcl_toAny_bool);
    maMap[ cppu::UnoType<double>::get() ] = Convert_t(&lcl_toXSD_double, &lcl_toAny_double);
    maMap[ cppu::UnoType<css::util::Date>::get() ] = Convert_t( &lcl_toXSD_UNODate, &lcl_toAny_UNODate );
    maMap[ cppu::UnoType<css::util::Time>::get() ] = Convert_t( &lcl_toXSD_UNOTime, &lcl_toAny_UNOTime );
    maMap[ cppu::UnoType<css::util::DateTime>::get() ] = Convert_t( &lcl_toXSD_UNODateTime, &lcl_toAny_UNODateTime );
}


Convert& Convert::get()
{
    // create our Singleton instance on demand
    static Convert aConvert;
    return aConvert;
}

bool Convert::hasType( const css::uno::Type& rType )
{
    return maMap.find( rType ) != maMap.end();
}

css::uno::Sequence<css::uno::Type> Convert::getTypes() const
{
    return comphelper::mapKeysToSequence( maMap );
}

OUString Convert::toXSD( const css::uno::Any& rAny )
{
    Map_t::iterator aIter = maMap.find( rAny.getValueType() );
    return aIter != maMap.end() ? aIter->second.first( rAny ) : OUString();
}

css::uno::Any Convert::toAny( const OUString& rValue,
                              const css::uno::Type& rType )
{
    Map_t::iterator aIter = maMap.find( rType );
    return aIter != maMap.end() ? aIter->second.second( rValue ) : css::uno::Any();
}


OUString Convert::collapseWhitespace( const OUString& _rString )
{
    sal_Int32 nLength = _rString.getLength();
    OUStringBuffer aBuffer( nLength );
    const sal_Unicode* pStr = _rString.getStr();
    bool bStrip = true;
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        sal_Unicode c = pStr[i];
        if( c == u'\x0008' ||
            c == u'\x000A' ||
            c == u'\x000D' ||
            c == u' ' )
        {
            if( ! bStrip )
            {
                aBuffer.append( u' ' );
                bStrip = true;
            }
        }
        else
        {
            bStrip = false;
            aBuffer.append( c );
        }
    }
    if( aBuffer[ aBuffer.getLength() - 1 ] == u' ' )
        aBuffer.setLength( aBuffer.getLength() - 1 );
    return aBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
