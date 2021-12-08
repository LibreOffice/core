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

#include "Axis.hxx"
#include "GridProperties.hxx"
#include <CharacterProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <CloneHelper.hxx>
#include <AxisHelper.hxx>
#include <EventListenerHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ref.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_AXIS_SHOW,
    PROP_AXIS_CROSSOVER_POSITION,
    PROP_AXIS_CROSSOVER_VALUE,
    PROP_AXIS_DISPLAY_LABELS,
    PROP_AXIS_NUMBERFORMAT,
    PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE,
    PROP_AXIS_LABEL_POSITION,
    PROP_AXIS_TEXT_ROTATION,
    PROP_AXIS_TEXT_BREAK,
    PROP_AXIS_TEXT_OVERLAP,
    PROP_AXIS_TEXT_STACKED,
    PROP_AXIS_TEXT_ARRANGE_ORDER,
    PROP_AXIS_REFERENCE_DIAGRAM_SIZE,

    PROP_AXIS_MAJOR_TICKMARKS,
    PROP_AXIS_MINOR_TICKMARKS,
    PROP_AXIS_MARK_POSITION,

    PROP_AXIS_DISPLAY_UNITS,
    PROP_AXIS_BUILTINUNIT,

    PROP_AXIS_TRY_STAGGERING_FIRST,
    PROP_AXIS_MAJOR_ORIGIN
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Show",
                  PROP_AXIS_SHOW,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "CrossoverPosition",
                  PROP_AXIS_CROSSOVER_POSITION,
                  cppu::UnoType<css::chart::ChartAxisPosition>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "CrossoverValue",
                  PROP_AXIS_CROSSOVER_VALUE,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "DisplayLabels",
                  PROP_AXIS_DISPLAY_LABELS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_NUMFMT,
                  PROP_AXIS_NUMBERFORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_LINK_TO_SRC_NUMFMT,
                  PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "LabelPosition",
                  PROP_AXIS_LABEL_POSITION,
                  cppu::UnoType<css::chart::ChartAxisLabelPosition>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "TextRotation",
                  PROP_AXIS_TEXT_ROTATION,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "TextBreak",
                  PROP_AXIS_TEXT_BREAK,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "TextOverlap",
                  PROP_AXIS_TEXT_OVERLAP,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "StackCharacters",
                  PROP_AXIS_TEXT_STACKED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ArrangeOrder",
                  PROP_AXIS_TEXT_ARRANGE_ORDER,
                  cppu::UnoType<css::chart::ChartAxisArrangeOrderType>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ReferencePageSize",
                  PROP_AXIS_REFERENCE_DIAGRAM_SIZE,
                  cppu::UnoType<awt::Size>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "MajorTickmarks",
                  PROP_AXIS_MAJOR_TICKMARKS,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "MinorTickmarks",
                  PROP_AXIS_MINOR_TICKMARKS,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "MarkPosition",
                  PROP_AXIS_MARK_POSITION,
                  cppu::UnoType<css::chart::ChartAxisMarkPosition>::get(),
                  beans::PropertyAttribute::MAYBEDEFAULT );

    //Properties for display units:
    rOutProperties.emplace_back( "DisplayUnits",
                  PROP_AXIS_DISPLAY_UNITS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    //Properties for labels:
    rOutProperties.emplace_back( "BuiltInUnit",
                  PROP_AXIS_BUILTINUNIT,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // Compatibility option: starting from LibreOffice 5.1 the rotated
    // layout is preferred to staggering for axis labels.
    rOutProperties.emplace_back( "TryStaggeringFirst",
                  PROP_AXIS_TRY_STAGGERING_FIRST,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "MajorOrigin",
                  PROP_AXIS_MAJOR_ORIGIN,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

}

struct StaticAxisDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::CharacterProperties::AddDefaultsToMap( rOutMap );
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );

        rOutMap.setPropertyValueDefault( PROP_AXIS_SHOW, true );
        rOutMap.setPropertyValueDefault( PROP_AXIS_CROSSOVER_POSITION, css::chart::ChartAxisPosition_ZERO );
        rOutMap.setPropertyValueDefault( PROP_AXIS_DISPLAY_LABELS, true );
        rOutMap.setPropertyValueDefault( PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE, true );
        rOutMap.setPropertyValueDefault( PROP_AXIS_LABEL_POSITION, css::chart::ChartAxisLabelPosition_NEAR_AXIS );
        rOutMap.setPropertyValueDefault< double >( PROP_AXIS_TEXT_ROTATION, 0.0 );
        rOutMap.setPropertyValueDefault( PROP_AXIS_TEXT_BREAK, false );
        rOutMap.setPropertyValueDefault( PROP_AXIS_TEXT_OVERLAP, false );
        rOutMap.setPropertyValueDefault( PROP_AXIS_TEXT_STACKED, false );
        rOutMap.setPropertyValueDefault( PROP_AXIS_TEXT_ARRANGE_ORDER, css::chart::ChartAxisArrangeOrderType_AUTO );

        float fDefaultCharHeight = 10.0;
        rOutMap.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        rOutMap.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        rOutMap.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );

        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_AXIS_MAJOR_TICKMARKS, 2 /* CHAXIS_MARK_OUTER */ );
        rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_AXIS_MINOR_TICKMARKS, 0 /* CHAXIS_MARK_NONE */ );
        rOutMap.setPropertyValueDefault( PROP_AXIS_MARK_POSITION, css::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS );
        rOutMap.setPropertyValueDefault( PROP_AXIS_DISPLAY_UNITS, false );
        rOutMap.setPropertyValueDefault( PROP_AXIS_TRY_STAGGERING_FIRST, false );
    }
};

