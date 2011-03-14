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
#include <basegfx/numeric/ftools.hxx>

#include "VPolarAngleAxis.hxx"
#include "VPolarGrid.hxx"
#include "ShapeFactory.hxx"
#include "macros.hxx"
#include "NumberFormatterWrapper.hxx"
#include "PolarLabelPositionHelper.hxx"
#include <tools/color.hxx>

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
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
    ShapeFactory aShapeFactory(m_xShapeFactory);

    FixedNumberFormatter aFixedNumberFormatter(
        m_xNumberFormatsSupplier, rAxisLabelProperties.nNumberFormatKey );

    //------------------------------------------------
    //prepare text properties for multipropertyset-interface of shape
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    uno::Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , rAxisLabelProperties.m_aFontReferenceSize );

    uno::Any* pColorAny = PropertyMapper::getValuePointer(aPropValues,aPropNames,C2U("CharColor"));
    sal_Int32 nColor = Color( COL_AUTO ).GetColor();
    if(pColorAny)
        *pColorAny >>= nColor;

    const uno::Sequence< rtl::OUString >* pLabels = m_bUseTextLabels? &m_aTextLabels : 0;

    //------------------------------------------------

    //TickInfo* pLastVisibleNeighbourTickInfo = NULL;
    sal_Int32 nTick = 0;

    for( TickInfo* pTickInfo = rTickIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rTickIter.nextInfo(), nTick++ )
    {
        //don't create labels which does not fit into the rhythm
        if( nTick%rAxisLabelProperties.nRhythm != 0)
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
            sal_Int32 nExtraColor=0;

            rtl::OUString aLabel;
            if(pLabels)
            {
                sal_Int32 nIndex = static_cast< sal_Int32 >(pTickInfo->getUnscaledTickValue()) - 1; //first category (index 0) matches with real number 1.0
                if( nIndex>=0 && nIndex<pLabels->getLength() )
                    aLabel = (*pLabels)[nIndex];
            }
            else
                aLabel = aFixedNumberFormatter.getFormattedString( pTickInfo->getUnscaledTickValue(), nExtraColor, bHasExtraColor );

            if(pColorAny)
                *pColorAny = uno::makeAny(bHasExtraColor?nExtraColor:nColor);

            double fLogicAngle = pTickInfo->getUnscaledTickValue();

            LabelAlignment eLabelAlignment(LABEL_ALIGN_CENTER);
            PolarLabelPositionHelper aPolarLabelPositionHelper(m_pPosHelper,nDimensionCount,xTarget,&aShapeFactory);
            sal_Int32 nScreenValueOffsetInRadiusDirection = m_aAxisLabelProperties.m_aMaximumSpaceForLabels.Height/15;
            awt::Point aAnchorScreenPosition2D( aPolarLabelPositionHelper.getLabelScreenPositionAndAlignmentForLogicValues(
                    eLabelAlignment, fLogicAngle, fLogicRadius, fLogicZ, nScreenValueOffsetInRadiusDirection ));
            LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, eLabelAlignment );

            // #i78696# use mathematically correct rotation now
            const double fRotationAnglePi(rAxisLabelProperties.fRotationAngleDegree * (F_PI / -180.0));

            uno::Any aATransformation = ShapeFactory::makeTransformation( aAnchorScreenPosition2D, fRotationAnglePi );
            rtl::OUString aStackedLabel = ShapeFactory::getStackedString( aLabel, rAxisLabelProperties.bStackCharacters );

            pTickInfo->xTextShape = aShapeFactory.createText( xTarget, aStackedLabel, aPropNames, aPropValues, aATransformation );
        }

        //if NO OVERLAP -> remove overlapping shapes
        //@todo
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
    //todo: really only update the positions

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
        //-----------------------------------------
        //get the transformed screen values for all tickmarks in aAllTickInfos
        std::auto_ptr< TickFactory > apTickFactory( this->createTickFactory() );

        //create tick mark text shapes
        //@todo: iterate through all tick depth wich should be labeled

        EquidistantTickIter aTickIter( m_aAllTickInfos, m_aIncrement, 0, 0 );
        this->updateUnscaledValuesAtTicks( aTickIter );

        removeTextShapesFromTicks();

        AxisLabelProperties aAxisLabelProperties( m_aAxisLabelProperties );
        aAxisLabelProperties.bOverlapAllowed = true;
        double fLogicZ      = 1.0;//as defined
        while( !createTextShapes_ForAngleAxis( m_xTextTarget, aTickIter
                        , aAxisLabelProperties
                        , fLogicRadius, fLogicZ
                        ) )
        {
        };

        //no staggering for polar angle axis
    }
}

void VPolarAngleAxis::createShapes()
{
    if( !prepareShapeCreation() )
        return;

    double fLogicRadius = m_pPosHelper->getOuterLogicRadius();
    double fLogicZ      = 1.0;//as defined

    //-----------------------------------------
    //create axis main lines
    drawing::PointSequenceSequence aPoints(1);
    VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, m_aAllTickInfos, m_aIncrement, m_aScale, m_pPosHelper, fLogicRadius, fLogicZ );
    uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
            m_xGroupShape_Shapes, aPoints, &m_aAxisProperties.m_aLineProperties );
    //because of this name this line will be used for marking the axis
    m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );

    //-----------------------------------------
    //create labels
    createLabels();
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
