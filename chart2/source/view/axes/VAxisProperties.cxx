/*************************************************************************
 *
 *  $RCSfile: VAxisProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-13 09:52:35 $
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
#include "VAxisProperties.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE2_HPP_
#include <com/sun/star/text/WritingMode2.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

sal_Int32 AxisProperties::calcTickLengthForDepth(sal_Int32 nDepth,sal_Int32 nTickmarkStyle) const
{
    sal_Int32 nWidth = AXIS2D_TICKLENGTH; //@maybefuturetodo this length could be offered by the model
    double fPercent = 1.0;
    switch(nDepth)
    {
        case 0:
            fPercent = 1.0;
            break;
        case 1:
            fPercent = 0.75;//percentage like in the old chart
            break;
        case 2:
            fPercent = 0.5;
            break;
        default:
            fPercent = 0.3;
            break;
    }
    if(nTickmarkStyle==3)//inner and outer tickmarks
        fPercent*=2.0;
    return static_cast<sal_Int32>(nWidth*fPercent);
}

sal_Int32 getTickOffset(sal_Int32 nLength,sal_Int32 nTickmarkStyle)
{
    double fPercent = 0.0; //0<=fPercent<=1
    //0.0: completly inner
    //1.0: completly outer
    //0.5: half and half

    /*
    nTickmarkStyle:
    1: inner tickmarks
    2: outer tickmarks
    3: inner and outer tickmarks
    */
    switch(nTickmarkStyle)
    {
        case 1:
            fPercent = 0.0;
            break;
        case 2:
            fPercent = 1.0;
            break;
        default:
            fPercent = 0.5;
            break;
    }
    return static_cast<sal_Int32>(fPercent*nLength);
}

VLineProperties AxisProperties::makeLinePropertiesForDepth( sal_Int32 nDepth ) const
{
    //@todo get this from somewhere; maybe for each subincrement
    //so far the model does not offer different settings for each tick depth
    return m_aLineProperties;
}

TickmarkProperties AxisProperties::makeTickmarkProperties(
                        sal_Int32 nDepth ) const
{
    /*
    nTickmarkStyle:
    1: inner tickmarks
    2: outer tickmarks
    3: inner and outer tickmarks
    */
    sal_Int32 nTickmarkStyle = 1;
    if(nDepth==0)
    {
        nTickmarkStyle = m_nMajorTickmarks;
        if(!nTickmarkStyle)
        {
            //create major tickmarks as if they were minor tickmarks
            nDepth = 1;
            nTickmarkStyle = m_nMinorTickmarks;
        }
    }
    else if( nDepth==1)
    {
        nTickmarkStyle = m_nMinorTickmarks;
    }

    TickmarkProperties aTickmarkProperties;
    aTickmarkProperties.Length = this->calcTickLengthForDepth(nDepth,nTickmarkStyle);

    sal_Int32 nSign = m_bIsLeftOrBottomAxis ? 1 : -1;
    if(m_bIsYAxis)
        nSign *= -1;
    aTickmarkProperties.RelativePos = nSign*getTickOffset(aTickmarkProperties.Length,nTickmarkStyle);
    aTickmarkProperties.Length *= -nSign;
    aTickmarkProperties.aLineProperties = this->makeLinePropertiesForDepth( nDepth );
    return aTickmarkProperties;
}

//--------------------------------------------------------------------------

AxisProperties::AxisProperties()
    : m_xAxisModel(NULL)
    , m_bIsYAxis(true)
    , m_bIsLeftOrBottomAxis(true)
    , m_pfMainLinePositionAtOtherAxis(NULL)
    , m_pfExrtaLinePositionAtOtherAxis(NULL)
    /*
    , m_nOrthogonalAxisScreenPosition(0)
    , m_nOrthogonalAxisExtraLineScreenPosition(0)
    */
    , m_eRelativeLabelPosition(LEFTORBOTTOM_OF_AXIS)
    , m_nMajorTickmarks(1)
    , m_nMinorTickmarks(1)
    , m_aTickmarkPropertiesList()
    , m_aLineProperties()
    , m_bTESTTEST_HorizontalAdjustmentIsLeft(true)
{
}

