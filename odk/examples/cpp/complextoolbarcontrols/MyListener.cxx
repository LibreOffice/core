/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#include "MyListener.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>

namespace css = ::com::sun::star;


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
        if (p[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Environment" ) ) )
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
        if (p[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Model" ) ) )
        {
            p[i].Value >>= xModel;
            break;
        }
        if (p[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Frame" ) ) )
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
    sal_Bool bCalc   = xInfo->supportsService(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocument" ) ) );
    sal_Bool bWriter = (
                         xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextDocument" ) ) ) &&
                        !xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.WebDocument" ) ) ) &&
                        !xInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.GlobalDocument" ) ) )
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


::rtl::OUString SAL_CALL MyListener::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MYLISTENER_IMPLEMENTATIONNAME ) );
}


css::uno::Sequence< ::rtl::OUString > SAL_CALL MyListener::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > lNames(1);
    lNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MYLISTENER_SERVICENAME ) );
    return lNames;
}


sal_Bool SAL_CALL MyListener::supportsService(const ::rtl::OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    return (
            sServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MYLISTENER_SERVICENAME )) ||
            sServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.task.Job" ) )
           );
}


css::uno::Reference< css::uno::XInterface > MyListener::st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    MyListener* pListener = new MyListener(xSMGR);
    css::uno::Reference< css::uno::XInterface > xListener(static_cast< css::task::XJob* >(pListener), css::uno::UNO_QUERY);
    return xListener;
}
