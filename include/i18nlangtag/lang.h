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

#ifndef INCLUDED_I18NLANGTAG_LANG_H
#define INCLUDED_I18NLANGTAG_LANG_H

/**
   These are MS LANGIDs, the lower 10 bits (mask 0x03ff, values below 0x0400
   aren't real locale IDs) represent the primary language ID, the upper 6 bits
   represent the sublanguage ID, which in most cases together with the primary
   language simply designates a country. A LANGID is constructed by
   UINT16 nLangID = ((((UINT16)(SubLangId)) << 10) | (UINT16)(PriLangId));

   A user-defined primary language ID is a value in the range 0x0200 to 0x03ff.
   All other values are reserved for Windows system use.

   A user-defined sublanguage identifier is a value in the range 0x20 to 0x3f.
   All other values are reserved for Windows system use.
   If there is no sublanguage ID for a primary language ID, use SUBLANG_DEFAULT
   (0x01, which shifted to the left by 10 bits results in the frequently seen
   0x0400). OR'ed with a 0x0200 primary results in 0x0600.

   Values added as of 2006-04-18, a helper script: ../../source/isolang/lcid.awk
   Utility to compare MS-LANGID definitions with those defined in this file.

   For further information about MS-LANGIDs please see include/winnt.h of a
   recent MSDEV version and the following web pages.


   The once complete list, not necessarily supported by Windows:
   List of Locale ID (LCID) Values as Assigned by Microsoft
   http://www.microsoft.com/globaldev/reference/lcid-all.mspx

   As a complete list is never complete, some more that came with WinXP SP2:
   Windows XP/Server 2003 - List of Locale IDs, Input Locale, and Language Collection
   http://www.microsoft.com/globaldev/reference/winxp/xp-lcid.mspx

   And of course 2 lists aren't enough, so Windows Vista needs an extra one.
   Which at least seems to include values of other versions of Windows.
   Language Identifier Constants and Strings
   http://msdn.microsoft.com/en-us/library/ms776294.aspx

 ! Hey, yet another list, maybe this one will not move around? It seems to be
 ! quite complete..
 ! Language Identifier Constants and Strings (Windows)
 ! http://msdn.microsoft.com/en-us/library/dd318693.aspx
 !
 ! Use THAT ^^^ as of 2013-09-17 it includes also Windows 8

 ! BUT, you can download a PDF document from
 ! http://msdn.microsoft.com/library/cc233965.aspx
 ! that has YET MORE definitions, sigh.. didn't cross-check if any are missing
 ! from that.. however, it also contains a few MS-reserved definitions that use
 ! ISO 639-3 codes reserved for local use, such as 'qps-ploc' and 'qps-ploca'
 ! (sic!), or strange things like 'ar-Ploc-SA' and 'ja-Ploc-JP' ('Ploc'??).
 !
 ! Use THAT ^^^ as the ultimate reference source (haha?) as of 2013-10-17 it
 ! includes also Windows 8.1 (Release: Monday, July 22, 2013; well, its table
 ! says 08/08/2013 Revision 6.0, but hey, who cares...)

   For completeness, you can never have enough lists:
   List of supported locale identifiers in Word
   Applies to Microsoft Word 2000 and 2002
   http://support.microsoft.com/default.aspx?scid=KB;en-us;q221435
 */

#include <sal/types.h>
#include <o3tl/strong_int.hxx>
#include <ostream>
struct LanguageTypeTag {};
typedef o3tl::strong_int<sal_uInt16, LanguageTypeTag> LanguageType;
inline std::ostream& operator<<(std::ostream& os, LanguageType const & lt) { os << sal_uInt16(lt); return os; }
constexpr LanguageType primary(LanguageType lt) { return LanguageType(sal_uInt16(lt) & 0x03ff); }

namespace o3tl
{
    // delete "sal_Int16" constructor via specialization: values > 0x7FFF are
    // actually used, and unfortunately passed around in the API as signed
    // "short", so use this to find all places where casts must be inserted
    template<> template<> constexpr strong_int<unsigned short,LanguageTypeTag>::strong_int(short, std::enable_if<std::is_integral<short>::value, int>::type) = delete;
}

#define LANGUAGE_MASK_PRIMARY 0x03ff

#define LANGUAGE_DONTKNOW                   LanguageType(0x03FF)  /* yes, the mask */
#define LANGUAGE_NONE                       LanguageType(0x00FF)
#define LANGUAGE_HID_HUMAN_INTERFACE_DEVICE LanguageType(0x04FF)
#define LANGUAGE_SYSTEM                     LanguageType(0x0000)  /* OOo/SO definition */

/* The Invariant Locale (Locale ID = 0x007f) is a locale that can be used by
 * applications when a consistent and locale-independent result is required.
 * The invariant locale can be used, for example, when comparing character
 * strings using the CompareString() API and a consistent result regardless of
 * the User Locale is expected.
 * The settings of the Invariant Locale are similar to US-English international
 * standards, but should not be used to display formatted data. */
/* NOTE: this is taken from the MS documentation! Not supported by OOo/SO! */
#define LANGUAGE_INVARIANT                  LanguageType(0x007F)

/* Naming conventions:
 * * ..._LSO suffix indicates a Language-Script-Only mapping, i.e. no country
 *   assignment in i18nlangtag/source/isolang/isolang.cxx.
 *   Used for backward compatibility or where MS assigned such values and they
 *   should be explicitly handled.
 * * LANGUAGE_USER_... for our user-defined IDs.
 * * LANGUAGE_OBSOLETE_USER_... for a user-defined ID that has been replaced by
 *   a value defined by MS.
 */

