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

#include <config_probes.h>
#include <rtl/strbuf.h>

#if USE_SDT_PROBES
#define RTL_LOG_STRING_BITS 8
#endif

#include "strtmpl.hxx"

/*************************************************************************
 *  rtl_stringbuffer_newFromStr_WithLength
 */
void SAL_CALL rtl_stringbuffer_newFromStr_WithLength( rtl_String ** newStr,
                                                      const char * value,
                                                      sal_Int32 count )
{
    rtl::str::stringbuffer_newFromStr_WithLength(newStr, value, count);
}

/*************************************************************************
 *  rtl_stringbuffer_newFromStringBuffer
 */
sal_Int32 SAL_CALL rtl_stringbuffer_newFromStringBuffer( rtl_String ** newStr,
                                                         sal_Int32 capacity,
                                                         rtl_String * oldStr )
{
    return rtl::str::stringbuffer_newFromStringBuffer(newStr, capacity, oldStr);
}

/*************************************************************************
 *  rtl_stringbuffer_ensureCapacity
 */
void SAL_CALL rtl_stringbuffer_ensureCapacity
    (rtl_String ** This, sal_Int32* capacity, sal_Int32 minimumCapacity)
{
    rtl::str::stringbuffer_ensureCapacity(This, capacity, minimumCapacity);
}

/*************************************************************************
 *  rtl_stringbuffer_insert
 */
void SAL_CALL rtl_stringbuffer_insert( rtl_String ** This,
                                       sal_Int32 * capacity,
                                       sal_Int32 offset,
                                       const char * str,
                                       sal_Int32 len )
{
    rtl::str::stringbuffer_insert(This, capacity, offset, str, len);
}

/*************************************************************************
 *  rtl_stringbuffer_remove
 */
void SAL_CALL rtl_stringbuffer_remove( rtl_String ** This,
                                       sal_Int32 start,
                                       sal_Int32 len )
{
    rtl::str::stringbuffer_remove(This, start, len);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
