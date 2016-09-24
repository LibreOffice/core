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
#ifndef INCLUDED_RTL_UUID_H
#define INCLUDED_RTL_UUID_H

#include <sal/config.h>

#include <rtl/string.h>
#include <sal/saldllapi.h>
#include <sal/types.h>

/**
   @file
   Specification (from draft-leach-uuids-guids-01.txt )

   <p>
   A UUID is an identifier that is unique across both space and time,
   with respect to the space of all UUIDs. To be precise, the UUID
   consists of a finite bit space. Thus, collision cannot be avoided in
   principle. A UUID can be used for multiple purposes, from tagging objects
   with an extremely short lifetime, to reliably identifying very persistent
   objects across a network.

   <p>
    The generation of UUIDs does not require that a registration
    authority be contacted for each identifier. Instead, Version 4 UUIDs are
    generated from (pseudo unique) sequences of (pseudo) random bits.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Generates a new Version 4 (random number based) UUID (Universally Unique
    IDentifier).

    @param pTargetUUID          pointer to at least 16 bytes of memory. After the call it contains
                                the newly generated uuid in network byte order.
    @param pPredecessorUUID     ignored (was used when this function returned
                                Version 1 instead of Version 4 UUIDs).
    @param bUseEthernetAddress  ignored (was used when this function returned
                                Version 1 instead of Version 4 UUIDs).
 */
SAL_DLLPUBLIC void SAL_CALL rtl_createUuid(
                              sal_uInt8 *pTargetUUID,
                              const sal_uInt8 *pPredecessorUUID,
                              sal_Bool bUseEthernetAddress );

/** Compare two UUID's lexically

    <p>
    Note:   lexical ordering is not temporal ordering!
    <p>
    Note:   For equalnesschecking, a memcmp(pUUID1,pUUID2,16) is more efficient

    @return
    <ul>
    <li>-1   u1 is lexically before u2
    <li>0   u1 is equal to u2
    <li>1   u1 is lexically after u2
    </ul>

 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_compareUuid(
        const sal_uInt8 *pUUID1 , const sal_uInt8 *pUUID2 );

/** Creates named UUIDs.

    <p>
    The version 3 UUID is meant for generating UUIDs from <em>names</em> that
    are drawn from, and unique within, some <em>name space</em>. Some examples
    of names (and, implicitly, name spaces) might be DNS names, URLs, ISO
    Object IDs (OIDs), reserved words in a programming language, or X.500
    Distinguished Names (DNs); thus, the concept of name and name space
    should be broadly construed, and not limited to textual names.

    <p>
    The requirements for such UUIDs are as follows:

    <ul>
    <li> The UUIDs generated at different times from the same name in the
         same namespace MUST be equal

    <li> The UUIDs generated from two different names in the same namespace
         should be different (with very high probability)

    <li> The UUIDs generated from the same name in two different namespaces
         should be different with (very high probability)

    <li> If two UUIDs that were generated from names are equal, then they
         were generated from the same name in the same namespace (with very
         high probability).
    </ul>

    @param pTargetUUID pointer to at least 16 bytes of memory. After the call
                       it contains the newly generated uuid in network byte order.
    @param pNameSpaceUUID The namespace uuid. Below are some predefined ones,
                          but any arbitray uuid can be used as namespace.

    @param pName the name
 */
SAL_DLLPUBLIC void SAL_CALL rtl_createNamedUuid(
    sal_uInt8  *pTargetUUID,
    const sal_uInt8  *pNameSpaceUUID,
    const rtl_String *pName
    );



/*
    Predefined Namespaces
    (Use them the following way : sal_uInt8 aNsDNS[16])  = RTL_UUID_NAMESPACE_DNS;
 */
/** namespace DNS

    <p>
    (Use them the following way : sal_uInt8 aNsDNS[16])  = RTL_UUID_NAMESPACE_DNS;
    <p>
   6ba7b810-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_DNS {\
      0x6b,0xa7,0xb8,0x10,\
      0x9d,0xad,\
      0x11,0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

/** namespace URL

   <p>
   6ba7b811-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_URL { \
      0x6b, 0xa7, 0xb8, 0x11,\
      0x9d, 0xad,\
      0x11, 0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

/** namespace oid

    <p>
    6ba7b812-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_OID {\
      0x6b, 0xa7, 0xb8, 0x12,\
      0x9d, 0xad,\
      0x11, 0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

/** namespace X500

    <p>
    6ba7b814-9dad-11d1-80b4-00c04fd430c8 */
#define RTL_UUID_NAMESPACE_X500 {\
      0x6b, 0xa7, 0xb8, 0x14,\
      0x9d, 0xad,\
      0x11, 0xd1,\
      0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8\
    }

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