#define LANGUAGE_AFRIKAANS                  LanguageType(0x0436)
#define LANGUAGE_ALBANIAN                   LanguageType(0x041C)
#define LANGUAGE_ALSATIAN_FRANCE            LanguageType(0x0484)
#define LANGUAGE_AMHARIC_ETHIOPIA           LanguageType(0x045E)
#define LANGUAGE_ARABIC_ALGERIA             LanguageType(0x1401)
#define LANGUAGE_ARABIC_BAHRAIN             LanguageType(0x3C01)
#define LANGUAGE_ARABIC_EGYPT               LanguageType(0x0C01)
#define LANGUAGE_ARABIC_IRAQ                LanguageType(0x0801)
#define LANGUAGE_ARABIC_JORDAN              LanguageType(0x2C01)
#define LANGUAGE_ARABIC_KUWAIT              LanguageType(0x3401)
#define LANGUAGE_ARABIC_LEBANON             LanguageType(0x3001)
#define LANGUAGE_ARABIC_LIBYA               LanguageType(0x1001)
#define LANGUAGE_ARABIC_MOROCCO             LanguageType(0x1801)
#define LANGUAGE_ARABIC_OMAN                LanguageType(0x2001)
#define LANGUAGE_ARABIC_PRIMARY_ONLY        LanguageType(0x0001)  /* primary only, not a locale! */
#define LANGUAGE_ARABIC_QATAR               LanguageType(0x4001)
#define LANGUAGE_ARABIC_SAUDI_ARABIA        LanguageType(0x0401)
#define LANGUAGE_ARABIC_SYRIA               LanguageType(0x2801)
#define LANGUAGE_ARABIC_TUNISIA             LanguageType(0x1C01)
#define LANGUAGE_ARABIC_UAE                 LanguageType(0x3801)
#define LANGUAGE_ARABIC_YEMEN               LanguageType(0x2401)
#define LANGUAGE_ARMENIAN                   LanguageType(0x042B)
#define LANGUAGE_ASSAMESE                   LanguageType(0x044D)
#define LANGUAGE_AZERI_CYRILLIC             LanguageType(0x082C)
#define LANGUAGE_AZERI_CYRILLIC_LSO         LanguageType(0x742C)
#define LANGUAGE_AZERI_LATIN                LanguageType(0x042C)
#define LANGUAGE_AZERI_LATIN_LSO            LanguageType(0x782C)
#define LANGUAGE_BASHKIR_RUSSIA             LanguageType(0x046D)
#define LANGUAGE_BASQUE                     LanguageType(0x042D)
#define LANGUAGE_BELARUSIAN                 LanguageType(0x0423)
#define LANGUAGE_BENGALI                    LanguageType(0x0445)  /* in India */
#define LANGUAGE_BENGALI_BANGLADESH         LanguageType(0x0845)
#define LANGUAGE_BOSNIAN_CYRILLIC_LSO                   LanguageType(0x641A)
#define LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA    LanguageType(0x201A)
#define LANGUAGE_BOSNIAN_LATIN_LSO                      LanguageType(0x681A)
#define LANGUAGE_BOSNIAN_LSO                            LanguageType(0x781A)
#define LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA       LanguageType(0x141A)
#define LANGUAGE_BOSNIAN_BOSNIA_HERZEGOVINA             LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA   /* TODO: remove, only for langtab.src & localize.sdf compatibility */
#define LANGUAGE_BRETON_FRANCE              LanguageType(0x047E)  /* obsoletes LANGUAGE_USER_BRETON 0x0629 */
#define LANGUAGE_BULGARIAN                  LanguageType(0x0402)
#define LANGUAGE_BURMESE                    LanguageType(0x0455)
#define LANGUAGE_CATALAN                    LanguageType(0x0403)
#define LANGUAGE_CATALAN_VALENCIAN          LanguageType(0x0803)  /* obsoletes LANGUAGE_USER_CATALAN_VALENCIAN 0x8003 */
#define LANGUAGE_CHEROKEE_UNITED_STATES     LanguageType(0x045C)
#define LANGUAGE_CHEROKEE_CHEROKEE_LSO      LanguageType(0x7C5C)
#define LANGUAGE_CHINESE_HONGKONG           LanguageType(0x0C04)
#define LANGUAGE_CHINESE_LSO                LanguageType(0x7804)
#define LANGUAGE_CHINESE_MACAU              LanguageType(0x1404)
#define LANGUAGE_CHINESE_SIMPLIFIED         LanguageType(0x0804)
#define LANGUAGE_CHINESE_SINGAPORE          LanguageType(0x1004)
#define LANGUAGE_CHINESE_TRADITIONAL        LanguageType(0x0404)
#define LANGUAGE_CHINESE_SIMPLIFIED_LEGACY  LanguageType(0x0004)  /* MS-.NET 'zh-CHS', primary only! but maps to 'zh-CN' */
#define LANGUAGE_CHINESE_TRADITIONAL_LSO    LanguageType(0x7C04)  /* MS-.NET 'zh-CHT' but maps to 'zh-Hant' */
#define LANGUAGE_CHINESE                    LANGUAGE_CHINESE_SIMPLIFIED     /* most code uses LANGUAGE_CHINESE */
#define LANGUAGE_CORSICAN_FRANCE            LanguageType(0x0483)
#define LANGUAGE_CROATIAN                   LanguageType(0x041A)
#define LANGUAGE_CROATIAN_BOSNIA_HERZEGOVINA LanguageType(0x101A)
#define LANGUAGE_CZECH                      LanguageType(0x0405)
#define LANGUAGE_DANISH                     LanguageType(0x0406)
#define LANGUAGE_DARI_AFGHANISTAN           LanguageType(0x048C)  /* AKA Zoroastrian Dari */
#define LANGUAGE_DHIVEHI                    LanguageType(0x0465)  /* AKA Divehi */
#define LANGUAGE_DUTCH                      LanguageType(0x0413)
#define LANGUAGE_DUTCH_BELGIAN              LanguageType(0x0813)
#define LANGUAGE_EDO                        LanguageType(0x0466)
#define LANGUAGE_ENGLISH                    LanguageType(0x0009)  /* primary only, not a locale! */
#define LANGUAGE_ENGLISH_ARAB_EMIRATES      LanguageType(0x4C09)
#define LANGUAGE_ENGLISH_AUS                LanguageType(0x0C09)
#define LANGUAGE_ENGLISH_BAHRAIN            LanguageType(0x5009)
#define LANGUAGE_ENGLISH_BELIZE             LanguageType(0x2809)
#define LANGUAGE_ENGLISH_CAN                LanguageType(0x1009)
#define LANGUAGE_ENGLISH_CARRIBEAN          LanguageType(0x2409)
#define LANGUAGE_ENGLISH_EGYPT              LanguageType(0x5409)
#define LANGUAGE_ENGLISH_EIRE               LanguageType(0x1809)
#define LANGUAGE_ENGLISH_HONG_KONG_SAR      LanguageType(0x3C09)
#define LANGUAGE_ENGLISH_INDIA              LanguageType(0x4009)
#define LANGUAGE_ENGLISH_INDONESIA          LanguageType(0x3809)
#define LANGUAGE_ENGLISH_JAMAICA            LanguageType(0x2009)
#define LANGUAGE_ENGLISH_JORDAN             LanguageType(0x5809)
#define LANGUAGE_ENGLISH_KUWAIT             LanguageType(0x5C09)
#define LANGUAGE_ENGLISH_MALAYSIA           LanguageType(0x4409)
#define LANGUAGE_ENGLISH_NZ                 LanguageType(0x1409)
#define LANGUAGE_ENGLISH_PHILIPPINES        LanguageType(0x3409)
#define LANGUAGE_ENGLISH_SAFRICA            LanguageType(0x1C09)
#define LANGUAGE_ENGLISH_SINGAPORE          LanguageType(0x4809)
#define LANGUAGE_ENGLISH_TRINIDAD           LanguageType(0x2C09)
#define LANGUAGE_ENGLISH_TURKEY             LanguageType(0x6009)
#define LANGUAGE_ENGLISH_UK                 LanguageType(0x0809)
#define LANGUAGE_ENGLISH_US                 LanguageType(0x0409)
#define LANGUAGE_ENGLISH_YEMEN              LanguageType(0x6409)
#define LANGUAGE_ENGLISH_ZIMBABWE           LanguageType(0x3009)
#define LANGUAGE_ESTONIAN                   LanguageType(0x0425)
#define LANGUAGE_FAEROESE                   LanguageType(0x0438)
#define LANGUAGE_FARSI                      LanguageType(0x0429)
#define LANGUAGE_FILIPINO                   LanguageType(0x0464)
#define LANGUAGE_FINNISH                    LanguageType(0x040B)
#define LANGUAGE_FRENCH                     LanguageType(0x040C)
#define LANGUAGE_FRENCH_BELGIAN             LanguageType(0x080C)
#define LANGUAGE_FRENCH_CAMEROON            LanguageType(0x2C0C)
#define LANGUAGE_FRENCH_CANADIAN            LanguageType(0x0C0C)
#define LANGUAGE_FRENCH_COTE_D_IVOIRE       LanguageType(0x300C)

