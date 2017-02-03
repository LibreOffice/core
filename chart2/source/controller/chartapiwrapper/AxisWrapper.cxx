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

#include "AxisWrapper.hxx"
#include "AxisHelper.hxx"
#include "TitleHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "macros.hxx"
#include "WrappedDirectStateProperty.hxx"
#include "GridWrapper.hxx"
#include "TitleWrapper.hxx"
#include "DisposeHelper.hxx"
#include <unonames.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>

#include "CharacterProperties.hxx"
#include "LinePropertiesHelper.hxx"
#include "UserDefinedProperties.hxx"
#include "WrappedCharacterHeightProperty.hxx"
#include "WrappedTextRotationProperty.hxx"
#include "WrappedGapwidthProperty.hxx"
#include "WrappedScaleProperty.hxx"
#include "WrappedDefaultProperty.hxx"
#include "WrappedNumberFormatProperty.hxx"
#include "WrappedScaleTextProperties.hxx"

#include <algorithm>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace
{

enum
{
    PROP_AXIS_MAX,
    PROP_AXIS_MIN,
    PROP_AXIS_STEPMAIN,
    PROP_AXIS_STEPHELP, //deprecated property use 'StepHelpCount' instead
    PROP_AXIS_STEPHELP_COUNT,
    PROP_AXIS_AUTO_MAX,
    PROP_AXIS_AUTO_MIN,
    PROP_AXIS_AUTO_STEPMAIN,
    PROP_AXIS_AUTO_STEPHELP,
    PROP_AXIS_TYPE,
    PROP_AXIS_TIME_INCREMENT,
    PROP_AXIS_EXPLICIT_TIME_INCREMENT,
    PROP_AXIS_LOGARITHMIC,
    PROP_AXIS_REVERSEDIRECTION,
    PROP_AXIS_VISIBLE,
    PROP_AXIS_CROSSOVER_POSITION,
    PROP_AXIS_CROSSOVER_VALUE,
    PROP_AXIS_ORIGIN,
    PROP_AXIS_AUTO_ORIGIN,
    PROP_AXIS_MARKS,
    PROP_AXIS_HELPMARKS,
    PROP_AXIS_MARK_POSITION,
    PROP_AXIS_DISPLAY_LABELS,
    PROP_AXIS_NUMBERFORMAT,
    PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE,
    PROP_AXIS_LABEL_POSITION,
    PROP_AXIS_TEXT_ROTATION,
    PROP_AXIS_ARRANGE_ORDER,
    PROP_AXIS_TEXTBREAK,
    PROP_AXIS_CAN_OVERLAP,
    PROP_AXIS_STACKEDTEXT,
    PROP_AXIS_OVERLAP,
    PROP_AXIS_GAP_WIDTH,
    PROP_AXIS_DISPLAY_UNITS,
    PROP_AXIS_BUILTINUNIT,
    PROP_AXIS_TRY_STAGGERING_FIRST
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    //Properties for scaling:
    rOutProperties.push_back(
        Property( "Max",
                  PROP_AXIS_MAX,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "Min",
                  PROP_AXIS_MIN,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "StepMain",
                  PROP_AXIS_STEPMAIN,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "StepHelpCount",
                  PROP_AXIS_STEPHELP_COUNT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    //deprecated property use 'StepHelpCount' instead
    rOutProperties.push_back(
        Property( "StepHelp",
                  PROP_AXIS_STEPHELP,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "AutoMax",
                  PROP_AXIS_AUTO_MAX,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "AutoMin",
                  PROP_AXIS_AUTO_MIN,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "AutoStepMain",
                  PROP_AXIS_AUTO_STEPMAIN,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "AutoStepHelp",
                  PROP_AXIS_AUTO_STEPHELP,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "AxisType",
                  PROP_AXIS_TYPE,
                  cppu::UnoType<sal_Int32>::get(), //type css::chart::ChartAxisType
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TimeIncrement",
                  PROP_AXIS_TIME_INCREMENT,
                  cppu::UnoType<css::chart::TimeIncrement>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "ExplicitTimeIncrement",
                  PROP_AXIS_EXPLICIT_TIME_INCREMENT,
                  cppu::UnoType<css::chart::TimeIncrement>::get(),
                  beans::PropertyAttribute::READONLY |
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "Logarithmic",
                  PROP_AXIS_LOGARITHMIC,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ReverseDirection",
                  PROP_AXIS_REVERSEDIRECTION,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    //todo: this property is missing in the API
    rOutProperties.push_back(
        Property( "Visible",
                  PROP_AXIS_VISIBLE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "CrossoverPosition",
                  PROP_AXIS_CROSSOVER_POSITION,
                  cppu::UnoType<css::chart::ChartAxisPosition>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "CrossoverValue",
                  PROP_AXIS_CROSSOVER_VALUE,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "Origin",
                  PROP_AXIS_ORIGIN,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "AutoOrigin",
                  PROP_AXIS_AUTO_ORIGIN,
                  cppu::UnoType<bool>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    //Properties for interval marks:
    rOutProperties.push_back(
        Property( "Marks",
                  PROP_AXIS_MARKS,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "HelpMarks",
                  PROP_AXIS_HELPMARKS,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "MarkPosition",
                  PROP_AXIS_MARK_POSITION,
                  cppu::UnoType<css::chart::ChartAxisMarkPosition>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    //Properties for labels:
    rOutProperties.push_back(
        Property( "DisplayLabels",
                  PROP_AXIS_DISPLAY_LABELS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_NUMFMT,
                  PROP_AXIS_NUMBERFORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_LINK_TO_SRC_NUMFMT,
                  PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "LabelPosition",
                  PROP_AXIS_LABEL_POSITION,
                  cppu::UnoType<css::chart::ChartAxisLabelPosition>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TextRotation",
                  PROP_AXIS_TEXT_ROTATION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ArrangeOrder",
                  PROP_AXIS_ARRANGE_ORDER,
                  cppu::UnoType<css::chart::ChartAxisArrangeOrderType>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TextBreak",
                  PROP_AXIS_TEXTBREAK,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TextCanOverlap",
                  PROP_AXIS_CAN_OVERLAP,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "StackedText",
                  PROP_AXIS_STACKEDTEXT,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Properties related to bar charts:
    rOutProperties.push_back(
        Property( "Overlap",
                  PROP_AXIS_OVERLAP,
                  cppu::UnoType<sal_Int32>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "GapWidth",
                  PROP_AXIS_GAP_WIDTH,
                  cppu::UnoType<sal_Int32>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    //Properties for display units:
    rOutProperties.push_back(
        Property( "DisplayUnits",
                  PROP_AXIS_DISPLAY_UNITS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //Properties for labels:
    rOutProperties.push_back(
        Property( "BuiltInUnit",
                  PROP_AXIS_BUILTINUNIT,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Compatibility option: starting from LibreOffice 5.1 the rotated
    // layout is preferred to staggering for axis labels.
    rOutProperties.push_back(
        Property( "TryStaggeringFirst",
                  PROP_AXIS_TRY_STAGGERING_FIRST,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticAxisWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    static Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
        ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticAxisWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticAxisWrapperPropertyArray_Initializer >
{
};

} // anonymous namespace

namespace chart
{
namespace wrapper
{

AxisWrapper::AxisWrapper(
    tAxisType eType, const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex ),
        m_eType( eType )
{
}

AxisWrapper::~AxisWrapper()
{
}

// ____ chart::XAxis ____
Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getAxisTitle()
{
    if( !m_xAxisTitle.is() )
    {
        TitleHelper::eTitleType eTitleType( TitleHelper::X_AXIS_TITLE    );
        switch( m_eType )
        {
            case X_AXIS:
                eTitleType = TitleHelper::X_AXIS_TITLE;
                break;
            case Y_AXIS:
                eTitleType = TitleHelper::Y_AXIS_TITLE;
                break;
            case Z_AXIS:
                eTitleType = TitleHelper::Z_AXIS_TITLE;
                break;
            case SECOND_X_AXIS:
                eTitleType = TitleHelper::SECONDARY_X_AXIS_TITLE;
                break;
            case SECOND_Y_AXIS:
                eTitleType = TitleHelper::SECONDARY_Y_AXIS_TITLE;
                break;
            default:
                return nullptr;
        }
        m_xAxisTitle = new TitleWrapper( eTitleType, m_spChart2ModelContact );
    }
    return m_xAxisTitle;
}
Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getMajorGrid()
{
    if( !m_xMajorGrid.is() )
    {
        GridWrapper::tGridType eGridType( GridWrapper::X_MAJOR_GRID );
        switch( m_eType )
        {
            case X_AXIS:
                eGridType = GridWrapper::X_MAJOR_GRID;
                break;
            case Y_AXIS:
                eGridType = GridWrapper::Y_MAJOR_GRID;
                break;
            case Z_AXIS:
                eGridType = GridWrapper::Z_MAJOR_GRID;
                break;
            default:
                return nullptr;
        }
        m_xMajorGrid = new GridWrapper( eGridType, m_spChart2ModelContact );
    }
    return m_xMajorGrid;
}
Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getMinorGrid()
{
    if( !m_xMinorGrid.is() )
    {
        GridWrapper::tGridType eGridType( GridWrapper::X_MAJOR_GRID );
        switch( m_eType )
        {
            case X_AXIS:
                eGridType = GridWrapper::X_MINOR_GRID;
                break;
            case Y_AXIS:
                eGridType = GridWrapper::Y_MINOR_GRID;
                break;
            case Z_AXIS:
                eGridType = GridWrapper::Z_MINOR_GRID;
                break;
            default:
                return nullptr;
        }
        m_xMinorGrid = new GridWrapper( eGridType, m_spChart2ModelContact );
    }
    return m_xMinorGrid;
}

// ____ XShape ____
awt::Point SAL_CALL AxisWrapper::getPosition()
{
    awt::Point aResult( m_spChart2ModelContact->GetAxisPosition( this->getAxis() ) );
    return aResult;
}

void SAL_CALL AxisWrapper::setPosition( const awt::Point& /*aPosition*/ )
{
    OSL_FAIL( "trying to set position of Axis" );
}

awt::Size SAL_CALL AxisWrapper::getSize()
{
    awt::Size aSize( m_spChart2ModelContact->GetAxisSize( this->getAxis() ) );
    return aSize;
}

void SAL_CALL AxisWrapper::setSize( const awt::Size& /*aSize*/ )
{
    OSL_FAIL( "trying to set size of Axis" );
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL AxisWrapper::getShapeType()
{
    return OUString("com.sun.star.chart.ChartAxis");
}

// ____ XNumberFormatsSupplier ____
uno::Reference< beans::XPropertySet > SAL_CALL AxisWrapper::getNumberFormatSettings()
{
    Reference< util::XNumberFormatsSupplier > xNumSuppl( m_spChart2ModelContact->getChartModel(), uno::UNO_QUERY );
    if( xNumSuppl.is() )
        return xNumSuppl->getNumberFormatSettings();

    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< util::XNumberFormats > SAL_CALL AxisWrapper::getNumberFormats()
{
    Reference< util::XNumberFormatsSupplier > xNumSuppl( m_spChart2ModelContact->getChartModel(), uno::UNO_QUERY );
    if( xNumSuppl.is() )
        return xNumSuppl->getNumberFormats();

    return uno::Reference< util::XNumberFormats >();
}

void AxisWrapper::getDimensionAndMainAxisBool( tAxisType eType, sal_Int32& rnDimensionIndex, bool& rbMainAxis )
{
    switch( eType )
    {
        case X_AXIS:
            rnDimensionIndex = 0; rbMainAxis = true; break;
        case Y_AXIS:
            rnDimensionIndex = 1; rbMainAxis = true; break;
        case Z_AXIS:
            rnDimensionIndex = 2; rbMainAxis = true; break;
        case SECOND_X_AXIS:
            rnDimensionIndex = 0; rbMainAxis = false; break;
        case SECOND_Y_AXIS:
            rnDimensionIndex = 1; rbMainAxis = false; break;
    }
}

// ____ XComponent ____
void SAL_CALL AxisWrapper::dispose()
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    DisposeHelper::DisposeAndClear( m_xAxisTitle );
    DisposeHelper::DisposeAndClear( m_xMajorGrid );
    DisposeHelper::DisposeAndClear( m_xMinorGrid );

    clearWrappedPropertySet();
}

void SAL_CALL AxisWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL AxisWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    m_aEventListenerContainer.removeInterface( aListener );
}

//ReferenceSizePropertyProvider
void AxisWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( this->getAxis(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue("ReferencePageSize").hasValue() )
            xProp->setPropertyValue("ReferencePageSize", uno::Any(
            m_spChart2ModelContact->GetPageSize() ));
    }
}
Any AxisWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( this->getAxis(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue("ReferencePageSize");
    return aRet;
}
awt::Size AxisWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

Reference< chart2::XAxis > AxisWrapper::getAxis()
{
    Reference< chart2::XAxis > xAxis;
    try
    {
        sal_Int32 nDimensionIndex = 0;
        bool  bMainAxis = true;
        AxisWrapper::getDimensionAndMainAxisBool( m_eType, nDimensionIndex, bMainAxis );

        Reference< XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        xAxis = AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram );
        if( !xAxis.is() )
        {
            xAxis = AxisHelper::createAxis( nDimensionIndex, bMainAxis, xDiagram, m_spChart2ModelContact->m_xContext );
            Reference< beans::XPropertySet > xProp( xAxis, uno::UNO_QUERY );
            if( xProp.is() )
                xProp->setPropertyValue("Show", uno::Any( false ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xAxis;
}

// WrappedPropertySet
Reference< beans::XPropertySet > AxisWrapper::getInnerPropertySet()
{
    return Reference< beans::XPropertySet >( this->getAxis(), uno::UNO_QUERY );
}

const Sequence< beans::Property >& AxisWrapper::getPropertySequence()
{
    return *StaticAxisWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > AxisWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedTextRotationProperty() );
    aWrappedProperties.push_back( new WrappedProperty("Marks","MajorTickmarks") );
    aWrappedProperties.push_back( new WrappedProperty("HelpMarks","MinorTickmarks") );
    aWrappedProperties.push_back( new WrappedProperty("TextCanOverlap","TextOverlap") );
    aWrappedProperties.push_back( new WrappedProperty("ArrangeOrder","ArrangeOrder") );
    aWrappedProperties.push_back( new WrappedProperty("Visible","Show") );
    aWrappedProperties.push_back( new WrappedDirectStateProperty("DisplayLabels","DisplayLabels") );
    aWrappedProperties.push_back( new WrappedDirectStateProperty("TryStaggeringFirst","TryStaggeringFirst") );
    aWrappedProperties.push_back( new WrappedDirectStateProperty("TextBreak","TextBreak") );
    aWrappedProperties.push_back( new WrappedNumberFormatProperty(m_spChart2ModelContact) );
    aWrappedProperties.push_back( new WrappedLinkNumberFormatProperty );
    aWrappedProperties.push_back( new WrappedProperty("StackedText","StackCharacters") );
    aWrappedProperties.push_back( new WrappedDirectStateProperty("CrossoverPosition","CrossoverPosition") );
    {
        WrappedGapwidthProperty* pWrappedGapwidthProperty( new WrappedGapwidthProperty( m_spChart2ModelContact ) );
        WrappedBarOverlapProperty* pWrappedBarOverlapProperty( new WrappedBarOverlapProperty( m_spChart2ModelContact ) );
        sal_Int32 nDimensionIndex = 0;
        bool  bMainAxis = true;
        sal_Int32 nAxisIndex = 0;
        AxisWrapper::getDimensionAndMainAxisBool( m_eType, nDimensionIndex, bMainAxis );
        if( !bMainAxis )
            nAxisIndex = 1;
        pWrappedGapwidthProperty->setDimensionAndAxisIndex( nDimensionIndex, nAxisIndex );
        pWrappedBarOverlapProperty->setDimensionAndAxisIndex( nDimensionIndex, nAxisIndex );
        aWrappedProperties.push_back( pWrappedGapwidthProperty );
        aWrappedProperties.push_back( pWrappedBarOverlapProperty );
    }

    WrappedScaleProperty::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

OUString SAL_CALL AxisWrapper::getImplementationName()
{
    return OUString("com.sun.star.comp.chart.Axis");
}

sal_Bool SAL_CALL AxisWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL AxisWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartAxis",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.style.CharacterProperties"
    };
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
