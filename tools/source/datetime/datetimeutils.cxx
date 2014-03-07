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


/// Append the number as 2-digit when less than 10.
static void lcl_AppendTwoDigits( OStringBuffer &rBuffer, sal_Int32 nNum )
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

OString DateTimeToOString( const DateTime& rDateTime )
{
    DateTime aInUTC( rDateTime );
// HACK: this is correct according to the spec, but MSOffice believes everybody lives
// in UTC+0 when reading it back
//    aInUTC.ConvertToUTC();

    OStringBuffer aBuffer( 25 );
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

OString DateToOString( const Date& rDate )
{
    Time aTime( Time::EMPTY );
    return DateTimeToOString( DateTime( rDate, aTime ) );
}

OString DateToDDMMYYYYOString( const Date& rDate )
{
    OStringBuffer aBuffer( 25 );
    lcl_AppendTwoDigits( aBuffer, rDate.GetDay() );
    aBuffer.append( '/' );

    lcl_AppendTwoDigits( aBuffer, rDate.GetMonth() );
    aBuffer.append( '/' );

    aBuffer.append( sal_Int32( rDate.GetYear() ) );

    return aBuffer.makeStringAndClear();
}
