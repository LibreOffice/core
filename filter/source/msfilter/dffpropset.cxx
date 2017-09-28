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

#include <algorithm>
#include <filter/msfilter/dffpropset.hxx>
#include <filter/msfilter/dffrecordheader.hxx>
#include <svx/msdffdef.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/stream.hxx>

static const DffPropSetEntry mso_PropSetDefaults[] = {

// 0
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },

// 64
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           // DFF_Prop_LockAgainstGrouping

// 128
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0010 },      // DFF_Prop_FitTextToShape

// 192
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           // DFF_Prop_gtextFStrikethrough

//256
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           // DFF_Prop_pictureActive

// 320
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0039 },      // DFF_Prop_fFillOK

// 384
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0xffffff },    // DFF_Prop_fillColor
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x001c },      // DFF_Prop_fNoFillHitTest

// 448
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x001e },      // DFF_Prop_fNoLineDrawDash

// 512
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           // DFF_Prop_fshadowObscured

// 576
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           // DFF_Prop_fPerspective

// 640
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0001 },      // DFF_Prop_fc3DLightFace

// 704
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0016 },      // DFF_Prop_fc3DFillHarsh

// 768
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           // DFF_Prop_fBackground

// 832
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0010 },      // DFF_Prop_fCalloutLengthSpecified

// 896
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0001 },      // DFF_Prop_fPrint

// 960
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 }

};

DffPropSet::DffPropSet()
{
    mpPropSetEntries = reinterpret_cast< DffPropSetEntry* >( new sal_uInt8[ 1024 * sizeof( DffPropSetEntry ) ] );
}

DffPropSet::~DffPropSet()
{
    delete[] reinterpret_cast< sal_uInt8* >( mpPropSetEntries );
}

void DffPropSet::ReadPropSet( SvStream& rIn, bool bSetUninitializedOnly )
{
    DffRecordHeader aHd;
    ReadDffRecordHeader( rIn, aHd );

    if ( !bSetUninitializedOnly )
    {
        InitializePropSet( aHd.nRecType );
        maOffsets.clear();
    }

    sal_uInt32 nPropCount = aHd.nRecInstance;

    sal_uInt32 nComplexDataFilePos = rIn.Tell() + ( nPropCount * 6 );

    for( sal_uInt32 nPropNum = 0; nPropNum < nPropCount; nPropNum++ )
    {
        sal_uInt16 nTmp;
        sal_uInt32 nRecType, nContent;
        rIn.ReadUInt16( nTmp )
           .ReadUInt32( nContent );

        nRecType = nTmp & 0x3fff;

        if ( nRecType > 0x3ff )
            break;
        if ( ( nRecType & 0x3f ) == 0x3f )
        {
            if ( bSetUninitializedOnly )
            {
                sal_uInt32 nCurrentFlags = mpPropSetEntries[ nRecType ].nContent;
                sal_uInt32 nMergeFlags = nContent;

                nMergeFlags &=  ( nMergeFlags >> 16 ) | 0xffff0000;             // clearing low word
                nMergeFlags &= ( ( nCurrentFlags & 0xffff0000 )                 // remove already hard set
                                | ( nCurrentFlags >> 16 ) ) ^ 0xffffffff;       // attributes from mergeflags
                nCurrentFlags &= ( ( nMergeFlags & 0xffff0000 )                 // apply zero master bits
                                | ( nMergeFlags >> 16 ) ) ^ 0xffffffff;
                nCurrentFlags |= (sal_uInt16)nMergeFlags;                       // apply filled master bits
                mpPropSetEntries[ nRecType ].nContent = nCurrentFlags;
                mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr |= static_cast< sal_uInt16 >( nContent >> 16 );
            }
            else
            {
                // clear flags that have to be cleared
                mpPropSetEntries[ nRecType ].nContent &= ( ( nContent >> 16 ) ^ 0xffffffff );
                // set flags that have to be set
                mpPropSetEntries[ nRecType ].nContent |= nContent;
                mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr = static_cast< sal_uInt16 >( nContent >> 16 );
            }
        }
        else
        {
            bool bSetProperty = !bSetUninitializedOnly || ( !IsProperty( nRecType ) || !IsHardAttribute( nRecType ) );

            DffPropFlags aPropFlag = { true, false, false, false };
            if ( nTmp & 0x4000 )
                aPropFlag.bBlip = true;
            if ( nTmp & 0x8000 )
                aPropFlag.bComplex = true;
            if ( aPropFlag.bComplex && nContent && ( nComplexDataFilePos < aHd.GetRecEndFilePos() ) )
            {
                // normally nContent is the complete size of the complex property,
                // but this is not always true for IMsoArrays ( what the hell is a IMsoArray ? )

                // I love special treatments :-(
                if ( ( nRecType == DFF_Prop_pVertices ) || ( nRecType == DFF_Prop_pSegmentInfo )
                    || ( nRecType == DFF_Prop_fillShadeColors ) || ( nRecType == DFF_Prop_lineDashStyle )
                        || ( nRecType == DFF_Prop_pWrapPolygonVertices ) || ( nRecType == DFF_Prop_connectorPoints )
                            || ( nRecType == DFF_Prop_Handles ) || ( nRecType == DFF_Prop_pFormulas )
                                || ( nRecType == DFF_Prop_textRectangles ) )
                {
                    // now check if the current content size is possible, or 6 bytes too small
                    sal_uInt32  nOldPos = rIn.Tell();
                    sal_Int16   nNumElem, nNumElemReserved, nSize;

                    rIn.Seek( nComplexDataFilePos );
                    rIn. ReadInt16( nNumElem ).ReadInt16( nNumElemReserved ).ReadInt16( nSize );
                    if ( nNumElemReserved >= nNumElem )
                    {
                        // the size of these array elements is nowhere defined,
                        // what if the size is negative ?
                        // ok, we will make it positive and shift it.
                        // for -16 this works
                        if ( nSize < 0 )
                            nSize = ( -nSize ) >> 2;
                        sal_uInt32 nDataSize = (sal_uInt32)( nSize * nNumElem );

                        // sometimes the content size is 6 bytes too small (array header information is missing )
                        if ( nDataSize == nContent )
                            nContent += 6;

                        // check if array fits into the PropertyContainer
                        if ( ( nComplexDataFilePos + nContent ) > aHd.GetRecEndFilePos() )
                            nContent = 0;
                    }
                    else
                        nContent = 0;
                    rIn.Seek( nOldPos );
                }
                if ( nContent )
                {
                    if ( bSetProperty )
                    {
                        mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr = static_cast< sal_uInt16 >( maOffsets.size() );
                        maOffsets.push_back( nComplexDataFilePos );     // insert the filepos of this property;
                    }
                    nComplexDataFilePos += nContent;                    // store filepos, that is used for the next complex property
                }
                else                                                    // a complex property needs content
                    aPropFlag.bSet = false;                             // otherwise something is wrong
            }
            if ( bSetProperty )
            {
                mpPropSetEntries[ nRecType ].nContent = nContent;
                mpPropSetEntries[ nRecType ].aFlags = aPropFlag;
            }
        }
    }
    aHd.SeekToEndOfRecord( rIn );
}

