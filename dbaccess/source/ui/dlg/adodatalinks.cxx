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


#if defined(_WIN32)
// LO/windows.h conflict
#undef WB_LEFT
#undef WB_RIGHT
#include <msdasc.h>

#include <comphelper/scopeguard.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <systools/win32/comtools.hxx>
#include <systools/win32/oleauto.hxx>

#include <initguid.h>
#include <adoid.h>
#include <adoint.h>

#include "adodatalinks.hxx"

namespace {

OUString PromptNew(sal_IntPtr hWnd)
{
    try
    {
        // Initialize COM
        sal::systools::CoInitializeGuard aGuard(COINIT_APARTMENTTHREADED);

        // Instantiate DataLinks object.
        sal::systools::COMReference<IDataSourceLocator> dlPrompt;
        dlPrompt.CoCreateInstance(CLSID_DataLinks,       //clsid -- Data Links UI
                                  nullptr,               //pUnkOuter
                                  CLSCTX_INPROC_SERVER); //dwClsContext

        sal::systools::ThrowIfFailed(dlPrompt->put_hWnd(hWnd), "put_hWnd failed");

        // Prompt for connection information.
        sal::systools::COMReference<IDispatch> piDispatch;
        sal::systools::ThrowIfFailed(dlPrompt->PromptNew(&piDispatch), "PromptNew failed");
        sal::systools::COMReference<ADOConnection> piTmpConnection(piDispatch,
                                                                   sal::systools::COM_QUERY_THROW);

        sal::systools::BStr _result;
        sal::systools::ThrowIfFailed(piTmpConnection->get_ConnectionString(&_result),
                                     "get_ConnectionString failed");

        return OUString(_result);
    }
    catch (const sal::systools::ComError&)
    {
        return OUString();
    }
}

OUString PromptEdit(sal_IntPtr hWnd, OUString const & connstr)
{
    try
    {
        // Initialize COM
        sal::systools::CoInitializeGuard aGuard(COINIT_APARTMENTTHREADED);

        sal::systools::COMReference<ADOConnection> piTmpConnection;
        piTmpConnection.CoCreateInstance(CLSID_CADOConnection, nullptr, CLSCTX_INPROC_SERVER);

        sal::systools::ThrowIfFailed(
            piTmpConnection->put_ConnectionString(sal::systools::BStr(connstr)),
            "put_ConnectionString failed");

        // Instantiate DataLinks object.
        sal::systools::COMReference<IDataSourceLocator> dlPrompt;
        dlPrompt.CoCreateInstance(CLSID_DataLinks,       //clsid -- Data Links UI
                                  nullptr,               //pUnkOuter
                                  CLSCTX_INPROC_SERVER); //dwClsContext

        sal::systools::ThrowIfFailed(dlPrompt->put_hWnd(hWnd), "put_hWnd failed");

        try
        {
            // Prompt for connection information.
            IDispatch* piDispatch = piTmpConnection.get();
            VARIANT_BOOL pbSuccess;
            sal::systools::ThrowIfFailed(dlPrompt->PromptEdit(&piDispatch, &pbSuccess),
                                         "PromptEdit failed");
            if (!pbSuccess) //if user press cancel then sal_False == pbSuccess
                return connstr;
        }
        catch (const sal::systools::ComError&)
        {
            // Prompt for new connection information.
            sal::systools::COMReference<IDispatch> piDispatch;
            sal::systools::ThrowIfFailed(dlPrompt->PromptNew(&piDispatch), "PromptNew failed");
            piTmpConnection.set(piDispatch, sal::systools::COM_QUERY_THROW);
        }

        sal::systools::BStr _result;
        sal::systools::ThrowIfFailed(piTmpConnection->get_ConnectionString(&_result),
                                     "get_ConnectionString failed");

        return OUString(_result);
    }
    catch (const sal::systools::ComError&)
    {
        return connstr;
    }
}

}

OUString getAdoDatalink(sal_IntPtr hWnd,OUString const & oldLink)
{
    OUString dataLink;
    if (!oldLink.isEmpty())
    {
        dataLink=PromptEdit(hWnd,oldLink);
    }
    else
        dataLink=PromptNew(hWnd);
    return dataLink;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
