/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include "i18nlangtag/mslangid.hxx"
#include "i18nlangtag/languagetag.hxx"

using namespace com::sun::star;



static const LanguageType kSAME = 0xffff;

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
    ::com::sun::star::lang::Locale getLocale() const;
};

struct IsoLanguageScriptCountryEntry
{
    LanguageType  mnLang;
    sal_Char      maLanguageScript[9];      
    sal_Char      maCountry[3];
    LanguageType  mnOverride;

    /** Obtain a language tag string with '-' separator. */
    OUString getTagString() const;

    /** Obtain a locale. */
    ::com::sun::star::lang::Locale getLocale() const;

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

    /** Obtain a language tag string with '-' separator. */
    OUString getTagString() const;

    /** Obtain a locale. */
    ::com::sun::star::lang::Locale getLocale() const;
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







//


//




























/* Currently (2013-08-29) only these primary LangID are still used literally in
 * code:
 * LANGUAGE_ENGLISH  LANGUAGE_ARABIC_PRIMARY_ONLY
 */

static IsoLanguageCountryEntry const aImplIsoLangEntries[] =
{
    
    { LANGUAGE_ENGLISH,                     "en", ""  , 0     },
    { LANGUAGE_ENGLISH_US,                  "en", "US", 0     },
    { LANGUAGE_ENGLISH_UK,                  "en", "GB", 0     },
    { LANGUAGE_ENGLISH_AUS,                 "en", "AU", 0     },
    { LANGUAGE_ENGLISH_CAN,                 "en", "CA", 0     },
    { LANGUAGE_FRENCH,                      "fr", "FR", 0     },
    { LANGUAGE_GERMAN,                      "de", "DE", 0     },
    { LANGUAGE_ITALIAN,                     "it", "IT", 0     },
    { LANGUAGE_DUTCH,                       "nl", "NL", 0     },
    { LANGUAGE_SPANISH_MODERN,              "es", "ES", 0     },
    { LANGUAGE_SPANISH_DATED,               "es", "ES", 0     },
    { LANGUAGE_PORTUGUESE,                  "pt", "PT", 0     },
    { LANGUAGE_PORTUGUESE_BRAZILIAN,        "pt", "BR", 0     },
    { LANGUAGE_DANISH,                      "da", "DK", 0     },
    { LANGUAGE_GREEK,                       "el", "GR", 0     },
    { LANGUAGE_CHINESE_SIMPLIFIED,          "zh", "CN", 0     },
    { LANGUAGE_CHINESE_SIMPLIFIED_LEGACY,   "zh", "CN", 0     },
    { LANGUAGE_CHINESE_TRADITIONAL,         "zh", "TW", 0     },
    { LANGUAGE_CHINESE_HONGKONG,            "zh", "HK", 0     },
    { LANGUAGE_CHINESE_SINGAPORE,           "zh", "SG", 0     },
    { LANGUAGE_CHINESE_MACAU,               "zh", "MO", 0     },
    { LANGUAGE_CHINESE_LSO,                 "zh", ""  , 0     },
    { LANGUAGE_YUE_CHINESE_HONGKONG,       "yue", "HK", 0     },
    { LANGUAGE_ENGLISH_HONG_KONG_SAR,       "en", "HK", 0     },
    { LANGUAGE_JAPANESE,                    "ja", "JP", 0     },
    { LANGUAGE_KOREAN,                      "ko", "KR", 0     },
    { LANGUAGE_KOREAN_JOHAB,                "ko", "KR", 0     },
    { LANGUAGE_USER_KOREAN_NORTH,           "ko", "KP", 0     },
    { LANGUAGE_SWEDISH,                     "sv", "SE", 0     },
    { LANGUAGE_SWEDISH_FINLAND,             "sv", "FI", 0     },
    { LANGUAGE_FINNISH,                     "fi", "FI", 0     },
    { LANGUAGE_RUSSIAN,                     "ru", "RU", 0     },
    { LANGUAGE_TATAR,                       "tt", "RU", 0     },
    { LANGUAGE_ENGLISH_NZ,                  "en", "NZ", 0     },
    { LANGUAGE_ENGLISH_EIRE,                "en", "IE", 0     },
    { LANGUAGE_DUTCH_BELGIAN,               "nl", "BE", 0     },
    { LANGUAGE_FRENCH_BELGIAN,              "fr", "BE", 0     },
    { LANGUAGE_FRENCH_CANADIAN,             "fr", "CA", 0     },
    { LANGUAGE_FRENCH_SWISS,                "fr", "CH", 0     },
    { LANGUAGE_GERMAN_SWISS,                "de", "CH", 0     },
    { LANGUAGE_GERMAN_AUSTRIAN,             "de", "AT", 0     },
    { LANGUAGE_ITALIAN_SWISS,               "it", "CH", 0     },
    { LANGUAGE_ALBANIAN,                    "sq", "AL", 0     },
    { LANGUAGE_ARABIC_SAUDI_ARABIA,         "ar", "SA", 0     },
    { LANGUAGE_ARABIC_EGYPT,                "ar", "EG", 0     },
    { LANGUAGE_ARABIC_UAE,                  "ar", "AE", 0     },
    { LANGUAGE_ARABIC_IRAQ,                 "ar", "IQ", 0     },
    { LANGUAGE_ARABIC_LIBYA,                "ar", "LY", 0     },
    { LANGUAGE_ARABIC_ALGERIA,              "ar", "DZ", 0     },
    { LANGUAGE_ARABIC_MOROCCO,              "ar", "MA", 0     },
    { LANGUAGE_ARABIC_TUNISIA,              "ar", "TN", 0     },
    { LANGUAGE_ARABIC_OMAN,                 "ar", "OM", 0     },
    { LANGUAGE_ARABIC_YEMEN,                "ar", "YE", 0     },
    { LANGUAGE_ARABIC_SYRIA,                "ar", "SY", 0     },
    { LANGUAGE_ARABIC_JORDAN,               "ar", "JO", 0     },
    { LANGUAGE_ARABIC_LEBANON,              "ar", "LB", 0     },
    { LANGUAGE_ARABIC_KUWAIT,               "ar", "KW", 0     },
    { LANGUAGE_ARABIC_BAHRAIN,              "ar", "BH", 0     },
    { LANGUAGE_ARABIC_QATAR,                "ar", "QA", 0     },
    { LANGUAGE_USER_ARABIC_CHAD,            "ar", "TD", 0     },
    { LANGUAGE_USER_ARABIC_COMOROS,         "ar", "KM", 0     },
    { LANGUAGE_USER_ARABIC_DJIBOUTI,        "ar", "DJ", 0     },
    { LANGUAGE_USER_ARABIC_ERITREA,         "ar", "ER", 0     },
    { LANGUAGE_USER_ARABIC_ISRAEL,          "ar", "IL", 0     },
    { LANGUAGE_USER_ARABIC_MAURITANIA,      "ar", "MR", 0     },
    { LANGUAGE_USER_ARABIC_PALESTINE,       "ar", "PS", 0     },
    { LANGUAGE_USER_ARABIC_SOMALIA,         "ar", "SO", 0     },
    { LANGUAGE_USER_ARABIC_SUDAN,           "ar", "SD", 0     },
    { LANGUAGE_ARABIC_PRIMARY_ONLY,         "ar", ""  , 0     },
    { LANGUAGE_BASQUE,                      "eu", ""  , 0     },
    { LANGUAGE_BULGARIAN,                   "bg", "BG", 0     },
    { LANGUAGE_CZECH,                       "cs", "CZ", 0     },
    { LANGUAGE_CZECH,                       "cz", ""  , kSAME },
    { LANGUAGE_ENGLISH_JAMAICA,             "en", "JM", 0     },
    { LANGUAGE_ENGLISH_CARRIBEAN,           "en", "BS", 0     },    
    { LANGUAGE_ENGLISH_BELIZE,              "en", "BZ", 0     },
    { LANGUAGE_ENGLISH_TRINIDAD,            "en", "TT", 0     },
    { LANGUAGE_ENGLISH_ZIMBABWE,            "en", "ZW", 0     },
    { LANGUAGE_ENGLISH_INDONESIA,           "en", "ID", 0     },
    { LANGUAGE_ESTONIAN,                    "et", "EE", 0     },
    { LANGUAGE_FAEROESE,                    "fo", "FO", 0     },
    { LANGUAGE_FARSI,                       "fa", "IR", 0     },
    { LANGUAGE_FRENCH_LUXEMBOURG,           "fr", "LU", 0     },
    { LANGUAGE_FRENCH_MONACO,               "fr", "MC", 0     },
    { LANGUAGE_GERMAN_LUXEMBOURG,           "de", "LU", 0     },
    { LANGUAGE_GERMAN_LIECHTENSTEIN,        "de", "LI", 0     },
    { LANGUAGE_HEBREW,                      "he", "IL", 0     },    
    { LANGUAGE_HEBREW,                      "iw", "IL", kSAME },    
    { LANGUAGE_HUNGARIAN,                   "hu", "HU", 0     },
    { LANGUAGE_ICELANDIC,                   "is", "IS", 0     },
    { LANGUAGE_INDONESIAN,                  "id", "ID", 0     },    
    { LANGUAGE_INDONESIAN,                  "in", "ID", kSAME },    
    { LANGUAGE_NORWEGIAN,                   "no", "NO", 0     },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "nb", "NO", 0     },
    { LANGUAGE_NORWEGIAN_BOKMAL_LSO,        "nb", ""  , 0     },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "nn", "NO", 0     },
    { LANGUAGE_NORWEGIAN_NYNORSK_LSO,       "nn", ""  , 0     },
    { LANGUAGE_POLISH,                      "pl", "PL", 0     },
    { LANGUAGE_RHAETO_ROMAN,                "rm", "CH", 0     },
    { LANGUAGE_ROMANIAN,                    "ro", "RO", 0     },
    { LANGUAGE_ROMANIAN_MOLDOVA,            "ro", "MD", 0     },
    { LANGUAGE_SLOVAK,                      "sk", "SK", 0     },
    { LANGUAGE_SLOVENIAN,                   "sl", "SI", 0     },
    { LANGUAGE_SPANISH_MEXICAN,             "es", "MX", 0     },
    { LANGUAGE_SPANISH_GUATEMALA,           "es", "GT", 0     },
    { LANGUAGE_SPANISH_COSTARICA,           "es", "CR", 0     },
    { LANGUAGE_SPANISH_PANAMA,              "es", "PA", 0     },
    { LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,  "es", "DO", 0     },
    { LANGUAGE_SPANISH_VENEZUELA,           "es", "VE", 0     },
    { LANGUAGE_SPANISH_COLOMBIA,            "es", "CO", 0     },
    { LANGUAGE_SPANISH_PERU,                "es", "PE", 0     },
    { LANGUAGE_SPANISH_ARGENTINA,           "es", "AR", 0     },
    { LANGUAGE_SPANISH_ECUADOR,             "es", "EC", 0     },
    { LANGUAGE_SPANISH_CHILE,               "es", "CL", 0     },
    { LANGUAGE_SPANISH_URUGUAY,             "es", "UY", 0     },
    { LANGUAGE_SPANISH_PARAGUAY,            "es", "PY", 0     },
    { LANGUAGE_SPANISH_BOLIVIA,             "es", "BO", 0     },
    { LANGUAGE_SPANISH_EL_SALVADOR,         "es", "SV", 0     },
    { LANGUAGE_SPANISH_HONDURAS,            "es", "HN", 0     },
    { LANGUAGE_SPANISH_NICARAGUA,           "es", "NI", 0     },
    { LANGUAGE_SPANISH_PUERTO_RICO,         "es", "PR", 0     },
    { LANGUAGE_SPANISH_UNITED_STATES,       "es", "US", 0     },
    { LANGUAGE_SPANISH_LATIN_AMERICA,       "es", ""  , 0     },
    { LANGUAGE_TURKISH,                     "tr", "TR", 0     },
    { LANGUAGE_UKRAINIAN,                   "uk", "UA", 0     },
    { LANGUAGE_VIETNAMESE,                  "vi", "VN", 0     },
    { LANGUAGE_LATVIAN,                     "lv", "LV", 0     },
    { LANGUAGE_MACEDONIAN,                  "mk", "MK", 0     },
    { LANGUAGE_MALAY_MALAYSIA,              "ms", "MY", 0     },
    { LANGUAGE_MALAY_BRUNEI_DARUSSALAM,     "ms", "BN", 0     },
    { LANGUAGE_ENGLISH_MALAYSIA,            "en", "MY", 0     },
    { LANGUAGE_THAI,                        "th", "TH", 0     },
    { LANGUAGE_LITHUANIAN,                  "lt", "LT", 0     },
    { LANGUAGE_LITHUANIAN_CLASSIC,          "lt", "LT", 0     },
    { LANGUAGE_CROATIAN,                    "hr", "HR", 0     },    
    { LANGUAGE_CROATIAN_BOSNIA_HERZEGOVINA, "hr", "BA", 0     },
    { LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA,          "bs", "BA", 0     },
    { LANGUAGE_BOSNIAN_LSO,                               "bs", ""  , 0     },  
    { LANGUAGE_SERBIAN_CYRILLIC_SERBIA,                   "sr", "RS", 0     },  
    { LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_SERBIA,     "sr", "RS", 0     },
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,                      "sr", "CS", 0     },  
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,                      "sr", "YU", kSAME },  
    { LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO,               "sr", "ME", 0     },
    { LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_MONTENEGRO, "sr", "ME", 0     },
    { LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA,       "sr", "BA", 0     },
    { LANGUAGE_SERBIAN_CYRILLIC_LSO,                      "sr", ""  , 0     },
    { LANGUAGE_SERBIAN_LATIN_SERBIA,                      "sh", "RS", kSAME },  
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA,        "sh", "RS", kSAME },  
    { LANGUAGE_SERBIAN_LATIN_SAM,                         "sh", "CS", kSAME },  
    { LANGUAGE_SERBIAN_LATIN_SAM,                         "sh", "YU", kSAME },  
    { LANGUAGE_SERBIAN_LATIN_MONTENEGRO,                  "sh", "ME", kSAME },  
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO,    "sh", "ME", kSAME },  
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,          "sh", "BA", kSAME },  
    { LANGUAGE_SERBIAN_LATIN_LSO,                         "sh", ""  , kSAME },  
    { LANGUAGE_ARMENIAN,                    "hy", "AM", 0     },
    { LANGUAGE_AZERI_LATIN,                 "az", "AZ", 0     },    
    { LANGUAGE_UZBEK_LATIN,                 "uz", "UZ", 0     },    
    { LANGUAGE_UZBEK_LATIN_LSO,             "uz", ""  , 0     },    
    { LANGUAGE_BENGALI_BANGLADESH,          "bn", "BD", 0     },
    { LANGUAGE_BENGALI,                     "bn", "IN", 0     },
    { LANGUAGE_BURMESE,                     "my", "MM", 0     },
    { LANGUAGE_KAZAKH,                      "kk", "KZ", 0     },
    { LANGUAGE_ENGLISH_INDIA,               "en", "IN", 0     },
    { LANGUAGE_URDU_INDIA,                  "ur", "IN", 0     },
    { LANGUAGE_URDU_PAKISTAN,               "ur", "PK", 0     },
    { LANGUAGE_HINDI,                       "hi", "IN", 0     },
    { LANGUAGE_GUJARATI,                    "gu", "IN", 0     },
    { LANGUAGE_KANNADA,                     "kn", "IN", 0     },
    { LANGUAGE_ASSAMESE,                    "as", "IN", 0     },
    { LANGUAGE_KASHMIRI_INDIA,              "ks", "IN", 0     },
    { LANGUAGE_KASHMIRI,                    "ks", ""  , kSAME },    
    { LANGUAGE_MALAYALAM,                   "ml", "IN", 0     },
    { LANGUAGE_MANIPURI,                   "mni", "IN", 0     },
    { LANGUAGE_MARATHI,                     "mr", "IN", 0     },
    { LANGUAGE_KONKANI,                    "kok", "IN", 0     },
    { LANGUAGE_NEPALI,                      "ne", "NP", 0     },
    { LANGUAGE_NEPALI_INDIA,                "ne", "IN", 0     },
    { LANGUAGE_ODIA,                        "or", "IN", 0     },
    { LANGUAGE_PUNJABI,                     "pa", "IN", 0     },
    { LANGUAGE_SANSKRIT,                    "sa", "IN", 0     },
    { LANGUAGE_TAMIL,                       "ta", "IN", 0     },
    { LANGUAGE_TAMIL_SRI_LANKA,             "ta", "LK", 0     },
    { LANGUAGE_TELUGU,                      "te", "IN", 0     },
    { LANGUAGE_PUNJABI_PAKISTAN,           "pnb", "PK", 0     },
    { LANGUAGE_PUNJABI_ARABIC_LSO,         "pnb", ""  , 0     },
    { LANGUAGE_PUNJABI_PAKISTAN,           "lah", "PK", kSAME },    
    { LANGUAGE_PUNJABI_PAKISTAN,            "pa", "PK", kSAME },    
    { LANGUAGE_SINDHI_PAKISTAN,             "sd", "PK", kSAME },    
    { LANGUAGE_SINDHI,                      "sd", "IN", 0     },    
    { LANGUAGE_BELARUSIAN,                  "be", "BY", 0     },
    { LANGUAGE_CATALAN,                     "ca", "ES", 0     },    
    { LANGUAGE_CATALAN,                     "ca", "AD", 0     },    
    
    { LANGUAGE_CATALAN_VALENCIAN,           "ca", "XV", kSAME },    
    { LANGUAGE_CATALAN_VALENCIAN,          "qcv", "ES", kSAME },    
    { LANGUAGE_FRENCH_CAMEROON,             "fr", "CM", 0     },
    { LANGUAGE_FRENCH_COTE_D_IVOIRE,        "fr", "CI", 0     },
    { LANGUAGE_FRENCH_MALI,                 "fr", "ML", 0     },
    { LANGUAGE_FRENCH_SENEGAL,              "fr", "SN", 0     },
    { LANGUAGE_FRENCH_ZAIRE,                "fr", "CD", 0     },    
    { LANGUAGE_FRENCH_MOROCCO,              "fr", "MA", 0     },
    { LANGUAGE_FRENCH_REUNION,              "fr", "RE", 0     },
    { LANGUAGE_FRENCH,                      "fr", ""  , 0     },    
    { LANGUAGE_FRENCH_NORTH_AFRICA,         "fr", ""  , 0     },
    { LANGUAGE_FRENCH_WEST_INDIES,          "fr", ""  , 0     },    
    { LANGUAGE_FRISIAN_NETHERLANDS,         "fy", "NL", 0     },
    { LANGUAGE_GAELIC_IRELAND,              "ga", "IE", 0     },
    { LANGUAGE_GAELIC_SCOTLAND,             "gd", "GB", 0     },
    { LANGUAGE_GAELIC_SCOTLAND_LEGACY,      "gd", "GB", 0     },
    { LANGUAGE_GALICIAN,                    "gl", "ES", 0     },
    { LANGUAGE_GEORGIAN,                    "ka", "GE", 0     },
    { LANGUAGE_KHMER,                       "km", "KH", 0     },
    { LANGUAGE_KIRGHIZ,                     "ky", "KG", 0     },
    { LANGUAGE_LAO,                         "lo", "LA", 0     },
    { LANGUAGE_MALTESE,                     "mt", "MT", 0     },
    { LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA, "mn", "MN", 0     },    
    { LANGUAGE_MONGOLIAN_CYRILLIC_LSO,      "mn", ""  , 0     },    
    { LANGUAGE_RUSSIAN_MOLDOVA,             "mo", "MD", 0     },
    { LANGUAGE_SWAHILI,                     "sw", "KE", 0     },
    { LANGUAGE_USER_SWAHILI_TANZANIA,       "sw", "TZ", 0     },
    { LANGUAGE_TAJIK,                       "tg", "TJ", 0     },
    { LANGUAGE_TAJIK_LSO,                   "tg", ""  , 0     },
    { LANGUAGE_TIBETAN,                     "bo", "CN", 0     },    
    { LANGUAGE_USER_TIBETAN_INDIA,          "bo", "IN", 0     },
    { LANGUAGE_USER_TIBETAN_BHUTAN,         "bo", "BT", 0     },    
    { LANGUAGE_DZONGKHA,                    "dz", "BT", 0     },
    { LANGUAGE_USER_DZONGKHA_MAP_LONLY,     "dz", ""  , 0     },    
    { LANGUAGE_TURKMEN,                     "tk", "TM", 0     },
    { LANGUAGE_WELSH,                       "cy", "GB", 0     },
    { LANGUAGE_SESOTHO,                     "st", "ZA", 0     },
    { LANGUAGE_SEPEDI,                     "nso", "ZA", 0     },
    { LANGUAGE_SEPEDI,                      "ns", "ZA", kSAME },    
    { LANGUAGE_TSONGA,                      "ts", "ZA", 0     },
    { LANGUAGE_TSWANA,                      "tn", "ZA", 0     },
    { LANGUAGE_ENGLISH_SAFRICA,             "en", "ZA", 0     },
    { LANGUAGE_AFRIKAANS,                   "af", "ZA", 0     },
    { LANGUAGE_VENDA,                       "ve", "ZA", 0     },    
    { LANGUAGE_VENDA,                      "ven", "ZA", kSAME },    
    { LANGUAGE_XHOSA,                       "xh", "ZA", 0     },
    { LANGUAGE_ZULU,                        "zu", "ZA", 0     },

    { LANGUAGE_QUECHUA_ECUADOR,            "quz", "EC", 0     },    
    { LANGUAGE_QUECHUA_ECUADOR,             "qu", "EC", kSAME },    
    { LANGUAGE_QUECHUA_PERU,               "quz", "PE", 0     },    
    { LANGUAGE_QUECHUA_PERU,                "qu", "PE", kSAME },    
    { LANGUAGE_QUECHUA_BOLIVIA,             "qu", "BO", 0     },    
    { LANGUAGE_PASHTO,                      "ps", "AF", 0     },
    { LANGUAGE_OROMO,                       "om", "ET", 0     },
    { LANGUAGE_DHIVEHI,                     "dv", "MV", 0     },
    { LANGUAGE_UIGHUR_CHINA,                "ug", "CN", 0     },
    { LANGUAGE_TIGRIGNA_ETHIOPIA,           "ti", "ET", 0     },
    { LANGUAGE_TIGRIGNA_ERITREA,            "ti", "ER", 0     },
    { LANGUAGE_AMHARIC_ETHIOPIA,            "am", "ET", 0     },
    { LANGUAGE_GUARANI_PARAGUAY,           "gug", "PY", 0     },
    { LANGUAGE_HAWAIIAN_UNITED_STATES,     "haw", "US", 0     },
    { LANGUAGE_EDO,                        "bin", "NG", 0     },
    { LANGUAGE_FULFULDE_NIGERIA,           "fuv", "NG", 0     },
    { LANGUAGE_FULFULDE_NIGERIA,            "ff", "NG", kSAME },    
    { LANGUAGE_FULFULDE_SENEGAL,            "ff", "SN", 0     },    
    { LANGUAGE_HAUSA_NIGERIA,               "ha", "NG", kSAME },
    { LANGUAGE_USER_HAUSA_GHANA,            "ha", "GH", kSAME },
    { LANGUAGE_IGBO_NIGERIA,                "ig", "NG", 0     },
    { LANGUAGE_KANURI_NIGERIA,              "kr", "NG", 0     },
    { LANGUAGE_YORUBA,                      "yo", "NG", 0     },
    { LANGUAGE_SOMALI,                      "so", "SO", 0     },
    { LANGUAGE_PAPIAMENTU,                 "pap", "AN", 0     },
    { LANGUAGE_USER_PAPIAMENTU_ARUBA,      "pap", "AW", 0     },
    { LANGUAGE_USER_PAPIAMENTU_CURACAO,    "pap", "CW", 0     },
    { LANGUAGE_USER_PAPIAMENTU_BONAIRE,    "pap", "BQ", 0     },
    { LANGUAGE_ENGLISH_SINGAPORE,           "en", "SG", 0     },
    { LANGUAGE_USER_YIDDISH_US,             "yi", "US", 0     },
    { LANGUAGE_YIDDISH,                     "yi", "IL", 0     },    
    { LANGUAGE_YIDDISH,                     "ji", "IL", kSAME },    
    { LANGUAGE_SYRIAC,                     "syr", "TR", 0     },    
    { LANGUAGE_SINHALESE_SRI_LANKA,         "si", "LK", 0     },
    { LANGUAGE_CHEROKEE_UNITED_STATES,     "chr", "US", kSAME },
    { LANGUAGE_INUKTITUT_LATIN_CANADA,      "iu", "CA", kSAME },    
    { LANGUAGE_INUKTITUT_LATIN_LSO,         "iu", ""  , kSAME },    
    { LANGUAGE_SAMI_NORTHERN_NORWAY,        "se", "NO", 0     },
    { LANGUAGE_SAMI_INARI,                 "smn", "FI", 0     },
    { LANGUAGE_SAMI_INARI_LSO,             "smn", ""  , 0     },
    { LANGUAGE_SAMI_LULE_NORWAY,           "smj", "NO", 0     },
    { LANGUAGE_SAMI_LULE_SWEDEN,           "smj", "SE", 0     },
    { LANGUAGE_SAMI_LULE_LSO,              "smj", ""  , 0     },
    { LANGUAGE_SAMI_NORTHERN_FINLAND,       "se", "FI", 0     },
    { LANGUAGE_SAMI_NORTHERN_SWEDEN,        "se", "SE", 0     },
    { LANGUAGE_SAMI_SKOLT,                 "sms", "FI", 0     },
    { LANGUAGE_SAMI_SKOLT_LSO,             "sms", ""  , 0     },
    { LANGUAGE_SAMI_SOUTHERN_NORWAY,       "sma", "NO", 0     },
    { LANGUAGE_SAMI_SOUTHERN_SWEDEN,       "sma", "SE", 0     },
    { LANGUAGE_SAMI_SOUTHERN_LSO,          "sma", ""  , 0     },
    { LANGUAGE_USER_SAMI_KILDIN_RUSSIA,    "sjd", "RU", 0     },
    { LANGUAGE_MAPUDUNGUN_CHILE,           "arn", "CL", 0     },
    { LANGUAGE_CORSICAN_FRANCE,             "co", "FR", 0     },
    { LANGUAGE_ALSATIAN_FRANCE,            "gsw", "FR", 0     },    
    { LANGUAGE_YAKUT_RUSSIA,               "sah", "RU", 0     },
    { LANGUAGE_MOHAWK_CANADA,              "moh", "CA", 0     },
    { LANGUAGE_BASHKIR_RUSSIA,              "ba", "RU", 0     },
    { LANGUAGE_KICHE_GUATEMALA,            "qut", "GT", 0     },
    { LANGUAGE_DARI_AFGHANISTAN,           "prs", "AF", 0     },
    { LANGUAGE_DARI_AFGHANISTAN,           "gbz", "AF", kSAME },    
    { LANGUAGE_WOLOF_SENEGAL,               "wo", "SN", 0     },
    { LANGUAGE_FILIPINO,                   "fil", "PH", 0     },
    { LANGUAGE_USER_TAGALOG,                "tl", "PH", 0     },
    { LANGUAGE_ENGLISH_PHILIPPINES,         "en", "PH", 0     },
    { LANGUAGE_IBIBIO_NIGERIA,             "ibb", "NG", 0     },
    { LANGUAGE_YI,                          "ii", "CN", 0     },
    { LANGUAGE_ENGLISH_ARAB_EMIRATES,       "en", "AE", 0     },    
    { LANGUAGE_ENGLISH_BAHRAIN,             "en", "BH", 0     },    
    { LANGUAGE_ENGLISH_EGYPT,               "en", "EG", 0     },    
    { LANGUAGE_ENGLISH_JORDAN,              "en", "JO", 0     },    
    { LANGUAGE_ENGLISH_KUWAIT,              "en", "KW", 0     },    
    { LANGUAGE_ENGLISH_TURKEY,              "en", "TR", 0     },    
    { LANGUAGE_ENGLISH_YEMEN,               "en", "YE", 0     },    
    { LANGUAGE_TAMAZIGHT_LATIN_ALGERIA,    "kab", "DZ", 0     },    
    { LANGUAGE_OBSOLETE_USER_KABYLE,       "kab", "DZ", 0     },
    { LANGUAGE_TAMAZIGHT_LATIN_ALGERIA,    "ber", "DZ", kSAME },    
    { LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO, "tmz", "MA", kSAME },
    { LANGUAGE_TAMAZIGHT_MOROCCO,          "tmz", "MA", 0     },    
    { LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO, "ber", "MA", kSAME },    
    { LANGUAGE_USER_LATIN_VATICAN,          "la", "VA", 0     },
    { LANGUAGE_OBSOLETE_USER_LATIN,         "la", "VA", 0     },
    { LANGUAGE_LATIN_LSO,                   "la", ""  , 0     },
    { LANGUAGE_USER_ESPERANTO,              "eo", ""  , 0     },
    { LANGUAGE_USER_INTERLINGUA,            "ia", ""  , 0     },
    { LANGUAGE_MAORI_NEW_ZEALAND,           "mi", "NZ", 0     },
    { LANGUAGE_OBSOLETE_USER_MAORI,         "mi", "NZ", 0     },
    { LANGUAGE_KINYARWANDA_RWANDA,          "rw", "RW", 0     },
    { LANGUAGE_OBSOLETE_USER_KINYARWANDA,   "rw", "RW", 0     },
    { LANGUAGE_UPPER_SORBIAN_GERMANY,      "hsb", "DE", 0     },    
    { LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN,"hsb", "DE", 0     },
    { LANGUAGE_LOWER_SORBIAN_GERMANY,      "dsb", "DE", 0     },    
    { LANGUAGE_LOWER_SORBIAN_LSO,          "dsb", ""  , 0     },
    { LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN,"dsb", "DE", 0     },
    { LANGUAGE_OCCITAN_FRANCE,              "oc", "FR", 0     },
    { LANGUAGE_OBSOLETE_USER_OCCITAN,       "oc", "FR", 0     },
    { LANGUAGE_USER_KURDISH_TURKEY,        "kmr", "TR", kSAME },
    { LANGUAGE_USER_KURDISH_TURKEY,         "ku", "TR", kSAME },
    { LANGUAGE_USER_KURDISH_SYRIA,         "kmr", "SY", kSAME },
    { LANGUAGE_USER_KURDISH_SYRIA,          "ku", "SY", kSAME },
    { LANGUAGE_KURDISH_ARABIC_IRAQ,        "ckb", "IQ", 0     },
    { LANGUAGE_KURDISH_ARABIC_IRAQ,         "ku", "IQ", kSAME },
    { LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ,  "ku", "IQ", LANGUAGE_KURDISH_ARABIC_IRAQ },
    { LANGUAGE_USER_KURDISH_SOUTHERN_IRAN, "sdh", "IR", 0     },
    { LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ, "sdh", "IQ", 0     },
    { LANGUAGE_USER_KURDISH_IRAN,          "ckb", "IR", 0     },
    { LANGUAGE_USER_KURDISH_IRAN,           "ku", "IR", kSAME },
    { LANGUAGE_KURDISH_ARABIC_LSO,         "ckb", ""  , 0     },
    { LANGUAGE_USER_SARDINIAN,              "sc", "IT", 0     },    
    { LANGUAGE_USER_SARDINIAN_CAMPIDANESE, "sro", "IT", 0     },
    { LANGUAGE_USER_SARDINIAN_GALLURESE,   "sdn", "IT", 0     },
    { LANGUAGE_USER_SARDINIAN_LOGUDORESE,  "src", "IT", 0     },
    { LANGUAGE_USER_SARDINIAN_SASSARESE,   "sdc", "IT", 0     },
    { LANGUAGE_BRETON_FRANCE,               "br", "FR", 0     },
    { LANGUAGE_OBSOLETE_USER_BRETON,        "br", "FR", 0     },
    { LANGUAGE_KALAALLISUT_GREENLAND,       "kl", "GL", 0     },
    { LANGUAGE_OBSOLETE_USER_KALAALLISUT,   "kl", "GL", 0     },
    { LANGUAGE_USER_SWAZI,                  "ss", "ZA", 0     },
    { LANGUAGE_USER_NDEBELE_SOUTH,          "nr", "ZA", 0     },
    { LANGUAGE_TSWANA_BOTSWANA,             "tn", "BW", 0     },
    { LANGUAGE_OBSOLETE_USER_TSWANA_BOTSWANA, "tn", "BW", 0     },
    { LANGUAGE_USER_MOORE,                 "mos", "BF", 0     },
    { LANGUAGE_USER_BAMBARA,                "bm", "ML", 0     },
    { LANGUAGE_USER_AKAN,                   "ak", "GH", 0     },
    { LANGUAGE_LUXEMBOURGISH_LUXEMBOURG,    "lb", "LU", 0     },
    { LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH, "lb", "LU", 0     },
    { LANGUAGE_USER_FRIULIAN,              "fur", "IT", 0     },
    { LANGUAGE_USER_FIJIAN,                 "fj", "FJ", 0     },
    { LANGUAGE_USER_AFRIKAANS_NAMIBIA,      "af", "NA", 0     },
    { LANGUAGE_USER_ENGLISH_NAMIBIA,        "en", "NA", 0     },
    { LANGUAGE_USER_WALLOON,                "wa", "BE", 0     },
    { LANGUAGE_USER_COPTIC,                "cop", "EG", 0     },
    { LANGUAGE_USER_GASCON,                "gsc", "FR", 0     },
    { LANGUAGE_USER_GERMAN_BELGIUM,         "de", "BE", 0     },
    { LANGUAGE_USER_CHUVASH,                "cv", "RU", 0     },
    { LANGUAGE_USER_EWE_GHANA,              "ee", "GH", 0     },
    { LANGUAGE_USER_ENGLISH_GHANA,          "en", "GH", 0     },
    { LANGUAGE_USER_SANGO,                  "sg", "CF", 0     },
    { LANGUAGE_USER_GANDA,                  "lg", "UG", 0     },
    { LANGUAGE_USER_LINGALA_DRCONGO,        "ln", "CD", 0     },
    { LANGUAGE_USER_LOW_GERMAN,            "nds", "DE", 0     },
    { LANGUAGE_USER_HILIGAYNON,            "hil", "PH", 0     },
    { LANGUAGE_USER_ENGLISH_MALAWI,         "en", "MW", 0     },   /* en default for MW */
    { LANGUAGE_USER_NYANJA,                 "ny", "MW", 0     },
    { LANGUAGE_USER_KASHUBIAN,             "csb", "PL", 0     },
    { LANGUAGE_USER_SPANISH_CUBA,           "es", "CU", 0     },
    { LANGUAGE_USER_QUECHUA_NORTH_BOLIVIA, "qul", "BO", 0     },
    { LANGUAGE_USER_QUECHUA_SOUTH_BOLIVIA, "quh", "BO", 0     },
    { LANGUAGE_USER_BODO_INDIA,            "brx", "IN", 0     },
    { LANGUAGE_USER_DOGRI_INDIA,           "dgo", "IN", 0     },
    { LANGUAGE_USER_MAITHILI_INDIA,        "mai", "IN", 0     },
    { LANGUAGE_USER_SANTALI_INDIA,         "sat", "IN", 0     },
    { LANGUAGE_USER_TETUN,                 "tet", "ID", 0     },
    { LANGUAGE_USER_TETUN_TIMOR_LESTE,     "tet", "TL", 0     },
    { LANGUAGE_USER_TOK_PISIN,             "tpi", "PG", 0     },
    { LANGUAGE_USER_SHUSWAP,               "shs", "CA", 0     },
    { LANGUAGE_USER_ANCIENT_GREEK,         "grc", "GR", 0     },
    { LANGUAGE_USER_ASTURIAN,              "ast", "ES", 0     },
    { LANGUAGE_USER_LATGALIAN,             "ltg", "LV", 0     },
    { LANGUAGE_USER_MAORE,                 "swb", "YT", 0     },
    { LANGUAGE_USER_BUSHI,                 "buc", "YT", 0     },
    { LANGUAGE_USER_TAHITIAN,               "ty", "PF", 0     },
    { LANGUAGE_MALAGASY_PLATEAU,           "plt", "MG", 0     },
    { LANGUAGE_MALAGASY_PLATEAU,            "mg", "MG", kSAME },
    { LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU, "plt", "MG", 0     },
    { LANGUAGE_USER_BAFIA,                 "ksf", "CM", 0     },
    { LANGUAGE_USER_GIKUYU,                 "ki", "KE", 0     },
    { LANGUAGE_USER_RUSYN_UKRAINE,         "rue", "UA", 0     },
    { LANGUAGE_USER_RUSYN_SLOVAKIA,        "rue", "SK", 0     },
    { LANGUAGE_USER_LIMBU,                 "lif", "NP", 0     },
    { LANGUAGE_USER_LOJBAN,                "jbo", ""  , 0     },
    { LANGUAGE_USER_HAITIAN,                "ht", "HT", 0     },
    { LANGUAGE_FRENCH_HAITI,                "fr", "HT", 0     },
    { LANGUAGE_USER_BEEMBE,                "beq", "CG", 0     },
    { LANGUAGE_USER_BEKWEL,                "bkw", "CG", 0     },
    { LANGUAGE_USER_KITUBA,                "mkw", "CG", 0     },
    { LANGUAGE_USER_LARI,                  "ldi", "CG", 0     },
    { LANGUAGE_USER_MBOCHI,                "mdw", "CG", 0     },
    { LANGUAGE_USER_TEKE_EBOO,             "ebo", "CG", 0     },
    { LANGUAGE_USER_TEKE_IBALI,            "tek", "CG", 0     },
    { LANGUAGE_USER_TEKE_TYEE,             "tyx", "CG", 0     },
    { LANGUAGE_USER_VILI,                  "vif", "CG", 0     },
    { LANGUAGE_USER_PORTUGUESE_ANGOLA,      "pt", "AO", 0     },
    { LANGUAGE_USER_MANX,                   "gv", "GB", 0     },
    { LANGUAGE_USER_ARAGONESE,              "an", "ES", 0     },
    { LANGUAGE_USER_KEYID,                 "qtz", ""  , 0     },    
    { LANGUAGE_USER_PALI_LATIN,            "pli", ""  , kSAME },    
    { LANGUAGE_USER_KYRGYZ_CHINA,           "ky", "CN", 0     },
    { LANGUAGE_USER_KOMI_ZYRIAN,           "kpv", "RU", 0     },
    { LANGUAGE_USER_KOMI_PERMYAK,          "koi", "RU", 0     },
    { LANGUAGE_USER_PITJANTJATJARA,        "pjt", "AU", 0     },
    { LANGUAGE_USER_ERZYA,                 "myv", "RU", 0     },
    { LANGUAGE_USER_MARI_MEADOW,           "mhr", "RU", 0     },
    { LANGUAGE_USER_KHANTY,                "kca", "RU", 0     },
    { LANGUAGE_USER_LIVONIAN,              "liv", "RU", 0     },
    { LANGUAGE_USER_MOKSHA,                "mdf", "RU", 0     },
    { LANGUAGE_USER_MARI_HILL,             "mrj", "RU", 0     },
    { LANGUAGE_USER_NGANASAN,              "nio", "RU", 0     },
    { LANGUAGE_USER_OLONETS,               "olo", "RU", 0     },
    { LANGUAGE_USER_VEPS,                  "vep", "RU", 0     },
    { LANGUAGE_USER_VORO,                  "vro", "EE", 0     },
    { LANGUAGE_USER_NENETS,                "yrk", "RU", 0     },
    { LANGUAGE_USER_AKA,                   "axk", "CF", 0     },
    { LANGUAGE_USER_AKA_CONGO,             "axk", "CG", 0     },
    { LANGUAGE_USER_DIBOLE,                "bvx", "CG", 0     },
    { LANGUAGE_USER_DOONDO,                "dde", "CG", 0     },
    { LANGUAGE_USER_KAAMBA,                "xku", "CG", 0     },
    { LANGUAGE_USER_KOONGO,                "kng", "CD", 0     },
    { LANGUAGE_USER_KOONGO_CONGO,          "kng", "CG", 0     },
    { LANGUAGE_USER_KUNYI,                 "njx", "CG", 0     },
    { LANGUAGE_USER_NGUNGWEL,              "ngz", "CG", 0     },
    { LANGUAGE_USER_NJYEM,                 "njy", "CM", 0     },
    { LANGUAGE_USER_NJYEM_CONGO,           "njy", "CG", 0     },
    { LANGUAGE_USER_PUNU,                  "puu", "GA", 0     },
    { LANGUAGE_USER_PUNU_CONGO,            "puu", "CG", 0     },
    { LANGUAGE_USER_SUUNDI,                "sdj", "CG", 0     },
    { LANGUAGE_USER_TEKE_KUKUYA,           "kkw", "CG", 0     },
    { LANGUAGE_USER_TSAANGI,               "tsa", "CG", 0     },
    { LANGUAGE_USER_YAKA,                  "iyx", "CG", 0     },
    { LANGUAGE_USER_YOMBE,                 "yom", "CD", 0     },
    { LANGUAGE_USER_YOMBE_CONGO,           "yom", "CG", 0     },
    { LANGUAGE_USER_SIDAMA,                "sid", "ET", 0     },
    { LANGUAGE_USER_NKO,                   "nqo", "GN", 0     },
    { LANGUAGE_USER_UDMURT,                "udm", "RU", 0     },
    { LANGUAGE_USER_CORNISH,               "kw",  "UK", 0     },
    { LANGUAGE_USER_SAMI_PITE_SWEDEN,      "sje", "SE", 0     },
    { LANGUAGE_USER_NGAEBERE,              "gym", "PA", 0     },
    { LANGUAGE_USER_KUMYK,                 "kum", "RU", 0     },
    { LANGUAGE_USER_NOGAI,                 "nog", "RU", 0     },
    { LANGUAGE_USER_LADIN,                 "lld", "IT", 0     },
    { LANGUAGE_USER_FRENCH_BURKINA_FASO,    "fr", "BF", 0     },
    { LANGUAGE_USER_PUINAVE,               "pui", "CO", 0     },
    { LANGUAGE_MULTIPLE,                   "mul", ""  , 0     },    
    { LANGUAGE_UNDETERMINED,               "und", ""  , 0     },    
    { LANGUAGE_NONE,                       "zxx", ""  , 0     },    
    { LANGUAGE_DONTKNOW,                    "",   ""  , 0     }     
};

