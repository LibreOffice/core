/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:40:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _STORE_TYPES_H_
#define _STORE_TYPES_H_ "$Revision: 1.6 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

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
#define STORE_ATTRIB_ISLINK  0x10000000UL
#define STORE_ATTRIB_ISDIR   0x20000000UL
#define STORE_ATTRIB_ISFILE  0x40000000UL


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

