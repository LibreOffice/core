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

#include <sal/config.h>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <sal/macros.h>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>

#include "itemholder1.hxx"

using namespace com::sun::star;

#define FILE_PROTOCOL       "file:///"

namespace
{
    class theSvtLinguConfigItemMutex :
        public rtl::Static< osl::Mutex, theSvtLinguConfigItemMutex > {};
}

static bool lcl_SetLocale( sal_Int16 &rLanguage, const uno::Any &rVal )
{
    bool bSucc = false;

    lang::Locale aNew;
    if (rVal >>= aNew)  // conversion successful?
    {
        sal_Int16 nNew = LanguageTag::convertToLanguageType( aNew, false);
        if (nNew != rLanguage)
        {
            rLanguage = nNew;
            bSucc = true;
        }
    }
    return bSucc;
}

static inline const OUString lcl_LanguageToCfgLocaleStr( sal_Int16 nLanguage )
{
    OUString aRes;
    if (LANGUAGE_SYSTEM != nLanguage)
        aRes = LanguageTag::convertToBcp47( nLanguage );
    return aRes;
}

static sal_Int16 lcl_CfgAnyToLanguage( const uno::Any &rVal )
{
    OUString aTmp;
    rVal >>= aTmp;
    return (aTmp.isEmpty()) ? LANGUAGE_SYSTEM : LanguageTag::convertToLanguageTypeWithFallback( aTmp );
}

SvtLinguOptions::SvtLinguOptions()
    : bROActiveDics(false)
    , bROActiveConvDics(false)
    , nHyphMinLeading(2)
    , nHyphMinTrailing(2)
    , nHyphMinWordLength(0)
    , bROHyphMinLeading(false)
    , bROHyphMinTrailing(false)
    , bROHyphMinWordLength(false)
    , nDefaultLanguage(LANGUAGE_NONE)
    , nDefaultLanguage_CJK(LANGUAGE_NONE)
    , nDefaultLanguage_CTL(LANGUAGE_NONE)
    , bRODefaultLanguage(false)
    , bRODefaultLanguage_CJK(false)
    , bRODefaultLanguage_CTL(false)
    , bIsSpellSpecial(true)
    , bIsSpellAuto(false)
    , bIsSpellReverse(false)
    , bROIsSpellSpecial(false)
    , bROIsSpellAuto(false)
    , bROIsSpellReverse(false)
    , bIsHyphSpecial(true)
    , bIsHyphAuto(false)
    , bROIsHyphSpecial(false)
    , bROIsHyphAuto(false)
    , bIsUseDictionaryList(true)
    , bIsIgnoreControlCharacters(true)
    , bROIsUseDictionaryList(false)
    , bROIsIgnoreControlCharacters(false)
    , bIsSpellWithDigits(false)
    , bIsSpellUpperCase(false)
    , bIsSpellCapitalization(true)
    , bROIsSpellWithDigits(false)
    , bROIsSpellUpperCase(false)
    , bROIsSpellCapitalization(false)
    , bIsIgnorePostPositionalWord(true)
    , bIsAutoCloseDialog(false)
    , bIsShowEntriesRecentlyUsedFirst(false)
    , bIsAutoReplaceUniqueEntries(false)
    , bIsDirectionToSimplified(true)
    , bIsUseCharacterVariants(false)
    , bIsTranslateCommonTerms(false)
    , bIsReverseMapping(false)
    , bROIsIgnorePostPositionalWord(false)
    , bROIsAutoCloseDialog(false)
    , bROIsShowEntriesRecentlyUsedFirst(false)
    , bROIsAutoReplaceUniqueEntries(false)
    , bROIsDirectionToSimplified(false)
    , bROIsUseCharacterVariants(false)
    , bROIsTranslateCommonTerms(false)
    , bROIsReverseMapping(false)
    , nDataFilesChangedCheckValue(0)
    , bRODataFilesChangedCheckValue(false)
    , bIsGrammarAuto(false)
    , bIsGrammarInteractive(false)
    , bROIsGrammarAuto(false)
    , bROIsGrammarInteractive(false)
{
}

class SvtLinguConfigItem : public utl::ConfigItem
{
    SvtLinguOptions     aOpt;

    static bool GetHdlByName( sal_Int32 &rnHdl, const OUString &rPropertyName, bool bFullPropName = false );
    static const uno::Sequence< OUString > GetPropertyNames();
    bool                LoadOptions( const uno::Sequence< OUString > &rProperyNames );
    bool                SaveOptions( const uno::Sequence< OUString > &rProperyNames );

