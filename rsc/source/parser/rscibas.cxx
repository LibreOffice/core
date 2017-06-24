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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <comphelper/string.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <tools/fldunit.hxx>
#include <tools/gen.hxx>
#include <tools/mapunit.hxx>
#include <tools/wintypes.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>

#include <com/sun/star/awt/Key.hpp>

#include <rscconst.hxx>
#include <rscarray.hxx>
#include <rscclass.hxx>
#include <rsccont.hxx>
#include <rscdb.hxx>
#include <rsclex.hxx>
#include <rscyacc.hxx>

#include <unordered_map>

void RscTypCont::SETCONST( RscEnum * pClass, const char * szString, sal_uInt32 nVal )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "setconst : %s\n", szString );
#endif
    pClass->SetConstant( aNmTb.Put( szString,
                         CONSTNAME, nVal ), nVal );
}

typedef std::unordered_map< OString, sal_uInt32, OStringHash > langmap;
static langmap ULong_Iso_map;

sal_uInt32 GetLangId(const OString &rLang)
{
    langmap::iterator pIter = ULong_Iso_map.find( rLang );
    if ( pIter != ULong_Iso_map.end())
        return pIter->second;
    return 0;
}

void RscLangEnum::Init( RscNameTable& rNames )
{
    SetConstant( rNames.Put( "SYSTEM", CONSTNAME, (sal_uInt16)LANGUAGE_SYSTEM ), (sal_uInt16)LANGUAGE_SYSTEM );
    SetConstant( rNames.Put( "DONTKNOW", CONSTNAME, (sal_uInt16)LANGUAGE_DONTKNOW ), (sal_uInt16)LANGUAGE_DONTKNOW );

    sal_Int32 nIndex = 0;
    mnLangId = 0x400; // stay away from selfdefined...

    const ::std::vector< MsLangId::LanguagetagMapping > aList( MsLangId::getDefinedLanguagetags());
    for (::std::vector< MsLangId::LanguagetagMapping >::const_iterator iTag( aList.begin()); iTag != aList.end(); ++iTag)
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "ISO Language in : %d, 0x%04x, %s\n",
                (int)nIndex,
                (unsigned)(*iTag).mnLang,
                OUStringToOString( (*iTag).maBcp47, RTL_TEXTENCODING_ASCII_US).getStr());
        fprintf( stderr, "ISO Language out:");
#endif
        LanguageTag aLanguageTag( (*iTag).maBcp47);
        ::std::vector< OUString > aFallbacks( aLanguageTag.getFallbackStrings( true));
        for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
        {
            OString aLang( OUStringToOString( *it, RTL_TEXTENCODING_ASCII_US));
            SetConstant( rNames.Put( aLang.getStr(), CONSTNAME, mnLangId ), mnLangId );
            bool bAdd = (GetLangId( aLang ) == 0);
            if ( bAdd )
                ULong_Iso_map[ aLang ] = mnLangId;
#if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, " %s 0x%lx (%s)", aLang.getStr(), mnLangId, (bAdd ? "added" : "exists") );
#endif
            mnLangId++;
        }
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "\n");
#endif
        nIndex++;
    }
    // hack - survive "x-no-translate"
    /* XXX: that ^^^ was the original comment, but we're adding "x-comment"
     * here? Which is good anyway. */
    SetConstant( rNames.Put( "x-comment", CONSTNAME, mnLangId ), mnLangId );
    mnLangId++;

    OString aEnvIsoTokens = getenv( "RSC_LANG_ISO" );
    if ( !aEnvIsoTokens.isEmpty() )
    {
        OString aIsoToken;
        sal_uInt16 nTokenCounter = 0;
        bool bOneMore = true;
        while ( bOneMore )
        {
            aIsoToken = aEnvIsoTokens.getToken(nTokenCounter, ' ');
            if ( !aIsoToken.isEmpty() )
            {
                SetConstant( rNames.Put( aIsoToken.getStr(), CONSTNAME, mnLangId ), mnLangId );
                bool bAdd = (GetLangId( aIsoToken ) == 0);
                if ( bAdd )
                    ULong_Iso_map[ aIsoToken ] = mnLangId;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "Env ISO Language out: %s 0x%lx (%s)\n",
                        aIsoToken.getStr(), mnLangId, (bAdd ? "added" : "exists") );
#endif
                mnLangId++;
            }
            else
                bOneMore = false;

            nTokenCounter++;
        }
    }
}

