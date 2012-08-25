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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>

#include <cppuhelper/factory.hxx>

#include "unomodel.hxx"

namespace basctl
{

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

extern "C" {

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplementationName,
                                     void* pServiceManager,
                                     void* pRegistryKey )
{
    (void)pRegistryKey;

    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory >   xFactory                                                                                                ;
        Reference< XMultiServiceFactory >    xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if( SIDEModel::getImplementationName_Static().equalsAscii( pImplementationName ) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SIDEModel::getImplementationName_Static(),
            SIDEModel_createInstance,
            SIDEModel::getSupportedServiceNames_Static() );
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
