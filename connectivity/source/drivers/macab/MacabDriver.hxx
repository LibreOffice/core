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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABDRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABDRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <osl/module.h>

// the address book driver's version
#define MACAB_DRIVER_VERSION        "0.1"
#define MACAB_DRIVER_VERSION_MAJOR  0
#define MACAB_DRIVER_VERSION_MINOR  1

namespace connectivity
{
    namespace macab
    {
        class MacabConnection;
        class MacabDriver;

        typedef void*   (SAL_CALL * ConnectionFactoryFunction)( void* _pDriver );

        typedef std::vector< css::uno::WeakReferenceHelper > OWeakRefArray;


        // = MacabImplModule

        class MacabImplModule
        {
        private:
            /// Did we already attempt to load the module and to retrieve the symbols?
            bool    m_bAttemptedLoadModule;
            oslModule                   m_hConnectorModule;
            ConnectionFactoryFunction   m_pConnectionFactoryFunc;

        public:
            MacabImplModule();

            /** determines whether there is a mac OS present in the environment
            */
            bool isMacOSPresent();

            /** initializes the implementation module.

                @throws css::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
                @throws css::sdbc::SQLException
                    if no Mac OS was found at all
            */
            void init();

            /** shuts down the impl module
            */
            void shutdown();

            /** creates a new connection
                @precond
                    <member>init</member> has been called before
                @throws css::uno::RuntimeException
                    if no connection object could be created (which is a severe error, normally impossible)
            */
            MacabConnection*  createConnection( MacabDriver* _pDriver ) const;

        private:
            /** loads the implementation module and retrieves the needed symbols

                Save against being called multiple times.

                @return <TRUE/> if the module could be loaded successfully.

                @throws css::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
            */
            bool    impl_loadModule();

            /** unloads the implementation module, and resets all function pointers to <NULL/>
                @precond m_hConnectorModule is not <NULL/>
            */
            void    impl_unloadModule();
        };


        // = MacabDriver

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XDriver,
                                                   css::lang::XServiceInfo,
                                                   css::frame::XTerminateListener > MacabDriver_BASE;
        class MacabDriver : public MacabDriver_BASE
        {
        protected:
            ::osl::Mutex                m_aMutex;           // mutex is need to control member access
            OWeakRefArray               m_xConnections;     // vector containing a list of all the
                                                            //  MacabConnection objects for this Driver
            css::uno::Reference< css::uno::XComponentContext >
                                        m_xContext;       // the multi-service factory
            MacabImplModule             m_aImplModule;

        public:
            /// @throws css::uno::Exception
            static css::uno::Reference< css::uno::XInterface > SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory);

            // XServiceInfo - static versions
            /// @throws css::uno::RuntimeException
            static OUString getImplementationName_Static(  );
            /// @throws css::uno::RuntimeException
            static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  );

            css::uno::Reference< css::uno::XComponentContext > const &
            getComponentContext() const { return m_xContext; }

            /** returns the path of our configuration settings
            */
            static OUString  impl_getConfigurationSettingsPath();

        protected:
            explicit MacabDriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

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

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABDRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
