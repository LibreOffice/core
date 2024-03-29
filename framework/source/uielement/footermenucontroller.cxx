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

#include <uielement/footermenucontroller.hxx>

#include <services.h>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <cppuhelper/supportsservice.hxx>

//  Defines

using namespace com::sun::star::uno;

namespace framework
{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL FooterMenuController::getImplementationName()
{
    return "com.sun.star.comp.framework.FooterMenuController";
}

sal_Bool SAL_CALL FooterMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FooterMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

FooterMenuController::FooterMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    HeaderMenuController( xContext,true )
{
}

FooterMenuController::~FooterMenuController()
{
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_FooterMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::FooterMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
