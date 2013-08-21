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


#include "transliterationImpl.hxx"
#include "servicename.hxx"

#include <com/sun/star/i18n/LocaleData.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/instance.hxx>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <algorithm>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


namespace com { namespace sun { namespace star { namespace i18n {

#define ERROR RuntimeException()

#define TmItem1( name ) \
  {TransliterationModules_##name, TransliterationModulesNew_##name, #name}

#define TmItem2( name ) \
  {(TransliterationModules)0, TransliterationModulesNew_##name, #name}

// Ignore Module list
static struct TMlist {
  TransliterationModules        tm;
  TransliterationModulesNew     tmn;
  const sal_Char               *implName;
} TMlist[] = {                                  //      Modules      ModulesNew
  TmItem1 (IGNORE_CASE),                        // 0. (1<<8        256) (7)
  TmItem1 (IGNORE_WIDTH),                       // 1. (1<<9        512) (8)
  TmItem1 (IGNORE_KANA),                        // 2. (1<<10      1024) (9)
// No enum define for this trans. application has to use impl name to load it
//  TmItem1 (IGNORE_CASE_SIMPLE),                       // (1<<11      1024) (66)

  TmItem1 (ignoreTraditionalKanji_ja_JP),       // 3. (1<<12      4096) (10)
  TmItem1 (ignoreTraditionalKana_ja_JP),        // 4. (1<<13      8192) (11)
  TmItem1 (ignoreMinusSign_ja_JP),              // 5. (1<<13     16384) (12)
  TmItem1 (ignoreIterationMark_ja_JP),          // 6. (1<<14     32768) (13)
  TmItem1 (ignoreSeparator_ja_JP),              // 7. (1<<15     65536) (14)
  TmItem1 (ignoreSize_ja_JP),                   // 15. (1<<23  16777216) (22)
  TmItem1 (ignoreMiddleDot_ja_JP),              // 17. (1<<25  67108864) (24)
  TmItem1 (ignoreSpace_ja_JP),                  // 18. (1<<26 134217728) (25)
  TmItem1 (ignoreZiZu_ja_JP),                   // 8. (1<<16    131072) (15)
  TmItem1 (ignoreBaFa_ja_JP),                   // 9. (1<<17    262144) (16)
  TmItem1 (ignoreTiJi_ja_JP),                   // 10. (1<<18    524288) (17)
  TmItem1 (ignoreHyuByu_ja_JP),                 // 11. (1<<19   1048576) (18)
  TmItem1 (ignoreSeZe_ja_JP),                   // 12. (1<<20   2097152) (19)
  TmItem1 (ignoreIandEfollowedByYa_ja_JP),      // 13. (1<<21   4194304) (20)
  TmItem1 (ignoreKiKuFollowedBySa_ja_JP),       // 14. (1<<22   8388608) (21)
  TmItem1 (ignoreProlongedSoundMark_ja_JP),     // 16. (1<<24  33554432) (23)

  TmItem1 (UPPERCASE_LOWERCASE),        // 19. (1) (1)
  TmItem1 (LOWERCASE_UPPERCASE),        // 20. (2) (2)
  TmItem1 (HALFWIDTH_FULLWIDTH),        // 21. (3) (3)
  TmItem1 (FULLWIDTH_HALFWIDTH),        // 22. (4) (4)
  TmItem1 (KATAKANA_HIRAGANA),          // 23. (5) (5)
  TmItem1 (HIRAGANA_KATAKANA),          // 24. (6) (6)

  TmItem1 (smallToLarge_ja_JP),         // 25. (1<<27 268435456) (26)
  TmItem1 (largeToSmall_ja_JP),         // 26. (1<<28 536870912) (27)
  TmItem2 (NumToTextLower_zh_CN),       // 27. () (28)
  TmItem2 (NumToTextUpper_zh_CN),       // 28. () (29)
  TmItem2 (NumToTextLower_zh_TW),       // 29. () (30)
  TmItem2 (NumToTextUpper_zh_TW),       // 30. () (31)
  TmItem2 (NumToTextFormalHangul_ko),   // 31. () (32)
  TmItem2 (NumToTextFormalLower_ko),    // 32. () (33)
  TmItem2 (NumToTextFormalUpper_ko),    // 33. () (34)
  TmItem2 (NumToTextInformalHangul_ko), // 34. () (35)
  TmItem2 (NumToTextInformalLower_ko),  // 35. () (36)
  TmItem2 (NumToTextInformalUpper_ko),  // 36. () (37)
  TmItem2 (NumToCharLower_zh_CN),       // 37. () (38)
  TmItem2 (NumToCharUpper_zh_CN),       // 38. () (39)
  TmItem2 (NumToCharLower_zh_TW),       // 39. () (40)
  TmItem2 (NumToCharUpper_zh_TW),       // 40. () (41)
  TmItem2 (NumToCharHangul_ko),         // 41. () (42)
  TmItem2 (NumToCharLower_ko),          // 42. () (43)
  TmItem2 (NumToCharUpper_ko),          // 43. () (44)
  TmItem2 (NumToCharFullwidth),         // 44. () (45)
  TmItem2 (NumToCharKanjiShort_ja_JP),  // 45. () (46)
  TmItem2 (TextToNumLower_zh_CN),       // 46. () (47)
  TmItem2 (TextToNumUpper_zh_CN),       // 47. () (48)
  TmItem2 (TextToNumLower_zh_TW),       // 48. () (49)
  TmItem2 (TextToNumUpper_zh_TW),       // 49. () (50)
  TmItem2 (TextToNumFormalHangul_ko),   // 50. () (51)
  TmItem2 (TextToNumFormalLower_ko),    // 51. () (52)
  TmItem2 (TextToNumFormalUpper_ko),    // 52. () (53)
  TmItem2 (TextToNumInformalHangul_ko), // 53. () (54)
  TmItem2 (TextToNumInformalLower_ko),  // 54. () (55)
  TmItem2 (TextToNumInformalUpper_ko),  // 55. () (56)

  TmItem2 (CharToNumLower_zh_CN),       // 56. () (59)
  TmItem2 (CharToNumUpper_zh_CN),       // 57. () (60)
  TmItem2 (CharToNumLower_zh_TW),       // 58. () (61)
  TmItem2 (CharToNumUpper_zh_TW),       // 59. () (62)
  TmItem2 (CharToNumHangul_ko),         // 60. () (63)
  TmItem2 (CharToNumLower_ko),          // 61. () (64)
  TmItem2 (CharToNumUpper_ko),          // 62. () (65)

// no enum defined for these trans. application has to use impl name to load them
//  TmItem2 (NumToCharArabic_Indic),    // () (67)
//  TmItem2 (NumToCharEstern_Arabic_Indic),// () (68)
//  TmItem2 (NumToCharIndic),           // () (69)
//  TmItem2 (NumToCharThai),            // () (70)
  {(TransliterationModules)0, (TransliterationModulesNew)0,  NULL}
};

// Constructor/Destructor
TransliterationImpl::TransliterationImpl(const Reference <XComponentContext>& xContext) : mxContext(xContext)
{
    numCascade = 0;
    caseignoreOnly = sal_True;

    mxLocaledata.set(LocaleData::create(xContext));
}

TransliterationImpl::~TransliterationImpl()
{
    mxLocaledata.clear();
    clear();
}


// Methods
OUString SAL_CALL
TransliterationImpl::getName() throw(RuntimeException)
{
    if (numCascade == 1 && bodyCascade[0].is())
        return bodyCascade[0]->getName();
    if (numCascade < 1)
        return ( OUString("Not Loaded"));
    throw ERROR;
}

sal_Int16 SAL_CALL
TransliterationImpl::getType() throw(RuntimeException)
{
    if (numCascade > 1)
        return (TransliterationType::CASCADE|TransliterationType::IGNORE);
    if (numCascade > 0 && bodyCascade[0].is())
        return(bodyCascade[0]->getType());
    throw ERROR;
}

void SAL_CALL
TransliterationImpl::loadModule( TransliterationModules modType, const Locale& rLocale )
        throw(RuntimeException)
{
        clear();
    if (modType&TransliterationModules_IGNORE_MASK && modType&TransliterationModules_NON_IGNORE_MASK) {
        throw ERROR;
    } else if (modType&TransliterationModules_IGNORE_MASK) {
#define TransliterationModules_IGNORE_CASE_MASK (TransliterationModules_IGNORE_CASE | \
                                                TransliterationModules_IGNORE_WIDTH | \
                                                TransliterationModules_IGNORE_KANA)
        sal_Int32 mask = ((modType&TransliterationModules_IGNORE_CASE_MASK) == modType) ?
                TransliterationModules_IGNORE_CASE_MASK : TransliterationModules_IGNORE_MASK;
        for (sal_Int16 i = 0; TMlist[i].tm & mask; i++) {
            if (modType & TMlist[i].tm)
                if (loadModuleByName(OUString::createFromAscii(TMlist[i].implName),
                                                bodyCascade[numCascade], rLocale))
                    numCascade++;
        }
        // additional transliterations from TranslationModuleExtra (we cannot extend TransliterationModule)
        if (modType & TransliterationModulesExtra::ignoreDiacritics_CTL)
        {
            if (loadModuleByName(OUString("ignoreDiacritics_CTL"), bodyCascade[numCascade], rLocale))
                numCascade++;
        }
    } else if (modType&TransliterationModules_NON_IGNORE_MASK) {
        for (sal_Int16 i = 0; TMlist[i].tm; i++) {
            if (TMlist[i].tm == modType) {
                if (loadModuleByName(OUString::createFromAscii(TMlist[i].implName), bodyCascade[numCascade], rLocale))
                    numCascade++;
                break;
            }
        }
    }
}

void SAL_CALL
TransliterationImpl::loadModuleNew( const Sequence < TransliterationModulesNew > & modType, const Locale& rLocale )
  throw(RuntimeException)
{
    clear();
    sal_Int32 mask = 0, count = modType.getLength();
    if (count > maxCascade)
        throw ERROR; // could not handle more than maxCascade
    for (sal_Int16 i = 0; i < count; i++) {
        for (sal_Int16 j = 0; TMlist[j].tmn; j++) {
            if (TMlist[j].tmn == modType[i]) {
                if (mask == 0)
                    mask = TMlist[i].tm && (TMlist[i].tm&TransliterationModules_IGNORE_MASK) ?
                        TransliterationModules_IGNORE_MASK : TransliterationModules_NON_IGNORE_MASK;
                else if (mask == (sal_Int32) TransliterationModules_IGNORE_MASK &&
                        (TMlist[i].tm&TransliterationModules_IGNORE_MASK) == 0)
                    throw ERROR; // could not mess up ignore trans. with non_ignore trans.
                if (loadModuleByName(OUString::createFromAscii(TMlist[j].implName), bodyCascade[numCascade], rLocale))
                    numCascade++;
                break;
            }
        }
    }
}

void SAL_CALL
TransliterationImpl::loadModuleByImplName(const OUString& implName, const Locale& rLocale)
  throw(RuntimeException)
{
    clear();
    if (loadModuleByName(implName, bodyCascade[numCascade], rLocale))
        numCascade++;
}


void SAL_CALL
TransliterationImpl::loadModulesByImplNames(const Sequence< OUString >& implNameList, const Locale& rLocale ) throw(RuntimeException)
{
    if (implNameList.getLength() > maxCascade || implNameList.getLength() <= 0)
        throw ERROR;

    clear();
    for (sal_Int32 i = 0; i < implNameList.getLength(); i++)
        if (loadModuleByName(implNameList[i], bodyCascade[numCascade], rLocale))
            numCascade++;
}


Sequence<OUString> SAL_CALL
TransliterationImpl::getAvailableModules( const Locale& rLocale, sal_Int16 sType ) throw(RuntimeException)
{
    const Sequence<OUString> &translist = mxLocaledata->getTransliterations(rLocale);
    Sequence<OUString> r(translist.getLength());
    Reference<XExtendedTransliteration> body;
    sal_Int32 n = 0;
    for (sal_Int32 i = 0; i < translist.getLength(); i++)
    {
        if (loadModuleByName(translist[i], body, rLocale)) {
            if (body->getType() & sType)
                r[n++] = translist[i];
            body.clear();
        }
    }
    r.realloc(n);
    return (r);
}


OUString SAL_CALL
TransliterationImpl::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
                    Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    if (numCascade == 0)
        return inStr;

    if (offset.getLength() != nCount)
        offset.realloc(nCount);
    if (numCascade == 1)
    {
        if ( startPos == 0 && nCount == inStr.getLength() )
            return bodyCascade[0]->transliterate( inStr, 0, nCount, offset);
        else
        {
            OUString tmpStr = inStr.copy(startPos, nCount);
            tmpStr = bodyCascade[0]->transliterate(tmpStr, 0, nCount, offset);
            if ( startPos )
            {
                sal_Int32 * pArr = offset.getArray();
                nCount = offset.getLength();
                for (sal_Int32 j = 0; j < nCount; j++)
                    pArr[j] += startPos;
            }
            return tmpStr;
        }
    }
    else
    {
        OUString tmpStr = inStr.copy(startPos, nCount);
        sal_Int32 * pArr = offset.getArray();
        for (sal_Int32 j = 0; j < nCount; j++)
            pArr[j] = startPos + j;

        sal_Int16 from = 0, to = 1, tmp;
        Sequence<sal_Int32> off[2];

        off[to] = offset;
        off[from].realloc(nCount);
        for (sal_Int32 i = 0; i < numCascade; i++) {
            tmpStr = bodyCascade[i]->transliterate(tmpStr, 0, nCount, off[from]);

            nCount = tmpStr.getLength();

            tmp = from; from = to; to = tmp;
            for (sal_Int32 j = 0; j < nCount; j++)
                off[to][j] = off[from][off[to][j]];
        }
        offset = off[to];
        return tmpStr;
    }
}


//
OUString SAL_CALL
TransliterationImpl::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    if (numCascade == 0)
        return inStr;

