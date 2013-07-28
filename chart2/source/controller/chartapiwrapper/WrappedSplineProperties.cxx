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
#include "macros.hxx"
#include "FastPropertyIdRanges.hxx"
#include "DiagramHelper.hxx"
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

//PROPERTYTYPE is the type of the outer property

template< typename PROPERTYTYPE >
class WrappedSplineProperty : public WrappedProperty
{
public:
    explicit WrappedSplineProperty( const OUString& rOuterName, const OUString& rInnerName
        , const ::com::sun::star::uno::Any& rDefaulValue
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(rOuterName,OUString())
            , m_spChart2ModelContact(spChart2ModelContact)
            , m_aOuterValue(rDefaulValue)
            , m_aDefaultValue(rDefaulValue)
            , m_aOwnInnerName(rInnerName)
    {
    }
    virtual ~WrappedSplineProperty() {};

    bool detectInnerValue( PROPERTYTYPE& rValue, bool& rHasAmbiguousValue ) const
    {
        bool bHasDetectableInnerValue = false;
        rHasAmbiguousValue = false;
        Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > > aChartTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
        for( sal_Int32 nN = aChartTypes.getLength(); nN--; )
        {
            try
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xChartTypePropertySet( aChartTypes[nN], ::com::sun::star::uno::UNO_QUERY );

                Any aSingleValue = this->convertInnerToOuterValue( xChartTypePropertySet->getPropertyValue(m_aOwnInnerName) );
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
    void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        PROPERTYTYPE aNewValue;
        if( ! (rOuterValue >>= aNewValue) )
            throw ::com::sun::star::lang::IllegalArgumentException( "spline property requires different type", 0, 0 );

        m_aOuterValue = rOuterValue;

        bool bHasAmbiguousValue = false;
        PROPERTYTYPE aOldValue = PROPERTYTYPE();
        if( detectInnerValue( aOldValue, bHasAmbiguousValue ) )
        {
            if( bHasAmbiguousValue || aNewValue != aOldValue )
            {
                Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > > aChartTypes(
                    ::chart::DiagramHelper::getChartTypesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
                for( sal_Int32 nN = aChartTypes.getLength(); nN--; )
                {
                    try
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xChartTypePropertySet( aChartTypes[nN], ::com::sun::star::uno::UNO_QUERY );
                        if( xChartTypePropertySet.is() )
                        {
                            xChartTypePropertySet->setPropertyValue(m_aOwnInnerName,this->convertOuterToInnerValue(uno::makeAny(aNewValue)));
                        }
                    }
                    catch( uno::Exception & ex )
                    {
                        //spline properties are not supported by all charttypes
                        //in that cases this exception is ok
                        ex.Context.is();//to have debug information without compilation warnings
                    }
                }
            }
        }
    }

    ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        bool bHasAmbiguousValue = false;
        PROPERTYTYPE aValue;
        if( detectInnerValue( aValue, bHasAmbiguousValue ) )
        {
            m_aOuterValue <<= aValue;
        }
        return m_aOuterValue;
    }

    ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
    {
        return m_aDefaultValue;
    }

protected:
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable ::com::sun::star::uno::Any     m_aOuterValue;
    ::com::sun::star::uno::Any             m_aDefaultValue;
    // this inner name is not set as inner name at the base class
    const OUString m_aOwnInnerName;
};

class WrappedSplineTypeProperty : public WrappedSplineProperty< sal_Int32 >
{
public:
    explicit WrappedSplineTypeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedSplineTypeProperty();

    virtual ::com::sun::star::uno::Any convertInnerToOuterValue( const ::com::sun::star::uno::Any& rInnerValue ) const;
    virtual ::com::sun::star::uno::Any convertOuterToInnerValue( const ::com::sun::star::uno::Any& rOuterValue ) const;
};

namespace
{
enum
{
    //spline properties
      PROP_CHART_SPLINE_TYPE = FAST_PROPERTY_ID_START_CHART_SPLINE_PROP
    , PROP_CHART_SPLINE_ORDER
    , PROP_CHART_SPLINE_RESOLUTION
};

}//anonymous namespace

void WrappedSplineProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "SplineType",
                  PROP_CHART_SPLINE_TYPE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "SplineOrder",
                  PROP_CHART_SPLINE_ORDER,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "SplineResolution",
                  PROP_CHART_SPLINE_RESOLUTION,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void WrappedSplineProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedSplineTypeProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedSplineProperty<sal_Int32>( "SplineOrder", "SplineOrder", uno::makeAny(sal_Int32(3)), spChart2ModelContact ) );
    rList.push_back( new WrappedSplineProperty<sal_Int32>( "SplineResolution", "CurveResolution", uno::makeAny(sal_Int32(20)), spChart2ModelContact ) );
}

WrappedSplineTypeProperty::WrappedSplineTypeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedSplineProperty<sal_Int32>( "SplineType", "CurveStyle", uno::makeAny(sal_Int32(0)), spChart2ModelContact )
{
}
WrappedSplineTypeProperty::~WrappedSplineTypeProperty()
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

    return uno::makeAny(nOuterValue);
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
            SAL_WARN_IF(chart2::CurveStyle_LINES != 0, "chart2", "Unknown line style");
            aInnerValue = chart2::CurveStyle_LINES;
    }

    return uno::makeAny(aInnerValue);
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
