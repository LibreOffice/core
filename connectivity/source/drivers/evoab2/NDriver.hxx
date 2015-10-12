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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NDRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NDRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/compbase.hxx>
#include <connectivity/CommonTools.hxx>
#include <osl/module.h>

#define EVOAB_DRIVER_IMPL_NAME  "com.sun.star.comp.sdbc.evoab.OEvoabDriver"

namespace connectivity
{
    namespace evoab
    {

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OEvoabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );


        typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::sdbc::XDriver,
                                ::com::sun::star::lang::XServiceInfo > ODriver_BASE;


        class OEvoabDriver : public ODriver_BASE
        {

        protected:
            ::osl::Mutex                                        m_aMutex;
            connectivity::OWeakRefArray                         m_xConnections;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

        public:
            explicit OEvoabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);
            virtual ~OEvoabDriver();

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XInterface
            static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;


            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        public:
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        & getMSFactory() const { return  m_xFactory; }
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        getComponentContext( ) const { return comphelper::getComponentContext( m_xFactory ); }

            // static methods
            static bool acceptsURL_Stat( const OUString& url );
        };
    }

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NDRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
