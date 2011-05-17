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

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to supply the types and defines for Environment independent
 *    building of the packethandler
 *    Only adaption of this file should be necessary. Else it is a bug!
 *    Ether including apropriate files or defining the types when not available in the environment
 *
 ************************************************************************/

/** defines al least types
    comm_BYTE
    comm_WORD;
    comm_DWORD;
    comm_BOOL
    comm_ULONG
    comm_USHORT
    comm_UINT16
    comm_UINT32
    comm_UniChar
**/

#ifndef _COMMTYPES_HXX
#define _COMMTYPES_HXX

#include <sal/types.h>
typedef sal_Unicode comm_UniChar;
#include "rtl/string.hxx"

#include <tools/debug.hxx>

#include <tools/solar.h>
typedef sal_uInt8 comm_BYTE;
typedef sal_Bool comm_BOOL;
typedef sal_uInt32 comm_ULONG;
typedef sal_uInt16 comm_USHORT;
typedef sal_uInt16 comm_UINT16;
typedef sal_uInt32 comm_UINT32;

typedef sal_uInt16      comm_WORD;
typedef sal_uInt32      comm_DWORD;

class String;
#define comm_String String

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
