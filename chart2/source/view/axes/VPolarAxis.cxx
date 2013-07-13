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
#include "macros.hxx"
#include "Tickmarks.hxx"
#include "ShapeFactory.hxx"

#include <boost/scoped_ptr.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VPolarAxis* VPolarAxis::createAxis( const AxisProperties& rAxisProperties
           , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount )
{
    if( 0==nDimensionIndex )
        return new VPolarAngleAxis( rAxisProperties, xNumberFormatsSupplier, nDimensionCount );
    return new VPolarRadiusAxis( rAxisProperties, xNumberFormatsSupplier, nDimensionCount );
}

VPolarAxis::VPolarAxis( const AxisProperties& rAxisProperties
            , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount )
            : VAxisBase( nDimensionIndex, nDimensionCount, rAxisProperties, xNumberFormatsSupplier )
            , m_pPosHelper( new PolarPlottingPositionHelper() )
            , m_aIncrements()
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

VPolarAxis::~VPolarAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VPolarAxis::setIncrements( const std::vector< ExplicitIncrementData >& rIncrements )
{
    m_aIncrements = rIncrements;
}

sal_Bool VPolarAxis::isAnythingToDraw()
{
    return ( 2==m_nDimension && VAxisBase::isAnythingToDraw() );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
