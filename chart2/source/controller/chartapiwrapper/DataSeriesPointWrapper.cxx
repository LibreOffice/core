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
#include "DataSeriesPointWrapper.hxx"
#include "macros.hxx"
#include "RegressionCurveHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "ChartTypeHelper.hxx"
#include "DiagramHelper.hxx"
#include "ContainerHelper.hxx"
#include "ChartModelHelper.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "WrappedCharacterHeightProperty.hxx"
#include "WrappedProperty.hxx"
#include "WrappedIgnoreProperty.hxx"
#include "WrappedStatisticProperties.hxx"
#include "WrappedSymbolProperties.hxx"
#include "WrappedDataCaptionProperties.hxx"
#include "WrappedSeriesAreaOrLineProperty.hxx"
#include "WrappedScaleTextProperties.hxx"
#include "WrappedNumberFormatProperty.hxx"
#include "WrappedTextRotationProperty.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
// header for define DBG_ASSERT
#include <tools/debug.hxx>

#include <algorithm>
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

using namespace ::com::sun::star;
using namespace ::chart::wrapper;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.DataSeries" ));

enum
{
    //data point  properties
    PROP_SERIES_DATAPOINT_SOLIDTYPE,
    PROP_SERIES_DATAPOINT_SEGMENT_OFFSET,
    PROP_SERIES_DATAPOINT_PERCENT_DIAGONAL,
    PROP_SERIES_DATAPOINT_LABEL_SEPARATOR,
    PROP_SERIES_NUMBERFORMAT,
    PROP_SERIES_LINK_NUMBERFORMAT_TO_SOURCE,
    PROP_SERIES_PERCENTAGE_NUMBERFORMAT,
    PROP_SERIES_DATAPOINT_LABEL_PLACEMENT,
    //other series properties
    PROP_SERIES_ATTACHED_AXIS,
    PROP_SERIES_DATAPOINT_TEXT_ROTATION
};

void lcl_AddPropertiesToVector_PointProperties(
    ::std::vector< Property > & rOutProperties )
{
    //service chart::Chart3DBarProperties
    rOutProperties.push_back(
        Property( C2U( "SolidType" ),
                  PROP_SERIES_DATAPOINT_SOLIDTYPE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "SegmentOffset" ),
                  PROP_SERIES_DATAPOINT_SEGMENT_OFFSET,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "D3DPercentDiagonal" ),
                  PROP_SERIES_DATAPOINT_PERCENT_DIAGONAL,
                  ::getCppuType( reinterpret_cast< sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "LabelSeparator" ),
                  PROP_SERIES_DATAPOINT_LABEL_SEPARATOR,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_SERIES_NUMBERFORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "PercentageNumberFormat" ),
                  PROP_SERIES_PERCENTAGE_NUMBERFORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "LabelPlacement" ),
                  PROP_SERIES_DATAPOINT_LABEL_PLACEMENT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_SERIES_DATAPOINT_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddPropertiesToVector_SeriesOnly(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Axis" ),
                  PROP_SERIES_ATTACHED_AXIS,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LinkNumberFormatToSource" ),
                  PROP_SERIES_LINK_NUMBERFORMAT_TO_SOURCE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

uno::Sequence< Property > lcl_GetPropertySequence( DataSeriesPointWrapper::eType _eType )
{
    ::std::vector< ::com::sun::star::beans::Property > aProperties;

    lcl_AddPropertiesToVector_PointProperties( aProperties );
    if( _eType == DataSeriesPointWrapper::DATA_SERIES )
    {
        lcl_AddPropertiesToVector_SeriesOnly( aProperties );
        WrappedStatisticProperties::addProperties( aProperties );
    }
    WrappedSymbolProperties::addProperties( aProperties ); //for series and  points
    WrappedDataCaptionProperties::addProperties( aProperties ); //for series and  points

    ::chart::FillProperties::AddPropertiesToVector( aProperties );
    ::chart::LineProperties::AddPropertiesToVector( aProperties );
    ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
    ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
    ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

    ::std::sort( aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess() );

    return ::chart::ContainerHelper::ContainerToSequence( aProperties );
}

struct StaticSeriesWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence( DataSeriesPointWrapper::DATA_SERIES ) );
        return &aPropSeq;
    }
};

struct StaticSeriesWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticSeriesWrapperPropertyArray_Initializer >
{
};

