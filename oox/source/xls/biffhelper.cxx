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

#include "oox/xls/biffhelper.hxx"
#include <rtl/math.hxx>
#include <rtl/tencinfo.h>
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/biffoutputstream.hxx"
#include "oox/xls/worksheethelper.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_Int32 BIFF_RK_100FLAG             = 0x00000001;
const sal_Int32 BIFF_RK_INTFLAG             = 0x00000002;
const sal_Int32 BIFF_RK_VALUEMASK           = 0xFFFFFFFC;

const sal_Int32 BITMAPFILEHEADER_SIZE       = 14;
const sal_Int32 BITMAPCOREHEADER_SIZE       = 12;
const sal_Int32 BITMAPINFOHEADER_SIZE       = 40;

const sal_uInt16 BIFF_IMGDATA_WMF           = 2;
const sal_uInt16 BIFF_IMGDATA_DIB           = 9;
const sal_uInt16 BIFF_IMGDATA_NATIVE        = 14;

// ----------------------------------------------------------------------------

union DecodedDouble
{
    double              mfValue;
    sal_math_Double     maStruct;

    inline explicit     DecodedDouble() {}
    inline explicit     DecodedDouble( double fValue ) : mfValue( fValue ) {}
};

bool lclCalcRkFromDouble( sal_Int32& ornRkValue, const DecodedDouble& rDecDbl )
{
    // double
    if( (rDecDbl.maStruct.w32_parts.lsw == 0) && ((rDecDbl.maStruct.w32_parts.msw & 0x3) == 0) )
    {
        ornRkValue = static_cast< sal_Int32 >( rDecDbl.maStruct.w32_parts.msw );
        return true;
    }

    // integer
    double fInt = 0.0;
    double fFrac = modf( rDecDbl.mfValue, &fInt );
    if( (fFrac == 0.0) && (-536870912.0 <= fInt) && (fInt <= 536870911.0) ) // 2^29
    {
        ornRkValue = static_cast< sal_Int32 >( fInt );
        ornRkValue <<= 2;
        ornRkValue |= BIFF_RK_INTFLAG;
        return true;
    }

    return false;
}

bool lclCalcRkFromDouble( sal_Int32& ornRkValue, double fValue )
{
    DecodedDouble aDecDbl( fValue );
    if( lclCalcRkFromDouble( ornRkValue, aDecDbl ) )
        return true;

    aDecDbl.mfValue *= 100.0;
    if( lclCalcRkFromDouble( ornRkValue, aDecDbl ) )
    {
        ornRkValue |= BIFF_RK_100FLAG;
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------

void lclImportImgDataDib( StreamDataSequence& orDataSeq, BiffInputStream& rStrm, sal_Int32 nBytes, BiffType eBiff )
{
    /*  The IMGDATA record for bitmap format contains a Windows DIB (a bitmap
        file without the 'BITMAPFILEHEADER' header structure). Usually, the DIB
        header consists of 12 bytes (called 'OS/2 V1 header' or
        'BITMAPCOREHEADER', see http://en.wikipedia.org/wiki/BMP_file_format)
        followed by the remaining pixel data, but the 'Windows V3' or
        'BITMAPINFOHEADER' is also supported here. This function creates a
        complete 'BMP file' that can be read by the OOo graphic provider used
        to import graphic objects. For that, the BITMAPFILEHEADER has to be
        inserted before the DIB data, and it has to contain the correct offset
        to the pixel data. Currently, in real life there are only 24-bit and
        32-bit DIBs (without color palette) in use. This code relies on this
        fact and calculates the offset to the pixel data according to the size
        of the DIB header.
        Remaining tasks are (if really used somewhere):
        - Support of DIBs with color palette,
        - Support of 'Windows V4' and 'Windows V5' DIB header. */

    // read and check validity of DIB header
    sal_Int64 nInStrmPos = rStrm.tell();
    sal_Int32 nDibHdrSize = rStrm.readInt32();
    sal_uInt16 nPlanes = 0, nDepth = 0;
    switch( nDibHdrSize )
    {
        case BITMAPCOREHEADER_SIZE:
            rStrm.skip( 4 );    // width/height as 16-bit integer
            rStrm >> nPlanes >> nDepth;
        break;
        case BITMAPINFOHEADER_SIZE:
            rStrm.skip( 8 );    // width/height as 32-bit integer
            rStrm >> nPlanes >> nDepth;
        break;
    }
    rStrm.seek( nInStrmPos );

    if( (nPlanes == 1) && ((nDepth == 24) || (nDepth == 32)) )
    {
        // allocate enough space for the BITMAPFILEHEADER and the DIB data
        orDataSeq.realloc( BITMAPFILEHEADER_SIZE + nBytes );
        SequenceOutputStream aOutStrm( orDataSeq );

        // write the BITMAPFILEHEADER of a regular BMP file
        sal_Int32 nBmpSize = BITMAPFILEHEADER_SIZE + nBytes;
        sal_Int32 nOffset = BITMAPFILEHEADER_SIZE + nDibHdrSize;
        aOutStrm << sal_uInt16( 0x4D42 ) << nBmpSize << sal_Int32( 0 ) << nOffset;

        // copy the DIB header
        rStrm.copyToStream( aOutStrm, nDibHdrSize );
        nBytes -= nDibHdrSize;

        /*  Excel 3.x and Excel 4.x seem to write broken or out-dated DIB data.
            Usually they write a BITMAPCOREHEADER containing width, height,
            planes as usual. The pixel depth field is set to 32 bit (though
            this is not allowed according to documentation). Between that
            header and the actual pixel data, 3 unused bytes are inserted. This
            does even confuse Excel 5.x and later, which cannot read the image
            data correctly. */
        if( (eBiff <= BIFF4) && (nDibHdrSize == BITMAPCOREHEADER_SIZE) && (nDepth == 32) )
        {
            // skip the dummy bytes in input stream
            rStrm.skip( 3 );
            nBytes -= 3;
            // correct the total BMP file size in output stream
            sal_Int64 nOutStrmPos = aOutStrm.tell();
            aOutStrm.seek( 2 );
            aOutStrm << sal_Int32( nBmpSize - 3 );
            aOutStrm.seek( nOutStrmPos );
        }

        // copy remaining pixel data to output stream
        rStrm.copyToStream( aOutStrm, nBytes );
    }
    rStrm.seek( nInStrmPos + nBytes );
}

} // namespace

// ============================================================================

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

/*static*/ bool BiffHelper::calcRkFromDouble( sal_Int32& ornRkValue, double fValue )
{
    if( lclCalcRkFromDouble( ornRkValue, fValue ) )
        return true;

    if( lclCalcRkFromDouble( ornRkValue, fValue * 100 ) )
    {
        ornRkValue |= BIFF_RK_100FLAG;
        return true;
    }

    return false;
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
        default:    OSL_ENSURE( false, "BiffHelper::calcDoubleFromError - unknown error code" );
    }
    DecodedDouble aDecDbl;
    ::rtl::math::setNan( &aDecDbl.mfValue );
    aDecDbl.maStruct.nan_parts.fraction_lo = nApiError;
    return aDecDbl.mfValue;
}

