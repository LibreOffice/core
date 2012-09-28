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

#ifndef _RTL_RANDOM_H_
#define _RTL_RANDOM_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtlRandom interface.
 *
 *======================================================================*/
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
    @param  Pool [in] a Random Pool.
    @return none. Pool is invalid.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_random_destroyPool (
    rtlRandomPool Pool
) SAL_THROW_EXTERN_C();


/** Add bytes to a Random Pool.
    @param[in] Pool   a Random Pool.
    @param[in] Buffer a buffer containing the bytes to add.
    @param[in] Bytes  the number of bytes to read from the buffer.
    @return rtl_Random_E_None upon success.
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
    @return rtl_Random_E_None upon success.
 */
SAL_DLLPUBLIC rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool  Pool,
    void          *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_RANDOM_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