static IsoLanguageScriptCountryEntry const aImplIsoLangScriptEntries[] =
{
    
    { LANGUAGE_SERBIAN_LATIN_SERBIA,                "sr-Latn", "RS", 0     },
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA,  "sr-Latn", "RS", 0     },
    { LANGUAGE_SERBIAN_LATIN_MONTENEGRO,            "sr-Latn", "ME", 0     },
    { LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO,"sr-Latn", "ME", 0     },
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,    "sr-Latn", "BA", 0     },
    { LANGUAGE_SERBIAN_LATIN_SAM,                   "sr-Latn", "CS", 0     },   
    { LANGUAGE_SERBIAN_LATIN_SAM,                   "sr-Latn", "YU", 0     },   
    { LANGUAGE_SERBIAN_LATIN_LSO,                   "sr-Latn", ""  , 0     },
    { LANGUAGE_SERBIAN_LATIN_NEUTRAL,               "sr-Latn", ""  , LANGUAGE_SERBIAN_LATIN_LSO },  
    { LANGUAGE_SERBIAN_CYRILLIC_SERBIA,             "sr-Cyrl", "RS", kSAME },   
    { LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO,         "sr-Cyrl", "ME", kSAME },   
    { LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "sr-Cyrl", "BA", kSAME },   
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,                "sr-Cyrl", "CS", kSAME },   
    { LANGUAGE_SERBIAN_CYRILLIC_LSO,                "sr-Cyrl", ""  , kSAME },   
    { LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "bs-Cyrl", "BA", 0     },
    { LANGUAGE_BOSNIAN_CYRILLIC_LSO,                "bs-Cyrl", ""  , 0     },
    { LANGUAGE_AZERI_CYRILLIC,                      "az-Cyrl", "AZ", 0     },   
    { LANGUAGE_AZERI_CYRILLIC_LSO,                  "az-Cyrl", ""  , 0     },   
    { LANGUAGE_UZBEK_CYRILLIC,                      "uz-Cyrl", "UZ", 0     },   
    { LANGUAGE_UZBEK_CYRILLIC_LSO,                  "uz-Cyrl", ""  , 0     },   
    { LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA,         "mn-Cyrl", "MN", 0     },   
    { LANGUAGE_MONGOLIAN_CYRILLIC_LSO,              "mn-Cyrl", ""  , 0     },   
    { LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA,        "mn-Mong", "MN", 0     },   
    { LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA,           "mn-Mong", "CN", 0     },   
    { LANGUAGE_MONGOLIAN_MONGOLIAN_LSO,             "mn-Mong", ""  , 0     },   
    { LANGUAGE_USER_PALI_LATIN,                     "pi-Latn", ""  , 0     },
    { LANGUAGE_USER_KARAKALPAK_LATIN,              "kaa-Latn", "UZ", 0     },
    { LANGUAGE_TAJIK,                               "tg-Cyrl", "TJ", 0     },   
    { LANGUAGE_TAJIK_LSO,                           "tg-Cyrl", ""  , 0     },   
    { LANGUAGE_AZERI_LATIN,                         "az-Latn", "AZ", 0     },   
    { LANGUAGE_AZERI_LATIN_LSO,                     "az-Latn", ""  , 0     },   
    { LANGUAGE_USER_YIDDISH_US,                     "yi-Hebr", "US", kSAME },   
    { LANGUAGE_YIDDISH,                             "yi-Hebr", "IL", kSAME },   
    { LANGUAGE_UZBEK_LATIN,                         "uz-Latn", "UZ", 0     },   
    { LANGUAGE_UZBEK_LATIN_LSO,                     "uz-Latn", ""  , 0     },

    { LANGUAGE_SINDHI_PAKISTAN,                     "sd-Arab", "PK", 0     },   
    { LANGUAGE_SINDHI_ARABIC_LSO,                   "sd-Arab", ""  , 0     },
    { LANGUAGE_CHEROKEE_UNITED_STATES,             "chr-Cher", "US", 0     },   
    { LANGUAGE_CHEROKEE_CHEROKEE_LSO,              "chr-Cher", ""  , 0     },
    { LANGUAGE_INUKTITUT_SYLLABICS_CANADA,          "iu-Cans", "CA", 0     },   
    { LANGUAGE_INUKTITUT_SYLLABICS_LSO,             "iu-Cans", ""  , 0     },   
    { LANGUAGE_INUKTITUT_LATIN_CANADA,              "iu-Latn", "CA", 0     },   
    { LANGUAGE_INUKTITUT_LATIN_LSO,                 "iu-Latn", ""  , 0     },   
    { LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO,         "tzm-Tfng", "MA", 0     },
    { LANGUAGE_TAMAZIGHT_TIFINAGH_LSO,             "tzm-Tfng", ""  , 0     },
    { LANGUAGE_KASHMIRI_INDIA,                      "ks-Deva", "IN", 0     },   
    { LANGUAGE_KASHMIRI,                            "ks-Arab", ""  , 0     },   
    { LANGUAGE_HAUSA_NIGERIA,                       "ha-Latn", "NG", 0     },   
    { LANGUAGE_USER_HAUSA_GHANA,                    "ha-Latn", "GH", 0     },
    { LANGUAGE_HAUSA_LATIN_LSO,                     "ha-Latn", ""  , 0     },
    { LANGUAGE_LATIN_LSO,                           "la-Latn", ""  , kSAME },   
    { LANGUAGE_TAI_NUA_CHINA,                      "tdd-Tale", "CN", 0     },   
    { LANGUAGE_LU_CHINA,                           "khb-Talu", "CN", 0     },   
    { LANGUAGE_KURDISH_ARABIC_IRAQ,                 "ku-Arab", "IQ", kSAME },   
    { LANGUAGE_KURDISH_ARABIC_LSO,                  "ku-Arab", ""  , kSAME },   
    { LANGUAGE_USER_KURDISH_TURKEY,                "kmr-Latn", "TR", 0     },
    { LANGUAGE_USER_KURDISH_SYRIA,                 "kmr-Latn", "SY", 0     },
    { LANGUAGE_PUNJABI_PAKISTAN,                   "pnb-Arab", "PK", 0     },
    { LANGUAGE_PUNJABI_ARABIC_LSO,                 "pnb-Arab", ""  , 0     },
    { LANGUAGE_PUNJABI_PAKISTAN,                    "pa-Arab", "PK", 0     },   
    { LANGUAGE_PUNJABI_ARABIC_LSO,                  "pa-Arab", ""  , 0     },   
    { LANGUAGE_TAMAZIGHT_LATIN_ALGERIA,            "tzm-Latn", "DZ", kSAME },   
    { LANGUAGE_TAMAZIGHT_LATIN_LSO,                "tzm-Latn", ""  , 0     },   
    { LANGUAGE_FULFULDE_SENEGAL,                    "ff-Latn", "SN", 0     },   
    { LANGUAGE_FULFULDE_LATIN_LSO,                  "ff-Latn", ""  , 0     },   
    { LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA,    "bs-Latn", "BA", kSAME },   
    { LANGUAGE_BOSNIAN_LATIN_LSO,                   "bs-Latn", ""  , LANGUAGE_BOSNIAN_LSO },   
    { LANGUAGE_CHINESE_TRADITIONAL_LSO,             "zh-Hant", ""  , 0     },
    { LANGUAGE_DONTKNOW,                            "",        ""  , 0     }    
};

