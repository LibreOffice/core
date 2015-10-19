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

// Only very limited few functions that are guaranteed to not be called from
// LanguageTag may use LanguageTag ...
#include "i18nlangtag/languagetag.hxx"


LanguageType MsLangId::nConfiguredSystemLanguage   = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredSystemUILanguage = LANGUAGE_SYSTEM;

LanguageType MsLangId::nConfiguredWesternFallback  = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredAsianFallback    = LANGUAGE_SYSTEM;
LanguageType MsLangId::nConfiguredComplexFallback  = LANGUAGE_SYSTEM;

// static
void MsLangId::LanguageTagAccess::setConfiguredSystemLanguage( LanguageType nLang )
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
    if (nType != css::i18n::ScriptType::WEAK && getScriptType(nLang) != nType)
    {
        switch(nType)
        {
            case css::i18n::ScriptType::ASIAN:
                if (nConfiguredAsianFallback == LANGUAGE_SYSTEM)
                    nLang = LANGUAGE_CHINESE_SIMPLIFIED;
                else
                    nLang = nConfiguredAsianFallback;
                break;
            case css::i18n::ScriptType::COMPLEX:
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
css::lang::Locale MsLangId::Conversion::convertLanguageToLocale(
        LanguageType nLang, bool bResolveSystem )
{
    css::lang::Locale aLocale;
    if (!bResolveSystem && simplifySystemLanguages( nLang) == LANGUAGE_SYSTEM)
        ;   // nothing => empty locale
    else
    {
        // Still resolve LANGUAGE_DONTKNOW if resolving is not requested,
        // but not LANGUAGE_SYSTEM or others.
        LanguageType nOrigLang = nLang;
        if (bResolveSystem || nLang == LANGUAGE_DONTKNOW)
            nLang = MsLangId::getRealLanguage( nLang);
        convertLanguageToLocaleImpl( nLang, aLocale, true);
        if (bResolveSystem && aLocale.Language.isEmpty() && simplifySystemLanguages( nOrigLang) == LANGUAGE_SYSTEM)
        {
            // None found but resolve requested, last resort is "en-US".
            aLocale.Language = "en";
            aLocale.Country  = "US";
            aLocale.Variant.clear();
        }
    }
    return aLocale;
}


// static
LanguageType MsLangId::Conversion::convertLocaleToLanguage(
        const css::lang::Locale& rLocale )
{
    // empty language => LANGUAGE_SYSTEM
    if (rLocale.Language.isEmpty())
        return LANGUAGE_SYSTEM;

    return convertLocaleToLanguageImpl( rLocale);
}


// static
css::lang::Locale MsLangId::getFallbackLocale(
            const css::lang::Locale & rLocale )
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
        case LANGUAGE_URDU_PAKISTAN       & LANGUAGE_MASK_PRIMARY :
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
    switch (nLang)
    {
        case LANGUAGE_USER_KURDISH_IRAN:
        case LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ:
        case LANGUAGE_KURDISH_ARABIC_IRAQ:
        case LANGUAGE_KURDISH_ARABIC_LSO:
        case LANGUAGE_USER_KURDISH_SOUTHERN_IRAN:
        case LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ:
            return true;

        default:
            break;
    }
    return false;
}

// static
bool MsLangId::isRightToLeftMath( LanguageType nLang )
{
    //http://www.w3.org/TR/arabic-math/
    if (nLang == LANGUAGE_FARSI || nLang == LANGUAGE_ARABIC_MOROCCO)
        return false;
    return isRightToLeft(nLang);
}

// static
bool MsLangId::isSimplifiedChinese( LanguageType nLang )
{
    return isChinese(nLang) && !isTraditionalChinese(nLang);
}

// static
bool MsLangId::isSimplifiedChinese( const css::lang::Locale & rLocale )
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
bool MsLangId::isTraditionalChinese( const css::lang::Locale & rLocale )
{
    return rLocale.Language == "zh" && (rLocale.Country == "TW" || rLocale.Country == "HK" || rLocale.Country == "MO");
}

//static
bool MsLangId::isChinese( LanguageType nLang )
{
    return MsLangId::getPrimaryLanguage(nLang) == MsLangId::getPrimaryLanguage(LANGUAGE_CHINESE) ||
        MsLangId::getPrimaryLanguage(nLang) == MsLangId::getPrimaryLanguage(LANGUAGE_YUE_CHINESE_HONGKONG);
}

//static
bool MsLangId::isKorean( LanguageType nLang )
{
    return MsLangId::getPrimaryLanguage(nLang) == MsLangId::getPrimaryLanguage(LANGUAGE_KOREAN);
}

// static
bool MsLangId::isCJK( LanguageType nLang )
{
    switch (nLang & LANGUAGE_MASK_PRIMARY)
    {
        case LANGUAGE_CHINESE              & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_YUE_CHINESE_HONGKONG & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_JAPANESE             & LANGUAGE_MASK_PRIMARY:
        case LANGUAGE_KOREAN               & LANGUAGE_MASK_PRIMARY:
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
        // CTL
        case LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA:
        case LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA:
        case LANGUAGE_MONGOLIAN_MONGOLIAN_LSO:
        case LANGUAGE_USER_KURDISH_IRAN:
        case LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ:
        case LANGUAGE_KURDISH_ARABIC_IRAQ:
        case LANGUAGE_KURDISH_ARABIC_LSO:
        case LANGUAGE_USER_KURDISH_SOUTHERN_IRAN:
        case LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ:
        case LANGUAGE_USER_KYRGYZ_CHINA:
            nScript = css::i18n::ScriptType::COMPLEX;
            break;

        // "Western"
        case LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA:
        case LANGUAGE_MONGOLIAN_CYRILLIC_LSO:
        case LANGUAGE_USER_KURDISH_SYRIA:
        case LANGUAGE_USER_KURDISH_TURKEY:
            nScript = css::i18n::ScriptType::LATIN;
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
                case LANGUAGE_CHINESE              & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_YUE_CHINESE_HONGKONG & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_JAPANESE             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_KOREAN               & LANGUAGE_MASK_PRIMARY:
                    nScript = css::i18n::ScriptType::ASIAN;
                    break;

                    // CTL catcher
                case LANGUAGE_AMHARIC_ETHIOPIA    & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_ARABIC_SAUDI_ARABIA & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_ASSAMESE            & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_BENGALI             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_BURMESE             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_DHIVEHI             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_FARSI               & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_GUJARATI            & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_HEBREW              & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_HINDI               & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_KANNADA             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_KASHMIRI            & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_KHMER               & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_LAO                 & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_MALAYALAM           & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_MANIPURI            & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_MARATHI             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_NEPALI              & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_ODIA                & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_PUNJABI             & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_SANSKRIT            & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_SINDHI              & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_SINHALESE_SRI_LANKA & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_SYRIAC              & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_TAMIL               & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_TELUGU              & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_THAI                & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_TIBETAN             & LANGUAGE_MASK_PRIMARY:  // also LANGUAGE_DZONGKHA
                case LANGUAGE_UIGHUR_CHINA        & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_URDU_INDIA          & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_USER_BODO_INDIA     & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_USER_DOGRI_INDIA    & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_USER_LIMBU          & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_USER_MAITHILI_INDIA & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_USER_NKO            & LANGUAGE_MASK_PRIMARY:
                case LANGUAGE_YIDDISH             & LANGUAGE_MASK_PRIMARY:
                    nScript = css::i18n::ScriptType::COMPLEX;
                    break;

                // Western (actually not necessarily Latin but also Cyrillic,
                // for example)
                default:
                    nScript = css::i18n::ScriptType::LATIN;
            }
            break;
    }
    return nScript;
}


