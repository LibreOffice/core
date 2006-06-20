/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSRunnable.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:48:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#include "MNSRunnable.hxx"
#endif

#ifndef _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_
#include "MDatabaseMetaDataHelper.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#include "MQuery.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

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

