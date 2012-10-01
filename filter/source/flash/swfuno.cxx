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
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace swf {
extern OUString FlashExportFilter_getImplementationName() throw ( RuntimeException );
extern sal_Bool SAL_CALL FlashExportFilter_supportsService( const OUString& ServiceName ) throw ( RuntimeException );
extern Sequence< OUString > SAL_CALL FlashExportFilter_getSupportedServiceNames()  throw ( RuntimeException );
extern Reference< XInterface > SAL_CALL FlashExportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw ( Exception );
}

extern rtl::OUString SWFDialog_getImplementationName () throw (com::sun::star::uno::RuntimeException);
extern com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL SWFDialog_getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);
extern com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SWFDialog_createInstance( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( com::sun::star::uno::Exception );

using namespace ::swf;

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL flash_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    if( pServiceManager )
    {
        Reference< XSingleServiceFactory > xFactory;

        OUString implName = OUString::createFromAscii( pImplName );
        if ( implName.equals(FlashExportFilter_getImplementationName()) )
        {
            xFactory = createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                FlashExportFilter_createInstance, FlashExportFilter_getSupportedServiceNames() );

        }
        else if ( implName.equals(SWFDialog_getImplementationName()) )
        {
            xFactory = createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SWFDialog_createInstance, SWFDialog_getSupportedServiceNames() );
        }

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
