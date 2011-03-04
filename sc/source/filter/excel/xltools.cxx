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
#include "precompiled_sc.hxx"

#include <algorithm>
#include <math.h>
#include <sal/mathconf.h>
#include <unotools/fontcvt.hxx>
#include <sfx2/objsh.hxx>
#include <sal/macros.h>
#include <editeng/editstat.hxx>
#include <filter/msfilter/msvbahelper.hxx>
#include "xestream.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "editutil.hxx"
#include "formula/errorcodes.hxx"
#include "globstr.hrc"
#include "xlstyle.hxx"
#include "xlname.hxx"
#include "xistream.hxx"
#include "xiroot.hxx"
#include "xltools.hxx"

using ::rtl::OUString;

// GUID import/export =========================================================

XclGuid::XclGuid()
{
    ::std::fill( mpnData, STATIC_TABLE_END( mpnData ), 0 );
}

XclGuid::XclGuid(
        sal_uInt32 nData1, sal_uInt16 nData2, sal_uInt16 nData3,
        sal_uInt8 nData41, sal_uInt8 nData42, sal_uInt8 nData43, sal_uInt8 nData44,
        sal_uInt8 nData45, sal_uInt8 nData46, sal_uInt8 nData47, sal_uInt8 nData48 )
{
    // convert to little endian -> makes streaming easy
    UInt32ToSVBT32( nData1, mpnData );
    ShortToSVBT16( nData2, mpnData + 4 );
    ShortToSVBT16( nData3, mpnData + 6 );
    mpnData[  8 ] = nData41;
    mpnData[  9 ] = nData42;
    mpnData[ 10 ] = nData43;
    mpnData[ 11 ] = nData44;
    mpnData[ 12 ] = nData45;
    mpnData[ 13 ] = nData46;
    mpnData[ 14 ] = nData47;
    mpnData[ 15 ] = nData48;
}

bool operator==( const XclGuid& rCmp1, const XclGuid& rCmp2 )
{
    return ::std::equal( rCmp1.mpnData, STATIC_TABLE_END( rCmp1.mpnData ), rCmp2.mpnData );
}

bool operator<( const XclGuid& rCmp1, const XclGuid& rCmp2 )
{
    return ::std::lexicographical_compare(
        rCmp1.mpnData, STATIC_TABLE_END( rCmp1.mpnData ),
        rCmp2.mpnData, STATIC_TABLE_END( rCmp2.mpnData ) );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclGuid& rGuid )
{
    rStrm.Read( rGuid.mpnData, 16 );     // mpnData always in little endian
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclGuid& rGuid )
{
    rStrm.Write( rGuid.mpnData, 16 );    // mpnData already in little endian
    return rStrm;
}

// Excel Tools ================================================================

// GUID's ---------------------------------------------------------------------

const XclGuid XclTools::maGuidStdLink(
    0x79EAC9D0, 0xBAF9, 0x11CE, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B );

const XclGuid XclTools::maGuidUrlMoniker(
    0x79EAC9E0, 0xBAF9, 0x11CE, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B );

const XclGuid XclTools::maGuidFileMoniker(
    0x00000303, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 );

// numeric conversion ---------------------------------------------------------

double XclTools::GetDoubleFromRK( sal_Int32 nRKValue )
{
    union
    {
        double fVal;
        sal_math_Double smD;
    };
    fVal = 0.0;

    if( ::get_flag( nRKValue, EXC_RK_INTFLAG ) )
    {
        sal_Int32 nTemp = nRKValue >> 2;
        ::set_flag< sal_Int32 >( nTemp, 0xE0000000, nRKValue < 0 );
        fVal = nTemp;
    }
    else
    {
        smD.w32_parts.msw = nRKValue & EXC_RK_VALUEMASK;
    }

    if( ::get_flag( nRKValue, EXC_RK_100FLAG ) )
        fVal /= 100.0;

    return fVal;
}

