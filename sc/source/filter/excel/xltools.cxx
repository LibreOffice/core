/*************************************************************************
 *
 *  $RCSfile: xltools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:38 $
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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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
#ifndef SC_ADDINCOL_HXX
#include "addincol.hxx"
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


sal_Int32 XclTools::CalcX( sal_uInt16 nTab, sal_uInt16 nCol, sal_uInt16 nColOffset, double fScale, ScDocument* pDoc )
{
    return static_cast< sal_Int32 >( fScale *
        (pDoc->GetColOffset( nCol, nTab ) +
        ::std::min< sal_uInt16 >( nColOffset, 1024 ) / 1024.0 * pDoc->GetColWidth( nCol, nTab )) );
}

sal_Int32 XclTools::CalcY( sal_uInt16 nTab, sal_uInt16 nRow, sal_uInt16 nRowOffset, double fScale, ScDocument* pDoc )
{
    return static_cast< sal_Int32 >( fScale *
        (pDoc->GetRowOffset( nRow, nTab ) +
        ::std::min< sal_uInt16 >( nRowOffset, 256 ) / 256.0 * pDoc->GetRowHeight( nRow, nTab )) );
}


sal_uInt16 XclTools::GetTwipsFromInch( double fInches )
{
    return static_cast< sal_uInt16 >(
        ::std::min( ::std::max( (fInches * EXC_TWIPS_PER_INCH + 0.5), 0.0 ), 65535.0 ) );
}

double XclTools::GetInchFromTwips( sal_uInt16 nTwips )
{
    return static_cast< double >( nTwips ) / EXC_TWIPS_PER_INCH;
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


// built-in names -------------------------------------------------------------

static const sal_Char* ppNames[] =
{
    "BuiltIn_Consolidate_Area",
    "BuiltIn_Auto_Open",
    "BuiltIn_Auto_Close",
    "BuiltIn_Extract",
    "BuiltIn_Database",
    "BuiltIn_Criteria",
    "BuiltIn_Print_Area",
    "BuiltIn_Print_Titles",
    "BuiltIn_Recorder",
    "BuiltIn_Data_Form",
    "BuiltIn_Auto_Activate",
    "BuiltIn_Auto_Deactivate",
    "BuiltIn_SheetTitle",
    "BuiltIn_AutoFilter",
    "BuiltIn_UNKNOWN"
};

const sal_Char* XclTools::GetBuiltInName( sal_Unicode nIndex )
{
    DBG_ASSERT( STATIC_TABLE_SIZE( ppNames ) == EXC_BUILTIN_UNKNOWN + 1,
        "XclTools::GetBuiltInName - Built-in name list modified" );
    if(  (nIndex < 0) || (nIndex > EXC_BUILTIN_UNKNOWN) )
        nIndex = EXC_BUILTIN_UNKNOWN;
    return ppNames[ nIndex ];
}

void XclTools::GetBuiltInName( String& rName, sal_Unicode nIndex, sal_uInt16 nSheet )
{
    rName.AssignAscii( GetBuiltInName( nIndex ) );
    rName.AppendAscii( "___" );
    rName += String::CreateFromInt32( nSheet );
}

bool XclTools::IsBuiltInName( sal_uInt16& rnSheet, const String& rName, sal_Unicode nIndex )
{
    String aBuiltIn;
    aBuiltIn.AssignAscii( GetBuiltInName( nIndex ) );
    aBuiltIn.AppendAscii( "___" );
    if( !rName.EqualsIgnoreCaseAscii( aBuiltIn, 0, aBuiltIn.Len() ) )
        return false;
    sal_Int32 nTab = rName.Copy( aBuiltIn.Len() ).ToInt32();
    if( (nTab < 1) || (nTab > MAXTAB + 1) )
        return false;
    if( String::CreateFromInt32( nTab ).Len() != (rName.Len() - aBuiltIn.Len()) )
        return false;
    rnSheet = static_cast< sal_uInt16 >( nTab );
    return true;
}


// Languages ------------------------------------------------------------------

/** Table entry for Excel country -> Calc language conversion. */
struct XclLanguageEntry
{
    sal_uInt16                  mnXclCountry;   /// Excel country ID.
    LanguageType                meLanguage;     /// Corresponding language.
};

/** Streact weak ordering function object for ::std::lower_bound(). */
struct XclLanguageEntrySWO
{
    inline bool                 operator()( const XclLanguageEntry& rEntry, sal_uInt16 nXclCountry )
                                    { return rEntry.mnXclCountry < nXclCountry; }
};

/** Table with Excel<->Calc languages. Must be sorted by Excel country ID! */
static const XclLanguageEntry pLanguages[] =
{
    {   1,  LANGUAGE_ENGLISH_US             },
    {   2,  LANGUAGE_ENGLISH_CAN            },
    {   3,  LANGUAGE_SPANISH                },
    {  31,  LANGUAGE_DUTCH                  },
    {  32,  LANGUAGE_DUTCH_BELGIAN          },
    {  33,  LANGUAGE_FRENCH                 },
    {  34,  LANGUAGE_SPANISH                },
    {  39,  LANGUAGE_ITALIAN                },
    {  41,  LANGUAGE_GERMAN_SWISS           },
    {  43,  LANGUAGE_GERMAN_AUSTRIAN        },
    {  44,  LANGUAGE_ENGLISH_UK             },
    {  45,  LANGUAGE_DANISH                 },
    {  46,  LANGUAGE_SWEDISH                },
    {  47,  LANGUAGE_NORWEGIAN              },
    {  49,  LANGUAGE_GERMAN                 },
    {  52,  LANGUAGE_SPANISH_MEXICAN        },
    {  55,  LANGUAGE_PORTUGUESE_BRAZILIAN   },
    {  61,  LANGUAGE_ENGLISH_AUS            },
    {  64,  LANGUAGE_ENGLISH_NZ             },
    {  81,  LANGUAGE_JAPANESE               },
    {  82,  LANGUAGE_KOREAN                 },
    { 351,  LANGUAGE_PORTUGUESE             },
    { 354,  LANGUAGE_ICELANDIC              },
    { 358,  LANGUAGE_SWEDISH_FINLAND        },
    { 785,  LANGUAGE_ARABIC_SAUDI_ARABIA    },
    { 886,  LANGUAGE_CHINESE                },
    { 972,  LANGUAGE_HEBREW                 }
};

bool XclTools::GetScLanguage( LanguageType& reScLang, sal_uInt16 nXclCountry )
{
    const XclLanguageEntry* pLast = pLanguages + STATIC_TABLE_SIZE( pLanguages );
    const XclLanguageEntry* pResult = ::std::lower_bound( pLanguages, pLast, nXclCountry, XclLanguageEntrySWO() );
    if( (pResult != pLast) && (pResult->mnXclCountry == nXclCountry) )
    {
        reScLang = pResult->meLanguage;
        return true;
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


// Add-in function names ======================================================

XclAddInNameTranslator::XclAddInNameTranslator() :
        mrAddInColl( *ScGlobal::GetAddInCollection() ),
        meLanguage( Application::GetSettings().GetUILanguage() )
{
}

String XclAddInNameTranslator::GetScName( const String& rXclName )
{
    String aScName;
    if( mrAddInColl.GetCalcName( rXclName, aScName ) )
        return aScName;
    return rXclName;
}


String XclAddInNameTranslator::GetXclName( const String& rScName )
{
    String aXclName;
    if( mrAddInColl.GetExcelName( rScName, meLanguage, aXclName ) )
        return aXclName;
    return rScName;
}


// ============================================================================

