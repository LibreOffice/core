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

#ifndef INCLUDED_SVX_SOURCE_CUSTOMSHAPES_ENHANCEDCUSTOMSHAPEHANDLE_HXX
#define INCLUDED_SVX_SOURCE_CUSTOMSHAPES_ENHANCEDCUSTOMSHAPEHANDLE_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeHandle.hpp>
#include <com/sun/star/awt/Point.hpp>

class EnhancedCustomShapeHandle : public cppu::WeakImplHelper
<
    css::drawing::XCustomShapeHandle,
    css::lang::XInitialization
>
{
    sal_uInt32                                  mnIndex;
    css::uno::Reference< css::drawing::XShape > mxCustomShape;

public:

            EnhancedCustomShapeHandle( css::uno::Reference< css::drawing::XShape > const & xCustomShape, sal_uInt32 nIndex );
    virtual ~EnhancedCustomShapeHandle() override;

    // XInterface
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XCustomShapeHandle
    virtual css::awt::Point SAL_CALL getPosition() override;
    virtual void SAL_CALL setControllerPosition( const css::awt::Point& ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
