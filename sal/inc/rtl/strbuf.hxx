/*************************************************************************
 *
 *  $RCSfile: strbuf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-16 11:08:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_STRBUF_HXX_
#define _RTL_STRBUF_HXX_

#ifndef _RTL_STRBUF_H_
#include <rtl/strbuf.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifdef __cplusplus

namespace rtl
{

/**
 * A string buffer implements a mutable sequence of characters.
 * <p>
 * String buffers are safe for use by multiple threads. The methods
 * are synchronized where necessary so that all the operations on any
 * particular instance behave as if they occur in some serial order.
 * <p>
 * String buffers are used by the compiler to implement the binary
 * string concatenation operator <code>+</code>. For example, the code:
 * <p><blockquote><pre>
 *     x = "a" + 4 + "c"
 * </pre></blockquote><p>
 * is compiled to the equivalent of:
 * <p><blockquote><pre>
 *     x = new OStringBuffer().append("a").append(4).append("c")
 *                           .toString()
 * </pre></blockquote><p>
 * The principal operations on a <code>OStringBuffer</code> are the
 * <code>append</code> and <code>insert</code> methods, which are
 * overloaded so as to accept data of any type. Each effectively
 * converts a given datum to a string and then appends or inserts the
 * characters of that string to the string buffer. The
 * <code>append</code> method always adds these characters at the end
 * of the buffer; the <code>insert</code> method adds the characters at
 * a specified point.
 * <p>
 * For example, if <code>z</code> refers to a string buffer object
 * whose current contents are "<code>start</code>", then
 * the method call <code>z.append("le")</code> would cause the string
 * buffer to contain "<code>startle</code>", whereas
 * <code>z.insert(4, "le")</code> would alter the string buffer to
 * contain "<code>starlet</code>".
 * <p>
 * Every string buffer has a capacity. As long as the length of the
 * character sequence contained in the string buffer does not exceed
 * the capacity, it is not necessary to allocate a new internal
 * buffer array. If the internal buffer overflows, it is
 * automatically made larger.
 */
class OStringBuffer
{
public:
    /**
     * Constructs a string buffer with no characters in it and an
     * initial capacity of 16 characters.
     */
    OStringBuffer()
        : pData(NULL)
        , nCapacity( 16 )
    {
        rtl_string_new_WithLength( &pData, nCapacity );
    }

    /**
     * Allocates a new string buffer that contains the same sequence of
     * characters as the string buffer argument.
     *
     * @param   value   a <code>OStringBuffer</code>.
     */
    OStringBuffer( const OStringBuffer & value )
        : pData(NULL)
        , nCapacity( value.nCapacity )
    {
        rtl_stringbuffer_newFromStringBuffer( &pData, value.nCapacity, value.pData );
    }

    /**
     * Constructs a string buffer with no characters in it and an
     * initial capacity specified by the <code>length</code> argument.
     *
     * @param      length   the initial capacity.
     */
    OStringBuffer(sal_Int32 length)
        : pData(NULL)
        , nCapacity( length )
    {
        rtl_string_new_WithLength( &pData, length );
    }

    /**
     * Constructs a string buffer so that it represents the same
     * sequence of characters as the string argument. The initial
     * capacity of the string buffer is <code>16</code> plus the length
     * of the string argument.
     *
     * @param   value   the initial string value.
     */
    OStringBuffer(OString value)
        : pData(NULL)
        , nCapacity( value.getLength() + 16 )
    {
        rtl_stringbuffer_newFromStr_WithLength( &pData, value.getStr(), value.getLength() );
    }

    /**
     * Release the string own string data and notice and acquire the string data of value.
     */
    OStringBuffer& operator = ( const OStringBuffer& value )
    {
        rtl_string_assign( &pData, value.pData );
        return *this;
    }

    /**
     * Release the string data.
     */
    ~OStringBuffer()
    {
        rtl_string_release( pData );
    }

