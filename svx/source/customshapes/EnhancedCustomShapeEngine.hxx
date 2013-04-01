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

#ifndef _ENHANCED_CUSTOMSHAPE_ENGINE_HXX
#define _ENHANCED_CUSTOMSHAPE_ENGINE_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>

// ---------------------------
// - EnhancedCustomShapeEngine -
// ---------------------------
//

class SdrObject;
class SdrObjCustomShape;
class EnhancedCustomShapeEngine : public cppu::WeakImplHelper3
<
    css::lang::XInitialization,
    css::lang::XServiceInfo,
    css::drawing::XCustomShapeEngine
>
{
    css::uno::Reference< css::lang::XMultiServiceFactory >      mxFact;
    css::uno::Reference< css::drawing::XShape >      mxShape;
    sal_Bool                                    mbForceGroupWithText;

    SdrObject* ImplForceGroupWithText( const SdrObjCustomShape* pCustoObj, SdrObject* pRenderedShape );

public:
                            EnhancedCustomShapeEngine( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMgr );
    virtual                 ~EnhancedCustomShapeEngine();

    // XInterface
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& rServiceName )
        throw ( css::uno::RuntimeException );
    virtual css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw ( css::uno::RuntimeException );

    // XCustomShapeEngine
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL render()
        throw ( css::uno::RuntimeException );
    virtual css::awt::Rectangle SAL_CALL getTextBounds()
        throw ( css::uno::RuntimeException );
    virtual css::drawing::PolyPolygonBezierCoords SAL_CALL getLineGeometry()
        throw ( css::uno::RuntimeException );
    virtual css::uno::Sequence< css::uno::Reference< css::drawing::XCustomShapeHandle > > SAL_CALL getInteraction()
        throw ( css::uno::RuntimeException );
};

rtl::OUString EnhancedCustomShapeEngine_getImplementationName()
    throw ( css::uno::RuntimeException );
sal_Bool SAL_CALL EnhancedCustomShapeEngine_supportsService( const rtl::OUString& rServiceName )
    throw( css::uno::RuntimeException );
css::uno::Sequence< rtl::OUString > SAL_CALL EnhancedCustomShapeEngine_getSupportedServiceNames()
    throw( css::uno::RuntimeException );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
