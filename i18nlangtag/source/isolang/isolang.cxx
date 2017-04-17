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

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include "i18nlangtag/mslangid.hxx"
#include "i18nlangtag/languagetag.hxx"

using namespace com::sun::star;


static const LanguageType kSAME(0xffff);

namespace {
inline LanguageType getOverrideLang( LanguageType nLang, LanguageType nOverride )
{
    return nOverride ? ((nOverride == kSAME) ? nLang : nOverride) : nLang;
}
}

/* Usage of override mechanism:
 * If a table entry's mnOverride is not 0, an override entry with an mnLang
 * value of (mnOverride==kSAME ? mnLang : mnOverride) exists that should be
 * used instead. There MUST exist one such entry that does not have an
 * mnOverride value and within one table it MUST be located before any entry
 * with that mnLang and an mnOverride value of not 0. Usually kSAME is used as
 * override value, with rare exceptions, see tables below.
 *
 * The override serves these purposes:
 * - With getOverride() it indicates that there is a different language tag
 *   (locale) that the requested language tag should be "canonicalized" to.
 * - With lookupFallbackLocale() a locale may be returned where the language
 *   tag differs.
 * - With convertLanguageToLocaleImpl() and bIgnoreOverride=false the override
 *   is followed and the override locale returned.
 * - With convertLocaleToLanguageImpl() a different LangID may be returned in
 *   rare cases where the actual mapped ID differs.
 */

struct IsoLanguageCountryEntry
{
    LanguageType  mnLang;
    sal_Char      maLanguage[4];
    sal_Char      maCountry[3];
    LanguageType  mnOverride;

    /** Obtain a language tag string with '-' separator. */
    OUString getTagString() const;

    /** Obtain a locale. */
    css::lang::Locale getLocale() const;
};

struct IsoLanguageScriptCountryEntry
{
    LanguageType  mnLang;
    sal_Char      maLanguageScript[9];      ///< "ll-Ssss" or "lll-Ssss"
    sal_Char      maCountry[3];
    LanguageType  mnOverride;

    /** Obtain a language tag string with '-' separator. */
    OUString getTagString() const;

    /** Obtain a locale. */
    css::lang::Locale getLocale() const;

    /** If rStr starts with maLanguageScript ignoring case.

        We don't have OUString::startsWithIgnoreAsciiCaseAscii()
     */
    bool startsInIgnoreAsciiCase( const OUString & rStr ) const;
};

struct Bcp47CountryEntry
{
    LanguageType    mnLang;
    const sal_Char* mpBcp47;
    sal_Char        maCountry[3];
    const sal_Char* mpFallback;
    LanguageType    mnOverride;

    /** Obtain a language tag string with '-' separator. */
    OUString getTagString() const;

    /** Obtain a locale. */
    css::lang::Locale getLocale() const;
};

struct IsoLangEngEntry
{
    LanguageType        mnLang;
    sal_Char            maCountry[3];
};

struct IsoLangNoneStdEntry
{
    LanguageType        mnLang;
    sal_Char            maLanguage[4];
    sal_Char            maCountry[9];
};

struct IsoLangOtherEntry
{
    LanguageType        mnLang;
    const sal_Char*     mpLanguage;
};


// Entries for languages are lower case, for countries upper case, as
// recommended by rfc5646 (obsoletes rfc4646 (obsoletes rfc3066 (obsoletes
// rfc1766))). convertIsoNamesToLanguage(), convertLocaleToLanguageImpl()
// and lookupFallbackLocale() are case insensitive.
//
// Sort order: Most used first and within one language the default fallback
// locale of that language first.
//
// The default entry for a LangID <-> ISO mapping has to be first. For
// conversion of legacy mappings one LangID can map to multiple ISO codes
// except if the LangID is primary-only, and one ISO code combination can map
// to multiple LangIDs. For compatibility with already existing calls it can
// also be a sequence as follows:

// LANGUAGE_ENGLISH,    "en", ""
// LANGUAGE_ENGLISH_US, "en", "US"

// Here, in a convertIsoNamesToLanguage() call "en-US" is converted to
// LANGUAGE_ENGLISH_US and "en" is converted to LANGUAGE_ENGLISH. A call with
// "en-ZZ" (not in table) would result in LANGUAGE_ENGLISH because the first
// entry matching the language and not having a country is returned, regardless
// of whether being sorted before or after other entries of the same language
// with some country. To obtain a _locale_ (not language only) in the order
// given, lookupFallbackLocale() must be called.

// If the sequence instead was

// LANGUAGE_ENGLISH_US, "en", "US"
// LANGUAGE_ENGLISH,    "en", ""

// in a convertIsoNamesToLanguage() call "en-US" would still be converted to
// LANGUAGE_ENGLISH_US, but "en" would _also_ be converted to
// LANGUAGE_ENGLISH_US because no country was passed and it is the first entry
// to match the language, see code. A call with "en-ZZ" (not in table) would
// still result in LANGUAGE_ENGLISH.

/* Currently (2013-08-29) only these primary LangID are still used literally in
 * code:
 * LANGUAGE_ENGLISH  LANGUAGE_ARABIC_PRIMARY_ONLY
 */
#define LANGUAGE_ZERO                       LanguageType(0)

