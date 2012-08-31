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

#include "ftools.hxx"
#include <rtl/strbuf.hxx>
#include <tools/color.hxx>
#include <unotools/charclass.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/poolitem.hxx>
#include <sot/storage.hxx>

#include <math.h>
#include "global.hxx"
#include "document.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "compiler.hxx"
#include "orcusfiltersimpl.hxx"

#include <stdio.h>

// ============================================================================
// ScFilterTools::ReadLongDouble()

#ifdef _MSC_VER
#if _MSC_VER <= 800
#undef __SIMPLE_FUNC
#define __SIMPLE_FUNC
#endif
#endif

double ScfTools::ReadLongDouble( SvStream& rStrm )

#ifdef __SIMPLE_FUNC                // for <=VC 1.5
{
    long double fRet;
    rStrm.Read( &fRet, 10 );
    return static_cast< double >( fRet );
}
#undef __SIMPLE_FUNC

#else                               // detailed for all others
{

/*
" M a p p i n g - G u i d e " 10-Byte Intel

77777777 77666666 66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000   x10
98765432 10987654 32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210   Bit-# total
9      9 8      8 7      7 6      6 5      5 4      4 3      3 2      2 1      1 0      0   Byte-#
76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210   Bit-# in Byte
SEEEEEEE EEEEEEEE IMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM   Group
01111110 00000000 06665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000       x10
14321098 76543210 02109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210   Bit in Group
*/

    register long double lfDouble = 0.0;
    register long double lfFakt = 256.0;
    sal_uInt8 pDouble10[ 10 ];

    rStrm.Read( pDouble10, 10 );            // Intel-10 in pDouble10

    lfDouble  = static_cast< long double >( pDouble10[ 7 ] );   // Byte 7
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 6 ] );   // Byte 6
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 5 ] );   // Byte 5
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 4 ] );   // Byte 4
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 3 ] );   // Byte 3
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 2 ] );   // Byte 2
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 1 ] );   // Byte 1
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 0 ] );   // Byte 0

    //  For value 0.0 all bits are zero; pow(2.0,-16446) does not work with CSet compilers
    if( lfDouble != 0.0 )
    {
        // exponent
        register sal_Int32 nExp;
        nExp = pDouble10[ 9 ] & 0x7F;
        nExp <<= 8;
        nExp += pDouble10[ 8 ];
        nExp -= 16446;

        lfDouble *= pow( 2.0, static_cast< double >( nExp ) );
    }

    // sign
    if( pDouble10[ 9 ] & 0x80 )
        lfDouble *= static_cast< long double >( -1.0 );

    return static_cast< double >( lfDouble );
}
#endif

// *** common methods *** -----------------------------------------------------

rtl_TextEncoding ScfTools::GetSystemTextEncoding()
{
    return osl_getThreadTextEncoding();
}

String ScfTools::GetHexStr( sal_uInt16 nValue )
{
    const sal_Char pHex[] = "0123456789ABCDEF";
    String aStr;

    aStr += pHex[ nValue >> 12 ];
    aStr += pHex[ (nValue >> 8) & 0x000F ];
    aStr += pHex[ (nValue >> 4) & 0x000F ];
    aStr += pHex[ nValue & 0x000F ];
    return aStr;
}

sal_uInt8 ScfTools::GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans )
{
    sal_Int32 nTemp = ((static_cast< sal_Int32 >( nBack ) - nFore) * nTrans) / 0x80 + nFore;
    return static_cast< sal_uInt8 >( nTemp );
}

Color ScfTools::GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt8 nTrans )
{
    return Color(
        GetMixedColorComp( rFore.GetRed(), rBack.GetRed(), nTrans ),
        GetMixedColorComp( rFore.GetGreen(), rBack.GetGreen(), nTrans ),
        GetMixedColorComp( rFore.GetBlue(), rBack.GetBlue(), nTrans ) );
}

// *** conversion of names *** ------------------------------------------------

/* XXX As in sc/source/core/tool/rangenam.cxx ScRangeData::IsValidName() */

void ScfTools::ConvertToScDefinedName( String& rName )
{
    //fdo#37872: we don't allow points in range names any more
    rName.SearchAndReplaceAll(static_cast<sal_Unicode>('.'),
        static_cast<sal_Unicode>('_'));
    xub_StrLen nLen = rName.Len();
    if( nLen && !ScCompiler::IsCharFlagAllConventions( rName, 0, SC_COMPILER_C_CHAR_NAME ) )
        rName.SetChar( 0, '_' );
    for( xub_StrLen nPos = 1; nPos < nLen; ++nPos )
        if( !ScCompiler::IsCharFlagAllConventions( rName, nPos, SC_COMPILER_C_NAME ) )
            rName.SetChar( nPos, '_' );
}

