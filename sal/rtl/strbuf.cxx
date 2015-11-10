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

#include <string.h>

#include <osl/interlck.h>
#include <rtl/strbuf.hxx>

/*************************************************************************
 *  rtl_stringbuffer_newFromStr_WithLength
 */
void SAL_CALL rtl_stringbuffer_newFromStr_WithLength( rtl_String ** newStr,
                                                      const sal_Char * value,
                                                      sal_Int32 count )
{
    assert(newStr);
    assert(count >= 0);
    if (!value)
    {
        rtl_string_new_WithLength( newStr, 16 );
        return;
    }

    rtl_string_new_WithLength( newStr, count + 16 );
    (*newStr)->length = count;
    memcpy( (*newStr)->buffer, value, count );
    return;
}

/*************************************************************************
 *  rtl_stringbuffer_newFromStringBuffer
 */
sal_Int32 SAL_CALL rtl_stringbuffer_newFromStringBuffer( rtl_String ** newStr,
                                                         sal_Int32 capacity,
                                                         rtl_String * oldStr )
{
    assert(newStr);
    assert(oldStr);
    assert(capacity >= 0);
    sal_Int32 newCapacity = capacity;

    if (newCapacity < oldStr->length)
        newCapacity = oldStr->length;

    rtl_string_new_WithLength( newStr, newCapacity );
    if (oldStr->length > 0) {
        (*newStr)->length = oldStr->length;
        memcpy( (*newStr)->buffer, oldStr->buffer, oldStr->length );
    }
    return newCapacity;
}

/*************************************************************************
 *  rtl_stringbuffer_ensureCapacity
 */
void SAL_CALL rtl_stringbuffer_ensureCapacity
    (rtl_String ** This, sal_Int32* capacity, sal_Int32 minimumCapacity)
{
    assert(This);
//    assert(capacity && *capacity >= 0);
//    assert(minimumCapacity >= 0);
    if (minimumCapacity > *capacity)
    {
        rtl_String * pTmp = *This;
        rtl_String * pNew = nullptr;
        *capacity = ((*This)->length + 1) * 2;
        if (minimumCapacity > *capacity)
            /* still lower, set to the minimum capacity */
            *capacity = minimumCapacity;

        rtl_string_new_WithLength(&pNew, *capacity);
        pNew->length = (*This)->length;
        *This = pNew;

        memcpy( (*This)->buffer, pTmp->buffer, pTmp->length );
        rtl_string_release( pTmp );
    }
}

/*************************************************************************
 *  rtl_stringbuffer_insert
 */
void SAL_CALL rtl_stringbuffer_insert( rtl_String ** This,
                                       sal_Int32 * capacity,
                                       sal_Int32 offset,
                                       const sal_Char * str,
                                       sal_Int32 len )
{
    assert(This);
    assert(capacity && *capacity >= 0);
    assert(offset >= 0 && offset <= (**This).length);
    assert(len >= 0);
    sal_Int32 nOldLen;
    sal_Char * pBuf;
    sal_Int32 n;
    if( len != 0 )
    {
        if (*capacity < (*This)->length + len)
            rtl_stringbuffer_ensureCapacity( This, capacity, (*This)->length + len );

        nOldLen = (*This)->length;
        pBuf = (*This)->buffer;

        /* copy the tail */
        n = (nOldLen - offset);
        if( n == 1 )
            /* optimized for 1 character */
            pBuf[offset + len] = pBuf[offset];
        else if( n > 1 )
            memmove( pBuf + offset + len, pBuf + offset, n * sizeof(sal_Char) );

        /* insert the new characters */
        if( str != nullptr )
        {
            if( len == 1 )
                            /* optimized for 1 character */
                pBuf[offset] = *str;
            else
                memcpy( pBuf + offset, str, len * sizeof(sal_Char) );
        }
        (*This)->length = nOldLen + len;
        pBuf[ nOldLen + len ] = 0;
    }
}

/*************************************************************************
 *  rtl_stringbuffer_remove
 */
void SAL_CALL rtl_stringbuffer_remove( rtl_String ** This,
                                       sal_Int32 start,
                                       sal_Int32 len )
{
    assert(This);
    assert(start >= 0 && start <= (**This).length);
    assert(len >= 0);
    sal_Int32 nTailLen;
    sal_Char * pBuf;

    if (len > (*This)->length - start)
        len = (*This)->length - start;

    //remove nothing
    if (!len)
        return;

    pBuf = (*This)->buffer;
    nTailLen = (*This)->length - ( start + len );

    if (nTailLen)
    {
        /* move the tail */
        memmove(pBuf + start, pBuf + start + len, nTailLen * sizeof(sal_Char));
    }

    (*This)->length-=len;
    pBuf[ (*This)->length ] = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
