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

#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <facreg.hxx>
#include <sddll.hxx>

#include <sfx2/sfxmodelfactory.hxx>
#include <sal/types.h>

#include <unordered_map>

using namespace com::sun::star;

// Declaration and initialization of a map from service names to locally
// unique factory identifiers.

enum FactoryId
{
    SdDrawingDocumentFactoryId,
    SdPresentationDocumentFactoryId,
};
typedef std::unordered_map<OUString, FactoryId> FactoryMap;

namespace {
FactoryMap const & GetFactoryMap()
{
    static FactoryMap aFactoryMap
    {
        { SdDrawingDocument_getImplementationName(), SdDrawingDocumentFactoryId },
        { SdPresentationDocument_getImplementationName(), SdPresentationDocumentFactoryId }
    };
    return aFactoryMap;
};
} // end of anonymous namespace

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * sd_component_getFactory(
    const sal_Char * pImplName,
    void           * pServiceManager,
    void           *  )
{
    void * pRet = nullptr;

    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( static_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference<lang::XSingleServiceFactory> xFactory;
        uno::Reference<lang::XSingleComponentFactory> xComponentFactory;

        const FactoryMap& rFactoryMap (GetFactoryMap());
        OUString sImplementationName (OUString::createFromAscii(pImplName));
        FactoryMap::const_iterator iFactory (rFactoryMap.find(sImplementationName));
        if (iFactory != rFactoryMap.end())
        {
            switch (iFactory->second)
            {
                case SdDrawingDocumentFactoryId:
                    xFactory = ::sfx2::createSfxModelFactory(
                        xMSF,
                        SdDrawingDocument_getImplementationName(),
                        SdDrawingDocument_createInstance,
                        SdDrawingDocument_getSupportedServiceNames());
                    break;

                case SdPresentationDocumentFactoryId:
                    xFactory = ::sfx2::createSfxModelFactory(
                        xMSF,
                        SdPresentationDocument_getImplementationName(),
                        SdPresentationDocument_createInstance,
                        SdPresentationDocument_getSupportedServiceNames());
                    break;

                default:
                    break;
            }
            if (xComponentFactory.is())
            {
                xComponentFactory->acquire();
                pRet = xComponentFactory.get();
            }
            else if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
    }

    if (pRet != nullptr)
        SdDLL::Init();
    return pRet;
}

} // end of extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
