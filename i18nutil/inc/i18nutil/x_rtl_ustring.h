/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x_rtl_ustring.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:40:05 $
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
#ifndef INCLUDED_I18NUTIL_X_RTL_USTRING_H
#define INCLUDED_I18NUTIL_X_RTL_USTRING_H

#ifndef _RTL_STRING_HXX_
#include <rtl/strbuf.hxx>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#include <rtl/alloc.h>


/**
 * Allocates a new <code>rtl_uString</code> which can hold nLen + 1 characters.
 * The reference count is 0. The characters of room is not cleared.
 * This method is similar to rtl_uString_new_WithLength in rtl/ustring.h, but
 * can allocate a new string more efficiently. You need to "acquire" by such as
 * OUString( rtl_uString * value ) if you intend to use it for a while.
 * @param   [output] newStr
 * @param   [input]  nLen
 */
inline void SAL_CALL x_rtl_uString_new_WithLength( rtl_uString ** newStr, sal_Int32 nLen, sal_Int32 _refCount = 0 )
{
  *newStr = (rtl_uString*) rtl_allocateMemory ( sizeof(rtl_uString) + sizeof(sal_Unicode) * nLen);
  (*newStr)->refCount = _refCount;
  (*newStr)->length = nLen;

  // rtl_uString is defined in rtl/ustring.h as below:
  //typedef struct _rtl_uString
  //{
  //    sal_Int32       refCount;
  //    sal_Int32       length;
  //    sal_Unicode     buffer[1];
  //} rtl_uString;
}

inline rtl_uString * SAL_CALL x_rtl_uString_new_WithLength( sal_Int32 nLen, sal_Int32 _refCount = 0 )
{
  rtl_uString *newStr = (rtl_uString*) rtl_allocateMemory ( sizeof(rtl_uString) + sizeof(sal_Unicode) * nLen);
  newStr->refCount = _refCount;
  newStr->length = nLen;
  return newStr;
}

/**
 * Release <code>rtl_uString</code> regardless its reference count.
 */
inline void SAL_CALL x_rtl_uString_release( rtl_uString * value )
{
  rtl_freeMemory(value);
}


#endif // #ifndef _I18N_X_RTL_USTRING_H_
