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

#include "DataSeriesPointWrapper.hxx"
#include <RegressionCurveHelper.hxx>
#include "Chart2ModelContact.hxx"
#include <ChartTypeHelper.hxx>
#include <DiagramHelper.hxx>
#include <ChartModelHelper.hxx>
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <CharacterProperties.hxx>
#include <UserDefinedProperties.hxx>
#include "WrappedCharacterHeightProperty.hxx"
#include <WrappedProperty.hxx>
#include <WrappedIgnoreProperty.hxx>
#include "WrappedStatisticProperties.hxx"
#include "WrappedSymbolProperties.hxx"
#include "WrappedDataCaptionProperties.hxx"
#include "WrappedSeriesAreaOrLineProperty.hxx"
#include "WrappedScaleTextProperties.hxx"
#include "WrappedNumberFormatProperty.hxx"
#include "WrappedTextRotationProperty.hxx"
#include <unonames.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#include <algorithm>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>

using namespace ::com::sun::star;
using namespace ::chart::wrapper;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;

namespace
{

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
    PROP_SERIES_DATAPOINT_TEXT_WORD_WRAP,
    PROP_SERIES_DATAPOINT_LABEL_PLACEMENT,
    //other series properties
    PROP_SERIES_ATTACHED_AXIS,
    PROP_SERIES_DATAPOINT_TEXT_ROTATION,
    PROP_SERIES_DATAPOINT_LABEL_BORDER_STYLE,
    PROP_SERIES_DATAPOINT_LABEL_BORDER_WIDTH,
    PROP_SERIES_DATAPOINT_LABEL_BORDER_COLOR,
    PROP_SERIES_DATAPOINT_LABEL_BORDER_TRANS
};

