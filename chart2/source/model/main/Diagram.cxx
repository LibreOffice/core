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
#include <PropertyHelper.hxx>
#include "Wall.hxx"
#include <ModifyListenerHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <ConfigColorScheme.hxx>
#include <DiagramHelper.hxx>
#include <ThreeDHelper.hxx>
#include <CloneHelper.hxx>
#include <SceneProperties.hxx>
#include <unonames.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>

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
    rOutProperties.emplace_back( "DataTableHBorder",
               PROP_DIAGRAM_DATATABLEHBORDER,
                 cppu::UnoType<bool>::get(),
                 beans::PropertyAttribute::BOUND
                 | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "DataTableVBorder",
               PROP_DIAGRAM_DATATABLEVBORDER,
                 cppu::UnoType<bool>::get(),
                 beans::PropertyAttribute::BOUND
                 | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "DataTableOutline",
               PROP_DIAGRAM_DATATABLEOUTLINE,
                 cppu::UnoType<bool>::get(),
                 beans::PropertyAttribute::BOUND
                 | beans::PropertyAttribute::MAYBEDEFAULT );
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
        aMap.setPropertyValueDefault( PROP_DIAGRAM_POSSIZE_EXCLUDE_LABELS, true );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_SORT_BY_X_VALUES, false );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_CONNECT_BARS, false );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_GROUP_BARS_PER_AXIS, true );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS, true );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_RIGHT_ANGLED_AXES, false );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_DATATABLEHBORDER, false );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_DATATABLEVBORDER, false );
        aMap.setPropertyValueDefault( PROP_DIAGRAM_DATATABLEOUTLINE, false );
        aMap.setPropertyValueDefault< sal_Int32 >( PROP_DIAGRAM_STARTING_ANGLE, 90 );
        aMap.setPropertyValueDefault< sal_Int32 >( PROP_DIAGRAM_3DRELATIVEHEIGHT, 100 );
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

/// clones a UNO-sequence of UNO-References
typedef Reference< chart2::XCoordinateSystem > lcl_tCooSysRef;
typedef std::vector< lcl_tCooSysRef >          lcl_tCooSysVector;

void lcl_CloneCoordinateSystems(
        const lcl_tCooSysVector & rSource,
        lcl_tCooSysVector & rDestination )
{
    for( auto const & i : rSource )
    {
        lcl_tCooSysRef xClone;
        css::uno::Reference< css::util::XCloneable > xCloneable( i, css::uno::UNO_QUERY );
        if( xCloneable.is())
            xClone.set( xCloneable->createClone(), css::uno::UNO_QUERY );

        if( xClone.is())
        {
            rDestination.push_back( xClone );
        }
        else
            rDestination.push_back( i );
    }
}

} // anonymous namespace

namespace chart
{

Diagram::Diagram( uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_xContext( xContext ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
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
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    lcl_CloneCoordinateSystems( rOther.m_aCoordSystems, m_aCoordSystems );
    ModifyListenerHelper::addListenerToAllElements( m_aCoordSystems, m_xModifyEventForwarder );

    m_xWall.set( CloneHelper::CreateRefClone< beans::XPropertySet >()( rOther.m_xWall ));
    m_xFloor.set( CloneHelper::CreateRefClone< beans::XPropertySet >()( rOther.m_xFloor ));
    m_xTitle.set( CloneHelper::CreateRefClone< chart2::XTitle >()( rOther.m_xTitle ));
    m_xLegend.set( CloneHelper::CreateRefClone< chart2::XLegend >()( rOther.m_xLegend ));

    ModifyListenerHelper::addListener( m_xWall, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xFloor, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xTitle, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( m_xLegend, m_xModifyEventForwarder );
}

Diagram::~Diagram()
{
    try
    {
        ModifyListenerHelper::removeListenerFromAllElements( m_aCoordSystems, m_xModifyEventForwarder );

        ModifyListenerHelper::removeListener( m_xWall, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListener( m_xFloor, m_xModifyEventForwarder );
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
    uno::Reference< beans::XPropertySet > xRet;
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
        ModifyListenerHelper::addListener( xRet, m_xModifyEventForwarder );
    return xRet;
}

uno::Reference< beans::XPropertySet > SAL_CALL Diagram::getFloor()
{
    uno::Reference< beans::XPropertySet > xRet;
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
        ModifyListenerHelper::addListener( xRet, m_xModifyEventForwarder );
    return xRet;
}

uno::Reference< chart2::XLegend > SAL_CALL Diagram::getLegend()
{
    MutexGuard aGuard( m_aMutex );
    return m_xLegend;
}

void SAL_CALL Diagram::setLegend( const uno::Reference< chart2::XLegend >& xNewLegend )
{
    Reference< chart2::XLegend > xOldLegend;
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
    uno::Reference< lang::XMultiServiceFactory > xChartTypeManager( m_xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.chart2.ChartTypeManager", m_xContext ), uno::UNO_QUERY );
    DiagramHelper::tTemplateWithServiceName aTemplateAndService = DiagramHelper::getTemplateForDiagram( this, xChartTypeManager );
    uno::Reference< chart2::XChartTypeTemplate > xTemplate( aTemplateAndService.first );
    if( !xTemplate.is() )
        xTemplate.set( xChartTypeManager->createInstance( "com.sun.star.chart2.template.Column" ), uno::UNO_QUERY );
    if(!xTemplate.is())
        return;
    xTemplate->changeDiagramData( this, xDataSource, aArguments );
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
    {
        MutexGuard aGuard( m_aMutex );
        if( std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), aCoordSys )
            != m_aCoordSystems.end())
            throw lang::IllegalArgumentException("coordsys not found", static_cast<cppu::OWeakObject*>(this), 1);

        if( !m_aCoordSystems.empty() )
        {
            OSL_FAIL( "more than one coordinatesystem is not supported yet by the fileformat" );
            return;
        }
        m_aCoordSystems.push_back( aCoordSys );
    }
    ModifyListenerHelper::addListener( aCoordSys, m_xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL Diagram::removeCoordinateSystem(
    const uno::Reference< chart2::XCoordinateSystem >& aCoordSys )
{
    {
        MutexGuard aGuard( m_aMutex );
        std::vector< uno::Reference< chart2::XCoordinateSystem > >::iterator
              aIt( std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), aCoordSys ));
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
    return comphelper::containerToSequence( m_aCoordSystems );
}

void SAL_CALL Diagram::setCoordinateSystems(
    const Sequence< Reference< chart2::XCoordinateSystem > >& aCoordinateSystems )
{
    tCoordinateSystemContainerType aNew;
    tCoordinateSystemContainerType aOld;
    if( aCoordinateSystems.hasElements() )
    {
        OSL_ENSURE( aCoordinateSystems.getLength()<=1, "more than one coordinatesystem is not supported yet by the fileformat" );
        aNew.push_back( aCoordinateSystems[0] );
    }
    {
        MutexGuard aGuard( m_aMutex );
        std::swap( aOld, m_aCoordSystems );
        m_aCoordSystems = aNew;
    }
    ModifyListenerHelper::removeListenerFromAllElements( aOld, m_xModifyEventForwarder );
    ModifyListenerHelper::addListenerToAllElements( aNew, m_xModifyEventForwarder );
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
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL Diagram::removeModifyListener( const Reference< util::XModifyListener >& aListener )
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
    rStaticDefaults.get(nHandle, rAny);
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

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_Diagram_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::Diagram(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
