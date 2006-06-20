/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscibas.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:46:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/****************** I N C L U D E S **************************************/
// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <i18npool/mslangid.hxx>
#include <tools/rc.h>
#include <tools/color.hxx>

#include <vclrsc.hxx>

#ifndef _RSCCONST_HXX
#include <rscconst.hxx>
#endif
#ifndef _RSCARRAY_HXX
#include <rscarray.hxx>
#endif
#ifndef _RSCCLASS_HXX
#include <rscclass.hxx>
#endif
#ifndef _RSCCONT_HXX
#include <rsccont.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif


#include "rsclex.hxx"
#include <yyrscyacc.hxx>

#include <hash_map>

/****************** M A C R O S ******************************************/
void RscTypCont::SETCONST( RscConst * pClass, const char * szString, UINT32 nVal )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "setconst : %s\n", szString );
#endif
    pClass->SetConstant( aNmTb.Put( szString,
                         CONSTNAME, nVal ), nVal );
}

void RscTypCont::SETCONST( RscConst * pClass, Atom nName, UINT32 nVal )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "setconst hash: %d\n", nName );
#endif
    pClass->SetConstant( aNmTb.Put( nName,
                         CONSTNAME, nVal ), nVal );
}

/****************** C O D E **********************************************/
/*************************************************************************
|*    RscTypCont::InitLangType()
|*
|*    Beschreibung
*************************************************************************/

typedef std::hash_map< rtl::OString, sal_uInt32, rtl::OStringHash > langmap;
static langmap ULong_Iso_map;

sal_uInt32 GetLangId( const ByteString& aLang )
{
    langmap::iterator pIter = ULong_Iso_map.find( aLang );
    if ( pIter != ULong_Iso_map.end())
        return pIter->second;
    return 0;
}

void RscLangEnum::Init( RscNameTable& rNames )
{
    SetConstant( rNames.Put( "SYSTEM", CONSTNAME, (long)LANGUAGE_SYSTEM ), LANGUAGE_SYSTEM );
    SetConstant( rNames.Put( "DONTKNOW", CONSTNAME, LANGUAGE_DONTKNOW ), LANGUAGE_DONTKNOW );

    sal_Int32 nIndex = 0;
    unsigned long nI = 0x400; // stay away from selfdefined...
    char csep = '-';
    const MsLangId::IsoLangEntry* pLangEntry;
    ByteString aCountry, aLang;

    while ( NULL != ( pLangEntry = MsLangId::getIsoLangEntry( nIndex )) && ( pLangEntry->mnLang != LANGUAGE_DONTKNOW ))
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "ISO Language in : %d %d %s\n",
                 nIndex,
                 pLangEntry->meLang,
                 MsLangId::convertLanguageToIsoByteString( pLangEntry->meLang ).GetBuffer() );
