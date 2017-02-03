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

#include "AxisItemConverter.hxx"
#include "ItemPropertyMap.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "CommonConverters.hxx"
#include "ChartTypeHelper.hxx"
#include <unonames.hxx>
#include <memory>

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/AxisOrientation.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

#include <o3tl/any.hxx>
#include <svl/eitem.hxx>
#include <svx/chrtitem.hxx>
#include <svl/intitem.hxx>
#include <rtl/math.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::chart::TimeInterval;
using ::com::sun::star::chart::TimeIncrement;

namespace chart { namespace wrapper {

namespace {

ItemPropertyMapType & lcl_GetAxisPropertyMap()
{
    static ItemPropertyMapType aAxisPropertyMap{
        {SCHATTR_AXIS_SHOWDESCR,     {"DisplayLabels",    0}},
        {SCHATTR_AXIS_TICKS,         {"MajorTickmarks",   0}},
        {SCHATTR_AXIS_HELPTICKS,     {"MinorTickmarks",   0}},
        {SCHATTR_AXIS_LABEL_ORDER,   {"ArrangeOrder",     0}},
        {SCHATTR_TEXT_STACKED,       {"StackCharacters",  0}},
        {SCHATTR_AXIS_LABEL_BREAK,   {"TextBreak",        0}},
        {SCHATTR_AXIS_LABEL_OVERLAP, {"TextOverlap",      0}}};
    return aAxisPropertyMap;
};

} // anonymous namespace

AxisItemConverter::AxisItemConverter(
    const Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const Reference< chart2::XChartDocument > & xChartDoc,
    ::chart::ExplicitScaleData * pScale /* = NULL */,
    ::chart::ExplicitIncrementData * pIncrement /* = NULL */,
    const awt::Size* pRefSize ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_xChartDoc( xChartDoc ),
        m_pExplicitScale( nullptr ),
        m_pExplicitIncrement( nullptr )
{
    Reference< lang::XMultiServiceFactory > xNamedPropertyContainerFactory( xChartDoc, uno::UNO_QUERY );

    if( pScale )
        m_pExplicitScale = new ::chart::ExplicitScaleData( *pScale );
    if( pIncrement )
        m_pExplicitIncrement = new ::chart::ExplicitIncrementData( *pIncrement );

    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 xNamedPropertyContainerFactory,
                                 GraphicPropertyItemConverter::LINE_PROPERTIES ));
    m_aConverters.push_back(
        new CharacterPropertyItemConverter(rPropertySet, rItemPool, pRefSize, "ReferencePageSize"));

    m_xAxis.set( Reference< chart2::XAxis >( rPropertySet, uno::UNO_QUERY ) );
    OSL_ASSERT( m_xAxis.is());
}

AxisItemConverter::~AxisItemConverter()
{
    delete m_pExplicitScale;
    delete m_pExplicitIncrement;

    ::std::for_each(m_aConverters.begin(), m_aConverters.end(), std::default_delete<ItemConverter>());
}

void AxisItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    for( const auto& pConv : m_aConverters )
        pConv->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool AxisItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    for( const auto& pConv : m_aConverters )
        bResult = pConv->ApplyItemSet( rItemSet ) || bResult;

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const sal_uInt16 * AxisItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nAxisWhichPairs;
}

bool AxisItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ItemPropertyMapType & rMap( lcl_GetAxisPropertyMap());
    ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;

    return true;
}

bool lcl_hasTimeIntervalValue( const uno::Any& rAny )
{
    bool bRet = false;
    TimeInterval aValue;
    if( rAny >>= aValue )
        bRet = true;
    return bRet;
}

