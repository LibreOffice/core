/*************************************************************************
 *
 *  $RCSfile: lngopt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-11-28 12:21:19 $
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

#include "lngopt.hxx"
#include "lngprops.hxx"
#include "misc.hxx"

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations

#include <cppuhelper/factory.hxx>   // helper for factories
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/Locale.hpp>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using namespace com::sun::star::registry;

///////////////////////////////////////////////////////////////////////////


static const char cLocaleDelim = '-';

INT16 CfgLocaleStrToLanguage( const OUString &rCfgLocaleStr )
{
    INT16 nRes = LANGUAGE_NONE;
    if (rCfgLocaleStr.getLength())
    {
        INT32 nCount = rCfgLocaleStr.getTokenCount( cLocaleDelim );
        if (nCount <= 3)
        {
            OUString aPart[3];
            for (INT32 i = 0;  i < nCount;  ++i)
            {
                aPart[i] = rCfgLocaleStr.getToken( i, cLocaleDelim );
            }
            nRes = LocaleToLanguage( Locale( aPart[0], aPart[1], aPart[2] ) );
        }
        else
            DBG_ERROR( "invalid Locale string" );
    }
    return nRes;
}


const OUString LanguageToCfgLocaleStr( INT16 nLanguage )
{
    static const OUString aLocaleDelim( cLocaleDelim );
    OUString aRes;
    if (LANGUAGE_NONE != nLanguage)
    {
        Locale aLocale( CreateLocale( nLanguage ) );
        if (aLocale.Language.getLength())
            aRes += aLocale.Language;
        if (aLocale.Country.getLength())
            (aRes += aLocaleDelim) += aLocale.Country;
        if (aLocale.Variant.getLength())
            (aRes += aLocaleDelim) += aLocale.Variant;
    }
    return aRes;
}


INT16 CfgAnyToLanguage( const Any &rVal )
{
    OUString aTmp;
    rVal >>= aTmp;
    return CfgLocaleStrToLanguage( aTmp );
}


///////////////////////////////////////////////////////////////////////////


LinguOptionsData::LinguOptionsData()
{
    // get initial language to use (in case that it is not set later)
    nDefaultLanguage = ::GetSystemLanguage();
    if( nDefaultLanguage == LANGUAGE_SYSTEM )
        nDefaultLanguage = System::GetLanguage();

    nDefaultLanguage_CJK = LANGUAGE_NONE;
    nDefaultLanguage_CTL = LANGUAGE_NONE;

    // general options
    bIsGermanPreReform      = FALSE;
    bIsUseDictionaryList    =
    bIsIgnoreControlCharacters  = TRUE;

    // spelling options
    bIsSpellCapitalization  =
    bIsSpellSpecial         = TRUE;
    bIsSpellAuto            =
    bIsSpellInAllLanguages  =
    bIsSpellHideMarkings    =
    bIsSpellReverse         =
    bIsSpellWithDigits      =
    bIsSpellUpperCase       = FALSE;

    // hyphenation options
    bIsHyphSpecial          = TRUE;
    bIsHyphAuto             = FALSE;
    nHyphMinLeading         =
    nHyphMinTrailing        = 2;
    nHyphMinWordLength      = 0;

    // OtherLingu options
    nOtherIndex             = -1;
    bIsStdSpell             =
    bIsStdThes              =
    bIsStdHyph              = FALSE;
    bIsModified             = FALSE;

    aSaveTimer.SetTimeout( 10000 );
    aSaveTimer.SetTimeoutHdl( LINK( this, LinguOptionsData, TimeOut ));

    LoadConfig();
}


Sequence< OUString > LinguOptConfig::GetPropertyNames( INT16 nCfgItem )
{
    static const char * aPropNames1[] =
    {
        "DefaultLocale",                        //  0
        "DictionaryList/ActiveDictionaries",    //  1
        "DictionaryList/IsUseDictionaryList",   //  2
        "IsIgnoreControlCharacters",            //  3
        "IsGermanPreReform",                    //  4
        "DefaultLocale_CJK",                    //  5
        "DefaultLocale_CTL",                        //  6
        0
    };

    static const char * aPropNames2[] =
    {
        "IsSpellUpperCase",             //  5
        "IsSpellWithDigits",                //  6
        "IsSpellCapitalization",            //  7
        "IsSpellAuto",                  //  8
        "IsSpellSpecial",                   //  9
        "IsSpellInAllLocales",          // 10
        "IsHideMarkings",                   // 11
        "IsReverseDirection",               // 12
        0
    };

    static const char * aPropNames3[] =
    {
        "MinLeading",                       // 13
        "MinTrailing",                      // 14
        "MinWordLength",                    // 15
        "IsHyphSpecial",                    // 16
        "IsHyphAuto",                       // 17
        0
    };

    static const char * aPropNames4[] =
    {
        "OtherLinguIndex",           // 18
        "IsUseStandardSpellChecker",    // 19
        "IsUseStandardHyphenator",  // 20
        "IsUseStandardThesaurus",   // 21
        0
    };

    static const char** aPropNames[ 4 ] = {
            aPropNames1, aPropNames2, aPropNames3, aPropNames4 };

    const char** ppPropName = aPropNames[ nCfgItem ];

    Sequence< OUString > aNames( 10 );
    OUString *pNames = aNames.getArray();
    for( INT32 i = 0; *ppPropName;  ++i, ++ppPropName )
    {
        pNames[i] = A2OU( *ppPropName );
    }
    aNames.realloc( i );
    return aNames;
}

static const char* aRootNames[ 4 ] = {
                "Office.Linguistic/General",
                "Office.Linguistic/SpellChecking",
                "Office.Linguistic/Hyphenation",
                "Office.Linguistic/ExternalLinguistic" };



BOOL LinguOptionsData::LoadConfig()
{
    BOOL bRes = FALSE;

    for( INT16 nCfgItem = 0; nCfgItem < 4; ++nCfgItem )
    {
        LinguOptConfig aCfg( String::CreateFromAscii( aRootNames[ nCfgItem ]));
        Sequence< OUString > aNames = aCfg.GetPropertyNames( nCfgItem );
        INT32 nProps = aNames.getLength();


        const Sequence< Any > aValues = aCfg.GetProperties( aNames );

        if (nProps  &&  aValues.getLength() == nProps)
        {
            const Any *pValue = aValues.getConstArray();
            INT16 nCfgOff = 10 * nCfgItem;
            for (INT32 i = 0;  i < nProps;  ++i)
            {
                const Any &rVal = pValue[i];
                if( rVal.hasValue() )
                    switch ( i + nCfgOff )
                    {
                        case  0: nDefaultLanguage = CfgAnyToLanguage( rVal );   break;
                        case  1: rVal >>= aActiveDics;  break;
                        case  2: rVal >>= bIsUseDictionaryList; break;
                        case  3: rVal >>= bIsIgnoreControlCharacters;   break;
                        case  4: rVal >>= bIsGermanPreReform;   break;
                        case  5: nDefaultLanguage_CJK = CfgAnyToLanguage( rVal );   break;
                        case  6: nDefaultLanguage_CTL = CfgAnyToLanguage( rVal );   break;

                        case 10: rVal >>= bIsSpellUpperCase;    break;
                        case 11: rVal >>= bIsSpellWithDigits;   break;
                        case 12: rVal >>= bIsSpellCapitalization;   break;
                        case 13: rVal >>= bIsSpellAuto; break;
                        case 14: rVal >>= bIsSpellSpecial;  break;
                        case 15: rVal >>= bIsSpellInAllLanguages;   break;
                        case 16: rVal >>= bIsSpellHideMarkings; break;
                        case 17: rVal >>= bIsSpellReverse;  break;

                        case 20: rVal >>= nHyphMinLeading;  break;
                        case 21: rVal >>= nHyphMinTrailing; break;
                        case 22: rVal >>= nHyphMinWordLength;   break;
                        case 23: rVal >>= bIsHyphSpecial;   break;
                        case 24: rVal >>= bIsHyphAuto;  break;

                        case 30: rVal >>= nOtherIndex;  break;
                        case 31: rVal >>= bIsStdSpell;  break;
                        case 32: rVal >>= bIsStdHyph;   break;
                        case 33: rVal >>= bIsStdThes;   break;

                        default:
                            DBG_ERROR( "unexpected case" );
                    }
            }
        }

        bRes = TRUE;
    }
    DBG_ASSERT( bRes, "LoadConfig failed" );

    return bRes;
}


BOOL LinguOptionsData::SaveConfig()
{
    if( !bIsModified )
        return TRUE;

    BOOL bRet = FALSE;
    const Type &rBOOL     = ::getBooleanCppuType();
    const Type &rINT16    = ::getCppuType( (INT16 *) NULL );

    for( INT16 nCfgItem = 0; nCfgItem < 4; ++nCfgItem )
    {
        LinguOptConfig aCfg( String::CreateFromAscii( aRootNames[ nCfgItem ]));
        const Sequence< OUString > aNames = aCfg.GetPropertyNames( nCfgItem );
        INT32 nProps = aNames.getLength();
        Sequence< Any > aValues( aNames.getLength() );
        Any *pValue = aValues.getArray();

         switch( nCfgItem )
        {
        case 0:
            {
                OUString aTmp( LanguageToCfgLocaleStr( nDefaultLanguage ) );
                *pValue++ = makeAny( aTmp );
                *pValue++ = makeAny( aActiveDics );
                pValue->setValue( &bIsUseDictionaryList, rBOOL );
                ++pValue;
                pValue->setValue( &bIsIgnoreControlCharacters, rBOOL );
                ++pValue;
                pValue->setValue( &bIsGermanPreReform, rBOOL );
                ++pValue;

                aTmp = LanguageToCfgLocaleStr( nDefaultLanguage_CJK );
                *pValue++ = makeAny( aTmp );

                aTmp = LanguageToCfgLocaleStr( nDefaultLanguage_CTL );
                *pValue++ = makeAny( aTmp );
            }
            break;
        case 1:
            pValue->setValue( &bIsSpellUpperCase, rBOOL );      ++pValue;
            pValue->setValue( &bIsSpellWithDigits, rBOOL );     ++pValue;
            pValue->setValue( &bIsSpellCapitalization, rBOOL ); ++pValue;
            pValue->setValue( &bIsSpellAuto, rBOOL );           ++pValue;
            pValue->setValue( &bIsSpellSpecial, rBOOL );        ++pValue;
            pValue->setValue( &bIsSpellInAllLanguages, rBOOL ); ++pValue;
            pValue->setValue( &bIsSpellHideMarkings, rBOOL );   ++pValue;
            pValue->setValue( &bIsSpellReverse, rBOOL );        ++pValue;
            break;
        case 2:
            pValue->setValue( &nHyphMinLeading, rINT16 );   ++pValue;
            pValue->setValue( &nHyphMinTrailing, rINT16 );  ++pValue;
            pValue->setValue( &nHyphMinWordLength, rINT16 );++pValue;
            pValue->setValue( &bIsHyphSpecial, rBOOL );     ++pValue;
            pValue->setValue( &bIsHyphAuto, rBOOL );        ++pValue;
            break;
        case 3:
            pValue->setValue( &nOtherIndex, rINT16 );       ++pValue;
            pValue->setValue( &bIsStdSpell, rBOOL );        ++pValue;
            pValue->setValue( &bIsStdHyph, rBOOL );         ++pValue;
            pValue->setValue( &bIsStdThes, rBOOL );         ++pValue;
            break;
        }

        bRet |= aCfg.PutProperties( aNames, aValues );
    }

    if( bRet )
        bIsModified = FALSE;

    return bRet;
}

IMPL_LINK( LinguOptionsData, TimeOut, Timer*, p )
{
    SaveConfig();
    return 0;
}

/*
void LinguOptions::LinguOptionsData::Notify( const Sequence< OUString >& rPropertyNames )
{
    DBG_ERROR("properties have been changed");
}


void LinguOptions::LinguOptionsData::Commit()
{
    SaveConfig();
}
*/

