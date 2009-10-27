/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: biffhelper.cxx,v $
 * $Revision: 1.3.22.1 $
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
#include <algorithm>
#include <rtl/math.hxx>
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

static const struct CodePageEntry
{
    sal_uInt16          mnCodePage;
    rtl_TextEncoding    meTextEnc;
}
spCodePages[] =
{
    {     437,  RTL_TEXTENCODING_IBM_437        },  // OEM US
//  {     720,  RTL_TEXTENCODING_IBM_720        },  // OEM Arabic
    {     737,  RTL_TEXTENCODING_IBM_737        },  // OEM Greek
    {     775,  RTL_TEXTENCODING_IBM_775        },  // OEM Baltic
    {     850,  RTL_TEXTENCODING_IBM_850        },  // OEM Latin I
    {     852,  RTL_TEXTENCODING_IBM_852        },  // OEM Latin II (Central European)
    {     855,  RTL_TEXTENCODING_IBM_855        },  // OEM Cyrillic
    {     857,  RTL_TEXTENCODING_IBM_857        },  // OEM Turkish
//  {     858,  RTL_TEXTENCODING_IBM_858        },  // OEM Multilingual Latin I with Euro
    {     860,  RTL_TEXTENCODING_IBM_860        },  // OEM Portugese
    {     861,  RTL_TEXTENCODING_IBM_861        },  // OEM Icelandic
    {     862,  RTL_TEXTENCODING_IBM_862        },  // OEM Hebrew
    {     863,  RTL_TEXTENCODING_IBM_863        },  // OEM Canadian (French)
    {     864,  RTL_TEXTENCODING_IBM_864        },  // OEM Arabic
    {     865,  RTL_TEXTENCODING_IBM_865        },  // OEM Nordic
    {     866,  RTL_TEXTENCODING_IBM_866        },  // OEM Cyrillic (Russian)
    {     869,  RTL_TEXTENCODING_IBM_869        },  // OEM Greek (Modern)
    {     874,  RTL_TEXTENCODING_MS_874         },  // MS Windows Thai
    {     932,  RTL_TEXTENCODING_MS_932         },  // MS Windows Japanese Shift-JIS
    {     936,  RTL_TEXTENCODING_MS_936         },  // MS Windows Chinese Simplified GBK
    {     949,  RTL_TEXTENCODING_MS_949         },  // MS Windows Korean (Wansung)
    {     950,  RTL_TEXTENCODING_MS_950         },  // MS Windows Chinese Traditional BIG5
    {    1200,  RTL_TEXTENCODING_DONTKNOW       },  // Unicode (BIFF8) - return *_DONTKNOW to preserve old code page
    {    1250,  RTL_TEXTENCODING_MS_1250        },  // MS Windows Latin II (Central European)
    {    1251,  RTL_TEXTENCODING_MS_1251        },  // MS Windows Cyrillic
    {    1252,  RTL_TEXTENCODING_MS_1252        },  // MS Windows Latin I (BIFF4-BIFF8)
    {    1253,  RTL_TEXTENCODING_MS_1253        },  // MS Windows Greek
    {    1254,  RTL_TEXTENCODING_MS_1254        },  // MS Windows Turkish
    {    1255,  RTL_TEXTENCODING_MS_1255        },  // MS Windows Hebrew
    {    1256,  RTL_TEXTENCODING_MS_1256        },  // MS Windows Arabic
    {    1257,  RTL_TEXTENCODING_MS_1257        },  // MS Windows Baltic
    {    1258,  RTL_TEXTENCODING_MS_1258        },  // MS Windows Vietnamese
    {    1361,  RTL_TEXTENCODING_MS_1361        },  // MS Windows Korean (Johab)
    {   10000,  RTL_TEXTENCODING_APPLE_ROMAN    },  // Apple Roman
    {   32768,  RTL_TEXTENCODING_APPLE_ROMAN    },  // Apple Roman
    {   32769,  RTL_TEXTENCODING_MS_1252        }   // MS Windows Latin I (BIFF2-BIFF3)
};

/** Predicate to search by given code page. */
struct CodePageEntry_CPPred
{
    inline explicit     CodePageEntry_CPPred( sal_uInt16 nCodePage ) : mnCodePage( nCodePage ) {}
    inline bool         operator()( const CodePageEntry& rEntry ) const { return rEntry.mnCodePage == mnCodePage; }
    sal_uInt16          mnCodePage;
};

/** Predicate to search by given text encoding. */
struct CodePageEntry_TEPred
{
    inline explicit     CodePageEntry_TEPred( rtl_TextEncoding eTextEnc ) : meTextEnc( eTextEnc ) {}
    inline bool         operator()( const CodePageEntry& rEntry ) const { return rEntry.meTextEnc == meTextEnc; }
    rtl_TextEncoding    meTextEnc;
};