void lcl_AddPropertiesToVector_PointProperties(
    std::vector< Property > & rOutProperties )
{
    //service chart::Chart3DBarProperties
    rOutProperties.emplace_back( "SolidType",
                  PROP_SERIES_DATAPOINT_SOLIDTYPE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "SegmentOffset",
                  PROP_SERIES_DATAPOINT_SEGMENT_OFFSET,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "D3DPercentDiagonal",
                  PROP_SERIES_DATAPOINT_PERCENT_DIAGONAL,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "LabelSeparator",
                  PROP_SERIES_DATAPOINT_LABEL_SEPARATOR,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_NUMFMT,
                  PROP_SERIES_NUMBERFORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "PercentageNumberFormat",
                  PROP_SERIES_PERCENTAGE_NUMBERFORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "TextWordWrap",
                  PROP_SERIES_DATAPOINT_TEXT_WORD_WRAP,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "LabelPlacement",
                  PROP_SERIES_DATAPOINT_LABEL_PLACEMENT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "TextRotation",
                  PROP_SERIES_DATAPOINT_TEXT_ROTATION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_LABEL_BORDER_STYLE,
                  PROP_SERIES_DATAPOINT_LABEL_BORDER_STYLE,
                  cppu::UnoType<drawing::LineStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_LABEL_BORDER_WIDTH,
                  PROP_SERIES_DATAPOINT_LABEL_BORDER_WIDTH,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_LABEL_BORDER_COLOR,
                  PROP_SERIES_DATAPOINT_LABEL_BORDER_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_LABEL_BORDER_TRANS,
                  PROP_SERIES_DATAPOINT_LABEL_BORDER_TRANS,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void lcl_AddPropertiesToVector_SeriesOnly(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Axis",
                  PROP_SERIES_ATTACHED_AXIS,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_LINK_TO_SRC_NUMFMT,
                  PROP_SERIES_LINK_NUMBERFORMAT_TO_SOURCE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

uno::Sequence< Property > lcl_GetPropertySequence( DataSeriesPointWrapper::eType _eType )
{
    std::vector< css::beans::Property > aProperties;

    lcl_AddPropertiesToVector_PointProperties( aProperties );
    if( _eType == DataSeriesPointWrapper::DATA_SERIES )
    {
        lcl_AddPropertiesToVector_SeriesOnly( aProperties );
        WrappedStatisticProperties::addProperties( aProperties );
    }
    WrappedSymbolProperties::addProperties( aProperties ); //for series and  points
    WrappedDataCaptionProperties::addProperties( aProperties ); //for series and  points

    ::chart::FillProperties::AddPropertiesToVector( aProperties );
    ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
    ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
    ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
    ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

    std::sort( aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess() );

    return comphelper::containerToSequence( aProperties );
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

//PROP_SERIES_ATTACHED_AXIS
class WrappedAttachedAxisProperty : public ::chart::WrappedProperty
{
public:
    explicit WrappedAttachedAxisProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

WrappedAttachedAxisProperty::WrappedAttachedAxisProperty(
                const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact )
                : WrappedProperty("Axis",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

Any WrappedAttachedAxisProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= css::chart::ChartAxisAssign::PRIMARY_Y;
    return aRet;
}

Any WrappedAttachedAxisProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet;

    uno::Reference< chart2::XDataSeries > xDataSeries( xInnerPropertySet, uno::UNO_QUERY );
    bool bAttachedToMainAxis = ::chart::DiagramHelper::isSeriesAttachedToMainAxis( xDataSeries );
    if( bAttachedToMainAxis )
        aRet <<= css::chart::ChartAxisAssign::PRIMARY_Y;
    else
        aRet <<= css::chart::ChartAxisAssign::SECONDARY_Y;
    return aRet;
}

void WrappedAttachedAxisProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    uno::Reference< chart2::XDataSeries > xDataSeries( xInnerPropertySet, uno::UNO_QUERY );

    sal_Int32 nChartAxisAssign = css::chart::ChartAxisAssign::PRIMARY_Y;
    if( ! (rOuterValue >>= nChartAxisAssign) )
        throw lang::IllegalArgumentException("Property Axis requires value of type sal_Int32", nullptr, 0 );

    bool bNewAttachedToMainAxis = nChartAxisAssign == css::chart::ChartAxisAssign::PRIMARY_Y;
    bool bOldAttachedToMainAxis = ::chart::DiagramHelper::isSeriesAttachedToMainAxis( xDataSeries );

    if( bNewAttachedToMainAxis != bOldAttachedToMainAxis)
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        if( xDiagram.is() )
            ::chart::DiagramHelper::attachSeriesToAxis( bNewAttachedToMainAxis, xDataSeries, xDiagram, m_spChart2ModelContact->m_xContext, false );
    }
}

class WrappedSegmentOffsetProperty : public ::chart::WrappedProperty
{
public:
    WrappedSegmentOffsetProperty();

protected:
    virtual Any convertInnerToOuterValue( const Any& rInnerValue ) const override;
    virtual Any convertOuterToInnerValue( const Any& rOuterValue ) const override;
};

WrappedSegmentOffsetProperty::WrappedSegmentOffsetProperty() :
        WrappedProperty("SegmentOffset","Offset")
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

class WrappedLineColorProperty : public WrappedSeriesAreaOrLineProperty
{
public:
    explicit WrappedLineColorProperty( DataSeriesPointWrapper* pDataSeriesPointWrapper );

    virtual void setPropertyValue( const Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual void setPropertyToDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected:
    DataSeriesPointWrapper* m_pDataSeriesPointWrapper;
    mutable Any             m_aDefaultValue;
    mutable Any             m_aOuterValue;
};

WrappedLineColorProperty::WrappedLineColorProperty(
                DataSeriesPointWrapper* pDataSeriesPointWrapper )
                : WrappedSeriesAreaOrLineProperty("LineColor","BorderColor","Color", pDataSeriesPointWrapper )
                , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
                , m_aDefaultValue(uno::Any(sal_Int32( 0x0099ccff )))  // blue 8
                , m_aOuterValue(m_aDefaultValue)
{
}

void WrappedLineColorProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
        m_aOuterValue = rOuterValue;
    else
        WrappedSeriesAreaOrLineProperty::setPropertyValue( rOuterValue, xInnerPropertySet );
}

void WrappedLineColorProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
{
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
        m_aOuterValue = m_aDefaultValue;
    else
        WrappedSeriesAreaOrLineProperty::setPropertyToDefault( xInnerPropertyState );
}

Any WrappedLineColorProperty::getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
{
    if( m_pDataSeriesPointWrapper && !m_pDataSeriesPointWrapper->isSupportingAreaProperties() )
        return m_aDefaultValue;
    else
        return WrappedSeriesAreaOrLineProperty::getPropertyDefault( xInnerPropertyState );
}

class WrappedLineStyleProperty : public WrappedSeriesAreaOrLineProperty
{
public:
    explicit WrappedLineStyleProperty( DataSeriesPointWrapper* pDataSeriesPointWrapper );

    virtual void setPropertyValue( const Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual void setPropertyToDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected:
    DataSeriesPointWrapper* m_pDataSeriesPointWrapper;
    mutable Any             m_aDefaultValue;
    mutable Any             m_aOuterValue;
};

WrappedLineStyleProperty::WrappedLineStyleProperty(
                DataSeriesPointWrapper* pDataSeriesPointWrapper )
                : WrappedSeriesAreaOrLineProperty("LineStyle","BorderStyle", "LineStyle", pDataSeriesPointWrapper )
                , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
                , m_aDefaultValue(uno::Any(drawing::LineStyle_SOLID))
                , m_aOuterValue(m_aDefaultValue)
{
}

void WrappedLineStyleProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aNewValue(rOuterValue);
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
    {
        m_aOuterValue = rOuterValue;
        aNewValue <<= drawing::LineStyle_NONE;
    }
    WrappedSeriesAreaOrLineProperty::setPropertyValue( aNewValue, xInnerPropertySet );
}

void WrappedLineStyleProperty::setPropertyToDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
{
    if( m_pDataSeriesPointWrapper && m_pDataSeriesPointWrapper->isLinesForbidden() )
        m_aOuterValue = m_aDefaultValue;
    else
        WrappedSeriesAreaOrLineProperty::setPropertyToDefault( xInnerPropertyState );
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

DataSeriesPointWrapper::DataSeriesPointWrapper(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : m_spChart2ModelContact( spChart2ModelContact )
    , m_aEventListenerContainer( m_aMutex )
    , m_eType( DATA_SERIES )
    , m_nSeriesIndexInNewAPI( -1 )
    , m_nPointIndex( -1 )
    , m_bLinesAllowed(true)
    , m_xDataSeries(nullptr)
{
    //need initialize call afterwards
}

void SAL_CALL DataSeriesPointWrapper::initialize( const uno::Sequence< uno::Any >& aArguments )
{
    OSL_PRECOND(aArguments.getLength() >= 1,"need at least 1 argument to initialize the DataSeriesPointWrapper: series reference + optional datapoint index");

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
            "DataSeries index invalid", static_cast< ::cppu::OWeakObject * >( this ));

    //todo: check upper border of point index

    if( m_nPointIndex >= 0 )
        m_eType = DATA_POINT;
    else
        m_eType = DATA_SERIES;
}

DataSeriesPointWrapper::DataSeriesPointWrapper(eType _eType,
                                               sal_Int32 nSeriesIndexInNewAPI ,
                                               sal_Int32 nPointIndex, //ignored for series
                                               const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : m_spChart2ModelContact( spChart2ModelContact )
    , m_aEventListenerContainer( m_aMutex )
    , m_eType( _eType )
    , m_nSeriesIndexInNewAPI( nSeriesIndexInNewAPI )
    , m_nPointIndex( (_eType == DATA_POINT) ? nPointIndex : -1 )
    , m_bLinesAllowed( false )
    , m_xDataSeries(nullptr)
{
}

DataSeriesPointWrapper::~DataSeriesPointWrapper()
{
}

// ____ XComponent ____
void SAL_CALL DataSeriesPointWrapper::dispose()
{
    uno::Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    m_xDataSeries.clear();
    clearWrappedPropertySet();
}

void SAL_CALL DataSeriesPointWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL DataSeriesPointWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ____ XEventListener ____
void SAL_CALL DataSeriesPointWrapper::disposing( const lang::EventObject& /*Source*/ )
{
}

bool DataSeriesPointWrapper::isSupportingAreaProperties()
{
    Reference< chart2::XDataSeries > xSeries( getDataSeries() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ) );
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );

    return ChartTypeHelper::isSupportingAreaProperties( xChartType, nDimensionCount );
}

Reference< chart2::XDataSeries > DataSeriesPointWrapper::getDataSeries()
{
    Reference< chart2::XDataSeries > xSeries( m_xDataSeries );
    if( !xSeries.is() )
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
            ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );

        if( m_nSeriesIndexInNewAPI >= 0 && m_nSeriesIndexInNewAPI < static_cast<sal_Int32>(aSeriesList.size()) )
            xSeries = aSeriesList[m_nSeriesIndexInNewAPI];
    }

    return xSeries;
}

