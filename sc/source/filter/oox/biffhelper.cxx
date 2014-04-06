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

#include "biffhelper.hxx"

#include <rtl/math.hxx>
#include <rtl/tencinfo.h>
#include "biffinputstream.hxx"
#include "worksheethelper.hxx"
#include "oox/helper/binaryoutputstream.hxx"

namespace oox {
namespace xls {




namespace {

const sal_Int32 BIFF_RK_100FLAG             = 0x00000001;
const sal_Int32 BIFF_RK_INTFLAG             = 0x00000002;
const sal_Int32 BIFF_RK_VALUEMASK           = 0xFFFFFFFC;

union DecodedDouble
{
    double              mfValue;
    sal_math_Double     maStruct;

    inline explicit     DecodedDouble() {}
    inline explicit     DecodedDouble( double fValue ) : mfValue( fValue ) {}
};

} // namespace

// conversion -----------------------------------------------------------------

/*static*/ double BiffHelper::calcDoubleFromRk( sal_Int32 nRkValue )
{
    DecodedDouble aDecDbl( 0.0 );
    if( getFlag( nRkValue, BIFF_RK_INTFLAG ) )
    {
        sal_Int32 nTemp = nRkValue >> 2;
        setFlag< sal_Int32 >( nTemp, 0xE0000000, nRkValue < 0 );
        aDecDbl.mfValue = nTemp;
    }
    else
    {
        aDecDbl.maStruct.w32_parts.msw = static_cast< sal_uInt32 >( nRkValue & BIFF_RK_VALUEMASK );
    }

    if( getFlag( nRkValue, BIFF_RK_100FLAG ) )
        aDecDbl.mfValue /= 100.0;

    return aDecDbl.mfValue;
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
    DecodedDouble aDecDbl;
    ::rtl::math::setNan( &aDecDbl.mfValue );
    aDecDbl.maStruct.nan_parts.fraction_lo = nApiError;
    return aDecDbl.mfValue;
}

// BIFF12 import --------------------------------------------------------------

/*static*/ OUString BiffHelper::readString( SequenceInputStream& rStrm, bool b32BitLen, bool bAllowNulChars )
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
            aString = rStrm.readUnicodeArray( nCharCount, bAllowNulChars );
        }
    }
    return aString;
}

// BIFF2-BIFF8 import ---------------------------------------------------------

/*static*/ bool BiffHelper::isBofRecord( BiffInputStream& rStrm )
{
    return
        (rStrm.getRecId() == BIFF2_ID_BOF) ||
        (rStrm.getRecId() == BIFF3_ID_BOF) ||
        (rStrm.getRecId() == BIFF4_ID_BOF) ||
        (rStrm.getRecId() == BIFF5_ID_BOF);
}

/*static*/ bool BiffHelper::skipRecordBlock( BiffInputStream& rStrm, sal_uInt16 nEndRecId )
{
    sal_uInt16 nStartRecId = rStrm.getRecId();
    while( rStrm.startNextRecord() && (rStrm.getRecId() != nEndRecId) )
        if( rStrm.getRecId() == nStartRecId )
            skipRecordBlock( rStrm, nEndRecId );
    return !rStrm.isEof() && (rStrm.getRecId() == nEndRecId);
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
