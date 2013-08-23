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
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

#include "i18nlangtag/mslangid.hxx"


LanguageType MsLangId::nConfiguredSystemLanguage   = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredSystemUILanguage = LANGUAGE_SYSTEM;

LanguageType MsLangId::nConfiguredWesternFallback  = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredAsianFallback    = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredComplexFallback  = LANGUAGE_SYSTEM;

// static
void MsLangId::setConfiguredSystemLanguage( LanguageType nLang )
{
    nConfiguredSystemLanguage = nLang;
}


// static
void MsLangId::setConfiguredSystemUILanguage( LanguageType nLang )
{
    nConfiguredSystemUILanguage = nLang;
}

// static
void MsLangId::setConfiguredWesternFallback( LanguageType nLang )
{
    nConfiguredWesternFallback = nLang;
}

// static
void MsLangId::setConfiguredAsianFallback( LanguageType nLang )
{
    nConfiguredAsianFallback = nLang;
}

// static
void MsLangId::setConfiguredComplexFallback( LanguageType nLang )
{
    nConfiguredComplexFallback = nLang;
}

// static
inline LanguageType MsLangId::simplifySystemLanguages( LanguageType nLang )
{
    switch (nLang)
    {
        case LANGUAGE_PROCESS_OR_USER_DEFAULT :
        case LANGUAGE_SYSTEM_DEFAULT :
        case LANGUAGE_SYSTEM :
            nLang = LANGUAGE_SYSTEM;
            break;
        default:
            ;   // nothing
    }
    return nLang;
}

// static
LanguageType MsLangId::getRealLanguage( LanguageType nLang )
{
    switch (simplifySystemLanguages( nLang))
    {
        case LANGUAGE_SYSTEM :
            if (nConfiguredSystemLanguage == LANGUAGE_SYSTEM)
                nLang = getSystemLanguage();
            else
                nLang = nConfiguredSystemLanguage;
            break;
        case LANGUAGE_HID_HUMAN_INTERFACE_DEVICE :
            if (nConfiguredSystemUILanguage == LANGUAGE_SYSTEM)
                nLang = getSystemUILanguage();
            else
                nLang = nConfiguredSystemUILanguage;
            break;
        default:
            /* TODO: would this be useful here? */
            //nLang = MsLangId::getReplacementForObsoleteLanguage( nLang);
            ;   // nothing
    }
    if (nLang == LANGUAGE_DONTKNOW)
        nLang = LANGUAGE_ENGLISH_US;
    return nLang;
}


// static
LanguageType MsLangId::resolveSystemLanguageByScriptType( LanguageType nLang, sal_Int16 nType )
{
    if (nLang == LANGUAGE_NONE)
        return nLang;

    nLang = getRealLanguage(nLang);
    if (nType != ::com::sun::star::i18n::ScriptType::WEAK && getScriptType(nLang) != nType)
    {
        switch(nType)
        {
            case ::com::sun::star::i18n::ScriptType::ASIAN:
                if (nConfiguredAsianFallback == LANGUAGE_SYSTEM)
                    nLang = LANGUAGE_CHINESE_SIMPLIFIED;
                else
                    nLang = nConfiguredAsianFallback;
                break;
            case ::com::sun::star::i18n::ScriptType::COMPLEX:
                if (nConfiguredComplexFallback == LANGUAGE_SYSTEM)
                    nLang = LANGUAGE_HINDI;
                else
                    nLang = nConfiguredComplexFallback;
                break;
            default:
                if (nConfiguredWesternFallback == LANGUAGE_SYSTEM)
                    nLang = LANGUAGE_ENGLISH_US;
                else
                    nLang = nConfiguredWesternFallback;
                break;
        }
    }
    return nLang;
}


// static
::com::sun::star::lang::Locale MsLangId::Conversion::convertLanguageToLocale(
        LanguageType nLang, bool bResolveSystem )
{
    ::com::sun::star::lang::Locale aLocale;
    if (!bResolveSystem && simplifySystemLanguages( nLang) == LANGUAGE_SYSTEM)
        ;   // nothing => empty locale
    else
    {
        // Still resolve LANGUAGE_DONTKNOW if resolving is not requested,
        // but not LANGUAGE_SYSTEM or others.
        if (bResolveSystem || nLang == LANGUAGE_DONTKNOW)
            nLang = MsLangId::getRealLanguage( nLang);
        convertLanguageToLocaleImpl( nLang, aLocale);
    }
    return aLocale;
}


// static
LanguageType MsLangId::Conversion::convertLocaleToLanguage(
        const ::com::sun::star::lang::Locale& rLocale )
{
    // empty language => LANGUAGE_SYSTEM
    if (rLocale.Language.isEmpty())
        return LANGUAGE_SYSTEM;

    /* FIXME: this x-... is temporary until conversion will be moved up to
     * LanguageTag. Also handle the nasty "*" joker as privateuse. */
    LanguageType nRet = ((!rLocale.Variant.isEmpty() &&
                (rLocale.Variant.startsWithIgnoreAsciiCase( "x-") || (rLocale.Variant == "*"))) ?
            convertPrivateUseToLanguage( rLocale.Variant) :
            convertIsoNamesToLanguage( rLocale.Language, rLocale.Country));
    if (nRet == LANGUAGE_DONTKNOW)
        nRet = LANGUAGE_SYSTEM;

    return nRet;
}