Reference< beans::XPropertySet > DataSeriesPointWrapper::getDataPointProperties()
{
    Reference< beans::XPropertySet > xPointProp;

    Reference< chart2::XDataSeries > xSeries( getDataSeries() );

    // may throw an IllegalArgumentException
    if( xSeries.is() )
        xPointProp = xSeries->getDataPointByIndex( m_nPointIndex );

    return xPointProp;
}

//ReferenceSizePropertyProvider
void DataSeriesPointWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue("ReferencePageSize").hasValue() )
            xProp->setPropertyValue("ReferencePageSize", uno::Any(
                m_spChart2ModelContact->GetPageSize() ));
    }
}
Any DataSeriesPointWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue("ReferencePageSize");
    return aRet;
}
awt::Size DataSeriesPointWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

// WrappedPropertySet

//XPropertyState
beans::PropertyState SAL_CALL DataSeriesPointWrapper::getPropertyState( const OUString& rPropertyName )
{
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );
    try
    {
        if (rPropertyName == "SymbolBitmap" || rPropertyName == "SymbolBitmapURL")
        {
            uno::Any aAny = WrappedPropertySet::getPropertyValue("SymbolType");
            sal_Int32 nVal = css::chart::ChartSymbolType::NONE;
            if (aAny >>= nVal)
            {
                if (nVal != css::chart::ChartSymbolType::BITMAPURL)
                    return beans::PropertyState::PropertyState_DEFAULT_VALUE;
            }
        }

        if( m_eType == DATA_SERIES )
            aState = WrappedPropertySet::getPropertyState( rPropertyName );
        else
        {
            if( rPropertyName == "FillColor")
            {
                Reference< beans::XPropertySet > xSeriesProp( getDataSeries(), uno::UNO_QUERY );
                bool bVaryColorsByPoint = false;
                if( xSeriesProp.is() && (xSeriesProp->getPropertyValue("VaryColorsByPoint") >>= bVaryColorsByPoint)
                    && bVaryColorsByPoint )
                    return beans::PropertyState_DIRECT_VALUE;
            }
            else if( rPropertyName == "Lines"
                ||  rPropertyName == "SymbolType"
                ||  rPropertyName == "SymbolSize" )
                return WrappedPropertySet::getPropertyState( rPropertyName );

            uno::Any aDefault( getPropertyDefault( rPropertyName ) );
            uno::Any aValue( getPropertyValue( rPropertyName ) );
            if( aDefault==aValue )
                aState = beans::PropertyState_DEFAULT_VALUE;
        }
    }
    catch( const beans::UnknownPropertyException& )
    {
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        throw;
    }
    catch( const lang::WrappedTargetException& e )
    {
        css::uno::Any a(e.TargetException);
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }
    catch( const uno::Exception& e )
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            "wrapped Exception " + e.Message,
            css::uno::Reference<css::uno::XInterface>(), a);
    }
    return aState;
}

