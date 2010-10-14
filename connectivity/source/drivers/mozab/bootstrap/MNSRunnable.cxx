/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "MNSRunnable.hxx"
#include "MDatabaseMetaDataHelper.hxx"
#include "MQuery.hxx"
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

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
  m_oThreadID = osl_getThreadIdentifier(NULL);
#endif
  AddRef();
}

MNSRunnable::~MNSRunnable()
{
}
sal_Int32 MNSRunnable::StartProxy(const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode)
{
    OSL_TRACE( "IN : MNSRunnable::StartProxy() \n" );
    ::osl::MutexGuard aGuard(m_aThreadMutex);
    xCode = aCode;
    return ProxiedObject()->Run();
}


NS_IMETHODIMP MNSRunnable::Run()
{
    return static_cast<nsresult>(xCode->run());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
