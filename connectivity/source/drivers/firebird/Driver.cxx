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

#include "Connection.hxx"
#include "Driver.hxx"

#include <connectivity/dbexception.hxx>
#include <resource/common_res.hrc>
#include <resource/hsqldb_res.hrc>
#include <resource/sharedresources.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <svtools/miscopt.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

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
            return *(new FirebirdDriver(comphelper::getComponentContext(_rxFactory)));
        }
    }
}

// Static const member variables
const OUString FirebirdDriver::our_sFirebirdTmpVar("FIREBIRD_TMP");
const OUString FirebirdDriver::our_sFirebirdLockVar("FIREBIRD_LOCK");
const OUString FirebirdDriver::our_sFirebirdMsgVar("FIREBIRD_MSG");

FirebirdDriver::FirebirdDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext)
    : ODriver_BASE(m_aMutex)
    , m_aContext(_rxContext)
    , m_firebirdTMPDirectory(NULL, true)
    , m_firebirdLockDirectory(NULL, true)
{
    m_firebirdTMPDirectory.EnableKillingFile();
    m_firebirdLockDirectory.EnableKillingFile();

    // ::utl::TempFile uses a unique temporary directory (subdirectory of
    // /tmp or other user specific tmp directory) per instance in which
    // we can create directories for firebird at will.

    // Overrides firebird's default of /tmp or c:\temp
    osl_setEnvironment(our_sFirebirdTmpVar.pData, m_firebirdTMPDirectory.GetFileName().pData);

    // Overrides firebird's default of /tmp/firebird or c:\temp\firebird
    osl_setEnvironment(our_sFirebirdLockVar.pData, m_firebirdLockDirectory.GetFileName().pData);

#ifndef SYSTEM_FIREBIRD
    // Overrides firebird's hardcoded default of /usr/local/firebird on *nix,
    // however on Windows it seems to use the current directory as a default.
    OUString sMsgURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/firebird");
    ::rtl::Bootstrap::expandMacros(sMsgURL);
    OUString sMsgPath;
    ::osl::FileBase::getSystemPathFromFileURL(sMsgURL, sMsgPath);
    osl_setEnvironment(our_sFirebirdMsgVar.pData, sMsgPath.pData);
#endif
}

void FirebirdDriver::disposing()
{
    MutexGuard aGuard(m_aMutex);

    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    osl_clearEnvironment(our_sFirebirdTmpVar.pData);
    osl_clearEnvironment(our_sFirebirdLockVar.pData);

#ifndef SYSTEM_FIREBIRD
    osl_clearEnvironment(our_sFirebirdMsgVar.pData);
#endif

    ODriver_BASE::disposing();
}

//----- static ServiceInfo ---------------------------------------------------
rtl::OUString FirebirdDriver::getImplementationName_Static() throw(RuntimeException)
{
    return rtl::OUString("com.sun.star.comp.sdbc.firebird.Driver");
}

Sequence< OUString > FirebirdDriver::getSupportedServiceNames_Static() throw (RuntimeException)
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = "com.sun.star.sdbc.Driver";
    aSNS[0] = "com.sun.star.sdbcx.Driver";
    return aSNS;
}

OUString SAL_CALL FirebirdDriver::getImplementationName() throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL FirebirdDriver::supportsService(const OUString& _rServiceName)
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL FirebirdDriver::getSupportedServiceNames()
    throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

// ----  XDriver -------------------------------------------------------------
Reference< XConnection > SAL_CALL FirebirdDriver::connect(
    const OUString& url, const Sequence< PropertyValue >& info )
    throw(SQLException, RuntimeException, std::exception)
{
    Reference< XConnection > xConnection;

    SAL_INFO("connectivity.firebird", "connect(), URL: " << url );

    MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
       throw DisposedException();

    if ( ! acceptsURL(url) )
        return NULL;

    Connection* pCon = new Connection(this);
    Reference< XConnection > xCon = pCon;
    pCon->construct(url, info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL FirebirdDriver::acceptsURL( const OUString& url )
    throw(SQLException, RuntimeException, std::exception)
{
    SvtMiscOptions aMiscOptions;

    return aMiscOptions.IsExperimentalMode() &&
        (url.equals("sdbc:embedded:firebird") || url.startsWith("sdbc:firebird:"));
}

Sequence< DriverPropertyInfo > SAL_CALL FirebirdDriver::getPropertyInfo(
    const OUString& url, const Sequence< PropertyValue >& info )
    throw(SQLException, RuntimeException, std::exception)
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

sal_Int32 SAL_CALL FirebirdDriver::getMajorVersion(  ) throw(RuntimeException, std::exception)
{
    // The major and minor version are sdbc driver specific. Must begin with 1.0
    // as per http://api.libreoffice.org/docs/common/ref/com/sun/star/sdbc/XDriver.html
    return 1;
}

sal_Int32 SAL_CALL FirebirdDriver::getMinorVersion(  ) throw(RuntimeException, std::exception)
{
    return 0;
}

//----- XDataDefinitionSupplier
uno::Reference< XTablesSupplier > SAL_CALL FirebirdDriver::getDataDefinitionByConnection(
                                    const uno::Reference< XConnection >& rConnection)
    throw(SQLException, RuntimeException, std::exception)
{
    Connection* pConnection = static_cast< Connection* >(rConnection.get());
    return uno::Reference< XTablesSupplier >(pConnection->createCatalog(), UNO_QUERY);
}

uno::Reference< XTablesSupplier > SAL_CALL FirebirdDriver::getDataDefinitionByURL(
                    const OUString& rURL,
                    const uno::Sequence< PropertyValue >& rInfo)
    throw(SQLException, RuntimeException, std::exception)
{
    uno::Reference< XConnection > xConnection = connect(rURL, rInfo);
    return getDataDefinitionByConnection(xConnection);
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

        void checkDisposed(bool _bThrow) throw ( DisposedException )
        {
            if (_bThrow)
                throw DisposedException();

        }

    }
} // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