bool XclTools::GetRKFromDouble( sal_Int32& rnRKValue, double fValue )
{
    double fFrac, fInt;

    // integer
    fFrac = modf( fValue, &fInt );
    if( (fFrac == 0.0) && (fInt >= -536870912.0) && (fInt <= 536870911.0) ) // 2^29
    {
        rnRKValue = static_cast< sal_Int32 >( fInt );
        rnRKValue <<= 2;
        rnRKValue |= EXC_RK_INT;
        return true;
    }

    // integer/100
    fFrac = modf( fValue * 100.0, &fInt );
    if( (fFrac == 0.0) && (fInt >= -536870912.0) && (fInt <= 536870911.0) )
    {
        rnRKValue = static_cast< sal_Int32 >( fInt );
        rnRKValue <<= 2;
        rnRKValue |= EXC_RK_INT100;
        return true;
    }

    // double
    return false;
}

sal_Int32 XclTools::GetScRotation( sal_uInt16 nXclRot, sal_Int32 nRotForStacked )
{
    if( nXclRot == EXC_ROT_STACKED )
        return nRotForStacked;
    DBG_ASSERT( nXclRot <= 180, "XclTools::GetScRotation - illegal rotation angle" );
    return static_cast< sal_Int32 >( (nXclRot <= 180) ? (100 * ((nXclRot > 90) ? (450 - nXclRot) : nXclRot)) : 0 );
}

sal_uInt8 XclTools::GetXclRotation( sal_Int32 nScRot )
{
    sal_Int32 nXclRot = nScRot / 100;
    if( (0 <= nXclRot) && (nXclRot <= 90) )
        return static_cast< sal_uInt8 >( nXclRot );
    if( nXclRot < 180 )
        return static_cast< sal_uInt8 >( 270 - nXclRot );
    if( nXclRot < 270 )
        return static_cast< sal_uInt8 >( nXclRot - 180 );
    if( nXclRot < 360 )
        return static_cast< sal_uInt8 >( 450 - nXclRot );
    return 0;
}

sal_uInt8 XclTools::GetXclRotFromOrient( sal_uInt8 nXclOrient )
{
    switch( nXclOrient )
    {
        case EXC_ORIENT_NONE:       return EXC_ROT_NONE;
        case EXC_ORIENT_STACKED:    return EXC_ROT_STACKED;
        case EXC_ORIENT_90CCW:      return EXC_ROT_90CCW;
        case EXC_ORIENT_90CW:       return EXC_ROT_90CW;
        default:    DBG_ERRORFILE( "XclTools::GetXclRotFromOrient - unknown text orientation" );
    }
    return EXC_ROT_NONE;
}

sal_uInt8 XclTools::GetXclOrientFromRot( sal_uInt16 nXclRot )
{
    if( nXclRot == EXC_ROT_STACKED )
        return EXC_ORIENT_STACKED;
    DBG_ASSERT( nXclRot <= 180, "XclTools::GetXclOrientFromRot - unknown text rotation" );
    if( (45 < nXclRot) && (nXclRot <= 90) )
        return EXC_ORIENT_90CCW;
    if( (135 < nXclRot) && (nXclRot <= 180) )
        return EXC_ORIENT_90CW;
    return EXC_ORIENT_NONE;
}

sal_uInt8 XclTools::GetXclErrorCode( USHORT nScError )
{
    using namespace ScErrorCodes;
    switch( nScError )
    {
        case errIllegalArgument:        return EXC_ERR_VALUE;
        case errIllegalFPOperation:     return EXC_ERR_NUM;     // maybe DIV/0 or NUM...
        case errDivisionByZero:         return EXC_ERR_DIV0;
        case errIllegalParameter:       return EXC_ERR_VALUE;
        case errPairExpected:           return EXC_ERR_VALUE;
        case errOperatorExpected:       return EXC_ERR_VALUE;
        case errVariableExpected:       return EXC_ERR_VALUE;
        case errParameterExpected:      return EXC_ERR_VALUE;
        case errNoValue:                return EXC_ERR_VALUE;
        case errCircularReference:      return EXC_ERR_VALUE;
        case errNoCode:                 return EXC_ERR_NULL;
        case errNoRef:                  return EXC_ERR_REF;
        case errNoName:                 return EXC_ERR_NAME;
        case errNoAddin:                return EXC_ERR_NAME;
        case errNoMacro:                return EXC_ERR_NAME;
        case NOTAVAILABLE:              return EXC_ERR_NA;
    }
    return EXC_ERR_NA;
}