    SvtLinguConfigItem(const SvtLinguConfigItem&) = delete;
    SvtLinguConfigItem& operator=(const SvtLinguConfigItem&) = delete;
    virtual void    ImplCommit() override;

public:
    SvtLinguConfigItem();
    virtual ~SvtLinguConfigItem();

    // utl::ConfigItem
    virtual void    Notify( const css::uno::Sequence< OUString > &rPropertyNames ) override;

    // make some protected functions of utl::ConfigItem public
    using utl::ConfigItem::GetNodeNames;
    using utl::ConfigItem::GetProperties;
    //using utl::ConfigItem::PutProperties;
    //using utl::ConfigItem::SetSetProperties;
    using utl::ConfigItem::ReplaceSetProperties;
    //using utl::ConfigItem::GetReadOnlyStates;

    css::uno::Any
            GetProperty( const OUString &rPropertyName ) const;
    css::uno::Any
            GetProperty( sal_Int32 nPropertyHandle ) const;

    bool    SetProperty( const OUString &rPropertyName,
                         const css::uno::Any &rValue );
    bool    SetProperty( sal_Int32 nPropertyHandle,
                         const css::uno::Any &rValue );

    const SvtLinguOptions& GetOptions() const;

    bool    IsReadOnly( const OUString &rPropertyName ) const;
    bool    IsReadOnly( sal_Int32 nPropertyHandle ) const;
};

SvtLinguConfigItem::SvtLinguConfigItem() :
    utl::ConfigItem( OUString("Office.Linguistic") )
{
    const uno::Sequence< OUString > &rPropertyNames = GetPropertyNames();
    LoadOptions( rPropertyNames );
    ClearModified();

    // request notify events when properties change
    EnableNotification( rPropertyNames );
}

SvtLinguConfigItem::~SvtLinguConfigItem()
{
    //! Commit (SaveOptions) will be called by the d-tor of the base called !
}

void SvtLinguConfigItem::Notify( const uno::Sequence< OUString > &rPropertyNames )
{
    LoadOptions( rPropertyNames );
    NotifyListeners(0);
}

void SvtLinguConfigItem::ImplCommit()
{
    SaveOptions( GetPropertyNames() );
}

static struct NamesToHdl
{
    const char   *pFullPropName;      // full qualified name as used in configuration
    const char   *pPropName;          // property name only (atom) of above
    sal_Int32   nHdl;               // numeric handle representing the property
}aNamesToHdl[] =
{
{/*  0 */    "General/DefaultLocale",                         UPN_DEFAULT_LOCALE,                    UPH_DEFAULT_LOCALE},
{/*  1 */    "General/DictionaryList/ActiveDictionaries",     UPN_ACTIVE_DICTIONARIES,               UPH_ACTIVE_DICTIONARIES},
{/*  2 */    "General/DictionaryList/IsUseDictionaryList",    UPN_IS_USE_DICTIONARY_LIST,            UPH_IS_USE_DICTIONARY_LIST},
{/*  3 */    "General/IsIgnoreControlCharacters",             UPN_IS_IGNORE_CONTROL_CHARACTERS,      UPH_IS_IGNORE_CONTROL_CHARACTERS},
{/*  5 */    "General/DefaultLocale_CJK",                     UPN_DEFAULT_LOCALE_CJK,                UPH_DEFAULT_LOCALE_CJK},
{/*  6 */    "General/DefaultLocale_CTL",                     UPN_DEFAULT_LOCALE_CTL,                UPH_DEFAULT_LOCALE_CTL},

{/*  7 */    "SpellChecking/IsSpellUpperCase",                UPN_IS_SPELL_UPPER_CASE,               UPH_IS_SPELL_UPPER_CASE},
{/*  8 */    "SpellChecking/IsSpellWithDigits",               UPN_IS_SPELL_WITH_DIGITS,              UPH_IS_SPELL_WITH_DIGITS},
{/*  9 */    "SpellChecking/IsSpellCapitalization",           UPN_IS_SPELL_CAPITALIZATION,           UPH_IS_SPELL_CAPITALIZATION},
{/* 10 */    "SpellChecking/IsSpellAuto",                     UPN_IS_SPELL_AUTO,                     UPH_IS_SPELL_AUTO},
{/* 11 */    "SpellChecking/IsSpellSpecial",                  UPN_IS_SPELL_SPECIAL,                  UPH_IS_SPELL_SPECIAL},
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

{/* 30 */    "GrammarChecking/IsAutoCheck",                   UPN_IS_GRAMMAR_AUTO,                      UPH_IS_GRAMMAR_AUTO},
{/* 31 */    "GrammarChecking/IsInteractiveCheck",            UPN_IS_GRAMMAR_INTERACTIVE,               UPH_IS_GRAMMAR_INTERACTIVE},

            /* similar to entry 0 (thus no own configuration entry) but with different property name and type */
{            nullptr,                                           UPN_DEFAULT_LANGUAGE,                      UPH_DEFAULT_LANGUAGE},

{            nullptr,                                            nullptr,                                      -1}
};

