/*************************************************************************
 *
 *  $RCSfile: xltools.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:17:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif

#include <math.h>

#ifndef _SV_FONTCVT_HXX
#include <vcl/fontcvt.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DOCUNO_HXX
#include "docuno.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif


// GUID import/export =========================================================

XclGuid::XclGuid()
{
    memset( mpData, 0, 16 );
}

XclGuid::XclGuid(
        sal_uInt32 nData1, sal_uInt16 nData2, sal_uInt16 nData3,
        sal_uInt8 nData41, sal_uInt8 nData42, sal_uInt8 nData43, sal_uInt8 nData44,
        sal_uInt8 nData45, sal_uInt8 nData46, sal_uInt8 nData47, sal_uInt8 nData48 )
{
    // convert to little endian -> makes streaming easy
    LongToSVBT32( nData1, mpData );
    ShortToSVBT16( nData2, mpData + 4 );
    ShortToSVBT16( nData3, mpData + 6 );
    mpData[  8 ] = nData41;
    mpData[  9 ] = nData42;
    mpData[ 10 ] = nData43;
    mpData[ 11 ] = nData44;
    mpData[ 12 ] = nData45;
    mpData[ 13 ] = nData46;
    mpData[ 14 ] = nData47;
    mpData[ 15 ] = nData48;
}

bool operator==( const XclGuid& rCmp1, const XclGuid& rCmp2 )
{
    return memcmp( rCmp1.mpData, rCmp2.mpData, 16 ) == 0;
}

XclImpStream& operator>>( XclImpStream& rStrm, XclGuid& rGuid )
{
    rStrm.Read( rGuid.mpData, 16 );     // mpData always in little endian
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclGuid& rGuid )
{
    rStrm.Write( rGuid.mpData, 16 );    // mpData already in little endian
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
    double fVal;

    if( ::get_flag( nRKValue, EXC_RK_INTFLAG ) )
        fVal = nRKValue >> 2;
    else
    {
        // create a Little-Endian buffer
        SVBT64 pBuffer;
        pBuffer[ 0 ] = pBuffer[ 1 ] = pBuffer[ 2 ] = pBuffer[ 3 ] = 0;
        LongToSVBT32( nRKValue & EXC_RK_VALUEMASK, pBuffer + 4 );
        // create the double from buffer
        fVal = SVBT64ToDouble( pBuffer );
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


sal_Int32 XclTools::GetScRotation( sal_uInt16 nXclRot )
{
    if( nXclRot > 180 )
        return 27000;
    return static_cast< sal_Int32 >( 100 * ((nXclRot > 90) ? 450 - nXclRot : nXclRot) );
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


XclBoolError XclTools::ErrorToEnum( double& rfDblValue, sal_uInt8 bErrOrBool, sal_uInt8 nValue )
{
    XclBoolError eType;
    if( bErrOrBool )
    {
        // error value
        switch( nValue )
        {
            case 0x00:  eType = xlErrNull;      break;
            case 0x07:  eType = xlErrDiv0;      break;
            case 0x0F:  eType = xlErrValue;     break;
            case 0x17:  eType = xlErrRef;       break;
            case 0x1D:  eType = xlErrName;      break;
            case 0x24:  eType = xlErrNum;       break;
            case 0x2A:  eType = xlErrNA;        break;
            default:    eType = xlErrUnknown;
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


sal_Int32 XclTools::CalcX( ScDocument& rDoc, sal_uInt16 nTab, sal_uInt16 nCol, sal_uInt16 nColOffset, double fScale )
{
    return static_cast< sal_Int32 >( fScale *
        (rDoc.GetColOffset( nCol, nTab ) +
        ::std::min< sal_uInt16 >( nColOffset, 1024 ) / 1024.0 * rDoc.GetColWidth( nCol, nTab )) );
}

sal_Int32 XclTools::CalcY( ScDocument& rDoc, sal_uInt16 nTab, sal_uInt16 nRow, sal_uInt16 nRowOffset, double fScale )
{
    return static_cast< sal_Int32 >( fScale *
        (rDoc.GetRowOffset( nRow, nTab ) +
        ::std::min< sal_uInt16 >( nRowOffset, 256 ) / 256.0 * rDoc.GetRowHeight( nRow, nTab )) );
}


sal_uInt16 XclTools::GetTwipsFromInch( double fInches )
{
    return static_cast< sal_uInt16 >(
        ::std::min( ::std::max( (fInches * EXC_TWIPS_PER_INCH + 0.5), 0.0 ), 65535.0 ) );
}

sal_uInt16 XclTools::GetTwipsFromHmm( sal_Int32 nHmm )
{
    return GetTwipsFromInch( nHmm / 1000.0 / CM_PER_INCH );
}

double XclTools::GetInchFromTwips( sal_uInt16 nTwips )
{
    return static_cast< double >( nTwips ) / EXC_TWIPS_PER_INCH;
}

sal_Int32 XclTools::GetHmmFromTwips( sal_uInt16 nTwips )
{
    return static_cast< sal_Int32 >( GetInchFromTwips( nTwips ) * CM_PER_INCH * 1000 );
}

sal_uInt16 XclTools::GetScColumnWidth( sal_uInt16 nXclWidth, long nScCharWidth )
{
    double fScWidth = static_cast< double >( nXclWidth ) / 256.0 * nScCharWidth + 0.5;
    return static_cast< sal_uInt16 >( ::std::min( fScWidth, 65535.0 ) );
}

sal_uInt16 XclTools::GetXclColumnWidth( sal_uInt16 nScWidth, long nScCharWidth )
{
    double fXclWidth = static_cast< double >( nScWidth ) * 256.0 / nScCharWidth + 0.5;
    return static_cast< sal_uInt16 >( ::std::min( fXclWidth, 65535.0 ) );
}

// text encoding --------------------------------------------------------------

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
    inline explicit             XclCodePageEntry_CPPred( sal_uInt16 nCodePage ) : mnCodePage( nCodePage ) {}
    inline bool                 operator()( const XclCodePageEntry& rEntry ) const { return rEntry.mnCodePage == mnCodePage; }
    sal_uInt16                  mnCodePage;
};

struct XclCodePageEntry_TEPred
{
    inline explicit             XclCodePageEntry_TEPred( rtl_TextEncoding eTextEnc ) : meTextEnc( eTextEnc ) {}
    inline bool                 operator()( const XclCodePageEntry& rEntry ) const { return rEntry.meTextEnc == meTextEnc; }
    rtl_TextEncoding            meTextEnc;
};

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
    const XclCodePageEntry* pEntry = ::std::find_if( pCodePageTable, pCodePageTableEnd, XclCodePageEntry_TEPred( eTextEnc ) );
    if( pEntry == pCodePageTableEnd )
    {
        DBG_ERROR1( "XclTools::GetXclCodePage - unsupported text encoding: %d", eTextEnc );
        return 1252;
    }
    return pEntry->mnCodePage;
}


// font names -----------------------------------------------------------------

String XclTools::GetXclFontName( const String& rFontName )
{
    // #106246# substitute with MS fonts
    String aNewName( GetSubsFontName( rFontName, SUBSFONT_ONLYONE | SUBSFONT_MS ) );
    if( aNewName.Len() )
        return aNewName;
    return rFontName;
}

// built-in names -------------------------------------------------------------

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

String XclTools::GetBuiltInName( sal_Unicode nIndex )
{
    DBG_ASSERT( STATIC_TABLE_SIZE( ppcDefNames ) == EXC_BUILTIN_UNKNOWN,
        "XclTools::GetBuiltInName - built-in defined name list modified" );

    String aName( maDefNamePrefix );
    if( nIndex < STATIC_TABLE_SIZE( ppcDefNames ) )
        aName.AppendAscii( ppcDefNames[ nIndex ] );
    else
        aName.Append( String::CreateFromInt32( nIndex ) );
    return aName;
}

String XclTools::GetBuiltInName( sal_Unicode nIndex, sal_uInt16 nSheet )
{
    return GetBuiltInName( nIndex ).Append( '_' ).Append( String::CreateFromInt32( nSheet ) );
}

bool XclTools::IsBuiltInName( sal_uInt16& rnSheet, const String& rName, sal_Unicode nIndex )
{
    String aTestName( GetBuiltInName( nIndex ).Append( '_' ) );
    if( !rName.EqualsIgnoreCaseAscii( aTestName, 0, aTestName.Len() ) )
        return false;
    sal_Int32 nTab = rName.Copy( aTestName.Len() ).ToInt32();
    if( (nTab < 1) || (nTab > MAXTAB + 1) )
        return false;
    if( String::CreateFromInt32( nTab ).Len() != (rName.Len() - aTestName.Len()) )
        return false;
    rnSheet = static_cast< sal_uInt16 >( nTab );
    return true;
}


// built-in style names -------------------------------------------------------

const String XclTools::maStyleNamePrefix( RTL_CONSTASCII_USTRINGPARAM( "Excel_BuiltIn_" ) );

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

String XclTools::GetBuiltInStyleName( sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    String aStyleName;

    if( nStyleId == EXC_STYLE_NORMAL )  // "Normal" becomes "Default" style
        aStyleName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
    else if( nStyleId < STATIC_TABLE_SIZE( ppcStyleNames ) )
        aStyleName.Assign( maStyleNamePrefix ).AppendAscii( ppcStyleNames[ nStyleId ] );

    if( (nStyleId == EXC_STYLE_ROWLEVEL) || (nStyleId == EXC_STYLE_COLLEVEL) )
        aStyleName.Append( String::CreateFromInt32( nLevel + 1 ) );

    return aStyleName;
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
    xub_StrLen nPrefixLen = maStyleNamePrefix.Len();
    sal_uInt8 nFoundId;
    xub_StrLen nNextChar = 0;
    if( rStyleName.EqualsIgnoreCaseAscii( maStyleNamePrefix, 0, nPrefixLen ) )
    {
        String aShortName;
        for( sal_uInt8 nId = 0; nId < STATIC_TABLE_SIZE( ppcStyleNames ); ++nId )
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
    return false;
}

bool XclTools::GetBuiltInStyleId( sal_uInt8& rnStyleId, sal_uInt8& rnLevel, const String& rStyleName )
{
    sal_uInt8 nStyleId;
    xub_StrLen nNextChar;
    if( IsBuiltInStyleName( rStyleName, &nStyleId, &nNextChar ) )
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

sal_uInt16 XclTools::GetBuiltInXFIndex( sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    switch( nStyleId )
    {
        case EXC_STYLE_NORMAL:      return EXC_XF_DEFAULTSTYLE;
        case EXC_STYLE_ROWLEVEL:
            DBG_ASSERT( nLevel < EXC_STYLE_LEVELCOUNT, "XclTools::GetBuiltInXFIndex - invalid level" );
            return 1 + 2 * nLevel;
        case EXC_STYLE_COLLEVEL:
            DBG_ASSERT( nLevel < EXC_STYLE_LEVELCOUNT, "XclTools::GetBuiltInXFIndex - invalid level" );
            return 2 + 2 * nLevel;
        case EXC_STYLE_COMMA:               return 16;
        case EXC_STYLE_COMMA_0:             return 17;
        case EXC_STYLE_CURRENCY:            return 18;
        case EXC_STYLE_CURRENCY_0:          return 19;
        case EXC_STYLE_PERCENT:             return 20;
        case EXC_STYLE_HYPERLINK:           return 21;
        case EXC_STYLE_FOLLOWED_HYPERLINK:  return 22;
        default:
            DBG_ERRORFILE( "XclTools::GetBuiltInXFIndex - unknown style id" );
    }
    return EXC_XF_DEFAULTSTYLE;
}


// conditional formatting style names -----------------------------------------

const String XclTools::maCFStyleNamePrefix( RTL_CONSTASCII_USTRINGPARAM( "Excel_CondFormat_" ) );

String XclTools::GetCondFormatStyleName( sal_uInt16 nScTab, sal_Int32 nFormat, sal_uInt16 nCondition )
{
    return String( maCFStyleNamePrefix ).Append( String::CreateFromInt32( nScTab + 1 ) ).
                Append( '_' ).Append( String::CreateFromInt32( nFormat + 1 ) ).
                Append( '_' ).Append( String::CreateFromInt32( nCondition + 1 ) );
}

bool XclTools::IsCondFormatStyleName( const String& rStyleName, xub_StrLen* pnNextChar )
{
    xub_StrLen nPrefixLen = maCFStyleNamePrefix.Len();
    if( rStyleName.EqualsIgnoreCaseAscii( maCFStyleNamePrefix, 0, nPrefixLen ) )
    {
        if( pnNextChar ) *pnNextChar = nPrefixLen;
        return true;
    }
    return false;
}


// form control tag for linked range address ----------------------------------

const String XclTools::maCtrlCellLinkPrefix( RTL_CONSTASCII_USTRINGPARAM( "Excel_Cell_Link=" ) );
const String XclTools::maCtrlSrcRangePrefix( RTL_CONSTASCII_USTRINGPARAM( "Excel_Source_Data=" ) );

String XclTools::GetCtrlLinkTag( ScDocument& rDoc, const ScAddress* pCellLink, const ScRange* pSrcRange )
{
    String aTag;
    if( pCellLink )
    {
        String aCellTag;
        pCellLink->Format( aCellTag, SCA_VALID | SCA_TAB_3D, &rDoc );
        aCellTag.Insert( maCtrlCellLinkPrefix, 0 );
        ScGlobal::AddToken( aTag, aCellTag, ';' );
    }
    if( pSrcRange )
    {
        String aRangeTag;
        pSrcRange->Format( aRangeTag, SCA_VALID | SCA_TAB_3D, &rDoc );
        aRangeTag.Insert( maCtrlSrcRangePrefix, 0 );
        ScGlobal::AddToken( aTag, aRangeTag, ';' );
    }
    return aTag;
}

bool XclTools::GetCtrlCellLinkFromTag( ScAddress& rCellLink, ScDocument& rDoc, const String& rTag )
{
    xub_StrLen nPrefixLen = maCtrlCellLinkPrefix.Len();
    xub_StrLen nTokenCnt = rTag.GetTokenCount( ';' );
    for( xub_StrLen nToken = 0, nStringIx = 0; nToken < nTokenCnt; ++nToken )
    {
        String aToken( rTag.GetToken( 0, ';', nStringIx ) );
        if( aToken.EqualsIgnoreCaseAscii( maCtrlCellLinkPrefix, 0, nPrefixLen ) )
        {
            USHORT nRefFlags = rCellLink.Parse( aToken.Copy( nPrefixLen ), &rDoc );
            return (nRefFlags & SCA_VALID) != 0;
        }
    }
    return false;
}

bool XclTools::GetCtrlSrcRangeFromTag( ScRange& rSrcRange, ScDocument& rDoc, const String& rTag )
{
    xub_StrLen nPrefixLen = maCtrlSrcRangePrefix.Len();
    xub_StrLen nTokenCnt = rTag.GetTokenCount( ';' );
    for( xub_StrLen nToken = 0, nStringIx = 0; nToken < nTokenCnt; ++nToken )
    {
        String aToken( rTag.GetToken( 0, ';', nStringIx ) );
        if( aToken.EqualsIgnoreCaseAscii( maCtrlSrcRangePrefix, 0, nPrefixLen ) )
        {
            USHORT nRefFlags = rSrcRange.ParseAny( aToken.Copy( nPrefixLen ), &rDoc );
            return (nRefFlags & SCA_VALID) != 0;
        }
    }
    return false;
}


// read/write range lists -----------------------------------------------------

XclImpStream& operator>>( XclImpStream& rStrm, ScRangeList& rRanges )
{
    sal_uInt16 nTab = rStrm.GetRoot().GetScTab();
    sal_uInt16 nCount, nRow1, nRow2, nCol1, nCol2;
    rStrm >> nCount;
    for( ; nCount; --nCount )
    {
        rStrm >> nRow1 >> nRow2 >> nCol1 >> nCol2;
        rRanges.Append( ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab ) );
    }
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const ScRangeList& rRanges )
{
    sal_uInt16 nCount = static_cast< sal_uInt16 >( ::std::min( rRanges.Count(), 0xFFFFUL ) );
    rStrm << nCount;
    rStrm.SetSliceSize( 8 );

    for( sal_uInt16 nRange = 0; nRange < nCount; ++nRange )
    {
        const ScRange* pRange = rRanges.GetObject( nRange );
        DBG_ASSERT( pRange, "XclTools::WriteCellRangeList - missing range" );
        if( pRange )
            rStrm   << static_cast< sal_uInt16 >( pRange->aStart.Row() )
                    << static_cast< sal_uInt16 >( pRange->aEnd.Row() )
                    << static_cast< sal_uInt16 >( pRange->aStart.Col() )
                    << static_cast< sal_uInt16 >( pRange->aEnd.Col() );
        else
            // write dummy range to keep file format valid
            rStrm.WriteZeroBytes( 8 );
    }
    return rStrm;
}


// Rich-string formatting runs ================================================

XclImpStream& operator>>( XclImpStream& rStrm, XclFormatRun& rRun )
{
    return rStrm >> rRun.mnChar >> rRun.mnFontIx;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclFormatRun& rRun )
{
    return rStrm << rRun.mnChar << rRun.mnFontIx;
}


// ============================================================================

