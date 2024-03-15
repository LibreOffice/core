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

#include "WrappedSplineProperties.hxx"
#include "Chart2ModelContact.hxx"
#include <FastPropertyIdRanges.hxx>
#include <ChartType.hxx>
#include <WrappedProperty.hxx>
#include <unonames.hxx>

#include <sal/log.hxx>

#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <utility>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::Property;

namespace chart::wrapper
{

namespace
{

//PROPERTYTYPE is the type of the outer property

template< typename PROPERTYTYPE >
class WrappedSplineProperty : public WrappedProperty
{
public:
    explicit WrappedSplineProperty( const OUString& rOuterName, OUString aInnerName
        , const css::uno::Any& rDefaulValue
        , std::shared_ptr<Chart2ModelContact> spChart2ModelContact )
            : WrappedProperty(rOuterName,OUString())
            , m_spChart2ModelContact(std::move(spChart2ModelContact))
            , m_aOuterValue(rDefaulValue)
            , m_aDefaultValue(rDefaulValue)
            , m_aOwnInnerName(std::move(aInnerName))
    {
    }

    bool detectInnerValue( PROPERTYTYPE& rValue, bool& rHasAmbiguousValue ) const
    {
        rHasAmbiguousValue = false;
        rtl::Reference<Diagram> xDiagram = m_spChart2ModelContact->getDiagram();
        if (!xDiagram)
            return false;
        bool bHasDetectableInnerValue = false;
        std::vector< rtl::Reference< ChartType > > aChartTypes = xDiagram->getChartTypes();
        for( sal_Int32 nN = aChartTypes.size(); nN--; )
        {
            try
            {
                Any aSingleValue = convertInnerToOuterValue( aChartTypes[nN]->getPropertyValue(m_aOwnInnerName) );
                PROPERTYTYPE aCurValue = PROPERTYTYPE();
                aSingleValue >>= aCurValue;
                if( !bHasDetectableInnerValue )
                    rValue = aCurValue;
                else
                {
                    if( rValue != aCurValue )
                    {
                        rHasAmbiguousValue = true;
                        break;
                    }
                    else
                        rValue = aCurValue;
                }
                bHasDetectableInnerValue = true;
            }
            catch( uno::Exception & ex )
            {
                //spline properties are not supported by all charttypes
                //in that cases this exception is ok
                ex.Context.is();//to have debug information without compilation warnings
            }
        }
        return bHasDetectableInnerValue;
    }
    void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const override
    {
        PROPERTYTYPE aNewValue;
        if( ! (rOuterValue >>= aNewValue) )
            throw css::lang::IllegalArgumentException( "spline property requires different type", nullptr, 0 );

        m_aOuterValue = rOuterValue;

        bool bHasAmbiguousValue = false;
        PROPERTYTYPE aOldValue = PROPERTYTYPE();
        if( !detectInnerValue( aOldValue, bHasAmbiguousValue ) )
            return;

        if( !(bHasAmbiguousValue || aNewValue != aOldValue) )
            return;

        std::vector< rtl::Reference< ChartType > > aChartTypes =
            m_spChart2ModelContact->getDiagram()->getChartTypes();
        for( sal_Int32 nN = aChartTypes.size(); nN--; )
        {
            try
            {
                aChartTypes[nN]->setPropertyValue(m_aOwnInnerName,convertOuterToInnerValue(uno::Any(aNewValue)));
            }
            catch( uno::Exception & ex )
            {
                //spline properties are not supported by all charttypes
                //in that cases this exception is ok
                ex.Context.is();//to have debug information without compilation warnings
            }
        }
    }

    css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const override
    {
        bool bHasAmbiguousValue = false;
        PROPERTYTYPE aValue;
        if( detectInnerValue( aValue, bHasAmbiguousValue ) )
        {
            m_aOuterValue <<= aValue;
        }
        return m_aOuterValue;
    }