struct StaticAxisDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticAxisDefaults_Initializer >
{
};

struct StaticAxisInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    static Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticAxisInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticAxisInfoHelper_Initializer >
{
};

struct StaticAxisInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticAxisInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticAxisInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticAxisInfo_Initializer >
{
};

typedef uno::Reference< beans::XPropertySet > lcl_tSubGridType;

void lcl_CloneSubGrids(
    const uno::Sequence< lcl_tSubGridType > & rSource, uno::Sequence< lcl_tSubGridType > & rDestination )
{
    rDestination.realloc( rSource.getLength());
    lcl_tSubGridType * pDestBegin = rDestination.getArray();
    lcl_tSubGridType * pDestEnd   = pDestBegin + rDestination.getLength();
    lcl_tSubGridType * pDestIt    = pDestBegin;

    for( Reference< beans::XPropertySet > const & i : rSource )
    {
        Reference< beans::XPropertySet > xSubGrid( i );
        if( xSubGrid.is())
        {
            Reference< util::XCloneable > xCloneable( xSubGrid, uno::UNO_QUERY );
            if( xCloneable.is())
                xSubGrid.set( xCloneable->createClone(), uno::UNO_QUERY );
        }

        (*pDestIt) = xSubGrid;
        OSL_ASSERT( pDestIt != pDestEnd );
        ++pDestIt;
    }
    OSL_ASSERT( pDestIt == pDestEnd );
}

} // anonymous namespace

