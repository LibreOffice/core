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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VPolarAxis.hxx"
#include "VPolarAngleAxis.hxx"
#include "VPolarRadiusAxis.hxx"
#include "macros.hxx"
#include "Tickmarks.hxx"
#include "ShapeFactory.hxx"

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
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

//.............................................................................
} //namespace chart
//.............................................................................
