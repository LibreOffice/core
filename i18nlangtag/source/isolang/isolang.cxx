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

// =======================================================================

struct IsoLanguageCountryEntry
{
    LanguageType  mnLang;
    sal_Char      maLanguage[4];
    sal_Char      maCountry[3];
    /** If TRUE, a higher (!) level override with the same mnLang (!) exists
        that should be used instead. */
    bool          mbOverrideExists;

    /** Obtain a language tag string with '-' separator. */
    OUString getTagString() const;

    /** Obtain a locale. */
    ::com::sun::star::lang::Locale getLocale() const;
};

struct IsoLanguageScriptCountryEntry
{
    LanguageType  mnLang;
    sal_Char      maLanguageScript[9];      ///< "ll-Ssss" or "lll-Ssss"
    sal_Char      maCountry[3];

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

// -----------------------------------------------------------------------

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

static IsoLanguageCountryEntry const aImplIsoLangEntries[] =
{
    // MS-LANGID codes               ISO639-1/2/3 ISO3166
    { LANGUAGE_ENGLISH,                     "en", ""  , false },
    { LANGUAGE_ENGLISH_US,                  "en", "US", false },
    { LANGUAGE_ENGLISH_UK,                  "en", "GB", false },
    { LANGUAGE_ENGLISH_AUS,                 "en", "AU", false },
    { LANGUAGE_ENGLISH_CAN,                 "en", "CA", false },
    { LANGUAGE_FRENCH,                      "fr", "FR", false },
    { LANGUAGE_GERMAN,                      "de", "DE", false },
    { LANGUAGE_ITALIAN,                     "it", "IT", false },
    { LANGUAGE_DUTCH,                       "nl", "NL", false },
    { LANGUAGE_SPANISH_MODERN,              "es", "ES", false },
    { LANGUAGE_SPANISH_DATED,               "es", "ES", false },
    { LANGUAGE_PORTUGUESE,                  "pt", "PT", false },
    { LANGUAGE_PORTUGUESE_BRAZILIAN,        "pt", "BR", false },
    { LANGUAGE_DANISH,                      "da", "DK", false },
    { LANGUAGE_GREEK,                       "el", "GR", false },
    { LANGUAGE_CHINESE_SIMPLIFIED,          "zh", "CN", false },
    { LANGUAGE_CHINESE_SIMPLIFIED_LEGACY,   "zh", "CN", false },
    { LANGUAGE_CHINESE_TRADITIONAL,         "zh", "TW", false },
    { LANGUAGE_CHINESE_TRADITIONAL_LEGACY,  "zh", "TW", false },
    { LANGUAGE_CHINESE_HONGKONG,            "zh", "HK", false },
    { LANGUAGE_CHINESE_SINGAPORE,           "zh", "SG", false },
    { LANGUAGE_CHINESE_MACAU,               "zh", "MO", false },
    { LANGUAGE_ENGLISH_HONG_KONG_SAR,       "en", "HK", false },
    { LANGUAGE_JAPANESE,                    "ja", "JP", false },
    { LANGUAGE_KOREAN,                      "ko", "KR", false },
    { LANGUAGE_KOREAN_JOHAB,                "ko", "KR", false },
    { LANGUAGE_USER_KOREAN_NORTH,           "ko", "KP", false },
    { LANGUAGE_SWEDISH,                     "sv", "SE", false },
    { LANGUAGE_SWEDISH_FINLAND,             "sv", "FI", false },
    { LANGUAGE_FINNISH,                     "fi", "FI", false },
    { LANGUAGE_RUSSIAN,                     "ru", "RU", false },
    { LANGUAGE_TATAR,                       "tt", "RU", false },
    { LANGUAGE_ENGLISH_NZ,                  "en", "NZ", false },
    { LANGUAGE_ENGLISH_EIRE,                "en", "IE", false },
    { LANGUAGE_DUTCH_BELGIAN,               "nl", "BE", false },
    { LANGUAGE_FRENCH_BELGIAN,              "fr", "BE", false },
    { LANGUAGE_FRENCH_CANADIAN,             "fr", "CA", false },
    { LANGUAGE_FRENCH_SWISS,                "fr", "CH", false },
    { LANGUAGE_GERMAN_SWISS,                "de", "CH", false },
    { LANGUAGE_GERMAN_AUSTRIAN,             "de", "AT", false },
    { LANGUAGE_ITALIAN_SWISS,               "it", "CH", false },
    { LANGUAGE_ALBANIAN,                    "sq", "AL", false },
    { LANGUAGE_ARABIC_SAUDI_ARABIA,         "ar", "SA", false },
    { LANGUAGE_ARABIC_EGYPT,                "ar", "EG", false },
    { LANGUAGE_ARABIC_UAE,                  "ar", "AE", false },
    { LANGUAGE_ARABIC_IRAQ,                 "ar", "IQ", false },
    { LANGUAGE_ARABIC_LIBYA,                "ar", "LY", false },
    { LANGUAGE_ARABIC_ALGERIA,              "ar", "DZ", false },
    { LANGUAGE_ARABIC_MOROCCO,              "ar", "MA", false },
    { LANGUAGE_ARABIC_TUNISIA,              "ar", "TN", false },
    { LANGUAGE_ARABIC_OMAN,                 "ar", "OM", false },
    { LANGUAGE_ARABIC_YEMEN,                "ar", "YE", false },
    { LANGUAGE_ARABIC_SYRIA,                "ar", "SY", false },
    { LANGUAGE_ARABIC_JORDAN,               "ar", "JO", false },
    { LANGUAGE_ARABIC_LEBANON,              "ar", "LB", false },
    { LANGUAGE_ARABIC_KUWAIT,               "ar", "KW", false },
    { LANGUAGE_ARABIC_BAHRAIN,              "ar", "BH", false },
    { LANGUAGE_ARABIC_QATAR,                "ar", "QA", false },
    { LANGUAGE_USER_ARABIC_CHAD,            "ar", "TD", false },
    { LANGUAGE_USER_ARABIC_COMOROS,         "ar", "KM", false },
    { LANGUAGE_USER_ARABIC_DJIBOUTI,        "ar", "DJ", false },
    { LANGUAGE_USER_ARABIC_ERITREA,         "ar", "ER", false },
    { LANGUAGE_USER_ARABIC_ISRAEL,          "ar", "IL", false },
    { LANGUAGE_USER_ARABIC_MAURITANIA,      "ar", "MR", false },
    { LANGUAGE_USER_ARABIC_PALESTINE,       "ar", "PS", false },
    { LANGUAGE_USER_ARABIC_SOMALIA,         "ar", "SO", false },
    { LANGUAGE_USER_ARABIC_SUDAN,           "ar", "SD", false },
    { LANGUAGE_ARABIC_PRIMARY_ONLY,         "ar", ""  , false },
    { LANGUAGE_BASQUE,                      "eu", ""  , false },
    { LANGUAGE_BULGARIAN,                   "bg", "BG", false },
    { LANGUAGE_CZECH,                       "cs", "CZ", false },
    { LANGUAGE_CZECH,                       "cz", ""  , true  },
    { LANGUAGE_ENGLISH_JAMAICA,             "en", "JM", false },
    { LANGUAGE_ENGLISH_CARRIBEAN,           "en", "BS", false },   // not 100%, because AG is Bahamas
    { LANGUAGE_ENGLISH_BELIZE,              "en", "BZ", false },
    { LANGUAGE_ENGLISH_TRINIDAD,            "en", "TT", false },
    { LANGUAGE_ENGLISH_ZIMBABWE,            "en", "ZW", false },
    { LANGUAGE_ENGLISH_INDONESIA,           "en", "ID", false },
    { LANGUAGE_ESTONIAN,                    "et", "EE", false },
    { LANGUAGE_FAEROESE,                    "fo", "FO", false },
    { LANGUAGE_FARSI,                       "fa", "IR", false },
    { LANGUAGE_FRENCH_LUXEMBOURG,           "fr", "LU", false },
    { LANGUAGE_FRENCH_MONACO,               "fr", "MC", false },
    { LANGUAGE_GERMAN_LUXEMBOURG,           "de", "LU", false },
    { LANGUAGE_GERMAN_LIECHTENSTEIN,        "de", "LI", false },
    { LANGUAGE_HEBREW,                      "he", "IL", false },   // new: old was "iw"
    { LANGUAGE_HEBREW,                      "iw", "IL", true  },   // old: new is "he"
    { LANGUAGE_HUNGARIAN,                   "hu", "HU", false },
    { LANGUAGE_ICELANDIC,                   "is", "IS", false },
    { LANGUAGE_INDONESIAN,                  "id", "ID", false },   // new: old was "in"
    { LANGUAGE_INDONESIAN,                  "in", "ID", true  },   // old: new is "id"
    { LANGUAGE_NORWEGIAN,                   "no", "NO", false },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "nb", "NO", false },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "nn", "NO", false },
    { LANGUAGE_POLISH,                      "pl", "PL", false },
    { LANGUAGE_RHAETO_ROMAN,                "rm", "CH", false },
    { LANGUAGE_ROMANIAN,                    "ro", "RO", false },
    { LANGUAGE_ROMANIAN_MOLDOVA,            "ro", "MD", false },
    { LANGUAGE_SLOVAK,                      "sk", "SK", false },
    { LANGUAGE_SLOVENIAN,                   "sl", "SI", false },
    { LANGUAGE_SPANISH_MEXICAN,             "es", "MX", false },
    { LANGUAGE_SPANISH_GUATEMALA,           "es", "GT", false },
    { LANGUAGE_SPANISH_COSTARICA,           "es", "CR", false },
    { LANGUAGE_SPANISH_PANAMA,              "es", "PA", false },
    { LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,  "es", "DO", false },
    { LANGUAGE_SPANISH_VENEZUELA,           "es", "VE", false },
    { LANGUAGE_SPANISH_COLOMBIA,            "es", "CO", false },
    { LANGUAGE_SPANISH_PERU,                "es", "PE", false },
    { LANGUAGE_SPANISH_ARGENTINA,           "es", "AR", false },
    { LANGUAGE_SPANISH_ECUADOR,             "es", "EC", false },
    { LANGUAGE_SPANISH_CHILE,               "es", "CL", false },
    { LANGUAGE_SPANISH_URUGUAY,             "es", "UY", false },
    { LANGUAGE_SPANISH_PARAGUAY,            "es", "PY", false },
    { LANGUAGE_SPANISH_BOLIVIA,             "es", "BO", false },
    { LANGUAGE_SPANISH_EL_SALVADOR,         "es", "SV", false },
    { LANGUAGE_SPANISH_HONDURAS,            "es", "HN", false },
    { LANGUAGE_SPANISH_NICARAGUA,           "es", "NI", false },
    { LANGUAGE_SPANISH_PUERTO_RICO,         "es", "PR", false },
    { LANGUAGE_SPANISH_UNITED_STATES,       "es", "US", false },
    { LANGUAGE_SPANISH_LATIN_AMERICA,       "es", ""  , false },
    { LANGUAGE_TURKISH,                     "tr", "TR", false },
    { LANGUAGE_UKRAINIAN,                   "uk", "UA", false },
    { LANGUAGE_VIETNAMESE,                  "vi", "VN", false },
    { LANGUAGE_LATVIAN,                     "lv", "LV", false },
    { LANGUAGE_MACEDONIAN,                  "mk", "MK", false },
    { LANGUAGE_MALAY_MALAYSIA,              "ms", "MY", false },
    { LANGUAGE_MALAY_BRUNEI_DARUSSALAM,     "ms", "BN", false },
    { LANGUAGE_ENGLISH_MALAYSIA,            "en", "MY", false },
    { LANGUAGE_THAI,                        "th", "TH", false },
    { LANGUAGE_LITHUANIAN,                  "lt", "LT", false },
    { LANGUAGE_LITHUANIAN_CLASSIC,          "lt", "LT", false },
    { LANGUAGE_CROATIAN,                    "hr", "HR", false },   // Croatian in Croatia
    { LANGUAGE_CROATIAN_BOSNIA_HERZEGOVINA, "hr", "BA", false },
    { LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA,        "bs", "BA", false },
    { LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA,        "sr", "RS", false },   // Serbian Cyrillic in Serbia
    { LANGUAGE_SERBIAN_CYRILLIC,                    "sr", "CS", false },   // Serbian Cyrillic in Serbia and Montenegro
    { LANGUAGE_SERBIAN_CYRILLIC,                    "sr", "YU", true  },   // legacy Serbian Cyrillic in Serbia and Montenegro (former Yugoslavia); kludge, sr_CS not supported by ICU 2.6 (3.4 does)
    { LANGUAGE_USER_SERBIAN_CYRILLIC_MONTENEGRO,    "sr", "ME", false },
    { LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "sr", "BA", false },
    { LANGUAGE_USER_SERBIAN_LATIN_SERBIA,           "sh", "RS", true  },   // legacy kludge, is sr-Latn-RS now
    { LANGUAGE_SERBIAN_LATIN,                       "sh", "CS", true  },   // legacy kludge, is sr-Latn-CS now
    { LANGUAGE_SERBIAN_LATIN,                       "sh", "YU", true  },   // legacy kludge, is sr-Latn-YU now
    { LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO,       "sh", "ME", true  },   // legacy kludge, is sr-Latn-ME now
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,    "sh", "BA", true  },   // legacy kludge, is sr-Latn-BA now
    { LANGUAGE_SERBIAN_LATIN_NEUTRAL,               "sh", ""  , true  },   // legacy kludge, is sr-Latn now
    { LANGUAGE_ARMENIAN,                    "hy", "AM", false },
    { LANGUAGE_AZERI_LATIN,                 "az", "AZ", false },    // macrolanguage code
    { LANGUAGE_UZBEK_LATIN,                 "uz", "UZ", false },    // macrolanguage code
    { LANGUAGE_BENGALI_BANGLADESH,          "bn", "BD", false },
    { LANGUAGE_BENGALI,                     "bn", "IN", false },
    { LANGUAGE_BURMESE,                     "my", "MM", false },
    { LANGUAGE_KAZAKH,                      "kk", "KZ", false },
    { LANGUAGE_ENGLISH_INDIA,               "en", "IN", false },
    { LANGUAGE_URDU_INDIA,                  "ur", "IN", false },
    { LANGUAGE_URDU_PAKISTAN,               "ur", "PK", false },
    { LANGUAGE_HINDI,                       "hi", "IN", false },
    { LANGUAGE_GUJARATI,                    "gu", "IN", false },
    { LANGUAGE_KANNADA,                     "kn", "IN", false },
    { LANGUAGE_ASSAMESE,                    "as", "IN", false },
    { LANGUAGE_KASHMIRI_INDIA,              "ks", "IN", false },
    { LANGUAGE_KASHMIRI,                    "ks", ""  , false },   // Kashmiri in "Jammu and Kashmir" ... no ISO3166 code for that
    { LANGUAGE_MALAYALAM,                   "ml", "IN", false },
    { LANGUAGE_MANIPURI,                   "mni", "IN", false },
    { LANGUAGE_MARATHI,                     "mr", "IN", false },
    { LANGUAGE_KONKANI,                    "kok", "IN", false },
    { LANGUAGE_NEPALI,                      "ne", "NP", false },
    { LANGUAGE_NEPALI_INDIA,                "ne", "IN", false },
    { LANGUAGE_ORIYA,                       "or", "IN", false },
    { LANGUAGE_PUNJABI,                     "pa", "IN", false },
    { LANGUAGE_SANSKRIT,                    "sa", "IN", false },
    { LANGUAGE_SINDHI,                      "sd", "IN", false },
    { LANGUAGE_TAMIL,                       "ta", "IN", false },
    { LANGUAGE_TELUGU,                      "te", "IN", false },
    { LANGUAGE_PUNJABI_PAKISTAN,           "lah", "PK", false },   // preferring "lah" over "pa" for Western Punjabi, see http://www.ethnologue.com/show_language.asp?code=PNB
    { LANGUAGE_PUNJABI_PAKISTAN,            "pa", "PK", false },
    { LANGUAGE_SINDHI_PAKISTAN,             "sd", "PK", false },
    { LANGUAGE_BELARUSIAN,                  "be", "BY", false },
    { LANGUAGE_CATALAN,                     "ca", "ES", false },   // Spain (default)
    { LANGUAGE_CATALAN,                     "ca", "AD", false },   // Andorra
    //LANGUAGE_CATALAN_VALENCIAN ca-ES-valencia Bcp47CountryEntry takes precedence
    { LANGUAGE_CATALAN_VALENCIAN,           "ca", "XV", true  },   // XV: ISO 3166 user-assigned; old workaround for UI localization only, in case it escaped to document content
    { LANGUAGE_CATALAN_VALENCIAN,          "qcv", "ES", true  },   // qcv: ISO 639-3 reserved-for-local-use; old UI localization quirk only, in case it escaped to document content
    { LANGUAGE_FRENCH_CAMEROON,             "fr", "CM", false },
    { LANGUAGE_FRENCH_COTE_D_IVOIRE,        "fr", "CI", false },
    { LANGUAGE_FRENCH_MALI,                 "fr", "ML", false },
    { LANGUAGE_FRENCH_SENEGAL,              "fr", "SN", false },
    { LANGUAGE_FRENCH_ZAIRE,                "fr", "CD", false },   // Democratic Republic Of Congo
    { LANGUAGE_FRENCH_MOROCCO,              "fr", "MA", false },
    { LANGUAGE_FRENCH_REUNION,              "fr", "RE", false },
    { LANGUAGE_FRENCH,                      "fr", ""  , false },   // needed as a catcher before other "fr" entries!
    { LANGUAGE_FRENCH_NORTH_AFRICA,         "fr", ""  , false },
    { LANGUAGE_FRENCH_WEST_INDIES,          "fr", ""  , false },   // unknown ISO country code
    { LANGUAGE_FRISIAN_NETHERLANDS,         "fy", "NL", false },
    { LANGUAGE_GAELIC_IRELAND,              "ga", "IE", false },
    { LANGUAGE_GAELIC_SCOTLAND,             "gd", "GB", false },
    { LANGUAGE_GALICIAN,                    "gl", "ES", false },
    { LANGUAGE_GEORGIAN,                    "ka", "GE", false },
    { LANGUAGE_KHMER,                       "km", "KH", false },
    { LANGUAGE_KIRGHIZ,                     "ky", "KG", false },
    { LANGUAGE_LAO,                         "lo", "LA", false },
    { LANGUAGE_MALTESE,                     "mt", "MT", false },
    { LANGUAGE_MONGOLIAN,                   "mn", "MN", true  },   // Cyrillic script
    { LANGUAGE_RUSSIAN_MOLDOVA,             "mo", "MD", false },
    { LANGUAGE_SWAHILI,                     "sw", "KE", false },
    { LANGUAGE_USER_SWAHILI_TANZANIA,       "sw", "TZ", false },
    { LANGUAGE_TAJIK,                       "tg", "TJ", false },
    { LANGUAGE_TIBETAN,                     "bo", "CN", false },   // CN politically correct?
    { LANGUAGE_USER_TIBETAN_INDIA,          "bo", "IN", false },
    { LANGUAGE_DZONGKHA,                    "dz", "BT", false },
    { LANGUAGE_TURKMEN,                     "tk", "TM", false },
    { LANGUAGE_WELSH,                       "cy", "GB", false },
    { LANGUAGE_SESOTHO,                     "st", "ZA", false },
    { LANGUAGE_SEPEDI,                     "nso", "ZA", false },
    { LANGUAGE_SEPEDI,                      "ns", "ZA", true  },   // fake "ns" for compatibility with existing OOo1.1.x localization to be able to read those documents
    { LANGUAGE_TSONGA,                      "ts", "ZA", false },
    { LANGUAGE_TSWANA,                      "tn", "ZA", false },
    { LANGUAGE_ENGLISH_SAFRICA,             "en", "ZA", false },
    { LANGUAGE_AFRIKAANS,                   "af", "ZA", false },
    { LANGUAGE_VENDA,                       "ve", "ZA", false },   // default 639-1
    { LANGUAGE_VENDA,                      "ven", "ZA", true  },   // 639-2 may have been used temporarily since 2004-07-23
    { LANGUAGE_XHOSA,                       "xh", "ZA", false },
    { LANGUAGE_ZULU,                        "zu", "ZA", false },
    { LANGUAGE_QUECHUA_ECUADOR,             "qu", "EC", false },
    { LANGUAGE_QUECHUA_PERU,                "qu", "PE", false },
    { LANGUAGE_QUECHUA_BOLIVIA,             "qu", "BO", false },   // macro: quh-BO, qul-BO
    { LANGUAGE_PASHTO,                      "ps", "AF", false },
    { LANGUAGE_OROMO,                       "om", "ET", false },
    { LANGUAGE_DHIVEHI,                     "dv", "MV", false },
    { LANGUAGE_UIGHUR_CHINA,                "ug", "CN", false },
    { LANGUAGE_TIGRIGNA_ETHIOPIA,           "ti", "ET", false },
    { LANGUAGE_TIGRIGNA_ERITREA,            "ti", "ER", false },
    { LANGUAGE_AMHARIC_ETHIOPIA,            "am", "ET", false },
    { LANGUAGE_GUARANI_PARAGUAY,           "gug", "PY", false },
    { LANGUAGE_HAWAIIAN_UNITED_STATES,     "haw", "US", false },
    { LANGUAGE_EDO,                        "bin", "NG", false },
    { LANGUAGE_FULFULDE_NIGERIA,            "ff", "NG", false },
    { LANGUAGE_HAUSA_NIGERIA,               "ha", "NG", false },
    { LANGUAGE_USER_HAUSA_GHANA,            "ha", "GH", false },
    { LANGUAGE_IGBO_NIGERIA,                "ig", "NG", false },
    { LANGUAGE_KANURI_NIGERIA,              "kr", "NG", false },
    { LANGUAGE_YORUBA,                      "yo", "NG", false },
    { LANGUAGE_SOMALI,                      "so", "SO", false },
    { LANGUAGE_PAPIAMENTU,                 "pap", "AN", false },
    { LANGUAGE_USER_PAPIAMENTU_ARUBA,      "pap", "AW", false },
    { LANGUAGE_USER_PAPIAMENTU_CURACAO,    "pap", "CW", false },
    { LANGUAGE_USER_PAPIAMENTU_BONAIRE,    "pap", "BQ", false },
    { LANGUAGE_ENGLISH_SINGAPORE,           "en", "SG", false },
    { LANGUAGE_USER_YIDDISH_US,             "yi", "US", false },
    { LANGUAGE_YIDDISH,                     "yi", "IL", false },   // new: old was "ji"
    { LANGUAGE_YIDDISH,                     "ji", "IL", true  },   // old: new is "yi"
    { LANGUAGE_SYRIAC,                     "syr", "TR", false },   // "TR" according to http://www.ethnologue.com/show_language.asp?code=SYC
    { LANGUAGE_SINHALESE_SRI_LANKA,         "si", "LK", false },
    { LANGUAGE_CHEROKEE_UNITED_STATES,     "chr", "US", false },
    { LANGUAGE_INUKTITUT_LATIN_CANADA,      "iu", "CA", false },
