/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/numeric/ftools.hxx>

#include "VPolarAngleAxis.hxx"
#include "VPolarGrid.hxx"
#include "AbstractShapeFactory.hxx"
#include "macros.hxx"
#include "NumberFormatterWrapper.hxx"
#include "PolarLabelPositionHelper.hxx"
#include <tools/color.hxx>

#include <boost/scoped_ptr.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;

VPolarAngleAxis::VPolarAngleAxis( const AxisProperties& rAxisProperties
            , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionCount )
            : VPolarAxis( rAxisProperties, xNumberFormatsSupplier, 0/*nDimensionIndex*/, nDimensionCount )
{
}

VPolarAngleAxis::~VPolarAngleAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

bool VPolarAngleAxis::createTextShapes_ForAngleAxis(
                       const uno::Reference< drawing::XShapes >& xTarget
                     , EquidistantTickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , double fLogicRadius
                     , double fLogicZ )
{
    sal_Int32 nDimensionCount = 2;
    AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);

    FixedNumberFormatter aFixedNumberFormatter(
        m_xNumberFormatsSupplier, rAxisLabelProperties.nNumberFormatKey );

    
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    uno::Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , rAxisLabelProperties.m_aFontReferenceSize );

    uno::Any* pColorAny = PropertyMapper::getValuePointer(aPropValues,aPropNames,"CharColor");
    sal_Int32 nColor = Color( COL_AUTO ).GetColor();
    if(pColorAny)
        *pColorAny >>= nColor;

    const uno::Sequence< OUString >* pLabels = m_bUseTextLabels? &m_aTextLabels : 0;

    
    sal_Int32 nTick = 0;

    for( TickInfo* pTickInfo = rTickIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rTickIter.nextInfo(), nTick++ )
    {
        
        if( nTick%rAxisLabelProperties.nRhythm != 0)
            continue;

        
        if( !pTickInfo->bPaintIt )
            continue;

        
        
        

        if(!pTickInfo->xTextShape.is())
        {
            
            bool bHasExtraColor=false;
            sal_Int32 nExtraColor=0;

            OUString aLabel;
            if(pLabels)
            {
                sal_Int32 nIndex = static_cast< sal_Int32 >(pTickInfo->getUnscaledTickValue()) - 1; 
                if( nIndex>=0 && nIndex<pLabels->getLength() )
                    aLabel = (*pLabels)[nIndex];
            }
            else
                aLabel = aFixedNumberFormatter.getFormattedString( pTickInfo->getUnscaledTickValue(), nExtraColor, bHasExtraColor );

            if(pColorAny)
                *pColorAny = uno::makeAny(bHasExtraColor?nExtraColor:nColor);

            double fLogicAngle = pTickInfo->getUnscaledTickValue();

            LabelAlignment eLabelAlignment(LABEL_ALIGN_CENTER);
            PolarLabelPositionHelper aPolarLabelPositionHelper(m_pPosHelper,nDimensionCount,xTarget, pShapeFactory);
            sal_Int32 nScreenValueOffsetInRadiusDirection = m_aAxisLabelProperties.m_aMaximumSpaceForLabels.Height/15;
            awt::Point aAnchorScreenPosition2D( aPolarLabelPositionHelper.getLabelScreenPositionAndAlignmentForLogicValues(
                    eLabelAlignment, fLogicAngle, fLogicRadius, fLogicZ, nScreenValueOffsetInRadiusDirection ));
            LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, eLabelAlignment );

            
            const double fRotationAnglePi(rAxisLabelProperties.fRotationAngleDegree * (F_PI / -180.0));

            uno::Any aATransformation = AbstractShapeFactory::makeTransformation( aAnchorScreenPosition2D, fRotationAnglePi );
            OUString aStackedLabel = AbstractShapeFactory::getStackedString( aLabel, rAxisLabelProperties.bStackCharacters );

            pTickInfo->xTextShape = pShapeFactory->createText( xTarget, aStackedLabel, aPropNames, aPropValues, aATransformation );
        }

        
        
    }
    return true;
}

void VPolarAngleAxis::createMaximumLabels()
{
    if( !prepareShapeCreation() )
        return;

    createLabels();
}

void VPolarAngleAxis::updatePositions()
{
    

    if( !prepareShapeCreation() )
        return;

    createLabels();
}

void VPolarAngleAxis::createLabels()
{
    if( !prepareShapeCreation() )
        return;

    double fLogicRadius = m_pPosHelper->getOuterLogicRadius();

    if( m_aAxisProperties.m_bDisplayLabels )
    {
        
        boost::scoped_ptr< TickFactory > apTickFactory( this->createTickFactory() );

        
        

        EquidistantTickIter aTickIter( m_aAllTickInfos, m_aIncrement, 0, 0 );
        this->updateUnscaledValuesAtTicks( aTickIter );

        removeTextShapesFromTicks();

        AxisLabelProperties aAxisLabelProperties( m_aAxisLabelProperties );
        aAxisLabelProperties.bOverlapAllowed = true;
        double fLogicZ      = 1.0;
        while( !createTextShapes_ForAngleAxis( m_xTextTarget, aTickIter
                        , aAxisLabelProperties
                        , fLogicRadius, fLogicZ
                        ) )
        {
        };

        
    }
}

void VPolarAngleAxis::createShapes()
{
    if( !prepareShapeCreation() )
        return;

    double fLogicRadius = m_pPosHelper->getOuterLogicRadius();
    double fLogicZ      = 1.0;

    
    drawing::PointSequenceSequence aPoints(1);
    VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, m_aAllTickInfos, m_aIncrement, m_aScale, m_pPosHelper, fLogicRadius, fLogicZ );
    uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
            m_xGroupShape_Shapes, aPoints, &m_aAxisProperties.m_aLineProperties );
    
    m_pShapeFactory->setShapeName( xShape, "MarkHandles" );

    
    createLabels();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
