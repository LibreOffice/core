/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ustrbuf.c,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:46:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

/*
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
*/



/*************************************************************************
 *  rtl_uStringbuffer_newFromStr_WithLength
 */
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
    rtl_copyMemory( (*newStr)->buffer, value, count * sizeof(sal_Unicode));
    return;
}

/*************************************************************************
 *  rtl_uStringbuffer_newFromStringBuffer
 */
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
        rtl_copyMemory( (*newStr)->buffer, oldStr->buffer, oldStr->length * sizeof(sal_Unicode));
    }
    return newCapacity;
}

/*************************************************************************
 *  rtl_uStringbuffer_ensureCapacity
 */
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

        rtl_copyMemory( (*This)->buffer, pTmp->buffer, pTmp->length * sizeof(sal_Unicode) );
        rtl_uString_release( pTmp );
    }
}

/*************************************************************************
 *  rtl_uStringbuffer_insert
 */
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
            rtl_moveMemory( pBuf + offset + len, pBuf + offset, n * sizeof(sal_Unicode) );

        /* insert the new characters */
        if( len == 1 )
            /* optimized for 1 character */
            pBuf[offset] = *str;
        else if( len > 1 )
            rtl_copyMemory( pBuf + offset, str, len * sizeof(sal_Unicode) );
        (*This)->length = nOldLen + len;
        pBuf[ nOldLen + len ] = 0;
    }
}

void rtl_uStringbuffer_insertUtf32(
    rtl_uString ** pThis, sal_Int32 * capacity, sal_Int32 offset, sal_uInt32 c)
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

/*************************************************************************
 *  rtl_uStringbuffer_insert_ascii
 */
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
            rtl_moveMemory( pBuf + offset + len, pBuf + offset, n * sizeof(sal_Unicode) );

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