#define LANGUAGE_FRENCH_HAITI               LanguageType(0x3C0C)
#define LANGUAGE_FRENCH_LUXEMBOURG          LanguageType(0x140C)
#define LANGUAGE_FRENCH_MALI                LanguageType(0x340C)
#define LANGUAGE_FRENCH_MONACO              LanguageType(0x180C)
#define LANGUAGE_FRENCH_MOROCCO             LanguageType(0x380C)
#define LANGUAGE_FRENCH_NORTH_AFRICA        LanguageType(0xE40C)
#define LANGUAGE_FRENCH_REUNION             LanguageType(0x200C)
#define LANGUAGE_FRENCH_SENEGAL             LanguageType(0x280C)
#define LANGUAGE_FRENCH_SWISS               LanguageType(0x100C)
#define LANGUAGE_FRENCH_WEST_INDIES         LanguageType(0x1C0C)  /* MS in its MS-LCID.pdf now says "Neither defined nor reserved" */
#define LANGUAGE_FRENCH_ZAIRE               LanguageType(0x240C)
#define LANGUAGE_FRISIAN_NETHERLANDS        LanguageType(0x0462)
#define LANGUAGE_FULFULDE_LATIN_LSO         LanguageType(0x7C67)
#define LANGUAGE_FULFULDE_NIGERIA           LanguageType(0x0467)
#define LANGUAGE_FULFULDE_SENEGAL           LanguageType(0x0867)
#define LANGUAGE_GAELIC_IRELAND             LanguageType(0x083C)
#define LANGUAGE_GAELIC_SCOTLAND            LanguageType(0x0491)  /* apparently it occurred to MS that those are different languages */
#define LANGUAGE_GAELIC_SCOTLAND_LEGACY     LanguageType(0x043C)
#define LANGUAGE_GALICIAN                   LanguageType(0x0456)
#define LANGUAGE_GEORGIAN                   LanguageType(0x0437)
#define LANGUAGE_GERMAN                     LanguageType(0x0407)
#define LANGUAGE_GERMAN_AUSTRIAN            LanguageType(0x0C07)
#define LANGUAGE_GERMAN_LIECHTENSTEIN       LanguageType(0x1407)
#define LANGUAGE_GERMAN_LUXEMBOURG          LanguageType(0x1007)
#define LANGUAGE_GERMAN_SWISS               LanguageType(0x0807)
#define LANGUAGE_GREEK                      LanguageType(0x0408)
#define LANGUAGE_GUARANI_PARAGUAY           LanguageType(0x0474)
#define LANGUAGE_GUJARATI                   LanguageType(0x0447)
#define LANGUAGE_HAUSA_LATIN_LSO            LanguageType(0x7C68)
#define LANGUAGE_HAUSA_NIGERIA              LanguageType(0x0468)
#define LANGUAGE_HAWAIIAN_UNITED_STATES     LanguageType(0x0475)
#define LANGUAGE_HEBREW                     LanguageType(0x040D)
#define LANGUAGE_HINDI                      LanguageType(0x0439)
#define LANGUAGE_HUNGARIAN                  LanguageType(0x040E)
#define LANGUAGE_IBIBIO_NIGERIA             LanguageType(0x0469)
#define LANGUAGE_ICELANDIC                  LanguageType(0x040F)
#define LANGUAGE_IGBO_NIGERIA               LanguageType(0x0470)
#define LANGUAGE_INDONESIAN                 LanguageType(0x0421)
#define LANGUAGE_INUKTITUT_SYLLABICS_CANADA LanguageType(0x045D)
#define LANGUAGE_INUKTITUT_SYLLABICS_LSO    LanguageType(0x785D)
#define LANGUAGE_INUKTITUT_LATIN_CANADA     LanguageType(0x085D)
#define LANGUAGE_INUKTITUT_LATIN_LSO        LanguageType(0x7C5D)
#define LANGUAGE_ITALIAN                    LanguageType(0x0410)
#define LANGUAGE_ITALIAN_SWISS              LanguageType(0x0810)
#define LANGUAGE_JAPANESE                   LanguageType(0x0411)
#define LANGUAGE_KALAALLISUT_GREENLAND      LanguageType(0x046F)  /* obsoletes LANGUAGE_USER_KALAALLISUT 0x062A */
#define LANGUAGE_KANNADA                    LanguageType(0x044B)
#define LANGUAGE_KANURI_NIGERIA             LanguageType(0x0471)
#define LANGUAGE_KASHMIRI                   LanguageType(0x0460)
#define LANGUAGE_KASHMIRI_INDIA             LanguageType(0x0860)
#define LANGUAGE_KAZAKH                     LanguageType(0x043F)
#define LANGUAGE_KHMER                      LanguageType(0x0453)
#define LANGUAGE_KICHE_GUATEMALA            LanguageType(0x0486)  /* AKA K'iche', West Central Quiche,  */
#define LANGUAGE_KINYARWANDA_RWANDA         LanguageType(0x0487)  /* obsoletes LANGUAGE_USER_KINYARWANDA 0x0621 */
#define LANGUAGE_KIRGHIZ                    LanguageType(0x0440)  /* AKA Kyrgyz */
#define LANGUAGE_KONKANI                    LanguageType(0x0457)
#define LANGUAGE_KOREAN                     LanguageType(0x0412)
#define LANGUAGE_KOREAN_JOHAB               LanguageType(0x0812)  /* not mentioned in MS-LCID.pdf, oh joy */
#define LANGUAGE_KURDISH_ARABIC_IRAQ        LanguageType(0x0492)  /* obsoletes LANGUAGE_USER_KURDISH_IRAQ 0x0E26 */
#define LANGUAGE_KURDISH_ARABIC_LSO         LanguageType(0x7C92)
#define LANGUAGE_LAO                        LanguageType(0x0454)
#define LANGUAGE_LATIN_LSO                  LanguageType(0x0476)  /* obsoletes LANGUAGE_USER_LATIN 0x0610 */
#define LANGUAGE_LATVIAN                    LanguageType(0x0426)
#define LANGUAGE_LITHUANIAN                 LanguageType(0x0427)
#define LANGUAGE_LITHUANIAN_CLASSIC         LanguageType(0x0827)  /* MS in its MS-LCID.pdf now says "Neither defined nor reserved" */
#define LANGUAGE_LU_CHINA                   LanguageType(0x0490)
#define LANGUAGE_LUXEMBOURGISH_LUXEMBOURG   LanguageType(0x046E)  /* obsoletes LANGUAGE_USER_LUXEMBOURGISH 0x0630 */
#define LANGUAGE_MACEDONIAN                 LanguageType(0x042F)
#define LANGUAGE_MALAGASY_PLATEAU           LanguageType(0x048D)  /* obsoletes LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU */
#define LANGUAGE_MALAYALAM                  LanguageType(0x044C)  /* in India */
#define LANGUAGE_MALAY_BRUNEI_DARUSSALAM    LanguageType(0x083E)
#define LANGUAGE_MALAY_MALAYSIA             LanguageType(0x043E)
#define LANGUAGE_MALTESE                    LanguageType(0x043A)
#define LANGUAGE_MANIPURI                   LanguageType(0x0458)
#define LANGUAGE_MAORI_NEW_ZEALAND          LanguageType(0x0481)  /* obsoletes LANGUAGE_USER_MAORI 0x0620 */
#define LANGUAGE_MAPUDUNGUN_CHILE           LanguageType(0x047A)  /* AKA Araucanian */
#define LANGUAGE_MARATHI                    LanguageType(0x044E)
#define LANGUAGE_MOHAWK_CANADA              LanguageType(0x047C)
#define LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA    LanguageType(0x0450)
#define LANGUAGE_MONGOLIAN_CYRILLIC_LSO         LanguageType(0x7850)
#define LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA   LanguageType(0x0C50)
#define LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA      LanguageType(0x0850)
#define LANGUAGE_MONGOLIAN_MONGOLIAN_LSO        LanguageType(0x7C50)
#define LANGUAGE_NEPALI                     LanguageType(0x0461)
#define LANGUAGE_NEPALI_INDIA               LanguageType(0x0861)
#define LANGUAGE_NORWEGIAN                  LanguageType(0x0014)  /* primary only, not a locale! */
#define LANGUAGE_NORWEGIAN_BOKMAL           LanguageType(0x0414)
#define LANGUAGE_NORWEGIAN_BOKMAL_LSO       LanguageType(0x7C14)
#define LANGUAGE_NORWEGIAN_NYNORSK          LanguageType(0x0814)
#define LANGUAGE_NORWEGIAN_NYNORSK_LSO      LanguageType(0x7814)
#define LANGUAGE_OCCITAN_FRANCE             LanguageType(0x0482)  /* obsoletes LANGUAGE_USER_OCCITAN 0x0625 */
#define LANGUAGE_ODIA                       LanguageType(0x0448)
#define LANGUAGE_OROMO                      LanguageType(0x0472)
#define LANGUAGE_PAPIAMENTU                 LanguageType(0x0479)
#define LANGUAGE_PASHTO                     LanguageType(0x0463)
#define LANGUAGE_POLISH                     LanguageType(0x0415)
#define LANGUAGE_PORTUGUESE                 LanguageType(0x0816)
#define LANGUAGE_PORTUGUESE_BRAZILIAN       LanguageType(0x0416)
#define LANGUAGE_PUNJABI                    LanguageType(0x0446)
#define LANGUAGE_PUNJABI_ARABIC_LSO         LanguageType(0x7C46)
#define LANGUAGE_PUNJABI_PAKISTAN           LanguageType(0x0846)
#define LANGUAGE_QUECHUA_BOLIVIA            LanguageType(0x046B)
#define LANGUAGE_QUECHUA_COLOMBIA           LanguageType(0x0493)  /* different primary ID */
#define LANGUAGE_QUECHUA_ECUADOR            LanguageType(0x086B)
#define LANGUAGE_QUECHUA_PERU               LanguageType(0x0C6B)
#define LANGUAGE_RHAETO_ROMAN               LanguageType(0x0417)
#define LANGUAGE_ROMANIAN                   LanguageType(0x0418)
#define LANGUAGE_ROMANIAN_MOLDOVA           LanguageType(0x0818)
#define LANGUAGE_RUSSIAN                    LanguageType(0x0419)
#define LANGUAGE_RUSSIAN_MOLDOVA            LanguageType(0x0819)
#define LANGUAGE_SAMI_NORTHERN_NORWAY       LanguageType(0x043B)
#define LANGUAGE_SAMI_LAPPISH               LANGUAGE_SAMI_NORTHERN_NORWAY   /* the old MS definition */
#define LANGUAGE_SAMI_INARI                 LanguageType(0x243B)
#define LANGUAGE_SAMI_INARI_LSO             LanguageType(0x703B)
#define LANGUAGE_SAMI_LULE_LSO              LanguageType(0x7C3B)
#define LANGUAGE_SAMI_LULE_NORWAY           LanguageType(0x103B)
#define LANGUAGE_SAMI_LULE_SWEDEN           LanguageType(0x143B)
#define LANGUAGE_SAMI_NORTHERN_FINLAND      LanguageType(0x0C3B)
#define LANGUAGE_SAMI_NORTHERN_SWEDEN       LanguageType(0x083B)
#define LANGUAGE_SAMI_SKOLT                 LanguageType(0x203B)
#define LANGUAGE_SAMI_SKOLT_LSO             LanguageType(0x743B)
#define LANGUAGE_SAMI_SOUTHERN_LSO          LanguageType(0x783B)
#define LANGUAGE_SAMI_SOUTHERN_NORWAY       LanguageType(0x183B)
#define LANGUAGE_SAMI_SOUTHERN_SWEDEN       LanguageType(0x1C3B)
#define LANGUAGE_SANSKRIT                   LanguageType(0x044F)
#define LANGUAGE_SEPEDI                     LanguageType(0x046C)
#define LANGUAGE_NORTHERNSOTHO              LANGUAGE_SEPEDI /* just an alias for the already existing localization */
#define LANGUAGE_SERBIAN_CYRILLIC_LSO       LanguageType(0x6C1A)
#define LANGUAGE_SERBIAN_CYRILLIC_SAM       LanguageType(0x0C1A)  /* Serbia and Montenegro (former) */
#define LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA    LanguageType(0x1C1A)
#define LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO LanguageType(0x301A)
#define LANGUAGE_SERBIAN_CYRILLIC_SERBIA    LanguageType(0x281A)
#define LANGUAGE_SERBIAN_LATIN_LSO          LanguageType(0x701A)
#define LANGUAGE_SERBIAN_LATIN_SAM          LanguageType(0x081A)
#define LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA       LanguageType(0x181A)
#define LANGUAGE_SERBIAN_LATIN_MONTENEGRO   LanguageType(0x2C1A)
#define LANGUAGE_SERBIAN_LATIN_NEUTRAL      LanguageType(0x7C1A)
#define LANGUAGE_SERBIAN_LATIN_SERBIA       LanguageType(0x241A)
#define LANGUAGE_SESOTHO                    LanguageType(0x0430)  /* also called Sutu now by MS */
#define LANGUAGE_SINDHI                     LanguageType(0x0459)
#define LANGUAGE_SINDHI_ARABIC_LSO          LanguageType(0x7C59)
#define LANGUAGE_SINDHI_PAKISTAN            LanguageType(0x0859)
#define LANGUAGE_SINHALESE_SRI_LANKA        LanguageType(0x045B)
#define LANGUAGE_SLOVAK                     LanguageType(0x041B)
#define LANGUAGE_SLOVENIAN                  LanguageType(0x0424)
#define LANGUAGE_SOMALI                     LanguageType(0x0477)
#define LANGUAGE_UPPER_SORBIAN_GERMANY      LanguageType(0x042E)  /* obsoletes LANGUAGE_USER_UPPER_SORBIAN 0x0623 */
#define LANGUAGE_LOWER_SORBIAN_GERMANY      LanguageType(0x082E)  /* obsoletes LANGUAGE_USER_LOWER_SORBIAN 0x0624. NOTE: the primary ID is identical to Upper Sorbian, which is not quite correct because they're distinct languages */
#define LANGUAGE_LOWER_SORBIAN_LSO          LanguageType(0x7C2E)
#define LANGUAGE_SORBIAN                    LANGUAGE_USER_UPPER_SORBIAN /* a strange MS definition */
#define LANGUAGE_SPANISH_DATED              LanguageType(0x040A)  /* old collation, not supported, see #i94435# */
#define LANGUAGE_SPANISH_ARGENTINA          LanguageType(0x2C0A)
#define LANGUAGE_SPANISH_BOLIVIA            LanguageType(0x400A)
#define LANGUAGE_SPANISH_CHILE              LanguageType(0x340A)
#define LANGUAGE_SPANISH_COLOMBIA           LanguageType(0x240A)
#define LANGUAGE_SPANISH_COSTARICA          LanguageType(0x140A)
#define LANGUAGE_SPANISH_CUBA               LanguageType(0x5C0A)
#define LANGUAGE_SPANISH_DOMINICAN_REPUBLIC LanguageType(0x1C0A)
#define LANGUAGE_SPANISH_ECUADOR            LanguageType(0x300A)
#define LANGUAGE_SPANISH_EL_SALVADOR        LanguageType(0x440A)
#define LANGUAGE_SPANISH_GUATEMALA          LanguageType(0x100A)
#define LANGUAGE_SPANISH_HONDURAS           LanguageType(0x480A)
#define LANGUAGE_SPANISH_LATIN_AMERICA      LanguageType(0xE40A)  /* no locale possible */
#define LANGUAGE_SPANISH_MEXICAN            LanguageType(0x080A)
#define LANGUAGE_SPANISH_MODERN             LanguageType(0x0C0A)
#define LANGUAGE_SPANISH_NICARAGUA          LanguageType(0x4C0A)
#define LANGUAGE_SPANISH_PANAMA             LanguageType(0x180A)
#define LANGUAGE_SPANISH_PARAGUAY           LanguageType(0x3C0A)
#define LANGUAGE_SPANISH_PERU               LanguageType(0x280A)
#define LANGUAGE_SPANISH_PUERTO_RICO        LanguageType(0x500A)
#define LANGUAGE_SPANISH_UNITED_STATES      LanguageType(0x540A)
#define LANGUAGE_SPANISH_URUGUAY            LanguageType(0x380A)
#define LANGUAGE_SPANISH_VENEZUELA          LanguageType(0x200A)
#define LANGUAGE_SPANISH                    LANGUAGE_SPANISH_MODERN     /* modern collation, see #i94435# */
#define LANGUAGE_SWAHILI                    LanguageType(0x0441)  /* Kenya */
#define LANGUAGE_SWEDISH                    LanguageType(0x041D)
#define LANGUAGE_SWEDISH_FINLAND            LanguageType(0x081D)
#define LANGUAGE_SYRIAC                     LanguageType(0x045A)
#define LANGUAGE_TAI_NUA_CHINA              LanguageType(0x048F)
#define LANGUAGE_TAJIK                      LanguageType(0x0428)
#define LANGUAGE_TAJIK_LSO                  LanguageType(0x7C28)
#define LANGUAGE_TAMAZIGHT_ARABIC_MOROCCO   LanguageType(0x045F)
#define LANGUAGE_TAMAZIGHT_LATIN_ALGERIA    LanguageType(0x085F)
#define LANGUAGE_TAMAZIGHT_LATIN_LSO        LanguageType(0x7C5F)
#define LANGUAGE_TAMAZIGHT_MOROCCO          LanguageType(0x0C5F)
#define LANGUAGE_TAMAZIGHT_TIFINAGH_MOROCCO LanguageType(0x105F)
#define LANGUAGE_TAMAZIGHT_TIFINAGH_LSO     LanguageType(0x785F)
#define LANGUAGE_TAMIL                      LanguageType(0x0449)
#define LANGUAGE_TAMIL_SRI_LANKA            LanguageType(0x0849)
#define LANGUAGE_TATAR                      LanguageType(0x0444)
#define LANGUAGE_TELUGU                     LanguageType(0x044A)
#define LANGUAGE_THAI                       LanguageType(0x041E)
#define LANGUAGE_TIBETAN                    LanguageType(0x0451)
#define LANGUAGE_TIBETAN_BHUTAN             LanguageType(0x0851)  /* was used as Dzongkha, a MS error, see #i53497# */
#define LANGUAGE_DZONGKHA_BHUTAN            LanguageType(0x0C51)  /* they finally got it ... ([MS-LCID] rev. 7.0 2015-06-30) */
#define LANGUAGE_TIGRIGNA_ERITREA           LanguageType(0x0873)
#define LANGUAGE_TIGRIGNA_ETHIOPIA          LanguageType(0x0473)
#define LANGUAGE_TSONGA                     LanguageType(0x0431)
#define LANGUAGE_TSWANA                     LanguageType(0x0432)  /* AKA Setsuana, for South Africa */
#define LANGUAGE_TSWANA_BOTSWANA            LanguageType(0x0832)  /* obsoletes LANGUAGE_USER_TSWANA_BOTSWANA 0x8032 */
#define LANGUAGE_TURKISH                    LanguageType(0x041F)
#define LANGUAGE_TURKMEN                    LanguageType(0x0442)
#define LANGUAGE_UIGHUR_CHINA               LanguageType(0x0480)
#define LANGUAGE_UKRAINIAN                  LanguageType(0x0422)
#define LANGUAGE_URDU_INDIA                 LanguageType(0x0820)
#define LANGUAGE_URDU_PAKISTAN              LanguageType(0x0420)
#define LANGUAGE_UZBEK_CYRILLIC             LanguageType(0x0843)
#define LANGUAGE_UZBEK_CYRILLIC_LSO         LanguageType(0x7843)
#define LANGUAGE_UZBEK_LATIN                LanguageType(0x0443)
#define LANGUAGE_UZBEK_LATIN_LSO            LanguageType(0x7C43)
#define LANGUAGE_VENDA                      LanguageType(0x0433)
#define LANGUAGE_VIETNAMESE                 LanguageType(0x042A)
#define LANGUAGE_WELSH                      LanguageType(0x0452)
#define LANGUAGE_WOLOF_SENEGAL              LanguageType(0x0488)
#define LANGUAGE_XHOSA                      LanguageType(0x0434)  /* AKA isiZhosa */
#define LANGUAGE_YAKUT_RUSSIA               LanguageType(0x0485)
#define LANGUAGE_YI                         LanguageType(0x0478)  /* Sichuan Yi */
#define LANGUAGE_YIDDISH                    LanguageType(0x043D)
#define LANGUAGE_YORUBA                     LanguageType(0x046A)
#define LANGUAGE_YUE_CHINESE_HONGKONG       LanguageType(0x048E)
#define LANGUAGE_ZULU                       LanguageType(0x0435)

