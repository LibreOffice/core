/*************************************************************************
 *
 *  $RCSfile: numtotext_cjk.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2002-03-26 17:13:19 $
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

#define TRANSLITERATION_ALL
#include <numtotext_cjk.hxx>
#include <data/numberchar.h>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define NUMBER_OMIT_ZERO (1 << 0)
#define NUMBER_OMIT_ONE  (1 << 1)

#define NUMBER_ZERO 0x0030
#define NUMBER_ONE  0x0031

#define isNum(n)    ( 0x0030 <= n && n <= 0x0039 )

NumToText_CJK::NumToText_CJK() {
    numberChar = NULL;
}

sal_Bool SAL_CALL NumToText_CJK::numberMaker(const sal_Unicode *str, sal_Int32 begin, sal_Int32 len,
    sal_Unicode *dst, sal_Int32& count, sal_Unicode multiplierChar, sal_Int32** offset)
{
    if ( len == 1 ) {
        **offset++ = count;
        if (str[begin] != NUMBER_ZERO) {
        if (!(numberFlag & NUMBER_OMIT_ONE) || str[begin] != NUMBER_ONE)
            dst[count++] = numberChar[(sal_Int16)(str[begin] - NUMBER_ZERO)];
        if (multiplierChar > 0)
            dst[count++] = multiplierChar;
        } else if (!(numberFlag & NUMBER_OMIT_ZERO) && dst[count-1] != numberChar[0])
        dst[count++] = numberChar[0];
        return str[begin] != NUMBER_ZERO;
    } else {
        sal_Bool printPower = sal_False;
        sal_Int16 last = 0;
        for (sal_Int16 i = 1; numberMultiplier[i].power >= 0; i++) {
        sal_Int32 tmp = len - numberMultiplier[i].power;
        if (tmp > 0) {
            printPower |= numberMaker(str, begin, tmp, dst, count,
                    numberMultiplier[i].multiplierChar, offset);
            begin += tmp;
            len -= tmp;
        }
        }
        if (printPower) {
        if (dst[count-1] == numberChar[0])
            count--;
        if (multiplierChar > 0)
            dst[count++] = multiplierChar;
        }
        return printPower;
    }
}

OUString SAL_CALL NumToText_CJK::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    if (! numberChar) throw RuntimeException();

    sal_Int32 strLen = inStr.getLength() - startPos;

    if (nCount > strLen)
        nCount = strLen;

    if (nCount > 0) {
        const sal_Unicode *str = inStr.getStr() + startPos;
        rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount * 2);
        sal_Int32 i, len = 0, count = 0, begin, end;

        offset.realloc( nCount * 2 );
        sal_Int32 *p = offset.getArray();

        for (i = 0; i <= nCount; i++) {
        if (i < nCount && isNum(str[i])) {
            if (len == 0)
            begin = i;
            len++;
        } else {
            if (len > 0) {
            for (end = begin + (len % numberMultiplier[0].power);
                end <= i; begin = end, end += numberMultiplier[0].power)
                numberMaker(str, begin, end - begin, newStr->buffer, count,
                        end == i ? 0 : numberMultiplier[0].multiplierChar, &p);
            len = 0;
            }
            if (i < nCount) {
            *p++ = count;
            newStr->buffer[count++] = str[i];
            }
        }
        }

        offset.realloc(count);
        for (i = 0; i < count; i++)
        offset[i] += startPos;
        return OUString(newStr->buffer, count);
    }
    return OUString();
}

static NumberMultiplier multiplier_Lower_zh[] = {
    { 12, 0x5146 },     // fourth four digits group, ten billion
    { 8,  0x4EBF },     // third four digits group, hundred million
    { 4,  0x4E07 },     // second four digits group, ten thousand
    { 3,  0x5343 },     // Unicode Chinese Lower Thousand
    { 2,  0x767E },     // Unicode Chinese Lower Hundred
    { 1,  0x5341 },     // Unicode Chinese Lower Ten
    { 0,  0x0000 }
};

NumToTextLower_zh_CN::NumToTextLower_zh_CN() {
    numberChar = NumberChar[NumberChar_Lower_zh];
    numberMultiplier = multiplier_Lower_zh;
    numberFlag = 0;
    transliterationName = "NumToTextLower_zh_CN";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextLower_zh_CN";
}


static NumberMultiplier multiplier_Upper_zh[] = {
    { 12, 0x5146 },     // fourth four digits group, ten billion
    { 8,  0x4EBF },     // third four digits group, hundred million
    { 4,  0x4E07 },     // second four digits group, ten thousand
    { 3,  0x4EDF },     // Unicode Chinese Lower Thousand
    { 2,  0x4F70 },     // Unicode Chinese Lower Hundred
    { 1,  0x62FE },     // Unicode Chinese Lower Ten
    { 0,  0x0000 }
};

NumToTextUpper_zh_CN::NumToTextUpper_zh_CN() {
    numberChar = NumberChar[NumberChar_Upper_zh];
    numberMultiplier = multiplier_Upper_zh;
    numberFlag = 0;
    transliterationName = "NumToTextUpper_zh_CN";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextUpper_zh_CN";
}

static NumberMultiplier multiplier_Lower_zh_TW[] = {
    { 12, 0x5146 },     // fourth four digits group, ten billion
    { 8,  0x5104 },     // third four digits group, hundred million
    { 4,  0x842C },     // second four digits group, ten thousand
    { 3,  0x5343 },     // Unicode Chinese Lower Thousand
    { 2,  0x767E },     // Unicode Chinese Lower Hundred
    { 1,  0x5341 },     // Unicode Chinese Lower Ten
    { 0,  0x0000 }
};

NumToTextLower_zh_TW::NumToTextLower_zh_TW() {
    numberChar = NumberChar[NumberChar_Lower_zh];
    numberMultiplier = multiplier_Lower_zh_TW;
    numberFlag = 0;
    transliterationName = "NumToTextLower_zh_TW";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextLower_zh_TW";
}

static NumberMultiplier multiplier_Upper_zh_TW[] = {
    { 12, 0x5146 },     // fourth four digits group, ten billion
    { 8,  0x5104 },     // third four digits group, hundred million
    { 4,  0x842C },     // second four digits group, ten thousand
    { 3,  0x4EDF },     // Unicode Chinese Lower Thousand
    { 2,  0x4F70 },     // Unicode Chinese Lower Hundred
    { 1,  0x62FE },     // Unicode Chinese Lower Ten
    { 0,  0x0000 }
};

NumToTextUpper_zh_TW::NumToTextUpper_zh_TW() {
    numberChar = NumberChar[NumberChar_Upper_zh_TW];
    numberMultiplier = multiplier_Upper_zh_TW;
    numberFlag = 0;
    transliterationName = "NumToTextUpper_zh_TW";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextUpper_zh_TW";
}

NumToTextFormalLower_ko::NumToTextFormalLower_ko() {
    numberChar = NumberChar[NumberChar_Lower_ko];
    numberMultiplier = multiplier_Lower_zh_TW;
    numberFlag = NUMBER_OMIT_ZERO;
    transliterationName = "NumToTextFormalLower_ko";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextFormalLower_ko";
}

NumToTextFormalUpper_ko::NumToTextFormalUpper_ko() {
    numberChar = NumberChar[NumberChar_Upper_ko];
    numberMultiplier = multiplier_Lower_zh_TW;
    numberFlag = NUMBER_OMIT_ZERO;
    transliterationName = "NumToTextFormalUpper_ko";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextFormalUpper_ko";
}

NumToTextInformalLower_ko::NumToTextInformalLower_ko() {
    numberChar = NumberChar[NumberChar_Lower_ko];
    numberMultiplier = multiplier_Lower_zh_TW;
    numberFlag = NUMBER_OMIT_ZERO | NUMBER_OMIT_ONE;
    transliterationName = "NumToTextInformalLower_ko";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextInformalLower_ko";
}

NumToTextInformalUpper_ko::NumToTextInformalUpper_ko() {
    numberChar = NumberChar[NumberChar_Upper_ko];
    numberMultiplier = multiplier_Lower_zh_TW;
    numberFlag = NUMBER_OMIT_ZERO | NUMBER_OMIT_ONE;
    transliterationName = "NumToTextInformalUpper_ko";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextInformalUpper_ko";
}

static NumberMultiplier multiplier_Hangul_ko[] = {
    { 12, 0xC870 },     // fourth four digits group, ten billion
    { 8,  0xC5B5 },     // third four digits group, hundred million
    { 4,  0xB9CC },     // second four digits group, ten thousand
    { 3,  0xCC9C },     // Unicode Chinese Lower Thousand
    { 2,  0xBC31 },     // Unicode Chinese Lower Hundred
    { 1,  0xC2ED },     // Unicode Chinese Lower Ten
    { 0,  0x0000 }
};

NumToTextFormalHangul_ko::NumToTextFormalHangul_ko() {
    numberChar = NumberChar[NumberChar_Hangul_ko];
    numberMultiplier = multiplier_Hangul_ko;
    numberFlag = NUMBER_OMIT_ZERO;
    transliterationName = "NumToTextFormalHangul_ko";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextFormalHangul_ko";
}

NumToTextInformalHangul_ko::NumToTextInformalHangul_ko() {
    numberChar = NumberChar[NumberChar_Hangul_ko];
    numberMultiplier = multiplier_Hangul_ko;
    numberFlag = NUMBER_OMIT_ZERO | NUMBER_OMIT_ONE;
    transliterationName = "NumToTextInformalHangul_ko";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextInformalHangul_ko";
}

static NumberMultiplier multiplier_Traditional_ja[] = {
    { 9, 0x62FE },  //  billion           // 10 * 100000000
    { 8, 0x5104 },  //  hundred million   // 1 * 100000000   // needs a preceding "one"
    { 7, 0x9621 },  //  ten million       // 1000 * 10000
    { 6, 0x767E },  //  million           // 100 * 10000
    { 5, 0x62FE },  //  hundred thousand  // 10 * 10000
    { 4, 0x842C },  //  ten thousand      // 1 * 10000       // needs a preceding "one"
    { 3, 0x9621 },  //  thousand          // 1000
    { 2, 0x767E },  //  hundred           // 100
    { 1, 0x62FE },  //  ten               // 10
    { 0, 0x0000 }   //  one               // 1               // needs a "one"
};

NumToTextKanjiLongTraditional_ja_JP::NumToTextKanjiLongTraditional_ja_JP() {
    numberChar = NumberChar[NumberChar_Traditional_ja];
    numberMultiplier = multiplier_Traditional_ja;
    numberFlag = NUMBER_OMIT_ZERO | NUMBER_OMIT_ONE;
    transliterationName = "NumToTextKanjiLongTraditional_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextKanjiLongTraditional_ja_JP";
}

static NumberMultiplier multiplier_Modern_ja[] = {
    { 9, 0x5341 },  //  billion           // 10 * 100000000
    { 8, 0x5104 },  //  hundred million   // 1 * 100000000   // needs a preceding "one"
    { 7, 0x5343 },  //  ten million       // 1000 * 10000
    { 6, 0x767E },  //  million           // 100 * 10000
    { 5, 0x5341 },  //  hundred thousand  // 10 * 10000
    { 4, 0x4E07 },  //  ten thousand      // 1 * 10000       // needs a preceding "one"
    { 3, 0x5343 },  //  thousand          // 1000
    { 2, 0x767E },  //  hundred           // 100
    { 1, 0x5341 },  //  ten               // 10
    { 0, 0x0000 }   //  one               // 1               // needs a "one"
};

NumToTextKanjiLongModern_ja_JP::NumToTextKanjiLongModern_ja_JP() {
    numberChar = NumberChar[NumberChar_Modern_ja];
    numberMultiplier = multiplier_Modern_ja;
    numberFlag = NUMBER_OMIT_ZERO | NUMBER_OMIT_ONE;
    transliterationName = "NumToTextKanjiLongModern_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.NumToTextKanjiLongModern_ja_JP";
}

} } } }
