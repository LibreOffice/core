/*************************************************************************
 *
 *  $RCSfile: textToPronounce_zh.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 10:54:50 $
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

// prevent internal compiler error with MSVC6SP3
#include <stl/utility>

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

OUString SAL_CALL
TextToPronounce_zh::folding(const OUString & inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 > & offset) throw (RuntimeException)
{
    return transliterate(inStr, startPos, nCount, offset);
}

OUString SAL_CALL
TextToPronounce_zh::transliterate(const OUString & inStr, sal_Int32 startPos,
    sal_Int32 nCount, Sequence< sal_Int32 > & offset) throw (RuntimeException)
{
    sal_Unicode u;
    OUStringBuffer sb;
    const sal_Unicode * chArr = inStr.getStr() + startPos;

    sal_Int32 j;
    if (startPos < 0)
        throw RuntimeException();

    if (startPos + nCount > inStr.getLength())
        nCount = inStr.getLength() - startPos;

    offset[0] = 0;
    for (sal_Int32 i = 0; i < nCount; i++) {
        u = chArr[i];
        j = pronTab[u];
        if (j == -1) {
            offset[i + 1] = offset[i];
            continue;
        }

        if (j == -1)
            break;
        sb.append(&pronList[pronIdx[j]], pronIdx[j + 1] - pronIdx[j]);

        offset[i + 1] = offset[i] + pronIdx[j + 1] - pronIdx[j];
    }
    return OUString(sb.getStr());
}

sal_Bool SAL_CALL
TextToPronounce_zh::equals( const OUString & str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32 & nMatch1,
    const OUString & str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32 & nMatch2)
    throw (RuntimeException)
{
    sal_Int32 realCount;
    int i;  // loop variable
    const sal_Unicode * s1, * s2;
    sal_Unicode u1, u2;

    if (nCount1 + pos1 > str1.getLength())
        nCount1 = str1.getLength() - pos1;

    if (nCount2 + pos2 > str2.getLength())
        nCount2 = str2.getLength() - pos2;

    realCount = ((nCount1 > nCount2) ? nCount2 : nCount1);

    s1 = str1.getStr() + pos1;
    s2 = str2.getStr() + pos2;
    for (i = 0; i < realCount; i++) {
        u1 = * s1++;
        u2 = * s2 ++;
        if (pronTab[u1] != pronTab[u2]) {
            nMatch1 = nMatch2 = i;
            return sal_False;
        }
    }
    nMatch1 = nMatch2 = realCount;
    return sal_True && (nCount1 == nCount2);
}

#include <data/pron_zh_cn.h>

TextToPinyin_zh_CN::TextToPinyin_zh_CN() {
    pronList = pronList_zh_cn;
    pronIdx = pronIdx_zh_cn;
    pronTab = pronTab_zh_cn;
    transliterationName = "ChineseCharacterToPinyin";
    implementationName = "com.sun.star.i18n.Transliteration.TextToPinyin_zh_CN";
}

#include <data/pron_zh_tw.h>

TextToChuyin_zh_TW::TextToChuyin_zh_TW() {
    pronList = pronList_zh_tw;
    pronIdx = pronIdx_zh_tw;
    pronTab = pronTab_zh_tw;
    transliterationName = "ChineseCharacterToChuyin";
    implementationName = "com.sun.star.i18n.Transliteration.TextToChuyin_zh_TW";
}

} } } }
