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


#include "MyListener.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <cppuhelper/supportsservice.hxx>

MyListener::MyListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : m_xSMGR(xSMGR)
{}

MyListener::~MyListener()
{}

css::uno::Any SAL_CALL MyListener::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
    throw (css::lang::IllegalArgumentException,
           css::uno::Exception,
           css::uno::RuntimeException)
{
    css::uno::Sequence< css::beans::NamedValue > lEnv;

    sal_Int32                     i = 0;
    sal_Int32                     c = lArguments.getLength();
    const css::beans::NamedValue* p = lArguments.getConstArray();
    for (i=0; i<c; ++i)
    {
        if ( p[i].Name == "Environment" )
        {
            p[i].Value >>= lEnv;
            break;
        }
    }

    css::uno::Reference< css::frame::XModel > xModel;

    c = lEnv.getLength();
    p = lEnv.getConstArray();
    for (i=0; i<c; ++i)
    {
        if ( p[i].Name == "Model" )
        {
            p[i].Value >>= xModel;
            break;
        }
        if ( p[i].Name == "Frame" )
        {
            css::uno::Reference< css::frame::XController > xController;
            css::uno::Reference< css::frame::XFrame >      xFrame;
            p[i].Value >>= xFrame;
            if (xFrame.is())
                xController = xFrame->getController();
            if (xController.is())
                xModel = xController->getModel();
            break;
        }
    }

    if (!xModel.is())
        return css::uno::Any();

    css::uno::Reference< css::lang::XServiceInfo > xInfo(xModel, css::uno::UNO_QUERY);
    sal_Bool bCalc   = xInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument");
    sal_Bool bWriter = ( xInfo->supportsService("com.sun.star.text.TextDocument") &&
                        !xInfo->supportsService("com.sun.star.text.WebDocument") &&
                        !xInfo->supportsService("com.sun.star.text.GlobalDocument") );

    // We are interested only in Writer and Calc. However, here we are
    // notified of all newly opened Documents...
    if (!bCalc && !bWriter)
        return css::uno::Any();

    void* pListener = 0;
    if (bCalc)
        pListener = (void*)(new CalcListener(m_xSMGR));
    else
    if (bWriter)
        pListener = (void*)(new WriterListener(m_xSMGR));

    css::uno::Reference< css::document::XEventListener >    xDocListener     (static_cast< css::document::XEventListener* >(pListener), css::uno::UNO_QUERY);
    css::uno::Reference< css::document::XEventBroadcaster > xDocBroadcaster  (xModel   , css::uno::UNO_QUERY);

    xDocBroadcaster->addEventListener(xDocListener);

    return css::uno::Any();
}

::rtl::OUString SAL_CALL MyListener::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ::rtl::OUString( MYLISTENER_IMPLEMENTATIONNAME );
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL MyListener::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > lNames(1);
    lNames[0] = ::rtl::OUString( MYLISTENER_SERVICENAME );
    return lNames;
}

sal_Bool SAL_CALL MyListener::supportsService(const ::rtl::OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Reference< css::uno::XInterface > MyListener::st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    MyListener* pListener = new MyListener(xSMGR);
    css::uno::Reference< css::uno::XInterface > xListener(static_cast< css::task::XJob* >(pListener), css::uno::UNO_QUERY);
    return xListener;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