// ----------------------------------------------------------------------------

bool lclCalcRkFromDouble( sal_Int32& ornRkValue, double fValue )
{
    // double
    const sal_math_Double* pValue = reinterpret_cast< const sal_math_Double* >( &fValue );
    if( (pValue->w32_parts.lsw == 0) && ((pValue->w32_parts.msw & 0x3) == 0) )
    {
        ornRkValue = static_cast< sal_Int32 >( pValue->w32_parts.msw );
        return true;
    }

    // integer
    double fInt = 0.0;
    double fFrac = modf( fValue, &fInt );
    if( (fFrac == 0.0) && (-536870912.0 <= fInt) && (fInt <= 536870911.0) ) // 2^29
    {
        ornRkValue = static_cast< sal_Int32 >( fInt );
        ornRkValue <<= 2;
        ornRkValue |= BIFF_RK_INTFLAG;
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
        aOutStrm.copyStream( rStrm, nDibHdrSize );
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

        // copy remaining pixel data top output stream
        aOutStrm.copyStream( rStrm, nBytes );
    }
    rStrm.seek( nInStrmPos + nBytes );
}

} // namespace

// ============================================================================

// conversion -----------------------------------------------------------------

/*static*/ double BiffHelper::calcDoubleFromRk( sal_Int32 nRkValue )
{
    double fValue = 0.0;
    if( getFlag( nRkValue, BIFF_RK_INTFLAG ) )
    {
        sal_Int32 nTemp = nRkValue >> 2;
        setFlag< sal_Int32 >( nTemp, 0xE0000000, nRkValue < 0 );
        fValue = nTemp;
    }
    else
    {
        sal_math_Double* pDouble = reinterpret_cast< sal_math_Double* >( &fValue );
        pDouble->w32_parts.msw = static_cast< sal_uInt32 >( nRkValue & BIFF_RK_VALUEMASK );
    }

    if( getFlag( nRkValue, BIFF_RK_100FLAG ) )
        fValue /= 100.0;

    return fValue;
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
    double fValue;
    ::rtl::math::setNan( &fValue );
    reinterpret_cast< sal_math_Double* >( &fValue )->nan_parts.fraction_lo = nApiError;
    return fValue;
}

/*static*/ rtl_TextEncoding BiffHelper::calcTextEncodingFromCodePage( sal_uInt16 nCodePage )
{
    const CodePageEntry* pEntry = ::std::find_if( spCodePages, STATIC_ARRAY_END( spCodePages ), CodePageEntry_CPPred( nCodePage ) );
    if( pEntry == STATIC_ARRAY_END( spCodePages ) )
    {
        OSL_ENSURE( false, "UnitConverter::calcTextEncodingFromCodePage - unknown code page" );
        return RTL_TEXTENCODING_DONTKNOW;
    }
    return pEntry->meTextEnc;
}

/*static*/ sal_uInt16 BiffHelper::calcCodePageFromTextEncoding( rtl_TextEncoding eTextEnc )
{
    const CodePageEntry* pEntry = ::std::find_if( spCodePages, STATIC_ARRAY_END( spCodePages ), CodePageEntry_TEPred( eTextEnc ) );
    if( pEntry == STATIC_ARRAY_END( spCodePages ) )
    {
        OSL_ENSURE( false, "UnitConverter::calcCodePageFromTextEncoding - unsupported text encoding" );
        return 1252;
    }
    return pEntry->mnCodePage;
}

/*static*/ void BiffHelper::importImgData( StreamDataSequence& orDataSeq, BiffInputStream& rStrm, BiffType eBiff )
{
    sal_uInt16 nFormat, nEnv;
    sal_Int32 nBytes;
    rStrm >> nFormat >> nEnv >> nBytes;
    OSL_ENSURE( (nFormat == BIFF_IMGDATA_WMF) || (nFormat == BIFF_IMGDATA_DIB) || (nFormat == BIFF_IMGDATA_NATIVE), "BiffHelper::importImgData - unknown format" );
    OSL_ENSURE( nBytes > 0, "BiffHelper::importImgData - invalid data size" );
    if( (0 < nBytes) && (nBytes <= rStrm.getRemaining()) )
    {
        switch( nFormat )
        {
            case BIFF_IMGDATA_WMF:      /* TODO */                                              break;
            case BIFF_IMGDATA_DIB:      lclImportImgDataDib( orDataSeq, rStrm, nBytes, eBiff ); break;
            case BIFF_IMGDATA_NATIVE:   /* TODO */                                              break;
            default:                    OSL_ENSURE( false, "BiffHelper::importImgData - unknown image format" );
        }
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