#endif
        aLang = pLangEntry->maLangStr;
        aCountry = pLangEntry->maCountry;
        if ( aLang.EqualsIgnoreCaseAscii( aCountry ) ||  ! aCountry.Len() )
        {
            SetConstant( rNames.Put( aLang.GetBuffer(), CONSTNAME, nI ), nI );
            if ( ! GetLangId( aLang ))
                ULong_Iso_map[ aLang ] = nI;
#if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, "ISO Language out: %s 0x%hx\n", aLang.GetBuffer(), nI );
#endif
            nI++;
        }
        else
        {
            SetConstant( rNames.Put( aLang.GetBuffer(), CONSTNAME, nI ), nI );
            if ( ! GetLangId( aLang ))
                ULong_Iso_map[ aLang ] = nI;
#if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, "ISO Language out: %s 0x%hx", aLang.GetBuffer(), nI );
#endif
            nI++;
            aLang += csep;
            aLang += aCountry.ToUpperAscii();
            SetConstant( rNames.Put( aLang.GetBuffer(), CONSTNAME, nI ), nI );
            if ( ! GetLangId( aLang ))
                ULong_Iso_map[ aLang ] = nI;
#if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, " %s 0x%hx\n", aLang.GetBuffer(), nI );
#endif
            nI++;
// hack - survive "x-no-translate"
            if ( aLang == "en-US" )
            {
//                SetConstant( rNames.Put( "x-no-translate", CONSTNAME, nI ), nI );
//                nI++;
                SetConstant( rNames.Put( "x-comment", CONSTNAME, nI ), nI );
                nI++;
            }
        }
        nIndex++;
    }

    ByteString aEnvIsoTokens = getenv( "RSC_LANG_ISO" );
    if ( aEnvIsoTokens.Len() )
    {
        ByteString aIsoToken;
        sal_uInt16 nTokenCounter = 0;
        sal_Bool bOneMore = 1;
        while ( bOneMore )
        {
            aIsoToken = aEnvIsoTokens.GetToken( nTokenCounter, ' ' );
            if ( aIsoToken.Len() )
            {
                SetConstant( rNames.Put( aIsoToken.GetBuffer(), CONSTNAME, nI ), nI );
                if ( ! GetLangId( aIsoToken ))
                    ULong_Iso_map[ aIsoToken ] = nI;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "Env ISO Language out: %s 0x%hx\n", aIsoToken.GetBuffer(), nI );
#endif
                nI++;
            }
            else
                bOneMore = 0;
            nTokenCounter++;
        }
    }

    SetConstant( rNames.Put( "LANGUAGE_USER1", CONSTNAME, LANGUAGE_USER1 ), LANGUAGE_USER1 );
    SetConstant( rNames.Put( "LANGUAGE_USER2", CONSTNAME, LANGUAGE_USER2 ), LANGUAGE_USER2 );
    SetConstant( rNames.Put( "LANGUAGE_USER3", CONSTNAME, LANGUAGE_USER3 ), LANGUAGE_USER3 );
    SetConstant( rNames.Put( "LANGUAGE_USER4", CONSTNAME, LANGUAGE_USER4 ), LANGUAGE_USER4 );
    SetConstant( rNames.Put( "LANGUAGE_USER5", CONSTNAME, LANGUAGE_USER5 ), LANGUAGE_USER5 );
    SetConstant( rNames.Put( "LANGUAGE_USER6", CONSTNAME, LANGUAGE_USER6 ), LANGUAGE_USER6 );
    SetConstant( rNames.Put( "LANGUAGE_USER7", CONSTNAME, LANGUAGE_USER7 ), LANGUAGE_USER7 );
    SetConstant( rNames.Put( "LANGUAGE_USER8", CONSTNAME, LANGUAGE_USER8 ), LANGUAGE_USER8 );
    SetConstant( rNames.Put( "EXTERN", CONSTNAME, LANGUAGE_USER9 ), LANGUAGE_USER9 );
}

RscEnum * RscTypCont::InitLangType()
{
    aLangType.Init( aNmTb );
    return( &aLangType );
}

/*************************************************************************
|*
|*    RscTypCont::InitFieldUnitsType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.03.91
|*    Letzte Aenderung  MM 27.06.91
|*
*************************************************************************/
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
    SETCONST( pFieldUnits, "FUNIT_PERCENT", FUNIT_PERCENT );

    return pFieldUnits;
}