void AxisItemConverter::FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
{
    if( !m_xAxis.is() )
        return;

    const chart2::ScaleData&     rScale( m_xAxis->getScaleData() );
    const chart2::IncrementData& rIncrement( rScale.IncrementData );
    const uno::Sequence< chart2::SubIncrement >& rSubIncrements( rScale.IncrementData.SubIncrements );
    const TimeIncrement& rTimeIncrement( rScale.TimeIncrement );
    bool bDateAxis = (chart2::AxisType::DATE == rScale.AxisType);
    if( m_pExplicitScale )
        bDateAxis = (chart2::AxisType::DATE == m_pExplicitScale->AxisType);

    switch( nWhichId )
    {
        case SCHATTR_AXIS_AUTO_MAX:
                rOutItemSet.Put( SfxBoolItem( nWhichId, !hasDoubleValue(rScale.Maximum) ) );
            break;

        case SCHATTR_AXIS_MAX:
            {
                double fMax = 10.0;
                if( rScale.Maximum >>= fMax )
                    rOutItemSet.Put( SvxDoubleItem( fMax, nWhichId ) );
                else
                {
                    if( m_pExplicitScale )
                        fMax = m_pExplicitScale->Maximum;
                    rOutItemSet.Put( SvxDoubleItem( fMax, nWhichId ) );
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_MIN:
                rOutItemSet.Put( SfxBoolItem( nWhichId, !hasDoubleValue(rScale.Minimum) ) );
            break;

        case SCHATTR_AXIS_MIN:
            {
                double fMin = 0.0;
                if( rScale.Minimum >>= fMin )
                    rOutItemSet.Put( SvxDoubleItem( fMin, nWhichId ) );
                else if( m_pExplicitScale )
                    rOutItemSet.Put( SvxDoubleItem( m_pExplicitScale->Minimum, nWhichId ));
            }
            break;

        case SCHATTR_AXIS_LOGARITHM:
            {
                bool bValue = AxisHelper::isLogarithmic( rScale.Scaling );
                rOutItemSet.Put( SfxBoolItem( nWhichId, bValue ));
            }
            break;

        case SCHATTR_AXIS_REVERSE:
                rOutItemSet.Put( SfxBoolItem( nWhichId, (AxisOrientation_REVERSE == rScale.Orientation) ));
            break;

        // Increment
        case SCHATTR_AXIS_AUTO_STEP_MAIN:
            if( bDateAxis )
                rOutItemSet.Put( SfxBoolItem( nWhichId, !lcl_hasTimeIntervalValue(rTimeIncrement.MajorTimeInterval) ) );
            else
                rOutItemSet.Put( SfxBoolItem( nWhichId, !hasDoubleValue(rIncrement.Distance) ) );
            break;

        case SCHATTR_AXIS_MAIN_TIME_UNIT:
            {
                TimeInterval aTimeInterval;
                if( rTimeIncrement.MajorTimeInterval >>= aTimeInterval )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, aTimeInterval.TimeUnit ) );
                else if( m_pExplicitIncrement )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, m_pExplicitIncrement->MajorTimeInterval.TimeUnit ) );
            }
            break;

        case SCHATTR_AXIS_STEP_MAIN:
            if( bDateAxis )
            {
                TimeInterval aTimeInterval;
                if( rTimeIncrement.MajorTimeInterval >>= aTimeInterval )
                    rOutItemSet.Put( SvxDoubleItem(aTimeInterval.Number, nWhichId ));
                else if( m_pExplicitIncrement )
                    rOutItemSet.Put( SvxDoubleItem( m_pExplicitIncrement->MajorTimeInterval.Number, nWhichId ));
            }
            else
            {
                double fDistance = 1.0;
                if( rIncrement.Distance >>= fDistance )
                    rOutItemSet.Put( SvxDoubleItem(fDistance, nWhichId ));
                else if( m_pExplicitIncrement )
                    rOutItemSet.Put( SvxDoubleItem( m_pExplicitIncrement->Distance, nWhichId ));
            }
            break;

        // SubIncrement
        case SCHATTR_AXIS_AUTO_STEP_HELP:
            if( bDateAxis )
                rOutItemSet.Put( SfxBoolItem( nWhichId, !lcl_hasTimeIntervalValue(rTimeIncrement.MinorTimeInterval) ) );
            else
                rOutItemSet.Put( SfxBoolItem( nWhichId,
                    ! ( rSubIncrements.getLength() > 0 && rSubIncrements[0].IntervalCount.hasValue() )));
            break;

        case SCHATTR_AXIS_HELP_TIME_UNIT:
            {
                TimeInterval aTimeInterval;
                if( rTimeIncrement.MinorTimeInterval >>= aTimeInterval )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, aTimeInterval.TimeUnit ) );
                else if( m_pExplicitIncrement )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, m_pExplicitIncrement->MinorTimeInterval.TimeUnit ) );
            }
            break;

        case SCHATTR_AXIS_STEP_HELP:
            if( bDateAxis )
            {
                TimeInterval aTimeInterval;
                if( rTimeIncrement.MinorTimeInterval >>= aTimeInterval )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, aTimeInterval.Number ));
                else if( m_pExplicitIncrement )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, m_pExplicitIncrement->MinorTimeInterval.Number ));
            }
            else
            {
                if( rSubIncrements.getLength() > 0 && rSubIncrements[0].IntervalCount.hasValue())
                {
                    rOutItemSet.Put( SfxInt32Item( nWhichId,
                            *o3tl::doAccess<sal_Int32>(
                                rSubIncrements[0].IntervalCount) ));
                }
                else
                {
                    if( m_pExplicitIncrement && !m_pExplicitIncrement->SubIncrements.empty() )
                    {
                        rOutItemSet.Put( SfxInt32Item( nWhichId,
                                m_pExplicitIncrement->SubIncrements[0].IntervalCount ));
                    }
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_TIME_RESOLUTION:
            {
                rOutItemSet.Put( SfxBoolItem( nWhichId,
                        !rTimeIncrement.TimeResolution.hasValue() ));
            }
            break;
        case SCHATTR_AXIS_TIME_RESOLUTION:
            {
                long nTimeResolution=0;
                if( rTimeIncrement.TimeResolution >>= nTimeResolution )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, nTimeResolution ) );
                else if( m_pExplicitScale )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, m_pExplicitScale->TimeResolution ) );
            }
            break;

        case SCHATTR_AXIS_AUTO_ORIGIN:
        {
            rOutItemSet.Put( SfxBoolItem( nWhichId, ( !hasDoubleValue(rScale.Origin) )));
        }
        break;

        case SCHATTR_AXIS_ORIGIN:
        {
            double fOrigin = 0.0;
            if( !(rScale.Origin >>= fOrigin) )
            {
                if( m_pExplicitScale )
                    fOrigin = m_pExplicitScale->Origin;
            }
            rOutItemSet.Put( SvxDoubleItem( fOrigin, nWhichId ));
        }
        break;

        case SCHATTR_AXIS_POSITION:
        {
            css::chart::ChartAxisPosition eAxisPos( css::chart::ChartAxisPosition_ZERO );
            GetPropertySet()->getPropertyValue( "CrossoverPosition" ) >>= eAxisPos;
            rOutItemSet.Put( SfxInt32Item( nWhichId, eAxisPos ) );
        }
        break;

        case SCHATTR_AXIS_POSITION_VALUE:
        {
            double fValue = 0.0;
            if( GetPropertySet()->getPropertyValue( "CrossoverValue" ) >>= fValue )
                rOutItemSet.Put( SvxDoubleItem( fValue, nWhichId ) );
        }
        break;

        case SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT:
        {
            //read only item
            //necessary tp display the crossing value with an appropriate format

            Reference< chart2::XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis(
                m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ) );

            Reference< chart2::XAxis > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( m_xAxis, xCooSys ) );

            sal_Int32 nFormatKey = ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                xCrossingMainAxis, xCooSys, m_xChartDoc);

            rOutItemSet.Put( SfxUInt32Item( nWhichId, nFormatKey ));
        }
        break;

        case SCHATTR_AXIS_LABEL_POSITION:
        {
            css::chart::ChartAxisLabelPosition ePos( css::chart::ChartAxisLabelPosition_NEAR_AXIS );
            GetPropertySet()->getPropertyValue( "LabelPosition" ) >>= ePos;
            rOutItemSet.Put( SfxInt32Item( nWhichId, ePos ) );
        }
        break;

        case SCHATTR_AXIS_MARK_POSITION:
        {
            css::chart::ChartAxisMarkPosition ePos( css::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS );
            GetPropertySet()->getPropertyValue( "MarkPosition" ) >>= ePos;
            rOutItemSet.Put( SfxInt32Item( nWhichId, ePos ) );
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            // convert double to int (times 100)
            double fVal = 0;

            if( GetPropertySet()->getPropertyValue( "TextRotation" ) >>= fVal )
            {
                rOutItemSet.Put( SfxInt32Item( nWhichId, static_cast< sal_Int32 >(
                                                   ::rtl::math::round( fVal * 100.0 ) ) ));
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            if( m_pExplicitScale )
            {
                Reference< chart2::XCoordinateSystem > xCooSys(
                        AxisHelper::getCoordinateSystemOfAxis(
                              m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ) );

                sal_Int32 nFormatKey = ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                    m_xAxis, xCooSys, m_xChartDoc);

                rOutItemSet.Put( SfxUInt32Item( nWhichId, nFormatKey ));
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_SOURCE:
        {
            bool bLinkToSource = true;
            GetPropertySet()->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkToSource;
            rOutItemSet.Put(SfxBoolItem(nWhichId, bLinkToSource));
        }
        break;

        case SCHATTR_AXISTYPE:
            rOutItemSet.Put( SfxInt32Item( nWhichId, rScale.AxisType ));
        break;

        case SCHATTR_AXIS_AUTO_DATEAXIS:
            rOutItemSet.Put( SfxBoolItem( nWhichId, rScale.AutoDateAxis ));
        break;

        case SCHATTR_AXIS_ALLOW_DATEAXIS:
        {
            Reference< chart2::XCoordinateSystem > xCooSys(
                AxisHelper::getCoordinateSystemOfAxis( m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ) );
            sal_Int32 nDimensionIndex=0; sal_Int32 nAxisIndex=0;
            AxisHelper::getIndicesForAxis(m_xAxis, xCooSys, nDimensionIndex, nAxisIndex );
            bool bChartTypeAllowsDateAxis = ChartTypeHelper::isSupportingDateAxis( AxisHelper::getChartTypeByIndex( xCooSys, 0 ), 2, nDimensionIndex );
            rOutItemSet.Put( SfxBoolItem( nWhichId, bChartTypeAllowsDateAxis ));
        }
        break;
    }
}

