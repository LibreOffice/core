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

#include <ChartDocumentWrapper.hxx>
#include <servicenames.hxx>
#include <PropertyHelper.hxx>
#include <TitleHelper.hxx>
#include <LegendHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <ModifyListenerHelper.hxx>
#include <DisposeHelper.hxx>
#include "DataSeriesPointWrapper.hxx"
#include <chartview/ExplicitValueProvider.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include "Chart2ModelContact.hxx"

#include <ChartModel.hxx>

#include <DiagramHelper.hxx>
#include <DataSourceHelper.hxx>
#include <ChartModelHelper.hxx>
#include <AxisHelper.hxx>
#include <ThreeDHelper.hxx>

#include "TitleWrapper.hxx"
#include "ChartDataWrapper.hxx"
#include "DiagramWrapper.hxx"
#include "LegendWrapper.hxx"
#include "AreaWrapper.hxx"
#include "WrappedAddInProperty.hxx"
#include <WrappedIgnoreProperty.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <tools/diagnose_ex.h>

#include <vector>
#include <algorithm>
#include <functional>
#include <map>
#include <o3tl/functional.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{
enum eServiceType
{
    SERVICE_NAME_AREA_DIAGRAM = 0,
    SERVICE_NAME_BAR_DIAGRAM,
    SERVICE_NAME_DONUT_DIAGRAM,
    SERVICE_NAME_LINE_DIAGRAM,
    SERVICE_NAME_NET_DIAGRAM,
    SERVICE_NAME_FILLED_NET_DIAGRAM,
    SERVICE_NAME_PIE_DIAGRAM,
    SERVICE_NAME_STOCK_DIAGRAM,
    SERVICE_NAME_XY_DIAGRAM,
    SERVICE_NAME_BUBBLE_DIAGRAM,

    SERVICE_NAME_DASH_TABLE,
    SERVICE_NAME_GARDIENT_TABLE,
    SERVICE_NAME_HATCH_TABLE,
    SERVICE_NAME_BITMAP_TABLE,
    SERVICE_NAME_TRANSP_GRADIENT_TABLE,
    SERVICE_NAME_MARKER_TABLE,

    SERVICE_NAME_NAMESPACE_MAP,
    SERVICE_NAME_EXPORT_GRAPHIC_STORAGE_RESOLVER,
    SERVICE_NAME_IMPORT_GRAPHIC_STORAGE_RESOLVER
};

typedef std::map< OUString, enum eServiceType > tServiceNameMap;

tServiceNameMap & lcl_getStaticServiceNameMap()
{
    static tServiceNameMap aServiceNameMap {
        {"com.sun.star.chart.AreaDiagram",                    SERVICE_NAME_AREA_DIAGRAM},
        {"com.sun.star.chart.BarDiagram",                     SERVICE_NAME_BAR_DIAGRAM},
        {"com.sun.star.chart.DonutDiagram",                   SERVICE_NAME_DONUT_DIAGRAM},
        {"com.sun.star.chart.LineDiagram",                    SERVICE_NAME_LINE_DIAGRAM},
        {"com.sun.star.chart.NetDiagram",                     SERVICE_NAME_NET_DIAGRAM},
        {"com.sun.star.chart.FilledNetDiagram",               SERVICE_NAME_FILLED_NET_DIAGRAM},
        {"com.sun.star.chart.PieDiagram",                     SERVICE_NAME_PIE_DIAGRAM},
        {"com.sun.star.chart.StockDiagram",                   SERVICE_NAME_STOCK_DIAGRAM},
        {"com.sun.star.chart.XYDiagram",                      SERVICE_NAME_XY_DIAGRAM},
        {"com.sun.star.chart.BubbleDiagram",                  SERVICE_NAME_BUBBLE_DIAGRAM},

        {"com.sun.star.drawing.DashTable",                    SERVICE_NAME_DASH_TABLE},
        {"com.sun.star.drawing.GradientTable",                SERVICE_NAME_GARDIENT_TABLE},
        {"com.sun.star.drawing.HatchTable",                   SERVICE_NAME_HATCH_TABLE},
        {"com.sun.star.drawing.BitmapTable",                  SERVICE_NAME_BITMAP_TABLE},
        {"com.sun.star.drawing.TransparencyGradientTable",    SERVICE_NAME_TRANSP_GRADIENT_TABLE},
        {"com.sun.star.drawing.MarkerTable",                  SERVICE_NAME_MARKER_TABLE},

        {"com.sun.star.xml.NamespaceMap",                     SERVICE_NAME_NAMESPACE_MAP},
        {"com.sun.star.document.ExportGraphicStoreageHandler", SERVICE_NAME_EXPORT_GRAPHIC_STORAGE_RESOLVER},
        {"com.sun.star.document.ImportGraphicStoreageHandler", SERVICE_NAME_IMPORT_GRAPHIC_STORAGE_RESOLVER}
    };

    return aServiceNameMap;
}

