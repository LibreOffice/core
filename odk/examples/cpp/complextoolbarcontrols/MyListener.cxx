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


#include "MyListener.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>

namespace css = ::com::sun::star;

/*-----------------------------------------------------
    20.11.2003 11:31
-----------------------------------------------------*/
MyListener::MyListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : m_xSMGR(xSMGR)
{}

/*-----------------------------------------------------
    20.11.2003 11:32
-----------------------------------------------------*/
MyListener::~MyListener()
{}

/*-----------------------------------------------------
    20.11.2003 12:04
-----------------------------------------------------*/
css::uno::Any SAL_CALL MyListener::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
    throw (css::lang::IllegalArgumentException,
           css::uno::Exception,
           css::uno::RuntimeException)
{
    css::uno::Sequence< css::beans::NamedValue > lEnv;

          sal_Int32               i = 0;
          sal_Int32               c = lArguments.getLength();
    const css::beans::NamedValue* p = lArguments.getConstArray();
    for (i=0; i<c; ++i)
    {
        if (p[i].Name.equalsAscii("Environment"))
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
        if (p[i].Name.equalsAscii("Model"))
        {
            p[i].Value >>= xModel;
            break;
        }
        if (p[i].Name.equalsAscii("Frame"))
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
    sal_Bool bCalc   = xInfo->supportsService(::rtl::OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument"));
    sal_Bool bWriter = (
                         xInfo->supportsService(::rtl::OUString::createFromAscii("com.sun.star.text.TextDocument"  )) &&
                        !xInfo->supportsService(::rtl::OUString::createFromAscii("com.sun.star.text.WebDocument"   )) &&
                        !xInfo->supportsService(::rtl::OUString::createFromAscii("com.sun.star.text.GlobalDocument"))
                       );

    // Wir interessieren uns nur für Writer und Calc. Werden hier aber für
    // alle neu geöffneten Dokumente benachrichtigt ...
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

/*-----------------------------------------------------
    20.11.2003 12:13
-----------------------------------------------------*/
::rtl::OUString SAL_CALL MyListener::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii(MYLISTENER_IMPLEMENTATIONNAME);
}

/*-----------------------------------------------------
    20.11.2003 12:13
-----------------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > SAL_CALL MyListener::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > lNames(1);
    lNames[0] = ::rtl::OUString::createFromAscii(MYLISTENER_SERVICENAME);
    return lNames;
}

/*-----------------------------------------------------
    20.11.2003 12:14
-----------------------------------------------------*/
sal_Bool SAL_CALL MyListener::supportsService(const ::rtl::OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    return (
            sServiceName.equalsAscii(MYLISTENER_SERVICENAME) ||
            sServiceName.equalsAscii("com.sun.star.task.Job"    )
           );
}

/*-----------------------------------------------------
    20.11.2003 11:31
-----------------------------------------------------*/
css::uno::Reference< css::uno::XInterface > MyListener::st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    MyListener* pListener = new MyListener(xSMGR);
    css::uno::Reference< css::uno::XInterface > xListener(static_cast< css::task::XJob* >(pListener), css::uno::UNO_QUERY);
    return xListener;
}
