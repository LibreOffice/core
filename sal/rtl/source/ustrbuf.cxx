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

#include <rtl/ustrbuf.hxx>
#include <strimp.hxx>

void SAL_CALL rtl_uStringbuffer_newFromStr_WithLength( rtl_uString ** newStr,
                                                       const sal_Unicode * value,
                                                       sal_Int32 count)
{
    if (!value)
    {
        rtl_uString_new_WithLength( newStr, 16 );
        return;
    }

    rtl_uString_new_WithLength( newStr, count + 16 );
    (*newStr)->length = count;
    memcpy( (*newStr)->buffer, value, count * sizeof(sal_Unicode));
    RTL_LOG_STRING_NEW( *newStr );
    return;
}

rtl_uString * SAL_CALL rtl_uStringBuffer_refReturn( rtl_uString * pThis )
{
    RTL_LOG_STRING_NEW( pThis );
    rtl_uString_acquire( pThis );
    return pThis;
}

rtl_uString * SAL_CALL rtl_uStringBuffer_makeStringAndClear( rtl_uString ** ppThis,
                                                             sal_Int32 *nCapacity )
{
    // avoid an un-necessary atomic ref/unref pair
    rtl_uString *pStr = *ppThis;
    *ppThis = NULL;

    rtl_uString_new (ppThis);
    *nCapacity = 0;

    RTL_LOG_STRING_NEW( pStr );

    return pStr;
}

sal_Int32 SAL_CALL rtl_uStringbuffer_newFromStringBuffer( rtl_uString ** newStr,
                                                          sal_Int32 capacity,
                                                          rtl_uString * oldStr )
{
    sal_Int32 newCapacity = capacity;

    if (newCapacity < oldStr->length)
        newCapacity = oldStr->length;

    rtl_uString_new_WithLength( newStr, newCapacity );

    if (oldStr->length > 0) {
        (*newStr)->length = oldStr->length;
        memcpy( (*newStr)->buffer, oldStr->buffer, oldStr->length * sizeof(sal_Unicode));
    }
    RTL_LOG_STRING_NEW( *newStr );
    return newCapacity;
}

void SAL_CALL rtl_uStringbuffer_ensureCapacity
    (rtl_uString ** This, sal_Int32* capacity, sal_Int32 minimumCapacity)
{
    if (minimumCapacity > *capacity)
    {
        rtl_uString * pTmp = *This;
        rtl_uString * pNew = NULL;
        *capacity = ((*This)->length + 1) * 2;
        if (minimumCapacity > *capacity)
            /* still lower, set to the minimum capacity */
            *capacity = minimumCapacity;

        rtl_uString_new_WithLength(&pNew, *capacity);
        pNew->length = (*This)->length;
        *This = pNew;

        memcpy( (*This)->buffer, pTmp->buffer, pTmp->length * sizeof(sal_Unicode) );

        RTL_LOG_STRING_NEW( pTmp ); // with accurate contents
        rtl_uString_release( pTmp );
    }
}

void SAL_CALL rtl_uStringbuffer_insert( rtl_uString ** This,
                                        sal_Int32 * capacity,
                                        sal_Int32 offset,
                                        const sal_Unicode * str,
                                        sal_Int32 len)
{
    sal_Int32 nOldLen;
    sal_Unicode * pBuf;
    sal_Int32 n;
    if( len != 0 )
    {
        if (*capacity < (*This)->length + len)
            rtl_uStringbuffer_ensureCapacity( This, capacity, (*This)->length + len );

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
            memmove( pBuf + offset + len, pBuf + offset, n * sizeof(sal_Unicode) );

        /* insert the new characters */
        if( len == 1 )
            /* optimized for 1 character */
            pBuf[offset] = *str;
        else if( len > 1 )
            memcpy( pBuf + offset, str, len * sizeof(sal_Unicode) );
        (*This)->length = nOldLen + len;
        pBuf[ nOldLen + len ] = 0;
    }
}

void rtl_uStringbuffer_insertUtf32(
    rtl_uString ** pThis, sal_Int32 * capacity, sal_Int32 offset, sal_uInt32 c)
    SAL_THROW_EXTERN_C()
{
    sal_Unicode buf[2];
    sal_Int32 len;
    OSL_ASSERT(c <= 0x10FFFF && !(c >= 0xD800 && c <= 0xDFFF));
    if (c <= 0xFFFF) {
        buf[0] = (sal_Unicode) c;
        len = 1;
    } else {
        c -= 0x10000;
        buf[0] = (sal_Unicode) ((c >> 10) | 0xD800);
        buf[1] = (sal_Unicode) ((c & 0x3FF) | 0xDC00);
        len = 2;
    }
    rtl_uStringbuffer_insert(pThis, capacity, offset, buf, len);
}

void SAL_CALL rtl_uStringbuffer_insert_ascii(   /*inout*/rtl_uString ** This,
                                                /*inout*/sal_Int32 * capacity,
                                                sal_Int32 offset,
                                                const sal_Char * str,
                                                sal_Int32 len)
{
    sal_Int32 nOldLen;
    sal_Unicode * pBuf;
    sal_Int32 n;
    if( len != 0 )
    {
        if (*capacity < (*This)->length + len)
            rtl_uStringbuffer_ensureCapacity( This, capacity, (*This)->length + len );

        nOldLen = (*This)->length;
        pBuf = (*This)->buffer;

        /* copy the tail */
        n = (nOldLen - offset);
        if( n == 1 )
            /* optimized for 1 character */
            pBuf[offset + len] = pBuf[offset];
        else if( n > 1 )
            memmove( pBuf + offset + len, pBuf + offset, n * sizeof(sal_Unicode) );

        /* insert the new characters */
        for( n = 0; n < len; n++ )
        {
            /* Check ASCII range */
            OSL_ENSURE( (*str & 0x80) == 0, "Found ASCII char > 127");

            pBuf[offset + n] = (sal_Unicode)*(str++);
        }

        (*This)->length = nOldLen + len;
        pBuf[ nOldLen + len ] = 0;
    }
}

/*************************************************************************
 *  rtl_uStringbuffer_remove
 */
void SAL_CALL rtl_uStringbuffer_remove( rtl_uString ** This,
                                       sal_Int32 start,
                                       sal_Int32 len )
{
    sal_Int32 nTailLen;
    sal_Unicode * pBuf;

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
        memmove(pBuf + start, pBuf + start + len, nTailLen * sizeof(sal_Unicode));
    }

    (*This)->length-=len;
    pBuf[ (*This)->length ] = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
