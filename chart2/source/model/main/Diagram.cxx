/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Diagram.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 16:00:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "Diagram.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"
#include "Wall.hxx"
#include "ContextHelper.hxx"
#include "UserDefinedProperties.hxx"
#include "ConfigColorScheme.hxx"
#include "ContainerHelper.hxx"
#include "ThreeDHelper.hxx"
#include "CloneHelper.hxx"
#include "AxisHelper.hxx"
#include "SceneProperties.hxx"
#include "DisposeHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_RELATIVEPOSITION_HPP_
#include <com/sun/star/chart2/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_RELATIVESIZE_HPP_
#include <com/sun/star/chart2/RelativeSize.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif

#include <com/sun/star/drawing/HomogenMatrix.hpp>

#include <algorithm>
#include <iterator>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// ======================================================================

namespace
{

enum
{
    PROP_DIAGRAM_REL_POS,
    PROP_DIAGRAM_REL_SIZE,
    PROP_DIAGRAM_SORT_BY_X_VALUES,
    PROP_DIAGRAM_CONNECT_BARS,
    PROP_DIAGRMA_RIGHT_ANGLED_AXES,
    PROP_DIAGRAM_STARTING_ANGLE,
    PROP_DIAGRAM_GROUP_BARS_PER_AXIS
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "RelativePosition" ),
                  PROP_DIAGRAM_REL_POS,
                  ::getCppuType( reinterpret_cast< const chart2::RelativePosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "RelativeSize" ),
                  PROP_DIAGRAM_REL_SIZE,
                  ::getCppuType( reinterpret_cast< const chart2::RelativeSize * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "SortByXValues" ),
                  PROP_DIAGRAM_SORT_BY_X_VALUES,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U("ConnectBars"),
                  PROP_DIAGRAM_CONNECT_BARS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U("RightAngledAxes"),
                  PROP_DIAGRMA_RIGHT_ANGLED_AXES,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "StartingAngle" ),
                  PROP_DIAGRAM_STARTING_ANGLE,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0) ),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U("GroupBarsPerAxis"),
                  PROP_DIAGRAM_GROUP_BARS_PER_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DIAGRAM_SORT_BY_X_VALUES, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DIAGRAM_CONNECT_BARS, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DIAGRMA_RIGHT_ANGLED_AXES, false );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_DIAGRAM_STARTING_ANGLE, 90 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DIAGRAM_GROUP_BARS_PER_AXIS, true );
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::SceneProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

/// clones a UNO-sequence of UNO-References
typedef Reference< chart2::XCoordinateSystem > lcl_tCooSysRef;
typedef ::std::map< lcl_tCooSysRef, lcl_tCooSysRef >  lcl_tCooSysMapping;
typedef ::std::vector< lcl_tCooSysRef >               lcl_tCooSysVector;

lcl_tCooSysMapping lcl_CloneCoordinateSystems(
        const lcl_tCooSysVector & rSource,
        lcl_tCooSysVector & rDestination )
{
    lcl_tCooSysMapping aResult;

    for( lcl_tCooSysVector::const_iterator aIt( rSource.begin());
         aIt != rSource.end(); ++aIt )
    {
        lcl_tCooSysRef xClone;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >
              xCloneable( *aIt, ::com::sun::star::uno::UNO_QUERY );
        if( xCloneable.is())
            xClone.set( xCloneable->createClone(), ::com::sun::star::uno::UNO_QUERY );

        if( xClone.is())
        {
            rDestination.push_back( xClone );
            aResult.insert( lcl_tCooSysMapping::value_type( *aIt, xClone ));
        }
        else
            rDestination.push_back( *aIt );
    }

    return aResult;
}

} // anonymous namespace

// ======================================================================

namespace chart
{

Diagram::Diagram( uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_xContext( xContext ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    // Set camera position to a default position (that should be set hard, so
    // that it will be exported.  The property default is a camera looking
    // straight ono the scene).  These defaults have been acquired from the old
    // chart implemetation.
    setFastPropertyValue_NoBroadcast(
        SceneProperties::PROP_SCENE_CAMERA_GEOMETRY, uno::makeAny(
            ThreeDHelper::getDefaultCameraGeometry()));
}

Diagram::Diagram( const Diagram & rOther ) :
        MutexContainer(),
        impl::Diagram_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    lcl_tCooSysMapping aCooSysMapping =
        lcl_CloneCoordinateSystems( rOther.m_aCoordSystems, m_aCoordSystems );
    ModifyListenerHelper::addListenerToAllElements( m_aCoordSystems, m_xModifyEventForwarder );

    m_xWall.set( CloneHelper::CreateRefClone< Reference< beans::XPropertySet > >()( rOther.m_xWall ));
    m_xFloor.set( CloneHelper::CreateRefClone< Reference< beans::XPropertySet > >()( rOther.m_xFloor ));
    m_xTitle.set( CloneHelper::CreateRefClone< Reference< chart2::XTitle > >()( rOther.m_xTitle ));
    m_xLegend.set( CloneHelper::CreateRefClone< Reference< chart2::XLegend > >()( rOther.m_xLegend ));

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
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XDiagram ____
uno::Reference< beans::XPropertySet > SAL_CALL Diagram::getWall()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    if( ! m_xWall.is())
    {
        m_xWall.set( new Wall());
        ModifyListenerHelper::addListener( m_xWall, m_xModifyEventForwarder );
    }
    return m_xWall;
    // \--
}

uno::Reference< beans::XPropertySet > SAL_CALL Diagram::getFloor()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    if( ! m_xFloor.is())
    {
        m_xFloor.set( new Wall());
        ModifyListenerHelper::addListener( m_xFloor, m_xModifyEventForwarder );
    }
    return m_xFloor;
    // \--
}