enum
{
    PROP_DOCUMENT_HAS_MAIN_TITLE,
    PROP_DOCUMENT_HAS_SUB_TITLE,
    PROP_DOCUMENT_HAS_LEGEND,
    PROP_DOCUMENT_LABELS_IN_FIRST_ROW,
    PROP_DOCUMENT_LABELS_IN_FIRST_COLUMN,
    PROP_DOCUMENT_ADDIN,
    PROP_DOCUMENT_BASEDIAGRAM,
    PROP_DOCUMENT_ADDITIONAL_SHAPES,
    PROP_DOCUMENT_UPDATE_ADDIN,
    PROP_DOCUMENT_NULL_DATE,
    PROP_DOCUMENT_ENABLE_COMPLEX_CHARTTYPES,
    PROP_DOCUMENT_ENABLE_DATATABLE_DIALOG
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "HasMainTitle",
                  PROP_DOCUMENT_HAS_MAIN_TITLE,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasSubTitle",
                  PROP_DOCUMENT_HAS_SUB_TITLE,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasLegend",
                  PROP_DOCUMENT_HAS_LEGEND,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );

    // really needed?
    rOutProperties.emplace_back( "DataSourceLabelsInFirstRow",
                  PROP_DOCUMENT_LABELS_IN_FIRST_ROW,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "DataSourceLabelsInFirstColumn",
                  PROP_DOCUMENT_LABELS_IN_FIRST_COLUMN,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );

    //add-in
    rOutProperties.emplace_back( "AddIn",
                  PROP_DOCUMENT_ADDIN,
                  cppu::UnoType<util::XRefreshable>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "BaseDiagram",
                  PROP_DOCUMENT_BASEDIAGRAM,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "AdditionalShapes",
                  PROP_DOCUMENT_ADDITIONAL_SHAPES,
                  cppu::UnoType<drawing::XShapes>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::READONLY );
    rOutProperties.emplace_back( "RefreshAddInAllowed",
                  PROP_DOCUMENT_UPDATE_ADDIN,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::TRANSIENT );

    // table:null-date // i99104
    rOutProperties.emplace_back( "NullDate",
                  PROP_DOCUMENT_NULL_DATE,
                  ::cppu::UnoType<css::util::DateTime>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "EnableComplexChartTypes",
                  PROP_DOCUMENT_ENABLE_COMPLEX_CHARTTYPES,
                  cppu::UnoType<bool>::get(),
                  //#i112666# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "EnableDataTableDialog",
                  PROP_DOCUMENT_ENABLE_DATATABLE_DIALOG,
                  cppu::UnoType<bool>::get(),
                  //#i112666# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );
}

struct StaticChartDocumentWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    static uno::Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticChartDocumentWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticChartDocumentWrapperPropertyArray_Initializer >
{
};

} //  anonymous namespace

namespace chart
{
namespace wrapper
{

//PROP_DOCUMENT_LABELS_IN_FIRST_ROW
class WrappedDataSourceLabelsInFirstRowProperty : public WrappedProperty
{
public:
    explicit WrappedDataSourceLabelsInFirstRowProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedDataSourceLabelsInFirstRowProperty::WrappedDataSourceLabelsInFirstRowProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("DataSourceLabelsInFirstRow",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedDataSourceLabelsInFirstRowProperty::getPropertyDefault( nullptr );
}

void WrappedDataSourceLabelsInFirstRowProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bLabelsInFirstRow = true;
    if( ! (rOuterValue >>= bLabelsInFirstRow) )
        throw lang::IllegalArgumentException("Property DataSourceLabelsInFirstRow requires value of type boolean", nullptr, 0 );

    m_aOuterValue = rOuterValue;
    bool bNewValue = bLabelsInFirstRow;

    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        if( bUseColumns && bNewValue != bFirstCellAsLabel )
        {
            DataSourceHelper::setRangeSegmentation(
                m_spChart2ModelContact->getChartModel(), aSequenceMapping, bUseColumns ,bNewValue, bHasCategories );
        }
        else if( !bUseColumns && bNewValue != bHasCategories )
        {
            DataSourceHelper::setRangeSegmentation(
                m_spChart2ModelContact->getChartModel(), aSequenceMapping, bUseColumns , bFirstCellAsLabel, bNewValue );
        }
    }
}

Any WrappedDataSourceLabelsInFirstRowProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        bool bLabelsInFirstRow = true;
        if( bUseColumns )
            bLabelsInFirstRow = bFirstCellAsLabel;
        else
            bLabelsInFirstRow = bHasCategories;

        m_aOuterValue <<= bLabelsInFirstRow;
    }
    return m_aOuterValue;
}

Any WrappedDataSourceLabelsInFirstRowProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= true;
    return aRet;
}