const uno::Sequence< OUString > SvtLinguConfigItem::GetPropertyNames()
{
    uno::Sequence< OUString > aNames;

    sal_Int32 nMax = SAL_N_ELEMENTS(aNamesToHdl);

    aNames.realloc( nMax );
    OUString *pNames = aNames.getArray();
    sal_Int32 nIdx = 0;
    for (sal_Int32 i = 0; i < nMax;  ++i)
    {
        const sal_Char *pFullPropName = aNamesToHdl[i].pFullPropName;
        if (pFullPropName)
            pNames[ nIdx++ ] = OUString::createFromAscii( pFullPropName );
    }
    aNames.realloc( nIdx );

    return aNames;
}

bool SvtLinguConfigItem::GetHdlByName(
    sal_Int32 &rnHdl,
    const OUString &rPropertyName,
    bool bFullPropName )
{
    NamesToHdl *pEntry = &aNamesToHdl[0];

    if (bFullPropName)
    {
        while (pEntry && pEntry->pFullPropName != nullptr)
        {
            if (rPropertyName.equalsAscii( pEntry->pFullPropName ))
            {
                rnHdl = pEntry->nHdl;
                break;
            }
            ++pEntry;
        }
        return pEntry && pEntry->pFullPropName != nullptr;
    }
    else
    {
        while (pEntry && pEntry->pPropName != nullptr)
        {
            if (rPropertyName.equalsAscii( pEntry->pPropName ))
            {
                rnHdl = pEntry->nHdl;
                break;
            }
            ++pEntry;
        }
        return pEntry && pEntry->pPropName != nullptr;
    }
}

uno::Any SvtLinguConfigItem::GetProperty( const OUString &rPropertyName ) const
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    sal_Int32 nHdl;
    return GetHdlByName( nHdl, rPropertyName ) ? GetProperty( nHdl ) : uno::Any();
}

uno::Any SvtLinguConfigItem::GetProperty( sal_Int32 nPropertyHandle ) const
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    uno::Any aRes;

    const sal_Int16 *pnVal = nullptr;
    const bool  *pbVal = nullptr;
    const sal_Int32 *pnInt32Val = nullptr;

    const SvtLinguOptions &rOpt = const_cast< SvtLinguConfigItem * >(this)->aOpt;
    switch (nPropertyHandle)
    {
        case UPH_IS_USE_DICTIONARY_LIST :   pbVal = &rOpt.bIsUseDictionaryList; break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &rOpt.bIsIgnoreControlCharacters;   break;
        case UPH_IS_HYPH_AUTO :             pbVal = &rOpt.bIsHyphAuto;  break;
        case UPH_IS_HYPH_SPECIAL :          pbVal = &rOpt.bIsHyphSpecial;   break;
        case UPH_IS_SPELL_AUTO :            pbVal = &rOpt.bIsSpellAuto; break;
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
            lang::Locale aLocale( LanguageTag::convertToLocale( rOpt.nDefaultLanguage, false) );
            aRes.setValue( &aLocale, cppu::UnoType<lang::Locale>::get());
            break;
        }
        case UPH_DEFAULT_LOCALE_CJK :
        {
            lang::Locale aLocale( LanguageTag::convertToLocale( rOpt.nDefaultLanguage_CJK, false) );
            aRes.setValue( &aLocale, cppu::UnoType<lang::Locale>::get());
            break;
        }
        case UPH_DEFAULT_LOCALE_CTL :
        {
            lang::Locale aLocale( LanguageTag::convertToLocale( rOpt.nDefaultLanguage_CTL, false) );
            aRes.setValue( &aLocale, cppu::UnoType<lang::Locale>::get());
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
        case UPH_IS_GRAMMAR_AUTO:                       pbVal = &rOpt.bIsGrammarAuto; break;
        case UPH_IS_GRAMMAR_INTERACTIVE:                pbVal = &rOpt.bIsGrammarInteractive; break;
        default :
            SAL_WARN( "unotools", "unexpected property handle" );
    }

    if (pbVal)
        aRes <<= *pbVal;
    else if (pnVal)
        aRes <<= *pnVal;
    else if (pnInt32Val)
        aRes <<= *pnInt32Val;

    return aRes;
}

