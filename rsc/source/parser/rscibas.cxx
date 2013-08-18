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
#include <tools/mapunit.hxx>
#include <tools/rc.h>
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

#include <boost/unordered_map.hpp>

void RscTypCont::SETCONST( RscConst * pClass, const char * szString, sal_uInt32 nVal )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "setconst : %s\n", szString );
#endif
    pClass->SetConstant( aNmTb.Put( szString,
                         CONSTNAME, nVal ), nVal );
}

void RscTypCont::SETCONST( RscConst * pClass, Atom nName, sal_uInt32 nVal )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "setconst hash: %u\n", (unsigned int)nName );
#endif
    pClass->SetConstant( aNmTb.Put( nName,
                         CONSTNAME, nVal ), nVal );
}


typedef boost::unordered_map< OString, sal_uInt32, OStringHash > langmap;
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
    SetConstant( rNames.Put( "SYSTEM", CONSTNAME, (long)LANGUAGE_SYSTEM ), LANGUAGE_SYSTEM );
    SetConstant( rNames.Put( "DONTKNOW", CONSTNAME, LANGUAGE_DONTKNOW ), LANGUAGE_DONTKNOW );

    sal_Int32 nIndex = 0;
    mnLangId = 0x400; // stay away from selfdefined...
    const MsLangId::IsoLangEntry* pLangEntry;

    while ( NULL != ( pLangEntry = MsLangId::getIsoLangEntry( nIndex )) && ( pLangEntry->mnLang != LANGUAGE_DONTKNOW ))
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "ISO Language in : %d, 0x%04x, %s\n",
                (int)nIndex,
                (unsigned)pLangEntry->mnLang,
                OUStringToOString( pLangEntry->getTagString(), RTL_TEXTENCODING_ASCII_US).getStr());
        fprintf( stderr, "ISO Language out:");
#endif
        LanguageTag aLanguageTag( pLangEntry->getTagString());
        ::std::vector< OUString > aFallbacks( aLanguageTag.getFallbackStrings());
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
        bool bOneMore = 1;
        while ( bOneMore )
        {
            aIsoToken = comphelper::string::getToken(aEnvIsoTokens, nTokenCounter, ' ');
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
                bOneMore = 0;

            nTokenCounter++;
        }
    }
}

Atom RscLangEnum::AddLanguage( const char* pLang, RscNameTable& rNames )
{
    Atom nResult = 0;
    bool bAdd = false;
    KEY_STRUCT aStruct;
    if( ! rNames.Get( nResult = pHS->getID( pLang ), &aStruct ) )
    {
        SetConstant( nResult = rNames.Put( pLang, CONSTNAME, mnLangId ), mnLangId );
        // insert new lang to ULong_Iso_map
        OString aLang( pLang );
        bAdd = (GetLangId( aLang ) == 0);
        if ( bAdd )
            ULong_Iso_map[ aLang ] = mnLangId;
        // increase id counter
        mnLangId++;
    }
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "AddLanguage( '%s' ) = %d (%s)\n", pLang, nResult, (bAdd ? "added" : "exists") );
#endif
    return nResult;
}

