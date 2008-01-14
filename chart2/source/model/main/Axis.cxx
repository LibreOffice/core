/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Axis.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:59:49 $
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
#include "Axis.hxx"
#include "GridProperties.hxx"
#include "macros.hxx"
#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"
#include "AxisHelper.hxx"
#include "EventListenerHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_AXISTYPE_HPP_
#include <com/sun/star/chart2/AxisType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXISPOSITION_HPP_
#include <com/sun/star/chart2/AxisPosition.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.Axis" ));

enum
{
    PROP_AXIS_SHOW,
    PROP_AXIS_POSITION,
    PROP_AXIS_DISPLAY_LABELS,
    PROP_AXIS_TEXT_ROTATION,
    PROP_AXIS_TEXT_BREAK,
    PROP_AXIS_TEXT_OVERLAP,
    PROP_AXIS_TEXT_STACKED,
    PROP_AXIS_TEXT_ARRANGE_ORDER,
    PROP_AXIS_NUMBER_FORMAT,
    PROP_AXIS_REFERENCE_DIAGRAM_SIZE,

    // for Testing only!
    PROP_AXIS_MAJOR_TICKMARKS,
    // for Testing only!
    PROP_AXIS_MINOR_TICKMARKS
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Show" ),
                  PROP_AXIS_SHOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AxisPosition" ),
                  PROP_AXIS_POSITION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "DisplayLabels" ),
                  PROP_AXIS_DISPLAY_LABELS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_AXIS_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextBreak" ),
                  PROP_AXIS_TEXT_BREAK,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextOverlap" ),
                  PROP_AXIS_TEXT_OVERLAP,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "StackCharacters" ),
                  PROP_AXIS_TEXT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ArrangeOrder" ),
                  PROP_AXIS_TEXT_ARRANGE_ORDER,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisArrangeOrderType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_AXIS_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "ReferenceDiagramSize" ),
                  PROP_AXIS_REFERENCE_DIAGRAM_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // for Testing only!
    rOutProperties.push_back(
        Property( C2U( "MajorTickmarks" ),
                  PROP_AXIS_MAJOR_TICKMARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // for Testing only!
    rOutProperties.push_back(
        Property( C2U( "MinorTickmarks" ),
                  PROP_AXIS_MINOR_TICKMARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_SHOW, true );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_POSITION, chart2::AxisPosition::MAIN );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_DISPLAY_LABELS, true );
    ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_AXIS_TEXT_ROTATION, 0.0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_TEXT_BREAK, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_TEXT_OVERLAP, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_TEXT_STACKED, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_AXIS_TEXT_ARRANGE_ORDER, ::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO );

    float fDefaultCharHeight = 8.0;
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );

    // for Testing only!
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_AXIS_MAJOR_TICKMARKS, 2 /* CHAXIS_MARK_OUTER */ );
    // for Testing only!
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_AXIS_MINOR_TICKMARKS, 0 /* CHAXIS_MARK_NONE */ );
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

typedef uno::Reference< beans::XPropertySet > lcl_tSubGridType;
typedef uno::Sequence< lcl_tSubGridType >     lcl_tSubGridSeq;

void lcl_CloneSubGrids(
    const lcl_tSubGridSeq & rSource, lcl_tSubGridSeq & rDestination )
{
    const lcl_tSubGridType * pBegin = rSource.getConstArray();
    const lcl_tSubGridType * pEnd = pBegin + rSource.getLength();

    rDestination.realloc( rSource.getLength());
    lcl_tSubGridType * pDestBegin = rDestination.getArray();
    lcl_tSubGridType * pDestEnd   = pDestBegin + rDestination.getLength();
    lcl_tSubGridType * pDestIt    = pDestBegin;

    for( const lcl_tSubGridType * pIt = pBegin; pIt != pEnd; ++pIt )
    {
        Reference< beans::XPropertySet > xSubGrid( *pIt );
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
    (void)(pDestEnd); // avoid warning
}

} // anonymous namespace

// ================================================================================

namespace chart
{

Axis::Axis( Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder()),
        m_aScaleData( AxisHelper::createDefaultScale() ),
        m_xGrid( new GridProperties() ),
        m_aSubGridProperties(),
        m_xTitle()
{
    setFastPropertyValue_NoBroadcast(
        ::chart::LineProperties::PROP_LINE_COLOR, uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ) );  // gray30

    if( m_xGrid.is())
        ModifyListenerHelper::addListener( m_xGrid, m_xModifyEventForwarder );
    if( m_aScaleData.Categories.is())
        ModifyListenerHelper::addListener( m_aScaleData.Categories, m_xModifyEventForwarder );

    AllocateSubGrids();
}

Axis::Axis( const Axis & rOther ) :
        MutexContainer(),
        impl::Axis_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder()),
    m_aScaleData( rOther.m_aScaleData )
{
    m_xGrid.set( CloneHelper::CreateRefClone< Reference< beans::XPropertySet > >()( rOther.m_xGrid ));
    if( m_xGrid.is())
        ModifyListenerHelper::addListener( m_xGrid, m_xModifyEventForwarder );

    if( m_aScaleData.Categories.is())
        ModifyListenerHelper::addListener( m_aScaleData.Categories, m_xModifyEventForwarder );

    if( rOther.m_aSubGridProperties.getLength() != 0 )
        lcl_CloneSubGrids( rOther.m_aSubGridProperties, m_aSubGridProperties );
    ModifyListenerHelper::addListenerToAllSequenceElements( m_aSubGridProperties, m_xModifyEventForwarder );

    m_xTitle.set( CloneHelper::CreateRefClone< Reference< chart2::XTitle > >()( rOther.m_xTitle ));
    if( m_xTitle.is())
        ModifyListenerHelper::addListener( m_xTitle, m_xModifyEventForwarder );
}

