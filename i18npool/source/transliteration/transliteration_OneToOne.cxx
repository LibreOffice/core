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

#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL transliteration_OneToOne::getType() throw(RuntimeException, std::exception)
{
        // This type is also defined in com/sun/star/util/TransliterationType.hdl
        return TransliterationType::ONE_TO_ONE;
}

OUString SAL_CALL
transliteration_OneToOne::folding( const OUString& /*inStr*/, sal_Int32 /*startPos*/,
        sal_Int32 /*nCount*/, Sequence< sal_Int32 >& /*offset*/) throw(RuntimeException, std::exception)
{
        throw RuntimeException();
}

sal_Bool SAL_CALL
transliteration_OneToOne::equals( const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/,
        sal_Int32& /*nMatch1*/, const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/ )
        throw(RuntimeException, std::exception)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
transliteration_OneToOne::transliterateRange( const OUString& /*str1*/, const OUString& /*str2*/ )
        throw(RuntimeException, std::exception)
{
    throw RuntimeException();
}

OUString SAL_CALL
transliteration_OneToOne::transliterate( const OUString& inStr, sal_Int32 startPos,
    sal_Int32 nCount, Sequence< sal_Int32 >& offset)
    throw(RuntimeException, std::exception)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 1 now.
    rtl_uString * newStr = rtl_uString_alloc(nCount);
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    // Allocate nCount length to offset argument.
    sal_Int32 *p = nullptr;
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

    return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

sal_Unicode SAL_CALL
transliteration_OneToOne::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException, std::exception)
{
    return func ? func( inChar) : (*table)[ inChar ];
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
