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
#ifndef _CONNECTIVITY_ZCONNECTIONPOOL_HXX_
#define _CONNECTIVITY_ZCONNECTIONPOOL_HXX_

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>
#include <osl/mutex.hxx>
#include <salhelper/timer.hxx>
#include <rtl/ref.hxx>
#include <rtl/digest.h>

namespace connectivity
{
    class OConnectionPool;
    //==========================================================================
    /// OPoolTimer - Invalidates the connection pool
    //==========================================================================
    class OPoolTimer : public ::salhelper::Timer
    {
        OConnectionPool* m_pPool;
    public:
        OPoolTimer(OConnectionPool* _pPool,const ::salhelper::TTimeValue& _Time)
            : ::salhelper::Timer(_Time)
            ,m_pPool(_pPool)
        {}
    protected:
        virtual void SAL_CALL onShot();
    };

    //==========================================================================
    //= OConnectionPool - the one-instance service for PooledConnections
    //= manages the active connections and the connections in the pool
    //==========================================================================
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener>  OConnectionPool_Base;

    // typedef for the interanl structure
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPooledConnection> > TPooledConnections;

     // contains the currently pooled connections
    typedef struct
    {
        TPooledConnections  aConnections;
        sal_Int32           nALiveCount; // will be decremented everytime a time says to, when will reach zero the pool will be deleted
    } TConnectionPool;

    struct TDigestHolder
    {
        sal_uInt8 m_pBuffer[RTL_DIGEST_LENGTH_SHA1];
        TDigestHolder()
        {
            m_pBuffer[0] = 0;
        }

    };

    //  typedef TDigestHolder

    struct TDigestLess : public ::std::binary_function< TDigestHolder, TDigestHolder, bool>
    {
        bool operator() (const TDigestHolder& x, const TDigestHolder& y) const
        {
            sal_uInt32 i;
            for(i=0;i < RTL_DIGEST_LENGTH_SHA1 && (x.m_pBuffer[i] >= y.m_pBuffer[i]); ++i)
                ;
            return i < RTL_DIGEST_LENGTH_SHA1;
        }
    };

    typedef ::std::map< TDigestHolder,TConnectionPool,TDigestLess> TConnectionMap;

    // contains additional information about a activeconnection which is needed to put it back to the pool
    typedef struct
    {
        TConnectionMap::iterator aPos;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPooledConnection> xPooledConnection;
    } TActiveConnectionInfo;

    typedef ::std::map< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>,
                        TActiveConnectionInfo> TActiveConnectionMap;

    class OConnectionPool : public OConnectionPool_Base
    {
        TConnectionMap          m_aPool;                // the pooled connections
        TActiveConnectionMap    m_aActiveConnections;   // the currently active connections

        ::osl::Mutex            m_aMutex;
        ::rtl::Reference<OPoolTimer>    m_xInvalidator;         // invalidates the conntection pool when shot

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >             m_xDriver;      // the one and only driver for this connectionpool
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >           m_xDriverNode;  // config node entry
        ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory > m_xProxyFactory;
        sal_Int32               m_nTimeOut;
        sal_Int32               m_nALiveCount;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> createNewConnection(const ::rtl::OUString& _rURL,
                                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getPooledConnection(TConnectionMap::iterator& _rIter);
        // calculate the timeout and the corresponding ALiveCount
        void calculateTimeOuts();

    protected:
        // the dtor will be called from the last instance  (last release call)
        virtual ~OConnectionPool();
    public:
        OConnectionPool(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _xDriver,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xDriverNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory);

        // delete all refs
        void clear(sal_Bool _bDispose);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        void invalidatePooledConnections();
    };
}
#endif // _CONNECTIVITY_ZCONNECTIONPOOL_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
