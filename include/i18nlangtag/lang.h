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
   http://msdn2.microsoft.com/en-us/library/ms776294.aspx

   Hey, yet another list, maybe this one will not move around? It seems to be
   quite complete..
   Language Identifier Constants and Strings (Windows)
   http://msdn.microsoft.com/en-us/library/dd318693(VS.85).aspx

   List of supported locale identifiers in Word
   http://support.microsoft.com/default.aspx?scid=KB;en-us;q221435
 */


/* It must be safe to include this file in plain C code, so only C style
 * comments are used. Do NOT use // C++ style comments. */

/* disable typedef for usage in svtools/source/misc/langtab.src */
#ifndef RSC_RESOURCE_USAGE
typedef unsigned short LanguageType;
#endif

#define LANGUAGE_MASK_PRIMARY 0x03ff

#ifdef __cplusplus
/* Please use the methods provided in mslangid.hxx for type-safety! */
#else
#define MSLANGID_MAKELANGID( nSubLangId, nPriLangId )  \
   (((nSubLangId) << 10) | (nPriLangId))
#define MSLANGID_GETPRIMARYLANGUAGE( nLangID )         \
    ((nLangID) & LANGUAGE_MASK_PRIMARY)
#define MSLANGID_GETSUBLANGUAGE( nLangID )             \
    (((nLangID) & ~LANGUAGE_MASK_PRIMARY) >> 10)
#endif


#define LANGUAGE_DONTKNOW                   0x03FF  /* yes, the mask */
#define LANGUAGE_NONE                       0x00FF
#define LANGUAGE_HID_HUMAN_INTERFACE_DEVICE 0x04FF
#define LANGUAGE_SYSTEM                     0x0000  /* OOo/SO definition */

/* The Invariant Locale (Locale ID = 0x007f) is a locale that can be used by
 * applications when a consistent and locale-independent result is required.
 * The invariant locale can be used, for example, when comparing character
 * strings using the CompareString() API and a consistent result regardless of
 * the User Locale is expected.
 * The settings of the Invariant Locale are similar to US-English international
 * standards, but should not be used to display formatted data. */
/* NOTE: this is taken from the MS documentation! Not supported by OOo/SO! */
#define LANGUAGE_INVARIANT                  0x007F

