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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FACREG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FACREG_HXX

#include <sal/config.h>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XComponentContext; }
} } }

css::uno::Reference< css::uno::XInterface > SAL_CALL SdDrawingDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory, const sal_uInt64 _nCreationFlags );
OUString SdDrawingDocument_getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface > SAL_CALL SdPresentationDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory, const sal_uInt64 _nCreationFlags );
OUString SdPresentationDocument_getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface > SAL_CALL SdHtmlOptionsDialog_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory );
OUString SdHtmlOptionsDialog_getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames() throw( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface > SAL_CALL SdUnoModule_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory );
OUString SdUnoModule_getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString > SAL_CALL SdUnoModule_getSupportedServiceNames() throw( css::uno::RuntimeException );

namespace sd
{
css::uno::Reference< css::uno::XInterface > SAL_CALL RandomNode_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory );
OUString RandomNode__getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString > SAL_CALL RandomNode_getSupportedServiceNames() throw( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface > SAL_CALL SlideLayoutController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory );
OUString SlideLayoutController_getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString >  SlideLayoutController_getSupportedServiceNames() throw( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface > SAL_CALL InsertSlideController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory );
OUString InsertSlideController_getImplementationName() throw( css::uno::RuntimeException );
css::uno::Sequence< OUString >  InsertSlideController_getSupportedServiceNames() throw( css::uno::RuntimeException );
}

namespace sd { namespace framework {

css::uno::Reference<css::uno::XInterface> SAL_CALL Configuration_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString Configuration_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL Configuration_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL ConfigurationController_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString ConfigurationController_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL ConfigurationController_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL ModuleController_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString ModuleController_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL ModuleController_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL BasicPaneFactory_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString BasicPaneFactory_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL BasicPaneFactory_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL BasicToolBarFactory_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString BasicToolBarFactory_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL BasicToolBarFactory_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL BasicViewFactory_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString BasicViewFactory_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL BasicViewFactory_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL ResourceId_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString ResourceId_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL ResourceId_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL PresentationFactoryProvider_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString PresentationFactoryProvider_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL PresentationFactoryProvider_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);
} }

namespace sd { namespace sidebar {

css::uno::Reference<css::uno::XInterface> SAL_CALL PanelFactory_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString PanelFactory_getImplementationName(void) throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL PanelFactory_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

} }

namespace sd { namespace presenter {
css::uno::Reference<css::uno::XInterface> SAL_CALL SlideRenderer_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString SlideRenderer_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL SlideRenderer_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL PresenterCanvas_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString PresenterCanvas_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL PresenterCanvas_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL PresenterTextViewService_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString PresenterTextViewService_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL PresenterTextViewService_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL PresenterHelperService_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString PresenterHelperService_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL PresenterHelperService_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);

css::uno::Reference<css::uno::XInterface> SAL_CALL PresenterPreviewCache_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString PresenterPreviewCache_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL PresenterPreviewCache_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);
} }


namespace sd { namespace slidesorter {
css::uno::Reference<css::uno::XInterface> SAL_CALL SlideSorterService_createInstance(
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    throw(css::uno::Exception);
OUString SlideSorterService_getImplementationName(void)
    throw (css::uno::RuntimeException);
css::uno::Sequence<OUString> SAL_CALL SlideSorterService_getSupportedServiceNames (void)
    throw (css::uno::RuntimeException);
} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
