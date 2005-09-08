/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCartesianAxis.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:38:01 $
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
#include "VCartesianAxis.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "PropertyMapper.hxx"
#include "chartview/NumberFormatterWrapper.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "LabelPositionHelper.hxx"
#include "DoubleRectangle.hxx"

#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE2_HPP_
#include <com/sun/star/text/WritingMode2.hpp>
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

void lcl_correctRotation_Left( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct label positions left of an axis with right centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = -aSize.Height*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfXCorrection  = -aSize.Width *rtl::math::sin( beta )
            -aSize.Height *rtl::math::cos( beta )/2.0;
        rfYCorrection = -aSize.Width *rtl::math::cos( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - F_PI;
        rfXCorrection  = -aSize.Width *rtl::math::cos( beta )
            -aSize.Height*rtl::math::sin( beta )/2.0;
        rfYCorrection = aSize.Width *rtl::math::sin( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection = -aSize.Height*rtl::math::sin( beta )/2.0;

    }
}

void lcl_correctRotation_Right( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct label positions right of an axis with left centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree== 0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Height*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        rfXCorrection  = aSize.Width *rtl::math::cos( beta )
            + aSize.Height*rtl::math::sin( beta )/2.0;
        rfYCorrection  = aSize.Width *rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = aSize.Width *rtl::math::sin( beta )
                    +aSize.Height*rtl::math::cos( beta )/2.0;
        rfYCorrection  = -aSize.Width *rtl::math::cos( beta );
    }
    else
    {
        rfXCorrection  = aSize.Height*rtl::math::sin( 2*F_PI - fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Top( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct label positions on top of an axis with bottom centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree== 0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = aSize.Width*rtl::math::cos( fAnglePi )/2.0
            +aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        rfYCorrection = -aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi - F_PI/2.0;
        rfYCorrection = -aSize.Width*rtl::math::cos( beta )/2.0
            - aSize.Height*rtl::math::sin( beta );
        rfXCorrection = - aSize.Width*rtl::math::sin( beta )/2.0
            + aSize.Height*rtl::math::cos( beta )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - F_PI;
        rfXCorrection = +aSize.Width *rtl::math::cos( beta )/2.0
            -aSize.Height *rtl::math::sin( beta )/2.0; ;
        rfYCorrection = -aSize.Width *rtl::math::sin( beta )/2.0
            -aSize.Height *rtl::math::cos( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection = -aSize.Width*rtl::math::cos( fAnglePi )/2.0
            +aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Bottom( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct label positions below of an axis with top centered alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = -aSize.Width *rtl::math::cos( fAnglePi )/2.0
                         -aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        rfYCorrection = aSize.Width*rtl::math::sin( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = fAnglePi-F_PI/2.0;
        rfYCorrection = aSize.Width *rtl::math::cos( beta )/2.0
            +aSize.Height*rtl::math::sin( beta );
        rfXCorrection  = aSize.Width *rtl::math::sin( beta )/2.0
                    -aSize.Height*rtl::math::cos( beta )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = -aSize.Width *rtl::math::sin( beta )/2.0
                         +aSize.Height*rtl::math::cos( beta )/2.0;
        rfYCorrection = aSize.Height*rtl::math::sin( beta )
                        +aSize.Width*rtl::math::cos( beta )/2.0;
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection  = aSize.Height*rtl::math::sin( beta )/2.0
                        +aSize.Width*rtl::math::cos( beta )/2.0;
        rfYCorrection = aSize.Width*rtl::math::sin( beta )/2.0;
    }
}

void lcl_getPositionCorrectionForRotation(
                  double& rfXCorrection
                , double& rfYCorrection
                , LabelAlignment eLabelAlignment
                , const double fRotationAngle
                , const awt::Size& aSize )
{
    double fAnglePositiveDegree = fRotationAngle;
    while(fAnglePositiveDegree<0.0)
        fAnglePositiveDegree+=360.0;

    switch(eLabelAlignment)
    {
        case LABEL_ALIGN_LEFT:
        case LABEL_ALIGN_LEFT_TOP:
        case LABEL_ALIGN_LEFT_BOTTOM:
            lcl_correctRotation_Left( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
            break;
        case LABEL_ALIGN_RIGHT:
        case LABEL_ALIGN_RIGHT_TOP:
        case LABEL_ALIGN_RIGHT_BOTTOM:
            lcl_correctRotation_Right( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
            break;
        case LABEL_ALIGN_TOP:
            lcl_correctRotation_Top( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
            break;
        case LABEL_ALIGN_BOTTOM:
            lcl_correctRotation_Bottom( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
        default: //LABEL_ALIGN_CENTER
            break;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VCartesianAxis::VCartesianAxis( const AxisProperties& rAxisProperties
            , NumberFormatterWrapper* pNumberFormatterWrapper
            , sal_Int32 nDimensionCount
            , PlottingPositionHelper* pPosHelper )//takes ownership
            : VMeterBase( uno::Reference<XMeter>::query(rAxisProperties.m_xAxisModel)
                , nDimensionCount )
            , m_aAxisProperties( rAxisProperties )
            , m_pNumberFormatterWrapper( pNumberFormatterWrapper )
{
    if( pPosHelper )
        m_pPosHelper = pPosHelper;
    else
        m_pPosHelper = new PlottingPositionHelper();
}

VCartesianAxis::~VCartesianAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uno::Reference< drawing::XShape > createSingleLabel(
            const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
          , const uno::Reference< drawing::XShapes >& xTarget
          , const awt::Point& rAnchorScreenPosition2D
          , const rtl::OUString& rLabel
          , const AxisLabelProperties& rAxisLabelProperties
          , const AxisProperties& rAxisProperties
          , const tNameSequence& rPropNames
          , const tAnySequence& rPropValues
          )
{
    double fRotationAnglePi = rAxisLabelProperties.fRotationAngleDegree*F_PI/180.0;
    uno::Any aATransformation = ShapeFactory::makeTransformation( rAnchorScreenPosition2D, fRotationAnglePi );
    rtl::OUString aLabel = ShapeFactory::getStackedString( rLabel, rAxisLabelProperties.bStackCharacters );

    uno::Reference< drawing::XShape > xShape2DText = ShapeFactory(xShapeFactory)
                    .createText( xTarget, aLabel, rPropNames, rPropValues, aATransformation );
    //-------------
    //correctPositionForRotation
    awt::Point aOldPos = xShape2DText->getPosition();
    awt::Size  aSize   = xShape2DText->getSize();

    double fYCorrection = 0.0;
    double fXCorrection  = 0.0;

    lcl_getPositionCorrectionForRotation( fXCorrection, fYCorrection
        , rAxisProperties.m_aLabelAlignment
        , rAxisLabelProperties.fRotationAngleDegree, aSize );

    xShape2DText->setPosition( awt::Point(
          static_cast<sal_Int32>(aOldPos.X + fXCorrection  )
        , static_cast<sal_Int32>(aOldPos.Y + fYCorrection ) ) );
    //-------------

    return xShape2DText;
}

bool lcl_doesOverlap( const uno::Reference< drawing::XShape >& xShape
        , const Vector2D& rTickScreenPosition )
{
    if(!xShape.is())
        return false;

    DoublePoint aP(rTickScreenPosition.X(),rTickScreenPosition.Y());
    DoubleRectangle aRect(xShape->getPosition(),xShape->getSize());
    return aRect.isInside(aP);
}

bool doesOverlap( const uno::Reference< drawing::XShape >& xShape1
                , const uno::Reference< drawing::XShape >& xShape2 )
{
    DoubleRectangle aRect1(xShape1->getPosition(),xShape1->getSize());
    DoubleRectangle aRect2(xShape2->getPosition(),xShape2->getSize());
    return aRect1.isOverlap(aRect2);
}

void removeShapesAtWrongRythm( TickIter& rIter
                              , sal_Int32 nCorrectRhythm
                              , sal_Int32 nMaxTickToCheck
                              , const uno::Reference< drawing::XShapes >& xTarget )
{
    sal_Int32 nTick = 0;
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo && nTick <= nMaxTickToCheck
        ; pTickInfo = rIter.nextInfo(), nTick++ )
    {
        //remove labels which does not fit into the rythm
        if( nTick%nCorrectRhythm != 0)
        {
            if(pTickInfo->xTextShape.is())
            {
                xTarget->remove(pTickInfo->xTextShape);
                pTickInfo->xTextShape = NULL;
            }
        }
    }
}

class LabelIterator : private TickIter
{
    //this Iterator iterates over existing text labels

    //if the labels are staggered and bInnerLine is true
    //we iterate only through the labels which are lying more inside the diagram

    //if the labels are staggered and bInnerLine is false
    //we iterate only through the labels which are lying more outside the diagram

    //if the labels are not staggered
    //we iterate through all labels

public:
    LabelIterator( ::std::vector< ::std::vector< TickInfo > >& rTickInfos
            , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement
            , const AxisLabelStaggering eAxisLabelStaggering
            , bool bInnerLine
            , sal_Int32 nMinDepth=0, sal_Int32 nMaxDepth=-1 );

    TickInfo*   firstInfo();
    TickInfo*   nextInfo();

private: //methods
    LabelIterator();

private: //member
    const AxisLabelStaggering   m_eAxisLabelStaggering;
    bool        m_bInnerLine;
};

LabelIterator::LabelIterator( ::std::vector< ::std::vector< TickInfo > >& rTickInfos
            , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement
            , const AxisLabelStaggering eAxisLabelStaggering
            , bool bInnerLine
            , sal_Int32 nMinDepth, sal_Int32 nMaxDepth )
            : TickIter( rTickInfos, rIncrement, nMinDepth, nMaxDepth )
            , m_eAxisLabelStaggering(eAxisLabelStaggering)
            , m_bInnerLine(bInnerLine)
{
}

TickInfo* LabelIterator::firstInfo()
{
    TickInfo* pTickInfo = TickIter::firstInfo();
    while( pTickInfo && !pTickInfo->xTextShape.is() )
        pTickInfo = TickIter::nextInfo();
    if(!pTickInfo)
        return NULL;
    if( (STAGGER_EVEN==m_eAxisLabelStaggering && !m_bInnerLine)
        ||
        (STAGGER_ODD==m_eAxisLabelStaggering && m_bInnerLine)
        )
    {
        //skip first label
        do
            pTickInfo = TickIter::nextInfo();
        while( pTickInfo && !pTickInfo->xTextShape.is() );
    }
    if(!pTickInfo)
        return NULL;
    return pTickInfo;
}

TickInfo* LabelIterator::nextInfo()
{
    TickInfo* pTickInfo = NULL;
    //get next label
    do
        pTickInfo = TickIter::nextInfo();
    while( pTickInfo && !pTickInfo->xTextShape.is() );

    if(  STAGGER_EVEN==m_eAxisLabelStaggering
      || STAGGER_ODD==m_eAxisLabelStaggering )
    {
        //skip one label
        do
            pTickInfo = TickIter::nextInfo();
        while( pTickInfo && !pTickInfo->xTextShape.is() );
    }
    return pTickInfo;
}

Vector2D lcl_getStaggerDistance( LabelIterator& rIter, const Vector2D& rStaggerDirection )
{
    //calculates the height or width of the first line of labels
    //thus the second line of labels needs to be shifted for that distance

    sal_Int32 nDistance=0;
    uno::Reference< drawing::XShape >  xShape2DText(NULL);
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rIter.nextInfo() )
    {
        xShape2DText = pTickInfo->xTextShape;
        DBG_ASSERT(xShape2DText.is(),"LabelIterator does not work correctly");

        awt::Size aSize  = xShape2DText->getSize();
        if(rStaggerDirection.X()>rStaggerDirection.Y())
            nDistance = ::std::max(nDistance,aSize.Width);
        else
            nDistance = ::std::max(nDistance,aSize.Height);
    }
    return rStaggerDirection*nDistance;
}

void lcl_correctPositionForStaggering( LabelIterator& rIter, const Vector2D& rStaggerDistance )
{
    if(rStaggerDistance.GetLength()==0.0)
        return;
    uno::Reference< drawing::XShape >  xShape2DText(NULL);
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rIter.nextInfo() )
    {
        xShape2DText = pTickInfo->xTextShape;
        DBG_ASSERT(xShape2DText.is(),"LabelIterator does not work correctly");

        awt::Point aPos  = xShape2DText->getPosition();
        aPos.X += static_cast<sal_Int32>(rStaggerDistance.X());
        aPos.Y += static_cast<sal_Int32>(rStaggerDistance.Y());
        xShape2DText->setPosition( aPos );
    }
}

bool VCartesianAxis::createTextShapes(
                       const uno::Reference< drawing::XShapes >& xTarget
                     , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                     , AxisLabelProperties& rAxisLabelProperties
                     , TickmarkHelper_2D* pTickmarkHelper )
{
    //returns true if the text shapes have been created succesfully
    //otherwise false - in this case the AxisLabelProperties have changed
    //and contain new instructions for the next try for text shape creation

    uno::Reference< XScaling > xInverseScaling( NULL );
    if( m_aScale.Scaling.is() )
        xInverseScaling = m_aScale.Scaling->getInverseScaling();

    FixedNumberFormatter aFixedNumberFormatter(
                m_pNumberFormatterWrapper, rAxisLabelProperties.aNumberFormat );

    Vector2D aTextToTickDistance( pTickmarkHelper->getDistanceTickToText( m_aAxisProperties ) );

    //@todo: iterate through all tick depth wich should be labeled
    TickIter aIter( rAllTickInfos, m_aIncrement, 0, 0 );
    TickInfo* pPreviousVisibleTickInfo = NULL;
    TickInfo* pPREPreviousVisibleTickInfo = NULL;
    TickInfo* pLastVisibleNeighbourTickInfo = NULL;
    bool bIsStaggered = rAxisLabelProperties.getIsStaggered();
    sal_Int32 nLimitedSpaceForStaggering = -1;
    if( bIsStaggered && rAxisLabelProperties.bLineBreakAllowed
        && rAxisLabelProperties.fRotationAngleDegree == 0.0 //use line break only, if labels are not rotated
        )
    {
        nLimitedSpaceForStaggering = 2*TickmarkHelper_2D::getTickScreenDistance( aIter );
        if( nLimitedSpaceForStaggering > 0 )
        { //reduce space for a small amount to have a visible distance between the labels:
            sal_Int32 nReduce = (nLimitedSpaceForStaggering*5)/100;
            if(!nReduce)
                nReduce = 1;
            nLimitedSpaceForStaggering -= nReduce;
        }
        //maybe @todo in future:
        //if the labeled tickmarks are not equidistant [this is not considered to be a relevant case so far]
        //the limited space maybe needs to be calculated for each tickmark seperatly
        //or the staggering could be ignored in that case
    }

    //------------------------------------------------
    //prepare properties for multipropertyset-interface of shape
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    uno::Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false
        , nLimitedSpaceForStaggering, fabs(aTextToTickDistance.X()) > fabs(aTextToTickDistance.Y()) );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , m_aAxisProperties.m_aReferenceSize );
    LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, m_aAxisProperties.m_aLabelAlignment );

    uno::Any* pColorAny = PropertyMapper::getValuePointer(aPropValues,aPropNames,C2U("CharColor"));
    sal_Int32 nColor = Color( COL_AUTO ).GetColor();
    if(pColorAny)
        *pColorAny >>= nColor;
    //------------------------------------------------

    sal_Int32 nTick = 0;
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        pLastVisibleNeighbourTickInfo = bIsStaggered ?
                    pPREPreviousVisibleTickInfo : pPreviousVisibleTickInfo;

        //don't create labels which does not fit into the rythm
        if( nTick%rAxisLabelProperties.nRhythm != 0)
            continue;

        //don't create labels for invisible ticks
        if( !pTickInfo->bPaintIt )
            continue;

        //if NO OVERLAP -> don't create labels where the tick overlaps
        //with the text of the last neighbour tickmark
        if( pLastVisibleNeighbourTickInfo && !rAxisLabelProperties.bOverlapAllowed )
        {
            if( lcl_doesOverlap( pLastVisibleNeighbourTickInfo->xTextShape, pTickInfo->aTickScreenPosition ) )
            {
                if( rAxisLabelProperties.bRhythmIsFix )
                    continue;
                rAxisLabelProperties.nRhythm++;
                TickIter aRemoveIter( rAllTickInfos, m_aIncrement, 0, 0 );
                removeShapesAtWrongRythm( aRemoveIter, rAxisLabelProperties.nRhythm, nTick, xTarget );
                return false;
            }
        }

        pTickInfo->updateUnscaledValue( xInverseScaling );

        bool bHasExtraColor=false;
        sal_Int32 nExtraColor=0;
        rtl::OUString aLabel = aFixedNumberFormatter.getFormattedString( pTickInfo->fUnscaledTickValue, nExtraColor, bHasExtraColor );
        if(pColorAny)
            *pColorAny = uno::makeAny(bHasExtraColor?nExtraColor:nColor);

        Vector2D aTickScreenPos2D( pTickInfo->aTickScreenPosition );
        aTickScreenPos2D += aTextToTickDistance;
        awt::Point aAnchorScreenPosition2D(
            static_cast<sal_Int32>(aTickScreenPos2D.X())
            ,static_cast<sal_Int32>(aTickScreenPos2D.Y()));

        //create single label
        if(!pTickInfo->xTextShape.is())
            pTickInfo->xTextShape = createSingleLabel( m_xShapeFactory, xTarget
                                    , aAnchorScreenPosition2D, aLabel
                                    , rAxisLabelProperties, m_aAxisProperties
                                    , aPropNames, aPropValues );

        //if NO OVERLAP -> remove overlapping shapes
        if( pLastVisibleNeighbourTickInfo && !rAxisLabelProperties.bOverlapAllowed )
        {
            if( doesOverlap( pLastVisibleNeighbourTickInfo->xTextShape, pTickInfo->xTextShape ) )
            {
                if( rAxisLabelProperties.bRhythmIsFix )
                {
                    xTarget->remove(pTickInfo->xTextShape);
                    pTickInfo->xTextShape = NULL;
                    continue;
                }
                rAxisLabelProperties.nRhythm++;
                TickIter aRemoveIter( rAllTickInfos, m_aIncrement, 0, 0 );
                removeShapesAtWrongRythm( aRemoveIter, rAxisLabelProperties.nRhythm, nTick, xTarget );
                return false;
            }
        }

        pPREPreviousVisibleTickInfo = pPreviousVisibleTickInfo;
        pPreviousVisibleTickInfo = pTickInfo;
    }
    return true;
}

drawing::PointSequenceSequence lcl_makePointSequence( Vector2D& rStart, Vector2D& rEnd )
{
    drawing::PointSequenceSequence aPoints(1);
    aPoints[0].realloc(2);
    aPoints[0][0].X = static_cast<sal_Int32>(rStart.X());
    aPoints[0][0].Y = static_cast<sal_Int32>(rStart.Y());
    aPoints[0][1].X = static_cast<sal_Int32>(rEnd.X());
    aPoints[0][1].Y = static_cast<sal_Int32>(rEnd.Y());
    return aPoints;
}

double VCartesianAxis::getLogicValueWhereMainLineCrossesOtherAxis() const
{
    sal_Int32 nDimensionIndex = m_aAxisProperties.m_xAxisModel->getRepresentedDimension();
    double fMin = (nDimensionIndex==1) ? m_pPosHelper->getLogicMinX() : m_pPosHelper->getLogicMinY();
    double fMax = (nDimensionIndex==1) ? m_pPosHelper->getLogicMaxX() : m_pPosHelper->getLogicMaxY();

    double fCrossesOtherAxis;
    if(m_aAxisProperties.m_pfMainLinePositionAtOtherAxis)
        fCrossesOtherAxis = *m_aAxisProperties.m_pfMainLinePositionAtOtherAxis;
    else
    {
        bool bMinimumForLeftAxis = ( (nDimensionIndex==1) && m_pPosHelper->isMathematicalOrientationY() )
                            || ( (nDimensionIndex!=1) && m_pPosHelper->isMathematicalOrientationX() );
        fCrossesOtherAxis = ( bMinimumForLeftAxis && m_aAxisProperties.m_bIsMainAxis ) ? fMin : fMax;
    }
    return fCrossesOtherAxis;
}
bool VCartesianAxis::getLogicValueWhereExtraLineCrossesOtherAxis( double& fCrossesOtherAxis ) const
{
    if( !m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        return false;
    sal_Int32 nDimensionIndex = m_aAxisProperties.m_xAxisModel->getRepresentedDimension();
    double fMin = (nDimensionIndex==1) ? m_pPosHelper->getLogicMinX() : m_pPosHelper->getLogicMinY();
    double fMax = (nDimensionIndex==1) ? m_pPosHelper->getLogicMaxX() : m_pPosHelper->getLogicMaxY();
    if( *m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis <= fMin
        || *m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis >= fMax )
        return false;
    fCrossesOtherAxis = *m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis;
    return true;
}
void VCartesianAxis::get2DAxisMainLine( Vector2D& rStart, Vector2D& rEnd, double fCrossesOtherAxis ) const
{
    sal_Int32 nDimensionIndex = m_aAxisProperties.m_xAxisModel->getRepresentedDimension();
    double fXStart = (nDimensionIndex==1) ? fCrossesOtherAxis : m_pPosHelper->getLogicMinX();
    double fYStart = (nDimensionIndex==1) ? m_pPosHelper->getLogicMinY() : fCrossesOtherAxis;
    drawing::Position3D aSceneStart = m_pPosHelper->transformLogicToScene( fXStart, fYStart, 0, true );

    double fXEnd = (nDimensionIndex==1) ? fCrossesOtherAxis : m_pPosHelper->getLogicMaxX();
    double fYEnd = (nDimensionIndex==1) ? m_pPosHelper->getLogicMaxY() : fCrossesOtherAxis;
    drawing::Position3D aSceneEnd = m_pPosHelper->transformLogicToScene( fXEnd, fYEnd, 0, true );

    rStart.X() = aSceneStart.PositionX;
    rStart.Y() = aSceneStart.PositionY;
    rEnd.X() = aSceneEnd.PositionX;
    rEnd.Y() = aSceneEnd.PositionY;
}

void SAL_CALL VCartesianAxis::createShapes()
{
    if(2!=m_nDimension) //@todo remove this restriction if 3D axes are available
        return;

    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    Vector2D aStart, aEnd;
    this->get2DAxisMainLine( aStart, aEnd, this->getLogicValueWhereMainLineCrossesOtherAxis() );
    std::auto_ptr< TickmarkHelper_2D > apTickmarkHelper(
        new TickmarkHelper_2D( m_aScale, m_aIncrement, aStart, aEnd ) );

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
    ::std::vector< ::std::vector< TickInfo > > aAllTickInfos;
    apTickmarkHelper->getAllTicks( aAllTickInfos );

    //-----------------------------------------
    //create tick mark line shapes
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = aAllTickInfos.begin();
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = aAllTickInfos.end();

    if(aDepthIter == aDepthEnd)//no tickmarks at all
        return;

    sal_Int32 nTickmarkPropertiesCount = m_aAxisProperties.m_aTickmarkPropertiesList.size();
    for( sal_Int32 nDepth=0
        ; aDepthIter != aDepthEnd && nDepth < nTickmarkPropertiesCount
        ; aDepthIter++, nDepth++ )
    {
        const TickmarkProperties& rTickmarkProperties = m_aAxisProperties.m_aTickmarkPropertiesList[nDepth];

        sal_Int32 nPointCount = (*aDepthIter).size();
        drawing::PointSequenceSequence aPoints(nPointCount);

        ::std::vector< TickInfo >::const_iterator       aTickIter = (*aDepthIter).begin();
        const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
        sal_Int32 nN = 0;
        for( ; aTickIter != aTickEnd; aTickIter++ )
        {
            if( !(*aTickIter).bPaintIt )
                continue;
            apTickmarkHelper->addPointSequenceForTickLine( aPoints, nN, (*aTickIter).fScaledTickValue
                , m_aAxisProperties.m_fInnerDirectionSign, rTickmarkProperties );
            nN++;
        }
        aPoints.realloc(nN);
        m_pShapeFactory->createLine2D( xGroupShape_Shapes, aPoints
                                     , rTickmarkProperties.aLineProperties );
    }
    //-----------------------------------------
    //create axis main lines
    if(2==m_nDimension)
    {
        //TickmarkHelper_2D& aTickmarkHelper = *dynamic_cast<TickmarkHelper_2D*>(apTickmarkHelper.get());
        TickmarkHelper_2D& aTickmarkHelper = *reinterpret_cast<TickmarkHelper_2D*>(apTickmarkHelper.get());
        //create axis line
        //it serves also as the handle shape for the axis selection
        {
            drawing::PointSequenceSequence aPoints(1);
            aTickmarkHelper.createPointSequenceForAxisMainLine( aPoints );
            uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                    xGroupShape_Shapes, aPoints
                    , m_aAxisProperties.m_aLineProperties );
            //because of this name this line will be used for marking the axis
            m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
        }
        //-----------------------------------------
        //create an additional line at NULL
        double fExtraLineCrossesOtherAxis;
        if( getLogicValueWhereExtraLineCrossesOtherAxis(fExtraLineCrossesOtherAxis) )
        {
            this->get2DAxisMainLine( aStart, aEnd, fExtraLineCrossesOtherAxis );
            drawing::PointSequenceSequence aPoints( lcl_makePointSequence(aStart,aEnd) );
            uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                    xGroupShape_Shapes, aPoints, m_aAxisProperties.m_aLineProperties );
        }
    }
    //-----------------------------------------
    //create labels
    if( m_aAxisProperties.m_bDisplayLabels )
    {
        AxisLabelProperties aAxisLabelProperties;
        aAxisLabelProperties.init(m_aAxisProperties.m_xAxisModel);

        //create tick mark text shapes
        while( !createTextShapes( xGroupShape_Shapes, aAllTickInfos
                        , aAxisLabelProperties, apTickmarkHelper.get()
                        ) )
        {
        };

        //staggering
        if( aAxisLabelProperties.getIsStaggered() )
        {
            LabelIterator aInnerIter( aAllTickInfos, m_aIncrement
                , aAxisLabelProperties.eStaggering, true, 0, 0 );
            LabelIterator aOuterIter( aAllTickInfos, m_aIncrement
                , aAxisLabelProperties.eStaggering, false, 0, 0 );

            Vector2D aStaggerDirection( apTickmarkHelper->getDistanceTickToText( m_aAxisProperties ) );
            aStaggerDirection.Normalize();

            lcl_correctPositionForStaggering( aOuterIter
                , lcl_getStaggerDistance( aInnerIter
                    , aStaggerDirection ) );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