///////////////////////////////////////////////////////////////////////////

// static member intialization
LinguOptionsData *  LinguOptions::pData = NULL;


LinguOptions::LinguOptions()
{
    if (!pData)
    {
        pData = new LinguOptionsData;
    }

    ++pData->aRefCount;
}


LinguOptions::LinguOptions(const LinguOptions &rOpt)
{
    DBG_ASSERT( pData, "lng : data missing" );
    ++pData->aRefCount;
}


LinguOptions::~LinguOptions()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (--pData->aRefCount == 0)
    {
        delete pData;   pData  = NULL;
    }
}


BOOL LinguOptions::SetLocale_Impl( INT16 &rLanguage, Any &rOld, const Any &rVal)
{
    BOOL bRes = FALSE;

    Locale  aNew;
    rVal >>= aNew;
    INT16 nNew = LocaleToLanguage( aNew );
    if (nNew != rLanguage)
    {
        Locale  aLocale( CreateLocale( rLanguage ) );
        rOld.setValue( &aLocale, ::getCppuType((Locale*)0 ));
        rLanguage = nNew;
        bRes = TRUE;
    }

    return bRes;
}


BOOL LinguOptions::SetValue( Any &rOld, const Any &rVal, INT32 nWID )
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;

    INT16 *pnVal = 0;
    BOOL  *pbVal = 0;

    switch( nWID )
    {
        case WID_IS_GERMAN_PRE_REFORM :     pbVal = &pData->bIsGermanPreReform; break;
        case WID_IS_USE_DICTIONARY_LIST :   pbVal = &pData->bIsUseDictionaryList;   break;
        case WID_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &pData->bIsIgnoreControlCharacters; break;
        case WID_IS_HYPH_AUTO :             pbVal = &pData->bIsHyphAuto;    break;
        case WID_IS_HYPH_SPECIAL :          pbVal = &pData->bIsHyphSpecial; break;
        case WID_IS_SPELL_AUTO :            pbVal = &pData->bIsSpellAuto;   break;
        case WID_IS_SPELL_HIDE :            pbVal = &pData->bIsSpellHideMarkings;   break;
        case WID_IS_SPELL_IN_ALL_LANGUAGES :pbVal = &pData->bIsSpellInAllLanguages; break;
        case WID_IS_SPELL_SPECIAL :         pbVal = &pData->bIsSpellSpecial;    break;
        case WID_IS_WRAP_REVERSE :          pbVal = &pData->bIsSpellReverse;    break;
        case WID_DEFAULT_LANGUAGE :         pnVal = &pData->nDefaultLanguage;   break;
        case WID_IS_STANDARD_HYPHENATOR :   pbVal = &pData->bIsStdHyph; break;
        case WID_IS_STANDARD_SPELL_CHECKER :pbVal = &pData->bIsStdSpell;    break;
        case WID_IS_STANDARD_THESAURUS :    pbVal = &pData->bIsStdThes; break;
        case WID_OTHER_LINGU_INDEX :        pnVal = &pData->nOtherIndex;    break;
        case WID_IS_SPELL_CAPITALIZATION :  pbVal = &pData->bIsSpellCapitalization;     break;
        case WID_IS_SPELL_WITH_DIGITS :     pbVal = &pData->bIsSpellWithDigits; break;
        case WID_IS_SPELL_UPPER_CASE :      pbVal = &pData->bIsSpellUpperCase;      break;
        case WID_HYPH_MIN_LEADING :         pnVal = &pData->nHyphMinLeading;        break;
        case WID_HYPH_MIN_TRAILING :        pnVal = &pData->nHyphMinTrailing;   break;
        case WID_HYPH_MIN_WORD_LENGTH :     pnVal = &pData->nHyphMinWordLength; break;
        case WID_DEFAULT_LOCALE :
        {
            bRes = SetLocale_Impl( pData->nDefaultLanguage, rOld, rVal );
            break;
        }
        case WID_DEFAULT_LOCALE_CJK :
        {
            bRes = SetLocale_Impl( pData->nDefaultLanguage_CJK, rOld, rVal );
            break;
        }
        case WID_DEFAULT_LOCALE_CTL :
        {
            bRes = SetLocale_Impl( pData->nDefaultLanguage_CTL, rOld, rVal );
            break;
        }
        default :
        {
            DBG_ERROR("lng : unknown WID");
            bRes = FALSE;
        }
    }

    if (pbVal)
    {
        BOOL bNew;
        rVal >>= bNew;
        if (bNew != *pbVal)
        {
            rOld <<= *pbVal;
            *pbVal = bNew;
            bRes = TRUE;
        }
    }
    if (pnVal)
    {
        INT16 nNew;
        rVal >>= nNew;
        if (nNew != *pnVal)
        {
            rOld <<= *pnVal;
            *pnVal = nNew;
            bRes = TRUE;
        }
    }

    if (bRes)
        pData->SetModified();

    return bRes;
}

