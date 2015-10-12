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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_AXES_VPOLARANGLEAXIS_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_AXES_VPOLARANGLEAXIS_HXX

#include "VPolarAxis.hxx"
#include "Tickmarks_Equidistant.hxx"

namespace chart
{

/**
*/

class VPolarAngleAxis : public VPolarAxis
{
public:
    VPolarAngleAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionCount );
    virtual ~VPolarAngleAxis();

    virtual void createMaximumLabels() override;
    virtual void createLabels() override;
    virtual void updatePositions() override;

    virtual void createShapes() override;

private: //methods
    bool createTextShapes_ForAngleAxis(
                       const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                     , EquidistantTickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , double fLogicRadius, double fLogicZ );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
