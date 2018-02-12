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

#include "ole2uno.hxx"

#include <osl/thread.hxx>
#include <sal/log.hxx>

using namespace std;

void o2u_attachCurrentThread()
{
    static osl::ThreadData oleThreadData;

    if (!bool(reinterpret_cast<sal_IntPtr>(oleThreadData.getData())))
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (!SUCCEEDED(hr))
        {   // FIXME: is it a problem that this ends up in STA currently?
            assert(RPC_E_CHANGED_MODE == hr);
            SAL_INFO("extensions.olebridge",
                    "CoInitializeEx fail: probably thread is in STA already?");
        }
        oleThreadData.setData(reinterpret_cast<void*>(true));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