void LinguOptions::GetValue( Any &rVal, INT32 nWID ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT16 *pnVal = 0;
    BOOL  *pbVal = 0;

    switch( nWID )
    {
        case WID_IS_GERMAN_PRE_REFORM :     pbVal = &pData->bIsGermanPreReform; break;
        case WID_IS_USE_DICTIONARY_LIST :   pbVal = &pData->bIsUseDictionaryList;   break;
        case WID_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &pData->bIsIgnoreControlCharacters; break;
        case WID_IS_HYPH_AUTO :             pbVal = &pData->bIsHyphAuto;    break;
        case WID_IS_HYPH_SPECIAL :          pbVal = &pData->bIsHyphSpecial; break;
        case WID_IS_SPELL_AUTO :            pbVal = &pData->bIsSpellAuto;   break;
        case WID_IS_SPELL_HIDE :            pbVal = &pData->bIsSpellHideMarkings;   break;
        case WID_IS_SPELL_IN_ALL_LANGUAGES :pbVal = &pData->bIsSpellInAllLanguages; break;
        case WID_IS_SPELL_SPECIAL :         pbVal = &pData->bIsSpellSpecial;    break;
        case WID_IS_WRAP_REVERSE :          pbVal = &pData->bIsSpellReverse;    break;
        case WID_DEFAULT_LANGUAGE :         pnVal = &pData->nDefaultLanguage;   break;
        case WID_IS_STANDARD_HYPHENATOR :   pbVal = &pData->bIsStdHyph; break;
        case WID_IS_STANDARD_SPELL_CHECKER :pbVal = &pData->bIsStdSpell;    break;
        case WID_IS_STANDARD_THESAURUS :    pbVal = &pData->bIsStdThes; break;
        case WID_OTHER_LINGU_INDEX :        pnVal = &pData->nOtherIndex;    break;
        case WID_IS_SPELL_CAPITALIZATION :  pbVal = &pData->bIsSpellCapitalization;     break;
        case WID_IS_SPELL_WITH_DIGITS :     pbVal = &pData->bIsSpellWithDigits; break;
        case WID_IS_SPELL_UPPER_CASE :      pbVal = &pData->bIsSpellUpperCase;      break;
        case WID_HYPH_MIN_LEADING :         pnVal = &pData->nHyphMinLeading;        break;
        case WID_HYPH_MIN_TRAILING :        pnVal = &pData->nHyphMinTrailing;   break;
        case WID_HYPH_MIN_WORD_LENGTH :     pnVal = &pData->nHyphMinWordLength; break;
        case WID_DEFAULT_LOCALE :
        {
            Locale aLocale( CreateLocale( pData->nDefaultLanguage ) );
            rVal.setValue( &aLocale, ::getCppuType((Locale*)0 ));
            break;
        }
        case WID_DEFAULT_LOCALE_CJK :
        {
            Locale aLocale( CreateLocale( pData->nDefaultLanguage_CJK ) );
            rVal.setValue( &aLocale, ::getCppuType((Locale*)0 ));
            break;
        }
        case WID_DEFAULT_LOCALE_CTL :
        {
            Locale aLocale( CreateLocale( pData->nDefaultLanguage_CTL ) );
            rVal.setValue( &aLocale, ::getCppuType((Locale*)0 ));
            break;
        }
        default :
        {
            DBG_ERROR("lng : unknown WID");
        }
    }

    if (pbVal)
        rVal <<= *pbVal;
    if (pnVal)
        rVal <<= *pnVal;
}


