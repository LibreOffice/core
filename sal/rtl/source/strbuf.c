/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strbuf.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:04:50 $
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
#include <rtl/strbuf.hxx>
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
    rtl_copyMemory( (*newStr)->buffer, value, count );
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
    (*newStr)->length = oldStr->length;
    rtl_copyMemory( (*newStr)->buffer, oldStr->buffer, oldStr->length );
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

        rtl_copyMemory( (*This)->buffer, pTmp->buffer, pTmp->length );
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
            rtl_moveMemory( pBuf + offset + len, pBuf + offset, n * sizeof(sal_Char) );

        /* insert the new characters */
        n = len;
        if( len == 1 )
                            /* optimized for 1 character */
            pBuf[offset] = *str;
        else if( n > 1 )
            rtl_copyMemory( pBuf + offset, str, len * sizeof(sal_Char) );
        (*This)->length = nOldLen + len;
        pBuf[ nOldLen + len ] = 0;
    }
}

