/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lingucfg.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:46:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"


#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include "com/sun/star/util/XMacroExpander.hpp"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include "com/sun/star/beans/XPropertySet.hpp"
#endif

#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <linguprops.hxx>
#endif

#include <comphelper/processfactory.hxx>

#include <itemholder1.hxx>

using namespace rtl;
using namespace com::sun::star;

#define A2OU(x)        ::rtl::OUString::createFromAscii( x )
#define EXPAND_PROTOCOL     "vnd.sun.star.expand:"
#define FILE_PROTOCOL       "file:///"

///////////////////////////////////////////////////////////////////////////


static osl::Mutex &  GetOwnMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}


///////////////////////////////////////////////////////////////////////////


static lang::Locale lcl_CreateLocale( LanguageType eLang )
{
    lang::Locale aLocale;
    if ( eLang != LANGUAGE_NONE )
        MsLangId::convertLanguageToLocale( eLang, aLocale );

    return aLocale;
}


static INT16 lcl_LocaleToLanguage( const lang::Locale& rLocale )
{
    //  empty Locale -> LANGUAGE_NONE
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_NONE;

    //  Variant of Locale is ignored
    return MsLangId::convertLocaleToLanguage( rLocale );
}


static BOOL lcl_SetLocale( INT16 &rLanguage, const uno::Any &rVal )
{
    BOOL bSucc = FALSE;

    lang::Locale aNew;
    if (rVal >>= aNew)  // conversion successful?
    {
        INT16 nNew = lcl_LocaleToLanguage( aNew );
        if (nNew != rLanguage)
        {
            rLanguage = nNew;
            bSucc = TRUE;
        }
    }

    return bSucc;
}


static inline INT16 lcl_CfgLocaleStrToLanguage( const OUString &rCfgLocaleStr )
{
    INT16 nRes = LANGUAGE_NONE;
    if (0 != rCfgLocaleStr.getLength())
        nRes = MsLangId::convertIsoStringToLanguage( rCfgLocaleStr );
    return nRes;
}


static inline const OUString lcl_LanguageToCfgLocaleStr( INT16 nLanguage )
{
    OUString aRes;
    if (LANGUAGE_NONE != nLanguage)
        aRes = MsLangId::convertLanguageToIsoString( nLanguage );
    return aRes;
}


static void lcl_CfgAnyToLanguage( const uno::Any &rVal, INT16& rLanguage )
{
    OUString aTmp;
    if ((rVal >>= aTmp)  &&  0 != aTmp.getLength())
       rLanguage = MsLangId::convertIsoStringToLanguage( aTmp );
}


//////////////////////////////////////////////////////////////////////

SvtLinguOptions::SvtLinguOptions()
{
    nDefaultLanguage = LANGUAGE_NONE;

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

    // text conversion options
    bIsIgnorePostPositionalWord     = TRUE;
    bIsAutoCloseDialog              =
    bIsShowEntriesRecentlyUsedFirst =
    bIsAutoReplaceUniqueEntries     = FALSE;
    bIsDirectionToSimplified        = TRUE;
    bIsUseCharacterVariants         =
    bIsTranslateCommonTerms         =
    bIsReverseMapping               = FALSE;

    bROIsDirectionToSimplified      =
    bROIsUseCharacterVariants       =
    bROIsTranslateCommonTerms       =
    bROIsReverseMapping             = FALSE;

    // hyphenation options
    bIsHyphSpecial          = TRUE;
    bIsHyphAuto             = FALSE;
    nHyphMinLeading         =
    nHyphMinTrailing        = 2;
    nHyphMinWordLength      = 0;

    nDataFilesChangedCheckValue = 0;
}


//////////////////////////////////////////////////////////////////////


class SvtLinguConfigItem : public utl::ConfigItem
{
    SvtLinguOptions     aOpt;

    // disallow copy-constructor and assignment-operator for now
    SvtLinguConfigItem( const SvtLinguConfigItem & );
    SvtLinguConfigItem & operator = ( const SvtLinguConfigItem & );

    static BOOL GetHdlByName( INT32 &rnHdl, const OUString &rPropertyName, sal_Bool bFullPropName = sal_False );
    static const uno::Sequence< OUString > & GetPropertyNames();
    BOOL                LoadOptions( const uno::Sequence< OUString > &rProperyNames );
    BOOL                SaveOptions( const uno::Sequence< OUString > &rProperyNames );

public:
    SvtLinguConfigItem();
    virtual ~SvtLinguConfigItem();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

    // make some protected functions of utl::ConfigItem public
    using utl::ConfigItem::GetNodeNames;
    using utl::ConfigItem::GetProperties;
    //using utl::ConfigItem::PutProperties;
    //using utl::ConfigItem::SetSetProperties;
    using utl::ConfigItem::ReplaceSetProperties;
    //using utl::ConfigItem::GetReadOnlyStates;


    com::sun::star::uno::Any
            GetProperty( const rtl::OUString &rPropertyName ) const;
    com::sun::star::uno::Any
            GetProperty( INT32 nPropertyHandle ) const;

    BOOL    SetProperty( const rtl::OUString &rPropertyName,
                         const com::sun::star::uno::Any &rValue );
    BOOL    SetProperty( INT32 nPropertyHandle,
                         const com::sun::star::uno::Any &rValue );

    BOOL    GetOptions( SvtLinguOptions &rOptions ) const;
    BOOL    SetOptions( const SvtLinguOptions &rOptions );

    BOOL    IsReadOnly( const rtl::OUString &rPropertyName ) const;
    BOOL    IsReadOnly( INT32 nPropertyHandle ) const;
};


SvtLinguConfigItem::SvtLinguConfigItem() :
    utl::ConfigItem( String::CreateFromAscii( "Office.Linguistic" ) )
{
    LoadOptions( GetPropertyNames() );
    ClearModified();

    // request notify events when properties change
    EnableNotification( GetPropertyNames() );
}


SvtLinguConfigItem::~SvtLinguConfigItem()
{
    //! Commit (SaveOptions) will be called by the d-tor of the base called !
}


void SvtLinguConfigItem::Notify( const uno::Sequence< OUString > &rPropertyNames )
{
    LoadOptions( rPropertyNames );
}


void SvtLinguConfigItem::Commit()
{
    SaveOptions( GetPropertyNames() );
}