RscEnum * RscTypCont::InitLangType()
{
    aLangType.Init( aNmTb );
    return &aLangType;
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

RscEnum * RscTypCont::InitTimeFieldFormat()
{
    RscEnum * pTimeFieldFormat;
    pTimeFieldFormat = new RscEnum( pHS->getID( "EnumTimeFieldFormat" ),
                                    RSC_NOTYPE );

    SETCONST( pTimeFieldFormat, "TIMEF_NONE", TIMEF_NONE );
    SETCONST( pTimeFieldFormat, "TIMEF_SEC", TIMEF_SEC );
    SETCONST( pTimeFieldFormat, "TIMEF_100TH_SEC", TIMEF_100TH_SEC );

    return pTimeFieldFormat;
}

RscEnum * RscTypCont::InitColor()
{
    RscEnum * pColor;
    pColor = new RscEnum( pHS->getID( "EnumColor" ), RSC_NOTYPE );

    SETCONST( pColor, "COL_BLACK",                  COL_BLACK );
    SETCONST( pColor, "COL_BLUE",                   COL_BLUE );
    SETCONST( pColor, "COL_GREEN",                  COL_GREEN );
    SETCONST( pColor, "COL_CYAN",                   COL_CYAN );
    SETCONST( pColor, "COL_RED",                    COL_RED );
    SETCONST( pColor, "COL_MAGENTA",                COL_MAGENTA );
    SETCONST( pColor, "COL_BROWN",                  COL_BROWN );
    SETCONST( pColor, "COL_GRAY",                   COL_GRAY );
    SETCONST( pColor, "COL_LIGHTGRAY",              COL_LIGHTGRAY );
    SETCONST( pColor, "COL_LIGHTBLUE",              COL_LIGHTBLUE );
    SETCONST( pColor, "COL_LIGHTGREEN",             COL_LIGHTGREEN );
    SETCONST( pColor, "COL_LIGHTCYAN",              COL_LIGHTCYAN );
    SETCONST( pColor, "COL_LIGHTRED",               COL_LIGHTRED );
    SETCONST( pColor, "COL_LIGHTMAGENTA",           COL_LIGHTMAGENTA );
    SETCONST( pColor, "COL_YELLOW",                 COL_YELLOW );
    SETCONST( pColor, "COL_WHITE",                  COL_WHITE );

    return pColor;
}

RscEnum * RscTypCont::InitMapUnit()
{
    RscEnum * pMapUnit;
    pMapUnit = new RscEnum( pHS->getID( "EnumMapUnit" ), RSC_NOTYPE );

    SETCONST( pMapUnit, "MAP_PIXEL",                  MAP_PIXEL );
    SETCONST( pMapUnit, "MAP_SYSFONT",                MAP_SYSFONT );
    SETCONST( pMapUnit, "MAP_100TH_MM",               MAP_100TH_MM );
    SETCONST( pMapUnit, "MAP_10TH_MM",                MAP_10TH_MM );
    SETCONST( pMapUnit, "MAP_MM",                     MAP_MM );
    SETCONST( pMapUnit, "MAP_CM",                     MAP_CM );
    SETCONST( pMapUnit, "MAP_1000TH_INCH",            MAP_1000TH_INCH );
    SETCONST( pMapUnit, "MAP_100TH_INCH",             MAP_100TH_INCH );
    SETCONST( pMapUnit, "MAP_10TH_INCH",              MAP_10TH_INCH );
    SETCONST( pMapUnit, "MAP_INCH",                   MAP_INCH );
    SETCONST( pMapUnit, "MAP_POINT",                  MAP_POINT );
    SETCONST( pMapUnit, "MAP_TWIP",                   MAP_TWIP );
    SETCONST( pMapUnit, "MAP_APPFONT",                MAP_APPFONT );
    SETCONST( pMapUnit, "MAP_SV",                     RSC_EXTRAMAPUNIT );
    return pMapUnit;
}

RscEnum * RscTypCont::InitKey()
{
    RscEnum * pKey;
    pKey = new RscEnum( pHS->getID( "EnumKey" ), RSC_NOTYPE );

    SETCONST( pKey, "KEY_0",                    com::sun::star::awt::Key::NUM0 );
    SETCONST( pKey, "KEY_1",                    com::sun::star::awt::Key::NUM1 );
    SETCONST( pKey, "KEY_2",                    com::sun::star::awt::Key::NUM2 );
    SETCONST( pKey, "KEY_3",                    com::sun::star::awt::Key::NUM3 );
    SETCONST( pKey, "KEY_4",                    com::sun::star::awt::Key::NUM4 );
    SETCONST( pKey, "KEY_5",                    com::sun::star::awt::Key::NUM5 );
    SETCONST( pKey, "KEY_6",                    com::sun::star::awt::Key::NUM6 );
    SETCONST( pKey, "KEY_7",                    com::sun::star::awt::Key::NUM7 );
    SETCONST( pKey, "KEY_8",                    com::sun::star::awt::Key::NUM8 );
    SETCONST( pKey, "KEY_9",                    com::sun::star::awt::Key::NUM9 );

    SETCONST( pKey, "KEY_A",                    com::sun::star::awt::Key::A );
    SETCONST( pKey, "KEY_B",                    com::sun::star::awt::Key::B );
    SETCONST( pKey, "KEY_C",                    com::sun::star::awt::Key::C );
    SETCONST( pKey, "KEY_D",                    com::sun::star::awt::Key::D );
    SETCONST( pKey, "KEY_E",                    com::sun::star::awt::Key::E );
    SETCONST( pKey, "KEY_F",                    com::sun::star::awt::Key::F );
    SETCONST( pKey, "KEY_G",                    com::sun::star::awt::Key::G );
    SETCONST( pKey, "KEY_H",                    com::sun::star::awt::Key::H );
    SETCONST( pKey, "KEY_I",                    com::sun::star::awt::Key::I );
    SETCONST( pKey, "KEY_J",                    com::sun::star::awt::Key::J );
    SETCONST( pKey, "KEY_K",                    com::sun::star::awt::Key::K );
    SETCONST( pKey, "KEY_L",                    com::sun::star::awt::Key::L );
    SETCONST( pKey, "KEY_M",                    com::sun::star::awt::Key::M );
    SETCONST( pKey, "KEY_N",                    com::sun::star::awt::Key::N );
    SETCONST( pKey, "KEY_O",                    com::sun::star::awt::Key::O );
    SETCONST( pKey, "KEY_P",                    com::sun::star::awt::Key::P );
    SETCONST( pKey, "KEY_Q",                    com::sun::star::awt::Key::Q );
    SETCONST( pKey, "KEY_R",                    com::sun::star::awt::Key::R );
    SETCONST( pKey, "KEY_S",                    com::sun::star::awt::Key::S );
    SETCONST( pKey, "KEY_T",                    com::sun::star::awt::Key::T );
    SETCONST( pKey, "KEY_U",                    com::sun::star::awt::Key::U );
    SETCONST( pKey, "KEY_V",                    com::sun::star::awt::Key::V );
    SETCONST( pKey, "KEY_W",                    com::sun::star::awt::Key::W );
    SETCONST( pKey, "KEY_X",                    com::sun::star::awt::Key::X );
    SETCONST( pKey, "KEY_Y",                    com::sun::star::awt::Key::Y );
    SETCONST( pKey, "KEY_Z",                    com::sun::star::awt::Key::Z );

    SETCONST( pKey, "KEY_F1",                   com::sun::star::awt::Key::F1 );
    SETCONST( pKey, "KEY_F2",                   com::sun::star::awt::Key::F2 );
    SETCONST( pKey, "KEY_F3",                   com::sun::star::awt::Key::F3 );
    SETCONST( pKey, "KEY_F4",                   com::sun::star::awt::Key::F4 );
    SETCONST( pKey, "KEY_F5",                   com::sun::star::awt::Key::F5 );
    SETCONST( pKey, "KEY_F6",                   com::sun::star::awt::Key::F6 );
    SETCONST( pKey, "KEY_F7",                   com::sun::star::awt::Key::F7 );
    SETCONST( pKey, "KEY_F8",                   com::sun::star::awt::Key::F8 );
    SETCONST( pKey, "KEY_F9",                   com::sun::star::awt::Key::F9 );
    SETCONST( pKey, "KEY_F10",                  com::sun::star::awt::Key::F10 );
    SETCONST( pKey, "KEY_F11",                  com::sun::star::awt::Key::F11 );
    SETCONST( pKey, "KEY_F12",                  com::sun::star::awt::Key::F12 );
    SETCONST( pKey, "KEY_F13",                  com::sun::star::awt::Key::F13 );
    SETCONST( pKey, "KEY_F14",                  com::sun::star::awt::Key::F14 );
    SETCONST( pKey, "KEY_F15",                  com::sun::star::awt::Key::F15 );
    SETCONST( pKey, "KEY_F16",                  com::sun::star::awt::Key::F16 );
    SETCONST( pKey, "KEY_F17",                  com::sun::star::awt::Key::F17 );
    SETCONST( pKey, "KEY_F18",                  com::sun::star::awt::Key::F18 );
    SETCONST( pKey, "KEY_F19",                  com::sun::star::awt::Key::F19 );
    SETCONST( pKey, "KEY_F20",                  com::sun::star::awt::Key::F20 );
    SETCONST( pKey, "KEY_F21",                  com::sun::star::awt::Key::F21 );
    SETCONST( pKey, "KEY_F22",                  com::sun::star::awt::Key::F22 );
    SETCONST( pKey, "KEY_F23",                  com::sun::star::awt::Key::F23 );
    SETCONST( pKey, "KEY_F24",                  com::sun::star::awt::Key::F24 );
    SETCONST( pKey, "KEY_F25",                  com::sun::star::awt::Key::F25 );
    SETCONST( pKey, "KEY_F26",                  com::sun::star::awt::Key::F26 );

    SETCONST( pKey, "KEY_DOWN",                 com::sun::star::awt::Key::DOWN );
    SETCONST( pKey, "KEY_UP",                   com::sun::star::awt::Key::UP );
    SETCONST( pKey, "KEY_LEFT",                 com::sun::star::awt::Key::LEFT );
    SETCONST( pKey, "KEY_RIGHT",                com::sun::star::awt::Key::RIGHT );
    SETCONST( pKey, "KEY_HOME",                 com::sun::star::awt::Key::HOME );
    SETCONST( pKey, "KEY_END",                  com::sun::star::awt::Key::END );
    SETCONST( pKey, "KEY_PAGEUP",               com::sun::star::awt::Key::PAGEUP );
    SETCONST( pKey, "KEY_PAGEDOWN",             com::sun::star::awt::Key::PAGEDOWN );

    SETCONST( pKey, "KEY_RETURN",               com::sun::star::awt::Key::RETURN );
    SETCONST( pKey, "KEY_ESCAPE",               com::sun::star::awt::Key::ESCAPE );
    SETCONST( pKey, "KEY_TAB",                  com::sun::star::awt::Key::TAB );
    SETCONST( pKey, "KEY_BACKSPACE",            com::sun::star::awt::Key::BACKSPACE );
    SETCONST( pKey, "KEY_SPACE",                com::sun::star::awt::Key::SPACE );
    SETCONST( pKey, "KEY_INSERT",               com::sun::star::awt::Key::INSERT );
    SETCONST( pKey, "KEY_DELETE",               com::sun::star::awt::Key::DELETE );

    SETCONST( pKey, "KEY_ADD",                  com::sun::star::awt::Key::ADD );
    SETCONST( pKey, "KEY_SUBTRACT",             com::sun::star::awt::Key::SUBTRACT );
    SETCONST( pKey, "KEY_MULTIPLY",             com::sun::star::awt::Key::MULTIPLY );
    SETCONST( pKey, "KEY_DIVIDE",               com::sun::star::awt::Key::DIVIDE );
    SETCONST( pKey, "KEY_POINT",                com::sun::star::awt::Key::POINT );
    SETCONST( pKey, "KEY_COMMA",                com::sun::star::awt::Key::COMMA );
    SETCONST( pKey, "KEY_LESS",                 com::sun::star::awt::Key::LESS );
    SETCONST( pKey, "KEY_GREATER",              com::sun::star::awt::Key::GREATER );
    SETCONST( pKey, "KEY_EQUAL",                com::sun::star::awt::Key::EQUAL );

    SETCONST( pKey, "KEY_OPEN",                 com::sun::star::awt::Key::OPEN );
    SETCONST( pKey, "KEY_CUT",                  com::sun::star::awt::Key::CUT );
    SETCONST( pKey, "KEY_COPY",                 com::sun::star::awt::Key::COPY );
    SETCONST( pKey, "KEY_PASTE",                com::sun::star::awt::Key::PASTE );
    SETCONST( pKey, "KEY_UNDO",                 com::sun::star::awt::Key::UNDO );
    SETCONST( pKey, "KEY_REPEAT",               com::sun::star::awt::Key::REPEAT );
    SETCONST( pKey, "KEY_FIND",                 com::sun::star::awt::Key::FIND );
    SETCONST( pKey, "KEY_PROPERTIES",           com::sun::star::awt::Key::PROPERTIES );
    SETCONST( pKey, "KEY_FRONT",                com::sun::star::awt::Key::FRONT );
    SETCONST( pKey, "KEY_CONTEXTMENU",          com::sun::star::awt::Key::CONTEXTMENU );
    SETCONST( pKey, "KEY_HELP",                 com::sun::star::awt::Key::HELP );
    SETCONST( pKey, "KEY_HANGUL_HANJA",         com::sun::star::awt::Key::HANGUL_HANJA );
    SETCONST( pKey, "KEY_DECIMAL",              com::sun::star::awt::Key::DECIMAL );
    SETCONST( pKey, "KEY_TILDE",                com::sun::star::awt::Key::TILDE );
    SETCONST( pKey, "KEY_QUOTELEFT",            com::sun::star::awt::Key::QUOTELEFT );

    return pKey;
}

RscEnum * RscTypCont::InitTriState()
{
    RscEnum * pTriState;
    pTriState = new RscEnum( pHS->getID( "EnumTriState" ), RSC_NOTYPE );

    SETCONST( pTriState, "STATE_NOCHECK",      STATE_NOCHECK  );
    SETCONST( pTriState, "STATE_CHECK",        STATE_CHECK    );
    SETCONST( pTriState, "STATE_DONTKNOW",     STATE_DONTKNOW );

    return pTriState;
}

RscEnum * RscTypCont::InitMessButtons()
{
    RscEnum * pMessButtons;
    pMessButtons = new RscEnum( pHS->getID( "EnumMessButtons" ), RSC_NOTYPE );
    SETCONST( pMessButtons, "WB_OK",                      sal::static_int_cast<sal_uInt32>(WB_OK) );
    SETCONST( pMessButtons, "WB_OK_CANCEL",               sal::static_int_cast<sal_uInt32>(WB_OK_CANCEL) );
    SETCONST( pMessButtons, "WB_YES_NO",                  sal::static_int_cast<sal_uInt32>(WB_YES_NO) );
    SETCONST( pMessButtons, "WB_YES_NO_CANCEL",           sal::static_int_cast<sal_uInt32>(WB_YES_NO_CANCEL) );
    SETCONST( pMessButtons, "WB_RETRY_CANCEL",            sal::static_int_cast<sal_uInt32>(WB_RETRY_CANCEL) );
    SETCONST( pMessButtons, "WB_ABORT_RETRY_IGNORE",      sal::static_int_cast<sal_uInt32>(WB_ABORT_RETRY_IGNORE) );
    return pMessButtons;
}

RscEnum * RscTypCont::InitMessDefButton()
{
    RscEnum * pMessDefButton;
    pMessDefButton = new RscEnum( pHS->getID( "EnumMessDefButton" ),
                                  RSC_NOTYPE );

    SETCONST( pMessDefButton, "WB_DEF_OK",                  sal::static_int_cast<sal_uInt32>(WB_DEF_OK) );
    SETCONST( pMessDefButton, "WB_DEF_CANCEL",              sal::static_int_cast<sal_uInt32>(WB_DEF_CANCEL) );
    SETCONST( pMessDefButton, "WB_DEF_RETRY",               sal::static_int_cast<sal_uInt32>(WB_DEF_RETRY) );
    SETCONST( pMessDefButton, "WB_DEF_YES",                 sal::static_int_cast<sal_uInt32>(WB_DEF_YES) );
    SETCONST( pMessDefButton, "WB_DEF_NO",                  sal::static_int_cast<sal_uInt32>(WB_DEF_NO) );
    SETCONST( pMessDefButton, "WB_DEF_IGNORE",              sal::static_int_cast<sal_uInt32>(WB_DEF_IGNORE) );
    return pMessDefButton;
}

RscTupel * RscTypCont::InitGeometry()
{
    RscTop *    pTupel;
    Atom        nId;

    // Clientvariablen einfuegen
    pTupel = new RscTupel( pHS->getID( "TupelDeltaSystem" ),
                                RSC_NOTYPE, NULL );
    nId = aNmTb.Put( "X", VARNAME );
    pTupel->SetVariable( nId, &aShort );
    nId = aNmTb.Put( "Y", VARNAME );
    pTupel->SetVariable( nId, &aShort );
    nId = aNmTb.Put( "WIDTH", VARNAME );
    pTupel->SetVariable( nId, &aShort );
    nId = aNmTb.Put( "HEIGHT", VARNAME );
    pTupel->SetVariable( nId, &aShort );

    return (RscTupel *)pTupel;
}

RscArray * RscTypCont::InitLangGeometry( RscTupel * pGeo )
{
    return new RscArray( pHS->getID( "Lang_TupelGeometry" ), RSC_NOTYPE, pGeo, &aLangType );
}

RscCont * RscTypCont::InitStringList()
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "Chars[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( &aString );

    return pCont;
}

