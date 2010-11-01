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

#ifndef _CONNECTIVITY_MACAB_DRIVER_HXX_
#define _CONNECTIVITY_MACAB_DRIVER_HXX_

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
/** === end UNO includes === **/
#include <cppuhelper/compbase3.hxx>
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

        typedef std::vector< ::com::sun::star::uno::WeakReferenceHelper > OWeakRefArray;

        // ===============================================================
        // = MacabImplModule
        // ===============================================================
        class MacabImplModule
        {
        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                        m_xORB;

            /// Did we already attempt to load the module and to retrieve the symbols?
            bool    m_bAttemptedLoadModule;
            oslModule                   m_hConnectorModule;
            ConnectionFactoryFunction   m_pConnectionFactoryFunc;

        public:
            MacabImplModule( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

            /** determines whether there is a mac OS present in the environment
            */
            bool isMacOSPresent();

            /** initializes the implementation module.

                @raises ::com::sun::star::uno::RuntimeException
                    if the module could be loaded, but required symbols are missing
                @raises ::com::sun::star::sdbc::SQLException
                    if no Mac OS was found at all
            */
            void init();

            /** shuts down the impl module
            */
            void shutdown();

            /** creates a new connection
                @precond
                    <member>init</member> has been called before
                @raises ::com::sun::star::uno::RuntimeException
                    if no connection object could be created (which is a severe error, normally impossible)
            */
            MacabConnection*  createConnection( MacabDriver* _pDriver ) const;

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

            /** throws an SQLException saying than no Mac OS installation was found
            */
            void    impl_throwNoMacOSException();

            /** throws a generic SQL exception with SQLState S1000 and error code 0
            */
            void    impl_throwGenericSQLException( const ::rtl::OUString& _rMessage );

        };

        // ===============================================================
        // = MacabDriver
        // ===============================================================
        typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::sdbc::XDriver,
                                                    ::com::sun::star::lang::XServiceInfo,
                                                    ::com::sun::star::frame::XTerminateListener > MacabDriver_BASE;
        class MacabDriver : public MacabDriver_BASE
        {
        protected:
            ::osl::Mutex                m_aMutex;           // mutex is need to control member access
            OWeakRefArray               m_xConnections;     // vector containing a list of all the
                                                            //  MacabConnection objects for this Driver
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                        m_xMSFactory;       // the multi-service factory
            MacabImplModule               m_aImplModule;

        public:
            static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

            // XServiceInfo - static versions
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                    getMSFactory() const { return m_xMSFactory; }

            /** returns the driver's implementation name (being pure ASCII) for reference in various places
            */
            static const sal_Char*  impl_getAsciiImplementationName();

            /** returns the path of our configuration settings
            */
            static ::rtl::OUString  impl_getConfigurationSettingsPath();

        protected:
            MacabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // OComponentHelper
            virtual void SAL_CALL disposing(void);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion() throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion() throw(::com::sun::star::uno::RuntimeException);

            // XTerminateListener
            virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        private:
            /** shuts down the library which contains the real implementations

                This method is safe against being called multiple times

                @precond our mutex is locked
            */
            void impl_shutdownImplementationModule();
        };
    }

}

#endif // _CONNECTIVITY_MACAB_DRIVER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
