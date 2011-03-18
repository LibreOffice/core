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

#ifndef _SHL_HXX
#define _SHL_HXX

#include <tools/solar.h>
#include "tools/toolsdllapi.h"

// ----------------
// - GetAppData() -
// ----------------

#define SHL_SHL1        0
#define SHL_SHL2        1
#define SHL_SHL3        2
#define SHL_APP1        3
#define SHL_APP2        4
#define SHL_APP3        5
#define SHL_TOOLS       6
#define SHL_SV          7
#define SHL_SVT         8
#define SHL_SVDDE       9
#define SHL_ERR         10
#define SHL_IPC         11
#define SHL_SVX         12
#define SHL_ITEM        13
#define SHL_SVD         14
#define SHL_SI          15
#define SHL_SFC         16
#define SHL_SFX         17
//#define SHL_SO2       18
#define SHL_IDL         19
#define SHL_IDE         20
#define SHL_EDIT        21
#define SHL_VCED        22
#define SHL_BASIC       23
#define SHL_HELP        24
#define SHL_LNKCCH      25
#define SHL_CHANNEL     26
#define SHL_SBX         27
#define SHL_SBC         28
#define SHL_SDB         29
#define SHL_SBA         30
#define SHL_SBA2        31
#define SHL_SCH         32
#define SHL_SIM         33
#define SHL_SM          34
#define SHL_SGA         35
#define SHL_DOCMGR      36
// 37 removed
#define SHL_MAIL        38
#define SHL_NEWS        39
#define SHL_OFFAPP      40
#define SHL_INET        41
#define SHL_CHAOS       42
#define SHL_HISTORY     43
#define SHL_SJ          44
#define SHL_CALC        45
#define SHL_DRAW        46
#define SHL_WRITER      47
#define SHL_MONEY       48
#define SHL_HBCI        49
#define SHL_BASE3D      50
#define SHL_BRUSHITEM   51
#define SHL_SFONTITEM   52

// #110743#
// #define SHL_COUNT        53
// the following added for binary filter project
// Sice an array is created for that values, it is necessary to
// put them directly behind the defined ones. Else, some space is
// wasted.
#define BF_SHL_SVX      53
#define BF_SHL_ITEM     54
#define BF_SHL_SVD      55
#define BF_SHL_EDIT     56
#define BF_SHL_SCH      57
#define BF_SHL_SM       58
#define BF_SHL_CALC     59
#define BF_SHL_DRAW     60
#define BF_SHL_WRITER   61
#define BF_SHL_SVT      62
#define BF_SHL_SVDDE    63

#define SHL_COUNT       64

TOOLS_DLLPUBLIC void** GetAppData( sal_uInt16 nSharedLib );

#endif // _SHL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