// *** streams and storages *** -----------------------------------------------

SotStorageRef ScfTools::OpenStorageRead( SotStorageRef xStrg, const String& rStrgName )
{
    SotStorageRef xSubStrg;
    if( xStrg.Is() && xStrg->IsContained( rStrgName ) )
        xSubStrg = xStrg->OpenSotStorage( rStrgName, STREAM_STD_READ );
    return xSubStrg;
}

SotStorageRef ScfTools::OpenStorageWrite( SotStorageRef xStrg, const String& rStrgName )
{
    SotStorageRef xSubStrg;
    if( xStrg.Is() )
        xSubStrg = xStrg->OpenSotStorage( rStrgName, STREAM_STD_WRITE );
    return xSubStrg;
}

SotStorageStreamRef ScfTools::OpenStorageStreamRead( SotStorageRef xStrg, const String& rStrmName )
{
    SotStorageStreamRef xStrm;
    if( xStrg.Is() && xStrg->IsContained( rStrmName ) && xStrg->IsStream( rStrmName ) )
        xStrm = xStrg->OpenSotStream( rStrmName, STREAM_STD_READ );
    return xStrm;
}

SotStorageStreamRef ScfTools::OpenStorageStreamWrite( SotStorageRef xStrg, const String& rStrmName )
{
    OSL_ENSURE( !xStrg || !xStrg->IsContained( rStrmName ), "ScfTools::OpenStorageStreamWrite - stream exists already" );
    SotStorageStreamRef xStrm;
    if( xStrg.Is() )
        xStrm = xStrg->OpenSotStream( rStrmName, STREAM_STD_WRITE | STREAM_TRUNC );
    return xStrm;
}

// *** item handling *** ------------------------------------------------------

bool ScfTools::CheckItem( const SfxItemSet& rItemSet, sal_uInt16 nWhichId, bool bDeep )
{
    return rItemSet.GetItemState( nWhichId, bDeep ) == SFX_ITEM_SET;
}

bool ScfTools::CheckItems( const SfxItemSet& rItemSet, const sal_uInt16* pnWhichIds, bool bDeep )
{
    OSL_ENSURE( pnWhichIds, "ScfTools::CheckItems - no which id list" );
    for( const sal_uInt16* pnWhichId = pnWhichIds; *pnWhichId != 0; ++pnWhichId )
        if( CheckItem( rItemSet, *pnWhichId, bDeep ) )
            return true;
    return false;
}

void ScfTools::PutItem( SfxItemSet& rItemSet, const SfxPoolItem& rItem, sal_uInt16 nWhichId, bool bSkipPoolDef )
{
    if( !bSkipPoolDef || (rItem != rItemSet.GetPool()->GetDefaultItem( nWhichId )) )
        rItemSet.Put( rItem, nWhichId );
}

void ScfTools::PutItem( SfxItemSet& rItemSet, const SfxPoolItem& rItem, bool bSkipPoolDef )
{
    PutItem( rItemSet, rItem, rItem.Which(), bSkipPoolDef );
}

// *** style sheet handling *** -----------------------------------------------

namespace {

ScStyleSheet& lclMakeStyleSheet( ScStyleSheetPool& rPool, const String& rStyleName, SfxStyleFamily eFamily, bool bForceName )
{
    // find an unused name
    String aNewName( rStyleName );
    sal_Int32 nIndex = 0;
    SfxStyleSheetBase* pOldStyleSheet = 0;
    while( SfxStyleSheetBase* pStyleSheet = rPool.Find( aNewName, eFamily ) )
    {
        if( !pOldStyleSheet )
            pOldStyleSheet = pStyleSheet;
        aNewName.Assign( rStyleName ).Append( ' ' ).Append( String::CreateFromInt32( ++nIndex ) );
    }

    // rename existing style
    if( pOldStyleSheet && bForceName )
    {
        pOldStyleSheet->SetName( aNewName );
        aNewName = rStyleName;
    }

    // create new style sheet
    return static_cast< ScStyleSheet& >( rPool.Make( aNewName, eFamily, SFXSTYLEBIT_USERDEF ) );
}

} // namespace