static struct NamesToHdl
{
    const char   *pFullPropName;      // full qualified name as used in configuration
    const char   *pPropName;          // property name only (atom) of above
    INT32   nHdl;               // numeric handle representing the property
}aNamesToHdl[] =
{
{/*  0 */    "General/DefaultLocale",                         UPN_DEFAULT_LOCALE,                    UPH_DEFAULT_LOCALE},
{/*  1 */    "General/DictionaryList/ActiveDictionaries",     UPN_ACTIVE_DICTIONARIES,               UPH_ACTIVE_DICTIONARIES},
{/*  2 */    "General/DictionaryList/IsUseDictionaryList",    UPN_IS_USE_DICTIONARY_LIST,            UPH_IS_USE_DICTIONARY_LIST},
{/*  3 */    "General/IsIgnoreControlCharacters",             UPN_IS_IGNORE_CONTROL_CHARACTERS,      UPH_IS_IGNORE_CONTROL_CHARACTERS},
{/*  4 */    "General/IsGermanPreReform",                     UPN_IS_GERMAN_PRE_REFORM,              UPH_IS_GERMAN_PRE_REFORM},
{/*  5 */    "General/DefaultLocale_CJK",                     UPN_DEFAULT_LOCALE_CJK,                UPH_DEFAULT_LOCALE_CJK},
{/*  6 */    "General/DefaultLocale_CTL",                     UPN_DEFAULT_LOCALE_CTL,                UPH_DEFAULT_LOCALE_CTL},

{/*  7 */    "SpellChecking/IsSpellUpperCase",                UPN_IS_SPELL_UPPER_CASE,               UPH_IS_SPELL_UPPER_CASE},
{/*  8 */    "SpellChecking/IsSpellWithDigits",               UPN_IS_SPELL_WITH_DIGITS,              UPH_IS_SPELL_WITH_DIGITS},
{/*  9 */    "SpellChecking/IsSpellCapitalization",           UPN_IS_SPELL_CAPITALIZATION,           UPH_IS_SPELL_CAPITALIZATION},
{/* 10 */    "SpellChecking/IsSpellAuto",                     UPN_IS_SPELL_AUTO,                     UPH_IS_SPELL_AUTO},
{/* 11 */    "SpellChecking/IsSpellSpecial",                  UPN_IS_SPELL_SPECIAL,                  UPH_IS_SPELL_SPECIAL},
{/* 12 */    "SpellChecking/IsSpellInAllLocales",             UPN_IS_SPELL_IN_ALL_LANGUAGES,         UPH_IS_SPELL_IN_ALL_LANGUAGES},
{/* 13 */    "SpellChecking/IsHideMarkings",                  UPN_IS_SPELL_HIDE,                     UPH_IS_SPELL_HIDE},
{/* 14 */    "SpellChecking/IsReverseDirection",              UPN_IS_WRAP_REVERSE,                   UPH_IS_WRAP_REVERSE},

{/* 15 */    "Hyphenation/MinLeading",                        UPN_HYPH_MIN_LEADING,                  UPH_HYPH_MIN_LEADING},
{/* 16 */    "Hyphenation/MinTrailing",                       UPN_HYPH_MIN_TRAILING,                 UPH_HYPH_MIN_TRAILING},
{/* 17 */    "Hyphenation/MinWordLength",                     UPN_HYPH_MIN_WORD_LENGTH,              UPH_HYPH_MIN_WORD_LENGTH},
{/* 18 */    "Hyphenation/IsHyphSpecial",                     UPN_IS_HYPH_SPECIAL,                   UPH_IS_HYPH_SPECIAL},
{/* 19 */    "Hyphenation/IsHyphAuto",                        UPN_IS_HYPH_AUTO,                      UPH_IS_HYPH_AUTO},

{/* 20 */    "TextConversion/ActiveConversionDictionaries",   UPN_ACTIVE_CONVERSION_DICTIONARIES,        UPH_ACTIVE_CONVERSION_DICTIONARIES},
{/* 21 */    "TextConversion/IsIgnorePostPositionalWord",     UPN_IS_IGNORE_POST_POSITIONAL_WORD,        UPH_IS_IGNORE_POST_POSITIONAL_WORD},
{/* 22 */    "TextConversion/IsAutoCloseDialog",              UPN_IS_AUTO_CLOSE_DIALOG,                  UPH_IS_AUTO_CLOSE_DIALOG},
{/* 23 */    "TextConversion/IsShowEntriesRecentlyUsedFirst", UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST,   UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST},
{/* 24 */    "TextConversion/IsAutoReplaceUniqueEntries",     UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES,        UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES},
{/* 25 */    "TextConversion/IsDirectionToSimplified",        UPN_IS_DIRECTION_TO_SIMPLIFIED,            UPH_IS_DIRECTION_TO_SIMPLIFIED},
{/* 26 */    "TextConversion/IsUseCharacterVariants",         UPN_IS_USE_CHARACTER_VARIANTS,             UPH_IS_USE_CHARACTER_VARIANTS},
{/* 27 */    "TextConversion/IsTranslateCommonTerms",         UPN_IS_TRANSLATE_COMMON_TERMS,             UPH_IS_TRANSLATE_COMMON_TERMS},
{/* 28 */    "TextConversion/IsReverseMapping",               UPN_IS_REVERSE_MAPPING,                    UPH_IS_REVERSE_MAPPING},

{/* 29 */    "ServiceManager/DataFilesChangedCheckValue",     UPN_DATA_FILES_CHANGED_CHECK_VALUE,        UPH_DATA_FILES_CHANGED_CHECK_VALUE},

            /* similar to entry 0 (thus no own configuration entry) but with different property name and type */
{            NULL,                                           UPN_DEFAULT_LANGUAGE,                      UPH_DEFAULT_LANGUAGE},

{            NULL,                                            NULL,                                      -1}
};


const uno::Sequence< OUString > & SvtLinguConfigItem::GetPropertyNames()
{
    static uno::Sequence< OUString > aNames;
    static sal_Bool bInitialized = sal_False;

    if (!bInitialized)
    {
        INT32 nMax = sizeof(aNamesToHdl) / sizeof(aNamesToHdl[0]);

        aNames.realloc( nMax );
        OUString *pNames = aNames.getArray();
        INT32 nIdx = 0;
        for (INT32 i = 0; i < nMax;  ++i)
        {
            const sal_Char *pFullPropName = aNamesToHdl[i].pFullPropName;
            if (pFullPropName)
                pNames[ nIdx++ ] = A2OU( pFullPropName );
        }
        aNames.realloc( nIdx );
        bInitialized = sal_True;
    }
    return aNames;
}


BOOL SvtLinguConfigItem::GetHdlByName(
    INT32 &rnHdl,
    const OUString &rPropertyName,
    sal_Bool bFullPropName )
{
    NamesToHdl *pEntry = &aNamesToHdl[0];

    if (bFullPropName)
    {
        while (pEntry && pEntry->pFullPropName != NULL)
        {
            if (0 == rPropertyName.compareToAscii( pEntry->pFullPropName ))
            {
                rnHdl = pEntry->nHdl;
                break;
            }
            ++pEntry;
        }
        return pEntry && pEntry->pFullPropName != NULL;
    }
    else
    {
        while (pEntry && pEntry->pPropName != NULL)
        {
            if (0 == rPropertyName.compareToAscii( pEntry->pPropName ))
            {
                rnHdl = pEntry->nHdl;
                break;
            }
            ++pEntry;
        }
        return pEntry && pEntry->pPropName != NULL;
    }
}


