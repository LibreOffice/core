/*************************************************************************
 *
 *  $RCSfile: VCartesianAxis.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2004-01-17 13:09:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
#include "LabelPositionHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <drafts/com/sun/star/chart2/XIdentifiable.hpp>
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
using namespace ::drafts::com::sun::star::chart2;
using namespace ::rtl::math;

LabelAlignment lcl_getLabelAlignment( const AxisProperties& rAxisProperties )
{
    sal_Int16 nWritingMode( text::WritingMode2::LR_TB );//@todo get correct one
    bool bIsYAxis = rAxisProperties.m_bIsYAxis;
    bool bIsLeftOrBottomAxis = rAxisProperties.m_bIsLeftOrBottomAxis;

    bool bTop = !bIsYAxis && !bIsLeftOrBottomAxis;
    bool bLeft = true;
    switch( nWritingMode )
    {
        case text::WritingMode2::RL_TB:
        case text::WritingMode2::TB_RL:
            bLeft = !( bIsYAxis && !bIsLeftOrBottomAxis );
        case text::WritingMode2::TB_LR:
        default:
            bLeft = bIsYAxis && bIsLeftOrBottomAxis;
    }

    LabelAlignment aRet( LABEL_ALIGN_LEFT );
    if(bTop)
    {
        if(bLeft)
            aRet = LABEL_ALIGN_LEFT_TOP;
        else
            aRet = LABEL_ALIGN_RIGHT_TOP;
    }
    else
    {
        if(bLeft)
            aRet = LABEL_ALIGN_LEFT_BOTTOM;
        else
            aRet = LABEL_ALIGN_RIGHT_BOTTOM;
    }
    return aRet;
}

sal_Int32 lcl_getAxisScreenPosition( double fCrossOtherAxis
        , const PlottingPositionHelper& rPosHelper, bool bIsYAxis )
{
    double fX = bIsYAxis ? fCrossOtherAxis : rPosHelper.getLogicMinX();
    double fY = bIsYAxis ? rPosHelper.getLogicMinY() : fCrossOtherAxis;

    rPosHelper.clipLogicValues( &fX,&fY,0 );
    rPosHelper.doLogicScaling( &fX,&fY,0 );
    drawing::Position3D aPos( fX, fY, 0);

    uno::Reference< XTransformation > xTransformation =
        rPosHelper.getTransformationLogicToScene();
    uno::Sequence< double > aSeq =
        xTransformation->transform( Position3DToSequence(aPos) );

    return static_cast<sal_Int32>(
        bIsYAxis ? aSeq[0] : aSeq[1] );
}

sal_Int32 lcl_getMainLineScreenPosition(
            const PlottingPositionHelper& rPosHelper
          , const AxisProperties& rAxisProperties )
{
    double fMin = rAxisProperties.m_bIsYAxis ? rPosHelper.getLogicMinX() : rPosHelper.getLogicMinY();
    double fMax = rAxisProperties.m_bIsYAxis ? rPosHelper.getLogicMaxX() : rPosHelper.getLogicMaxY();

    double fCrossOtherAxis;
    if(rAxisProperties.m_pfMainLinePositionAtOtherAxis)
        fCrossOtherAxis = *rAxisProperties.m_pfMainLinePositionAtOtherAxis;
    else
    {
        bool bMinimumForLeftAxis = ( rAxisProperties.m_bIsYAxis && rPosHelper.isMathematicalOrientationY() )
                            || ( !rAxisProperties.m_bIsYAxis && rPosHelper.isMathematicalOrientationX() );
        fCrossOtherAxis = ( bMinimumForLeftAxis && rAxisProperties.m_bIsLeftOrBottomAxis ) ? fMin : fMax;
    }
    sal_Int32 nRet = lcl_getAxisScreenPosition( fCrossOtherAxis, rPosHelper
                        , rAxisProperties.m_bIsYAxis );
    return nRet;
}

bool lcl_getExtraLineScreenPosition(
            sal_Int32& rnExtraLineScreenPosition
            , const PlottingPositionHelper& rPosHelper
            , const AxisProperties& rAxisProperties )
{
    if( !rAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        return false;

    double fMin = rAxisProperties.m_bIsYAxis ? rPosHelper.getLogicMinX() : rPosHelper.getLogicMinY();
    double fMax = rAxisProperties.m_bIsYAxis ? rPosHelper.getLogicMaxX() : rPosHelper.getLogicMaxY();
    if( *rAxisProperties.m_pfExrtaLinePositionAtOtherAxis <= fMin
        || *rAxisProperties.m_pfExrtaLinePositionAtOtherAxis >= fMax )
        return false;
    rnExtraLineScreenPosition = lcl_getAxisScreenPosition(
                    *rAxisProperties.m_pfExrtaLinePositionAtOtherAxis
                    , rPosHelper, rAxisProperties.m_bIsYAxis );
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VAxis::VAxis( const AxisProperties& rAxisProperties
            , NumberFormatterWrapper* pNumberFormatterWrapper
            , sal_Int32 nDimensionCount )
            : VMeterBase( uno::Reference<XMeter>::query(rAxisProperties.m_xAxisModel)
                , nDimensionCount )
            , m_aAxisProperties( rAxisProperties )
            , m_pNumberFormatterWrapper( pNumberFormatterWrapper )
{
    m_pPosHelper = new PlottingPositionHelper();
}

VAxis::~VAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void lcl_correctRotation_Left( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for left y-axis with right top alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
        rfXCorrection  = 0.0;
        rfYCorrection = -aSize.Height/2.0;
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = -aSize.Height*rtl::math::sin( fAnglePi );
        rfYCorrection = -aSize.Height*rtl::math::cos( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        rfXCorrection  = -aSize.Width *rtl::math::sin( fAnglePi - F_PI/2.0 )
                    -aSize.Height*rtl::math::cos( fAnglePi - F_PI/2.0 );
        rfYCorrection = -aSize.Width *rtl::math::cos( fAnglePi - F_PI/2.0 )
                    +aSize.Height*rtl::math::sin( fAnglePi - F_PI/2.0 )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = fAnglePi - F_PI;
        double gamma = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = -aSize.Width *rtl::math::cos( beta );
        rfYCorrection = +aSize.Height*rtl::math::sin( gamma )/2.0
                    +aSize.Width *rtl::math::sin( beta );
    }
    else
    {
        rfXCorrection = 0.0;
        rfYCorrection = -aSize.Height*rtl::math::cos( fAnglePi )/2.0;
    }
}

void lcl_correctRotation_Right( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for right y-axis with left top alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
        rfXCorrection  = 0.0;
        rfYCorrection = -aSize.Height/2.0;
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection = 0.0;
        rfYCorrection = -aSize.Height*rtl::math::cos( fAnglePi )/2.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        double gamma = fAnglePi - F_PI/2.0;
        rfXCorrection  = aSize.Width *rtl::math::cos( beta );
        rfYCorrection = +aSize.Height*rtl::math::sin( gamma )/2.0
                    +aSize.Width *rtl::math::sin( beta );
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = aSize.Width *rtl::math::sin( beta )
                    +aSize.Height*rtl::math::cos( beta );
        rfYCorrection = -aSize.Width *rtl::math::cos( beta )
                    +aSize.Height*rtl::math::sin( beta )/2.0;
    }
    else
    {
        rfXCorrection  = aSize.Height*rtl::math::sin( 2*F_PI - fAnglePi );
        rfYCorrection = -aSize.Height*rtl::math::cos( 2*F_PI - fAnglePi )/2.0;
    }
}
void lcl_correctRotation_Bottom_Left( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for bottom x-axis with left top alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
        rfXCorrection  = -aSize.Width/2.0;
        rfYCorrection = 0.0;
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = -aSize.Height*rtl::math::sin( fAnglePi )/2.0
                    -aSize.Width *rtl::math::cos( fAnglePi );
        rfYCorrection = aSize.Width *rtl::math::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        rfYCorrection = aSize.Width *rtl::math::sin( beta )
                    +aSize.Height*rtl::math::cos( beta );
        rfXCorrection  = aSize.Width *rtl::math::cos( beta )
                    -aSize.Height*rtl::math::sin( beta )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = aSize.Height*rtl::math::cos( beta )/2.0;
        rfYCorrection = aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection  = aSize.Height*rtl::math::sin( beta )/2.0;
        rfYCorrection = 0.0;
    }
}

void lcl_correctRotation_Bottom_Right( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for bottom x-axis with right top alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
        rfXCorrection  = aSize.Width/2.0;
        rfYCorrection = 0.0;
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = -aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        rfYCorrection = 0.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        rfYCorrection = aSize.Height*rtl::math::cos( beta );
        rfXCorrection  = -aSize.Height*rtl::math::sin( beta )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = aSize.Height*rtl::math::cos( beta )/2.0
                       -aSize.Width*rtl::math::sin( beta );

        rfYCorrection = aSize.Height*rtl::math::sin( beta )
                      +aSize.Width*rtl::math::cos( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection  = aSize.Height*rtl::math::sin( beta )/2.0
                      +aSize.Width*rtl::math::cos( beta );
        rfYCorrection = aSize.Width*rtl::math::sin( beta );
    }
}

void lcl_correctRotation_Top_Left( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for top x-axis with left top alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
        rfXCorrection  = -aSize.Width/2.0;
        rfYCorrection = 0.0;
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = +aSize.Height*rtl::math::sin( fAnglePi )/2.0;
        rfYCorrection = 0.0;
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        rfYCorrection = -aSize.Height*rtl::math::cos( beta );
        rfXCorrection  =  aSize.Height*rtl::math::sin( beta )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = -aSize.Height*rtl::math::cos( beta )/2.0
                        +aSize.Width *rtl::math::sin( beta );
        rfYCorrection = -aSize.Width *rtl::math::cos( beta )
                        -aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection  = - aSize.Width*rtl::math::cos( beta )
                        - aSize.Height*rtl::math::sin( beta )/2.0;
        rfYCorrection = - aSize.Width*rtl::math::sin( beta );
    }
}

void lcl_correctRotation_Top_Right( double& rfXCorrection, double& rfYCorrection
                           , double fAnglePositiveDegree, const awt::Size& aSize )
{
    //correct position for top x-axis with right top alignment
    double fAnglePi = fAnglePositiveDegree*F_PI/180.0;
    if( fAnglePositiveDegree==0.0 )
    {
        rfXCorrection  = aSize.Width/2.0;
        rfYCorrection = 0.0;
    }
    else if( fAnglePositiveDegree<= 90.0 )
    {
        rfXCorrection  = aSize.Height*rtl::math::sin( fAnglePi )/2.0
                       +aSize.Width*rtl::math::cos( fAnglePi );
        rfYCorrection = -aSize.Width*rtl::math::sin( fAnglePi );
    }
    else if( fAnglePositiveDegree<= 180.0 )
    {
        double beta = F_PI - fAnglePi;
        rfYCorrection = -aSize.Height*rtl::math::cos( beta )
                       -aSize.Width*rtl::math::sin( beta );
        rfXCorrection  = -aSize.Width*rtl::math::cos( beta )
                       +aSize.Height*rtl::math::sin( beta )/2.0;
    }
    else if( fAnglePositiveDegree<= 270.0 )
    {
        double beta = 3*F_PI/2.0 - fAnglePi;
        rfXCorrection  = -aSize.Height*rtl::math::cos( beta )/2.0;
        rfYCorrection = -aSize.Height*rtl::math::sin( beta );
    }
    else
    {
        double beta = 2*F_PI - fAnglePi;
        rfXCorrection  = - aSize.Height*rtl::math::sin( beta )/2.0;
        rfYCorrection = - 0.0;
    }
}

enum ShiftDirection { SHIFT_LEFT, SHIFT_TOP, SHIFT_RIGHT, SHIFT_BOTTOM };
enum RotationCentre { ROTATE_LEFT_TOP, ROTATE_LEFT_BOTTOM, ROTATE_RIGHT_TOP, ROTATE_RIGHT_BOTTOM };

ShiftDirection lcl_getLabelShiftDirection( const AxisProperties& rAxisProperties )
{
    ShiftDirection aShiftDirection = SHIFT_RIGHT;
    if(rAxisProperties.m_bIsYAxis)
    {
        if(rAxisProperties.m_bIsLeftOrBottomAxis)
            aShiftDirection = SHIFT_LEFT;
    }
    else
    {
        if(rAxisProperties.m_bIsLeftOrBottomAxis)
            aShiftDirection = SHIFT_BOTTOM;
        else
            aShiftDirection = SHIFT_TOP;
    }
    return aShiftDirection;
}

RotationCentre lcl_getLabelRotationCentre(
                const AxisProperties& rAxisProperties
                , const AxisLabelProperties& rAxisLabelProperties )
{
    //indicate where the centre of the rotation lies (e.g. top left or bottom left corner of the text shape)
    RotationCentre aRotationCentre = ROTATE_LEFT_TOP;
    {
        LabelAlignment eLabelAlignment( lcl_getLabelAlignment( rAxisProperties ) );
        switch(eLabelAlignment)
        {
        case LABEL_ALIGN_LEFT_TOP:
            aRotationCentre = ROTATE_RIGHT_BOTTOM;
            break;
        case LABEL_ALIGN_LEFT_BOTTOM:
            aRotationCentre = ROTATE_RIGHT_TOP;
            break;
        case LABEL_ALIGN_RIGHT_TOP:
            aRotationCentre = ROTATE_LEFT_BOTTOM;
            break;
        case LABEL_ALIGN_RIGHT_BOTTOM:
            aRotationCentre = ROTATE_LEFT_TOP;
            break;
        case LABEL_ALIGN_LEFT:
        case LABEL_ALIGN_TOP:
        case LABEL_ALIGN_RIGHT:
        case LABEL_ALIGN_BOTTOM:
        default: //LABEL_ALIGN_CENTER
            ROTATE_RIGHT_BOTTOM;
            break;
        };
    }
    return aRotationCentre;
}

void lcl_getPositionCorrectionForRotation(
                  double& rfXCorrection
                , double& rfYCorrection
                , const ShiftDirection& aShiftDirection
                , const RotationCentre& aRotationCentre
                , const double fRotationAngle
                , const awt::Size& aSize )
{
    double fAnglePositiveDegree = fRotationAngle;
    while(fAnglePositiveDegree<0.0)
        fAnglePositiveDegree+=360.0;

    if( SHIFT_LEFT ==aShiftDirection )
    {
        DBG_ASSERT( ROTATE_RIGHT_TOP ==aRotationCentre, "it is assumed that rotation centre is at left top corner if labels are shifted to the right" );

        lcl_correctRotation_Left( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
    }
    else if( SHIFT_RIGHT ==aShiftDirection )
    {
        DBG_ASSERT( ROTATE_LEFT_TOP ==aRotationCentre, "it is assumed that rotation centre is at left top corner if labels are shifted to the right" );

        lcl_correctRotation_Right( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
    }
    else if( SHIFT_BOTTOM ==aShiftDirection )
    {
        DBG_ASSERT( ROTATE_LEFT_TOP ==aRotationCentre
                 || ROTATE_RIGHT_TOP ==aRotationCentre
                 , "it is assumed that rotation centre is at left or right top corner if labels are shifted to the bottom" );

        if( ROTATE_LEFT_TOP ==aRotationCentre )
            lcl_correctRotation_Bottom_Left( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
        else
            lcl_correctRotation_Bottom_Right( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
    }
    else if( SHIFT_TOP ==aShiftDirection )
    {
        DBG_ASSERT( ROTATE_LEFT_BOTTOM ==aRotationCentre
                 || ROTATE_RIGHT_BOTTOM ==aRotationCentre
                 , "it is assumed that rotation centre is at left or right bottom corner if labels are shifted to the top" );

        if( ROTATE_LEFT_BOTTOM ==aRotationCentre )
            lcl_correctRotation_Top_Left( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
        else
            lcl_correctRotation_Top_Right( rfXCorrection, rfYCorrection, fAnglePositiveDegree, aSize );
    }
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
    //correctPositionForRotation()
    awt::Point aOldPos = xShape2DText->getPosition();
    awt::Size  aSize   = xShape2DText->getSize();

    ShiftDirection aShiftDirection = lcl_getLabelShiftDirection( rAxisProperties );
    RotationCentre aRotationCentre = lcl_getLabelRotationCentre( rAxisProperties, rAxisLabelProperties );

    double fYCorrection = 0.0;
    double fXCorrection  = 0.0;

    lcl_getPositionCorrectionForRotation( fXCorrection, fYCorrection
        , aShiftDirection, aRotationCentre
        , rAxisLabelProperties.fRotationAngleDegree, aSize );

    xShape2DText->setPosition( awt::Point(
          static_cast<sal_Int32>(aOldPos.X + fXCorrection  )
        , static_cast<sal_Int32>(aOldPos.Y + fYCorrection ) ) );
    //-------------

    return xShape2DText;
}

bool lcl_doesOverlap( const uno::Reference< drawing::XShape >& xShape
, sal_Int32 nScreenTickValue, bool bCheckDirectionIsY )
{
    if(!xShape.is())
        return false;

    awt::Size  aSize = xShape->getSize();
    awt::Point aPos  = xShape->getPosition();

    sal_Int32 nMin = bCheckDirectionIsY ? aPos.Y : aPos.X;
    sal_Int32 nMax = nMin + ( bCheckDirectionIsY ? aSize.Height : aSize.Width );

    return  nMin <= nScreenTickValue && nScreenTickValue <= nMax;
}

bool doesOverlap( const uno::Reference< drawing::XShape >& xShape1
                , const uno::Reference< drawing::XShape >& xShape2
                , bool bCheckDirectionIsY )
{
    awt::Size  aSize1 = xShape1->getSize();
    awt::Point aPos1  = xShape1->getPosition();

    sal_Int32 nMin1 = bCheckDirectionIsY ? aPos1.Y : aPos1.X;
    sal_Int32 nMax1 = nMin1 + ( bCheckDirectionIsY ? aSize1.Height : aSize1.Width );

    awt::Size  aSize2 = xShape2->getSize();
    awt::Point aPos2  = xShape2->getPosition();

    sal_Int32 nMin2 = bCheckDirectionIsY ? aPos2.Y : aPos2.X;
    sal_Int32 nMax2 = nMin2 + ( bCheckDirectionIsY ? aSize2.Height : aSize2.Width );

    if( nMax1 < nMin2 )
        return false;
    if( nMax2 < nMin1 )
        return false;
    return true;
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
            , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement
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
            , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement
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

sal_Int32 lcl_getStaggerDistance(
            LabelIterator& rIter
            , const ShiftDirection aShiftDirection )
{
    //calculates the height or width of the first line of labels
    //thus the second line of labels needs to be shifted for that distance

    sal_Int32 nRet=0;
    uno::Reference< drawing::XShape >  xShape2DText(NULL);
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rIter.nextInfo() )
    {
        xShape2DText = pTickInfo->xTextShape;
        DBG_ASSERT(xShape2DText.is(),"LabelIterator does not work correctly");

        awt::Size aSize  = xShape2DText->getSize();
        switch(aShiftDirection)
        {
            case SHIFT_TOP:
                nRet = ::std::max(nRet,aSize.Height); break;
            case SHIFT_BOTTOM:
                nRet = ::std::max(nRet,aSize.Height); break;
            case SHIFT_LEFT:
                nRet = ::std::max(nRet,aSize.Width); break;
            case SHIFT_RIGHT:
                nRet = ::std::max(nRet,aSize.Width); break;
        }
    }
    return nRet;
}

void lcl_correctPositionForStaggering(
            LabelIterator& rIter
            , const ShiftDirection aShiftDirection
            , sal_Int32 nStaggerDistance
            )
{
    uno::Reference< drawing::XShape >  xShape2DText(NULL);
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = rIter.nextInfo() )
    {
        xShape2DText = pTickInfo->xTextShape;
        DBG_ASSERT(xShape2DText.is(),"LabelIterator does not work correctly");

        awt::Point aPos  = xShape2DText->getPosition();
        switch(aShiftDirection)
        {
            case SHIFT_TOP:
                aPos.Y -= nStaggerDistance; break;
            case SHIFT_BOTTOM:
                aPos.Y += nStaggerDistance; break;
            case SHIFT_LEFT:
                aPos.X -= nStaggerDistance; break;
            case SHIFT_RIGHT:
                aPos.X += nStaggerDistance; break;
        }
        xShape2DText->setPosition( aPos );
    }
}

bool createTextShapes( const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory
                     , const uno::Reference< drawing::XShapes >& xTarget
                     , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                     , const ExplicitIncrementData& rIncrement
                     , AxisLabelProperties& rAxisLabelProperties
                     , const AxisProperties& rAxisProperties
                     , sal_Int32 nTextReferenceScreenPosition
                     , const FixedNumberFormatter& rFixedNumberFormatter
                     , const uno::Reference< XScaling >& xInverseScaling )
{
    //returns true if the text shapes have been created succesfully
    //otherwise false - in this case the AxisLabelProperties have changed
    //and contain new instructions for the next try for text shape creation

    bool bOverlapCheckDirectionIsY = rAxisProperties.m_bIsYAxis;
    //@todo: iterate through all tick depth wich should be labeled
    TickIter aIter( rAllTickInfos, rIncrement, 0, 0 );
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

    uno::Reference< beans::XPropertySet > xProps( rAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    PropertyMapper::getTextLabelMultiPropertyLists( xProps, aPropNames, aPropValues, false
        , nLimitedSpaceForStaggering, rAxisProperties.m_bIsYAxis );
    LabelPositionHelper::doDynamicFontResize( aPropValues, aPropNames, xProps
        , rAxisProperties.m_aReferenceSize );
    LabelPositionHelper::changeTextAdjustment( aPropValues, aPropNames, lcl_getLabelAlignment( rAxisProperties ) );

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
            if( lcl_doesOverlap( pLastVisibleNeighbourTickInfo->xTextShape, pTickInfo->nScreenTickValue, bOverlapCheckDirectionIsY ) )
            {
                if( rAxisLabelProperties.bRhythmIsFix )
                    continue;
                rAxisLabelProperties.nRhythm++;
                TickIter aRemoveIter( rAllTickInfos, rIncrement, 0, 0 );
                removeShapesAtWrongRythm( aRemoveIter, rAxisLabelProperties.nRhythm, nTick, xTarget );
                return false;
            }
        }

        pTickInfo->updateUnscaledValue( xInverseScaling );

        bool bHasExtraColor=false;
        sal_Int32 nExtraColor=0;
        rtl::OUString aLabel = rFixedNumberFormatter.getFormattedString( pTickInfo->fUnscaledTickValue, nExtraColor, bHasExtraColor );
        if(pColorAny)
            *pColorAny = uno::makeAny(bHasExtraColor?nExtraColor:nColor);

        awt::Point aAnchorScreenPosition2D;
        {//get anchor position
            sal_Int32 nAxisPos = nTextReferenceScreenPosition;
            sal_Int32 nTickPos = pTickInfo->nScreenTickValue;
            if( rAxisProperties.m_bIsYAxis )
                aAnchorScreenPosition2D = awt::Point(nAxisPos,nTickPos);
            else
                aAnchorScreenPosition2D = awt::Point(nTickPos,nAxisPos);
        }

        //create single label
        if(!pTickInfo->xTextShape.is())
            pTickInfo->xTextShape = createSingleLabel( xShapeFactory, xTarget
                                    , aAnchorScreenPosition2D, aLabel
                                    , rAxisLabelProperties, rAxisProperties
                                    , aPropNames, aPropValues );

        //if NO OVERLAP -> remove overlapping shapes
        if( pLastVisibleNeighbourTickInfo && !rAxisLabelProperties.bOverlapAllowed )
        {
            if( doesOverlap( pLastVisibleNeighbourTickInfo->xTextShape, pTickInfo->xTextShape, bOverlapCheckDirectionIsY ) )
            {
                if( rAxisLabelProperties.bRhythmIsFix )
                {
                    xTarget->remove(pTickInfo->xTextShape);
                    pTickInfo->xTextShape = NULL;
                    continue;
                }
                rAxisLabelProperties.nRhythm++;
                TickIter aRemoveIter( rAllTickInfos, rIncrement, 0, 0 );
                removeShapesAtWrongRythm( aRemoveIter, rAxisLabelProperties.nRhythm, nTick, xTarget );
                return false;
            }
        }

        pPREPreviousVisibleTickInfo = pPreviousVisibleTickInfo;
        pPreviousVisibleTickInfo = pTickInfo;
    }
    return true;
}

sal_Int32 lcl_getTextReferenceScreenPosition( const ::std::vector<TickmarkProperties>& rTickmarkPropertiesList
                                   , bool bIsYAxis
                                   , bool bIsLeftOrBottomAxis
                                   , sal_Int32 nAxisPos )
{
    sal_Int32 nRet = 0;

    bool bFindMin = (bIsYAxis && bIsLeftOrBottomAxis) || (!bIsYAxis && !bIsLeftOrBottomAxis);
    ::std::vector<TickmarkProperties>::const_iterator aIter    = rTickmarkPropertiesList.begin();
    const ::std::vector<TickmarkProperties>::const_iterator aIterEnd = rTickmarkPropertiesList.end();
    for( ; aIter != aIterEnd; aIter++  )
    {
        if( ( bFindMin && (*aIter).RelativePos < nRet )
            ||
            ( !bFindMin && (*aIter).RelativePos > nRet )
            )
        {
            nRet = (*aIter).RelativePos;
        }
    }
    nRet+=nAxisPos;

    if(bFindMin)
        nRet-=AXIS2D_TICKLABELSPACING;
    else
        nRet+=AXIS2D_TICKLABELSPACING;
    return nRet;
}

void addLine( drawing::PointSequenceSequence&  rPoints, sal_Int32 nIndex
             , sal_Int32 nScreenTickValue, sal_Int32 nOrthogonalAxisScreenPosition
             , const TickmarkProperties& rTickmarkProperties, bool bIsYAxis )
{
    sal_Int32 nStartX = bIsYAxis ? nOrthogonalAxisScreenPosition + rTickmarkProperties.RelativePos : nScreenTickValue;
    sal_Int32 nStartY = bIsYAxis ? nScreenTickValue : nOrthogonalAxisScreenPosition + rTickmarkProperties.RelativePos;

    sal_Int32 nEndX = nStartX;
    sal_Int32 nEndY = nStartY;
    if( bIsYAxis )
        nEndX += rTickmarkProperties.Length;
    else
        nEndY += rTickmarkProperties.Length;

    rPoints[nIndex].realloc(2);
    rPoints[nIndex][0].X = nStartX;
    rPoints[nIndex][0].Y = nStartY;
    rPoints[nIndex][1].X = nEndX;
    rPoints[nIndex][1].Y = nEndY;
}

void createPointSequenceForAxisMainLine( drawing::PointSequenceSequence& rPoints,
         bool bIsYAxis, sal_Int32 nOrthogonalPos, sal_Int32 nMin, sal_Int32 nMax )
{
    rPoints[0].realloc(2);
    rPoints[0][0].X = bIsYAxis ? nOrthogonalPos : nMin;
    rPoints[0][0].Y = bIsYAxis ? nMin : nOrthogonalPos;
    rPoints[0][1].X = bIsYAxis ? nOrthogonalPos : nMax;
    rPoints[0][1].Y = bIsYAxis ? nMax : nOrthogonalPos;
}

void SAL_CALL VAxis::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    m_aAxisProperties.init();
    bool bIsYAxis = m_aAxisProperties.m_bIsYAxis;
    bool bIsLeftOrBottomAxis = m_aAxisProperties.m_bIsLeftOrBottomAxis;
    sal_Int32 nMainLineScreenPosition = lcl_getMainLineScreenPosition( *m_pPosHelper, m_aAxisProperties );

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
            addLine( aPoints, nN
                , (*aTickIter).nScreenTickValue, nMainLineScreenPosition
                , rTickmarkProperties, bIsYAxis);
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
            createPointSequenceForAxisMainLine( aPoints
                , bIsYAxis, nMainLineScreenPosition
                , aTickmarkHelper.getScreenValueForMinimum(), aTickmarkHelper.getScreenValueForMaximum() );

            uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                    xGroupShape_Shapes, aPoints
                    , m_aAxisProperties.m_aLineProperties );
            //because of this name this line will be used for marking the axis
            m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
        }
        //-----------------------------------------
        //create an additional line at NULL
        sal_Int32 nExtraLineScreenPosition;
        if( lcl_getExtraLineScreenPosition( nExtraLineScreenPosition, *m_pPosHelper, m_aAxisProperties ) )
        {
            drawing::PointSequenceSequence aPoints(1);
            createPointSequenceForAxisMainLine( aPoints
                , bIsYAxis, nExtraLineScreenPosition
                , aTickmarkHelper.getScreenValueForMinimum(), aTickmarkHelper.getScreenValueForMaximum() );

            uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                    xGroupShape_Shapes, aPoints
                    , m_aAxisProperties.m_aLineProperties );
        }
    }
    //-----------------------------------------
    //create labels
    AxisLabelProperties aAxisLabelProperties;
    aAxisLabelProperties.init(m_aAxisProperties.m_xAxisModel);
    if( aAxisLabelProperties.bDisplayLabels )
    {
        FixedNumberFormatter aFixedNumberFormatter(
                m_pNumberFormatterWrapper, aAxisLabelProperties.aNumberFormat );

        uno::Reference< XScaling > xInverseScaling( NULL );
        if( m_aScale.Scaling.is() )
            xInverseScaling = m_aScale.Scaling->getInverseScaling();

        sal_Int32 nTextReferenceScreenPosition = lcl_getTextReferenceScreenPosition(
                m_aAxisProperties.m_aTickmarkPropertiesList
                , bIsYAxis, bIsLeftOrBottomAxis, nMainLineScreenPosition );

        //create tick mark text shapes
        while( !createTextShapes( m_xShapeFactory, xGroupShape_Shapes, aAllTickInfos
                        , m_aIncrement, aAxisLabelProperties, m_aAxisProperties
                        , nTextReferenceScreenPosition
                        , aFixedNumberFormatter, xInverseScaling
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

            sal_Int32 nStaggerDistance = lcl_getStaggerDistance( aInnerIter
                , lcl_getLabelShiftDirection( m_aAxisProperties ) );

            lcl_correctPositionForStaggering( aOuterIter
                , lcl_getLabelShiftDirection( m_aAxisProperties )
                , nStaggerDistance );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