struct WID_Name
{
    INT32        nWID;
    const char  *pPropertyName;
};

//! order of entries is import (see LinguOptions::GetName)
//! since the WID is used as index in this table!
WID_Name aWID_Name[] =
{
    WID_IS_GERMAN_PRE_REFORM,           UPN_IS_GERMAN_PRE_REFORM,
    WID_IS_USE_DICTIONARY_LIST,         UPN_IS_USE_DICTIONARY_LIST,
    WID_IS_IGNORE_CONTROL_CHARACTERS,   UPN_IS_IGNORE_CONTROL_CHARACTERS,
    WID_IS_SPELL_UPPER_CASE,            UPN_IS_SPELL_UPPER_CASE,
    WID_IS_SPELL_WITH_DIGITS,           UPN_IS_SPELL_WITH_DIGITS,
    WID_IS_SPELL_CAPITALIZATION,        UPN_IS_SPELL_CAPITALIZATION,
    WID_HYPH_MIN_LEADING,               UPN_HYPH_MIN_LEADING,
    WID_HYPH_MIN_TRAILING,              UPN_HYPH_MIN_TRAILING,
    WID_HYPH_MIN_WORD_LENGTH,           UPN_HYPH_MIN_WORD_LENGTH,
    WID_DEFAULT_LOCALE,                 UPN_DEFAULT_LOCALE,
    WID_IS_SPELL_AUTO,                  UPN_IS_SPELL_AUTO,
    WID_IS_SPELL_HIDE,                  UPN_IS_SPELL_HIDE,
    WID_IS_SPELL_IN_ALL_LANGUAGES,      UPN_IS_SPELL_IN_ALL_LANGUAGES,
    WID_IS_SPELL_SPECIAL,               UPN_IS_SPELL_SPECIAL,
    WID_IS_HYPH_AUTO,                   UPN_IS_HYPH_AUTO,
    WID_IS_HYPH_SPECIAL,                UPN_IS_HYPH_SPECIAL,
    WID_IS_WRAP_REVERSE,                UPN_IS_WRAP_REVERSE,
    WID_IS_STANDARD_HYPHENATOR,         UPN_IS_STANDARD_HYPHENATOR,
    WID_IS_STANDARD_SPELL_CHECKER,      UPN_IS_STANDARD_SPELL_CHECKER,
    WID_IS_STANDARD_THESAURUS,          UPN_IS_STANDARD_THESAURUS,
    WID_OTHER_LINGU_INDEX,              UPN_OTHER_LINGU_INDEX,
    WID_DEFAULT_LANGUAGE,               UPN_DEFAULT_LANGUAGE,
    WID_DEFAULT_LOCALE_CJK,             UPN_DEFAULT_LOCALE_CJK,
    WID_DEFAULT_LOCALE_CTL,             UPN_DEFAULT_LOCALE_CTL
};


