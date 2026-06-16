/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <stdlib.h>

#include <rtl/random.h>
#include <rtl/uuid.h>

#define SWAP_NETWORK_TO_INT16(x)\
               { sal_uInt16 y = x;\
                 sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(&(y));\
                 x = ( ( (static_cast<sal_uInt16>(p[0])) & 0xff) << 8 ) |\
                     ( (  static_cast<sal_uInt16>(p[1])) & 0xff);\
               }
#define SWAP_NETWORK_TO_INT32(x)\
               { sal_uInt32 y = x;\
                 sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(&(y)); \
                 x = ( ( (static_cast<sal_uInt32>(p[0])) & 0xff) << 24 ) |\
                     ( ( (static_cast<sal_uInt32>(p[1])) & 0xff) << 16 ) |\
                     ( ( (static_cast<sal_uInt32>(p[2])) & 0xff) << 8  ) |\
                     ( (  static_cast<sal_uInt32>(p[3])) & 0xff);\
               }

namespace {

struct UUID
{
      sal_uInt32          time_low;
      sal_uInt16          time_mid;
      sal_uInt16          time_hi_and_version;
      sal_uInt8           clock_seq_hi_and_reserved;
      sal_uInt8           clock_seq_low;
      sal_uInt8           node[6];
};

}

extern "C" void SAL_CALL rtl_createUuid(sal_uInt8 *pTargetUUID ,
                                        SAL_UNUSED_PARAMETER const sal_uInt8 *,
                                        SAL_UNUSED_PARAMETER bool)
{
    rtl_random_getBytes(pTargetUUID, 16);

    // See ITU-T Recommendation X.667:
    pTargetUUID[6] &= 0x0F;
    pTargetUUID[6] |= 0x40;
    pTargetUUID[8] &= 0x3F;
    pTargetUUID[8] |= 0x80;
}

extern "C" sal_Int32 SAL_CALL rtl_compareUuid(const sal_uInt8 *pUUID1, const sal_uInt8 *pUUID2)
{
    int i;
    UUID u1;
    UUID u2;
    memcpy(&u1, pUUID1, 16 );
    memcpy(&u2, pUUID2, 16 );

    SWAP_NETWORK_TO_INT32(u1.time_low);
    SWAP_NETWORK_TO_INT16(u1.time_mid);
    SWAP_NETWORK_TO_INT16(u1.time_hi_and_version);

    SWAP_NETWORK_TO_INT32(u2.time_low);
    SWAP_NETWORK_TO_INT16(u2.time_mid);
    SWAP_NETWORK_TO_INT16(u2.time_hi_and_version);

#define CHECK(f1, f2) if (f1 != f2) return f1 < f2 ? -1 : 1;
    CHECK(u1.time_low, u2.time_low);
    CHECK(u1.time_mid, u2.time_mid);
    CHECK(u1.time_hi_and_version, u2.time_hi_and_version);
    CHECK(u1.clock_seq_hi_and_reserved, u2.clock_seq_hi_and_reserved);
    CHECK(u1.clock_seq_low, u2.clock_seq_low);
    for (i = 0; i < 6; i++)
    {
        if (u1.node[i] < u2.node[i])
            return -1;
        if (u1.node[i] > u2.node[i])
            return 1;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
