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
#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <i18nlangtag/mslangid.hxx>

// Only very limited few functions that are guaranteed to not be called from
// LanguageTag may use LanguageTag ...
#include <i18nlangtag/languagetag.hxx>


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
    if (nLang.anyOf( LANGUAGE_PROCESS_OR_USER_DEFAULT,
        LANGUAGE_SYSTEM_DEFAULT,
        LANGUAGE_SYSTEM))
        nLang = LANGUAGE_SYSTEM;
    return nLang;
}

// static
LanguageType MsLangId::getRealLanguage( LanguageType nLang )
{
    LanguageType simplifyLang = simplifySystemLanguages( nLang);
    if (simplifyLang == LANGUAGE_SYSTEM )
        nLang = getConfiguredSystemLanguage();
    else if (simplifyLang == LANGUAGE_HID_HUMAN_INTERFACE_DEVICE)
        nLang = getConfiguredSystemUILanguage();
    else
    {
        /* TODO: would this be useful here? */
        //nLang = MsLangId::getReplacementForObsoleteLanguage( nLang);
        ;   // nothing
    }
    if (nLang == LANGUAGE_DONTKNOW)
        nLang = LANGUAGE_ENGLISH_US;
    return nLang;
}


// static
LanguageType MsLangId::getConfiguredSystemLanguage()
{
    if (nConfiguredSystemLanguage != LANGUAGE_SYSTEM)
        return nConfiguredSystemLanguage;
    SAL_WARN("i18nlangtag", "MsLangId::getConfiguredSystemLanguage() - not configured yet");
    return getSystemLanguage();
}


// static
LanguageType MsLangId::getConfiguredSystemUILanguage()
{
    if (nConfiguredSystemUILanguage != LANGUAGE_SYSTEM)
        return nConfiguredSystemUILanguage;
    SAL_WARN("i18nlangtag", "MsLangId::getConfiguredSystemUILanguage() - not configured yet");
    return getSystemUILanguage();
}


// static
LanguageType MsLangId::getSystemLanguage()
{
    return getPlatformSystemLanguage();
}