#define LANGUAGE_qps_ploc                   LanguageType(0x0501)  /* 'qps-ploc', qps is a reserved for local use code */
#define LANGUAGE_qps_ploca                  LanguageType(0x05FE)  /* 'qps-ploca', qps is a reserved for local use code */
#define LANGUAGE_qps_plocm                  LanguageType(0x09FF)  /* 'qps-plocm', qps is a reserved for local use code */

#define LANGUAGE_ar_Ploc_SA_reserved        LanguageType(0x4401)  /* 'ar-Ploc-SA', 'Ploc'?? */
#define LANGUAGE_ja_Ploc_JP_reserved        LanguageType(0x0811)  /* 'ja-Ploc-JP', 'Ploc'?? */
#define LANGUAGE_pap_029_reserved           LanguageType(0x0479)  /* 'pap-029' */
#define LANGUAGE_ar_145_reserved            LanguageType(0x4801)  /* 'ar-145' */
#define LANGUAGE_es_419                     LanguageType(0x580A)  /* 'es-419', not reserved, used? */

/* Seems these values were used or reserved at one point of time ... */
#define LANGUAGE_Neither_defined_nor_reserved_0x007B LanguageType(0x007B)
#define LANGUAGE_Neither_defined_nor_reserved_0x007D LanguageType(0x007D)
#define LANGUAGE_Neither_defined_nor_reserved_0x0089 LanguageType(0x0089)
#define LANGUAGE_Neither_defined_nor_reserved_0x008A LanguageType(0x008A)
#define LANGUAGE_Neither_defined_nor_reserved_0x008B LanguageType(0x008B)
#define LANGUAGE_Neither_defined_nor_reserved_0x008D LanguageType(0x008D)
#define LANGUAGE_Neither_defined_nor_reserved_0x008E LanguageType(0x008E)
#define LANGUAGE_Neither_defined_nor_reserved_0x008F LanguageType(0x008F)
#define LANGUAGE_Neither_defined_nor_reserved_0x0090 LanguageType(0x0090)
#define LANGUAGE_Neither_defined_nor_reserved_0x0827 LanguageType(0x0827)
#define LANGUAGE_Neither_defined_nor_reserved_0x1C0C LanguageType(0x1C0C)
#define LANGUAGE_Neither_defined_nor_reserved_0x2008 LanguageType(0x2008)

