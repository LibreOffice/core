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


#include <sal/config.h>

#include <xmloff/dllapi.h>

#include <string.h>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/factory.hxx>

#include <facreg.hxx>

using namespace com::sun::star;

extern "C"
{

#define SINGLEFACTORY(classname)\
        if( classname##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )\
        {\
            xFactory = ::cppu::createSingleFactory( xMSF,\
                classname##_getImplementationName(),\
                classname##_createInstance,\
                classname##_getSupportedServiceNames() );\
        }

XMLOFF_DLLPUBLIC void * xo_component_getFactory( const char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( static_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );

        // auto text import/export
        SINGLEFACTORY( XMLAutoTextEventExport )
        else SINGLEFACTORY( XMLAutoTextEventImport )
        else SINGLEFACTORY( XMLAutoTextEventExportOOO )

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