AxisProperties::AxisProperties( const AxisProperties& rAxisProperties )
    : m_xAxisModel( rAxisProperties.m_xAxisModel )
    , m_bIsYAxis( rAxisProperties.m_bIsYAxis )
    , m_bIsLeftOrBottomAxis( rAxisProperties.m_bIsLeftOrBottomAxis )
    , m_pfMainLinePositionAtOtherAxis( NULL )
    , m_pfExrtaLinePositionAtOtherAxis( NULL )
    , m_eRelativeLabelPosition( rAxisProperties.m_eRelativeLabelPosition )
    , m_nMajorTickmarks( rAxisProperties.m_nMajorTickmarks )
    , m_nMinorTickmarks( rAxisProperties.m_nMinorTickmarks )
    , m_aTickmarkPropertiesList( rAxisProperties.m_aTickmarkPropertiesList )
    , m_aLineProperties( rAxisProperties.m_aLineProperties )
    , m_bTESTTEST_HorizontalAdjustmentIsLeft( rAxisProperties.m_bTESTTEST_HorizontalAdjustmentIsLeft )
{
    if( rAxisProperties.m_pfMainLinePositionAtOtherAxis )
        m_pfMainLinePositionAtOtherAxis = new double(*rAxisProperties.m_pfMainLinePositionAtOtherAxis);
    if( rAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        m_pfExrtaLinePositionAtOtherAxis = new double (*rAxisProperties.m_pfExrtaLinePositionAtOtherAxis);
}

AxisProperties::~AxisProperties()
{
    delete m_pfMainLinePositionAtOtherAxis;
    delete m_pfExrtaLinePositionAtOtherAxis;
}

void AxisProperties::init()
{
    if( !m_xAxisModel.is() )
        return;
    sal_Int32 nDimension = m_xAxisModel->getRepresentedDimension();
    m_bIsYAxis = (nDimension==1);

    //init LineProperties
    m_aLineProperties.initFromPropertySet( uno::Reference<beans::XPropertySet>::query( m_xAxisModel ) );

    //init TickmarkProperties
    uno::Reference< beans::XPropertySet > xProp =
        uno::Reference<beans::XPropertySet>::query( this->m_xAxisModel );
    if(xProp.is())
    {
        try
        {
            xProp->getPropertyValue( C2U( "MajorTickmarks" ) ) >>= m_nMajorTickmarks;
            xProp->getPropertyValue( C2U( "MinorTickmarks" ) ) >>= m_nMinorTickmarks;

            sal_Int32 nMaxDepth = 0;
            if(m_nMinorTickmarks!=0)
                nMaxDepth=2;
            else if(m_nMajorTickmarks!=0)
                nMaxDepth=1;

            this->m_aTickmarkPropertiesList.clear();
            for( sal_Int32 nDepth=0; nDepth<nMaxDepth; nDepth++ )
            {
                TickmarkProperties aTickmarkProperties = this->makeTickmarkProperties( nDepth );
                this->m_aTickmarkPropertiesList.push_back( aTickmarkProperties );
            }
        }
        catch( uno::Exception& e )
        {
             e;
        }
    }
}

sal_Int32 getAxisScreenPosition( double fCrossOtherAxis
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



sal_Int32 AxisProperties::getMainLineScreenPosition(
            const PlottingPositionHelper& rPosHelper ) const
{
    double fMin = m_bIsYAxis ? rPosHelper.getLogicMinX() : rPosHelper.getLogicMinY();
    double fMax = m_bIsYAxis ? rPosHelper.getLogicMaxX() : rPosHelper.getLogicMaxY();

    double fCrossOtherAxis;
    if(m_pfMainLinePositionAtOtherAxis)
        fCrossOtherAxis = *m_pfMainLinePositionAtOtherAxis;
    else
        fCrossOtherAxis = m_bIsLeftOrBottomAxis ? fMin : fMax;
    sal_Int32 nRet = getAxisScreenPosition( fCrossOtherAxis, rPosHelper, m_bIsYAxis );
    return nRet;
}

bool AxisProperties::getExtraLineScreenPosition(
            sal_Int32& rnExtraLineScreenPosition, const PlottingPositionHelper& rPosHelper ) const
{
    if( !m_pfExrtaLinePositionAtOtherAxis )
        return false;

    double fMin = m_bIsYAxis ? rPosHelper.getLogicMinX() : rPosHelper.getLogicMinY();
    double fMax = m_bIsYAxis ? rPosHelper.getLogicMaxX() : rPosHelper.getLogicMaxY();
    if( *m_pfExrtaLinePositionAtOtherAxis <= fMin
        || *m_pfExrtaLinePositionAtOtherAxis >= fMax )
        return false;
    rnExtraLineScreenPosition = getAxisScreenPosition(
                    *m_pfExrtaLinePositionAtOtherAxis, rPosHelper, m_bIsYAxis );
    return true;
}

//-----------------------------------------------------------------------------

AxisLabelProperties::AxisLabelProperties()
                        : aNumberFormat()
                        , bDisplayLabels( true )
                        , eStaggering( SIDE_BY_SIDE )
                        , bLineBreakAllowed( true )
                        , bOverlapAllowed( false )
                        , bStackCharacters( false )
                        , fRotationAngleDegree( 0.0 )
                        , nRhythm( 1 )
                        , bRhythmIsFix(false)
{
    /*
    aLocale.Language = C2U( "en" );
    aLocale.Country  = C2U( "US" );

    //aLocale.Language = C2U( "ar" );
    //aLocale.Country  = C2U( "IR" );

    //aLocale.Language = C2U( "ja" );
    //aLocale.Country  = C2U( "JP" );
    */
}

void AxisLabelProperties::init( const uno::Reference< XAxis >& xAxisModel )
{
    uno::Reference< beans::XPropertySet > xProp =
        uno::Reference<beans::XPropertySet>::query( xAxisModel );
    if(xProp.is())
    {
        try
        {
            if( !( xProp->getPropertyValue( C2U( "NumberFormat" ) ) >>= this->aNumberFormat ) )
            {
                //@todo get number format from calc
            }

            xProp->getPropertyValue( C2U( "DisplayLabels" ) ) >>= this->bDisplayLabels;
            xProp->getPropertyValue( C2U( "TextBreak" ) ) >>= this->bLineBreakAllowed;
            xProp->getPropertyValue( C2U( "TextOverlap" ) ) >>= this->bOverlapAllowed;
            xProp->getPropertyValue( C2U( "StackCharacters" ) ) >>= this->bStackCharacters;
            xProp->getPropertyValue( C2U( "TextRotation" ) ) >>= this->fRotationAngleDegree;

            ::com::sun::star::chart::ChartAxisArrangeOrderType eArrangeOrder;
            xProp->getPropertyValue( C2U( "ArrangeOrder" ) ) >>= eArrangeOrder;
            switch(eArrangeOrder)
            {
                case ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
                    this->eStaggering = SIDE_BY_SIDE;
                    break;
                case ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
                    this->eStaggering = STAGGER_EVEN;
                    break;
                case ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD:
                    this->eStaggering = STAGGER_ODD;
                    break;
                default:
                    this->eStaggering = STAGGER_AUTO;
                    break;
            }
        }
        catch( uno::Exception& e )
        {
             e;
        }
    }
}

/*
sal_Int16 getSwappedWritingMode( sal_Int16 nWritingMode )
{
    //LR_TB == LT
    //RL_TB == RT (Arabic, Hebrew)
    //TB_RL == TR (Japanese, Chinese, Korean)
    // ?? TL (Mongolian) see also text::WritingMode2

    switch(nWritingMode)
    {
        case text::WritingMode2::RL_TB:
            return  text::WritingMode2::TB_RL;
        case text::WritingMode2::TB_RL:
            return  text::WritingMode2::RL_TB;
        case text::WritingMode2::LR_TB:
            return  text::WritingMode2::TB_LR;
        default:
            return  text::WritingMode2::LR_TB;
    }
}
*/

sal_Bool AxisLabelProperties::getIsStaggered() const
{
    if( STAGGER_ODD == eStaggering || STAGGER_EVEN == eStaggering )
        return sal_True;
    return sal_False;
}

//------------------------

drawing::TextVerticalAdjust AxisProperties::getVerticalAdjustment() const
{
    drawing::TextVerticalAdjust aRet =
                !m_bIsYAxis && !m_bIsLeftOrBottomAxis
                ? drawing::TextVerticalAdjust_BOTTOM
                : drawing::TextVerticalAdjust_TOP;
    return aRet;
}

sal_Int16 AxisProperties::getWritingMode()const
{
    //@todo get this dependent on the locale ...
    sal_Int16 nWritingMode( text::WritingMode2::LR_TB );
    return nWritingMode;
}

drawing::TextHorizontalAdjust AxisProperties::getHorizontalAdjustment() const
{
    return m_bTESTTEST_HorizontalAdjustmentIsLeft
                        ? drawing::TextHorizontalAdjust_LEFT
                        : drawing::TextHorizontalAdjust_RIGHT;
//

    bool bIsYAxis = m_bIsYAxis;
    bool bIsLeftOrBottomAxis = m_bIsLeftOrBottomAxis;
    sal_Int16 nWritingMode = getWritingMode();

    switch( nWritingMode )
    {
        case text::WritingMode2::RL_TB:
        case text::WritingMode2::TB_RL:
            return  bIsYAxis && !bIsLeftOrBottomAxis
                        ? drawing::TextHorizontalAdjust_LEFT
                        : drawing::TextHorizontalAdjust_RIGHT;
        case text::WritingMode2::TB_LR:
        default:
            return bIsYAxis && bIsLeftOrBottomAxis
                        ? drawing::TextHorizontalAdjust_RIGHT
                        : drawing::TextHorizontalAdjust_LEFT;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
