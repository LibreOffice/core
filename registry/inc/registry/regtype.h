/*************************************************************************
 *
 *  $RCSfile: regtype.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:42 $
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

#ifndef _REGISTRY_REGTYPE_H_
#define _REGISTRY_REGTYPE_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// version number of the library. This number is used for the load on call
// mechanism and must be modifed when the library will be upgraded to a new version.
#define LIBRARY_VERSION 2

typedef void*       RegHandle;

typedef void*       RegKeyHandle;

typedef void*       RegValue;

typedef sal_uInt16  RegAccessMode;

#define REG_READONLY        0x0001  // allow read accesses only
#define REG_READWRITE       0x0002  // allow read and write accesses

/// RegKeyType specify the type of the key, if it is a real key or a link
enum RegKeyType
{
    /// The key is a real key
    RG_KEYTYPE,
    /// The key is a link
    RG_LINKTYPE
};

/// RegValueType specify the type of the value of a key
enum RegValueType
{
    /// The key has no value
    RG_VALUETYPE_NOT_DEFINED,
    /// The key has a value of type long
    RG_VALUETYPE_LONG,
    /// The key has a value of type ascii
    RG_VALUETYPE_STRING,
    /// The key has a value of type unicode
    RG_VALUETYPE_UNICODE,
    /// The key has a value of type binary
    RG_VALUETYPE_BINARY,
    /// The key has a value of type long list
    RG_VALUETYPE_LONGLIST,
    /// The key has a value of type ascii list
    RG_VALUETYPE_STRINGLIST,
    /// The key has a value of type unicode list
    RG_VALUETYPE_UNICODELIST
};

/// RegError specify the possible error codes
enum RegError
{
    /// no error
    REG_NO_ERROR,
    /// internal registry error
    REG_INTERNAL_ERROR,

    /// registry is not open
    REG_REGISTRY_NOT_OPEN,
    /// registry does not exists
    REG_REGISTRY_NOT_EXISTS,
    /// registry is readonly
    REG_REGISTRY_READONLY,
    /// destroy registry failed. Ther are may be any open keys.
    REG_DESTROY_REGISTRY_FAILED,
    /** registry cannot open for readwrite because the registry is already
        open for readwrite anywhere.
    */
    REG_CANNOT_OPEN_FOR_READWRITE,
    /** registry is in a invalid state or the registry does not point to
        a valid registry data file
    */
    REG_INVALID_REGISTRY,

    /// The key is not open
    REG_KEY_NOT_OPEN,
    /// The key not exists
    REG_KEY_NOT_EXISTS,
    /// The key cannot created
    REG_CREATE_KEY_FAILED,
    /// The key cannot not deleted
    REG_DELETE_KEY_FAILED,
    /** The keyname is invalid. This error will return if the keyname
        is NULL but should not be NULL in the context of called function.
    */
    REG_INVALID_KEYNAME,
    /// The key is not in a valid state
    REG_INVALID_KEY,

    /// key has no value
    REG_VALUE_NOT_EXISTS,
    ///
    REG_SET_VALUE_FAILED,
    ///
    REG_DELETE_VALUE_FAILED,
    ///
    REG_INVALID_VALUE,

    ///
    REG_MERGE_ERROR,
    ///
    REG_MERGE_CONFLICT,

    ///
    REG_DETECT_RECURSION,
    ///
    REG_INVALID_LINK,
    ///
    REG_INVALID_LINKNAME,
    ///
    REG_INVALID_LINKTARGET,
    ///
    REG_LINKTARGET_NOT_EXIST,
    ///
    REG_BUFFERSIZE_TOSMALL
};


#define REGISTRY_CALLTYPE   SAL_CALL

#endif
