/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "VPolarRadiusAxis.hxx"
#include "VCartesianAxis.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"
#include "Tickmarks_Equidistant.hxx"
#include <rtl/math.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;

VPolarRadiusAxis::VPolarRadiusAxis( const AxisProperties& rAxisProperties
            , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionCount )
            : VPolarAxis( rAxisProperties, xNumberFormatsSupplier, 1/*nDimensionIndex*/, nDimensionCount )
{
    m_aAxisProperties.m_fLabelDirectionSign=0.0;
    m_aAxisProperties.m_fInnerDirectionSign=0.0;
    m_aAxisProperties.m_bIsMainAxis=false;
    m_aAxisProperties.m_aLabelAlignment=LABEL_ALIGN_RIGHT;
    m_aAxisProperties.init();

    m_apAxisWithLabels = std::auto_ptr<VCartesianAxis>( new VCartesianAxis(
        m_aAxisProperties,xNumberFormatsSupplier,1/*nDimensionIndex*/,nDimensionCount
        ,new PolarPlottingPositionHelper() ) );
}

VPolarRadiusAxis::~VPolarRadiusAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VPolarRadiusAxis::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    VPolarAxis::setTransformationSceneToScreen( rMatrix );
    m_apAxisWithLabels->setTransformationSceneToScreen( rMatrix );
}

void VPolarRadiusAxis::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
            throw (uno::RuntimeException)
{
    VPolarAxis::setExplicitScaleAndIncrement( rScale, rIncrement );
    m_apAxisWithLabels->setExplicitScaleAndIncrement( rScale, rIncrement );
}

void VPolarRadiusAxis::initPlotter(  const uno::Reference< drawing::XShapes >& xLogicTarget
       , const uno::Reference< drawing::XShapes >& xFinalTarget
       , const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory
       , const rtl::OUString& rCID )
            throw (uno::RuntimeException)
{
    VPolarAxis::initPlotter(  xLogicTarget, xFinalTarget, xShapeFactory, rCID );
    m_apAxisWithLabels->initPlotter(  xLogicTarget, xFinalTarget, xShapeFactory, rCID );
}

void VPolarRadiusAxis::setScales( const std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis )
{
    VPolarAxis::setScales( rScales, bSwapXAndYAxis );
    m_apAxisWithLabels->setScales( rScales, bSwapXAndYAxis );
}

void VPolarRadiusAxis::initAxisLabelProperties( const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels )
{
    VPolarAxis::initAxisLabelProperties( rFontReferenceSize, rMaximumSpaceForLabels );
    m_apAxisWithLabels->initAxisLabelProperties( rFontReferenceSize, rMaximumSpaceForLabels );
}

sal_Int32 VPolarRadiusAxis::estimateMaximumAutoMainIncrementCount()
{
    return 2;
}

bool VPolarRadiusAxis::prepareShapeCreation()
{
    //returns true if all is ready for further shape creation and any shapes need to be created
    if( !isAnythingToDraw() )
        return false;

    if( m_xGroupShape_Shapes.is() )
        return true;

    return true;
}

void VPolarRadiusAxis::createMaximumLabels()
{
    m_apAxisWithLabels->createMaximumLabels();
}

void VPolarRadiusAxis::updatePositions()
{
    m_apAxisWithLabels->updatePositions();
}

void VPolarRadiusAxis::createLabels()
{
    m_apAxisWithLabels->createLabels();
}

void VPolarRadiusAxis::createShapes()
{
    if( !prepareShapeCreation() )
        return;

    const ExplicitScaleData& rAngleScale         = m_pPosHelper->getScales()[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];

    ::std::vector< ::std::vector< TickInfo > > aAngleTickInfos;
    TickFactory aAngleTickFactory( rAngleScale, rAngleIncrement );
    aAngleTickFactory.getAllTicks( aAngleTickInfos );

    uno::Reference< XScaling > xInverseScaling( NULL );
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    AxisProperties aAxisProperties(m_aAxisProperties);

    sal_Int32 nTick = 0;
    EquidistantTickIter aIter( aAngleTickInfos, rAngleIncrement, 0, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        if( nTick == 0 )
        {
            m_apAxisWithLabels->createShapes();
            continue;
        }

        //xxxxx pTickInfo->updateUnscaledValue( xInverseScaling );
        aAxisProperties.m_pfMainLinePositionAtOtherAxis = new double( pTickInfo->getUnscaledTickValue() );
        aAxisProperties.m_bDisplayLabels=false;

        //-------------------
        VCartesianAxis aAxis(aAxisProperties,m_xNumberFormatsSupplier
            ,1,2,new PolarPlottingPositionHelper());
        aAxis.setExplicitScaleAndIncrement( m_aScale, m_aIncrement );
        aAxis.initPlotter(m_xLogicTarget,m_xFinalTarget,m_xShapeFactory, m_aCID );
        aAxis.setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix( m_aMatrixScreenToScene ) );
        aAxis.setScales( m_pPosHelper->getScales(), false );
        aAxis.initAxisLabelProperties(m_aAxisLabelProperties.m_aFontReferenceSize,m_aAxisLabelProperties.m_aMaximumSpaceForLabels);
        aAxis.createShapes();
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