#define LANGUAGE_AFRIKAANS                  0x0436
#define LANGUAGE_ALBANIAN                   0x041C
#define LANGUAGE_ALSATIAN_FRANCE            0x0484
#define LANGUAGE_AMHARIC_ETHIOPIA           0x045E
#define LANGUAGE_ARABIC_ALGERIA             0x1401
#define LANGUAGE_ARABIC_BAHRAIN             0x3C01
#define LANGUAGE_ARABIC_EGYPT               0x0C01
#define LANGUAGE_ARABIC_IRAQ                0x0801
#define LANGUAGE_ARABIC_JORDAN              0x2C01
#define LANGUAGE_ARABIC_KUWAIT              0x3401
#define LANGUAGE_ARABIC_LEBANON             0x3001
#define LANGUAGE_ARABIC_LIBYA               0x1001
#define LANGUAGE_ARABIC_MOROCCO             0x1801
#define LANGUAGE_ARABIC_OMAN                0x2001
#define LANGUAGE_ARABIC_QATAR               0x4001
#define LANGUAGE_ARABIC_SAUDI_ARABIA        0x0401
#define LANGUAGE_ARABIC_SYRIA               0x2801
#define LANGUAGE_ARABIC_TUNISIA             0x1C01
#define LANGUAGE_ARABIC_UAE                 0x3801
#define LANGUAGE_ARABIC_YEMEN               0x2401
#define LANGUAGE_ARABIC_PRIMARY_ONLY        0x0001  /* primary only, not a locale! */
#define LANGUAGE_ARMENIAN                   0x042B
#define LANGUAGE_ASSAMESE                   0x044D
#define LANGUAGE_AZERI_CYRILLIC             0x082C
#define LANGUAGE_AZERI_LATIN                0x042C
#define LANGUAGE_BASHKIR_RUSSIA             0x046D
#define LANGUAGE_BASQUE                     0x042D
#define LANGUAGE_BELARUSIAN                 0x0423
#define LANGUAGE_BENGALI                    0x0445  /* in India */
#define LANGUAGE_BENGALI_BANGLADESH         0x0845
#define LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA       0x141A
#define LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA    0x201A
#define LANGUAGE_BOSNIAN_BOSNIA_HERZEGOVINA     LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA   /* TODO: remove, only for langtab.src & localize.sdf compatibility */
#define LANGUAGE_BRETON_FRANCE              0x047E  /* obsoletes LANGUAGE_USER_BRETON 0x0629 */
#define LANGUAGE_BULGARIAN                  0x0402
#define LANGUAGE_BURMESE                    0x0455
#define LANGUAGE_CATALAN                    0x0403
#define LANGUAGE_CATALAN_VALENCIAN          0x0803
#define LANGUAGE_CHEROKEE_UNITED_STATES     0x045C
#define LANGUAGE_CHINESE_HONGKONG           0x0C04
#define LANGUAGE_CHINESE_MACAU              0x1404
#define LANGUAGE_CHINESE_SIMPLIFIED         0x0804
#define LANGUAGE_CHINESE_SINGAPORE          0x1004
#define LANGUAGE_CHINESE_TRADITIONAL        0x0404
#define LANGUAGE_CHINESE_SIMPLIFIED_LEGACY  0x0004  /* MS-.NET 'zh-CHS', primary only! but maps to 'zh-CN' */
#define LANGUAGE_CHINESE_TRADITIONAL_LEGACY 0x7C04  /* MS-.NET 'zh-CHT' */
#define LANGUAGE_CHINESE                    LANGUAGE_CHINESE_SIMPLIFIED     /* most code uses LANGUAGE_CHINESE */
#define LANGUAGE_CORSICAN_FRANCE            0x0483
#define LANGUAGE_CROATIAN                   0x041A
#define LANGUAGE_CROATIAN_BOSNIA_HERZEGOVINA 0x101A
#define LANGUAGE_CZECH                      0x0405
#define LANGUAGE_DANISH                     0x0406
#define LANGUAGE_DARI_AFGHANISTAN           0x048C  /* AKA Zoroastrian Dari */
#define LANGUAGE_DHIVEHI                    0x0465  /* AKA Divehi */
#define LANGUAGE_DUTCH                      0x0413
#define LANGUAGE_DUTCH_BELGIAN              0x0813
#define LANGUAGE_EDO                        0x0466
#define LANGUAGE_ENGLISH                    0x0009  /* primary only, not a locale! */
#define LANGUAGE_ENGLISH_AUS                0x0C09
#define LANGUAGE_ENGLISH_BELIZE             0x2809
#define LANGUAGE_ENGLISH_CAN                0x1009
#define LANGUAGE_ENGLISH_CARRIBEAN          0x2409
#define LANGUAGE_ENGLISH_EIRE               0x1809
#define LANGUAGE_ENGLISH_HONG_KONG_SAR      0x3C09
#define LANGUAGE_ENGLISH_INDIA              0x4009
#define LANGUAGE_ENGLISH_INDONESIA          0x3809
#define LANGUAGE_ENGLISH_JAMAICA            0x2009
#define LANGUAGE_ENGLISH_MALAYSIA           0x4409
#define LANGUAGE_ENGLISH_NZ                 0x1409
#define LANGUAGE_ENGLISH_PHILIPPINES        0x3409
#define LANGUAGE_ENGLISH_SAFRICA            0x1C09
#define LANGUAGE_ENGLISH_SINGAPORE          0x4809
#define LANGUAGE_ENGLISH_TRINIDAD           0x2C09
#define LANGUAGE_ENGLISH_UK                 0x0809
#define LANGUAGE_ENGLISH_US                 0x0409
#define LANGUAGE_ENGLISH_ZIMBABWE           0x3009
#define LANGUAGE_ESTONIAN                   0x0425
#define LANGUAGE_FAEROESE                   0x0438
#define LANGUAGE_FARSI                      0x0429
#define LANGUAGE_FILIPINO                   0x0464
#define LANGUAGE_FINNISH                    0x040B
#define LANGUAGE_FRENCH                     0x040C
#define LANGUAGE_FRENCH_BELGIAN             0x080C
#define LANGUAGE_FRENCH_CAMEROON            0x2C0C
#define LANGUAGE_FRENCH_CANADIAN            0x0C0C
#define LANGUAGE_FRENCH_COTE_D_IVOIRE       0x300C

