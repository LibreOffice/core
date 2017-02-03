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

#include "WrappedScaleProperty.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "AxisHelper.hxx"
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart/ChartAxisType.hpp>
#include <chartview/ExplicitScaleValues.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::chart::TimeIncrement;

namespace chart
{
namespace wrapper
{

WrappedScaleProperty::WrappedScaleProperty(tScaleProperty eScaleProperty
                , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_eScaleProperty( eScaleProperty )
{
    switch( m_eScaleProperty )
    {
        case SCALE_PROP_MAX:
            m_aOuterName = "Max";
            break;
        case SCALE_PROP_MIN:
            m_aOuterName = "Min";
            break;
        case SCALE_PROP_ORIGIN:
            m_aOuterName = "Origin";
            break;
        case SCALE_PROP_STEPMAIN:
            m_aOuterName = "StepMain";
            break;
        case SCALE_PROP_STEPHELP:
            m_aOuterName = "StepHelp";
            break;
        case SCALE_PROP_STEPHELP_COUNT:
            m_aOuterName = "StepHelpCount";
            break;
        case SCALE_PROP_AUTO_MAX:
            m_aOuterName = "AutoMax";
            break;
        case SCALE_PROP_AUTO_MIN:
            m_aOuterName = "AutoMin";
            break;
        case SCALE_PROP_AUTO_ORIGIN:
            m_aOuterName = "AutoOrigin";
            break;
        case SCALE_PROP_AUTO_STEPMAIN:
            m_aOuterName = "AutoStepMain";
            break;
        case SCALE_PROP_AUTO_STEPHELP:
            m_aOuterName = "AutoStepHelp";
            break;
        case SCALE_PROP_AXIS_TYPE:
            m_aOuterName = "AxisType";
            break;
        case SCALE_PROP_DATE_INCREMENT:
            m_aOuterName = "TimeIncrement";
            break;
        case SCALE_PROP_EXPLICIT_DATE_INCREMENT:
            m_aOuterName = "ExplicitTimeIncrement";
            break;
        case SCALE_PROP_LOGARITHMIC:
            m_aOuterName = "Logarithmic";
            break;
        case SCALE_PROP_REVERSEDIRECTION:
            m_aOuterName = "ReverseDirection";
            break;
        default:
            OSL_FAIL("unknown scale property");
            break;
    }
}

WrappedScaleProperty::~WrappedScaleProperty()
{
}

void WrappedScaleProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_MAX, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_MIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_ORIGIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_STEPMAIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_STEPHELP, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_STEPHELP_COUNT, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_MAX, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_MIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_ORIGIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_STEPMAIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_STEPHELP, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AXIS_TYPE, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_DATE_INCREMENT, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_EXPLICIT_DATE_INCREMENT, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_LOGARITHMIC, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_REVERSEDIRECTION, spChart2ModelContact ) );
}

void WrappedScaleProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    setPropertyValue( m_eScaleProperty, rOuterValue, xInnerPropertySet );
}

Any WrappedScaleProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    return getPropertyValue( m_eScaleProperty, xInnerPropertySet );
}

