/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _ENHANCED_CUSTOMSHAPE_HANDLE_HXX
#define _ENHANCED_CUSTOMSHAPE_HANDLE_HXX

#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeHandle.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <cppuhelper/weakref.hxx>

class EnhancedCustomShapeHandle : public cppu::WeakImplHelper2
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
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XCustomShapeHandle
    virtual com::sun::star::awt::Point SAL_CALL getPosition()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setControllerPosition( const com::sun::star::awt::Point& )
        throw ( com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
