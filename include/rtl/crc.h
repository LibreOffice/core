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

#ifndef INCLUDED_RTL_CRC_H
#define INCLUDED_RTL_CRC_H

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Evaluate CRC32 over given data.

    This function evaluates the CRC polynomial 0xEDB88320.

    @param[in] Crc    CRC32 over previous data or zero.
    @param[in] Data   data buffer.
    @param[in] DatLen data buffer length.
    @return new CRC32 value.
 */
SAL_DLLPUBLIC sal_uInt32 SAL_CALL rtl_crc32 (
    sal_uInt32  Crc,
    const void *Data, sal_uInt32 DatLen
) SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_CRC_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