RscArray * RscTypCont::InitLangStringList( RscCont * pStrLst )
{
    return new RscArray( pHS->getID( "Lang_CharsList" ),
                         RSC_NOTYPE, pStrLst, &aLangType );
}

RscTupel * RscTypCont::InitStringTupel()
{
    RscTop *    pTupel;
    Atom        nId;

    // Clientvariablen einfuegen
    pTupel = new RscTupel( pHS->getID( "CharsTupel" ), RSC_NOTYPE, NULL );
    nId = aNmTb.Put( "FILTER", VARNAME );
    pTupel->SetVariable( nId, &aString );
    nId = aNmTb.Put( "MASK", VARNAME );
    pTupel->SetVariable( nId, &aString );

    return (RscTupel *)pTupel;
}

RscTupel * RscTypCont::InitStringLongTupel()
{
    RscTop *    pTupel;
    Atom        nId;

    // Clientvariablen einfuegen
    pTupel = new RscTupel( pHS->getID( "CharsLongTupel" ), RSC_NOTYPE, NULL );
    nId = aNmTb.Put( "ItemText", VARNAME );
    pTupel->SetVariable( nId, &aString );
    nId = aNmTb.Put( "ItemId", VARNAME );
    pTupel->SetVariable( nId, &aEnumLong );

    return (RscTupel *)pTupel;
}

RscCont * RscTypCont::InitStringTupelList( RscTupel * pTupelString )
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "CharsCharsTupel[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( pTupelString );

    return pCont;
}

RscCont * RscTypCont::InitStringLongTupelList( RscTupel * pStringLong )
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "CharsLongTupel[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( pStringLong );

    return pCont;
}

RscArray * RscTypCont::InitLangStringTupelList( RscCont * pStrTupelLst )
{
    return new RscArray( pHS->getID( "Lang_CharsCharsTupel" ),
                         RSC_NOTYPE, pStrTupelLst, &aLangType );
}

RscArray * RscTypCont::InitLangStringLongTupelList( RscCont * pStrLongTupelLst )
{
    return new RscArray( pHS->getID( "Lang_CharsLongTupelList" ),
                         RSC_NOTYPE, pStrLongTupelLst, &aLangType );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
