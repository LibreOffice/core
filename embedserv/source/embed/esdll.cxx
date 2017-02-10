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

#define _WIN32_DCOM
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "stdafx.h"

#include <atlbase.h>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4710 )
#endif
CComModule _Module;
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <atlcom.h>

BEGIN_OBJECT_MAP(ObjectMap)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
END_OBJECT_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

// DLL Entry Point


#include "syswinwrapper.hxx"
#include "docholder.hxx"


HINSTANCE DocumentHolder::m_hInstance;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    DocumentHolder::m_hInstance = hInstance;
    if (!winwrap::HatchWindowRegister(hInstance))
        return FALSE;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }
    return TRUE;    // ok
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
