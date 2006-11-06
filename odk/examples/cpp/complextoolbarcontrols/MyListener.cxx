/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MyListener.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 15:05:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _MyListener_HXX
#include "MyListener.h"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

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
