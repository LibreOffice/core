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


#include <utility>
#include <comphelper/string.hxx>
#define TRANSLITERATION_ProlongedSoundMark_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


namespace com { namespace sun { namespace star { namespace i18n {

static const sal_Unicode table_normalwidth[] = {
    //  0x0000,   // 0x3040
    0x3041,       // 0x3041 HIRAGANA LETTER SMALL A
    0x3042,       // 0x3042 HIRAGANA LETTER A
    0x3043,       // 0x3043 HIRAGANA LETTER SMALL I
    0x3044,       // 0x3044 HIRAGANA LETTER I
    0x3045,       // 0x3045 HIRAGANA LETTER SMALL U
    0x3046,       // 0x3046 HIRAGANA LETTER U
    0x3047,       // 0x3047 HIRAGANA LETTER SMALL E
    0x3048,       // 0x3048 HIRAGANA LETTER E
    0x3049,       // 0x3049 HIRAGANA LETTER SMALL O
    0x304a,       // 0x304a HIRAGANA LETTER O
    0x3042,       // 0x304b HIRAGANA LETTER KA
    0x3042,       // 0x304c HIRAGANA LETTER GA
    0x3044,       // 0x304d HIRAGANA LETTER KI
    0x3044,       // 0x304e HIRAGANA LETTER GI
    0x3046,       // 0x304f HIRAGANA LETTER KU
    0x3046,       // 0x3050 HIRAGANA LETTER GU
    0x3048,       // 0x3051 HIRAGANA LETTER KE
    0x3048,       // 0x3052 HIRAGANA LETTER GE
    0x304a,       // 0x3053 HIRAGANA LETTER KO
    0x304a,       // 0x3054 HIRAGANA LETTER GO
    0x3042,       // 0x3055 HIRAGANA LETTER SA
    0x3042,       // 0x3056 HIRAGANA LETTER ZA
    0x3044,       // 0x3057 HIRAGANA LETTER SI
    0x3044,       // 0x3058 HIRAGANA LETTER ZI
    0x3046,       // 0x3059 HIRAGANA LETTER SU
    0x3046,       // 0x305a HIRAGANA LETTER ZU
    0x3048,       // 0x305b HIRAGANA LETTER SE
    0x3048,       // 0x305c HIRAGANA LETTER ZE
    0x304a,       // 0x305d HIRAGANA LETTER SO
    0x304a,       // 0x305e HIRAGANA LETTER ZO
    0x3042,       // 0x305f HIRAGANA LETTER TA
    0x3042,       // 0x3060 HIRAGANA LETTER DA
    0x3044,       // 0x3061 HIRAGANA LETTER TI
    0x3044,       // 0x3062 HIRAGANA LETTER DI
    0x3045,       // 0x3063 HIRAGANA LETTER SMALL TU
    0x3046,       // 0x3064 HIRAGANA LETTER TU
    0x3046,       // 0x3065 HIRAGANA LETTER DU
    0x3048,       // 0x3066 HIRAGANA LETTER TE
    0x3048,       // 0x3067 HIRAGANA LETTER DE
    0x304a,       // 0x3068 HIRAGANA LETTER TO
    0x304a,       // 0x3069 HIRAGANA LETTER DO
    0x3042,       // 0x306a HIRAGANA LETTER NA
    0x3044,       // 0x306b HIRAGANA LETTER NI
    0x3046,       // 0x306c HIRAGANA LETTER NU
    0x3048,       // 0x306d HIRAGANA LETTER NE
    0x304a,       // 0x306e HIRAGANA LETTER NO
    0x3042,       // 0x306f HIRAGANA LETTER HA
    0x3042,       // 0x3070 HIRAGANA LETTER BA
    0x3042,       // 0x3071 HIRAGANA LETTER PA
    0x3044,       // 0x3072 HIRAGANA LETTER HI
    0x3044,       // 0x3073 HIRAGANA LETTER BI
    0x3044,       // 0x3074 HIRAGANA LETTER PI
    0x3046,       // 0x3075 HIRAGANA LETTER HU
    0x3046,       // 0x3076 HIRAGANA LETTER BU
    0x3046,       // 0x3077 HIRAGANA LETTER PU
    0x3048,       // 0x3078 HIRAGANA LETTER HE
    0x3048,       // 0x3079 HIRAGANA LETTER BE
    0x3048,       // 0x307a HIRAGANA LETTER PE
    0x304a,       // 0x307b HIRAGANA LETTER HO
    0x304a,       // 0x307c HIRAGANA LETTER BO
    0x304a,       // 0x307d HIRAGANA LETTER PO
    0x3042,       // 0x307e HIRAGANA LETTER MA
    0x3044,       // 0x307f HIRAGANA LETTER MI
    0x3046,       // 0x3080 HIRAGANA LETTER MU
    0x3048,       // 0x3081 HIRAGANA LETTER ME
    0x304a,       // 0x3082 HIRAGANA LETTER MO
    0x3041,       // 0x3083 HIRAGANA LETTER SMALL YA
    0x3042,       // 0x3084 HIRAGANA LETTER YA
    0x3045,       // 0x3085 HIRAGANA LETTER SMALL YU
    0x3046,       // 0x3086 HIRAGANA LETTER YU
    0x3049,       // 0x3087 HIRAGANA LETTER SMALL YO
    0x304a,       // 0x3088 HIRAGANA LETTER YO
    0x3042,       // 0x3089 HIRAGANA LETTER RA
    0x3044,       // 0x308a HIRAGANA LETTER RI
    0x3046,       // 0x308b HIRAGANA LETTER RU
    0x3048,       // 0x308c HIRAGANA LETTER RE
    0x304a,       // 0x308d HIRAGANA LETTER RO
    0x3041,       // 0x308e HIRAGANA LETTER SMALL WA
    0x3042,       // 0x308f HIRAGANA LETTER WA
    0x3044,       // 0x3090 HIRAGANA LETTER WI
    0x3048,       // 0x3091 HIRAGANA LETTER WE
    0x304a,       // 0x3092 HIRAGANA LETTER WO
    0x0000,       // 0x3093 HIRAGANA LETTER N
    0x3046,       // 0x3094 HIRAGANA LETTER VU
    0x0000,       // 0x3095
    0x0000,       // 0x3096
    0x0000,       // 0x3097
    0x0000,       // 0x3098
    0x0000,       // 0x3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
    0x0000,       // 0x309a COMBINING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK
    0x0000,       // 0x309b KATAKANA-HIRAGANA VOICED SOUND MARK
    0x0000,       // 0x309c KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK
    0x0000,       // 0x309d HIRAGANA ITERATION MARK
    0x0000,       // 0x309e HIRAGANA VOICED ITERATION MARK
    0x0000,       // 0x309f
    0x0000,       // 0x30a0
    0x30a1,       // 0x30a1 KATAKANA LETTER SMALL A
    0x30a2,       // 0x30a2 KATAKANA LETTER A
    0x30a3,       // 0x30a3 KATAKANA LETTER SMALL I
    0x30a4,       // 0x30a4 KATAKANA LETTER I
    0x30a5,       // 0x30a5 KATAKANA LETTER SMALL U
    0x30a6,       // 0x30a6 KATAKANA LETTER U
    0x30a7,       // 0x30a7 KATAKANA LETTER SMALL E
    0x30a8,       // 0x30a8 KATAKANA LETTER E
    0x30a9,       // 0x30a9 KATAKANA LETTER SMALL O
    0x30aa,       // 0x30aa KATAKANA LETTER O
    0x30a2,       // 0x30ab KATAKANA LETTER KA
    0x30a2,       // 0x30ac KATAKANA LETTER GA
    0x30a4,       // 0x30ad KATAKANA LETTER KI
    0x30a4,       // 0x30ae KATAKANA LETTER GI
    0x30a6,       // 0x30af KATAKANA LETTER KU
    0x30a6,       // 0x30b0 KATAKANA LETTER GU
    0x30a8,       // 0x30b1 KATAKANA LETTER KE
    0x30a8,       // 0x30b2 KATAKANA LETTER GE
    0x30aa,       // 0x30b3 KATAKANA LETTER KO
    0x30aa,       // 0x30b4 KATAKANA LETTER GO
    0x30a2,       // 0x30b5 KATAKANA LETTER SA
    0x30a2,       // 0x30b6 KATAKANA LETTER ZA
    0x30a4,       // 0x30b7 KATAKANA LETTER SI
    0x30a4,       // 0x30b8 KATAKANA LETTER ZI
    0x30a6,       // 0x30b9 KATAKANA LETTER SU
    0x30a6,       // 0x30ba KATAKANA LETTER ZU
    0x30a8,       // 0x30bb KATAKANA LETTER SE
    0x30a8,       // 0x30bc KATAKANA LETTER ZE
    0x30aa,       // 0x30bd KATAKANA LETTER SO
    0x30aa,       // 0x30be KATAKANA LETTER ZO
    0x30a2,       // 0x30bf KATAKANA LETTER TA
    0x30a2,       // 0x30c0 KATAKANA LETTER DA
    0x30a4,       // 0x30c1 KATAKANA LETTER TI
    0x30a4,       // 0x30c2 KATAKANA LETTER DI
    0x30a5,       // 0x30c3 KATAKANA LETTER SMALL TU
    0x30a6,       // 0x30c4 KATAKANA LETTER TU
    0x30a6,       // 0x30c5 KATAKANA LETTER DU
    0x30a8,       // 0x30c6 KATAKANA LETTER TE
    0x30a8,       // 0x30c7 KATAKANA LETTER DE
    0x30aa,       // 0x30c8 KATAKANA LETTER TO
    0x30aa,       // 0x30c9 KATAKANA LETTER DO
    0x30a2,       // 0x30ca KATAKANA LETTER NA
    0x30a4,       // 0x30cb KATAKANA LETTER NI
    0x30a6,       // 0x30cc KATAKANA LETTER NU
    0x30a8,       // 0x30cd KATAKANA LETTER NE
    0x30aa,       // 0x30ce KATAKANA LETTER NO
    0x30a2,       // 0x30cf KATAKANA LETTER HA
    0x30a2,       // 0x30d0 KATAKANA LETTER BA
    0x30a2,       // 0x30d1 KATAKANA LETTER PA
    0x30a4,       // 0x30d2 KATAKANA LETTER HI
    0x30a4,       // 0x30d3 KATAKANA LETTER BI
    0x30a4,       // 0x30d4 KATAKANA LETTER PI
    0x30a6,       // 0x30d5 KATAKANA LETTER HU
    0x30a6,       // 0x30d6 KATAKANA LETTER BU
    0x30a6,       // 0x30d7 KATAKANA LETTER PU
    0x30a8,       // 0x30d8 KATAKANA LETTER HE
    0x30a8,       // 0x30d9 KATAKANA LETTER BE
    0x30a8,       // 0x30da KATAKANA LETTER PE
    0x30aa,       // 0x30db KATAKANA LETTER HO
    0x30aa,       // 0x30dc KATAKANA LETTER BO
    0x30aa,       // 0x30dd KATAKANA LETTER PO
    0x30a2,       // 0x30de KATAKANA LETTER MA
    0x30a4,       // 0x30df KATAKANA LETTER MI
    0x30a6,       // 0x30e0 KATAKANA LETTER MU
    0x30a8,       // 0x30e1 KATAKANA LETTER ME
    0x30aa,       // 0x30e2 KATAKANA LETTER MO
    0x30a1,       // 0x30e3 KATAKANA LETTER SMALL YA
    0x30a2,       // 0x30e4 KATAKANA LETTER YA
    0x30a5,       // 0x30e5 KATAKANA LETTER SMALL YU
    0x30a6,       // 0x30e6 KATAKANA LETTER YU
    0x30a9,       // 0x30e7 KATAKANA LETTER SMALL YO
    0x30aa,       // 0x30e8 KATAKANA LETTER YO
    0x30a2,       // 0x30e9 KATAKANA LETTER RA
    0x30a4,       // 0x30ea KATAKANA LETTER RI
    0x30a6,       // 0x30eb KATAKANA LETTER RU
    0x30a8,       // 0x30ec KATAKANA LETTER RE
    0x30aa,       // 0x30ed KATAKANA LETTER RO
    0x30a1,       // 0x30ee KATAKANA LETTER SMALL WA
    0x30a2,       // 0x30ef KATAKANA LETTER WA
    0x30a4,       // 0x30f0 KATAKANA LETTER WI
    0x30a8,       // 0x30f1 KATAKANA LETTER WE
    0x30aa,       // 0x30f2 KATAKANA LETTER WO
    0x0000,       // 0x30f3 KATAKANA LETTER N
    0x30a6,       // 0x30f4 KATAKANA LETTER VU
    0x30a1,       // 0x30f5 KATAKANA LETTER SMALL KA
    0x30a7,       // 0x30f6 KATAKANA LETTER SMALL KE
    0x30a2,       // 0x30f7 KATAKANA LETTER VA
    0x30a4,       // 0x30f8 KATAKANA LETTER VI
    0x30a8,       // 0x30f9 KATAKANA LETTER VE
    0x30aa        // 0x30fa KATAKANA LETTER VO
    //  0x0000,   // 0x30fb KATAKANA MIDDLE DOT
    //  0x0000,   // 0x30fc KATAKANA-HIRAGANA PROLONGED SOUND MARK
    //  0x0000,   // 0x30fd KATAKANA ITERATION MARK
    //  0x0000,   // 0x30fe KATAKANA VOICED ITERATION MARK
    //  0x0000    // 0x30ff
};

static const sal_Unicode table_halfwidth[] = {
    //  0x0000,   // 0xff61 HALFWIDTH IDEOGRAPHIC FULL STOP
    //  0x0000,   // 0xff62 HALFWIDTH LEFT CORNER BRACKET
    //  0x0000,   // 0xff63 HALFWIDTH RIGHT CORNER BRACKET
    //  0x0000,   // 0xff64 HALFWIDTH IDEOGRAPHIC COMMA
    //  0x0000,   // 0xff65 HALFWIDTH KATAKANA MIDDLE DOT
    0xff75,       // 0xff66 HALFWIDTH KATAKANA LETTER WO
    0xff67,       // 0xff67 HALFWIDTH KATAKANA LETTER SMALL A
    0xff68,       // 0xff68 HALFWIDTH KATAKANA LETTER SMALL I
    0xff69,       // 0xff69 HALFWIDTH KATAKANA LETTER SMALL U
    0xff6a,       // 0xff6a HALFWIDTH KATAKANA LETTER SMALL E
    0xff6b,       // 0xff6b HALFWIDTH KATAKANA LETTER SMALL O
    0xff67,       // 0xff6c HALFWIDTH KATAKANA LETTER SMALL YA
    0xff69,       // 0xff6d HALFWIDTH KATAKANA LETTER SMALL YU
    0xff6b,       // 0xff6e HALFWIDTH KATAKANA LETTER SMALL YO
    0xff69,       // 0xff6f HALFWIDTH KATAKANA LETTER SMALL TU
    0x0000,       // 0xff70 HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK
    0xff71,       // 0xff71 HALFWIDTH KATAKANA LETTER A
    0xff72,       // 0xff72 HALFWIDTH KATAKANA LETTER I
    0xff73,       // 0xff73 HALFWIDTH KATAKANA LETTER U
    0xff74,       // 0xff74 HALFWIDTH KATAKANA LETTER E
    0xff75,       // 0xff75 HALFWIDTH KATAKANA LETTER O
    0xff71,       // 0xff76 HALFWIDTH KATAKANA LETTER KA
    0xff72,       // 0xff77 HALFWIDTH KATAKANA LETTER KI
    0xff73,       // 0xff78 HALFWIDTH KATAKANA LETTER KU
    0xff74,       // 0xff79 HALFWIDTH KATAKANA LETTER KE
    0xff75,       // 0xff7a HALFWIDTH KATAKANA LETTER KO
    0xff71,       // 0xff7b HALFWIDTH KATAKANA LETTER SA
    0xff72,       // 0xff7c HALFWIDTH KATAKANA LETTER SI
    0xff73,       // 0xff7d HALFWIDTH KATAKANA LETTER SU
    0xff74,       // 0xff7e HALFWIDTH KATAKANA LETTER SE
    0xff75,       // 0xff7f HALFWIDTH KATAKANA LETTER SO
    0xff71,       // 0xff80 HALFWIDTH KATAKANA LETTER TA
    0xff72,       // 0xff81 HALFWIDTH KATAKANA LETTER TI
    0xff73,       // 0xff82 HALFWIDTH KATAKANA LETTER TU
    0xff74,       // 0xff83 HALFWIDTH KATAKANA LETTER TE
    0xff75,       // 0xff84 HALFWIDTH KATAKANA LETTER TO
    0xff71,       // 0xff85 HALFWIDTH KATAKANA LETTER NA
    0xff72,       // 0xff86 HALFWIDTH KATAKANA LETTER NI
    0xff73,       // 0xff87 HALFWIDTH KATAKANA LETTER NU
    0xff74,       // 0xff88 HALFWIDTH KATAKANA LETTER NE
    0xff75,       // 0xff89 HALFWIDTH KATAKANA LETTER NO
    0xff71,       // 0xff8a HALFWIDTH KATAKANA LETTER HA
    0xff72,       // 0xff8b HALFWIDTH KATAKANA LETTER HI
    0xff73,       // 0xff8c HALFWIDTH KATAKANA LETTER HU
    0xff74,       // 0xff8d HALFWIDTH KATAKANA LETTER HE
    0xff75,       // 0xff8e HALFWIDTH KATAKANA LETTER HO
    0xff71,       // 0xff8f HALFWIDTH KATAKANA LETTER MA
    0xff72,       // 0xff90 HALFWIDTH KATAKANA LETTER MI
    0xff73,       // 0xff91 HALFWIDTH KATAKANA LETTER MU
    0xff74,       // 0xff92 HALFWIDTH KATAKANA LETTER ME
    0xff75,       // 0xff93 HALFWIDTH KATAKANA LETTER MO
    0xff71,       // 0xff94 HALFWIDTH KATAKANA LETTER YA
    0xff73,       // 0xff95 HALFWIDTH KATAKANA LETTER YU
    0xff75,       // 0xff96 HALFWIDTH KATAKANA LETTER YO
    0xff71,       // 0xff97 HALFWIDTH KATAKANA LETTER RA
    0xff72,       // 0xff98 HALFWIDTH KATAKANA LETTER RI
    0xff73,       // 0xff99 HALFWIDTH KATAKANA LETTER RU
    0xff74,       // 0xff9a HALFWIDTH KATAKANA LETTER RE
    0xff75,       // 0xff9b HALFWIDTH KATAKANA LETTER RO
    0xff71        // 0xff9c HALFWIDTH KATAKANA LETTER WA
    //  0x0000,   // 0xff9d HALFWIDTH KATAKANA LETTER N
    //  0x0000,   // 0xff9e HALFWIDTH KATAKANA VOICED SOUND MARK
    //  0x0000    // 0xff9f HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK
};


OUString SAL_CALL
ignoreProlongedSoundMark_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 1 now.
    rtl_uString * newStr = rtl_uString_alloc(nCount);
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    sal_Int32 *p = 0;
    sal_Int32 position = 0;

    if (useOffset) {
        // Allocate nCount length to offset argument.
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
    }

    //
    sal_Unicode previousChar = *src ++;
    sal_Unicode currentChar;

    // Conversion
    while (-- nCount > 0) {
        currentChar = *src ++;

        if (currentChar == 0x30fc || // KATAKANA-HIRAGANA PROLONGED SOUND MARK
                currentChar == 0xff70) { // HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK

            if (0x3041 <= previousChar && previousChar <= 0x30fa) {
                currentChar = table_normalwidth[ previousChar - 0x3041 ];
            }
            else if (0xff66 <= previousChar && previousChar <= 0xff9c) {
                currentChar = table_halfwidth[ previousChar - 0xff66 ];
            }
        }

        if (useOffset)
            *p ++ = position ++;
        *dst ++ = previousChar;
        previousChar = currentChar;
    }

    if (nCount == 0) {
        if (useOffset)
            *p = position;
        *dst ++ = previousChar;
    }

    *dst = (sal_Unicode) 0;

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE); // take ownership

}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
