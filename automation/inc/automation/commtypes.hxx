/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commtypes.hxx,v $
 * $Revision: 1.3 $
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
 *    Eather including apropriate files or defining the types when not available in the environment
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

// this is a trick to be sure the SO types are unknown, so the compiler will yell at us in case we use them
#ifndef _SOLAR_H
#define NO_SOLAR
#endif

#ifdef NO_SOLAR
#define BYTE BYTE_hidden
#define BOOL BOOL_hidden
#define ULONG ULONG_hidden
#define USHORT USHORT_hidden
#define UINT16 UINT16_hidden
#define UINT32 UINT32_hidden

#endif

//#define DBG_ASSERT( cond, text )
//#define DBG_ERROR( text )
#include <tools/debug.hxx>

#include <tools/solar.h>
typedef BYTE comm_BYTE;
typedef BOOL comm_BOOL;
typedef sal_uInt32 comm_ULONG;
typedef sal_uInt16 comm_USHORT;
typedef UINT16 comm_UINT16;
typedef UINT32 comm_UINT32;

#ifdef NO_SOLAR
#undef BYTE
#undef BOOL
#undef ULONG
#undef USHORT
#undef UINT16
#undef UINT32

#endif

typedef sal_uInt16      comm_WORD;
typedef sal_uInt32      comm_DWORD;

class SmartId;
class String;
#define comm_String String

#endif