//  { LANGUAGE_INUKTITUT_SYLLABICS_CANADA,  "iu", "CA", false },   // script codes not supported yet
    { LANGUAGE_SAMI_NORTHERN_NORWAY,        "se", "NO", false },
    { LANGUAGE_SAMI_INARI,                 "smn", "FI", false },
    { LANGUAGE_SAMI_LULE_NORWAY,           "smj", "NO", false },
    { LANGUAGE_SAMI_LULE_SWEDEN,           "smj", "SE", false },
    { LANGUAGE_SAMI_NORTHERN_FINLAND,       "se", "FI", false },
    { LANGUAGE_SAMI_NORTHERN_SWEDEN,        "se", "SE", false },
    { LANGUAGE_SAMI_SKOLT,                 "sms", "FI", false },
    { LANGUAGE_SAMI_SOUTHERN_NORWAY,       "sma", "NO", false },
    { LANGUAGE_SAMI_SOUTHERN_SWEDEN,       "sma", "SE", false },
    { LANGUAGE_USER_SAMI_KILDIN_RUSSIA,    "sjd", "RU", false },
    { LANGUAGE_MAPUDUNGUN_CHILE,           "arn", "CL", false },
    { LANGUAGE_CORSICAN_FRANCE,             "co", "FR", false },
    { LANGUAGE_ALSATIAN_FRANCE,            "gsw", "FR", false },   // in fact 'gsw' is Schwyzerduetsch (Swiss German), which is a dialect of Alemannic German, as is Alsatian. They aren't distinct languages and share this code.
    { LANGUAGE_YAKUT_RUSSIA,               "sah", "RU", false },
    { LANGUAGE_MOHAWK_CANADA,              "moh", "CA", false },
    { LANGUAGE_BASHKIR_RUSSIA,              "ba", "RU", false },
    { LANGUAGE_KICHE_GUATEMALA,            "qut", "GT", false },
    { LANGUAGE_DARI_AFGHANISTAN,           "gbz", "AF", false },
    { LANGUAGE_WOLOF_SENEGAL,               "wo", "SN", false },
    { LANGUAGE_FILIPINO,                   "fil", "PH", false },
    { LANGUAGE_USER_TAGALOG,                "tl", "PH", false },
    { LANGUAGE_ENGLISH_PHILIPPINES,         "en", "PH", false },
