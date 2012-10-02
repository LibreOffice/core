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

#include "filter/msfilter/countryid.hxx"

#include <algorithm>
#include <sal/macros.h>

// ----------------------------------------------------------------------------

namespace msfilter {

// Mapping table ==============================================================

namespace {

// ----------------------------------------------------------------------------

/** Table entry for Windows country ID <-> language type conversion.

    The first member is the Windows country ID, as defined in the header.

    The second member contains the corresponding language type for each country
    ID. This must be a full language, not only the primary language type.

    The last bool flag defines, if the sub language type should be evaluated to
    find the country ID from a language. If not set, all languages map to the
    country which contain the given primary language type.

    Example: The language entry (COUNTRY_USA,LANGUAGE_ENGLISH_US,false) maps
    the country ID for USA to the language LANGUAGE_ENGLISH_US. The clear sub
    language flag causes all english languages LANGUAGE_ENGLISH_*** to map to
    this country ID by default. To map the special case LANGUAGE_ENGLISH_EIRE
    to the country ID COUNTRY_IRELAND, the sub language flag must be set in the
    respective table entry, here (COUNTRY_IRELAND,LANGUAGE_ENGLISH_EIRE,true).
 */
struct CountryEntry
{
    CountryId                   meCountry;      /// Windows country ID.
    LanguageType                meLanguage;     /// Corresponding language type.
    bool                        mbUseSubLang;   /// false = Primary only, true = Primary and sub language.
};

// ----------------------------------------------------------------------------

/** Table for Windows country ID <-> language type conversion.

    To map the same language to different country IDs, some of the entries
    should contain a set sub language flag (see description of CountryEntry).
    All table entries with a set flag take priority over the entry with the
    same primary language, but cleared sub language flag, regardless of the
    position in the table.

    To map different languages to the same country ID, several entries with the
    same country ID may be inserted. In this case the conversion to a language
    is done with the first found entry (starting from top) containing the given
    country ID.

    For now all entries are sorted by country ID, but this is not required.
 */
static const CountryEntry pTable[] =
{
    { COUNTRY_USA,                  LANGUAGE_ENGLISH_US,                    false   },
    { COUNTRY_DOMINICAN_REPUBLIC,   LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,    true    },
    { COUNTRY_JAMAICA,              LANGUAGE_ENGLISH_JAMAICA,               true    },
    { COUNTRY_PUERTO_RICO,          LANGUAGE_SPANISH_PUERTO_RICO,           true    },
    { COUNTRY_TRINIDAD_Y_TOBAGO,    LANGUAGE_ENGLISH_TRINIDAD,              true    },
    { COUNTRY_CANADA,               LANGUAGE_ENGLISH_CAN,                   true    },
    { COUNTRY_CANADA,               LANGUAGE_FRENCH_CANADIAN,               true    },
    { COUNTRY_RUSSIA,               LANGUAGE_RUSSIAN,                       false   },
    { COUNTRY_KAZAKHSTAN,           LANGUAGE_KAZAK,                         false   },
    { COUNTRY_TATARSTAN,            LANGUAGE_TATAR,                         false   },
    { COUNTRY_EGYPT,                LANGUAGE_ARABIC_EGYPT,                  true    },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_AFRIKAANS,                     false   },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_ENGLISH_SAFRICA,               true    },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_TSONGA,                        false   },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_VENDA,                         false   },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_XHOSA,                         false   },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_ZULU,                          false   },
    { COUNTRY_GREECE,               LANGUAGE_GREEK,                         false   },
    { COUNTRY_NETHERLANDS,          LANGUAGE_DUTCH,                         false   },
    { COUNTRY_NETHERLANDS,          LANGUAGE_FRISIAN_NETHERLANDS,           false   },
    { COUNTRY_BELGIUM,              LANGUAGE_DUTCH_BELGIAN,                 true    },
    { COUNTRY_BELGIUM,              LANGUAGE_FRENCH_BELGIAN,                true    },
    { COUNTRY_FRANCE,               LANGUAGE_FRENCH,                        false   },
    { COUNTRY_SPAIN,                LANGUAGE_SPANISH_MODERN,                false   },
    { COUNTRY_SPAIN,                LANGUAGE_SPANISH_DATED,                 false   },
    { COUNTRY_SPAIN,                LANGUAGE_CATALAN,                       false   },
    { COUNTRY_SPAIN,                LANGUAGE_BASQUE,                        false   },
    { COUNTRY_SPAIN,                LANGUAGE_GALICIAN,                      false   },
    { COUNTRY_HUNGARY,              LANGUAGE_HUNGARIAN,                     false   },
    { COUNTRY_ITALY,                LANGUAGE_ITALIAN,                       false   },
    { COUNTRY_ROMANIA,              LANGUAGE_ROMANIAN,                      false   },
    { COUNTRY_SWITZERLAND,          LANGUAGE_GERMAN_SWISS,                  true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_FRENCH_SWISS,                  true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_ITALIAN_SWISS,                 true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_RHAETO_ROMAN,                  false   },
    { COUNTRY_AUSTRIA,              LANGUAGE_GERMAN_AUSTRIAN,               true    },
    { COUNTRY_UNITED_KINGDOM,       LANGUAGE_ENGLISH_UK,                    true    },
    { COUNTRY_UNITED_KINGDOM,       LANGUAGE_GAELIC_SCOTLAND,               true    },
    { COUNTRY_UNITED_KINGDOM,       LANGUAGE_WELSH,                         false   },
    { COUNTRY_DENMARK,              LANGUAGE_DANISH,                        false   },
    { COUNTRY_SWEDEN,               LANGUAGE_SWEDISH,                       false   },
    { COUNTRY_SWEDEN,               LANGUAGE_SAMI_LAPPISH,                  false   },
    { COUNTRY_NORWAY,               LANGUAGE_NORWEGIAN_BOKMAL,              false   },
    { COUNTRY_POLAND,               LANGUAGE_POLISH,                        false   },
    { COUNTRY_GERMANY,              LANGUAGE_GERMAN,                        false   },
    { COUNTRY_GERMANY,              LANGUAGE_SORBIAN,                       false   },
    { COUNTRY_PERU,                 LANGUAGE_SPANISH_PERU,                  true    },
    { COUNTRY_MEXICO,               LANGUAGE_SPANISH_MEXICAN,               true    },
    { COUNTRY_ARGENTINIA,           LANGUAGE_SPANISH_ARGENTINA,             true    },
    { COUNTRY_BRAZIL,               LANGUAGE_PORTUGUESE_BRAZILIAN,          true    },
    { COUNTRY_CHILE,                LANGUAGE_SPANISH_CHILE,                 true    },
    { COUNTRY_COLOMBIA,             LANGUAGE_SPANISH_COLOMBIA,              true    },
    { COUNTRY_VENEZUELA,            LANGUAGE_SPANISH_VENEZUELA,             true    },
    { COUNTRY_MALAYSIA,             LANGUAGE_MALAY_MALAYSIA,                false   },
    { COUNTRY_AUSTRALIA,            LANGUAGE_ENGLISH_AUS,                   true    },
    { COUNTRY_INDONESIA,            LANGUAGE_INDONESIAN,                    false   },
    { COUNTRY_PHILIPPINES,          LANGUAGE_ENGLISH_PHILIPPINES,           true    },
    { COUNTRY_NEW_ZEALAND,          LANGUAGE_MAORI_NEW_ZEALAND,             false   },
    { COUNTRY_NEW_ZEALAND,          LANGUAGE_ENGLISH_NZ,                    true    },
    { COUNTRY_SINGAPORE,            LANGUAGE_CHINESE_SINGAPORE,             true    },
    { COUNTRY_THAILAND,             LANGUAGE_THAI,                          false   },
    { COUNTRY_JAPAN,                LANGUAGE_JAPANESE,                      false   },
    { COUNTRY_SOUTH_KOREA,          LANGUAGE_KOREAN,                        false   },
    { COUNTRY_VIET_NAM,             LANGUAGE_VIETNAMESE,                    false   },
    { COUNTRY_PR_CHINA,             LANGUAGE_CHINESE_SIMPLIFIED,            false   },
    { COUNTRY_TIBET,                LANGUAGE_TIBETAN,                       false   },
    { COUNTRY_TURKEY,               LANGUAGE_TURKISH,                       false   },
    { COUNTRY_INDIA,                LANGUAGE_HINDI,                         false   },
    { COUNTRY_INDIA,                LANGUAGE_URDU_INDIA,                    true    },
    { COUNTRY_INDIA,                LANGUAGE_PUNJABI,                       false   },
    { COUNTRY_INDIA,                LANGUAGE_GUJARATI,                      false   },
    { COUNTRY_INDIA,                LANGUAGE_ORIYA,                         false   },
    { COUNTRY_INDIA,                LANGUAGE_TAMIL,                         false   },
    { COUNTRY_INDIA,                LANGUAGE_TELUGU,                        false   },
    { COUNTRY_INDIA,                LANGUAGE_KANNADA,                       false   },
    { COUNTRY_INDIA,                LANGUAGE_MALAYALAM,                     false   },
    { COUNTRY_INDIA,                LANGUAGE_ASSAMESE,                      false   },
    { COUNTRY_INDIA,                LANGUAGE_MARATHI,                       false   },
    { COUNTRY_INDIA,                LANGUAGE_SANSKRIT,                      false   },
    { COUNTRY_INDIA,                LANGUAGE_KONKANI,                       false   },
    { COUNTRY_INDIA,                LANGUAGE_MANIPURI,                      false   },
    { COUNTRY_INDIA,                LANGUAGE_SINDHI,                        false   },
    { COUNTRY_INDIA,                LANGUAGE_KASHMIRI,                      false   },
    { COUNTRY_PAKISTAN,             LANGUAGE_URDU_PAKISTAN,                 false   },
    { COUNTRY_MYANMAR,              LANGUAGE_BURMESE,                       false   },
    { COUNTRY_MOROCCO,              LANGUAGE_ARABIC_MOROCCO,                true    },
    { COUNTRY_ALGERIA,              LANGUAGE_ARABIC_ALGERIA,                true    },
    { COUNTRY_TUNISIA,              LANGUAGE_ARABIC_TUNISIA,                true    },
    { COUNTRY_LIBYA,                LANGUAGE_ARABIC_LIBYA,                  true    },
    { COUNTRY_SENEGAL,              LANGUAGE_FRENCH_SENEGAL,                true    },
    { COUNTRY_MALI,                 LANGUAGE_FRENCH_MALI,                   true    },
    { COUNTRY_COTE_D_IVOIRE,        LANGUAGE_FRENCH_COTE_D_IVOIRE,          true    },
    { COUNTRY_CAMEROON,             LANGUAGE_FRENCH_CAMEROON,               true    },
    { COUNTRY_ZAIRE,                LANGUAGE_FRENCH_ZAIRE,                  true    },
    { COUNTRY_RWANDA,               LANGUAGE_KINYARWANDA_RWANDA,            false   },
    { COUNTRY_KENYA,                LANGUAGE_SWAHILI,                       false   },
    { COUNTRY_REUNION,              LANGUAGE_FRENCH_REUNION,                true    },
    { COUNTRY_ZIMBABWE,             LANGUAGE_ENGLISH_ZIMBABWE,              true    },
    { COUNTRY_LESOTHO,              LANGUAGE_SESOTHO,                       false   },
    { COUNTRY_BOTSWANA,             LANGUAGE_TSWANA,                        false   },
    { COUNTRY_FAEROE_ISLANDS,       LANGUAGE_FAEROESE,                      false   },
    { COUNTRY_PORTUGAL,             LANGUAGE_PORTUGUESE,                    false   },
    { COUNTRY_LUXEMBOURG,           LANGUAGE_GERMAN_LUXEMBOURG,             true    },
    { COUNTRY_LUXEMBOURG,           LANGUAGE_FRENCH_LUXEMBOURG,             true    },
    { COUNTRY_IRELAND,              LANGUAGE_ENGLISH_EIRE,                  true    },
    { COUNTRY_IRELAND,              LANGUAGE_GAELIC_IRELAND,                true    },
    { COUNTRY_ICELAND,              LANGUAGE_ICELANDIC,                     false   },
    { COUNTRY_ALBANIA,              LANGUAGE_ALBANIAN,                      false   },
    { COUNTRY_MALTA,                LANGUAGE_MALTESE,                       false   },
    { COUNTRY_FINLAND,              LANGUAGE_FINNISH,                       false   },
    { COUNTRY_FINLAND,              LANGUAGE_SWEDISH_FINLAND,               true    },
    { COUNTRY_BULGARIA,             LANGUAGE_BULGARIAN,                     false   },
    { COUNTRY_LITHUANIA,            LANGUAGE_LITHUANIAN,                    false   },
    { COUNTRY_LATVIA,               LANGUAGE_LATVIAN,                       false   },
    { COUNTRY_ESTONIA,              LANGUAGE_ESTONIAN,                      false   },
    { COUNTRY_MOLDOVA,              LANGUAGE_ROMANIAN_MOLDOVA,              true    },
    { COUNTRY_MOLDOVA,              LANGUAGE_RUSSIAN_MOLDOVA,               true    },
    { COUNTRY_ARMENIA,              LANGUAGE_ARMENIAN,                      false   },
    { COUNTRY_BELARUS,              LANGUAGE_BELARUSIAN,                    false   },
    { COUNTRY_MONACO,               LANGUAGE_FRENCH_MONACO,                 true    },
    { COUNTRY_UKRAINE,              LANGUAGE_UKRAINIAN,                     false   },
    { COUNTRY_SERBIA,               LANGUAGE_SERBIAN_LATIN,                 false   },
    { COUNTRY_CROATIA,              LANGUAGE_CROATIAN,                      true    },  // sub type of LANGUAGE_SERBIAN
    { COUNTRY_SLOVENIA,             LANGUAGE_SLOVENIAN,                     false   },
    { COUNTRY_MACEDONIA,            LANGUAGE_MACEDONIAN,                    false   },
    { COUNTRY_CZECH,                LANGUAGE_CZECH,                         false   },
    { COUNTRY_SLOVAK,               LANGUAGE_SLOVAK,                        false   },
    { COUNTRY_LIECHTENSTEIN,        LANGUAGE_GERMAN_LIECHTENSTEIN,          true    },
    { COUNTRY_BELIZE,               LANGUAGE_ENGLISH_BELIZE,                true    },
    { COUNTRY_GUATEMALA,            LANGUAGE_SPANISH_GUATEMALA,             true    },
    { COUNTRY_EL_SALVADOR,          LANGUAGE_SPANISH_EL_SALVADOR,           true    },
    { COUNTRY_HONDURAS,             LANGUAGE_SPANISH_HONDURAS,              true    },
    { COUNTRY_NICARAGUA,            LANGUAGE_SPANISH_NICARAGUA,             true    },
    { COUNTRY_COSTA_RICA,           LANGUAGE_SPANISH_COSTARICA,             true    },
    { COUNTRY_PANAMA,               LANGUAGE_SPANISH_PANAMA,                true    },
    { COUNTRY_BOLIVIA,              LANGUAGE_SPANISH_BOLIVIA,               true    },
    { COUNTRY_ECUADOR,              LANGUAGE_SPANISH_ECUADOR,               true    },
    { COUNTRY_PARAGUAY,             LANGUAGE_SPANISH_PARAGUAY,              true    },
    { COUNTRY_URUGUAY,              LANGUAGE_SPANISH_URUGUAY,               true    },
    { COUNTRY_BRUNEI_DARUSSALAM,    LANGUAGE_MALAY_BRUNEI_DARUSSALAM,       true    },
    { COUNTRY_HONG_KONG,            LANGUAGE_CHINESE_HONGKONG,              true    },
    { COUNTRY_MACAU,                LANGUAGE_CHINESE_MACAU,                 true    },
    { COUNTRY_CAMBODIA,             LANGUAGE_KHMER,                         false   },
    { COUNTRY_LAOS,                 LANGUAGE_LAO,                           false   },
    { COUNTRY_BANGLADESH,           LANGUAGE_BENGALI,                       false   },
    { COUNTRY_TAIWAN,               LANGUAGE_CHINESE_TRADITIONAL,           true    },
    { COUNTRY_MALDIVES,             LANGUAGE_DHIVEHI,                       false   },
    { COUNTRY_LEBANON,              LANGUAGE_ARABIC_LEBANON,                true    },
    { COUNTRY_JORDAN,               LANGUAGE_ARABIC_JORDAN,                 true    },
    { COUNTRY_SYRIA,                LANGUAGE_ARABIC_SYRIA,                  true    },
    { COUNTRY_IRAQ,                 LANGUAGE_ARABIC_IRAQ,                   true    },
    { COUNTRY_KUWAIT,               LANGUAGE_ARABIC_KUWAIT,                 true    },
    { COUNTRY_SAUDI_ARABIA,         LANGUAGE_ARABIC_SAUDI_ARABIA,           true    },
    { COUNTRY_YEMEN,                LANGUAGE_ARABIC_YEMEN,                  true    },
    { COUNTRY_OMAN,                 LANGUAGE_ARABIC_OMAN,                   true    },
    { COUNTRY_UAE,                  LANGUAGE_ARABIC_UAE,                    true    },
    { COUNTRY_ISRAEL,               LANGUAGE_HEBREW,                        false   },
    { COUNTRY_BAHRAIN,              LANGUAGE_ARABIC_BAHRAIN,                true    },
    { COUNTRY_QATAR,                LANGUAGE_ARABIC_QATAR,                  true    },
    { COUNTRY_MONGOLIA,             LANGUAGE_MONGOLIAN,                     false   },
    { COUNTRY_NEPAL,                LANGUAGE_NEPALI,                        false   },
    { COUNTRY_IRAN,                 LANGUAGE_FARSI,                         false   },
    { COUNTRY_TAJIKISTAN,           LANGUAGE_TAJIK,                         false   },
    { COUNTRY_TURKMENISTAN,         LANGUAGE_TURKMEN,                       false   },
    { COUNTRY_AZERBAIJAN,           LANGUAGE_AZERI_LATIN,                   false   },
    { COUNTRY_GEORGIA,              LANGUAGE_GEORGIAN,                      false   },
    { COUNTRY_KYRGYZSTAN,           LANGUAGE_KIRGHIZ,                       false   },
    { COUNTRY_UZBEKISTAN,           LANGUAGE_UZBEK_LATIN,                   false   }
};