static IsoLanguageCountryEntry const aImplIsoLangEntries[] =
{
    // MS-LANGID codes,             ISO639-1/2/3, ISO3166, override
    { LANGUAGE_ENGLISH,                     "en", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_US,                  "en", "US", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_UK,                  "en", "GB", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_AUS,                 "en", "AU", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_CAN,                 "en", "CA", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH,                      "fr", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_GERMAN,                      "de", "DE", LANGUAGE_ZERO     },
    { LANGUAGE_ITALIAN,                     "it", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_DUTCH,                       "nl", "NL", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_MODERN,              "es", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_DATED,               "es", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_PORTUGUESE,                  "pt", "PT", LANGUAGE_ZERO     },
    { LANGUAGE_PORTUGUESE_BRAZILIAN,        "pt", "BR", LANGUAGE_ZERO     },
    { LANGUAGE_DANISH,                      "da", "DK", LANGUAGE_ZERO     },
    { LANGUAGE_GREEK,                       "el", "GR", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_SIMPLIFIED,          "zh", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_SIMPLIFIED_LEGACY,   "zh", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_TRADITIONAL,         "zh", "TW", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_HONGKONG,            "zh", "HK", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_SINGAPORE,           "zh", "SG", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_MACAU,               "zh", "MO", LANGUAGE_ZERO     },
    { LANGUAGE_CHINESE_LSO,                 "zh", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_YUE_CHINESE_HONGKONG,       "yue", "HK", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_HONG_KONG_SAR,       "en", "HK", LANGUAGE_ZERO     },
    { LANGUAGE_JAPANESE,                    "ja", "JP", LANGUAGE_ZERO     },
    { LANGUAGE_KOREAN,                      "ko", "KR", LANGUAGE_ZERO     },
    { LANGUAGE_KOREAN_JOHAB,                "ko", "KR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KOREAN_NORTH,           "ko", "KP", LANGUAGE_ZERO     },
    { LANGUAGE_SWEDISH,                     "sv", "SE", LANGUAGE_ZERO     },
    { LANGUAGE_SWEDISH_FINLAND,             "sv", "FI", LANGUAGE_ZERO     },
    { LANGUAGE_FINNISH,                     "fi", "FI", LANGUAGE_ZERO     },
    { LANGUAGE_RUSSIAN,                     "ru", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_TATAR,                       "tt", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_NZ,                  "en", "NZ", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_EIRE,                "en", "IE", LANGUAGE_ZERO     },
    { LANGUAGE_DUTCH_BELGIAN,               "nl", "BE", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_BELGIAN,              "fr", "BE", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_CANADIAN,             "fr", "CA", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_SWISS,                "fr", "CH", LANGUAGE_ZERO     },
    { LANGUAGE_GERMAN_SWISS,                "de", "CH", LANGUAGE_ZERO     },
    { LANGUAGE_GERMAN_AUSTRIAN,             "de", "AT", LANGUAGE_ZERO     },
    { LANGUAGE_ITALIAN_SWISS,               "it", "CH", LANGUAGE_ZERO     },
    { LANGUAGE_ALBANIAN,                    "sq", "AL", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_SAUDI_ARABIA,         "ar", "SA", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_EGYPT,                "ar", "EG", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_UAE,                  "ar", "AE", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_IRAQ,                 "ar", "IQ", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_LIBYA,                "ar", "LY", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_ALGERIA,              "ar", "DZ", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_MOROCCO,              "ar", "MA", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_TUNISIA,              "ar", "TN", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_OMAN,                 "ar", "OM", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_YEMEN,                "ar", "YE", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_SYRIA,                "ar", "SY", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_JORDAN,               "ar", "JO", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_LEBANON,              "ar", "LB", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_KUWAIT,               "ar", "KW", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_BAHRAIN,              "ar", "BH", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_QATAR,                "ar", "QA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_CHAD,            "ar", "TD", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_COMOROS,         "ar", "KM", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_DJIBOUTI,        "ar", "DJ", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_ERITREA,         "ar", "ER", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_ISRAEL,          "ar", "IL", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_MAURITANIA,      "ar", "MR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_PALESTINE,       "ar", "PS", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_SOMALIA,         "ar", "SO", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARABIC_SUDAN,           "ar", "SD", LANGUAGE_ZERO     },
    { LANGUAGE_ARABIC_PRIMARY_ONLY,         "ar", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_BASQUE,                      "eu", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_BASQUE,                      "eu", ""  , kSAME },    // our earlier definition
    { LANGUAGE_BULGARIAN,                   "bg", "BG", LANGUAGE_ZERO     },
    { LANGUAGE_CZECH,                       "cs", "CZ", LANGUAGE_ZERO     },
    { LANGUAGE_CZECH,                       "cz", ""  , kSAME },
    { LANGUAGE_ENGLISH_JAMAICA,             "en", "JM", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_CARRIBEAN,           "en", "BS", LANGUAGE_ZERO     },    // not 100%, because AG is Bahamas
    { LANGUAGE_ENGLISH_BELIZE,              "en", "BZ", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_TRINIDAD,            "en", "TT", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_ZIMBABWE,            "en", "ZW", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_INDONESIA,           "en", "ID", LANGUAGE_ZERO     },
    { LANGUAGE_ESTONIAN,                    "et", "EE", LANGUAGE_ZERO     },
    { LANGUAGE_FAEROESE,                    "fo", "FO", LANGUAGE_ZERO     },
    { LANGUAGE_FARSI,                       "fa", "IR", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_LUXEMBOURG,           "fr", "LU", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_MONACO,               "fr", "MC", LANGUAGE_ZERO     },
    { LANGUAGE_GERMAN_LUXEMBOURG,           "de", "LU", LANGUAGE_ZERO     },
    { LANGUAGE_GERMAN_LIECHTENSTEIN,        "de", "LI", LANGUAGE_ZERO     },
    { LANGUAGE_HEBREW,                      "he", "IL", LANGUAGE_ZERO     },    // new: old was "iw"
    { LANGUAGE_HEBREW,                      "iw", "IL", kSAME },    // old: new is "he"
    { LANGUAGE_HUNGARIAN,                   "hu", "HU", LANGUAGE_ZERO     },
    { LANGUAGE_ICELANDIC,                   "is", "IS", LANGUAGE_ZERO     },
    { LANGUAGE_INDONESIAN,                  "id", "ID", LANGUAGE_ZERO     },    // new: old was "in"
    { LANGUAGE_INDONESIAN,                  "in", "ID", kSAME },    // old: new is "id"
    { LANGUAGE_NORWEGIAN,                   "no", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "nb", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_NORWEGIAN_BOKMAL_LSO,        "nb", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "nn", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_NORWEGIAN_NYNORSK_LSO,       "nn", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_POLISH,                      "pl", "PL", LANGUAGE_ZERO     },
    { LANGUAGE_RHAETO_ROMAN,                "rm", "CH", LANGUAGE_ZERO     },
    { LANGUAGE_ROMANIAN,                    "ro", "RO", LANGUAGE_ZERO     },
    { LANGUAGE_ROMANIAN_MOLDOVA,            "ro", "MD", LANGUAGE_ZERO     },
    { LANGUAGE_SLOVAK,                      "sk", "SK", LANGUAGE_ZERO     },
    { LANGUAGE_SLOVENIAN,                   "sl", "SI", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_MEXICAN,             "es", "MX", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_GUATEMALA,           "es", "GT", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_COSTARICA,           "es", "CR", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_PANAMA,              "es", "PA", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,  "es", "DO", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_VENEZUELA,           "es", "VE", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_COLOMBIA,            "es", "CO", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_PERU,                "es", "PE", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_ARGENTINA,           "es", "AR", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_ECUADOR,             "es", "EC", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_CHILE,               "es", "CL", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_URUGUAY,             "es", "UY", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_PARAGUAY,            "es", "PY", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_BOLIVIA,             "es", "BO", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_EL_SALVADOR,         "es", "SV", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_HONDURAS,            "es", "HN", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_NICARAGUA,           "es", "NI", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_PUERTO_RICO,         "es", "PR", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_UNITED_STATES,       "es", "US", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_LATIN_AMERICA,       "es", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_TURKISH,                     "tr", "TR", LANGUAGE_ZERO     },
    { LANGUAGE_UKRAINIAN,                   "uk", "UA", LANGUAGE_ZERO     },
    { LANGUAGE_VIETNAMESE,                  "vi", "VN", LANGUAGE_ZERO     },
    { LANGUAGE_LATVIAN,                     "lv", "LV", LANGUAGE_ZERO     },
    { LANGUAGE_MACEDONIAN,                  "mk", "MK", LANGUAGE_ZERO     },
    { LANGUAGE_MALAY_MALAYSIA,              "ms", "MY", LANGUAGE_ZERO     },
    { LANGUAGE_MALAY_BRUNEI_DARUSSALAM,     "ms", "BN", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_MALAYSIA,            "en", "MY", LANGUAGE_ZERO     },
    { LANGUAGE_THAI,                        "th", "TH", LANGUAGE_ZERO     },
    { LANGUAGE_LITHUANIAN,                  "lt", "LT", LANGUAGE_ZERO     },
    { LANGUAGE_LITHUANIAN_CLASSIC,          "lt", "LT", LANGUAGE_ZERO     },
    { LANGUAGE_CROATIAN,                    "hr", "HR", LANGUAGE_ZERO     },    // Croatian in Croatia
    { LANGUAGE_CROATIAN_BOSNIA_HERZEGOVINA, "hr", "BA", LANGUAGE_ZERO     },
    { LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA,          "bs", "BA", LANGUAGE_ZERO     },
    { LANGUAGE_BOSNIAN_LSO,                               "bs", ""  , LANGUAGE_ZERO     },  // so what is 'bs' vs 'bs-Latn'?
    { LANGUAGE_SERBIAN_CYRILLIC_SERBIA,                   "sr", "RS", LANGUAGE_ZERO     },  // Serbian Cyrillic in Serbia
    { LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_SERBIA,     "sr", "RS", LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,                      "sr", "CS", LANGUAGE_ZERO     },  // Serbian Cyrillic in Serbia and Montenegro
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,                      "sr", "YU", kSAME },  // legacy Serbian Cyrillic in Serbia and Montenegro (former Yugoslavia); kludge, sr_CS not supported by ICU 2.6 (3.4 does)
    { LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO,               "sr", "ME", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_MONTENEGRO, "sr", "ME", LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA,       "sr", "BA", LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_CYRILLIC_LSO,                      "sr", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_LATIN_SERBIA,                      "sh", "RS", kSAME },  // legacy kludge, is sr-Latn-RS now
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA,        "sh", "RS", kSAME },  // legacy kludge, is sr-Latn-RS now
    { LANGUAGE_SERBIAN_LATIN_SAM,                         "sh", "CS", kSAME },  // legacy kludge, is sr-Latn-CS now
    { LANGUAGE_SERBIAN_LATIN_SAM,                         "sh", "YU", kSAME },  // legacy kludge, is sr-Latn-YU now
    { LANGUAGE_SERBIAN_LATIN_MONTENEGRO,                  "sh", "ME", kSAME },  // legacy kludge, is sr-Latn-ME now
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO,    "sh", "ME", kSAME },  // legacy kludge, is sr-Latn-ME now
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,          "sh", "BA", kSAME },  // legacy kludge, is sr-Latn-BA now
    { LANGUAGE_SERBIAN_LATIN_LSO,                         "sh", ""  , kSAME },  // legacy kludge, is sr-Latn now
    { LANGUAGE_ARMENIAN,                    "hy", "AM", LANGUAGE_ZERO     },
    { LANGUAGE_AZERI_LATIN,                 "az", "AZ", LANGUAGE_ZERO     },    // macrolanguage code
    { LANGUAGE_UZBEK_LATIN,                 "uz", "UZ", LANGUAGE_ZERO     },    // macrolanguage code
    { LANGUAGE_UZBEK_LATIN_LSO,             "uz", ""  , LANGUAGE_ZERO     },    // macrolanguage code
    { LANGUAGE_BENGALI_BANGLADESH,          "bn", "BD", LANGUAGE_ZERO     },
    { LANGUAGE_BENGALI,                     "bn", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_BURMESE,                     "my", "MM", LANGUAGE_ZERO     },
    { LANGUAGE_KAZAKH,                      "kk", "KZ", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_INDIA,               "en", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_URDU_INDIA,                  "ur", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_URDU_PAKISTAN,               "ur", "PK", LANGUAGE_ZERO     },
    { LANGUAGE_HINDI,                       "hi", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_GUJARATI,                    "gu", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_KANNADA,                     "kn", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_ASSAMESE,                    "as", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_KASHMIRI_INDIA,              "ks", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_KASHMIRI,                    "ks", ""  , kSAME },    // Kashmiri in "Jammu and Kashmir" ... no ISO3166 code for that
    { LANGUAGE_MALAYALAM,                   "ml", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_MANIPURI,                   "mni", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_MARATHI,                     "mr", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_KONKANI,                    "kok", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_NEPALI,                      "ne", "NP", LANGUAGE_ZERO     },
    { LANGUAGE_NEPALI_INDIA,                "ne", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_ODIA,                        "or", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI,                     "pa", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_SANSKRIT,                    "sa", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_TAMIL,                       "ta", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_TAMIL_SRI_LANKA,             "ta", "LK", LANGUAGE_ZERO     },
    { LANGUAGE_TELUGU,                      "te", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI_PAKISTAN,           "pnb", "PK", LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI_ARABIC_LSO,         "pnb", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI_PAKISTAN,           "lah", "PK", kSAME },    // macrolanguage code, earlier preferred 'lah' over 'pa' for Western Panjabi, now there is 'pnb'
    { LANGUAGE_PUNJABI_PAKISTAN,            "pa", "PK", kSAME },    // MS maps this to 'pa-Arab-PK', but 'pa'='pan' Eastern Panjabi is not used in PK, only in
    { LANGUAGE_SINDHI_PAKISTAN,             "sd", "PK", kSAME },    // Arabic script
    { LANGUAGE_SINDHI,                      "sd", "IN", LANGUAGE_ZERO     },    // TODO: there's Deva(nagari) and Arab(ic) script, MS maps this to 'sd-Deva-IN'
    { LANGUAGE_BELARUSIAN,                  "be", "BY", LANGUAGE_ZERO     },
    { LANGUAGE_CATALAN,                     "ca", "ES", LANGUAGE_ZERO     },    // Spain (default)
    { LANGUAGE_CATALAN,                     "ca", "AD", LANGUAGE_ZERO     },    // Andorra
    //LANGUAGE_CATALAN_VALENCIAN ca-ES-valencia Bcp47CountryEntry takes precedence
    { LANGUAGE_CATALAN_VALENCIAN,           "ca", "XV", kSAME },    // XV: ISO 3166 user-assigned; old workaround for UI localization only, in case it escaped to document content
    { LANGUAGE_CATALAN_VALENCIAN,          "qcv", "ES", kSAME },    // qcv: ISO 639-3 reserved-for-local-use; old UI localization quirk only, in case it escaped to document content
    { LANGUAGE_FRENCH_CAMEROON,             "fr", "CM", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_COTE_D_IVOIRE,        "fr", "CI", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_MALI,                 "fr", "ML", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_SENEGAL,              "fr", "SN", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_ZAIRE,                "fr", "CD", LANGUAGE_ZERO     },    // Democratic Republic Of Congo
    { LANGUAGE_FRENCH_MOROCCO,              "fr", "MA", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_REUNION,              "fr", "RE", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH,                      "fr", ""  , LANGUAGE_ZERO     },    // needed as a catcher before other "fr" entries!
    { LANGUAGE_FRENCH_NORTH_AFRICA,         "fr", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_WEST_INDIES,          "fr", ""  , LANGUAGE_ZERO     },    // no ISO country code; MS "Neither defined nor reserved"
    { LANGUAGE_FRISIAN_NETHERLANDS,         "fy", "NL", LANGUAGE_ZERO     },
    { LANGUAGE_GAELIC_IRELAND,              "ga", "IE", LANGUAGE_ZERO     },
    { LANGUAGE_GAELIC_SCOTLAND,             "gd", "GB", LANGUAGE_ZERO     },
    { LANGUAGE_GAELIC_SCOTLAND_LEGACY,      "gd", "GB", LANGUAGE_ZERO     },
    { LANGUAGE_GALICIAN,                    "gl", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_GEORGIAN,                    "ka", "GE", LANGUAGE_ZERO     },
    { LANGUAGE_KHMER,                       "km", "KH", LANGUAGE_ZERO     },
    { LANGUAGE_KIRGHIZ,                     "ky", "KG", LANGUAGE_ZERO     },
    { LANGUAGE_LAO,                         "lo", "LA", LANGUAGE_ZERO     },
    { LANGUAGE_MALTESE,                     "mt", "MT", LANGUAGE_ZERO     },
    { LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA, "mn", "MN", LANGUAGE_ZERO     },    // macrolanguage code; should be khk-MN; Cyrillic script
    { LANGUAGE_MONGOLIAN_CYRILLIC_LSO,      "mn", ""  , LANGUAGE_ZERO     },    // macrolanguage code; should be khk; Cyrillic script
    { LANGUAGE_ROMANIAN_MOLDOVA,            "ro", "MD", LANGUAGE_ZERO     },
    { LANGUAGE_ROMANIAN_MOLDOVA,            "mo", "MD", LANGUAGE_ZERO     },    // mo-MD was associated with Russian Moldova LCID, apparently an error; 'mo' is retired, merged with 'ro', see http://www-01.sil.org/iso639-3/documentation.asp?id=mol
    { LANGUAGE_RUSSIAN_MOLDOVA,             "ru", "MD", LANGUAGE_ZERO     },    // as per [MS-LCID] rev. 7.0 2015-06-30
    { LANGUAGE_SWAHILI,                     "sw", "KE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SWAHILI_TANZANIA,       "sw", "TZ", LANGUAGE_ZERO     },
    { LANGUAGE_TAJIK,                       "tg", "TJ", LANGUAGE_ZERO     },
    { LANGUAGE_TAJIK_LSO,                   "tg", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_TIBETAN,                     "bo", "CN", LANGUAGE_ZERO     },    // CN politically correct?
    { LANGUAGE_USER_TIBETAN_INDIA,          "bo", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TIBETAN_BHUTAN,         "bo", "BT", LANGUAGE_ZERO     },
    { LANGUAGE_DZONGKHA_BHUTAN,             "dz", "BT", LANGUAGE_ZERO     },
    { LANGUAGE_TIBETAN_BHUTAN,              "dz", "BT", LANGUAGE_ZERO     },    // MS reserved for bo-BT, but LCID was used as Dzongkha, see #i53497#
    { LANGUAGE_USER_DZONGKHA_MAP_LONLY,     "dz", ""  , LANGUAGE_ZERO     },    // because of the MS error, see lang.h
    { LANGUAGE_TURKMEN,                     "tk", "TM", LANGUAGE_ZERO     },
    { LANGUAGE_WELSH,                       "cy", "GB", LANGUAGE_ZERO     },
    { LANGUAGE_SESOTHO,                     "st", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_SEPEDI,                     "nso", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_SEPEDI,                      "ns", "ZA", kSAME },    // fake "ns" for compatibility with existing OOo1.1.x localization to be able to read those documents
    { LANGUAGE_TSONGA,                      "ts", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_TSWANA,                      "tn", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_SAFRICA,             "en", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_AFRIKAANS,                   "af", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_VENDA,                       "ve", "ZA", LANGUAGE_ZERO     },    // default 639-1
    { LANGUAGE_VENDA,                      "ven", "ZA", kSAME },    // 639-2 may have been used temporarily since 2004-07-23
    { LANGUAGE_XHOSA,                       "xh", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_ZULU,                        "zu", "ZA", LANGUAGE_ZERO     },
//  { LANGUAGE_QUECHUA_COLOMBIA,           "quc", "CO", LANGUAGE_ZERO     },    // MS reserved, and looks wrong, quc would be in Guatemala, not Colombia
    { LANGUAGE_QUECHUA_ECUADOR,            "quz", "EC", LANGUAGE_ZERO     },    // MS
    { LANGUAGE_QUECHUA_ECUADOR,             "qu", "EC", kSAME },    // macrolanguage code
    { LANGUAGE_QUECHUA_PERU,               "quz", "PE", LANGUAGE_ZERO     },    // MS
    { LANGUAGE_QUECHUA_PERU,                "qu", "PE", kSAME },    // macrolanguage code
    { LANGUAGE_QUECHUA_BOLIVIA,             "qu", "BO", LANGUAGE_ZERO     },    // macrolanguage code, TODO instead: quh-BO or qul-BO; MS says quz-BO which is wrong
    { LANGUAGE_PASHTO,                      "ps", "AF", LANGUAGE_ZERO     },
    { LANGUAGE_OROMO,                       "om", "ET", LANGUAGE_ZERO     },
    { LANGUAGE_DHIVEHI,                     "dv", "MV", LANGUAGE_ZERO     },
    { LANGUAGE_UIGHUR_CHINA,                "ug", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_TIGRIGNA_ETHIOPIA,           "ti", "ET", LANGUAGE_ZERO     },
    { LANGUAGE_TIGRIGNA_ERITREA,            "ti", "ER", LANGUAGE_ZERO     },
    { LANGUAGE_AMHARIC_ETHIOPIA,            "am", "ET", LANGUAGE_ZERO     },
    { LANGUAGE_GUARANI_PARAGUAY,           "gug", "PY", LANGUAGE_ZERO     },
    { LANGUAGE_HAWAIIAN_UNITED_STATES,     "haw", "US", LANGUAGE_ZERO     },
    { LANGUAGE_EDO,                        "bin", "NG", LANGUAGE_ZERO     },
    { LANGUAGE_FULFULDE_NIGERIA,           "fuv", "NG", LANGUAGE_ZERO     },
    { LANGUAGE_FULFULDE_NIGERIA,            "ff", "NG", kSAME },    // macrolanguage code
    { LANGUAGE_FULFULDE_SENEGAL,            "ff", "SN", LANGUAGE_ZERO     },    // macrolanguage code
    { LANGUAGE_HAUSA_NIGERIA,               "ha", "NG", kSAME },
    { LANGUAGE_USER_HAUSA_GHANA,            "ha", "GH", kSAME },
    { LANGUAGE_IGBO_NIGERIA,                "ig", "NG", LANGUAGE_ZERO     },
    { LANGUAGE_KANURI_NIGERIA,              "kr", "NG", LANGUAGE_ZERO     },
    { LANGUAGE_YORUBA,                      "yo", "NG", LANGUAGE_ZERO     },
    { LANGUAGE_SOMALI,                      "so", "SO", LANGUAGE_ZERO     },
    { LANGUAGE_PAPIAMENTU,                 "pap", "AN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PAPIAMENTU_ARUBA,      "pap", "AW", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PAPIAMENTU_CURACAO,    "pap", "CW", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PAPIAMENTU_BONAIRE,    "pap", "BQ", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_SINGAPORE,           "en", "SG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_YIDDISH_US,             "yi", "US", LANGUAGE_ZERO     },
    { LANGUAGE_YIDDISH,                     "yi", "IL", LANGUAGE_ZERO     },    // new: old was "ji"
    { LANGUAGE_YIDDISH,                     "ji", "IL", kSAME },    // old: new is "yi"
    { LANGUAGE_SYRIAC,                     "syr", "TR", LANGUAGE_ZERO     },    // "TR" according to http://www.ethnologue.com/show_language.asp?code=SYC
    { LANGUAGE_SINHALESE_SRI_LANKA,         "si", "LK", LANGUAGE_ZERO     },
    { LANGUAGE_CHEROKEE_UNITED_STATES,     "chr", "US", kSAME },
    { LANGUAGE_INUKTITUT_LATIN_CANADA,      "iu", "CA", kSAME },    // macrolanguage code
    { LANGUAGE_INUKTITUT_LATIN_LSO,         "iu", ""  , kSAME },    // macrolanguage code
    { LANGUAGE_SAMI_NORTHERN_NORWAY,        "se", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_INARI,                 "smn", "FI", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_INARI_LSO,             "smn", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_LULE_NORWAY,           "smj", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_LULE_SWEDEN,           "smj", "SE", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_LULE_LSO,              "smj", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_NORTHERN_FINLAND,       "se", "FI", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_NORTHERN_SWEDEN,        "se", "SE", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_SKOLT,                 "sms", "FI", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_SKOLT_LSO,             "sms", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_SOUTHERN_NORWAY,       "sma", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_SOUTHERN_SWEDEN,       "sma", "SE", LANGUAGE_ZERO     },
    { LANGUAGE_SAMI_SOUTHERN_LSO,          "sma", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_SAMI_KILDIN_RUSSIA,    "sjd", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_MAPUDUNGUN_CHILE,           "arn", "CL", LANGUAGE_ZERO     },
    { LANGUAGE_CORSICAN_FRANCE,             "co", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_ALSATIAN_FRANCE,            "gsw", "FR", LANGUAGE_ZERO     },    // in fact 'gsw' is Schwyzerduetsch (Swiss German), which is a dialect of Alemannic German, as is Alsatian. They aren't distinct languages and share this code.
    { LANGUAGE_YAKUT_RUSSIA,               "sah", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_MOHAWK_CANADA,              "moh", "CA", LANGUAGE_ZERO     },
    { LANGUAGE_BASHKIR_RUSSIA,              "ba", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_KICHE_GUATEMALA,            "qut", "GT", LANGUAGE_ZERO     },
    { LANGUAGE_DARI_AFGHANISTAN,           "prs", "AF", LANGUAGE_ZERO     },
    { LANGUAGE_DARI_AFGHANISTAN,           "gbz", "AF", kSAME },    // was an error
    { LANGUAGE_WOLOF_SENEGAL,               "wo", "SN", LANGUAGE_ZERO     },
    { LANGUAGE_FILIPINO,                   "fil", "PH", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TAGALOG,                "tl", "PH", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_PHILIPPINES,         "en", "PH", LANGUAGE_ZERO     },
    { LANGUAGE_IBIBIO_NIGERIA,             "ibb", "NG", LANGUAGE_ZERO     },
    { LANGUAGE_YI,                          "ii", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_ENGLISH_ARAB_EMIRATES,       "en", "AE", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_ENGLISH_BAHRAIN,             "en", "BH", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_ENGLISH_EGYPT,               "en", "EG", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_ENGLISH_JORDAN,              "en", "JO", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_ENGLISH_KUWAIT,              "en", "KW", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_ENGLISH_TURKEY,              "en", "TR", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_ENGLISH_YEMEN,               "en", "YE", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_TAMAZIGHT_LATIN_ALGERIA,    "kab", "DZ", LANGUAGE_ZERO     },    // In practice Kabyle is the language used for this
    { LANGUAGE_OBSOLETE_USER_KABYLE,       "kab", "DZ", LANGUAGE_ZERO     },
    { LANGUAGE_TAMAZIGHT_LATIN_ALGERIA,    "ber", "DZ", kSAME },    // In practice Algeria has standardized on Kabyle as the member of the "ber" collective which gets used there.
    { LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO, "tmz", "MA", kSAME },
    { LANGUAGE_TAMAZIGHT_MOROCCO,          "tmz", "MA", LANGUAGE_ZERO     },    // MS reserved
    { LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO, "ber", "MA", kSAME },    // Morocco is officially using Tifinagh for its Berber languages, old kludge to distinguish from LANGUAGE_TAMAZIGHT_LATIN_ALGERIA
    { LANGUAGE_USER_LATIN_VATICAN,          "la", "VA", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_LATIN,         "la", "VA", LANGUAGE_ZERO     },
    { LANGUAGE_LATIN_LSO,                   "la", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_ESPERANTO,              "eo", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_INTERLINGUA,            "ia", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_INTERLINGUE,            "ie", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_MAORI_NEW_ZEALAND,           "mi", "NZ", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_MAORI,         "mi", "NZ", LANGUAGE_ZERO     },
    { LANGUAGE_KINYARWANDA_RWANDA,          "rw", "RW", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_KINYARWANDA,   "rw", "RW", LANGUAGE_ZERO     },
    { LANGUAGE_UPPER_SORBIAN_GERMANY,      "hsb", "DE", LANGUAGE_ZERO     },    // MS maps this to 'wen-DE', which is nonsense. 'wen' is a collective language code, 'WEN' is a SIL code, see http://www.ethnologue.com/14/show_iso639.asp?code=wen and http://www.ethnologue.com/14/show_language.asp?code=WEN
    { LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN,"hsb", "DE", LANGUAGE_ZERO     },
    { LANGUAGE_LOWER_SORBIAN_GERMANY,      "dsb", "DE", LANGUAGE_ZERO     },    // MS maps this to 'wee-DE', which is nonsense. 'WEE' is a SIL code, see http://www.ethnologue.com/14/show_language.asp?code=WEE
    { LANGUAGE_LOWER_SORBIAN_LSO,          "dsb", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN,"dsb", "DE", LANGUAGE_ZERO     },
    { LANGUAGE_OCCITAN_FRANCE,              "oc", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_OCCITAN,       "oc", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KURDISH_TURKEY,        "kmr", "TR", kSAME },
    { LANGUAGE_USER_KURDISH_TURKEY,         "ku", "TR", kSAME },
    { LANGUAGE_USER_KURDISH_SYRIA,         "kmr", "SY", kSAME },
    { LANGUAGE_USER_KURDISH_SYRIA,          "ku", "SY", kSAME },
    { LANGUAGE_KURDISH_ARABIC_IRAQ,        "ckb", "IQ", LANGUAGE_ZERO     },
    { LANGUAGE_KURDISH_ARABIC_IRAQ,         "ku", "IQ", kSAME },
    { LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ,  "ku", "IQ", LANGUAGE_KURDISH_ARABIC_IRAQ },
    { LANGUAGE_USER_KURDISH_SOUTHERN_IRAN, "sdh", "IR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ, "sdh", "IQ", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KURDISH_IRAN,          "ckb", "IR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KURDISH_IRAN,           "ku", "IR", kSAME },
    { LANGUAGE_KURDISH_ARABIC_LSO,         "ckb", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_SARDINIAN,              "sc", "IT", LANGUAGE_ZERO     },    // macrolanguage code
    { LANGUAGE_USER_SARDINIAN_CAMPIDANESE, "sro", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SARDINIAN_GALLURESE,   "sdn", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SARDINIAN_LOGUDORESE,  "src", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SARDINIAN_SASSARESE,   "sdc", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_BRETON_FRANCE,               "br", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_BRETON,        "br", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_KALAALLISUT_GREENLAND,       "kl", "GL", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_KALAALLISUT,   "kl", "GL", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SWAZI,                  "ss", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NDEBELE_SOUTH,          "nr", "ZA", LANGUAGE_ZERO     },
    { LANGUAGE_TSWANA_BOTSWANA,             "tn", "BW", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_TSWANA_BOTSWANA, "tn", "BW", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ENGLISH_BOTSWANA,       "en", "BW", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MOORE,                 "mos", "BF", LANGUAGE_ZERO     },
    { LANGUAGE_USER_BAMBARA,                "bm", "ML", LANGUAGE_ZERO     },
    { LANGUAGE_USER_AKAN,                   "ak", "GH", LANGUAGE_ZERO     },
    { LANGUAGE_LUXEMBOURGISH_LUXEMBOURG,    "lb", "LU", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH, "lb", "LU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FRIULIAN,              "fur", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FIJIAN,                 "fj", "FJ", LANGUAGE_ZERO     },
    { LANGUAGE_USER_AFRIKAANS_NAMIBIA,      "af", "NA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ENGLISH_NAMIBIA,        "en", "NA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_WALLOON,                "wa", "BE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_COPTIC,                "cop", "EG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_GASCON,                "gsc", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_GERMAN_BELGIUM,         "de", "BE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_CHUVASH,                "cv", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_EWE_GHANA,              "ee", "GH", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ENGLISH_GHANA,          "en", "GH", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SANGO,                  "sg", "CF", LANGUAGE_ZERO     },
    { LANGUAGE_USER_GANDA,                  "lg", "UG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LINGALA_DRCONGO,        "ln", "CD", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LOW_GERMAN,            "nds", "DE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_HILIGAYNON,            "hil", "PH", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ENGLISH_MALAWI,         "en", "MW", LANGUAGE_ZERO     },   /* en default for MW */
    { LANGUAGE_USER_NYANJA,                 "ny", "MW", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KASHUBIAN,             "csb", "PL", LANGUAGE_ZERO     },
    { LANGUAGE_SPANISH_CUBA,                "es", "CU", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_SPANISH_CUBA,  "es", "CU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_QUECHUA_NORTH_BOLIVIA, "qul", "BO", LANGUAGE_ZERO     },
    { LANGUAGE_USER_QUECHUA_SOUTH_BOLIVIA, "quh", "BO", LANGUAGE_ZERO     },
    { LANGUAGE_USER_BODO_INDIA,            "brx", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_DOGRI_INDIA,           "dgo", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MAITHILI_INDIA,        "mai", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SANTALI_INDIA,         "sat", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TETUN,                 "tet", "ID", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TETUN_TIMOR_LESTE,     "tet", "TL", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TOK_PISIN,             "tpi", "PG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SHUSWAP,               "shs", "CA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ANCIENT_GREEK,         "grc", "GR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ASTURIAN,              "ast", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LATGALIAN,             "ltg", "LV", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MAORE,                 "swb", "YT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_BUSHI,                 "buc", "YT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TAHITIAN,               "ty", "PF", LANGUAGE_ZERO     },
    { LANGUAGE_MALAGASY_PLATEAU,           "plt", "MG", LANGUAGE_ZERO     },
    { LANGUAGE_MALAGASY_PLATEAU,            "mg", "MG", kSAME },
    { LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU, "plt", "MG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_BAFIA,                 "ksf", "CM", LANGUAGE_ZERO     },
    { LANGUAGE_USER_GIKUYU,                 "ki", "KE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_RUSYN_UKRAINE,         "rue", "UA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_RUSYN_SLOVAKIA,        "rue", "SK", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LIMBU,                 "lif", "NP", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LOJBAN,                "jbo", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_HAITIAN,                "ht", "HT", LANGUAGE_ZERO     },
    { LANGUAGE_FRENCH_HAITI,                "fr", "HT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_BEEMBE,                "beq", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_BEKWEL,                "bkw", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KITUBA,                "mkw", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LARI,                  "ldi", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MBOCHI,                "mdw", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TEKE_EBOO,             "ebo", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TEKE_IBALI,            "tek", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TEKE_TYEE,             "tyx", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_VILI,                  "vif", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PORTUGUESE_ANGOLA,      "pt", "AO", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MANX,                   "gv", "GB", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARAGONESE,              "an", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KEYID,                 "qtz", ""  , LANGUAGE_ZERO     },    // key id pseudolanguage used for UI testing
    { LANGUAGE_USER_PALI_LATIN,            "pli", ""  , kSAME },    // Pali with Latin script, ISO 639-3 (sigh..) back-compat, Latin is not a default script though..
    { LANGUAGE_USER_KYRGYZ_CHINA,           "ky", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KOMI_ZYRIAN,           "kpv", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KOMI_PERMYAK,          "koi", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PITJANTJATJARA,        "pjt", "AU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ERZYA,                 "myv", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MARI_MEADOW,           "mhr", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KHANTY,                "kca", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LIVONIAN,              "liv", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MOKSHA,                "mdf", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MARI_HILL,             "mrj", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NGANASAN,              "nio", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_OLONETS,               "olo", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_VEPS,                  "vep", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_VORO,                  "vro", "EE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NENETS,                "yrk", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_AKA,                   "axk", "CF", LANGUAGE_ZERO     },
    { LANGUAGE_USER_AKA_CONGO,             "axk", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_DIBOLE,                "bvx", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_DOONDO,                "dde", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KAAMBA,                "xku", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KOONGO,                "kng", "CD", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KOONGO_CONGO,          "kng", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KUNYI,                 "njx", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NGUNGWEL,              "ngz", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NJYEM,                 "njy", "CM", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NJYEM_CONGO,           "njy", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PUNU,                  "puu", "GA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PUNU_CONGO,            "puu", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SUUNDI,                "sdj", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TEKE_KUKUYA,           "kkw", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_TSAANGI,               "tsa", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_YAKA,                  "iyx", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_YOMBE,                 "yom", "CD", LANGUAGE_ZERO     },
    { LANGUAGE_USER_YOMBE_CONGO,           "yom", "CG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SIDAMA,                "sid", "ET", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NKO,                   "nqo", "GN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_UDMURT,                "udm", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_CORNISH,               "kw",  "UK", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SAMI_PITE_SWEDEN,      "sje", "SE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NGAEBERE,              "gym", "PA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KUMYK,                 "kum", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_NOGAI,                 "nog", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LADIN,                 "lld", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FRENCH_BURKINA_FASO,    "fr", "BF", LANGUAGE_ZERO     },
    { LANGUAGE_USER_PUINAVE,               "pui", "CO", LANGUAGE_ZERO     },
    { LANGUAGE_USER_AVAR,                   "av", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_LENGO,                 "lgr", "SB", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FRENCH_BENIN,           "fr", "BJ", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FRENCH_NIGER,           "fr", "NE", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FRENCH_TOGO,            "fr", "TG", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KVEN_FINNISH,          "fkv", "NO", LANGUAGE_ZERO     },
    { LANGUAGE_USER_CHURCH_SLAVIC,          "cu", "RU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_VENETIAN,              "vec", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ENGLISH_GAMBIA,         "en", "GM", LANGUAGE_ZERO     },
    { LANGUAGE_USER_OCCITAN_ARANESE,        "oc", "ES", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARPITAN_FRANCE,        "frp", "FR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARPITAN_ITALY,         "frp", "IT", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ARPITAN_SWITZERLAND,   "frp", "CH", LANGUAGE_ZERO     },
    { LANGUAGE_USER_APATANI,               "apt", "IN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_ENGLISH_MAURITIUS,      "en", "MU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_FRENCH_MAURITIUS,       "fr", "MU", LANGUAGE_ZERO     },
    { LANGUAGE_USER_SILESIAN,              "szl", "PL", LANGUAGE_ZERO     },
    { LANGUAGE_USER_MANCHU,                "mnc", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_XIBE,                  "sjo", "CN", LANGUAGE_ZERO     },
    { LANGUAGE_MULTIPLE,                   "mul", ""  , LANGUAGE_ZERO     },    // multiple languages, many languages are used
    { LANGUAGE_UNDETERMINED,               "und", ""  , LANGUAGE_ZERO     },    // undetermined language, language cannot be identified
    { LANGUAGE_NONE,                       "zxx", ""  , LANGUAGE_ZERO     },    // added to ISO 639-2 on 2006-01-11: Used to declare the absence of linguistic information
    { LANGUAGE_DONTKNOW,                    "",   ""  , LANGUAGE_ZERO     }     // marks end of table
};

static IsoLanguageScriptCountryEntry const aImplIsoLangScriptEntries[] =
{
    // MS-LangID,                             ISO639-ISO15924, ISO3166, override
    { LANGUAGE_SERBIAN_LATIN_SERBIA,                "sr-Latn", "RS", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA,  "sr-Latn", "RS", LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_LATIN_MONTENEGRO,            "sr-Latn", "ME", LANGUAGE_ZERO     },
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO,"sr-Latn", "ME", LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,    "sr-Latn", "BA", LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_LATIN_SAM,                   "sr-Latn", "CS", LANGUAGE_ZERO     },   // Serbian Latin in Serbia and Montenegro; note that not all applications may know about the 'CS' reusage mess, see https://en.wikipedia.org/wiki/ISO_3166-2:CS
    { LANGUAGE_SERBIAN_LATIN_SAM,                   "sr-Latn", "YU", LANGUAGE_ZERO     },   // legacy Serbian Latin in Yugoslavia
    { LANGUAGE_SERBIAN_LATIN_LSO,                   "sr-Latn", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_SERBIAN_LATIN_NEUTRAL,               "sr-Latn", ""  , LANGUAGE_SERBIAN_LATIN_LSO },  // MS lists this as 'sr' only, what a mess
    { LANGUAGE_SERBIAN_CYRILLIC_SERBIA,             "sr-Cyrl", "RS", kSAME },   // MS
    { LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO,         "sr-Cyrl", "ME", kSAME },   // MS
    { LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "sr-Cyrl", "BA", kSAME },   // MS
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,                "sr-Cyrl", "CS", kSAME },   // MS
    { LANGUAGE_SERBIAN_CYRILLIC_LSO,                "sr-Cyrl", ""  , kSAME },   // MS
    { LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "bs-Cyrl", "BA", LANGUAGE_ZERO     },
    { LANGUAGE_BOSNIAN_CYRILLIC_LSO,                "bs-Cyrl", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_AZERI_CYRILLIC,                      "az-Cyrl", "AZ", LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_AZERI_CYRILLIC_LSO,                  "az-Cyrl", ""  , LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_UZBEK_CYRILLIC,                      "uz-Cyrl", "UZ", LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_UZBEK_CYRILLIC_LSO,                  "uz-Cyrl", ""  , LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA,         "mn-Cyrl", "MN", LANGUAGE_ZERO     },   // macrolanguage code; should be khk-MN or khk-Cyrl-MN
    { LANGUAGE_MONGOLIAN_CYRILLIC_LSO,              "mn-Cyrl", ""  , LANGUAGE_ZERO     },   // macrolanguage code; MS, should be khk or khk-Cyrl
    { LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA,        "mn-Mong", "MN", LANGUAGE_ZERO     },   // macrolanguage code; MS, should be khk-Mong-MN
    { LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA,           "mn-Mong", "CN", LANGUAGE_ZERO     },   // macrolanguage code; MS, should actually be mvf-CN
    { LANGUAGE_MONGOLIAN_MONGOLIAN_LSO,             "mn-Mong", ""  , LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_USER_PALI_LATIN,                     "pi-Latn", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_KARAKALPAK_LATIN,              "kaa-Latn", "UZ", LANGUAGE_ZERO     },
    { LANGUAGE_TAJIK,                               "tg-Cyrl", "TJ", LANGUAGE_ZERO     },   // MS
    { LANGUAGE_TAJIK_LSO,                           "tg-Cyrl", ""  , LANGUAGE_ZERO     },   // MS
    { LANGUAGE_AZERI_LATIN,                         "az-Latn", "AZ", LANGUAGE_ZERO     },   // macrolanguage code; MS
    { LANGUAGE_AZERI_LATIN_LSO,                     "az-Latn", ""  , LANGUAGE_ZERO     },   // macrolanguage code; MS
    { LANGUAGE_USER_YIDDISH_US,                     "yi-Hebr", "US", kSAME },   // macrolanguage code; MS, Hebr is suppress-script
    { LANGUAGE_YIDDISH,                             "yi-Hebr", "IL", kSAME },   // macrolanguage code; MS, Hebr is suppress-script
    { LANGUAGE_UZBEK_LATIN,                         "uz-Latn", "UZ", LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_UZBEK_LATIN_LSO,                     "uz-Latn", ""  , LANGUAGE_ZERO     },
//  { LANGUAGE_SINDHI,                              "sd-Deva", "IN", LANGUAGE_ZERO     },   // MS, TODO: see comment above in aImplIsoLangEntries
    { LANGUAGE_SINDHI_PAKISTAN,                     "sd-Arab", "PK", LANGUAGE_ZERO     },   // MS
    { LANGUAGE_SINDHI_ARABIC_LSO,                   "sd-Arab", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_CHEROKEE_UNITED_STATES,             "chr-Cher", "US", LANGUAGE_ZERO     },   // MS
    { LANGUAGE_CHEROKEE_CHEROKEE_LSO,              "chr-Cher", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_INUKTITUT_SYLLABICS_CANADA,          "iu-Cans", "CA", LANGUAGE_ZERO     },   // macrolanguage code, MS
    { LANGUAGE_INUKTITUT_SYLLABICS_LSO,             "iu-Cans", ""  , LANGUAGE_ZERO     },   // macrolanguage code, MS
    { LANGUAGE_INUKTITUT_LATIN_CANADA,              "iu-Latn", "CA", LANGUAGE_ZERO     },   // macrolanguage code, MS
    { LANGUAGE_INUKTITUT_LATIN_LSO,                 "iu-Latn", ""  , LANGUAGE_ZERO     },   // macrolanguage code, MS
    { LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO,         "tzm-Tfng", "MA", LANGUAGE_ZERO     },
    { LANGUAGE_TAMAZIGHT_TIFINAGH_LSO,             "tzm-Tfng", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_KASHMIRI_INDIA,                      "ks-Deva", "IN", LANGUAGE_ZERO     },   // MS
    { LANGUAGE_KASHMIRI,                            "ks-Arab", ""  , LANGUAGE_ZERO     },   // MS, Kashmiri in "Jammu and Kashmir" ... no ISO3166 code for that
    { LANGUAGE_HAUSA_NIGERIA,                       "ha-Latn", "NG", LANGUAGE_ZERO     },   // MS
    { LANGUAGE_USER_HAUSA_GHANA,                    "ha-Latn", "GH", LANGUAGE_ZERO     },
    { LANGUAGE_HAUSA_LATIN_LSO,                     "ha-Latn", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_LATIN_LSO,                           "la-Latn", ""  , kSAME },   // MS, though Latn is suppress-script
    { LANGUAGE_TAI_NUA_CHINA,                      "tdd-Tale", "CN", LANGUAGE_ZERO     },   // MS reserved
    { LANGUAGE_LU_CHINA,                           "khb-Talu", "CN", LANGUAGE_ZERO     },   // MS reserved
    { LANGUAGE_KURDISH_ARABIC_IRAQ,                 "ku-Arab", "IQ", kSAME },   // macrolanguage code, MS
    { LANGUAGE_KURDISH_ARABIC_LSO,                  "ku-Arab", ""  , kSAME },   // macrolanguage code, MS
    { LANGUAGE_USER_KURDISH_TURKEY,                "kmr-Latn", "TR", LANGUAGE_ZERO     },
    { LANGUAGE_USER_KURDISH_SYRIA,                 "kmr-Latn", "SY", LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI_PAKISTAN,                   "pnb-Arab", "PK", LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI_ARABIC_LSO,                 "pnb-Arab", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_PUNJABI_PAKISTAN,                    "pa-Arab", "PK", LANGUAGE_ZERO     },   // MS, incorrect
    { LANGUAGE_PUNJABI_ARABIC_LSO,                  "pa-Arab", ""  , LANGUAGE_ZERO     },   // MS, incorrect
    { LANGUAGE_TAMAZIGHT_LATIN_ALGERIA,            "tzm-Latn", "DZ", kSAME },   // MS
    { LANGUAGE_TAMAZIGHT_LATIN_LSO,                "tzm-Latn", ""  , LANGUAGE_ZERO     },   // MS
    { LANGUAGE_FULFULDE_SENEGAL,                    "ff-Latn", "SN", LANGUAGE_ZERO     },   // macrolanguage code, MS
    { LANGUAGE_FULFULDE_LATIN_LSO,                  "ff-Latn", ""  , LANGUAGE_ZERO     },   // macrolanguage code
    { LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA,    "bs-Latn", "BA", kSAME },   // MS, though Latn is suppress-script
    { LANGUAGE_BOSNIAN_LATIN_LSO,                   "bs-Latn", ""  , LANGUAGE_BOSNIAN_LSO },   // MS, though Latn is suppress-script
    { LANGUAGE_CHINESE_TRADITIONAL_LSO,             "zh-Hant", ""  , LANGUAGE_ZERO     },
    { LANGUAGE_USER_MANINKAKAN_EASTERN_LATIN,      "emk-Latn", "GN", LANGUAGE_ZERO     },
    { LANGUAGE_USER_CREE_PLAINS_LATIN,             "crk-Latn", "CA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_CREE_PLAINS_SYLLABICS,         "crk-Cans", "CA", LANGUAGE_ZERO     },
    { LANGUAGE_USER_CREE_PLAINS_LATIN,             "crk-Latn", "CN", kSAME },   // erroneous tdf#73973
    { LANGUAGE_USER_CREE_PLAINS_SYLLABICS,         "crk-Cans", "CN", kSAME },   // erroneous tdf#73973
    { LANGUAGE_USER_HUNGARIAN_ROVAS,                "hu-Hung", "HU", LANGUAGE_ZERO     },
    { LANGUAGE_DONTKNOW,                            "",        ""  , LANGUAGE_ZERO     }    // marks end of table
};

static Bcp47CountryEntry const aImplBcp47CountryEntries[] =
{
    // MS-LangID                              full BCP47, ISO3166, ISO639-Variant or other fallback
    { LANGUAGE_CATALAN_VALENCIAN,        "ca-ES-valencia", "ES", "ca-valencia", LANGUAGE_ZERO },
    { LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN, "ca-ES-valencia", "ES", "", LANGUAGE_ZERO },   // In case MS format files using the old value escaped into the wild, map them back.
    { LANGUAGE_USER_ENGLISH_UK_OXENDICT, "en-GB-oxendict", "GB", "", LANGUAGE_ZERO },
    { LANGUAGE_USER_ENGLISH_UK_OED,           "en-GB-oed", "GB", "", LANGUAGE_USER_ENGLISH_UK_OXENDICT },   // grandfathered, deprecated, prefer en-GB-oxendict
//  { LANGUAGE_YUE_CHINESE_HONGKONG,         "zh-yue-HK", "HK", "", 0 },   // MS reserved, prefer yue-HK; do not add unless LanguageTag::simpleExtract() can handle it to not call liblangtag for rsc!
    { LANGUAGE_DONTKNOW,                    "", "", "", LANGUAGE_ZERO }    // marks end of table
};

static IsoLanguageCountryEntry aLastResortFallbackEntry =
{ LANGUAGE_ENGLISH_US, "en", "US", LANGUAGE_ZERO };

OUString IsoLanguageCountryEntry::getTagString() const
{
    if (maCountry[0])
        return OUString( OUString::createFromAscii( maLanguage) + "-" + OUString::createFromAscii( maCountry));
    else
        return OUString::createFromAscii( maLanguage);
}

css::lang::Locale IsoLanguageCountryEntry::getLocale() const
{
    return lang::Locale( OUString::createFromAscii( maLanguage), OUString::createFromAscii( maCountry), OUString());
}

OUString IsoLanguageScriptCountryEntry::getTagString() const
{
    if (maCountry[0])
        return OUString( OUString::createFromAscii( maLanguageScript) + "-" + OUString::createFromAscii( maCountry));
    else
        return OUString::createFromAscii( maLanguageScript);
}

css::lang::Locale IsoLanguageScriptCountryEntry::getLocale() const
{
    return lang::Locale( I18NLANGTAG_QLT, OUString::createFromAscii( maCountry), getTagString());
}

bool IsoLanguageScriptCountryEntry::startsInIgnoreAsciiCase( const OUString & rStr ) const
{
    return rStr.matchIgnoreAsciiCaseAsciiL( maLanguageScript, strlen( maLanguageScript) );
}

OUString Bcp47CountryEntry::getTagString() const
{
    return OUString::createFromAscii( mpBcp47);
}

css::lang::Locale Bcp47CountryEntry::getLocale() const
{
    return lang::Locale( I18NLANGTAG_QLT, OUString::createFromAscii( maCountry), getTagString());
}


// In this table are the countries which should mapped to a specific
// english language
static IsoLangEngEntry const aImplIsoLangEngEntries[] =
{
    { LANGUAGE_ENGLISH_UK,                  "AO" },         // Angola
    { LANGUAGE_ENGLISH_UK,                  "BJ" },         // Benin
    { LANGUAGE_ENGLISH_UK,                  "BW" },         // Botswana
    { LANGUAGE_ENGLISH_UK,                  "BI" },         // Burundi
    { LANGUAGE_ENGLISH_UK,                  "CM" },         // Cameroon
    { LANGUAGE_ENGLISH_UK,                  "GA" },         // Gabon
    { LANGUAGE_ENGLISH_UK,                  "GM" },         // Gambia
    { LANGUAGE_ENGLISH_UK,                  "GH" },         // Ghana
    { LANGUAGE_ENGLISH_UK,                  "GN" },         // Guinea
    { LANGUAGE_ENGLISH_UK,                  "LS" },         // Lesotho
    { LANGUAGE_ENGLISH_UK,                  "MW" },         // Malawi
    { LANGUAGE_ENGLISH_UK,                  "MT" },         // Malta
    { LANGUAGE_ENGLISH_UK,                  "NA" },         // Namibia
    { LANGUAGE_ENGLISH_UK,                  "NG" },         // Nigeria
    { LANGUAGE_ENGLISH_UK,                  "UG" },         // Uganda
    { LANGUAGE_ENGLISH_UK,                  "ZM" },         // Zambia
    { LANGUAGE_ENGLISH_UK,                  "ZW" },         // Zimbabwe
    { LANGUAGE_ENGLISH_UK,                  "SZ" },         // Swaziland
    { LANGUAGE_ENGLISH_UK,                  "NG" },         // Sierra Leone
    { LANGUAGE_ENGLISH_UK,                  "KN" },         // Saint Kitts and Nevis
    { LANGUAGE_ENGLISH_UK,                  "SH" },         // St. Helena
    { LANGUAGE_ENGLISH_UK,                  "IO" },         // British Indian Oceanic Territory
    { LANGUAGE_ENGLISH_UK,                  "FK" },         // Falkland Islands
    { LANGUAGE_ENGLISH_UK,                  "GI" },         // Gibraltar
    { LANGUAGE_ENGLISH_UK,                  "KI" },         // Kiribati
    { LANGUAGE_ENGLISH_UK,                  "VG" },         // Virgin Islands
    { LANGUAGE_ENGLISH_UK,                  "MU" },         // Mauritius
    { LANGUAGE_ENGLISH_UK,                  "FJ" },         // Fiji
    { LANGUAGE_ENGLISH_US,                  "KI" },         // Kiribati
    { LANGUAGE_ENGLISH_US,                  "LR" },         // Liberia
    { LANGUAGE_ENGLISH_US,                  "GU" },         // Guam
    { LANGUAGE_ENGLISH_US,                  "MH" },         // Marshall Islands
    { LANGUAGE_ENGLISH_US,                  "PW" },         // Palau
    { LANGUAGE_ENGLISH_CARRIBEAN,           "AI" },         // Anguilla
    { LANGUAGE_ENGLISH_CARRIBEAN,           "AG" },         // Antigua and Barbuda
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BS" },         // Bahamas
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BB" },         // Barbados
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BM" },         // Bermuda
    { LANGUAGE_ENGLISH_CARRIBEAN,           "KY" },         // Cayman Islands
    { LANGUAGE_ENGLISH_CARRIBEAN,           "GD" },         // Grenada
    { LANGUAGE_ENGLISH_CARRIBEAN,           "DM" },         // Dominica
    { LANGUAGE_ENGLISH_CARRIBEAN,           "HT" },         // Haiti
    { LANGUAGE_ENGLISH_CARRIBEAN,           "MS" },         // Montserrat
    { LANGUAGE_ENGLISH_CARRIBEAN,           "FM" },         // Micronesia
    { LANGUAGE_ENGLISH_CARRIBEAN,           "VC" },         // St. Vincent / Grenadines
    { LANGUAGE_ENGLISH_CARRIBEAN,           "LC" },         // Saint Lucia
    { LANGUAGE_ENGLISH_CARRIBEAN,           "TC" },         // Turks & Caicos Islands
    { LANGUAGE_ENGLISH_CARRIBEAN,           "GY" },         // Guyana
    { LANGUAGE_ENGLISH_CARRIBEAN,           "TT" },         // Trinidad and Tobago
    { LANGUAGE_ENGLISH_AUS,                 "CX" },         // Christmas Islands
    { LANGUAGE_ENGLISH_AUS,                 "CC" },         // Cocos (Keeling) Islands
    { LANGUAGE_ENGLISH_AUS,                 "NF" },         // Norfolk Island
    { LANGUAGE_ENGLISH_AUS,                 "PG" },         // Papua New Guinea
    { LANGUAGE_ENGLISH_AUS,                 "SB" },         // Solomon Islands
    { LANGUAGE_ENGLISH_AUS,                 "TV" },         // Tuvalu
    { LANGUAGE_ENGLISH_AUS,                 "NR" },         // Nauru
    { LANGUAGE_ENGLISH_NZ,                  "CK" },         // Cook Islands
    { LANGUAGE_ENGLISH_NZ,                  "NU" },         // Niue
    { LANGUAGE_ENGLISH_NZ,                  "TK" },         // Tokelau
    { LANGUAGE_ENGLISH_NZ,                  "TO" },         // Tonga
    { LANGUAGE_DONTKNOW,                    ""   }          // marks end of table
};


static IsoLangNoneStdEntry const aImplIsoNoneStdLangEntries[] =
{
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "BOK"      }, // registered subtags for "no" in rfc1766
    { LANGUAGE_NORWEGIAN_NYNORSK,           "no", "NYN"      }, // registered subtags for "no" in rfc1766
    { LANGUAGE_SERBIAN_LATIN_SAM,           "sr", "latin"    },
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,        "sr", "cyrillic" },
    { LANGUAGE_AZERI_LATIN,                 "az", "latin"    },
    { LANGUAGE_AZERI_CYRILLIC,              "az", "cyrillic" },
    { LANGUAGE_DONTKNOW,                    "",   ""         }  // marks end of table
};


// in this table are only names to find the best language
static IsoLangNoneStdEntry const aImplIsoNoneStdLangEntries2[] =
{
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "bokmaal"  },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "bokmal"   },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "no", "nynorsk"  },
    { LANGUAGE_DONTKNOW,                    "",   ""         }  // marks end of table
};


// in this table are only names to find the best language
static IsoLangOtherEntry const aImplOtherEntries[] =
{
    { LANGUAGE_ENGLISH_US,                  "c"              },
    { LANGUAGE_CHINESE,                     "chinese"        },
    { LANGUAGE_GERMAN,                      "german"         },
    { LANGUAGE_JAPANESE,                    "japanese"       },
    { LANGUAGE_KOREAN,                      "korean"         },
    { LANGUAGE_ENGLISH_US,                  "posix"          },
    { LANGUAGE_CHINESE_TRADITIONAL,         "tchinese"       },
    { LANGUAGE_DONTKNOW,                    nullptr             }  // marks end of table
};


// in this table are only privateuse names
static IsoLangOtherEntry const aImplPrivateUseEntries[] =
{
    { LANGUAGE_USER_PRIV_NOTRANSLATE,       "x-no-translate" }, //! not BCP47 but legacy in .xcu configmgr
    { LANGUAGE_USER_PRIV_DEFAULT,           "x-default"      },
    { LANGUAGE_USER_PRIV_COMMENT,           "x-comment"      },
    { LANGUAGE_USER_PRIV_JOKER,             "*"              }, //! not BCP47 but transferable in configmgr
    { LANGUAGE_DONTKNOW,                    nullptr             }  // marks end of table
};


// static
void MsLangId::Conversion::convertLanguageToLocaleImpl( LanguageType nLang,
        css::lang::Locale & rLocale, bool bIgnoreOverride )
{
    const Bcp47CountryEntry* pBcp47EntryOverride = nullptr;
    const IsoLanguageScriptCountryEntry* pScriptEntryOverride = nullptr;
    const IsoLanguageCountryEntry* pEntryOverride = nullptr;

Label_Override_Lang_Locale:

    // Search for LangID in BCP47
    for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
            pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
    {
        if (pBcp47Entry->mnLang == nLang)
        {
            if (bIgnoreOverride || !pBcp47Entry->mnOverride)
            {
                rLocale.Language = I18NLANGTAG_QLT;
                rLocale.Country  = OUString::createFromAscii( pBcp47Entry->maCountry);
                rLocale.Variant  = pBcp47Entry->getTagString();
                return;
            }
            else if (pBcp47Entry->mnOverride && pBcp47EntryOverride != pBcp47Entry)
            {
                pBcp47EntryOverride = pBcp47Entry;
                nLang = getOverrideLang( pBcp47Entry->mnLang, pBcp47Entry->mnOverride);
                goto Label_Override_Lang_Locale;
            }
        }
    }

    // Search for LangID in ISO lll-Ssss-CC
    for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
            pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
    {
        if (pScriptEntry->mnLang == nLang)
        {
            if (bIgnoreOverride || !pScriptEntry->mnOverride)
            {
                rLocale.Language = I18NLANGTAG_QLT;
                rLocale.Country  = OUString::createFromAscii( pScriptEntry->maCountry);
                rLocale.Variant  = pScriptEntry->getTagString();
                return;
            }
            else if (pScriptEntry->mnOverride && pScriptEntryOverride != pScriptEntry)
            {
                pScriptEntryOverride = pScriptEntry;
                nLang = getOverrideLang( pScriptEntry->mnLang, pScriptEntry->mnOverride);
                goto Label_Override_Lang_Locale;
            }
        }
    }

    // Search for LangID in ISO lll-CC
    for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
            pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if (pEntry->mnLang == nLang)
        {
            if (bIgnoreOverride || !pEntry->mnOverride)
            {
                rLocale.Language = OUString::createFromAscii( pEntry->maLanguage );
                rLocale.Country  = OUString::createFromAscii( pEntry->maCountry );
                rLocale.Variant.clear();
                return;
            }
            else if (pEntry->mnOverride && pEntryOverride != pEntry)
            {
                pEntryOverride = pEntry;
                nLang = getOverrideLang( pEntry->mnLang, pEntry->mnOverride);
                goto Label_Override_Lang_Locale;
            }
        }
    }

    // Look for privateuse definitions.
    for (const IsoLangOtherEntry* pPrivateEntry = aImplPrivateUseEntries;
            pPrivateEntry->mnLang != LANGUAGE_DONTKNOW; ++pPrivateEntry)
    {
        if (pPrivateEntry->mnLang == nLang)
        {
            rLocale.Language = I18NLANGTAG_QLT;
            rLocale.Country.clear();
            rLocale.Variant  = OUString::createFromAscii( pPrivateEntry->mpLanguage );
            return;
        }
    }

    // Not found. Passed rLocale argument remains unchanged.
}


// static
css::lang::Locale MsLangId::Conversion::getLocale( const IsoLanguageCountryEntry * pEntry )
{
    if (pEntry->mnOverride)
    {
        lang::Locale aLocale;
        convertLanguageToLocaleImpl( getOverrideLang( pEntry->mnLang, pEntry->mnOverride), aLocale, false);
        return aLocale;
    }
    else
        return pEntry->getLocale();
}

// static
css::lang::Locale MsLangId::Conversion::getLocale( const IsoLanguageScriptCountryEntry * pEntry )
{
    if (pEntry->mnOverride)
    {
        lang::Locale aLocale;
        convertLanguageToLocaleImpl( getOverrideLang( pEntry->mnLang, pEntry->mnOverride), aLocale, false);
        return aLocale;
    }
    else
        return pEntry->getLocale();
}

// static
css::lang::Locale MsLangId::Conversion::getLocale( const Bcp47CountryEntry * pEntry )
{
    if (pEntry->mnOverride)
    {
        lang::Locale aLocale;
        convertLanguageToLocaleImpl( getOverrideLang( pEntry->mnLang, pEntry->mnOverride), aLocale, false);
        return aLocale;
    }
    else
        return pEntry->getLocale();
}

// static
css::lang::Locale MsLangId::Conversion::lookupFallbackLocale(
        const css::lang::Locale & rLocale )
{
    // language is lower case in table
    OUString aLowerLang = rLocale.Language.toAsciiLowerCase();
    // country is upper case in table
    OUString aUpperCountry = rLocale.Country.toAsciiUpperCase();
    sal_Int32 nCountryLen = aUpperCountry.getLength();

    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        // Search in BCP47, only full match and one fallback, for other
        // fallbacks only LanguageTag can decide.
        for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
                pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
        {
            if (    rLocale.Variant.equalsIgnoreAsciiCase( pBcp47Entry->getTagString()) ||
                    rLocale.Variant.equalsIgnoreAsciiCaseAscii( pBcp47Entry->mpFallback))
                return getLocale( pBcp47Entry);     // may override
        }

        // Search in ISO lll-Ssss-CC
        const IsoLanguageScriptCountryEntry* pFirstScript = nullptr;
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return getLocale( pScriptEntry);    // may override
                if (!pFirstScript)
                    pFirstScript = pScriptEntry;
            }
        }
        // If at least a lll-Ssss matched, try that with country or use it as
        // fallback.
        if (pFirstScript)
        {
            // Check for country only if there is more than lll-Ssss-CC in tag
            // string, else we would had matched it already.
            if (!aUpperCountry.isEmpty() && rLocale.Variant.getLength() > 11)
            {
                for (const IsoLanguageScriptCountryEntry* pScriptEntry = pFirstScript;
                        pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
                {
                    if (aUpperCountry.equalsAscii( pScriptEntry->maCountry) &&
                            pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
                        return getLocale( pScriptEntry);    // may override
                }
            }
            return getLocale( pFirstScript);    // may override
        }

        // Extract language from tag string, country is used as present in
        // Locale because in the tables that follow we have only ISO 3166
        // countries and if that is in the tag string we also have it in the
        // Locale.
        sal_Int32 nIndex = 0;
        aLowerLang = rLocale.Variant.getToken( 0, '-', nIndex).toAsciiLowerCase();
        // Nothing with "x-..." or "i-..." or any 1 letter in lll-CC table that
        // follows.
        if (aLowerLang.getLength() == 1)
            return aLastResortFallbackEntry.getLocale();
    }

    // Search for locale and remember first lang-only.
    const IsoLanguageCountryEntry* pFirstLang = nullptr;
    const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
    for ( ; pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if (aLowerLang.equalsAscii( pEntry->maLanguage))
        {
            if (*pEntry->maCountry)
            {
                if (nCountryLen && aUpperCountry.equalsAscii( pEntry->maCountry))
                    return getLocale( pEntry);  // may override
            }
            else
            {
                if (
                    // These are known to have no country assigned.
                    pEntry->mnLang == LANGUAGE_USER_ESPERANTO
                    || pEntry->mnLang == LANGUAGE_USER_INTERLINGUA
                    || pEntry->mnLang ==  LANGUAGE_USER_INTERLINGUE
                    || pEntry->mnLang ==  LANGUAGE_USER_LOJBAN
                    || pEntry->mnLang ==  LANGUAGE_KASHMIRI
                    || pEntry->mnLang ==  LANGUAGE_USER_KEYID
                    // And the special codes without country.
                    || pEntry->mnLang ==  LANGUAGE_MULTIPLE
                    || pEntry->mnLang ==  LANGUAGE_UNDETERMINED
                    || pEntry->mnLang ==  LANGUAGE_NONE)
                {
                    return getLocale( pEntry);  // may override
                }
            }
            if (!pFirstLang)
                pFirstLang = pEntry;
        }
    }

    // Language not found at all => use default.
    if (!pFirstLang)
        return aLastResortFallbackEntry.getLocale();

    // Search for first entry of language with any country.
    pEntry = pFirstLang;
    for ( ; pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if (aLowerLang.equalsAscii( pEntry->maLanguage))
        {
            if (*pEntry->maCountry)
                return getLocale( pEntry);  // may override
        }
    }

    return aLastResortFallbackEntry.getLocale();
}


// static
LanguageType MsLangId::Conversion::convertPrivateUseToLanguage( const OUString& rPriv )
{
    for (const IsoLangOtherEntry* pPrivateEntry = aImplPrivateUseEntries;
            pPrivateEntry->mnLang != LANGUAGE_DONTKNOW; ++pPrivateEntry)
    {
        if ( rPriv.equalsIgnoreAsciiCaseAscii( pPrivateEntry->mpLanguage ) )
            return pPrivateEntry->mnLang;
    }
    return LANGUAGE_DONTKNOW;
}


// static
LanguageType MsLangId::Conversion::convertLocaleToLanguageImpl(
        const css::lang::Locale& rLocale )
{
    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        // "x-..." private use and the nasty "*" joker
        if (rLocale.Variant.startsWithIgnoreAsciiCase( "x-") || (rLocale.Variant == "*"))
            return convertPrivateUseToLanguage( rLocale.Variant);

        // Search in BCP47
        for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
                pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
        {
            if (rLocale.Variant.equalsIgnoreAsciiCase( pBcp47Entry->getTagString()))
                return getOverrideLang( pBcp47Entry->mnLang, pBcp47Entry->mnOverride);
        }

        // Search in ISO lll-Ssss-CC
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return getOverrideLang( pScriptEntry->mnLang, pScriptEntry->mnOverride);
            }
        }
    }
    else
    {
        // language is lower case in table
        OUString aLowerLang = rLocale.Language.toAsciiLowerCase();
        // country is upper case in table
        OUString aUpperCountry = rLocale.Country.toAsciiUpperCase();

        // Search in ISO lll-CC
        for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
                pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
        {
            if (aLowerLang.equalsAscii( pEntry->maLanguage) && aUpperCountry.equalsAscii( pEntry->maCountry))
                return getOverrideLang( pEntry->mnLang, pEntry->mnOverride);
        }
    }
    return LANGUAGE_DONTKNOW;
}


