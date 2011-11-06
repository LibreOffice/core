/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