#define LANGUAGE_FRENCH_HAITI               0x3C0C
#define LANGUAGE_FRENCH_LUXEMBOURG          0x140C
#define LANGUAGE_FRENCH_MALI                0x340C
#define LANGUAGE_FRENCH_MONACO              0x180C
#define LANGUAGE_FRENCH_MOROCCO             0x380C
#define LANGUAGE_FRENCH_NORTH_AFRICA        0xE40C
#define LANGUAGE_FRENCH_REUNION             0x200C
#define LANGUAGE_FRENCH_SENEGAL             0x280C
#define LANGUAGE_FRENCH_SWISS               0x100C
#define LANGUAGE_FRENCH_WEST_INDIES         0x1C0C
#define LANGUAGE_FRENCH_ZAIRE               0x240C
#define LANGUAGE_FRISIAN_NETHERLANDS        0x0462
#define LANGUAGE_FULFULDE_NIGERIA           0x0467
#define LANGUAGE_GAELIC_IRELAND             0x083C
#define LANGUAGE_GAELIC_SCOTLAND            0x043C
#define LANGUAGE_GALICIAN                   0x0456
#define LANGUAGE_GEORGIAN                   0x0437
#define LANGUAGE_GERMAN                     0x0407
#define LANGUAGE_GERMAN_AUSTRIAN            0x0C07
#define LANGUAGE_GERMAN_LIECHTENSTEIN       0x1407
#define LANGUAGE_GERMAN_LUXEMBOURG          0x1007
#define LANGUAGE_GERMAN_SWISS               0x0807
#define LANGUAGE_GREEK                      0x0408
#define LANGUAGE_GUARANI_PARAGUAY           0x0474
#define LANGUAGE_GUJARATI                   0x0447
#define LANGUAGE_HAUSA_NIGERIA              0x0468
#define LANGUAGE_HAWAIIAN_UNITED_STATES     0x0475
#define LANGUAGE_HEBREW                     0x040D
#define LANGUAGE_HINDI                      0x0439
#define LANGUAGE_HUNGARIAN                  0x040E
#define LANGUAGE_IBIBIO_NIGERIA             0x0469
#define LANGUAGE_ICELANDIC                  0x040F
#define LANGUAGE_IGBO_NIGERIA               0x0470
#define LANGUAGE_INDONESIAN                 0x0421
#define LANGUAGE_INUKTITUT_SYLLABICS_CANADA 0x045D
#define LANGUAGE_INUKTITUT_LATIN_CANADA     0x085D
#define LANGUAGE_ITALIAN                    0x0410
#define LANGUAGE_ITALIAN_SWISS              0x0810
#define LANGUAGE_JAPANESE                   0x0411
#define LANGUAGE_KALAALLISUT_GREENLAND      0x046F  /* obsoletes LANGUAGE_USER_KALAALLISUT 0x062A */
#define LANGUAGE_KANNADA                    0x044B
#define LANGUAGE_KANURI_NIGERIA             0x0471
#define LANGUAGE_KASHMIRI                   0x0460
#define LANGUAGE_KASHMIRI_INDIA             0x0860
#define LANGUAGE_KAZAKH                     0x043F
#define LANGUAGE_KHMER                      0x0453
#define LANGUAGE_KICHE_GUATEMALA            0x0486  /* AKA K'iche', West Central Quiche,  */
#define LANGUAGE_KINYARWANDA_RWANDA         0x0487  /* obsoletes LANGUAGE_USER_KINYARWANDA 0x0621 */
#define LANGUAGE_KIRGHIZ                    0x0440  /* AKA Kyrgyz */
#define LANGUAGE_KONKANI                    0x0457
#define LANGUAGE_KOREAN                     0x0412
#define LANGUAGE_KOREAN_JOHAB               0x0812
#define LANGUAGE_LAO                        0x0454
#define LANGUAGE_LATIN                      0x0476  /* obsoletes LANGUAGE_USER_LATIN 0x0610 */
#define LANGUAGE_LATVIAN                    0x0426
#define LANGUAGE_LITHUANIAN                 0x0427
#define LANGUAGE_LITHUANIAN_CLASSIC         0x0827
#define LANGUAGE_LUXEMBOURGISH_LUXEMBOURG   0x046E  /* obsoletes LANGUAGE_USER_LUXEMBOURGISH 0x0630 */
#define LANGUAGE_MACEDONIAN                 0x042F
#define LANGUAGE_MALAYALAM                  0x044C  /* in India */
#define LANGUAGE_MALAY_BRUNEI_DARUSSALAM    0x083E
#define LANGUAGE_MALAY_MALAYSIA             0x043E
#define LANGUAGE_MALTESE                    0x043A
#define LANGUAGE_MANIPURI                   0x0458
#define LANGUAGE_MAORI_NEW_ZEALAND          0x0481  /* obsoletes LANGUAGE_USER_MAORI 0x0620 */
#define LANGUAGE_MAPUDUNGUN_CHILE           0x047A  /* AKA Araucanian */
#define LANGUAGE_MARATHI                    0x044E
#define LANGUAGE_MOHAWK_CANADA              0x047C
#define LANGUAGE_MONGOLIAN                  0x0450  /* Cyrillic script */
#define LANGUAGE_MONGOLIAN_MONGOLIAN        0x0850
#define LANGUAGE_NEPALI                     0x0461
#define LANGUAGE_NEPALI_INDIA               0x0861
#define LANGUAGE_NORWEGIAN                  0x0014  /* primary only, not a locale! */
#define LANGUAGE_NORWEGIAN_BOKMAL           0x0414
#define LANGUAGE_NORWEGIAN_NYNORSK          0x0814
#define LANGUAGE_OCCITAN_FRANCE             0x0482  /* obsoletes LANGUAGE_USER_OCCITAN 0x0625 */
#define LANGUAGE_ORIYA                      0x0448
#define LANGUAGE_OROMO                      0x0472
#define LANGUAGE_PAPIAMENTU                 0x0479
#define LANGUAGE_PASHTO                     0x0463
#define LANGUAGE_POLISH                     0x0415
#define LANGUAGE_PORTUGUESE                 0x0816
#define LANGUAGE_PORTUGUESE_BRAZILIAN       0x0416
#define LANGUAGE_PUNJABI                    0x0446
#define LANGUAGE_PUNJABI_PAKISTAN           0x0846
#define LANGUAGE_QUECHUA_BOLIVIA            0x046B
#define LANGUAGE_QUECHUA_ECUADOR            0x086B
#define LANGUAGE_QUECHUA_PERU               0x0C6B
#define LANGUAGE_RHAETO_ROMAN               0x0417
#define LANGUAGE_ROMANIAN                   0x0418
#define LANGUAGE_ROMANIAN_MOLDOVA           0x0818
#define LANGUAGE_RUSSIAN                    0x0419
#define LANGUAGE_RUSSIAN_MOLDOVA            0x0819
#define LANGUAGE_SAMI_NORTHERN_NORWAY       0x043B
#define LANGUAGE_SAMI_LAPPISH               LANGUAGE_SAMI_NORTHERN_NORWAY   /* the old MS definition */
#define LANGUAGE_SAMI_INARI                 0x243B
#define LANGUAGE_SAMI_LULE_NORWAY           0x103B
#define LANGUAGE_SAMI_LULE_SWEDEN           0x143B
#define LANGUAGE_SAMI_NORTHERN_FINLAND      0x0C3B
#define LANGUAGE_SAMI_NORTHERN_SWEDEN       0x083B
#define LANGUAGE_SAMI_SKOLT                 0x203B
#define LANGUAGE_SAMI_SOUTHERN_NORWAY       0x183B
#define LANGUAGE_SAMI_SOUTHERN_SWEDEN       0x1C3B
#define LANGUAGE_SANSKRIT                   0x044F
#define LANGUAGE_SEPEDI                     0x046C
#define LANGUAGE_NORTHERNSOTHO              LANGUAGE_SEPEDI /* just an alias for the already existing localization */
#define LANGUAGE_SERBIAN                    0x001A  /* primary only, not a locale! */
#define LANGUAGE_SERBIAN_CYRILLIC           0x0C1A  /* MS lists this as Serbian (Cyrillic, Serbia) 'sr-Cyrl-SP', but they use 'SP' since at least Windows2003 where it was Serbia and Montenegro! */
#define LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA    0x1C1A
#define LANGUAGE_SERBIAN_LATIN              0x081A  /* MS lists this as Serbian (Latin, Serbia) 'sr-Latn-SP', but they use 'SP' since at least Windows2003 where it was Serbia and Montenegro! */
#define LANGUAGE_SERBIAN_LATIN_BOSNIA_HERZEGOVINA       0x181A
#define LANGUAGE_SERBIAN_LATIN_NEUTRAL      0x7C1A  /* MS lists this as 'sr' only. What a mess. */
#define LANGUAGE_SESOTHO                    0x0430  /* also called Sutu now by MS */
#define LANGUAGE_SINDHI                     0x0459
#define LANGUAGE_SINDHI_PAKISTAN            0x0859
#define LANGUAGE_SINHALESE_SRI_LANKA        0x045B
#define LANGUAGE_SLOVAK                     0x041B
#define LANGUAGE_SLOVENIAN                  0x0424
#define LANGUAGE_SOMALI                     0x0477
#define LANGUAGE_UPPER_SORBIAN_GERMANY      0x042E  /* obsoletes LANGUAGE_USER_UPPER_SORBIAN 0x0623 */
#define LANGUAGE_LOWER_SORBIAN_GERMANY      0x082E  /* obsoletes LANGUAGE_USER_LOWER_SORBIAN 0x0624. NOTE: the primary ID is identical to Upper Sorbian, which is not quite correct because they're distinct languages */
#define LANGUAGE_SORBIAN                    LANGUAGE_USER_UPPER_SORBIAN /* a strange MS definition */
#define LANGUAGE_SPANISH_DATED              0x040A  /* old collation, not supported, see #i94435# */
#define LANGUAGE_SPANISH_ARGENTINA          0x2C0A
#define LANGUAGE_SPANISH_BOLIVIA            0x400A
#define LANGUAGE_SPANISH_CHILE              0x340A
#define LANGUAGE_SPANISH_COLOMBIA           0x240A
#define LANGUAGE_SPANISH_COSTARICA          0x140A
#define LANGUAGE_SPANISH_DOMINICAN_REPUBLIC 0x1C0A
#define LANGUAGE_SPANISH_ECUADOR            0x300A
#define LANGUAGE_SPANISH_EL_SALVADOR        0x440A
#define LANGUAGE_SPANISH_GUATEMALA          0x100A
#define LANGUAGE_SPANISH_HONDURAS           0x480A
#define LANGUAGE_SPANISH_LATIN_AMERICA      0xE40A  /* no locale possible */
#define LANGUAGE_SPANISH_MEXICAN            0x080A
#define LANGUAGE_SPANISH_MODERN             0x0C0A
#define LANGUAGE_SPANISH_NICARAGUA          0x4C0A
#define LANGUAGE_SPANISH_PANAMA             0x180A
#define LANGUAGE_SPANISH_PARAGUAY           0x3C0A
#define LANGUAGE_SPANISH_PERU               0x280A
#define LANGUAGE_SPANISH_PUERTO_RICO        0x500A
#define LANGUAGE_SPANISH_UNITED_STATES      0x540A
#define LANGUAGE_SPANISH_URUGUAY            0x380A
#define LANGUAGE_SPANISH_VENEZUELA          0x200A
#define LANGUAGE_SPANISH                    LANGUAGE_SPANISH_MODERN     /* modern collation, see #i94435# */
#define LANGUAGE_SWAHILI                    0x0441  /* Kenya */
#define LANGUAGE_SWEDISH                    0x041D
#define LANGUAGE_SWEDISH_FINLAND            0x081D
#define LANGUAGE_SYRIAC                     0x045A
#define LANGUAGE_TAJIK                      0x0428
#define LANGUAGE_TAMAZIGHT_ARABIC           0x045F
#define LANGUAGE_TAMAZIGHT_LATIN            0x085F
#define LANGUAGE_TAMAZIGHT_TIFINAGH         0x0C5F
#define LANGUAGE_TAMIL                      0x0449
#define LANGUAGE_TATAR                      0x0444
#define LANGUAGE_TELUGU                     0x044A
#define LANGUAGE_THAI                       0x041E
#define LANGUAGE_TIBETAN                    0x0451
#define LANGUAGE_DZONGKHA                   0x0851
#define LANGUAGE_TIBETAN_BHUTAN             LANGUAGE_DZONGKHA   /* a MS error, see #i53497# */
#define LANGUAGE_TIGRIGNA_ERITREA           0x0873
#define LANGUAGE_TIGRIGNA_ETHIOPIA          0x0473
#define LANGUAGE_TSONGA                     0x0431
#define LANGUAGE_TSWANA                     0x0432  /* AKA Setsuana, for South Africa */
#define LANGUAGE_TURKISH                    0x041F
#define LANGUAGE_TURKMEN                    0x0442
#define LANGUAGE_UIGHUR_CHINA               0x0480
#define LANGUAGE_UKRAINIAN                  0x0422
#define LANGUAGE_URDU_INDIA                 0x0820
#define LANGUAGE_URDU_PAKISTAN              0x0420
#define LANGUAGE_UZBEK_CYRILLIC             0x0843
#define LANGUAGE_UZBEK_LATIN                0x0443
#define LANGUAGE_VENDA                      0x0433
#define LANGUAGE_VIETNAMESE                 0x042A
#define LANGUAGE_WELSH                      0x0452
#define LANGUAGE_WOLOF_SENEGAL              0x0488
#define LANGUAGE_XHOSA                      0x0434  /* AKA isiZhosa */
#define LANGUAGE_YAKUT_RUSSIA               0x0485
#define LANGUAGE_YI                         0x0478  /* Sichuan Yi */
#define LANGUAGE_YIDDISH                    0x043D
#define LANGUAGE_YORUBA                     0x046A
#define LANGUAGE_ZULU                       0x0435

