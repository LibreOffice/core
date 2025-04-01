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

#pragma once


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
 ! quite complete...
 ! Language Identifier Constants and Strings (Windows)
 ! http://msdn.microsoft.com/en-us/library/dd318693.aspx
 !
 ! Use THAT ^^^ as of 2013-09-17 it includes also Windows 8

 ! BUT, you can download a PDF document from
 ! http://msdn.microsoft.com/library/cc233965.aspx
 ! that has YET MORE definitions, sigh... didn't cross-check if any are missing
 ! from that... however, it also contains a few MS-reserved definitions that use
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

#include <config_options.h>
#include <sal/types.h>
#include <o3tl/strong_int.hxx>
#include <ostream>

typedef o3tl::strong_int<sal_uInt16, struct LanguageTypeTag> LanguageType;
inline std::ostream& operator<<(std::ostream& os, LanguageType const & lt) { os << sal_uInt16(lt); return os; }
constexpr LanguageType primary(LanguageType lt) { return LanguageType(sal_uInt16(lt) & 0x03ff); }

namespace o3tl
{
// when compiling LO on macOS, debug builds will display a linking error where, see
// <https://lists.freedesktop.org/archives/libreoffice/2024-February/091564.html>, "Our Clang
// --enable-pch setup is known broken":
#if !(defined MACOSX && defined __clang__ && __clang_major__ == 16 && ENABLE_PCH)
    // delete "sal_Int16" constructor via specialization: values > 0x7FFF are
    // actually used, and unfortunately passed around in the API as signed
    // "short", so use this to find all places where casts must be inserted
    template<> template<> constexpr strong_int<unsigned short,LanguageTypeTag>::strong_int(short, std::enable_if<std::is_integral<short>::value, int>::type) = delete;
#endif
}

 0x03ff

                   LanguageType(0x03FF)  /* yes, the mask */
                       LanguageType(0x00FF)
 LanguageType(0x04FF)
                     LanguageType(0x0000)  /* OOo/SO definition */

/* The Invariant Locale (Locale ID = 0x007f) is a locale that can be used by
 * applications when a consistent and locale-independent result is required.
 * The invariant locale can be used, for example, when comparing character
 * strings using the CompareString() API and a consistent result regardless of
 * the User Locale is expected.
 * The settings of the Invariant Locale are similar to US-English international
 * standards, but should not be used to display formatted data. */
/* NOTE: this is taken from the MS documentation! Not supported by OOo/SO! */
                  LanguageType(0x007F)