bool SvtLinguConfigItem::SetProperty( const OUString &rPropertyName, const uno::Any &rValue )
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    bool bSucc = false;
    sal_Int32 nHdl;
    if (GetHdlByName( nHdl, rPropertyName ))
        bSucc = SetProperty( nHdl, rValue );
    return bSucc;
}

bool SvtLinguConfigItem::SetProperty( sal_Int32 nPropertyHandle, const uno::Any &rValue )
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    bool bSucc = false;
    if (!rValue.hasValue())
        return bSucc;

    bool bMod = false;

    sal_Int16 *pnVal = nullptr;
    bool  *pbVal = nullptr;
    sal_Int32 *pnInt32Val = nullptr;

    SvtLinguOptions &rOpt = aOpt;
    switch (nPropertyHandle)
    {
        case UPH_IS_USE_DICTIONARY_LIST :   pbVal = &rOpt.bIsUseDictionaryList;    break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &rOpt.bIsIgnoreControlCharacters;  break;
        case UPH_IS_HYPH_AUTO :             pbVal = &rOpt.bIsHyphAuto; break;
        case UPH_IS_HYPH_SPECIAL :          pbVal = &rOpt.bIsHyphSpecial;  break;
        case UPH_IS_SPELL_AUTO :            pbVal = &rOpt.bIsSpellAuto;    break;
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
            bMod = true;
            break;
        }
        case UPH_ACTIVE_CONVERSION_DICTIONARIES :
        {
            rValue >>= rOpt.aActiveConvDics;
            bMod = true;
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
        case UPH_IS_GRAMMAR_AUTO:                       pbVal = &rOpt.bIsGrammarAuto; break;
        case UPH_IS_GRAMMAR_INTERACTIVE:                pbVal = &rOpt.bIsGrammarInteractive; break;
        default :
            SAL_WARN( "unotools", "unexpected property handle" );
    }

    if (pbVal)
    {
        bool bNew = bool();
        if (rValue >>= bNew)
        {
            if (bNew != *pbVal)
            {
                *pbVal = bNew;
                bMod = true;
            }
            bSucc = true;
        }
    }
    else if (pnVal)
    {
        sal_Int16 nNew = sal_Int16();
        if (rValue >>= nNew)
        {
            if (nNew != *pnVal)
            {
                *pnVal = nNew;
                bMod = true;
            }
            bSucc = true;
        }
    }
    else if (pnInt32Val)
    {
        sal_Int32 nNew = sal_Int32();
        if (rValue >>= nNew)
        {
            if (nNew != *pnInt32Val)
            {
                *pnInt32Val = nNew;
                bMod = true;
            }
            bSucc = true;
        }
    }

    if (bMod)
        SetModified();

    NotifyListeners(0);
    return bSucc;
}

const SvtLinguOptions& SvtLinguConfigItem::GetOptions() const
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());
    return aOpt;
}

