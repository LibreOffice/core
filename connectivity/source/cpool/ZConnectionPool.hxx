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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZCONNECTIONPOOL_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZCONNECTIONPOOL_HXX

#include <sal/config.h>

#include <map>
#include <vector>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <salhelper/timer.hxx>
#include <rtl/ref.hxx>
#include <rtl/digest.h>

namespace connectivity
{
    class OConnectionPool;

    /// OPoolTimer - Invalidates the connection pool

    class OPoolTimer : public ::salhelper::Timer
    {
        OConnectionPool* m_pPool;
    public:
        OPoolTimer(OConnectionPool* _pPool,const ::salhelper::TTimeValue& _Time)
            : ::salhelper::Timer(_Time)
            ,m_pPool(_pPool)
        {}
    protected:
        virtual void SAL_CALL onShot() override;
    };


    // OConnectionPool - the one-instance service for PooledConnections
    // manages the active connections and the connections in the pool

    typedef ::cppu::WeakImplHelper< ::com::sun::star::beans::XPropertyChangeListener>  OConnectionPool_Base;

    // typedef for the internal structure
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPooledConnection> > TPooledConnections;

     // contains the currently pooled connections
    typedef struct
    {
        TPooledConnections  aConnections;
        sal_Int32           nALiveCount; // will be decremented every time a time says to, when will reach zero the pool will be deleted
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
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> createNewConnection(const OUString& _rURL,
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
        void clear(bool _bDispose);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        void invalidatePooledConnections();
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZCONNECTIONPOOL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