/*! use only for import/export of MS documents, number formatter maps it to
 *! LANGUAGE_SYSTEM and then to effective system language */
#define LANGUAGE_SYSTEM_DEFAULT             LanguageType(0x0800)

/*! use only for import/export of MS documents, number formatter maps it to
 *! LANGUAGE_SYSTEM and then to effective system language */
#define LANGUAGE_PROCESS_OR_USER_DEFAULT    LanguageType(0x0400)


/* And now the extensions we define,
 * with primary language IDs from 0x200 to 0x3FF valid in blocks from
 *
 * 0x0610 to 0x07FF with sublanguage ID 0x01 (default)
 * 0x0A00 to 0x0BFF with sublanguage ID 0x02
 * ...
 * 0x8200 to 0x83FF with sublanguage ID 0x20
 * 0x8600 to 0x87FF with sublanguage ID 0x21
 * ...
 * 0xFA00 to 0xFBFF with sublanguage ID 0x3E
 * 0xFE00 to 0xFFFF with sublanguage ID 0x3F
 *
 * Of which we define reserved blocks:
 * All primary language IDs 0x3E0 to 0x3FE with any sublanguage ID, not to be
 * assigned as user-defined IDs:
 *
 * 0x07E0 to 0x07FE with sublanguage ID 0x01, first on-the-fly block
 * 0x0BE0 to 0x0BFE with sublanguage ID 0x02, second on-the-fly block
 * ...
 * 0x83E0 to 0x83FE with sublanguage ID 0x20
 * 0x87E0 to 0x87FE with sublanguage ID 0x21
 * ...
 * 0xFBE0 to 0xFBFE with sublanguage ID 0x3E, last on-the-fly block
 * 0xFFE0 to 0xFFFE with sublanguage ID 0x3F, 32 privateuse and specials
 *
 * This leaves room for 479 user-defined primary language IDs (instead of 512)
 * with primary bitmask comparability, and 1922 on-the-fly IDs (instead of 0)
 * with arbitrary assignment ((0x3E - 0x01 + 1) * (0x3FE - 0x3E0 + 1)).
 *
 * The primary ID 0x3FF is reserved and unassigned as it is the same as the
 * LANGUAGE_MASK_PRIMARY and LANGUAGE_DONTKNOW value, so to not be mistaken as
 * such.
 *
 * Obsolete OOo user defines now have other values assigned by MS, and
 * different name. Mapping an obsolete value to ISO code should work provided
 * that such a mapping exists in i18nlangtag/source/isolang/isolang.cxx, but
 * mapping ISO back to LANGID will return the new value.
 */