// static
css::lang::Locale MsLangId::Conversion::getOverride( const css::lang::Locale& rLocale )
{
    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        // "x-..." private use and the nasty "*" joker
        if (rLocale.Variant.startsWithIgnoreAsciiCase( "x-") || (rLocale.Variant == "*"))
            return rLocale;     // no overrides

        // Search in BCP47
        for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
                pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
        {
            if (rLocale.Variant.equalsIgnoreAsciiCase( pBcp47Entry->getTagString()))
                return getLocale( pBcp47Entry);     // may override
        }

        // Search in ISO lll-Ssss-CC
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return getLocale( pScriptEntry);    // may override
            }
        }
    }
    else
    {
        // language is lower case in table
        OUString aLowerLang = rLocale.Language.toAsciiLowerCase();
        // country is upper case in table
        OUString aUpperCountry = rLocale.Country.toAsciiUpperCase();

        // Search in ISO lll-CC
        for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
                pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
        {
            if (aLowerLang.equalsAscii( pEntry->maLanguage) && aUpperCountry.equalsAscii( pEntry->maCountry))
                return getLocale( pEntry);  // may override
        }
    }
    return lang::Locale();
}


// static
LanguageType MsLangId::Conversion::convertIsoNamesToLanguage( const OUString& rLang,
        const OUString& rCountry, bool bSkipIsoTable )
{
    // language is lower case in table
    OUString aLowerLang = rLang.toAsciiLowerCase();
    // country is upper case in table
    OUString aUpperCountry = rCountry.toAsciiUpperCase();

    const IsoLanguageCountryEntry* pFirstLang = nullptr;

    if (!bSkipIsoTable)
    {
        //  first look for exact match
        for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
                pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
        {
            if ( aLowerLang.equalsAscii( pEntry->maLanguage ) )
            {
                if ( aUpperCountry.isEmpty() ||
                        aUpperCountry.equalsAscii( pEntry->maCountry ) )
                    return pEntry->mnLang;
                if ( !pFirstLang )
                    pFirstLang = pEntry;
                else if ( !*pEntry->maCountry )
                    pFirstLang = pEntry;
            }
        }

        // some eng countries should be mapped to a specific english language
        if ( aLowerLang == "en" )
        {
            for (const IsoLangEngEntry* pEngEntry = aImplIsoLangEngEntries;
                    pEngEntry->mnLang != LANGUAGE_DONTKNOW; ++pEngEntry)
            {
                if ( aUpperCountry.equalsAscii( pEngEntry->maCountry ) )
                    return pEngEntry->mnLang;
            }
        }
    }

    // test for specific languages which are not used standard ISO 3166 codes
    for (const IsoLangNoneStdEntry* pNoneStdEntry = aImplIsoNoneStdLangEntries;
            pNoneStdEntry->mnLang != LANGUAGE_DONTKNOW; ++pNoneStdEntry)
    {
        if ( aLowerLang.equalsAscii( pNoneStdEntry->maLanguage ) )
        {
            // The countries in this table are not all in upper case
            if ( aUpperCountry.equalsIgnoreAsciiCaseAscii( pNoneStdEntry->maCountry ) )
                return pNoneStdEntry->mnLang;
        }
    }
    for (const IsoLangNoneStdEntry* pNoneStdEntry2 = aImplIsoNoneStdLangEntries2;
            pNoneStdEntry2->mnLang != LANGUAGE_DONTKNOW; ++pNoneStdEntry2)
    {
        if ( aLowerLang.equalsAscii( pNoneStdEntry2->maLanguage ) )
        {
            // The countries in this table are not all in upper case
            if ( aUpperCountry.equalsIgnoreAsciiCaseAscii( pNoneStdEntry2->maCountry ) )
                return pNoneStdEntry2->mnLang;
        }
    }

    if (!bSkipIsoTable)
    {
        // If the language is correct, then we return the default language
        if ( pFirstLang )
            return pFirstLang->mnLang;

        //  if only the country is set, look for any entry matching the country
        //  (to allow reading country and language in separate steps, in any order)
        if ( !rCountry.isEmpty() && rLang.isEmpty() )
        {
            for (const IsoLanguageCountryEntry* pEntry2 = aImplIsoLangEntries;
                    pEntry2->mnLang != LANGUAGE_DONTKNOW; ++pEntry2)
            {
                if ( aUpperCountry.equalsAscii( pEntry2->maCountry ) )
                    return pEntry2->mnLang;
            }

            aLowerLang = aUpperCountry.toAsciiLowerCase();
        }
    }

    // Look for privateuse definitions.
    LanguageType nLang = convertPrivateUseToLanguage( aLowerLang);
    if (nLang != LANGUAGE_DONTKNOW)
        return nLang;

    // Now look for all other definitions, which are not standard
    for (const IsoLangOtherEntry* pOtherEntry = aImplOtherEntries;
            pOtherEntry->mnLang != LANGUAGE_DONTKNOW; ++pOtherEntry)
    {
        if ( aLowerLang.equalsAscii( pOtherEntry->mpLanguage ) )
            return pOtherEntry->mnLang;
    }

    return LANGUAGE_DONTKNOW;
}


