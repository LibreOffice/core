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

#ifndef _RTL_CRC_H_
#define _RTL_CRC_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtl_crc32 interface.
 *
 *======================================================================*/
/** Evaluate CRC32 over given data.
    @descr This function evaluates the CRC polynomial 0xEDB88320.

    @param  Crc    [in] CRC32 over previous data or zero.
    @param  Data   [in] data buffer.
    @param  DatLen [in] data buffer length.
    @return new CRC32 value.
 */
SAL_DLLPUBLIC sal_uInt32 SAL_CALL rtl_crc32 (
    sal_uInt32  Crc,
    const void *Data, sal_uInt32 DatLen
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_CRC_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