void WrappedScaleProperty::setPropertyValue( tScaleProperty eScaleProperty, const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    m_aOuterValue = rOuterValue;

    Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
    OSL_ENSURE(xAxis.is(),"need an XAxis");
    if(!xAxis.is())
        return;

    bool bSetScaleData     = false;

    chart2::ScaleData aScaleData( xAxis->getScaleData() );

    bool bBool = false;
    switch( eScaleProperty )
    {
        case SCALE_PROP_MAX:
        {
            aScaleData.Maximum = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_MIN:
        {
            aScaleData.Minimum = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_STEPMAIN:
        {
            aScaleData.IncrementData.Distance = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_STEPHELP:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() == 0 )
                rSubIncrements.realloc( 1 );

            double fStepHelp = 0;
            if( (rOuterValue >>= fStepHelp) )
            {
                double fStepMain = 0;
                if( AxisHelper::isLogarithmic(aScaleData.Scaling) )
                {
                    sal_Int32 nIntervalCount = static_cast< sal_Int32 >(fStepHelp);
                    rSubIncrements[ 0 ].IntervalCount <<= nIntervalCount;
                }
                else if( (fStepHelp != 0.0) &&
                    (aScaleData.IncrementData.Distance >>= fStepMain) )
                {
                    // approximate interval count
                    sal_Int32 nIntervalCount = static_cast< sal_Int32 >(fStepMain / fStepHelp);//cppcheck-suppress zerodiv
                    rSubIncrements[ 0 ].IntervalCount <<= nIntervalCount;
                }
            }
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_STEPHELP_COUNT:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() == 0 )
                rSubIncrements.realloc( 1 );
            sal_Int32 nIntervalCount=0;
            if( rOuterValue>>=nIntervalCount )
                rSubIncrements[ 0 ].IntervalCount <<= nIntervalCount;
            else
                rSubIncrements[ 0 ].IntervalCount  = Any();
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_MAX:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.Maximum = Any();
            else
                aScaleData.Maximum = getPropertyValue( SCALE_PROP_MAX, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_MIN:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.Minimum = Any();
            else
                aScaleData.Minimum = getPropertyValue( SCALE_PROP_MIN, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_STEPMAIN:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.IncrementData.Distance = Any();
            else
                aScaleData.IncrementData.Distance = getPropertyValue( SCALE_PROP_STEPMAIN, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_STEPHELP:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() == 0 )
                rSubIncrements.realloc( 1 );

            if( (rOuterValue >>= bBool) && bBool )
                rSubIncrements[ 0 ].IntervalCount = Any();
            else
                rSubIncrements[ 0 ].IntervalCount = getPropertyValue( SCALE_PROP_STEPHELP_COUNT, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_ORIGIN:
        {
            aScaleData.Origin = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_ORIGIN:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.Origin = Any();
            else
                aScaleData.Origin = getPropertyValue( SCALE_PROP_ORIGIN, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AXIS_TYPE:
        {
            sal_Int32 nType = 0;
            if( (rOuterValue >>= nType) )
            {
                if( css::chart::ChartAxisType::AUTOMATIC == nType )
                {
                    aScaleData.AutoDateAxis = true;
                    if( aScaleData.AxisType == AxisType::DATE )
                        aScaleData.AxisType = AxisType::CATEGORY;
                }
                else if( css::chart::ChartAxisType::CATEGORY == nType )
                {
                    aScaleData.AutoDateAxis = false;
                    if( aScaleData.AxisType == AxisType::DATE )
                        aScaleData.AxisType = AxisType::CATEGORY;
                }
                else if( css::chart::ChartAxisType::DATE == nType )
                {
                    if( aScaleData.AxisType == AxisType::CATEGORY )
                        aScaleData.AxisType = AxisType::DATE;
                }
                bSetScaleData = true;
            }
            break;
        }
        case SCALE_PROP_DATE_INCREMENT:
        {
            TimeIncrement aTimeIncrement;
            rOuterValue >>= aTimeIncrement;
            aScaleData.TimeIncrement = aTimeIncrement;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_EXPLICIT_DATE_INCREMENT:
            //read only property
            break;
        case SCALE_PROP_LOGARITHMIC:
        {
            if( rOuterValue >>= bBool )
            {
                bool bWasLogarithm = AxisHelper::isLogarithmic( aScaleData.Scaling );

                // safe comparison between sal_Bool and bool
                if( (!bBool) != (!bWasLogarithm) )
                {
                    if( bBool )
                        aScaleData.Scaling = AxisHelper::createLogarithmicScaling( 10.0 );
                    else
                        aScaleData.Scaling = nullptr;
                    bSetScaleData = true;
                }
            }
            break;
        }
        case SCALE_PROP_REVERSEDIRECTION:
        {
            if( rOuterValue >>= bBool )
            {
                bool bWasReverse = ( AxisOrientation_REVERSE == aScaleData.Orientation );
                if( (!bBool) != (!bWasReverse) ) // safe comparison between sal_Bool and bool
                {
                    aScaleData.Orientation = bBool ? AxisOrientation_REVERSE : AxisOrientation_MATHEMATICAL;
                    bSetScaleData = true;
                }
            }
            break;
        }
        default:
        {
            OSL_FAIL("unknown scale property");
            break;
        }
    }

    if( bSetScaleData )
        xAxis->setScaleData( aScaleData );
}

Any WrappedScaleProperty::getPropertyValue( tScaleProperty eScaleProperty, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet( m_aOuterValue );

    Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
    OSL_ENSURE(xAxis.is(),"need an XAxis");
    if(!xAxis.is())
        return aRet;

    chart2::ScaleData aScaleData( xAxis->getScaleData() );

    ExplicitScaleData aExplicitScale;
    ExplicitIncrementData aExplicitIncrement;

    switch( eScaleProperty )
    {
        case SCALE_PROP_MAX:
        {
            aRet = aScaleData.Maximum;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitScale.Maximum;
            }
            break;
        }
        case SCALE_PROP_MIN:
        {
            aRet = aScaleData.Minimum;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitScale.Minimum;
            }
            break;
        }

        case SCALE_PROP_STEPMAIN:
        {
            aRet = aScaleData.IncrementData.Distance;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitIncrement.Distance;
            }
            break;
        }
        case SCALE_PROP_STEPHELP:
        {
            // todo: evaluate PostEquidistant
            bool bNeedToCalculateExplicitValues = true;

            bool bLogarithmic( AxisHelper::isLogarithmic(aScaleData.Scaling) );
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( bLogarithmic )
            {
                if( rSubIncrements.getLength() > 0 )
                {
                    sal_Int32 nIntervalCount = 0;
                    rSubIncrements[ 0 ].IntervalCount >>= nIntervalCount;
                    aRet = uno::Any( double(nIntervalCount) );
                    bNeedToCalculateExplicitValues = false;
                }
            }
            else if( aScaleData.IncrementData.Distance.hasValue() )
            {
                if( rSubIncrements.getLength() > 0 )
                {
                    double fStepMain = 0;
                    sal_Int32 nIntervalCount = 0;
                    if( (aScaleData.IncrementData.Distance >>= fStepMain) &&
                        (rSubIncrements[ 0 ].IntervalCount >>= nIntervalCount) &&
                        nIntervalCount > 0 )
                    {
                        aRet <<= ( fStepMain / static_cast< double >( nIntervalCount ) );
                        bNeedToCalculateExplicitValues = false;
                    }
                }
                else
                {
                    aRet = aScaleData.IncrementData.Distance;
                    bNeedToCalculateExplicitValues = false;
                }
            }

            if( bNeedToCalculateExplicitValues )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );

                if( !aExplicitIncrement.SubIncrements.empty() &&
                     aExplicitIncrement.SubIncrements[ 0 ].IntervalCount > 0 )
                {
                    if( bLogarithmic )
                    {
                        if( rSubIncrements.getLength() > 0 )
                        {
                            sal_Int32 nIntervalCount = aExplicitIncrement.SubIncrements[ 0 ].IntervalCount;
                            aRet = uno::Any( double(nIntervalCount) );
                        }
                    }
                    else
                        aRet <<= ( aExplicitIncrement.Distance /
                                static_cast< double >(
                                    aExplicitIncrement.SubIncrements[ 0 ].IntervalCount ));
                }
                else
                {
                    if( bLogarithmic )
                        aRet <<= 5.0;
                    else
                        aRet <<= aExplicitIncrement.Distance;
                }
            }
            break;
        }
        case SCALE_PROP_STEPHELP_COUNT:
        {
            sal_Int32 nIntervalCount = 0;
            bool bNeedToCalculateExplicitValues = true;
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() > 0 )
            {
                if( (rSubIncrements[ 0 ].IntervalCount >>= nIntervalCount) && (nIntervalCount > 0) )
                    bNeedToCalculateExplicitValues = false;
            }
            if( bNeedToCalculateExplicitValues )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis( xAxis, aExplicitScale, aExplicitIncrement );
                if( !aExplicitIncrement.SubIncrements.empty() )
                    nIntervalCount = aExplicitIncrement.SubIncrements[ 0 ].IntervalCount;
            }
            aRet = uno::Any( nIntervalCount );
            break;
        }
        case SCALE_PROP_AUTO_MAX:
        {
            aRet <<= !aScaleData.Maximum.hasValue();
            break;
        }
        case SCALE_PROP_AUTO_MIN:
        {
            aRet <<= !aScaleData.Minimum.hasValue();
            break;
        }
        case SCALE_PROP_AUTO_STEPMAIN:
        {
            aRet <<= !aScaleData.IncrementData.Distance.hasValue();
            break;
        }
        case SCALE_PROP_AUTO_STEPHELP:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() > 0 )
                aRet <<= !rSubIncrements[ 0 ].IntervalCount.hasValue();
            else
                aRet <<= true;
            break;
        }
        case SCALE_PROP_ORIGIN:
        {
            aRet = aScaleData.Origin;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitScale.Origin;
            }
            break;
        }
        case SCALE_PROP_AUTO_ORIGIN:
        {
            aRet <<= !hasDoubleValue(aScaleData.Origin);
            break;
        }
        case SCALE_PROP_AXIS_TYPE:
        {
            sal_Int32 nType = css::chart::ChartAxisType::AUTOMATIC;
            if( aScaleData.AxisType == AxisType::DATE )
            {
                nType = css::chart::ChartAxisType::DATE;
            }
            else if( aScaleData.AxisType == AxisType::CATEGORY )
            {
                if( !aScaleData.AutoDateAxis )
                    nType = css::chart::ChartAxisType::CATEGORY;
            }
            aRet = uno::Any( nType );
            break;
        }
        case SCALE_PROP_DATE_INCREMENT:
        {
            if( aScaleData.AxisType == AxisType::DATE || aScaleData.AutoDateAxis )
                aRet = uno::Any( aScaleData.TimeIncrement );
            break;
        }
        case SCALE_PROP_EXPLICIT_DATE_INCREMENT:
        {
            if( aScaleData.AxisType == AxisType::DATE || aScaleData.AutoDateAxis )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis( xAxis, aExplicitScale, aExplicitIncrement );
                if( aExplicitScale.AxisType == AxisType::DATE )
                {
                    TimeIncrement aTimeIncrement;
                    aTimeIncrement.MajorTimeInterval = uno::Any( aExplicitIncrement.MajorTimeInterval );
                    aTimeIncrement.MinorTimeInterval = uno::Any( aExplicitIncrement.MinorTimeInterval );
                    aTimeIncrement.TimeResolution = uno::Any( aExplicitScale.TimeResolution );
                    aRet = uno::Any(aTimeIncrement);
                }
            }

            if( aScaleData.AxisType == AxisType::DATE || aScaleData.AutoDateAxis )
                aRet = uno::Any( aScaleData.TimeIncrement );
            break;
        }
        case SCALE_PROP_LOGARITHMIC:
        {
            aRet <<= AxisHelper::isLogarithmic(aScaleData.Scaling);
            break;
        }
        case SCALE_PROP_REVERSEDIRECTION:
        {
            aRet <<= AxisOrientation_REVERSE == aScaleData.Orientation;
            break;
        }
        default:
        {
            OSL_FAIL("unknown scale property");
            break;
        }
    }

    return aRet;
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
