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

#ifndef INCLUDED_RTL_RANDOM_H
#define INCLUDED_RTL_RANDOM_H

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Random Pool opaque type.
 */
typedef void* rtlRandomPool;


/** Error Code enumeration.
 */
enum __rtl_RandomError
{
    rtl_Random_E_None,
    rtl_Random_E_Argument,
    rtl_Random_E_Memory,
    rtl_Random_E_Unknown,
    rtl_Random_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Error Code type.
 */
typedef enum __rtl_RandomError rtlRandomError;


/** Create a Random Pool.
    @return initialized Random Pool, or NULL upon failure.
 */
SAL_DLLPUBLIC rtlRandomPool SAL_CALL rtl_random_createPool (void) SAL_THROW_EXTERN_C();


/** Destroy a Random Pool.
    @param[in] Pool a Random Pool.
    @return none. Pool is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_random_destroyPool (
    rtlRandomPool Pool
) SAL_THROW_EXTERN_C();


/** Add bytes to a Random Pool.
    @param[in] Pool   a Random Pool.
    @param[in] Buffer a buffer containing the bytes to add.
    @param[in] Bytes  the number of bytes to read from the buffer.
    @retval rtl_Random_E_None upon success.
 */
SAL_DLLPUBLIC rtlRandomError SAL_CALL rtl_random_addBytes (
    rtlRandomPool  Pool,
    const void    *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();


/** Retrieve bytes from a Random Pool.
    @param[in] Pool    a Random Pool.
    @param[in,out] Buffer a buffer to receive the random bytes.
    @param[in] Bytes the number of bytes to write to the buffer.
    @retval rtl_Random_E_None upon success.
 */
SAL_DLLPUBLIC rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool  Pool,
    void          *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_RANDOM_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
