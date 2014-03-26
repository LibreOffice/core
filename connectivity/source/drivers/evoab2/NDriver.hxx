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

#ifndef _CONNECTIVITY_EVOAB_DRIVER_HXX_
#define _CONNECTIVITY_EVOAB_DRIVER_HXX_

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/compbase2.hxx>
#include "connectivity/CommonTools.hxx"
#include <osl/module.h>

#define EVOAB_EVOLUTION_SCHEMA  "evolution"
/*In Future, when separate schema is required for ldap, groupwise*/
#define EVOAB_LDAP_SCHEMA   "ldap"
#define EVOAB_GWISE_SCHEMA  "groupwise"

#define EVOAB_DRIVER_IMPL_NAME  "com.sun.star.comp.sdbc.evoab.OEvoabDriver"

namespace connectivity
{
    namespace evoab
    {

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OEvoabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );


        typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::sdbc::XDriver,
                                ::com::sun::star::lang::XServiceInfo > ODriver_BASE;


        class OEvoabDriver : public ODriver_BASE
        {

        protected:
            ::osl::Mutex                                        m_aMutex;
            connectivity::OWeakRefArray                         m_xConnections;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

        public:
            OEvoabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);
            virtual ~OEvoabDriver();

            // OComponentHelper
            virtual void SAL_CALL disposing(void) SAL_OVERRIDE;

            // XInterface
            static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        public:
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        & getMSFactory(void) const { return  m_xFactory; }
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        getComponentContext( ) const { return comphelper::getComponentContext( m_xFactory ); }

            // static methods
            static sal_Bool acceptsURL_Stat( const OUString& url );
        };
    }

}
#endif //_CONNECTIVITY_EVOAB_DRIVER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