static Bcp47CountryEntry const aImplBcp47CountryEntries[] =
{
    
    { LANGUAGE_CATALAN_VALENCIAN,       "ca-ES-valencia", "ES", "ca-valencia" },
    { LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN, "ca-ES-valencia", "ES", "" },   
    { LANGUAGE_USER_ENGLISH_UK_OED,          "en-GB-oed", "GB", "" },   

    { LANGUAGE_DONTKNOW,                    "", "", "" }    
};

static IsoLanguageCountryEntry aLastResortFallbackEntry =
{ LANGUAGE_ENGLISH_US, "en", "US", 0 };

OUString IsoLanguageCountryEntry::getTagString() const
{
    if (maCountry[0])
        return OUString( OUString::createFromAscii( maLanguage) + "-" + OUString::createFromAscii( maCountry));
    else
        return OUString::createFromAscii( maLanguage);
}

::com::sun::star::lang::Locale IsoLanguageCountryEntry::getLocale() const
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

::com::sun::star::lang::Locale IsoLanguageScriptCountryEntry::getLocale() const
{
    return lang::Locale( I18NLANGTAG_QLT, OUString::createFromAscii( maCountry), getTagString());
}

bool IsoLanguageScriptCountryEntry::startsInIgnoreAsciiCase( const OUString & rStr ) const
{
    return rStr.matchIgnoreAsciiCaseAsciiL( maLanguageScript, strlen( maLanguageScript), 0);
}