// static
bool MsLangId::isNonLatinWestern( LanguageType nLang )
{
    switch (nLang)
    {
        case LANGUAGE_AZERI_CYRILLIC:
        case LANGUAGE_AZERI_CYRILLIC_LSO:
        case LANGUAGE_BELARUSIAN:
        case LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA:
        case LANGUAGE_BOSNIAN_CYRILLIC_LSO:
        case LANGUAGE_BULGARIAN:
        case LANGUAGE_GREEK:
        case LANGUAGE_MONGOLIAN_CYRILLIC_LSO:
        case LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA:
        case LANGUAGE_RUSSIAN:
        case LANGUAGE_RUSSIAN_MOLDOVA:
        case LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA:
        case LANGUAGE_SERBIAN_CYRILLIC_LSO:
        case LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO:
        case LANGUAGE_SERBIAN_CYRILLIC_SAM:
        case LANGUAGE_SERBIAN_CYRILLIC_SERBIA:
        case LANGUAGE_UKRAINIAN:
        case LANGUAGE_UZBEK_CYRILLIC:
        case LANGUAGE_UZBEK_CYRILLIC_LSO:
            return true;
        default:
            {
                if (getScriptType( nLang) != css::i18n::ScriptType::LATIN)
                    return false;
                LanguageTag aLanguageTag( nLang);
                if (aLanguageTag.hasScript())
                    return aLanguageTag.getScript() != "Latn";
            }
    }
    return false;
}


