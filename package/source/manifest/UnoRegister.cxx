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

#include <ManifestReader.hxx>
#include <ManifestWriter.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>
#include <ZipPackage.hxx>

#include <zipfileaccess.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::packages;

/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL package2_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;
    uno::Reference< XMultiServiceFactory > xSMgr(
            static_cast< XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< XSingleServiceFactory > xFactory;

    if (ManifestReader::static_getImplementationName().equalsAscii( pImplName ) )
        xFactory = ManifestReader::createServiceFactory ( xSMgr );
    else if (ManifestWriter::static_getImplementationName().equalsAscii( pImplName ) )
        xFactory = ManifestWriter::createServiceFactory ( xSMgr );
    else if (ZipPackage::static_getImplementationName().equalsAscii( pImplName ) )
        xFactory = ZipPackage::createServiceFactory ( xSMgr );
    else if ( OZipFileAccess::impl_staticGetImplementationName().equalsAscii( pImplName ) )
        xFactory = ::cppu::createSingleFactory( xSMgr,
                                           OZipFileAccess::impl_staticGetImplementationName(),
                                           OZipFileAccess::impl_staticCreateSelfInstance,
                                           OZipFileAccess::impl_staticGetSupportedServiceNames() );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