OUString LinguOptions::GetName( INT32 nWID ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    OUString aRes;

    INT32 nLen = sizeof( aWID_Name ) / sizeof( aWID_Name[0] );
    if (0 <= nWID  &&  nWID < nLen
        && aWID_Name[ nWID ].nWID == nWID)
    {
        aRes = OUString( RTL_CONSTASCII_USTRINGPARAM(
                aWID_Name[ nWID ].pPropertyName ) );
    }
    else
    {
        DBG_ERROR("lng : unknown WID");
    }

    return aRes;
}


void LinguOptions::SetCfgActiveDictionaries(
            Reference< XDictionaryList > &rDicList )
{
    if (rDicList.is())
    {
        Sequence< Reference< XDictionary > > aDics( rDicList->getDictionaries() );
        const Reference< XDictionary > *pDic = aDics.getConstArray();
        INT32 nCount = aDics.getLength();

        pData->aActiveDics.realloc( nCount );
        OUString *pActiveDic = pData->aActiveDics.getArray();
        INT32 nLen = 0;
        for (INT32 i = 0;  i < nCount;  ++i)
        {
            const Reference< XDictionary > &rDic = pDic[i];
            if (rDic.is()  &&  rDic->isActive())
                pActiveDic[ nLen++ ] = rDic->getName();
        }
        pData->aActiveDics.realloc( nLen );

        pData->SetModified();
    }
}


