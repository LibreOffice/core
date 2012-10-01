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
    if (minimumCapacity > *capacity)
    {
        rtl_String * pTmp = *This;
        rtl_String * pNew = NULL;
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
    sal_Int32 nOldLen;
    sal_Char * pBuf;
    sal_Int32 n;
    if( len != 0 )
    {
        if (*capacity < (*This)->length + len)
            rtl_stringbuffer_ensureCapacity( This, capacity, (*This)->length + len );

        /*
        if( len == 1 )
            This->buffer
        */
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
        n = len;
        if( len == 1 )
                            /* optimized for 1 character */
            pBuf[offset] = *str;
        else if( n > 1 )
            memcpy( pBuf + offset, str, len * sizeof(sal_Char) );
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
