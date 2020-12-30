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

#pragma once

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** PageSize (recommended) default.
    @see store_openFile()
 */
constexpr sal_uInt16 STORE_DEFAULT_PAGESIZE = 0x0400;


/** PageSize (enforced) limits.
    @see store_openFile()
 */
constexpr sal_uInt16 STORE_MINIMUM_PAGESIZE = 0x0200;
constexpr sal_uInt16 STORE_MAXIMUM_PAGESIZE = 0x8000;


/** NameSize (enforced) limit.
    @see any param pName
    @see store_E_NameTooLong
 */
constexpr auto STORE_MAXIMUM_NAMESIZE = 256;


/** Attributes (predefined).
    @see store_attrib()
 */
constexpr sal_uInt32 STORE_ATTRIB_ISLINK = 0x10000000;
constexpr sal_uInt32 STORE_ATTRIB_ISDIR = 0x20000000;
constexpr sal_uInt32 STORE_ATTRIB_ISFILE = 0x40000000;


/** Access Mode enumeration.
    @see store_openFile()
    @see store_openDirectory()
    @see store_openStream()
 */
enum class storeAccessMode
{
    Create,
    ReadWrite,
    ReadOnly,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};


/** Error Code enumeration.
 */
typedef enum
{
    store_E_None = 0,
    store_E_AccessViolation,
    store_E_LockingViolation,
    store_E_CantSeek,
    store_E_CantRead,
    store_E_CantWrite,
    store_E_InvalidAccess,
    store_E_InvalidHandle,
    store_E_InvalidParameter,
    store_E_InvalidChecksum,
    store_E_AlreadyExists,
    store_E_NotExists,
    store_E_NotDirectory,
    store_E_NotFile,
    store_E_NoMoreFiles,
    store_E_NameTooLong,
    store_E_OutOfMemory,
    store_E_OutOfSpace,
    store_E_Pending,
    store_E_WrongFormat,
    store_E_WrongVersion,
    store_E_Unknown,
    store_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} storeError;


/** Find Data structure.
    @see store_findFirst()
    @see store_findNext()
 */
typedef struct
{
    /** Name.
        @see m_nLength
     */
    sal_Unicode m_pszName[STORE_MAXIMUM_NAMESIZE];

    /** Name Length.
        @see m_pszName
     */
    sal_Int32 m_nLength;

    /** Attributes.
        @see store_attrib()
     */
    sal_uInt32 m_nAttrib;

    /** Reserved for internal use.
     */
    sal_uInt32 m_nReserved;
} storeFindData;

#ifdef __cplusplus
}
#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
