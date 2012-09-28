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

#ifndef _RTL_STRBUF_H_
#define _RTL_STRBUF_H_

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

    @param   capacity         in: old capicity, out: new capacity.
    @param   minimumCapacity   the minimum desired capacity.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_ensureCapacity(
                                                /*inout*/rtl_String ** This,
                                                /*inout*/sal_Int32* capacity,
                                                sal_Int32 minimumCapacity);


/**
    Inserts the string representation of the <code>char</code> array
    argument into this string buffer.

    The characters of the array argument are inserted into the
    contents of this string buffer at the position indicated by
    <code>offset</code>. The length of this string buffer increases by
    the length of the argument.

    @param  capacity    the capacity of the string buffer
    @param  offset      the offset.
    @param  str         a character array.
    @param  len         the number of characters to append.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_insert(
                                       /*inout*/rtl_String ** This,
                                       /*inout*/sal_Int32 * capacity,
                                       sal_Int32 offset,
                                       const sal_Char * str,
                                       sal_Int32 len);

/**
    Removes the characters in a substring of this sequence.

    The substring begins at the specified <code>start</code> and
    is <code>len</code> characters long.

    start must be >= 0 && <= This->length

    @param  start       The beginning index, inclusive
    @param  len         The substring length
 */
SAL_DLLPUBLIC void SAL_CALL rtl_stringbuffer_remove(
                                       /*inout*/rtl_String ** This,
                                       sal_Int32 start,
                                       sal_Int32 len );

#ifdef __cplusplus
}
#endif

#endif  /* _RTL_STRBUF_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
