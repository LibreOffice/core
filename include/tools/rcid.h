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

#ifndef INCLUDED_TOOLS_RCID_H
#define INCLUDED_TOOLS_RCID_H

#include <tools/resid.hxx>

// Definition of the version number
#define RSCVERSION_ID           200U
// Definition of an invalid identifier
#define RC_NO_ID                (sal_uInt32(0xFFFFFFFF))

// Resource types
// Minimum is 0x100 due to MS-Windows resource types
// (RSC_NOTYPE=0x100) is defined in resid.hxx
#define RSC_VERSIONCONTROL      (RSC_NOTYPE + 0x02) // Version control

#define RSC_RESOURCE            (RSC_NOTYPE + 0x10)
#define RSC_STRING              (RSC_NOTYPE + 0x11)
#define RSC_BITMAP              (RSC_NOTYPE + 0x13)
#define RSC_MENU                (RSC_NOTYPE + 0x1c)
#define RSC_MENUITEM            (RSC_NOTYPE + 0x1d) // only used internally

#define RSC_STRINGARRAY         (RSC_NOTYPE + 0x79)

// (RSC_NOTYPE + 0x200) - (RSC_NOTYPE + 0x300) reserved for Sfx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
