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

#include "dbase/DDriver.hxx"
#include "dbase/DConnection.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/dbexception.hxx"
#include "resource/dbase_res.hrc"

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;


// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString("com.sun.star.comp.sdbc.dbase.ODriver");
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL ODriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::dbase::ODriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ODriver(_rxFactory));
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if ( ! acceptsURL(url) )
        return NULL;

    ODbaseConnection* pCon = new ODbaseConnection(this);
    pCon->construct(url,info);
    Reference< XConnection > xCon = pCon;
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODriver::acceptsURL( const ::rtl::OUString& url ) throw(SQLException, RuntimeException)
{
    return url.startsWith("sdbc:dbase:");
}
// -----------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< ::rtl::OUString > aBoolean(2);
        aBoolean[0] = ::rtl::OUString("0");
        aBoolean[1] = ::rtl::OUString("1");

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("CharSet")
                ,::rtl::OUString("CharSet of the database.")
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("ShowDeleted")
                ,::rtl::OUString("Display inactive records.")
                ,sal_False
                ,::rtl::OUString("0")
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("EnableSQL92Check")
                ,::rtl::OUString("Use SQL92 naming constraints.")
                ,sal_False
                ,::rtl::OUString("0")
                ,aBoolean)
                );
        return Sequence< DriverPropertyInfo >(&(aDriverInfo[0]),aDriverInfo.size());
    }

    SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    return Sequence< DriverPropertyInfo >();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
