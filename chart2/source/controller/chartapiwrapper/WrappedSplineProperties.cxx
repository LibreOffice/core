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
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{

//-----------------------------------------------------------------------------
//PROPERTYTYPE is the type of the outer property

template< typename PROPERTYTYPE >
class WrappedSplineProperty : public WrappedProperty
{
public:
    explicit WrappedSplineProperty( const ::rtl::OUString& rOuterName, const ::rtl::OUString& rInnerName
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
            throw ::com::sun::star::lang::IllegalArgumentException( C2U("spline property requires different type"), 0, 0 );

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

//-----------------------------------------------------------------------------
void WrappedSplineProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "SplineType" ),
                  PROP_CHART_SPLINE_TYPE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "SplineOrder" ),
                  PROP_CHART_SPLINE_ORDER,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "SplineResolution" ),
                  PROP_CHART_SPLINE_RESOLUTION,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

//-----------------------------------------------------------------------------
void WrappedSplineProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedSplineTypeProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedSplineProperty<sal_Int32>( C2U("SplineOrder"),      C2U("SplineOrder"), uno::makeAny(sal_Int32(3)), spChart2ModelContact ) );
    rList.push_back( new WrappedSplineProperty<sal_Int32>( C2U("SplineResolution"), C2U("CurveResolution"), uno::makeAny(sal_Int32(20)), spChart2ModelContact ) );
}

//-----------------------------------------------------------------------------

WrappedSplineTypeProperty::WrappedSplineTypeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedSplineProperty<sal_Int32>( C2U("SplineType"), C2U("CurveStyle"), uno::makeAny(sal_Int32(0)), spChart2ModelContact )
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
    if( chart2::CurveStyle_CUBIC_SPLINES == aInnerValue )
        nOuterValue = 1;
    else if( chart2::CurveStyle_B_SPLINES == aInnerValue )
        nOuterValue = 2;
    else
        nOuterValue = 0;

    return uno::makeAny(nOuterValue);
}
Any WrappedSplineTypeProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    sal_Int32 nOuterValue=0;
    rOuterValue >>= nOuterValue;

    chart2::CurveStyle aInnerValue;

    if(1==nOuterValue)
        aInnerValue = chart2::CurveStyle_CUBIC_SPLINES;
    else if(2==nOuterValue)
        aInnerValue = chart2::CurveStyle_B_SPLINES;
    else
        aInnerValue = chart2::CurveStyle_LINES;

    return uno::makeAny(aInnerValue);
}

} //namespace wrapper
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