BOOL LinguOptions::Load()
{
    BOOL bRes = FALSE;
    DBG_ASSERT( pData, "NULL Pointer" );
    if (pData)
        bRes = pData->LoadConfig();
    return bRes;
}


BOOL LinguOptions::Save()
{
    BOOL bRes = FALSE;
    DBG_ASSERT( pData, "NULL Pointer" );
    if (pData)
        bRes = pData->SaveConfig();
    return bRes;
}

///////////////////////////////////////////////////////////////////////////

void LinguProps::MyAppExitListener::AtExit()
{
    // save PropertySet upon application exit
    rMyOpt.Save();
}

//! map must be sorted by first entry in alphabetical increasing order.

static SfxItemPropertyMap aLinguProps[] =
{
    { MAP_CHAR_LEN(UPN_DEFAULT_LANGUAGE),           WID_DEFAULT_LANGUAGE,
            &::getCppuType( (sal_Int16*)0 ),    0, 0 },
    { MAP_CHAR_LEN(UPN_DEFAULT_LOCALE),             WID_DEFAULT_LOCALE,
            &::getCppuType( (Locale* )0),       0, 0 },
    { MAP_CHAR_LEN(UPN_DEFAULT_LOCALE_CJK),         WID_DEFAULT_LOCALE,
            &::getCppuType( (Locale* )0),       0, 0 },
    { MAP_CHAR_LEN(UPN_DEFAULT_LOCALE_CTL),         WID_DEFAULT_LOCALE,
            &::getCppuType( (Locale* )0),       0, 0 },
    { MAP_CHAR_LEN(UPN_HYPH_MIN_LEADING),           WID_HYPH_MIN_LEADING,
            &::getCppuType( (sal_Int16*)0 ),    0, 0 },
    { MAP_CHAR_LEN(UPN_HYPH_MIN_TRAILING),          WID_HYPH_MIN_TRAILING,
            &::getCppuType( (sal_Int16*)0 ),    0, 0 },
    { MAP_CHAR_LEN(UPN_HYPH_MIN_WORD_LENGTH),       WID_HYPH_MIN_WORD_LENGTH,
            &::getCppuType( (sal_Int16*)0 ),    0, 0 },
    { MAP_CHAR_LEN(UPN_IS_GERMAN_PRE_REFORM),       WID_IS_GERMAN_PRE_REFORM,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_HYPH_AUTO),               WID_IS_HYPH_AUTO,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_HYPH_SPECIAL),            WID_IS_HYPH_SPECIAL,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_IGNORE_CONTROL_CHARACTERS),   WID_IS_IGNORE_CONTROL_CHARACTERS,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_AUTO),              WID_IS_SPELL_AUTO,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_CAPITALIZATION),    WID_IS_SPELL_CAPITALIZATION,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_HIDE),              WID_IS_SPELL_HIDE,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_IN_ALL_LANGUAGES),  WID_IS_SPELL_IN_ALL_LANGUAGES,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_SPECIAL),           WID_IS_SPELL_SPECIAL,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_UPPER_CASE),        WID_IS_SPELL_UPPER_CASE,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_SPELL_WITH_DIGITS),       WID_IS_SPELL_WITH_DIGITS,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_STANDARD_HYPHENATOR),     WID_IS_STANDARD_HYPHENATOR,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_STANDARD_SPELL_CHECKER),  WID_IS_STANDARD_SPELL_CHECKER,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_STANDARD_THESAURUS),      WID_IS_STANDARD_THESAURUS,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_USE_DICTIONARY_LIST),     WID_IS_USE_DICTIONARY_LIST,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_IS_WRAP_REVERSE),            WID_IS_WRAP_REVERSE,
            &::getBooleanCppuType(),            0, 0 },
    { MAP_CHAR_LEN(UPN_OTHER_LINGU_INDEX),          WID_OTHER_LINGU_INDEX,
            &::getCppuType( (sal_Int16*)0 ),    0, 0 },
    { 0,0,0,0 }
};