uno::Any SvtLinguConfigItem::GetProperty( const OUString &rPropertyName ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    INT32 nHdl;
    return GetHdlByName( nHdl, rPropertyName ) ? GetProperty( nHdl ) : uno::Any();
}


uno::Any SvtLinguConfigItem::GetProperty( INT32 nPropertyHandle ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    uno::Any aRes;

    const INT16 *pnVal = 0;
    const BOOL  *pbVal = 0;
    const INT32 *pnInt32Val = 0;

    const SvtLinguOptions &rOpt = const_cast< SvtLinguConfigItem * >(this)->aOpt;
    switch (nPropertyHandle)
    {
        case UPH_IS_GERMAN_PRE_REFORM :     pbVal = &rOpt.bIsGermanPreReform;   break;
        case UPH_IS_USE_DICTIONARY_LIST :   pbVal = &rOpt.bIsUseDictionaryList; break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &rOpt.bIsIgnoreControlCharacters;   break;
        case UPH_IS_HYPH_AUTO :             pbVal = &rOpt.bIsHyphAuto;  break;
        case UPH_IS_HYPH_SPECIAL :          pbVal = &rOpt.bIsHyphSpecial;   break;
        case UPH_IS_SPELL_AUTO :            pbVal = &rOpt.bIsSpellAuto; break;
        case UPH_IS_SPELL_HIDE :            pbVal = &rOpt.bIsSpellHideMarkings; break;
        case UPH_IS_SPELL_IN_ALL_LANGUAGES :pbVal = &rOpt.bIsSpellInAllLanguages;   break;
        case UPH_IS_SPELL_SPECIAL :         pbVal = &rOpt.bIsSpellSpecial;  break;
        case UPH_IS_WRAP_REVERSE :          pbVal = &rOpt.bIsSpellReverse;  break;
        case UPH_DEFAULT_LANGUAGE :         pnVal = &rOpt.nDefaultLanguage; break;
        case UPH_IS_SPELL_CAPITALIZATION :  pbVal = &rOpt.bIsSpellCapitalization;       break;
        case UPH_IS_SPELL_WITH_DIGITS :     pbVal = &rOpt.bIsSpellWithDigits;   break;
        case UPH_IS_SPELL_UPPER_CASE :      pbVal = &rOpt.bIsSpellUpperCase;        break;
        case UPH_HYPH_MIN_LEADING :         pnVal = &rOpt.nHyphMinLeading;      break;
        case UPH_HYPH_MIN_TRAILING :        pnVal = &rOpt.nHyphMinTrailing; break;
        case UPH_HYPH_MIN_WORD_LENGTH :     pnVal = &rOpt.nHyphMinWordLength;   break;
        case UPH_ACTIVE_DICTIONARIES :
        {
            aRes <<= rOpt.aActiveDics;
            break;
        }
        case UPH_ACTIVE_CONVERSION_DICTIONARIES :
        {
            aRes <<= rOpt.aActiveConvDics;
            break;
        }
        case UPH_DEFAULT_LOCALE :
        {
            lang::Locale aLocale( lcl_CreateLocale( rOpt.nDefaultLanguage ) );
            aRes.setValue( &aLocale, ::getCppuType((lang::Locale*)0 ));
            break;
        }
        case UPH_DEFAULT_LOCALE_CJK :
        {
            lang::Locale aLocale( lcl_CreateLocale( rOpt.nDefaultLanguage_CJK ) );
            aRes.setValue( &aLocale, ::getCppuType((lang::Locale*)0 ));
            break;
        }
        case UPH_DEFAULT_LOCALE_CTL :
        {
            lang::Locale aLocale( lcl_CreateLocale( rOpt.nDefaultLanguage_CTL ) );
            aRes.setValue( &aLocale, ::getCppuType((lang::Locale*)0 ));
            break;
        }
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       pbVal = &rOpt.bIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 pbVal = &rOpt.bIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  pbVal = &rOpt.bIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       pbVal = &rOpt.bIsAutoReplaceUniqueEntries; break;

        case UPH_IS_DIRECTION_TO_SIMPLIFIED:            pbVal = &rOpt.bIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS :            pbVal = &rOpt.bIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS :            pbVal = &rOpt.bIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :                   pbVal = &rOpt.bIsReverseMapping; break;

        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       pnInt32Val = &rOpt.nDataFilesChangedCheckValue; break;
        default :
            DBG_ERROR( "unexpected property handle" );
    }

    if (pbVal)
        aRes <<= *pbVal;
    else if (pnVal)
        aRes <<= *pnVal;
    else if (pnInt32Val)
        aRes <<= *pnInt32Val;

    return aRes;
}


BOOL SvtLinguConfigItem::SetProperty( const OUString &rPropertyName, const uno::Any &rValue )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bSucc = FALSE;
    INT32 nHdl;
    if (GetHdlByName( nHdl, rPropertyName ))
        bSucc = SetProperty( nHdl, rValue );
    return bSucc;
}


