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


#ifndef _RTL_MEMORY_H_
#define _RTL_MEMORY_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

SAL_DEPRECATED("Use memset instead")
SAL_DLLPUBLIC void SAL_CALL rtl_zeroMemory(void *Ptr, sal_Size Bytes);
SAL_DEPRECATED("Use memset instead")
SAL_DLLPUBLIC void SAL_CALL rtl_fillMemory(void *Ptr, sal_Size Bytes, sal_uInt8 Fill);
SAL_DEPRECATED("Use memcpy instead")
SAL_DLLPUBLIC void SAL_CALL rtl_copyMemory(void *Dst, const void *Src, sal_Size Bytes);
SAL_DEPRECATED("Use memmove instead")
SAL_DLLPUBLIC void SAL_CALL rtl_moveMemory(void *Dst, const void *Src, sal_Size Bytes);
SAL_DEPRECATED("Use memcmp instead")
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_compareMemory(const void *MemA, const void *MemB, sal_Size Bytes);
SAL_DEPRECATED("Use memchr instead")
SAL_DLLPUBLIC void* SAL_CALL rtl_findInMemory(const void *MemA, sal_uInt8 ch, sal_Size Bytes);

#ifdef __cplusplus
}
#endif


#endif /*_RTL_MEMORY_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