#define LANGUAGE_OBSOLETE_USER_LATIN        LanguageType(0x0610)
#define LANGUAGE_USER_LATIN                 LANGUAGE_LATIN_LSO
#define LANGUAGE_USER_LATIN_VATICAN         LanguageType(0x8076)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_LATIN_LSO)) */
#define LANGUAGE_USER_ESPERANTO             LanguageType(0x0611)  /* no locale possible */
#define LANGUAGE_USER_INTERLINGUA           LanguageType(0x0612)  /* no locale, but conventions */
#define LANGUAGE_OBSOLETE_USER_MAORI        LanguageType(0x0620)
#define LANGUAGE_USER_MAORI                 LANGUAGE_MAORI_NEW_ZEALAND
#define LANGUAGE_OBSOLETE_USER_KINYARWANDA  LanguageType(0x0621)
#define LANGUAGE_USER_KINYARWANDA           LANGUAGE_KINYARWANDA_RWANDA
/* was reserved for Northern Sotho but never used: 0x0622 */  /* obsoleted by LANGUAGE_SEPEDI */
#define LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN LanguageType(0x0623)
#define LANGUAGE_USER_UPPER_SORBIAN         LANGUAGE_UPPER_SORBIAN_GERMANY
#define LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN LanguageType(0x0624)
#define LANGUAGE_USER_LOWER_SORBIAN         LANGUAGE_LOWER_SORBIAN_GERMANY
#define LANGUAGE_OBSOLETE_USER_OCCITAN      LanguageType(0x0625)
#define LANGUAGE_USER_OCCITAN               LANGUAGE_OCCITAN_FRANCE /* reserved to languedocian */

