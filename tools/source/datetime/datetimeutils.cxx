/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/datetimeutils.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>


/// Append the number as 2-digit when less than 10.
template<class TStringBuffer>
static void lcl_AppendTwoDigits( TStringBuffer &rBuffer, sal_Int32 nNum )
{
    if ( nNum < 0 || nNum > 99 )
    {
        rBuffer.append( "00" );
        return;
    }

    if ( nNum < 10 )
        rBuffer.append( '0' );

    rBuffer.append( nNum );
}

template<class TString, class TStringBuffer>
static TString DateTimeToStringImpl( const DateTime& rDateTime )
{
    const DateTime& aInUTC( rDateTime );
// HACK: this is correct according to the spec, but MSOffice believes everybody lives
// in UTC+0 when reading it back
//    aInUTC.ConvertToUTC();

    TStringBuffer aBuffer( 25 );
    aBuffer.append( sal_Int32( aInUTC.GetYear() ) );
    aBuffer.append( '-' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetMonth() );
    aBuffer.append( '-' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetDay() );
    aBuffer.append( 'T' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetHour() );
    aBuffer.append( ':' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetMin() );
    aBuffer.append( ':' );

    lcl_AppendTwoDigits( aBuffer, aInUTC.GetSec() );
    aBuffer.append( 'Z' ); // we are in UTC

    return aBuffer.makeStringAndClear();
}

OString DateTimeToOString( const DateTime& rDateTime )
{
    return DateTimeToStringImpl<OString,OStringBuffer>(rDateTime);
}

OUString DateTimeToOUString( const DateTime& rDateTime )
{
    return DateTimeToStringImpl<OUString,OUStringBuffer>(rDateTime);
}

OString DateToOString( const Date& rDate )
{
    tools::Time aTime( tools::Time::EMPTY );
    return DateTimeToOString( DateTime( rDate, aTime ) );
}

OUString DateToDDMMYYYYOUString( const Date& rDate )
{
    OUStringBuffer aBuffer( 25 );
    lcl_AppendTwoDigits( aBuffer, rDate.GetDay() );
    aBuffer.append( '/' );

    lcl_AppendTwoDigits( aBuffer, rDate.GetMonth() );
    aBuffer.append( '/' );

    aBuffer.append( sal_Int32( rDate.GetYear() ) );

    return aBuffer.makeStringAndClear();
}

std::ostream& operator<<(std::ostream& os, const Date& rDate)
{
    os << rDate.GetYear() << "-" << rDate.GetMonth() << "-" << rDate.GetDay();
    return os;
}