OUString Bcp47CountryEntry::getTagString() const
{
    return OUString::createFromAscii( mpBcp47);
}

::com::sun::star::lang::Locale Bcp47CountryEntry::getLocale() const
{
    return lang::Locale( I18NLANGTAG_QLT, OUString::createFromAscii( maCountry), getTagString());
}





static IsoLangEngEntry const aImplIsoLangEngEntries[] =
{
    { LANGUAGE_ENGLISH_UK,                  "AO" },         
    { LANGUAGE_ENGLISH_UK,                  "BJ" },         
    { LANGUAGE_ENGLISH_UK,                  "BW" },         
    { LANGUAGE_ENGLISH_UK,                  "BI" },         
    { LANGUAGE_ENGLISH_UK,                  "CM" },         
    { LANGUAGE_ENGLISH_UK,                  "GA" },         
    { LANGUAGE_ENGLISH_UK,                  "GM" },         
    { LANGUAGE_ENGLISH_UK,                  "GH" },         
    { LANGUAGE_ENGLISH_UK,                  "GN" },         
    { LANGUAGE_ENGLISH_UK,                  "LS" },         
    { LANGUAGE_ENGLISH_UK,                  "MW" },         
    { LANGUAGE_ENGLISH_UK,                  "MT" },         
    { LANGUAGE_ENGLISH_UK,                  "NA" },         
    { LANGUAGE_ENGLISH_UK,                  "NG" },         
    { LANGUAGE_ENGLISH_UK,                  "UG" },         
    { LANGUAGE_ENGLISH_UK,                  "ZM" },         
    { LANGUAGE_ENGLISH_UK,                  "ZW" },         
    { LANGUAGE_ENGLISH_UK,                  "SZ" },         
    { LANGUAGE_ENGLISH_UK,                  "NG" },         
    { LANGUAGE_ENGLISH_UK,                  "KN" },         
    { LANGUAGE_ENGLISH_UK,                  "SH" },         
    { LANGUAGE_ENGLISH_UK,                  "IO" },         
    { LANGUAGE_ENGLISH_UK,                  "FK" },         
    { LANGUAGE_ENGLISH_UK,                  "GI" },         
    { LANGUAGE_ENGLISH_UK,                  "KI" },         
    { LANGUAGE_ENGLISH_UK,                  "VG" },         
    { LANGUAGE_ENGLISH_UK,                  "MU" },         
    { LANGUAGE_ENGLISH_UK,                  "FJ" },         
    { LANGUAGE_ENGLISH_US,                  "KI" },         
    { LANGUAGE_ENGLISH_US,                  "LR" },         
    { LANGUAGE_ENGLISH_US,                  "GU" },         
    { LANGUAGE_ENGLISH_US,                  "MH" },         
    { LANGUAGE_ENGLISH_US,                  "PW" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "AI" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "AG" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BS" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BB" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BM" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "KY" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "GD" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "DM" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "HT" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "MS" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "FM" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "VC" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "LC" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "TC" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "GY" },         
    { LANGUAGE_ENGLISH_CARRIBEAN,           "TT" },         
    { LANGUAGE_ENGLISH_AUS,                 "CX" },         
    { LANGUAGE_ENGLISH_AUS,                 "CC" },         
    { LANGUAGE_ENGLISH_AUS,                 "NF" },         
    { LANGUAGE_ENGLISH_AUS,                 "PG" },         
    { LANGUAGE_ENGLISH_AUS,                 "SB" },         
    { LANGUAGE_ENGLISH_AUS,                 "TV" },         
    { LANGUAGE_ENGLISH_AUS,                 "NR" },         
    { LANGUAGE_ENGLISH_NZ,                  "CK" },         
    { LANGUAGE_ENGLISH_NZ,                  "NU" },         
    { LANGUAGE_ENGLISH_NZ,                  "TK" },         
    { LANGUAGE_ENGLISH_NZ,                  "TO" },         
    { LANGUAGE_DONTKNOW,                    ""   }          
};



static IsoLangNoneStdEntry const aImplIsoNoneStdLangEntries[] =
{
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "BOK"      }, 
    { LANGUAGE_NORWEGIAN_NYNORSK,           "no", "NYN"      }, 
    { LANGUAGE_SERBIAN_LATIN_SAM,           "sr", "latin"    },
    { LANGUAGE_SERBIAN_CYRILLIC_SAM,        "sr", "cyrillic" },
    { LANGUAGE_AZERI_LATIN,                 "az", "latin"    },
    { LANGUAGE_AZERI_CYRILLIC,              "az", "cyrillic" },
    { LANGUAGE_DONTKNOW,                    "",   ""         }  
};




