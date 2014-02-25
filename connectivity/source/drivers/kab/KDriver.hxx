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

#ifndef _CONNECTIVITY_KAB_DRIVER_HXX_
#define _CONNECTIVITY_KAB_DRIVER_HXX_

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/compbase3.hxx>
#include <osl/module.h>

namespace connectivity
{
    namespace kab
    {
        class KabConnection;
        class KabDriver;

        typedef void*   (SAL_CALL * ConnectionFactoryFunction)( void* _pDriver );
        typedef void    (SAL_CALL * ApplicationInitFunction)( void );
        typedef void    (SAL_CALL * ApplicationShutdownFunction)( void );
        typedef int     (SAL_CALL * KDEVersionCheckFunction)( void );

        typedef std::vector< ::com::sun::star::uno::WeakReferenceHelper > OWeakRefArray;

        // ===============================================================
        // = KabImplModule
        // ===============================================================
        class KabImplModule
        {
        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                        m_xContext;

            /// Did we already attempt to load the module and to retrieve the symbols?
            bool    m_bAttemptedLoadModule;
            /// Did we already check the KDE version and initialize the impl module (or at least attempted to)?
            bool    m_bAttemptedInitialize;

            oslModule                   m_hConnectorModule;
            ConnectionFactoryFunction   m_pConnectionFactoryFunc;
            ApplicationInitFunction     m_pApplicationInitFunc;
            ApplicationShutdownFunction m_pApplicationShutdownFunc;
            KDEVersionCheckFunction     m_pKDEVersionCheckFunc;

        public:
            KabImplModule( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

            /** determines whether there is a KDE present in the environment
            */
            bool isKDEPresent();

            enum KDEVersionType
            {
                eTooOld,
                eSupported,
                eToNew
            };
            /** checks whether the KDE version we're running against is supported
                @precond
                    the module is loaded, i.e impl_loadModule has successfully been called
            */
            KDEVersionType matchKDEVersion();

            /** initializes the implementation module.

                @raises ::com::sun::star::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
                @raises ::com::sun::star::sdbc::SQLException
                    if the KDE version we're running against is not supported, or no KDE was found at all
            */
            void init();

            /** shuts down the impl module (and the KDE application, if we own it)
            */
            void shutdown();

            /** creates a new connection
                @precond
                    <member>init</member> has been called before
                @raises ::com::sun::star::uno::RuntimeException
                    if no connection object could be created (which is a severe error, normally impossible)
            */
            KabConnection*  createConnection( KabDriver* _pDriver ) const;

        private:
            /** loads the implementation module and retrieves the needed symbols

                Save against being called multiple times.

                @return <TRUE/> if the module could be loaded successfully.

                @raises ::com::sun::star::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
            */
            bool    impl_loadModule();

            /** unloads the implementation module, and resets all function pointers to <NULL/>
                @precond m_hConnectorModule is not <NULL/>
            */
            void    impl_unloadModule();

            /** throws an SQLException saying than no KDE installation was found
            */
            void    impl_throwNoKdeException();

            /** throws an SQLException saying that the found KDE version is too old
            */
            void    impl_throwKdeTooOldException();

            /** throws an SQLException saying that the found KDE version is too new
            */
            void    impl_throwKdeTooNewException();

            /** throws a generic SQL exception with SQLState S1000 and error code 0
            */
            void    impl_throwGenericSQLException( const OUString& _rMessage );

            /** determines whether it's allowed to run on a too-new (not confirmed to work) version
            */
            bool    impl_doAllowNewKDEVersion();
        };

        // ===============================================================
        // = KabDriver
        // ===============================================================
        typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::sdbc::XDriver,
                                                    ::com::sun::star::lang::XServiceInfo,
                                                    ::com::sun::star::frame::XTerminateListener > KDriver_BASE;
        class KabDriver : public KDriver_BASE
        {
        protected:
            ::osl::Mutex                m_aMutex;           // mutex is need to control member access
            OWeakRefArray               m_xConnections;     // vector containing a list of all the
                                                            //  KabConnection objects for this Driver
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                        m_xContext;       // the multi-service factory
            KabImplModule               m_aImplModule;

        public:
            static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

            // XServiceInfo - static versions
            static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
            getComponentContext() const { return m_xContext; }

            /** returns the driver's implementation name (being pure ASCII) for reference in various places
            */
            static const sal_Char*  impl_getAsciiImplementationName();

            /** returns the path of our configuration settings
            */
            static OUString  impl_getConfigurationSettingsPath();

        protected:
            KabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

            // OComponentHelper
            virtual void SAL_CALL disposing(void);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception);
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
            virtual sal_Int32 SAL_CALL getMajorVersion() throw(::com::sun::star::uno::RuntimeException, std::exception);
            virtual sal_Int32 SAL_CALL getMinorVersion() throw(::com::sun::star::uno::RuntimeException, std::exception);

            // XTerminateListener
            virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException, std::exception);
            virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        private:
            /** shuts down the library which contains the real implementations

                This method is safe against being called multiple times

                @precond our mutex is locked
            */
            void impl_shutdownImplementationModule();
        };
    }

}

#endif // _CONNECTIVITY_KAB_DRIVER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
