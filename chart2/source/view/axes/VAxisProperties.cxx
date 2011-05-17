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
#include "VAxisProperties.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"

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

TickmarkProperties AxisProperties::makeTickmarkPropertiesForComplexCategories(
    sal_Int32 nTickLength, sal_Int32 nTickStartDistanceToAxis, sal_Int32 /*nTextLevel*/ ) const
{
    sal_Int32 nTickmarkStyle = (m_fLabelDirectionSign==m_fInnerDirectionSign) ? 2/*outside*/ : 1/*inside*/;

    TickmarkProperties aTickmarkProperties;
    aTickmarkProperties.Length = nTickLength;// + nTextLevel*( lcl_calcTickLengthForDepth(0,nTickmarkStyle) );
    aTickmarkProperties.RelativePos = static_cast<sal_Int32>(lcl_getTickOffset(aTickmarkProperties.Length+nTickStartDistanceToAxis,nTickmarkStyle));
    aTickmarkProperties.aLineProperties = this->makeLinePropertiesForDepth( 0 );
    return aTickmarkProperties;
}

TickmarkProperties AxisProperties::getBiggestTickmarkProperties()
{
    TickmarkProperties aTickmarkProperties;
    sal_Int32 nDepth = 0;
    sal_Int32 nTickmarkStyle = 3;//inner and outer tickmarks
    aTickmarkProperties.Length = lcl_calcTickLengthForDepth( nDepth,nTickmarkStyle );
    aTickmarkProperties.RelativePos = static_cast<sal_Int32>( lcl_getTickOffset( aTickmarkProperties.Length, nTickmarkStyle ) );
    return aTickmarkProperties;
}

//--------------------------------------------------------------------------

AxisProperties::AxisProperties( const uno::Reference< XAxis >& xAxisModel
                              , ExplicitCategoriesProvider* pExplicitCategoriesProvider )
    : m_xAxisModel(xAxisModel)
    , m_nDimensionIndex(0)
    , m_bIsMainAxis(true)
    , m_bSwapXAndY(false)
    , m_eCrossoverType( ::com::sun::star::chart::ChartAxisPosition_ZERO )
    , m_eLabelPos( ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS )
    , m_eTickmarkPos( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS )
    , m_pfMainLinePositionAtOtherAxis(NULL)
    , m_pfExrtaLinePositionAtOtherAxis(NULL)
    , m_bCrossingAxisHasReverseDirection(false)
    , m_bCrossingAxisIsCategoryAxes(false)
    , m_fLabelDirectionSign(1.0)
    , m_fInnerDirectionSign(1.0)
    , m_aLabelAlignment(LABEL_ALIGN_RIGHT_TOP)
    , m_bDisplayLabels( true )
    , m_nNumberFormatKey(0)
    , m_nMajorTickmarks(1)
    , m_nMinorTickmarks(1)
    , m_aTickmarkPropertiesList()
    , m_aLineProperties()
    //for category axes
    , m_nAxisType(AxisType::REALNUMBER)
    , m_bComplexCategories(false)
    , m_pExplicitCategoriesProvider(pExplicitCategoriesProvider)
    , m_xAxisTextProvider(0)
{
}

AxisProperties::AxisProperties( const AxisProperties& rAxisProperties )
    : m_xAxisModel( rAxisProperties.m_xAxisModel )
    , m_nDimensionIndex( m_nDimensionIndex )
    , m_bIsMainAxis( rAxisProperties.m_bIsMainAxis )
    , m_bSwapXAndY( rAxisProperties.m_bSwapXAndY )
    , m_eCrossoverType( rAxisProperties.m_eCrossoverType )
    , m_eLabelPos( rAxisProperties.m_eLabelPos )
    , m_eTickmarkPos( rAxisProperties.m_eTickmarkPos )
    , m_pfMainLinePositionAtOtherAxis( NULL )
    , m_pfExrtaLinePositionAtOtherAxis( NULL )
    , m_bCrossingAxisHasReverseDirection( rAxisProperties.m_bCrossingAxisHasReverseDirection )
    , m_bCrossingAxisIsCategoryAxes( rAxisProperties.m_bCrossingAxisIsCategoryAxes )
    , m_fLabelDirectionSign( rAxisProperties.m_fLabelDirectionSign )
    , m_fInnerDirectionSign( rAxisProperties.m_fInnerDirectionSign )
    , m_aLabelAlignment( rAxisProperties.m_aLabelAlignment )
    , m_bDisplayLabels( rAxisProperties.m_bDisplayLabels )
    , m_nNumberFormatKey( rAxisProperties.m_nNumberFormatKey )
    , m_nMajorTickmarks( rAxisProperties.m_nMajorTickmarks )
    , m_nMinorTickmarks( rAxisProperties.m_nMinorTickmarks )
    , m_aTickmarkPropertiesList( rAxisProperties.m_aTickmarkPropertiesList )
    , m_aLineProperties( rAxisProperties.m_aLineProperties )
    //for category axes
    , m_nAxisType( rAxisProperties.m_nAxisType )
    , m_bComplexCategories( rAxisProperties.m_bComplexCategories )
    , m_pExplicitCategoriesProvider( rAxisProperties.m_pExplicitCategoriesProvider )
    , m_xAxisTextProvider( rAxisProperties.m_xAxisTextProvider )
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
    LabelAlignment aRet( LABEL_ALIGN_RIGHT );
    if( rAxisProperties.m_fLabelDirectionSign<0 )
        aRet = LABEL_ALIGN_LEFT;
    return aRet;
}

