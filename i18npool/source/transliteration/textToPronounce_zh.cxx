/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#define TRANSLITERATION_ALL
#include <textToPronounce_zh.hxx>

using namespace com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL TextToPronounce_zh::getType() throw (RuntimeException)
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
            return &idx[2][idx[1][address + (ch & 0xFF)]];
    }
    return emptyString;
}

OUString SAL_CALL
TextToPronounce_zh::folding(const OUString & inStr, sal_Int32 startPos,
        sal_Int32 nCount, Sequence< sal_Int32 > & offset) throw (RuntimeException)
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
TextToPronounce_zh::transliterateChar2String( sal_Unicode inChar) throw(RuntimeException)
{
    return OUString(getPronounce(inChar));
}

sal_Unicode SAL_CALL
TextToPronounce_zh::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException)
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
        throw (RuntimeException)
{
    sal_Int32 realCount;
    int i;  // loop variable
    const sal_Unicode * s1, * s2;
    const sal_Unicode *pron1, *pron2;

    if (nCount1 + pos1 > str1.getLength())
        nCount1 = str1.getLength() - pos1;

    if (nCount2 + pos2 > str2.getLength())
        nCount2 = str2.getLength() - pos2;

    realCount = ((nCount1 > nCount2) ? nCount2 : nCount1);

    s1 = str1.getStr() + pos1;
    s2 = str2.getStr() + pos2;
    for (i = 0; i < realCount; i++) {
        pron1=getPronounce(*s1++);
        pron2=getPronounce(*s2++);
        if (pron1 != pron2) {
            nMatch1 = nMatch2 = i;
            return sal_False;
        }
    }
    nMatch1 = nMatch2 = realCount;
    return (nCount1 == nCount2);
}

TextToPinyin_zh_CN::TextToPinyin_zh_CN() : TextToPronounce_zh("get_zh_pinyin")
{
        transliterationName = "ChineseCharacterToPinyin";
        implementationName = "com.sun.star.i18n.Transliteration.TextToPinyin_zh_CN";
}

TextToChuyin_zh_TW::TextToChuyin_zh_TW() : TextToPronounce_zh("get_zh_zhuyin")
{
        transliterationName = "ChineseCharacterToChuyin";
        implementationName = "com.sun.star.i18n.Transliteration.TextToChuyin_zh_TW";
}

extern "C" { static void SAL_CALL thisModule() {} }

TextToPronounce_zh::TextToPronounce_zh(const sal_Char* func_name)
{
#ifdef SAL_DLLPREFIX
    OUString lib(RTL_CONSTASCII_USTRINGPARAM(SAL_DLLPREFIX"index_data"SAL_DLLEXTENSION));
#else
    OUString lib(RTL_CONSTASCII_USTRINGPARAM("index_data"SAL_DLLEXTENSION));
#endif
    hModule = osl_loadModuleRelative(
        &thisModule, lib.pData, SAL_LOADMODULE_DEFAULT );
    idx=NULL;
    if (hModule) {
        sal_uInt16** (*function)() = (sal_uInt16** (*)()) osl_getFunctionSymbol(hModule, OUString::createFromAscii(func_name).pData);
        if (function)
            idx=function();
    }
}
TextToPronounce_zh::~TextToPronounce_zh()
{
    if (hModule) osl_unloadModule(hModule);
}
} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