// static
::com::sun::star::lang::Locale MsLangId::getFallbackLocale(
            const ::com::sun::star::lang::Locale & rLocale )
{
    // empty language => LANGUAGE_SYSTEM
    if (rLocale.Language.isEmpty())
        return Conversion::lookupFallbackLocale( Conversion::convertLanguageToLocale( LANGUAGE_SYSTEM, true));
    else
        return Conversion::lookupFallbackLocale( rLocale);
}

// static
bool MsLangId::isRightToLeft( LanguageType nLang )
{
    switch( nLang & LANGUAGE_MASK_PRIMARY )
    {
        case LANGUAGE_ARABIC_SAUDI_ARABIA & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_HEBREW              & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_YIDDISH             & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_URDU                & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_FARSI               & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_KASHMIRI            & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_SINDHI              & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_UIGHUR_CHINA        & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_USER_KYRGYZ_CHINA   & LANGUAGE_MASK_PRIMARY :
        case LANGUAGE_USER_NKO            & LANGUAGE_MASK_PRIMARY :
            return true;

        default:
            break;
    }
    return false;
}

// static
bool MsLangId::isSimplifiedChinese( LanguageType nLang )
{
    return isChinese(nLang) && !isTraditionalChinese(nLang);
}

// static
bool MsLangId::isSimplifiedChinese( const ::com::sun::star::lang::Locale & rLocale )
{
    return rLocale.Language == "zh" && !isTraditionalChinese(rLocale);
}

// static
bool MsLangId::isTraditionalChinese( LanguageType nLang )
{
    bool bRet = false;
    switch (nLang)
    {
        case LANGUAGE_CHINESE_TRADITIONAL:
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_MACAU:
            bRet = true;
        default:
            break;
    }
    return bRet;
}

// static
bool MsLangId::isTraditionalChinese( const ::com::sun::star::lang::Locale & rLocale )
{
    return rLocale.Language == "zh" && (rLocale.Country == "TW" || rLocale.Country == "HK" || rLocale.Country == "MO");
}

//static
bool MsLangId::isChinese( LanguageType nLang )
{
    return MsLangId::getPrimaryLanguage(nLang) == LANGUAGE_CHINESE;
}

//static
bool MsLangId::isKorean( LanguageType nLang )
{
    return MsLangId::getPrimaryLanguage(nLang) == LANGUAGE_KOREAN;
}

// static
bool MsLangId::isCJK( LanguageType nLang )
{
    switch (nLang & LANGUAGE_MASK_PRIMARY)
    {
        case LANGUAGE_CHINESE  & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_JAPANESE & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_KOREAN   & LANGUAGE_MASK_PRIMARY:
            return true;
        default:
            break;
    }
    return false;
}

// static
bool MsLangId::isFamilyNameFirst( LanguageType nLang )
{
    return isCJK(nLang) || nLang == LANGUAGE_HUNGARIAN;
}

// static
bool MsLangId::hasForbiddenCharacters( LanguageType nLang )
{
    return isCJK(nLang);
}


// static
bool MsLangId::needsSequenceChecking( LanguageType nLang )
{
    switch (nLang & LANGUAGE_MASK_PRIMARY)
    {
        case LANGUAGE_BURMESE & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_KHMER   & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_LAO     & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_THAI    & LANGUAGE_MASK_PRIMARY:
            return true;
        default:
            break;
    }
    return false;
}