bool lcl_isDateAxis( const SfxItemSet & rItemSet )
{
    sal_Int32 nAxisType = static_cast< const SfxInt32Item & >( rItemSet.Get( SCHATTR_AXISTYPE )).GetValue();//css::chart2::AxisType
    return (chart2::AxisType::DATE == nAxisType);
}

bool lcl_isAutoMajor( const SfxItemSet & rItemSet )
{
    bool bRet = static_cast< const SfxBoolItem & >( rItemSet.Get( SCHATTR_AXIS_AUTO_STEP_MAIN )).GetValue();
    return bRet;
}

bool lcl_isAutoMinor( const SfxItemSet & rItemSet )
{
    bool bRet = static_cast< const SfxBoolItem & >( rItemSet.Get( SCHATTR_AXIS_AUTO_STEP_HELP )).GetValue();
    return bRet;
}

bool AxisItemConverter::ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    if( !m_xAxis.is() )
        return false;

    chart2::ScaleData     aScale( m_xAxis->getScaleData() );

    bool bSetScale    = false;
    bool bChangedOtherwise = false;

    uno::Any aValue;

    switch( nWhichId )
    {
        case SCHATTR_AXIS_AUTO_MAX:
            if( (static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                aScale.Maximum.clear();
                bSetScale = true;
            }
            // else SCHATTR_AXIS_MAX must have some value
            break;

        case SCHATTR_AXIS_MAX:
            // only if auto if false
            if( ! (static_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_MAX )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aScale.Maximum != aValue )
                {
                    aScale.Maximum = aValue;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_MIN:
            if( (static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                aScale.Minimum.clear();
                bSetScale = true;
            }
            // else SCHATTR_AXIS_MIN must have some value
            break;

        case SCHATTR_AXIS_MIN:
            // only if auto if false
            if( ! (static_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_MIN )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aScale.Minimum != aValue )
                {
                    aScale.Minimum = aValue;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_LOGARITHM:
        {
            bool bWasLogarithm = AxisHelper::isLogarithmic( aScale.Scaling );

            if( (static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                // logarithm is true
                if( ! bWasLogarithm )
                {
                    aScale.Scaling = AxisHelper::createLogarithmicScaling( 10.0 );
                    bSetScale = true;
                }
            }
            else
            {
                // logarithm is false => linear scaling
                if( bWasLogarithm )
                {
                    aScale.Scaling = AxisHelper::createLinearScaling();
                    bSetScale = true;
                }
            }
        }
        break;

        case SCHATTR_AXIS_REVERSE:
        {
            bool bWasReverse = ( AxisOrientation_REVERSE == aScale.Orientation );
            bool bNewReverse = (static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() );
            if( bWasReverse != bNewReverse )
            {
                aScale.Orientation = bNewReverse ? AxisOrientation_REVERSE : AxisOrientation_MATHEMATICAL;
                bSetScale = true;
            }
        }
        break;

        // Increment
        case SCHATTR_AXIS_AUTO_STEP_MAIN:
            if( lcl_isAutoMajor(rItemSet) )
            {
                aScale.IncrementData.Distance.clear();
                aScale.TimeIncrement.MajorTimeInterval.clear();
                bSetScale = true;
            }
            // else SCHATTR_AXIS_STEP_MAIN must have some value
            break;

        case SCHATTR_AXIS_MAIN_TIME_UNIT:
            if( !lcl_isAutoMajor(rItemSet) )
            {
                if( rItemSet.Get( nWhichId ).QueryValue( aValue ) )
                {
                    TimeInterval aTimeInterval;
                    aScale.TimeIncrement.MajorTimeInterval >>= aTimeInterval;
                    aValue >>= aTimeInterval.TimeUnit;
                    aScale.TimeIncrement.MajorTimeInterval <<= aTimeInterval;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_STEP_MAIN:
            // only if auto if false
            if( !lcl_isAutoMajor(rItemSet) )
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );
                if( lcl_isDateAxis(rItemSet) )
                {
                    double fValue = 1.0;
                    if( aValue >>= fValue )
                    {
                        TimeInterval aTimeInterval;
                        aScale.TimeIncrement.MajorTimeInterval >>= aTimeInterval;
                        aTimeInterval.Number = static_cast<sal_Int32>(fValue);
                        aScale.TimeIncrement.MajorTimeInterval <<= aTimeInterval;
                        bSetScale = true;
                    }
                }
                else if( aScale.IncrementData.Distance != aValue )
                {
                    aScale.IncrementData.Distance = aValue;
                    bSetScale = true;
                }
            }
            break;

        // SubIncrement
        case SCHATTR_AXIS_AUTO_STEP_HELP:
            if( lcl_isAutoMinor(rItemSet) )
            {
                if( aScale.IncrementData.SubIncrements.getLength() > 0 &&
                    aScale.IncrementData.SubIncrements[0].IntervalCount.hasValue() )
                {
                        aScale.IncrementData.SubIncrements[0].IntervalCount.clear();
                        bSetScale = true;
                }
                if( aScale.TimeIncrement.MinorTimeInterval.hasValue() )
                {
                    aScale.TimeIncrement.MinorTimeInterval.clear();
                    bSetScale = true;
                }
            }
            // else SCHATTR_AXIS_STEP_MAIN must have some value
            break;

        case SCHATTR_AXIS_HELP_TIME_UNIT:
            if( !lcl_isAutoMinor(rItemSet) )
            {
                if( rItemSet.Get( nWhichId ).QueryValue( aValue ) )
                {
                    TimeInterval aTimeInterval;
                    aScale.TimeIncrement.MinorTimeInterval >>= aTimeInterval;
                    aValue >>= aTimeInterval.TimeUnit;
                    aScale.TimeIncrement.MinorTimeInterval <<= aTimeInterval;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_STEP_HELP:
            // only if auto is false
            if( !lcl_isAutoMinor(rItemSet) )
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );
                if( lcl_isDateAxis(rItemSet) )
                {
                    TimeInterval aTimeInterval;
                    aScale.TimeIncrement.MinorTimeInterval >>= aTimeInterval;
                    aValue >>= aTimeInterval.Number;
                    aScale.TimeIncrement.MinorTimeInterval <<= aTimeInterval;
                    bSetScale = true;
                }
                else if( aScale.IncrementData.SubIncrements.getLength() > 0 )
                {
                    if( ! aScale.IncrementData.SubIncrements[0].IntervalCount.hasValue() ||
                        aScale.IncrementData.SubIncrements[0].IntervalCount != aValue )
                    {
                        OSL_ASSERT( aValue.getValueTypeClass() == uno::TypeClass_LONG );
                        aScale.IncrementData.SubIncrements[0].IntervalCount = aValue;
                        bSetScale = true;
                    }
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_TIME_RESOLUTION:
            if( (static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue() ))
            {
                aScale.TimeIncrement.TimeResolution.clear();
                bSetScale = true;
            }
            break;
        case SCHATTR_AXIS_TIME_RESOLUTION:
            // only if auto is false
            if( ! (static_cast< const SfxBoolItem & >( rItemSet.Get( SCHATTR_AXIS_AUTO_TIME_RESOLUTION )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aScale.TimeIncrement.TimeResolution != aValue )
                {
                    aScale.TimeIncrement.TimeResolution = aValue;
                    bSetScale = true;
                }
            }
            break;

        case SCHATTR_AXIS_AUTO_ORIGIN:
        {
            if( (static_cast< const SfxBoolItem & >(
                     rItemSet.Get( nWhichId )).GetValue() ))
            {
                aScale.Origin.clear();
                bSetScale = true;
            }
        }
        break;

        case SCHATTR_AXIS_ORIGIN:
        {
            // only if auto is false
            if( ! (static_cast< const SfxBoolItem & >(
                       rItemSet.Get( SCHATTR_AXIS_AUTO_ORIGIN )).GetValue() ))
            {
                rItemSet.Get( nWhichId ).QueryValue( aValue );

                if( aScale.Origin != aValue )
                {
                    aScale.Origin = aValue;
                    bSetScale = true;

                    if( !AxisHelper::isAxisPositioningEnabled() )
                    {
                        //keep old and new settings for axis positioning in sync somehow
                        Reference< chart2::XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis(
                            m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ) );

                        sal_Int32 nDimensionIndex=0;
                        sal_Int32 nAxisIndex=0;
                        if( AxisHelper::getIndicesForAxis( m_xAxis, xCooSys, nDimensionIndex, nAxisIndex ) && nAxisIndex==0 )
                        {
                            Reference< beans::XPropertySet > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( m_xAxis, xCooSys ), uno::UNO_QUERY );
                            if( xCrossingMainAxis.is() )
                            {
                                double fValue = 0.0;
                                if( aValue >>= fValue )
                                {
                                    xCrossingMainAxis->setPropertyValue( "CrossoverPosition" , uno::Any( css::chart::ChartAxisPosition_VALUE ));
                                    xCrossingMainAxis->setPropertyValue( "CrossoverValue" , uno::Any( fValue ));
                                }
                                else
                                    xCrossingMainAxis->setPropertyValue( "CrossoverPosition" , uno::Any( css::chart::ChartAxisPosition_START ));
                            }
                        }
                    }
                }
            }
        }
        break;

        case SCHATTR_AXIS_POSITION:
        {
            css::chart::ChartAxisPosition eAxisPos =
                (css::chart::ChartAxisPosition)
                static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();

            css::chart::ChartAxisPosition eOldAxisPos( css::chart::ChartAxisPosition_ZERO );
            bool bPropExisted = ( GetPropertySet()->getPropertyValue( "CrossoverPosition" ) >>= eOldAxisPos );

            if( !bPropExisted || ( eOldAxisPos != eAxisPos ))
            {
                GetPropertySet()->setPropertyValue( "CrossoverPosition" , uno::Any( eAxisPos ));
                bChangedOtherwise = true;

                //move the parallel axes to the other side if necessary
                if( eAxisPos==css::chart::ChartAxisPosition_START || eAxisPos==css::chart::ChartAxisPosition_END )
                {
                    Reference< beans::XPropertySet > xParallelAxis( AxisHelper::getParallelAxis( m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ), uno::UNO_QUERY );
                    if( xParallelAxis.is() )
                    {
                        css::chart::ChartAxisPosition eOtherPos;
                        if( xParallelAxis->getPropertyValue( "CrossoverPosition" ) >>= eOtherPos )
                        {
                            if( eOtherPos == eAxisPos )
                            {
                                css::chart::ChartAxisPosition eOppositePos =
                                    (eAxisPos==css::chart::ChartAxisPosition_START)
                                    ? css::chart::ChartAxisPosition_END
                                    : css::chart::ChartAxisPosition_START;
                                xParallelAxis->setPropertyValue( "CrossoverPosition" , uno::Any( eOppositePos ));
                            }
                        }
                    }
                }
            }
        }
        break;

        case SCHATTR_AXIS_POSITION_VALUE:
        {
            double fValue = static_cast< const SvxDoubleItem & >( rItemSet.Get( nWhichId )).GetValue();

            double fOldValue = 0.0;
            bool bPropExisted = ( GetPropertySet()->getPropertyValue( "CrossoverValue" ) >>= fOldValue );

            if( !bPropExisted || ( fOldValue != fValue ))
            {
                GetPropertySet()->setPropertyValue( "CrossoverValue" , uno::Any( fValue ));
                bChangedOtherwise = true;

                //keep old and new settings for axis positioning in sync somehow
                {
                    Reference< chart2::XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis(
                        m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ) );

                    sal_Int32 nDimensionIndex=0;
                    sal_Int32 nAxisIndex=0;
                    if( AxisHelper::getIndicesForAxis( m_xAxis, xCooSys, nDimensionIndex, nAxisIndex ) && nAxisIndex==0 )
                    {
                        Reference< chart2::XAxis > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( m_xAxis, xCooSys ) );
                        if( xCrossingMainAxis.is() )
                        {
                            ScaleData aCrossingScale( xCrossingMainAxis->getScaleData() );
                            aCrossingScale.Origin = uno::Any(fValue);
                            xCrossingMainAxis->setScaleData(aCrossingScale);
                        }
                    }
                }
            }
        }
        break;

        case SCHATTR_AXIS_LABEL_POSITION:
        {
            css::chart::ChartAxisLabelPosition ePos =
                (css::chart::ChartAxisLabelPosition)
                static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();

            css::chart::ChartAxisLabelPosition eOldPos( css::chart::ChartAxisLabelPosition_NEAR_AXIS );
            bool bPropExisted = ( GetPropertySet()->getPropertyValue( "LabelPosition" ) >>= eOldPos );

            if( !bPropExisted || ( eOldPos != ePos ))
            {
                GetPropertySet()->setPropertyValue( "LabelPosition" , uno::Any( ePos ));
                bChangedOtherwise = true;

                //move the parallel axes to the other side if necessary
                if( ePos==css::chart::ChartAxisLabelPosition_OUTSIDE_START || ePos==css::chart::ChartAxisLabelPosition_OUTSIDE_END )
                {
                    Reference< beans::XPropertySet > xParallelAxis( AxisHelper::getParallelAxis( m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ), uno::UNO_QUERY );
                    if( xParallelAxis.is() )
                    {
                        css::chart::ChartAxisLabelPosition eOtherPos;
                        if( xParallelAxis->getPropertyValue( "LabelPosition" ) >>= eOtherPos )
                        {
                            if( eOtherPos == ePos )
                            {
                                css::chart::ChartAxisLabelPosition eOppositePos =
                                    (ePos==css::chart::ChartAxisLabelPosition_OUTSIDE_START)
                                    ? css::chart::ChartAxisLabelPosition_OUTSIDE_END
                                    : css::chart::ChartAxisLabelPosition_OUTSIDE_START;
                                xParallelAxis->setPropertyValue( "LabelPosition" , uno::Any( eOppositePos ));
                            }
                        }
                    }
                }
            }
        }
        break;

        case SCHATTR_AXIS_MARK_POSITION:
        {
            css::chart::ChartAxisMarkPosition ePos =
                (css::chart::ChartAxisMarkPosition)
                static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();

            css::chart::ChartAxisMarkPosition eOldPos( css::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS );
            bool bPropExisted = ( GetPropertySet()->getPropertyValue( "MarkPosition" ) >>= eOldPos );

            if( !bPropExisted || ( eOldPos != ePos ))
            {
                GetPropertySet()->setPropertyValue( "MarkPosition" , uno::Any( ePos ));
                bChangedOtherwise = true;
            }
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            // convert int to double (divided by 100)
            double fVal = static_cast< double >(
                static_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue()) / 100.0;
            double fOldVal = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( "TextRotation" ) >>= fOldVal );

            if( ! bPropExisted || fOldVal != fVal )
            {
                GetPropertySet()->setPropertyValue( "TextRotation" , uno::Any( fVal ));
                bChangedOtherwise = true;
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            if( m_pExplicitScale )
            {
                bool bUseSourceFormat =
                    (static_cast< const SfxBoolItem & >(
                        rItemSet.Get( SID_ATTR_NUMBERFORMAT_SOURCE )).GetValue() );

                if( ! bUseSourceFormat )
                {
                    sal_Int32 nFmt = static_cast< sal_Int32 >(
                        static_cast< const SfxUInt32Item & >(
                            rItemSet.Get( nWhichId )).GetValue());

                    aValue <<= nFmt;
                    if (GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT) != aValue)
                    {
                        GetPropertySet()->setPropertyValue(CHART_UNONAME_NUMFMT , aValue);
                        bChangedOtherwise = true;
                    }
                }
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_SOURCE:
        {
            bool bUseSourceFormat =
                (static_cast< const SfxBoolItem & >(
                    rItemSet.Get( nWhichId )).GetValue() );
            GetPropertySet()->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(bUseSourceFormat));

            bool bNumberFormatIsSet = GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT).hasValue();

            bChangedOtherwise = (bUseSourceFormat == bNumberFormatIsSet);
            if( bChangedOtherwise )
            {
                if( ! bUseSourceFormat )
                {
                    SfxItemState aState = rItemSet.GetItemState( SID_ATTR_NUMBERFORMAT_VALUE );
                    if( aState == SfxItemState::SET )
                    {
                        sal_Int32 nFormatKey = static_cast< sal_Int32 >(
                        static_cast< const SfxUInt32Item & >(
                            rItemSet.Get( SID_ATTR_NUMBERFORMAT_VALUE )).GetValue());
                        aValue <<= nFormatKey;
                    }
                    else
                    {
                        Reference< chart2::XCoordinateSystem > xCooSys(
                        AxisHelper::getCoordinateSystemOfAxis(
                              m_xAxis, ChartModelHelper::findDiagram( m_xChartDoc ) ) );

                        sal_Int32 nFormatKey = ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                            m_xAxis, xCooSys, m_xChartDoc);

                        aValue <<= nFormatKey;
                    }
                }
                // else set a void Any
                GetPropertySet()->setPropertyValue(CHART_UNONAME_NUMFMT , aValue);
            }
        }
        break;

        case SCHATTR_AXISTYPE:
        {
            sal_Int32 nNewAxisType = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();//css::chart2::AxisType
            aScale.AxisType = nNewAxisType;
            bSetScale = true;
        }
        break;

        case SCHATTR_AXIS_AUTO_DATEAXIS:
        {
            bool bNewValue = static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
            bool bOldValue = aScale.AutoDateAxis;
            if( bOldValue != bNewValue )
            {
                aScale.AutoDateAxis = bNewValue;
                bSetScale = true;
            }
        }
        break;
    }

    if( bSetScale )
        m_xAxis->setScaleData( aScale );

    return (bSetScale || bChangedOtherwise);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
