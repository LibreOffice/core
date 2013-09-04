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
    { LANGUAGE_ENGLISH,                     "en", ""   },
    { LANGUAGE_ENGLISH_US,                  "en", "US" },
    { LANGUAGE_ENGLISH_UK,                  "en", "GB" },
    { LANGUAGE_ENGLISH_AUS,                 "en", "AU" },
    { LANGUAGE_ENGLISH_CAN,                 "en", "CA" },
    { LANGUAGE_FRENCH,                      "fr", "FR" },
    { LANGUAGE_GERMAN,                      "de", "DE" },
    { LANGUAGE_ITALIAN,                     "it", "IT" },
    { LANGUAGE_DUTCH,                       "nl", "NL" },
    { LANGUAGE_SPANISH_MODERN,              "es", "ES" },
    { LANGUAGE_SPANISH_DATED,               "es", "ES" },
    { LANGUAGE_PORTUGUESE,                  "pt", "PT" },
    { LANGUAGE_PORTUGUESE_BRAZILIAN,        "pt", "BR" },
    { LANGUAGE_DANISH,                      "da", "DK" },
    { LANGUAGE_GREEK,                       "el", "GR" },
    { LANGUAGE_CHINESE_SIMPLIFIED,          "zh", "CN" },
    { LANGUAGE_CHINESE_SIMPLIFIED_LEGACY,   "zh", "CN" },
    { LANGUAGE_CHINESE_TRADITIONAL,         "zh", "TW" },
    { LANGUAGE_CHINESE_TRADITIONAL_LEGACY,  "zh", "TW" },
    { LANGUAGE_CHINESE_HONGKONG,            "zh", "HK" },
    { LANGUAGE_CHINESE_SINGAPORE,           "zh", "SG" },
    { LANGUAGE_CHINESE_MACAU,               "zh", "MO" },
    { LANGUAGE_ENGLISH_HONG_KONG_SAR,       "en", "HK" },
    { LANGUAGE_JAPANESE,                    "ja", "JP" },
    { LANGUAGE_KOREAN,                      "ko", "KR" },
    { LANGUAGE_KOREAN_JOHAB,                "ko", "KR" },
    { LANGUAGE_USER_KOREAN_NORTH,           "ko", "KP" },
    { LANGUAGE_SWEDISH,                     "sv", "SE" },
    { LANGUAGE_SWEDISH_FINLAND,             "sv", "FI" },
    { LANGUAGE_FINNISH,                     "fi", "FI" },
    { LANGUAGE_RUSSIAN,                     "ru", "RU" },
    { LANGUAGE_TATAR,                       "tt", "RU" },
    { LANGUAGE_ENGLISH_NZ,                  "en", "NZ" },
    { LANGUAGE_ENGLISH_EIRE,                "en", "IE" },
    { LANGUAGE_DUTCH_BELGIAN,               "nl", "BE" },
    { LANGUAGE_FRENCH_BELGIAN,              "fr", "BE" },
    { LANGUAGE_FRENCH_CANADIAN,             "fr", "CA" },
    { LANGUAGE_FRENCH_SWISS,                "fr", "CH" },
    { LANGUAGE_GERMAN_SWISS,                "de", "CH" },
    { LANGUAGE_GERMAN_AUSTRIAN,             "de", "AT" },
    { LANGUAGE_ITALIAN_SWISS,               "it", "CH" },
    { LANGUAGE_ALBANIAN,                    "sq", "AL" },
    { LANGUAGE_ARABIC_SAUDI_ARABIA,         "ar", "SA" },
    { LANGUAGE_ARABIC_EGYPT,                "ar", "EG" },
    { LANGUAGE_ARABIC_UAE,                  "ar", "AE" },
    { LANGUAGE_ARABIC_IRAQ,                 "ar", "IQ" },
    { LANGUAGE_ARABIC_LIBYA,                "ar", "LY" },
    { LANGUAGE_ARABIC_ALGERIA,              "ar", "DZ" },
    { LANGUAGE_ARABIC_MOROCCO,              "ar", "MA" },
    { LANGUAGE_ARABIC_TUNISIA,              "ar", "TN" },
    { LANGUAGE_ARABIC_OMAN,                 "ar", "OM" },
    { LANGUAGE_ARABIC_YEMEN,                "ar", "YE" },
    { LANGUAGE_ARABIC_SYRIA,                "ar", "SY" },
    { LANGUAGE_ARABIC_JORDAN,               "ar", "JO" },
    { LANGUAGE_ARABIC_LEBANON,              "ar", "LB" },
    { LANGUAGE_ARABIC_KUWAIT,               "ar", "KW" },
    { LANGUAGE_ARABIC_BAHRAIN,              "ar", "BH" },
    { LANGUAGE_ARABIC_QATAR,                "ar", "QA" },
    { LANGUAGE_USER_ARABIC_CHAD,            "ar", "TD" },
    { LANGUAGE_USER_ARABIC_COMOROS,         "ar", "KM" },
    { LANGUAGE_USER_ARABIC_DJIBOUTI,        "ar", "DJ" },
    { LANGUAGE_USER_ARABIC_ERITREA,         "ar", "ER" },
    { LANGUAGE_USER_ARABIC_ISRAEL,          "ar", "IL" },
    { LANGUAGE_USER_ARABIC_MAURITANIA,      "ar", "MR" },
    { LANGUAGE_USER_ARABIC_PALESTINE,       "ar", "PS" },
    { LANGUAGE_USER_ARABIC_SOMALIA,         "ar", "SO" },
    { LANGUAGE_USER_ARABIC_SUDAN,           "ar", "SD" },
    { LANGUAGE_ARABIC_PRIMARY_ONLY,         "ar", ""   },
    { LANGUAGE_BASQUE,                      "eu", ""   },
    { LANGUAGE_BULGARIAN,                   "bg", "BG" },
    { LANGUAGE_CZECH,                       "cs", "CZ" },
    { LANGUAGE_CZECH,                       "cz", ""   },
    { LANGUAGE_ENGLISH_JAMAICA,             "en", "JM" },
    { LANGUAGE_ENGLISH_CARRIBEAN,           "en", "BS" },   // not 100%, because AG is Bahamas
    { LANGUAGE_ENGLISH_BELIZE,              "en", "BZ" },
    { LANGUAGE_ENGLISH_TRINIDAD,            "en", "TT" },
    { LANGUAGE_ENGLISH_ZIMBABWE,            "en", "ZW" },
    { LANGUAGE_ENGLISH_INDONESIA,           "en", "ID" },
    { LANGUAGE_ESTONIAN,                    "et", "EE" },
    { LANGUAGE_FAEROESE,                    "fo", "FO" },
    { LANGUAGE_FARSI,                       "fa", "IR" },
    { LANGUAGE_FRENCH_LUXEMBOURG,           "fr", "LU" },
    { LANGUAGE_FRENCH_MONACO,               "fr", "MC" },
    { LANGUAGE_GERMAN_LUXEMBOURG,           "de", "LU" },
    { LANGUAGE_GERMAN_LIECHTENSTEIN,        "de", "LI" },
    { LANGUAGE_HEBREW,                      "he", "IL" },   // new: old was "iw"
    { LANGUAGE_HEBREW,                      "iw", "IL" },   // old: new is "he"
    { LANGUAGE_HUNGARIAN,                   "hu", "HU" },
    { LANGUAGE_ICELANDIC,                   "is", "IS" },
    { LANGUAGE_INDONESIAN,                  "id", "ID" },   // new: old was "in"
    { LANGUAGE_INDONESIAN,                  "in", "ID" },   // old: new is "id"
    { LANGUAGE_NORWEGIAN,                   "no", "NO" },
    { LANGUAGE_NORWEGIAN_BOKMAL,            "nb", "NO" },
    { LANGUAGE_NORWEGIAN_NYNORSK,           "nn", "NO" },
    { LANGUAGE_POLISH,                      "pl", "PL" },
    { LANGUAGE_RHAETO_ROMAN,                "rm", "CH" },
    { LANGUAGE_ROMANIAN,                    "ro", "RO" },
    { LANGUAGE_ROMANIAN_MOLDOVA,            "ro", "MD" },
    { LANGUAGE_SLOVAK,                      "sk", "SK" },
    { LANGUAGE_SLOVENIAN,                   "sl", "SI" },
    { LANGUAGE_SPANISH_MEXICAN,             "es", "MX" },
    { LANGUAGE_SPANISH_GUATEMALA,           "es", "GT" },
    { LANGUAGE_SPANISH_COSTARICA,           "es", "CR" },
    { LANGUAGE_SPANISH_PANAMA,              "es", "PA" },
    { LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,  "es", "DO" },
    { LANGUAGE_SPANISH_VENEZUELA,           "es", "VE" },
    { LANGUAGE_SPANISH_COLOMBIA,            "es", "CO" },
    { LANGUAGE_SPANISH_PERU,                "es", "PE" },
    { LANGUAGE_SPANISH_ARGENTINA,           "es", "AR" },
    { LANGUAGE_SPANISH_ECUADOR,             "es", "EC" },
    { LANGUAGE_SPANISH_CHILE,               "es", "CL" },
    { LANGUAGE_SPANISH_URUGUAY,             "es", "UY" },
    { LANGUAGE_SPANISH_PARAGUAY,            "es", "PY" },
    { LANGUAGE_SPANISH_BOLIVIA,             "es", "BO" },
    { LANGUAGE_SPANISH_EL_SALVADOR,         "es", "SV" },
    { LANGUAGE_SPANISH_HONDURAS,            "es", "HN" },
    { LANGUAGE_SPANISH_NICARAGUA,           "es", "NI" },
    { LANGUAGE_SPANISH_PUERTO_RICO,         "es", "PR" },
    { LANGUAGE_SPANISH_UNITED_STATES,       "es", "US" },
    { LANGUAGE_SPANISH_LATIN_AMERICA,       "es", ""   },
    { LANGUAGE_TURKISH,                     "tr", "TR" },
    { LANGUAGE_UKRAINIAN,                   "uk", "UA" },
    { LANGUAGE_VIETNAMESE,                  "vi", "VN" },
    { LANGUAGE_LATVIAN,                     "lv", "LV" },
    { LANGUAGE_MACEDONIAN,                  "mk", "MK" },
    { LANGUAGE_MALAY_MALAYSIA,              "ms", "MY" },
    { LANGUAGE_MALAY_BRUNEI_DARUSSALAM,     "ms", "BN" },
    { LANGUAGE_ENGLISH_MALAYSIA,            "en", "MY" },
    { LANGUAGE_THAI,                        "th", "TH" },
    { LANGUAGE_LITHUANIAN,                  "lt", "LT" },
    { LANGUAGE_LITHUANIAN_CLASSIC,          "lt", "LT" },
    { LANGUAGE_CROATIAN,                    "hr", "HR" },   // Croatian in Croatia
    { LANGUAGE_CROATIAN_BOSNIA_HERZEGOVINA, "hr", "BA" },
    { LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA,        "bs", "BA" },
