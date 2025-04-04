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

#include "VPolarRadiusAxis.hxx"
#include "VCartesianAxis.hxx"
#include <PlottingPositionHelper.hxx>
#include <Axis.hxx>
#include <CommonConverters.hxx>
#include "Tickmarks_Equidistant.hxx"

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VPolarRadiusAxis::VPolarRadiusAxis( const AxisProperties& rAxisProperties
            , const rtl::Reference< SvNumberFormatsSupplierObj >& xNumberFormatsSupplier
            , sal_Int32 nDimensionCount )
            : VPolarAxis( rAxisProperties, xNumberFormatsSupplier, 1/*nDimensionIndex*/, nDimensionCount )
{
    m_aAxisProperties.maLabelAlignment.mfLabelDirection = 0.0;
    m_aAxisProperties.maLabelAlignment.mfInnerTickDirection = 0.0;
    m_aAxisProperties.maLabelAlignment.meAlignment = LABEL_ALIGN_RIGHT;
    m_aAxisProperties.m_bIsMainAxis=false;
    m_aAxisProperties.init();

    m_apAxisWithLabels.reset( new VCartesianAxis(
        m_aAxisProperties,xNumberFormatsSupplier,1/*nDimensionIndex*/,nDimensionCount
        ,new PolarPlottingPositionHelper() ) );
}

VPolarRadiusAxis::~VPolarRadiusAxis()
{
}

void VPolarRadiusAxis::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    VPolarAxis::setTransformationSceneToScreen( rMatrix );
    m_apAxisWithLabels->setTransformationSceneToScreen( rMatrix );
}

void VPolarRadiusAxis::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
{
    VPolarAxis::setExplicitScaleAndIncrement( rScale, rIncrement );
    m_apAxisWithLabels->setExplicitScaleAndIncrement( rScale, rIncrement );
}

void VPolarRadiusAxis::initPlotter(  const rtl::Reference<SvxShapeGroupAnyD>& xLogicTarget
       , const rtl::Reference<SvxShapeGroupAnyD>& xFinalTarget
       , const OUString& rCID )
{
    VPolarAxis::initPlotter(  xLogicTarget, xFinalTarget, rCID );
    m_apAxisWithLabels->initPlotter(  xLogicTarget, xFinalTarget, rCID );
}

void VPolarRadiusAxis::setScales( std::vector< ExplicitScaleData >&& rScales, bool bSwapXAndYAxis )
{
    VPolarAxis::setScales( std::vector(rScales), bSwapXAndYAxis );
    m_apAxisWithLabels->setScales( std::move(rScales), bSwapXAndYAxis );
}

void VPolarRadiusAxis::initAxisLabelProperties( const css::awt::Size& rFontReferenceSize
                  , const css::awt::Rectangle& rMaximumSpaceForLabels )
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

    TickInfoArraysType aAngleTickInfos;
    TickFactory aAngleTickFactory( rAngleScale, rAngleIncrement );
    aAngleTickFactory.getAllTicks( aAngleTickInfos );

    uno::Reference< XScaling > xInverseScaling;
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    AxisProperties aAxisProperties(m_aAxisProperties);

    sal_Int32 nTick = 0;
    EquidistantTickIter aIter( aAngleTickInfos, rAngleIncrement, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        if( nTick == 0 )
        {
            m_apAxisWithLabels->createShapes();
            continue;
        }

        //xxxxx pTickInfo->updateUnscaledValue( xInverseScaling );
        aAxisProperties.m_pfMainLinePositionAtOtherAxis = pTickInfo->getUnscaledTickValue();
        aAxisProperties.m_bDisplayLabels=false;

        VCartesianAxis aAxis(aAxisProperties,m_xNumberFormatsSupplier
            ,1,2,new PolarPlottingPositionHelper());
        aAxis.setExplicitScaleAndIncrement( m_aScale, m_aIncrement );
        aAxis.initPlotter(m_xLogicTarget,m_xFinalTarget, m_aCID );
        aAxis.setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix( m_aMatrixScreenToScene ) );
        aAxis.setScales( std::vector(m_pPosHelper->getScales()), false );
        aAxis.initAxisLabelProperties(m_aAxisLabelProperties.m_aFontReferenceSize,m_aAxisLabelProperties.m_aMaximumSpaceForLabels);
        aAxis.createShapes();
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