ScStyleSheet& ScfTools::MakeCellStyleSheet( ScStyleSheetPool& rPool, const String& rStyleName, bool bForceName )
{
    return lclMakeStyleSheet( rPool, rStyleName, SFX_STYLE_FAMILY_PARA, bForceName );
}

ScStyleSheet& ScfTools::MakePageStyleSheet( ScStyleSheetPool& rPool, const String& rStyleName, bool bForceName )
{
    return lclMakeStyleSheet( rPool, rStyleName, SFX_STYLE_FAMILY_PAGE, bForceName );
}

// *** byte string import operations *** --------------------------------------

rtl::OString ScfTools::read_zeroTerminated_uInt8s_ToOString(SvStream& rStrm, sal_Int32& rnBytesLeft)
{
    rtl::OString aRet(::read_zeroTerminated_uInt8s_ToOString(rStrm));
    rnBytesLeft -= aRet.getLength(); //we read this number of bytes anyway
    if (rStrm.good()) //if the stream is happy we read the null terminator as well
        --rnBytesLeft;
    return aRet;
}

void ScfTools::AppendCString( SvStream& rStrm, String& rString, rtl_TextEncoding eTextEnc )
{
    rString += ::read_zeroTerminated_uInt8s_ToOUString(rStrm, eTextEnc);
}

// *** HTML table names <-> named range names *** -----------------------------

const String& ScfTools::GetHTMLDocName()
{
    static const String saHTMLDoc( RTL_CONSTASCII_USTRINGPARAM( "HTML_all" ) );
    return saHTMLDoc;
}

const String& ScfTools::GetHTMLTablesName()
{
    static const String saHTMLTables( RTL_CONSTASCII_USTRINGPARAM( "HTML_tables" ) );
    return saHTMLTables;
}

const String& ScfTools::GetHTMLIndexPrefix()
{
    static const String saHTMLIndexPrefix( RTL_CONSTASCII_USTRINGPARAM( "HTML_" ) );
    return saHTMLIndexPrefix;

}

const String& ScfTools::GetHTMLNamePrefix()
{
    static const String saHTMLNamePrefix( RTL_CONSTASCII_USTRINGPARAM( "HTML__" ) );
    return saHTMLNamePrefix;
}

String ScfTools::GetNameFromHTMLIndex( sal_uInt32 nIndex )
{
    String aName( GetHTMLIndexPrefix() );
    aName += String::CreateFromInt32( static_cast< sal_Int32 >( nIndex ) );
    return aName;
}

String ScfTools::GetNameFromHTMLName( const String& rTabName )
{
    String aName( GetHTMLNamePrefix() );
    aName += rTabName;
    return aName;
}

bool ScfTools::IsHTMLDocName( const String& rSource )
{
    return rSource.EqualsIgnoreCaseAscii( GetHTMLDocName() );
}

bool ScfTools::IsHTMLTablesName( const String& rSource )
{
    return rSource.EqualsIgnoreCaseAscii( GetHTMLTablesName() );
}

bool ScfTools::GetHTMLNameFromName( const String& rSource, String& rName )
{
    rName.Erase();
    if( rSource.EqualsIgnoreCaseAscii( GetHTMLNamePrefix(), 0, GetHTMLNamePrefix().Len() ) )
    {
        rName = rSource.Copy( GetHTMLNamePrefix().Len() );
        ScGlobal::AddQuotes( rName, '"', false );
    }
    else if( rSource.EqualsIgnoreCaseAscii( GetHTMLIndexPrefix(), 0, GetHTMLIndexPrefix().Len() ) )
    {
        String aIndex( rSource.Copy( GetHTMLIndexPrefix().Len() ) );
        if( CharClass::isAsciiNumeric( aIndex ) && (aIndex.ToInt32() > 0) )
            rName = aIndex;
    }
    return rName.Len() > 0;
}

ScFormatFilterPluginImpl::ScFormatFilterPluginImpl() {}
ScFormatFilterPluginImpl::~ScFormatFilterPluginImpl() {}

ScOrcusFilters* ScFormatFilterPluginImpl::GetOrcusFilters()
{
    static ScOrcusFiltersImpl aImpl;
    return &aImpl;
}

SAL_DLLPUBLIC_EXPORT ScFormatFilterPlugin * SAL_CALL ScFilterCreate(void)
{
    return new ScFormatFilterPluginImpl();
}

// implementation class inside the filters

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
