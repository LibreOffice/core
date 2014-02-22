/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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




XclGuid::XclGuid()
{
    ::std::fill( mpnData, STATIC_ARRAY_END( mpnData ), 0 );
}

XclGuid::XclGuid(
        sal_uInt32 nData1, sal_uInt16 nData2, sal_uInt16 nData3,
        sal_uInt8 nData41, sal_uInt8 nData42, sal_uInt8 nData43, sal_uInt8 nData44,
        sal_uInt8 nData45, sal_uInt8 nData46, sal_uInt8 nData47, sal_uInt8 nData48 )
{
    
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
    return ::std::equal( rCmp1.mpnData, STATIC_ARRAY_END( rCmp1.mpnData ), rCmp2.mpnData );
}

bool operator<( const XclGuid& rCmp1, const XclGuid& rCmp2 )
{
    return ::std::lexicographical_compare(
        rCmp1.mpnData, STATIC_ARRAY_END( rCmp1.mpnData ),
        rCmp2.mpnData, STATIC_ARRAY_END( rCmp2.mpnData ) );
}

XclImpStream& operator>>( XclImpStream& rStrm, XclGuid& rGuid )
{
    rStrm.Read( rGuid.mpnData, 16 );     
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclGuid& rGuid )
{
    rStrm.Write( rGuid.mpnData, 16 );    
    return rStrm;
}





const XclGuid XclTools::maGuidStdLink(
    0x79EAC9D0, 0xBAF9, 0x11CE, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B );

const XclGuid XclTools::maGuidUrlMoniker(
    0x79EAC9E0, 0xBAF9, 0x11CE, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B );

const XclGuid XclTools::maGuidFileMoniker(
    0x00000303, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 );



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

    
    fFrac = modf( fValue, &fInt );
    if( (fFrac == 0.0) && (fInt >= -536870912.0) && (fInt <= 536870911.0) ) 
    {
        rnRKValue = static_cast< sal_Int32 >( fInt );
        rnRKValue <<= 2;
        rnRKValue |= EXC_RK_INT;
        return true;
    }

    
    fFrac = modf( fValue * 100.0, &fInt );
    if( (fFrac == 0.0) && (fInt >= -536870912.0) && (fInt <= 536870911.0) )
    {
        rnRKValue = static_cast< sal_Int32 >( fInt );
        rnRKValue <<= 2;
        rnRKValue |= EXC_RK_INT100;
        return true;
    }

    
    return false;
}

sal_Int32 XclTools::GetScRotation( sal_uInt16 nXclRot, sal_Int32 nRotForStacked )
{
    if( nXclRot == EXC_ROT_STACKED )
        return nRotForStacked;
    OSL_ENSURE( nXclRot <= 180, "XclTools::GetScRotation - illegal rotation angle" );
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
        default:    OSL_FAIL( "XclTools::GetXclRotFromOrient - unknown text orientation" );
    }
    return EXC_ROT_NONE;
}

sal_uInt8 XclTools::GetXclOrientFromRot( sal_uInt16 nXclRot )
{
    if( nXclRot == EXC_ROT_STACKED )
        return EXC_ORIENT_STACKED;
    OSL_ENSURE( nXclRot <= 180, "XclTools::GetXclOrientFromRot - unknown text rotation" );
    if( (45 < nXclRot) && (nXclRot <= 90) )
        return EXC_ORIENT_90CCW;
    if( (135 < nXclRot) && (nXclRot <= 180) )
        return EXC_ORIENT_90CW;
    return EXC_ORIENT_NONE;
}