void SAL_CALL DataSeriesPointWrapper::setPropertyToDefault( const OUString& rPropertyName )
{
    if( m_eType == DATA_SERIES )
        WrappedPropertySet::setPropertyToDefault( rPropertyName );
    else
    {
        //for data points the default is given by the series
        setPropertyValue( rPropertyName, getPropertyDefault( rPropertyName ) );
    }
}
Any SAL_CALL DataSeriesPointWrapper::getPropertyDefault( const OUString& rPropertyName )
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
    catch( const beans::UnknownPropertyException& )
    {
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

std::vector< std::unique_ptr<WrappedProperty> > DataSeriesPointWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );

    if( m_eType == DATA_SERIES )
    {
        WrappedStatisticProperties::addWrappedPropertiesForSeries( aWrappedProperties, m_spChart2ModelContact );
        aWrappedProperties.emplace_back( new WrappedAttachedAxisProperty( m_spChart2ModelContact ) );

        aWrappedProperties.emplace_back( new WrappedNumberFormatProperty(m_spChart2ModelContact) );
        aWrappedProperties.emplace_back( new WrappedLinkNumberFormatProperty );
    }

    WrappedSymbolProperties::addWrappedPropertiesForSeries( aWrappedProperties, m_spChart2ModelContact );
    WrappedDataCaptionProperties::addWrappedPropertiesForSeries( aWrappedProperties, m_spChart2ModelContact );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    //add unnamed line properties (different inner names here)

    aWrappedProperties.emplace_back( new WrappedProperty("FillColor","Color") );
    aWrappedProperties.emplace_back( new WrappedLineStyleProperty( this ) );
    aWrappedProperties.emplace_back( new WrappedLineColorProperty( this ) );
    aWrappedProperties.emplace_back( new WrappedSeriesAreaOrLineProperty("LineDashName","BorderDashName","LineDashName", this ) );
    aWrappedProperties.emplace_back( new WrappedSeriesAreaOrLineProperty("LineTransparence","BorderTransparency","Transparency", this ) );
    aWrappedProperties.emplace_back( new WrappedSeriesAreaOrLineProperty("LineWidth","BorderWidth","LineWidth", this ) );
    aWrappedProperties.emplace_back( new WrappedProperty("FillStyle","FillStyle" ) );
    aWrappedProperties.emplace_back( new WrappedProperty("FillTransparence","Transparency") );

    aWrappedProperties.emplace_back( new WrappedIgnoreProperty("LineJoint", uno::Any( drawing::LineJoint_ROUND ) ) );
    aWrappedProperties.emplace_back( new WrappedProperty("FillTransparenceGradientName","TransparencyGradientName") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillGradientName","GradientName") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillGradientStepCount","GradientStepCount") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillHatchName","HatchName") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapName","FillBitmapName") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBackground","FillBackground") );

    //bitmap properties
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapMode","FillBitmapMode") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapSizeX","FillBitmapSizeX") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapSizeY","FillBitmapSizeY") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapLogicalSize","FillBitmapLogicalSize") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapOffsetX","FillBitmapOffsetX") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapOffsetY","FillBitmapOffsetY") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapRectanglePoint","FillBitmapRectanglePoint") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapPositionOffsetX","FillBitmapPositionOffsetX") );
    aWrappedProperties.emplace_back( new WrappedProperty("FillBitmapPositionOffsetY","FillBitmapPositionOffsetY") );

    aWrappedProperties.emplace_back( new WrappedProperty("SolidType","Geometry3D") );
    aWrappedProperties.emplace_back( new WrappedSegmentOffsetProperty() );
    aWrappedProperties.emplace_back( new WrappedProperty("D3DPercentDiagonal","PercentDiagonal") );

    aWrappedProperties.emplace_back( new WrappedTextRotationProperty() );

    return aWrappedProperties;
}