    css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& /*xInnerPropertyState*/ ) const override
    {
        return m_aDefaultValue;
    }

protected:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable css::uno::Any                   m_aOuterValue;
    css::uno::Any                           m_aDefaultValue;
    // this inner name is not set as inner name at the base class
    const OUString m_aOwnInnerName;
};

class WrappedSplineTypeProperty : public WrappedSplineProperty< sal_Int32 >
{
public:
    explicit WrappedSplineTypeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual css::uno::Any convertInnerToOuterValue( const css::uno::Any& rInnerValue ) const override;
    virtual css::uno::Any convertOuterToInnerValue( const css::uno::Any& rOuterValue ) const override;
};

enum
{
    //spline properties
      PROP_CHART_SPLINE_TYPE = FAST_PROPERTY_ID_START_CHART_SPLINE_PROP
    , PROP_CHART_SPLINE_ORDER
    , PROP_CHART_SPLINE_RESOLUTION
};

}//anonymous namespace

void WrappedSplineProperties::addProperties( std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( CHART_UNONAME_SPLINE_TYPE,
                  PROP_CHART_SPLINE_TYPE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( CHART_UNONAME_SPLINE_ORDER,
                  PROP_CHART_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( CHART_UNONAME_SPLINE_RESOLUTION,
                  PROP_CHART_SPLINE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
}

void WrappedSplineProperties::addWrappedProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    rList.emplace_back( new WrappedSplineTypeProperty( spChart2ModelContact ) );
    rList.emplace_back(
        new WrappedSplineProperty<sal_Int32>(
            CHART_UNONAME_SPLINE_ORDER, CHART_UNONAME_SPLINE_ORDER,
            uno::Any(sal_Int32(3)), spChart2ModelContact));
    rList.emplace_back(
        new WrappedSplineProperty<sal_Int32>(
            CHART_UNONAME_SPLINE_RESOLUTION, CHART_UNONAME_CURVE_RESOLUTION,
            uno::Any(sal_Int32(20)), spChart2ModelContact));
}

WrappedSplineTypeProperty::WrappedSplineTypeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : WrappedSplineProperty<sal_Int32>(CHART_UNONAME_SPLINE_TYPE, CHART_UNONAME_CURVE_STYLE, uno::Any(sal_Int32(0)), spChart2ModelContact )
{
}

Any WrappedSplineTypeProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    chart2::CurveStyle aInnerValue = chart2::CurveStyle_LINES;
    rInnerValue >>= aInnerValue;

    sal_Int32 nOuterValue;
    switch (aInnerValue)
    {
        case chart2::CurveStyle_CUBIC_SPLINES:
            nOuterValue = 1;
            break;
        case chart2::CurveStyle_B_SPLINES:
            nOuterValue = 2;
            break;
        case chart2::CurveStyle_STEP_START:
            nOuterValue = 3;
            break;
        case chart2::CurveStyle_STEP_END:
            nOuterValue = 4;
            break;
        case chart2::CurveStyle_STEP_CENTER_X:
            nOuterValue = 5;
            break;
        case chart2::CurveStyle_STEP_CENTER_Y:
            nOuterValue = 6;
            break;
        default:
            nOuterValue = 0;
    }

    return uno::Any(nOuterValue);
}
Any WrappedSplineTypeProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    sal_Int32 nOuterValue=0;
    rOuterValue >>= nOuterValue;

    chart2::CurveStyle aInnerValue;

    switch (nOuterValue)
    {
        case 1:
            aInnerValue = chart2::CurveStyle_CUBIC_SPLINES;
            break;
        case 2:
            aInnerValue = chart2::CurveStyle_B_SPLINES;
            break;
        case 3:
            aInnerValue = chart2::CurveStyle_STEP_START;
            break;
        case 4:
            aInnerValue = chart2::CurveStyle_STEP_END;
            break;
        case 5:
            aInnerValue = chart2::CurveStyle_STEP_CENTER_X;
            break;
        case 6:
            aInnerValue = chart2::CurveStyle_STEP_CENTER_Y;
            break;
        default:
            SAL_WARN_IF(nOuterValue != 0, "chart2", "Unknown line style");
            aInnerValue = chart2::CurveStyle_LINES;
    }

    return uno::Any(aInnerValue);
}

} //namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
