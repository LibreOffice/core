/*************************************************************************
 *
 *  $RCSfile: uuid.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:15 $
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
#ifndef _RTL_UUID_H_
#define _RTL_UUID_H_


#include <sal/types.h>
#include <rtl/string.h>

/***
 * (from <draft-leach-uuids-guids-01.txt> )
 * Specification
 *
 * A UUID is an identifier that is unique across both space and time,
 * with respect to the space of all UUIDs. To be precise, the UUID
 * consists of a finite bit space. Thus the time value used for
 * constructing a UUID is limited and will roll over in the future
 * (approximately at A.D. 3400, based on the specified algorithm). A
 * UUID can be used for multiple purposes, from tagging objects with an
 * extremely short lifetime, to reliably identifying very persistent
 * objects across a network.
 *
 * The generation of UUIDs does not require that a registration
 * authority be contacted for each identifier. Instead, it requires a
 * unique value over space for each UUID generator. This spatially
 * unique value is specified as an IEEE 802 address, which is usually
 * already available to network-connected systems. This 48-bit address
 * can be assigned based on an address block obtained through the IEEE
 * registration authority.
 *
 *****/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generates a new UUID (Universally Unique IDentifier).
 * If available, the ethernetaddress of a networkcard is used, otherwise
 * a 6 Byte random number is generated( for which rtlRandomPool is used ).
 *
 *
 * @param pTargetUUID          pointer to at least 16 bytes of memory. After the call it contains
 *                             the newly generated uuid in network byte order.
 * @param pPredecessorUUID     pointer to the previously generated uuid in network byte
 *                             order. The generator reuses the 6-Byte random value
 *                             and the two byte context value and ensures,
 *                             that pTargetUUID is generated with a later timestamp.
 *                             Set pPredecessorUUID to 0 if no predecessor is available.
 *                             The caller is responsible for making the value persistent
 *                             (if desired).
 * @param bUseEthernetAddress  if sal_True, the generator uses the ethernet address of a
 *                             network card (if available). <br>
 *                             if sal_False, the generator generates a new 6-Byte random
 *                             value each time it is called with pPredecessorUUID = 0.
 *
 **/
void SAL_CALL rtl_createUuid( sal_uInt8 *pTargetUUID ,
                              const sal_uInt8 *pPredecessorUUID,
                              sal_Bool bUseEthernetAddress );

/****
 * uuid_compare --  Compare two UUID's "lexically" and return
 *        -1   u1 is lexically before u2
 *         0   u1 is equal to u2
 *         1   u1 is lexically after u2
 *
 *  Note:   lexical ordering is not temporal ordering!
 *  Note:   For equalnesschecking, a memcmp(pUUID1,pUUID2,16) is more efficient
 ****/
sal_Int32 SAL_CALL rtl_compareUuid( const sal_uInt8 *pUUID1 , const sal_uInt8 *pUUID2 );

/****
 * The version 3 UUID is meant for generating UUIDs from "names" that
 * are drawn from, and unique within, some "name space". Some examples
 * of names (and, implicitly, name spaces) might be DNS names, URLs, ISO
 * Object IDs (OIDs), reserved words in a programming language, or X.500
 * Distinguished Names (DNs); thus, the concept of name and name space
 * should be broadly construed, and not limited to textual names.
 *
 * The requirements for such UUIDs are as follows:
 *
 * - The UUIDs generated at different times from the same name in the
 *   same namespace MUST be equal
 *
 * - The UUIDs generated from two different names in the same namespace
 *   should be different (with very high probability)
 *
 * - The UUIDs generated from the same name in two different namespaces
 *   should be different with (very high probability)
 *
 * - If two UUIDs that were generated from names are equal, then they
 *   were generated from the same name in the same namespace (with very
 *   high probability).
 *
 * @param pTargetUUID pointer to at least 16 bytes of memory. After the call
 *                    it contains the newly generated uuid in network byte order.
 * @param pNameSpaceUUID The namespace uuid. Below are some predefined ones,
 *                       but any arbitray uuid can be used as namespace.
 *
 * @param pName the name
 *
 ****/
void SAL_CALL rtl_createNamedUuid(
    sal_uInt8  *pTargetUUID,
    const sal_uInt8  *pNameSpaceUUID,
    const rtl_String *pName
    );



/****
 * Predefined Namespaces
 * (Use them the following way : sal_uInt8 aNsDNS[16])  = RTL_UUID_NAMESPACE_DNS;
 *
 ****/
/* 6ba7b810-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_DNS {\
      0x6b,0xa7,0xb8,0x10,\
      0x9d,0xad,\
      0x11,0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

/* 6ba7b811-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_URL { \
      0x6b, 0xa7, 0xb8, 0x11,\
      0x9d, 0xad,\
      0x11, 0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

/* 6ba7b812-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_OID {\
      0x6b, 0xa7, 0xb8, 0x12,\
      0x9d, 0xad,\
      0x11, 0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

/* 6ba7b814-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_X500 {\
      0x6b, 0xa7, 0xb8, 0x14,\
      0x9d, 0xad,\
      0x11, 0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }


/**
 * This macro must have a value below the system time resolution of the
 * system. The uuid routines use this value  as an upper limit for adding ticks to the
 * the predecessor time value if system times are equal.
 ***/
#ifdef SAL_W32
#define UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS 1000
#elif LINUX
#define UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS 10
#elif SOLARIS
#define UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS 10
#elif MACOSX
#define UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS 100000
#elif MACOS
#define UUID_SYSTEM_TIME_RESOLUTION_100NS_TICKS 100000
#else
#error "System time resolution must be calculated!"
#endif

#ifdef __cplusplus
}
#endif

#endif