    /**
     * Fill the string data in the new string and clear the buffer.<BR>
     * This method is more efficient than the contructor of the string. It does
     * not copy the buffer.
     *
     * @return the string previously contained in the buffer.
     */
    OString makeStringAndClear()
    {
        OString aRet( pData );
        RTL_STRING_NEW(&pData);
        nCapacity = 0;
        return aRet;
    }

    /**
     * Returns the length (character count) of this string buffer.
     *
     * @return  the number of characters in this string buffer.
     */
    sal_Int32 getLength()
    {
        return pData->length;
    }

    /**
     * Returns the current capacity of the String buffer. The capacity
     * is the amount of storage available for newly inserted
     * characters. The real buffer size is 2 bytes longer, because
     * all strings are 0 terminated.
     *
     * @return  the current capacity of this string buffer.
     */
    sal_Int32 getCapacity()
    {
        return nCapacity;
    }

    /**
     * Ensures that the capacity of the buffer is at least equal to the
     * specified minimum.
     * If the current capacity of this string buffer is less than the
     * argument, then a new internal buffer is allocated with greater
     * capacity. The new capacity is the larger of:
     * <ul>
     * <li>The <code>minimumCapacity</code> argument.
     * <li>Twice the old capacity, plus <code>2</code>.
     * </ul>
     * If the <code>minimumCapacity</code> argument is nonpositive, this
     * method takes no action and simply returns.
     *
     * @param   minimumCapacity   the minimum desired capacity.
     */
    void ensureCapacity(sal_Int32 minimumCapacity)
    {
        rtl_stringbuffer_ensureCapacity( &pData, &nCapacity, minimumCapacity );
    }

    /**
     * Sets the length of this String buffer.
     * If the <code>newLength</code> argument is less than the current
     * length of the string buffer, the string buffer is truncated to
     * contain exactly the number of characters given by the
     * <code>newLength</code> argument.
     * <p>
     * If the <code>newLength</code> argument is greater than or equal
     * to the current length, sufficient null characters
     * (<code>'&#92;u0000'</code>) are appended to the string buffer so that
     * length becomes the <code>newLength</code> argument.
     * <p>
     * The <code>newLength</code> argument must be greater than or equal
     * to <code>0</code>.
     *
     * @param      newLength   the new length of the buffer.
     */
    void setLength(sal_Int32 newLength)
    {
        if( newLength > nCapacity )
            rtl_stringbuffer_ensureCapacity(&pData, &nCapacity, newLength);
        else
            pData->buffer[newLength] = '\0';
        pData->length = newLength;
    }

    /**
     * Returns the character at a specific index in this string buffer.
     * <p>
     * The first character of a string buffer is at index
     * <code>0</code>, the next at index <code>1</code>, and so on, for
     * array indexing.
     * <p>
     * The index argument must be greater than or equal to
     * <code>0</code>, and less than the length of this string buffer.
     *
     * @param      index   the index of the desired character.
     * @return     the character at the specified index of this string buffer.
     */
    sal_Char charAt( sal_Int32 index )
    {
        return pData->buffer[ index ];
    }

    /**
     * Return a null terminated character array.
     */
    operator        const sal_Char *() const { return pData->buffer; }

    /**
     * Return a null terminated character array.
     */
    const sal_Char* getStr() const { return pData->buffer; }


    /**
     * The character at the specified index of this string buffer is set
     * to <code>ch</code>.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than the length of this string buffer.
     *
     * @param      index   the index of the character to modify.
     * @param      ch      the new character.
     */
    OStringBuffer & setCharAt(sal_Int32 index, sal_Char ch)
    {
        pData->buffer[ index ] = ch;
        return *this;
    }

    /**
     * Appends the string to this string buffer.
     * <p>
     * The characters of the <code>String</code> argument are appended, in
     * order, to the contents of this string buffer, increasing the
     * length of this string buffer by the length of the argument.
     *
     * @param   str   a string.
     * @return  this string buffer.
     */
    OStringBuffer & append(const OString &str)
    {
        return append( str.getStr(), str.getLength() );
    }

