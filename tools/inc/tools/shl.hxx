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

#ifndef _SHL_HXX
#define _SHL_HXX

#include <tools/solar.h>
#include "tools/toolsdllapi.h"

// GetAppData()

#define SHL_SHL1        0
#define SHL_SHL2        1
// 2 (SHL_SHL3) removed
// 3 (SHL_APP1) removed
// 4 (SHL_APP2) removed
// 5 (SHL_APP3) removed
// 6 (SHL_TOOLS) removed
// 7 (SHL_SV) removed
// 8 (SHL_SVT) removed
#define SHL_SVDDE       9
#define SHL_ERR         10
//11 (SHL_IPC) removed
//12 (SHL_SVX) removed
//13 (SHL_ITEM) removed
#define SHL_SVD         14
//15 (SHL_SI) removed
//16 (SHL_SFC) removed
//17 (SHL_SFX) removed
//18 (SHL_SO2) removed
#define SHL_IDL         19
#define SHL_IDE         20
//21 (SHL_EDIT) removed
//22 (SHL_VCED) removed
#define SHL_BASIC       23
//24 (SHL_HELP) removed
//25 (SHL_LNKCCH) removed
//26 (SHL_CHANNEL) removed
//27 (SHL_SBX) removed
#define SHL_SBC         28
//29 (SHL_SDB) removed
//30 (SHL_SBA) removed
//31 (SHL_SBA2) removed
//32 (SHL_SCH) removed
//33 (SHL_SIM) removed
#define SHL_SM          34
//35 (SHL_SGA) removed
//36 (SHL_DOCMGR) removed
//37 removed
//38 (SHL_MAIL) removed
//39 (SHL_NEWS) removed
//40 (SHL_OFFAPP) removed
//41 (SHL_INET) removed
//43 (SHL_CHAOS) removed
//43 (SHL_HISTORY) removed
//44 (SHL_SJ) removed
#define SHL_CALC        45
#define SHL_DRAW        46
#define SHL_WRITER      47
//48 (SHL_MONEY) removed
//49 (SHL_HBCI) removed
//50 (SHL_BASE3D) removed
#define SHL_BRUSHITEM   51
//52 (SHL_SFONTITEM) removed

// #110743#
// #define SHL_COUNT        53
// the following added for binary filter project
// Since an array is created for that values, it is necessary to
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