// static
sal_Int16 MsLangId::getScriptType( LanguageType nLang )
{
    sal_Int16 nScript;
    switch( nLang )
    {
        // CJK
        // all LANGUAGE_CHINESE_... are caught below
        case LANGUAGE_JAPANESE:
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
        case LANGUAGE_USER_KOREAN_NORTH:
            nScript = ::com::sun::star::i18n::ScriptType::ASIAN;
            break;

        // CTL
        // all LANGUAGE_ARABIC_... are caught below
        case LANGUAGE_AMHARIC_ETHIOPIA:
        case LANGUAGE_ASSAMESE:
        case LANGUAGE_BENGALI:
        case LANGUAGE_BENGALI_BANGLADESH:
        case LANGUAGE_BURMESE:
        case LANGUAGE_FARSI:
        case LANGUAGE_HEBREW:
        case LANGUAGE_YIDDISH:
        case LANGUAGE_USER_YIDDISH_US:
        case LANGUAGE_MARATHI:
        case LANGUAGE_PUNJABI:
        case LANGUAGE_GUJARATI:
        case LANGUAGE_HINDI:
        case LANGUAGE_KANNADA:
        case LANGUAGE_KASHMIRI:
        case LANGUAGE_KASHMIRI_INDIA:
        case LANGUAGE_KHMER:
        case LANGUAGE_LAO:
        case LANGUAGE_MALAYALAM:
        case LANGUAGE_MANIPURI:
        case LANGUAGE_MONGOLIAN_MONGOLIAN:
        case LANGUAGE_NEPALI:
        case LANGUAGE_NEPALI_INDIA:
        case LANGUAGE_ORIYA:
        case LANGUAGE_SANSKRIT:
        case LANGUAGE_SINDHI:
        case LANGUAGE_SINDHI_PAKISTAN:
        case LANGUAGE_SINHALESE_SRI_LANKA:
        case LANGUAGE_SYRIAC:
        case LANGUAGE_TAMIL:
        case LANGUAGE_TELUGU:
        case LANGUAGE_THAI:
        case LANGUAGE_TIBETAN:
        case LANGUAGE_DZONGKHA:
        case LANGUAGE_USER_TIBETAN_INDIA:
        case LANGUAGE_URDU:
        case LANGUAGE_URDU_PAKISTAN:
        case LANGUAGE_URDU_INDIA:
        case LANGUAGE_USER_KURDISH_IRAQ:
        case LANGUAGE_USER_KURDISH_IRAN:
        case LANGUAGE_DHIVEHI:
        case LANGUAGE_USER_BODO_INDIA:
        case LANGUAGE_USER_DOGRI_INDIA:
        case LANGUAGE_USER_MAITHILI_INDIA:
        case LANGUAGE_UIGHUR_CHINA:
        case LANGUAGE_USER_LIMBU:
        case LANGUAGE_USER_KYRGYZ_CHINA:
        case LANGUAGE_USER_NKO:
            nScript = ::com::sun::star::i18n::ScriptType::COMPLEX;
            break;

// currently not knowing scripttype - defaulted to LATIN:
/*
#define LANGUAGE_ARMENIAN                   0x042B
#define LANGUAGE_INDONESIAN                 0x0421
#define LANGUAGE_KAZAKH                     0x043F
#define LANGUAGE_KONKANI                    0x0457
#define LANGUAGE_MACEDONIAN                 0x042F
#define LANGUAGE_TATAR                      0x0444
*/

    default:
        switch ( nLang & LANGUAGE_MASK_PRIMARY )
        {
            // CJK catcher
            case LANGUAGE_CHINESE & LANGUAGE_MASK_PRIMARY:
                nScript = ::com::sun::star::i18n::ScriptType::ASIAN;
                break;
            // CTL catcher
            case LANGUAGE_ARABIC_SAUDI_ARABIA & LANGUAGE_MASK_PRIMARY:
                nScript = ::com::sun::star::i18n::ScriptType::COMPLEX;
                break;
            // Western (actually not necessarily Latin but also Cyrillic, for example)
            default:
                nScript = ::com::sun::star::i18n::ScriptType::LATIN;
        }
        break;
    }
    return nScript;
}


// static
LanguageType MsLangId::getReplacementForObsoleteLanguage( LanguageType nLang, bool bUserInterfaceSelection )
{
    switch (nLang)
    {
        default:
            break;  // nothing
        case LANGUAGE_OBSOLETE_USER_LATIN:
            nLang = LANGUAGE_LATIN;
            break;
        case LANGUAGE_OBSOLETE_USER_MAORI:
            nLang = LANGUAGE_MAORI_NEW_ZEALAND;
            break;
        case LANGUAGE_OBSOLETE_USER_KINYARWANDA:
            nLang = LANGUAGE_KINYARWANDA_RWANDA;
            break;
        case LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN:
            nLang = LANGUAGE_UPPER_SORBIAN_GERMANY;
            break;
        case LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN:
            nLang = LANGUAGE_LOWER_SORBIAN_GERMANY;
            break;
        case LANGUAGE_OBSOLETE_USER_OCCITAN:
            nLang = LANGUAGE_OCCITAN_FRANCE;
            break;
        case LANGUAGE_OBSOLETE_USER_BRETON:
            nLang = LANGUAGE_BRETON_FRANCE;
            break;
        case LANGUAGE_OBSOLETE_USER_KALAALLISUT:
            nLang = LANGUAGE_KALAALLISUT_GREENLAND;
            break;
        case LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH:
            nLang = LANGUAGE_LUXEMBOURGISH_LUXEMBOURG;
            break;
        case LANGUAGE_OBSOLETE_USER_KABYLE:
            nLang = LANGUAGE_TAMAZIGHT_LATIN;
            break;

        // The following are not strictly obsolete but should be mapped to a
        // replacement locale when encountered.

        // no_NO is an alias for nb_NO
        case LANGUAGE_NORWEGIAN:
            nLang = LANGUAGE_NORWEGIAN_BOKMAL;
            break;

        // #i94435# A Spanish variant that differs only in collation details we
        // do not support.
        case LANGUAGE_SPANISH_DATED:
            nLang = LANGUAGE_SPANISH_MODERN;
            break;

         // Do not use ca-XV for document content.
         /* TODO: remove in case we implement BCP47 language tags. */
        case LANGUAGE_USER_CATALAN_VALENCIAN:
            if (!bUserInterfaceSelection)
                nLang = LANGUAGE_CATALAN;
            break;
    }
    return nLang;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