struct StaticPointWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence( DataSeriesPointWrapper::DATA_POINT ) );
        return &aPropSeq;
    }
};

struct StaticPointWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticPointWrapperPropertyArray_Initializer >
{
};

//----------------------------------------------------------------------------------------------------------------------


//PROP_SERIES_ATTACHED_AXIS
class WrappedAttachedAxisProperty : public ::chart::WrappedProperty
{
public:
    WrappedAttachedAxisProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedAttachedAxisProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected:
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

WrappedAttachedAxisProperty::WrappedAttachedAxisProperty(
                ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
                : WrappedProperty(C2U("Axis"),rtl::OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

WrappedAttachedAxisProperty::~WrappedAttachedAxisProperty()
{
}

Any WrappedAttachedAxisProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= ::com::sun::star::chart::ChartAxisAssign::PRIMARY_Y;
    return aRet;
}

Any WrappedAttachedAxisProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;

    uno::Reference< chart2::XDataSeries > xDataSeries( xInnerPropertySet, uno::UNO_QUERY );
    bool bAttachedToMainAxis = ::chart::DiagramHelper::isSeriesAttachedToMainAxis( xDataSeries );
    if( bAttachedToMainAxis )
        aRet <<= ::com::sun::star::chart::ChartAxisAssign::PRIMARY_Y;
    else
        aRet <<= ::com::sun::star::chart::ChartAxisAssign::SECONDARY_Y;
    return aRet;
}

void WrappedAttachedAxisProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< chart2::XDataSeries > xDataSeries( xInnerPropertySet, uno::UNO_QUERY );

    sal_Int32 nChartAxisAssign = ::com::sun::star::chart::ChartAxisAssign::PRIMARY_Y;
    if( ! (rOuterValue >>= nChartAxisAssign) )
        throw lang::IllegalArgumentException( C2U("Property Axis requires value of type sal_Int32"), 0, 0 );

    bool bNewAttachedToMainAxis = nChartAxisAssign == ::com::sun::star::chart::ChartAxisAssign::PRIMARY_Y;
    bool bOldAttachedToMainAxis = ::chart::DiagramHelper::isSeriesAttachedToMainAxis( xDataSeries );

    if( bNewAttachedToMainAxis != bOldAttachedToMainAxis)
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        if( xDiagram.is() )
            ::chart::DiagramHelper::attachSeriesToAxis( bNewAttachedToMainAxis, xDataSeries, xDiagram, m_spChart2ModelContact->m_xContext, false );
    }
}

//----------------------------------------------------------------------------------------------------------------------

class WrappedSegmentOffsetProperty : public ::chart::WrappedProperty
{
public:
    WrappedSegmentOffsetProperty();
    virtual ~WrappedSegmentOffsetProperty();

protected:
    virtual Any convertInnerToOuterValue( const Any& rInnerValue ) const;
    virtual Any convertOuterToInnerValue( const Any& rOuterValue ) const;
};

WrappedSegmentOffsetProperty::WrappedSegmentOffsetProperty() :
        WrappedProperty( C2U( "SegmentOffset" ), C2U( "Offset" ) )
{}

WrappedSegmentOffsetProperty::~WrappedSegmentOffsetProperty()
{}

Any WrappedSegmentOffsetProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    // convert new double offset to former integer segment-offset
    double fOffset = 0;
    Any aResult( rInnerValue );

    if( rInnerValue >>= fOffset )
        aResult <<= static_cast< sal_Int32 >( ::rtl::math::round( fOffset * 100.0 ));

    return aResult;
}

Any WrappedSegmentOffsetProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    // convert former integer segment-offset to new double offset
    sal_Int32 nOffset = 0;
    Any aResult( rOuterValue );

    if( rOuterValue >>= nOffset )
        aResult <<= (static_cast< double >( nOffset ) / 100.0);

    return aResult;
}

//----------------------------------------------------------------------------------------------------------------------

class WrappedLineColorProperty : public WrappedSeriesAreaOrLineProperty
{
public:
    WrappedLineColorProperty( DataSeriesPointWrapper* pDataSeriesPointWrapper );
    virtual ~WrappedLineColorProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual void setPropertyToDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected:
    DataSeriesPointWrapper* m_pDataSeriesPointWrapper;
    mutable Any             m_aDefaultValue;
    mutable Any             m_aOuterValue;
};