const CountryEntry * const pEnd = pTable + SAL_N_ELEMENTS( pTable );

/** Predicate comparing a country ID with the member of a CountryEntry. */
struct CountryEntryPred_Country
{
    CountryId                   meCountry;

    inline explicit             CountryEntryPred_Country( CountryId eCountry ) :
                                    meCountry( eCountry ) {}

    inline bool                 operator()( const CountryEntry& rCmp ) const
                                    { return rCmp.meCountry == meCountry; }
};

/** Predicate comparing a language type with the member of a CountryEntry.

    Compares by primary language only, if the passed CountryEntry allows it
    (the member mbUseSubLang is cleared), otherwise by full language type. */
struct CountryEntryPred_Language
{
    LanguageType                meLanguage;

    inline explicit             CountryEntryPred_Language( LanguageType eLanguage ) :
                                    meLanguage( eLanguage ) {}

    inline bool                 operator()( const CountryEntry& rCmp ) const;
};

inline bool CountryEntryPred_Language::operator()( const CountryEntry& rCmp ) const
{
    //  rCmp.mbUseSubLang==true  -> compare full language type
    //  rCmp.mbUseSubLang==false -> compare primary language only
    return rCmp.mbUseSubLang ? (meLanguage == rCmp.meLanguage) :
                ((meLanguage & 0x03FF) == (rCmp.meLanguage & 0x03FF));
}

} // namespace

// Country ID <-> Language type conversion ====================================

CountryId ConvertLanguageToCountry( LanguageType eLanguage )
{
    // country of a found primary language type
    CountryId ePrimCountry = COUNTRY_DONTKNOW;

    // find an exact match and a primary-language-only match, in one pass
    const CountryEntry* pEntry = pTable;
    do
    {
        pEntry = std::find_if( pEntry, pEnd, CountryEntryPred_Language( eLanguage ) );
        if( pEntry != pEnd )
        {
            if( pEntry->mbUseSubLang )
                return pEntry->meCountry;       // exact match found -> return
            if( ePrimCountry == COUNTRY_DONTKNOW )
                ePrimCountry = pEntry->meCountry;
            ++pEntry;   // one entry forward for next find_if() call
        }
    }
    while( pEntry != pEnd );

    return ePrimCountry;
}

LanguageType ConvertCountryToLanguage( CountryId eCountry )
{
    // just find the first occurrence of eCountry and return the language type
    const CountryEntry* pEntry = std::find_if( pTable, pEnd, CountryEntryPred_Country( eCountry ) );
    return (pEntry != pEnd) ? pEntry->meLanguage : LANGUAGE_DONTKNOW;
}

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
