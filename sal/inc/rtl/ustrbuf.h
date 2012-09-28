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

#ifndef _RTL_USTRBUF_H_
#define _RTL_USTRBUF_H_

#include "sal/config.h"

#include "rtl/ustring.h"
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
    @param  value   the initial value of the string.
    @param  count    the length of value.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uStringbuffer_newFromStr_WithLength(
                                                      rtl_uString ** newStr,
                                                      const sal_Unicode * value,
                                                      sal_Int32 count );

/**
    Allocates a new <code>String</code> that contains the same sequence of
    characters as the string argument.

    The initial capacity is the larger of:
    <ul>
    <li> The <code>bufferLen</code> argument.
    <li> The <code>length</code> of the string argument.
    </ul>

    @param  newStr       out parameter, contains the new string. The reference count is 1.
    @param  capacity     the initial len of the string buffer.
    @param  oldStr       the initial value of the string.
    @return the new capacity of the string buffer
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_uStringbuffer_newFromStringBuffer(
                                                          rtl_uString ** newStr,
                                                          sal_Int32 capacity,
                                                          rtl_uString * oldStr );

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

    @param   capacity          in: old capicity, out: new capacity.
    @param   minimumCapacity   the minimum desired capacity.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uStringbuffer_ensureCapacity(
                                                /*inout*/rtl_uString ** This,
                                                /*inout*/sal_Int32* capacity,
                                                sal_Int32 minimumCapacity);

/**
    Inserts the string representation of the <code>str</code> array
    argument into this string buffer.

    The characters of the array argument are inserted into the
    contents of this string buffer at the position indicated by
    <code>offset</code>. The length of this string buffer increases by
    the length of the argument.

    @param   This        The string, on that the operation should take place
    @param   capacity    the capacity of the string buffer
    @param   offset      the offset.
    @param   str         a character array.
    @param   len         the number of characters to append.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uStringbuffer_insert(
                                        /*inout*/rtl_uString ** This,
                                        /*inout*/sal_Int32 * capacity,
                                        sal_Int32 offset,
                                        const sal_Unicode * str,
                                        sal_Int32 len);

/**
   Inserts a single UTF-32 character into this string buffer.

   <p>The single UTF-32 character will be represented within the string buffer
   as either one or two UTF-16 code units.</p>

   @param pThis the string buffer on which the operation is performed

   @param capacity the capacity of the string buffer

   @param offset the offset into this string buffer (from zero to the length
   of this string buffer, inclusive)

   @param c a well-formed UTF-32 code unit (that is, a value in the range
   <code>0</code>&ndash;<code>0x10FFFF</code>, but excluding
   <code>0xD800</code>&ndash;<code>0xDFFF</code>)
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uStringbuffer_insertUtf32(
    rtl_uString ** pThis, sal_Int32 * capacity, sal_Int32 offset, sal_uInt32 c)
    SAL_THROW_EXTERN_C();

/**
    Inserts the 8-Bit ASCII string representation of the <code>str</code>
    array argument into this string buffer.

    Since this function is optimized
    for performance, the ASCII character values are not converted in any way.
    The caller has to make sure that all ASCII characters are in the allowed
    range between 0 and 127.
    <p>
    The characters of the array argument are inserted into the
    contents of this string buffer at the position indicated by
    <code>offset</code>. The length of this string buffer increases by
    the length of the argument.

    @param   This        The string, on that the operation should take place
    @param   capacity    the capacity of the string buffer
    @param   offset      the offset.
    @param   str         a character array.
    @param   len         the number of characters to append.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_uStringbuffer_insert_ascii(
                                                /*inout*/rtl_uString ** This,
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
SAL_DLLPUBLIC void SAL_CALL rtl_uStringbuffer_remove(
                                       /*inout*/rtl_uString ** This,
                                       sal_Int32 start,
                                       sal_Int32 len );

/**
    Returns an immutable rtl_uString object, while clearing the string buffer.

    This method is primarily used to allow these completed
    string allocation events to be traced.

    @param  ppThis      The string, on that the operation should take place
    @param  nCapacity   pointer to the capacity of the string buffer

    @since LibreOffice 3.6
 */
SAL_DLLPUBLIC rtl_uString * SAL_CALL rtl_uStringBuffer_makeStringAndClear(
                                        /*inout*/ rtl_uString ** ppThis,
                                        sal_Int32 *nCapacity );

/**
    References and returns an immutable rtl_uString object, from a mutable
    string-buffer object.

    This method is primarily used to allow legacy 'String' class
    conversions to OUString to be accurately traced.

    @param  pThis      The string, on that the operation should take place

    @since LibreOffice 3.6
 */
SAL_DLLPUBLIC rtl_uString * SAL_CALL rtl_uStringBuffer_refReturn( rtl_uString *pThis );

#ifdef __cplusplus
}
#endif

#endif  /* _RTL_USTRBUF_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