WrappedLineColorProperty::WrappedLineColorProperty(
                DataSeriesPointWrapper* pDataSeriesPointWrapper )
                : WrappedSeriesAreaOrLineProperty(C2U( "LineColor" ), C2U( "BorderColor" ), C2U( "Color" ), pDataSeriesPointWrapper )
                , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
                , m_aDefaultValue(uno::makeAny(sal_Int32( 0x0099ccff )))  // blue 8
                , m_aOuterValue(m_aDefaultValue)
{
}

WrappedLineColorProperty::~WrappedLineColorProperty()
{
}

void WrappedLineColorProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
        m_aOuterValue = rOuterValue;
    else
        WrappedSeriesAreaOrLineProperty::setPropertyValue( rOuterValue, xInnerPropertySet );
}

void WrappedLineColorProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
        m_aOuterValue = m_aDefaultValue;
    else
        WrappedSeriesAreaOrLineProperty::setPropertyToDefault( xInnerPropertyState );
}

Any WrappedLineColorProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( m_pDataSeriesPointWrapper && !m_pDataSeriesPointWrapper->isSupportingAreaProperties() )
        return m_aDefaultValue;
    else
        return WrappedSeriesAreaOrLineProperty::getPropertyDefault( xInnerPropertyState );
}

//----------------------------------------------------------------------------------------------------------------------

class WrappedLineStyleProperty : public WrappedSeriesAreaOrLineProperty
{
public:
    WrappedLineStyleProperty( DataSeriesPointWrapper* pDataSeriesPointWrapper );
    virtual ~WrappedLineStyleProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual void setPropertyToDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

protected:
    DataSeriesPointWrapper* m_pDataSeriesPointWrapper;
    mutable Any             m_aDefaultValue;
    mutable Any             m_aOuterValue;
};

WrappedLineStyleProperty::WrappedLineStyleProperty(
                DataSeriesPointWrapper* pDataSeriesPointWrapper )
                : WrappedSeriesAreaOrLineProperty(C2U( "LineStyle" ), C2U( "BorderStyle" ), C2U( "LineStyle" ), pDataSeriesPointWrapper )
                , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
                , m_aDefaultValue(uno::makeAny(drawing::LineStyle_SOLID))
                , m_aOuterValue(m_aDefaultValue)
{
}

WrappedLineStyleProperty::~WrappedLineStyleProperty()
{
}

void WrappedLineStyleProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aNewValue(rOuterValue);
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
    {
        m_aOuterValue = rOuterValue;
        aNewValue = uno::makeAny(drawing::LineStyle_NONE);
    }
    WrappedSeriesAreaOrLineProperty::setPropertyValue( aNewValue, xInnerPropertySet );
}

void WrappedLineStyleProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
        m_aOuterValue = m_aDefaultValue;
    else
        WrappedSeriesAreaOrLineProperty::setPropertyToDefault( xInnerPropertyState );
}

} // anonymous namespace


// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

DataSeriesPointWrapper::DataSeriesPointWrapper(
            ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : m_spChart2ModelContact( spChart2ModelContact )
        , m_aEventListenerContainer( m_aMutex )
        , m_eType( DATA_SERIES )
        , m_nSeriesIndexInNewAPI( -1 )
        , m_nPointIndex( -1 )
        , m_bLinesAllowed(sal_True)
        , m_xDataSeries(0)
{
    //need initialize call afterwards
}

void SAL_CALL DataSeriesPointWrapper::initialize( const uno::Sequence< uno::Any >& aArguments )
                throw ( uno::Exception, uno::RuntimeException)
{
    DBG_ASSERT(aArguments.getLength() >= 1,"need at least 1 argument to initialize the DataSeriesPointWrapper: series reference + optional datapoint index");

    m_nSeriesIndexInNewAPI = -1;//ignored in this case
    m_nPointIndex = -1;
    if( aArguments.getLength() >= 1 )
    {
        aArguments[0] >>= m_xDataSeries;
        if( aArguments.getLength() >= 2 )
            aArguments[1] >>= m_nPointIndex;
    }

    if( !m_xDataSeries.is() )
        throw uno::Exception(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    //todo: check upper border of point index

    if( m_nPointIndex >= 0 )
        m_eType = DATA_POINT;
    else
        m_eType = DATA_SERIES;
}

DataSeriesPointWrapper::DataSeriesPointWrapper( eType _eType
            , sal_Int32 nSeriesIndexInNewAPI
            , sal_Int32 nPointIndex //ignored for series
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : m_spChart2ModelContact( spChart2ModelContact )
        , m_aEventListenerContainer( m_aMutex )
        , m_eType( _eType )
        , m_nSeriesIndexInNewAPI( nSeriesIndexInNewAPI )
        , m_nPointIndex( (_eType == DATA_POINT) ? nPointIndex : -1 )
        , m_xDataSeries(0)
{
}

DataSeriesPointWrapper::~DataSeriesPointWrapper()
{
}

// ____ XComponent ____
void SAL_CALL DataSeriesPointWrapper::dispose()
    throw (uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    m_xDataSeries.clear();
    clearWrappedPropertySet();
}

void SAL_CALL DataSeriesPointWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL DataSeriesPointWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ____ XEventListener ____
void SAL_CALL DataSeriesPointWrapper::disposing( const lang::EventObject& /*Source*/ )
    throw (uno::RuntimeException)
{
}

// ================================================================================

bool DataSeriesPointWrapper::isSupportingAreaProperties()
{
    Reference< chart2::XDataSeries > xSeries( this->getDataSeries() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ) );
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );

    return ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount );
}

