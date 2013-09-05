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


#include <transliteration_commonclass.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

transliteration_commonclass::transliteration_commonclass()
{
    transliterationName = "";
    implementationName = "";
    useOffset = sal_True;
}

OUString SAL_CALL transliteration_commonclass::getName() throw(RuntimeException)
{
    return OUString::createFromAscii(transliterationName);
}

void SAL_CALL transliteration_commonclass::loadModule( TransliterationModules /*modName*/, const Locale& rLocale )
throw(RuntimeException)
{
    aLocale = rLocale;
}


void SAL_CALL
transliteration_commonclass::loadModuleNew( const Sequence < TransliterationModulesNew >& /*modName*/, const Locale& /*rLocale*/ )
throw(RuntimeException)
{
    throw RuntimeException();
}


void SAL_CALL
transliteration_commonclass::loadModuleByImplName( const OUString& /*implName*/, const Locale& /*rLocale*/ )
throw(RuntimeException)
{
    throw RuntimeException();
}

void SAL_CALL
transliteration_commonclass::loadModulesByImplNames(const Sequence< OUString >& /*modNamelist*/, const Locale& /*rLocale*/)
throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
transliteration_commonclass::getAvailableModules( const Locale& /*rLocale*/, sal_Int16 /*sType*/ )
throw(RuntimeException)
{
    throw RuntimeException();
}

sal_Int32 SAL_CALL
transliteration_commonclass::compareSubstring(
        const OUString& str1, sal_Int32 off1, sal_Int32 len1,
        const OUString& str2, sal_Int32 off2, sal_Int32 len2)
throw(RuntimeException)
{
    const sal_Unicode* unistr1 = NULL;
    const sal_Unicode* unistr2 = NULL;
    sal_uInt32 strlen1;
    sal_uInt32 strlen2;

    Sequence <sal_Int32> offset1(2*len1);
    Sequence <sal_Int32> offset2(2*len2);

    OUString in_str1 = this->transliterate(str1, off1, len1, offset1);
    OUString in_str2 = this->transliterate(str2, off2, len2, offset2);
    strlen1 = in_str1.getLength();
    strlen2 = in_str2.getLength();
    unistr1 = in_str1.getStr();
    unistr2 = in_str2.getStr();

    while (strlen1 && strlen2)
    {
        sal_uInt32 ret = *unistr1 - *unistr2;
        if (ret)
            return ret;

        unistr1++;
        unistr2++;
        strlen1--;
        strlen2--;
    }
    return strlen1 - strlen2;
}

sal_Int32 SAL_CALL
transliteration_commonclass::compareString( const OUString& str1, const OUString& str2 ) throw ( RuntimeException)
{
    return( this->compareSubstring(str1, 0, str1.getLength(), str2, 0, str2.getLength()));
}

OUString SAL_CALL
transliteration_commonclass::transliterateString2String( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount ) throw(RuntimeException)
{
    Sequence < sal_Int32 > dummy_offset;
    useOffset = sal_False;
    OUString tmpStr = transliterate(inStr, startPos, nCount, dummy_offset);
    useOffset = sal_True;
    return tmpStr;
}

OUString SAL_CALL
transliteration_commonclass::transliterateChar2String( sal_Unicode inChar ) throw(RuntimeException)
{
    return transliteration_commonclass::transliterateString2String(OUString(&inChar, 1), 0, 1);
}

OUString SAL_CALL transliteration_commonclass::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(implementationName);
}

const sal_Char cTrans[] = "com.sun.star.i18n.Transliteration.l10n";

sal_Bool SAL_CALL transliteration_commonclass::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.equalsAscii(cTrans);
}

Sequence< OUString > SAL_CALL transliteration_commonclass::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cTrans);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
