/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VAxisProperties.cxx,v $
 * $Revision: 1.13 $
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
#include "VAxisProperties.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XAXISPOSITION_HPP_
#include <com/sun/star/chart2/AxisPosition.hpp>
#endif
#include <tools/color.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

sal_Int32 lcl_calcTickLengthForDepth(sal_Int32 nDepth,sal_Int32 nTickmarkStyle)
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

double lcl_getTickOffset(sal_Int32 nLength,sal_Int32 nTickmarkStyle)
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
    return fPercent*nLength;
}

VLineProperties AxisProperties::makeLinePropertiesForDepth( sal_Int32 /* nDepth */ ) const
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

    if( m_fInnerDirectionSign == 0.0 )
    {
        if( nTickmarkStyle != 0 )
            nTickmarkStyle = 3; //inner and outer tickmarks
    }

    TickmarkProperties aTickmarkProperties;
    aTickmarkProperties.Length = lcl_calcTickLengthForDepth(nDepth,nTickmarkStyle);
    aTickmarkProperties.RelativePos = static_cast<sal_Int32>(lcl_getTickOffset(aTickmarkProperties.Length,nTickmarkStyle));
    aTickmarkProperties.aLineProperties = this->makeLinePropertiesForDepth( nDepth );
    return aTickmarkProperties;
}

//--------------------------------------------------------------------------

AxisProperties::AxisProperties( const uno::Reference< XAxis >& xAxisModel
                              , const uno::Reference< data::XTextualDataSequence >& xAxisTextProvider )
    : m_xAxisModel(xAxisModel)
    , m_nDimensionIndex(0)
    , m_bIsMainAxis(true)
    , m_bSwapXAndY(false)
    , m_pfMainLinePositionAtOtherAxis(NULL)
    , m_pfExrtaLinePositionAtOtherAxis(NULL)
    , m_fInnerDirectionSign(1.0)
    , m_bLabelsOutside(true)
    , m_aLabelAlignment(LABEL_ALIGN_RIGHT_TOP)
    , m_bDisplayLabels( true )
    , m_nNumberFormatKey(0)
//    , m_eRelativeLabelPosition(LEFTORBOTTOM_OF_AXIS)
    , m_nMajorTickmarks(1)
    , m_nMinorTickmarks(1)
    , m_aTickmarkPropertiesList()
    , m_aLineProperties()
    //for category axes
    , m_nAxisType(AxisType::REALNUMBER)
    , m_xAxisTextProvider(xAxisTextProvider)
    , m_bTickmarksAtIndicatedValue(false)
{
}

AxisProperties::AxisProperties( const AxisProperties& rAxisProperties )
    : m_xAxisModel( rAxisProperties.m_xAxisModel )
    , m_nDimensionIndex( m_nDimensionIndex )
    , m_bIsMainAxis( rAxisProperties.m_bIsMainAxis )
    , m_bSwapXAndY( rAxisProperties.m_bSwapXAndY )
    , m_pfMainLinePositionAtOtherAxis( NULL )
    , m_pfExrtaLinePositionAtOtherAxis( NULL )
    , m_fInnerDirectionSign( rAxisProperties.m_fInnerDirectionSign )
    , m_bLabelsOutside( rAxisProperties.m_bLabelsOutside )
    , m_aLabelAlignment( rAxisProperties.m_aLabelAlignment )
    , m_bDisplayLabels( rAxisProperties.m_bDisplayLabels )
    , m_nNumberFormatKey( rAxisProperties.m_nNumberFormatKey )
//    , m_eRelativeLabelPosition( rAxisProperties.m_eRelativeLabelPosition )
    , m_nMajorTickmarks( rAxisProperties.m_nMajorTickmarks )
    , m_nMinorTickmarks( rAxisProperties.m_nMinorTickmarks )
    , m_aTickmarkPropertiesList( rAxisProperties.m_aTickmarkPropertiesList )
    , m_aLineProperties( rAxisProperties.m_aLineProperties )
    //for category axes
    , m_nAxisType( rAxisProperties.m_nAxisType )
    , m_xAxisTextProvider( rAxisProperties.m_xAxisTextProvider )
    , m_bTickmarksAtIndicatedValue( rAxisProperties.m_bTickmarksAtIndicatedValue )
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

