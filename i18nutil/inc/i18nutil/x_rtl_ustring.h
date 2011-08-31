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
#ifndef INCLUDED_I18NUTIL_X_RTL_USTRING_H
#define INCLUDED_I18NUTIL_X_RTL_USTRING_H

#include <rtl/strbuf.hxx>
#include <rtl/memory.h>
#include <rtl/alloc.h>
#include "i18nutildllapi.h"


/**
 * Allocates a new <code>rtl_uString</code> with capacity of nLen + 1
 * characters.
 *
 * The reference count is 1. The characters of the capacity are not cleared,
 * unlike the similar method of rtl_uString_new_WithLength in rtl/ustring.h, so
 * is more efficient for allocating a new string.
 *
 * call rtl_uString_release to release the string
 * alternatively pass ownership to an OUString with
 * rtl::OUString(newStr, SAL_NO_ACQUIRE);
 *
 * @param   nLen
 * @return  newStr
 */
I18NUTIL_DLLPUBLIC inline rtl_uString * SAL_CALL x_rtl_uString_new_WithLength( sal_Int32 nLen )
{
    //rtl_uString contains sal_Unicode buffer[1], so an input of nLen allocates
    //a buffer of nLen + 1
    rtl_uString *newStr = (rtl_uString*) rtl_allocateMemory ( sizeof(rtl_uString) + sizeof(sal_Unicode) * nLen);
    newStr->refCount = 1;
    newStr->length = nLen;
    return newStr;
}

/**
 * Release <code>rtl_uString</code> regardless its reference count.
 */
I18NUTIL_DLLPUBLIC inline void SAL_CALL x_rtl_uString_release( rtl_uString * value )
{
  rtl_freeMemory(value);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
