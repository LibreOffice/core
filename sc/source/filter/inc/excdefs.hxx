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

#define EXC_STREAM_USERNAMES        "User Names"
#define EXC_STREAM_REVLOG           "Revision Log"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