    /**
     * Appends the string representation of the <code>char</code> array
     * argument to this string buffer.
     * <p>
     * The characters of the array argument are appended, in order, to
     * the contents of this string buffer. The length of this string
     * buffer increases by the length of the argument.
     *
     * @param   str   the characters to be appended.
     * @return  this string buffer.
     */
    OStringBuffer & append( const sal_Char * str )
    {
        return append( str, rtl_str_getLength( str ) );
    }

    /**
     * Appends the string representation of the <code>char</code> array
     * argument to this string buffer.
     * <p>
     * Characters of the character array <code>str</code> are appended,
     * in order, to the contents of this string buffer. The length of this
     * string buffer increases by the value of <code>len</code>.
     *
     * @param   str      the characters to be appended.
     * @param   len      the number of characters to append.
     * @return  this string buffer.
     */
    OStringBuffer & append( const sal_Char * str, sal_Int32 len)
    {
        // insert behind the last character
        rtl_stringbuffer_insert( &pData, &nCapacity, getLength(), str, len );
        return *this;
    }

    /**
     * Appends the string representation of the <code>sal_Bool</code>
     * argument to the string buffer.
     * <p>
     * The argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then appended to this string buffer.
     *
     * @param   b   a <code>sal_Bool</code>.
     * @return  this string buffer.
     */
    OStringBuffer & append(sal_Bool b)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFBOOLEAN];
        return append( sz, rtl_str_valueOfBoolean( sz, b ) );
    }

    /**
     * Appends the string representation of the <code>char</code>
     * argument to this string buffer.
     * <p>
     * The argument is appended to the contents of this string buffer.
     * The length of this string buffer increases by <code>1</code>.
     *
     * @param   ch   a <code>char</code>.
     * @return  this string buffer.
     */
    OStringBuffer & append(sal_Char c)
    {
        return append( &c, 1 );
    }

    /**
     * Appends the string representation of the <code>sal_Int32</code>
     * argument to this string buffer.
     * <p>
     * The argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then appended to this string buffer.
     *
     * @param   i   an <code>sal_Int32</code>.
     * @return  this string buffer.
     */
    OStringBuffer & append(sal_Int32 i, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT32];
        return append( sz, rtl_str_valueOfInt32( sz, i, radix ) );
    }

    /**
     * Appends the string representation of the <code>long</code>
     * argument to this string buffer.
     * <p>
     * The argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then appended to this string buffer.
     *
     * @param   l   a <code>long</code>.
     * @return  this string buffer.
     */
    OStringBuffer & append(sal_Int64 l, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT64];
        return append( sz, rtl_str_valueOfInt64( sz, l, radix ) );
    }

    /**
     * Appends the string representation of the <code>float</code>
     * argument to this string buffer.
     * <p>
     * The argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then appended to this string buffer.
     *
     * @param   f   a <code>float</code>.
     * @return  this string buffer.
     */
    OStringBuffer & append(float f)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFFLOAT];
        return append( sz, rtl_str_valueOfFloat( sz, f ) );
    }

    /**
     * Appends the string representation of the <code>double</code>
     * argument to this string buffer.
     * <p>
     * The argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then appended to this string buffer.
     *
     * @param   d   a <code>double</code>.
     * @return  this string buffer.
     */
    OStringBuffer & append(double d)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFDOUBLE];
        return append( sz, rtl_str_valueOfDouble( sz, d ) );
    }

    /**
     * Inserts the string into this string buffer.
     * <p>
     * The characters of the <code>String</code> argument are inserted, in
     * order, into this string buffer at the indicated offset. The length
     * of this string buffer is increased by the length of the argument.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      str      a string.
     * @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, const OString & str)
    {
        return insert( offset, str.getStr(), str.getLength() );
    }

    /**
     * Inserts the string representation of the <code>char</code> array
     * argument into this string buffer.
     * <p>
     * The characters of the array argument are inserted into the
     * contents of this string buffer at the position indicated by
     * <code>offset</code>. The length of this string buffer increases by
     * the length of the argument.
     *
     * @param      offset   the offset.
     * @param      ch       a character array.
     * @return     this string buffer.
     */
    OStringBuffer & insert( sal_Int32 offset, const sal_Char * str )
    {
        return insert( offset, str, rtl_str_getLength( str ) );
    }

    /**
     * Inserts the string representation of the <code>char</code> array
     * argument into this string buffer.
     * <p>
     * The characters of the array argument are inserted into the
     * contents of this string buffer at the position indicated by
     * <code>offset</code>. The length of this string buffer increases by
     * the length of the argument.
     *
     * @param      offset   the offset.
     * @param      ch       a character array.
     * @param       len     the number of characters to append.
     * @return     this string buffer.
     */
    OStringBuffer & insert( sal_Int32 offset, const sal_Char * str, sal_Int32 len)
    {
        // insert behind the last character
        rtl_stringbuffer_insert( &pData, &nCapacity, offset, str, len );
        return *this;
    }

    /**
     * Inserts the string representation of the <code>sal_Bool</code>
     * argument into this string buffer.
     * <p>
     * The second argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then inserted into this string buffer at the indicated
     * offset.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      b        a <code>sal_Bool</code>.
     * @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Bool b)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFBOOLEAN];
        return insert( offset, sz, rtl_str_valueOfBoolean( sz, b ) );
    }

    /**
     * Inserts the string representation of the <code>char</code>
     * argument into this string buffer.
     * <p>
     * The second argument is inserted into the contents of this string
     * buffer at the position indicated by <code>offset</code>. The length
     * of this string buffer increases by one.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      ch       a <code>char</code>.
     * @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Char c)
    {
        return insert( offset, &c, 1 );
    }

    /**
     * Inserts the string representation of the second <code>sal_Int32</code>
     * argument into this string buffer.
     * <p>
     * The second argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then inserted into this string buffer at the indicated
     * offset.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      b        an <code>sal_Int32</code>.
     * @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Int32 i, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT32];
        return insert( offset, sz, rtl_str_valueOfInt32( sz, i, radix ) );
    }

    /**
     * Inserts the string representation of the <code>long</code>
     * argument into this string buffer.
     * <p>
     * The second argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then inserted into this string buffer at the indicated
     * offset.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      b        a <code>long</code>.
     * @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, sal_Int64 l, sal_Int16 radix = 10 )
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFINT64];
        return insert( offset, sz, rtl_str_valueOfInt64( sz, l, radix ) );
    }

    /**
     * Inserts the string representation of the <code>float</code>
     * argument into this string buffer.
     * <p>
     * The second argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then inserted into this string buffer at the indicated
     * offset.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      b        a <code>float</code>.
     * @return     this string buffer.
     */
    OStringBuffer insert(sal_Int32 offset, float f)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFFLOAT];
        return insert( offset, sz, rtl_str_valueOfFloat( sz, f ) );
    }

    /**
     * Inserts the string representation of the <code>double</code>
     * argument into this string buffer.
     * <p>
     * The second argument is converted to a string as if by the method
     * <code>String.valueOf</code>, and the characters of that
     * string are then inserted into this string buffer at the indicated
     * offset.
     * <p>
     * The offset argument must be greater than or equal to
     * <code>0</code>, and less than or equal to the length of this
     * string buffer.
     *
     * @param      offset   the offset.
     * @param      b        a <code>double</code>.
     * @return     this string buffer.
     */
    OStringBuffer & insert(sal_Int32 offset, double d)
    {
        sal_Char sz[RTL_STR_MAX_VALUEOFDOUBLE];
        return insert( offset, sz, rtl_str_valueOfDouble( sz, d ) );
    }
private:
    /**
     * A pointer to the data structur which contains the data.
     */
    rtl_String * pData;

    /**
     * The len of the pData->buffer.
     */
    sal_Int32       nCapacity;
};

}

#endif  /* __cplusplus */
#endif  /* _RTL_STRBUF_HXX_ */