BOOL SvtLinguConfigItem::SetProperty( INT32 nPropertyHandle, const uno::Any &rValue )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bSucc = FALSE;
    if (!rValue.hasValue())
        return bSucc;

    BOOL bMod = FALSE;

    INT16 *pnVal = 0;
    BOOL  *pbVal = 0;
    INT32 *pnInt32Val = 0;

    SvtLinguOptions &rOpt = aOpt;
    switch (nPropertyHandle)
    {
        case UPH_IS_GERMAN_PRE_REFORM :     pbVal = &rOpt.bIsGermanPreReform;  break;
        case UPH_IS_USE_DICTIONARY_LIST :   pbVal = &rOpt.bIsUseDictionaryList;    break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &rOpt.bIsIgnoreControlCharacters;  break;
        case UPH_IS_HYPH_AUTO :             pbVal = &rOpt.bIsHyphAuto; break;
        case UPH_IS_HYPH_SPECIAL :          pbVal = &rOpt.bIsHyphSpecial;  break;
        case UPH_IS_SPELL_AUTO :            pbVal = &rOpt.bIsSpellAuto;    break;
        case UPH_IS_SPELL_HIDE :            pbVal = &rOpt.bIsSpellHideMarkings;    break;
        case UPH_IS_SPELL_IN_ALL_LANGUAGES :pbVal = &rOpt.bIsSpellInAllLanguages;  break;
        case UPH_IS_SPELL_SPECIAL :         pbVal = &rOpt.bIsSpellSpecial; break;
        case UPH_IS_WRAP_REVERSE :          pbVal = &rOpt.bIsSpellReverse; break;
        case UPH_DEFAULT_LANGUAGE :         pnVal = &rOpt.nDefaultLanguage;    break;
        case UPH_IS_SPELL_CAPITALIZATION :  pbVal = &rOpt.bIsSpellCapitalization;      break;
        case UPH_IS_SPELL_WITH_DIGITS :     pbVal = &rOpt.bIsSpellWithDigits;  break;
        case UPH_IS_SPELL_UPPER_CASE :      pbVal = &rOpt.bIsSpellUpperCase;       break;
        case UPH_HYPH_MIN_LEADING :         pnVal = &rOpt.nHyphMinLeading;     break;
        case UPH_HYPH_MIN_TRAILING :        pnVal = &rOpt.nHyphMinTrailing;    break;
        case UPH_HYPH_MIN_WORD_LENGTH :     pnVal = &rOpt.nHyphMinWordLength;  break;
        case UPH_ACTIVE_DICTIONARIES :
        {
            rValue >>= rOpt.aActiveDics;
            bMod = TRUE;
            break;
        }
        case UPH_ACTIVE_CONVERSION_DICTIONARIES :
        {
            rValue >>= rOpt.aActiveConvDics;
            bMod = TRUE;
            break;
        }
        case UPH_DEFAULT_LOCALE :
        {
            bSucc = lcl_SetLocale( rOpt.nDefaultLanguage, rValue );
            bMod = bSucc;
            break;
        }
        case UPH_DEFAULT_LOCALE_CJK :
        {
            bSucc = lcl_SetLocale( rOpt.nDefaultLanguage_CJK, rValue );
            bMod = bSucc;
            break;
        }
        case UPH_DEFAULT_LOCALE_CTL :
        {
            bSucc = lcl_SetLocale( rOpt.nDefaultLanguage_CTL, rValue );
            bMod = bSucc;
            break;
        }
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       pbVal = &rOpt.bIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 pbVal = &rOpt.bIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  pbVal = &rOpt.bIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       pbVal = &rOpt.bIsAutoReplaceUniqueEntries; break;

        case UPH_IS_DIRECTION_TO_SIMPLIFIED :           pbVal = &rOpt.bIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS :            pbVal = &rOpt.bIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS :            pbVal = &rOpt.bIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :                   pbVal = &rOpt.bIsReverseMapping; break;

        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       pnInt32Val = &rOpt.nDataFilesChangedCheckValue; break;
        default :
            DBG_ERROR( "unexpected property handle" );
    }

    if (pbVal)
    {
        BOOL bNew = BOOL();
        if (rValue >>= bNew)
        {
            if (bNew != *pbVal)
            {
                *pbVal = bNew;
                bMod = TRUE;
            }
            bSucc = TRUE;
        }
    }
    else if (pnVal)
    {
        INT16 nNew = INT16();
        if (rValue >>= nNew)
        {
            if (nNew != *pnVal)
            {
                *pnVal = nNew;
                bMod = TRUE;
            }
            bSucc = TRUE;
        }
    }
    else if (pnInt32Val)
    {
        INT32 nNew = INT32();
        if (rValue >>= nNew)
        {
            if (nNew != *pnInt32Val)
            {
                *pnInt32Val = nNew;
                bMod = TRUE;
            }
            bSucc = TRUE;
        }
    }

    if (bMod)
        SetModified();

    return bSucc;
}


BOOL SvtLinguConfigItem::GetOptions( SvtLinguOptions &rOptions ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    rOptions = aOpt;
    return TRUE;
}


BOOL SvtLinguConfigItem::SetOptions( const SvtLinguOptions &rOptions )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    aOpt = rOptions;
    SetModified();
    return TRUE;
}


