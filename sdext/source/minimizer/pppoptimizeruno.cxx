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
