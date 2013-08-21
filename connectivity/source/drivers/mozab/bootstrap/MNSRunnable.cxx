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

#include "MNSRunnable.hxx"
#include "mozillasrc/MDatabaseMetaDataHelper.hxx"
#include "mozillasrc/MQuery.hxx"
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>

#include "pre_include_mozilla.h"
#include <nsIProxyObjectManager.h>
// More Mozilla includes for LDAP Connection Test
#include "prprf.h"
#include "nsILDAPURL.h"
#include "nsILDAPMessage.h"
#include "nsILDAPMessageListener.h"
#include "nsILDAPErrors.h"
#include "nsILDAPConnection.h"
#include "nsILDAPOperation.h"
#include "post_include_mozilla.h"

using namespace connectivity::mozab;

/* Implementation file */
NS_IMPL_ISUPPORTS1(MNSRunnable, nsIRunnable)

static ::osl::Mutex m_aThreadMutex;

nsIRunnable * MNSRunnable::ProxiedObject()
{
    if (!_ProxiedObject)
    {

        nsresult rv = NS_GetProxyForObject(NS_UI_THREAD_EVENTQ,
                NS_GET_IID(nsIRunnable),
                this,
                PROXY_SYNC,
                (void**)&_ProxiedObject);
        (void)rv;

        _ProxiedObject->AddRef();
    }
    return _ProxiedObject;
}

MNSRunnable::MNSRunnable()
{
  NS_INIT_ISUPPORTS();
  _ProxiedObject=NULL;
#if OSL_DEBUG_LEVEL > 0
  m_oThreadID = osl::Thread::getCurrentIdentifier();
#endif
  AddRef();
}

MNSRunnable::~MNSRunnable()
{
}
sal_Int32 MNSRunnable::StartProxy(const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode)
{
    OSL_TRACE( "IN : MNSRunnable::StartProxy()" );
    ::osl::MutexGuard aGuard(m_aThreadMutex);
    xCode = aCode;
    return ProxiedObject()->Run();
}


NS_IMETHODIMP MNSRunnable::Run()
{
    return static_cast<nsresult>(xCode->run());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
