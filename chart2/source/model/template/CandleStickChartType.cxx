/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CandleStickChartType.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:04:38 $
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

#include "CandleStickChartType.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "StockBar.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_CANDLESTICKCHARTTYPE_JAPANESE,
    PROP_CANDLESTICKCHARTTYPE_WHITE_DAY,
    PROP_CANDLESTICKCHARTTYPE_BLACK_DAY,

    PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST,
    PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Japanese" ),
                  PROP_CANDLESTICKCHARTTYPE_JAPANESE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "WhiteDay" ),
                  PROP_CANDLESTICKCHARTTYPE_WHITE_DAY,
                  ::getCppuType( reinterpret_cast< Reference< beans::XPropertySet > *>(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "BlackDay" ),
                  PROP_CANDLESTICKCHARTTYPE_BLACK_DAY,
                  ::getCppuType( reinterpret_cast< Reference< beans::XPropertySet > *>(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "ShowFirst" ),
                  PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ShowHighLow" ),
                  PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap,
    ::osl::Mutex & rMutex )
{
    ::osl::MutexGuard aGuard( rMutex );
    // must match default in CTOR!
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CANDLESTICKCHARTTYPE_JAPANESE, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW, true );
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

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

} // anonymous namespace

namespace chart
{

CandleStickChartType::CandleStickChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{
    Reference< beans::XPropertySet > xWhiteDayProps( new ::chart::StockBar( true ));
    Reference< beans::XPropertySet > xBlackDayProps( new ::chart::StockBar( false ));

    ModifyListenerHelper::addListener( xWhiteDayProps, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( xBlackDayProps, m_xModifyEventForwarder );

    setFastPropertyValue_NoBroadcast(
        PROP_CANDLESTICKCHARTTYPE_WHITE_DAY, uno::makeAny( xWhiteDayProps ));
    setFastPropertyValue_NoBroadcast(
        PROP_CANDLESTICKCHARTTYPE_BLACK_DAY, uno::makeAny( xBlackDayProps ));
}

CandleStickChartType::CandleStickChartType( const CandleStickChartType & rOther ) :
        ChartType( rOther )
{
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_WHITE_DAY );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_BLACK_DAY );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );
}

CandleStickChartType::~CandleStickChartType()
{
    try
    {
        Reference< beans::XPropertySet > xPropertySet;
        uno::Any aValue;

        getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_WHITE_DAY );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );

        getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_BLACK_DAY );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL CandleStickChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new CandleStickChartType( *this ));
}

// ____ XChartType ____
::rtl::OUString SAL_CALL CandleStickChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK;
}

uno::Sequence< ::rtl::OUString > SAL_CALL CandleStickChartType::getSupportedMandatoryRoles()
    throw (uno::RuntimeException)
{
    bool bShowFirst = true;
    bool bShowHiLow = false;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST ) >>= bShowFirst;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW ) >>= bShowHiLow;

    ::std::vector< OUString > aMandRoles;

    aMandRoles.push_back( C2U( "label" ));
    if( bShowFirst )
        aMandRoles.push_back( C2U( "values-first" ));

    if( bShowHiLow )
    {
        aMandRoles.push_back( C2U( "values-min" ));
        aMandRoles.push_back( C2U( "values-max" ));
    }

    aMandRoles.push_back( C2U( "values-last" ));

    return ContainerHelper::ContainerToSequence( aMandRoles );
}

Sequence< OUString > SAL_CALL CandleStickChartType::getSupportedOptionalRoles()
    throw (uno::RuntimeException)
{
    bool bShowFirst = true;
    bool bShowHiLow = false;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST ) >>= bShowFirst;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW ) >>= bShowHiLow;

    ::std::vector< OUString > aOptRoles;

    if( ! bShowFirst )
        aOptRoles.push_back( C2U( "values-first" ));

    if( ! bShowHiLow )
    {
        aOptRoles.push_back( C2U( "values-min" ));
        aOptRoles.push_back( C2U( "values-max" ));
    }

    return ContainerHelper::ContainerToSequence( aOptRoles );
}

OUString SAL_CALL CandleStickChartType::getRoleOfSequenceForSeriesLabel()
    throw (uno::RuntimeException)
{
    return C2U( "values-last" );
}

// ____ OPropertySet ____
uno::Any CandleStickChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults, GetMutex() );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL CandleStickChartType::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper( lcl_GetPropertySequence(),
                                                      /* bSorted = */ sal_True );

    return aArrayHelper;
}


// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    CandleStickChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

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

void SAL_CALL CandleStickChartType::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    if(    nHandle == PROP_CANDLESTICKCHARTTYPE_WHITE_DAY
        || nHandle == PROP_CANDLESTICKCHARTTYPE_BLACK_DAY )
    {
        uno::Any aOldValue;
        Reference< util::XModifyBroadcaster > xBroadcaster;
        this->getFastPropertyValue( aOldValue, nHandle );
        if( aOldValue.hasValue() &&
            (aOldValue >>= xBroadcaster) &&
            xBroadcaster.is())
        {
            ModifyListenerHelper::removeListener( xBroadcaster, m_xModifyEventForwarder );
        }

        OSL_ASSERT( rValue.getValueType().getTypeClass() == uno::TypeClass_INTERFACE );
        if( rValue.hasValue() &&
            (rValue >>= xBroadcaster) &&
            xBroadcaster.is())
        {
            ModifyListenerHelper::addListener( xBroadcaster, m_xModifyEventForwarder );
        }
    }

    ::property::OPropertySet::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

uno::Sequence< ::rtl::OUString > CandleStickChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( CandleStickChartType,
                             C2U( "com.sun.star.comp.chart.CandleStickChartType" ));

} //  namespace chart