//  { LANGUAGE_IBIBIO_NIGERIA,             "nic", "NG", false },   // ISO "nic" is only a collective language code
    { LANGUAGE_YI,                          "ii", "CN", false },
    { LANGUAGE_TAMAZIGHT_LATIN,            "kab", "DZ", false },   // In practice Kabyle is the language used for this
    { LANGUAGE_OBSOLETE_USER_KABYLE,       "kab", "DZ", false },
    { LANGUAGE_TAMAZIGHT_LATIN,            "ber", "DZ", false },   // In practice Algeria has standardized on Kabyle as the member of the "ber" collective which gets used there.
    { LANGUAGE_TAMAZIGHT_TIFINAGH,         "ber", "MA", false },   // Morocco is officially using Tifinagh for its Berber languages so store it to distinguish explicitly from LANGUAGE_TAMAZIGHT_LATIN, even though as a collective language its not of much use
//  { LANGUAGE_TAMAZIGHT_ARABIC,           "ber", ""  , false },   // ISO "ber" only collective!
    { LANGUAGE_LATIN,                       "la", "VA", false },
    { LANGUAGE_OBSOLETE_USER_LATIN,         "la", "VA", false },
    { LANGUAGE_USER_ESPERANTO,              "eo", ""  , false },
    { LANGUAGE_USER_INTERLINGUA,            "ia", ""  , false },
    { LANGUAGE_MAORI_NEW_ZEALAND,           "mi", "NZ", false },
    { LANGUAGE_OBSOLETE_USER_MAORI,         "mi", "NZ", false },
    { LANGUAGE_KINYARWANDA_RWANDA,          "rw", "RW", false },
    { LANGUAGE_OBSOLETE_USER_KINYARWANDA,   "rw", "RW", false },
    { LANGUAGE_UPPER_SORBIAN_GERMANY,      "hsb", "DE", false },   // MS maps this to 'wen-DE', which is nonsense. 'wen' is a collective language code, 'WEN' is a SIL code, see http://www.ethnologue.com/14/show_iso639.asp?code=wen and http://www.ethnologue.com/14/show_language.asp?code=WEN
    { LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN,"hsb", "DE", false },
    { LANGUAGE_LOWER_SORBIAN_GERMANY,      "dsb", "DE", false },   // MS maps this to 'wee-DE', which is nonsense. 'WEE' is a SIL code, see http://www.ethnologue.com/14/show_language.asp?code=WEE
    { LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN,"dsb", "DE", false },
    { LANGUAGE_OCCITAN_FRANCE,              "oc", "FR", false },
    { LANGUAGE_OBSOLETE_USER_OCCITAN,       "oc", "FR", false },
    { LANGUAGE_USER_KURDISH_TURKEY,         "ku", "TR", false },
    { LANGUAGE_USER_KURDISH_SYRIA,          "ku", "SY", false },
    { LANGUAGE_USER_KURDISH_IRAQ,           "ku", "IQ", false },
    { LANGUAGE_USER_KURDISH_IRAN,           "ku", "IR", false },
    { LANGUAGE_USER_SARDINIAN,              "sc", "IT", false },   // macrolanguage code
    { LANGUAGE_USER_SARDINIAN_CAMPIDANESE, "sro", "IT", false },
    { LANGUAGE_USER_SARDINIAN_GALLURESE,   "sdn", "IT", false },
    { LANGUAGE_USER_SARDINIAN_LOGUDORESE,  "src", "IT", false },
    { LANGUAGE_USER_SARDINIAN_SASSARESE,   "sdc", "IT", false },
    { LANGUAGE_BRETON_FRANCE,               "br", "FR", false },
    { LANGUAGE_OBSOLETE_USER_BRETON,        "br", "FR", false },
    { LANGUAGE_KALAALLISUT_GREENLAND,       "kl", "GL", false },
    { LANGUAGE_OBSOLETE_USER_KALAALLISUT,   "kl", "GL", false },
    { LANGUAGE_USER_SWAZI,                  "ss", "ZA", false },
    { LANGUAGE_USER_NDEBELE_SOUTH,          "nr", "ZA", false },
    { LANGUAGE_USER_TSWANA_BOTSWANA,        "tn", "BW", false },
    { LANGUAGE_USER_MOORE,                 "mos", "BF", false },
    { LANGUAGE_USER_BAMBARA,                "bm", "ML", false },
    { LANGUAGE_USER_AKAN,                   "ak", "GH", false },
    { LANGUAGE_LUXEMBOURGISH_LUXEMBOURG,    "lb", "LU", false },
    { LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH, "lb", "LU", false },
    { LANGUAGE_USER_FRIULIAN,              "fur", "IT", false },
    { LANGUAGE_USER_FIJIAN,                 "fj", "FJ", false },
    { LANGUAGE_USER_AFRIKAANS_NAMIBIA,      "af", "NA", false },
    { LANGUAGE_USER_ENGLISH_NAMIBIA,        "en", "NA", false },
    { LANGUAGE_USER_WALLOON,                "wa", "BE", false },
    { LANGUAGE_USER_COPTIC,                "cop", "EG", false },
    { LANGUAGE_USER_GASCON,                "gsc", "FR", false },
    { LANGUAGE_USER_GERMAN_BELGIUM,         "de", "BE", false },
    { LANGUAGE_USER_CHUVASH,                "cv", "RU", false },
    { LANGUAGE_USER_EWE_GHANA,              "ee", "GH", false },
    { LANGUAGE_USER_ENGLISH_GHANA,          "en", "GH", false },
    { LANGUAGE_USER_SANGO,                  "sg", "CF", false },
    { LANGUAGE_USER_GANDA,                  "lg", "UG", false },
    { LANGUAGE_USER_LINGALA_DRCONGO,        "ln", "CD", false },
    { LANGUAGE_USER_LOW_GERMAN,            "nds", "DE", false },
    { LANGUAGE_USER_HILIGAYNON,            "hil", "PH", false },
    { LANGUAGE_USER_ENGLISH_MALAWI,         "en", "MW", false },   /* en default for MW */
    { LANGUAGE_USER_NYANJA,                 "ny", "MW", false },
    { LANGUAGE_USER_KASHUBIAN,             "csb", "PL", false },
    { LANGUAGE_USER_SPANISH_CUBA,           "es", "CU", false },
    { LANGUAGE_USER_QUECHUA_NORTH_BOLIVIA, "qul", "BO", false },
    { LANGUAGE_USER_QUECHUA_SOUTH_BOLIVIA, "quh", "BO", false },
    { LANGUAGE_USER_BODO_INDIA,            "brx", "IN", false },
    { LANGUAGE_USER_DOGRI_INDIA,           "dgo", "IN", false },
    { LANGUAGE_USER_MAITHILI_INDIA,        "mai", "IN", false },
    { LANGUAGE_USER_SANTALI_INDIA,         "sat", "IN", false },
    { LANGUAGE_USER_TETUN,                 "tet", "ID", false },
    { LANGUAGE_USER_TETUN_TIMOR_LESTE,     "tet", "TL", false },
    { LANGUAGE_USER_TOK_PISIN,             "tpi", "PG", false },
    { LANGUAGE_USER_SHUSWAP,               "shs", "CA", false },
    { LANGUAGE_USER_ANCIENT_GREEK,         "grc", "GR", false },
    { LANGUAGE_USER_ASTURIAN,              "ast", "ES", false },
    { LANGUAGE_USER_LATGALIAN,             "ltg", "LV", false },
    { LANGUAGE_USER_MAORE,                 "swb", "YT", false },
    { LANGUAGE_USER_BUSHI,                 "buc", "YT", false },
    { LANGUAGE_USER_TAHITIAN,               "ty", "PF", false },
    { LANGUAGE_USER_MALAGASY_PLATEAU,      "plt", "MG", false },
    { LANGUAGE_USER_MALAGASY_PLATEAU,       "mg", "MG", false },
    { LANGUAGE_USER_BAFIA,                 "ksf", "CM", false },
    { LANGUAGE_USER_GIKUYU,                 "ki", "KE", false },
    { LANGUAGE_USER_RUSYN_UKRAINE,         "rue", "UA", false },
    { LANGUAGE_USER_RUSYN_SLOVAKIA,        "rue", "SK", false },
    { LANGUAGE_USER_LIMBU,                 "lif", "NP", false },
    { LANGUAGE_USER_LOJBAN,                "jbo", ""  , false },
    { LANGUAGE_USER_HAITIAN,                "ht", "HT", false },
    { LANGUAGE_FRENCH_HAITI,                "fr", "HT", false },
    { LANGUAGE_USER_BEEMBE,                "beq", "CG", false },
    { LANGUAGE_USER_BEKWEL,                "bkw", "CG", false },
    { LANGUAGE_USER_KITUBA,                "mkw", "CG", false },
    { LANGUAGE_USER_LARI,                  "ldi", "CG", false },
    { LANGUAGE_USER_MBOCHI,                "mdw", "CG", false },
    { LANGUAGE_USER_TEKE_EBOO,             "ebo", "CG", false },
    { LANGUAGE_USER_TEKE_IBALI,            "tek", "CG", false },
    { LANGUAGE_USER_TEKE_TYEE,             "tyx", "CG", false },
    { LANGUAGE_USER_VILI,                  "vif", "CG", false },
    { LANGUAGE_USER_PORTUGUESE_ANGOLA,      "pt", "AO", false },
    { LANGUAGE_USER_MANX,                   "gv", "GB", false },
    { LANGUAGE_USER_ARAGONESE,              "an", "ES", false },
    { LANGUAGE_USER_KEYID,                 "qtz", ""  , false },   // key id pseudolanguage used for UI testing
    { LANGUAGE_USER_PALI_LATIN,            "pli", ""  , true  },   // Pali with Latin script, ISO 639-3 (sigh..) back-compat, Latin is not a default script though..
    { LANGUAGE_USER_KYRGYZ_CHINA,           "ky", "CN", false },
    { LANGUAGE_USER_KOMI_ZYRIAN,           "kpv", "RU", false },
    { LANGUAGE_USER_KOMI_PERMYAK,          "koi", "RU", false },
    { LANGUAGE_USER_PITJANTJATJARA,        "pjt", "AU", false },
    { LANGUAGE_USER_ERZYA,                 "myv", "RU", false },
    { LANGUAGE_USER_MARI_MEADOW,           "mhr", "RU", false },
    { LANGUAGE_USER_KHANTY,                "kca", "RU", false },
    { LANGUAGE_USER_LIVONIAN,              "liv", "RU", false },
    { LANGUAGE_USER_MOKSHA,                "mdf", "RU", false },
    { LANGUAGE_USER_MARI_HILL,             "mrj", "RU", false },
    { LANGUAGE_USER_NGANASAN,              "nio", "RU", false },
    { LANGUAGE_USER_OLONETS,               "olo", "RU", false },
    { LANGUAGE_USER_VEPS,                  "vep", "RU", false },
    { LANGUAGE_USER_VORO,                  "vro", "EE", false },
    { LANGUAGE_USER_NENETS,                "yrk", "RU", false },
    { LANGUAGE_USER_AKA,                   "axk", "CF", false },
    { LANGUAGE_USER_AKA_CONGO,             "axk", "CG", false },
    { LANGUAGE_USER_DIBOLE,                "bvx", "CG", false },
    { LANGUAGE_USER_DOONDO,                "dde", "CG", false },
    { LANGUAGE_USER_KAAMBA,                "xku", "CG", false },
    { LANGUAGE_USER_KOONGO,                "kng", "CD", false },
    { LANGUAGE_USER_KOONGO_CONGO,          "kng", "CG", false },
    { LANGUAGE_USER_KUNYI,                 "njx", "CG", false },
    { LANGUAGE_USER_NGUNGWEL,              "ngz", "CG", false },
    { LANGUAGE_USER_NJYEM,                 "njy", "CM", false },
    { LANGUAGE_USER_NJYEM_CONGO,           "njy", "CG", false },
    { LANGUAGE_USER_PUNU,                  "puu", "GA", false },
    { LANGUAGE_USER_PUNU_CONGO,            "puu", "CG", false },
    { LANGUAGE_USER_SUUNDI,                "sdj", "CG", false },
    { LANGUAGE_USER_TEKE_KUKUYA,           "kkw", "CG", false },
    { LANGUAGE_USER_TSAANGI,               "tsa", "CG", false },
    { LANGUAGE_USER_YAKA,                  "iyx", "CG", false },
    { LANGUAGE_USER_YOMBE,                 "yom", "CD", false },
    { LANGUAGE_USER_YOMBE_CONGO,           "yom", "CG", false },
    { LANGUAGE_USER_SIDAMA,                "sid", "ET", false },
    { LANGUAGE_USER_NKO,                   "nqo", "GN", false },
    { LANGUAGE_USER_UDMURT,                "udm", "RU", false },
    { LANGUAGE_USER_CORNISH,               "kw",  "UK", false },
    { LANGUAGE_USER_SAMI_PITE_SWEDEN,      "sje", "SE", false },
    { LANGUAGE_USER_NGAEBERE,              "gym", "PA", false },
    { LANGUAGE_MULTIPLE,                   "mul", ""  , false },   // multiple languages, many languages are used
    { LANGUAGE_UNDETERMINED,               "und", ""  , false },   // undetermined language, language cannot be identified
    { LANGUAGE_NONE,                       "zxx", ""  , false },   // added to ISO 639-2 on 2006-01-11: Used to declare the absence of linguistic information
    { LANGUAGE_DONTKNOW,                    "",   ""  , false }    // marks end of table
};

