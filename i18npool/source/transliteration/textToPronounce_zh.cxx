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

#include <sal/config.h>

#include <com/sun/star/i18n/MultipleCharsOutputException.hpp>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <textToPronounce_zh.hxx>

using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL TextToPronounce_zh::getType()
{
    return TransliterationType::ONE_TO_ONE| TransliterationType::IGNORE;
}

const sal_Unicode* SAL_CALL
TextToPronounce_zh::getPronounce(const sal_Unicode ch)
{
    static const sal_Unicode emptyString[]={0};
    if (idx) {
        sal_uInt16 address = idx[0][ch>>8];
        if (address != 0xFFFF)
            return reinterpret_cast<sal_Unicode *>(
                &idx[2][idx[1][address + (ch & 0xFF)]]);
    }
    return emptyString;
}

OUString SAL_CALL
TextToPronounce_zh::folding(const OUString & inStr, sal_Int32 startPos,
        sal_Int32 nCount, Sequence< sal_Int32 > & offset)
{
    OUStringBuffer sb;
    const sal_Unicode * chArr = inStr.getStr() + startPos;

    if (startPos < 0)
        throw RuntimeException();

    if (startPos + nCount > inStr.getLength())
        nCount = inStr.getLength() - startPos;

    offset[0] = 0;
    for (sal_Int32 i = 0; i < nCount; i++) {
        OUString pron(getPronounce(chArr[i]));
        sb.append(pron);

        if (useOffset)
            offset[i + 1] = offset[i] + pron.getLength();
    }
    return sb.makeStringAndClear();
}

OUString SAL_CALL
TextToPronounce_zh::transliterateChar2String( sal_Unicode inChar)
{
    return OUString(getPronounce(inChar));
}

sal_Unicode SAL_CALL
TextToPronounce_zh::transliterateChar2Char( sal_Unicode inChar)
{
    const sal_Unicode* pron=getPronounce(inChar);
    if (!pron || !pron[0])
        return 0;
    if (pron[1])
        throw MultipleCharsOutputException();
    return *pron;
}

sal_Bool SAL_CALL
TextToPronounce_zh::equals( const OUString & str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32 & nMatch1,
        const OUString & str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32 & nMatch2)
{
    sal_Int32 realCount;
    int i;  // loop variable
    const sal_Unicode * s1, * s2;

    if (nCount1 + pos1 > str1.getLength())
        nCount1 = str1.getLength() - pos1;

    if (nCount2 + pos2 > str2.getLength())
        nCount2 = str2.getLength() - pos2;

    realCount = ((nCount1 > nCount2) ? nCount2 : nCount1);

    s1 = str1.getStr() + pos1;
    s2 = str2.getStr() + pos2;
    for (i = 0; i < realCount; i++) {
        const sal_Unicode *pron1 = getPronounce(*s1++);
        const sal_Unicode *pron2 = getPronounce(*s2++);
        if (pron1 != pron2) {
            nMatch1 = nMatch2 = i;
            return false;
        }
    }
    nMatch1 = nMatch2 = realCount;
    return (nCount1 == nCount2);
}

#ifdef DISABLE_DYNLOADING

extern "C" {

sal_uInt16** get_zh_zhuyin();
sal_uInt16** get_zh_pinyin();

}

#endif

TextToPinyin_zh_CN::TextToPinyin_zh_CN() :
#ifndef DISABLE_DYNLOADING
    TextToPronounce_zh("get_zh_pinyin")
#else
    TextToPronounce_zh(get_zh_pinyin)
#endif
{
        transliterationName = "ChineseCharacterToPinyin";
        implementationName = "com.sun.star.i18n.Transliteration.TextToPinyin_zh_CN";
}

TextToChuyin_zh_TW::TextToChuyin_zh_TW() :
#ifndef DISABLE_DYNLOADING
    TextToPronounce_zh("get_zh_zhuyin")
#else
    TextToPronounce_zh(get_zh_zhuyin)
#endif
{
        transliterationName = "ChineseCharacterToChuyin";
        implementationName = "com.sun.star.i18n.Transliteration.TextToChuyin_zh_TW";
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

TextToPronounce_zh::TextToPronounce_zh(const sal_Char* func_name)
{
#ifdef SAL_DLLPREFIX
    OUString lib(SAL_DLLPREFIX"index_data" SAL_DLLEXTENSION);
#else
    OUString lib("index_data" SAL_DLLEXTENSION);
#endif
    hModule = osl_loadModuleRelative(
        &thisModule, lib.pData, SAL_LOADMODULE_DEFAULT );
    idx=nullptr;
    if (hModule) {
        sal_uInt16** (*function)() = reinterpret_cast<sal_uInt16** (*)()>(osl_getFunctionSymbol(hModule, OUString::createFromAscii(func_name).pData));
        if (function)
            idx=function();
    }
}

#else

TextToPronounce_zh::TextToPronounce_zh(sal_uInt16 ** (*function)())
{
    idx = function();
}

#endif

TextToPronounce_zh::~TextToPronounce_zh()
{
#ifndef DISABLE_DYNLOADING
    if (hModule) osl_unloadModule(hModule);
#endif
}
} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