/*static*/ rtl_TextEncoding BiffHelper::calcTextEncodingFromCodePage( sal_uInt16 nCodePage )
{
    // some specials for BIFF
    switch( nCodePage )
    {
        case 1200:  return RTL_TEXTENCODING_DONTKNOW;       // BIFF8 Unicode
        case 32768: return RTL_TEXTENCODING_APPLE_ROMAN;
        case 32769: return RTL_TEXTENCODING_MS_1252;        // BIFF2-BIFF3
    }

    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
    OSL_ENSURE( eTextEnc != RTL_TEXTENCODING_DONTKNOW, "BiffHelper::calcTextEncodingFromCodePage - unknown code page" );
    return eTextEnc;
}

/*static*/ sal_uInt16 BiffHelper::calcCodePageFromTextEncoding( rtl_TextEncoding eTextEnc )
{
    sal_uInt32 nCodePage = rtl_getWindowsCodePageFromTextEncoding( eTextEnc );
    OSL_ENSURE( (0 < nCodePage) && (nCodePage <= SAL_MAX_UINT16), "BiffHelper::calcCodePageFromTextEncoding - unknown text encoding" );
    return static_cast< sal_uInt16 >( (nCodePage == 0) ? 1252 : nCodePage );
}

/*static*/ void BiffHelper::importImgData( StreamDataSequence& orDataSeq, BiffInputStream& rStrm, BiffType eBiff )
{
    sal_uInt16 nFormat, nEnv;
    sal_Int32 nBytes;
    rStrm >> nFormat >> nEnv >> nBytes;
    OSL_ENSURE( nBytes > 0, "BiffHelper::importImgData - invalid data size" );
    if( (0 < nBytes) && (nBytes <= rStrm.getRemaining()) )
    {
        switch( nFormat )
        {
//            case BIFF_IMGDATA_WMF:      /* TODO */                                              break;
            case BIFF_IMGDATA_DIB:      lclImportImgDataDib( orDataSeq, rStrm, nBytes, eBiff ); break;
//            case BIFF_IMGDATA_NATIVE:   /* TODO */                                              break;
            default:                    OSL_ENSURE( false, "BiffHelper::importImgData - unknown image format" );
        }
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