    if (offset.getLength() != nCount)
        offset.realloc(nCount);
    if (numCascade == 1)
    {
        if ( startPos == 0 && nCount == inStr.getLength() )
            return bodyCascade[0]->folding( inStr, 0, nCount, offset);
        else
        {
            OUString tmpStr = inStr.copy(startPos, nCount);
            tmpStr = bodyCascade[0]->folding(tmpStr, 0, nCount, offset);
            if ( startPos )
            {
                sal_Int32 * pArr = offset.getArray();
                nCount = offset.getLength();
                for (sal_Int32 j = 0; j < nCount; j++)
                    pArr[j] += startPos;
            }
            return tmpStr;
        }
    }
    else
    {
        OUString tmpStr = inStr.copy(startPos, nCount);
        sal_Int32 * pArr = offset.getArray();
        for (sal_Int32 j = 0; j < nCount; j++)
            pArr[j] = startPos + j;

        sal_Int16 from = 0, to = 1, tmp;
        Sequence<sal_Int32> off[2];

        off[to] = offset;
        for (sal_Int32 i = 0; i < numCascade; i++) {
            tmpStr = bodyCascade[i]->folding(tmpStr, 0, nCount, off[from]);

            nCount = tmpStr.getLength();

            tmp = from; from = to; to = tmp;
            for (sal_Int32 j = 0; j < nCount; j++)
                off[to][j] = off[from][off[to][j]];
        }
        offset = off[to];
        return tmpStr;
    }
}

OUString SAL_CALL
TransliterationImpl::transliterateString2String( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount ) throw(RuntimeException)
{
    if (numCascade == 0)
        return inStr;
    else if (numCascade == 1)
        return bodyCascade[0]->transliterateString2String( inStr, startPos, nCount);
    else {
        OUString tmpStr = bodyCascade[0]->transliterateString2String(inStr, startPos, nCount);

        for (sal_Int32 i = 1; i < numCascade; i++)
            tmpStr = bodyCascade[i]->transliterateString2String(tmpStr, 0, tmpStr.getLength());
        return tmpStr;
    }
}

OUString SAL_CALL
TransliterationImpl::transliterateChar2String( sal_Unicode inChar ) throw(RuntimeException)
{
    if (numCascade == 0)
        return OUString(&inChar, 1);
    else if (numCascade == 1)
        return bodyCascade[0]->transliterateChar2String( inChar);
    else {
        OUString tmpStr = bodyCascade[0]->transliterateChar2String(inChar);

        for (sal_Int32 i = 1; i < numCascade; i++)
            tmpStr = bodyCascade[i]->transliterateString2String(tmpStr, 0, tmpStr.getLength());
        return tmpStr;
    }
}

sal_Unicode SAL_CALL
TransliterationImpl::transliterateChar2Char( sal_Unicode inChar ) throw(MultipleCharsOutputException, RuntimeException)
{
    sal_Unicode tmpChar = inChar;
    for (sal_Int32 i = 0; i < numCascade; i++)
        tmpChar = bodyCascade[i]->transliterateChar2Char(tmpChar);
    return tmpChar;
}


sal_Bool SAL_CALL
TransliterationImpl::equals(
    const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
    const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2)
    throw(RuntimeException)
{
    // since this is an API function make it user fail safe
    if ( nCount1 < 0 ) {
        pos1 += nCount1;
        nCount1 = -nCount1;
    }
    if ( nCount2 < 0 ) {
        pos2 += nCount2;
        nCount2 = -nCount2;
    }
    if ( !nCount1 || !nCount2 ||
            pos1 >= str1.getLength() || pos2 >= str2.getLength() ||
            pos1 < 0 || pos2 < 0 ) {
        nMatch1 = nMatch2 = 0;
        // two empty strings return true, else false
        return !nCount1 && !nCount2 && pos1 == str1.getLength() && pos2 == str2.getLength();
    }
    if ( pos1 + nCount1 > str1.getLength() )
        nCount1 = str1.getLength() - pos1;
    if ( pos2 + nCount2 > str2.getLength() )
        nCount2 = str2.getLength() - pos2;

    if (caseignoreOnly && caseignore.is())
        return caseignore->equals(str1, pos1, nCount1, nMatch1, str2, pos2, nCount2, nMatch2);

    Sequence<sal_Int32> offset1, offset2;

    OUString tmpStr1 = folding(str1, pos1, nCount1, offset1);
    OUString tmpStr2 = folding(str2, pos2, nCount2, offset2);
    // Length of offset1 and offset2 may still be 0 if there was no folding
    // necessary!

    const sal_Unicode *p1 = tmpStr1.getStr();
    const sal_Unicode *p2 = tmpStr2.getStr();
    sal_Int32 i, nLen = ::std::min( tmpStr1.getLength(), tmpStr2.getLength());
    for (i = 0; i < nLen; ++i, ++p1, ++p2 ) {
        if (*p1 != *p2) {
            // return number of matched code points so far
            nMatch1 = (i < offset1.getLength()) ? offset1[i] : i;
            nMatch2 = (i < offset2.getLength()) ? offset2[i] : i;
            return sal_False;
        }
    }
    // i==nLen
    if ( tmpStr1.getLength() != tmpStr2.getLength() ) {
        // return number of matched code points so far
        nMatch1 = (i <= offset1.getLength()) ? offset1[i-1] + 1 : i;
        nMatch2 = (i <= offset2.getLength()) ? offset2[i-1] + 1 : i;
        return sal_False;
    } else {
        nMatch1 = nCount1;
        nMatch2 = nCount2;
        return sal_True;
    }
}

#define MaxOutput 2

Sequence< OUString > SAL_CALL
TransliterationImpl::getRange(const Sequence< OUString > &inStrs,
                const sal_Int32 length, sal_Int16 _numCascade) throw(RuntimeException)
{
    if (_numCascade >= numCascade || ! bodyCascade[_numCascade].is())
        return inStrs;

    sal_Int32 j_tmp = 0;
    Sequence< OUString > ostr(MaxOutput*length);
    for (sal_Int32 j = 0; j < length; j+=2) {
        const Sequence< OUString >& temp = bodyCascade[_numCascade]->transliterateRange(inStrs[j], inStrs[j+1]);

        for ( sal_Int32 k = 0; k < temp.getLength(); k++) {
            if ( j_tmp >= MaxOutput*length ) throw ERROR;
            ostr[j_tmp++]  = temp[k];
        }
    }
    ostr.realloc(j_tmp);

    return this->getRange(ostr, j_tmp, ++_numCascade);
}


Sequence< OUString > SAL_CALL
TransliterationImpl::transliterateRange( const OUString& str1, const OUString& str2 )
throw(RuntimeException)
{
    if (numCascade == 1)
        return bodyCascade[0]->transliterateRange(str1, str2);

    Sequence< OUString > ostr(2);
    ostr[0] = str1;
    ostr[1] = str2;

    return this->getRange(ostr, 2, 0);
}


sal_Int32 SAL_CALL
TransliterationImpl::compareSubstring(
    const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2)
    throw(RuntimeException)
{
    if (caseignoreOnly && caseignore.is())
        return caseignore->compareSubstring(str1, off1, len1, str2, off2, len2);

    Sequence <sal_Int32> offset;

    OUString in_str1 = this->transliterate(str1, off1, len1, offset);
    OUString in_str2 = this->transliterate(str2, off2, len2, offset);
    const sal_Unicode* unistr1 = in_str1.getStr();
    const sal_Unicode* unistr2 = in_str2.getStr();
    sal_Int32 strlen1 = in_str1.getLength();
    sal_Int32 strlen2 = in_str2.getLength();

    while (strlen1 && strlen2) {
        if (*unistr1 != *unistr2)
           return *unistr1 > *unistr2 ? 1 : -1;

        unistr1++; unistr2++; strlen1--; strlen2--;
    }
    return strlen1 == strlen2 ? 0 : (strlen1 > strlen2 ? 1 : -1);
}


sal_Int32 SAL_CALL
TransliterationImpl::compareString(const OUString& str1, const OUString& str2 ) throw (RuntimeException)
{
    if (caseignoreOnly && caseignore.is())
        return caseignore->compareString(str1, str2);
    else
        return this->compareSubstring(str1, 0, str1.getLength(), str2, 0, str2.getLength());
}


void
TransliterationImpl::clear()
{
    for (sal_Int32 i = 0; i < numCascade; i++)
        if (bodyCascade[i].is())
            bodyCascade[i].clear();
    numCascade = 0;
    caseignore.clear();
    caseignoreOnly = sal_True;
}

namespace
{
    /** structure to cache the last transliteration body used. */
    struct TransBody
    {
        OUString Name;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XExtendedTransliteration > Body;
    };
    class theTransBodyMutex : public rtl::Static<osl::Mutex, theTransBodyMutex> {};
}

void TransliterationImpl::loadBody( OUString &implName, Reference<XExtendedTransliteration>& body )
    throw (RuntimeException)
{
    assert(!implName.isEmpty());
    ::osl::MutexGuard guard(theTransBodyMutex::get());
    static TransBody lastTransBody;
    if (implName != lastTransBody.Name)
    {
        lastTransBody.Body.set(
            mxContext->getServiceManager()->createInstanceWithContext(implName, mxContext), UNO_QUERY_THROW);
        lastTransBody.Name = implName;
    }
    body = lastTransBody.Body;
}

sal_Bool SAL_CALL
TransliterationImpl::loadModuleByName( const OUString& implName,
        Reference<XExtendedTransliteration>& body, const Locale& rLocale) throw(RuntimeException)
{
    OUString cname = OUString(TRLT_IMPLNAME_PREFIX) + implName;
    loadBody(cname, body);
    if (body.is()) {
        body->loadModule((TransliterationModules)0, rLocale); // toUpper/toLoad need rLocale

        // if the module is ignore case/kana/width, load caseignore for equals/compareString mothed
        for (sal_Int16 i = 0; i < 3; i++) {
            if (implName.compareToAscii(TMlist[i].implName) == 0) {
                if (i == 0) // current module is caseignore
                    body->loadModule(TMlist[0].tm, rLocale); // caseingore need to setup module name
                if (! caseignore.is()) {
                    OUString bname = OUString(TRLT_IMPLNAME_PREFIX) +
                                OUString::createFromAscii(TMlist[0].implName);
                    loadBody(bname, caseignore);
                }
                if (caseignore.is())
                    caseignore->loadModule(TMlist[i].tm, rLocale);
                return sal_True;
            }
        }
        caseignoreOnly = sal_False; // has other module than just ignore case/kana/width
    }
    return body.is();
}

const sal_Char cTrans[] = "com.sun.star.i18n.Transliteration";

OUString SAL_CALL
TransliterationImpl::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cTrans);
}


sal_Bool SAL_CALL
TransliterationImpl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cTrans);
}

Sequence< OUString > SAL_CALL
TransliterationImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cTrans);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