static IsoLangNoneStdEntry const aImplIsoNoneStdLangEntries2[] =
{
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "bokmaal"  },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "bokmal"   },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "no", "nynorsk"  },
    { LANGUAGE_DONTKNOW,                    "",   ""         }  
};




static IsoLangOtherEntry const aImplOtherEntries[] =
{
    { LANGUAGE_ENGLISH_US,                  "c"              },
    { LANGUAGE_CHINESE,                     "chinese"        },
    { LANGUAGE_GERMAN,                      "german"         },
    { LANGUAGE_JAPANESE,                    "japanese"       },
    { LANGUAGE_KOREAN,                      "korean"         },
    { LANGUAGE_ENGLISH_US,                  "posix"          },
    { LANGUAGE_CHINESE_TRADITIONAL,         "tchinese"       },
    { LANGUAGE_DONTKNOW,                    NULL             }  
};



static IsoLangOtherEntry const aImplPrivateUseEntries[] =
{
    { LANGUAGE_USER_PRIV_NOTRANSLATE,       "x-no-translate" }, 
    { LANGUAGE_USER_PRIV_DEFAULT,           "x-default"      },
    { LANGUAGE_USER_PRIV_COMMENT,           "x-comment"      },
    { LANGUAGE_USER_PRIV_JOKER,             "*"              }, 
    { LANGUAGE_DONTKNOW,                    NULL             }  
};




