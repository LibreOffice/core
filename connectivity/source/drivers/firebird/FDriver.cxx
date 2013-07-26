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

#include "FDriver.hxx"
#include "FConnection.hxx"
#include "connectivity/dbexception.hxx"
#include "resource/common_res.hrc"
#include "resource/hsqldb_res.hrc"
#include "resource/sharedresources.hxx"

#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

using namespace ::osl;

using namespace connectivity::firebird;

namespace connectivity
{
    namespace firebird
    {
        Reference< XInterface >  SAL_CALL FirebirdDriver_CreateInstance(
            const Reference< XMultiServiceFactory >& _rxFactory) throw( Exception )
        {
            SAL_INFO("connectivity.firebird", "FirebirdDriver_CreateInstance()" );
            (void) _rxFactory;
            return *(new FirebirdDriver());
        }
    }
}

FirebirdDriver::FirebirdDriver()
    : ODriver_BASE(m_aMutex)
{
}

void FirebirdDriver::disposing()
{
    MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

//----- static ServiceInfo ---------------------------------------------------
rtl::OUString FirebirdDriver::getImplementationName_Static() throw(RuntimeException)
{
    return rtl::OUString("com.sun.star.comp.sdbc.firebird.Driver");
}

Sequence< OUString > FirebirdDriver::getSupportedServiceNames_Static() throw (RuntimeException)
{
    // TODO: add com.sun.star.sdbcx.Driver once all sdbc functionality is implemented
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = OUString("com.sun.star.sdbc.Driver");
    return aSNS;
}

OUString SAL_CALL FirebirdDriver::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL FirebirdDriver::supportsService(const OUString& _rServiceName)
    throw(RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

Sequence< OUString > SAL_CALL FirebirdDriver::getSupportedServiceNames()
    throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// ----  XDriver -------------------------------------------------------------
Reference< XConnection > SAL_CALL FirebirdDriver::connect(
    const OUString& url, const Sequence< PropertyValue >& info )
    throw(SQLException, RuntimeException)
{
    Reference< XConnection > xConnection;

    SAL_INFO("connectivity.firebird", "connect(), URL: " << url );

    MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
       throw DisposedException();

    if ( ! acceptsURL(url) )
        return NULL;

    // create a new connection with the given properties and append it to our vector
    OConnection* pCon = new OConnection(this);
    Reference< XConnection > xCon = pCon;   // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
    pCon->construct(url,info); // late constructor call which can throw exception and allows a correct dtor call when so
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL FirebirdDriver::acceptsURL( const OUString& url )
    throw(SQLException, RuntimeException)
{
    return url.equals("sdbc:embedded:firebird") || url.startsWith("sdbc:firebird:");
}

Sequence< DriverPropertyInfo > SAL_CALL FirebirdDriver::getPropertyInfo(
    const OUString& url, const Sequence< PropertyValue >& info )
    throw(SQLException, RuntimeException)
{
    (void) info;
    if ( ! acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }

    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL FirebirdDriver::getMajorVersion(  ) throw(RuntimeException)
{
    // The major and minor version are sdbc driver specific. Must begin with 1.0
    // as per http://api.libreoffice.org/docs/common/ref/com/sun/star/sdbc/XDriver.html
    return 1;
}

sal_Int32 SAL_CALL FirebirdDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}


namespace connectivity
{
    namespace firebird
    {

        void release(oslInterlockedCount& _refCount, ::cppu::OBroadcastHelper& rBHelper,
                     Reference< XInterface >& _xInterface, XComponent* _pObject)
        {
            if (osl_atomic_decrement( &_refCount ) == 0)
            {
                osl_atomic_increment( &_refCount );

                if (!rBHelper.bDisposed && !rBHelper.bInDispose)
                {
                    // remember the parent
                    Reference< XInterface > xParent;
                    {
                        MutexGuard aGuard( rBHelper.rMutex );
                        xParent = _xInterface;
                        _xInterface = NULL;
                    }

                    // First dispose
                    _pObject->dispose();

                    // only the alive ref holds the object
                    OSL_ASSERT( _refCount == 1 );

                    // release the parent in the ~
                    if (xParent.is())
                    {
                        MutexGuard aGuard( rBHelper.rMutex );
                        _xInterface = xParent;
                    }
                }
            }
            else
                osl_atomic_increment( &_refCount );
        }

        void checkDisposed(sal_Bool _bThrow) throw ( DisposedException )
        {
            if (_bThrow)
                throw DisposedException();

        }

    }
} // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
