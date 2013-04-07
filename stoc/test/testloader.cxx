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


#include <stdio.h>

#include <sal/main.h>
#include <osl/module.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/factory.hxx>

#if defined ( UNX )
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::lang;
using namespace osl;
using namespace cppu;


class EmptyComponentContext : public WeakImplHelper1< XComponentContext >
{
public:
    virtual Any SAL_CALL getValueByName( const OUString& /*Name*/ )
        throw (RuntimeException)
        {
            return Any();
        }
    virtual Reference< XMultiComponentFactory > SAL_CALL getServiceManager(  )
        throw (RuntimeException)
        {
            return Reference< XMultiComponentFactory > ();
        }

};


SAL_IMPLEMENT_MAIN()
{
    Reference<XInterface> xIFace;

    Module module;

    OUString dllName(
        "bootstrap.uno" SAL_DLLEXTENSION );

    if (module.load(dllName))
    {
        // try to get provider from module
        component_getFactoryFunc pCompFactoryFunc = (component_getFactoryFunc)
            module.getFunctionSymbol( OUString(COMPONENT_GETFACTORY) );

        if (pCompFactoryFunc)
        {
            XSingleServiceFactory * pRet = (XSingleServiceFactory *)(*pCompFactoryFunc)(
                "com.sun.star.comp.stoc.DLLComponentLoader", 0, 0 );
            if (pRet)
            {
                xIFace = pRet;
                pRet->release();
            }
        }
    }

    OSL_ENSURE( xIFace.is(), "testloader error1");

    Reference<XSingleComponentFactory> xFactory( Reference<XSingleComponentFactory>::query(xIFace) );

    OSL_ENSURE( xFactory.is(), "testloader error2");

    Reference<XInterface> xLoader = xFactory->createInstanceWithContext( new EmptyComponentContext );

    OSL_ENSURE( xLoader.is(), "testloader error3");

    Reference<XServiceInfo> xServInfo( Reference<XServiceInfo>::query(xLoader) );

    OSL_ENSURE( xServInfo.is(), "testloader error4");

    OSL_ENSURE( xServInfo->getImplementationName() == "com.sun.star.comp.stoc.DLLComponentLoader", "testloader error5");
    OSL_ENSURE( xServInfo->supportsService(OUString( "com.sun.star.loader.SharedLibrary") ), "testloader error6");
    OSL_ENSURE( xServInfo->getSupportedServiceNames().getLength() == 1, "testloader error7");

    xIFace.clear();
    xFactory.clear();
    xLoader.clear();
    xServInfo.clear();

    printf("Test Dll ComponentLoader, OK!\n");

    return(0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