BOOL SvtLinguConfigItem::LoadOptions( const uno::Sequence< OUString > &rProperyNames )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bRes = FALSE;

    const OUString *pProperyNames = rProperyNames.getConstArray();
    INT32 nProps = rProperyNames.getLength();

    const uno::Sequence< uno::Any > aValues = GetProperties( rProperyNames );
    const uno::Sequence< sal_Bool > aROStates = GetReadOnlyStates( rProperyNames );

    if (nProps  &&  aValues.getLength() == nProps &&  aROStates.getLength() == nProps)
    {
        SvtLinguOptions &rOpt = aOpt;

        const uno::Any *pValue = aValues.getConstArray();
        const sal_Bool *pROStates = aROStates.getConstArray();
        for (INT32 i = 0;  i < nProps;  ++i)
        {
            const uno::Any &rVal = pValue[i];
            INT32 nPropertyHandle;
            GetHdlByName( nPropertyHandle, pProperyNames[i], sal_True );
            switch ( nPropertyHandle )
            {
                case UPH_DEFAULT_LOCALE :
                    { rOpt.bRODefaultLanguage = pROStates[i]; lcl_CfgAnyToLanguage( rVal, rOpt.nDefaultLanguage ); } break;
                case UPH_ACTIVE_DICTIONARIES :
                    { rOpt.bROActiveDics = pROStates[i]; rVal >>= rOpt.aActiveDics;   } break;
                case UPH_IS_USE_DICTIONARY_LIST :
                    { rOpt.bROIsUseDictionaryList = pROStates[i]; rVal >>= rOpt.bIsUseDictionaryList;  } break;
                case UPH_IS_IGNORE_CONTROL_CHARACTERS :
                    { rOpt.bROIsIgnoreControlCharacters = pROStates[i]; rVal >>= rOpt.bIsIgnoreControlCharacters;    } break;
                case UPH_IS_GERMAN_PRE_REFORM :
                    { rOpt.bROIsGermanPreReform = pROStates[i]; rVal >>= rOpt.bIsGermanPreReform;    } break;
                case UPH_DEFAULT_LOCALE_CJK :
                    { rOpt.bRODefaultLanguage_CJK = pROStates[i]; lcl_CfgAnyToLanguage( rVal, rOpt.nDefaultLanguage_CJK );    } break;
                case UPH_DEFAULT_LOCALE_CTL :
                    { rOpt.bRODefaultLanguage_CTL = pROStates[i]; lcl_CfgAnyToLanguage( rVal, rOpt.nDefaultLanguage_CTL );    } break;

                case UPH_IS_SPELL_UPPER_CASE :
                    { rOpt.bROIsSpellUpperCase = pROStates[i]; rVal >>= rOpt.bIsSpellUpperCase; } break;
                case UPH_IS_SPELL_WITH_DIGITS :
                    { rOpt.bROIsSpellWithDigits = pROStates[i]; rVal >>= rOpt.bIsSpellWithDigits;    } break;
                case UPH_IS_SPELL_CAPITALIZATION :
                    { rOpt.bROIsSpellCapitalization = pROStates[i]; rVal >>= rOpt.bIsSpellCapitalization;    } break;
                case UPH_IS_SPELL_AUTO :
                    { rOpt.bROIsSpellAuto = pROStates[i]; rVal >>= rOpt.bIsSpellAuto;  } break;
                case UPH_IS_SPELL_SPECIAL :
                    { rOpt.bROIsSpellSpecial = pROStates[i]; rVal >>= rOpt.bIsSpellSpecial;   } break;
                case UPH_IS_SPELL_IN_ALL_LANGUAGES :
                    { rOpt.bROIsSpellInAllLanguages = pROStates[i]; rVal >>= rOpt.bIsSpellInAllLanguages;    } break;
                case UPH_IS_SPELL_HIDE :
                    { rOpt.bROIsSpellHideMarkings = pROStates[i]; rVal >>= rOpt.bIsSpellHideMarkings;  } break;
                case UPH_IS_WRAP_REVERSE :
                    { rOpt.bROIsSpellReverse = pROStates[i]; rVal >>= rOpt.bIsSpellReverse;   } break;

                case UPH_HYPH_MIN_LEADING :
                    { rOpt.bROHyphMinLeading = pROStates[i]; rVal >>= rOpt.nHyphMinLeading;   } break;
                case UPH_HYPH_MIN_TRAILING :
                    { rOpt.bROHyphMinTrailing = pROStates[i]; rVal >>= rOpt.nHyphMinTrailing;  } break;
                case UPH_HYPH_MIN_WORD_LENGTH :
                    { rOpt.bROHyphMinWordLength = pROStates[i]; rVal >>= rOpt.nHyphMinWordLength;    } break;
                case UPH_IS_HYPH_SPECIAL :
                    { rOpt.bROIsHyphSpecial = pROStates[i]; rVal >>= rOpt.bIsHyphSpecial;    } break;
                case UPH_IS_HYPH_AUTO :
                    { rOpt.bROIsHyphAuto = pROStates[i]; rVal >>= rOpt.bIsHyphAuto;   } break;

                case UPH_ACTIVE_CONVERSION_DICTIONARIES : { rOpt.bROActiveConvDics = pROStates[i]; rVal >>= rOpt.aActiveConvDics;   } break;

                case UPH_IS_IGNORE_POST_POSITIONAL_WORD :
                    { rOpt.bROIsIgnorePostPositionalWord = pROStates[i]; rVal >>= rOpt.bIsIgnorePostPositionalWord;  } break;
                case UPH_IS_AUTO_CLOSE_DIALOG :
                    { rOpt.bROIsAutoCloseDialog = pROStates[i]; rVal >>= rOpt.bIsAutoCloseDialog;  } break;
                case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :
                    { rOpt.bROIsShowEntriesRecentlyUsedFirst = pROStates[i]; rVal >>= rOpt.bIsShowEntriesRecentlyUsedFirst;  } break;
                case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :
                    { rOpt.bROIsAutoReplaceUniqueEntries = pROStates[i]; rVal >>= rOpt.bIsAutoReplaceUniqueEntries;  } break;

                case UPH_IS_DIRECTION_TO_SIMPLIFIED :
                    { rOpt.bROIsDirectionToSimplified = pROStates[i];
                            if( ! (rVal >>= rOpt.bIsDirectionToSimplified) )
                            {
                                //default is locale dependent:
                                if(  rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_HONGKONG
                                  || rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_MACAU
                                  || rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_TRADITIONAL )
                                {
                                    rOpt.bIsDirectionToSimplified = FALSE;
                                }
                                else
                                {
                                    rOpt.bIsDirectionToSimplified = TRUE;
                                }
                            }
                    } break;
                case UPH_IS_USE_CHARACTER_VARIANTS :
                    { rOpt.bROIsUseCharacterVariants = pROStates[i]; rVal >>= rOpt.bIsUseCharacterVariants;  } break;
                case UPH_IS_TRANSLATE_COMMON_TERMS :
                    { rOpt.bROIsTranslateCommonTerms = pROStates[i]; rVal >>= rOpt.bIsTranslateCommonTerms;  } break;
                case UPH_IS_REVERSE_MAPPING :
                    { rOpt.bROIsReverseMapping = pROStates[i]; rVal >>= rOpt.bIsReverseMapping;  } break;

                case UPH_DATA_FILES_CHANGED_CHECK_VALUE :
                    { rOpt.bRODataFilesChangedCheckValue = pROStates[i]; rVal >>= rOpt.nDataFilesChangedCheckValue;  } break;
                default:
                    DBG_ERROR( "unexpected case" );
            }
        }

        bRes = TRUE;
    }
    DBG_ASSERT( bRes, "LoadOptions failed" );

    return bRes;
}


BOOL SvtLinguConfigItem::SaveOptions( const uno::Sequence< OUString > &rProperyNames )
{
    if (!IsModified())
        return TRUE;

    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bRet = FALSE;
    const uno::Type &rBOOL     = ::getBooleanCppuType();
    const uno::Type &rINT16    = ::getCppuType( (INT16 *) NULL );
    const uno::Type &rINT32    = ::getCppuType( (INT32 *) NULL );

    INT32 nProps = rProperyNames.getLength();
    uno::Sequence< uno::Any > aValues( nProps );
    uno::Any *pValue = aValues.getArray();

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const SvtLinguOptions &rOpt = aOpt;

        OUString aTmp( lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage ) );
        *pValue++ = uno::makeAny( aTmp );                               //   0
        *pValue++ = uno::makeAny( rOpt.aActiveDics );                   //   1
        pValue++->setValue( &rOpt.bIsUseDictionaryList, rBOOL );        //   2
        pValue++->setValue( &rOpt.bIsIgnoreControlCharacters, rBOOL );  //   3
        pValue++->setValue( &rOpt.bIsGermanPreReform, rBOOL );          //   4
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CJK );
        *pValue++ = uno::makeAny( aTmp );                               //   5
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CTL );
        *pValue++ = uno::makeAny( aTmp );                               //   6

        pValue++->setValue( &rOpt.bIsSpellUpperCase, rBOOL );          //   7
        pValue++->setValue( &rOpt.bIsSpellWithDigits, rBOOL );         //   8
        pValue++->setValue( &rOpt.bIsSpellCapitalization, rBOOL );     //   9
        pValue++->setValue( &rOpt.bIsSpellAuto, rBOOL );               //  10
        pValue++->setValue( &rOpt.bIsSpellSpecial, rBOOL );            //  11
        pValue++->setValue( &rOpt.bIsSpellInAllLanguages, rBOOL );     //  12
        pValue++->setValue( &rOpt.bIsSpellHideMarkings, rBOOL );       //  13
        pValue++->setValue( &rOpt.bIsSpellReverse, rBOOL );            //  14

        pValue++->setValue( &rOpt.nHyphMinLeading, rINT16 );           //  15
        pValue++->setValue( &rOpt.nHyphMinTrailing, rINT16 );          //  16
        pValue++->setValue( &rOpt.nHyphMinWordLength, rINT16 );        //  17
        pValue++->setValue( &rOpt.bIsHyphSpecial, rBOOL );             //  18
        pValue++->setValue( &rOpt.bIsHyphAuto, rBOOL );                //  19

        *pValue++ = uno::makeAny( rOpt.aActiveConvDics );               //   20

        pValue++->setValue( &rOpt.bIsIgnorePostPositionalWord, rBOOL ); //  21
        pValue++->setValue( &rOpt.bIsAutoCloseDialog, rBOOL );          //  22
        pValue++->setValue( &rOpt.bIsShowEntriesRecentlyUsedFirst, rBOOL ); //  23
        pValue++->setValue( &rOpt.bIsAutoReplaceUniqueEntries, rBOOL ); //  24

        pValue++->setValue( &rOpt.bIsDirectionToSimplified, rBOOL ); //  25
        pValue++->setValue( &rOpt.bIsUseCharacterVariants, rBOOL ); //  26
        pValue++->setValue( &rOpt.bIsTranslateCommonTerms, rBOOL ); //  27
        pValue++->setValue( &rOpt.bIsReverseMapping, rBOOL ); //  28

        pValue++->setValue( &rOpt.nDataFilesChangedCheckValue, rINT32 ); //  29

        bRet |= PutProperties( rProperyNames, aValues );
    }

    if (bRet)
        ClearModified();

    return bRet;
}