/* Naming conventions:
 * * ..._LSO suffix indicates a Language-Script-Only mapping, i.e. no country
 *   assignment in i18nlangtag/source/isolang/isolang.cxx.
 *   Used for backward compatibility or where MS assigned such values and they
 *   should be explicitly handled.
 * * LANGUAGE_USER_... for our user-defined IDs.
 * * LANGUAGE_OBSOLETE_USER_... for a user-defined ID that has been replaced by
 *   a value defined by MS.
 */

                  LanguageType(0x0436)
                   LanguageType(0x041C)
            LanguageType(0x0484)
           LanguageType(0x045E)
             LanguageType(0x1401)
             LanguageType(0x3C01)
               LanguageType(0x0C01)
                LanguageType(0x0801)
              LanguageType(0x2C01)
              LanguageType(0x3401)
             LanguageType(0x3001)
               LanguageType(0x1001)
             LanguageType(0x1801)
                LanguageType(0x2001)
        LanguageType(0x0001)  /* primary only, not a locale! */
               LanguageType(0x4001)
        LanguageType(0x0401)
               LanguageType(0x2801)
             LanguageType(0x1C01)
                 LanguageType(0x3801)
               LanguageType(0x2401)
                   LanguageType(0x042B)  /* Eastern Armenian */
                   LanguageType(0x044D)
             LanguageType(0x082C)
         LanguageType(0x742C)
                LanguageType(0x042C)
            LanguageType(0x782C)
             LanguageType(0x046D)
                     LanguageType(0x042D)
                 LanguageType(0x0423)
                    LanguageType(0x0445)  /* in India */
         LanguageType(0x0845)
                   LanguageType(0x641A)
    LanguageType(0x201A)
                      LanguageType(0x681A)
                            LanguageType(0x781A)
       LanguageType(0x141A)
             LANGUAGE_BOSNIAN_LATIN_BOSNIA_HERZEGOVINA   /* TODO: remove, only for langtab.src & localize.sdf compatibility */
              LanguageType(0x047E)  /* obsoletes LANGUAGE_USER_BRETON 0x0629 */
                  LanguageType(0x0402)
                    LanguageType(0x0455)
                    LanguageType(0x0403)
          LanguageType(0x0803)  /* obsoletes LANGUAGE_USER_CATALAN_VALENCIAN 0x8003 */
     LanguageType(0x045C)
      LanguageType(0x7C5C)
           LanguageType(0x0C04)
                LanguageType(0x7804)
              LanguageType(0x1404)
         LanguageType(0x0804)
          LanguageType(0x1004)
        LanguageType(0x0404)
  LanguageType(0x0004)  /* MS-.NET 'zh-CHS', primary only! but maps to 'zh-CN' */
    LanguageType(0x7C04)  /* MS-.NET 'zh-CHT' but maps to 'zh-Hant' */
                    LANGUAGE_CHINESE_SIMPLIFIED     /* most code uses LANGUAGE_CHINESE */
            LanguageType(0x0483)
                   LanguageType(0x041A)
 LanguageType(0x101A)
                      LanguageType(0x0405)
                     LanguageType(0x0406)
           LanguageType(0x048C)  /* AKA Zoroastrian Dari */
                    LanguageType(0x0465)  /* AKA Divehi */
                      LanguageType(0x0413)
              LanguageType(0x0813)
                        LanguageType(0x0466)
                    LanguageType(0x0009)  /* primary only, not a locale! */
      LanguageType(0x4C09)
                LanguageType(0x0C09)
            LanguageType(0x5009)
             LanguageType(0x2809)
                LanguageType(0x1009)
          LanguageType(0x2409)  /* MS reserved since rev.15, "en-029" */
              LanguageType(0x5409)
               LanguageType(0x1809)
      LanguageType(0x3C09)
              LanguageType(0x4009)
          LanguageType(0x3809)
            LanguageType(0x2009)
             LanguageType(0x5809)
             LanguageType(0x5C09)
           LanguageType(0x4409)
                 LanguageType(0x1409)
        LanguageType(0x3409)
            LanguageType(0x1C09)
          LanguageType(0x4809)
           LanguageType(0x2C09)
             LanguageType(0x6009)
                 LanguageType(0x0809)
                 LanguageType(0x0409)
              LanguageType(0x6409)
           LanguageType(0x3009)
                   LanguageType(0x0425)
                   LanguageType(0x0438)
                      LanguageType(0x0429)
                   LanguageType(0x0464)
                    LanguageType(0x040B)
                     LanguageType(0x040C)
             LanguageType(0x080C)
            LanguageType(0x2C0C)
            LanguageType(0x0C0C)
       LanguageType(0x300C)

               LanguageType(0x3C0C)
          LanguageType(0x140C)
                LanguageType(0x340C)
              LanguageType(0x180C)
             LanguageType(0x380C)
        LanguageType(0xE40C)  /* MS reserved, "fr-015", "015"="Northern Africa" */
             LanguageType(0x200C)
             LanguageType(0x280C)
               LanguageType(0x100C)
         LanguageType(0x1C0C)  /* MS-LCID.pdf in between said "Neither defined nor reserved" but since rev.15 it's "fr-029" */
               LanguageType(0x240C)
        LanguageType(0x0462)
         LanguageType(0x7C67)
           LanguageType(0x0467)
           LanguageType(0x0867)
             LanguageType(0x083C)
            LanguageType(0x0491)  /* apparently it occurred to MS that those are different languages */
     LanguageType(0x043C)
                   LanguageType(0x0456)
                   LanguageType(0x0437)
                     LanguageType(0x0407)
            LanguageType(0x0C07)
       LanguageType(0x1407)
          LanguageType(0x1007)
               LanguageType(0x0807)
                      LanguageType(0x0408)
           LanguageType(0x0474)
                   LanguageType(0x0447)
            LanguageType(0x7C68)
              LanguageType(0x0468)
     LanguageType(0x0475)
                     LanguageType(0x040D)
                      LanguageType(0x0439)
                  LanguageType(0x040E)
             LanguageType(0x0469)
                  LanguageType(0x040F)
               LanguageType(0x0470)
                 LanguageType(0x0421)
 LanguageType(0x045D)
    LanguageType(0x785D)
     LanguageType(0x085D)
        LanguageType(0x7C5D)
                    LanguageType(0x0410)
              LanguageType(0x0810)
                   LanguageType(0x0411)
      LanguageType(0x046F)  /* obsoletes LANGUAGE_USER_KALAALLISUT 0x062A */
                    LanguageType(0x044B)
             LanguageType(0x0471)
                   LanguageType(0x0460)
             LanguageType(0x0860)
                     LanguageType(0x043F)
               LanguageType(0x083F)
        LanguageType(0x783F)
           LanguageType(0x7C3F)
                      LanguageType(0x0453)
            LanguageType(0x0486)  /* AKA K'iche', West Central Quiche */
         LanguageType(0x0487)  /* obsoletes LANGUAGE_USER_KINYARWANDA 0x0621 */
                    LanguageType(0x0440)  /* AKA Kyrgyz */
                    LanguageType(0x0457)
                     LanguageType(0x0412)
               LanguageType(0x0812)  /* not mentioned in MS-LCID.pdf, oh joy */
        LanguageType(0x0492)  /* obsoletes LANGUAGE_USER_KURDISH_IRAQ 0x0E26 */
         LanguageType(0x7C92)
                        LanguageType(0x0454)
                      LanguageType(0x0476)  /* obsoletes LANGUAGE_USER_LATIN 0x0610 and LANGUAGE_USER_LATIN_VATICAN 0x8076 */
                    LanguageType(0x0426)
                 LanguageType(0x0427)
         LanguageType(0x0827)  /* MS in its MS-LCID.pdf now says "Neither defined nor reserved" */
                   LanguageType(0x0490)
   LanguageType(0x046E)  /* obsoletes LANGUAGE_USER_LUXEMBOURGISH 0x0630 */
                 LanguageType(0x042F)
           LanguageType(0x048D)  /* obsoletes LANGUAGE_OBSOLETE_USER_MALAGASY_PLATEAU */
                  LanguageType(0x044C)  /* in India */
    LanguageType(0x083E)
             LanguageType(0x043E)
                    LanguageType(0x043A)
                   LanguageType(0x0458)
          LanguageType(0x0481)  /* obsoletes LANGUAGE_USER_MAORI 0x0620 */
           LanguageType(0x047A)  /* AKA Araucanian */
                    LanguageType(0x044E)
              LanguageType(0x047C)
    LanguageType(0x0450)
         LanguageType(0x7850)
   LanguageType(0x0C50)
      LanguageType(0x0850)
        LanguageType(0x7C50)
                     LanguageType(0x0461)
               LanguageType(0x0861)
                  LanguageType(0x0014)  /* primary only, not a locale! */
           LanguageType(0x0414)
       LanguageType(0x7C14)
          LanguageType(0x0814)
      LanguageType(0x7814)
             LanguageType(0x0482)  /* obsoletes LANGUAGE_USER_OCCITAN 0x0625 */
                       LanguageType(0x0448)
                      LanguageType(0x0472)
                 LanguageType(0x0479)
                     LanguageType(0x0463)
                     LanguageType(0x0415)
                 LanguageType(0x0816)
       LanguageType(0x0416)
                    LanguageType(0x0446)
         LanguageType(0x7C46)
           LanguageType(0x0846)
            LanguageType(0x046B)
           LanguageType(0x0493)  /* different primary ID */
            LanguageType(0x086B)
               LanguageType(0x0C6B)
               LanguageType(0x0417)
                   LanguageType(0x0418)
           LanguageType(0x0818)
                    LanguageType(0x0419)
            LanguageType(0x0819)
       LanguageType(0x043B)
               LANGUAGE_SAMI_NORTHERN_NORWAY   /* the old MS definition */
                 LanguageType(0x243B)
             LanguageType(0x703B)
              LanguageType(0x7C3B)
           LanguageType(0x103B)
           LanguageType(0x143B)
      LanguageType(0x0C3B)
       LanguageType(0x083B)
                 LanguageType(0x203B)
             LanguageType(0x743B)
          LanguageType(0x783B)
       LanguageType(0x183B)
       LanguageType(0x1C3B)
                   LanguageType(0x044F)
                     LanguageType(0x046C)
              LANGUAGE_SEPEDI /* just an alias for the already existing localization */
       LanguageType(0x6C1A)
       LanguageType(0x0C1A)  /* Serbia and Montenegro (former) */
    LanguageType(0x1C1A)
 LanguageType(0x301A)
    LanguageType(0x281A)
          LanguageType(0x701A)
          LanguageType(0x081A)
       LanguageType(0x181A)
   LanguageType(0x2C1A)
      LanguageType(0x7C1A)
       LanguageType(0x241A)
                    LanguageType(0x0430)  /* also called Sutu now by MS */
                     LanguageType(0x0459)
          LanguageType(0x7C59)
            LanguageType(0x0859)
        LanguageType(0x045B)
                     LanguageType(0x041B)
                  LanguageType(0x0424)
                     LanguageType(0x0477)
      LanguageType(0x042E)  /* obsoletes LANGUAGE_USER_UPPER_SORBIAN 0x0623 */
      LanguageType(0x082E)  /* obsoletes LANGUAGE_USER_LOWER_SORBIAN 0x0624. NOTE: the primary ID is identical to Upper Sorbian, which is not quite correct because they're distinct languages */
          LanguageType(0x7C2E)
                    LANGUAGE_USER_UPPER_SORBIAN /* a strange MS definition */
              LanguageType(0x040A)  /* old collation, not supported, see #i94435# */
          LanguageType(0x2C0A)
            LanguageType(0x400A)
              LanguageType(0x340A)
           LanguageType(0x240A)
          LanguageType(0x140A)
               LanguageType(0x5C0A)
 LanguageType(0x1C0A)
            LanguageType(0x300A)
        LanguageType(0x440A)
          LanguageType(0x100A)
           LanguageType(0x480A)
            LanguageType(0x080A)
             LanguageType(0x0C0A)
          LanguageType(0x4C0A)
             LanguageType(0x180A)
           LanguageType(0x3C0A)
               LanguageType(0x280A)
        LanguageType(0x500A)
      LanguageType(0x540A)
            LanguageType(0x380A)
          LanguageType(0x200A)
                    LANGUAGE_SPANISH_MODERN     /* modern collation, see #i94435# */
                    LanguageType(0x0441)  /* Kenya */
                    LanguageType(0x041D)
            LanguageType(0x081D)
                     LanguageType(0x045A)
              LanguageType(0x048F)
                      LanguageType(0x0428)
                  LanguageType(0x7C28)
   LanguageType(0x045F)
    LanguageType(0x085F)
        LanguageType(0x7C5F)
          LanguageType(0x0C5F)
 LanguageType(0x105F)
     LanguageType(0x785F)
                      LanguageType(0x0449)
            LanguageType(0x0849)
                      LanguageType(0x0444)
                     LanguageType(0x044A)
                       LanguageType(0x041E)
                    LanguageType(0x0451)
             LanguageType(0x0851)  /* was used as Dzongkha, a MS error, see #i53497# */
            LanguageType(0x0C51)  /* they finally got it ... ([MS-LCID] rev. 7.0 2015-06-30) */
           LanguageType(0x0873)
          LanguageType(0x0473)
                     LanguageType(0x0431)
                     LanguageType(0x0432)  /* AKA Setsuana, for South Africa */
            LanguageType(0x0832)  /* obsoletes LANGUAGE_USER_TSWANA_BOTSWANA 0x8032 */
                    LanguageType(0x041F)
                    LanguageType(0x0442)
               LanguageType(0x0480)
                  LanguageType(0x0422)
                 LanguageType(0x0820)
              LanguageType(0x0420)
             LanguageType(0x0843)
         LanguageType(0x7843)
                LanguageType(0x0443)
            LanguageType(0x7C43)
                      LanguageType(0x0433)
                 LanguageType(0x042A)
                      LanguageType(0x0452)
              LanguageType(0x0488)
                      LanguageType(0x0434)  /* AKA isiZhosa */
               LanguageType(0x0485)
                         LanguageType(0x0478)  /* Sichuan Yi */
                    LanguageType(0x043D)
                     LanguageType(0x046A)
       LanguageType(0x048E)
                       LanguageType(0x0435)

                   LanguageType(0x0501)  /* 'qps-ploc', qps is a reserved for local use code */
                  LanguageType(0x05FE)  /* 'qps-ploca', qps is a reserved for local use code */
                  LanguageType(0x09FF)  /* 'qps-plocm', qps is a reserved for local use code */

        LanguageType(0x4401)  /* 'ar-Ploc-SA', 'Ploc'?? */
        LanguageType(0x0811)  /* 'ja-Ploc-JP', 'Ploc'?? */
           LanguageType(0x0479)  /* "pap-029", "029"="Caribbean" */
            LanguageType(0x4801)  /* "ar-145", "145"="Western Asia" */
            LanguageType(0x580A)  /* MS reserved since rev.15, "es-419", "419"="Latin America and the Caribbean" */

