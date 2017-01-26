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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KDRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KDRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/compbase.hxx>
#include <osl/module.h>

namespace connectivity
{
    namespace kab
    {
        class KabDriver;

        extern "C" {
        typedef css::uno::XInterface * (SAL_CALL * ConnectionFactoryFunction)(
            css::uno::Reference<css::uno::XComponentContext> const &,
            css::uno::Reference<css::sdbc::XDriver> const &);
        typedef void    (SAL_CALL * ApplicationInitFunction)( void );
        typedef void    (SAL_CALL * ApplicationShutdownFunction)( void );
        typedef int     (SAL_CALL * KDEVersionCheckFunction)( void );
        }

        typedef std::vector< css::uno::WeakReferenceHelper > OWeakRefArray;


        // = KabImplModule

        class KabImplModule
        {
        private:
            css::uno::Reference< css::uno::XComponentContext >
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
            explicit KabImplModule( const css::uno::Reference< css::uno::XComponentContext >& _rxContext );

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

                @throws css::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
                @throws css::sdbc::SQLException
                    if the KDE version we're running against is not supported, or no KDE was found at all
            */
            void init();

            /** shuts down the impl module (and the KDE application, if we own it)
            */
            void shutdown();

            /** creates a new connection
                @precond
                    <member>init</member> has been called before
                @throws css::uno::RuntimeException
                    if no connection object could be created (which is a severe error, normally impossible)
            */
            css::uno::Reference<css::sdbc::XConnection> createConnection(
                KabDriver * driver) const;

        private:
            /** loads the implementation module and retrieves the needed symbols

                Save against being called multiple times.

                @return <TRUE/> if the module could be loaded successfully.

                @throws css::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
            */
            bool    impl_loadModule();

            /** unloads the implementation module, and resets all function pointers to <nullptr/>
                @precond m_hConnectorModule is not <nullptr/>
            */
            void    impl_unloadModule();

            /** determines whether it's allowed to run on a too-new (not confirmed to work) version
            */
            bool    impl_doAllowNewKDEVersion();
        };


        // = KabDriver

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XDriver,
                                                   css::lang::XServiceInfo,
                                                   css::frame::XTerminateListener > KDriver_BASE;
        class KabDriver : public KDriver_BASE
        {
        protected:
            ::osl::Mutex                m_aMutex;           // mutex is need to control member access
            OWeakRefArray               m_xConnections;     // vector containing a list of all the
                                                            //  KabConnection objects for this Driver
            css::uno::Reference< css::uno::XComponentContext >
                                        m_xContext;       // the multi-service factory
            KabImplModule               m_aImplModule;

        public:
            static css::uno::Reference< css::uno::XInterface > SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory);

            // XServiceInfo - static versions
            static OUString getImplementationName_Static(  );
            static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  );

            /** returns the path of our configuration settings
            */
            static OUString  impl_getConfigurationSettingsPath();

        protected:
            explicit KabDriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

            // XDriver
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) override;
            virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
            virtual sal_Int32 SAL_CALL getMajorVersion() override;
            virtual sal_Int32 SAL_CALL getMinorVersion() override;

            // XTerminateListener
            virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) override;
            virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        private:
            /** shuts down the library which contains the real implementations

                This method is safe against being called multiple times

                @precond our mutex is locked
            */
            void impl_shutdownImplementationModule();
        };
    }

}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KDRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
