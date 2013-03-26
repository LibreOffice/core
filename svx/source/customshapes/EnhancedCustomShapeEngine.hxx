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

// -----------------------------------------------------------------------------

#define NMSP_UNO        com::sun::star::uno
#define NMSP_BEANS      com::sun::star::beans
#define NMSP_LANG       com::sun::star::lang


#define REF( _def_Obj )         NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )         NMSP_UNO::Sequence< _def_Obj >

// ---------------------------
// - EnhancedCustomShapeEngine -
// ---------------------------
//

class SdrObject;
class SdrObjCustomShape;
class EnhancedCustomShapeEngine : public cppu::WeakImplHelper3
<
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::drawing::XCustomShapeEngine
>
{
    REF( NMSP_LANG::XMultiServiceFactory )      mxFact;
    REF( com::sun::star::drawing::XShape )      mxShape;
    sal_Bool                                    mbForceGroupWithText;

    SdrObject* ImplForceGroupWithText( const SdrObjCustomShape* pCustoObj, SdrObject* pRenderedShape );

public:
                            EnhancedCustomShapeEngine( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                 ~EnhancedCustomShapeEngine();

    // XInterface
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const SEQ( NMSP_UNO::Any )& aArguments )
        throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw ( NMSP_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& rServiceName )
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( rtl::OUString ) SAL_CALL getSupportedServiceNames()
        throw ( NMSP_UNO::RuntimeException );

    // XCustomShapeEngine
    virtual REF( com::sun::star::drawing::XShape ) SAL_CALL render()
        throw ( NMSP_UNO::RuntimeException );
    virtual com::sun::star::awt::Rectangle SAL_CALL getTextBounds()
        throw ( NMSP_UNO::RuntimeException );
    virtual com::sun::star::drawing::PolyPolygonBezierCoords SAL_CALL getLineGeometry()
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( REF( com::sun::star::drawing::XCustomShapeHandle ) ) SAL_CALL getInteraction()
        throw ( NMSP_UNO::RuntimeException );
};

rtl::OUString EnhancedCustomShapeEngine_getImplementationName()
    throw ( NMSP_UNO::RuntimeException );
sal_Bool SAL_CALL EnhancedCustomShapeEngine_supportsService( const rtl::OUString& rServiceName )
    throw( NMSP_UNO::RuntimeException );
SEQ( rtl::OUString ) SAL_CALL EnhancedCustomShapeEngine_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
