/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarAxis.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:40:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "VPolarAxis.hxx"
#include "VPolarGrid.hxx"
#include "VCartesianAxis.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "TickmarkHelper.hxx"
#include "PropertyMapper.hxx"
#include "chartview/NumberFormatterWrapper.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "PolarLabelPositionHelper.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif

#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif

#include <algorithm>
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;

VPolarAxis::VPolarAxis( const AxisProperties& rAxisProperties
            , NumberFormatterWrapper* pNumberFormatterWrapper
            , sal_Int32 nDimensionCount )
            : VMeterBase( uno::Reference<XMeter>::query(rAxisProperties.m_xAxisModel)
                , nDimensionCount )
            , m_aAxisProperties( rAxisProperties )
            , m_pNumberFormatterWrapper( pNumberFormatterWrapper )
            , m_pPosHelper( new PolarPlottingPositionHelper(false) )
            , m_aIncrements()
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

VPolarAxis::~VPolarAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VPolarAxis::setIncrements( const uno::Sequence< ExplicitIncrementData >& rIncrements )
{
    m_aIncrements = rIncrements;
}

bool createTextShapes_ForAngleAxis( const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory
                     , const uno::Reference< drawing::XShapes >& xTarget
                     , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                     , const ExplicitIncrementData& rIncrement
                     , AxisLabelProperties& rAxisLabelProperties
                     , const AxisProperties& rAxisProperties
                     , PolarPlottingPositionHelper* pPosHelper
                     , double fLogicRadius
                     , double fLogicZ
                     , const FixedNumberFormatter& rFixedNumberFormatter )
{
    sal_Int32 nDimensionCount = 2;
    ShapeFactory aShapeFactory(xShapeFactory);

    //------------------------------------------------
    //prepare text properties for multipropertyset-interface of shape
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    uno::Reference< beans::XPropertySet > xProps( rAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , rAxisProperties.m_aReferenceSize );

    uno::Any* pColorAny = PropertyMapper::getValuePointer(aPropValues,aPropNames,C2U("CharColor"));
    sal_Int32 nColor = Color( COL_AUTO ).GetColor();
    if(pColorAny)
        *pColorAny >>= nColor;
    //------------------------------------------------

    //TickInfo* pLastVisibleNeighbourTickInfo = NULL;
    sal_Int32 nTick = 0;
    TickIter aIter( rAllTickInfos, rIncrement, 0, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        //don't create labels which does not fit into the rythm
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
            rtl::OUString aLabel = rFixedNumberFormatter.getFormattedString( pTickInfo->fUnscaledTickValue, nExtraColor, bHasExtraColor );
            if(pColorAny)
                *pColorAny = uno::makeAny(bHasExtraColor?nExtraColor:nColor);

            double fLogicAngle = pTickInfo->fUnscaledTickValue;

            LabelAlignment eLabelAlignment(LABEL_ALIGN_CENTER);
            PolarLabelPositionHelper aPolarLabelPositionHelper(pPosHelper,nDimensionCount,xTarget,&aShapeFactory);
            awt::Point aAnchorScreenPosition2D( aPolarLabelPositionHelper.getLabelScreenPositionAndAlignment(
                    eLabelAlignment, true, fLogicAngle, fLogicAngle, fLogicRadius, fLogicRadius, fLogicZ ));
            LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, eLabelAlignment );

            double fRotationAnglePi = rAxisLabelProperties.fRotationAngleDegree*F_PI/180.0;
            uno::Any aATransformation = ShapeFactory::makeTransformation( aAnchorScreenPosition2D, fRotationAnglePi );
            rtl::OUString aStackedLabel = ShapeFactory::getStackedString( aLabel, rAxisLabelProperties.bStackCharacters );

            pTickInfo->xTextShape = aShapeFactory.createText( xTarget, aStackedLabel, aPropNames, aPropValues, aATransformation );
        }

        //if NO OVERLAP -> remove overlapping shapes
        //@todo
    }
    return true;
}

