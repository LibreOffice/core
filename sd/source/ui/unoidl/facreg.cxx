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

#include <com/sun/star/registry/XRegistryKey.hpp>

#include "sddll.hxx"

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
#include <sfx2/sfxmodelfactory.hxx>
#include "osl/diagnose.h"
#include "sal/types.h"

#include <string.h>

#include <comphelper/stl_types.hxx>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

using namespace com::sun::star;


// Forward declarations of the factories.

extern uno::Reference< uno::XInterface > SAL_CALL SdDrawingDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory, const sal_uInt64 _nCreationFlags );
extern OUString SdDrawingDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdPresentationDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory, const sal_uInt64 _nCreationFlags );
extern OUString SdPresentationDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdHtmlOptionsDialog_CreateInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdHtmlOptionsDialog_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdUnoModule_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdUnoModule_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdUnoModule_getSupportedServiceNames() throw( uno::RuntimeException );

namespace sd
{
extern uno::Reference< uno::XInterface > SAL_CALL RandomNode_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString RandomNode__getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL RandomNode_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SlideLayoutController_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SlideLayoutController_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString >  SlideLayoutController_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL InsertSlideController_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString InsertSlideController_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString >  InsertSlideController_getSupportedServiceNames() throw( uno::RuntimeException );
}

namespace sd { namespace framework {

extern uno::Reference<uno::XInterface> SAL_CALL Configuration_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString Configuration_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL Configuration_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ConfigurationController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ConfigurationController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ConfigurationController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ModuleController_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ModuleController_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ModuleController_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL BasicPaneFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString BasicPaneFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL BasicPaneFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL BasicToolBarFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString BasicToolBarFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL BasicToolBarFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL BasicViewFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString BasicViewFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL BasicViewFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL ResourceId_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString ResourceId_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL ResourceId_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL PresentationFactoryProvider_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PresentationFactoryProvider_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PresentationFactoryProvider_getSupportedServiceNames (void)
    throw (uno::RuntimeException);
} }

namespace sd { namespace sidebar {

extern uno::Reference<uno::XInterface> SAL_CALL PanelFactory_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PanelFactory_getImplementationName(void) throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PanelFactory_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

} }

namespace sd { namespace presenter {
extern uno::Reference<uno::XInterface> SAL_CALL SlideRenderer_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString SlideRenderer_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL SlideRenderer_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL PresenterCanvas_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PresenterCanvas_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PresenterCanvas_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL PresenterTextViewService_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PresenterTextViewService_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PresenterTextViewService_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL PresenterHelperService_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PresenterHelperService_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PresenterHelperService_getSupportedServiceNames (void)
    throw (uno::RuntimeException);

extern uno::Reference<uno::XInterface> SAL_CALL PresenterPreviewCache_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString PresenterPreviewCache_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL PresenterPreviewCache_getSupportedServiceNames (void)
    throw (uno::RuntimeException);
} }


namespace sd { namespace slidesorter {
extern uno::Reference<uno::XInterface> SAL_CALL SlideSorterService_createInstance(
    const uno::Reference<uno::XComponentContext>& rxContext)
    throw(uno::Exception);
extern OUString SlideSorterService_getImplementationName(void)
    throw (uno::RuntimeException);
extern uno::Sequence<OUString> SAL_CALL SlideSorterService_getSupportedServiceNames (void)
    throw (uno::RuntimeException);
} }

using namespace ::sd;
using namespace ::sd::framework;
using namespace ::sd::presenter;
using namespace ::sd::slidesorter;
using namespace ::sd::sidebar;




// ----------------------------------------------------------------------------
// Declaration and initialization of a map from service names to locally
// unique factory identifiers.

enum FactoryId
{
    SdDrawingDocumentFactoryId,
    SdPresentationDocumentFactoryId,
    SdHtmlOptionsDialogFactoryId,
    SdUnoModuleFactoryId,
    RandomNodeFactoryId,

    ConfigurationFactoryId,
    ConfigurationControllerFactoryId,
    ModuleControllerFactoryId,
    BasicPaneFactoryFactoryId,
    BasicToolBarFactoryFactoryId,
    BasicViewFactoryFactoryId,
    PanelFactoryFactoryId,
    ResourceIdFactoryId,
    PresentationFactoryProviderFactoryId,
    SlideRendererFactoryId,
    PresenterCanvasFactoryId,
    PresenterTextViewServiceFactoryId,
    PresenterHelperServiceFactoryId,
    PresenterPreviewCacheFactoryId,
    SlideSorterServiceFactoryId,
    SlideLayoutControllerFactoryId,
    InsertSlideControllerFactoryId,
};
typedef ::boost::unordered_map<OUString, FactoryId, OUStringHash, comphelper::UStringEqual> FactoryMap;


