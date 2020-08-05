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
#pragma once

#include "BaseCoordinateSystem.hxx"

namespace chart
{

class CartesianCoordinateSystem : public BaseCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem( sal_Int32 nDimensionCount );
    explicit CartesianCoordinateSystem( const CartesianCoordinateSystem & rSource );
    virtual ~CartesianCoordinateSystem() override;

    // ____ XCoordinateSystem ____
    virtual OUString SAL_CALL getCoordinateSystemType() override;
    virtual OUString SAL_CALL getViewServiceName() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XServiceInfo ____
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class CartesianCoordinateSystem2d : public CartesianCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem2d();
    virtual ~CartesianCoordinateSystem2d() override;

    // ____ XServiceInfo ____
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class CartesianCoordinateSystem3d : public CartesianCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem3d();
    virtual ~CartesianCoordinateSystem3d() override;

    // ____ XServiceInfo ____
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