LabelAlignment lcl_getLabelAlignmentForZAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_LEFT );
    if( rAxisProperties.m_bLabelsOutside )
        aRet = LABEL_ALIGN_RIGHT;
    else
        aRet = LABEL_ALIGN_LEFT;
    return aRet;
}

LabelAlignment lcl_getLabelAlignmentForYAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_LEFT );
    if(rAxisProperties.m_bIsMainAxis)
    {
        if( rAxisProperties.m_bLabelsOutside )
            aRet = LABEL_ALIGN_LEFT;
        else
            aRet = LABEL_ALIGN_RIGHT;
    }
    else
    {
        if( !rAxisProperties.m_bLabelsOutside )
            aRet = LABEL_ALIGN_LEFT;
        else
            aRet = LABEL_ALIGN_RIGHT;
    }
    return aRet;
}

LabelAlignment lcl_getLabelAlignmentForXAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_LEFT );
    if(rAxisProperties.m_bIsMainAxis )
    {
        if(rAxisProperties.m_bLabelsOutside)
            aRet = LABEL_ALIGN_BOTTOM;
        else
            aRet = LABEL_ALIGN_TOP;
    }
    else
    {
        if(!rAxisProperties.m_bLabelsOutside)
            aRet = LABEL_ALIGN_BOTTOM;
        else
            aRet = LABEL_ALIGN_TOP;
    }
    return aRet;
}

void AxisProperties::init( bool bCartesian )
{
    uno::Reference< beans::XPropertySet > xProp =
        uno::Reference<beans::XPropertySet>::query( this->m_xAxisModel );
    if( !xProp.is() )
        return;
    if( bCartesian )
    {
        try
        {
            //todo nAxisPosition and nAxisIndex are the same values so far and maybe need to be seperated in future
            sal_Int32 nAxisPosition=AxisPosition::MAIN;
            xProp->getPropertyValue( C2U( "AxisPosition" ) ) >>= nAxisPosition;
            m_bIsMainAxis = nAxisPosition==AxisPosition::MAIN;
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }

        m_fInnerDirectionSign = m_bIsMainAxis ? 1 : -1;

        if( m_nDimensionIndex==2 )
            m_aLabelAlignment = lcl_getLabelAlignmentForZAxis(*this);
        else
        {
            bool bIsYAxisPosition = (m_nDimensionIndex==1 && !m_bSwapXAndY)
                || (m_nDimensionIndex==0 && m_bSwapXAndY);
            if( bIsYAxisPosition )
                m_fInnerDirectionSign*=-1;

            if( bIsYAxisPosition )
                m_aLabelAlignment = lcl_getLabelAlignmentForYAxis(*this);
            else
                m_aLabelAlignment = lcl_getLabelAlignmentForXAxis(*this);
        }
    }

    try
    {
        //init LineProperties
        m_aLineProperties.initFromPropertySet( xProp );

        //init display labels
        xProp->getPropertyValue( C2U( "DisplayLabels" ) ) >>= m_bDisplayLabels;

        //init categories
        ScaleData aScaleData = m_xAxisModel->getScaleData();
        m_nAxisType = aScaleData.AxisType;

        //init TickmarkProperties
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
        ASSERT_EXCEPTION( e );
    }
}

//-----------------------------------------------------------------------------

AxisLabelProperties::AxisLabelProperties()
                        : m_aFontReferenceSize( 8000, 7000 )
                        , m_aMaximumSpaceForLabels( 0 , 0, 8000, 7000 )
                        , nNumberFormatKey(0)
                        , eStaggering( SIDE_BY_SIDE )
                        , bLineBreakAllowed( false )
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
            ASSERT_EXCEPTION( e );
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

//.............................................................................
} //namespace chart
//.............................................................................