/* Seems these values were used or reserved at one point of time ... */
 LanguageType(0x007B)
 LanguageType(0x007D)
 LanguageType(0x0089)
 LanguageType(0x008A)
 LanguageType(0x008B)
 LanguageType(0x008D)
 LanguageType(0x008E)
 LanguageType(0x008F)
 LanguageType(0x0090)
 LanguageType(0x0827)
 LanguageType(0x1C0C)
 LanguageType(0x2008)

/* MS defines these as reserved, whatever that might imply... */
            LanguageType(0xEEEE)  /* primary 0x2ee, sub 0x3b */
            LanguageType(0xF2EE)  /* primary 0x2ee, sub 0x3c */

/*! use only for import/export of MS documents, number formatter maps it to
 *! LANGUAGE_SYSTEM and then to effective system language */
             LanguageType(0x0800)

/*! use only for import/export of MS documents, number formatter maps it to
 *! LANGUAGE_SYSTEM and then to effective system language */
    LanguageType(0x0400)

/* Number format code modifier attribute for system time.
 * Primary language 0x000, sublanguage 0x3D. */
             LanguageType(0xF400)

/* Number format code modifier attribute for system date.
 * Primary language 0x000, sublanguage 0x3E. */
             LanguageType(0xF800)


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
 LanguageType(0xE40A)  /* no locale possible, legacy for es-419, makeLangID( 0x39, getPrimaryLanguage( LANGUAGE_SPANISH)) */
      LANGUAGE_es_419_reserved
        LanguageType(0x0610)
                 LANGUAGE_LATIN
 LanguageType(0x8076)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_LATIN)) */
         LANGUAGE_LATIN
             LanguageType(0x0611)  /* no locale possible */
           LanguageType(0x0612)  /* no locale, but conventions */
        LanguageType(0x0620)
                 LANGUAGE_MAORI_NEW_ZEALAND
  LanguageType(0x0621)
           LANGUAGE_KINYARWANDA_RWANDA