//PROP_DOCUMENT_LABELS_IN_FIRST_COLUMN
class WrappedDataSourceLabelsInFirstColumnProperty : public WrappedProperty
{
public:
    explicit WrappedDataSourceLabelsInFirstColumnProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedDataSourceLabelsInFirstColumnProperty::WrappedDataSourceLabelsInFirstColumnProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("DataSourceLabelsInFirstColumn",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedDataSourceLabelsInFirstColumnProperty::getPropertyDefault( nullptr );
}

void WrappedDataSourceLabelsInFirstColumnProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bLabelsInFirstRow = true;
    if( ! (rOuterValue >>= bLabelsInFirstRow) )
        throw lang::IllegalArgumentException("Property DataSourceLabelsInFirstRow requires value of type boolean", nullptr, 0 );

    m_aOuterValue = rOuterValue;
    bool bNewValue = bLabelsInFirstRow;

    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        if( bUseColumns && bNewValue != bHasCategories )
        {
            DataSourceHelper::setRangeSegmentation(
                m_spChart2ModelContact->getChartModel(), aSequenceMapping, bUseColumns, bFirstCellAsLabel, bNewValue );
        }
        else if( !bUseColumns && bNewValue != bFirstCellAsLabel )
        {
            DataSourceHelper::setRangeSegmentation(
                m_spChart2ModelContact->getChartModel(), aSequenceMapping, bUseColumns , bNewValue, bHasCategories );
        }
    }
}

Any WrappedDataSourceLabelsInFirstColumnProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        bool bLabelsInFirstColumn = true;
        if( bUseColumns )
            bLabelsInFirstColumn = bHasCategories;
        else
            bLabelsInFirstColumn = bFirstCellAsLabel;

        m_aOuterValue <<= bLabelsInFirstColumn;
    }
    return m_aOuterValue;
}

Any WrappedDataSourceLabelsInFirstColumnProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= true;
    return aRet;
}

//PROP_DOCUMENT_HAS_LEGEND
class WrappedHasLegendProperty : public WrappedProperty
{
public:
    explicit WrappedHasLegendProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact > m_spChart2ModelContact;
};

WrappedHasLegendProperty::WrappedHasLegendProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : WrappedProperty("HasLegend",OUString())
    , m_spChart2ModelContact( spChart2ModelContact )
{
}

void WrappedHasLegendProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = true;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException("Property HasLegend requires value of type boolean", nullptr, 0 );

    try
    {
        Reference< chart2::XLegend > xLegend( LegendHelper::getLegend( *m_spChart2ModelContact->getModel(), m_spChart2ModelContact->m_xContext,bNewValue ));
        if(xLegend.is())
        {
            Reference< beans::XPropertySet > xLegendProp( xLegend, uno::UNO_QUERY_THROW );
            bool bOldValue = true;
            Any aAOld = xLegendProp->getPropertyValue("Show");
            aAOld >>= bOldValue;
            if( bOldValue != bNewValue )
                xLegendProp->setPropertyValue("Show", uno::Any( bNewValue ));
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

Any WrappedHasLegendProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Any aRet;
    try
    {
        Reference< beans::XPropertySet > xLegendProp(
            LegendHelper::getLegend( *m_spChart2ModelContact->getModel() ), uno::UNO_QUERY );
        if( xLegendProp.is())
            aRet = xLegendProp->getPropertyValue("Show");
        else
            aRet <<= false;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aRet;
}

Any WrappedHasLegendProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

//PROP_DOCUMENT_HAS_MAIN_TITLE
class WrappedHasMainTitleProperty : public WrappedProperty
{
public:
    explicit WrappedHasMainTitleProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact > m_spChart2ModelContact;
};

WrappedHasMainTitleProperty::WrappedHasMainTitleProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : WrappedProperty("HasMainTitle",OUString())
    , m_spChart2ModelContact( spChart2ModelContact )
{
}

void WrappedHasMainTitleProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = true;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException("Property HasMainTitle requires value of type boolean", nullptr, 0 );

    try
    {
        if( bNewValue )
            TitleHelper::createTitle( TitleHelper::MAIN_TITLE, "main-title", m_spChart2ModelContact->getChartModel(), m_spChart2ModelContact->m_xContext );
        else
            TitleHelper::removeTitle( TitleHelper::MAIN_TITLE, m_spChart2ModelContact->getChartModel() );
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

Any WrappedHasMainTitleProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Any aRet;
    try
    {
        aRet <<= TitleHelper::getTitle( TitleHelper::MAIN_TITLE, m_spChart2ModelContact->getChartModel() ).is();
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aRet;
}

Any WrappedHasMainTitleProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

//PROP_DOCUMENT_HAS_SUB_TITLE
class WrappedHasSubTitleProperty : public WrappedProperty
{
public:
    explicit WrappedHasSubTitleProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact > m_spChart2ModelContact;
};

WrappedHasSubTitleProperty::WrappedHasSubTitleProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("HasSubTitle",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

void WrappedHasSubTitleProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = true;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException("Property HasSubTitle requires value of type boolean", nullptr, 0 );

    try
    {
        if( bNewValue )
            TitleHelper::createTitle( TitleHelper::SUB_TITLE, "", m_spChart2ModelContact->getChartModel(), m_spChart2ModelContact->m_xContext );
        else
            TitleHelper::removeTitle( TitleHelper::SUB_TITLE, m_spChart2ModelContact->getChartModel() );
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

Any WrappedHasSubTitleProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Any aRet;
    try
    {
        aRet <<= TitleHelper::getTitle( TitleHelper::SUB_TITLE, m_spChart2ModelContact->getChartModel() ).is();
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aRet;
}

Any WrappedHasSubTitleProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

ChartDocumentWrapper::ChartDocumentWrapper(
    const Reference< uno::XComponentContext > & xContext ) :
        m_spChart2ModelContact( new Chart2ModelContact( xContext ) ),
        m_bUpdateAddIn( true ),
        m_bIsDisposed( false )
{
}

ChartDocumentWrapper::~ChartDocumentWrapper()
{
    stopAllComponentListening();
}

// ____ XInterface (for new interfaces) ____
uno::Any SAL_CALL ChartDocumentWrapper::queryInterface( const uno::Type& aType )
{
    if( m_xDelegator.is())
        // calls queryAggregation if the delegator doesn't know aType
        return m_xDelegator->queryInterface( aType );
    else
        return queryAggregation( aType );
}

// ____ chart::XChartDocument (old API wrapper) ____
Reference< drawing::XShape > SAL_CALL ChartDocumentWrapper::getTitle()
{
    if( !m_xTitle.is()  )
    {
        ControllerLockGuardUNO aCtrlLockGuard( Reference< frame::XModel >( m_spChart2ModelContact->getChart2Document(), uno::UNO_QUERY ));
        m_xTitle = new TitleWrapper( TitleHelper::MAIN_TITLE, m_spChart2ModelContact );
    }
    return m_xTitle;
}

Reference< drawing::XShape > SAL_CALL ChartDocumentWrapper::getSubTitle()
{
    if( !m_xSubTitle.is() )
    {
        ControllerLockGuardUNO aCtrlLockGuard( Reference< frame::XModel >( m_spChart2ModelContact->getChart2Document(), uno::UNO_QUERY ));
        m_xSubTitle = new TitleWrapper( TitleHelper::SUB_TITLE, m_spChart2ModelContact );
    }
    return m_xSubTitle;
}

Reference< drawing::XShape > SAL_CALL ChartDocumentWrapper::getLegend()
{
    if( ! m_xLegend.is())
    {
        m_xLegend = new LegendWrapper( m_spChart2ModelContact );
    }

    return m_xLegend;
}

Reference< beans::XPropertySet > SAL_CALL ChartDocumentWrapper::getArea()
{
    if( ! m_xArea.is())
    {
        m_xArea.set( new AreaWrapper( m_spChart2ModelContact ) );
    }

    return m_xArea;
}

Reference< XDiagram > SAL_CALL ChartDocumentWrapper::getDiagram()
{
    if( !m_xDiagram.is()  )
    {
        try
        {
            m_xDiagram = new DiagramWrapper( m_spChart2ModelContact );
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return m_xDiagram;
}

void SAL_CALL ChartDocumentWrapper::setDiagram( const Reference< XDiagram >& xDiagram )
{
    uno::Reference< util::XRefreshable > xAddIn( xDiagram, uno::UNO_QUERY );
    if( xAddIn.is() )
    {
        setAddIn( xAddIn );
    }
    else if( xDiagram.is() && xDiagram != m_xDiagram )
    {
        // set new wrapped diagram at new chart.  This requires the old
        // diagram given as parameter to implement the new interface.  If
        // this is not possible throw an exception
        Reference< chart2::XDiagramProvider > xNewDiaProvider( xDiagram, uno::UNO_QUERY_THROW );
        Reference< chart2::XDiagram > xNewDia( xNewDiaProvider->getDiagram());

        try
        {
            Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
            if( xChartDoc.is() )
            {
                // set the new diagram
                xChartDoc->setFirstDiagram( xNewDia );
                m_xDiagram = xDiagram;
            }
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

Reference< XChartData > SAL_CALL ChartDocumentWrapper::getData()
{
    if( !m_xChartData.is() )
    {
        m_xChartData.set( new ChartDataWrapper( m_spChart2ModelContact ) );
    }
    //@todo: check hasInternalDataProvider also in else?

    return m_xChartData;
}

void SAL_CALL ChartDocumentWrapper::attachData( const Reference< XChartData >& xNewData )
{
    if( !xNewData.is() )
        return;

    ControllerLockGuardUNO aCtrlLockGuard( Reference< frame::XModel >( m_spChart2ModelContact->getChart2Document(), uno::UNO_QUERY ));
    m_xChartData.set( new ChartDataWrapper( m_spChart2ModelContact, xNewData ) );
}

// ____ XModel ____
sal_Bool SAL_CALL ChartDocumentWrapper::attachResource(
    const OUString& URL,
    const Sequence< beans::PropertyValue >& Arguments )
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        return xModel->attachResource( URL, Arguments );
    return false;
}

OUString SAL_CALL ChartDocumentWrapper::getURL()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        return xModel->getURL();
    return OUString();
}

Sequence< beans::PropertyValue > SAL_CALL ChartDocumentWrapper::getArgs()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        return xModel->getArgs();
    return Sequence< beans::PropertyValue >();
}

void SAL_CALL ChartDocumentWrapper::connectController( const Reference< frame::XController >& Controller )
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->connectController( Controller );
}

void SAL_CALL ChartDocumentWrapper::disconnectController(
    const Reference< frame::XController >& Controller )
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->disconnectController( Controller );
}

void SAL_CALL ChartDocumentWrapper::lockControllers()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->lockControllers();
}

void SAL_CALL ChartDocumentWrapper::unlockControllers()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->unlockControllers();
}

sal_Bool SAL_CALL ChartDocumentWrapper::hasControllersLocked()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        return xModel->hasControllersLocked();
    return false;
}

Reference< frame::XController > SAL_CALL ChartDocumentWrapper::getCurrentController()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        return xModel->getCurrentController();
    return nullptr;
}

void SAL_CALL ChartDocumentWrapper::setCurrentController(
    const Reference< frame::XController >& Controller )
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->setCurrentController( Controller );
}