/*************************************************************************
|*
|*    RscTypCont::InitTimeFieldFormat()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.03.91
|*    Letzte Aenderung  MM 27.06.91
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscTypCont::InitColor()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscEnum * RscTypCont::InitColor(){
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

    return( pColor );
}

/*************************************************************************
|*
|*    RscTypCont::InitMapUnit()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscEnum * RscTypCont::InitMapUnit(){
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
    return( pMapUnit );
}

/*************************************************************************
|*
|*    RscTypCont::InitKey()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscEnum * RscTypCont::InitKey(){
    RscEnum * pKey;
    pKey = new RscEnum( pHS->getID( "EnumKey" ), RSC_NOTYPE );

    SETCONST( pKey, "KEY_0",                    KEY_0 );
    SETCONST( pKey, "KEY_1",                    KEY_1 );
    SETCONST( pKey, "KEY_2",                    KEY_2 );
    SETCONST( pKey, "KEY_3",                    KEY_3 );
    SETCONST( pKey, "KEY_4",                    KEY_4 );
    SETCONST( pKey, "KEY_5",                    KEY_5 );
    SETCONST( pKey, "KEY_6",                    KEY_6 );
    SETCONST( pKey, "KEY_7",                    KEY_7 );
    SETCONST( pKey, "KEY_8",                    KEY_8 );
    SETCONST( pKey, "KEY_9",                    KEY_9 );

    SETCONST( pKey, "KEY_A",                    KEY_A );
    SETCONST( pKey, "KEY_B",                    KEY_B );
    SETCONST( pKey, "KEY_C",                    KEY_C );
    SETCONST( pKey, "KEY_D",                    KEY_D );
    SETCONST( pKey, "KEY_E",                    KEY_E );
    SETCONST( pKey, "KEY_F",                    KEY_F );
    SETCONST( pKey, "KEY_G",                    KEY_G );
    SETCONST( pKey, "KEY_H",                    KEY_H );
    SETCONST( pKey, "KEY_I",                    KEY_I );
    SETCONST( pKey, "KEY_J",                    KEY_J );
    SETCONST( pKey, "KEY_K",                    KEY_K );
    SETCONST( pKey, "KEY_L",                    KEY_L );
    SETCONST( pKey, "KEY_M",                    KEY_M );
    SETCONST( pKey, "KEY_N",                    KEY_N );
    SETCONST( pKey, "KEY_O",                    KEY_O );
    SETCONST( pKey, "KEY_P",                    KEY_P );
    SETCONST( pKey, "KEY_Q",                    KEY_Q );
    SETCONST( pKey, "KEY_R",                    KEY_R );
    SETCONST( pKey, "KEY_S",                    KEY_S );
    SETCONST( pKey, "KEY_T",                    KEY_T );
    SETCONST( pKey, "KEY_U",                    KEY_U );
    SETCONST( pKey, "KEY_V",                    KEY_V );
    SETCONST( pKey, "KEY_W",                    KEY_W );
    SETCONST( pKey, "KEY_X",                    KEY_X );
    SETCONST( pKey, "KEY_Y",                    KEY_Y );
    SETCONST( pKey, "KEY_Z",                    KEY_Z );

    SETCONST( pKey, "KEY_F1",                   KEY_F1 );
    SETCONST( pKey, "KEY_F2",                   KEY_F2 );
    SETCONST( pKey, "KEY_F3",                   KEY_F3 );
    SETCONST( pKey, "KEY_F4",                   KEY_F4 );
    SETCONST( pKey, "KEY_F5",                   KEY_F5 );
    SETCONST( pKey, "KEY_F6",                   KEY_F6 );
    SETCONST( pKey, "KEY_F7",                   KEY_F7 );
    SETCONST( pKey, "KEY_F8",                   KEY_F8 );
    SETCONST( pKey, "KEY_F9",                   KEY_F9 );
    SETCONST( pKey, "KEY_F10",                  KEY_F10 );
    SETCONST( pKey, "KEY_F11",                  KEY_F11 );
    SETCONST( pKey, "KEY_F12",                  KEY_F12 );
    SETCONST( pKey, "KEY_F13",                  KEY_F13 );
    SETCONST( pKey, "KEY_F14",                  KEY_F14 );
    SETCONST( pKey, "KEY_F15",                  KEY_F15 );
    SETCONST( pKey, "KEY_F16",                  KEY_F16 );
    SETCONST( pKey, "KEY_F17",                  KEY_F17 );
    SETCONST( pKey, "KEY_F18",                  KEY_F18 );
    SETCONST( pKey, "KEY_F19",                  KEY_F19 );
    SETCONST( pKey, "KEY_F20",                  KEY_F20 );
    SETCONST( pKey, "KEY_F21",                  KEY_F21 );
    SETCONST( pKey, "KEY_F22",                  KEY_F22 );
    SETCONST( pKey, "KEY_F23",                  KEY_F23 );
    SETCONST( pKey, "KEY_F24",                  KEY_F24 );
    SETCONST( pKey, "KEY_F25",                  KEY_F25 );
    SETCONST( pKey, "KEY_F26",                  KEY_F26 );

    SETCONST( pKey, "KEY_DOWN",                 KEY_DOWN );
    SETCONST( pKey, "KEY_UP",                   KEY_UP );
    SETCONST( pKey, "KEY_LEFT",                 KEY_LEFT );
    SETCONST( pKey, "KEY_RIGHT",                KEY_RIGHT );
    SETCONST( pKey, "KEY_HOME",                 KEY_HOME );
    SETCONST( pKey, "KEY_END",                  KEY_END );
    SETCONST( pKey, "KEY_PAGEUP",               KEY_PAGEUP );
    SETCONST( pKey, "KEY_PAGEDOWN",             KEY_PAGEDOWN );

    SETCONST( pKey, "KEY_RETURN",               KEY_RETURN );
    SETCONST( pKey, "KEY_ESCAPE",               KEY_ESCAPE );
    SETCONST( pKey, "KEY_TAB",                  KEY_TAB );
    SETCONST( pKey, "KEY_BACKSPACE",            KEY_BACKSPACE );
    SETCONST( pKey, "KEY_SPACE",                KEY_SPACE );
    SETCONST( pKey, "KEY_INSERT",               KEY_INSERT );
    SETCONST( pKey, "KEY_DELETE",               KEY_DELETE );

    SETCONST( pKey, "KEY_ADD",                  KEY_ADD );
    SETCONST( pKey, "KEY_SUBTRACT",             KEY_SUBTRACT );
    SETCONST( pKey, "KEY_MULTIPLY",             KEY_MULTIPLY );
    SETCONST( pKey, "KEY_DIVIDE",               KEY_DIVIDE );
    SETCONST( pKey, "KEY_POINT",                KEY_POINT );
    SETCONST( pKey, "KEY_COMMA",                KEY_COMMA );
    SETCONST( pKey, "KEY_LESS",                 KEY_LESS );
    SETCONST( pKey, "KEY_GREATER",              KEY_GREATER );
    SETCONST( pKey, "KEY_EQUAL",                KEY_EQUAL );

    SETCONST( pKey, "KEY_OPEN",                 KEY_OPEN );
    SETCONST( pKey, "KEY_CUT",                  KEY_CUT );
    SETCONST( pKey, "KEY_COPY",                 KEY_COPY );
    SETCONST( pKey, "KEY_PASTE",                KEY_PASTE );
    SETCONST( pKey, "KEY_UNDO",                 KEY_UNDO );
    SETCONST( pKey, "KEY_REPEAT",               KEY_REPEAT );
    SETCONST( pKey, "KEY_FIND",                 KEY_FIND );
    SETCONST( pKey, "KEY_PROPERTIES",           KEY_PROPERTIES );
    SETCONST( pKey, "KEY_FRONT",                KEY_FRONT );
    SETCONST( pKey, "KEY_CONTEXTMENU",          KEY_CONTEXTMENU );
    SETCONST( pKey, "KEY_HELP",                 KEY_HELP );
    SETCONST( pKey, "KEY_HANGUL_HANJA",         KEY_HANGUL_HANJA );
    SETCONST( pKey, "KEY_DECIMAL",              KEY_DECIMAL );
    SETCONST( pKey, "KEY_TILDE",                KEY_TILDE );
    SETCONST( pKey, "KEY_QUOTELEFT",            KEY_QUOTELEFT );

    return( pKey );
}

/*************************************************************************
|*
|*    RscTypCont::InitTriState()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.11.91
|*    Letzte Aenderung  MM 26.11.91
|*
*************************************************************************/
RscEnum * RscTypCont::InitTriState(){
    RscEnum * pTriState;
    pTriState = new RscEnum( pHS->getID( "EnumTriState" ), RSC_NOTYPE );

    SETCONST( pTriState, "STATE_NOCHECK",      STATE_NOCHECK  );
    SETCONST( pTriState, "STATE_CHECK",        STATE_CHECK    );
    SETCONST( pTriState, "STATE_DONTKNOW",     STATE_DONTKNOW );

    return( pTriState );
}

