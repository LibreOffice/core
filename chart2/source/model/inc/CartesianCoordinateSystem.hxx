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
#ifndef _CHART_CARTESIANCOORDINATESYSTEM_HXX
#define _CHART_CARTESIANCOORDINATESYSTEM_HXX

#include "ServiceMacros.hxx"
#include "BaseCoordinateSystem.hxx"

namespace chart
{

class CartesianCoordinateSystem : public BaseCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        sal_Int32 nDimensionCount = 2,
        sal_Bool bSwapXAndYAxis = sal_False );
    explicit CartesianCoordinateSystem( const CartesianCoordinateSystem & rSource );
    virtual ~CartesianCoordinateSystem();

    // ____ XCoordinateSystem ____
    virtual OUString SAL_CALL getCoordinateSystemType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getViewServiceName()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

class CartesianCoordinateSystem2d : public CartesianCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem2d(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~CartesianCoordinateSystem2d();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( CartesianCoordinateSystem2d )
    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

class CartesianCoordinateSystem3d : public CartesianCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem3d(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~CartesianCoordinateSystem3d();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( CartesianCoordinateSystem3d )
    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

}  // namespace chart

// _CHART_CARTESIANCOORDINATESYSTEM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
