/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <tools/datetimeutils.hxx>
#include <rtl/strbuf.hxx>


/
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
    aBuffer.append( 'Z' ); 

    return aBuffer.makeStringAndClear();
}
