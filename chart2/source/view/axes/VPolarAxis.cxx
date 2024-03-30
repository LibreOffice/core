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

#include "VPolarAxis.hxx"
#include "VPolarAngleAxis.hxx"
#include "VPolarRadiusAxis.hxx"
#include <PlottingPositionHelper.hxx>

namespace chart
{
using namespace ::com::sun::star;

std::shared_ptr<VPolarAxis> VPolarAxis::createAxis( const AxisProperties& rAxisProperties
           , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount )
{
    if( nDimensionIndex==0 )
        return std::make_shared<VPolarAngleAxis>( rAxisProperties, xNumberFormatsSupplier, nDimensionCount );
    return std::make_shared<VPolarRadiusAxis>( rAxisProperties, xNumberFormatsSupplier, nDimensionCount );
}

VPolarAxis::VPolarAxis( const AxisProperties& rAxisProperties
            , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount )
            : VAxisBase( nDimensionIndex, nDimensionCount, rAxisProperties, xNumberFormatsSupplier )
{
    PlotterBase::m_pPosHelper = &m_aPosHelper;
}

VPolarAxis::~VPolarAxis()
{
}

void VPolarAxis::setIncrements( std::vector< ExplicitIncrementData >&& rIncrements )
{
    m_aIncrements = std::move(rIncrements);
}

bool VPolarAxis::isAnythingToDraw()
{
    return ( m_nDimension==2 && VAxisBase::isAnythingToDraw() );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
