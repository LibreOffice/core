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

#include <cppuhelper/factory.hxx>

#include <com/sun/star/container/XSet.hpp>

#include <osl/diagnose.h>

#include "KDE4FilePicker.hxx"
#include "FPServiceInfo.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using ::com::sun::star::ui::dialogs::XFilePicker;

static Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >& serviceManager )
{
    return Reference< XInterface >(static_cast< XFilePicker* >( new KDE4FilePicker( serviceManager ) ) );
}

// the three uno functions that will be exported
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* )
    {
        void* pRet = 0;

        if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, FILE_PICKER_IMPL_NAME ) ) )
        {
            Sequence< OUString > aSNS( 1 );
            aSNS.getArray( )[0] = OUString(RTL_CONSTASCII_USTRINGPARAM( FILE_PICKER_SERVICE_NAME ));

            Reference< XSingleServiceFactory > xFactory ( createSingleFactory(
                        reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
                        OUString::createFromAscii( pImplName ),
                        createInstance,
                        aSNS ) );
            if ( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }

        return pRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
