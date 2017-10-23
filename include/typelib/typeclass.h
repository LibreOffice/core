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
#ifndef INCLUDED_TYPELIB_TYPECLASS_H
#define INCLUDED_TYPELIB_TYPECLASS_H

#include "sal/types.h"

/** This type class enum is binary compatible with the IDL enum com.sun.star.uno.TypeClass.
*/
typedef enum _typelib_TypeClass
{
    /** type class of void */
    typelib_TypeClass_VOID = 0,
    /** type class of char */
    typelib_TypeClass_CHAR = 1,
    /** type class of boolean */
    typelib_TypeClass_BOOLEAN = 2,
    /** type class of byte */
    typelib_TypeClass_BYTE = 3,
    /** type class of short */
    typelib_TypeClass_SHORT = 4,
    /** type class of unsigned short */
    typelib_TypeClass_UNSIGNED_SHORT = 5,
    /** type class of long */
    typelib_TypeClass_LONG = 6,
    /** type class of unsigned long */
    typelib_TypeClass_UNSIGNED_LONG = 7,
    /** type class of hyper */
    typelib_TypeClass_HYPER = 8,
    /** type class of unsigned hyper */
    typelib_TypeClass_UNSIGNED_HYPER = 9,
    /** type class of float */
    typelib_TypeClass_FLOAT = 10,
    /** type class of double */
    typelib_TypeClass_DOUBLE = 11,
    /** type class of string */
    typelib_TypeClass_STRING = 12,
    /** type class of type */
    typelib_TypeClass_TYPE = 13,
    /** type class of any */
    typelib_TypeClass_ANY = 14,
    /** type class of enum */
    typelib_TypeClass_ENUM = 15,
    /** type class of typedef */
    typelib_TypeClass_TYPEDEF = 16,
    /** type class of struct */
    typelib_TypeClass_STRUCT = 17,
    /** Deprecated, UNOIDL does not have a union concept.

        @deprecated
    */
    typelib_TypeClass_UNION = 18,
    /** type class of exception */
    typelib_TypeClass_EXCEPTION = 19,
    /** type class of sequence */
    typelib_TypeClass_SEQUENCE = 20,
    /** Deprecated, UNOIDL does not have an array concept.

        @deprecated
    */
    typelib_TypeClass_ARRAY = 21,
    /** type class of interface */
    typelib_TypeClass_INTERFACE = 22,
    /** type class of service (not implemented) */
    typelib_TypeClass_SERVICE = 23,
    /** type class of module (not implemented) */
    typelib_TypeClass_MODULE = 24,
    /** type class of interface method */
    typelib_TypeClass_INTERFACE_METHOD = 25,
    /** type class of interface attribute */
    typelib_TypeClass_INTERFACE_ATTRIBUTE = 26,
    /** type class of unknown type */
    typelib_TypeClass_UNKNOWN = 27,
    /** type class of properties */
    typelib_TypeClass_PROPERTY = 28,
    /** type class of constants */
    typelib_TypeClass_CONSTANT = 29,
    /** type class of constants groups */
    typelib_TypeClass_CONSTANTS = 30,
    /** type class of singletons */
    typelib_TypeClass_SINGLETON = 31,
    /** fixing enum size */
    typelib_TypeClass_MAKE_FIXED_SIZE = SAL_MAX_ENUM
} typelib_TypeClass;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
