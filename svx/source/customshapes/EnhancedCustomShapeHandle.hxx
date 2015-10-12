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

#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeHandle.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <cppuhelper/weakref.hxx>

class EnhancedCustomShapeHandle : public cppu::WeakImplHelper
<
    com::sun::star::drawing::XCustomShapeHandle,
    com::sun::star::lang::XInitialization
>
{
    sal_uInt32                                                          mnIndex;
    com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxCustomShape;

public:

            EnhancedCustomShapeHandle( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xCustomShape, sal_uInt32 nIndex );
    virtual ~EnhancedCustomShapeHandle();

    // XInterface
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XCustomShapeHandle
    virtual com::sun::star::awt::Point SAL_CALL getPosition()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setControllerPosition( const com::sun::star::awt::Point& )
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
