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

#include "PlottingPositionHelper.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/chart2/XTransformation.hpp>

namespace chart
{

class VPolarTransformation : public ::cppu::WeakImplHelper<
    css::chart2::XTransformation
    >
{
public:
    VPolarTransformation( const PolarPlottingPositionHelper& rPositionHelper );
    virtual ~VPolarTransformation() override;

    // ____ XTransformation ____
    /// @see css::chart2::XTransformation
    virtual css::uno::Sequence< double > SAL_CALL transform(
        const css::uno::Sequence< double >& rSourceValues ) override;
    /// @see css::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension() override;
    /// @see css::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension() override;

private:
    PolarPlottingPositionHelper     m_aPositionHelper;
    ::basegfx::B3DHomMatrix         m_aUnitCartesianToScene;
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
