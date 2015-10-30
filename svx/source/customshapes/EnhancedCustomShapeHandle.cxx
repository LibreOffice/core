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

#include "EnhancedCustomShapeHandle.hxx"
#include "svx/EnhancedCustomShape2d.hxx"
#include "svx/unoapi.hxx"



EnhancedCustomShapeHandle::EnhancedCustomShapeHandle( css::uno::Reference< css::drawing::XShape >& xCustomShape, sal_uInt32 nIndex ) :
    mnIndex     ( nIndex ),
    mxCustomShape ( xCustomShape )
{
}



EnhancedCustomShapeHandle::~EnhancedCustomShapeHandle()
{
}



void SAL_CALL EnhancedCustomShapeHandle::acquire() throw()
{
    OWeakObject::acquire();
}



void SAL_CALL EnhancedCustomShapeHandle::release() throw()
{
    OWeakObject::release();
}

// XCustomShapeHandle
css::awt::Point SAL_CALL EnhancedCustomShapeHandle::getPosition()
    throw ( css::uno::RuntimeException, std::exception )
{
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxCustomShape ) );
    if ( !pSdrObjCustomShape )
        throw css::uno::RuntimeException();

    Point aPosition;
    EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
    if ( !aCustomShape2d.GetHandlePosition( mnIndex, aPosition ) )
        throw css::uno::RuntimeException();
    return css::awt::Point( aPosition.X(), aPosition.Y() );
}

void SAL_CALL EnhancedCustomShapeHandle::setControllerPosition( const css::awt::Point& aPnt )
    throw ( css::uno::RuntimeException, std::exception )
{
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxCustomShape ) );
    if ( !pSdrObjCustomShape )
        throw css::uno::RuntimeException();

    EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
    if ( !aCustomShape2d.SetHandleControllerPosition( mnIndex, aPnt ) )
        throw css::uno::RuntimeException();
}

// XInitialization
void SAL_CALL EnhancedCustomShapeHandle::initialize( const css::uno::Sequence< css::uno::Any >& /* aArguments */ )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
