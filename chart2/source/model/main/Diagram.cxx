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

#include <Diagram.hxx>
#include <AxisHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartTypeManager.hxx>
#include <ChartTypeTemplate.hxx>
#include <ChartType.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <RegressionCurveHelper.hxx>
#include <RegressionCurveModel.hxx>
#include "Wall.hxx"
#include <ModifyListenerHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <ConfigColorScheme.hxx>
#include <DiagramHelper.hxx>
#include <ThreeDHelper.hxx>
#include <CloneHelper.hxx>
#include <SceneProperties.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>
#include <Legend.hxx>
#include <Axis.hxx>
#include <DataTable.hxx>
#include <servicenames_charttypes.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/math.hxx>

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_DIAGRAM_REL_POS,
    PROP_DIAGRAM_REL_SIZE,
    PROP_DIAGRAM_POSSIZE_EXCLUDE_LABELS,
    PROP_DIAGRAM_SORT_BY_X_VALUES,
    PROP_DIAGRAM_CONNECT_BARS,
    PROP_DIAGRAM_GROUP_BARS_PER_AXIS,
    PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS,
    PROP_DIAGRAM_STARTING_ANGLE,
    PROP_DIAGRAM_RIGHT_ANGLED_AXES,
    PROP_DIAGRAM_PERSPECTIVE,
    PROP_DIAGRAM_ROTATION_HORIZONTAL,
    PROP_DIAGRAM_ROTATION_VERTICAL,
    PROP_DIAGRAM_MISSING_VALUE_TREATMENT,
    PROP_DIAGRAM_3DRELATIVEHEIGHT,
    PROP_DIAGRAM_DATATABLEHBORDER,
    PROP_DIAGRAM_DATATABLEVBORDER,
    PROP_DIAGRAM_DATATABLEOUTLINE,
    PROP_DIAGRAM_EXTERNALDATA
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "RelativePosition",
                  PROP_DIAGRAM_REL_POS,
                  cppu::UnoType<chart2::RelativePosition>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RelativeSize",
                  PROP_DIAGRAM_REL_SIZE,
                  cppu::UnoType<chart2::RelativeSize>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "PosSizeExcludeAxes",
                  PROP_DIAGRAM_POSSIZE_EXCLUDE_LABELS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( CHART_UNONAME_SORT_BY_XVALUES,
                  PROP_DIAGRAM_SORT_BY_X_VALUES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ConnectBars",
                  PROP_DIAGRAM_CONNECT_BARS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "GroupBarsPerAxis",
                  PROP_DIAGRAM_GROUP_BARS_PER_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "IncludeHiddenCells",
                  PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "StartingAngle",
                  PROP_DIAGRAM_STARTING_ANGLE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "RightAngledAxes",
                  PROP_DIAGRAM_RIGHT_ANGLED_AXES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Perspective",
                  PROP_DIAGRAM_PERSPECTIVE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RotationHorizontal",
                  PROP_DIAGRAM_ROTATION_HORIZONTAL,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RotationVertical",
                  PROP_DIAGRAM_ROTATION_VERTICAL,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "MissingValueTreatment",
                  PROP_DIAGRAM_MISSING_VALUE_TREATMENT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "3DRelativeHeight",
                  PROP_DIAGRAM_3DRELATIVEHEIGHT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "ExternalData",
                  PROP_DIAGRAM_EXTERNALDATA,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::MAYBEVOID );
}

const ::chart::tPropertyValueMap& StaticDiagramDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults = []()
    {
        ::chart::tPropertyValueMap aMap;
        ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_DIAGRAM_POSSIZE_EXCLUDE_LABELS, true );
        ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_DIAGRAM_SORT_BY_X_VALUES, false );
        ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_DIAGRAM_CONNECT_BARS, false );
        ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_DIAGRAM_GROUP_BARS_PER_AXIS, true );
        ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS, true );
        ::chart::PropertyHelper::setPropertyValueDefault( aMap, PROP_DIAGRAM_RIGHT_ANGLED_AXES, false );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aMap, PROP_DIAGRAM_STARTING_ANGLE, 90 );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aMap, PROP_DIAGRAM_3DRELATIVEHEIGHT, 100 );
        ::chart::SceneProperties::AddDefaultsToMap( aMap );
        return aMap;
    }();
    return aStaticDefaults;
};

::cppu::OPropertyArrayHelper& StaticDiagramInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::SceneProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::cppu::OPropertyArrayHelper( aProperties.data(), aProperties.size() );
    }();
    return aPropHelper;
};

const uno::Reference< beans::XPropertySetInfo >& StaticDiagramInfo()
{
    static const uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticDiagramInfoHelper() ) );
    return xPropertySetInfo;
};

void lcl_CloneCoordinateSystems(
        const ::chart::Diagram::tCoordinateSystemContainerType & rSource,
        ::chart::Diagram::tCoordinateSystemContainerType & rDestination )
{
    for( rtl::Reference< ::chart::BaseCoordinateSystem > const & i : rSource )
    {
        auto xClone = i->createClone();
        ::chart::BaseCoordinateSystem* pClone = dynamic_cast<::chart::BaseCoordinateSystem*>(xClone.get());
        assert(pClone);
        rDestination.push_back( pClone );
    }
}

} // anonymous namespace

namespace chart
{

Diagram::Diagram( uno::Reference< uno::XComponentContext > xContext ) :
        m_xContext(std::move( xContext )),
        m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    // Set camera position to a default position (that should be set hard, so
    // that it will be exported.  The property default is a camera looking
    // straight ono the scene).  These defaults have been acquired from the old
    // chart implementation.
    setFastPropertyValue_NoBroadcast(
        SceneProperties::PROP_SCENE_CAMERA_GEOMETRY, uno::Any(
            ThreeDHelper::getDefaultCameraGeometry()));
}