static IsoLanguageScriptCountryEntry const aImplIsoLangScriptEntries[] =
{
    // MS-LangID                          ISO639-ISO15924, ISO3166
    { LANGUAGE_USER_SERBIAN_LATIN_SERBIA,       "sr-Latn", "RS" },
    { LANGUAGE_SERBIAN_LATIN,                   "sr-Latn", "CS" },  // Serbian Latin in Serbia and Montenegro; note that not all applications may know about the 'CS' reusage mess, see https://en.wikipedia.org/wiki/ISO_3166-2:CS
    { LANGUAGE_SERBIAN_LATIN,                   "sr-Latn", "YU" },  // legacy Serbian Latin in Yugoslavia
    { LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO,   "sr-Latn", "ME" },
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,"sr-Latn", "BA" },
    { LANGUAGE_SERBIAN_LATIN_NEUTRAL,           "sr-Latn", ""   },
    { LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "bs-Cyrl", "BA" },
    { LANGUAGE_AZERI_CYRILLIC,                  "az-Cyrl", "AZ" },  // macrolanguage code
    { LANGUAGE_UZBEK_CYRILLIC,                  "uz-Cyrl", "UZ" },  // macrolanguage code
    { LANGUAGE_MONGOLIAN,                       "mn-Cyrl", "MN" },  // macrolanguage code
    { LANGUAGE_MONGOLIAN_MONGOLIAN,             "mn-Mong", "MN" },  // macrolanguage code
    { LANGUAGE_USER_PALI_LATIN,                 "pi-Latn", ""   },
    { LANGUAGE_DONTKNOW,                        "",        ""   }   // marks end of table
};

