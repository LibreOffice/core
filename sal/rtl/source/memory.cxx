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


#include <string.h>
#include <rtl/memory.h>

void SAL_CALL rtl_zeroMemory(void *Ptr, sal_Size Bytes)
{
    memset(Ptr, 0, Bytes);
}

void SAL_CALL rtl_fillMemory(void *Ptr, sal_Size Bytes, sal_uInt8 Fill)
{
    memset(Ptr, Fill, Bytes);
}

void SAL_CALL rtl_copyMemory(void *Dst, const void *Src, sal_Size Bytes)
{
    memcpy(Dst, Src, Bytes);
}

void SAL_CALL rtl_moveMemory(void *Dst, const void *Src, sal_Size Bytes)
{
    memmove(Dst, Src, Bytes);
}

sal_Int32 SAL_CALL rtl_compareMemory(const void *MemA, const void *MemB, sal_Size Bytes)
{
    return memcmp(MemA, MemB, Bytes);
}

void* SAL_CALL rtl_findInMemory(const void *MemA, sal_uInt8 ch, sal_Size Bytes)
{
    return const_cast< void * >(memchr(MemA, ch, Bytes));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