void MsLangId::Conversion::convertLanguageToLocaleImpl( LanguageType nLang,
        ::com::sun::star::lang::Locale & rLocale, bool bIgnoreOverride )
{
    const IsoLanguageScriptCountryEntry* pScriptEntryOverride = NULL;
    const IsoLanguageCountryEntry* pEntryOverride = NULL;

Label_Override_Lang_Locale:

    
    for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
            pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
    {
        if (pBcp47Entry->mnLang == nLang)
        {
            rLocale.Language = I18NLANGTAG_QLT;
            rLocale.Country  = OUString::createFromAscii( pBcp47Entry->maCountry);
            rLocale.Variant  = pBcp47Entry->getTagString();
            return;
        }
    }

    
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

    
    for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
            pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if (pEntry->mnLang == nLang)
        {
            if (bIgnoreOverride || !pEntry->mnOverride)
            {
                rLocale.Language = OUString::createFromAscii( pEntry->maLanguage );
                rLocale.Country  = OUString::createFromAscii( pEntry->maCountry );
                rLocale.Variant  = OUString();
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

    
    for (const IsoLangOtherEntry* pPrivateEntry = aImplPrivateUseEntries;
            pPrivateEntry->mnLang != LANGUAGE_DONTKNOW; ++pPrivateEntry)
    {
        if (pPrivateEntry->mnLang == nLang)
        {
            rLocale.Language = I18NLANGTAG_QLT;
            rLocale.Country  = OUString();
            rLocale.Variant  = OUString::createFromAscii( pPrivateEntry->mpLanguage );
            return;
        }
    }

    
}




com::sun::star::lang::Locale MsLangId::Conversion::getLocale( const IsoLanguageCountryEntry * pEntry )
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


com::sun::star::lang::Locale MsLangId::Conversion::getLocale( const IsoLanguageScriptCountryEntry * pEntry )
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


::com::sun::star::lang::Locale MsLangId::Conversion::lookupFallbackLocale(
        const ::com::sun::star::lang::Locale & rLocale )
{
    
    OUString aLowerLang = rLocale.Language.toAsciiLowerCase();
    
    OUString aUpperCountry = rLocale.Country.toAsciiUpperCase();
    sal_Int32 nCountryLen = aUpperCountry.getLength();

    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        
        
        for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
                pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
        {
            if (    rLocale.Variant.equalsIgnoreAsciiCase( pBcp47Entry->getTagString()) ||
                    rLocale.Variant.equalsIgnoreAsciiCaseAscii( pBcp47Entry->mpFallback))
                return pBcp47Entry->getLocale();
        }

        
        const IsoLanguageScriptCountryEntry* pFirstScript = NULL;
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return getLocale( pScriptEntry);    
                if (!pFirstScript)
                    pFirstScript = pScriptEntry;
            }
        }
        
        
        if (pFirstScript)
        {
            
            
            if (!aUpperCountry.isEmpty() && rLocale.Variant.getLength() > 11)
            {
                for (const IsoLanguageScriptCountryEntry* pScriptEntry = pFirstScript;
                        pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
                {
                    if (aUpperCountry.equalsAscii( pScriptEntry->maCountry) &&
                            pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
                        return getLocale( pScriptEntry);    
                }
            }
            return getLocale( pFirstScript);    
        }

        
        
        
        
        sal_Int32 nIndex = 0;
        aLowerLang = rLocale.Variant.getToken( 0, '-', nIndex).toAsciiLowerCase();
        
        
        if (aLowerLang.getLength() == 1)
            return aLastResortFallbackEntry.getLocale();
    }

    
    const IsoLanguageCountryEntry* pFirstLang = NULL;
    const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
    for ( ; pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if (aLowerLang.equalsAscii( pEntry->maLanguage))
        {
            if (*pEntry->maCountry)
            {
                if (nCountryLen && aUpperCountry.equalsAscii( pEntry->maCountry))
                    return getLocale( pEntry);  
            }
            else
            {
                switch (pEntry->mnLang)
                {
                    
                    case LANGUAGE_BASQUE:
                    case LANGUAGE_USER_ESPERANTO:
                    case LANGUAGE_USER_INTERLINGUA:
                    case LANGUAGE_USER_LOJBAN:
                    case LANGUAGE_KASHMIRI:
                    case LANGUAGE_USER_KEYID:
                        return getLocale( pEntry);  
                    default:
                        ;   
                }
            }
            if (!pFirstLang)
                pFirstLang = pEntry;
        }
    }

    
    if (!pFirstLang)
        return aLastResortFallbackEntry.getLocale();

    
    pEntry = pFirstLang;
    for ( ; pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if (aLowerLang.equalsAscii( pEntry->maLanguage))
        {
            if (*pEntry->maCountry)
                return getLocale( pEntry);  
        }
    }

    return aLastResortFallbackEntry.getLocale();
}




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



LanguageType MsLangId::Conversion::convertLocaleToLanguageImpl(
        const ::com::sun::star::lang::Locale& rLocale )
{
    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        
        if (rLocale.Variant.startsWithIgnoreAsciiCase( "x-") || (rLocale.Variant == "*"))
            return convertPrivateUseToLanguage( rLocale.Variant);

        
        for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
                pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
        {
            if (rLocale.Variant.equalsIgnoreAsciiCase( pBcp47Entry->getTagString()))
                return pBcp47Entry->mnLang;
        }

        
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
        
        OUString aLowerLang = rLocale.Language.toAsciiLowerCase();
        
        OUString aUpperCountry = rLocale.Country.toAsciiUpperCase();

        
        for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
                pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
        {
            if (aLowerLang.equalsAscii( pEntry->maLanguage) && aUpperCountry.equalsAscii( pEntry->maCountry))
                return getOverrideLang( pEntry->mnLang, pEntry->mnOverride);
        }
    }
    return LANGUAGE_DONTKNOW;
}