BOOL SvtLinguConfigItem::IsReadOnly( const rtl::OUString &rPropertyName ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bReadOnly = FALSE;
    INT32 nHdl;
    if (GetHdlByName( nHdl, rPropertyName ))
        bReadOnly = IsReadOnly( nHdl );
    return bReadOnly;
}

BOOL SvtLinguConfigItem::IsReadOnly( INT32 nPropertyHandle ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    BOOL bReadOnly = FALSE;

    const SvtLinguOptions &rOpt = const_cast< SvtLinguConfigItem * >(this)->aOpt;
    switch(nPropertyHandle)
    {
        case UPH_IS_GERMAN_PRE_REFORM           : bReadOnly = rOpt.bROIsGermanPreReform        ; break;
        case UPH_IS_USE_DICTIONARY_LIST         : bReadOnly = rOpt.bROIsUseDictionaryList      ; break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS   : bReadOnly = rOpt.bROIsIgnoreControlCharacters; break;
        case UPH_IS_HYPH_AUTO                   : bReadOnly = rOpt.bROIsHyphAuto               ; break;
        case UPH_IS_HYPH_SPECIAL                : bReadOnly = rOpt.bROIsHyphSpecial            ; break;
        case UPH_IS_SPELL_AUTO                  : bReadOnly = rOpt.bROIsSpellAuto              ; break;
        case UPH_IS_SPELL_HIDE                  : bReadOnly = rOpt.bROIsSpellHideMarkings      ; break;
        case UPH_IS_SPELL_IN_ALL_LANGUAGES      : bReadOnly = rOpt.bROIsSpellInAllLanguages    ; break;
        case UPH_IS_SPELL_SPECIAL               : bReadOnly = rOpt.bROIsSpellSpecial           ; break;
        case UPH_IS_WRAP_REVERSE                : bReadOnly = rOpt.bROIsSpellReverse           ; break;
        case UPH_DEFAULT_LANGUAGE               : bReadOnly = rOpt.bRODefaultLanguage          ; break;
        case UPH_IS_SPELL_CAPITALIZATION        : bReadOnly = rOpt.bROIsSpellCapitalization    ; break;
        case UPH_IS_SPELL_WITH_DIGITS           : bReadOnly = rOpt.bROIsSpellWithDigits        ; break;
        case UPH_IS_SPELL_UPPER_CASE            : bReadOnly = rOpt.bROIsSpellUpperCase         ; break;
        case UPH_HYPH_MIN_LEADING               : bReadOnly = rOpt.bROHyphMinLeading           ; break;
        case UPH_HYPH_MIN_TRAILING              : bReadOnly = rOpt.bROHyphMinTrailing          ; break;
        case UPH_HYPH_MIN_WORD_LENGTH           : bReadOnly = rOpt.bROHyphMinWordLength        ; break;
        case UPH_ACTIVE_DICTIONARIES            : bReadOnly = rOpt.bROActiveDics               ; break;
        case UPH_ACTIVE_CONVERSION_DICTIONARIES : bReadOnly = rOpt.bROActiveConvDics           ; break;
        case UPH_DEFAULT_LOCALE                 : bReadOnly = rOpt.bRODefaultLanguage          ; break;
        case UPH_DEFAULT_LOCALE_CJK             : bReadOnly = rOpt.bRODefaultLanguage_CJK      ; break;
        case UPH_DEFAULT_LOCALE_CTL             : bReadOnly = rOpt.bRODefaultLanguage_CTL      ; break;
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       bReadOnly = rOpt.bROIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 bReadOnly = rOpt.bROIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  bReadOnly = rOpt.bROIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       bReadOnly = rOpt.bROIsAutoReplaceUniqueEntries; break;
        case UPH_IS_DIRECTION_TO_SIMPLIFIED : bReadOnly = rOpt.bROIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS : bReadOnly = rOpt.bROIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS : bReadOnly = rOpt.bROIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :        bReadOnly = rOpt.bROIsReverseMapping; break;
        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       bReadOnly = rOpt.bRODataFilesChangedCheckValue; break;
        default :
            DBG_ERROR( "unexpected property handle" );
    }
    return bReadOnly;
}

//////////////////////////////////////////////////////////////////////

static SvtLinguConfigItem *pCfgItem = 0;
static sal_Int32           nCfgItemRefCount = 0;

static const rtl::OUString aG_SupportedDictionaryFormats( A2OU("SupportedDictionaryFormats") );
static const rtl::OUString aG_Dictionaries( A2OU("Dictionaries") );
static const rtl::OUString aG_Locations( A2OU("Locations") );
static const rtl::OUString aG_Format( A2OU("Format") );
static const rtl::OUString aG_Locales( A2OU("Locales") );
static const rtl::OUString aG_DisabledDictionaries( A2OU("DisabledDictionaries") );
static const rtl::OUString aG_LastActiveDictionaries( A2OU("LastActiveDictionaries") );

SvtLinguConfig::SvtLinguConfig()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetOwnMutex() );
    ++nCfgItemRefCount;
}


SvtLinguConfig::~SvtLinguConfig()
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    if (pCfgItem && pCfgItem->IsModified())
        pCfgItem->Commit();

    if (--nCfgItemRefCount <= 0)
    {
        if (pCfgItem)
            delete pCfgItem;
        pCfgItem = 0;
    }
}


SvtLinguConfigItem & SvtLinguConfig::GetConfigItem()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetOwnMutex() );
    if (!pCfgItem)
    {
        pCfgItem = new SvtLinguConfigItem;
        ItemHolder1::holdConfigItem(E_LINGUCFG);
    }
    ++nCfgItemRefCount;
    return *pCfgItem;
}