USHORT XclTools::GetScErrorCode( sal_uInt8 nXclError )
{
    using namespace ScErrorCodes;
    switch( nXclError )
    {
        case EXC_ERR_NULL:  return errNoCode;
        case EXC_ERR_DIV0:  return errDivisionByZero;
        case EXC_ERR_VALUE: return errNoValue;
        case EXC_ERR_REF:   return errNoRef;
        case EXC_ERR_NAME:  return errNoName;
        case EXC_ERR_NUM:   return errIllegalFPOperation;
        case EXC_ERR_NA:    return NOTAVAILABLE;
        default:            DBG_ERRORFILE( "XclTools::GetScErrorCode - unknown error code" );
    }
    return NOTAVAILABLE;
}

double XclTools::ErrorToDouble( sal_uInt8 nXclError )
{
    union
    {
        double fVal;
        sal_math_Double smD;
    };
    ::rtl::math::setNan( &fVal );
    smD.nan_parts.fraction_lo = GetScErrorCode( nXclError );
    return fVal;
}

XclBoolError XclTools::ErrorToEnum( double& rfDblValue, sal_uInt8 bErrOrBool, sal_uInt8 nValue )
{
    XclBoolError eType;
    if( bErrOrBool )
    {
        // error value
        switch( nValue )
        {
            case EXC_ERR_NULL:  eType = xlErrNull;      break;
            case EXC_ERR_DIV0:  eType = xlErrDiv0;      break;
            case EXC_ERR_VALUE: eType = xlErrValue;     break;
            case EXC_ERR_REF:   eType = xlErrRef;       break;
            case EXC_ERR_NAME:  eType = xlErrName;      break;
            case EXC_ERR_NUM:   eType = xlErrNum;       break;
            case EXC_ERR_NA:    eType = xlErrNA;        break;
            default:            eType = xlErrUnknown;
        }
        rfDblValue = 0.0;
    }
    else
    {
        // Boolean value
        eType = nValue ? xlErrTrue : xlErrFalse;
        rfDblValue = nValue ? 1.0 : 0.0;
    }
    return eType;
}

sal_uInt16 XclTools::GetTwipsFromInch( double fInches )
{
    return static_cast< sal_uInt16 >(
        ::std::min( ::std::max( (fInches * EXC_TWIPS_PER_INCH + 0.5), 0.0 ), 65535.0 ) );
}

sal_uInt16 XclTools::GetTwipsFromHmm( sal_Int32 nHmm )
{
    return GetTwipsFromInch( static_cast< double >( nHmm ) / 1000.0 / CM_PER_INCH );
}

double XclTools::GetInchFromTwips( sal_Int32 nTwips )
{
    return static_cast< double >( nTwips ) / EXC_TWIPS_PER_INCH;
}

double XclTools::GetInchFromHmm( sal_Int32 nHmm )
{
    return GetInchFromTwips( GetTwipsFromHmm( nHmm ) );
}

sal_Int32 XclTools::GetHmmFromInch( double fInches )
{
    return static_cast< sal_Int32 >( fInches * CM_PER_INCH * 1000 );
}

sal_Int32 XclTools::GetHmmFromTwips( sal_Int32 nTwips )
{
    return GetHmmFromInch( GetInchFromTwips( nTwips ) );
}

USHORT XclTools::GetScColumnWidth( sal_uInt16 nXclWidth, long nScCharWidth )
{
    double fScWidth = static_cast< double >( nXclWidth ) / 256.0 * nScCharWidth + 0.5;
    return limit_cast< USHORT >( fScWidth );
}

sal_uInt16 XclTools::GetXclColumnWidth( USHORT nScWidth, long nScCharWidth )
{
    double fXclWidth = static_cast< double >( nScWidth ) * 256.0 / nScCharWidth + 0.5;
    return limit_cast< sal_uInt16 >( fXclWidth );
}

double XclTools::GetXclDefColWidthCorrection( long nXclDefFontHeight )
{
    return 40960.0 / ::std::max( nXclDefFontHeight - 15L, 60L ) + 50.0;
}