namespace chart
{

Axis::Axis() :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
        m_aScaleData( AxisHelper::createDefaultScale() ),
        m_xGrid( new GridProperties() )
{
    osl_atomic_increment(&m_refCount);
    setFastPropertyValue_NoBroadcast(
        ::chart::LinePropertiesHelper::PROP_LINE_COLOR, uno::Any( static_cast< sal_Int32 >( 0xb3b3b3 ) ) );  // gray30

    if( m_xGrid.is())
        ModifyListenerHelper::addListener( m_xGrid, m_xModifyEventForwarder );
    if( m_aScaleData.Categories.is())
        ModifyListenerHelper::addListener( m_aScaleData.Categories, m_xModifyEventForwarder );

    AllocateSubGrids();
    osl_atomic_decrement(&m_refCount);
}

Axis::Axis( const Axis & rOther ) :
        impl::Axis_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
    m_aScaleData( rOther.m_aScaleData )
{
    m_xGrid.set( CloneHelper::CreateRefClone< beans::XPropertySet >()( rOther.m_xGrid ));
    if( m_xGrid.is())
        ModifyListenerHelper::addListener( m_xGrid, m_xModifyEventForwarder );

    if( m_aScaleData.Categories.is())
        ModifyListenerHelper::addListener( m_aScaleData.Categories, m_xModifyEventForwarder );

    if( rOther.m_aSubGridProperties.hasElements() )
        lcl_CloneSubGrids( rOther.m_aSubGridProperties, m_aSubGridProperties );
    ModifyListenerHelper::addListenerToAllSequenceElements( m_aSubGridProperties, m_xModifyEventForwarder );

    m_xTitle.set( CloneHelper::CreateRefClone< chart2::XTitle >()( rOther.m_xTitle ));
    if( m_xTitle.is())
        ModifyListenerHelper::addListener( m_xTitle, m_xModifyEventForwarder );
}

// late initialization to call after copy-constructing
void Axis::Init()
{
    if( m_aScaleData.Categories.is())
        EventListenerHelper::addListener( m_aScaleData.Categories, this );
}

Axis::~Axis()
{
    try
    {
        ModifyListenerHelper::removeListener( m_xGrid, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListenerFromAllSequenceElements( m_aSubGridProperties, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListener( m_xTitle, m_xModifyEventForwarder );
        if( m_aScaleData.Categories.is())
        {
            ModifyListenerHelper::removeListener( m_aScaleData.Categories, m_xModifyEventForwarder );
            m_aScaleData.Categories.set(nullptr);
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    m_aSubGridProperties.realloc(0);
    m_xGrid = nullptr;
    m_xTitle = nullptr;
}

void Axis::AllocateSubGrids()
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    Reference< lang::XEventListener > xEventListener;
    std::vector< Reference< beans::XPropertySet > > aOldBroadcasters;
    std::vector< Reference< beans::XPropertySet > > aNewBroadcasters;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        xEventListener = this;

        sal_Int32 nNewSubIncCount = m_aScaleData.IncrementData.SubIncrements.getLength();
        sal_Int32 nOldSubIncCount = m_aSubGridProperties.getLength();

        if( nOldSubIncCount > nNewSubIncCount )
        {
            // remove superfluous entries
            for( sal_Int32 i = nNewSubIncCount; i < nOldSubIncCount; ++i )
                aOldBroadcasters.push_back( m_aSubGridProperties[ i ] );
            m_aSubGridProperties.realloc( nNewSubIncCount );
        }
        else if( nOldSubIncCount < nNewSubIncCount )
        {
            m_aSubGridProperties.realloc( nNewSubIncCount );
            auto pSubGridProperties = m_aSubGridProperties.getArray();

            // allocate new entries
            for( sal_Int32 i = nOldSubIncCount; i < nNewSubIncCount; ++i )
            {
                pSubGridProperties[ i ] = new GridProperties();
                LinePropertiesHelper::SetLineInvisible( m_aSubGridProperties[ i ] );
                LinePropertiesHelper::SetLineColor( m_aSubGridProperties[ i ], static_cast<sal_Int32>(0xdddddd) ); //gray2
                aNewBroadcasters.push_back( m_aSubGridProperties[ i ] );
            }
        }
    }
    //don't keep the mutex locked while calling out
    for (auto const& oldBroadcaster : aOldBroadcasters)
        ModifyListenerHelper::removeListener(oldBroadcaster, xModifyEventForwarder );
    for (auto const& newBroadcaster : aNewBroadcasters)
        ModifyListenerHelper::addListener( newBroadcaster, xModifyEventForwarder );
}

// ____ XAxis ____
void SAL_CALL Axis::setScaleData( const chart2::ScaleData& rScaleData )
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    Reference< lang::XEventListener > xEventListener;
    Reference< chart2::data::XLabeledDataSequence > xOldCategories;
    Reference< chart2::data::XLabeledDataSequence > xNewCategories = rScaleData.Categories;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        xEventListener = this;
        xOldCategories = m_aScaleData.Categories;
        m_aScaleData = rScaleData;
    }
    AllocateSubGrids();

    //don't keep the mutex locked while calling out
    if( xOldCategories.is() && xOldCategories != xNewCategories )
    {
        ModifyListenerHelper::removeListener( xOldCategories, xModifyEventForwarder );
        EventListenerHelper::removeListener( xOldCategories, xEventListener );
    }
    if( xNewCategories.is() && xOldCategories != xNewCategories )
    {
        ModifyListenerHelper::addListener( xNewCategories, m_xModifyEventForwarder );
        EventListenerHelper::addListener( xNewCategories, xEventListener );
    }
    fireModifyEvent();
}

chart2::ScaleData SAL_CALL Axis::getScaleData()
{
    MutexGuard aGuard( m_aMutex );
    return m_aScaleData;
}

Reference< beans::XPropertySet > SAL_CALL Axis::getGridProperties()
{
    MutexGuard aGuard( m_aMutex );
    return m_xGrid;
}
Sequence< Reference< beans::XPropertySet > > SAL_CALL Axis::getSubGridProperties()
{
    MutexGuard aGuard( m_aMutex );
    return m_aSubGridProperties;
}

Sequence< Reference< beans::XPropertySet > > SAL_CALL Axis::getSubTickProperties()
{
    OSL_FAIL( "Not implemented yet" );
    return Sequence< Reference< beans::XPropertySet > >();
}

// ____ XTitled ____
Reference< chart2::XTitle > SAL_CALL Axis::getTitleObject()
{
    MutexGuard aGuard( m_aMutex );
    return m_xTitle;
}

void SAL_CALL Axis::setTitleObject( const Reference< chart2::XTitle >& xNewTitle )
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    Reference< chart2::XTitle > xOldTitle;
    {
        MutexGuard aGuard( m_aMutex );
        xOldTitle = m_xTitle;
        xModifyEventForwarder = m_xModifyEventForwarder;
        m_xTitle = xNewTitle;
    }