uno::Sequence< OUString > SvtLinguConfig::GetNodeNames( const OUString &rNode )
{
    return GetConfigItem().GetNodeNames( rNode );
}


uno::Sequence< uno::Any > SvtLinguConfig::GetProperties( const uno::Sequence< OUString > &rNames )
{
    return GetConfigItem().GetProperties(rNames);
}


sal_Bool SvtLinguConfig::ReplaceSetProperties(
        const OUString &rNode, uno::Sequence< beans::PropertyValue > rValues )
{
    return GetConfigItem().ReplaceSetProperties( rNode, rValues );
}


uno::Any SvtLinguConfig::GetProperty( const OUString &rPropertyName ) const
{
    return GetConfigItem().GetProperty( rPropertyName );
}


uno::Any SvtLinguConfig::GetProperty( INT32 nPropertyHandle ) const
{
    return GetConfigItem().GetProperty( nPropertyHandle );
}


BOOL SvtLinguConfig::SetProperty( const OUString &rPropertyName, const uno::Any &rValue )
{
    return GetConfigItem().SetProperty( rPropertyName, rValue );
}


BOOL SvtLinguConfig::SetProperty( INT32 nPropertyHandle, const uno::Any &rValue )
{
    return GetConfigItem().SetProperty( nPropertyHandle, rValue );
}


BOOL SvtLinguConfig::GetOptions( SvtLinguOptions &rOptions ) const
{
    return GetConfigItem().GetOptions( rOptions );
}


BOOL SvtLinguConfig::SetOptions( const SvtLinguOptions &rOptions )
{
    return GetConfigItem().SetOptions( rOptions );
}


BOOL SvtLinguConfig::IsReadOnly( const rtl::OUString &rPropertyName ) const
{
    return GetConfigItem().IsReadOnly( rPropertyName );
}

BOOL SvtLinguConfig::IsReadOnly( INT32 nPropertyHandle ) const
{
    return GetConfigItem().IsReadOnly( nPropertyHandle );
}

uno::Reference< util::XChangesBatch > SvtLinguConfig::GetUpdateAccess() const
{
    if (!m_xUpdateAccess.is())
    {
        try
        {
            // get configuration provider
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider;
            uno::Reference< lang::XMultiServiceFactory > xMgr = comphelper::getProcessServiceFactory();
            if (xMgr.is())
            {
                xConfigurationProvider = uno::Reference< lang::XMultiServiceFactory > (
                        xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.configuration.ConfigurationProvider" ) ) ),
                        uno::UNO_QUERY_THROW ) ;
            }

            // get configuration update access
            beans::PropertyValue aValue;
            aValue.Name  = A2OU( "nodepath" );
            aValue.Value = uno::makeAny( A2OU("org.openoffice.Office.Linguistic/ServiceManager") );
            uno::Sequence< uno::Any > aProps(1);
            aProps[0] <<= aValue;
            m_xUpdateAccess = uno::Reference< util::XChangesBatch >(
                    xConfigurationProvider->createInstanceWithArguments(
                        A2OU( "com.sun.star.configuration.ConfigurationUpdateAccess" ), aProps ),
                        uno::UNO_QUERY_THROW );
        }
        catch (uno::Exception &)
        {
        }
    }

    return m_xUpdateAccess;
}

BOOL SvtLinguConfig::GetElementNamesFor(
     const rtl::OUString &rNodeName,
     uno::Sequence< rtl::OUString > &rElementNames ) const
{
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rNodeName ), uno::UNO_QUERY_THROW );
        rElementNames = xNA->getElementNames();
        bSuccess = true;
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

static uno::Reference< container::XNameAccess > GetOrCreateSetEntry_Impl(
    const uno::Reference< container::XNameAccess > &rxSetNameAccess,
    const rtl::OUString &rEntryName )
{
    uno::Reference< container::XNameAccess > xResult;
    try
    {
        if (!rxSetNameAccess->hasByName( rEntryName ))
        {
            uno::Reference< lang::XSingleServiceFactory > xFactory( rxSetNameAccess, uno::UNO_QUERY_THROW);
            uno::Reference< uno::XInterface > xNewEntry( xFactory->createInstance() );
            uno::Reference< container::XNameContainer > xNC( rxSetNameAccess, uno::UNO_QUERY_THROW );
            xNC->insertByName( rEntryName, makeAny( xNewEntry ) );
        }
        xResult.set( rxSetNameAccess->getByName( rEntryName ), uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception &)
    {
    }
    return xResult;
}

BOOL SvtLinguConfig::GetSupportedDictionaryFormatsFor(
    const rtl::OUString &rSetName,
    const rtl::OUString &rSetEntry,
    uno::Sequence< rtl::OUString > &rFormatList ) const
{
    if (rSetName.getLength() == 0 || rSetEntry.getLength() == 0)
        return sal_False;
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rSetName ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rSetEntry ), uno::UNO_QUERY_THROW );
        if (xNA->getByName( aG_SupportedDictionaryFormats ) >>= rFormatList)
            bSuccess = true;
        DBG_ASSERT( rFormatList.getLength(), "supported dictionary format list is empty" );
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

void SvtLinguConfig::SetOrCreateSupportedDictionaryFormatsFor(
    const rtl::OUString &rSetName,
    const rtl::OUString &rSetEntry,
    const uno::Sequence< rtl::OUString > &rFormatList  ) const
{
    if (rSetName.getLength() == 0 || rSetEntry.getLength() == 0)
        return;
    try
    {
        DBG_ASSERT( rFormatList.getLength(), "applying empty format list. Really??" );

        uno::Reference< util::XChangesBatch > xUpdateAccess( GetUpdateAccess() );
        uno::Reference< container::XNameAccess > xNA( xUpdateAccess, uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rSetName ), uno::UNO_QUERY_THROW );
        xNA = GetOrCreateSetEntry_Impl( xNA, rSetEntry );

        uno::Reference< container::XNameReplace > xNR( xNA, uno::UNO_QUERY_THROW );
        xNR->replaceByName( aG_SupportedDictionaryFormats, uno::makeAny( rFormatList ) );

        xUpdateAccess->commitChanges();
    }
    catch (uno::Exception &)
    {
    }
}


static uno::WeakReference< util::XMacroExpander > aG_xMacroExpander;

static uno::Reference< util::XMacroExpander > GetMacroExpander()
{
    uno::Reference< util::XMacroExpander > xMacroExpander( aG_xMacroExpander );
    if ( !xMacroExpander.is() )
    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( !xMacroExpander.is() )
        {
            uno::Reference< uno::XComponentContext > xContext;
            uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
            if ( xContext.is() )
            {
                aG_xMacroExpander =  uno::Reference< com::sun::star::util::XMacroExpander >( xContext->getValueByName(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))),
                                        uno::UNO_QUERY );
                xMacroExpander = aG_xMacroExpander;
            }
        }
    }

    return xMacroExpander;
}


