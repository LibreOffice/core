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

#include "ZPooledConnection.hxx"
#include "ZConnectionWrapper.hxx"
#include <connectivity/ConnectionWrapper.hxx>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <comphelper/types.hxx>
#include <cppuhelper/component.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace connectivity;
using namespace ::osl;

OPooledConnection::OPooledConnection(const Reference< XConnection >& _xConnection,
                                    const Reference< css::reflection::XProxyFactory >& _rxProxyFactory)
    : OPooledConnection_Base(m_aMutex)
    ,m_xRealConnection(_xConnection)
    ,m_xProxyFactory(_rxProxyFactory)
{

}

// OComponentHelper
void SAL_CALL OPooledConnection::disposing()
{
    MutexGuard aGuard(m_aMutex);
    if (m_xComponent.is())
        m_xComponent->removeEventListener(this);
    m_xComponent.clear();
    ::comphelper::disposeComponent(m_xRealConnection);
}

// XEventListener
void SAL_CALL OPooledConnection::disposing( const EventObject& /*Source*/ )
{
m_xComponent.clear();
}

//XPooledConnection
Reference< XConnection > OPooledConnection::getConnection()
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