static Bcp47CountryEntry const aImplBcp47CountryEntries[] =
{
    // MS-LangID                              full BCP47, ISO3166, ISO639-Variant or other fallback
    { LANGUAGE_CATALAN_VALENCIAN,       "ca-ES-valencia", "ES", "ca-valencia" },
    { LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN, "ca-ES-valencia", "ES", "" },   // In case MS format files using the old value escaped into the wild, map them back.
    { LANGUAGE_USER_ENGLISH_UK_OED,          "en-GB-oed", "GB", "" },   // grandfathered
    { LANGUAGE_DONTKNOW,                    "", "", "" }    // marks end of table
};

static IsoLanguageCountryEntry aLastResortFallbackEntry =
{ LANGUAGE_ENGLISH_US, "en", "US", false };

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

// -----------------------------------------------------------------------

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
    { LANGUAGE_ENGLISH_CARRIBEAN,           "BB" },         // Barbedos
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

// -----------------------------------------------------------------------

static IsoLangNoneStdEntry const aImplIsoNoneStdLangEntries[] =
{
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "BOK"      }, // registered subtags for "no" in rfc1766
    { LANGUAGE_NORWEGIAN_NYNORSK,           "no", "NYN"      }, // registered subtags for "no" in rfc1766
    { LANGUAGE_SERBIAN_LATIN,               "sr", "latin"    },
    { LANGUAGE_SERBIAN_CYRILLIC,            "sr", "cyrillic" },
    { LANGUAGE_AZERI_LATIN,                 "az", "latin"    },
    { LANGUAGE_AZERI_CYRILLIC,              "az", "cyrillic" },
    { LANGUAGE_DONTKNOW,                    "",   ""         }  // marks end of table
};

