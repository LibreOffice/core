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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLCOLLECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLCOLLECTION_HXX

#include <sal/config.h>

#include <map>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/XDriverManager2.hpp>
#include <com/sun/star/sdbc/XConnectionPool.hpp>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <comphelper/stl_types.hxx>
#include <osl/mutex.hxx>

namespace connectivity
{
    class OConnectionPool;

    // OPoolCollection - the one-instance service for PooledConnections
    // manages the active connections and the connections in the pool

    typedef ::cppu::WeakImplHelper<    ::com::sun::star::sdbc::XConnectionPool,
                                       ::com::sun::star::lang::XServiceInfo,
                                       ::com::sun::star::frame::XTerminateListener,
                                       ::com::sun::star::beans::XPropertyChangeListener
                                       >   OPoolCollection_Base;

    /// OPoolCollection: control the whole connection pooling for oo
    class OPoolCollection : public OPoolCollection_Base
    {


        typedef ::comphelper::OInterfaceCompare< ::com::sun::star::sdbc::XDriver >  ODriverCompare;
        typedef std::map<OUString, OConnectionPool*> OConnectionPools;

        typedef std::map<
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >,
                ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDriver >,
                ODriverCompare>
                MapDriver2DriverRef;

        MapDriver2DriverRef                                                                 m_aDriverProxies;
        ::osl::Mutex                                                                        m_aMutex;
        OConnectionPools                                                                    m_aPools;          // the driver pools
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriverManager2 >         m_xManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >     m_xProxyFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xConfigNode;      // config node for general connection pooling
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop2>               m_xDesktop;

    private:
        OPoolCollection(const OPoolCollection&) = delete;
        int operator= (const OPoolCollection&) = delete;

        explicit OPoolCollection(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

        // some configuration helper methods
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createWithServiceFactory(const OUString& _rPath) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getConfigPoolRoot();
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createWithProvider(   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxConfProvider,
                                                                                                    const OUString& _rPath);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > openNode( const OUString& _rPath,
                                                                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xTreeNode) throw();
        bool isPoolingEnabled();
        bool isDriverPoolingEnabled(const OUString& _sDriverImplName,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDriverNode);
        bool isPoolingEnabledByUrl( const OUString& _sUrl,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _rxDriver,
                                        OUString& _rsImplName,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDriverNode);

        OConnectionPool* getConnectionPool( const OUString& _sImplName,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _xDriver,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDriverNode);
        void clearConnectionPools(bool _bDispose);
        void clearDesktop();
    protected:
        virtual ~OPoolCollection();
    public:

        static ::com::sun::star::uno::Any getNodeValue( const OUString& _rPath,
                                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xTreeNode)throw();

    // XDriverManager
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XDriverAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > SAL_CALL getDriverByURL( const OUString& url ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo - static methods
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL CreateInstance(const::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        static OUString SAL_CALL getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
