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

// extern "C" void** __cdecl __current_exception()
// is defined in MSVS14.0/VC/crt/src/vcruntime/frame.cpp:
// return &__vcrt_getptd()->_curexception;
//
// __vcrt_getptd is defined in vcruntime_internal.h:
//typedef struct __vcrt_ptd
//{
//    // C++ Exception Handling (EH) state
//    unsigned long      _NLG_dwCode;      // Required by NLG routines
//[...]
//void*              _curexception;    // current exception
//[...]
extern "C" void** __current_exception();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
