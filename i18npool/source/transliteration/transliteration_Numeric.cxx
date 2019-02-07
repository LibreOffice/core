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


#include <transliteration_Numeric.hxx>
#include <nativenumbersupplier.hxx>
#include <defaultnumberingprovider.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;


namespace i18npool {

sal_Int16 SAL_CALL transliteration_Numeric::getType()
{
    return TransliterationType::NUMERIC;
}

OUString
    transliteration_Numeric::foldingImpl( const OUString& /*inStr*/, sal_Int32 /*startPos*/, sal_Int32 /*nCount*/, Sequence< sal_Int32 >& /*offset*/, bool )
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


#define isNumber(c) ((c) >= 0x30 && (c) <= 0x39)
#define NUMBER_ZERO 0x30

OUString
transliteration_Numeric::transliterateBullet( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset, bool useOffset )
{
    sal_Int32 number = -1, j = 0, endPos = startPos + nCount;

    if (endPos >  inStr.getLength())
        endPos = inStr.getLength();

    rtl_uString* pStr = rtl_uString_alloc(nCount);
    sal_Unicode* out = pStr->buffer;

    if (useOffset)
        offset.realloc(nCount);

    for (sal_Int32 i = startPos; i < endPos; i++) {
        if (isNumber(inStr[i]))
        {
            if (number == -1) {
                startPos = i;
                number = (inStr[i] - NUMBER_ZERO);
            } else  {
                number = number * 10 + (inStr[i] - NUMBER_ZERO);
            }
        } else {
            if (number == 0) {
                if (useOffset)
                    offset[j] = startPos;
                out[j++] = NUMBER_ZERO;
            } else if (number > tableSize && !recycleSymbol) {
                for (sal_Int32 k = startPos; k < i; k++) {
                    if (useOffset)
                        offset[j] = k;
                    out[j++] = inStr[k];
                }
            } else if (number > 0) {
                if (useOffset)
                    offset[j] = startPos;
                out[j++] = table[--number % tableSize];
            } else if (i < endPos) {
                if (useOffset)
                    offset[j] = i;
                out[j++] = inStr[i];
            }
            number = -1;
        }
    }
    out[j] = 0;

    if (useOffset)
        offset.realloc(j);

    return OUString( pStr, SAL_NO_ACQUIRE );
}

OUString
transliteration_Numeric::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset, bool useOffset )
{
    if (tableSize)
        return transliterateBullet( inStr, startPos, nCount, offset, useOffset);
    else
        return rtl::Reference<NativeNumberSupplierService>(new NativeNumberSupplierService(useOffset))->getNativeNumberString( inStr.copy(startPos, nCount), aLocale, nNativeNumberMode, offset );
}

sal_Unicode SAL_CALL
transliteration_Numeric::transliterateChar2Char( sal_Unicode inChar )
{
    if (tableSize) {
        if (isNumber(inChar)) {
            sal_Int16 number = inChar - NUMBER_ZERO;
            if (number <= tableSize || recycleSymbol)
                return table[--number % tableSize];
        }
        return inChar;
    }
    else
        return rtl::Reference<NativeNumberSupplierService>(new NativeNumberSupplierService)->getNativeNumberChar( inChar, aLocale, nNativeNumberMode );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