/*! use only for import/export of MS documents, number formatter maps it to
 *! LANGUAGE_SYSTEM and then to effective system language */
#define LANGUAGE_SYSTEM_DEFAULT             0x0800

/*! use only for import/export of MS documents, number formatter maps it to
 *! LANGUAGE_SYSTEM and then to effective system language */
#define LANGUAGE_PROCESS_OR_USER_DEFAULT    0x0400

/* And now the extensions we define, valid from
 * 0x0610 to 0x07FF with sublanguage ID 0x01 (default)
 * 0x0A00 to 0x0BFF with sublanguage ID 0x02
 * ...
 * 0x8200 to 0x83FF with sublanguage ID 0x20
 * 0x8600 to 0x87FF with sublanguage ID 0x21
 * ...
 * 0xFA00 to 0xFBFF with sublanguage ID 0x3E
 * 0xFE00 to 0xFFFF with sublanguage ID 0x3F
 *
 * Obsolete OOo user defines now have other values assigned by MS, and
 * different name. Mapping an obsolete value to ISO code should work provided
 * that such a mapping exists in i18nlangtag/source/isolang/isolang.cxx, but
 * mapping ISO back to LANGID will return the new value.
 */
#define LANGUAGE_OBSOLETE_USER_LATIN        0x0610
#define LANGUAGE_USER_LATIN                 LANGUAGE_LATIN
#define LANGUAGE_USER_ESPERANTO             0x0611  /* no locale possible */
#define LANGUAGE_USER_INTERLINGUA           0x0612  /* no locale, but conventions */
#define LANGUAGE_OBSOLETE_USER_MAORI        0x0620
#define LANGUAGE_USER_MAORI                 LANGUAGE_MAORI_NEW_ZEALAND
#define LANGUAGE_OBSOLETE_USER_KINYARWANDA  0x0621
#define LANGUAGE_USER_KINYARWANDA           LANGUAGE_KINYARWANDA_RWANDA
/* was reserved for Northern Sotho but never used: 0x0622 */  /* obsoleted by LANGUAGE_SEPEDI */
#define LANGUAGE_OBSOLETE_USER_UPPER_SORBIAN 0x0623
#define LANGUAGE_USER_UPPER_SORBIAN         LANGUAGE_UPPER_SORBIAN_GERMANY
#define LANGUAGE_OBSOLETE_USER_LOWER_SORBIAN 0x0624
#define LANGUAGE_USER_LOWER_SORBIAN         LANGUAGE_LOWER_SORBIAN_GERMANY
#define LANGUAGE_OBSOLETE_USER_OCCITAN      0x0625
#define LANGUAGE_USER_OCCITAN               LANGUAGE_OCCITAN_FRANCE /* reserved to languedocian */

