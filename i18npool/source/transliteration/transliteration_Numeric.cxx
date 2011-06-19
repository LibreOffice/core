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

#include <transliteration_Numeric.hxx>
#include <nativenumbersupplier.hxx>
#include <defaultnumberingprovider.hxx>

using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL transliteration_Numeric::getType() throw(RuntimeException)
{
        return TransliterationType::NUMERIC;
}

OUString SAL_CALL
transliteration_Numeric::folding( const OUString& /*inStr*/, sal_Int32 /*startPos*/, sal_Int32 /*nCount*/, Sequence< sal_Int32 >& /*offset*/ )
        throw(RuntimeException)
{
        throw (new RuntimeException());
}

sal_Bool SAL_CALL
transliteration_Numeric::equals( const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/, sal_Int32& /*nMatch1*/, const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/ )
        throw(RuntimeException)
{
        throw (new RuntimeException());
}

Sequence< OUString > SAL_CALL
transliteration_Numeric::transliterateRange( const OUString& /*str1*/, const OUString& /*str2*/ )
        throw(RuntimeException)
{
        throw (new RuntimeException());
}


#define isNumber(c) ((c) >= 0x30 && (c) <= 0x39)
#define NUMBER_ZERO 0x30

OUString SAL_CALL
transliteration_Numeric::transliterateBullet( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
        sal_Int32 number = -1, j = 0, endPos = startPos + nCount;

        if (endPos >  inStr.getLength())
            endPos = inStr.getLength();

        rtl_uString* pStr = x_rtl_uString_new_WithLength( nCount, 1 );  // our x_rtl_ustring.h
        sal_Unicode* out = pStr->buffer;

        if (useOffset)
            offset.realloc(nCount);

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
            offset.realloc(j);

        return OUString( pStr, SAL_NO_ACQUIRE );
}

OUString SAL_CALL
transliteration_Numeric::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
        if (tableSize)
            return transliterateBullet( inStr, startPos, nCount, offset);
        else
            return NativeNumberSupplier(useOffset).getNativeNumberString( inStr.copy(startPos, nCount), aLocale, nNativeNumberMode, offset );
}

sal_Unicode SAL_CALL
transliteration_Numeric::transliterateChar2Char( sal_Unicode inChar ) throw(RuntimeException, MultipleCharsOutputException)
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
            return NativeNumberSupplier().getNativeNumberChar( inChar, aLocale, nNativeNumberMode );
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
