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

#ifndef SC_EXCDEFS_HXX
#define SC_EXCDEFS_HXX

#include <sal/types.h>

// (0x009B, 0x009D, 0x009E) AUTOFILTER ========================================

// flags
const sal_uInt16 EXC_AFFLAG_AND             = 0x0000;
const sal_uInt16 EXC_AFFLAG_OR              = 0x0001;
const sal_uInt16 EXC_AFFLAG_ANDORMASK       = 0x0003;
const sal_uInt16 EXC_AFFLAG_SIMPLE1         = 0x0004;
const sal_uInt16 EXC_AFFLAG_SIMPLE2         = 0x0008;
const sal_uInt16 EXC_AFFLAG_TOP10           = 0x0010;
const sal_uInt16 EXC_AFFLAG_TOP10TOP        = 0x0020;
const sal_uInt16 EXC_AFFLAG_TOP10PERC       = 0x0040;

// data types
const sal_uInt8 EXC_AFTYPE_NOTUSED          = 0x00;
const sal_uInt8 EXC_AFTYPE_RK               = 0x02;
const sal_uInt8 EXC_AFTYPE_DOUBLE           = 0x04;
const sal_uInt8 EXC_AFTYPE_STRING           = 0x06;
const sal_uInt8 EXC_AFTYPE_BOOLERR          = 0x08;
const sal_uInt8 EXC_AFTYPE_INVALID          = 0x0A;
const sal_uInt8 EXC_AFTYPE_EMPTY            = 0x0C;
const sal_uInt8 EXC_AFTYPE_NOTEMPTY         = 0x0E;

// comparison operands
const sal_uInt8 EXC_AFOPER_NONE             = 0x00;
const sal_uInt8 EXC_AFOPER_LESS             = 0x01;
const sal_uInt8 EXC_AFOPER_EQUAL            = 0x02;
const sal_uInt8 EXC_AFOPER_LESSEQUAL        = 0x03;
const sal_uInt8 EXC_AFOPER_GREATER          = 0x04;
const sal_uInt8 EXC_AFOPER_NOTEQUAL         = 0x05;
const sal_uInt8 EXC_AFOPER_GREATEREQUAL     = 0x06;

// (0x00AE, 0x00AF) SCENARIO, SCENMAN =========================================

#define EXC_SCEN_MAXCELL            32

// defines for change tracking ================================================

#define EXC_STREAM_USERNAMES        CREATE_STRING( "User Names" )
#define EXC_STREAM_REVLOG           CREATE_STRING( "Revision Log" )

// opcodes
#define EXC_CHTR_OP_COLFLAG         0x0001
#define EXC_CHTR_OP_DELFLAG         0x0002
#define EXC_CHTR_OP_INSROW          0x0000
#define EXC_CHTR_OP_INSCOL          EXC_CHTR_OP_COLFLAG
#define EXC_CHTR_OP_DELROW          EXC_CHTR_OP_DELFLAG
#define EXC_CHTR_OP_DELCOL          (EXC_CHTR_OP_COLFLAG|EXC_CHTR_OP_DELFLAG)
#define EXC_CHTR_OP_MOVE            0x0004
#define EXC_CHTR_OP_INSTAB          0x0005
#define EXC_CHTR_OP_CELL            0x0008
#define EXC_CHTR_OP_RENAME          0x0009
#define EXC_CHTR_OP_NAME            0x000A
#define EXC_CHTR_OP_FORMAT          0x000B
#define EXC_CHTR_OP_UNKNOWN         0xFFFF

// data types
#define EXC_CHTR_TYPE_MASK          0x0007
#define EXC_CHTR_TYPE_FORMATMASK    0xFF00
#define EXC_CHTR_TYPE_EMPTY         0x0000
#define EXC_CHTR_TYPE_RK            0x0001
#define EXC_CHTR_TYPE_DOUBLE        0x0002
#define EXC_CHTR_TYPE_STRING        0x0003
#define EXC_CHTR_TYPE_BOOL          0x0004
#define EXC_CHTR_TYPE_FORMULA       0x0005

// accept flags
#define EXC_CHTR_NOTHING            0x0000
#define EXC_CHTR_ACCEPT             0x0001
#define EXC_CHTR_REJECT             0x0003

// ============================================================================

#endif // _EXCDEFS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
