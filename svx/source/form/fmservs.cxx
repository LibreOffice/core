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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <com/sun/star/container/XSet.hpp>
#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include "fmservs.hxx"

// ------------------------------------------------------------------------
#define DECL_SERVICE(ImplName)                      \
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_NewInstance_Impl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &) throw( ::com::sun::star::uno::Exception );

#define REGISTER_SERVICE(ImplName, ServiceName)                         \
    sString = (ServiceName);                                        \
    xSingleFactory = ::cppu::createSingleFactory(xServiceFactory,               \
                        ::rtl::OUString(), ImplName##_NewInstance_Impl,             \
                        ::com::sun::star::uno::Sequence< ::rtl::OUString>(&sString, 1));    \
    if (xSingleFactory.is())                                            \
        xSet->insert(::com::sun::star::uno::makeAny(xSingleFactory));


    DECL_SERVICE( FmXGridControl )
    DECL_SERVICE( FormController )
    DECL_SERVICE( LegacyFormController )


// ------------------------------------------------------------------------
namespace svxform
{

#define DECL_SELFAWARE_SERVICE( ClassName )                     \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClassName##_Create(      \
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );  \
    ::rtl::OUString SAL_CALL ClassName##_GetImplementationName();                                           \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ClassName##_GetSupportedServiceNames();     \


#define REGISTER_SELFAWARE_SERVICE( ClassName )                     \
    xSingleFactory = ::cppu::createSingleFactory( xServiceFactory,  \
                        ClassName##_GetImplementationName(),        \
                        ClassName##_Create,                         \
                        ClassName##_GetSupportedServiceNames()      \
                     );                                             \
    if ( xSingleFactory.is() )                                      \
        xSet->insert( ::com::sun::star::uno::makeAny( xSingleFactory ) );


    // ------------------------------------------------------------------------
    DECL_SELFAWARE_SERVICE( OAddConditionDialog )

    // ------------------------------------------------------------------------
    void ImplSmartRegisterUnoServices()
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xServiceFactory(::comphelper::getProcessServiceFactory(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XSet >  xSet(xServiceFactory, ::com::sun::star::uno::UNO_QUERY);
        if (!xSet.is())
            return;

        ::com::sun::star::uno::Sequence< ::rtl::OUString> aServices;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >  xSingleFactory;

        ::rtl::OUString sString;

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SERVICE( FormController, FM_FORM_CONTROLLER );
        REGISTER_SERVICE( LegacyFormController, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.FormController" ) ) );

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SELFAWARE_SERVICE( OAddConditionDialog );

        // ------------------------------------------------------------------------
        // DBGridControl
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRID);  // compatibility
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRIDCONTROL);
        REGISTER_SERVICE(FmXGridControl, FM_SUN_CONTROL_GRIDCONTROL);
    };

}   // namespace svxform

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
