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

#include "ftools.hxx"
#include <osl/diagnose.h>
#include <osl/thread.h>
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


// ScFilterTools::ReadLongDouble()

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
"Mapping - Guide" 10-Byte Intel

77777777 77666666 66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000   x10
98765432 10987654 32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210   Bit-# total
9      9 8      8 7      7 6      6 5      5 4      4 3      3 2      2 1      1 0      0   Byte-#
76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210   Bit-# in Byte
SEEEEEEE EEEEEEEE IMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM   Group
01111110 00000000 06665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000       x10
14321098 76543210 02109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210   Bit in Group
*/

    long double lfDouble;
    long double lfFakt = 256.0;
    sal_uInt8 pDouble10[ 10 ];

    rStrm.ReadBytes(pDouble10, 10);            // Intel-10 in pDouble10

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
        sal_Int32 nExp;
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

OUString ScfTools::GetHexStr( sal_uInt16 nValue )
{
    const sal_Char pHex[] = "0123456789ABCDEF";
    OUString aStr;

    aStr += OUString( pHex[ nValue >> 12 ] );
    aStr += OUString( pHex[ (nValue >> 8) & 0x000F ] );
    aStr += OUString( pHex[ (nValue >> 4) & 0x000F ] );
    aStr += OUString( pHex[ nValue & 0x000F ] );
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

OUString ScfTools::ConvertToScDefinedName(const OUString& rName )
{
    //fdo#37872: we don't allow points in range names any more
    OUString sName = rName.replace(static_cast<sal_Unicode>('.'),
        static_cast<sal_Unicode>('_'));
    sal_Int32 nLen = sName.getLength();
    if( nLen && !ScCompiler::IsCharFlagAllConventions( sName, 0, ScCharFlags::CharName ) )
        sName = sName.replaceAt( 0, 1, "_" );
    for( sal_Int32 nPos = 1; nPos < nLen; ++nPos )
        if( !ScCompiler::IsCharFlagAllConventions( sName, nPos, ScCharFlags::Name ) )
            sName = sName.replaceAt( nPos, 1, "_" );
    return sName;
}

// *** streams and storages *** -----------------------------------------------

tools::SvRef<SotStorage> ScfTools::OpenStorageRead( tools::SvRef<SotStorage> const & xStrg, const OUString& rStrgName )
{
    tools::SvRef<SotStorage> xSubStrg;
    if( xStrg.is() && xStrg->IsContained( rStrgName ) )
        xSubStrg = xStrg->OpenSotStorage( rStrgName, StreamMode::STD_READ );
    return xSubStrg;
}

tools::SvRef<SotStorage> ScfTools::OpenStorageWrite( tools::SvRef<SotStorage> const & xStrg, const OUString& rStrgName )
{
    tools::SvRef<SotStorage> xSubStrg;
    if( xStrg.is() )
        xSubStrg = xStrg->OpenSotStorage( rStrgName, StreamMode::STD_WRITE );
    return xSubStrg;
}

tools::SvRef<SotStorageStream> ScfTools::OpenStorageStreamRead( tools::SvRef<SotStorage> const & xStrg, const OUString& rStrmName )
{
    tools::SvRef<SotStorageStream> xStrm;
    if( xStrg.is() && xStrg->IsContained( rStrmName ) && xStrg->IsStream( rStrmName ) )
        xStrm = xStrg->OpenSotStream( rStrmName, StreamMode::STD_READ );
    return xStrm;
}

tools::SvRef<SotStorageStream> ScfTools::OpenStorageStreamWrite( tools::SvRef<SotStorage> const & xStrg, const OUString& rStrmName )
{
    OSL_ENSURE( !xStrg.is() || !xStrg->IsContained( rStrmName ), "ScfTools::OpenStorageStreamWrite - stream exists already" );
    tools::SvRef<SotStorageStream> xStrm;
    if( xStrg.is() )
        xStrm = xStrg->OpenSotStream( rStrmName, StreamMode::STD_WRITE | StreamMode::TRUNC );
    return xStrm;
}

// *** item handling *** ------------------------------------------------------

bool ScfTools::CheckItem( const SfxItemSet& rItemSet, sal_uInt16 nWhichId, bool bDeep )
{
    return rItemSet.GetItemState( nWhichId, bDeep ) == SfxItemState::SET;
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
    {
        std::unique_ptr<SfxPoolItem> pNewItem(rItem.CloneSetWhich(nWhichId));
        rItemSet.Put( *pNewItem );
    }
}

void ScfTools::PutItem( SfxItemSet& rItemSet, const SfxPoolItem& rItem, bool bSkipPoolDef )
{
    PutItem( rItemSet, rItem, rItem.Which(), bSkipPoolDef );
}

// *** style sheet handling *** -----------------------------------------------

namespace {

ScStyleSheet& lclMakeStyleSheet( ScStyleSheetPool& rPool, const OUString& rStyleName, SfxStyleFamily eFamily, bool bForceName )
{
    // find an unused name
    OUString aNewName( rStyleName );
    sal_Int32 nIndex = 0;
    SfxStyleSheetBase* pOldStyleSheet = nullptr;
    while( SfxStyleSheetBase* pStyleSheet = rPool.Find( aNewName, eFamily ) )
    {
        if( !pOldStyleSheet )
            pOldStyleSheet = pStyleSheet;
        aNewName = rStyleName + " " + OUString::number( ++nIndex );
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

ScStyleSheet& ScfTools::MakeCellStyleSheet( ScStyleSheetPool& rPool, const OUString& rStyleName, bool bForceName )
{
    return lclMakeStyleSheet( rPool, rStyleName, SfxStyleFamily::Para, bForceName );
}

ScStyleSheet& ScfTools::MakePageStyleSheet( ScStyleSheetPool& rPool, const OUString& rStyleName, bool bForceName )
{
    return lclMakeStyleSheet( rPool, rStyleName, SfxStyleFamily::Page, bForceName );
}

// *** byte string import operations *** --------------------------------------

OString ScfTools::read_zeroTerminated_uInt8s_ToOString(SvStream& rStrm, sal_Int32& rnBytesLeft)
{
    OString aRet(::read_zeroTerminated_uInt8s_ToOString(rStrm));
    rnBytesLeft -= aRet.getLength(); //we read this number of bytes anyway
    if (rStrm.good()) //if the stream is happy we read the null terminator as well
        --rnBytesLeft;
    return aRet;
}

void ScfTools::AppendCString( SvStream& rStrm, OUString& rString, rtl_TextEncoding eTextEnc )
{
    rString += ::read_zeroTerminated_uInt8s_ToOUString(rStrm, eTextEnc);
}

// *** HTML table names <-> named range names *** -----------------------------

const OUString& ScfTools::GetHTMLDocName()
{
    static const OUString saHTMLDoc( "HTML_all" );
    return saHTMLDoc;
}

const OUString& ScfTools::GetHTMLTablesName()
{
    static const OUString saHTMLTables( "HTML_tables" );
    return saHTMLTables;
}

const OUString& ScfTools::GetHTMLIndexPrefix()
{
    static const OUString saHTMLIndexPrefix( "HTML_" );
    return saHTMLIndexPrefix;

}

const OUString& ScfTools::GetHTMLNamePrefix()
{
    static const OUString saHTMLNamePrefix( "HTML__" );
    return saHTMLNamePrefix;
}

OUString ScfTools::GetNameFromHTMLIndex( sal_uInt32 nIndex )
{
    OUString aName = GetHTMLIndexPrefix() +
                     OUString::number( static_cast< sal_Int32 >( nIndex ) );
    return aName;
}

OUString ScfTools::GetNameFromHTMLName( const OUString& rTabName )
{
    OUString aName( GetHTMLNamePrefix() );
    aName += rTabName;
    return aName;
}

bool ScfTools::IsHTMLDocName( const OUString& rSource )
{
    return rSource.equalsIgnoreAsciiCase( GetHTMLDocName() );
}

bool ScfTools::IsHTMLTablesName( const OUString& rSource )
{
    return rSource.equalsIgnoreAsciiCase( GetHTMLTablesName() );
}

bool ScfTools::GetHTMLNameFromName( const OUString& rSource, OUString& rName )
{
    rName.clear();
    if( rSource.startsWithIgnoreAsciiCase( GetHTMLNamePrefix() ) )
    {
        rName = rSource.copy( GetHTMLNamePrefix().getLength() );
        ScGlobal::AddQuotes( rName, '"', false );
    }
    else if( rSource.startsWithIgnoreAsciiCase( GetHTMLIndexPrefix() ) )
    {
        OUString aIndex( rSource.copy( GetHTMLIndexPrefix().getLength() ) );
        if( CharClass::isAsciiNumeric( aIndex ) && (aIndex.toInt32() > 0) )
            rName = aIndex;
    }
    return !rName.isEmpty();
}

ScFormatFilterPluginImpl::ScFormatFilterPluginImpl() {}
ScFormatFilterPluginImpl::~ScFormatFilterPluginImpl() {}

ScOrcusFilters* ScFormatFilterPluginImpl::GetOrcusFilters()
{
    static ScOrcusFiltersImpl aImpl;
    return &aImpl;
}

ScFormatFilterPlugin * SAL_CALL ScFilterCreate()
{
    return new ScFormatFilterPluginImpl();
}

// implementation class inside the filters

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
