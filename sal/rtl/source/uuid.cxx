/*************************************************************************
 *
 *  $RCSfile: uuid.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 13:28:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <osl/time.h>
#include <osl/mutex.h>
#include <osl/util.h>
#include <osl/process.h>
#include <osl/diagnose.h>

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

/***
 * does time conversion (is locked when called)
 ***/
static sal_uInt64 getGregorianTime()
{
    TimeValue val;
    sal_uInt64 nTime;

    osl_getSystemTime( &val );
    /* Offset between UUID formatted times and Unix formatted times.
         UUID UTC base time is October 15, 1582.
         Unix base time is January 1, 1970.
    */                                            //  1234567                    1234567890123456

    nTime = ((sal_uInt64) val.Seconds) *((sal_uInt64)10000000) +
            ((sal_uInt64) val.Nanosec) /100 +
            (sal_uInt64)SAL_CONST_UINT64(0x01B21DD213814000);

    return nTime;
}



/***
 * get system time in gregorian time format. It is guaranteed, that
 * every call returns a later time than the previous call ( depending
 * on the UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS macro )
 ***/
static sal_uInt64 getSystemTime( )
{
    static sal_uInt64 nLastTime = 0;
    static sal_uInt64 nTicks = 0;
    sal_uInt64 nNow;

    if( ! nLastTime )
    {
        nLastTime = getGregorianTime( );
    }

    while( sal_True )
    {
        nNow = getGregorianTime();
        if( nNow != nLastTime )
        {
            nTicks = 0;
            break;
        }
        if (nTicks < UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS )
        {
             nTicks++;
             break;
        }
    }
    nLastTime = nNow;
    return nNow + nTicks;
}


class UuidRandomPoolHolder
{
    rtlRandomPool pool;
public:
    UuidRandomPoolHolder() : pool( rtl_random_createPool() ) {}
    ~UuidRandomPoolHolder();

    rtlRandomError addBytes( const void *Buffer, sal_Size Bytes )
    {
        return rtl_random_addBytes( pool, Buffer, Bytes );
    }

    rtlRandomError getBytes( void *Buffer, sal_Size Bytes )
    {
        return rtl_random_getBytes( pool, Buffer, Bytes );
    }
};

UuidRandomPoolHolder::~UuidRandomPoolHolder()
{
    if( pool )
        rtl_random_destroyPool( pool );
}

static sal_uInt16 getInt16RandomValue( sal_uInt64 nSystemTime )
{
    sal_uInt16 n;

    static UuidRandomPoolHolder pool;
    OSL_VERIFY( rtl_Random_E_None == pool.addBytes( &nSystemTime, sizeof( nSystemTime ) ) );
    OSL_VERIFY( rtl_Random_E_None == pool.getBytes( &n, 2 ) );
    return n;
}

static void get6ByteRandomValue( sal_uInt8 *pNode )
{
    static sal_uInt8 *pStaticNode = 0;
    if( !pStaticNode )
    {
        static sal_uInt8 node[ 6 ];
        oslProcessInfo data;
        memset(&data, 0, sizeof(data));
        rtlRandomPool pool = rtl_random_createPool ();

        /* improve random value with the process identifier. This reduces the chance
           that in two concurrent process the same random number is generated. (Two
           processes on one machine can quite likley generate an uuid at the same
           time (e.g. because if interprocess communictation).
        */
        data.Size = sizeof( data );
        osl_getProcessInfo( 0 , osl_Process_HEAPUSAGE | osl_Process_IDENTIFIER , &data );
        rtl_random_addBytes( pool, &data , sizeof( data ) );

        rtl_random_getBytes( pool, node, 6 );
        rtl_random_destroyPool( pool );
        node[0] |= 0x80;
        pStaticNode = node;
    }
    memcpy( pNode , pStaticNode , 6 );
}