LabelAlignment lcl_getLabelAlignmentForYAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_RIGHT );
    if( rAxisProperties.m_fLabelDirectionSign<0 )
        aRet = LABEL_ALIGN_LEFT;
    return aRet;
}

LabelAlignment lcl_getLabelAlignmentForXAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_BOTTOM );
    if( rAxisProperties.m_fLabelDirectionSign<0 )
        aRet = LABEL_ALIGN_TOP;
    return aRet;
}

void AxisProperties::initAxisPositioning( const uno::Reference< beans::XPropertySet >& xAxisProp )
{
    if( !xAxisProp.is() )
        return;
    try
    {
        if( AxisHelper::isAxisPositioningEnabled() )
        {
            xAxisProp->getPropertyValue(C2U( "CrossoverPosition" )) >>= m_eCrossoverType;
            if( ::com::sun::star::chart::ChartAxisPosition_VALUE == m_eCrossoverType )
            {
                double fValue = 0.0;
                xAxisProp->getPropertyValue(C2U( "CrossoverValue" )) >>= fValue;

                if( m_bCrossingAxisIsCategoryAxes )
                    fValue = ::rtl::math::round(fValue);
                m_pfMainLinePositionAtOtherAxis = new double(fValue);
            }
            else if( ::com::sun::star::chart::ChartAxisPosition_ZERO == m_eCrossoverType )
                m_pfMainLinePositionAtOtherAxis = new double(0.0);

            xAxisProp->getPropertyValue(C2U( "LabelPosition" )) >>= m_eLabelPos;
            xAxisProp->getPropertyValue(C2U( "MarkPosition" )) >>= m_eTickmarkPos;
        }
        else
        {
            m_eCrossoverType = ::com::sun::star::chart::ChartAxisPosition_START;
            if( m_bIsMainAxis == m_bCrossingAxisHasReverseDirection )
                m_eCrossoverType = ::com::sun::star::chart::ChartAxisPosition_END;
            m_eLabelPos = ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS;
            m_eTickmarkPos = ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS;
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

void AxisProperties::init( bool bCartesian )
{
    uno::Reference< beans::XPropertySet > xProp =
        uno::Reference<beans::XPropertySet>::query( this->m_xAxisModel );
    if( !xProp.is() )
        return;

    if( m_nDimensionIndex<2 )
        initAxisPositioning( xProp );

    ScaleData aScaleData = m_xAxisModel->getScaleData();
    if( m_nDimensionIndex==0 )
        AxisHelper::checkDateAxis( aScaleData, m_pExplicitCategoriesProvider, bCartesian );
    m_nAxisType = aScaleData.AxisType;

    if( bCartesian )
    {
        if( m_nDimensionIndex == 0 && m_nAxisType == AxisType::CATEGORY
                && m_pExplicitCategoriesProvider && m_pExplicitCategoriesProvider->hasComplexCategories() )
            m_bComplexCategories = true;

        if( ::com::sun::star::chart::ChartAxisPosition_END == m_eCrossoverType )
            m_fInnerDirectionSign = m_bCrossingAxisHasReverseDirection ? 1 : -1;
        else
            m_fInnerDirectionSign = m_bCrossingAxisHasReverseDirection ? -1 : 1;

        if( ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS == m_eLabelPos )
            m_fLabelDirectionSign = m_fInnerDirectionSign;
        else if( ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE == m_eLabelPos )
            m_fLabelDirectionSign = -m_fInnerDirectionSign;
        else if( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START == m_eLabelPos )
            m_fLabelDirectionSign = m_bCrossingAxisHasReverseDirection ? -1 : 1;
        else if( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END == m_eLabelPos )
            m_fLabelDirectionSign = m_bCrossingAxisHasReverseDirection ? 1 : -1;

        if( m_nDimensionIndex==2 )
            m_aLabelAlignment = lcl_getLabelAlignmentForZAxis(*this);
        else
        {
            bool bIsYAxisPosition = (m_nDimensionIndex==1 && !m_bSwapXAndY)
                || (m_nDimensionIndex==0 && m_bSwapXAndY);
            if( bIsYAxisPosition )
            {
                m_fLabelDirectionSign*=-1;
                m_fInnerDirectionSign*=-1;
            }

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
                        : m_aFontReferenceSize( ChartModelHelper::getDefaultPageSize() )
                        , m_aMaximumSpaceForLabels( 0 , 0, m_aFontReferenceSize.Width, m_aFontReferenceSize.Height )
                        , nNumberFormatKey(0)
                        , eStaggering( SIDE_BY_SIDE )
                        , bLineBreakAllowed( false )
                        , bOverlapAllowed( false )
                        , bStackCharacters( false )
                        , fRotationAngleDegree( 0.0 )
                        , nRhythm( 1 )
                        , bRhythmIsFix(false)
{

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

sal_Bool AxisLabelProperties::getIsStaggered() const
{
    if( STAGGER_ODD == eStaggering || STAGGER_EVEN == eStaggering )
        return sal_True;
    return sal_False;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
