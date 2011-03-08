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
#include "precompiled_stoc.hxx"

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

using ::rtl::OUString;

#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif

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
        RTL_CONSTASCII_USTRINGPARAM("bootstrap.uno" SAL_DLLEXTENSION) );

    if (module.load(dllName))
    {
        // try to get provider from module
        component_getFactoryFunc pCompFactoryFunc = (component_getFactoryFunc)
            module.getFunctionSymbol( OUString(RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY)) );

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

    TEST_ENSHURE( xIFace.is(), "testloader error1");

    Reference<XSingleComponentFactory> xFactory( Reference<XSingleComponentFactory>::query(xIFace) );

    TEST_ENSHURE( xFactory.is(), "testloader error2");

    Reference<XInterface> xLoader = xFactory->createInstanceWithContext( new EmptyComponentContext );

    TEST_ENSHURE( xLoader.is(), "testloader error3");

    Reference<XServiceInfo> xServInfo( Reference<XServiceInfo>::query(xLoader) );

    TEST_ENSHURE( xServInfo.is(), "testloader error4");

    TEST_ENSHURE( xServInfo->getImplementationName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.comp.stoc.DLLComponentLoader") ), "testloader error5");
    TEST_ENSHURE( xServInfo->supportsService(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")) ), "testloader error6");
    TEST_ENSHURE( xServInfo->getSupportedServiceNames().getLength() == 1, "testloader error7");

    xIFace.clear();
    xFactory.clear();
    xLoader.clear();
    xServInfo.clear();

    printf("Test Dll ComponentLoader, OK!\n");

    return(0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
