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

#include <basegfx/numeric/ftools.hxx>

#include "VPolarAngleAxis.hxx"
#include "VPolarGrid.hxx"
#include <ShapeFactory.hxx>
#include <Axis.hxx>
#include <NumberFormatterWrapper.hxx>
#include <PolarLabelPositionHelper.hxx>
#include <PlottingPositionHelper.hxx>
#include <tools/color.hxx>

#include <memory>

namespace chart
{
using namespace ::com::sun::star;

VPolarAngleAxis::VPolarAngleAxis( const AxisProperties& rAxisProperties
            , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionCount )
            : VPolarAxis( rAxisProperties, xNumberFormatsSupplier, 0/*nDimensionIndex*/, nDimensionCount )
{
}

VPolarAngleAxis::~VPolarAngleAxis()
{
}

void VPolarAngleAxis::createTextShapes_ForAngleAxis(
                       const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                     , EquidistantTickIter& rTickIter
                     , AxisLabelProperties const & rAxisLabelProperties
                     , double fLogicRadius
                     , double fLogicZ )
{
    FixedNumberFormatter aFixedNumberFormatter(
        m_xNumberFormatsSupplier, rAxisLabelProperties.m_nNumberFormatKey );

    //prepare text properties for multipropertyset-interface of shape
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    uno::Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false, -1, false, false );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , rAxisLabelProperties.m_aFontReferenceSize );

    uno::Any* pColorAny = PropertyMapper::getValuePointer(aPropValues,aPropNames,u"CharColor");
    Color nColor = COL_AUTO;
    if(pColorAny)
        *pColorAny >>= nColor;

    const uno::Sequence< OUString >* pLabels = m_bUseTextLabels? &m_aTextLabels : nullptr;

    //TickInfo* pLastVisibleNeighbourTickInfo = NULL;
    sal_Int32 nTick = 0;

    for( TickInfo* pTickInfo = rTickIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rTickIter.nextInfo(), nTick++ )
    {
        //don't create labels which does not fit into the rhythm
        if( nTick%rAxisLabelProperties.m_nRhythm != 0)
            continue;

        //don't create labels for invisible ticks
        if( !pTickInfo->bPaintIt )
            continue;

        //if NO OVERLAP -> don't create labels where the
        //anchor position is the same as for the last label
        //@todo

        if(!pTickInfo->xTextShape.is())
        {
            //create single label
            bool bHasExtraColor=false;
            Color nExtraColor;

            OUString aLabel;
            if(pLabels)
            {
                sal_Int32 nIndex = static_cast< sal_Int32 >(pTickInfo->getUnscaledTickValue()) - 1; //first category (index 0) matches with real number 1.0
                if( nIndex>=0 && nIndex<pLabels->getLength() )
                    aLabel = (*pLabels)[nIndex];
            }
            else
                aLabel = aFixedNumberFormatter.getFormattedString( pTickInfo->getUnscaledTickValue(), nExtraColor, bHasExtraColor );

            if(pColorAny)
                *pColorAny <<= bHasExtraColor?nExtraColor:nColor;

            double fLogicAngle = pTickInfo->getUnscaledTickValue();

            LabelAlignment eLabelAlignment(LABEL_ALIGN_CENTER);
            PolarLabelPositionHelper aPolarLabelPositionHelper(&m_aPosHelper, 2/*nDimensionCount*/, xTarget);
            sal_Int32 nScreenValueOffsetInRadiusDirection = m_aAxisLabelProperties.m_aMaximumSpaceForLabels.Height/15;
            awt::Point aAnchorScreenPosition2D( aPolarLabelPositionHelper.getLabelScreenPositionAndAlignmentForLogicValues(
                    eLabelAlignment, fLogicAngle, fLogicRadius, fLogicZ, nScreenValueOffsetInRadiusDirection ));
            LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, eLabelAlignment );

            // #i78696# use mathematically correct rotation now
            const double fRotationAnglePi(-basegfx::deg2rad(rAxisLabelProperties.m_fRotationAngleDegree));

            uno::Any aATransformation = ShapeFactory::makeTransformation( aAnchorScreenPosition2D, fRotationAnglePi );
            OUString aStackedLabel = ShapeFactory::getStackedString( aLabel, rAxisLabelProperties.m_bStackCharacters );

            pTickInfo->xTextShape = ShapeFactory::createText( xTarget, aStackedLabel, aPropNames, aPropValues, aATransformation );
        }

        //if NO OVERLAP -> remove overlapping shapes
        //@todo
    }
}

void VPolarAngleAxis::createMaximumLabels()
{
    if( !prepareShapeCreation() )
        return;

    createLabels();
}

void VPolarAngleAxis::updatePositions()
{
    //todo: really only update the positions

    if( !prepareShapeCreation() )
        return;

    createLabels();
}

void VPolarAngleAxis::createLabels()
{
    if( !prepareShapeCreation() )
        return;

    double fLogicRadius = m_aPosHelper.getOuterLogicRadius();

    if( !m_aAxisProperties.m_bDisplayLabels )
        return;

    //create tick mark text shapes
    //@todo: iterate through all tick depth which should be labeled

    EquidistantTickIter aTickIter( m_aAllTickInfos, m_aIncrement, 0 );
    updateUnscaledValuesAtTicks( aTickIter );

    removeTextShapesFromTicks();

    AxisLabelProperties aAxisLabelProperties( m_aAxisLabelProperties );
    aAxisLabelProperties.m_bOverlapAllowed = true;
    double const fLogicZ = 1.0;//as defined
    createTextShapes_ForAngleAxis( m_xTextTarget, aTickIter
                    , aAxisLabelProperties
                    , fLogicRadius, fLogicZ
                    );

    //no staggering for polar angle axis
}

void VPolarAngleAxis::createShapes()
{
    if( !prepareShapeCreation() )
        return;

    double fLogicRadius = m_aPosHelper.getOuterLogicRadius();
    double const fLogicZ = 1.0;//as defined

    //create axis main lines
    drawing::PointSequenceSequence aPoints(1);
    VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, m_aAllTickInfos, m_aIncrement, m_aScale, &m_aPosHelper, fLogicRadius, fLogicZ );
    rtl::Reference<SvxShapePolyPolygon> xShape = ShapeFactory::createLine2D(
            m_xGroupShape_Shapes, aPoints, &m_aAxisProperties.m_aLineProperties );
    //because of this name this line will be used for marking the axis
    ::chart::ShapeFactory::setShapeName( xShape, "MarkHandles" );

    //create labels
    createLabels();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
