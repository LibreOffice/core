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
#include <comphelper/string.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star::uno;


namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL transliteration_Numeric::getType() throw(RuntimeException, std::exception)
{
    return TransliterationType::NUMERIC;
}

OUString SAL_CALL
    transliteration_Numeric::folding( const OUString& /*inStr*/, sal_Int32 /*startPos*/, sal_Int32 /*nCount*/, Sequence< sal_Int32 >& /*offset*/ )
throw(RuntimeException, std::exception)
{
    throw RuntimeException();
}

sal_Bool SAL_CALL
    transliteration_Numeric::equals( const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/, sal_Int32& /*nMatch1*/, const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/ )
throw(RuntimeException, std::exception)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
    transliteration_Numeric::transliterateRange( const OUString& /*str1*/, const OUString& /*str2*/ )
throw(RuntimeException, std::exception)
{
    throw RuntimeException();
}


#define isNumber(c) ((c) >= 0x30 && (c) <= 0x39)
#define NUMBER_ZERO 0x30

OUString SAL_CALL
transliteration_Numeric::transliterateBullet( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        std::vector< sal_Int32 >& offset ) throw(RuntimeException)
{
    sal_Int32 number = -1, j = 0, endPos = startPos + nCount;

    if (endPos >  inStr.getLength())
        endPos = inStr.getLength();

    rtl_uString* pStr = rtl_uString_alloc(nCount);
    sal_Unicode* out = pStr->buffer;

    if (useOffset)
        offset.resize(nCount);

    for (sal_Int32 i = startPos; i < endPos; i++) {
        if (i < endPos && isNumber(inStr[i])) {
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
            } if (number > tableSize && !recycleSymbol) {
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
        offset.resize(j);

    return OUString( pStr, SAL_NO_ACQUIRE );
}

OUString SAL_CALL
transliteration_Numeric::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset ) throw(RuntimeException, std::exception)
{
    std::vector< sal_Int32 > aTmp;
    comphelper::sequenceToContainer(aTmp, offset);
    OUString ret;
    if (tableSize)
        ret = transliterateBullet( inStr, startPos, nCount, aTmp);
    else
        ret = NativeNumberSupplierService(useOffset).getNativeNumberString( inStr.copy(startPos, nCount), aLocale, nNativeNumberMode, aTmp );
    offset = comphelper::containerToSequence(aTmp);
    return ret;
}

sal_Unicode SAL_CALL
transliteration_Numeric::transliterateChar2Char( sal_Unicode inChar ) throw(RuntimeException, MultipleCharsOutputException, std::exception)
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
        return NativeNumberSupplierService().getNativeNumberChar( inChar, aLocale, nNativeNumberMode );
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
