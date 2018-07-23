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

#include <string.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>

#include "XMLFilterRegistration.hxx"

using namespace ::com::sun::star;

#define ENUMERATE_SERVICE( classname ) \
    { classname##_getImplementationName, classname##_getSupportedServiceNames, classname##_createInstance }

namespace
{
    typedef OUString (* GetImplementationName)();
    typedef uno::Sequence< OUString > (* GetSupportedServiceNames)();
    typedef uno::Reference< ::uno::XInterface > (* CreateInstance)(
        const uno::Reference< lang::XMultiServiceFactory >& );

    struct ServiceDescriptor
    {
        GetImplementationName const    getImplementationName;
        GetSupportedServiceNames const getSupportedServiceNames;
        CreateInstance const           createInstance;
    };

    const ServiceDescriptor* getServiceDescriptors()
    {
        static const ServiceDescriptor aDescriptors[] =
        {
            ENUMERATE_SERVICE( OOo2OasisTransformer ),
            ENUMERATE_SERVICE( Oasis2OOoTransformer ),

            ENUMERATE_SERVICE( XMLAutoTextEventImportOOO ),
            ENUMERATE_SERVICE( XMLMetaImportOOO ),
            ENUMERATE_SERVICE( XMLMathSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLMathMetaImportOOO ),
            ENUMERATE_SERVICE( XMLCalcSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLCalcMetaImportOOO ),
            ENUMERATE_SERVICE( XMLCalcContentImportOOO ),
            ENUMERATE_SERVICE( XMLCalcStylesImportOOO ),
            ENUMERATE_SERVICE( XMLCalcImportOOO ),
            ENUMERATE_SERVICE( XMLWriterSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLWriterMetaImportOOO ),
            ENUMERATE_SERVICE( XMLWriterContentImportOOO ),
            ENUMERATE_SERVICE( XMLWriterStylesImportOOO ),
            ENUMERATE_SERVICE( XMLWriterImportOOO ),
            ENUMERATE_SERVICE( XMLChartContentImportOOO ),
            ENUMERATE_SERVICE( XMLChartStylesImportOOO ),
            ENUMERATE_SERVICE( XMLChartImportOOO ),
            ENUMERATE_SERVICE( XMLDrawSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLDrawMetaImportOOO ),
            ENUMERATE_SERVICE( XMLDrawContentImportOOO ),
            ENUMERATE_SERVICE( XMLDrawStylesImportOOO ),
            ENUMERATE_SERVICE( XMLDrawImportOOO ),
            ENUMERATE_SERVICE( XMLImpressSettingsImportOOO ),
            ENUMERATE_SERVICE( XMLImpressMetaImportOOO ),
            ENUMERATE_SERVICE( XMLImpressContentImportOOO ),
            ENUMERATE_SERVICE( XMLImpressStylesImportOOO ),
            ENUMERATE_SERVICE( XMLImpressImportOOO ),
            { nullptr, nullptr, nullptr }
        };
        return aDescriptors;
    };
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* xof_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;
    if( pServiceManager )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xMSF( static_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

            const sal_Int32 nImplNameLen = strlen( pImplName );

            const ServiceDescriptor* pDescriptor = getServiceDescriptors();
            while ( pDescriptor->getImplementationName )
            {
                if ( pDescriptor->getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
                {
                    uno::Reference< lang::XSingleServiceFactory > xFactory =
                        ::cppu::createSingleFactory( xMSF,
                            pDescriptor->getImplementationName(),
                            pDescriptor->createInstance,
                            pDescriptor->getSupportedServiceNames()
                        );

                    if ( xFactory.is() )
                    {
                        xFactory->acquire();
                        pRet = xFactory.get();
                        break;
                    }
                }

                ++pDescriptor;
            }
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "xof::xof_component_getFactory: Exception!" );
        }
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
