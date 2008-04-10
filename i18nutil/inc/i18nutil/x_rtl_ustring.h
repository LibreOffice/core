/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: x_rtl_ustring.h,v $
 * $Revision: 1.4 $
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
#ifndef INCLUDED_I18NUTIL_X_RTL_USTRING_H
#define INCLUDED_I18NUTIL_X_RTL_USTRING_H

#ifndef _RTL_STRING_HXX_
#include <rtl/strbuf.hxx>
#endif
#include <rtl/memory.h>
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
