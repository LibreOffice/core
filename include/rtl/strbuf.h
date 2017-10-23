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

#ifndef INCLUDED_RTL_STRBUF_H
#define INCLUDED_RTL_STRBUF_H

#include "sal/config.h"

#include "rtl/string.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Allocates a new <code>String</code> that contains characters from
    the character array argument.

    The <code>count</code> argument specifies
    the length of the array. The initial capacity of the string buffer is
    <code>16</code> plus the length of the string argument.

    @param  newStr   out parameter, contains the new string. The reference count is 1.
    @param  value    the initial value of the string.
    @param  count    the length of value.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_newFromStr_WithLength(
                                                      rtl_String ** newStr,
                                                      const sal_Char * value,
                                                      sal_Int32 count);

/**
    Allocates a new <code>String</code> that contains the same sequence of
    characters as the string argument.

    The initial capacity is the larger of:
    <ul>
    <li> The <code>bufferLen</code> argument.
    <li> The <code>length</code> of the string argument.
    </ul>

    @param  newStr      out parameter, contains the new string. The reference count is 1.
    @param  capacity    the initial len of the string buffer.
    @param  oldStr      the initial value of the string.
    @return the new capacity of the string buffer
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_stringbuffer_newFromStringBuffer(
                                                         rtl_String ** newStr,
                                                         sal_Int32 capacity,
                                                         rtl_String * oldStr );

/**
    Ensures that the capacity of the buffer is at least equal to the
    specified minimum.

    If the current capacity of this string buffer is less than the
    argument, then a new internal buffer is allocated with greater
    capacity. The new capacity is the larger of:
    <ul>
    <li>The <code>minimumCapacity</code> argument.
    <li>Twice the old capacity, plus <code>2</code>.
    </ul>
    If the <code>minimumCapacity</code> argument is nonpositive, this
    method takes no action and simply returns.

    @param[in,out]   This              the String to operate on.
    @param[in,out]   capacity          in: old capacity, out: new capacity.
    @param[in]       minimumCapacity   the minimum desired capacity.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_ensureCapacity(
                                                rtl_String ** This,
                                                sal_Int32* capacity,
                                                sal_Int32 minimumCapacity);


/**
    Inserts the string representation of the <code>char</code> array
    argument into this string buffer.

    The characters of the array argument are inserted into the
    contents of this string buffer at the position indicated by
    <code>offset</code>. The length of this string buffer increases by
    the length of the argument.

    @param[in,out]  This        the String to operate on.
    @param[in,out]  capacity    the capacity of the string buffer
    @param[in]      offset      the offset.
    @param[in]      str         a character array.  Since LibreOffice 4.4, as a
                                special case, if str is null then the len added
                                characters are left uninitialized.
    @param[in]      len         the number of characters to append.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_insert(
                                       rtl_String ** This,
                                       sal_Int32 * capacity,
                                       sal_Int32 offset,
                                       const sal_Char * str,
                                       sal_Int32 len);

/**
    Removes the characters in a substring of this sequence.

    The substring begins at the specified <code>start</code> and
    is <code>len</code> characters long.

    start must be >= 0 && <= This->length

    @param[in,out]  This        The String to operate on.
    @param[in]      start       The beginning index, inclusive
    @param[in]      len         The substring length
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_remove(
                                       rtl_String ** This,
                                       sal_Int32 start,
                                       sal_Int32 len );

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_STRBUF_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