static void retrieve_v1( const sal_uInt8 *pPredecessorUUID,
                         sal_uInt64 *pTime ,
                         sal_uInt16 *pClockSeq,
                         sal_uInt8  *pNode )
{
    UUID uuid;

    memcpy( &uuid , pPredecessorUUID , 16 );

    SWAP_NETWORK_TO_INT32( uuid.time_low );
    SWAP_NETWORK_TO_INT16( uuid.time_mid );
    SWAP_NETWORK_TO_INT16( uuid.time_hi_and_version );

    *pTime = ( sal_uInt64 ) uuid.time_low |
             ( ( sal_uInt64 ) uuid.time_mid ) << 32 |
             ( ( sal_uInt64 ) uuid.time_hi_and_version << 48);

    *pClockSeq = ((sal_uInt16 )( uuid.clock_seq_hi_and_reserved << 8 )) +
                 ((sal_uInt16) uuid.clock_seq_low );

    memcpy( pNode, &( uuid.node ) , 6 );
    *pTime = *pTime & SAL_CONST_UINT64(0x0fffffffffffffff);
    *pClockSeq = *pClockSeq & 0x3fff;

}

static void write_v1( sal_uInt8 *pTargetUUID,
                      sal_uInt64 nTime,
                      sal_uInt16 nClockSeq,
                      sal_uInt8 *pNode)
{
    UUID uuid;
    /*                                1
                            0123456789012345 */
    nTime     = ( nTime & SAL_CONST_UINT64(0x0fffffffffffffff)) | SAL_CONST_UINT64(0x1000000000000000);
    nClockSeq = ( nClockSeq & 0x3fff ) | 0x8000;

    uuid.time_low = (sal_uInt32) ( nTime & 0xffffffff );
    uuid.time_mid = (sal_uInt16) ( ( nTime >> 32 ) & 0xffff);
    uuid.time_hi_and_version = ( sal_uInt16 ) ( ( nTime  >> 48 ) & 0xffff);
    uuid.clock_seq_hi_and_reserved = (sal_uInt8 ) (nClockSeq >> 8) & 0xff;
    uuid.clock_seq_low = (sal_uInt8 ) (nClockSeq  & 0xff );
    memcpy( uuid.node , pNode , 6 );

    // now swap to so called network byte order ( Most significant BYTE first )
    SWAP_INT32_TO_NETWORK( uuid.time_low );
    SWAP_INT16_TO_NETWORK( uuid.time_mid );
    SWAP_INT16_TO_NETWORK( uuid.time_hi_and_version );

    // final copy to avoid alignment problems
    memcpy( pTargetUUID, &uuid , 16 );
}


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
                                         const sal_uInt8 *pPredecessorUUID,
                                         sal_Bool bUseEthernetAddress )
{
    sal_uInt8 puNode[6];
    sal_uInt64 nTimeStamp;
    sal_uInt64 nLastTime;
    sal_uInt16 nClockSeq;

    /* at least guarantee that we are alone in the process */
    osl_acquireMutex( * osl_getGlobalMutex() );

    /* get current time */
    nTimeStamp = getSystemTime( );

    if( pPredecessorUUID )
    {
        retrieve_v1( pPredecessorUUID, &nLastTime, &nClockSeq, puNode );
    }

    /* get node ID */
    if( bUseEthernetAddress && osl_getEthernetAddress( puNode ) )
    {

    }
    else if( ! pPredecessorUUID )
    {
        get6ByteRandomValue( puNode );
    }

    if (!pPredecessorUUID || memcmp(puNode, ((UUID*)pPredecessorUUID)->node , 6 ))
    {
        nClockSeq = getInt16RandomValue( nTimeStamp );
    }
    else if ( nTimeStamp < nLastTime )
    {
        // Clock was set back
        nClockSeq++;
    }

    /* stuff fields into the UUID */
    write_v1( pTargetUUID , nTimeStamp , nClockSeq , puNode );

    /* release the mutex */
    osl_releaseMutex( * osl_getGlobalMutex() );
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

