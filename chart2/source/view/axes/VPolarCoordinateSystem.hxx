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

#include <VCoordinateSystem.hxx>

namespace chart
{

class VPolarCoordinateSystem : public VCoordinateSystem
{
public:
    VPolarCoordinateSystem() = delete;
    explicit VPolarCoordinateSystem( const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys );
    virtual ~VPolarCoordinateSystem() override;

    //better performance for big data
    virtual css::uno::Sequence< sal_Int32 > getCoordinateSystemResolution( const css::awt::Size& rPageSize
                                    , const css::awt::Size& rPageResolution ) override;

    virtual void createVAxisList(
            const css::uno::Reference< css::chart2::XChartDocument> & xChartDoc
            , const css::awt::Size& rFontReferenceSize
            , const css::awt::Rectangle& rMaximumSpaceForLabels
            , bool bLimitSpaceForLabels ) override;

    virtual void initVAxisInList() override;
    virtual void updateScalesAndIncrementsOnAxes() override;

    virtual void createGridShapes() override;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