//  { LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_AND_HERZEGOVINA, "bs", "BA" },   // script codes not supported yet
    { LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA,        "sr", "RS" },   // Serbian Cyrillic in Serbia
    { LANGUAGE_SERBIAN_CYRILLIC,                    "sr", "YU" },   // legacy Serbian Cyrillic in Serbia and Montenegro (former Yugoslavia); kludge, needed to be sr_CS instead, sr_CS not supported by ICU 2.6 (3.4 does)
    { LANGUAGE_SERBIAN_CYRILLIC,                    "sr", "CS" },   // alias to be able to integrate localizations, rsc needs it
    { LANGUAGE_USER_SERBIAN_CYRILLIC_MONTENEGRO,    "sr", "ME" },
    { LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA, "sr", "BA" },
    { LANGUAGE_USER_SERBIAN_LATIN_SERBIA,           "sh", "RS" },   // Serbian Latin in Serbia; kludge, needed to be sr_Latn_RS instead, script codes not supported yet
    { LANGUAGE_SERBIAN_LATIN,                       "sh", "YU" },   // legacy Serbian Latin in Serbia and Montenegro (former Yugoslavia); kludge, needed to be sr_Latn_CS instead, script codes not supported yet
    { LANGUAGE_SERBIAN_LATIN,                       "sh", "CS" },   // Serbian Latin in Serbia and Montenegro; kludge, needed to be sr_Latn_CS instead, script codes not supported yet
    { LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO,       "sh", "ME" },   // Serbian Latin in Montenegro; kludge, needed to be sr_Latn_ME instead, script codes not supported yet
    { LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA,    "sh", "BA" },
    { LANGUAGE_SERBIAN_LATIN_NEUTRAL,               "sh", ""   },   // kludge, needed to be sr_Latn instead, script codes not supported yet
    { LANGUAGE_ARMENIAN,                    "hy", "AM" },
    { LANGUAGE_AZERI_LATIN,                 "az", "AZ" },
