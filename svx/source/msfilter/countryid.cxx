/*************************************************************************
 *
 *  $RCSfile: countryid.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-23 17:26:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef INCLUDED_SVX_COUNTRYID_HXX
#include "countryid.hxx"
#endif

#include <algorithm>

// ----------------------------------------------------------------------------

namespace svx {

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

#ifndef LANGUAGE_DIVEHI
#define LANGUAGE_DIVEHI 0x0465
#endif

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
    { COUNTRY_USA,                  LANGUAGE_ENGLISH_US                             },
    { COUNTRY_CARRIBEAN,            LANGUAGE_ENGLISH_CARRIBEAN,             true    },
    { COUNTRY_DOMINICAN_REPUBLIC,   LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,    true    },
    { COUNTRY_JAMAICA,              LANGUAGE_ENGLISH_JAMAICA,               true    },
    { COUNTRY_PUERTO_RICO,          LANGUAGE_SPANISH_PUERTO_RICO,           true    },
    { COUNTRY_TRINIDAD_Y_TOBAGO,    LANGUAGE_ENGLISH_TRINIDAD,              true    },
    { COUNTRY_CANADA,               LANGUAGE_ENGLISH_CAN,                   true    },
    { COUNTRY_CANADA,               LANGUAGE_FRENCH_CANADIAN,               true    },
    { COUNTRY_RUSSIA,               LANGUAGE_RUSSIAN                                },
    { COUNTRY_KAZACHSTAN,           LANGUAGE_KAZAK                                  },
    { COUNTRY_TATARSTAN,            LANGUAGE_TATAR                                  },
    { COUNTRY_TAJIKISTAN,           LANGUAGE_TAJIK                                  },
    { COUNTRY_UZBEKISTAN,           LANGUAGE_UZBEK                                  },
    { COUNTRY_EGYPT,                LANGUAGE_ARABIC_EGYPT,                  true    },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_AFRIKAANS                              },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_ENGLISH_SAFRICA,               true    },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_TSONGA                                 },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_VENDA                                  },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_XHOSA                                  },
    { COUNTRY_SOUTH_AFRICA,         LANGUAGE_ZULU                                   },
    { COUNTRY_BOTSWANA,             LANGUAGE_TSWANA                                 },
    { COUNTRY_LESOTHO,              LANGUAGE_SESOTHO                                },
    { COUNTRY_GREECE,               LANGUAGE_GREEK                                  },
    { COUNTRY_NETHERLANDS,          LANGUAGE_DUTCH                                  },
    { COUNTRY_NETHERLANDS,          LANGUAGE_FRISIAN_NETHERLANDS                    },
    { COUNTRY_BELGIUM,              LANGUAGE_DUTCH_BELGIAN,                 true    },
    { COUNTRY_BELGIUM,              LANGUAGE_FRENCH_BELGIAN,                true    },
    { COUNTRY_FRANCE,               LANGUAGE_FRENCH                                 },
    { COUNTRY_CAMEROON,             LANGUAGE_FRENCH_CAMEROON,               true    },
    { COUNTRY_COTE_D_IVOIRE,        LANGUAGE_FRENCH_COTE_D_IVOIRE,          true    },
    { COUNTRY_MALI,                 LANGUAGE_FRENCH_MALI,                   true    },
    { COUNTRY_REUNION,              LANGUAGE_FRENCH_REUNION,                true    },
    { COUNTRY_SENEGAL,              LANGUAGE_FRENCH_SENEGAL,                true    },
    { COUNTRY_WEST_INDIES,          LANGUAGE_FRENCH_WEST_INDIES,            true    },
    { COUNTRY_ZAIRE,                LANGUAGE_FRENCH_ZAIRE,                  true    },
    { COUNTRY_SPAIN,                LANGUAGE_SPANISH                                },
    { COUNTRY_SPAIN,                LANGUAGE_CATALAN                                },
    { COUNTRY_SPAIN,                LANGUAGE_BASQUE                                 },
    { COUNTRY_SPAIN,                LANGUAGE_GALICIAN                               },
    { COUNTRY_HUNGARY,              LANGUAGE_HUNGARIAN                              },
    { COUNTRY_ITALY,                LANGUAGE_ITALIAN                                },
    { COUNTRY_ROMANIA,              LANGUAGE_ROMANIAN                               },
    { COUNTRY_MOLDOVA,              LANGUAGE_ROMANIAN_MOLDOVA,              true    },
    { COUNTRY_MOLDOVA,              LANGUAGE_RUSSIAN_MOLDOVA,               true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_GERMAN_SWISS,                  true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_FRENCH_SWISS,                  true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_ITALIAN_SWISS,                 true    },
    { COUNTRY_SWITZERLAND,          LANGUAGE_RHAETO_ROMAN                           },
    { COUNTRY_LIECHTENSTEIN,        LANGUAGE_GERMAN_LIECHTENSTEIN,          true    },
    { COUNTRY_AUSTRIA,              LANGUAGE_GERMAN_AUSTRIAN,               true    },
    { COUNTRY_UNITED_KINGDOM,       LANGUAGE_ENGLISH_UK,                    true    },
    { COUNTRY_UNITED_KINGDOM,       LANGUAGE_GAELIC_SCOTLAND,               true    },
    { COUNTRY_UNITED_KINGDOM,       LANGUAGE_WELSH                                  },
    { COUNTRY_DENMARK,              LANGUAGE_DANISH                                 },
    { COUNTRY_SWEDEN,               LANGUAGE_SWEDISH                                },
    { COUNTRY_SWEDEN,               LANGUAGE_SAMI_LAPPISH                           },
    { COUNTRY_NORWAY,               LANGUAGE_NORWEGIAN_BOKMAL                       },
    { COUNTRY_POLAND,               LANGUAGE_POLISH                                 },
    { COUNTRY_GERMANY,              LANGUAGE_GERMAN                                 },
    { COUNTRY_GERMANY,              LANGUAGE_SORBIAN                                },
    { COUNTRY_PERU,                 LANGUAGE_SPANISH_PERU,                  true    },
    { COUNTRY_MEXICO,               LANGUAGE_SPANISH_MEXICAN,               true    },
    { COUNTRY_ARGENTINIA,           LANGUAGE_SPANISH_ARGENTINA,             true    },
    { COUNTRY_BRAZIL,               LANGUAGE_PORTUGUESE_BRAZILIAN,          true    },
    { COUNTRY_CHILE,                LANGUAGE_SPANISH_CHILE,                 true    },
    { COUNTRY_COLOMBIA,             LANGUAGE_SPANISH_COLOMBIA,              true    },
    { COUNTRY_VENEZUELA,            LANGUAGE_SPANISH_VENEZUELA,             true    },
    { COUNTRY_MALAYSIA,             LANGUAGE_MALAY_MALAYSIA                         },
    { COUNTRY_AUSTRALIA,            LANGUAGE_ENGLISH_AUS,                   true    },
    { COUNTRY_INDONESIA,            LANGUAGE_INDONESIAN                             },
    { COUNTRY_PHILIPPINES,          LANGUAGE_ENGLISH_PHILIPPINES,           true    },
    { COUNTRY_NEW_ZEALAND,          LANGUAGE_ENGLISH_NZ,                    true    },
    { COUNTRY_SINGAPORE,            LANGUAGE_CHINESE_SINGAPORE,             true    },
    { COUNTRY_THAILAND,             LANGUAGE_THAI                                   },
    { COUNTRY_CAMBODIA,             LANGUAGE_KHMER                                  },
    { COUNTRY_JAPAN,                LANGUAGE_JAPANESE                               },
    { COUNTRY_SOUTH_KOREA,          LANGUAGE_KOREAN                                 },
    { COUNTRY_VIET_NAM,             LANGUAGE_VIETNAMESE                             },
    { COUNTRY_PR_CHINA,             LANGUAGE_CHINESE_SIMPLIFIED                     },
    { COUNTRY_TIBET,                LANGUAGE_TIBETAN                                },
    { COUNTRY_TURKEY,               LANGUAGE_TURKISH                                },
    { COUNTRY_INDIA,                LANGUAGE_HINDI                                  },
    { COUNTRY_INDIA,                LANGUAGE_URDU_INDIA,                    true    },
    { COUNTRY_INDIA,                LANGUAGE_PUNJABI                                },
    { COUNTRY_INDIA,                LANGUAGE_GUJARATI                               },
    { COUNTRY_INDIA,                LANGUAGE_ORIYA                                  },
    { COUNTRY_INDIA,                LANGUAGE_TAMIL                                  },
    { COUNTRY_INDIA,                LANGUAGE_TELUGU                                 },
    { COUNTRY_INDIA,                LANGUAGE_KANNADA                                },
    { COUNTRY_INDIA,                LANGUAGE_MALAYALAM                              },
    { COUNTRY_INDIA,                LANGUAGE_ASSAMESE                               },
    { COUNTRY_INDIA,                LANGUAGE_MARATHI                                },
    { COUNTRY_INDIA,                LANGUAGE_SANSKRIT                               },
    { COUNTRY_INDIA,                LANGUAGE_KONKANI                                },
    { COUNTRY_INDIA,                LANGUAGE_MANIPURI                               },
    { COUNTRY_INDIA,                LANGUAGE_SINDHI                                 },
    { COUNTRY_INDIA,                LANGUAGE_KASHMIRI                               },
    { COUNTRY_BANGLADESH,           LANGUAGE_BENGALI                                },
    { COUNTRY_NEPAL,                LANGUAGE_NEPALI                                 },
    { COUNTRY_PAKISTAN,             LANGUAGE_URDU_PAKISTAN                          },
    { COUNTRY_MOROCCO,              LANGUAGE_ARABIC_MOROCCO,                true    },
    { COUNTRY_ALGERIA,              LANGUAGE_ARABIC_ALGERIA,                true    },
    { COUNTRY_TUNISIA,              LANGUAGE_ARABIC_TUNISIA,                true    },
    { COUNTRY_LIBYA,                LANGUAGE_ARABIC_LIBYA,                  true    },
    { COUNTRY_KENYA,                LANGUAGE_SWAHILI                                },
    { COUNTRY_ZIMBABWE,             LANGUAGE_ENGLISH_ZIMBABWE,              true    },
    { COUNTRY_FAEROE_ISLANDS,       LANGUAGE_FAEROESE                               },
    { COUNTRY_PORTUGAL,             LANGUAGE_PORTUGUESE                             },
    { COUNTRY_LUXEMBOURG,           LANGUAGE_GERMAN_LUXEMBOURG,             true    },
    { COUNTRY_LUXEMBOURG,           LANGUAGE_FRENCH_LUXEMBOURG,             true    },
    { COUNTRY_IRELAND,              LANGUAGE_ENGLISH_EIRE,                  true    },
    { COUNTRY_IRELAND,              LANGUAGE_GAELIC_IRELAND,                true    },
    { COUNTRY_ICELAND,              LANGUAGE_ICELANDIC                              },
    { COUNTRY_ALBANIA,              LANGUAGE_ALBANIAN                               },
    { COUNTRY_FINLAND,              LANGUAGE_FINNISH                                },
    { COUNTRY_FINLAND,              LANGUAGE_SWEDISH_FINLAND,               true    },
    { COUNTRY_BULGARIA,             LANGUAGE_BULGARIAN                              },
    { COUNTRY_LITHUANIA,            LANGUAGE_LITHUANIAN                             },
    { COUNTRY_LATVIA,               LANGUAGE_LATVIAN                                },
    { COUNTRY_ESTONIA,              LANGUAGE_ESTONIAN                               },
    { COUNTRY_ARMENIA,              LANGUAGE_ARMENIAN                               },
    { COUNTRY_BELARUS,              LANGUAGE_BELARUSIAN                             },
    { COUNTRY_MONACO,               LANGUAGE_FRENCH_MONACO,                 true    },
    { COUNTRY_UKRAINE,              LANGUAGE_UKRAINIAN                              },
    { COUNTRY_SERBIA,               LANGUAGE_SERBIAN_LATIN                          },
    { COUNTRY_CROATIA,              LANGUAGE_CROATIAN,                      true    },  // sub type of LANGUAGE_SERBIAN
    { COUNTRY_SLOVENIA,             LANGUAGE_SLOVENIAN                              },
    { COUNTRY_MACEDONIA,            LANGUAGE_MACEDONIAN                             },
    { COUNTRY_CZECH,                LANGUAGE_CZECH                                  },
    { COUNTRY_SLOVAK,               LANGUAGE_SLOVAK                                 },
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
    { COUNTRY_TAIWAN,               LANGUAGE_CHINESE_TRADITIONAL,           true    },
    { COUNTRY_MALEDIVES,            LANGUAGE_DIVEHI                                 },
    { COUNTRY_LEBANON,              LANGUAGE_ARABIC_LEBANON,                true    },
    { COUNTRY_JORDAN,               LANGUAGE_ARABIC_JORDAN,                 true    },
    { COUNTRY_SYRIA,                LANGUAGE_ARABIC_SYRIA,                  true    },
    { COUNTRY_IRAQ,                 LANGUAGE_ARABIC_IRAQ,                   true    },
    { COUNTRY_KUWAIT,               LANGUAGE_ARABIC_KUWAIT,                 true    },
    { COUNTRY_SAUDI_ARABIA,         LANGUAGE_ARABIC_SAUDI_ARABIA,           true    },
    { COUNTRY_YEMEN,                LANGUAGE_ARABIC_YEMEN,                  true    },
    { COUNTRY_OMAN,                 LANGUAGE_ARABIC_OMAN,                   true    },
    { COUNTRY_UAE,                  LANGUAGE_ARABIC_UAE,                    true    },
    { COUNTRY_ISRAEL,               LANGUAGE_HEBREW                                 },
    { COUNTRY_BAHRAIN,              LANGUAGE_ARABIC_BAHRAIN,                true    },
    { COUNTRY_QATAR,                LANGUAGE_ARABIC_QATAR,                  true    },
    { COUNTRY_MONGOLIA,             LANGUAGE_MONGOLIAN                              },
    { COUNTRY_IRAN,                 LANGUAGE_FARSI                                  },
    { COUNTRY_TURKMENISTAN,         LANGUAGE_TURKMEN                                },
    { COUNTRY_AZERBAIJAN,           LANGUAGE_AZERI_LATIN                            },
    { COUNTRY_GEORGIA,              LANGUAGE_GEORGIAN                               },
    { COUNTRY_KYRGYZSTAN,           LANGUAGE_KIRGHIZ                                }
};

const CountryEntry * const pEnd = pTable + sizeof( pTable ) / sizeof( pTable[ 0 ] );

// ----------------------------------------------------------------------------

/** Predicate comparing a country ID with the member of a CountryEntry. */
struct CountryEntryPred_Country
{
    CountryId                   meCountry;

    inline explicit             CountryEntryPred_Country( CountryId eCountry ) :
                                    meCountry( eCountry ) {}

    inline bool                 operator()( const CountryEntry& rCmp ) const
                                    { return rCmp.meCountry == meCountry; }
};

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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
    // just find the first occurance of eCountry and return the language type
    const CountryEntry* pEntry = std::find_if( pTable, pEnd, CountryEntryPred_Country( eCountry ) );
    return (pEntry != pEnd) ? pEntry->meLanguage : LANGUAGE_DONTKNOW;
}

// ============================================================================

} // namespace svx

// ============================================================================