bool SvtLinguConfigItem::LoadOptions( const uno::Sequence< OUString > &rProperyNames )
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    bool bRes = false;

    const OUString *pProperyNames = rProperyNames.getConstArray();
    sal_Int32 nProps = rProperyNames.getLength();

    const uno::Sequence< uno::Any > aValues = GetProperties( rProperyNames );
    const uno::Sequence< sal_Bool > aROStates = GetReadOnlyStates( rProperyNames );

    if (nProps  &&  aValues.getLength() == nProps &&  aROStates.getLength() == nProps)
    {
        SvtLinguOptions &rOpt = aOpt;

        const uno::Any *pValue = aValues.getConstArray();
        const sal_Bool *pROStates = aROStates.getConstArray();
        for (sal_Int32 i = 0;  i < nProps;  ++i)
        {
            const uno::Any &rVal = pValue[i];
            sal_Int32 nPropertyHandle(0);
            GetHdlByName( nPropertyHandle, pProperyNames[i], true );
            switch ( nPropertyHandle )
            {
                case UPH_DEFAULT_LOCALE :
                    { rOpt.bRODefaultLanguage = pROStates[i]; rOpt.nDefaultLanguage = lcl_CfgAnyToLanguage( rVal ); } break;
                case UPH_ACTIVE_DICTIONARIES :
                    { rOpt.bROActiveDics = pROStates[i]; rVal >>= rOpt.aActiveDics;   } break;
                case UPH_IS_USE_DICTIONARY_LIST :
                    { rOpt.bROIsUseDictionaryList = pROStates[i]; rVal >>= rOpt.bIsUseDictionaryList;  } break;
                case UPH_IS_IGNORE_CONTROL_CHARACTERS :
                    { rOpt.bROIsIgnoreControlCharacters = pROStates[i]; rVal >>= rOpt.bIsIgnoreControlCharacters;    } break;
                case UPH_DEFAULT_LOCALE_CJK :
                    { rOpt.bRODefaultLanguage_CJK = pROStates[i]; rOpt.nDefaultLanguage_CJK = lcl_CfgAnyToLanguage( rVal );    } break;
                case UPH_DEFAULT_LOCALE_CTL :
                    { rOpt.bRODefaultLanguage_CTL = pROStates[i]; rOpt.nDefaultLanguage_CTL = lcl_CfgAnyToLanguage( rVal );    } break;

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
                                if (MsLangId::isTraditionalChinese(rOpt.nDefaultLanguage_CJK))
                                {
                                    rOpt.bIsDirectionToSimplified = false;
                                }
                                else
                                {
                                    rOpt.bIsDirectionToSimplified = true;
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

                case UPH_IS_GRAMMAR_AUTO:
                    { rOpt.bROIsGrammarAuto = pROStates[i]; rVal >>= rOpt.bIsGrammarAuto; }
                break;
                case UPH_IS_GRAMMAR_INTERACTIVE:
                    { rOpt.bROIsGrammarInteractive = pROStates[i]; rVal >>= rOpt.bIsGrammarInteractive; }
                break;

                default:
                    SAL_WARN( "unotools", "unexpected case" );
            }
        }

        bRes = true;
    }
    DBG_ASSERT( bRes, "LoadOptions failed" );

    return bRes;
}

bool SvtLinguConfigItem::SaveOptions( const uno::Sequence< OUString > &rProperyNames )
{
    if (!IsModified())
        return true;

    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    bool bRet = false;
    const uno::Type &rBOOL     = cppu::UnoType<bool>::get();
    const uno::Type &rINT16    = cppu::UnoType<sal_Int16>::get();
    const uno::Type &rINT32    = cppu::UnoType<sal_Int32>::get();

    sal_Int32 nProps = rProperyNames.getLength();
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
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CJK );
        *pValue++ = uno::makeAny( aTmp );                               //   5
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CTL );
        *pValue++ = uno::makeAny( aTmp );                               //   6

        pValue++->setValue( &rOpt.bIsSpellUpperCase, rBOOL );          //   7
        pValue++->setValue( &rOpt.bIsSpellWithDigits, rBOOL );         //   8
        pValue++->setValue( &rOpt.bIsSpellCapitalization, rBOOL );     //   9
        pValue++->setValue( &rOpt.bIsSpellAuto, rBOOL );               //  10
        pValue++->setValue( &rOpt.bIsSpellSpecial, rBOOL );            //  11
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
        pValue++->setValue( &rOpt.bIsGrammarAuto, rBOOL ); //  30
        pValue++->setValue( &rOpt.bIsGrammarInteractive, rBOOL ); // 31

        bRet |= PutProperties( rProperyNames, aValues );
    }

    if (bRet)
        ClearModified();

    return bRet;
}

bool SvtLinguConfigItem::IsReadOnly( const OUString &rPropertyName ) const
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    bool bReadOnly = false;
    sal_Int32 nHdl;
    if (GetHdlByName( nHdl, rPropertyName ))
        bReadOnly = IsReadOnly( nHdl );
    return bReadOnly;
}