#define LANGUAGE_USER_KOREAN_NORTH          LanguageType(0x8012)  /* North Korean as opposed to South Korean, makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_KOREAN)) */
#define LANGUAGE_USER_KURDISH_TURKEY        LanguageType(0x0626)  /* sublang 0x01, Latin script */
#define LANGUAGE_USER_KURDISH_SYRIA         LanguageType(0x0A26)  /* sublang 0x02, Latin script */
#define LANGUAGE_OBSOLETE_USER_KURDISH_IRAQ LanguageType(0x0E26)  /* sublang 0x03, Arabic script */
#define LANGUAGE_USER_KURDISH_IRAQ          LANGUAGE_KURDISH_ARABIC_IRAQ
#define LANGUAGE_USER_KURDISH_IRAN          LanguageType(0x1226)  /* sublang 0x04, Arabic script */
#define LANGUAGE_USER_KURDISH_SOUTHERN_IRAQ LanguageType(0x8092)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_KURDISH_ARABIC_LSO)) */
#define LANGUAGE_USER_KURDISH_SOUTHERN_IRAN LanguageType(0x8492)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_KURDISH_ARABIC_LSO)) */
#define LANGUAGE_USER_SARDINIAN             LanguageType(0x0627)
/* was reserved for Dzongkha but turned down with #i53497#: 0x0628 */  /* obsoleted by LANGUAGE_DZONGKHA_BHUTAN */
#define LANGUAGE_USER_DZONGKHA_MAP_LONLY    LanguageType(0xF851)  /* to map "dz" only, because of the MS error, and preserve CTL information, sub 0x3e */
#define LANGUAGE_USER_SWAHILI_TANZANIA      LanguageType(0x8041)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SWAHILI)) */
#define LANGUAGE_OBSOLETE_USER_BRETON       LanguageType(0x0629)
#define LANGUAGE_USER_BRETON                LANGUAGE_BRETON_FRANCE
#define LANGUAGE_OBSOLETE_USER_KALAALLISUT  LanguageType(0x062A)
#define LANGUAGE_USER_KALAALLISUT           LANGUAGE_KALAALLISUT_GREENLAND
#define LANGUAGE_USER_SWAZI                 LanguageType(0x062B)
#define LANGUAGE_USER_NDEBELE_SOUTH         LanguageType(0x062C)
#define LANGUAGE_OBSOLETE_USER_TSWANA_BOTSWANA LanguageType(0x8032)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_TSWANA)) */
#define LANGUAGE_USER_TSWANA_BOTSWANA       LANGUAGE_TSWANA_BOTSWANA
#define LANGUAGE_USER_MOORE                 LanguageType(0x062D)
#define LANGUAGE_USER_BAMBARA               LanguageType(0x062E)
#define LANGUAGE_USER_AKAN                  LanguageType(0x062F)
#define LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH LanguageType(0x0630)
#define LANGUAGE_USER_LUXEMBOURGISH         LANGUAGE_LUXEMBOURGISH_LUXEMBOURG
#define LANGUAGE_USER_FRIULIAN              LanguageType(0x0631)
#define LANGUAGE_USER_FIJIAN                LanguageType(0x0632)
#define LANGUAGE_USER_AFRIKAANS_NAMIBIA     LanguageType(0x8036)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_AFRIKAANS)) */
#define LANGUAGE_USER_ENGLISH_NAMIBIA       LanguageType(0x8009)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
#define LANGUAGE_USER_WALLOON               LanguageType(0x0633)
#define LANGUAGE_USER_COPTIC                LanguageType(0x0634)
#define LANGUAGE_USER_CHUVASH               LanguageType(0x0635)
#define LANGUAGE_USER_GASCON                LanguageType(0x0636)  /* Gascon France */
#define LANGUAGE_USER_GERMAN_BELGIUM        LanguageType(0x8007)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_GERMAN)) */
#define LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN    LanguageType(0x8003)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_CATALAN)) */
#define LANGUAGE_USER_CATALAN_VALENCIAN     LANGUAGE_CATALAN_VALENCIAN
#define LANGUAGE_USER_HAUSA_GHANA           LanguageType(0x8068)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_HAUSA_NIGERIA)) */
#define LANGUAGE_USER_EWE_GHANA             LanguageType(0x0637)
#define LANGUAGE_USER_ENGLISH_GHANA         LanguageType(0x8409)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
#define LANGUAGE_USER_TAGALOG               LanguageType(0x0638)
#define LANGUAGE_USER_LINGALA_DRCONGO       LanguageType(0x0639)
#define LANGUAGE_USER_SANGO                 LanguageType(0x063A)
#define LANGUAGE_USER_GANDA                 LanguageType(0x063B)
#define LANGUAGE_USER_LOW_GERMAN            LanguageType(0x063C)
#define LANGUAGE_USER_HILIGAYNON            LanguageType(0x063D)
#define LANGUAGE_USER_NYANJA                LanguageType(0x063E)
#define LANGUAGE_USER_KASHUBIAN             LanguageType(0x063F)
#define LANGUAGE_OBSOLETE_USER_SPANISH_CUBA LanguageType(0x800A)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SPANISH)) */
#define LANGUAGE_USER_SPANISH_CUBA          LANGUAGE_SPANISH_CUBA
#define LANGUAGE_USER_TETUN                 LanguageType(0x0640)
#define LANGUAGE_USER_QUECHUA_NORTH_BOLIVIA LanguageType(0x0641)
#define LANGUAGE_USER_QUECHUA_SOUTH_BOLIVIA LanguageType(0x0642)
#define LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_SERBIA      LanguageType(0x8C1A)  /* makeLangID( 0x20+0x03, getPrimaryLanguage( LANGUAGE_SERBIAN_CYRILLIC_LSO)) */
#define LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA               LANGUAGE_SERBIAN_CYRILLIC_SERBIA
#define LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_SERBIA         LanguageType(0x881A)  /* makeLangID( 0x20+0x02, getPrimaryLanguage( LANGUAGE_SERBIAN_LATIN_LSO)) */
#define LANGUAGE_USER_SERBIAN_LATIN_SERBIA                  LANGUAGE_SERBIAN_LATIN_SERBIA
#define LANGUAGE_OBSOLETE_USER_SERBIAN_CYRILLIC_MONTENEGRO  LanguageType(0xCC1A)  /* makeLangID( 0x20+0x13, getPrimaryLanguage( LANGUAGE_SERBIAN_CYRILLIC_LSO)) */
#define LANGUAGE_USER_SERBIAN_CYRILLIC_MONTENEGRO           LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO
#define LANGUAGE_OBSOLETE_USER_SERBIAN_LATIN_MONTENEGRO     LanguageType(0xC81A)  /* makeLangID( 0x20+0x12, getPrimaryLanguage( LANGUAGE_SERBIAN_LATIN_LSO)) */
#define LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO              LANGUAGE_SERBIAN_LATIN_MONTENEGRO
#define LANGUAGE_USER_SAMI_KILDIN_RUSSIA    LanguageType(0x803B)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SAMI_NORTHERN_NORWAY)) */
#define LANGUAGE_USER_BODO_INDIA            LanguageType(0x0643)
#define LANGUAGE_USER_DOGRI_INDIA           LanguageType(0x0644)
#define LANGUAGE_USER_MAITHILI_INDIA        LanguageType(0x0645)
#define LANGUAGE_USER_SANTALI_INDIA         LanguageType(0x0646)
#define LANGUAGE_USER_TETUN_TIMOR_LESTE     LanguageType(0x0A40)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_TETUN)) */
#define LANGUAGE_USER_TOK_PISIN             LanguageType(0x0647)
#define LANGUAGE_USER_SHUSWAP               LanguageType(0x0648)
#define LANGUAGE_USER_ARABIC_CHAD           LanguageType(0x8001)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_COMOROS        LanguageType(0x8401)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_DJIBOUTI       LanguageType(0x8801)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_ERITREA        LanguageType(0x8C01)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_ISRAEL         LanguageType(0x9001)  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_MAURITANIA     LanguageType(0x9401)  /* makeLangID( 0x25, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_PALESTINE      LanguageType(0x9801)  /* makeLangID( 0x26, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_SOMALIA        LanguageType(0x9C01)  /* makeLangID( 0x27, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_SUDAN          LanguageType(0xA001)  /* makeLangID( 0x28, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ANCIENT_GREEK         LanguageType(0x0649)
#define LANGUAGE_USER_ASTURIAN              LanguageType(0x064A)
#define LANGUAGE_USER_LATGALIAN             LanguageType(0x064B)
#define LANGUAGE_USER_MAORE                 LanguageType(0x064C)
#define LANGUAGE_USER_BUSHI                 LanguageType(0x064D)
#define LANGUAGE_USER_TAHITIAN              LanguageType(0x064E)
#define LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU LanguageType(0x064F)
#define LANGUAGE_USER_PAPIAMENTU_ARUBA      LanguageType(0x8079)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
#define LANGUAGE_USER_SARDINIAN_CAMPIDANESE LanguageType(0x0650)
#define LANGUAGE_USER_SARDINIAN_GALLURESE   LanguageType(0x0651)
#define LANGUAGE_USER_SARDINIAN_LOGUDORESE  LanguageType(0x0652)
#define LANGUAGE_USER_SARDINIAN_SASSARESE   LanguageType(0x0653)
#define LANGUAGE_USER_BAFIA                 LanguageType(0x0654)
#define LANGUAGE_USER_GIKUYU                LanguageType(0x0655)
#define LANGUAGE_USER_RUSYN_UKRAINE         LanguageType(0x0656)
#define LANGUAGE_USER_RUSYN_SLOVAKIA        LanguageType(0x8256)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_RUSYN_UKRAINE)) */
#define LANGUAGE_USER_YIDDISH_US            LanguageType(0x803D)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_YIDDISH)) */
#define LANGUAGE_USER_LIMBU                 LanguageType(0x0657)
#define LANGUAGE_USER_LOJBAN                LanguageType(0x0658)  /* no locale */
#define LANGUAGE_OBSOLETE_USER_KABYLE       LanguageType(0x0659)
#define LANGUAGE_USER_KABYLE                LANGUAGE_TAMAZIGHT_LATIN_ALGERIA
#define LANGUAGE_USER_HAITIAN               LanguageType(0x065A)
#define LANGUAGE_USER_BEEMBE                LanguageType(0x065B)
#define LANGUAGE_USER_BEKWEL                LanguageType(0x065C)
#define LANGUAGE_USER_KITUBA                LanguageType(0x065D)
#define LANGUAGE_USER_LARI                  LanguageType(0x065E)
#define LANGUAGE_USER_MBOCHI                LanguageType(0x065F)
#define LANGUAGE_USER_TEKE_IBALI            LanguageType(0x0660)
#define LANGUAGE_USER_TEKE_TYEE             LanguageType(0x0661)
#define LANGUAGE_USER_VILI                  LanguageType(0x0662)
#define LANGUAGE_USER_PORTUGUESE_ANGOLA     LanguageType(0x8016)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_PORTUGUESE)) */
#define LANGUAGE_USER_MANX                  LanguageType(0x0663)
#define LANGUAGE_USER_TEKE_EBOO             LanguageType(0x0664)
#define LANGUAGE_USER_ARAGONESE             LanguageType(0x0665)
#define LANGUAGE_USER_KEYID                 LanguageType(0x0666)  /* key id pseudolanguage */
#define LANGUAGE_USER_PALI_LATIN            LanguageType(0x0667)
#define LANGUAGE_USER_KYRGYZ_CHINA          LanguageType(0x0668)  /* not derived from LANGUAGE_KIRGHIZ as these may be different scripts, see http://www.omniglot.com/writing/kirghiz.htm */
#define LANGUAGE_USER_KOMI_ZYRIAN           LanguageType(0x0669)
#define LANGUAGE_USER_KOMI_PERMYAK          LanguageType(0x066A)
#define LANGUAGE_USER_PITJANTJATJARA        LanguageType(0x066B)
#define LANGUAGE_USER_ENGLISH_MALAWI        LanguageType(0x8809)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_ERZYA                 LanguageType(0x066C)
#define LANGUAGE_USER_MARI_MEADOW           LanguageType(0x066D)
#define LANGUAGE_USER_KHANTY                LanguageType(0x066E)
#define LANGUAGE_USER_LIVONIAN              LanguageType(0x066F)
#define LANGUAGE_USER_MOKSHA                LanguageType(0x0670)
#define LANGUAGE_USER_MARI_HILL             LanguageType(0x0671)
#define LANGUAGE_USER_NGANASAN              LanguageType(0x0672)
#define LANGUAGE_USER_OLONETS               LanguageType(0x0673)
#define LANGUAGE_USER_VEPS                  LanguageType(0x0674)
#define LANGUAGE_USER_VORO                  LanguageType(0x0675)
#define LANGUAGE_USER_NENETS                LanguageType(0x0676)
#define LANGUAGE_USER_PAPIAMENTU_CURACAO    LanguageType(0x8479)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
#define LANGUAGE_USER_PAPIAMENTU_BONAIRE    LanguageType(0x8879)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
#define LANGUAGE_USER_AKA                   LanguageType(0x0677)
#define LANGUAGE_USER_AKA_CONGO             LanguageType(0x8277)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_AKA)) */
#define LANGUAGE_USER_DIBOLE                LanguageType(0x0678)
#define LANGUAGE_USER_DOONDO                LanguageType(0x0679)
#define LANGUAGE_USER_KAAMBA                LanguageType(0x067A)
#define LANGUAGE_USER_KOONGO                LanguageType(0x067B)
#define LANGUAGE_USER_KOONGO_CONGO          LanguageType(0x827B)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_KOONGO)) */
#define LANGUAGE_USER_KUNYI                 LanguageType(0x067C)
#define LANGUAGE_USER_NGUNGWEL              LanguageType(0x067D)
#define LANGUAGE_USER_NJYEM                 LanguageType(0x067E)
#define LANGUAGE_USER_NJYEM_CONGO           LanguageType(0x827E)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_NJYEM)) */
#define LANGUAGE_USER_PUNU                  LanguageType(0x067F)
#define LANGUAGE_USER_PUNU_CONGO            LanguageType(0x827F)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_PUNU)) */
#define LANGUAGE_USER_SUUNDI                LanguageType(0x0680)
#define LANGUAGE_USER_TEKE_KUKUYA           LanguageType(0x0681)
#define LANGUAGE_USER_TSAANGI               LanguageType(0x0682)
#define LANGUAGE_USER_YAKA                  LanguageType(0x0683)
#define LANGUAGE_USER_YOMBE                 LanguageType(0x0684)
#define LANGUAGE_USER_YOMBE_CONGO           LanguageType(0x8284)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_YOMBE)) */
#define LANGUAGE_USER_SIDAMA                LanguageType(0x0685)
#define LANGUAGE_USER_NKO                   LanguageType(0x0686)
#define LANGUAGE_USER_UDMURT                LanguageType(0x0687)
#define LANGUAGE_USER_TIBETAN_INDIA         LanguageType(0x8051)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_TIBETAN)) */
#define LANGUAGE_USER_CORNISH               LanguageType(0x0688)
#define LANGUAGE_USER_SAMI_PITE_SWEDEN      LanguageType(0x843B)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_SAMI_NORTHERN_NORWAY)) */
#define LANGUAGE_USER_NGAEBERE              LanguageType(0x0689)
#define LANGUAGE_USER_ENGLISH_UK_OED        LanguageType(0x8C09)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_KUMYK                 LanguageType(0x068A)
#define LANGUAGE_USER_NOGAI                 LanguageType(0x068B)
#define LANGUAGE_USER_KARAKALPAK_LATIN      LanguageType(0x068C)
#define LANGUAGE_USER_LADIN                 LanguageType(0x068D)
#define LANGUAGE_USER_TIBETAN_BHUTAN        LanguageType(0x8451)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_TIBETAN)) */
#define LANGUAGE_USER_FRENCH_BURKINA_FASO   LanguageType(0x800C)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_FRENCH)) */
#define LANGUAGE_USER_PUINAVE               LanguageType(0x068E)
#define LANGUAGE_USER_MANINKAKAN_EASTERN_LATIN  LanguageType(0x068F)
#define LANGUAGE_USER_AVAR                  LanguageType(0x0690)
#define LANGUAGE_USER_CREE_PLAINS_LATIN     LanguageType(0x0691)
#define LANGUAGE_USER_CREE_PLAINS_SYLLABICS LanguageType(0x8291)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_CREE_PLAINS_LATIN)) */
#define LANGUAGE_USER_LENGO                 LanguageType(0x0692)
#define LANGUAGE_USER_FRENCH_BENIN          LanguageType(0x840C)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_FRENCH)) */
#define LANGUAGE_USER_FRENCH_NIGER          LanguageType(0x880C)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_FRENCH)) */
#define LANGUAGE_USER_FRENCH_TOGO           LanguageType(0x8C0C)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_FRENCH)) */
#define LANGUAGE_USER_KVEN_FINNISH          LanguageType(0x0693)
#define LANGUAGE_USER_CHURCH_SLAVIC         LanguageType(0x0694)
#define LANGUAGE_USER_VENETIAN              LanguageType(0x0695)
#define LANGUAGE_USER_ENGLISH_GAMBIA        LanguageType(0x9009)  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_OCCITAN_ARANESE       LanguageType(0x8082)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_OCCITAN_FRANCE)) */
#define LANGUAGE_USER_ENGLISH_UK_OXENDICT   LanguageType(0x9409)  /* makeLangID( 0x25, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_ARPITAN_FRANCE        LanguageType(0x0696)
#define LANGUAGE_USER_ARPITAN_ITALY         LanguageType(0x0A96)  /* makeLangID( 0x02, getPrimaryLanguage( LANGUAGE_USER_ARPITAN_FRANCE)) */
#define LANGUAGE_USER_ARPITAN_SWITZERLAND   LanguageType(0x0E96)  /* makeLangID( 0x03, getPrimaryLanguage( LANGUAGE_USER_ARPITAN_FRANCE)) */
#define LANGUAGE_USER_ENGLISH_BOTSWANA      LanguageType(0x9809)  /* makeLangID( 0x26, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_INTERLINGUE           LanguageType(0x0697)  /* no locale */
#define LANGUAGE_USER_APATANI               LanguageType(0x0698)
#define LANGUAGE_USER_ENGLISH_MAURITIUS     LanguageType(0x9C09)  /* makeLangID( 0x27, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_FRENCH_MAURITIUS      LanguageType(0x900C)  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_FRENCH)) */
#define LANGUAGE_USER_SILESIAN              LanguageType(0x0699)
#define LANGUAGE_USER_HUNGARIAN_ROVAS       LanguageType(0x800E)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_HUNGARIAN)) */
#define LANGUAGE_USER_MANCHU                LanguageType(0x069A)
#define LANGUAGE_USER_XIBE                  LanguageType(0x069B)
#define LANGUAGE_USER_KITUBA_DRCONGO        LanguageType(0x069C)


