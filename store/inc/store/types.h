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

#ifndef _STORE_TYPES_H_
#define _STORE_TYPES_H_ "$Revision: 1.8 $"

#include <sal/types.h>
#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C" {
#endif

/** PageSize (recommended) default.
    @see store_openFile()
 */
#define STORE_DEFAULT_PAGESIZE ((sal_uInt16)0x0400)


/** PageSize (enforced) limits.
    @see store_openFile()
 */
#define STORE_MINIMUM_PAGESIZE ((sal_uInt16)0x0200)
#define STORE_MAXIMUM_PAGESIZE ((sal_uInt16)0x8000)


/** NameSize (enforced) limit.
    @see any param pName
    @see store_E_NameTooLong
 */
#define STORE_MAXIMUM_NAMESIZE 256


/** Attributes (predefined).
    @see store_attrib()
 */
#define STORE_ATTRIB_ISLINK  ((sal_uInt32)0x10000000)
#define STORE_ATTRIB_ISDIR   ((sal_uInt32)0x20000000)
#define STORE_ATTRIB_ISFILE  ((sal_uInt32)0x40000000)


/** Access Mode enumeration.
    @see store_openFile()
    @see store_openDirectory()
    @see store_openStream()
 */
enum __store_AccessMode
{
    store_AccessCreate,
    store_AccessReadCreate,
    store_AccessReadWrite,
    store_AccessReadOnly,
    store_Access_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Access Mode type.
 */
typedef enum __store_AccessMode storeAccessMode;


/** Error Code enumeration.
 */
enum __store_Error
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
};

/** Error Code type.
 */
typedef enum __store_Error storeError;


/** Find Data structure.
    @see store_findFirst()
    @see store_findNext()
 */
struct __store_FindData
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

    /** Size.
        @see store_getStreamSize()
        @see store_setStreamSize()
     */
    sal_uInt32 m_nSize;

    /** Reserved for internal use.
     */
    sal_uInt32 m_nReserved;
};

/** Find Data type.
 */
typedef struct __store_FindData storeFindData;


/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _STORE_TYPES_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
