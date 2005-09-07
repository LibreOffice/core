/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LegendWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:01:53 $
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
#include "LegendWrapper.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "algohelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTLEGENDPOSITION_HPP_
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <com/sun/star/chart2/LegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_LEGENDEXPANSION_HPP_
#include <com/sun/star/chart2/LegendExpansion.hpp>
#endif

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"

#include <algorithm>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::property::OPropertySet;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Title" ));

enum
{
    PROP_LEGEND_ALIGNMENT
};

typedef ::std::map< sal_Int32, ::rtl::OUString > lcl_PropertyMapType;
typedef ::comphelper::MakeMap< sal_Int32, ::rtl::OUString > lcl_MakePropertyMapType;

lcl_PropertyMapType & lcl_GetPropertyMap()
{
    static lcl_PropertyMapType aMap(
        lcl_MakePropertyMapType
        ( PROP_LEGEND_ALIGNMENT,           C2U( "Alignment" ))
        );

    return aMap;
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Alignment" ),
                  PROP_LEGEND_ALIGNMENT,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartLegendPosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LEGEND_ALIGNMENT ));
    rOutMap[ PROP_LEGEND_ALIGNMENT ] =
        uno::makeAny( ::com::sun::star::chart::ChartLegendPosition_RIGHT );
}

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::LineProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::FillProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
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

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

LegendWrapper::LegendWrapper(
    const uno::Reference<
        ::com::sun::star::chart2::XChartDocument > & xModel,
    const uno::Reference< uno::XComponentContext > & xContext,
    ::osl::Mutex & _rMutex ) :
        OPropertySet( _rMutex ),
        m_rMutex( _rMutex ),
        m_xContext( xContext ),
        m_aEventListenerContainer( _rMutex ),
        m_xChartDoc( xModel )
{
    if( m_xChartDoc.is())
    {
        uno::Reference< chart2::XDiagram > xDia( m_xChartDoc->getDiagram());
        if( xDia.is())
            m_xLegend.set( xDia->getLegend());
    }

    m_xLegendProp.set( m_xLegend, uno::UNO_QUERY );
    m_xLegendFastProp.set( m_xLegend, uno::UNO_QUERY );
}

LegendWrapper::~LegendWrapper()
{}

::osl::Mutex & LegendWrapper::GetMutex() const
{
    return m_rMutex;
}

// ____ XShape ____
awt::Point SAL_CALL LegendWrapper::getPosition()
    throw (uno::RuntimeException)
{
    return awt::Point( 0, 0 );
}

void SAL_CALL LegendWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
}

awt::Size SAL_CALL LegendWrapper::getSize()
    throw (uno::RuntimeException)
{
    return awt::Size( 0, 0 );
}

void SAL_CALL LegendWrapper::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set size of title" );
}

// ____ XShapeDescriptor (base of XShape) ____
::rtl::OUString SAL_CALL LegendWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.ChartLegend" );
}

// ____ XComponent ____
void SAL_CALL LegendWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( *this ) );

    // /--
    MutexGuard aGuard( GetMutex());
    m_xChartDoc = NULL;
    m_xLegend = NULL;
    m_xLegendProp = NULL;
    m_xLegendFastProp = NULL;
    // \--
}

void SAL_CALL LegendWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL LegendWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

sal_Bool SAL_CALL LegendWrapper::convertFastPropertyValue
    ( uno::Any & rConvertedValue,
      uno::Any & rOldValue,
      sal_Int32 nHandle,
      const uno::Any& rValue )
    throw (lang::IllegalArgumentException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    switch( nHandle )
    {
        case PROP_LEGEND_ALIGNMENT:
        {
            getFastPropertyValue( rOldValue, nHandle );

            ::com::sun::star::chart::ChartLegendPosition ePos;
            chart2::LegendPosition eNewPos = chart2::LegendPosition_LINE_END;

            if( rValue >>= ePos )
            {
                switch( ePos )
                {
                    case ::com::sun::star::chart::ChartLegendPosition_LEFT:
                        eNewPos = chart2::LegendPosition_LINE_START;
                        break;
                    case ::com::sun::star::chart::ChartLegendPosition_RIGHT:
                        eNewPos = chart2::LegendPosition_LINE_END;
                        break;
                    case ::com::sun::star::chart::ChartLegendPosition_TOP:
                        eNewPos = chart2::LegendPosition_PAGE_START;
                        break;
                    case ::com::sun::star::chart::ChartLegendPosition_BOTTOM:
                        eNewPos = chart2::LegendPosition_PAGE_END;
                        break;

                    default: // NONE
                        break;
                }
                rConvertedValue <<= eNewPos;
                return sal_True;
            }
            break;
        }

        default:
            return OPropertySet::convertFastPropertyValue(
                rConvertedValue, rOldValue, nHandle, rValue );
    }

    rConvertedValue = rValue;

    return sal_True;
    // \--
}