//  { LANGUAGE_AZERI_CYRILLIC,              "az", "AZ" },   // script codes not supported yet
    { LANGUAGE_UZBEK_LATIN,                 "uz", "UZ" },
//  { LANGUAGE_UZBEK_CYRILLIC,              "uz", "UZ" },   // script codes not supported yet
    { LANGUAGE_BENGALI_BANGLADESH,          "bn", "BD" },
    { LANGUAGE_BENGALI,                     "bn", "IN" },
    { LANGUAGE_BURMESE,                     "my", "MM" },
    { LANGUAGE_KAZAKH,                      "kk", "KZ" },
    { LANGUAGE_ENGLISH_INDIA,               "en", "IN" },
    { LANGUAGE_URDU_INDIA,                  "ur", "IN" },
    { LANGUAGE_URDU_PAKISTAN,               "ur", "PK" },
    { LANGUAGE_HINDI,                       "hi", "IN" },
    { LANGUAGE_GUJARATI,                    "gu", "IN" },
    { LANGUAGE_KANNADA,                     "kn", "IN" },
    { LANGUAGE_ASSAMESE,                    "as", "IN" },
    { LANGUAGE_KASHMIRI_INDIA,              "ks", "IN" },
    { LANGUAGE_KASHMIRI,                    "ks", ""   },   // Kashmiri in "Jammu and Kashmir" ... no ISO3166 code for that
    { LANGUAGE_MALAYALAM,                   "ml", "IN" },
    { LANGUAGE_MANIPURI,                   "mni", "IN" },
    { LANGUAGE_MARATHI,                     "mr", "IN" },
    { LANGUAGE_KONKANI,                    "kok", "IN" },
    { LANGUAGE_NEPALI,                      "ne", "NP" },
    { LANGUAGE_NEPALI_INDIA,                "ne", "IN" },
    { LANGUAGE_ORIYA,                       "or", "IN" },
    { LANGUAGE_PUNJABI,                     "pa", "IN" },
    { LANGUAGE_SANSKRIT,                    "sa", "IN" },
    { LANGUAGE_SINDHI,                      "sd", "IN" },
    { LANGUAGE_TAMIL,                       "ta", "IN" },
    { LANGUAGE_TELUGU,                      "te", "IN" },
    { LANGUAGE_PUNJABI_PAKISTAN,           "lah", "PK" },   // preferring "lah" over "pa" for Western Punjabi, see http://www.ethnologue.com/show_language.asp?code=PNB
    { LANGUAGE_PUNJABI_PAKISTAN,            "pa", "PK" },
    { LANGUAGE_SINDHI_PAKISTAN,             "sd", "PK" },
    { LANGUAGE_BELARUSIAN,                  "be", "BY" },
    { LANGUAGE_CATALAN,                     "ca", "ES" },   // Spain (default)
    { LANGUAGE_CATALAN,                     "ca", "AD" },   // Andorra
    //LANGUAGE_CATALAN_VALENCIAN ca-ES-valencia Bcp47CountryEntry takes precedence
    { LANGUAGE_CATALAN_VALENCIAN,           "ca", "XV" },   // XV: ISO 3166 user-assigned; old workaround for UI localization only, in case it escaped to document content
    { LANGUAGE_CATALAN_VALENCIAN,          "qcv", "ES" },   // qcv: ISO 639-3 reserved-for-local-use; old UI localization quirk only, in case it escaped to document content
    { LANGUAGE_FRENCH_CAMEROON,             "fr", "CM" },
    { LANGUAGE_FRENCH_COTE_D_IVOIRE,        "fr", "CI" },
    { LANGUAGE_FRENCH_MALI,                 "fr", "ML" },
    { LANGUAGE_FRENCH_SENEGAL,              "fr", "SN" },
    { LANGUAGE_FRENCH_ZAIRE,                "fr", "CD" },   // Democratic Republic Of Congo
    { LANGUAGE_FRENCH_MOROCCO,              "fr", "MA" },
    { LANGUAGE_FRENCH_REUNION,              "fr", "RE" },
    { LANGUAGE_FRENCH,                      "fr", ""   },   // needed as a catcher before other "fr" entries!
    { LANGUAGE_FRENCH_NORTH_AFRICA,         "fr", ""   },
    { LANGUAGE_FRENCH_WEST_INDIES,          "fr", ""   },   // unknown ISO country code
    { LANGUAGE_FRISIAN_NETHERLANDS,         "fy", "NL" },
    { LANGUAGE_GAELIC_IRELAND,              "ga", "IE" },
    { LANGUAGE_GAELIC_SCOTLAND,             "gd", "GB" },
    { LANGUAGE_GALICIAN,                    "gl", "ES" },
    { LANGUAGE_GEORGIAN,                    "ka", "GE" },
    { LANGUAGE_KHMER,                       "km", "KH" },
    { LANGUAGE_KIRGHIZ,                     "ky", "KG" },
    { LANGUAGE_LAO,                         "lo", "LA" },
    { LANGUAGE_MALTESE,                     "mt", "MT" },
    { LANGUAGE_MONGOLIAN,                   "mn", "MN" },   // Cyrillic script
    { LANGUAGE_MONGOLIAN_MONGOLIAN,         "mn", "MN" },
    { LANGUAGE_RUSSIAN_MOLDOVA,             "mo", "MD" },
    { LANGUAGE_SWAHILI,                     "sw", "KE" },
    { LANGUAGE_USER_SWAHILI_TANZANIA,       "sw", "TZ" },
    { LANGUAGE_TAJIK,                       "tg", "TJ" },
    { LANGUAGE_TIBETAN,                     "bo", "CN" },   // CN politically correct?
    { LANGUAGE_USER_TIBETAN_INDIA,          "bo", "IN" },
    { LANGUAGE_DZONGKHA,                    "dz", "BT" },
    { LANGUAGE_TURKMEN,                     "tk", "TM" },
    { LANGUAGE_WELSH,                       "cy", "GB" },
    { LANGUAGE_SESOTHO,                     "st", "ZA" },
    { LANGUAGE_SEPEDI,                     "nso", "ZA" },
    { LANGUAGE_SEPEDI,                      "ns", "ZA" },   // fake "ns" for compatibility with existing OOo1.1.x localization to be able to read those documents
    { LANGUAGE_TSONGA,                      "ts", "ZA" },
    { LANGUAGE_TSWANA,                      "tn", "ZA" },
    { LANGUAGE_ENGLISH_SAFRICA,             "en", "ZA" },
    { LANGUAGE_AFRIKAANS,                   "af", "ZA" },
    { LANGUAGE_VENDA,                       "ve", "ZA" },   // default 639-1
    { LANGUAGE_VENDA,                      "ven", "ZA" },   // 639-2 may have been used temporarily since 2004-07-23
    { LANGUAGE_XHOSA,                       "xh", "ZA" },
    { LANGUAGE_ZULU,                        "zu", "ZA" },
    { LANGUAGE_QUECHUA_ECUADOR,             "qu", "EC" },
    { LANGUAGE_QUECHUA_PERU,                "qu", "PE" },
    { LANGUAGE_QUECHUA_BOLIVIA,             "qu", "BO" },   // macro: quh-BO, qul-BO
    { LANGUAGE_PASHTO,                      "ps", "AF" },
    { LANGUAGE_OROMO,                       "om", "ET" },
    { LANGUAGE_DHIVEHI,                     "dv", "MV" },
    { LANGUAGE_UIGHUR_CHINA,                "ug", "CN" },
    { LANGUAGE_TIGRIGNA_ETHIOPIA,           "ti", "ET" },
    { LANGUAGE_TIGRIGNA_ERITREA,            "ti", "ER" },
    { LANGUAGE_AMHARIC_ETHIOPIA,            "am", "ET" },
    { LANGUAGE_GUARANI_PARAGUAY,           "gug", "PY" },
    { LANGUAGE_HAWAIIAN_UNITED_STATES,     "haw", "US" },
    { LANGUAGE_EDO,                        "bin", "NG" },
    { LANGUAGE_FULFULDE_NIGERIA,            "ff", "NG" },
    { LANGUAGE_HAUSA_NIGERIA,               "ha", "NG" },
    { LANGUAGE_USER_HAUSA_GHANA,            "ha", "GH" },
    { LANGUAGE_IGBO_NIGERIA,                "ig", "NG" },
    { LANGUAGE_KANURI_NIGERIA,              "kr", "NG" },
    { LANGUAGE_YORUBA,                      "yo", "NG" },
    { LANGUAGE_SOMALI,                      "so", "SO" },
    { LANGUAGE_PAPIAMENTU,                 "pap", "AN" },
    { LANGUAGE_USER_PAPIAMENTU_ARUBA,      "pap", "AW" },
    { LANGUAGE_USER_PAPIAMENTU_CURACAO,    "pap", "CW" },
    { LANGUAGE_USER_PAPIAMENTU_BONAIRE,    "pap", "BQ" },
    { LANGUAGE_ENGLISH_SINGAPORE,           "en", "SG" },
    { LANGUAGE_USER_YIDDISH_US,             "yi", "US" },
    { LANGUAGE_YIDDISH,                     "yi", "IL" },   // new: old was "ji"
    { LANGUAGE_YIDDISH,                     "ji", "IL" },   // old: new is "yi"
    { LANGUAGE_SYRIAC,                     "syr", "TR" },   // "TR" according to http://www.ethnologue.com/show_language.asp?code=SYC
    { LANGUAGE_SINHALESE_SRI_LANKA,         "si", "LK" },
    { LANGUAGE_CHEROKEE_UNITED_STATES,     "chr", "US" },
    { LANGUAGE_INUKTITUT_LATIN_CANADA,      "iu", "CA" },
