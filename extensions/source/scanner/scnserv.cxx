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

#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>
#include "scanner.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::registry;


extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL scn_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    REF( ::com::sun::star::lang::XSingleServiceFactory ) xFactory;
    void*                                                pRet = 0;

    if( ::rtl::OUString::createFromAscii( pImplName ) == ScannerManager::getImplementationName_Static() )
    {
        xFactory = REF( ::com::sun::star::lang::XSingleServiceFactory )( ::cppu::createSingleFactory(
                        static_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ),
                        ScannerManager::getImplementationName_Static(),
                        ScannerManager_CreateInstance,
                        ScannerManager::getSupportedServiceNames_Static() ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