LinguProps::LinguProps() :
    aEvtListeners   (GetLinguMutex()),
    aPropListeners  (GetLinguMutex()),
    pMap            (aLinguProps)
{
    bDisposing = FALSE;
    pExitListener = new MyAppExitListener( aOpt );
    xExitListener = pExitListener;
    pExitListener->Activate();
}

LinguProps::~LinguProps()
{
    pExitListener->Deactivate();
}

void LinguProps::launchEvent( const PropertyChangeEvent &rEvt ) const
{
    cppu::OInterfaceContainerHelper *pContainer =
            aPropListeners.getContainer( rEvt.PropertyHandle );
    if (pContainer)
    {
        cppu::OInterfaceIteratorHelper aIt( *pContainer );
        while (aIt.hasMoreElements())
        {
            Reference< XPropertyChangeListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->propertyChange( rEvt );
        }
    }
}

Reference< XInterface > SAL_CALL LinguProps_CreateInstance(
            const Reference< XMultiServiceFactory > & rSMgr )
        throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*)new LinguProps;
    return xService;
}

Reference< XPropertySetInfo > SAL_CALL LinguProps::getPropertySetInfo()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    static Reference< XPropertySetInfo > aRef =
            new SfxItemPropertySetInfo( pMap );
    return aRef;
}

void SAL_CALL LinguProps::setPropertyValue(
            const OUString& rPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    const SfxItemPropertyMap* pCur =
            SfxItemPropertyMap::GetByName(pMap, rPropertyName);
    if (pCur)
    {
        Any aOld;
        if (aOpt.SetValue( aOld, aValue, pCur->nWID ))
        {
            PropertyChangeEvent aChgEvt( (XPropertySet *) this, rPropertyName,
                    FALSE, pCur->nWID, aOld, aValue );
            launchEvent( aChgEvt );
        }
    }
#ifdef LINGU_EXCEPTIONS
    else
    {
        throw UnknownPropertyException();
    }
#endif
}

Any SAL_CALL LinguProps::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRet;

    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(pMap, rPropertyName);
    if(pCur)
    {
        aOpt.GetValue( aRet, pCur->nWID );
    }
