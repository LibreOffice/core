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
#include <o3tl/typed_flags_set.hxx>

/// defines the type of a registry handle used in the C API.
typedef void* RegHandle;

/// defines the type of a registry key handle used in the C API.
typedef void* RegKeyHandle;

/// defines the type of a registry key value handle used in the C API.
typedef void* RegValue;

/** defines the open/access mode of the registry.

    Two modes are valid:
    -READONLY    allows readonly access
    -READWRITE   allows read and write access
 */
enum class RegAccessMode
{
    READONLY = 0x0001, /// This mode allows readonly access.
    READWRITE = 0x0002 /// This mode allows read and write access.
};
namespace o3tl
{
template <> struct typed_flags<RegAccessMode> : is_typed_flags<RegAccessMode, 0x03>
{
};
}

/** defines the type of a key value.

    A registry key can contain a value which has one of seven different types.
    Three simple types (long, ascii and unicode string) and a list type of
    these simple types. Furthermore a binary type which provides the possibility
    to define own data structures and store these types in the registry. The UNO
    core reflection data is stored as a binary blob in the type registry.
 */
enum class SAL_DLLPUBLIC_RTTI RegValueType
{
    /// The key has no value or the value type is unknown.
    NOT_DEFINED,
    /// The key has a value of type long
    LONG,
    /// The key has a value of type ascii string
    STRING,
    /// The key has a value of type unicode string
    UNICODE,
    /// The key has a value of type binary
    BINARY,
    /// The key has a value of type long list
    LONGLIST,
    /// The key has a value of type ascii string list
    STRINGLIST,
    /// The key has a value of type unicode string list
    UNICODELIST
};

/// specifies the possible error codes which can occur using the registry API.
enum class SAL_DLLPUBLIC_RTTI RegError
{
    /// no error.
    NO_ERROR,

    /// registry is not open.
    REGISTRY_NOT_OPEN,
    /// registry does not exists.
    REGISTRY_NOT_EXISTS,
    /// registry is open with readonly access rights.
    REGISTRY_READONLY,
    /// destroy a registry failed. There are may be any open keys.
    DESTROY_REGISTRY_FAILED,
    /** registry cannot be opened with readwrite access because the registry is already
        open with readwrite access anywhere.
    */
    CANNOT_OPEN_FOR_READWRITE,
    /** registry is in an invalid state or the registry does not point to
        a valid registry data file.
    */
    INVALID_REGISTRY,

    /// the key or key handle points to an invalid key or closed key.
    KEY_NOT_OPEN,
    /// the specified keyname points to a nonexisting key.
    KEY_NOT_EXISTS,
    /// the key with the specified keyname cannot be created.
    CREATE_KEY_FAILED,
    /// the specified key cannot be deleted. Maybe an open key handle exists to this key.
    DELETE_KEY_FAILED,
    /** the keyname is invalid. This error will return if the keyname
        is NULL but should not be NULL in the context of a called function.
    */
    INVALID_KEYNAME,
    /// the key is not in a valid state.
    INVALID_KEY,

    /// the key has no value
    VALUE_NOT_EXISTS,
    /// setting the specified value of a key failed.
    SET_VALUE_FAILED,
    /// deleting of the key value failed.
    DELETE_VALUE_FAILED,
    /// the key has an invalid value or the value type is unknown.
    INVALID_VALUE
};

/// specify the calling convention for the registry API
 SAL_CALL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