/* was reserved for Northern Sotho but never used: 0x0622 */  /* obsoleted by LANGUAGE_SEPEDI */
 LanguageType(0x0623)
         LANGUAGE_UPPER_SORBIAN_GERMANY
 LanguageType(0x0624)
         LANGUAGE_LOWER_SORBIAN_GERMANY
      LanguageType(0x0625)
               LANGUAGE_OCCITAN_FRANCE /* reserved to languedocian */

          LanguageType(0x8012)  /* North Korean as opposed to South Korean, makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_KOREAN)) */
        LanguageType(0x0626)  /* sublang 0x01, Latin script */
         LanguageType(0x0A26)  /* sublang 0x02, Latin script */
 LanguageType(0x0E26)  /* sublang 0x03, Arabic script */
          LANGUAGE_KURDISH_ARABIC_IRAQ
          LanguageType(0x1226)  /* sublang 0x04, Arabic script */
 LanguageType(0x8092)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_KURDISH_ARABIC_LSO)) */
 LanguageType(0x8492)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_KURDISH_ARABIC_LSO)) */
             LanguageType(0x0627)
/* was reserved for Dzongkha but turned down with #i53497#: 0x0628 */  /* obsoleted by LANGUAGE_DZONGKHA_BHUTAN */
    LanguageType(0xF851)  /* to map "dz" only, because of the MS error, and preserve CTL information, sub 0x3e */
      LanguageType(0x8041)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SWAHILI)) */
       LanguageType(0x0629)
                LANGUAGE_BRETON_FRANCE
  LanguageType(0x062A)
           LANGUAGE_KALAALLISUT_GREENLAND
                 LanguageType(0x062B)
         LanguageType(0x062C)
 LanguageType(0x8032)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_TSWANA)) */
       LANGUAGE_TSWANA_BOTSWANA
                 LanguageType(0x062D)
               LanguageType(0x062E)
                  LanguageType(0x062F)
 LanguageType(0x0630)
         LANGUAGE_LUXEMBOURGISH_LUXEMBOURG
              LanguageType(0x0631)
                LanguageType(0x0632)
     LanguageType(0x8036)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_AFRIKAANS)) */
       LanguageType(0x8009)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
               LanguageType(0x0633)
                LanguageType(0x0634)
               LanguageType(0x0635)
                LanguageType(0x0636)  /* Gascon France */
        LanguageType(0x8007)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_GERMAN)) */
    LanguageType(0x8003)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_CATALAN)) */
     LANGUAGE_CATALAN_VALENCIAN
           LanguageType(0x8068)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_HAUSA_NIGERIA)) */
             LanguageType(0x0637)
         LanguageType(0x8409)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
               LanguageType(0x0638)
       LanguageType(0x0639)
                 LanguageType(0x063A)
                 LanguageType(0x063B)
            LanguageType(0x063C)
            LanguageType(0x063D)
                LanguageType(0x063E)
             LanguageType(0x063F)
 LanguageType(0x800A)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SPANISH)) */
          LANGUAGE_SPANISH_CUBA
                 LanguageType(0x0640)
 LanguageType(0x0641)
 LanguageType(0x0642)
      LanguageType(0x8C1A)  /* makeLangID( 0x20+0x03, getPrimaryLanguage( LANGUAGE_SERBIAN_CYRILLIC_LSO)) */
               LANGUAGE_SERBIAN_CYRILLIC_SERBIA
         LanguageType(0x881A)  /* makeLangID( 0x20+0x02, getPrimaryLanguage( LANGUAGE_SERBIAN_LATIN_LSO)) */
                  LANGUAGE_SERBIAN_LATIN_SERBIA
  LanguageType(0xCC1A)  /* makeLangID( 0x20+0x13, getPrimaryLanguage( LANGUAGE_SERBIAN_CYRILLIC_LSO)) */
           LANGUAGE_SERBIAN_CYRILLIC_MONTENEGRO
     LanguageType(0xC81A)  /* makeLangID( 0x20+0x12, getPrimaryLanguage( LANGUAGE_SERBIAN_LATIN_LSO)) */
              LANGUAGE_SERBIAN_LATIN_MONTENEGRO
    LanguageType(0x803B)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SAMI_NORTHERN_NORWAY)) */
            LanguageType(0x0643)
           LanguageType(0x0644)
        LanguageType(0x0645)
         LanguageType(0x0646)  /* Devanagari script */
     LanguageType(0x0A40)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_TETUN)) */
             LanguageType(0x0647)
               LanguageType(0x0648)
           LanguageType(0x8001)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
        LanguageType(0x8401)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
       LanguageType(0x8801)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
        LanguageType(0x8C01)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
         LanguageType(0x9001)  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
     LanguageType(0x9401)  /* makeLangID( 0x25, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
      LanguageType(0x9801)  /* makeLangID( 0x26, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
        LanguageType(0x9C01)  /* makeLangID( 0x27, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
          LanguageType(0xA001)  /* makeLangID( 0x28, getPrimaryLanguage( LANGUAGE_ARABIC_SAUDI_ARABIA)) */
         LanguageType(0x0649)
              LanguageType(0x064A)
             LanguageType(0x064B)
                 LanguageType(0x064C)
                 LanguageType(0x064D)
              LanguageType(0x064E)
 LanguageType(0x064F)
      LanguageType(0x8079)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
 LanguageType(0x0650)
   LanguageType(0x0651)
  LanguageType(0x0652)
   LanguageType(0x0653)
                 LanguageType(0x0654)
                LanguageType(0x0655)
         LanguageType(0x0656)
        LanguageType(0x8256)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_RUSYN_UKRAINE)) */
            LanguageType(0x803D)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_YIDDISH)) */
                 LanguageType(0x0657)
                LanguageType(0x0658)  /* no locale */
       LanguageType(0x0659)
                LANGUAGE_TAMAZIGHT_LATIN_ALGERIA
               LanguageType(0x065A)
                LanguageType(0x065B)
                LanguageType(0x065C)
                LanguageType(0x065D)
                  LanguageType(0x065E)
                LanguageType(0x065F)
            LanguageType(0x0660)
             LanguageType(0x0661)
                  LanguageType(0x0662)
     LanguageType(0x8016)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_PORTUGUESE)) */
                  LanguageType(0x0663)
             LanguageType(0x0664)
             LanguageType(0x0665)
                 LanguageType(0x0666)  /* key id pseudolanguage */
            LanguageType(0x0667)
          LanguageType(0x0668)  /* not derived from LANGUAGE_KIRGHIZ as these may be different scripts, see http://www.omniglot.com/writing/kirghiz.htm */
           LanguageType(0x0669)
          LanguageType(0x066A)
        LanguageType(0x066B)
        LanguageType(0x8809)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
                 LanguageType(0x066C)
           LanguageType(0x066D)
                LanguageType(0x066E)
              LanguageType(0x066F)
                LanguageType(0x0670)
             LanguageType(0x0671)
              LanguageType(0x0672)
               LanguageType(0x0673)
                  LanguageType(0x0674)
                  LanguageType(0x0675)
                LanguageType(0x0676)
    LanguageType(0x8479)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
    LanguageType(0x8879)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_PAPIAMENTU)) */
                   LanguageType(0x0677)
             LanguageType(0x8277)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_AKA)) */
                LanguageType(0x0678)
                LanguageType(0x0679)
                LanguageType(0x067A)
                LanguageType(0x067B)
          LanguageType(0x827B)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_KOONGO)) */
                 LanguageType(0x067C)
              LanguageType(0x067D)
                 LanguageType(0x067E)
           LanguageType(0x827E)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_NJYEM)) */
                  LanguageType(0x067F)
            LanguageType(0x827F)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_PUNU)) */
                LanguageType(0x0680)
           LanguageType(0x0681)
               LanguageType(0x0682)
                  LanguageType(0x0683)
                 LanguageType(0x0684)
           LanguageType(0x8284)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_YOMBE)) */
                LanguageType(0x0685)
                   LanguageType(0x0686)
                LanguageType(0x0687)
         LanguageType(0x8051)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_TIBETAN)) */
               LanguageType(0x0688)
      LanguageType(0x843B)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_SAMI_NORTHERN_NORWAY)) */
              LanguageType(0x0689)
        LanguageType(0x8C09)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
                 LanguageType(0x068A)
                 LanguageType(0x068B)
      LanguageType(0x068C)
                 LanguageType(0x068D)
        LanguageType(0x8451)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_TIBETAN)) */
   LanguageType(0x800C)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_FRENCH)) */
               LanguageType(0x068E)
  LanguageType(0x068F)
                  LanguageType(0x0690)
     LanguageType(0x0691)
 LanguageType(0x8291)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_USER_CREE_PLAINS_LATIN)) */
                 LanguageType(0x0692)
          LanguageType(0x840C)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_FRENCH)) */
          LanguageType(0x880C)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_FRENCH)) */
           LanguageType(0x8C0C)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_FRENCH)) */
          LanguageType(0x0693)
         LanguageType(0x0694)
              LanguageType(0x0695)
        LanguageType(0x9009)  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
       LanguageType(0x8082)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_OCCITAN_FRANCE)) */
   LanguageType(0x9409)  /* makeLangID( 0x25, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
        LanguageType(0x0696)
         LanguageType(0x0A96)  /* makeLangID( 0x02, getPrimaryLanguage( LANGUAGE_USER_ARPITAN_FRANCE)) */
   LanguageType(0x0E96)  /* makeLangID( 0x03, getPrimaryLanguage( LANGUAGE_USER_ARPITAN_FRANCE)) */
      LanguageType(0x9809)  /* makeLangID( 0x26, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
           LanguageType(0x0697)  /* no locale */
               LanguageType(0x0698)
     LanguageType(0x9C09)  /* makeLangID( 0x27, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
      LanguageType(0x900C)  /* makeLangID( 0x24, getPrimaryLanguage( LANGUAGE_FRENCH)) */
              LanguageType(0x0699)
       LanguageType(0x800E)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_HUNGARIAN)) */
                LanguageType(0x069A)
                  LanguageType(0x069B)
        LanguageType(0x069C)
                   LanguageType(0x069D)
          LanguageType(0x069E)
       LanguageType(0x802B)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_ARMENIAN)) */
         LanguageType(0x842B)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_ARMENIAN)) */
      LanguageType(0x069F)
    LanguageType(0x06A0)
 LanguageType(0x803E)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_MALAY_MALAYSIA)) */
   LanguageType(0x843E)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_MALAY_BRUNEI_DARUSSALAM)) */
                LanguageType(0x06A1)
                  LanguageType(0x06A2)
                 LanguageType(0x06A3)
        LanguageType(0xA009)  /* makeLangID( 0x28, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
     LanguageType(0xA409)  /* makeLangID( 0x29, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
       LanguageType(0xA809)  /* makeLangID( 0x2A, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
             LanguageType(0x06A4)
    LanguageType(0x06A5)
              LanguageType(0x06A6)
           LanguageType(0x06A7)
             LanguageType(0x06A8)
          LanguageType(0x06A9)
         LanguageType(0xAC09)  /* makeLangID( 0x2B, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
             LanguageType(0x0A67)  /* makeLangID( 0x02, getPrimaryLanguage( LANGUAGE_USER_PALI_LATIN)) */
               LanguageType(0x06AA)
                LanguageType(0x06AB)
       LanguageType(0xB009)  /* makeLangID( 0x2C, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
       LanguageType(0x8030)  /* makeLangID( 0x20, getPrimaryLanguage( LANGUAGE_SESOTHO)) */
        LanguageType(0x843D)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_YIDDISH)) */
               LanguageType(0x06AC)
        LanguageType(0xB409)  /* makeLangID( 0x2D, getPrimaryLanguage( LANGUAGE_ENGLISH_US)) */
     LanguageType(0x06AD)
  LanguageType(0x06AE)
    LanguageType(0x06AF)
 LanguageType(0x8416)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_PORTUGUESE)) */
 LanguageType(0x840A)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_SPANISH)) */
   LanguageType(0x880A)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_SPANISH)) */
       LanguageType(0x8403)  /* makeLangID( 0x21, getPrimaryLanguage( LANGUAGE_CATALAN)) */
        LanguageType(0x8803)  /* makeLangID( 0x22, getPrimaryLanguage( LANGUAGE_CATALAN)) */
         LanguageType(0x8C03)  /* makeLangID( 0x23, getPrimaryLanguage( LANGUAGE_CATALAN)) */
 LanguageType(0xB809)  /* makeLangID( 0x2E, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */
               LanguageType(0x06B0)
       LanguageType(0x06B1)
              LanguageType(0x06B2)
 LanguageType(0x06B3)
         LanguageType(0x940C)  /* makeLangID( 0x25, getPrimaryLanguage( LANGUAGE_FRENCH)) */
                LanguageType(0x06B4)
        LanguageType(0xBC09)  /* makeLangID( 0x2F, getPrimaryLanguage( LANGUAGE_ENGLISH_UK)) */