SvStream& ReadDffPropSet( SvStream& rIn, DffPropSet& rRec )
{
    rRec.ReadPropSet( rIn, false );
    return rIn;
}

SvStream& operator|=( SvStream& rIn, DffPropSet& rRec )
{
    rRec.ReadPropSet( rIn, true );
    return rIn;
}

void DffPropSet::InitializePropSet( sal_uInt16 nPropSetType ) const
{
    /*
    cmc:
    " Boolean properties are grouped in bitfields by property set; note that
    the Boolean properties in each property set are contiguous. They are saved
    under the property ID of the last Boolean property in the set, and are
    placed in the value field in reverse order starting with the last property
    in the low bit. "

    e.g.

    fEditedWrap
    fBehindDocument
    fOnDblClickNotify
    fIsButton
    fOneD
    fHidden
    fPrint

    are all part of a group and all are by default false except for fPrint,
    which equates to a default bit sequence for the group of 0000001 -> 0x1

    If at a later stage word sets fBehindDocument away from the default it
    will be done by having a property named fPrint whose bitsequence will have
    the fBehindDocument bit set. e.g. a DFF_Prop_fPrint with value 0x200020
    has set bit 6 on so as to enable fBehindDocument (as well as disabling
    everything else)
    */
    if ( nPropSetType == DFF_msofbtOPT )
    {
        memcpy( mpPropSetEntries, mso_PropSetDefaults, 0x400 * sizeof( DffPropSetEntry ) );
    }
    else
    {
        memset( mpPropSetEntries, 0, 0x400 * sizeof( DffPropSetEntry ) );
    }
}

bool DffPropSet::IsHardAttribute( sal_uInt32 nId ) const
{
    bool bRetValue = true;
    nId &= 0x3ff;
    if ( ( nId & 0x3f ) >= 48 ) // is this a flag id
        bRetValue = (mpPropSetEntries[nId | 0x3f].nComplexIndexOrFlagsHAttr
                        & (1 << (0xf - (nId & 0xf)))) != 0;
    else
        bRetValue = !mpPropSetEntries[ nId ].aFlags.bSoftAttr;
    return bRetValue;
};

sal_uInt32 DffPropSet::GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault ) const
{
    nId &= 0x3ff;
    return ( mpPropSetEntries[ nId ].aFlags.bSet ) ? mpPropSetEntries[ nId ].nContent : nDefault;
};

bool DffPropSet::GetPropertyBool( sal_uInt32 nId ) const
{
    sal_uInt32 nBaseId = nId | 31;              // base ID to get the sal_uInt32 property value
    sal_uInt32 nMask = 1 << (nBaseId - nId);    // bit mask of the boolean property

    sal_uInt32 nPropValue = GetPropertyValue( nBaseId, 0 );
    return (nPropValue & nMask) != 0;
}

OUString DffPropSet::GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const
{
    sal_uInt64 const nOldPos = rStrm.Tell();
    OUStringBuffer aBuffer;
    sal_uInt32 nBufferSize = GetPropertyValue( nId, 0 );
    if( (nBufferSize > 0) && SeekToContent( nId, rStrm ) )
    {
        sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufferSize / 2 );
        //clip initial size of buffer to something sane in case of silly length
        //strings. If there really is a silly amount of data available it still
        //works out ok of course
        aBuffer.ensureCapacity(std::min(nStrLen,static_cast<sal_Int32>(8192)));
        for( sal_Int32 nCharIdx = 0; nCharIdx < nStrLen; ++nCharIdx )
        {
            sal_uInt16 nChar = 0;
            rStrm.ReadUInt16( nChar );
            if( nChar > 0 )
                aBuffer.append( static_cast< sal_Unicode >( nChar ) );
            else
                break;
        }
    }
    rStrm.Seek( nOldPos );
    return aBuffer.makeStringAndClear();
}

bool DffPropSet::SeekToContent( sal_uInt32 nRecType, SvStream& rStrm ) const
{
    nRecType &= 0x3ff;
    if ( mpPropSetEntries[ nRecType ].aFlags.bSet )
    {
        if ( mpPropSetEntries[ nRecType ].aFlags.bComplex )
        {
            sal_uInt16 nIndex = mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr;
            if ( nIndex < maOffsets.size() )
            {
                return checkSeek(rStrm, maOffsets[nIndex]);
            }
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