BOOL SvtLinguConfig::GetDictionaryEntry(
    const rtl::OUString &rNodeName,
    SvtLinguConfigDictionaryEntry &rDicEntry ) const
{
    if (rNodeName.getLength() == 0)
        return sal_False;
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( aG_Dictionaries ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rNodeName ), uno::UNO_QUERY_THROW );

        // read group data...
        uno::Sequence< rtl::OUString >  aLocations;
        rtl::OUString                   aFormatName;
        uno::Sequence< rtl::OUString >  aLocaleNames;
        bSuccess =  (xNA->getByName( aG_Locations ) >>= aLocations)  &&
                    (xNA->getByName( aG_Format )    >>= aFormatName) &&
                    (xNA->getByName( aG_Locales )   >>= aLocaleNames);
        DBG_ASSERT( aLocations.getLength(), "Dictionary locations not set" );
        DBG_ASSERT( aFormatName.getLength(), "Dictionary format name not set" );
        DBG_ASSERT( aLocaleNames.getLength(), "No locales set for the dictionary" );

        // if sucessful continue
        if (bSuccess)
        {
            // get file URL's for the locations
            uno::Reference< util::XMacroExpander > xMacroExpander( GetMacroExpander() );
            for (sal_Int32 i = 0;  i < aLocations.getLength();  ++i)
            {
                rtl::OUString aURL( aLocations[i] );
                if (( aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( EXPAND_PROTOCOL )) == 0 ) &&
                    xMacroExpander.is() )
                {
                    // cut protocol
                    OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
                    // decode uric class chars
                    aMacro = Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                    // expand macro string
                    aURL = xMacroExpander->expandMacros( aMacro );

                    bool bIsFileUrl = aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( FILE_PROTOCOL )) == 0;
                    if (!bIsFileUrl)
                    {
                        bSuccess = false;
                        DBG_ASSERT( bIsFileUrl, "not a file URL");
                    }

                    aLocations[i] = aURL;
                }
                else
                {
                    bSuccess = false;
                    DBG_ERROR( "failed to get file URL" );
                }
            }

            // if everything was fine return the result
            if (bSuccess)
            {
                rDicEntry.aLocations    = aLocations;
                rDicEntry.aFormatName   = aFormatName;
                rDicEntry.aLocaleNames  = aLocaleNames;
            }
        }
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

void SvtLinguConfig::SetOrCreateDictionaryEntry(
    const rtl::OUString &rNodeName,
    const SvtLinguConfigDictionaryEntry &rDicEntry ) const
{
    if (rNodeName.getLength() == 0)
        return;
    try
    {
        uno::Reference< util::XChangesBatch > xUpdateAccess( GetUpdateAccess() );
        uno::Reference< container::XNameAccess > xNA( xUpdateAccess, uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( aG_Dictionaries ), uno::UNO_QUERY_THROW );
        xNA = GetOrCreateSetEntry_Impl( xNA, rNodeName );

        DBG_ASSERT( rDicEntry.aLocations.getLength(), "Applying empty dictionary locations. Really correct??" );
        DBG_ASSERT( rDicEntry.aFormatName.getLength(), "Applying empty dictionary format name. Really correct??" );
        DBG_ASSERT( rDicEntry.aLocaleNames.getLength(), "Applying empty list of locales for the dictionary. Really correct??" );

        uno::Reference< container::XNameReplace > xNR( xNA, uno::UNO_QUERY_THROW );
        xNR->replaceByName( aG_Locations, uno::makeAny( rDicEntry.aLocations ) );
        xNR->replaceByName( aG_Format,    uno::makeAny( rDicEntry.aFormatName ) );
        xNR->replaceByName( aG_Locales,   uno::makeAny( rDicEntry.aLocaleNames ) );

        xUpdateAccess->commitChanges();
    }
    catch (uno::Exception &)
    {
    }
}

uno::Sequence< rtl::OUString > SvtLinguConfig::GetDisabledDictionaries() const
{
    uno::Sequence< rtl::OUString > aResult;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA->getByName( aG_DisabledDictionaries ) >>= aResult;
    }
    catch (uno::Exception &)
    {
    }
    return aResult;
}

void SvtLinguConfig::SetDisabledDictionaries(
    const uno::Sequence< rtl::OUString > &rDictionaries ) const
{
    try
    {
        uno::Reference< util::XChangesBatch > xUpdateAccess( GetUpdateAccess() );
        uno::Reference< container::XNameAccess > xNA( xUpdateAccess, uno::UNO_QUERY_THROW );
        if (xNA->hasByName( aG_DisabledDictionaries ))
        {
            uno::Reference< container::XNameReplace > xNR( xNA, uno::UNO_QUERY_THROW );
            xNR->replaceByName( aG_DisabledDictionaries, makeAny( rDictionaries ) );
        }
        else
        {
            uno::Reference< container::XNameContainer > xNC( xNA, uno::UNO_QUERY_THROW );
            xNC->insertByName( aG_DisabledDictionaries, makeAny( rDictionaries ) );
        }

        xUpdateAccess->commitChanges();
    }
    catch (uno::Exception &)
    {
    }
}

std::vector< SvtLinguConfigDictionaryEntry > SvtLinguConfig::GetActiveDictionariesByFormat(
    const rtl::OUString &rFormatName )
{
    std::vector< SvtLinguConfigDictionaryEntry > aRes;
    if (rFormatName.getLength() == 0)
        return aRes;

    try
    {
        uno::Sequence< rtl::OUString > aElementNames;
        GetElementNamesFor( aG_Dictionaries, aElementNames );
        sal_Int32 nLen = aElementNames.getLength();
        const rtl::OUString *pElementNames = aElementNames.getConstArray();

        SvtLinguConfigDictionaryEntry aDicEntry;
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            // does dictionary match the format we are looking for?
            if (GetDictionaryEntry( pElementNames[i], aDicEntry ) &&
                aDicEntry.aFormatName == rFormatName)
            {
                // check if it is active or not
                bool bDicIsActive = true;
                const uno::Sequence< rtl::OUString > aDisabledDics( GetDisabledDictionaries() );
                for (sal_Int32 k = 0;  bDicIsActive && k < aDisabledDics.getLength();  ++k)
                {
                    if (aDisabledDics[k] == pElementNames[i])
                        bDicIsActive = false;
                }

                if (bDicIsActive)
                {
                    DBG_ASSERT( aDicEntry.aFormatName.getLength(),
                            "FormatName not set" );
                    DBG_ASSERT( aDicEntry.aLocations.getLength(),
                            "Locations not set" );
                    DBG_ASSERT( aDicEntry.aLocaleNames.getLength(),
                            "Locales not set" );
                    aRes.push_back( aDicEntry );
                }
            }
        }
    }
    catch (uno::Exception &)
    {
    }

    return aRes;
}

//////////////////////////////////////////////////////////////////////

