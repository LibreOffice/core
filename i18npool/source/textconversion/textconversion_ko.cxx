/*************************************************************************
 *
 *  $RCSfile: textconversion_ko.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:54:00 $
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

#include <assert.h>
#include <textconversion.hxx>
#include <com/sun/star/i18n/TextConversionType.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/linguistic2/ConversionDirection.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <rtl/ustrbuf.hxx>
#include <x_rtl_ustring.h>
#include <unicode.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

// defined in hangul2hanja.cxx generated from hangul2hanja.dic by genconv_dict
extern const sal_Unicode* getHangul2HanjaData();
extern const Hangul_Index* getHangul2HanjaIndex();
extern const sal_Int16 getHangul2HanjaIndexCount();
extern const sal_uInt16* getHanja2HangulIndex();
extern const sal_Unicode* getHanja2HangulData();

#define SCRIPT_OTHERS   0
#define SCRIPT_HANJA    1
#define SCRIPT_HANGUL   2

TextConversion_ko::TextConversion_ko( const Reference < XMultiServiceFactory >& xMSF )
{
    Reference < XInterface > xI;

    xI = xMSF->createInstance(
        OUString::createFromAscii("com.sun.star.i18n.ConversionDictionary_ko"));

    if ( xI.is() )
        xI->queryInterface( getCppuType((const Reference< XConversionDictionary>*)0) ) >>= xCD;

    xI = xMSF->createInstance(
        OUString::createFromAscii("com.sun.star.linguist2.ConversionDictionaryList"));

    if ( xI.is() )
        xI->queryInterface( getCppuType((const Reference< XConversionDictionaryList>*)0) ) >>= xCDL;

    maxLeftLength = maxRightLength = 1;

    // get maximum length of word in dictionary
    if (xCDL.is()) {
        Locale loc(OUString::createFromAscii("ko"),
                    OUString::createFromAscii("KR"),
                    OUString());
        maxLeftLength = xCDL->queryMaxCharCount(loc,
                        ConversionDictionaryType::HANJA_HANGUL,
                        ConversionDirection_FROM_LEFT);
        maxRightLength = xCDL->queryMaxCharCount(loc,
                        ConversionDictionaryType::HANJA_HANGUL,
                        ConversionDirection_FROM_RIGHT);
        if (xCD.is()) {
            sal_Int32 tmp = xCD->getMaxCharCount(ConversionDirection_FROM_LEFT);
            if (tmp > maxLeftLength)
                maxLeftLength = tmp;
            tmp = xCD->getMaxCharCount(ConversionDirection_FROM_RIGHT);
            if (tmp > maxRightLength)
                maxRightLength = tmp;
        }
    } else if (xCD.is()) {
        maxLeftLength = xCD->getMaxCharCount(ConversionDirection_FROM_LEFT);
        maxRightLength = xCD->getMaxCharCount(ConversionDirection_FROM_RIGHT);
    }

    implementationName = "com.sun.star.i18n.TextConversion_ko";
}

sal_Int16 SAL_CALL checkScriptType(sal_Unicode c)
{
    static ScriptTypeList typeList[] = {
        { UnicodeScript_kHangulJamo, SCRIPT_HANGUL }, // 29
        { UnicodeScript_kCJKRadicalsSupplement, SCRIPT_HANJA },     // 57,
        { UnicodeScript_kKangxiRadicals,        SCRIPT_HANJA },     // 58,
        { UnicodeScript_kIdeographicDescriptionCharacters, SCRIPT_HANJA },  // 59,
        { UnicodeScript_kCJKSymbolPunctuation,  SCRIPT_HANJA },     // 60,
        { UnicodeScript_kHiragana,          SCRIPT_HANJA },     // 61,
        { UnicodeScript_kKatakana,          SCRIPT_HANJA },     // 62,
        { UnicodeScript_kBopomofo,          SCRIPT_HANJA },     // 63,
        { UnicodeScript_kHangulCompatibilityJamo,   SCRIPT_HANGUL },        // 64,
        { UnicodeScript_kKanbun,            SCRIPT_HANJA },     // 65,
        { UnicodeScript_kBopomofoExtended,      SCRIPT_HANJA },     // 66,
        { UnicodeScript_kEnclosedCJKLetterMonth,    SCRIPT_HANJA },     // 67,
        { UnicodeScript_kCJKCompatibility,      SCRIPT_HANJA },     // 68,
        { UnicodeScript_k_CJKUnifiedIdeographsExtensionA, SCRIPT_HANJA },   // 69,
        { UnicodeScript_kCJKUnifiedIdeograph,   SCRIPT_HANJA },     // 70,
        { UnicodeScript_kYiSyllables,       SCRIPT_HANJA },     // 71,
        { UnicodeScript_kYiRadicals,        SCRIPT_HANJA },     // 72,
        { UnicodeScript_kHangulSyllable,        SCRIPT_HANGUL },        // 73,
        { UnicodeScript_kCJKCompatibilityIdeograph, SCRIPT_HANJA },     // 78,
        { UnicodeScript_kCombiningHalfMark,     SCRIPT_HANJA },     // 81,
        { UnicodeScript_kCJKCompatibilityForm,  SCRIPT_HANJA },     // 82,
        { UnicodeScript_kSmallFormVariant,      SCRIPT_HANJA },     // 83,
        { UnicodeScript_kHalfwidthFullwidthForm,    SCRIPT_HANJA },     // 86,

        { UnicodeScript_kScriptCount, SCRIPT_OTHERS } // 87,
    };

    return unicode::getUnicodeScriptType(c, typeList, SCRIPT_OTHERS);
}

Sequence< OUString > SAL_CALL getCharConversions(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toHanja)
{
    sal_Unicode ch;
    Sequence< OUString > output;
    if (toHanja) {
        ch = aText[nStartPos];
        const Hangul_Index *Hangul_ko = getHangul2HanjaIndex();
        sal_Int16 top =  getHangul2HanjaIndexCount() - 1;
        sal_Int16 bottom = 0;

        while (bottom <= top) {
            sal_Int16 current = (top + bottom) / 2;
            sal_Unicode current_ch = Hangul_ko[current].code;
            if (ch < current_ch)
                top = current - 1;
            else if (ch > current_ch)
                bottom = current + 1;
            else {
                const sal_Unicode *ptr = getHangul2HanjaData() + Hangul_ko[current].address;
                sal_Int16 count = Hangul_ko[current].count;
                output.realloc(count);
                for (sal_Int16 i = 0; i < count; i++)
                    output[i] = OUString(ptr + i, 1);
                break;
            }
        }
    } else {
        rtl_uString * newStr = x_rtl_uString_new_WithLength( nLength ); // defined in x_rtl_ustring.h
        sal_Int32 count = 0;
        while (count < nLength) {
            ch = aText[nStartPos + count];
            sal_Unicode address = getHanja2HangulIndex()[ch>>8];
            if (address != 0xFFFF)
                address = getHanja2HangulData()[address + (ch & 0xFF)];

            if (address != 0xFFFF)
                newStr->buffer[count++] = address;
            else
                break;
        }
        if (count > 0) {
            output.realloc(1);
            output[0] = OUString( newStr->buffer, count);
        }
    }
    return output;
}

static Sequence< OUString >& operator += (Sequence< OUString > &rSeq1, Sequence< OUString > &rSeq2 )
{
    if (! rSeq1.hasElements() && rSeq2.hasElements())
        rSeq1 = rSeq2;
    else if (rSeq2.hasElements()) {
        sal_Int32 i, j, k, l;
        k = l = rSeq1.getLength();
        rSeq1.realloc(l + rSeq2.getLength());

        for (i = 0; i < rSeq2.getLength(); i++) {
            for (j = 0; j < l; j++)
                if (rSeq1[j] == rSeq2[i])
                    break;
            if (j == l)
                rSeq1[k++] = rSeq2[i];
        }
        if (rSeq1.getLength() > k)
            rSeq1.realloc(k);
    }
    return rSeq1;
}

TextConversionResult SAL_CALL
TextConversion_ko::getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& aLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    TextConversionResult result;
    Sequence <OUString> candidates;
    result.Boundary.startPos = result.Boundary.endPos = 0;

    // do conversion only when there are right conversion type and dictionary services.
    if (nConversionType == TextConversionType::TO_HANGUL ||
            nConversionType == TextConversionType::TO_HANJA) {
        sal_Int32 start, end, length = aText.getLength() - nStartPos;

        if (length < 0 || nStartPos < 0)
            length = 0;
        else if (length > nLength)
            length = nLength;

        sal_Int32 maxLength = 1;
        sal_Int16 scriptType = SCRIPT_OTHERS;
        ConversionDirection eDirection = ConversionDirection_FROM_LEFT;
        sal_Bool toHanja = sal_True;

        // search for a max length of convertible text
        for (start = 0, end = 0; start < length; start++) {

            if (end <= start) {
                scriptType = checkScriptType(aText[nStartPos + start]);
                /* wait for BI_DIRECTION being add to idl
                if (nConversionOptions & TextConversionOption::BI_DIRECTION) {
                    if (scriptType == SCRIPT_OTHERS) // skip non-Korean chararacters
                        continue;
                } else
                */
                if (nConversionType == TextConversionType::TO_HANJA) {
                    if (scriptType != SCRIPT_HANGUL) // skip non-Hangul characters
                        continue;
                } else {
                    if (scriptType != SCRIPT_HANJA) // skip non-Hanja characters
                        continue;
                }

                toHanja = (scriptType == SCRIPT_HANGUL);
                eDirection = toHanja ?
                        ConversionDirection_FROM_LEFT : ConversionDirection_FROM_RIGHT;

                maxLength = toHanja ? maxLeftLength : maxRightLength;
                if (maxLength == 0 || (nConversionOptions & TextConversionOption::CHARACTER_BY_CHARACTER))
                    maxLength = 1;

                end = start + 1;
            }

            for (; end < length && end - start < maxLength; end++)
                if (checkScriptType(aText[nStartPos + end]) != scriptType)
                    break;

            for (sal_Int32 len = end - start; len > 0; len--) {
                if (xCDL.is())
                    result.Candidates = xCDL->queryConversions(aText, start + nStartPos, len,
                        aLocale, ConversionDictionaryType::HANJA_HANGUL, eDirection, nConversionOptions); // user dictionary
                if (xCD.is() && toHanja) { // System dictionary would not do Hanja_to_Hangul conversion.
                                         // Char2char converison below is enough.
                    candidates = xCD->getConversions(aText, start + nStartPos, len, eDirection, nConversionOptions); // system dictionary
                    result.Candidates += candidates;
                }
                if (len == 1) {
                    if (!toHanja && !result.Candidates.hasElements()) {
                        // do whole word character 2 character conversion
                        result.Candidates = getCharConversions(aText, nStartPos + start, length - start, toHanja);
                        if (result.Candidates.hasElements())
                            len = result.Candidates[0].getLength();
                    } else {
                        candidates = getCharConversions(aText, nStartPos + start, 1, toHanja); // char2char conversion
                        result.Candidates += candidates;
                    }
                }

                // found match
                if (result.Candidates.hasElements()) {
                    result.Boundary.startPos = start + nStartPos;;
                    result.Boundary.endPos = start + len + nStartPos;
                    return result;
                }
            }
        }
    } else
        throw NoSupportException(); // Conversion type is not supported in this service.
    return result;
}

OUString SAL_CALL
TextConversion_ko::getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& aLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    sal_Int32 length = aText.getLength() - nStartPos;

    if (length <= 0 || nStartPos < 0)
        return OUString();
    else if (length > nLength)
        length = nLength;

    OUStringBuffer aBuf(length + 1);
    TextConversionResult result;
    const sal_Unicode *str = aText.getStr();

    for (sal_Int32 start = nStartPos; length + nStartPos > start; start = result.Boundary.endPos) {

        result = getConversions(aText, start, length + nStartPos - start, aLocale, nConversionType, nConversionOptions);

        if (result.Boundary.endPos > 0) {
            if (result.Boundary.startPos > start)
                aBuf.append(str + start, result.Boundary.startPos - start); // append skip portion
            aBuf.append(result.Candidates[0]); // append converted portion
        } else {
            if (length + nStartPos > start)
                aBuf.append(str + start, length + nStartPos - start); // append last portion
            break;
        }
    }

    return aBuf.makeStringAndClear();
}

sal_Bool SAL_CALL
TextConversion_ko::interactiveConversion( const Locale& aLocale, sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    return sal_True;
}

} } } }