//  { LANGUAGE_INUKTITUT_SYLLABICS_CANADA,  "iu", "CA" },   // script codes not supported yet
    { LANGUAGE_SAMI_NORTHERN_NORWAY,        "se", "NO" },
    { LANGUAGE_SAMI_INARI,                 "smn", "FI" },
    { LANGUAGE_SAMI_LULE_NORWAY,           "smj", "NO" },
    { LANGUAGE_SAMI_LULE_SWEDEN,           "smj", "SE" },
    { LANGUAGE_SAMI_NORTHERN_FINLAND,       "se", "FI" },
    { LANGUAGE_SAMI_NORTHERN_SWEDEN,        "se", "SE" },
    { LANGUAGE_SAMI_SKOLT,                 "sms", "FI" },
    { LANGUAGE_SAMI_SOUTHERN_NORWAY,       "sma", "NO" },
    { LANGUAGE_SAMI_SOUTHERN_SWEDEN,       "sma", "SE" },
    { LANGUAGE_USER_SAMI_KILDIN_RUSSIA,    "sjd", "RU" },
    { LANGUAGE_MAPUDUNGUN_CHILE,           "arn", "CL" },
    { LANGUAGE_CORSICAN_FRANCE,             "co", "FR" },
    { LANGUAGE_ALSATIAN_FRANCE,            "gsw", "FR" },   // in fact 'gsw' is Schwyzerduetsch (Swiss German), which is a dialect of Alemannic German, as is Alsatian. They aren't distinct languages and share this code.
    { LANGUAGE_YAKUT_RUSSIA,               "sah", "RU" },
    { LANGUAGE_MOHAWK_CANADA,              "moh", "CA" },
    { LANGUAGE_BASHKIR_RUSSIA,              "ba", "RU" },
    { LANGUAGE_KICHE_GUATEMALA,            "qut", "GT" },
    { LANGUAGE_DARI_AFGHANISTAN,           "gbz", "AF" },
    { LANGUAGE_WOLOF_SENEGAL,               "wo", "SN" },
    { LANGUAGE_FILIPINO,                   "fil", "PH" },
    { LANGUAGE_USER_TAGALOG,                "tl", "PH" },
    { LANGUAGE_ENGLISH_PHILIPPINES,         "en", "PH" },