// static
LanguageType MsLangId::getSystemUILanguage()
{
    return getPlatformSystemUILanguage();
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
bool MsLangId::usesHyphenation(LanguageType nLang)
{
    if (primary(nLang).anyOf(
            primary(LANGUAGE_ARABIC_PRIMARY_ONLY),
            primary(LANGUAGE_FARSI),
            primary(LANGUAGE_KASHMIRI),
            primary(LANGUAGE_KURDISH_ARABIC_IRAQ),
            primary(LANGUAGE_PUNJABI),
            primary(LANGUAGE_SINDHI),
            primary(LANGUAGE_USER_MALAY_ARABIC_MALAYSIA),
            primary(LANGUAGE_SOMALI),
            primary(LANGUAGE_SWAHILI),
            primary(LANGUAGE_URDU_PAKISTAN),
            primary(LANGUAGE_PASHTO),
            primary(LANGUAGE_VIETNAMESE))
        || isCJK(nLang))
    {
        return false;
    }
    return true;
}


// static
css::lang::Locale MsLangId::Conversion::convertLanguageToLocale(
        LanguageType nLang, bool bIgnoreOverride )
{
    css::lang::Locale aLocale;
    // Still resolve LANGUAGE_DONTKNOW if resolving is not requested,
    // but not LANGUAGE_SYSTEM or others.
    LanguageType nOrigLang = nLang;
    nLang = MsLangId::getRealLanguage(nLang);
    convertLanguageToLocaleImpl( nLang, aLocale, bIgnoreOverride );
    if (aLocale.Language.isEmpty() && simplifySystemLanguages(nOrigLang) == LANGUAGE_SYSTEM)
    {
        // None found but resolve requested, last resort is "en-US".
        aLocale.Language = "en";
        aLocale.Country  = "US";
        aLocale.Variant.clear();
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
        return Conversion::lookupFallbackLocale( Conversion::convertLanguageToLocale( LANGUAGE_SYSTEM, false));
    else
        return Conversion::lookupFallbackLocale( rLocale);
}

// static
bool MsLangId::isRightToLeft( LanguageType nLang )
{
    if (primary(nLang).anyOf(
                primary(LANGUAGE_ARABIC_SAUDI_ARABIA),
                primary(LANGUAGE_HEBREW),
                primary(LANGUAGE_YIDDISH),
                primary(LANGUAGE_URDU_PAKISTAN),
                primary(LANGUAGE_FARSI),
                primary(LANGUAGE_KASHMIRI),
                primary(LANGUAGE_SINDHI),
                primary(LANGUAGE_UIGHUR_CHINA),
                primary(LANGUAGE_USER_KYRGYZ_CHINA),
                primary(LANGUAGE_USER_NKO),
                primary(LANGUAGE_USER_SARAIKI)))
    {
        return true;
    }
    if (nLang.anyOf(
        LANGUAGE_USER_KURDISH_IRAN,
        LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ,
        LANGUAGE_KURDISH_ARABIC_IRAQ,
        LANGUAGE_KURDISH_ARABIC_LSO,
        LANGUAGE_USER_KURDISH_SOUTHERN_IRAN,
        LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ,
        LANGUAGE_USER_HUNGARIAN_ROVAS,
        LANGUAGE_USER_MALAY_ARABIC_MALAYSIA,
        LANGUAGE_USER_ROHINGYA_HANIFI,
        LANGUAGE_USER_MALAY_ARABIC_BRUNEI))
    {
            return true;
    }
    if (LanguageTag::isOnTheFlyID(nLang))
        return LanguageTag::getOnTheFlyScriptType(nLang) == LanguageTag::ScriptType::RTL;
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
    return nLang.anyOf(
         LANGUAGE_CHINESE_TRADITIONAL,
         LANGUAGE_CHINESE_HONGKONG,
         LANGUAGE_CHINESE_MACAU);
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
    if (primary(nLang).anyOf(
         primary(LANGUAGE_CHINESE),
         primary(LANGUAGE_YUE_CHINESE_HONGKONG),
         primary(LANGUAGE_JAPANESE),
         primary(LANGUAGE_KOREAN)))
    {
        return true;
    }
    if (LanguageTag::isOnTheFlyID(nLang))
        return LanguageTag::getOnTheFlyScriptType(nLang) == LanguageTag::ScriptType::CJK;
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
    return primary(nLang).anyOf(
        primary(LANGUAGE_BURMESE),
        primary(LANGUAGE_KHMER),
        primary(LANGUAGE_LAO),
        primary(LANGUAGE_THAI))
        || nLang.anyOf(
                LANGUAGE_USER_PALI_THAI);
}


// static
sal_Int16 MsLangId::getScriptType( LanguageType nLang )
{
    sal_Int16 nScript;

        // CTL
    if( nLang == LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA
        || nLang == LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA
        || nLang == LANGUAGE_MONGOLIAN_MONGOLIAN_LSO
        || nLang == LANGUAGE_USER_KURDISH_IRAN
        || nLang == LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ
        || nLang == LANGUAGE_KURDISH_ARABIC_IRAQ
        || nLang == LANGUAGE_KURDISH_ARABIC_LSO
        || nLang == LANGUAGE_USER_KURDISH_SOUTHERN_IRAN
        || nLang == LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ
        || nLang == LANGUAGE_USER_KYRGYZ_CHINA
        || nLang == LANGUAGE_USER_HUNGARIAN_ROVAS
        || nLang == LANGUAGE_USER_MANCHU
        || nLang == LANGUAGE_USER_XIBE
        || nLang == LANGUAGE_USER_MALAY_ARABIC_MALAYSIA
        || nLang == LANGUAGE_USER_MALAY_ARABIC_BRUNEI
        || nLang == LANGUAGE_USER_ROHINGYA_HANIFI
        || nLang == LANGUAGE_USER_PALI_THAI)
    {
            nScript = css::i18n::ScriptType::COMPLEX;
    }
        // "Western"
    else if ( nLang == LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA
        || nLang == LANGUAGE_MONGOLIAN_CYRILLIC_LSO
        || nLang == LANGUAGE_USER_KURDISH_SYRIA
        || nLang == LANGUAGE_USER_KURDISH_TURKEY)
    {
            nScript = css::i18n::ScriptType::LATIN;
    }
            // CJK catcher
    else if ( primary(nLang) == primary(LANGUAGE_CHINESE              )
        || primary(nLang) == primary(LANGUAGE_YUE_CHINESE_HONGKONG )
        || primary(nLang) == primary(LANGUAGE_JAPANESE             )
        || primary(nLang) == primary(LANGUAGE_KOREAN               )
        )
    {
            nScript = css::i18n::ScriptType::ASIAN;
    }
            // CTL catcher
    else if (primary(nLang) == primary(LANGUAGE_AMHARIC_ETHIOPIA    )
        || primary(nLang) == primary(LANGUAGE_ARABIC_SAUDI_ARABIA )
        || primary(nLang) == primary(LANGUAGE_ASSAMESE            )
        || primary(nLang) == primary(LANGUAGE_BENGALI             )
        || primary(nLang) == primary(LANGUAGE_BURMESE             )
        || primary(nLang) == primary(LANGUAGE_DHIVEHI             )
        || primary(nLang) == primary(LANGUAGE_FARSI               )
        || primary(nLang) == primary(LANGUAGE_GUJARATI            )
        || primary(nLang) == primary(LANGUAGE_HEBREW              )
        || primary(nLang) == primary(LANGUAGE_HINDI               )
        || primary(nLang) == primary(LANGUAGE_KANNADA             )
        || primary(nLang) == primary(LANGUAGE_KASHMIRI            )
        || primary(nLang) == primary(LANGUAGE_KHMER               )
        || primary(nLang) == primary(LANGUAGE_KONKANI             )
        || primary(nLang) == primary(LANGUAGE_LAO                 )
        || primary(nLang) == primary(LANGUAGE_MALAYALAM           )
        || primary(nLang) == primary(LANGUAGE_MANIPURI            )
        || primary(nLang) == primary(LANGUAGE_MARATHI             )
        || primary(nLang) == primary(LANGUAGE_NEPALI              )
        || primary(nLang) == primary(LANGUAGE_ODIA                )
        || primary(nLang) == primary(LANGUAGE_PUNJABI             )
        || primary(nLang) == primary(LANGUAGE_SANSKRIT            )
        || primary(nLang) == primary(LANGUAGE_SINDHI              )
        || primary(nLang) == primary(LANGUAGE_SINHALESE_SRI_LANKA )
        || primary(nLang) == primary(LANGUAGE_SYRIAC              )
        || primary(nLang) == primary(LANGUAGE_TAMIL               )
        || primary(nLang) == primary(LANGUAGE_TELUGU              )
        || primary(nLang) == primary(LANGUAGE_THAI                )
        || primary(nLang) == primary(LANGUAGE_TIBETAN             )  // also LANGUAGE_DZONGKHA
        || primary(nLang) == primary(LANGUAGE_TIGRIGNA_ETHIOPIA   )
        || primary(nLang) == primary(LANGUAGE_UIGHUR_CHINA        )
        || primary(nLang) == primary(LANGUAGE_URDU_INDIA          )
        || primary(nLang) == primary(LANGUAGE_USER_BODO_INDIA     )
        || primary(nLang) == primary(LANGUAGE_USER_DOGRI_INDIA    )
        || primary(nLang) == primary(LANGUAGE_USER_LIMBU          )
        || primary(nLang) == primary(LANGUAGE_USER_MAITHILI_INDIA )
        || primary(nLang) == primary(LANGUAGE_USER_NKO            )
        || primary(nLang) == primary(LANGUAGE_USER_SARAIKI        )
        || primary(nLang) == primary(LANGUAGE_YIDDISH             ))
    {
            nScript = css::i18n::ScriptType::COMPLEX;
    }
        // Western (actually not necessarily Latin but also Cyrillic,
        // for example)
    else if (LanguageTag::isOnTheFlyID(nLang))
    {
        switch (LanguageTag::getOnTheFlyScriptType(nLang))
        {
            case LanguageTag::ScriptType::CJK :
                nScript = css::i18n::ScriptType::ASIAN;
                break;
            case LanguageTag::ScriptType::CTL :
            case LanguageTag::ScriptType::RTL :
                nScript = css::i18n::ScriptType::COMPLEX;
                break;
            case LanguageTag::ScriptType::WESTERN :
            case LanguageTag::ScriptType::UNKNOWN :
            default:
                nScript = css::i18n::ScriptType::LATIN;
                break;
        }
    }
    else
    {
        nScript = css::i18n::ScriptType::LATIN;
    }
    return nScript;
}


// static
bool MsLangId::isNonLatinWestern( LanguageType nLang )
{
    if (nLang.anyOf(
        LANGUAGE_AZERI_CYRILLIC,
        LANGUAGE_AZERI_CYRILLIC_LSO,
        LANGUAGE_BELARUSIAN,
        LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA,
        LANGUAGE_BOSNIAN_CYRILLIC_LSO,
        LANGUAGE_BULGARIAN,
        LANGUAGE_GREEK,
        LANGUAGE_MONGOLIAN_CYRILLIC_LSO,
        LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA,
        LANGUAGE_RUSSIAN,
        LANGUAGE_RUSSIAN_MOLDOVA,
        LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA,
        LANGUAGE_SERBIAN_CYRILLIC_LSO,
        LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO,
        LANGUAGE_SERBIAN_CYRILLIC_SAM,
        LANGUAGE_SERBIAN_CYRILLIC_SERBIA,
        LANGUAGE_UKRAINIAN,
        LANGUAGE_UZBEK_CYRILLIC,
        LANGUAGE_UZBEK_CYRILLIC_LSO))
    {
            return true;
    }
    if (getScriptType( nLang) != css::i18n::ScriptType::LATIN)
        return false;
    LanguageTag aLanguageTag( nLang);
    if (aLanguageTag.hasScript())
        return aLanguageTag.getScript() != "Latn";
    return false;
}


// static
bool MsLangId::isLegacy( LanguageType nLang )
{
    return nLang.anyOf(
         LANGUAGE_SERBIAN_CYRILLIC_SAM,
         LANGUAGE_SERBIAN_LATIN_SAM);
            /* TODO: activate once dictionary was renamed from pap-AN to
             * pap-CW, or the pap-CW one supports also pap-AN, see fdo#44112 */
        //case LANGUAGE_PAPIAMENTU:
}


// static
LanguageType MsLangId::getReplacementForObsoleteLanguage( LanguageType nLang )
{
    if (nLang == LANGUAGE_OBSOLETE_USER_LATIN)
        nLang = LANGUAGE_LATIN;
    else if (nLang == LANGUAGE_OBSOLETE_USER_LATIN_VATICAN)
        nLang = LANGUAGE_LATIN;
    else if (nLang == LANGUAGE_OBSOLETE_USER_MAORI)
        nLang = LANGUAGE_MAORI_NEW_ZEALAND;
    else if (nLang == LANGUAGE_OBSOLETE_USER_KINYARWANDA)
        nLang = LANGUAGE_KINYARWANDA_RWANDA;
    else if (nLang == LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN)
        nLang = LANGUAGE_UPPER_SORBIAN_GERMANY;
    else if (nLang == LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN)
        nLang = LANGUAGE_LOWER_SORBIAN_GERMANY;
    else if (nLang == LANGUAGE_OBSOLETE_USER_OCCITAN)
        nLang = LANGUAGE_OCCITAN_FRANCE;
    else if (nLang == LANGUAGE_OBSOLETE_USER_BRETON)
        nLang = LANGUAGE_BRETON_FRANCE;
    else if (nLang == LANGUAGE_OBSOLETE_USER_KALAALLISUT)
        nLang = LANGUAGE_KALAALLISUT_GREENLAND;
    else if (nLang == LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH)
        nLang = LANGUAGE_LUXEMBOURGISH_LUXEMBOURG;
    else if (nLang == LANGUAGE_OBSOLETE_USER_KABYLE)
        nLang = LANGUAGE_TAMAZIGHT_LATIN_ALGERIA;
    else if (nLang == LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN)
        nLang = LANGUAGE_CATALAN_VALENCIAN;
    else if (nLang == LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU)
        nLang = LANGUAGE_MALAGASY_PLATEAU;
    else if (nLang == LANGUAGE_GAELIC_SCOTLAND_LEGACY)
        nLang = LANGUAGE_GAELIC_SCOTLAND;
    else if (nLang == LANGUAGE_OBSOLETE_USER_TSWANA_BOTSWANA)
        nLang = LANGUAGE_TSWANA_BOTSWANA;
    else if (nLang == LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA)
        nLang = LANGUAGE_SERBIAN_LATIN_SERBIA;
    else if (nLang == LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO)
        nLang = LANGUAGE_SERBIAN_LATIN_MONTENEGRO;
    else if (nLang == LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_SERBIA)
        nLang = LANGUAGE_SERBIAN_CYRILLIC_SERBIA;
    else if (nLang == LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_MONTENEGRO)
        nLang = LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO;
    else if (nLang == LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ)
        nLang = LANGUAGE_KURDISH_ARABIC_IRAQ;
    else if (nLang == LANGUAGE_OBSOLETE_USER_SPANISH_CUBA)
        nLang = LANGUAGE_SPANISH_CUBA;
    else if (nLang == LANGUAGE_OBSOLETE_USER_SPANISH_LATIN_AMERICA)
        nLang = LANGUAGE_SPANISH_LATIN_AMERICA;

    // The following are not strictly obsolete but should be mapped to a
    // replacement locale when encountered.

    // no_NO is an alias for nb_NO
    else if (nLang == LANGUAGE_NORWEGIAN)
        nLang = LANGUAGE_NORWEGIAN_BOKMAL;

    // The erroneous Tibetan vs. Dzongkha case, #i53497#
    // We (and MS) have stored LANGUAGE_TIBETAN_BHUTAN. This will need
    // special attention if MS one day decides to actually use
    // LANGUAGE_TIBETAN_BHUTAN for bo-BT instead of having it reserved;
    // then remove the mapping and hope every dz-BT user used ODF to store
    // documents ;-)
    else if (nLang == LANGUAGE_TIBETAN_BHUTAN)
        nLang = LANGUAGE_DZONGKHA_BHUTAN;

    // en-GB-oed is deprecated, use en-GB-oxendict instead.
    else if (nLang == LANGUAGE_USER_ENGLISH_UK_OED)
        nLang = LANGUAGE_USER_ENGLISH_UK_OXENDICT;
    return nLang;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