sal_uInt8 XclTools::GetXclErrorCode( sal_uInt16 nScError )
{
    using namespace ScErrorCodes;
    switch( nScError )
    {
        case errIllegalArgument:        return EXC_ERR_VALUE;
        case errIllegalFPOperation:     return EXC_ERR_NUM;     
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

sal_uInt16 XclTools::GetScErrorCode( sal_uInt8 nXclError )
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
        default:            OSL_FAIL( "XclTools::GetScErrorCode - unknown error code" );
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

sal_uInt16 XclTools::GetScColumnWidth( sal_uInt16 nXclWidth, long nScCharWidth )
{
    double fScWidth = static_cast< double >( nXclWidth ) / 256.0 * nScCharWidth + 0.5;
    return limit_cast< sal_uInt16 >( fScWidth );
}

sal_uInt16 XclTools::GetXclColumnWidth( sal_uInt16 nScWidth, long nScCharWidth )
{
    double fXclWidth = static_cast< double >( nScWidth ) * 256.0 / nScCharWidth + 0.5;
    return limit_cast< sal_uInt16 >( fXclWidth );
}

double XclTools::GetXclDefColWidthCorrection( long nXclDefFontHeight )
{
    return 40960.0 / ::std::max( nXclDefFontHeight - 15L, 60L ) + 50.0;
}



Color XclTools::GetPatternColor( const Color& rPattColor, const Color& rBackColor, sal_uInt16 nXclPattern )
{
    
    
    static const sal_uInt8 pnRatioTable[] =
    {
        0x80, 0x00, 0x40, 0x20, 0x60, 0x40, 0x40, 0x40,     
        0x40, 0x40, 0x20, 0x60, 0x60, 0x60, 0x60, 0x48,     
        0x50, 0x70, 0x78                                    
    };
    return (nXclPattern < SAL_N_ELEMENTS( pnRatioTable )) ?
        ScfTools::GetMixedColor( rPattColor, rBackColor, pnRatioTable[ nXclPattern ] ) : rPattColor;
}



namespace {

const struct XclCodePageEntry
{
    sal_uInt16                  mnCodePage;
    rtl_TextEncoding            meTextEnc;
}
pCodePageTable[] =
{
    {     437,  RTL_TEXTENCODING_IBM_437        },  

    {     737,  RTL_TEXTENCODING_IBM_737        },  
    {     775,  RTL_TEXTENCODING_IBM_775        },  
    {     850,  RTL_TEXTENCODING_IBM_850        },  
    {     852,  RTL_TEXTENCODING_IBM_852        },  
    {     855,  RTL_TEXTENCODING_IBM_855        },  
    {     857,  RTL_TEXTENCODING_IBM_857        },  

    {     860,  RTL_TEXTENCODING_IBM_860        },  
    {     861,  RTL_TEXTENCODING_IBM_861        },  
    {     862,  RTL_TEXTENCODING_IBM_862        },  
    {     863,  RTL_TEXTENCODING_IBM_863        },  
    {     864,  RTL_TEXTENCODING_IBM_864        },  
    {     865,  RTL_TEXTENCODING_IBM_865        },  
    {     866,  RTL_TEXTENCODING_IBM_866        },  
    {     869,  RTL_TEXTENCODING_IBM_869        },  
    {     874,  RTL_TEXTENCODING_MS_874         },  
    {     932,  RTL_TEXTENCODING_MS_932         },  
    {     936,  RTL_TEXTENCODING_MS_936         },  
    {     949,  RTL_TEXTENCODING_MS_949         },  
    {     950,  RTL_TEXTENCODING_MS_950         },  
    {    1200,  RTL_TEXTENCODING_DONTKNOW       },  
    {    1250,  RTL_TEXTENCODING_MS_1250        },  
    {    1251,  RTL_TEXTENCODING_MS_1251        },  
    {    1252,  RTL_TEXTENCODING_MS_1252        },  
    {    1253,  RTL_TEXTENCODING_MS_1253        },  
    {    1254,  RTL_TEXTENCODING_MS_1254        },  
    {    1255,  RTL_TEXTENCODING_MS_1255        },  
    {    1256,  RTL_TEXTENCODING_MS_1256        },  
    {    1257,  RTL_TEXTENCODING_MS_1257        },  
    {    1258,  RTL_TEXTENCODING_MS_1258        },  
    {    1361,  RTL_TEXTENCODING_MS_1361        },  
    {   10000,  RTL_TEXTENCODING_APPLE_ROMAN    },  
    {   32768,  RTL_TEXTENCODING_APPLE_ROMAN    },  
    {   32769,  RTL_TEXTENCODING_MS_1252        }   
};
const XclCodePageEntry* const pCodePageTableEnd = STATIC_ARRAY_END( pCodePageTable );

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

} 

rtl_TextEncoding XclTools::GetTextEncoding( sal_uInt16 nCodePage )
{
    const XclCodePageEntry* pEntry = ::std::find_if( pCodePageTable, pCodePageTableEnd, XclCodePageEntry_CPPred( nCodePage ) );
    if( pEntry == pCodePageTableEnd )
    {
        OSL_TRACE( "XclTools::GetTextEncoding - unknown code page: 0x%04hX (%d)", nCodePage, nCodePage );
        return RTL_TEXTENCODING_DONTKNOW;
    }
    return pEntry->meTextEnc;
}

sal_uInt16 XclTools::GetXclCodePage( rtl_TextEncoding eTextEnc )
{
    if( eTextEnc == RTL_TEXTENCODING_UNICODE )
        return 1200;    

    const XclCodePageEntry* pEntry = ::std::find_if( pCodePageTable, pCodePageTableEnd, XclCodePageEntry_TEPred( eTextEnc ) );
    if( pEntry == pCodePageTableEnd )
    {
        OSL_TRACE( "XclTools::GetXclCodePage - unsupported text encoding: %d", eTextEnc );
        return 1252;
    }
    return pEntry->mnCodePage;
}



OUString XclTools::GetXclFontName( const OUString& rFontName )
{
    
    OUString aNewName = GetSubsFontName(rFontName, SUBSFONT_ONLYONE | SUBSFONT_MS);
    return aNewName.isEmpty() ? rFontName : aNewName;
}



const OUString XclTools::maDefNamePrefix( "Excel_BuiltIn_" );

const OUString XclTools::maDefNamePrefixXml ( "_xlnm." );

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

OUString XclTools::GetXclBuiltInDefName( sal_Unicode cBuiltIn )
{
    OSL_ENSURE( SAL_N_ELEMENTS( ppcDefNames ) == EXC_BUILTIN_UNKNOWN,
        "XclTools::GetXclBuiltInDefName - built-in defined name list modified" );

    if( cBuiltIn < SAL_N_ELEMENTS( ppcDefNames ) )
        return OUString::createFromAscii(ppcDefNames[cBuiltIn]);
    else
        return OUString::number(cBuiltIn);
}

OUString XclTools::GetBuiltInDefName( sal_Unicode cBuiltIn )
{
    OUStringBuffer aBuf(maDefNamePrefix);
    aBuf.append(GetXclBuiltInDefName(cBuiltIn));
    return aBuf.makeStringAndClear();
}

OUString XclTools::GetBuiltInDefNameXml( sal_Unicode cBuiltIn )
{
    OUStringBuffer aBuf(maDefNamePrefixXml);
    aBuf.append(GetXclBuiltInDefName(cBuiltIn));
    return aBuf.makeStringAndClear();
}

sal_Unicode XclTools::GetBuiltInDefNameIndex( const OUString& rDefName )
{
    sal_Int32 nPrefixLen = maDefNamePrefix.getLength();
    if( rDefName.startsWithIgnoreAsciiCase( maDefNamePrefix ) )
    {
        for( sal_Unicode cBuiltIn = 0; cBuiltIn < EXC_BUILTIN_UNKNOWN; ++cBuiltIn )
        {
            OUString aBuiltInName(GetXclBuiltInDefName(cBuiltIn));
            sal_Int32 nBuiltInLen = aBuiltInName.getLength();
            if( rDefName.matchIgnoreAsciiCase( aBuiltInName, nPrefixLen ) )
            {
                
                sal_Int32 nNextCharPos = nPrefixLen + nBuiltInLen;
                sal_Unicode cNextChar = (rDefName.getLength() > nNextCharPos) ? rDefName[nNextCharPos] : '\0';
                if( (cNextChar == '\0') || (cNextChar == ' ') || (cNextChar == '_') )
                    return cBuiltIn;
            }
        }
    }
    return EXC_BUILTIN_UNKNOWN;
}



const OUString XclTools::maStyleNamePrefix1( "Excel_BuiltIn_" );
const OUString XclTools::maStyleNamePrefix2( "Excel Built-in " );

static const sal_Char* const ppcStyleNames[] =
{
    "",                 
    "RowLevel_",        
    "ColumnLevel_",     
    "Comma",
    "Currency",
    "Percent",
    "Comma_0",
    "Currency_0",
    "Hyperlink",
    "Followed_Hyperlink"
};

OUString XclTools::GetBuiltInStyleName( sal_uInt8 nStyleId, const OUString& rName, sal_uInt8 nLevel )
{
    OUString aStyleName;

    if( nStyleId == EXC_STYLE_NORMAL )  
    {
        aStyleName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
    }
    else
    {
        OUStringBuffer aBuf(maStyleNamePrefix1);
        if( nStyleId < SAL_N_ELEMENTS( ppcStyleNames ) )
            aBuf.appendAscii(ppcStyleNames[nStyleId]);
        else if (!rName.isEmpty())
            aBuf.append(rName);
        else
            aBuf.append(static_cast<sal_Int32>(nStyleId));

        if( (nStyleId == EXC_STYLE_ROWLEVEL) || (nStyleId == EXC_STYLE_COLLEVEL) )
            aBuf.append(static_cast<sal_Int32>(nLevel+1));

        aStyleName = aBuf.makeStringAndClear();
    }

    return aStyleName;
}

bool XclTools::IsBuiltInStyleName( const OUString& rStyleName, sal_uInt8* pnStyleId, sal_Int32* pnNextChar )
{
    
    if (rStyleName.equals(ScGlobal::GetRscString(STR_STYLENAME_STANDARD)))
    {
        if( pnStyleId ) *pnStyleId = EXC_STYLE_NORMAL;
        if( pnNextChar ) *pnNextChar = rStyleName.getLength();
        return true;
    }

    
    sal_uInt8 nFoundId = 0;
    sal_Int32 nNextChar = 0;

    sal_Int32 nPrefixLen = 0;
    if( rStyleName.startsWithIgnoreAsciiCase( maStyleNamePrefix1 ) )
        nPrefixLen = maStyleNamePrefix1.getLength();
    else if( rStyleName.startsWithIgnoreAsciiCase( maStyleNamePrefix2 ) )
        nPrefixLen = maStyleNamePrefix2.getLength();
    if( nPrefixLen > 0 )
    {
        for( sal_uInt8 nId = 0; nId < SAL_N_ELEMENTS( ppcStyleNames ); ++nId )
        {
            if( nId != EXC_STYLE_NORMAL )
            {
                OUString aShortName = OUString::createFromAscii(ppcStyleNames[nId]);
                if( rStyleName.matchIgnoreAsciiCase( aShortName, nPrefixLen ) &&
                    (nNextChar < nPrefixLen + aShortName.getLength()))
                {
                    nFoundId = nId;
                    nNextChar = nPrefixLen + aShortName.getLength();
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
    return nPrefixLen > 0;  
}

bool XclTools::GetBuiltInStyleId( sal_uInt8& rnStyleId, sal_uInt8& rnLevel, const OUString& rStyleName )
{
    sal_uInt8 nStyleId;
    sal_Int32 nNextChar;
    if( IsBuiltInStyleName( rStyleName, &nStyleId, &nNextChar ) && (nStyleId != EXC_STYLE_USERDEF) )
    {
        if( (nStyleId == EXC_STYLE_ROWLEVEL) || (nStyleId == EXC_STYLE_COLLEVEL) )
        {
            OUString aLevel = rStyleName.copy(nNextChar);
            sal_Int32 nLevel = aLevel.toInt32();
            if (OUString::number(nLevel) == aLevel && nLevel > 0 && nLevel <= EXC_STYLE_LEVELCOUNT)
            {
                rnStyleId = nStyleId;
                rnLevel = static_cast< sal_uInt8 >( nLevel - 1 );
                return true;
            }
        }
        else if( rStyleName.getLength() == nNextChar )
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



const OUString XclTools::maCFStyleNamePrefix1( "Excel_CondFormat_" );
const OUString XclTools::maCFStyleNamePrefix2( "ConditionalStyle_" );

OUString XclTools::GetCondFormatStyleName( SCTAB nScTab, sal_Int32 nFormat, sal_uInt16 nCondition )
{
    OUStringBuffer aBuf(maCFStyleNamePrefix1);
    aBuf.append(static_cast<sal_Int32>(nScTab+1));
    aBuf.append('_');
    aBuf.append(static_cast<sal_Int32>(nFormat+1));
    aBuf.append('_');
    aBuf.append(static_cast<sal_Int32>(nCondition+1));
    return aBuf.makeStringAndClear();
}

bool XclTools::IsCondFormatStyleName( const OUString& rStyleName )
{
    if( rStyleName.startsWithIgnoreAsciiCase( maCFStyleNamePrefix1 ) )
        return true;

    if( rStyleName.startsWithIgnoreAsciiCase( maCFStyleNamePrefix2 ) )
        return true;

    return false;
}



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



const OUString XclTools::maSbMacroPrefix( "vnd.sun.star.script:" );
const OUString XclTools::maSbMacroSuffix( "?language=Basic&location=document" );

OUString XclTools::GetSbMacroUrl( const OUString& rMacroName, SfxObjectShell* pDocShell )
{
    OSL_ENSURE( !rMacroName.isEmpty(), "XclTools::GetSbMacroUrl - macro name is empty" );
    ::ooo::vba::MacroResolvedInfo aMacroInfo = ::ooo::vba::resolveVBAMacro( pDocShell, rMacroName, false );
    if( aMacroInfo.mbFound )
        return ::ooo::vba::makeMacroURL( aMacroInfo.msResolvedMacro );
    return OUString();
}

OUString XclTools::GetXclMacroName( const OUString& rSbMacroUrl )
{
    sal_Int32 nSbMacroUrlLen = rSbMacroUrl.getLength();
    sal_Int32 nMacroNameLen = nSbMacroUrlLen - maSbMacroPrefix.getLength() - maSbMacroSuffix.getLength();
    if( (nMacroNameLen > 0) && rSbMacroUrl.matchIgnoreAsciiCase( maSbMacroPrefix, 0 ) &&
            rSbMacroUrl.matchIgnoreAsciiCase( maSbMacroSuffix, nSbMacroUrlLen - maSbMacroSuffix.getLength() ) )
    {
        sal_Int32 nPrjDot = rSbMacroUrl.indexOf( '.', maSbMacroPrefix.getLength() ) + 1;
        return rSbMacroUrl.copy( nPrjDot, nSbMacroUrlLen - nPrjDot - maSbMacroSuffix.getLength() );
    }
    return OUString();
}



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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