Reference< uno::XInterface > SAL_CALL ChartDocumentWrapper::getCurrentSelection()
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        return xModel->getCurrentSelection();
    return nullptr;
}

// ____ XComponent ____
void SAL_CALL ChartDocumentWrapper::dispose()
{
    if( m_bIsDisposed )
        throw lang::DisposedException("ChartDocumentWrapper is disposed",
            static_cast< ::cppu::OWeakObject* >( this ));

    m_bIsDisposed = true;

    try
    {
        Reference< lang::XComponent > xFormerDelegator( m_xDelegator, uno::UNO_QUERY );
        DisposeHelper::DisposeAndClear( m_xTitle );
        DisposeHelper::DisposeAndClear( m_xSubTitle );
        DisposeHelper::DisposeAndClear( m_xLegend );
        DisposeHelper::DisposeAndClear( m_xChartData );
        DisposeHelper::DisposeAndClear( m_xDiagram );
        DisposeHelper::DisposeAndClear( m_xArea );
        m_xChartView.set( nullptr );
        m_xShapeFactory.set( nullptr );
        m_xDelegator.set( nullptr );

        clearWrappedPropertySet();
        m_spChart2ModelContact->clear();
        impl_resetAddIn();

        stopAllComponentListening();

        try
        {
            if( xFormerDelegator.is())
                xFormerDelegator->dispose();
        }
        catch (const lang::DisposedException&)
        {
            // this is ok, don't panic
        }
    }
    catch (const uno::Exception &)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ChartDocumentWrapper::impl_resetAddIn()
{
    Reference< util::XRefreshable > xAddIn( m_xAddIn );
    m_xAddIn.set( nullptr );

    if( xAddIn.is() )
    {
        try
        {
            //make sure that the add-in does not hold a references to us anymore:
            Reference< lang::XComponent > xComp( xAddIn, uno::UNO_QUERY );
            if( xComp.is())
                xComp->dispose();
            else
            {
                uno::Reference< lang::XInitialization > xInit( xAddIn, uno::UNO_QUERY );
                if( xInit.is() )
                {
                    uno::Any aParam;
                    uno::Reference< css::chart::XChartDocument > xDoc( nullptr );
                    aParam <<= xDoc;
                    uno::Sequence< uno::Any > aSeq( &aParam, 1 );
                    xInit->initialize( aSeq );
                }
            }
        }
        catch (const uno::RuntimeException&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

void ChartDocumentWrapper::setBaseDiagram( const OUString& rBaseDiagram )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    m_aBaseDiagram = rBaseDiagram;

    uno::Reference< XDiagram > xDiagram( ChartDocumentWrapper::createInstance( rBaseDiagram ), uno::UNO_QUERY );
    if( xDiagram.is() )
        setDiagram( xDiagram );
}

void ChartDocumentWrapper::setAddIn( const Reference< util::XRefreshable >& xAddIn )
{
    if( m_xAddIn == xAddIn )
        return;

    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    impl_resetAddIn();
    m_xAddIn = xAddIn;
    // initialize AddIn with this as chart document
    uno::Reference< lang::XInitialization > xInit( m_xAddIn, uno::UNO_QUERY );
    if( xInit.is() )
    {
        uno::Any aParam;
        uno::Reference< XChartDocument > xDoc( static_cast<XChartDocument*>(this), uno::UNO_QUERY );
        aParam <<= xDoc;
        uno::Sequence< uno::Any > aSeq( &aParam, 1 );
        xInit->initialize( aSeq );
    }
}

void ChartDocumentWrapper::setUpdateAddIn( bool bUpdateAddIn )
{
    m_bUpdateAddIn = bUpdateAddIn;
}

Reference< drawing::XShapes > ChartDocumentWrapper::getAdditionalShapes() const
{
    // get additional non-chart shapes for XML export
    uno::Reference< drawing::XShapes > xFoundShapes;
    uno::Reference< drawing::XDrawPage > xDrawPage( impl_getDrawPage() );

    uno::Reference< drawing::XShapes > xDrawPageShapes( xDrawPage, uno::UNO_QUERY );
    if( !xDrawPageShapes.is() )
        return xFoundShapes;

    uno::Reference<drawing::XShapes> xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );

    // iterate 'flat' over all top-level objects
    // and determine all that are no chart objects
    std::vector< uno::Reference< drawing::XShape > > aShapeVector;
    sal_Int32 nSubCount = xDrawPageShapes->getCount();
    uno::Reference< drawing::XShape > xShape;
    for( sal_Int32 nS = 0; nS < nSubCount; nS++ )
    {
        if( xDrawPageShapes->getByIndex( nS ) >>= xShape )
        {
            if( xShape.is() && xChartRoot!=xShape )
                aShapeVector.push_back( xShape );
        }
    }

    if( !aShapeVector.empty() )
    {
        // create a shape collection
        xFoundShapes.set( drawing::ShapeCollection::create(
                    comphelper::getProcessComponentContext()), uno::UNO_QUERY );

        OSL_ENSURE( xFoundShapes.is(), "Couldn't create a shape collection!" );
        if( xFoundShapes.is())
        {
            for (auto const& shape : aShapeVector)
                xFoundShapes->add(shape);
        }
    }

    return xFoundShapes;
}

void SAL_CALL ChartDocumentWrapper::addEventListener( const Reference< lang::XEventListener >& xListener )
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->addEventListener( xListener );
}

void SAL_CALL ChartDocumentWrapper::removeEventListener( const Reference< lang::XEventListener >& aListener )
{
    Reference< frame::XModel > xModel( m_spChart2ModelContact->getChartModel() );
    if( xModel.is() )
        xModel->removeEventListener( aListener );
}

// ____ XDrawPageSupplier ____
uno::Reference< drawing::XDrawPage > SAL_CALL ChartDocumentWrapper::getDrawPage()
{
    return impl_getDrawPage();
}

uno::Reference< drawing::XDrawPage > ChartDocumentWrapper::impl_getDrawPage() const
{
    return m_spChart2ModelContact->getDrawPage();
}

namespace {

uno::Reference< lang::XMultiServiceFactory > getShapeFactory(const uno::Reference<uno::XInterface>& xChartView)
{
    Reference< lang::XUnoTunnel> xUnoTunnel(xChartView,uno::UNO_QUERY);
    if(xUnoTunnel.is())
    {
        ExplicitValueProvider* pProvider = reinterpret_cast<ExplicitValueProvider*>(xUnoTunnel->getSomething(
                    ExplicitValueProvider::getUnoTunnelId() ));
        if( pProvider )
            return pProvider->getDrawModelWrapper()->getShapeFactory();

    }
    return uno::Reference< lang::XMultiServiceFactory >();
}

}

// ____ XMultiServiceFactory ____
uno::Reference< uno::XInterface > SAL_CALL ChartDocumentWrapper::createInstance(
    const OUString& aServiceSpecifier )
{
    uno::Reference< uno::XInterface > xResult;

    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( !xChartDoc.is() )
        return xResult;

    bool bServiceFound = false;
    tServiceNameMap & rMap = lcl_getStaticServiceNameMap();

    tServiceNameMap::const_iterator aIt( rMap.find( aServiceSpecifier ));
    if( aIt != rMap.end())
    {
        bool bCreateDiagram = false;
        uno::Reference< lang::XMultiServiceFactory > xManagerFact(
            xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        uno::Reference< chart2::XChartTypeTemplate > xTemplate;

        switch( (*aIt).second )
        {
            case SERVICE_NAME_AREA_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Area"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_BAR_DIAGRAM:
                if( xManagerFact.is())
                {
                    // this is for bar and column (the latter is the default if
                    // no "Vertical=false" property was set)
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Column"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_DONUT_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Donut"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_LINE_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Line"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_NET_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Net"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_FILLED_NET_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.FilledNet"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_PIE_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Pie"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_STOCK_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.StockLowHighClose"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_XY_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.ScatterLineSymbol"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;

            case SERVICE_NAME_BUBBLE_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance("com.sun.star.chart2.template.Bubble"),
                                uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;

            case SERVICE_NAME_DASH_TABLE:
            case SERVICE_NAME_GARDIENT_TABLE:
            case SERVICE_NAME_HATCH_TABLE:
            case SERVICE_NAME_BITMAP_TABLE:
            case SERVICE_NAME_TRANSP_GRADIENT_TABLE:
            case SERVICE_NAME_MARKER_TABLE:
                {
                    uno::Reference< lang::XMultiServiceFactory > xTableFactory( xChartDoc, uno::UNO_QUERY );
                    OSL_ENSURE( xTableFactory.get() != this, "new model is expected to implement service factory for gradient table etc" );
                    if( xTableFactory.is() && xTableFactory.get() != this )
                        xResult.set( xTableFactory->createInstance( aIt->first ), uno::UNO_QUERY );
                }
                break;

            case SERVICE_NAME_NAMESPACE_MAP:
                break;
            case SERVICE_NAME_EXPORT_GRAPHIC_STORAGE_RESOLVER:
                break;
            case SERVICE_NAME_IMPORT_GRAPHIC_STORAGE_RESOLVER:
                break;
        }

        if( bCreateDiagram && xTemplate.is() )
        {
            try
            {
                uno::Reference< chart2::XDiagram > xDia( xChartDoc->getFirstDiagram());
                if( xDia.is())
                {
                    // locked controllers
                    Reference< frame::XModel > xModel( xChartDoc, uno::UNO_QUERY );
                    ControllerLockGuardUNO aCtrlLockGuard( xModel );
                    Reference< chart2::XDiagram > xDiagram = ChartModelHelper::findDiagram( xModel );
                    ThreeDLookScheme e3DScheme = ThreeDHelper::detectScheme( xDiagram );
                    Reference< lang::XMultiServiceFactory > xTemplateManager( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
                    DiagramHelper::tTemplateWithServiceName aTemplateWithService(
                        DiagramHelper::getTemplateForDiagram( xDiagram, xTemplateManager ));
                    if( aTemplateWithService.first.is())
                        aTemplateWithService.first->resetStyles( xDiagram );//#i109371#
                    xTemplate->changeDiagram( xDiagram );
                    if( AllSettings::GetMathLayoutRTL() )
                        AxisHelper::setRTLAxisLayout( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 ) );
                    ThreeDHelper::setScheme( xDiagram, e3DScheme );
                }
                else
                {
                    // locked controllers
                    ControllerLockGuardUNO aCtrlLockGuard( Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ));
                    xDia.set( xTemplate->createDiagramByDataSource(
                                  uno::Reference< chart2::data::XDataSource >(),
                                  uno::Sequence< beans::PropertyValue >()));
                    xChartDoc->setFirstDiagram( xDia );
                }

                xResult = static_cast< ::cppu::OWeakObject* >( new DiagramWrapper( m_spChart2ModelContact ));
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }

        bServiceFound = true;
    }
    else if( aServiceSpecifier == "com.sun.star.comp.chart2.DataSeriesWrapper" )
    {
        Reference< beans::XPropertySet > xDataSeries( new DataSeriesPointWrapper( m_spChart2ModelContact ) );
        xResult.set( xDataSeries );
        bServiceFound = true;
    }
    else if( aServiceSpecifier == CHART_VIEW_SERVICE_NAME )
    {
        if( !m_xChartView.is() )
        {
            Reference< lang::XMultiServiceFactory > xFact(
                m_spChart2ModelContact->m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );
            Reference< lang::XInitialization > xViewInit( xFact->createInstance(
                    CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
            if(xViewInit.is())
            {
                try
                {
                    m_xChartView = xViewInit;

                    Sequence< Any > aArguments(2);
                    aArguments[0] <<= Reference<frame::XModel>(this);
                    aArguments[1] <<= true; // bRefreshAddIn
                    xViewInit->initialize(aArguments);
                }
                catch (const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("chart2");
                }
            }
        }
        xResult.set( m_xChartView );
        bServiceFound = true;
    }
    else
    {
        // try to create a shape
        try
        {
            if( !m_xShapeFactory.is() && m_xChartView.is() )
            {
                m_xShapeFactory = getShapeFactory( m_xChartView );
            }
            else
            {
                ChartModel* pModel = m_spChart2ModelContact->getModel();
                if(pModel)
                {
                    m_xChartView = pModel->getChartView();
                    m_xShapeFactory = getShapeFactory( m_xChartView );
                }
            }

            if( m_xShapeFactory.is() )
            {
                xResult = m_xShapeFactory->createInstance( aServiceSpecifier );
                bServiceFound = true;
            }
        }
        catch (const uno::Exception&)
        {
            // couldn't create shape
        }
    }

    // finally, try to create an addin
    if( !bServiceFound )
    {
        try
        {
            Reference< lang::XMultiServiceFactory > xFact(
                m_spChart2ModelContact->m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );
            uno::Reference< util::XRefreshable > xAddIn(
                xFact->createInstance( aServiceSpecifier ), uno::UNO_QUERY );
            if( xAddIn.is() )
            {
                xResult = xAddIn;
            }
        }
        catch (const uno::Exception&)
        {
            // couldn't create service
        }
    }

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL ChartDocumentWrapper::createInstanceWithArguments(
    const OUString& ServiceSpecifier,
    const uno::Sequence< uno::Any >& Arguments )
{
    OSL_ENSURE( Arguments.getLength(), "createInstanceWithArguments: Warning: Arguments are ignored" );

    return createInstance( ServiceSpecifier );
}

uno::Sequence< OUString > SAL_CALL ChartDocumentWrapper::getAvailableServiceNames()
{
    return comphelper::mapKeysToSequence( lcl_getStaticServiceNameMap() );
}

// ____ XAggregation ____
void SAL_CALL ChartDocumentWrapper::setDelegator(
    const uno::Reference< uno::XInterface >& rDelegator )
{
    if( m_bIsDisposed )
    {
        if( rDelegator.is() )
            throw lang::DisposedException("ChartDocumentWrapper is disposed",
                static_cast< ::cppu::OWeakObject* >( this ));
        return;
    }

    if( rDelegator.is())
    {
        m_xDelegator = rDelegator;
        m_spChart2ModelContact->setModel( uno::Reference< frame::XModel >(m_xDelegator, uno::UNO_QUERY) );
    }
    else
    {
        // this is a sort of dispose() from the new model,so release resources here
        try
        {
            dispose();
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

uno::Any SAL_CALL ChartDocumentWrapper::queryAggregation( const uno::Type& rType )
{
    return ChartDocumentWrapper_Base::queryInterface( rType );
}

// ____ ::utl::OEventListenerAdapter ____
void ChartDocumentWrapper::_disposing( const lang::EventObject& rSource )
{
    if( rSource.Source == m_xTitle )
        m_xTitle.set( nullptr );
    else if( rSource.Source == m_xSubTitle )
        m_xSubTitle.set( nullptr );
    else if( rSource.Source == m_xLegend )
        m_xLegend.set( nullptr );
    else if( rSource.Source == m_xChartData )
        m_xChartData.set( nullptr );
    else if( rSource.Source == m_xDiagram )
        m_xDiagram.set( nullptr );
    else if( rSource.Source == m_xArea )
        m_xArea.set( nullptr );
    else if( rSource.Source == m_xAddIn )
        m_xAddIn.set( nullptr );
    else if( rSource.Source == m_xChartView )
        m_xChartView.set( nullptr );
}

// WrappedPropertySet
Reference< beans::XPropertySet > ChartDocumentWrapper::getInnerPropertySet()
{
    return nullptr;
}
const Sequence< beans::Property >& ChartDocumentWrapper::getPropertySequence()
{
    return *StaticChartDocumentWrapperPropertyArray::get();
}

std::vector< std::unique_ptr<WrappedProperty> > ChartDocumentWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;
    aWrappedProperties.emplace_back( new WrappedDataSourceLabelsInFirstRowProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedDataSourceLabelsInFirstColumnProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedHasLegendProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedHasMainTitleProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedHasSubTitleProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedAddInProperty( *this ) );
    aWrappedProperties.emplace_back( new WrappedBaseDiagramProperty( *this ) );
    aWrappedProperties.emplace_back( new WrappedAdditionalShapesProperty( *this ) );
    aWrappedProperties.emplace_back( new WrappedRefreshAddInAllowedProperty( *this ) );
    aWrappedProperties.emplace_back( new WrappedIgnoreProperty("NullDate",Any() ) ); // i99104
    aWrappedProperties.emplace_back( new WrappedIgnoreProperty("EnableComplexChartTypes", uno::Any(true) ) );
    aWrappedProperties.emplace_back( new WrappedIgnoreProperty("EnableDataTableDialog", uno::Any(true) ) );

    return aWrappedProperties;
}

OUString SAL_CALL ChartDocumentWrapper::getImplementationName()
{
    return OUString(CHART_CHARTAPIWRAPPER_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL ChartDocumentWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartDocumentWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartDocument",
        CHART_CHARTAPIWRAPPER_SERVICE_NAME,
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.beans.PropertySet"
    };
}

} //  namespace wrapper
} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_ChartDocumentWrapper_get_implementation(css::uno::XComponentContext *context,
                                                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::wrapper::ChartDocumentWrapper(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
