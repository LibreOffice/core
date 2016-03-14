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

#include "VAxisProperties.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart {

AxisLabelAlignment::AxisLabelAlignment() :
    mfLabelDirection(1.0),
    mfInnerTickDirection(1.0),
    meAlignment(LABEL_ALIGN_RIGHT_TOP) {}

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
    //0.0: completely inner
    //1.0: completely outer
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

    if (maLabelAlignment.mfInnerTickDirection == 0.0)
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
    sal_Int32 nTickmarkStyle = (maLabelAlignment.mfLabelDirection == maLabelAlignment.mfInnerTickDirection) ? 2/*outside*/ : 1/*inside*/;

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

AxisProperties::AxisProperties( const uno::Reference< XAxis >& xAxisModel
                              , ExplicitCategoriesProvider* pExplicitCategoriesProvider )
    : m_xAxisModel(xAxisModel)
    , m_nDimensionIndex(0)
    , m_bIsMainAxis(true)
    , m_bSwapXAndY(false)
    , m_eCrossoverType( css::chart::ChartAxisPosition_ZERO )
    , m_eLabelPos( css::chart::ChartAxisLabelPosition_NEAR_AXIS )
    , m_eTickmarkPos( css::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS )
    , m_bCrossingAxisHasReverseDirection(false)
    , m_bCrossingAxisIsCategoryAxes(false)
    , m_bDisplayLabels( true )
    , m_bTryStaggeringFirst( false )
    , m_nNumberFormatKey(0)
    , m_nMajorTickmarks(1)
    , m_nMinorTickmarks(1)
    , m_aTickmarkPropertiesList()
    , m_aLineProperties()
    //for category axes
    , m_nAxisType(AxisType::REALNUMBER)
    , m_bComplexCategories(false)
    , m_pExplicitCategoriesProvider(pExplicitCategoriesProvider)
    , m_xAxisTextProvider(nullptr)
{
}

AxisProperties::AxisProperties( const AxisProperties& rAxisProperties )
    : m_xAxisModel( rAxisProperties.m_xAxisModel )
    , m_nDimensionIndex( rAxisProperties.m_nDimensionIndex )
    , m_bIsMainAxis( rAxisProperties.m_bIsMainAxis )
    , m_bSwapXAndY( rAxisProperties.m_bSwapXAndY )
    , m_eCrossoverType( rAxisProperties.m_eCrossoverType )
    , m_eLabelPos( rAxisProperties.m_eLabelPos )
    , m_eTickmarkPos( rAxisProperties.m_eTickmarkPos )
    , m_bCrossingAxisHasReverseDirection( rAxisProperties.m_bCrossingAxisHasReverseDirection )
    , m_bCrossingAxisIsCategoryAxes( rAxisProperties.m_bCrossingAxisIsCategoryAxes )
    , maLabelAlignment( rAxisProperties.maLabelAlignment )
    , m_bDisplayLabels( rAxisProperties.m_bDisplayLabels )
    , m_bTryStaggeringFirst( rAxisProperties.m_bTryStaggeringFirst )
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
        m_pfMainLinePositionAtOtherAxis.reset(*rAxisProperties.m_pfMainLinePositionAtOtherAxis);
    if( rAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        m_pfExrtaLinePositionAtOtherAxis.reset(*rAxisProperties.m_pfExrtaLinePositionAtOtherAxis);
}

LabelAlignment lcl_getLabelAlignmentForZAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_RIGHT );
    if (rAxisProperties.maLabelAlignment.mfLabelDirection < 0)
        aRet = LABEL_ALIGN_LEFT;
    return aRet;
}

LabelAlignment lcl_getLabelAlignmentForYAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_RIGHT );
    if (rAxisProperties.maLabelAlignment.mfLabelDirection < 0)
        aRet = LABEL_ALIGN_LEFT;
    return aRet;
}

