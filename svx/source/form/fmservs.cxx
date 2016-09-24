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

using namespace com::sun::star;

#define REGISTER_SERVICE(ImplName, ServiceName)                     \
    sString = (ServiceName);                                        \
    xSingleFactory = ::cppu::createSingleFactory(xServiceFactory,   \
                        OUString(), ImplName##_NewInstance_Impl,    \
                        uno::Sequence< OUString>(&sString, 1));     \
    if (xSingleFactory.is())                                        \
        xSet->insert(uno::makeAny(xSingleFactory));

namespace svxform
{
    void ImplSmartRegisterUnoServices()
    {
        uno::Reference< lang::XMultiServiceFactory >  xServiceFactory(::comphelper::getProcessServiceFactory(), uno::UNO_QUERY);
        uno::Reference< container::XSet >  xSet(xServiceFactory, uno::UNO_QUERY);
        if (!xSet.is())
            return;

        uno::Reference< lang::XSingleServiceFactory >  xSingleFactory;

        OUString sString;


        // FormController
        REGISTER_SERVICE( FormController, "com.sun.star.form.runtime.FormController" );
        REGISTER_SERVICE( LegacyFormController, "com.sun.star.form.FormController" );


        // FormController - register selfaware service
        xSingleFactory = ::cppu::createSingleFactory( xServiceFactory,
                             OAddConditionDialog_GetImplementationName(),
                             OAddConditionDialog_Create,
                             OAddConditionDialog_GetSupportedServiceNames()
                         );
        if ( xSingleFactory.is() )
            xSet->insert( uno::makeAny( xSingleFactory ) );


        // DBGridControl
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRID);  // compatibility
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRIDCONTROL);
        REGISTER_SERVICE(FmXGridControl, FM_SUN_CONTROL_GRIDCONTROL);
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
