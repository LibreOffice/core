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

// GUID =======================================================================

BiffGuid::BiffGuid()
{
    ::std::fill_n( mpnData, sizeof( mpnData ), 0 );
}

BiffGuid::BiffGuid(
        sal_uInt32 nData1, sal_uInt16 nData2, sal_uInt16 nData3,
        sal_uInt8 nData41, sal_uInt8 nData42, sal_uInt8 nData43, sal_uInt8 nData44,
        sal_uInt8 nData45, sal_uInt8 nData46, sal_uInt8 nData47, sal_uInt8 nData48 )
{
    // convert to little endian -> makes streaming easy
    ByteOrderConverter::writeLittleEndian( mpnData, nData1 );
    ByteOrderConverter::writeLittleEndian( mpnData + 4, nData2 );
    ByteOrderConverter::writeLittleEndian( mpnData + 6, nData3 );
    mpnData[  8 ] = nData41;
    mpnData[  9 ] = nData42;
    mpnData[ 10 ] = nData43;
    mpnData[ 11 ] = nData44;
    mpnData[ 12 ] = nData45;
    mpnData[ 13 ] = nData46;
    mpnData[ 14 ] = nData47;
    mpnData[ 15 ] = nData48;
}

bool operator==( const BiffGuid& rGuid1, const BiffGuid& rGuid2 )
{
    return ::std::equal( rGuid1.mpnData, STATIC_ARRAY_END( rGuid1.mpnData ), rGuid2.mpnData );
}

bool operator<( const BiffGuid& rGuid1, const BiffGuid& rGuid2 )
{
    return ::std::lexicographical_compare(
        rGuid1.mpnData, STATIC_ARRAY_END( rGuid1.mpnData ),
        rGuid2.mpnData, STATIC_ARRAY_END( rGuid2.mpnData ) );
}

BiffInputStream& operator>>( BiffInputStream& rStrm, BiffGuid& rGuid )
{
    rStrm.readMemory( rGuid.mpnData, 16 );  // mpnData always in little endian
    return rStrm;
}

BiffOutputStream& operator<<( BiffOutputStream& rStrm, const BiffGuid& rGuid )
{
    rStrm.writeBlock( rGuid.mpnData, 16 ); // mpnData already in little endian
    return rStrm;
}

// ============================================================================

namespace {

const sal_Int32 BIFF_RK_100FLAG             = 0x00000001;
const sal_Int32 BIFF_RK_INTFLAG             = 0x00000002;
const sal_Int32 BIFF_RK_VALUEMASK           = 0xFFFFFFFC;

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

} // namespace

// ============================================================================

const BiffGuid BiffHelper::maGuidStdHlink(
    0x79EAC9D0, 0xBAF9, 0x11CE, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B );

const BiffGuid BiffHelper::maGuidUrlMoniker(
    0x79EAC9E0, 0xBAF9, 0x11CE, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B );

const BiffGuid BiffHelper::maGuidFileMoniker(
    0x00000303, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 );

// conversion -----------------------------------------------------------------

double BiffHelper::calcDoubleFromRk( sal_Int32 nRkValue )
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

namespace {

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

} // namespace

bool BiffHelper::calcRkFromDouble( sal_Int32& ornRkValue, double fValue )
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

double BiffHelper::calcDoubleFromError( sal_uInt8 nErrorCode )
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

rtl_TextEncoding BiffHelper::calcTextEncodingFromCodePage( sal_uInt16 nCodePage )
{
    const CodePageEntry* pEntry = ::std::find_if( spCodePages, STATIC_ARRAY_END( spCodePages ), CodePageEntry_CPPred( nCodePage ) );
    if( pEntry == STATIC_ARRAY_END( spCodePages ) )
    {
        OSL_ENSURE( false, "UnitConverter::calcTextEncodingFromCodePage - unknown code page" );
        return RTL_TEXTENCODING_DONTKNOW;
    }
    return pEntry->meTextEnc;
}

sal_uInt16 BiffHelper::calcCodePageFromTextEncoding( rtl_TextEncoding eTextEnc )
{
    const CodePageEntry* pEntry = ::std::find_if( spCodePages, STATIC_ARRAY_END( spCodePages ), CodePageEntry_TEPred( eTextEnc ) );
    if( pEntry == STATIC_ARRAY_END( spCodePages ) )
    {
        OSL_ENSURE( false, "UnitConverter::calcCodePageFromTextEncoding - unsupported text encoding" );
        return 1252;
    }
    return pEntry->mnCodePage;
}

// ============================================================================

} // namespace xls
} // namespace oox