::com::sun::star::lang::Locale MsLangId::Conversion::getOverride( const ::com::sun::star::lang::Locale& rLocale )
{
    if (rLocale.Language == I18NLANGTAG_QLT)
    {
        
        if (rLocale.Variant.startsWithIgnoreAsciiCase( "x-") || (rLocale.Variant == "*"))
            return rLocale;     

        
        for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
                pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
        {
            if (rLocale.Variant.equalsIgnoreAsciiCase( pBcp47Entry->getTagString()))
                return rLocale; 
        }

        
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return getLocale( pScriptEntry);    
            }
        }
    }
    else
    {
        
        OUString aLowerLang = rLocale.Language.toAsciiLowerCase();
        
        OUString aUpperCountry = rLocale.Country.toAsciiUpperCase();

        
        for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
                pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
        {
            if (aLowerLang.equalsAscii( pEntry->maLanguage) && aUpperCountry.equalsAscii( pEntry->maCountry))
                return getLocale( pEntry);  
        }
    }
    return lang::Locale();
}



LanguageType MsLangId::Conversion::convertIsoNamesToLanguage( const OUString& rLang,
        const OUString& rCountry )
{
    
    OUString aLowerLang = rLang.toAsciiLowerCase();
    
    OUString aUpperCountry = rCountry.toAsciiUpperCase();

    
    const IsoLanguageCountryEntry* pFirstLang = NULL;
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

    
    if ( aLowerLang == "en" )
    {
        for (const IsoLangEngEntry* pEngEntry = aImplIsoLangEngEntries;
                pEngEntry->mnLang != LANGUAGE_DONTKNOW; ++pEngEntry)
        {
            if ( aUpperCountry.equalsAscii( pEngEntry->maCountry ) )
                return pEngEntry->mnLang;
        }
    }

    
    for (const IsoLangNoneStdEntry* pNoneStdEntry = aImplIsoNoneStdLangEntries;
            pNoneStdEntry->mnLang != LANGUAGE_DONTKNOW; ++pNoneStdEntry)
    {
        if ( aLowerLang.equalsAscii( pNoneStdEntry->maLanguage ) )
        {
            
            if ( aUpperCountry.equalsIgnoreAsciiCaseAscii( pNoneStdEntry->maCountry ) )
                return pNoneStdEntry->mnLang;
        }
    }
    for (const IsoLangNoneStdEntry* pNoneStdEntry2 = aImplIsoNoneStdLangEntries2;
            pNoneStdEntry2->mnLang != LANGUAGE_DONTKNOW; ++pNoneStdEntry2)
    {
        if ( aLowerLang.equalsAscii( pNoneStdEntry2->maLanguage ) )
        {
            
            if ( aUpperCountry.equalsIgnoreAsciiCaseAscii( pNoneStdEntry2->maCountry ) )
                return pNoneStdEntry2->mnLang;
        }
    }

    
    if ( pFirstLang )
        return pFirstLang->mnLang;

    
    
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

    
    LanguageType nLang = convertPrivateUseToLanguage( aLowerLang);
    if (nLang != LANGUAGE_DONTKNOW)
        return nLang;

    
    for (const IsoLangOtherEntry* pOtherEntry = aImplOtherEntries;
            pOtherEntry->mnLang != LANGUAGE_DONTKNOW; ++pOtherEntry)
    {
        if ( aLowerLang.equalsAscii( pOtherEntry->mpLanguage ) )
            return pOtherEntry->mnLang;
    }

    return LANGUAGE_DONTKNOW;
}




