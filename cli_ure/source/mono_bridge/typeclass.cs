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

namespace uno.Typelib {

public class TypeClass
{
    /** type class of void */
    public const int VOID = 0;
    /** type class of char */
    public const int CHAR = 1;
    /** type class of boolean */
    public const int BOOLEAN = 2;
    /** type class of byte */
    public const int BYTE = 3;
    /** type class of short */
    public const int SHORT = 4;
    /** type class of unsigned short */
    public const int UNSIGNED_SHORT = 5;
    /** type class of long */
    public const int LONG = 6;
    /** type class of unsigned long */
    public const int UNSIGNED_LONG = 7;
    /** type class of hyper */
    public const int HYPER = 8;
    /** type class of unsigned hyper */
    public const int UNSIGNED_HYPER = 9;
    /** type class of float */
    public const int FLOAT = 10;
    /** type class of double */
    public const int DOUBLE = 11;
    /** type class of string */
    public const int STRING = 12;
    /** type class of type */
    public const int TYPE = 13;
    /** type class of any */
    public const int ANY = 14;
    /** type class of enum */
    public const int ENUM = 15;
    /** type class of typedef */
    public const int TYPEDEF = 16;
    /** type class of struct */
    public const int STRUCT = 17;
    /** type class of union (not implemented) */
    public const int UNION = 18;
    /** type class of exception */
    public const int EXCEPTION = 19;
    /** type class of sequence */
    public const int SEQUENCE = 20;
    /** type class of array (not implemented) */
    public const int ARRAY = 21;
    /** type class of interface */
    public const int INTERFACE = 22;
    /** type class of service (not implemented) */
    public const int SERVICE = 23;
    /** type class of module (not implemented) */
    public const int MODULE = 24;
    /** type class of interface method */
    public const int INTERFACE_METHOD = 25;
    /** type class of interface attribute */
    public const int INTERFACE_ATTRIBUTE = 26;
    /** type class of unknown type */
    public const int UNKNOWN = 27;
    /** type class of properties */
    public const int PROPERTY = 28;
    /** type class of constants */
    public const int CONSTANT = 29;
    /** type class of constants groups */
    public const int CONSTANTS = 30;
    /** type class of singletons */
    public const int SINGLETON = 31;
}

}