// formatting -----------------------------------------------------------------

Color XclTools::GetPatternColor( const Color& rPattColor, const Color& rBackColor, sal_uInt16 nXclPattern )
{
    // 0x00 == 0% transparence (full rPattColor)
    // 0x80 == 100% transparence (full rBackColor)
    static const sal_uInt8 pnRatioTable[] =
    {
        0x80, 0x00, 0x40, 0x20, 0x60, 0x40, 0x40, 0x40,     // 00 - 07
        0x40, 0x40, 0x20, 0x60, 0x60, 0x60, 0x60, 0x48,     // 08 - 15
        0x50, 0x70, 0x78                                    // 16 - 18
    };
    return (nXclPattern < SAL_N_ELEMENTS( pnRatioTable )) ?
        ScfTools::GetMixedColor( rPattColor, rBackColor, pnRatioTable[ nXclPattern ] ) : rPattColor;
}

// text encoding --------------------------------------------------------------

namespace {

const struct XclCodePageEntry
{
    sal_uInt16                  mnCodePage;
    rtl_TextEncoding            meTextEnc;
}
pCodePageTable[] =
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
const XclCodePageEntry* const pCodePageTableEnd = STATIC_TABLE_END( pCodePageTable );

struct XclCodePageEntry_CPPred
{
    inline explicit     XclCodePageEntry_CPPred( sal_uInt16 nCodePage ) : mnCodePage( nCodePage ) {}
    inline bool         operator()( const XclCodePageEntry& rEntry ) const { return rEntry.mnCodePage == mnCodePage; }
    sal_uInt16          mnCodePage;
};

struct XclCodePageEntry_TEPred
{
    inline explicit     XclCodePageEntry_TEPred( rtl_TextEncoding eTextEnc ) : meTextEnc( eTextEnc ) {}
    inline bool         operator()( const XclCodePageEntry& rEntry ) const { return rEntry.meTextEnc == meTextEnc; }
    rtl_TextEncoding    meTextEnc;
};

} // namespace

rtl_TextEncoding XclTools::GetTextEncoding( sal_uInt16 nCodePage )
{
    const XclCodePageEntry* pEntry = ::std::find_if( pCodePageTable, pCodePageTableEnd, XclCodePageEntry_CPPred( nCodePage ) );
    if( pEntry == pCodePageTableEnd )
    {
        DBG_ERROR2( "XclTools::GetTextEncoding - unknown code page: 0x%04hX (%d)", nCodePage, nCodePage );
        return RTL_TEXTENCODING_DONTKNOW;
    }
    return pEntry->meTextEnc;
}

sal_uInt16 XclTools::GetXclCodePage( rtl_TextEncoding eTextEnc )
{
    if( eTextEnc == RTL_TEXTENCODING_UNICODE )
        return 1200;    // for BIFF8

    const XclCodePageEntry* pEntry = ::std::find_if( pCodePageTable, pCodePageTableEnd, XclCodePageEntry_TEPred( eTextEnc ) );
    if( pEntry == pCodePageTableEnd )
    {
        OSL_TRACE( "XclTools::GetXclCodePage - unsupported text encoding: %d", eTextEnc );
        return 1252;
    }
    return pEntry->mnCodePage;
}

// font names -----------------------------------------------------------------

String XclTools::GetXclFontName( const String& rFontName )
{
    // substitute with MS fonts
    String aNewName( GetSubsFontName( rFontName, SUBSFONT_ONLYONE | SUBSFONT_MS ) );
    if( aNewName.Len() )
        return aNewName;
    return rFontName;
}

// built-in defined names -----------------------------------------------------

const String XclTools::maDefNamePrefix( RTL_CONSTASCII_USTRINGPARAM( "Excel_BuiltIn_" ) );

static const sal_Char* const ppcDefNames[] =
{
    "Consolidate_Area",
    "Auto_Open",
    "Auto_Close",
    "Extract",
    "Database",
    "Criteria",
    "Print_Area",
    "Print_Titles",
    "Recorder",
    "Data_Form",
    "Auto_Activate",
    "Auto_Deactivate",
    "Sheet_Title",
    "_FilterDatabase"
};

