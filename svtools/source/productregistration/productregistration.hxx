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

#ifndef SVTOOLS_PRODUCTREGISTRATION_HXX
#define SVTOOLS_PRODUCTREGISTRATION_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <cppuhelper/implbase3.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= OProductRegistration
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::task::XJobExecutor
                                    ,   ::com::sun::star::task::XJob
                                    >   OProductRegistration_Base;

    class OProductRegistration : public OProductRegistration_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;

    protected:
        OProductRegistration( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

    public:
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        // XServiceInfo - static version
        static ::rtl::OUString SAL_CALL getImplementationName_Static( );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XJobExecutor
        virtual void SAL_CALL trigger( const ::rtl::OUString& sEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XJob
        virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        // types of events which can be handled by this component
        enum EventType
        {
            etRegistrationRequired,

            etUnknown
        };

        // classifies a event
        EventType classify( const ::rtl::OUString& _rEventDesc );

        // do the online registration
        void doOnlineRegistration( );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_PRODUCTREGISTRATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