/*************************************************************************
|*
|*    RscTypCont::InitMessButtons()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscEnum * RscTypCont::InitMessButtons()
{
    RscEnum * pMessButtons;
    pMessButtons = new RscEnum( pHS->getID( "EnumMessButtons" ), RSC_NOTYPE );
    SETCONST( pMessButtons, "WB_OK",                      sal::static_int_cast<UINT32>(WB_OK) );
    SETCONST( pMessButtons, "WB_OK_CANCEL",               sal::static_int_cast<UINT32>(WB_OK_CANCEL) );
    SETCONST( pMessButtons, "WB_YES_NO",                  sal::static_int_cast<UINT32>(WB_YES_NO) );
    SETCONST( pMessButtons, "WB_YES_NO_CANCEL",           sal::static_int_cast<UINT32>(WB_YES_NO_CANCEL) );
    SETCONST( pMessButtons, "WB_RETRY_CANCEL",            sal::static_int_cast<UINT32>(WB_RETRY_CANCEL) );
    SETCONST( pMessButtons, "WB_ABORT_RETRY_IGNORE",      sal::static_int_cast<UINT32>(WB_ABORT_RETRY_IGNORE) );
    return( pMessButtons );
}

/*************************************************************************
|*
|*    RscTypCont::InitMessDefButton()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscEnum * RscTypCont::InitMessDefButton(){
    RscEnum * pMessDefButton;
    pMessDefButton = new RscEnum( pHS->getID( "EnumMessDefButton" ),
                                  RSC_NOTYPE );

    SETCONST( pMessDefButton, "WB_DEF_OK",                  sal::static_int_cast<UINT32>(WB_DEF_OK) );
    SETCONST( pMessDefButton, "WB_DEF_CANCEL",              sal::static_int_cast<UINT32>(WB_DEF_CANCEL) );
    SETCONST( pMessDefButton, "WB_DEF_RETRY",               sal::static_int_cast<UINT32>(WB_DEF_RETRY) );
    SETCONST( pMessDefButton, "WB_DEF_YES",                 sal::static_int_cast<UINT32>(WB_DEF_YES) );
    SETCONST( pMessDefButton, "WB_DEF_NO",                  sal::static_int_cast<UINT32>(WB_DEF_NO) );
    SETCONST( pMessDefButton, "WB_DEF_IGNORE",              sal::static_int_cast<UINT32>(WB_DEF_IGNORE) );
    return( pMessDefButton );
}

/*************************************************************************
|*
|*    RscTypCont::InitGeometry()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscTypCont::InitLangGeometry()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscArray * RscTypCont::InitLangGeometry( RscTupel * pGeo )
{
    return new RscArray( pHS->getID( "Lang_TupelGeometry" ), RSC_NOTYPE, pGeo, &aLangType );
}

/*************************************************************************
|*
|*    RscTypCont::InitStringList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscCont * RscTypCont::InitStringList()
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "Chars[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( &aString );

    return pCont;
}

/*************************************************************************
|*
|*    RscTypCont::InitLangStringList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscArray * RscTypCont::InitLangStringList( RscCont * pStrLst )
{
    return new RscArray( pHS->getID( "Lang_CharsList" ),
                         RSC_NOTYPE, pStrLst, &aLangType );
}

/*************************************************************************
|*
|*    RscTypCont::InitStringTupel()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscTypCont::InitStringLongTupel()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscTypCont::InitStringTupelList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscCont * RscTypCont::InitStringTupelList( RscTupel * pTupelString )
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "CharsCharsTupel[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( pTupelString );

    return pCont;
}

/*************************************************************************
|*
|*    RscTypCont::InitStringLongTupelList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscCont * RscTypCont::InitStringLongTupelList( RscTupel * pStringLong )
{
    RscCont * pCont;

    pCont = new RscCont( pHS->getID( "CharsLongTupel[]" ), RSC_NOTYPE );
    pCont->SetTypeClass( pStringLong );

    return pCont;
}

/*************************************************************************
|*
|*    RscTypCont::InitLangStringTupelList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscArray * RscTypCont::InitLangStringTupelList( RscCont * pStrTupelLst )
{
    return new RscArray( pHS->getID( "Lang_CharsCharsTupel" ),
                    RSC_NOTYPE, pStrTupelLst, &aLangType );
}

/*************************************************************************
|*
|*    RscTypCont::InitLangStringLongTupelList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.05.91
|*    Letzte Aenderung  MM 24.05.91
|*
*************************************************************************/
RscArray * RscTypCont::InitLangStringLongTupelList( RscCont * pStrLongTupelLst )
{
    return new RscArray( pHS->getID( "Lang_CharsLongTupelList" ),
                         RSC_NOTYPE, pStrLongTupelLst, &aLangType );
}