//  { LANGUAGE_IBIBIO_NIGERIA,             "nic", "NG" },   // ISO "nic" is only a collective language code
    { LANGUAGE_YI,                          "ii", "CN" },
    { LANGUAGE_TAMAZIGHT_LATIN,            "kab", "DZ" },   // In practice Kabyle is the language used for this
    { LANGUAGE_OBSOLETE_USER_KABYLE,       "kab", "DZ" },
    { LANGUAGE_TAMAZIGHT_LATIN,            "ber", "DZ" },   // In practice Algeria has standardized on Kabyle as the member of the "ber" collective which gets used there.
    { LANGUAGE_TAMAZIGHT_TIFINAGH,         "ber", "MA" },   // Morocco is officially using Tifinagh for its Berber languages so store it to distinguish explicitly from LANGUAGE_TAMAZIGHT_LATIN, even though as a collective language its not of much use
//  { LANGUAGE_TAMAZIGHT_ARABIC,           "ber", ""   },   // ISO "ber" only collective!
    { LANGUAGE_LATIN,                       "la", "VA" },
    { LANGUAGE_OBSOLETE_USER_LATIN,         "la", "VA" },
    { LANGUAGE_USER_ESPERANTO,              "eo", ""   },
    { LANGUAGE_USER_INTERLINGUA,            "ia", ""   },
    { LANGUAGE_MAORI_NEW_ZEALAND,           "mi", "NZ" },
    { LANGUAGE_OBSOLETE_USER_MAORI,         "mi", "NZ" },
    { LANGUAGE_KINYARWANDA_RWANDA,          "rw", "RW" },
    { LANGUAGE_OBSOLETE_USER_KINYARWANDA,   "rw", "RW" },
    { LANGUAGE_UPPER_SORBIAN_GERMANY,      "hsb", "DE" },   // MS maps this to 'wen-DE', which is nonsense. 'wen' is a collective language code, 'WEN' is a SIL code, see http://www.ethnologue.com/14/show_iso639.asp?code=wen and http://www.ethnologue.com/14/show_language.asp?code=WEN
    { LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN,"hsb", "DE" },
    { LANGUAGE_LOWER_SORBIAN_GERMANY,      "dsb", "DE" },   // MS maps this to 'wee-DE', which is nonsense. 'WEE' is a SIL code, see http://www.ethnologue.com/14/show_language.asp?code=WEE
    { LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN,"dsb", "DE" },
    { LANGUAGE_OCCITAN_FRANCE,              "oc", "FR" },
    { LANGUAGE_OBSOLETE_USER_OCCITAN,       "oc", "FR" },
    { LANGUAGE_USER_KURDISH_TURKEY,         "ku", "TR" },
    { LANGUAGE_USER_KURDISH_SYRIA,          "ku", "SY" },
    { LANGUAGE_USER_KURDISH_IRAQ,           "ku", "IQ" },
    { LANGUAGE_USER_KURDISH_IRAN,           "ku", "IR" },
    { LANGUAGE_USER_SARDINIAN,              "sc", "IT" },   // macrolanguage code
    { LANGUAGE_USER_SARDINIAN_CAMPIDANESE, "sro", "IT" },
    { LANGUAGE_USER_SARDINIAN_GALLURESE,   "sdn", "IT" },
    { LANGUAGE_USER_SARDINIAN_LOGUDORESE,  "src", "IT" },
    { LANGUAGE_USER_SARDINIAN_SASSARESE,   "sdc", "IT" },
    { LANGUAGE_BRETON_FRANCE,               "br", "FR" },
    { LANGUAGE_OBSOLETE_USER_BRETON,        "br", "FR" },
    { LANGUAGE_KALAALLISUT_GREENLAND,       "kl", "GL" },
    { LANGUAGE_OBSOLETE_USER_KALAALLISUT,   "kl", "GL" },
    { LANGUAGE_USER_SWAZI,                  "ss", "ZA" },
    { LANGUAGE_USER_NDEBELE_SOUTH,          "nr", "ZA" },
    { LANGUAGE_USER_TSWANA_BOTSWANA,        "tn", "BW" },
    { LANGUAGE_USER_MOORE,                 "mos", "BF" },
    { LANGUAGE_USER_BAMBARA,                "bm", "ML" },
    { LANGUAGE_USER_AKAN,                   "ak", "GH" },
    { LANGUAGE_LUXEMBOURGISH_LUXEMBOURG,    "lb", "LU" },
    { LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH, "lb", "LU" },
    { LANGUAGE_USER_FRIULIAN,              "fur", "IT" },
    { LANGUAGE_USER_FIJIAN,                 "fj", "FJ" },
    { LANGUAGE_USER_AFRIKAANS_NAMIBIA,      "af", "NA" },
    { LANGUAGE_USER_ENGLISH_NAMIBIA,        "en", "NA" },
    { LANGUAGE_USER_WALLOON,                "wa", "BE" },
    { LANGUAGE_USER_COPTIC,                "cop", "EG" },
    { LANGUAGE_USER_GASCON,                "gsc", "FR" },
    { LANGUAGE_USER_GERMAN_BELGIUM,         "de", "BE" },
    { LANGUAGE_USER_CHUVASH,                "cv", "RU" },
    { LANGUAGE_USER_EWE_GHANA,              "ee", "GH" },
    { LANGUAGE_USER_ENGLISH_GHANA,          "en", "GH" },
    { LANGUAGE_USER_SANGO,                  "sg", "CF" },
    { LANGUAGE_USER_GANDA,                  "lg", "UG" },
    { LANGUAGE_USER_LINGALA_DRCONGO,        "ln", "CD" },
    { LANGUAGE_USER_LOW_GERMAN,            "nds", "DE" },
    { LANGUAGE_USER_HILIGAYNON,            "hil", "PH" },
    { LANGUAGE_USER_ENGLISH_MALAWI,         "en", "MW" },   /* en default for MW */
    { LANGUAGE_USER_NYANJA,                 "ny", "MW" },
    { LANGUAGE_USER_KASHUBIAN,             "csb", "PL" },
    { LANGUAGE_USER_SPANISH_CUBA,           "es", "CU" },
    { LANGUAGE_USER_QUECHUA_NORTH_BOLIVIA, "qul", "BO" },
    { LANGUAGE_USER_QUECHUA_SOUTH_BOLIVIA, "quh", "BO" },
    { LANGUAGE_USER_BODO_INDIA,            "brx", "IN" },
    { LANGUAGE_USER_DOGRI_INDIA,           "dgo", "IN" },
    { LANGUAGE_USER_MAITHILI_INDIA,        "mai", "IN" },
    { LANGUAGE_USER_SANTALI_INDIA,         "sat", "IN" },
    { LANGUAGE_USER_TETUN,                 "tet", "ID" },
    { LANGUAGE_USER_TETUN_TIMOR_LESTE,     "tet", "TL" },
    { LANGUAGE_USER_TOK_PISIN,             "tpi", "PG" },
    { LANGUAGE_USER_SHUSWAP,               "shs", "CA" },
    { LANGUAGE_USER_ANCIENT_GREEK,         "grc", "GR" },
    { LANGUAGE_USER_ASTURIAN,              "ast", "ES" },
    { LANGUAGE_USER_LATGALIAN,             "ltg", "LV" },
    { LANGUAGE_USER_MAORE,                 "swb", "YT" },
    { LANGUAGE_USER_BUSHI,                 "buc", "YT" },
    { LANGUAGE_USER_TAHITIAN,               "ty", "PF" },
    { LANGUAGE_USER_MALAGASY_PLATEAU,      "plt", "MG" },
    { LANGUAGE_USER_MALAGASY_PLATEAU,       "mg", "MG" },
    { LANGUAGE_USER_BAFIA,                 "ksf", "CM" },
    { LANGUAGE_USER_GIKUYU,                 "ki", "KE" },
    { LANGUAGE_USER_RUSYN_UKRAINE,         "rue", "UA" },
    { LANGUAGE_USER_RUSYN_SLOVAKIA,        "rue", "SK" },
    { LANGUAGE_USER_LIMBU,                 "lif", "NP" },
    { LANGUAGE_USER_LOJBAN,                "jbo", ""   },
    { LANGUAGE_USER_HAITIAN,                "ht", "HT" },
    { LANGUAGE_FRENCH_HAITI,                "fr", "HT" },
    { LANGUAGE_USER_BEEMBE,                "beq", "CG" },
    { LANGUAGE_USER_BEKWEL,                "bkw", "CG" },
    { LANGUAGE_USER_KITUBA,                "mkw", "CG" },
    { LANGUAGE_USER_LARI,                  "ldi", "CG" },
    { LANGUAGE_USER_MBOCHI,                "mdw", "CG" },
    { LANGUAGE_USER_TEKE_EBOO,             "ebo", "CG" },
    { LANGUAGE_USER_TEKE_IBALI,            "tek", "CG" },
    { LANGUAGE_USER_TEKE_TYEE,             "tyx", "CG" },
    { LANGUAGE_USER_VILI,                  "vif", "CG" },
    { LANGUAGE_USER_PORTUGUESE_ANGOLA,      "pt", "AO" },
    { LANGUAGE_USER_MANX,                   "gv", "GB" },
    { LANGUAGE_USER_ARAGONESE,              "an", "ES" },
    { LANGUAGE_USER_KEYID,                 "qtz", ""   },   // key id pseudolanguage used for UI testing
    { LANGUAGE_USER_PALI_LATIN,             "pi", ""   },   // Pali with Latin script, ISO 639-1
    { LANGUAGE_USER_PALI_LATIN,            "pli", ""   },   // Pali with Latin script, ISO 639-3 (sigh..) back-compat
    { LANGUAGE_USER_KYRGYZ_CHINA,           "ky", "CN" },
    { LANGUAGE_USER_KOMI_ZYRIAN,           "kpv", "RU" },
    { LANGUAGE_USER_KOMI_PERMYAK,          "koi", "RU" },
    { LANGUAGE_USER_PITJANTJATJARA,        "pjt", "AU" },
    { LANGUAGE_USER_ERZYA,                 "myv", "RU" },
    { LANGUAGE_USER_MARI_MEADOW,           "mhr", "RU" },
    { LANGUAGE_USER_KHANTY,                "kca", "RU" },
    { LANGUAGE_USER_LIVONIAN,              "liv", "RU" },
    { LANGUAGE_USER_MOKSHA,                "mdf", "RU" },
    { LANGUAGE_USER_MARI_HILL,             "mrj", "RU" },
    { LANGUAGE_USER_NGANASAN,              "nio", "RU" },
    { LANGUAGE_USER_OLONETS,               "olo", "RU" },
    { LANGUAGE_USER_VEPS,                  "vep", "RU" },
    { LANGUAGE_USER_VORO,                  "vro", "EE" },
    { LANGUAGE_USER_NENETS,                "yrk", "RU" },
    { LANGUAGE_USER_AKA,                   "axk", "CF" },
    { LANGUAGE_USER_AKA_CONGO,             "axk", "CG" },
    { LANGUAGE_USER_DIBOLE,                "bvx", "CG" },
    { LANGUAGE_USER_DOONDO,                "dde", "CG" },
    { LANGUAGE_USER_KAAMBA,                "xku", "CG" },
    { LANGUAGE_USER_KOONGO,                "kng", "CD" },
    { LANGUAGE_USER_KOONGO_CONGO,          "kng", "CG" },
    { LANGUAGE_USER_KUNYI,                 "njx", "CG" },
    { LANGUAGE_USER_NGUNGWEL,              "ngz", "CG" },
    { LANGUAGE_USER_NJYEM,                 "njy", "CM" },
    { LANGUAGE_USER_NJYEM_CONGO,           "njy", "CG" },
    { LANGUAGE_USER_PUNU,                  "puu", "GA" },
    { LANGUAGE_USER_PUNU_CONGO,            "puu", "CG" },
    { LANGUAGE_USER_SUUNDI,                "sdj", "CG" },
    { LANGUAGE_USER_TEKE_KUKUYA,           "kkw", "CG" },
    { LANGUAGE_USER_TSAANGI,               "tsa", "CG" },
    { LANGUAGE_USER_YAKA,                  "iyx", "CG" },
    { LANGUAGE_USER_YOMBE,                 "yom", "CD" },
    { LANGUAGE_USER_YOMBE_CONGO,           "yom", "CG" },
    { LANGUAGE_USER_SIDAMA,                "sid", "ET" },
    { LANGUAGE_USER_NKO,                   "nqo", "GN" },
    { LANGUAGE_USER_UDMURT,                "udm", "RU" },
    { LANGUAGE_USER_CORNISH,               "kw",  "UK" },
    { LANGUAGE_USER_SAMI_PITE_SWEDEN,      "sje", "SE" },
    { LANGUAGE_USER_NGAEBERE,              "gym", "PA" },
    { LANGUAGE_MULTIPLE,                   "mul", ""   },   // multiple languages, many languages are used
    { LANGUAGE_UNDETERMINED,               "und", ""   },   // undetermined language, language cannot be identified
    { LANGUAGE_NONE,                       "zxx", ""   },   // added to ISO 639-2 on 2006-01-11: Used to declare the absence of linguistic information
    { LANGUAGE_DONTKNOW,                    "",   ""   }    // marks end of table
};

static IsoLanguageScriptCountryEntry const aImplIsoLangScriptEntries[] =
{
    // MS-LangID                      ISO639-ISO15924, ISO3166
//  { LANGUAGE_USER_SERBIAN_LATIN_SERBIA,   "sr-Latn", "RS" },  // for example, once we support it in l10n; TODO: adapt unit test in this case
    { LANGUAGE_DONTKNOW,                    "",        ""   }   // marks end of table
};

static Bcp47CountryEntry const aImplBcp47CountryEntries[] =
{
    // MS-LangID                              full BCP47, ISO3166, ISO639-Variant or other fallback
    { LANGUAGE_CATALAN_VALENCIAN,       "ca-ES-valencia", "ES", "ca-valencia" },
    { LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN, "ca-ES-valencia", "ES", "" },   // In case MS format files using the old value escaped into the wild, map them back.
    { LANGUAGE_DONTKNOW,                    "", "", "" }    // marks end of table
};

static IsoLanguageCountryEntry aLastResortFallbackEntry =
{ LANGUAGE_ENGLISH_US, "en", "US" };

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
                    return pEntry->getLocale();
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
                        return pEntry->getLocale();
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
                return pEntry->getLocale();
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