void SAL_CALL DataSeriesPointWrapper::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    if(rPropertyName == "Lines")
    {
        if( ! (rValue >>= m_bLinesAllowed) )
            throw lang::IllegalArgumentException("Property Lines requires value of type sal_Bool", nullptr, 0 );
    }

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    static const sal_Int32 nErrorCategoryHandle = getInfoHelper().getHandleByName("ErrorCategory");
    if( nErrorCategoryHandle == nHandle )
    {
        css::chart::ChartErrorCategory aNewValue = css::chart::ChartErrorCategory_NONE;
        rValue >>= aNewValue;
        Any aLow, aHigh;
        bool bSetHighAndLowValues = false;
        switch(aNewValue)
        {
            case css::chart::ChartErrorCategory_CONSTANT_VALUE:
                aHigh = getPropertyValue("ConstantErrorHigh");
                aLow = getPropertyValue("ConstantErrorLow");
                bSetHighAndLowValues = true;
                break;
            case css::chart::ChartErrorCategory_PERCENT:
                aHigh = aLow = getPropertyValue("PercentageError");
                bSetHighAndLowValues = true;
                break;
            case css::chart::ChartErrorCategory_ERROR_MARGIN:
                aHigh = aLow = getPropertyValue("ErrorMargin");
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
                case css::chart::ChartErrorCategory_CONSTANT_VALUE:
                    setPropertyValue("ConstantErrorHigh",aHigh);
                    setPropertyValue("ConstantErrorLow",aLow);
                    break;
                case css::chart::ChartErrorCategory_PERCENT:
                    setPropertyValue("PercentageError",aHigh);
                    break;
                case css::chart::ChartErrorCategory_ERROR_MARGIN:
                    setPropertyValue("ErrorMargin",aHigh);
                    break;
                default:
                    break;
            }
        }
    }
    else
        WrappedPropertySet::setPropertyValue( rPropertyName, rValue );
}

