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
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <pppoptimizer.hxx>
#include <pppoptimizerdialog.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void* SAL_CALL pptminimizer_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager,
        SAL_UNUSED_PARAMETER void * /*pRegistryKey*/ )
    {
        OUString    aImplName( OUString::createFromAscii( pImplName ) );
        void*       pRet = 0;

        if( pServiceManager )
        {
            Reference< XSingleComponentFactory > xFactory;
            if( aImplName.equals( PPPOptimizer_getImplementationName() ) )
            {
                xFactory = createSingleComponentFactory(
                        PPPOptimizer_createInstance,
                        OUString::createFromAscii( pImplName ),
                        PPPOptimizer_getSupportedServiceNames() );

            }
            else if( aImplName.equals( PPPOptimizerDialog_getImplementationName() ) )
            {
                xFactory = createSingleComponentFactory(
                        PPPOptimizerDialog_createInstance,
                        OUString::createFromAscii( pImplName ),
                        PPPOptimizerDialog_getSupportedServiceNames() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