bool SvtLinguConfigItem::IsReadOnly( sal_Int32 nPropertyHandle ) const
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    bool bReadOnly = false;

    const SvtLinguOptions &rOpt = const_cast< SvtLinguConfigItem * >(this)->aOpt;
    switch(nPropertyHandle)
    {
        case UPH_IS_USE_DICTIONARY_LIST         : bReadOnly = rOpt.bROIsUseDictionaryList; break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS   : bReadOnly = rOpt.bROIsIgnoreControlCharacters; break;
        case UPH_IS_HYPH_AUTO                   : bReadOnly = rOpt.bROIsHyphAuto; break;
        case UPH_IS_HYPH_SPECIAL                : bReadOnly = rOpt.bROIsHyphSpecial; break;
        case UPH_IS_SPELL_AUTO                  : bReadOnly = rOpt.bROIsSpellAuto; break;
        case UPH_IS_SPELL_SPECIAL               : bReadOnly = rOpt.bROIsSpellSpecial; break;
        case UPH_IS_WRAP_REVERSE                : bReadOnly = rOpt.bROIsSpellReverse; break;
        case UPH_DEFAULT_LANGUAGE               : bReadOnly = rOpt.bRODefaultLanguage; break;
        case UPH_IS_SPELL_CAPITALIZATION        : bReadOnly = rOpt.bROIsSpellCapitalization; break;
        case UPH_IS_SPELL_WITH_DIGITS           : bReadOnly = rOpt.bROIsSpellWithDigits; break;
        case UPH_IS_SPELL_UPPER_CASE            : bReadOnly = rOpt.bROIsSpellUpperCase; break;
        case UPH_HYPH_MIN_LEADING               : bReadOnly = rOpt.bROHyphMinLeading; break;
        case UPH_HYPH_MIN_TRAILING              : bReadOnly = rOpt.bROHyphMinTrailing; break;
        case UPH_HYPH_MIN_WORD_LENGTH           : bReadOnly = rOpt.bROHyphMinWordLength; break;
        case UPH_ACTIVE_DICTIONARIES            : bReadOnly = rOpt.bROActiveDics; break;
        case UPH_ACTIVE_CONVERSION_DICTIONARIES : bReadOnly = rOpt.bROActiveConvDics; break;
        case UPH_DEFAULT_LOCALE                 : bReadOnly = rOpt.bRODefaultLanguage; break;
        case UPH_DEFAULT_LOCALE_CJK             : bReadOnly = rOpt.bRODefaultLanguage_CJK; break;
        case UPH_DEFAULT_LOCALE_CTL             : bReadOnly = rOpt.bRODefaultLanguage_CTL; break;
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       bReadOnly = rOpt.bROIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 bReadOnly = rOpt.bROIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  bReadOnly = rOpt.bROIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       bReadOnly = rOpt.bROIsAutoReplaceUniqueEntries; break;
        case UPH_IS_DIRECTION_TO_SIMPLIFIED : bReadOnly = rOpt.bROIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS : bReadOnly = rOpt.bROIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS : bReadOnly = rOpt.bROIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :        bReadOnly = rOpt.bROIsReverseMapping; break;
        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       bReadOnly = rOpt.bRODataFilesChangedCheckValue; break;
        case UPH_IS_GRAMMAR_AUTO:                       bReadOnly = rOpt.bROIsGrammarAuto; break;
        case UPH_IS_GRAMMAR_INTERACTIVE:                bReadOnly = rOpt.bROIsGrammarInteractive; break;
        default :
            SAL_WARN( "unotools", "unexpected property handle" );
    }
    return bReadOnly;
}

static SvtLinguConfigItem *pCfgItem = nullptr;
static sal_Int32           nCfgItemRefCount = 0;

static const char aG_SupportedDictionaryFormats[] = "SupportedDictionaryFormats";
static const char aG_Dictionaries[] = "Dictionaries";
static const char aG_Locations[] = "Locations";
static const char aG_Format[] = "Format";
static const char aG_Locales[] = "Locales";
static const char aG_DisabledDictionaries[] = "DisabledDictionaries";

SvtLinguConfig::SvtLinguConfig()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());
    ++nCfgItemRefCount;
}

SvtLinguConfig::~SvtLinguConfig()
{
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());

    if (pCfgItem && pCfgItem->IsModified())
        pCfgItem->Commit();

    if (--nCfgItemRefCount <= 0)
    {
        delete pCfgItem;
        pCfgItem = nullptr;
    }
}

SvtLinguConfigItem & SvtLinguConfig::GetConfigItem()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard(theSvtLinguConfigItemMutex::get());
    if (!pCfgItem)
    {
        pCfgItem = new SvtLinguConfigItem;
        ItemHolder1::holdConfigItem(E_LINGUCFG);
    }
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

