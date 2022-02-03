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

#include <biffhelper.hxx>

#include <osl/diagnose.h>
#include <oox/helper/binaryinputstream.hxx>

#include <limits>

namespace oox::xls {

namespace {

const sal_Int32 BIFF_RK_100FLAG             = 0x00000001;
const sal_Int32 BIFF_RK_INTFLAG             = 0x00000002;
const sal_Int32 BIFF_RK_VALUEMASK           = 0xFFFFFFFC;

} // namespace

// conversion -----------------------------------------------------------------

/*static*/ double BiffHelper::calcDoubleFromRk( sal_Int32 nRkValue )
{
    sal_math_Double  aMathDouble{};
    if( getFlag( nRkValue, BIFF_RK_INTFLAG ) )
    {
        sal_Int32 nTemp = nRkValue >> 2;
        setFlag< sal_Int32 >( nTemp, 0xE0000000, nRkValue < 0 );
        aMathDouble.value = nTemp;
    }
    else
    {
        aMathDouble.w32_parts.msw = static_cast< sal_uInt32 >( nRkValue & BIFF_RK_VALUEMASK );
    }
    if( getFlag( nRkValue, BIFF_RK_100FLAG ) )
        aMathDouble.value /= 100.0;

    return aMathDouble.value;
}
/*static*/ double BiffHelper::calcDoubleFromError( sal_uInt8 nErrorCode )
{
    sal_uInt16 nApiError = 0x7FFF;
    switch( nErrorCode )
    {
        case BIFF_ERR_NULL:     nApiError = 521;    break;
        case BIFF_ERR_DIV0:     nApiError = 532;    break;
        case BIFF_ERR_VALUE:    nApiError = 519;    break;
        case BIFF_ERR_REF:      nApiError = 524;    break;
        case BIFF_ERR_NAME:     nApiError = 525;    break;
        case BIFF_ERR_NUM:      nApiError = 503;    break;
        case BIFF_ERR_NA:       nApiError = 0x7FFF; break;
        default:    OSL_FAIL( "BiffHelper::calcDoubleFromError - unknown error code" );
    }
    sal_math_Double  aMathDouble;
    aMathDouble.value = std::numeric_limits<double>::quiet_NaN();
    aMathDouble.nan_parts.fraction_lo = nApiError;
    return  aMathDouble.value;
}

// BIFF12 import --------------------------------------------------------------

/*static*/ OUString BiffHelper::readString( SequenceInputStream& rStrm, bool b32BitLen )
{
    OUString aString;
    if( !rStrm.isEof() )
    {
        sal_Int32 nCharCount = b32BitLen ? rStrm.readValue< sal_Int32 >() : rStrm.readValue< sal_Int16 >();
        // string length -1 is often used to indicate a missing string
        OSL_ENSURE( !rStrm.isEof() && (nCharCount >= -1), "BiffHelper::readString - invalid string length" );
        if( !rStrm.isEof() && (nCharCount > 0) )
        {
            // SequenceInputStream always supports getRemaining()
            nCharCount = ::std::min( nCharCount, static_cast< sal_Int32 >( rStrm.getRemaining() / 2 ) );
            aString = rStrm.readUnicodeArray( nCharCount );
        }
    }
    return aString;
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