String XclTools::GetXclBuiltInDefName( sal_Unicode cBuiltIn )
{
    DBG_ASSERT( SAL_N_ELEMENTS( ppcDefNames ) == EXC_BUILTIN_UNKNOWN,
        "XclTools::GetXclBuiltInDefName - built-in defined name list modified" );
    String aDefName;
    if( cBuiltIn < SAL_N_ELEMENTS( ppcDefNames ) )
        aDefName.AssignAscii( ppcDefNames[ cBuiltIn ] );
    else
        aDefName = String::CreateFromInt32( cBuiltIn );
    return aDefName;
}

String XclTools::GetBuiltInDefName( sal_Unicode cBuiltIn )
{
    return String( maDefNamePrefix ).Append( GetXclBuiltInDefName( cBuiltIn ) );
}

sal_Unicode XclTools::GetBuiltInDefNameIndex( const String& rDefName )
{
    xub_StrLen nPrefixLen = maDefNamePrefix.Len();
    if( rDefName.EqualsIgnoreCaseAscii( maDefNamePrefix, 0, nPrefixLen ) )
    {
        for( sal_Unicode cBuiltIn = 0; cBuiltIn < EXC_BUILTIN_UNKNOWN; ++cBuiltIn )
        {
            String aBuiltInName( GetXclBuiltInDefName( cBuiltIn ) );
            xub_StrLen nBuiltInLen = aBuiltInName.Len();
            if( rDefName.EqualsIgnoreCaseAscii( aBuiltInName, nPrefixLen, nBuiltInLen ) )
            {
                // name can be followed by underline or space character
                xub_StrLen nNextCharPos = nPrefixLen + nBuiltInLen;
                sal_Unicode cNextChar = (rDefName.Len() > nNextCharPos) ? rDefName.GetChar( nNextCharPos ) : '\0';
                if( (cNextChar == '\0') || (cNextChar == ' ') || (cNextChar == '_') )
                    return cBuiltIn;
            }
        }
    }
    return EXC_BUILTIN_UNKNOWN;
}

// built-in style names -------------------------------------------------------

const String XclTools::maStyleNamePrefix1( RTL_CONSTASCII_USTRINGPARAM( "Excel_BuiltIn_" ) );
const String XclTools::maStyleNamePrefix2( RTL_CONSTASCII_USTRINGPARAM( "Excel Built-in " ) );

static const sal_Char* const ppcStyleNames[] =
{
    "",                 // "Normal" not used directly, but localized "Default"
    "RowLevel_",        // outline level will be appended
    "ColumnLevel_",     // outline level will be appended
    "Comma",
    "Currency",
    "Percent",
    "Comma_0",
    "Currency_0",
    "Hyperlink",
    "Followed_Hyperlink"
};

String XclTools::GetBuiltInStyleName( sal_uInt8 nStyleId, const String& rName, sal_uInt8 nLevel )
{
    String aStyleName;

    if( nStyleId == EXC_STYLE_NORMAL )  // "Normal" becomes "Default" style
    {
        aStyleName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
    }
    else
    {
        aStyleName = maStyleNamePrefix1;
        if( nStyleId < SAL_N_ELEMENTS( ppcStyleNames ) )
            aStyleName.AppendAscii( ppcStyleNames[ nStyleId ] );
        else if( rName.Len() > 0 )
            aStyleName.Append( rName );
        else
            aStyleName.Append( String::CreateFromInt32( nStyleId ) );
        if( (nStyleId == EXC_STYLE_ROWLEVEL) || (nStyleId == EXC_STYLE_COLLEVEL) )
            aStyleName.Append( String::CreateFromInt32( nLevel + 1 ) );
    }

    return aStyleName;
}

String XclTools::GetBuiltInStyleName( const String& rStyleName )
{
    return String( maStyleNamePrefix1 ).Append( rStyleName );
}