/* XXX Add new user defined LCIDs ^^^ there.
 * Note that we run out of IDs with sublanguage ID 0x01 after 0x07DF ... */


/* Primary language ID range for on-the-fly assignment. */
#define LANGUAGE_ON_THE_FLY_START           LanguageType(0x03E0)
#define LANGUAGE_ON_THE_FLY_END             LanguageType(0x03FE)
/* Sublanguage ID range for on-the-fly assignment. */
#define LANGUAGE_ON_THE_FLY_SUB_START       LanguageType(0x01)
#define LANGUAGE_ON_THE_FLY_SUB_END         LanguageType(0x3E)

/* 0xFFE0 to 0xFFFF reserved for privateuse and specials. */
#define LANGUAGE_USER_PRIV_JOKER            LanguageType(0xFFEB)  /* privateuse "*" (sic! bad! nasty!), primary 0x3eb, sub 0x3f */
#define LANGUAGE_USER_PRIV_COMMENT          LanguageType(0xFFEC)  /* privateuse "x-comment", primary 0x3ec, sub 0x3f */
#define LANGUAGE_USER_PRIV_DEFAULT          LanguageType(0xFFED)  /* privateuse "x-default", primary 0x3ed, sub 0x3f */
#define LANGUAGE_USER_PRIV_NOTRANSLATE      LanguageType(0xFFEE)  /* privateuse "x-no-translate" (sic!), primary 0x3ee, sub 0x3f */
#define LANGUAGE_MULTIPLE                   LanguageType(0xFFEF)  /* multiple languages, primary 0x3ef, sub 0x3f */
#define LANGUAGE_UNDETERMINED               LanguageType(0xFFF0)  /* undetermined language, primary 0x3f0, sub 0x3f */
#define LANGUAGE_USER_SYSTEM_CONFIG         LanguageType(0xFFFE)  /* not a locale, to be used only in configuration context to obtain system default, primary 0x3fe, sub 0x3f */


#endif /* INCLUDED_I18NLANGTAG_LANG_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
