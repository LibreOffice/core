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

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "smplmailsuppl.hxx"
#include "smplmailclient.hxx"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::system::XSimpleMailClientSupplier;
using com::sun::star::system::XSimpleMailClient;

using namespace cppu;

CSmplMailSuppl::CSmplMailSuppl() :
    WeakComponentImplHelper<XSimpleMailClientSupplier, XServiceInfo>(m_aMutex)
{
}

CSmplMailSuppl::~CSmplMailSuppl()
{
}

Reference<XSimpleMailClient> SAL_CALL CSmplMailSuppl::querySimpleMailClient()
{
    /* We just try to load the MAPI dll as a test
       if a mail client is available */
    Reference<XSimpleMailClient> xSmplMailClient;
    HMODULE handle = LoadLibraryW(L"mapi32.dll");
    if ((handle != INVALID_HANDLE_VALUE) && (handle != nullptr))
    {
        FreeLibrary(handle);
        xSmplMailClient.set(new CSmplMailClient);
    }
    return xSmplMailClient;
}

// XServiceInfo

OUString SAL_CALL CSmplMailSuppl::getImplementationName()
{
    return "com.sun.star.sys.shell.SimpleSystemMail";
}

sal_Bool SAL_CALL CSmplMailSuppl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL CSmplMailSuppl::getSupportedServiceNames()
{
    return { "com.sun.star.system.SimpleSystemMail" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_CSmplMailSuppl_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new CSmplMailSuppl()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
