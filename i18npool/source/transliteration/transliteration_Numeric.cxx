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


#include <com/sun/star/i18n/TransliterationType.hpp>

#include <transliteration_Numeric.hxx>
#include <nativenumbersupplier.hxx>
#include <rtl/character.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;


namespace i18npool {

sal_Int16 SAL_CALL transliteration_Numeric::getType()
{
    return TransliterationType::NUMERIC;
}

OUString
    transliteration_Numeric::foldingImpl( const OUString& /*inStr*/, sal_Int32 /*startPos*/, sal_Int32 /*nCount*/, Sequence< sal_Int32 >* /*pOffset*/ )
{
    throw RuntimeException();
}

sal_Bool SAL_CALL
    transliteration_Numeric::equals( const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/, sal_Int32& /*nMatch1*/, const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/ )
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
    transliteration_Numeric::transliterateRange( const OUString& /*str1*/, const OUString& /*str2*/ )
{
    throw RuntimeException();
}


#define NUMBER_ZERO 0x30

OUString
transliteration_Numeric::transliterateBullet( std::u16string_view inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >* pOffset ) const
{
    sal_Int32 number = -1, j = 0, endPos = startPos + nCount;

    if (endPos > static_cast<sal_Int32>(inStr.size()))
        endPos = inStr.size();

    rtl_uString* pStr = rtl_uString_alloc(nCount);
    sal_Unicode* out = pStr->buffer;

    if (pOffset)
        pOffset->realloc(nCount);
    auto ppOffset = pOffset ? pOffset->getArray() : nullptr;

    for (sal_Int32 i = startPos; i < endPos; i++) {
        if (rtl::isAsciiDigit(inStr[i]))
        {
            if (number == -1) {
                startPos = i;
                number = (inStr[i] - NUMBER_ZERO);
            } else  {
                number = number * 10 + (inStr[i] - NUMBER_ZERO);
            }
        } else {
            if (number == 0) {
                if (ppOffset)
                    ppOffset[j] = startPos;
                out[j++] = NUMBER_ZERO;
            } else if (number > tableSize && !recycleSymbol) {
                for (sal_Int32 k = startPos; k < i; k++) {
                    if (ppOffset)
                        ppOffset[j] = k;
                    out[j++] = inStr[k];
                }
            } else if (number > 0) {
                if (ppOffset)
                    ppOffset[j] = startPos;
                out[j++] = table[--number % tableSize];
            } else if (i < endPos) {
                if (ppOffset)
                    ppOffset[j] = i;
                out[j++] = inStr[i];
            }
            number = -1;
        }
    }
    out[j] = 0;

    if (pOffset)
        pOffset->realloc(j);

    return OUString( pStr, SAL_NO_ACQUIRE );
}

OUString
transliteration_Numeric::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >* pOffset )
{
    if (tableSize)
        return transliterateBullet( inStr, startPos, nCount, pOffset);
    else
        return rtl::Reference(new NativeNumberSupplierService())->getNativeNumberString( inStr.copy(startPos, nCount), aLocale, nNativeNumberMode, pOffset );
}

sal_Unicode SAL_CALL
transliteration_Numeric::transliterateChar2Char( sal_Unicode inChar )
{
    if (tableSize) {
        if (rtl::isAsciiDigit(inChar)) {
            sal_Int16 number = inChar - NUMBER_ZERO;
            if (number <= tableSize || recycleSymbol)
                return table[--number % tableSize];
        }
        return inChar;
    }
    else
        return NativeNumberSupplierService::getNativeNumberChar( inChar, aLocale, nNativeNumberMode );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