LabelAlignment lcl_getLabelAlignmentForXAxis( const AxisProperties& rAxisProperties )
{
    LabelAlignment aRet( LABEL_ALIGN_BOTTOM );
    if (rAxisProperties.maLabelAlignment.mfLabelDirection < 0)
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
            xAxisProp->getPropertyValue("CrossoverPosition") >>= m_eCrossoverType;
            if( css::chart::ChartAxisPosition_VALUE == m_eCrossoverType )
            {
                double fValue = 0.0;
                xAxisProp->getPropertyValue("CrossoverValue") >>= fValue;

                if( m_bCrossingAxisIsCategoryAxes )
                    fValue = ::rtl::math::round(fValue);
                m_pfMainLinePositionAtOtherAxis.reset(fValue);
            }
            else if( css::chart::ChartAxisPosition_ZERO == m_eCrossoverType )
                m_pfMainLinePositionAtOtherAxis.reset(0.0);

            xAxisProp->getPropertyValue("LabelPosition") >>= m_eLabelPos;
            xAxisProp->getPropertyValue("MarkPosition") >>= m_eTickmarkPos;
        }
        else
        {
            m_eCrossoverType = css::chart::ChartAxisPosition_START;
            if( m_bIsMainAxis == m_bCrossingAxisHasReverseDirection )
                m_eCrossoverType = css::chart::ChartAxisPosition_END;
            m_eLabelPos = css::chart::ChartAxisLabelPosition_NEAR_AXIS;
            m_eTickmarkPos = css::chart::ChartAxisMarkPosition_AT_LABELS;
        }
    }
    catch( const uno::Exception& e )
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

        if( css::chart::ChartAxisPosition_END == m_eCrossoverType )
            maLabelAlignment.mfInnerTickDirection = m_bCrossingAxisHasReverseDirection ? 1.0 : -1.0;
        else
            maLabelAlignment.mfInnerTickDirection = m_bCrossingAxisHasReverseDirection ? -1.0 : 1.0;

        if( css::chart::ChartAxisLabelPosition_NEAR_AXIS == m_eLabelPos )
            maLabelAlignment.mfLabelDirection = maLabelAlignment.mfInnerTickDirection;
        else if( css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE == m_eLabelPos )
            maLabelAlignment.mfLabelDirection = -maLabelAlignment.mfInnerTickDirection;
        else if( css::chart::ChartAxisLabelPosition_OUTSIDE_START == m_eLabelPos )
            maLabelAlignment.mfLabelDirection = m_bCrossingAxisHasReverseDirection ? -1 : 1;
        else if( css::chart::ChartAxisLabelPosition_OUTSIDE_END == m_eLabelPos )
            maLabelAlignment.mfLabelDirection = m_bCrossingAxisHasReverseDirection ? 1 : -1;

        if( m_nDimensionIndex==2 )
            maLabelAlignment.meAlignment = lcl_getLabelAlignmentForZAxis(*this);
        else
        {
            bool bIsYAxisPosition = (m_nDimensionIndex==1 && !m_bSwapXAndY)
                || (m_nDimensionIndex==0 && m_bSwapXAndY);
            if( bIsYAxisPosition )
            {
                maLabelAlignment.mfLabelDirection *= -1.0;
                maLabelAlignment.mfInnerTickDirection *= -1.0;
            }

            if( bIsYAxisPosition )
                maLabelAlignment.meAlignment = lcl_getLabelAlignmentForYAxis(*this);
            else
                maLabelAlignment.meAlignment = lcl_getLabelAlignmentForXAxis(*this);
        }
    }

    try
    {
        //init LineProperties
        m_aLineProperties.initFromPropertySet( xProp );

        //init display labels
        xProp->getPropertyValue( "DisplayLabels" ) >>= m_bDisplayLabels;

        // Init layout strategy hint for axis labels.
        // Compatibility option: starting from LibreOffice 5.1 the rotated
        // layout is preferred to staggering for axis labels.
        xProp->getPropertyValue( "TryStaggeringFirst" ) >>= m_bTryStaggeringFirst;

        //init TickmarkProperties
        xProp->getPropertyValue( "MajorTickmarks" ) >>= m_nMajorTickmarks;
        xProp->getPropertyValue( "MinorTickmarks" ) >>= m_nMinorTickmarks;

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
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

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
            xProp->getPropertyValue( "TextBreak" ) >>= this->bLineBreakAllowed;
            xProp->getPropertyValue( "TextOverlap" ) >>= this->bOverlapAllowed;
            xProp->getPropertyValue( "StackCharacters" ) >>= this->bStackCharacters;
            xProp->getPropertyValue( "TextRotation" ) >>= this->fRotationAngleDegree;

            css::chart::ChartAxisArrangeOrderType eArrangeOrder;
            xProp->getPropertyValue( "ArrangeOrder" ) >>= eArrangeOrder;
            switch(eArrangeOrder)
            {
                case css::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
                    this->eStaggering = SIDE_BY_SIDE;
                    break;
                case css::chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
                    this->eStaggering = STAGGER_EVEN;
                    break;
                case css::chart::ChartAxisArrangeOrderType_STAGGER_ODD:
                    this->eStaggering = STAGGER_ODD;
                    break;
                default:
                    this->eStaggering = STAGGER_AUTO;
                    break;
            }
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

bool AxisLabelProperties::isStaggered() const
{
    return ( STAGGER_ODD == eStaggering || STAGGER_EVEN == eStaggering );
}

void AxisLabelProperties::autoRotate45()
{
    fRotationAngleDegree = 45;
    bLineBreakAllowed = false;
    eStaggering = SIDE_BY_SIDE;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
