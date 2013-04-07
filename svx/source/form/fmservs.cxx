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
                        OUString(), ImplName##_NewInstance_Impl,             \
                        ::com::sun::star::uno::Sequence< OUString>(&sString, 1));    \
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
    OUString SAL_CALL ClassName##_GetImplementationName();                                           \
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL ClassName##_GetSupportedServiceNames();     \


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

        ::com::sun::star::uno::Sequence< OUString> aServices;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >  xSingleFactory;

        OUString sString;

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SERVICE( FormController, OUString( "com.sun.star.form.runtime.FormController" ) );
        REGISTER_SERVICE( LegacyFormController, OUString( "com.sun.star.form.FormController" ) );

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
