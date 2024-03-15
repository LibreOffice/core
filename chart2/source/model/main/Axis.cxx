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

#include <Axis.hxx>
#include <GridProperties.hxx>
#include <CharacterProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <AxisHelper.hxx>
#include <EventListenerHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <Title.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
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
} // namespace

namespace chart
{
const ::chart::tPropertyValueMap &  StaticAxisDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults = []()
        {
            ::chart::tPropertyValueMap aMap;
            ::chart::CharacterProperties::AddDefaultsToMap( aMap );
            ::chart::LinePropertiesHelper::AddDefaultsToMap( aMap );

            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_SHOW, true );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_CROSSOVER_POSITION, css::chart::ChartAxisPosition_ZERO );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_DISPLAY_LABELS, true );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_LINK_NUMBERFORMAT_TO_SOURCE, true );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_LABEL_POSITION, css::chart::ChartAxisLabelPosition_NEAR_AXIS );
            ::chart::PropertyHelper::setPropertyValueDefault< double >( aMap, PROP_AXIS_TEXT_ROTATION, 0.0 );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_TEXT_BREAK, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_TEXT_OVERLAP, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_TEXT_STACKED, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_TEXT_ARRANGE_ORDER, css::chart::ChartAxisArrangeOrderType_AUTO );

            float fDefaultCharHeight = 10.0;
            ::chart::PropertyHelper::setPropertyValue( aMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
            ::chart::PropertyHelper::setPropertyValue( aMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
            ::chart::PropertyHelper::setPropertyValue( aMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );

            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aMap, PROP_AXIS_MAJOR_TICKMARKS, 2 /* CHAXIS_MARK_OUTER */ );
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aMap, PROP_AXIS_MINOR_TICKMARKS, 0 /* CHAXIS_MARK_NONE */ );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_MARK_POSITION, css::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_DISPLAY_UNITS, false );
            ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_AXIS_TRY_STAGGERING_FIRST, false );
            return aMap;
        }();
    return aStaticDefaults;
};
} // namespace chart

namespace
{
::cppu::OPropertyArrayHelper& StaticAxisInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            lcl_AddPropertiesToVector( aProperties );
            ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

std::vector< rtl::Reference< ::chart::GridProperties > > lcl_CloneSubGrids(
    const std::vector< rtl::Reference< ::chart::GridProperties > > & rSource )
{
    std::vector< rtl::Reference< ::chart::GridProperties > > aDestination;
    aDestination.reserve( rSource.size());
    for( rtl::Reference< ::chart::GridProperties > const & i : rSource )
    {
        aDestination.push_back(new ::chart::GridProperties(*i));
    }
    return aDestination;
}

} // anonymous namespace

namespace chart
{

Axis::Axis() :
        m_xModifyEventForwarder( new ModifyEventForwarder() ),
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
        ::property::OPropertySet( rOther ),
    m_xModifyEventForwarder( new ModifyEventForwarder() ),
    m_aScaleData( rOther.m_aScaleData )
{
    if (rOther.m_xGrid)
        m_xGrid = new ::chart::GridProperties(*rOther.m_xGrid);
    if( m_xGrid.is())
        ModifyListenerHelper::addListener( m_xGrid, m_xModifyEventForwarder );

    if( m_aScaleData.Categories.is())
        ModifyListenerHelper::addListener( m_aScaleData.Categories, m_xModifyEventForwarder );

    if( !rOther.m_aSubGridProperties.empty() )
        m_aSubGridProperties = lcl_CloneSubGrids( rOther.m_aSubGridProperties );
    ModifyListenerHelper::addListenerToAllElements( m_aSubGridProperties, m_xModifyEventForwarder );

    if ( rOther.m_xTitle )
        m_xTitle = new Title( *rOther.m_xTitle );
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
        ModifyListenerHelper::removeListenerFromAllElements( m_aSubGridProperties, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListener( m_xTitle, m_xModifyEventForwarder );
        if( m_aScaleData.Categories.is())
        {
            ModifyListenerHelper::removeListener( m_aScaleData.Categories, m_xModifyEventForwarder );
            m_aScaleData.Categories.clear();
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    m_aSubGridProperties.clear();
    m_xGrid = nullptr;
    m_xTitle = nullptr;
}

void Axis::AllocateSubGrids()
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    Reference< lang::XEventListener > xEventListener;
    std::vector< rtl::Reference< GridProperties > > aOldBroadcasters;
    std::vector< rtl::Reference< GridProperties > > aNewBroadcasters;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        xEventListener = this;

        sal_Int32 nNewSubIncCount = m_aScaleData.IncrementData.SubIncrements.getLength();
        sal_Int32 nOldSubIncCount = m_aSubGridProperties.size();

        if( nOldSubIncCount > nNewSubIncCount )
        {
            // remove superfluous entries
            for( sal_Int32 i = nNewSubIncCount; i < nOldSubIncCount; ++i )
                aOldBroadcasters.push_back( m_aSubGridProperties[ i ] );
            m_aSubGridProperties.resize( nNewSubIncCount );
        }
        else if( nOldSubIncCount < nNewSubIncCount )
        {
            m_aSubGridProperties.resize( nNewSubIncCount );

            // allocate new entries
            for( sal_Int32 i = nOldSubIncCount; i < nNewSubIncCount; ++i )
            {
                m_aSubGridProperties[ i ] = new GridProperties();
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
rtl::Reference< ::chart::GridProperties > Axis::getGridProperties2()
{
    MutexGuard aGuard( m_aMutex );
    return m_xGrid;
}
Sequence< Reference< beans::XPropertySet > > SAL_CALL Axis::getSubGridProperties()
{
    MutexGuard aGuard( m_aMutex );
    return comphelper::containerToSequence<Reference< beans::XPropertySet >>(m_aSubGridProperties);
}

std::vector< rtl::Reference< GridProperties > > Axis::getSubGridProperties2()
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

rtl::Reference< Title > Axis::getTitleObject2() const
{
    MutexGuard aGuard( m_aMutex );
    return m_xTitle;
}

void SAL_CALL Axis::setTitleObject( const Reference< chart2::XTitle >& xNewTitle )
{
    rtl::Reference<Title> xTitle = dynamic_cast<Title*>(xNewTitle.get());
    assert(!xNewTitle || xTitle);
    setTitleObject(xTitle);
}

void Axis::setTitleObject( const rtl::Reference< Title >& xNewTitle )
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    rtl::Reference< Title > xOldTitle;
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
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL Axis::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
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
    const tPropertyValueMap& rStaticDefaults = StaticAxisDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL Axis::getInfoHelper()
{
    return StaticAxisInfoHelper();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL Axis::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticAxisInfoHelper() ) );
    return xPropertySetInfo;
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
