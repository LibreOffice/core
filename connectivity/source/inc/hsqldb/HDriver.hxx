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
#ifndef CONNECTIVITY_HSQLDB_DRIVER_HXX
#define CONNECTIVITY_HSQLDB_DRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/CommonTools.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    class OMetaConnection;

    namespace hsqldb
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ODriverDelegator_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        typedef ::cppu::WeakComponentImplHelper5<   ::com::sun::star::sdbc::XDriver
                                        ,::com::sun::star::sdbcx::XDataDefinitionSupplier
                                        , ::com::sun::star::lang::XServiceInfo
                                        , ::com::sun::star::sdbcx::XCreateCatalog
                                        , ::com::sun::star::embed::XTransactionListener
                                        >   ODriverDelegator_BASE;

        typedef ::std::pair< ::com::sun::star::uno::WeakReferenceHelper,::com::sun::star::uno::WeakReferenceHelper> TWeakRefPair;
        typedef ::std::pair< ::rtl::OUString ,TWeakRefPair > TWeakConnectionPair;

        typedef ::std::pair< ::com::sun::star::uno::WeakReferenceHelper,TWeakConnectionPair> TWeakPair;
        typedef ::std::vector< TWeakPair > TWeakPairVector;


        /** delegates all calls to the orignal driver and extend the existing one with the SDBCX layer.

        */
        class ODriverDelegator : public ::comphelper::OBaseMutex
                                ,public ODriverDelegator_BASE
        {
            TWeakPairVector                                                     m_aConnections; //  vector containing a list
                                                                                                //  of all the Connection objects
                                                                                                //  for this Driver
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >                 m_xDriver;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xFactory;
            sal_Bool                                                                            m_bInShutDownConnections;

            /** load the driver we want to delegate.
                The <member>m_xDriver</member> may be <NULL/> if the driver could not be loaded.
                @return
                    The driver which was currently selected.
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > loadDriver( );

            /** shut down the connection and revoke the storage from the map
                @param  _aIter
                    The connection to shut down and storage to revoke.
            */
            void shutdownConnection(const TWeakPairVector::iterator& _aIter);

        public:
            /** creates a new delegator for a HSQLDB driver
            */
            ODriverDelegator(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException);

            // XDataDefinitionSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByURL( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // XCreateCatalog
            virtual void SAL_CALL createCatalog( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

            // XTransactionListener
            virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

            void shutdownConnections();
        protected:
            /// dtor
            virtual ~ODriverDelegator();
            // OComponentHelper
            virtual void SAL_CALL disposing(void);

            /** called when we connected to a newly created embedded database
            */
            void onConnectedNewDatabase(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
            );
        };
    }

//........................................................................
}   // namespace connectivity
//........................................................................
#endif // CONNECTIVITY_HSQLDB_DRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