void SAL_CALL LegendWrapper::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle,
      const uno::Any& rValue )
    throw (uno::Exception)
{
    // /--
    MutexGuard aGuard( GetMutex());

    // try same handle for FastPropertySet.  Caution!  Works for global
    // properties like FillProperties, LineProperties and CharacterProperties
    if( nHandle > FAST_PROPERTY_ID_START )
    {
        if( m_xLegendFastProp.is() )
            m_xLegendFastProp->setFastPropertyValue( nHandle, rValue );
    }
    else
    {
        switch( nHandle )
        {
            case PROP_LEGEND_ALIGNMENT:
                m_xLegendProp->setPropertyValue( C2U( "AnchorPosition" ), rValue );
                chart2::LegendPosition ePos;
                if( rValue >>= ePos )
                {
                    chart2::LegendExpansion eExp = chart2::LegendExpansion_WIDE;
                    if( ePos == chart2::LegendPosition_LINE_END ||
                        ePos == chart2::LegendPosition_LINE_START )
                        eExp = chart2::LegendExpansion_HIGH;

                    m_xLegendProp->setPropertyValue( C2U( "Expansion" ), uno::makeAny( eExp ));
                }
        }
    }
    // \--
}

void SAL_CALL LegendWrapper::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // /--
    MutexGuard aGuard( GetMutex());
    switch( nHandle )
    {
        case PROP_LEGEND_ALIGNMENT:
            if( m_xLegendProp.is())
            {
                ::com::sun::star::chart::ChartLegendPosition ePos;
                chart2::LegendPosition eNewPos;

                if( m_xLegendProp->getPropertyValue( C2U( "AnchorPosition" )) >>= eNewPos )
                {
                    switch( eNewPos )
                    {
                        case chart2::LegendPosition_LINE_START:
                            ePos = ::com::sun::star::chart::ChartLegendPosition_LEFT;
                            break;
                        case chart2::LegendPosition_LINE_END:
                            ePos = ::com::sun::star::chart::ChartLegendPosition_RIGHT;
                            break;
                        case chart2::LegendPosition_PAGE_START:
                            ePos = ::com::sun::star::chart::ChartLegendPosition_TOP;
                            break;
                        case chart2::LegendPosition_PAGE_END:
                            ePos = ::com::sun::star::chart::ChartLegendPosition_BOTTOM;
                            break;

                        default:
                            ePos = ::com::sun::star::chart::ChartLegendPosition_NONE;
                            break;
                    }
                    rValue <<= ePos;
                }
            }
            break;

        default:
            if( m_xLegendFastProp.is())
            {
                // Note: handles must be identical ! (Is the case with the
                // shared helpers for Character-, Line- and FillProperties
                rValue = m_xLegendFastProp->getFastPropertyValue( nHandle );
            }
            else
                OPropertySet::getFastPropertyValue( rValue, nHandle );
            break;
    }
    // \--
}

// --------------------------------------------------------------------------------

// ____ OPropertySet ____
uno::Any LegendWrapper::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
        CharacterProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        LineProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        FillProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL LegendWrapper::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    LegendWrapper::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

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

uno::Sequence< ::rtl::OUString > LegendWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartLegend" );
    aServices[ 1 ] = C2U( "com.sun.star.drawing.Shape" );
    aServices[ 2 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 3 ] = C2U( "com.sun.star.style.CharacterProperties" );
//     aServices[ 4 ] = C2U( "com.sun.star.beans.PropertySet" );
//     aServices[ 5 ] = C2U( "com.sun.star.drawing.FillProperties" );
//     aServices[ 6 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LegendWrapper, lcl_aServiceName );

// needed by MSC compiler
using impl::LegendWrapper_Base;

IMPLEMENT_FORWARD_XINTERFACE2( LegendWrapper, LegendWrapper_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( LegendWrapper, LegendWrapper_Base, OPropertySet )

} //  namespace wrapper
} //  namespace chart