// static
LanguageType MsLangId::Conversion::convertIsoNamesToLanguage( const OString& rLang,
        const OString& rCountry )
{
    OUString aLang = OStringToOUString( rLang, RTL_TEXTENCODING_ASCII_US);
    OUString aCountry = OStringToOUString( rCountry, RTL_TEXTENCODING_ASCII_US);
    return convertIsoNamesToLanguage( aLang, aCountry, false);
}


struct IsoLangGLIBCModifiersEntry
{
    LanguageType  mnLang;
    sal_Char      maLanguage[4];
    sal_Char      maCountry[3];
    sal_Char      maAtString[9];
};

static IsoLangGLIBCModifiersEntry const aImplIsoLangGLIBCModifiersEntries[] =
{
    // MS-LANGID codes               ISO639-1/2/3 ISO3166            glibc modifier
    { LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "bs", "BA", "cyrillic" },
    { LANGUAGE_USER_SERBIAN_LATIN_SERBIA,           "sr", "RS", "latin" },   // Serbian Latin in Serbia
    { LANGUAGE_SERBIAN_LATIN_SAM,                   "sr", "CS", "latin" },   // Serbian Latin in Serbia and Montenegro
    { LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO,       "sr", "ME", "latin" },   // Serbian Latin in Montenegro
    { LANGUAGE_SERBIAN_LATIN_LSO,                   "sr", "",   "latin" },
    { LANGUAGE_AZERI_CYRILLIC,                      "az", "AZ", "cyrillic" },
    { LANGUAGE_UZBEK_CYRILLIC,                      "uz", "UZ", "cyrillic" },
    { LANGUAGE_CATALAN_VALENCIAN,                   "ca", "ES", "valencia" },
    { LANGUAGE_DONTKNOW,                            "",   "",   ""   }       // marks end of table
};

