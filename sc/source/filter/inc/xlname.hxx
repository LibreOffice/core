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

// Constants and Enumerations =================================================

// (0x0018, 0x0218) NAME ------------------------------------------------------

const sal_uInt16 EXC_ID_NAME                = 0x0018;
const sal_uInt16 EXC_ID34_NAME              = 0x0218;

// flags
const sal_uInt16 EXC_NAME_DEFAULT           = 0x0000;
const sal_uInt16 EXC_NAME_HIDDEN            = 0x0001;
const sal_uInt16 EXC_NAME_FUNC              = 0x0002;
const sal_uInt16 EXC_NAME_VB                = 0x0004;
const sal_uInt16 EXC_NAME_PROC              = 0x0008;
const sal_uInt16 EXC_NAME_CALCEXP           = 0x0010;
const sal_uInt16 EXC_NAME_BUILTIN           = 0x0020;
const sal_uInt16 EXC_NAME_FGROUPMASK        = 0x0FC0;
const sal_uInt16 EXC_NAME_BIG               = 0x1000;

const sal_uInt8 EXC_NAME2_FUNC              = 0x02;     /// BIFF2 function/command flag.

const sal_uInt16 EXC_NAME_GLOBAL            = 0;        /// 0 = Globally defined name.

// codes for built-in names
const sal_Unicode EXC_BUILTIN_CONSOLIDATEAREA   = '\x00';
const sal_Unicode EXC_BUILTIN_AUTOOPEN          = '\x01';
const sal_Unicode EXC_BUILTIN_AUTOCLOSE         = '\x02';
const sal_Unicode EXC_BUILTIN_EXTRACT           = '\x03';
const sal_Unicode EXC_BUILTIN_DATABASE          = '\x04';
const sal_Unicode EXC_BUILTIN_CRITERIA          = '\x05';
const sal_Unicode EXC_BUILTIN_PRINTAREA         = '\x06';
const sal_Unicode EXC_BUILTIN_PRINTTITLES       = '\x07';
const sal_Unicode EXC_BUILTIN_RECORDER          = '\x08';
const sal_Unicode EXC_BUILTIN_DATAFORM          = '\x09';
const sal_Unicode EXC_BUILTIN_AUTOACTIVATE      = '\x0A';
const sal_Unicode EXC_BUILTIN_AUTODEACTIVATE    = '\x0B';
const sal_Unicode EXC_BUILTIN_SHEETTITLE        = '\x0C';
const sal_Unicode EXC_BUILTIN_FILTERDATABASE    = '\x0D';
const sal_Unicode EXC_BUILTIN_UNKNOWN           = '\x0E';

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