// late initialization to call after copy-constructing
void Axis::Init( const Axis & /* rOther */ )
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
            m_aScaleData.Categories.set(0);
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    m_aSubGridProperties.realloc(0);
    m_xGrid = 0;
    m_xTitle = 0;
}

void Axis::AllocateSubGrids()
{
    sal_Int32 nNewSubIncCount = m_aScaleData.IncrementData.SubIncrements.getLength();
    sal_Int32 nOldSubIncCount = m_aSubGridProperties.getLength();

    if( nOldSubIncCount > nNewSubIncCount )
    {
        // remove superfluous entries
        for( sal_Int32 i = nNewSubIncCount; i < nOldSubIncCount; ++i )
            ModifyListenerHelper::removeListener( m_aSubGridProperties[ i ], m_xModifyEventForwarder );
        m_aSubGridProperties.realloc( nNewSubIncCount );
    }
    else if( nOldSubIncCount < nNewSubIncCount )
    {
        m_aSubGridProperties.realloc( nNewSubIncCount );

        // allocate new entries
        for( sal_Int32 i = nOldSubIncCount; i < nNewSubIncCount; ++i )
        {
            m_aSubGridProperties[ i ] = new GridProperties();
            LineProperties::SetLineInvisible( m_aSubGridProperties[ i ] );
            ModifyListenerHelper::addListener( m_aSubGridProperties[ i ], m_xModifyEventForwarder );
        }
    }
}

// --------------------------------------------------------------------------------

// ____ XAxis ____
void SAL_CALL Axis::setScaleData( const chart2::ScaleData& rScaleData )
    throw (uno::RuntimeException)
{
    {
        // /--
        MutexGuard aGuard( m_aMutex );
        if( m_aScaleData.Categories.is())
        {
            ModifyListenerHelper::removeListener( m_aScaleData.Categories, m_xModifyEventForwarder );
            EventListenerHelper::removeListener( m_aScaleData.Categories, this );
        }
        m_aScaleData = rScaleData;
        ModifyListenerHelper::addListener( m_aScaleData.Categories, m_xModifyEventForwarder );
        EventListenerHelper::addListener( m_aScaleData.Categories, this );

        AllocateSubGrids();
        // \--
    }
    fireModifyEvent();
}

chart2::ScaleData SAL_CALL Axis::getScaleData()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return m_aScaleData;
    // \--
}

Reference< beans::XPropertySet > SAL_CALL Axis::getGridProperties()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return m_xGrid;
    // \--
}
Sequence< Reference< beans::XPropertySet > > SAL_CALL Axis::getSubGridProperties()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return m_aSubGridProperties;
    // \--
}

Sequence< Reference< beans::XPropertySet > > SAL_CALL Axis::getSubTickProperties()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet" );
    return Sequence< Reference< beans::XPropertySet > >();
}


// ____ XTitled ____
Reference< chart2::XTitle > SAL_CALL Axis::getTitleObject()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xTitle;
    // \--
}

void SAL_CALL Axis::setTitleObject( const Reference< chart2::XTitle >& Title )
    throw (uno::RuntimeException)
{
    {
        // /--
        MutexGuard aGuard( GetMutex() );
        if( m_xTitle.is())
            ModifyListenerHelper::removeListener( m_xTitle, m_xModifyEventForwarder );
        m_xTitle = Title;
        if( m_xTitle.is())
            ModifyListenerHelper::addListener( m_xTitle, m_xModifyEventForwarder );
        // \--
    }
    fireModifyEvent();
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL Axis::createClone()
    throw (uno::RuntimeException)
{
    Axis * pNewAxis( new Axis( *this ));
    // hold a reference to the clone
    Reference< util::XCloneable > xResult( pNewAxis );
    // do initialization that uses uno references to the clone
    pNewAxis->Init( *this );
    return xResult;
}

// ____ XModifyBroadcaster ____
void SAL_CALL Axis::addModifyListener( const Reference< util::XModifyListener >& aListener )
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

void SAL_CALL Axis::removeModifyListener( const Reference< util::XModifyListener >& aListener )
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
void SAL_CALL Axis::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Axis::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    if( Source.Source == m_aScaleData.Categories )
        m_aScaleData.Categories = 0;
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

// ================================================================================

// ____ OPropertySet ____
uno::Any Axis::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        CharacterProperties::AddDefaultsToMap( aStaticDefaults );
        LineProperties::AddDefaultsToMap( aStaticDefaults );

        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL Axis::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    Axis::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ================================================================================

Sequence< OUString > Axis::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.Axis" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

using impl::Axis_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Axis, Axis_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Axis, Axis_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Axis, lcl_aServiceName );

} //  namespace chart