// convert a unix locale string into LanguageType

// static
LanguageType MsLangId::convertUnxByteStringToLanguage(
        const OString& rString )
{
    OString  aLang;
    OString  aCountry;
    OString  aAtString;

    sal_Int32  nLangSepPos    = rString.indexOf( '_' );
    sal_Int32  nCountrySepPos = rString.indexOf( '.' );
    sal_Int32  nAtPos         = rString.indexOf( '@' );

    if (nCountrySepPos < 0)
        nCountrySepPos = nAtPos;
    if (nCountrySepPos < 0)
        nCountrySepPos = rString.getLength();

    if (nAtPos >= 0)
        aAtString = rString.copy( nAtPos+1 );

    if (((nLangSepPos >= 0) && (nLangSepPos > nCountrySepPos)) || (nLangSepPos < 0))
    {
        // eg. "el.sun_eu_greek", "tchinese", "es.ISO8859-15"
        aLang    = rString.copy( 0, nCountrySepPos );
    }
    else if ( nLangSepPos >= 0 )
    {
        // well formed iso names like "en_US.UTF-8", "sh_BA.ISO8859-2@bosnia"
        aLang    = rString.copy( 0, nLangSepPos );
        aCountry = rString.copy( nLangSepPos+1, nCountrySepPos - nLangSepPos - 1);
    }

    //  if there is a glibc modifier, first look for exact match in modifier table
    if (!aAtString.isEmpty())
    {
        // language is lower case in table
        OString aLowerLang = aLang.toAsciiLowerCase();
        // country is upper case in table
        OString aUpperCountry = aCountry.toAsciiUpperCase();
        for (const IsoLangGLIBCModifiersEntry* pGLIBCModifiersEntry = aImplIsoLangGLIBCModifiersEntries;
                pGLIBCModifiersEntry->mnLang != LANGUAGE_DONTKNOW; ++pGLIBCModifiersEntry)
        {                         // avoid embedded \0 warning
            if (aLowerLang.equals( static_cast< const char* >( pGLIBCModifiersEntry->maLanguage )) &&
                 aAtString.equals( static_cast< const char* >( pGLIBCModifiersEntry->maAtString )))
            {
                if (aUpperCountry.isEmpty() ||
                        aUpperCountry.equals( static_cast< const char* >( pGLIBCModifiersEntry->maCountry )))
                {
                    return pGLIBCModifiersEntry->mnLang;
                }
            }
        }
    }

    return Conversion::convertIsoNamesToLanguage( aLang, aCountry );
}


// static
::std::vector< MsLangId::LanguagetagMapping > MsLangId::getDefinedLanguagetags()
{
    ::std::vector< LanguagetagMapping > aVec;
    for (const Bcp47CountryEntry* pEntry = aImplBcp47CountryEntries;
            pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        aVec.push_back( LanguagetagMapping( pEntry->getTagString(), pEntry->mnLang));
    }
    for (const IsoLanguageScriptCountryEntry* pEntry = aImplIsoLangScriptEntries;
            pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        aVec.push_back( LanguagetagMapping( pEntry->getTagString(), pEntry->mnLang));
    }
    for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
            pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        aVec.push_back( LanguagetagMapping( pEntry->getTagString(), pEntry->mnLang));
    }
    return aVec;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
