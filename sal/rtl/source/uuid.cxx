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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <string.h>
#include <stdlib.h>

#include <osl/mutex.hxx>
#include <rtl/random.h>
#include <rtl/uuid.h>
#include <rtl/digest.h>

#define SWAP_INT32_TO_NETWORK(x)\
               { sal_uInt32 y = x;\
                 sal_uInt8 *p = (sal_uInt8 * )&(x); \
                 p[0] = (sal_uInt8) ( ( y >> 24 ) & 0xff );\
                 p[1] = (sal_uInt8) ( ( y >> 16 ) & 0xff );\
                 p[2] = (sal_uInt8) ( ( y >> 8 )  & 0xff );\
                 p[3] = (sal_uInt8) ( ( y ) & 0xff);\
               }
#define SWAP_INT16_TO_NETWORK(x)\
               { sal_uInt16 y = x;\
                 sal_uInt8 *p = (sal_uInt8 * )&(x); \
                 p[0] = (sal_uInt8) ( ( y >> 8 )  & 0xff );\
                 p[1] = (sal_uInt8) ( ( y ) & 0xff);\
               }

#define SWAP_NETWORK_TO_INT16(x)\
               { sal_uInt16 y = x;\
                 sal_uInt8 *p = (sal_uInt8 * )&(y);\
                 x = ( ( ((sal_uInt16)p[0]) & 0xff) << 8 ) |\
                     ( (  (sal_uInt16)p[1]) & 0xff);\
               }
#define SWAP_NETWORK_TO_INT32(x)\
               { sal_uInt32 y = x;\
                 sal_uInt8 *p = (sal_uInt8 * )&(y); \
                 x = ( ( ((sal_uInt32)p[0]) & 0xff) << 24 ) |\
                     ( ( ((sal_uInt32)p[1]) & 0xff) << 16 ) |\
                     ( ( ((sal_uInt32)p[2]) & 0xff) << 8  ) |\
                     ( (  (sal_uInt32)p[3]) & 0xff);\
               }

typedef struct _UUID
{
      sal_uInt32          time_low;
      sal_uInt16          time_mid;
      sal_uInt16          time_hi_and_version;
      sal_uInt8           clock_seq_hi_and_reserved;
      sal_uInt8           clock_seq_low;
      sal_uInt8           node[6];
} UUID;

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
                                         const sal_uInt8 *, sal_Bool )
{
    {
        osl::MutexGuard g(osl::Mutex::getGlobalMutex());
        static rtlRandomPool pool = NULL;
        if (pool == NULL) {
            pool = rtl_random_createPool();
            if (pool == NULL) {
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
