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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_DRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_DRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include "java/lang/Object.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/logging.hxx>

namespace connectivity
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL java_sql_Driver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

    class java_sql_Driver : public ::cppu::WeakImplHelper< ::com::sun::star::sdbc::XDriver,::com::sun::star::lang::XServiceInfo>
    {
        css::uno::Reference<css::uno::XComponentContext> m_aContext;
        ::comphelper::ResourceBasedEventLogger  m_aLogger;

    protected:
        virtual ~java_sql_Driver();

    public:
        java_sql_Driver(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

        static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XDriver
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        const css::uno::Reference<css::uno::XComponentContext>& getContext() const { return m_aContext; }
        const ::comphelper::ResourceBasedEventLogger&   getLogger() const { return m_aLogger; }
    };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_DRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