#ifdef LINGU_EXCEPTIONS
    else
    {
        throw UnknownPropertyException();
    }
#endif

    return aRet;
}

void SAL_CALL LinguProps::addPropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertyMap* pCur =
                SfxItemPropertyMap::GetByName( pMap, rPropertyName );
        if(pCur)
            aPropListeners.addInterface( pCur->nWID, rxListener );
#ifdef LINGU_EXCEPTIONS
        else
        {
            throw UnknownPropertyException();
        }
#endif
    }
}

void SAL_CALL LinguProps::removePropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertyMap* pCur =
                SfxItemPropertyMap::GetByName( pMap, rPropertyName );
        if(pCur)
            aPropListeners.removeInterface( pCur->nWID, rxListener );
#ifdef LINGU_EXCEPTIONS
        else
        {
            throw UnknownPropertyException();
        }
#endif
    }
}

void SAL_CALL LinguProps::addVetoableChangeListener(
            const OUString& PropertyName,
            const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
//  MutexGuard  aGuard( GetLinguMutex() );
}

void SAL_CALL LinguProps::removeVetoableChangeListener(
            const OUString& PropertyName,
            const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
//  MutexGuard  aGuard( GetLinguMutex() );
}


void SAL_CALL LinguProps::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aOld;
    if (aOpt.SetValue( aOld, rValue, nHandle ))
    {
        PropertyChangeEvent aChgEvt( (XPropertySet *) this,
                aOpt.GetName( nHandle ), FALSE, nHandle, aOld, rValue );
        launchEvent( aChgEvt );
    }
}


Any SAL_CALL LinguProps::getFastPropertyValue( sal_Int32 nHandle )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRes;
    aOpt.GetValue( aRes, nHandle );
    return aRes;
}


Sequence< PropertyValue > SAL_CALL
    LinguProps::getPropertyValues()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nLen = sizeof (aLinguProps) / sizeof aLinguProps[0] - 1;
    Sequence< PropertyValue > aProps( nLen );
    PropertyValue *pProp = aProps.getArray();
    for (INT32 i = 0;  i < nLen;  i++)
    {
        PropertyValue &rVal = pProp[i];
        SfxItemPropertyMap &rItem = aLinguProps[i];
        Any aAny;
        aOpt.GetValue( aAny, rItem.nWID );

        rVal.Name   = OUString( rItem.pName, rItem.nNameLen, GetTextEncoding() );
        rVal.Handle = rItem.nWID;
        rVal.Value  = aAny;
        rVal.State  = PropertyState_DIRECT_VALUE ;
    }
    return aProps;
}

void SAL_CALL
    LinguProps::setPropertyValues( const Sequence< PropertyValue >& rProps )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nLen = rProps.getLength();
    const PropertyValue *pVal = rProps.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        Any aOld;
        const PropertyValue &rVal = pVal[i];
        if (aOpt.SetValue( aOld, rVal.Value, rVal.Handle ))
        {
            PropertyChangeEvent aChgEvt( (XPropertySet *) this,
                    rVal.Name, FALSE, rVal.Handle, aOld, rVal.Value );
            launchEvent( aChgEvt );
        }
    }
}

void SAL_CALL
    LinguProps::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;

        //! its too late to save the options here!
        // (see AppExitListener for saving)
        //aOpt.Save();  // save (possible) changes before exiting

        EventObject aEvtObj( (XPropertySet *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        aPropListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL
    LinguProps::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    LinguProps::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


///////////////////////////////////////////////////////////////////////////
// Service specific part
//

// XServiceInfo
OUString SAL_CALL LinguProps::getImplementationName()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL LinguProps::supportsService( const OUString& ServiceName )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL LinguProps::getSupportedServiceNames()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
uno::Sequence< OUString > LinguProps::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNS( 1 );    // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( SN_LINGU_PROPERTIES );
    return aSNS;
}


sal_Bool SAL_CALL LinguProps_writeInfo( void * /*pServiceManager*/,
            XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += LinguProps::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< XRegistryKey > xNewKey =
            pRegistryKey->createKey(aImpl );
        uno::Sequence< OUString > aServices = LinguProps::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i]);

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}

void * SAL_CALL LinguProps_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory *pServiceManager, void * )
{
    void * pRet = 0;
    if ( !LinguProps::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                LinguProps::getImplementationName_Static(),
                LinguProps_CreateInstance,
                LinguProps::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

///////////////////////////////////////////////////////////////////////////

