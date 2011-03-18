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
#include "ZPooledConnection.hxx"
#include "ZConnectionWrapper.hxx"
#include "connectivity/ConnectionWrapper.hxx"
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <comphelper/types.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/compbase1.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace connectivity;
using namespace ::osl;

OPooledConnection::OPooledConnection(const Reference< XConnection >& _xConnection,
                                    const Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory)
    : OPooledConnection_Base(m_aMutex)
    ,m_xRealConnection(_xConnection)
    ,m_xProxyFactory(_rxProxyFactory)
{

}
// -----------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL OPooledConnection::disposing(void)
{
    MutexGuard aGuard(m_aMutex);
    if (m_xComponent.is())
        m_xComponent->removeEventListener(this);
m_xComponent.clear();
    ::comphelper::disposeComponent(m_xRealConnection);
}
// -----------------------------------------------------------------------------
// XEventListener
void SAL_CALL OPooledConnection::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
{
m_xComponent.clear();
}
// -----------------------------------------------------------------------------
//XPooledConnection
Reference< XConnection > OPooledConnection::getConnection()  throw(SQLException, RuntimeException)
{
    if(!m_xComponent.is() && m_xRealConnection.is())
    {
        Reference< XAggregation > xConProxy = m_xProxyFactory->createProxy(m_xRealConnection.get());
        m_xComponent = new OConnectionWeakWrapper(xConProxy);
        // register as event listener for the new connection
        if (m_xComponent.is())
            m_xComponent->addEventListener(this);
    }
    return Reference< XConnection >(m_xComponent,UNO_QUERY);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
