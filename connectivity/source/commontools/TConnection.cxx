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
#include "TConnection.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/types.hxx>
#include <comphelper/officeresourcebundle.hxx>
#include <connectivity/dbexception.hxx>

using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::beans;
using namespace ::osl;

//------------------------------------------------------------------------------
OMetaConnection::OMetaConnection()
    : OMetaConnection_BASE(m_aMutex)
    , m_nTextEncoding(RTL_TEXTENCODING_MS_1252)
{
}
//------------------------------------------------------------------------------
void OMetaConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xMetaData = WeakReference< XDatabaseMetaData>();
    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        try
        {
            Reference< XInterface > xStatement( i->get() );
            ::comphelper::disposeComponent( xStatement );
        }
        catch (DisposedException)
        {
        }
    }
    m_aStatements.clear();
}
//XUnoTunnel
sal_Int64 SAL_CALL OMetaConnection::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw (::com::sun::star::uno::RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        ? reinterpret_cast< sal_Int64 >( this )
        : (sal_Int64)0;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > OMetaConnection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
::dbtools::OPropertyMap& OMetaConnection::getPropMap()
{
    static ::dbtools::OPropertyMap s_aPropertyNameMap;
    return s_aPropertyNameMap;
}
// -----------------------------------------------------------------------------
void OMetaConnection::throwGenericSQLException( sal_uInt16 _nErrorResourceId,const Reference< XInterface>& _xContext )
{
    ::rtl::OUString sErrorMessage;
    if ( _nErrorResourceId )
        sErrorMessage = m_aResources.getResourceString( _nErrorResourceId );
    Reference< XInterface> xContext = _xContext;
    if ( !xContext.is() )
        xContext = *this;
    ::dbtools::throwGenericSQLException( sErrorMessage, xContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
