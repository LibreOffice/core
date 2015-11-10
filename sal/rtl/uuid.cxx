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
#include <stdlib.h>

#include <osl/mutex.hxx>
#include <rtl/random.h>
#include <rtl/uuid.h>
#include <rtl/digest.h>

#define SWAP_INT32_TO_NETWORK(x)\
               { sal_uInt32 y = x;\
                 sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(&(x)); \
                 p[0] = (sal_uInt8) ( ( y >> 24 ) & 0xff );\
                 p[1] = (sal_uInt8) ( ( y >> 16 ) & 0xff );\
                 p[2] = (sal_uInt8) ( ( y >> 8 )  & 0xff );\
                 p[3] = (sal_uInt8) ( ( y ) & 0xff);\
               }
#define SWAP_INT16_TO_NETWORK(x)\
               { sal_uInt16 y = x;\
                 sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(&(x)); \
                 p[0] = (sal_uInt8) ( ( y >> 8 )  & 0xff );\
                 p[1] = (sal_uInt8) ( ( y ) & 0xff);\
               }

#define SWAP_NETWORK_TO_INT16(x)\
               { sal_uInt16 y = x;\
                 sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(&(y));\
                 x = ( ( ((sal_uInt16)p[0]) & 0xff) << 8 ) |\
                     ( (  (sal_uInt16)p[1]) & 0xff);\
               }
#define SWAP_NETWORK_TO_INT32(x)\
               { sal_uInt32 y = x;\
                 sal_uInt8 *p = reinterpret_cast<sal_uInt8 *>(&(y)); \
                 x = ( ( ((sal_uInt32)p[0]) & 0xff) << 24 ) |\
                     ( ( ((sal_uInt32)p[1]) & 0xff) << 16 ) |\
                     ( ( ((sal_uInt32)p[2]) & 0xff) << 8  ) |\
                     ( (  (sal_uInt32)p[3]) & 0xff);\
               }

struct UUID
{
      sal_uInt32          time_low;
      sal_uInt16          time_mid;
      sal_uInt16          time_hi_and_version;
      sal_uInt8           clock_seq_hi_and_reserved;
      sal_uInt8           clock_seq_low;
      sal_uInt8           node[6];
};

static  void write_v3( sal_uInt8 *pUuid  )
{
    UUID uuid;
    // copy to avoid alignment problems
    memcpy( &uuid , pUuid , 16 );

    SWAP_NETWORK_TO_INT32( uuid.time_low );
    SWAP_NETWORK_TO_INT16( uuid.time_mid );
    SWAP_NETWORK_TO_INT16( uuid.time_hi_and_version );

    /* put in the variant and version bits */
    uuid.time_hi_and_version       &= 0x0FFF;
    uuid.time_hi_and_version       |= (3 << 12);
    uuid.clock_seq_hi_and_reserved &= 0x3F;
    uuid.clock_seq_hi_and_reserved |= 0x80;

    SWAP_INT32_TO_NETWORK( uuid.time_low );
    SWAP_INT16_TO_NETWORK( uuid.time_mid );
    SWAP_INT16_TO_NETWORK( uuid.time_hi_and_version );

    memcpy( pUuid , &uuid , 16 );
}

extern "C" void SAL_CALL rtl_createUuid( sal_uInt8 *pTargetUUID ,
                                         SAL_UNUSED_PARAMETER const sal_uInt8 *,
                                         SAL_UNUSED_PARAMETER sal_Bool )
{
    {
        osl::MutexGuard g(osl::Mutex::getGlobalMutex());
        static rtlRandomPool pool = nullptr;
        if (pool == nullptr) {
            pool = rtl_random_createPool();
            if (pool == nullptr) {
                abort();
                    // only possible way to signal failure here (rtl_createUuid
                    // being part of a fixed C API)
            }
        }
        if (rtl_random_getBytes(pool, pTargetUUID, 16) != rtl_Random_E_None) {
            abort();
                // only possible way to signal failure here (rtl_createUuid
                // being part of a fixed C API)
        }
    }
    // See ITU-T Recommendation X.667:
    pTargetUUID[6] &= 0x0F;
    pTargetUUID[6] |= 0x40;
    pTargetUUID[8] &= 0x3F;
    pTargetUUID[8] |= 0x80;
}

extern "C" void SAL_CALL rtl_createNamedUuid( sal_uInt8  *pTargetUUID,
                                              const sal_uInt8  *pNameSpaceUUID,
                                              const rtl_String *pName )
{
    rtlDigest digest = rtl_digest_createMD5  ();

    rtl_digest_updateMD5( digest, pNameSpaceUUID , 16 );
    rtl_digest_updateMD5( digest, pName->buffer , pName->length );

    rtl_digest_getMD5( digest, pTargetUUID , 16 );
    rtl_digest_destroyMD5 (digest);

    write_v3(pTargetUUID);
}

extern "C" sal_Int32 SAL_CALL rtl_compareUuid( const sal_uInt8 *pUUID1 , const sal_uInt8 *pUUID2 )
{
    int i;
    UUID u1;
    UUID u2;
    memcpy( &u1 , pUUID1 , 16 );
    memcpy( &u2 , pUUID2 , 16 );

    SWAP_NETWORK_TO_INT32( u1.time_low );
    SWAP_NETWORK_TO_INT16( u1.time_mid );
    SWAP_NETWORK_TO_INT16( u1.time_hi_and_version );

    SWAP_NETWORK_TO_INT32( u2.time_low );
    SWAP_NETWORK_TO_INT16( u2.time_mid );
    SWAP_NETWORK_TO_INT16( u2.time_hi_and_version );

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