bool XclTools::IsBuiltInStyleName( const String& rStyleName, sal_uInt8* pnStyleId, xub_StrLen* pnNextChar )
{
    // "Default" becomes "Normal"
    if( rStyleName == ScGlobal::GetRscString( STR_STYLENAME_STANDARD ) )
    {
        if( pnStyleId ) *pnStyleId = EXC_STYLE_NORMAL;
        if( pnNextChar ) *pnNextChar = rStyleName.Len();
        return true;
    }

    // try the other built-in styles
    sal_uInt8 nFoundId = 0;
    xub_StrLen nNextChar = 0;

    xub_StrLen nPrefixLen = 0;
    if( rStyleName.EqualsIgnoreCaseAscii( maStyleNamePrefix1, 0, maStyleNamePrefix1.Len() ) )
        nPrefixLen = maStyleNamePrefix1.Len();
    else if( rStyleName.EqualsIgnoreCaseAscii( maStyleNamePrefix2, 0, maStyleNamePrefix2.Len() ) )
        nPrefixLen = maStyleNamePrefix2.Len();
    if( nPrefixLen > 0 )
    {
        String aShortName;
        for( sal_uInt8 nId = 0; nId < SAL_N_ELEMENTS( ppcStyleNames ); ++nId )
        {
            if( nId != EXC_STYLE_NORMAL )
            {
                aShortName.AssignAscii( ppcStyleNames[ nId ] );
                if( rStyleName.EqualsIgnoreCaseAscii( aShortName, nPrefixLen, aShortName.Len() ) &&
                    (nNextChar < nPrefixLen + aShortName.Len()) )
                {
                    nFoundId = nId;
                    nNextChar = nPrefixLen + aShortName.Len();
                }
            }
        }
    }

    if( nNextChar > 0 )
    {
        if( pnStyleId ) *pnStyleId = nFoundId;
        if( pnNextChar ) *pnNextChar = nNextChar;
        return true;
    }

    if( pnStyleId ) *pnStyleId = EXC_STYLE_USERDEF;
    if( pnNextChar ) *pnNextChar = 0;
    return nPrefixLen > 0;  // also return true for unknown built-in styles
}

bool XclTools::GetBuiltInStyleId( sal_uInt8& rnStyleId, sal_uInt8& rnLevel, const String& rStyleName )
{
    sal_uInt8 nStyleId;
    xub_StrLen nNextChar;
    if( IsBuiltInStyleName( rStyleName, &nStyleId, &nNextChar ) && (nStyleId != EXC_STYLE_USERDEF) )
    {
        if( (nStyleId == EXC_STYLE_ROWLEVEL) || (nStyleId == EXC_STYLE_COLLEVEL) )
        {
            String aLevel( rStyleName, nNextChar, STRING_LEN );
            sal_Int32 nLevel = aLevel.ToInt32();
            if( (String::CreateFromInt32( nLevel ) == aLevel) && (nLevel > 0) && (nLevel <= EXC_STYLE_LEVELCOUNT) )
            {
                rnStyleId = nStyleId;
                rnLevel = static_cast< sal_uInt8 >( nLevel - 1 );
                return true;
            }
        }
        else if( rStyleName.Len() == nNextChar )
        {
            rnStyleId = nStyleId;
            rnLevel = EXC_STYLE_NOLEVEL;
            return true;
        }
    }
    rnStyleId = EXC_STYLE_USERDEF;
    rnLevel = EXC_STYLE_NOLEVEL;
    return false;
}

// conditional formatting style names -----------------------------------------

const String XclTools::maCFStyleNamePrefix1( RTL_CONSTASCII_USTRINGPARAM( "Excel_CondFormat_" ) );
const String XclTools::maCFStyleNamePrefix2( RTL_CONSTASCII_USTRINGPARAM( "ConditionalStyle_" ) );

String XclTools::GetCondFormatStyleName( SCTAB nScTab, sal_Int32 nFormat, sal_uInt16 nCondition )
{
    return String( maCFStyleNamePrefix1 ).Append( String::CreateFromInt32( nScTab + 1 ) ).
                Append( '_' ).Append( String::CreateFromInt32( nFormat + 1 ) ).
                Append( '_' ).Append( String::CreateFromInt32( nCondition + 1 ) );
}