bool DataSeriesPointWrapper::isLinesForbidden()
{
    return !m_bLinesAllowed;
}

Reference< chart2::XDataSeries > DataSeriesPointWrapper::getDataSeries()
{
    Reference< chart2::XDataSeries > xSeries( m_xDataSeries );
    if( !xSeries.is() )
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
            ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );

        if( m_nSeriesIndexInNewAPI >= 0 && m_nSeriesIndexInNewAPI < static_cast<sal_Int32>(aSeriesList.size()) )
            xSeries = aSeriesList[m_nSeriesIndexInNewAPI];
    }

    return xSeries;
}

Reference< beans::XPropertySet > DataSeriesPointWrapper::getDataPointProperties()
{
    Reference< beans::XPropertySet > xPointProp;

    Reference< chart2::XDataSeries > xSeries( this->getDataSeries() );

    // may throw an IllegalArgumentException
    if( xSeries.is() )
        xPointProp = xSeries->getDataPointByIndex( m_nPointIndex );

    return xPointProp;
}

// ================================================================================

//ReferenceSizePropertyProvider
void DataSeriesPointWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( C2U("ReferencePageSize") ).hasValue() )
            xProp->setPropertyValue( C2U("ReferencePageSize"), uno::makeAny(
                m_spChart2ModelContact->GetPageSize() ));
    }
}
Any DataSeriesPointWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( C2U("ReferencePageSize") );
    return aRet;
}
awt::Size DataSeriesPointWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

// ================================================================================

// WrappedPropertySet

//XPropertyState
beans::PropertyState SAL_CALL DataSeriesPointWrapper::getPropertyState( const ::rtl::OUString& rPropertyName )
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );

    if( m_eType == DATA_SERIES )
        aState = WrappedPropertySet::getPropertyState( rPropertyName );
    else
    {
        if( rPropertyName.equals(C2U("FillColor")) )
        {
            Reference< beans::XPropertySet > xSeriesProp( getDataSeries(), uno::UNO_QUERY );
            sal_Bool bVaryColorsByPoint = sal_False;
            if( xSeriesProp.is() && (xSeriesProp->getPropertyValue( C2U("VaryColorsByPoint") ) >>= bVaryColorsByPoint)
                && bVaryColorsByPoint )
                return beans::PropertyState_DIRECT_VALUE;
        }
        else if( rPropertyName.equals(C2U("Lines"))
            ||  rPropertyName.equals(C2U("SymbolType"))
            ||  rPropertyName.equals(C2U("SymbolSize")) )
            return WrappedPropertySet::getPropertyState( rPropertyName );

        uno::Any aDefault( getPropertyDefault( rPropertyName ) );
        uno::Any aValue( getPropertyValue( rPropertyName ) );
        if( aDefault==aValue )
            aState = beans::PropertyState_DEFAULT_VALUE;
    }
    return aState;
}