void VPolarAxis::create2DAngleAxis( const uno::Reference< drawing::XShapes >& xTarget, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos )
{
    double fLogicRadius = m_pPosHelper->getOuterLogicRadius();
    double fLogicZ      = -0.5;//as defined

    //-----------------------------------------
    //create axis main lines
    drawing::PointSequenceSequence aPoints(1);
    VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, rAllTickInfos, m_aIncrement, m_aScale, m_pPosHelper, fLogicRadius, fLogicZ );
    uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
            xTarget, aPoints, m_aAxisProperties.m_aLineProperties );
    //because of this name this line will be used for marking the axis
    m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );

    //-----------------------------------------
    //create labels
    if( m_aAxisProperties.m_bDisplayLabels )
    {
        AxisLabelProperties aAxisLabelProperties;
        aAxisLabelProperties.init(m_aAxisProperties.m_xAxisModel);

        FixedNumberFormatter aFixedNumberFormatter(
                m_pNumberFormatterWrapper, aAxisLabelProperties.aNumberFormat );

        while( !createTextShapes_ForAngleAxis( m_xShapeFactory, xTarget, rAllTickInfos
                        , m_aIncrement, aAxisLabelProperties, m_aAxisProperties, m_pPosHelper
                        , fLogicRadius, fLogicZ
                        , aFixedNumberFormatter
                        ) )
        {
        };

        //no staggering for polar angle axis
    }
}

void VPolarAxis::create2DRadiusAxis( const uno::Reference< drawing::XShapes >& xTarget, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos )
{
    const ExplicitScaleData& rAngleScale         = m_pPosHelper->getScales()[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];

    ::std::vector< ::std::vector< TickInfo > > aAngleTickInfos;
    TickmarkHelper aAngleTickmarkHelper( rAngleScale, rAngleIncrement );
    aAngleTickmarkHelper.getAllTicks( aAngleTickInfos );

    uno::Reference< XScaling > xInverseScaling( NULL );
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    AxisProperties aAxisProperties(m_aAxisProperties);
    aAxisProperties.m_fInnerDirectionSign=0.0;
    aAxisProperties.m_bLabelsOutside=true;
    aAxisProperties.m_bIsMainAxis=false;
    aAxisProperties.m_aLabelAlignment=LABEL_ALIGN_RIGHT;
    aAxisProperties.init();

    sal_Int32 nTick = 0;
    TickIter aIter( aAngleTickInfos, rAngleIncrement, 0, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        pTickInfo->updateUnscaledValue( xInverseScaling );
        aAxisProperties.m_pfMainLinePositionAtOtherAxis = new double( pTickInfo->fUnscaledTickValue );
        if(nTick)
            aAxisProperties.m_bDisplayLabels=false;

        //-------------------
        VCartesianAxis aAxis(aAxisProperties,m_pNumberFormatterWrapper
            ,2,new PolarPlottingPositionHelper(false));
        aAxis.setMeterData( m_aScale, m_aIncrement );
        aAxis.init(m_xLogicTarget,m_xFinalTarget,m_xShapeFactory);
        aAxis.setTransformationSceneToScreen( Matrix4DToHomogenMatrix( m_aMatrixScreenToScene ) );
        aAxis.setScales( m_pPosHelper->getScales() );
        aAxis.createShapes();
    }
}

void SAL_CALL VPolarAxis::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //-----------------------------------------
    //create named group shape
    uno::Reference< XIdentifiable > xIdent( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    DBG_ASSERT( xIdent.is(), "Axis should support XIdentifiable" );
    if( ! xIdent.is())
        return;
    uno::Reference< drawing::XShapes > xGroupShape_Shapes(
        m_pShapeFactory->createGroup2D( m_xLogicTarget
        , ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_AXIS, xIdent->getIdentifier() )
        ) );

    //-----------------------------------------
    //create all scaled tickmark values
    std::auto_ptr< TickmarkHelper > apTickmarkHelper( this->createTickmarkHelper() );
    ::std::vector< ::std::vector< TickInfo > > aAllTickInfos;
    apTickmarkHelper->getAllTicks( aAllTickInfos );

    //-----------------------------------------
    //create different axes
    if(2==m_nDimension)
    {
        sal_Int32 nDimensionIndex = m_xMeter->getRepresentedDimension();
        if(nDimensionIndex==0)
            this->create2DAngleAxis( xGroupShape_Shapes, aAllTickInfos );
        else if(nDimensionIndex==1)
            this->create2DRadiusAxis( xGroupShape_Shapes, aAllTickInfos );

    }
}

//.............................................................................
} //namespace chart
//.............................................................................
