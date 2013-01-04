/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#include "sysmailprov.hxx"
#include "sysmailclient.hxx"
#include "sysmapi.hxx"

using com::sun::star::system::XMailClient;
using rtl::OUString;

using namespace com::sun::star::uno;

#define COMP_SERVICE_NAME  "com.sun.star.system.SystemMailProvider"
#define COMP_IMPL_NAME     "com.sun.star.comp.system.win.SystemMailProvider"


namespace shell
{

WinSysMailProvider::WinSysMailProvider(
    const Reference< XComponentContext >& xContext )
    : WinSysMailProvider_Base( m_aMutex )
    , m_xContext( xContext )
{}

WinSysMailProvider::~WinSysMailProvider()
{
    m_xContext.clear();
}

Reference<XMailClient> SAL_CALL
WinSysMailProvider::queryMailClient()
throw (RuntimeException)
{
    /* We just try to load the MAPI dll as a test
       if a mail client is available */
    Reference<XMailClient> xMailClient;
    HMODULE handle = LoadLibrary("mapi32.dll");
    if ((handle != INVALID_HANDLE_VALUE) && (handle != NULL)) {

        FreeLibrary(handle);
        xMailClient = Reference<XMailClient>(
                new WinSysMailClient() );
    }

    return xMailClient;
}

OUString SAL_CALL
WinSysMailProvider::getImplementationName()
throw(RuntimeException)
{
    return getImplementationName_static();
}

sal_Bool SAL_CALL
WinSysMailProvider::supportsService(
    const OUString& ServiceName )
throw(RuntimeException)
{
    Sequence <OUString> SupportedServicesNames = getSupportedServiceNames_static();

    for (sal_Int32 n = SupportedServicesNames.getLength(); n--;)
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

Sequence< OUString > SAL_CALL
WinSysMailProvider::getSupportedServiceNames()
throw( RuntimeException )
{
    return getSupportedServiceNames_static();
}

Reference< XInterface >
WinSysMailProvider::Create(
    const Reference< XComponentContext > &xContext)
{
    return Reference< XInterface >(
        static_cast< cppu::OWeakObject *>(
            new WinSysMailProvider( xContext ) ) );
}

OUString
WinSysMailProvider::getImplementationName_static()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( COMP_IMPL_NAME ) );
}

Sequence< OUString >
WinSysMailProvider::getSupportedServiceNames_static()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( COMP_SERVICE_NAME ) );
    return aRet;
}

}