// -----------------------------------------------------------------------

// in this table are only names to find the best language
static IsoLangNoneStdEntry const aImplIsoNoneStdLangEntries2[] =
{
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "bokmaal"  },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "no", "bokmal"   },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "no", "nynorsk"  },
    { LANGUAGE_DONTKNOW,                    "",   ""         }  // marks end of table
};

// -----------------------------------------------------------------------

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
    { LANGUAGE_DONTKNOW,                    NULL             }  // marks end of table
};


// in this table are only privateuse names
static IsoLangOtherEntry const aImplPrivateUseEntries[] =
{
    { LANGUAGE_USER_PRIV_NOTRANSLATE,       "x-no-translate" }, //! not BCP47 but legacy in .xcu configmgr
    { LANGUAGE_USER_PRIV_DEFAULT,           "x-default"      },
    { LANGUAGE_USER_PRIV_COMMENT,           "x-comment"      },
    { LANGUAGE_USER_PRIV_JOKER,             "*"              }, //! not BCP47 but transferable in configmgr
    { LANGUAGE_DONTKNOW,                    NULL             }  // marks end of table
};

// =======================================================================

// static
void MsLangId::Conversion::convertLanguageToLocaleImpl( LanguageType nLang,
        ::com::sun::star::lang::Locale & rLocale )
{
    // Search for LangID in BCP47
    for (const Bcp47CountryEntry* pBcp47Entry = aImplBcp47CountryEntries;
            pBcp47Entry->mnLang != LANGUAGE_DONTKNOW; ++pBcp47Entry)
    {
        if ( pBcp47Entry->mnLang == nLang )
        {
            rLocale.Language = I18NLANGTAG_QLT;
            rLocale.Country  = OUString::createFromAscii( pBcp47Entry->maCountry);
            rLocale.Variant  = pBcp47Entry->getTagString();
            return;
        }
    }

    // Search for LangID in ISO lll-Ssss-CC
    for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
            pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
    {
        if ( pScriptEntry->mnLang == nLang )
        {
            rLocale.Language = I18NLANGTAG_QLT;
            rLocale.Country  = OUString::createFromAscii( pScriptEntry->maCountry);
            rLocale.Variant  = pScriptEntry->getTagString();
            return;
        }
    }

    // Search for LangID in ISO lll-CC
    for (const IsoLanguageCountryEntry* pEntry = aImplIsoLangEntries;
            pEntry->mnLang != LANGUAGE_DONTKNOW; ++pEntry)
    {
        if ( pEntry->mnLang == nLang )
        {
            rLocale.Language = OUString::createFromAscii( pEntry->maLanguage );
            rLocale.Country  = OUString::createFromAscii( pEntry->maCountry );
            rLocale.Variant  = OUString();
            return;
        }
    }

    // Look for privateuse definitions.
    for (const IsoLangOtherEntry* pPrivateEntry = aImplPrivateUseEntries;
            pPrivateEntry->mnLang != LANGUAGE_DONTKNOW; ++pPrivateEntry)
    {
        if ( pPrivateEntry->mnLang == nLang )
        {
            rLocale.Language = I18NLANGTAG_QLT;
            rLocale.Country  = OUString();
            rLocale.Variant  = OUString::createFromAscii( pPrivateEntry->mpLanguage );
            return;
        }
    }

    // Not found. Passed rLocale argument remains unchanged.
}

