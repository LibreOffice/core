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
#include <rtl/crc.h>
#include <cassert>

#include <zlib.h>

// Zlib's crc32() is very fast, so simply use that one instead
// of implementing this ourselves.

sal_uInt32 SAL_CALL rtl_crc32 (
    sal_uInt32  Crc,
    const void *Data, sal_uInt32 DatLen) SAL_THROW_EXTERN_C()
{
    // Check that our types map to zlib correctly.
    static_assert(sizeof(uLong) >= sizeof(sal_uInt32));
    static_assert(sizeof(uInt) >= sizeof(sal_uInt32));
    // Our API says that Crc should be initialized to 0, while
    // zlib says it should be initialized with 'crc32(0,Z_NULL,0)',
    // which however simply returns 0. Ensure this.
    assert(crc32(0, Z_NULL, 0) == 0);
    if (Data)
        Crc = crc32( Crc, static_cast<const Bytef*>(Data), DatLen);
    return Crc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