Any SAL_CALL DataSeriesPointWrapper::getPropertyValue( const OUString& rPropertyName )
{
    if( m_eType == DATA_POINT )
    {
        if( rPropertyName == "FillColor" )
        {
            Reference< beans::XPropertySet > xSeriesProp( getDataSeries(), uno::UNO_QUERY );
            bool bVaryColorsByPoint = false;
            if( xSeriesProp.is() && (xSeriesProp->getPropertyValue("VaryColorsByPoint") >>= bVaryColorsByPoint)
                && bVaryColorsByPoint )
            {
                uno::Reference< beans::XPropertyState > xPointState( DataSeriesPointWrapper::getDataPointProperties(), uno::UNO_QUERY );
                if( xPointState.is() && xPointState->getPropertyState("Color") == beans::PropertyState_DEFAULT_VALUE )
                {
                    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
                    if( xDiagram.is() )
                    {
                        Reference< chart2::XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme() );
                        if( xColorScheme.is() )
                            return uno::Any( xColorScheme->getColorByIndex( m_nPointIndex ) );
                    }
                }
            }
        }
    }
    return WrappedPropertySet::getPropertyValue( rPropertyName );
}

OUString SAL_CALL DataSeriesPointWrapper::getImplementationName()
{
    return OUString("com.sun.star.comp.chart.DataSeries");
}

sal_Bool SAL_CALL DataSeriesPointWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DataSeriesPointWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartDataRowProperties",
        "com.sun.star.chart.ChartDataPointProperties",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.beans.PropertySet",
        "com.sun.star.drawing.FillProperties",
        "com.sun.star.drawing.LineProperties",
        "com.sun.star.style.CharacterProperties"
    };
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
