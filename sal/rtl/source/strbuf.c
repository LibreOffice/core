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



#include <osl/interlck.h>

#ifndef _RTL_STRING_HXX_
#include <rtl/strbuf.hxx>
#endif
#include <rtl/memory.h>

/*
#include <rtl/alloc.h>
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
    if (oldStr->length > 0) {
        (*newStr)->length = oldStr->length;
        rtl_copyMemory( (*newStr)->buffer, oldStr->buffer, oldStr->length );
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

