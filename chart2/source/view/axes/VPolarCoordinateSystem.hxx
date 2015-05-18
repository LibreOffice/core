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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_AXES_VPOLARCOORDINATESYSTEM_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_AXES_VPOLARCOORDINATESYSTEM_HXX

#include "VCoordinateSystem.hxx"

namespace chart
{

/**
*/
class VPolarCoordinateSystem : public VCoordinateSystem
{
public:
    VPolarCoordinateSystem( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );
    virtual ~VPolarCoordinateSystem();

    //better performance for big data
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > getCoordinateSystemResolution( const ::com::sun::star::awt::Size& rPageSize
                                    , const ::com::sun::star::awt::Size& rPageResolution ) SAL_OVERRIDE;

    virtual void createVAxisList(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument> & xChartDoc
            , const ::com::sun::star::awt::Size& rFontReferenceSize
            , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels ) SAL_OVERRIDE;

    virtual void initVAxisInList() SAL_OVERRIDE;
    virtual void updateScalesAndIncrementsOnAxes() SAL_OVERRIDE;

    virtual void createGridShapes() SAL_OVERRIDE;

private:
    VPolarCoordinateSystem();
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
