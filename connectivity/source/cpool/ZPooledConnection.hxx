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
#ifndef CONNECTIVITY_POOLEDCONNECTION_HXX
#define CONNECTIVITY_POOLEDCONNECTION_HXX

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/reflection/XProxyFactory.hpp>


namespace connectivity
{
    //==========================================================================
    //= OPooledConnection -
    //= allows to pool a real connection
    //==========================================================================
    typedef ::cppu::WeakComponentImplHelper2<    ::com::sun::star::sdbc::XPooledConnection
                                                ,::com::sun::star::lang::XEventListener>    OPooledConnection_Base;

    class OPooledConnection : public ::comphelper::OBaseMutex
                             ,public OPooledConnection_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xRealConnection;  // the connection fom driver
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >              m_xComponent;       // the connection which wraps the real connection
        ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >     m_xProxyFactory;
    public:
        // OComponentHelper
        virtual void SAL_CALL disposing(void);

        OPooledConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory);

        //XPooledConnection
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    };

}
#endif // CONNECTIVITY_POOLEDCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