    //don't keep the mutex locked while calling out
    if( xOldTitle.is() && xOldTitle != xNewTitle )
        ModifyListenerHelper::removeListener( xOldTitle, xModifyEventForwarder );
    if( xNewTitle.is() && xOldTitle != xNewTitle )
        ModifyListenerHelper::addListener( xNewTitle, xModifyEventForwarder );
    fireModifyEvent();
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL Axis::createClone()
{
    rtl::Reference<Axis> pNewAxis( new Axis( *this ));
    // do initialization that uses uno references to the clone
    pNewAxis->Init();
    return pNewAxis;
}

// ____ XModifyBroadcaster ____
void SAL_CALL Axis::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception &)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL Axis::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XModifyListener ____
void SAL_CALL Axis::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Axis::disposing( const lang::EventObject& Source )
{
    if( Source.Source == m_aScaleData.Categories )
        m_aScaleData.Categories = nullptr;
}

// ____ OPropertySet ____
void Axis::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void Axis::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ____ OPropertySet ____
void Axis::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticAxisDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL Axis::getInfoHelper()
{
    return *StaticAxisInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL Axis::getPropertySetInfo()
{
    return *StaticAxisInfo::get();
}

using impl::Axis_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Axis, Axis_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Axis, Axis_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL Axis::getImplementationName()
{
    return "com.sun.star.comp.chart2.Axis";
}

sal_Bool SAL_CALL Axis::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Axis::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.Axis",
        "com.sun.star.beans.PropertySet" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_Axis_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::Axis);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