#define LANGUAGE_USER_KOREAN_NORTH          0x8012  /* North Korean as opposed to South Korean, makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_KOREAN)) */
#define LANGUAGE_USER_KURDISH_TURKEY        0x0626  /* sublang 0x01, Latin script */
#define LANGUAGE_USER_KURDISH_SYRIA         0x0A26  /* sublang 0x02, Latin script */
#define LANGUAGE_USER_KURDISH_IRAQ          0x0E26  /* sublang 0x03, Arabic script */
#define LANGUAGE_USER_KURDISH_IRAN          0x1226  /* sublang 0x04, Arabic script */
#define LANGUAGE_USER_SARDINIAN             0x0627
/* was reserved for Dzongkha but turned down with #i53497#: 0x0628 */  /* obsoleted by LANGUAGE_DZONGKHA */
#define LANGUAGE_USER_SWAHILI_TANZANIA      0x8041  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SWAHILI)) */
#define LANGUAGE_OBSOLETE_USER_BRETON       0x0629
#define LANGUAGE_USER_BRETON                LANGUAGE_BRETON_FRANCE
#define LANGUAGE_OBSOLETE_USER_KALAALLISUT  0x062A
#define LANGUAGE_USER_KALAALLISUT           LANGUAGE_KALAALLISUT_GREENLAND
#define LANGUAGE_USER_SWAZI                 0x062B
#define LANGUAGE_USER_NDEBELE_SOUTH         0x062C
#define LANGUAGE_USER_TSWANA_BOTSWANA       0x8032  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_TSWANA)) */
#define LANGUAGE_USER_MOORE                 0x062D
#define LANGUAGE_USER_BAMBARA               0x062E
#define LANGUAGE_USER_AKAN                  0x062F
#define LANGUAGE_OBSOLETE_USER_LUXEMBOURGISH 0x0630
#define LANGUAGE_USER_LUXEMBOURGISH         LANGUAGE_LUXEMBOURGISH_LUXEMBOURG
#define LANGUAGE_USER_FRIULIAN              0x0631
#define LANGUAGE_USER_FIJIAN                0x0632
#define LANGUAGE_USER_AFRIKAANS_NAMIBIA     0x8036  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_AFRIKAANS)) */
#define LANGUAGE_USER_ENGLISH_NAMIBIA       0x8009  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
#define LANGUAGE_USER_WALLOON               0x0633
#define LANGUAGE_USER_COPTIC                0x0634
#define LANGUAGE_USER_CHUVASH               0x0635
#define LANGUAGE_USER_GASCON                0x0636  /* Gascon France */
#define LANGUAGE_USER_GERMAN_BELGIUM        0x8007  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_GERMAN)) */
#define LANGUAGE_OBSOLETE_USER_CATALAN_VALENCIAN    0x8003  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_CATALAN)) */
#define LANGUAGE_USER_CATALAN_VALENCIAN     LANGUAGE_CATALAN_VALENCIAN
#define LANGUAGE_USER_HAUSA_GHANA           0x8068  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_HAUSA_NIGERIA)) */
#define LANGUAGE_USER_EWE_GHANA             0x0637
#define LANGUAGE_USER_ENGLISH_GHANA         0x8409  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
#define LANGUAGE_USER_TAGALOG               0x0638
#define LANGUAGE_USER_LINGALA_DRCONGO       0x0639
#define LANGUAGE_USER_SANGO                 0x063A
#define LANGUAGE_USER_GANDA                 0x063B
#define LANGUAGE_USER_LOW_GERMAN            0x063C
#define LANGUAGE_USER_HILIGAYNON            0x063D
#define LANGUAGE_USER_NYANJA                0x063E
#define LANGUAGE_USER_KASHUBIAN             0x063F
#define LANGUAGE_USER_SPANISH_CUBA          0x800A  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SPANISH)) */
#define LANGUAGE_USER_TETUN                 0x0640
#define LANGUAGE_USER_QUECHUA_NORTH_BOLIVIA 0x0641
#define LANGUAGE_USER_QUECHUA_SOUTH_BOLIVIA 0x0642
#define LANGUAGE_USER_SERBIAN_CYRILLIC_SERBIA       0x8C1A  /* makeLangID( 0x20+0x03, getPrimaryLanguage( LANGUAGE_SERBIAN_CYRILLIC)) */
#define LANGUAGE_USER_SERBIAN_LATIN_SERBIA          0x881A  /* makeLangID( 0x20+0x02, getPrimaryLanguage( LANGUAGE_SERBIAN_LATIN)) */
#define LANGUAGE_USER_SERBIAN_CYRILLIC_MONTENEGRO   0xCC1A  /* makeLangID( 0x20+0x13, getPrimaryLanguage( LANGUAGE_SERBIAN_CYRILLIC)) */
#define LANGUAGE_USER_SERBIAN_LATIN_MONTENEGRO      0xC81A  /* makeLangID( 0x20+0x12, getPrimaryLanguage( LANGUAGE_SERBIAN_LATIN)) */
#define LANGUAGE_USER_SAMI_KILDIN_RUSSIA    0x803B  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SAMI_NORTHERN_NORWAY)) */
#define LANGUAGE_USER_BODO_INDIA            0x0643
#define LANGUAGE_USER_DOGRI_INDIA           0x0644
#define LANGUAGE_USER_MAITHILI_INDIA        0x0645
#define LANGUAGE_USER_SANTALI_INDIA         0x0646
#define LANGUAGE_USER_TETUN_TIMOR_LESTE     0x0A40  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_TETUN)) */
#define LANGUAGE_USER_TOK_PISIN             0x0647
#define LANGUAGE_USER_SHUSWAP               0x0648
#define LANGUAGE_USER_ARABIC_CHAD           0x8001  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_COMOROS        0x8401  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_DJIBOUTI       0x8801  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_ERITREA        0x8C01  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_ISRAEL         0x9001  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_MAURITANIA     0x9401  /* makeLangID( 0x25, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_PALESTINE      0x9801  /* makeLangID( 0x26, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_SOMALIA        0x9C01  /* makeLangID( 0x27, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ARABIC_SUDAN          0xA001  /* makeLangID( 0x28, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
#define LANGUAGE_USER_ANCIENT_GREEK         0x0649
#define LANGUAGE_USER_ASTURIAN              0x064A
#define LANGUAGE_USER_LATGALIAN             0x064B
#define LANGUAGE_USER_MAORE                 0x064C
#define LANGUAGE_USER_BUSHI                 0x064D
#define LANGUAGE_USER_TAHITIAN              0x064E
#define LANGUAGE_USER_MALAGASY_PLATEAU      0x064F
#define LANGUAGE_USER_PAPIAMENTU_ARUBA      0x8079  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
#define LANGUAGE_USER_SARDINIAN_CAMPIDANESE 0x0650
#define LANGUAGE_USER_SARDINIAN_GALLURESE   0x0651
#define LANGUAGE_USER_SARDINIAN_LOGUDORESE  0x0652
#define LANGUAGE_USER_SARDINIAN_SASSARESE   0x0653
#define LANGUAGE_USER_BAFIA                 0x0654
#define LANGUAGE_USER_GIKUYU                0x0655
#define LANGUAGE_USER_RUSYN_UKRAINE         0x0656
#define LANGUAGE_USER_RUSYN_SLOVAKIA        0x8256  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_RUSYN_UKRAINE)) */
#define LANGUAGE_USER_YIDDISH_US            0x083D  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_YIDDISH)) */
#define LANGUAGE_USER_LIMBU                 0x0657
#define LANGUAGE_USER_LOJBAN                0x0658  /* no locale */
#define LANGUAGE_OBSOLETE_USER_KABYLE       0x0659
#define LANGUAGE_USER_KABYLE                LANGUAGE_TAMAZIGHT_LATIN
#define LANGUAGE_USER_HAITIAN               0x065A
#define LANGUAGE_USER_BEEMBE                0x065B
#define LANGUAGE_USER_BEKWEL                0x065C
#define LANGUAGE_USER_KITUBA                0x065D
#define LANGUAGE_USER_LARI                  0x065E
#define LANGUAGE_USER_MBOCHI                0x065F
#define LANGUAGE_USER_TEKE_IBALI            0x0660
#define LANGUAGE_USER_TEKE_TYEE             0x0661
#define LANGUAGE_USER_VILI                  0x0662
#define LANGUAGE_USER_PORTUGUESE_ANGOLA     0x8016  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_PORTUGUESE)) */
#define LANGUAGE_USER_MANX                  0x0663
#define LANGUAGE_USER_TEKE_EBOO             0x0664
#define LANGUAGE_USER_ARAGONESE             0x0665
#define LANGUAGE_USER_KEYID                 0x0666  /* key id pseudolanguage */
#define LANGUAGE_USER_PALI_LATIN            0x0667
#define LANGUAGE_USER_KYRGYZ_CHINA          0x0668  /* not derived from LANGUAGE_KIRGHIZ as these may be different scripts, see http://www.omniglot.com/writing/kirghiz.htm */
#define LANGUAGE_USER_KOMI_ZYRIAN           0x0669
#define LANGUAGE_USER_KOMI_PERMYAK          0x066A
#define LANGUAGE_USER_PITJANTJATJARA        0x066B
#define LANGUAGE_USER_ENGLISH_MALAWI        0x8809  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
#define LANGUAGE_USER_ERZYA                 0x066C
#define LANGUAGE_USER_MARI_MEADOW           0x066D
#define LANGUAGE_USER_KHANTY                0x066E
#define LANGUAGE_USER_LIVONIAN              0x066F
#define LANGUAGE_USER_MOKSHA                0x0670
#define LANGUAGE_USER_MARI_HILL             0x0671
#define LANGUAGE_USER_NGANASAN              0x0672
#define LANGUAGE_USER_OLONETS               0x0673
#define LANGUAGE_USER_VEPS                  0x0674
#define LANGUAGE_USER_VORO                  0x0675
#define LANGUAGE_USER_NENETS                0x0676
#define LANGUAGE_USER_PAPIAMENTU_CURACAO    0x8479  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
#define LANGUAGE_USER_PAPIAMENTU_BONAIRE    0x8879  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
#define LANGUAGE_USER_AKA                   0x0677
#define LANGUAGE_USER_AKA_CONGO             0x8277  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_AKA)) */
#define LANGUAGE_USER_DIBOLE                0x0678
#define LANGUAGE_USER_DOONDO                0x0679
#define LANGUAGE_USER_KAAMBA                0x067A
#define LANGUAGE_USER_KOONGO                0x067B
#define LANGUAGE_USER_KOONGO_CONGO          0x827B  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_KOONGO)) */
#define LANGUAGE_USER_KUNYI                 0x067C
#define LANGUAGE_USER_NGUNGWEL              0x067D
#define LANGUAGE_USER_NJYEM                 0x067E
#define LANGUAGE_USER_NJYEM_CONGO           0x827E  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_NJYEM)) */
#define LANGUAGE_USER_PUNU                  0x067F
#define LANGUAGE_USER_PUNU_CONGO            0x827F  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_PUNU)) */
#define LANGUAGE_USER_SUUNDI                0x0680
#define LANGUAGE_USER_TEKE_KUKUYA           0x0681
#define LANGUAGE_USER_TSAANGI               0x0682
#define LANGUAGE_USER_YAKA                  0x0683
#define LANGUAGE_USER_YOMBE                 0x0684
#define LANGUAGE_USER_YOMBE_CONGO           0x8284  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_YOMBE)) */
#define LANGUAGE_USER_SIDAMA                0x0685
#define LANGUAGE_USER_NKO                   0x0686
#define LANGUAGE_USER_UDMURT                0x0687
#define LANGUAGE_USER_TIBETAN_INDIA         0x8051  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_TIBETAN)) */
#define LANGUAGE_USER_CORNISH               0x0688
#define LANGUAGE_USER_SAMI_PITE_SWEDEN      0x843B  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_SAMI_NORTHERN_NORWAY)) */
#define LANGUAGE_USER_NGAEBERE              0x0689

#define LANGUAGE_USER_PRIV_JOKER            0xFFEB  /* privateuse "*" (sic! bad! nasty!), primary 0x3eb, sub 0x3f */
#define LANGUAGE_USER_PRIV_COMMENT          0xFFEC  /* privateuse "x-comment", primary 0x3ec, sub 0x3f */
#define LANGUAGE_USER_PRIV_DEFAULT          0xFFED  /* privateuse "x-default", primary 0x3ed, sub 0x3f */
#define LANGUAGE_USER_PRIV_NOTRANSLATE      0xFFEE  /* privateuse "x-no-translate" (sic!), primary 0x3ee, sub 0x3f */
#define LANGUAGE_MULTIPLE                   0xFFEF  /* multiple languages, primary 0x3ef, sub 0x3f */
#define LANGUAGE_UNDETERMINED               0xFFF0  /* undetermined language, primary 0x3f0, sub 0x3f */
#define LANGUAGE_USER_SYSTEM_CONFIG         0xFFFE  /* not a locale, to be used only in configuration context to obtain system default, primary 0x3fe, sub 0x3f */

#endif /* INCLUDED_I18NLANGTAG_LANG_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