LanguageType MsLangId::Conversion::convertIsoNamesToLanguage( const OString& rLang,
        const OString& rCountry )
{
    OUString aLang = OStringToOUString( rLang, RTL_TEXTENCODING_ASCII_US);
    OUString aCountry = OStringToOUString( rCountry, RTL_TEXTENCODING_ASCII_US);
    return convertIsoNamesToLanguage( aLang, aCountry);
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
    
    { LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "bs", "BA", "cyrillic" },
    { LANGUAGE_USER_SERBIAN_LATIN_SERBIA,           "sr", "RS", "latin" },   
    { LANGUAGE_SERBIAN_LATIN_SAM,                   "sr", "CS", "latin" },   
    { LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO,       "sr", "ME", "latin" },   
    { LANGUAGE_SERBIAN_LATIN_LSO,                   "sr", "",   "latin" },
    { LANGUAGE_AZERI_CYRILLIC,                      "az", "AZ", "cyrillic" },
    { LANGUAGE_UZBEK_CYRILLIC,                      "uz", "UZ", "cyrillic" },
    { LANGUAGE_CATALAN_VALENCIAN,                   "ca", "ES", "valencia" },
    { LANGUAGE_DONTKNOW,                            "",   "",   ""   }       
};




LanguageType MsLangId::convertUnxByteStringToLanguage(
        const OString& rString )
{
    OString  aLang;
    OString  aCountry;
    OString  aAtString;

    sal_Int32  nLangSepPos    = rString.indexOf( (sal_Char)'_' );
    sal_Int32  nCountrySepPos = rString.indexOf( (sal_Char)'.' );
    sal_Int32  nAtPos         = rString.indexOf( (sal_Char)'@' );

    if (nCountrySepPos < 0)
        nCountrySepPos = nAtPos;
    if (nCountrySepPos < 0)
        nCountrySepPos = rString.getLength();

    if (nAtPos >= 0)
        aAtString = rString.copy( nAtPos+1 );

    if (((nLangSepPos >= 0) && (nLangSepPos > nCountrySepPos)) || (nLangSepPos < 0))
    {
        
        aLang    = rString.copy( 0, nCountrySepPos );
    }
    else if ( nLangSepPos >= 0 )
    {
        
        aLang    = rString.copy( 0, nLangSepPos );
        aCountry = rString.copy( nLangSepPos+1, nCountrySepPos - nLangSepPos - 1);
    }

    
    if (!aAtString.isEmpty())
    {
        
        OString aLowerLang = aLang.toAsciiLowerCase();
        
        OString aUpperCountry = aCountry.toAsciiUpperCase();
        for (const IsoLangGLIBCModifiersEntry* pGLIBCModifiersEntry = aImplIsoLangGLIBCModifiersEntries;
                pGLIBCModifiersEntry->mnLang != LANGUAGE_DONTKNOW; ++pGLIBCModifiersEntry)
        {                         
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