bool SvtLinguConfig::ReplaceSetProperties(
        const OUString &rNode, const uno::Sequence< beans::PropertyValue >& rValues )
{
    return GetConfigItem().ReplaceSetProperties( rNode, rValues );
}

uno::Any SvtLinguConfig::GetProperty( const OUString &rPropertyName ) const
{
    return GetConfigItem().GetProperty( rPropertyName );
}

uno::Any SvtLinguConfig::GetProperty( sal_Int32 nPropertyHandle ) const
{
    return GetConfigItem().GetProperty( nPropertyHandle );
}

bool SvtLinguConfig::SetProperty( const OUString &rPropertyName, const uno::Any &rValue )
{
    return GetConfigItem().SetProperty( rPropertyName, rValue );
}

bool SvtLinguConfig::SetProperty( sal_Int32 nPropertyHandle, const uno::Any &rValue )
{
    return GetConfigItem().SetProperty( nPropertyHandle, rValue );
}

bool SvtLinguConfig::GetOptions( SvtLinguOptions &rOptions ) const
{
    rOptions = GetConfigItem().GetOptions();
    return true;
}

bool SvtLinguConfig::IsReadOnly( const OUString &rPropertyName ) const
{
    return GetConfigItem().IsReadOnly( rPropertyName );
}

bool SvtLinguConfig::GetElementNamesFor(
     const OUString &rNodeName,
     uno::Sequence< OUString > &rElementNames ) const
{
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName("ServiceManager"), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rNodeName ), uno::UNO_QUERY_THROW );
        rElementNames = xNA->getElementNames();
        bSuccess = true;
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

bool SvtLinguConfig::GetSupportedDictionaryFormatsFor(
    const OUString &rSetName,
    const OUString &rSetEntry,
    uno::Sequence< OUString > &rFormatList ) const
{
    if (rSetName.isEmpty() || rSetEntry.isEmpty())
        return false;
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName("ServiceManager"), uno::UNO_QUERY_THROW );
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

static bool lcl_GetFileUrlFromOrigin(
    OUString /*out*/ &rFileUrl,
    const OUString &rOrigin )
{
    OUString aURL(
        comphelper::getExpandedUri(
            comphelper::getProcessComponentContext(), rOrigin));
    if (aURL.startsWith( FILE_PROTOCOL ))
    {
        rFileUrl = aURL;
        return true;
    }
    else
    {
        SAL_WARN(
            "unotools.config", "not a file URL, <" << aURL << ">" );
        return false;
    }
}

bool SvtLinguConfig::GetDictionaryEntry(
    const OUString &rNodeName,
    SvtLinguConfigDictionaryEntry &rDicEntry ) const
{
    if (rNodeName.isEmpty())
        return false;
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName("ServiceManager"), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( aG_Dictionaries ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rNodeName ), uno::UNO_QUERY_THROW );

        // read group data...
        uno::Sequence< OUString >  aLocations;
        OUString                   aFormatName;
        uno::Sequence< OUString >  aLocaleNames;
        bSuccess =  (xNA->getByName( aG_Locations ) >>= aLocations)  &&
                    (xNA->getByName( aG_Format )    >>= aFormatName) &&
                    (xNA->getByName( aG_Locales )   >>= aLocaleNames);
        DBG_ASSERT( aLocations.getLength(), "Dictionary locations not set" );
        DBG_ASSERT( !aFormatName.isEmpty(), "Dictionary format name not set" );
        DBG_ASSERT( aLocaleNames.getLength(), "No locales set for the dictionary" );

        // if successful continue
        if (bSuccess)
        {
            // get file URL's for the locations
            for (sal_Int32 i = 0;  i < aLocations.getLength();  ++i)
            {
                OUString &rLocation = aLocations[i];
                if (!lcl_GetFileUrlFromOrigin( rLocation, rLocation ))
                    bSuccess = false;
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

uno::Sequence< OUString > SvtLinguConfig::GetDisabledDictionaries() const
{
    uno::Sequence< OUString > aResult;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName("ServiceManager"), uno::UNO_QUERY_THROW );
        xNA->getByName( aG_DisabledDictionaries ) >>= aResult;
    }
    catch (uno::Exception &)
    {
    }
    return aResult;
}