uno::Reference< chart2::XLegend > SAL_CALL Diagram::getLegend()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xLegend;
    // \--
}

void SAL_CALL Diagram::setLegend( const uno::Reference< chart2::XLegend >& xLegend )
    throw (uno::RuntimeException)
{
    // /--
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    if( xLegend != m_xLegend )
    {
        if( m_xLegend.is())
            ModifyListenerHelper::removeListener( m_xLegend, m_xModifyEventForwarder );
        m_xLegend = xLegend;
        if( m_xLegend.is())
            ModifyListenerHelper::addListener( m_xLegend, m_xModifyEventForwarder );
        aGuard.clear();
        // \--
        fireModifyEvent();
    }
}

Reference< chart2::XColorScheme > SAL_CALL Diagram::getDefaultColorScheme()
    throw (uno::RuntimeException)
{
    if( ! m_xColorScheme.is())
    {
        m_xColorScheme.set( new ConfigColorScheme( m_xContext ));
    }
    return m_xColorScheme;
}

void SAL_CALL Diagram::setDefaultColorScheme( const Reference< chart2::XColorScheme >& xColorScheme )
    throw (uno::RuntimeException)
{
    m_xColorScheme.set( xColorScheme );
    fireModifyEvent();
}

void SAL_CALL Diagram::setUnusedData( const Sequence< Reference< chart2::data::XLabeledDataSequence > >& aUnusedData )
    throw (uno::RuntimeException)
{
    m_aUnusedData = aUnusedData;
}

Sequence< Reference< chart2::data::XLabeledDataSequence > > SAL_CALL Diagram::getUnusedData()
    throw (uno::RuntimeException)
{
    return m_aUnusedData;
}

// ____ XTitled ____
uno::Reference< chart2::XTitle > SAL_CALL Diagram::getTitleObject()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xTitle;
    // \--
}

void SAL_CALL Diagram::setTitleObject( const uno::Reference< chart2::XTitle >& Title )
    throw (uno::RuntimeException)
{
    // /--
    ::osl::ClearableMutexGuard aGuard( GetMutex() );

    if( m_xTitle != Title )
    {
        ModifyListenerHelper::removeListener( m_xTitle, m_xModifyEventForwarder );
        m_xTitle = Title;
        ModifyListenerHelper::addListener( m_xTitle, m_xModifyEventForwarder );
        // \--
        aGuard.clear();
        fireModifyEvent();
    }
}

// ____ XCoordinateSystemContainer ____
void SAL_CALL Diagram::addCoordinateSystem(
    const uno::Reference< chart2::XCoordinateSystem >& aCoordSys )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ::std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), aCoordSys )
        != m_aCoordSystems.end())
        throw lang::IllegalArgumentException();

    if( m_aCoordSystems.size()>=1 )
    {
        OSL_ENSURE( false, "more than one coordinatesystem is not supported yet by the fileformat" );
        return;
    }

    m_aCoordSystems.push_back( aCoordSys );

    ModifyListenerHelper::addListener( aCoordSys, m_xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL Diagram::removeCoordinateSystem(
    const uno::Reference< chart2::XCoordinateSystem >& aCoordSys )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    ::std::vector< uno::Reference< chart2::XCoordinateSystem > >::iterator
          aIt( ::std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), aCoordSys ));
    if( aIt == m_aCoordSystems.end())
        throw container::NoSuchElementException(
            C2U( "The given coordinate-system is no element of the container" ),
            static_cast< uno::XWeak * >( this ));

    m_aCoordSystems.erase( aIt );

    ModifyListenerHelper::removeListener( aCoordSys, m_xModifyEventForwarder );
    fireModifyEvent();
}

uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > SAL_CALL Diagram::getCoordinateSystems()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aCoordSystems );
}

void SAL_CALL Diagram::setCoordinateSystems(
    const Sequence< Reference< chart2::XCoordinateSystem > >& aCoordinateSystems )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    Sequence< Reference< chart2::XCoordinateSystem > > aNew(aCoordinateSystems);

    if( aNew.getLength()>1 )
    {
        OSL_ENSURE( false, "more than one coordinatesystem is not supported yet by the fileformat" );
        aNew.realloc(1);
    }

    ModifyListenerHelper::removeListenerFromAllElements( m_aCoordSystems, m_xModifyEventForwarder );
    m_aCoordSystems = ContainerHelper::SequenceToVector( aNew );
    ModifyListenerHelper::addListenerToAllElements( m_aCoordSystems, m_xModifyEventForwarder );
    fireModifyEvent();
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL Diagram::createClone()
    throw (uno::RuntimeException)
{
    return Reference< util::XCloneable >( new Diagram( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL Diagram::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL Diagram::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL Diagram::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Diagram::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
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

// ============================================================

Sequence< OUString > Diagram::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );

    aServices[ 0 ] = C2U( "com.sun.star.chart2.Diagram" );
    aServices[ 1 ] = C2U( "com.sun.star.layout.LayoutElement" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any Diagram::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
        ::chart::SceneProperties::AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL Diagram::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper( lcl_GetPropertySequence(),
                                                      /* bSorted = */ sal_True );

    return aArrayHelper;
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    Diagram::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ================================================================================

using impl::Diagram_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Diagram, Diagram_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Diagram, Diagram_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Diagram,
                             C2U( "com.sun.star.comp.chart2.Diagram" ));

} //  namespace chart
