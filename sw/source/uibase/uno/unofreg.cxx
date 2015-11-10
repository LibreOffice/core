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

#include <config_features.h>

#include "SwXFilterOptions.hxx"
#include "swdll.hxx"
#include "unofreg.hxx"
#include "unomailmerge.hxx"
#include <sal/types.h>
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <vcl/svapp.hxx>

#include <string.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;

// #i73788#
#include <cppuhelper/implementationentry.hxx>
#include <finalthreadmanager.hxx>

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL sw_component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * )
{
    void * pRet = nullptr;
    if( pServiceManager )
    {
        uno::Reference< XMultiServiceFactory > xMSF(
            static_cast< XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( SwTextDocument_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::sfx2::createSfxModelFactory( xMSF,
                SwTextDocument_getImplementationName(),
                SwTextDocument_createInstance,
                SwTextDocument_getSupportedServiceNames() );
        }
        else if( SwUnoModule_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwUnoModule_getImplementationName(),
                SwUnoModule_createInstance,
                SwUnoModule_getSupportedServiceNames() );
        }

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

} // extern "C"

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
SwXMailMerge_get_implementation(css::uno::XComponentContext*,
                                css::uno::Sequence<css::uno::Any> const &)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    SolarMutexGuard aGuard;

    //the module may not be loaded
    SwGlobals::ensure();
    return cppu::acquire(new SwXMailMerge());
#else
    return nullptr;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