Diagram::Diagram( const Diagram & rOther ) :
        impl::Diagram_Base(rOther),
        ::property::OPropertySet( rOther ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    lcl_CloneCoordinateSystems( rOther.m_aCoordSystems, m_aCoordSystems );
    for (auto & xSystem : m_aCoordSystems)
        xSystem->addModifyListener(m_xModifyEventForwarder);

    if ( rOther.m_xWall )
        m_xWall = new Wall( *rOther.m_xWall );
    if ( rOther.m_xFloor )
        m_xFloor = new Wall( *rOther.m_xFloor );
    m_xTitle.set( CloneHelper::CreateRefClone< chart2::XTitle >()( rOther.m_xTitle ));
    if (rOther.m_xLegend)
        m_xLegend = new Legend(*rOther.m_xLegend);
    if (rOther.m_xDataTable)
        m_xDataTable = new DataTable(*rOther.m_xDataTable);

    if ( m_xWall )
        m_xWall->addModifyListener( m_xModifyEventForwarder );
    if ( m_xFloor )
        m_xFloor->addModifyListener( m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xTitle, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xLegend, m_xModifyEventForwarder );
}

Diagram::~Diagram()
{
    try
    {
        for (auto & xSystem : m_aCoordSystems)
            xSystem->removeModifyListener(m_xModifyEventForwarder);

        if ( m_xWall )
            m_xWall->removeModifyListener( m_xModifyEventForwarder );
        if ( m_xFloor )
            m_xFloor->removeModifyListener( m_xModifyEventForwarder );
        ModifyListenerHelper::removeListener( m_xTitle, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListener( m_xLegend, m_xModifyEventForwarder );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XDiagram ____
uno::Reference< beans::XPropertySet > SAL_CALL Diagram::getWall()
{
    rtl::Reference< Wall > xRet;
    bool bAddListener = false;
    {
        MutexGuard aGuard( m_aMutex );
        if( !m_xWall.is() )
        {
            m_xWall.set( new Wall() );
            bAddListener = true;
        }
        xRet =  m_xWall;
    }
    if(bAddListener)
        xRet->addModifyListener( m_xModifyEventForwarder );
    return xRet;
}

uno::Reference< beans::XPropertySet > SAL_CALL Diagram::getFloor()
{
    rtl::Reference< Wall > xRet;
    bool bAddListener = false;
    {
        MutexGuard aGuard( m_aMutex );
        if( !m_xFloor.is() )
        {
            m_xFloor.set( new Wall() );
            bAddListener = true;
        }
        xRet = m_xFloor;
    }
    if(bAddListener)
        xRet->addModifyListener( m_xModifyEventForwarder );
    return xRet;
}

uno::Reference< chart2::XLegend > SAL_CALL Diagram::getLegend()
{
    MutexGuard aGuard( m_aMutex );
    return m_xLegend;
}

void SAL_CALL Diagram::setLegend( const uno::Reference< chart2::XLegend >& xNewLegend )
{
    auto pLegend = dynamic_cast<Legend*>(xNewLegend.get());
    assert(!xNewLegend || pLegend);
    setLegend(rtl::Reference< Legend >(pLegend));
}

void Diagram::setLegend( const rtl::Reference< Legend >& xNewLegend )
{
    rtl::Reference< Legend > xOldLegend;
    {
        MutexGuard aGuard( m_aMutex );
        if( m_xLegend == xNewLegend )
            return;
        xOldLegend = m_xLegend;
        m_xLegend = xNewLegend;
    }
    if( xOldLegend.is())
        ModifyListenerHelper::removeListener( xOldLegend, m_xModifyEventForwarder );
    if( xNewLegend.is())
        ModifyListenerHelper::addListener( xNewLegend, m_xModifyEventForwarder );
    fireModifyEvent();
}

Reference< chart2::XColorScheme > SAL_CALL Diagram::getDefaultColorScheme()
{
    Reference< chart2::XColorScheme > xRet;
    {
        MutexGuard aGuard( m_aMutex );
        xRet = m_xColorScheme;
    }

    if( !xRet.is())
    {
        xRet.set( createConfigColorScheme( m_xContext ));
        MutexGuard aGuard( m_aMutex );
        m_xColorScheme = xRet;
    }
    return xRet;
}

void SAL_CALL Diagram::setDefaultColorScheme( const Reference< chart2::XColorScheme >& xColorScheme )
{
    {
        MutexGuard aGuard( m_aMutex );
        m_xColorScheme.set( xColorScheme );
    }
    fireModifyEvent();
}

void SAL_CALL Diagram::setDiagramData(
    const Reference< chart2::data::XDataSource >& xDataSource,
    const Sequence< beans::PropertyValue >& aArguments )
{
    rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = new ::chart::ChartTypeManager( m_xContext );
    Diagram::tTemplateWithServiceName aTemplateAndService = getTemplate( xChartTypeManager );
    rtl::Reference< ::chart::ChartTypeTemplate > xTemplate( aTemplateAndService.xChartTypeTemplate );
    if( !xTemplate.is() )
        xTemplate = xChartTypeManager->createTemplate( "com.sun.star.chart2.template.Column" );
    if(!xTemplate.is())
        return;
    xTemplate->changeDiagramData( rtl::Reference< ::chart::Diagram >(this), xDataSource, aArguments );
}

// ____ XTitled ____
uno::Reference< chart2::XTitle > SAL_CALL Diagram::getTitleObject()
{
    MutexGuard aGuard( m_aMutex );
    return m_xTitle;
}

void SAL_CALL Diagram::setTitleObject( const uno::Reference< chart2::XTitle >& xNewTitle )
{
    Reference< chart2::XTitle > xOldTitle;
    {
        MutexGuard aGuard( m_aMutex );
        if( m_xTitle == xNewTitle )
            return;
        xOldTitle = m_xTitle;
        m_xTitle = xNewTitle;
    }
    if( xOldTitle.is())
        ModifyListenerHelper::removeListener( xOldTitle, m_xModifyEventForwarder );
    if( xNewTitle.is())
        ModifyListenerHelper::addListener( xNewTitle, m_xModifyEventForwarder );
    fireModifyEvent();
}

// ____ X3DDefaultSetter ____
void SAL_CALL Diagram::set3DSettingsToDefault()
{
    ThreeDHelper::set3DSettingsToDefault( this );
}

void SAL_CALL Diagram::setDefaultRotation()
{
    ThreeDHelper::setDefaultRotation( this );
}

void SAL_CALL Diagram::setDefaultIllumination()
{
    ThreeDHelper::setDefaultIllumination( this );
}

// ____ XCoordinateSystemContainer ____
void SAL_CALL Diagram::addCoordinateSystem(
    const uno::Reference< chart2::XCoordinateSystem >& aCoordSys )
{
    ::chart::BaseCoordinateSystem* pCoordSys = dynamic_cast<::chart::BaseCoordinateSystem*>(aCoordSys.get());
    assert(pCoordSys);
    {
        MutexGuard aGuard( m_aMutex );
        if( std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), pCoordSys )
            != m_aCoordSystems.end())
            throw lang::IllegalArgumentException("coordsys not found", static_cast<cppu::OWeakObject*>(this), 1);

        if( !m_aCoordSystems.empty() )
        {
            OSL_FAIL( "more than one coordinatesystem is not supported yet by the fileformat" );
            return;
        }
        m_aCoordSystems.push_back( pCoordSys );
    }
    ModifyListenerHelper::addListener( aCoordSys, m_xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL Diagram::removeCoordinateSystem(
    const uno::Reference< chart2::XCoordinateSystem >& aCoordSys )
{
    ::chart::BaseCoordinateSystem* pCoordSys = dynamic_cast<::chart::BaseCoordinateSystem*>(aCoordSys.get());
    assert(pCoordSys);
    {
        MutexGuard aGuard( m_aMutex );
        auto aIt =  std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), pCoordSys );
        if( aIt == m_aCoordSystems.end())
            throw container::NoSuchElementException(
                "The given coordinate-system is no element of the container",
                static_cast< uno::XWeak * >( this ));
        m_aCoordSystems.erase( aIt );
    }
    ModifyListenerHelper::removeListener( aCoordSys, m_xModifyEventForwarder );
    fireModifyEvent();
}

uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > SAL_CALL Diagram::getCoordinateSystems()
{
    MutexGuard aGuard( m_aMutex );
    return comphelper::containerToSequence<uno::Reference< chart2::XCoordinateSystem >>( m_aCoordSystems );
}

void SAL_CALL Diagram::setCoordinateSystems(
    const Sequence< Reference< chart2::XCoordinateSystem > >& aCoordinateSystems )
{
    tCoordinateSystemContainerType aNew;
    tCoordinateSystemContainerType aOld;
    if( aCoordinateSystems.hasElements() )
    {
        OSL_ENSURE( aCoordinateSystems.getLength()<=1, "more than one coordinatesystem is not supported yet by the fileformat" );
        ::chart::BaseCoordinateSystem* pCoordSys = dynamic_cast<::chart::BaseCoordinateSystem*>(aCoordinateSystems[0].get());
        assert(pCoordSys);
        aNew.push_back( pCoordSys );
    }
    {
        MutexGuard aGuard( m_aMutex );
        std::swap( aOld, m_aCoordSystems );
        m_aCoordSystems = aNew;
    }
    for (auto & xSystem : aOld)
        xSystem->removeModifyListener(m_xModifyEventForwarder);
    for (auto & xSystem : aNew)
        xSystem->addModifyListener(m_xModifyEventForwarder);
    fireModifyEvent();
}

void Diagram::setCoordinateSystems(
    const std::vector< rtl::Reference< BaseCoordinateSystem > >& aCoordinateSystems )
{
    tCoordinateSystemContainerType aNew;
    tCoordinateSystemContainerType aOld;
    if( !aCoordinateSystems.empty() )
    {
        OSL_ENSURE( aCoordinateSystems.size()<=1, "more than one coordinatesystem is not supported yet by the fileformat" );
        aNew.push_back( aCoordinateSystems[0] );
    }
    {
        MutexGuard aGuard( m_aMutex );
        std::swap( aOld, m_aCoordSystems );
        m_aCoordSystems = aNew;
    }
    for (auto & xSystem : aOld)
        xSystem->removeModifyListener(m_xModifyEventForwarder);
    for (auto & xSystem : aNew)
        xSystem->addModifyListener(m_xModifyEventForwarder);
    fireModifyEvent();
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL Diagram::createClone()
{
    MutexGuard aGuard( m_aMutex );
    return Reference< util::XCloneable >( new Diagram( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL Diagram::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL Diagram::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL Diagram::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Diagram::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void Diagram::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void Diagram::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ____ OPropertySet ____
void Diagram::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticDiagramDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL Diagram::getInfoHelper()
{
    return StaticDiagramInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL Diagram::getPropertySetInfo()
{
    return StaticDiagramInfo();
}

// ____ XFastPropertySet ____
void SAL_CALL Diagram::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
{
    //special treatment for some 3D properties
    if( nHandle == PROP_DIAGRAM_PERSPECTIVE )
    {
        sal_Int32 fPerspective = 20;
        if( rValue >>=fPerspective )
            ThreeDHelper::setCameraDistance( this, ThreeDHelper::PerspectiveToCameraDistance( fPerspective ) );
    }
    else if( nHandle == PROP_DIAGRAM_ROTATION_HORIZONTAL
        || nHandle == PROP_DIAGRAM_ROTATION_VERTICAL )
    {
        sal_Int32 nNewAngleDegree = 0;
        if( rValue >>=nNewAngleDegree )
        {
            sal_Int32 nHorizontal, nVertical;
            ThreeDHelper::getRotationFromDiagram( this, nHorizontal, nVertical );
            if( nHandle == PROP_DIAGRAM_ROTATION_HORIZONTAL )
                nHorizontal = nNewAngleDegree;
            else
                nVertical = nNewAngleDegree;
            ThreeDHelper::setRotationToDiagram( this, nHorizontal, nVertical );
        }
    }
    else
        ::property::OPropertySet::setFastPropertyValue( nHandle, rValue );
}

void SAL_CALL Diagram::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    //special treatment for some 3D properties
    if( nHandle == PROP_DIAGRAM_PERSPECTIVE )
    {
        sal_Int32 nPerspective = ::basegfx::fround( ThreeDHelper::CameraDistanceToPerspective(
            ThreeDHelper::getCameraDistance( const_cast< Diagram* >( this ) ) ) );
        rValue <<= nPerspective;
    }
    else if( nHandle == PROP_DIAGRAM_ROTATION_HORIZONTAL
        || nHandle == PROP_DIAGRAM_ROTATION_VERTICAL )
    {
        sal_Int32 nHorizontal, nVertical;
        ThreeDHelper::getRotationFromDiagram( const_cast< Diagram* >( this ), nHorizontal, nVertical );
        sal_Int32 nAngleDegree = 0;
        if( nHandle == PROP_DIAGRAM_ROTATION_HORIZONTAL )
            nAngleDegree = nHorizontal;
        else
            nAngleDegree = nVertical;
        rValue <<= nAngleDegree;
    }
    else
        ::property::OPropertySet::getFastPropertyValue( rValue,nHandle );
}

uno::Reference<chart2::XDataTable> SAL_CALL Diagram::getDataTable()
{
    MutexGuard aGuard(m_aMutex);
    return m_xDataTable;
}

void SAL_CALL Diagram::setDataTable(const uno::Reference<chart2::XDataTable>& xDataTable)
{
    auto* pDataTable = dynamic_cast<DataTable*>(xDataTable.get());
    assert(!xDataTable || pDataTable);
    setDataTable(rtl::Reference<DataTable>(pDataTable));
}

void Diagram::setDataTable(const rtl::Reference<DataTable>& xNewDataTable)
{
    rtl::Reference<DataTable> xOldDataTable;
    {
        MutexGuard aGuard(m_aMutex);
        if (m_xDataTable == xNewDataTable)
            return;
        xOldDataTable = m_xDataTable;
        m_xDataTable = xNewDataTable;
    }
    if (xOldDataTable.is())
        ModifyListenerHelper::removeListener(xOldDataTable, m_xModifyEventForwarder);
    if (xNewDataTable.is())
        ModifyListenerHelper::addListener(xNewDataTable, m_xModifyEventForwarder);
    fireModifyEvent();
}

using impl::Diagram_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Diagram, Diagram_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Diagram, Diagram_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL Diagram::getImplementationName()
{
    return "com.sun.star.comp.chart2.Diagram";
}

sal_Bool SAL_CALL Diagram::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Diagram::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.Diagram",
        "com.sun.star.layout.LayoutElement",
        "com.sun.star.beans.PropertySet" };
}

DiagramPositioningMode Diagram::getDiagramPositioningMode()
{
    DiagramPositioningMode eMode = DiagramPositioningMode::Auto;
    chart2::RelativePosition aRelPos;
    chart2::RelativeSize aRelSize;
    if( (getFastPropertyValue(PROP_DIAGRAM_REL_POS) >>= aRelPos ) &&
        (getFastPropertyValue(PROP_DIAGRAM_REL_SIZE) >>= aRelSize ) )
    {
        bool bPosSizeExcludeAxes=false;
        getFastPropertyValue(PROP_DIAGRAM_POSSIZE_EXCLUDE_LABELS) >>= bPosSizeExcludeAxes;
        if( bPosSizeExcludeAxes )
            eMode = DiagramPositioningMode::Excluding;
        else
            eMode = DiagramPositioningMode::Including;
    }
    return eMode;
}


sal_Int32 Diagram::getCorrectedMissingValueTreatment(
            const rtl::Reference< ChartType >& xChartType )
{
    sal_Int32 nResult = css::chart::MissingValueTreatment::LEAVE_GAP;
    const uno::Sequence < sal_Int32 > aAvailableMissingValueTreatments(
                ChartTypeHelper::getSupportedMissingValueTreatments( xChartType ) );

    if( getFastPropertyValue( PROP_DIAGRAM_MISSING_VALUE_TREATMENT ) >>= nResult )
    {
        //ensure that the set value is supported by this charttype
        for( sal_Int32 n : aAvailableMissingValueTreatments )
            if( n == nResult )
                return nResult; //ok
    }

    //otherwise use the first supported one
    if( aAvailableMissingValueTreatments.hasElements() )
    {
        nResult = aAvailableMissingValueTreatments[0];
        return nResult;
    }

    return nResult;
}

void Diagram::setGeometry3D( sal_Int32 nNewGeometry )
{
    std::vector< rtl::Reference< DataSeries > > aSeriesVec =
        getDataSeries();

    for (auto const& series : aSeriesVec)
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(
            series, "Geometry3D", uno::Any( nNewGeometry ));
    }
}

sal_Int32 Diagram::getGeometry3D( bool& rbFound, bool& rbAmbiguous )
{
    sal_Int32 nCommonGeom( css::chart2::DataPointGeometry3D::CUBOID );
    rbFound = false;
    rbAmbiguous = false;

    std::vector< rtl::Reference< DataSeries > > aSeriesVec = getDataSeries();

    if( aSeriesVec.empty())
        rbAmbiguous = true;

    for (auto const& series : aSeriesVec)
    {
        try
        {
            sal_Int32 nGeom = 0;
            if( series->getPropertyValue( "Geometry3D") >>= nGeom )
            {
                if( ! rbFound )
                {
                    // first series
                    nCommonGeom = nGeom;
                    rbFound = true;
                }
                // further series: compare for uniqueness
                else if( nCommonGeom != nGeom )
                {
                    rbAmbiguous = true;
                    break;
                }
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return nCommonGeom;
}

bool Diagram::isPieOrDonutChart()
{
    rtl::Reference< ChartType > xChartType = getChartTypeByIndex( 0 );

    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
            return true;
    }
    return false;
}

bool Diagram::isSupportingFloorAndWall()
{
    //pies and donuts currently do not support this because of wrong files from older versions
    //todo: allow this in future again, if fileversion is available for OLE objects (metastream)
    //thus the wrong bottom can be removed on import

    const std::vector< rtl::Reference< ChartType > > aTypes = getChartTypes();
    for( rtl::Reference< ChartType > const & xType : aTypes )
    {
        OUString sChartType = xType->getChartType();
        if( sChartType.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return false;
        if( sChartType.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return false;
        if( sChartType.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return false;
    }
    return true;
}

 /**
 * This method implements the logic of checking if a series can be moved
 * forward/backward. Depending on the "bDoMove" parameter the series will
 * be moved (bDoMove = true) or the function just will test if the
 * series can be moved without doing the move (bDoMove = false).
 *
 * @param xDiagram
 *  Reference to the diagram that contains the series.
 *
 * @param xGivenDataSeries
 *  Reference to the series that should moved or tested for moving.
 *
 * @param bForward
 *  Direction in which the series should be moved or tested for moving.
 *
 * @param bDoMove
 *  Should this function really move the series (true) or just test if it is
 *  possible (false).
 *
 *
 * @returns
 *  in case of bDoMove == true
 *      - True : if the move was done
 *      - False : the move failed
 *  in case of bDoMove == false
 *      - True : the series can be moved
 *      - False : the series can not be moved
 *
 */
static bool lcl_moveSeriesOrCheckIfMoveIsAllowed(
    Diagram& rDiagram,
    const rtl::Reference< DataSeries >& xGivenDataSeries,
    bool bForward,
    bool bDoMove )
{
    bool bMovedOrMoveAllowed = false;

    try
    {
        if( !xGivenDataSeries.is() )
            return false;

        //find position of series.
        bool bFound = false;
        const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysList( rDiagram.getBaseCoordinateSystems() );

        for( std::size_t nCS = 0; !bFound && nCS < aCooSysList.size(); ++nCS )
        {
            const rtl::Reference< BaseCoordinateSystem > & xCooSys( aCooSysList[nCS] );

            //iterate through all chart types in the current coordinate system
            std::vector< rtl::Reference< ChartType > > aChartTypeList( xCooSys->getChartTypes2() );
            rtl::Reference< ChartType > xFormerChartType;

            for( std::size_t nT = 0; !bFound && nT < aChartTypeList.size(); ++nT )
            {
                rtl::Reference< ChartType > xCurrentChartType( aChartTypeList[nT] );

                //iterate through all series in this chart type

                std::vector< rtl::Reference< DataSeries > > aSeriesList = xCurrentChartType->getDataSeries2();

                for( std::size_t nS = 0; !bFound && nS < aSeriesList.size(); ++nS )
                {

                    // We found the series we are interested in!
                    if( xGivenDataSeries==aSeriesList[nS] )
                    {
                        std::size_t nOldSeriesIndex = nS;
                        bFound = true;

                        try
                        {
                            sal_Int32 nNewSeriesIndex = nS;

                            // tdf#34517 Bringing forward means increasing, backwards means decreasing series position
                            if( !bForward )
                                nNewSeriesIndex--;
                            else
                                nNewSeriesIndex++;

                            if( nNewSeriesIndex >= 0 && o3tl::make_unsigned(nNewSeriesIndex) < aSeriesList.size() )
                            {
                                //move series in the same charttype
                                bMovedOrMoveAllowed = true;
                                if( bDoMove )
                                {
                                    aSeriesList[ nOldSeriesIndex ] = aSeriesList[ nNewSeriesIndex ];
                                    aSeriesList[ nNewSeriesIndex ] = xGivenDataSeries;
                                    xCurrentChartType->setDataSeries( aSeriesList );
                                }
                            }
                            else if( nNewSeriesIndex<0 )
                            {
                                //exchange series with former charttype
                                if( xFormerChartType.is() && DiagramHelper::areChartTypesCompatible( xFormerChartType, xCurrentChartType ) )
                                {
                                    bMovedOrMoveAllowed = true;
                                    if( bDoMove )
                                    {
                                        std::vector< rtl::Reference< DataSeries > > aOtherSeriesList = xFormerChartType->getDataSeries2();
                                        sal_Int32 nOtherSeriesIndex = aOtherSeriesList.size()-1;
                                        if( nOtherSeriesIndex >= 0 && o3tl::make_unsigned(nOtherSeriesIndex) < aOtherSeriesList.size() )
                                        {
                                            rtl::Reference< DataSeries > xExchangeSeries( aOtherSeriesList[nOtherSeriesIndex] );
                                            aOtherSeriesList[nOtherSeriesIndex] = xGivenDataSeries;
                                            xFormerChartType->setDataSeries(aOtherSeriesList);

                                            aSeriesList[nOldSeriesIndex]=xExchangeSeries;
                                            xCurrentChartType->setDataSeries(aSeriesList);
                                        }
                                    }
                                }
                            }
                            else if( nT+1 < aChartTypeList.size() )
                            {
                                //exchange series with next charttype
                                rtl::Reference< ChartType > xOtherChartType( aChartTypeList[nT+1] );
                                if( xOtherChartType.is() && DiagramHelper::areChartTypesCompatible( xOtherChartType, xCurrentChartType ) )
                                {
                                    bMovedOrMoveAllowed = true;
                                    if( bDoMove )
                                    {
                                        std::vector< rtl::Reference< DataSeries > > aOtherSeriesList = xOtherChartType->getDataSeries2();
                                        if( !aOtherSeriesList.empty() )
                                        {
                                            rtl::Reference< DataSeries > xExchangeSeries( aOtherSeriesList[0] );
                                            aOtherSeriesList[0] = xGivenDataSeries;
                                            xOtherChartType->setDataSeries(aOtherSeriesList);

                                            aSeriesList[nOldSeriesIndex]=xExchangeSeries;
                                            xCurrentChartType->setDataSeries(aSeriesList);
                                        }
                                    }
                                }
                            }
                        }
                        catch( const util::CloseVetoException& )
                        {
                        }
                        catch( const uno::RuntimeException& )
                        {
                        }
                    }
                }
                xFormerChartType = xCurrentChartType;
            }
        }
    }
    catch( const util::CloseVetoException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    return bMovedOrMoveAllowed;
}

bool Diagram::isSeriesMoveable(
    const rtl::Reference< DataSeries >& xGivenDataSeries,
    bool bForward )
{
    const bool bDoMove = false;

    bool bIsMoveable = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        *this, xGivenDataSeries, bForward, bDoMove );

    return bIsMoveable;
}

bool Diagram::moveSeries( const rtl::Reference< DataSeries >& xGivenDataSeries, bool bForward )
{
    const bool bDoMove = true;

    bool bMoved = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        *this, xGivenDataSeries, bForward, bDoMove );

    return bMoved;
}

std::vector< rtl::Reference< ChartType > > Diagram::getChartTypes()
{
    std::vector< rtl::Reference< ChartType > > aResult;
    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & coords : getBaseCoordinateSystems() )
        {
            const std::vector< rtl::Reference< ChartType > > & aChartTypeSeq( coords->getChartTypes2());
            aResult.insert( aResult.end(), aChartTypeSeq.begin(), aChartTypeSeq.end() );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return aResult;
}

rtl::Reference< ChartType > Diagram::getChartTypeByIndex( sal_Int32 nIndex )
{
    rtl::Reference< ChartType > xChartType;

    //iterate through all coordinate systems
    sal_Int32 nTypesSoFar = 0;
    for( rtl::Reference< BaseCoordinateSystem > const & coords : getBaseCoordinateSystems() )
    {
        const std::vector< rtl::Reference< ChartType > > & aChartTypeList( coords->getChartTypes2() );
        if( nIndex >= 0 && o3tl::make_unsigned(nIndex) < nTypesSoFar + aChartTypeList.size() )
        {
            xChartType = aChartTypeList[nIndex - nTypesSoFar];
            break;
        }
        nTypesSoFar += aChartTypeList.size();
    }

    return xChartType;
}

bool Diagram::isSupportingDateAxis()
{
    return ::chart::ChartTypeHelper::isSupportingDateAxis( getChartTypeByIndex( 0 ), 0 );
}

static std::vector< rtl::Reference< Axis > > lcl_getAxisHoldingCategoriesFromDiagram(
    Diagram& rDiagram )
{
    std::vector< rtl::Reference< Axis > > aRet;

    // return first x-axis as fall-back
    rtl::Reference< Axis > xFallBack;
    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : rDiagram.getBaseCoordinateSystems() )
        {
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( nN,nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        chart2::ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.Categories.is() || (aScaleData.AxisType == chart2::AxisType::CATEGORY) )
                        {
                            aRet.push_back(xAxis);
                        }
                        if( (nN == 0) && !xFallBack.is())
                            xFallBack = xAxis;
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
    }

    if( aRet.empty() )
        aRet.push_back(xFallBack);

    return aRet;
}

uno::Reference< chart2::data::XLabeledDataSequence > Diagram::getCategories()
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;

    try
    {
        std::vector< rtl::Reference< Axis > > aCatAxes(
            lcl_getAxisHoldingCategoriesFromDiagram( *this ));
        //search for first categories
        if (aCatAxes.empty())
            return xResult;

        rtl::Reference< Axis > xCatAxis(aCatAxes[0]);
        if( !xCatAxis.is())
            return xResult;

        chart2::ScaleData aScaleData( xCatAxis->getScaleData());
        if( !aScaleData.Categories.is() )
            return xResult;

        xResult = aScaleData.Categories;
        uno::Reference<beans::XPropertySet> xProp(xResult->getValues(), uno::UNO_QUERY );
        if( xProp.is() )
        {
            try
            {
                xProp->setPropertyValue( "Role", uno::Any( OUString("categories") ) );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

void Diagram::setCategories(
    const uno::Reference< chart2::data::XLabeledDataSequence >& xCategories,
    bool bSetAxisType  /* = false */,
    bool bCategoryAxis /* = true */ )
{
    std::vector< rtl::Reference< Axis > > aCatAxes(
        lcl_getAxisHoldingCategoriesFromDiagram( *this ));

    for (const rtl::Reference< Axis >& xCatAxis : aCatAxes)
    {
        if( xCatAxis.is())
        {
            chart2::ScaleData aScaleData( xCatAxis->getScaleData());
            aScaleData.Categories = xCategories;
            if( bSetAxisType )
            {
                if( bCategoryAxis )
                    aScaleData.AxisType = chart2::AxisType::CATEGORY;
                else if( aScaleData.AxisType == chart2::AxisType::CATEGORY || aScaleData.AxisType == chart2::AxisType::DATE )
                    aScaleData.AxisType = chart2::AxisType::REALNUMBER;
            }
            xCatAxis->setScaleData( aScaleData );
        }
    }
}

bool Diagram::isCategory()
{
    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : getBaseCoordinateSystems() )
        {
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( nN,nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        chart2::ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.AxisType == chart2::AxisType::CATEGORY || aScaleData.AxisType == chart2::AxisType::DATE )
                            return true;
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return false;
}

std::vector< std::vector< rtl::Reference< DataSeries > > >
Diagram::getDataSeriesGroups()
{
    std::vector< std::vector< rtl::Reference< DataSeries > > > aResult;

    //iterate through all coordinate systems
    for( rtl::Reference< BaseCoordinateSystem > const & coords : getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        for( rtl::Reference< ChartType >  const & chartType : coords->getChartTypes2() )
        {
            aResult.push_back( chartType->getDataSeries2() );
        }
    }
    return aResult;
}

std::vector< rtl::Reference< ::chart::DataSeries > >
    Diagram::getDataSeries()
{
    std::vector< rtl::Reference< DataSeries > > aResult;
    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & coords : getBaseCoordinateSystems() )
        {
            for( rtl::Reference< ChartType> const & chartType : coords->getChartTypes2() )
            {
                const std::vector< rtl::Reference< DataSeries > > aSeriesSeq( chartType->getDataSeries2() );
                aResult.insert( aResult.end(), aSeriesSeq.begin(), aSeriesSeq.end() );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return aResult;
}

rtl::Reference< ChartType > Diagram::getChartTypeOfSeries(
        const rtl::Reference< DataSeries >& xGivenDataSeries )
{
    if( !xGivenDataSeries.is() )
        return nullptr;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems

    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        const std::vector< rtl::Reference< ChartType > > & aChartTypeList( xCooSys->getChartTypes2() );
        for( rtl::Reference< ChartType > const & xChartType : aChartTypeList )
        {
            //iterate through all series in this chart type
            for( rtl::Reference< DataSeries > const & dataSeries : xChartType->getDataSeries2() )
            {
                if( xGivenDataSeries==dataSeries )
                    return xChartType;
            }
        }
    }
    return nullptr;
}

rtl::Reference< Axis > Diagram::getAttachedAxis(
        const rtl::Reference< DataSeries >& xSeries )
{
    return AxisHelper::getAxis( 1, DiagramHelper::isSeriesAttachedToMainAxis( xSeries ), this );
}

bool Diagram::attachSeriesToAxis( bool bAttachToMainAxis
                        , const rtl::Reference< DataSeries >& xDataSeries
                        , const uno::Reference< uno::XComponentContext > & xContext
                        , bool bAdaptAxes )
{
    bool bChanged = false;

    //set property at axis

    sal_Int32 nNewAxisIndex = bAttachToMainAxis ? 0 : 1;
    sal_Int32 nOldAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
    rtl::Reference< Axis > xOldAxis = getAttachedAxis( xDataSeries );

    if( nOldAxisIndex != nNewAxisIndex )
    {
        try
        {
            xDataSeries->setPropertyValue( "AttachedAxisIndex", uno::Any( nNewAxisIndex ) );
            bChanged = true;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    if( bChanged )
    {
        rtl::Reference< Axis > xAxis = AxisHelper::getAxis( 1, bAttachToMainAxis, this );
        if(!xAxis.is()) //create an axis if necessary
            xAxis = AxisHelper::createAxis( 1, bAttachToMainAxis, this, xContext );
        if( bAdaptAxes )
        {
            AxisHelper::makeAxisVisible( xAxis );
            AxisHelper::hideAxisIfNoDataIsAttached( xOldAxis, this );
        }
    }

    return bChanged;
}

void Diagram::replaceCoordinateSystem(
    const rtl::Reference< BaseCoordinateSystem > & xCooSysToReplace,
    const rtl::Reference< BaseCoordinateSystem > & xReplacement )
{
    // update the coordinate-system container
    try
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xCategories = getCategories();

        // move chart types of xCooSysToReplace to xReplacement
        xReplacement->setChartTypes( xCooSysToReplace->getChartTypes());

        removeCoordinateSystem( xCooSysToReplace );
        addCoordinateSystem( xReplacement );

        if( xCategories.is() )
            setCategories( xCategories );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

sal_Int32 Diagram::getDimension()
{
    // -1: not yet set
    sal_Int32 nResult = -1;

    try
    {
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : getBaseCoordinateSystems() )
        {
            if(xCooSys.is())
            {
                nResult = xCooSys->getDimension();
                break;
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return nResult;
}

void Diagram::setDimension( sal_Int32 nNewDimensionCount )
{
    if( getDimension() == nNewDimensionCount )
        return;

    try
    {
        bool rbFound = false;
        bool rbAmbiguous = true;
        StackMode eStackMode = getStackMode( rbFound, rbAmbiguous );
        bool bIsSupportingOnlyDeepStackingFor3D=false;

        //change all coordinate systems:
        auto aCoordSystems = getBaseCoordinateSystems();
        for( rtl::Reference<BaseCoordinateSystem> const & xOldCooSys : aCoordSystems )
        {
            rtl::Reference< BaseCoordinateSystem > xNewCooSys;

            const std::vector< rtl::Reference< ChartType > > aChartTypeList( xOldCooSys->getChartTypes2() );
            for( rtl::Reference< ChartType > const & xChartType : aChartTypeList )
            {
                bIsSupportingOnlyDeepStackingFor3D = ChartTypeHelper::isSupportingOnlyDeepStackingFor3D( xChartType );
                if(!xNewCooSys.is())
                {
                    xNewCooSys = dynamic_cast<BaseCoordinateSystem*>(xChartType->createCoordinateSystem( nNewDimensionCount ).get());
                    assert(xNewCooSys);
                    break;
                }
                //@todo make sure that all following charttypes are also capable of the new dimension
                //otherwise separate them in a different group
                //BM: might be done in replaceCoordinateSystem()
            }

            // replace the old coordinate system at all places where it was used
            replaceCoordinateSystem( xOldCooSys, xNewCooSys );
        }

        //correct stack mode if necessary
        if( nNewDimensionCount==3 && eStackMode != StackMode::ZStacked && bIsSupportingOnlyDeepStackingFor3D )
            setStackMode( StackMode::ZStacked );
        else if( nNewDimensionCount==2 && eStackMode == StackMode::ZStacked )
            setStackMode( StackMode::NONE );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void Diagram::setStackMode( StackMode eStackMode )
{
    try
    {
        bool bValueFound = false;
        bool bIsAmbiguous = false;
        StackMode eOldStackMode = getStackMode( bValueFound, bIsAmbiguous );

        if( eStackMode == eOldStackMode && !bIsAmbiguous )
            return;

        chart2::StackingDirection eNewDirection = chart2::StackingDirection_NO_STACKING;
        if( eStackMode == StackMode::YStacked || eStackMode == StackMode::YStackedPercent )
            eNewDirection = chart2::StackingDirection_Y_STACKING;
        else if( eStackMode == StackMode::ZStacked )
            eNewDirection = chart2::StackingDirection_Z_STACKING;

        uno::Any aNewDirection( eNewDirection );

        bool bPercent = false;
        if( eStackMode == StackMode::YStackedPercent )
            bPercent = true;

        //iterate through all coordinate systems
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : getBaseCoordinateSystems() )
        {
            //set correct percent stacking
            const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(1);
            for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
            {
                rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( 1,nI );
                if( xAxis.is())
                {
                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                    if( (aScaleData.AxisType==chart2::AxisType::PERCENT) != bPercent )
                    {
                        if( bPercent )
                            aScaleData.AxisType = chart2::AxisType::PERCENT;
                        else
                            aScaleData.AxisType = chart2::AxisType::REALNUMBER;
                        xAxis->setScaleData( aScaleData );
                    }
                }
            }
            //iterate through all chart types in the current coordinate system
            const std::vector< rtl::Reference< ChartType > > & aChartTypeList( xCooSys->getChartTypes2() );
            if (aChartTypeList.empty())
                continue;

            rtl::Reference< ChartType > xChartType( aChartTypeList[0] );

            //iterate through all series in this chart type
            for( rtl::Reference< DataSeries > const & dataSeries : xChartType->getDataSeries2() )
            {
                dataSeries->setPropertyValue( "StackingDirection", aNewDirection );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

StackMode Diagram::getStackMode( bool& rbFound, bool& rbAmbiguous )
{
    rbFound=false;
    rbAmbiguous=false;

    StackMode eGlobalStackMode = StackMode::NONE;

    //iterate through all coordinate systems
    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        std::vector< rtl::Reference< ChartType > > aChartTypeList( xCooSys->getChartTypes2() );
        for( std::size_t nT = 0; nT < aChartTypeList.size(); ++nT )
        {
            rtl::Reference< ChartType > xChartType( aChartTypeList[nT] );

            StackMode eLocalStackMode = DiagramHelper::getStackModeFromChartType(
                xChartType, rbFound, rbAmbiguous, xCooSys );

            if( rbFound && eLocalStackMode != eGlobalStackMode && nT>0 )
            {
                rbAmbiguous = true;
                return eGlobalStackMode;
            }

            eGlobalStackMode = eLocalStackMode;
        }
    }

    return eGlobalStackMode;
}

void Diagram::setVertical( bool bVertical /* = true */ )
{
    try
    {
        uno::Any aValue;
        aValue <<= bVertical;
        for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : getBaseCoordinateSystems() )
        {
            bool bChanged = false;
            bool bOldSwap = false;
            if( !(xCooSys->getPropertyValue("SwapXAndYAxis") >>= bOldSwap)
                || bVertical != bOldSwap )
                bChanged = true;

            if( bChanged )
                xCooSys->setPropertyValue("SwapXAndYAxis", aValue);

            const sal_Int32 nDimensionCount = xCooSys->getDimension();
            sal_Int32 nDimIndex = 0;
            for (nDimIndex=0; nDimIndex < nDimensionCount; ++nDimIndex)
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
                for (sal_Int32 nI = 0; nI <= nMaximumScaleIndex; ++nI)
                {
                    rtl::Reference<Axis> xAxis = xCooSys->getAxisByDimension2(nDimIndex,nI);
                    if (!xAxis.is())
                        continue;

                    //adapt title rotation only when axis swapping has changed
                    if (!bChanged)
                        continue;

                    Reference< beans::XPropertySet > xTitleProps( xAxis->getTitleObject(), uno::UNO_QUERY );
                    if (!xTitleProps.is())
                        continue;

                    double fAngleDegree = 0.0;
                    xTitleProps->getPropertyValue("TextRotation") >>= fAngleDegree;
                    if (fAngleDegree != 0.0 &&
                        !rtl::math::approxEqual(fAngleDegree, 90.0))
                        continue;

                    double fNewAngleDegree = 0.0;
                    if( !bVertical && nDimIndex == 1 )
                        fNewAngleDegree = 90.0;
                    else if( bVertical && nDimIndex == 0 )
                        fNewAngleDegree = 90.0;

                    xTitleProps->setPropertyValue("TextRotation", uno::Any(fNewAngleDegree));
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

bool Diagram::getVertical( bool& rbFound, bool& rbAmbiguous )
{
    bool bValue = false;
    rbFound = false;
    rbAmbiguous = false;

    for (rtl::Reference<BaseCoordinateSystem> const & coords : getBaseCoordinateSystems())
    {
        bool bCurrent = false;
        if (coords->getPropertyValue("SwapXAndYAxis") >>= bCurrent)
        {
            if (!rbFound)
            {
                bValue = bCurrent;
                rbFound = true;
            }
            else if (bCurrent != bValue)
            {
                // ambiguous -> choose always first found
                rbAmbiguous = true;
            }
        }
    }
    return bValue;
}

Diagram::tTemplateWithServiceName
    Diagram::getTemplate(
        const rtl::Reference< ::chart::ChartTypeManager > & xChartTypeManager )
{
    tTemplateWithServiceName aResult;

    if( !xChartTypeManager )
        return aResult;

    Sequence< OUString > aServiceNames( xChartTypeManager->getAvailableServiceNames());
    const sal_Int32 nLength = aServiceNames.getLength();

    bool bTemplateFound = false;

    for( sal_Int32 i = 0; ! bTemplateFound && i < nLength; ++i )
    {
        try
        {
            rtl::Reference< ::chart::ChartTypeTemplate > xTempl =
                xChartTypeManager->createTemplate( aServiceNames[ i ] );

            if (xTempl.is() && xTempl->matchesTemplate2(this, true))
            {
                aResult.xChartTypeTemplate = xTempl;
                aResult.sServiceName = aServiceNames[ i ];
                bTemplateFound = true;
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return aResult;
}

std::vector< rtl::Reference< RegressionCurveModel > >
    Diagram::getAllRegressionCurvesNotMeanValueLine()
{
    std::vector< rtl::Reference< RegressionCurveModel > > aResult;
    std::vector< rtl::Reference< DataSeries > > aSeries( getDataSeries());
    for (auto const& elem : aSeries)
    {
        for( rtl::Reference< RegressionCurveModel > const & curve : elem->getRegressionCurves2() )
        {
            if( ! RegressionCurveHelper::isMeanValueLine( curve ))
                aResult.push_back( curve );
        }
    }

    return aResult;
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_Diagram_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::Diagram(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
