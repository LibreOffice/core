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
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace pwp {
extern OUString PlaceWareExportFilter_getImplementationName() throw ( RuntimeException );
extern sal_Bool SAL_CALL PlaceWareExportFilter_supportsService( const OUString& ServiceName ) throw ( RuntimeException );
extern Sequence< OUString > SAL_CALL PlaceWareExportFilter_getSupportedServiceNames()  throw ( RuntimeException );
extern Reference< XInterface > SAL_CALL PlaceWareExportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw ( Exception );
}

using namespace ::pwp;

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL placeware_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    OUString implName = OUString::createFromAscii( pImplName );
    if ( pServiceManager && implName.equals(PlaceWareExportFilter_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            PlaceWareExportFilter_createInstance, PlaceWareExportFilter_getSupportedServiceNames() ) );

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
