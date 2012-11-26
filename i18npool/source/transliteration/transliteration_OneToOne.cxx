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

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL transliteration_OneToOne::getType() throw(RuntimeException)
{
        // This type is also defined in com/sun/star/util/TransliterationType.hdl
        return TransliterationType::ONE_TO_ONE;
}

OUString SAL_CALL
transliteration_OneToOne::folding( const OUString& /*inStr*/, sal_Int32 /*startPos*/,
        sal_Int32 /*nCount*/, Sequence< sal_Int32 >& /*offset*/) throw(RuntimeException)
{
        throw RuntimeException();
}

sal_Bool SAL_CALL
transliteration_OneToOne::equals( const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/,
        sal_Int32& /*nMatch1*/, const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/ )
        throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
transliteration_OneToOne::transliterateRange( const OUString& /*str1*/, const OUString& /*str2*/ )
        throw(RuntimeException)
{
    throw RuntimeException();
}

OUString SAL_CALL
transliteration_OneToOne::transliterate( const OUString& inStr, sal_Int32 startPos,
    sal_Int32 nCount, Sequence< sal_Int32 >& offset)
    throw(RuntimeException)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 0 now.
    rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    // Allocate nCount length to offset argument.
    sal_Int32 *p = 0;
    sal_Int32 position = 0;
    if (useOffset) {
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
    }

    // Translation
    while (nCount -- > 0) {
    sal_Unicode c = *src++;
    *dst ++ = func ? func( c) : (*table)[ c ];
    if (useOffset)
        *p ++ = position ++;
    }
    *dst = (sal_Unicode) 0;

    return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}

sal_Unicode SAL_CALL
transliteration_OneToOne::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException)
{
    return func ? func( inChar) : (*table)[ inChar ];
}

} } } }