void SAL_CALL DataSeriesPointWrapper::setPropertyToDefault( const ::rtl::OUString& rPropertyName )
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    if( m_eType == DATA_SERIES )
        WrappedPropertySet::setPropertyToDefault( rPropertyName );
    else
    {
        //for data points the default is given by the series
        setPropertyValue( rPropertyName, getPropertyDefault( rPropertyName ) );
    }
}
Any SAL_CALL DataSeriesPointWrapper::getPropertyDefault( const ::rtl::OUString& rPropertyName )
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    try
    {
        sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
        if( nHandle > 0 )
        {
            //always take the series current value as default for points
            Reference< beans::XPropertySet > xInnerPropertySet( getDataSeries(), uno::UNO_QUERY );
            if( xInnerPropertySet.is() )
            {
                const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
                if( pWrappedProperty )
                    aRet = pWrappedProperty->getPropertyValue(xInnerPropertySet);
                else
                    aRet = xInnerPropertySet->getPropertyValue( rPropertyName );
            }
        }
    }
    catch( beans::UnknownPropertyException& ex )
    {
        (void)(ex);
        aRet = WrappedPropertySet::getPropertyDefault( rPropertyName );
    }
    return aRet;
}

Reference< beans::XPropertySet > DataSeriesPointWrapper::getInnerPropertySet()
{
    if( m_eType == DATA_SERIES )
        return Reference< beans::XPropertySet >( getDataSeries(), uno::UNO_QUERY );
    return getDataPointProperties();
}

const Sequence< beans::Property >& DataSeriesPointWrapper::getPropertySequence()
{
    if( m_eType == DATA_SERIES )
        return *StaticSeriesWrapperPropertyArray::get();
    else
        return *StaticPointWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > DataSeriesPointWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );

    if( m_eType == DATA_SERIES )
    {
        WrappedStatisticProperties::addWrappedPropertiesForSeries( aWrappedProperties, m_spChart2ModelContact );
        aWrappedProperties.push_back( new WrappedAttachedAxisProperty( m_spChart2ModelContact ) );

        WrappedNumberFormatProperty* pWrappedNumberFormatProperty = new WrappedNumberFormatProperty( m_spChart2ModelContact );
        aWrappedProperties.push_back( pWrappedNumberFormatProperty );
        aWrappedProperties.push_back( new WrappedLinkNumberFormatProperty(pWrappedNumberFormatProperty) );
    }

    WrappedSymbolProperties::addWrappedPropertiesForSeries( aWrappedProperties, m_spChart2ModelContact );
    WrappedDataCaptionProperties::addWrappedPropertiesForSeries( aWrappedProperties, m_spChart2ModelContact );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    //add unnamed line properties (different inner names here)

    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillColor" ), C2U( "Color" ) ) );
    aWrappedProperties.push_back( new WrappedLineStyleProperty( this ) );
    aWrappedProperties.push_back( new WrappedLineColorProperty( this ) );
    aWrappedProperties.push_back( new WrappedSeriesAreaOrLineProperty( C2U( "LineDashName" ), C2U( "BorderDashName" ), C2U( "LineDashName" ), this ) );
    aWrappedProperties.push_back( new WrappedSeriesAreaOrLineProperty( C2U( "LineTransparence" ), C2U( "BorderTransparency" ), C2U( "Transparency" ), this ) );
    aWrappedProperties.push_back( new WrappedSeriesAreaOrLineProperty( C2U( "LineWidth" ), C2U( "BorderWidth" ), C2U( "LineWidth" ), this ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillStyle" ), C2U( "FillStyle" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillTransparence" ), C2U( "Transparency" ) ) );

    aWrappedProperties.push_back( new WrappedIgnoreProperty( C2U( "LineJoint" ), uno::makeAny( drawing::LineJoint_ROUND ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillTransparenceGradientName" ), C2U( "TransparencyGradientName" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillGradientName" ), C2U( "GradientName" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillGradientStepCount" ), C2U( "GradientStepCount" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillHatchName" ), C2U( "HatchName" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapName" ), C2U( "FillBitmapName" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBackground" ), C2U( "FillBackground" ) ) );

    //bitmap properties
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapMode" ), C2U( "FillBitmapMode" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapSizeX" ), C2U( "FillBitmapSizeX" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapSizeY" ), C2U( "FillBitmapSizeY" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapLogicalSize" ), C2U( "FillBitmapLogicalSize" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapOffsetX" ), C2U( "FillBitmapOffsetX" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapOffsetY" ), C2U( "FillBitmapOffsetY" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapRectanglePoint" ), C2U( "FillBitmapRectanglePoint" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapPositionOffsetX" ), C2U( "FillBitmapPositionOffsetX" ) ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "FillBitmapPositionOffsetY" ), C2U( "FillBitmapPositionOffsetY" ) ) );

    aWrappedProperties.push_back( new WrappedProperty( C2U( "SolidType" ), C2U( "Geometry3D" ) ) );
    aWrappedProperties.push_back( new WrappedSegmentOffsetProperty() );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "D3DPercentDiagonal" ), C2U( "PercentDiagonal" ) ) );

    aWrappedProperties.push_back( new WrappedTextRotationProperty() );

    return aWrappedProperties;
}

