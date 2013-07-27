/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <transliteration_Numeric.hxx>
#include <nativenumbersupplier.hxx>
#include <defaultnumberingprovider.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

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

        rtl_uString* pStr = x_rtl_uString_new_WithLength( nCount );  // our x_rtl_ustring.h
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

        return OUString( pStr, SAL_NO_ACQUIRE ); // take over ownership of <pStr>
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
