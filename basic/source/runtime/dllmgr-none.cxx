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

#include "sal/config.h"

#if defined(WNT)
#include <windows.h>
#undef GetObject
#endif

#include <algorithm>
#include <cstddef>
#include <list>
#include <map>
#include <vector>

#include "basic/sbx.hxx"
#include "basic/sbxvar.hxx"
#include "osl/thread.h"
#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "dllmgr.hxx"

struct SbiDllMgr::Impl {};

SbError SbiDllMgr::Call(
    OUString const &, OUString const &, SbxArray *, SbxVariable &,
    bool)
{
    return ERRCODE_BASIC_NOT_IMPLEMENTED;
}

void SbiDllMgr::FreeDll(OUString const &) {}

SbiDllMgr::SbiDllMgr(): impl_(new Impl) {}

SbiDllMgr::~SbiDllMgr() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
