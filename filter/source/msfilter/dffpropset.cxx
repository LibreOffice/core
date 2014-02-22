/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <filter/msfilter/dffpropset.hxx>
#include <rtl/ustrbuf.hxx>

static const DffPropSetEntry mso_PropSetDefaults[] = {


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0010 },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0039 },      


{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0xffffff },    
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x001c },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x001e },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0001 },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0016 },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0 },           


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0010 },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ {  true, false, false,  true }, 0, 0x0001 },      


{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
{ { false, false, false, false }, 0, 0 },
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

                nMergeFlags &=  ( nMergeFlags >> 16 ) | 0xffff0000;             
                nMergeFlags &= ( ( nCurrentFlags & 0xffff0000 )                 
                                | ( nCurrentFlags >> 16 ) ) ^ 0xffffffff;       
                nCurrentFlags &= ( ( nMergeFlags & 0xffff0000 )                 
                                | ( nMergeFlags >> 16 ) ) ^ 0xffffffff;
                nCurrentFlags |= (sal_uInt16)nMergeFlags;                       
                mpPropSetEntries[ nRecType ].nContent = nCurrentFlags;
                mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr |= static_cast< sal_uInt16 >( nContent >> 16 );
            }
            else
            {
                
                mpPropSetEntries[ nRecType ].nContent &= ( ( nContent >> 16 ) ^ 0xffffffff );
                
                mpPropSetEntries[ nRecType ].nContent |= nContent;
                mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr = static_cast< sal_uInt16 >( nContent >> 16 );
            }
        }
        else
        {
            bool bSetProperty = !bSetUninitializedOnly || ( !IsProperty( nRecType ) || !IsHardAttribute( nRecType ) );

            DffPropFlags aPropFlag = { 1, 0, 0, 0 };
            if ( nTmp & 0x4000 )
                aPropFlag.bBlip = sal_True;
            if ( nTmp & 0x8000 )
                aPropFlag.bComplex = sal_True;
            if ( aPropFlag.bComplex && nContent && ( nComplexDataFilePos < aHd.GetRecEndFilePos() ) )
            {
                
                

                
                if ( ( nRecType == DFF_Prop_pVertices ) || ( nRecType == DFF_Prop_pSegmentInfo )
                    || ( nRecType == DFF_Prop_fillShadeColors ) || ( nRecType == DFF_Prop_lineDashStyle )
                        || ( nRecType == DFF_Prop_pWrapPolygonVertices ) || ( nRecType == DFF_Prop_connectorPoints )
                            || ( nRecType == DFF_Prop_Handles ) || ( nRecType == DFF_Prop_pFormulas )
                                || ( nRecType == DFF_Prop_textRectangles ) )
                {
                    
                    sal_uInt32  nOldPos = rIn.Tell();
                    sal_Int16   nNumElem, nNumElemReserved, nSize;

                    rIn.Seek( nComplexDataFilePos );
                    rIn. ReadInt16( nNumElem ).ReadInt16( nNumElemReserved ).ReadInt16( nSize );
                    if ( nNumElemReserved >= nNumElem )
                    {
                        
                        
                        
                        
                        if ( nSize < 0 )
                            nSize = ( -nSize ) >> 2;
                        sal_uInt32 nDataSize = (sal_uInt32)( nSize * nNumElem );

                        
                        if ( nDataSize == nContent )
                            nContent += 6;

                        
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
                        maOffsets.push_back( nComplexDataFilePos );     
                    }
                    nComplexDataFilePos += nContent;                    
                }
                else                                                    
                    aPropFlag.bSet = sal_False;                         
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

sal_Bool DffPropSet::IsHardAttribute( sal_uInt32 nId ) const
{
    sal_Bool bRetValue = sal_True;
    nId &= 0x3ff;
    if ( ( nId & 0x3f ) >= 48 ) 
        bRetValue = (mpPropSetEntries[nId | 0x3f].nComplexIndexOrFlagsHAttr
                        & (1 << (0xf - (nId & 0xf)))) != 0;
    else
        bRetValue = ( mpPropSetEntries[ nId ].aFlags.bSoftAttr == 0 );
    return bRetValue;
};

sal_uInt32 DffPropSet::GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault ) const
{
    nId &= 0x3ff;
    return ( mpPropSetEntries[ nId ].aFlags.bSet ) ? mpPropSetEntries[ nId ].nContent : nDefault;
};

bool DffPropSet::GetPropertyBool( sal_uInt32 nId, bool bDefault ) const
{
    sal_uInt32 nBaseId = nId | 31;              
    sal_uInt32 nMask = 1 << (nBaseId - nId);    

    sal_uInt32 nPropValue = GetPropertyValue( nBaseId, bDefault ? nMask : 0 );
    return (nPropValue & nMask) != 0;
}

OUString DffPropSet::GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const
{
    sal_Size nOldPos = rStrm.Tell();
    OUStringBuffer aBuffer;
    sal_uInt32 nBufferSize = GetPropertyValue( nId );
    if( (nBufferSize > 0) && SeekToContent( nId, rStrm ) )
    {
        sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufferSize / 2 );
        
        
        
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

sal_Bool DffPropSet::SeekToContent( sal_uInt32 nRecType, SvStream& rStrm ) const
{
    nRecType &= 0x3ff;
    if ( mpPropSetEntries[ nRecType ].aFlags.bSet )
    {
        if ( mpPropSetEntries[ nRecType ].aFlags.bComplex )
        {
            sal_uInt16 nIndex = mpPropSetEntries[ nRecType ].nComplexIndexOrFlagsHAttr;
            if ( nIndex < maOffsets.size() )
            {
                rStrm.Seek( maOffsets[ nIndex ] );
                return sal_True;
            }
        }
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