void SAL_CALL DataSeriesPointWrapper::setPropertyValue( const ::rtl::OUString& rPropertyName, const Any& rValue )
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(rPropertyName.equals(C2U("Lines")))
    {
        if( ! (rValue >>= m_bLinesAllowed) )
            throw lang::IllegalArgumentException( C2U("Property Lines requires value of type sal_Bool"), 0, 0 );
    }

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    static const sal_Int32 nErrorCategoryHandle = getInfoHelper().getHandleByName( C2U("ErrorCategory") );
    if( nErrorCategoryHandle == nHandle )
    {
        ::com::sun::star::chart::ChartErrorCategory aNewValue = ::com::sun::star::chart::ChartErrorCategory_NONE;
        rValue >>= aNewValue;
        Any aLow, aHigh;
        bool bSetHighAndLowValues = false;
        switch(aNewValue)
        {
            case ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE:
                aHigh = this->getPropertyValue(C2U("ConstantErrorHigh"));
                aLow = this->getPropertyValue(C2U("ConstantErrorLow"));
                bSetHighAndLowValues = true;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_PERCENT:
                aHigh = aLow = this->getPropertyValue(C2U("PercentageError"));
                bSetHighAndLowValues = true;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN:
                aHigh = aLow = this->getPropertyValue(C2U("ErrorMargin"));
                bSetHighAndLowValues = true;
                break;
            default:
                break;
        }

        WrappedPropertySet::setPropertyValue( rPropertyName, rValue );

        if(bSetHighAndLowValues)
        {
            switch(aNewValue)
            {
                case ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE:
                    this->setPropertyValue(C2U("ConstantErrorHigh"),aHigh);
                    this->setPropertyValue(C2U("ConstantErrorLow"),aLow);
                    break;
                case ::com::sun::star::chart::ChartErrorCategory_PERCENT:
                    this->setPropertyValue(C2U("PercentageError"),aHigh);
                    break;
                case ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN:
                    this->setPropertyValue(C2U("ErrorMargin"),aHigh);
                    break;
                default:
                    break;
            }
        }
    }
    else
        WrappedPropertySet::setPropertyValue( rPropertyName, rValue );
}

Any SAL_CALL DataSeriesPointWrapper::getPropertyValue( const rtl::OUString& rPropertyName )
                throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( m_eType == DATA_POINT )
    {
        if( rPropertyName.equals(C2U("FillColor")) )
        {
            Reference< beans::XPropertySet > xSeriesProp( getDataSeries(), uno::UNO_QUERY );
            sal_Bool bVaryColorsByPoint = sal_False;
            if( xSeriesProp.is() && (xSeriesProp->getPropertyValue( C2U("VaryColorsByPoint") ) >>= bVaryColorsByPoint)
                && bVaryColorsByPoint )
            {
                uno::Reference< beans::XPropertyState > xPointState( DataSeriesPointWrapper::getDataPointProperties(), uno::UNO_QUERY );
                if( xPointState.is() && xPointState->getPropertyState( C2U("Color")) == beans::PropertyState_DEFAULT_VALUE )
                {
                    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
                    if( xDiagram.is() )
                    {
                        Reference< chart2::XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme() );
                        if( xColorScheme.is() )
                            return uno::makeAny( xColorScheme->getColorByIndex( m_nPointIndex ) );
                    }
                }
            }
        }
    }
    return WrappedPropertySet::getPropertyValue( rPropertyName );
}

// ================================================================================

uno::Sequence< ::rtl::OUString > DataSeriesPointWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 7 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartDataRowProperties" );
    aServices[ 1 ] = C2U( "com.sun.star.chart.ChartDataPointProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 4 ] = C2U( "com.sun.star.drawing.FillProperties" );
    aServices[ 5 ] = C2U( "com.sun.star.drawing.LineProperties" );
    aServices[ 6 ] = C2U( "com.sun.star.style.CharacterProperties" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DataSeriesPointWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