// -----------------------------------------------------------------------

// static
com::sun::star::lang::Locale MsLangId::Conversion::getLocale( const IsoLanguageCountryEntry * pEntry )
{
    if (pEntry->mbOverrideExists)
    {
        lang::Locale aLocale;
        convertLanguageToLocaleImpl( pEntry->mnLang, aLocale);
        return aLocale;
    }
    else
        return pEntry->getLocale();
}

// static
::com::sun::star::lang::Locale MsLangId::Conversion::lookupFallbackLocale(
        const ::com::sun::star::lang::Locale & rLocale )
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
                return pBcp47Entry->getLocale();
        }

        // Search in ISO lll-Ssss-CC
        const IsoLanguageScriptCountryEntry* pFirstScript = NULL;
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return pScriptEntry->getLocale();
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
                        return pScriptEntry->getLocale();
                }
            }
            return pFirstScript->getLocale();
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
    const IsoLanguageCountryEntry* pFirstLang = NULL;
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
                switch (pEntry->mnLang)
                {
                    // These are known to have no country assigned.
                    case LANGUAGE_BASQUE:
                    case LANGUAGE_USER_ESPERANTO:
                    case LANGUAGE_USER_INTERLINGUA:
                    case LANGUAGE_USER_LOJBAN:
                    case LANGUAGE_KASHMIRI:
                    case LANGUAGE_USER_KEYID:
                        return getLocale( pEntry);  // may override
                    default:
                        ;   // nothing
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

// =======================================================================

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
        const ::com::sun::star::lang::Locale& rLocale )
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
                return pBcp47Entry->mnLang;
        }

        // Search in ISO lll-Ssss-CC
        for (const IsoLanguageScriptCountryEntry* pScriptEntry = aImplIsoLangScriptEntries;
                pScriptEntry->mnLang != LANGUAGE_DONTKNOW; ++pScriptEntry)
        {
            if (pScriptEntry->startsInIgnoreAsciiCase( rLocale.Variant))
            {
                if (rLocale.Variant.equalsIgnoreAsciiCase( pScriptEntry->getTagString()))
                    return pScriptEntry->mnLang;
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
                return pEntry->mnLang;
        }
    }
    return LANGUAGE_DONTKNOW;
}