std::vector< SvtLinguConfigDictionaryEntry > SvtLinguConfig::GetActiveDictionariesByFormat(
    const OUString &rFormatName )
{
    std::vector< SvtLinguConfigDictionaryEntry > aRes;
    if (rFormatName.isEmpty())
        return aRes;

    try
    {
        uno::Sequence< OUString > aElementNames;
        GetElementNamesFor( aG_Dictionaries, aElementNames );
        sal_Int32 nLen = aElementNames.getLength();
        const OUString *pElementNames = aElementNames.getConstArray();

        const uno::Sequence< OUString > aDisabledDics( GetDisabledDictionaries() );

        SvtLinguConfigDictionaryEntry aDicEntry;
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            // does dictionary match the format we are looking for?
            if (GetDictionaryEntry( pElementNames[i], aDicEntry ) &&
                aDicEntry.aFormatName == rFormatName)
            {
                // check if it is active or not
                bool bDicIsActive = true;
                for (sal_Int32 k = 0;  bDicIsActive && k < aDisabledDics.getLength();  ++k)
                {
                    if (aDisabledDics[k] == pElementNames[i])
                        bDicIsActive = false;
                }

                if (bDicIsActive)
                {
                    DBG_ASSERT( !aDicEntry.aFormatName.isEmpty(),
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

uno::Reference< util::XChangesBatch > SvtLinguConfig::GetMainUpdateAccess() const
{
    if (!m_xMainUpdateAccess.is())
    {
        try
        {
            // get configuration provider
            uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider =
                    configuration::theDefaultProvider::get( xContext );

            // get configuration update access
            beans::PropertyValue aValue;
            aValue.Name  = "nodepath";
            aValue.Value = uno::makeAny(OUString("org.openoffice.Office.Linguistic"));
            uno::Sequence< uno::Any > aProps(1);
            aProps[0] <<= aValue;
            m_xMainUpdateAccess.set(
                    xConfigurationProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess", aProps),
                        uno::UNO_QUERY_THROW );
        }
        catch (uno::Exception &)
        {
        }
    }

    return m_xMainUpdateAccess;
}

OUString SvtLinguConfig::GetVendorImageUrl_Impl(
    const OUString &rServiceImplName,
    const OUString &rImageName ) const
{
    OUString aRes;
    try
    {
        uno::Reference< container::XNameAccess > xImagesNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xImagesNA.set( xImagesNA->getByName("Images"), uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameAccess > xNA( xImagesNA->getByName("ServiceNameEntries"), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rServiceImplName ), uno::UNO_QUERY_THROW );
        uno::Any aAny(xNA->getByName("VendorImagesNode"));
        OUString aVendorImagesNode;
        if (aAny >>= aVendorImagesNode)
        {
            xNA = xImagesNA;
            xNA.set( xNA->getByName("VendorImages"), uno::UNO_QUERY_THROW );
            xNA.set( xNA->getByName( aVendorImagesNode ), uno::UNO_QUERY_THROW );
            aAny = xNA->getByName( rImageName );
            OUString aTmp;
            if (aAny >>= aTmp)
            {
                if (lcl_GetFileUrlFromOrigin( aTmp, aTmp ))
                    aRes = aTmp;
            }
        }
    }
    catch (uno::Exception &)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aRes;
}

OUString SvtLinguConfig::GetSpellAndGrammarContextSuggestionImage(
    const OUString &rServiceImplName
) const
{
    OUString   aRes;
    if (!rServiceImplName.isEmpty())
    {
        OUString aImageName( "SpellAndGrammarContextMenuSuggestionImage" );
        OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}

OUString SvtLinguConfig::GetSpellAndGrammarContextDictionaryImage(
    const OUString &rServiceImplName
) const
{
    OUString   aRes;
    if (!rServiceImplName.isEmpty())
    {
        OUString aImageName( "SpellAndGrammarContextMenuDictionaryImage" );
        OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}

OUString SvtLinguConfig::GetSynonymsContextImage(
    const OUString &rServiceImplName
) const
{
    OUString   aRes;
    if (!rServiceImplName.isEmpty())
    {
        OUString aImageName( "SynonymsContextMenuImage" );
        OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}

bool SvtLinguConfig::HasGrammarChecker() const
{
    bool bRes = false;

    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName("ServiceManager"), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName("GrammarCheckerList"), uno::UNO_QUERY_THROW );

        uno::Sequence< OUString > aElementNames( xNA->getElementNames() );
        bRes = aElementNames.getLength() > 0;
    }
    catch (const uno::Exception&)
    {
    }

    return bRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
