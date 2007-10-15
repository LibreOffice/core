/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textToPronounce_zh.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:54:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
using namespace rtl;

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
    OUString lib=OUString::createFromAscii(SAL_DLLPREFIX"index_data"SAL_DLLEXTENSION);
#else
    OUString lib=OUString::createFromAscii("index_data"SAL_DLLEXTENSION);
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
