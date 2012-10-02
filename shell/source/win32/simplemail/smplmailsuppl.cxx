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

#include <osl/diagnose.h>
#include "smplmailsuppl.hxx"
#include "smplmailclient.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::system::XSimpleMailClientSupplier;
using com::sun::star::system::XSimpleMailClient;
using rtl::OUString;
using osl::Mutex;

using namespace cppu;

#define COMP_IMPL_NAME  "com.sun.star.sys.shell.SimpleSystemMail"

namespace // private
{
    Sequence< OUString > SAL_CALL Component_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString("com.sun.star.sys.shell.SimpleSystemMail");
        return aRet;
    }

} // end private namespace

CSmplMailSuppl::CSmplMailSuppl() :
    WeakComponentImplHelper2<XSimpleMailClientSupplier, XServiceInfo>(m_aMutex)
{
}

CSmplMailSuppl::~CSmplMailSuppl()
{
}

Reference<XSimpleMailClient> SAL_CALL CSmplMailSuppl::querySimpleMailClient()
    throw (RuntimeException)
{
    /* We just try to load the MAPI dll as a test
       if a mail client is available */
    Reference<XSimpleMailClient> xSmplMailClient;
    HMODULE handle = LoadLibrary("mapi32.dll");
    if ((handle != INVALID_HANDLE_VALUE) && (handle != NULL))
    {
        FreeLibrary(handle);
        xSmplMailClient = Reference<XSimpleMailClient>(new CSmplMailClient());
    }
    return xSmplMailClient;
}

// XServiceInfo

OUString SAL_CALL CSmplMailSuppl::getImplementationName()
    throw(RuntimeException)
{
    return OUString(COMP_IMPL_NAME);
}

sal_Bool SAL_CALL CSmplMailSuppl::supportsService(const OUString& ServiceName)
    throw(RuntimeException)
{
    Sequence <OUString> SupportedServicesNames = Component_getSupportedServiceNames();

    for (sal_Int32 n = SupportedServicesNames.getLength(); n--;)
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

Sequence<OUString> SAL_CALL CSmplMailSuppl::getSupportedServiceNames()
    throw(RuntimeException)
{
    return Component_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
