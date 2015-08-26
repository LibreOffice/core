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

#include <sal/types.h>
#include <checksum.hxx>

/*========================================================================
 *
 * vcl_crc64 implementation.
 *
 *======================================================================*/
#define UPDCRC64(crc, octet) \
    (vcl_crc64Table[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))

/*
 * rtl_crc64.
 */
sal_uInt64 SAL_CALL vcl_crc64 (
    sal_uInt64  Crc,
    const void *Data, sal_uInt32 DatLen)  SAL_THROW_EXTERN_C()
{
    if (Data)
    {
        const sal_uInt8 *p = static_cast<const sal_uInt8 *>(Data);
        const sal_uInt8 *q = p + DatLen;

        Crc = ~Crc;
        while (p < q)
            Crc = UPDCRC64(Crc, *(p++));
        Crc = ~Crc;
    }
    return Crc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