namespace {
static ::boost::shared_ptr<FactoryMap> spFactoryMap;
::boost::shared_ptr<FactoryMap> GetFactoryMap (void)
{
    if (spFactoryMap.get() == NULL)
    {
        spFactoryMap.reset(new FactoryMap);
        (*spFactoryMap)[SdDrawingDocument_getImplementationName()] = SdDrawingDocumentFactoryId;
        (*spFactoryMap)[SdPresentationDocument_getImplementationName()] = SdPresentationDocumentFactoryId;
        (*spFactoryMap)[SdHtmlOptionsDialog_getImplementationName()] = SdHtmlOptionsDialogFactoryId;
        (*spFactoryMap)[SdUnoModule_getImplementationName()] = SdUnoModuleFactoryId;
        (*spFactoryMap)[RandomNode__getImplementationName()] = RandomNodeFactoryId;
        (*spFactoryMap)[Configuration_getImplementationName()] = ConfigurationFactoryId;
        (*spFactoryMap)[ConfigurationController_getImplementationName()] = ConfigurationControllerFactoryId;
        (*spFactoryMap)[ModuleController_getImplementationName()] = ModuleControllerFactoryId;
        (*spFactoryMap)[BasicPaneFactory_getImplementationName()] = BasicPaneFactoryFactoryId;
        (*spFactoryMap)[BasicToolBarFactory_getImplementationName()] = BasicToolBarFactoryFactoryId;
        (*spFactoryMap)[BasicViewFactory_getImplementationName()] = BasicViewFactoryFactoryId;
        (*spFactoryMap)[sidebar::PanelFactory_getImplementationName()] = PanelFactoryFactoryId;
        (*spFactoryMap)[ResourceId_getImplementationName()] = ResourceIdFactoryId;
        (*spFactoryMap)[PresentationFactoryProvider_getImplementationName()] = PresentationFactoryProviderFactoryId;
        (*spFactoryMap)[SlideRenderer_getImplementationName()] = SlideRendererFactoryId;
        (*spFactoryMap)[PresenterCanvas_getImplementationName()] = PresenterCanvasFactoryId;
        (*spFactoryMap)[PresenterTextViewService_getImplementationName()] = PresenterTextViewServiceFactoryId;
        (*spFactoryMap)[PresenterHelperService_getImplementationName()] = PresenterHelperServiceFactoryId;
        (*spFactoryMap)[PresenterPreviewCache_getImplementationName()] = PresenterPreviewCacheFactoryId;
        (*spFactoryMap)[SlideSorterService_getImplementationName()] = SlideSorterServiceFactoryId;
        (*spFactoryMap)[SlideLayoutController_getImplementationName()] = SlideLayoutControllerFactoryId;
        (*spFactoryMap)[InsertSlideController_getImplementationName()] = InsertSlideControllerFactoryId;
    }
    return spFactoryMap;
};
} // end of anonymous namespace


extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL sd_component_getFactory(
    const sal_Char * pImplName,
    void           * pServiceManager,
    void           *  )
{
    void * pRet = 0;

    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference<lang::XSingleServiceFactory> xFactory;
        uno::Reference<lang::XSingleComponentFactory> xComponentFactory;

        ::boost::shared_ptr<FactoryMap> pFactoryMap (GetFactoryMap());
        OUString sImplementationName (OUString::createFromAscii(pImplName));
        FactoryMap::const_iterator iFactory (pFactoryMap->find(sImplementationName));
        if (iFactory != pFactoryMap->end())
        {
            switch (iFactory->second)
            {
                case SdHtmlOptionsDialogFactoryId:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        SdHtmlOptionsDialog_getImplementationName(),
                        SdHtmlOptionsDialog_CreateInstance,
                        SdHtmlOptionsDialog_getSupportedServiceNames());
                    break;

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

                case SdUnoModuleFactoryId:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        SdUnoModule_getImplementationName(),
                        SdUnoModule_createInstance,
                        SdUnoModule_getSupportedServiceNames());
                    break;

                case RandomNodeFactoryId:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        sd::RandomNode__getImplementationName(),
                        sd::RandomNode_createInstance,
                        sd::RandomNode_getSupportedServiceNames());
                    break;

                case ConfigurationFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::Configuration_createInstance,
                        sd::framework::Configuration_getImplementationName(),
                        sd::framework::Configuration_getSupportedServiceNames());
                    break;

                case ConfigurationControllerFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ConfigurationController_createInstance,
                        sd::framework::ConfigurationController_getImplementationName(),
                        sd::framework::ConfigurationController_getSupportedServiceNames());
                    break;

                case ModuleControllerFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ModuleController_createInstance,
                        sd::framework::ModuleController_getImplementationName(),
                        sd::framework::ModuleController_getSupportedServiceNames());
                    break;

                case BasicPaneFactoryFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::BasicPaneFactory_createInstance,
                        sd::framework::BasicPaneFactory_getImplementationName(),
                        sd::framework::BasicPaneFactory_getSupportedServiceNames());
                    break;

                case BasicToolBarFactoryFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::BasicToolBarFactory_createInstance,
                        sd::framework::BasicToolBarFactory_getImplementationName(),
                        sd::framework::BasicToolBarFactory_getSupportedServiceNames());
                    break;

                case BasicViewFactoryFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::BasicViewFactory_createInstance,
                        sd::framework::BasicViewFactory_getImplementationName(),
                        sd::framework::BasicViewFactory_getSupportedServiceNames());
                    break;

                case PanelFactoryFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::sidebar::PanelFactory_createInstance,
                        sd::sidebar::PanelFactory_getImplementationName(),
                        sd::sidebar::PanelFactory_getSupportedServiceNames());
                    break;

                case ResourceIdFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::ResourceId_createInstance,
                        sd::framework::ResourceId_getImplementationName(),
                        sd::framework::ResourceId_getSupportedServiceNames());
                    break;

                case PresentationFactoryProviderFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::framework::PresentationFactoryProvider_createInstance,
                        sd::framework::PresentationFactoryProvider_getImplementationName(),
                        sd::framework::PresentationFactoryProvider_getSupportedServiceNames());
                    break;

                case SlideRendererFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::presenter::SlideRenderer_createInstance,
                        sd::presenter::SlideRenderer_getImplementationName(),
                        sd::presenter::SlideRenderer_getSupportedServiceNames());
                    break;

                case PresenterCanvasFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::presenter::PresenterCanvas_createInstance,
                        sd::presenter::PresenterCanvas_getImplementationName(),
                        sd::presenter::PresenterCanvas_getSupportedServiceNames());
                    break;

                case PresenterTextViewServiceFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::presenter::PresenterTextViewService_createInstance,
                        sd::presenter::PresenterTextViewService_getImplementationName(),
                        sd::presenter::PresenterTextViewService_getSupportedServiceNames());
                    break;

                case PresenterHelperServiceFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::presenter::PresenterHelperService_createInstance,
                        sd::presenter::PresenterHelperService_getImplementationName(),
                        sd::presenter::PresenterHelperService_getSupportedServiceNames());
                    break;

                case PresenterPreviewCacheFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::presenter::PresenterPreviewCache_createInstance,
                        sd::presenter::PresenterPreviewCache_getImplementationName(),
                        sd::presenter::PresenterPreviewCache_getSupportedServiceNames());
                    break;

                case SlideSorterServiceFactoryId:
                    xComponentFactory = ::cppu::createSingleComponentFactory(
                        sd::slidesorter::SlideSorterService_createInstance,
                        sd::slidesorter::SlideSorterService_getImplementationName(),
                        sd::slidesorter::SlideSorterService_getSupportedServiceNames());
                    break;

                case SlideLayoutControllerFactoryId:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        sd::SlideLayoutController_getImplementationName(),
                        sd::SlideLayoutController_createInstance,
                        sd::SlideLayoutController_getSupportedServiceNames());
                    break;

                case InsertSlideControllerFactoryId:
                    xFactory = ::cppu::createSingleFactory(
                        xMSF,
                        sd::InsertSlideController_getImplementationName(),
                        sd::InsertSlideController_createInstance,
                        sd::InsertSlideController_getSupportedServiceNames());
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

    if (pRet != NULL)
        SdDLL::Init();
    return pRet;
}

} // end of extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