/* XXX Add new user defined LCIDs ^^^ there.
 * Note that we run out of IDs with sublanguage ID 0x01 after 0x07DF ... */


/* Primary language ID range for on-the-fly assignment. */
           LanguageType(0x03E0)
             LanguageType(0x03FE)
/* Sublanguage ID range for on-the-fly assignment. */
       LanguageType(0x01)
         LanguageType(0x3E)

/* 0xFFE0 to 0xFFFF reserved for privateuse and specials. */
             LanguageType(0xFFEA)  /* privateuse "x-none", MS ill-escaped into the wild, primary 0x3ea, sub 0x3f */
            LanguageType(0xFFEB)  /* privateuse "*" (sic! bad! nasty!), primary 0x3eb, sub 0x3f */
          LanguageType(0xFFEC)  /* privateuse "x-comment", primary 0x3ec, sub 0x3f */
          LanguageType(0xFFED)  /* privateuse "x-default", primary 0x3ed, sub 0x3f */
      LanguageType(0xFFEE)  /* privateuse "x-no-translate" (sic!), primary 0x3ee, sub 0x3f */
                   LanguageType(0xFFEF)  /* multiple languages, primary 0x3ef, sub 0x3f */
               LanguageType(0xFFF0)  /* undetermined language, primary 0x3f0, sub 0x3f */
         LanguageType(0xFFFE)  /* not a locale, to be used only in configuration context to obtain system default, primary 0x3fe, sub 0x3f */




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