Atom RscLangEnum::AddLanguage( const char* pLang, RscNameTable& rNames )
{
    Atom nResult = 0;
    KEY_STRUCT aStruct;
    if( ! rNames.Get( nResult = pHS->getID( pLang ), &aStruct ) )
    {
        SetConstant( nResult = rNames.Put( pLang, CONSTNAME, mnLangId ), mnLangId );
        // insert new lang to ULong_Iso_map
        OString aLang( pLang );
        bool bAdd = (GetLangId( aLang ) == 0);
        if ( bAdd )
            ULong_Iso_map[ aLang ] = mnLangId;
        // increase id counter
        mnLangId++;
    }
    return nResult;
}

RscEnum * RscTypCont::InitFieldUnitsType()
{
    RscEnum * pFieldUnits;
    pFieldUnits = new RscEnum( pHS->getID( "EnumFieldUnit" ), RSC_NOTYPE );

    SETCONST( pFieldUnits, "FUNIT_NONE", FUNIT_NONE );
    SETCONST( pFieldUnits, "FUNIT_MM", FUNIT_MM );
    SETCONST( pFieldUnits, "FUNIT_CM", FUNIT_CM );
    SETCONST( pFieldUnits, "FUNIT_M", FUNIT_M );
    SETCONST( pFieldUnits, "FUNIT_KM", FUNIT_KM );
    SETCONST( pFieldUnits, "FUNIT_TWIP", FUNIT_TWIP );
    SETCONST( pFieldUnits, "FUNIT_POINT", FUNIT_POINT );
    SETCONST( pFieldUnits, "FUNIT_PICA", FUNIT_PICA );
    SETCONST( pFieldUnits, "FUNIT_INCH", FUNIT_INCH );
    SETCONST( pFieldUnits, "FUNIT_FOOT", FUNIT_FOOT );
    SETCONST( pFieldUnits, "FUNIT_MILE", FUNIT_MILE );
    SETCONST( pFieldUnits, "FUNIT_CUSTOM", FUNIT_CUSTOM );
    SETCONST( pFieldUnits, "FUNIT_CHAR", FUNIT_CHAR );
    SETCONST( pFieldUnits, "FUNIT_LINE", FUNIT_LINE );
    SETCONST( pFieldUnits, "FUNIT_PERCENT", FUNIT_PERCENT );
    SETCONST( pFieldUnits, "FUNIT_DEGREE",  FUNIT_DEGREE);
    SETCONST( pFieldUnits, "FUNIT_SECOND",  FUNIT_SECOND);
    SETCONST( pFieldUnits, "FUNIT_MILLISECOND",  FUNIT_MILLISECOND);
    SETCONST( pFieldUnits, "FUNIT_PIXEL",   FUNIT_PIXEL );

    return pFieldUnits;
}

RscTupel * RscTypCont::InitStringLongTupel()
{
    RscTop *    pTupel;
    Atom        nId;

    // insert client variables
    pTupel = new RscTupel( pHS->getID( "CharsLongTupel" ), RSC_NOTYPE );
    nId = aNmTb.Put( "ItemText", VARNAME );
    pTupel->SetVariable( nId, &aString );
    nId = aNmTb.Put( "ItemId", VARNAME );
    pTupel->SetVariable( nId, &aEnumLong );

    return static_cast<RscTupel *>(pTupel);
}

RscCont * RscTypCont::InitStringLongTupelList( RscTupel * pStringLong )
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "CharsLongTupel[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( pStringLong );

    return pCont;
}

RscArray * RscTypCont::InitLangStringLongTupelList( RscCont * pStrLongTupelLst )
{
    return new RscArray( pHS->getID( "Lang_CharsLongTupelList" ),
                         RSC_NOTYPE, pStrLongTupelLst, &aLangType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