// static
bool MsLangId::isLegacy( LanguageType nLang )
{
    switch (nLang)
    {
        case LANGUAGE_SERBIAN_CYRILLIC_SAM:
        case LANGUAGE_SERBIAN_LATIN_SAM:
            /* TODO: activate once dictionary was renamed from pap-AN to
             * pap-CW, or the pap-CW one supports also pap-AN, see fdo#44112 */
        //case LANGUAGE_PAPIAMENTU:
            return true;
    }
    return false;
}


// static
LanguageType MsLangId::getReplacementForObsoleteLanguage( LanguageType nLang, bool /*bUserInterfaceSelection*/ )
{
    switch (nLang)
    {
        default:
            break;  // nothing
        case LANGUAGE_OBSOLETE_USER_LATIN:
            nLang = LANGUAGE_USER_LATIN_VATICAN;
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
            nLang = LANGUAGE_TAMAZIGHT_LATIN_ALGERIA;
            break;
        case LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN:
            nLang = LANGUAGE_CATALAN_VALENCIAN;
            break;
        case LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU:
            nLang = LANGUAGE_MALAGASY_PLATEAU;
            break;
        case LANGUAGE_GAELIC_SCOTLAND_LEGACY:
            nLang = LANGUAGE_GAELIC_SCOTLAND;
            break;
        case LANGUAGE_OBSOLETE_USER_TSWANA_BOTSWANA:
            nLang = LANGUAGE_TSWANA_BOTSWANA;
            break;
        case LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA:
            nLang = LANGUAGE_SERBIAN_LATIN_SERBIA;
            break;
        case LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO:
            nLang = LANGUAGE_SERBIAN_LATIN_MONTENEGRO;
            break;
        case LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_SERBIA:
            nLang = LANGUAGE_SERBIAN_CYRILLIC_SERBIA;
            break;
        case LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_MONTENEGRO:
            nLang = LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO;
            break;
        case LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ:
            nLang = LANGUAGE_KURDISH_ARABIC_IRAQ;
            break;
        case LANGUAGE_OBSOLETE_USER_SPANISH_CUBA:
            nLang = LANGUAGE_SPANISH_CUBA;
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

        // The erroneous Tibetan vs. Dzongkha case, #i53497#
        // We (and MS) have stored LANGUAGE_TIBETAN_BHUTAN. This will need
        // special attention if MS one day decides to actually use
        // LANGUAGE_TIBETAN_BHUTAN for bo-BT instead of having it reserved;
        // then remove the mapping and hope every dz-BT user used ODF to store
        // documents ;-)
        case LANGUAGE_TIBETAN_BHUTAN:
            nLang = LANGUAGE_DZONGKHA_BHUTAN;
            break;
    }
    return nLang;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