bool XclTools::IsCondFormatStyleName( const String& rStyleName, xub_StrLen* pnNextChar )
{
    xub_StrLen nPrefixLen = 0;
    if( rStyleName.EqualsIgnoreCaseAscii( maCFStyleNamePrefix1, 0, maCFStyleNamePrefix1.Len() ) )
        nPrefixLen = maCFStyleNamePrefix1.Len();
    else if( rStyleName.EqualsIgnoreCaseAscii( maCFStyleNamePrefix2, 0, maCFStyleNamePrefix2.Len() ) )
        nPrefixLen = maCFStyleNamePrefix2.Len();
    if( pnNextChar ) *pnNextChar = nPrefixLen;
    return nPrefixLen > 0;
}

// stream handling ------------------------------------------------------------

void XclTools::SkipSubStream( XclImpStream& rStrm )
{
    bool bLoop = true;
    while( bLoop && rStrm.StartNextRecord() )
    {
        sal_uInt16 nRecId = rStrm.GetRecId();
        bLoop = nRecId != EXC_ID_EOF;
        if( (nRecId == EXC_ID2_BOF) || (nRecId == EXC_ID3_BOF) || (nRecId == EXC_ID4_BOF) || (nRecId == EXC_ID5_BOF) )
            SkipSubStream( rStrm );
    }
}

// Basic macro names ----------------------------------------------------------

const OUString XclTools::maSbMacroPrefix( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.script:" ) );
const OUString XclTools::maSbMacroSuffix( RTL_CONSTASCII_USTRINGPARAM( "?language=Basic&location=document" ) );

OUString XclTools::GetSbMacroUrl( const String& rMacroName, SfxObjectShell* pDocShell )
{
    OSL_ENSURE( rMacroName.Len() > 0, "XclTools::GetSbMacroUrl - macro name is empty" );
    ::ooo::vba::VBAMacroResolvedInfo aMacroInfo = ::ooo::vba::resolveVBAMacro( pDocShell, rMacroName, false );
    if( aMacroInfo.IsResolved() )
        return ::ooo::vba::makeMacroURL( aMacroInfo.ResolvedMacro() );
    return OUString();
}

OUString XclTools::GetSbMacroUrl( const String& rModuleName, const String& rMacroName, SfxObjectShell* pDocShell )
{
    OSL_ENSURE( rModuleName.Len() > 0, "XclTools::GetSbMacroUrl - module name is empty" );
    OSL_ENSURE( rMacroName.Len() > 0, "XclTools::GetSbMacroUrl - macro name is empty" );
    return GetSbMacroUrl( rModuleName + OUString( sal_Unicode( '.' ) ) + rMacroName, pDocShell );
}

String XclTools::GetXclMacroName( const OUString& rSbMacroUrl )
{
    sal_Int32 nSbMacroUrlLen = rSbMacroUrl.getLength();
    sal_Int32 nMacroNameLen = nSbMacroUrlLen - maSbMacroPrefix.getLength() - maSbMacroSuffix.getLength();
    if( (nMacroNameLen > 0) && rSbMacroUrl.matchIgnoreAsciiCase( maSbMacroPrefix, 0 ) &&
            rSbMacroUrl.matchIgnoreAsciiCase( maSbMacroSuffix, nSbMacroUrlLen - maSbMacroSuffix.getLength() ) )
    {
        sal_Int32 nPrjDot = rSbMacroUrl.indexOf( '.', maSbMacroPrefix.getLength() ) + 1;
        return rSbMacroUrl.copy( nPrjDot, nSbMacroUrlLen - nPrjDot - maSbMacroSuffix.getLength() );
    }
    return String::EmptyString();
}

// read/write colors ----------------------------------------------------------

XclImpStream& operator>>( XclImpStream& rStrm, Color& rColor )
{
    sal_uInt8 nR, nG, nB, nD;
    rStrm >> nR >> nG >> nB >> nD;
    rColor.SetColor( RGB_COLORDATA( nR, nG, nB ) );
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const Color& rColor )
{
    return rStrm << rColor.GetRed() << rColor.GetGreen() << rColor.GetBlue() << sal_uInt8( 0 );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