// static
LanguageType MsLangId::Conversion::convertIsoNamesToLanguage( const OUString& rLang,
        const OUString& rCountry )
{
    // language is lower case in table
    OUString aLowerLang = rLang.toAsciiLowerCase();
    // country is upper case in table
    OUString aUpperCountry = rCountry.toAsciiUpperCase();

    //  first look for exact match
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

// -----------------------------------------------------------------------

// static
LanguageType MsLangId::Conversion::convertIsoNamesToLanguage( const OString& rLang,
        const OString& rCountry )
{
    OUString aLang = OStringToOUString( rLang, RTL_TEXTENCODING_ASCII_US);
    OUString aCountry = OStringToOUString( rCountry, RTL_TEXTENCODING_ASCII_US);
    return convertIsoNamesToLanguage( aLang, aCountry);
}

// -----------------------------------------------------------------------

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
    { LANGUAGE_SERBIAN_LATIN,                       "sr", "CS", "latin" },   // Serbian Latin in Serbia and Montenegro
    { LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO,       "sr", "ME", "latin" },   // Serbian Latin in Montenegro
    { LANGUAGE_SERBIAN_LATIN_NEUTRAL,               "sr", "",   "latin" },
    { LANGUAGE_AZERI_CYRILLIC,                      "az", "AZ", "cyrillic" },
    { LANGUAGE_UZBEK_CYRILLIC,                      "uz", "UZ", "cyrillic" },
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
