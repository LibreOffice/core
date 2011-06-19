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
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sdbc.dbase.ODriver"));
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
    return !url.compareTo(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:dbase:")),11);
}
// -----------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< ::rtl::OUString > aBoolean(2);
        aBoolean[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
        aBoolean[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1"));

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet of the database."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowDeleted"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Display inactive records."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EnableSQL92Check"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Use SQL92 naming constraints."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
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
