/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesPointWrapper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:00:51 $
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
#include "DataSeriesPointWrapper.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "algohelper.hxx"
#include "RegressionCurveHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISASSIGN_HPP_
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATACAPTION_HPP_
#include <com/sun/star/chart/ChartDataCaption.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_TRANSPARENCYSTYLE_HPP_
#include <com/sun/star/chart2/TransparencyStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_CHARTERRORCATEGORY_HPP_
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORINDICATORTYPE_HPP_
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTREGRESSIONCURVETYPE_HPP_
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#endif

#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "CharacterProperties.hxx"
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
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.DataSeries" ));

enum
{
    PROP_SERIES_ATTACHED_AXIS,
    PROP_SERIES_DATA_CAPTION,

    PROP_SERIES_STAT_BEGIN,
    PROP_SERIES_CONST_ERROR_LOW = PROP_SERIES_STAT_BEGIN,
    PROP_SERIES_CONST_ERROR_HIGH,
    PROP_SERIES_MEAN_VALUE,
    PROP_SERIES_ERROR_CATEGORY,
    PROP_SERIES_PERCENT_ERROR,
    PROP_SERIES_ERROR_MARGIN,
    PROP_SERIES_ERROR_INDICATOR,
    PROP_SERIES_REGRESSION_CURVES,
    PROP_SERIES_STAT_END = PROP_SERIES_REGRESSION_CURVES
};

typedef ::std::map< sal_Int32, ::rtl::OUString > lcl_PropertyMapType;
typedef ::comphelper::MakeMap< sal_Int32, ::rtl::OUString > lcl_MakePropertyMapType;

lcl_PropertyMapType & lcl_GetPropertyMapLine()
{
    static lcl_PropertyMapType aMap(
        lcl_MakePropertyMapType
        ( ::chart::LineProperties::PROP_LINE_TRANSPARENCE,      C2U( "Transparency" ))
        ( ::chart::LineProperties::PROP_LINE_COLOR,             C2U( "Color" ))
        ( ::chart::LineProperties::PROP_LINE_STYLE,             C2U( "LineStyle" ))
        );

    return aMap;
};

lcl_PropertyMapType & lcl_GetPropertyMapFilled()
{
    static lcl_PropertyMapType aMap(
        lcl_MakePropertyMapType
        // Note: FillStyle has same name but must be mapped (ids differ)
        ( ::chart::FillProperties::PROP_FILL_STYLE,             C2U( "FillStyle" ))
        ( ::chart::FillProperties::PROP_FILL_COLOR,             C2U( "Color" ) )
        ( ::chart::FillProperties::PROP_FILL_TRANSPARENCE,      C2U( "Transparency" ))
        ( ::chart::FillProperties::PROP_FILL_GRADIENT,          C2U( "Gradient" ))
        ( ::chart::FillProperties::PROP_FILL_HATCH,             C2U( "Hatch" ))

        ( ::chart::LineProperties::PROP_LINE_STYLE,             C2U( "BorderStyle" ))
        ( ::chart::LineProperties::PROP_LINE_COLOR,             C2U( "BorderColor" ))
        ( ::chart::LineProperties::PROP_LINE_WIDTH,             C2U( "BorderWidth" ))
        );

    return aMap;
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Axis" ),
                  PROP_SERIES_ATTACHED_AXIS,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "DataCaption" ),
                  PROP_SERIES_DATA_CAPTION,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Statistics
    rOutProperties.push_back(
        Property( C2U( "ConstantErrorLow" ),
                  PROP_SERIES_CONST_ERROR_LOW,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ConstantErrorHigh" ),
                  PROP_SERIES_CONST_ERROR_HIGH,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "MeanValue" ),
                  PROP_SERIES_MEAN_VALUE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ErrorCategory" ),
                  PROP_SERIES_ERROR_CATEGORY,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartErrorCategory * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "PercentageError" ),
                  PROP_SERIES_PERCENT_ERROR,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ErrorMargin" ),
                  PROP_SERIES_ERROR_MARGIN,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ErrorIndicator" ),
                  PROP_SERIES_ERROR_INDICATOR,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartErrorIndicatorType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "RegressionCurves" ),
                  PROP_SERIES_REGRESSION_CURVES,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartRegressionCurveType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SERIES_ATTACHED_AXIS ));
    rOutMap[ PROP_SERIES_ATTACHED_AXIS ] =
        uno::makeAny( sal_Int32( ::com::sun::star::chart::ChartAxisAssign::PRIMARY_Y ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SERIES_DATA_CAPTION ));
    rOutMap[ PROP_SERIES_DATA_CAPTION ] =
        uno::makeAny( sal_Int32( ::com::sun::star::chart::ChartDataCaption::NONE ) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SERIES_CONST_ERROR_LOW ));
    rOutMap[ PROP_SERIES_CONST_ERROR_LOW ] =
        uno::makeAny( double( 0.0 ));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SERIES_CONST_ERROR_HIGH ));
    rOutMap[ PROP_SERIES_CONST_ERROR_HIGH ] =
        uno::makeAny( double( 0.0 ));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SERIES_MEAN_VALUE ));
    rOutMap[ PROP_SERIES_MEAN_VALUE ] =
        uno::makeAny( sal_False );
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
        ::chart::LineProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::FillProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::CharacterProperties::AddPropertiesToVector(
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

DataSeriesPointWrapper::DataSeriesPointWrapper(
    eParentType eType,
    const uno::Reference< beans::XPropertySet > & xParentProperties,
    const uno::Reference< uno::XComponentContext > & xContext,
    ::osl::Mutex & _rMutex ) :
        OPropertySet( _rMutex ),
        m_rMutex( _rMutex ),
        m_xContext( xContext ),
        m_aEventListenerContainer( _rMutex ),
        m_eParentType( eType ),
        m_xParentProperties( xParentProperties ),
        m_xParentFastProperties( xParentProperties, uno::UNO_QUERY )
{}

DataSeriesPointWrapper::~DataSeriesPointWrapper()
{}

::osl::Mutex & DataSeriesPointWrapper::GetMutex() const
{
    return m_rMutex;
}

// ____ XComponent ____
void SAL_CALL DataSeriesPointWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( *this ) );

    // /--
    MutexGuard aGuard( GetMutex());
    m_xParentProperties = NULL;
    m_xParentFastProperties = NULL;
    // \--
}

void SAL_CALL DataSeriesPointWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL DataSeriesPointWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ____ XEventListener ____
void SAL_CALL DataSeriesPointWrapper::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
}


sal_Bool SAL_CALL DataSeriesPointWrapper::convertFastPropertyValue
    ( uno::Any & rConvertedValue,
      uno::Any & rOldValue,
      sal_Int32 nHandle,
      const uno::Any& rValue )
    throw (lang::IllegalArgumentException)
{
    getFastPropertyValue( rOldValue, nHandle );

    switch( nHandle )
    {
        case PROP_SERIES_DATA_CAPTION:
        {
            sal_Int32 nCaptionStyle;
            if( rValue >>= nCaptionStyle )
            {
                chart2::DataPointLabel aConvertedLabel;

                if( nCaptionStyle & ::com::sun::star::chart::ChartDataCaption::VALUE )
                    aConvertedLabel.ShowNumber = true;
                if( nCaptionStyle & ::com::sun::star::chart::ChartDataCaption::PERCENT )
                    aConvertedLabel.ShowNumberInPercent = true;
                if( nCaptionStyle & ::com::sun::star::chart::ChartDataCaption::TEXT )
                    aConvertedLabel.ShowCategoryName = true;
                if( nCaptionStyle & ::com::sun::star::chart::ChartDataCaption::SYMBOL )
                    aConvertedLabel.ShowLegendSymbol = true;

                rConvertedValue <<= aConvertedLabel;
            }
            else
                return sal_False;
        }
        break;

        default:
            rConvertedValue = rValue;
            break;
    }

    return sal_True;
}

void SAL_CALL DataSeriesPointWrapper::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // /--
    MutexGuard aGuard( GetMutex());
    if( ! m_xParentProperties.is())
        return;

    if( nHandle >= PROP_SERIES_STAT_BEGIN &&
        nHandle <= PROP_SERIES_STAT_END )
    {
        getStatisticsPropertyValue( rValue, nHandle );
        return;
    }

    lcl_PropertyMapType & rMap( lcl_GetPropertyMapFilled());
    lcl_PropertyMapType::const_iterator aIt( rMap.find( nHandle ));
    if( aIt == rMap.end())
    {
        OPropertySet::getFastPropertyValue( rValue, nHandle );
    }
    else
    {
        rValue = m_xParentProperties->getPropertyValue( (*aIt).second );
    }
    // \--
}

void SAL_CALL DataSeriesPointWrapper::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    // /--
    MutexGuard aGuard( GetMutex());

    if( m_xParentProperties.is())
    {
        if( nHandle >= PROP_SERIES_STAT_BEGIN &&
            nHandle <= PROP_SERIES_STAT_END )
        {
            setStatisticsPropertyValue_NoBroadcast( nHandle, rValue );
            return;
        }

        lcl_PropertyMapType & rMap( lcl_GetPropertyMapFilled());
        lcl_PropertyMapType::const_iterator aIt( rMap.find( nHandle ));

        if( aIt != rMap.end())
        {
            // found in map
            m_xParentProperties->setPropertyValue( (*aIt).second, rValue );
        }
        else
        {
            // try same handle for FastPropertySet.  Caution!  Works for global
            // properties like FillProperties, LineProperties and
            // CharacterProperties
            if( m_xParentFastProperties.is() && nHandle > FAST_PROPERTY_ID_START )
                m_xParentFastProperties->setFastPropertyValue( nHandle, rValue );
            else
            {
                // get name for property handle
                try
                {
                    ::rtl::OUString aPropName;
                    getInfoHelper().fillPropertyMembersByHandle( & aPropName, NULL, nHandle );
                    m_xParentProperties->setPropertyValue( aPropName, rValue );
                }
                catch( beans::UnknownPropertyException ex )
                {
                    OPropertySet::setFastPropertyValue_NoBroadcast( nHandle, rValue );
                }
            }

            if( nHandle == FillProperties::PROP_FILL_TRANSPARENCE )
            {
                m_xParentProperties->setPropertyValue(
                    C2U( "TransparencyStyle" ),
                    uno::makeAny( chart2::TransparencyStyle_LINEAR ));
            }
        }
    }
    else
        OPropertySet::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    // \--
}

void SAL_CALL DataSeriesPointWrapper::getStatisticsPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    OSL_ASSERT( m_xParentProperties.is());

    if( nHandle == PROP_SERIES_REGRESSION_CURVES ||
        nHandle == PROP_SERIES_MEAN_VALUE )
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegCont( m_xParentProperties, uno::UNO_QUERY );
        if( xRegCont.is())
        {
            if( nHandle == PROP_SERIES_MEAN_VALUE )
                rValue <<= RegressionCurveHelper::hasMeanValueLine( xRegCont );
            else
            {
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xErrorBarProp;
        if(( m_xParentProperties->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProp ) &&
           xErrorBarProp.is())
        {
            switch( nHandle )
            {
                case PROP_SERIES_CONST_ERROR_LOW:
                case PROP_SERIES_CONST_ERROR_HIGH:
                case PROP_SERIES_ERROR_CATEGORY:
                case PROP_SERIES_PERCENT_ERROR:
                case PROP_SERIES_ERROR_MARGIN:
                case PROP_SERIES_ERROR_INDICATOR:
                    break;
            }
        }
    }
}

void SAL_CALL DataSeriesPointWrapper::setStatisticsPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{    OSL_ASSERT( m_xParentProperties.is());

    if( nHandle == PROP_SERIES_REGRESSION_CURVES ||
        nHandle == PROP_SERIES_MEAN_VALUE )
    {
        uno::Reference< chart2::XRegressionCurveContainer > xRegCont( m_xParentProperties, uno::UNO_QUERY );
        if( xRegCont.is())
        {
            sal_Bool bValue;
            if( (nHandle == PROP_SERIES_MEAN_VALUE) &&
                (rValue >>= bValue))
            {
                bool bHasMVL = RegressionCurveHelper::hasMeanValueLine( xRegCont );
                if( bHasMVL && ! bValue )
                    RegressionCurveHelper::removeMeanValueLine( xRegCont );
                else if( ! bHasMVL && bValue )
                    RegressionCurveHelper::addMeanValueLine(
                        xRegCont, uno::Reference< uno::XComponentContext >(), m_xParentProperties );
            }
            else
            {
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xErrorBarProp;
        if(( m_xParentProperties->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProp ) &&
           xErrorBarProp.is())
        {
            switch( nHandle )
            {
                case PROP_SERIES_CONST_ERROR_LOW:
                case PROP_SERIES_CONST_ERROR_HIGH:
                case PROP_SERIES_ERROR_CATEGORY:
                case PROP_SERIES_PERCENT_ERROR:
                case PROP_SERIES_ERROR_MARGIN:
                case PROP_SERIES_ERROR_INDICATOR:
                    break;
            }
        }
    }
}

// --------------------------------------------------------------------------------

// ____ OPropertySet ____
uno::Any DataSeriesPointWrapper::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
        LineProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        FillProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        CharacterProperties::AddDefaultsToMap(
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

::cppu::IPropertyArrayHelper & SAL_CALL DataSeriesPointWrapper::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    DataSeriesPointWrapper::getPropertySetInfo()
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

uno::Sequence< ::rtl::OUString > DataSeriesPointWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 7 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartDataRowProperties" );
    aServices[ 1 ] = C2U( "com.sun.star.chart.ChartDataPointProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 4 ] = C2U( "com.sun.star.drawing.FillProperties" );
    aServices[ 5 ] = C2U( "com.sun.star.drawing.LineProperties" );
    aServices[ 6 ] = C2U( "com.sun.star.drawing.CharacterProperties" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DataSeriesPointWrapper, lcl_aServiceName );

// needed by MSC compiler
using impl::DataSeriesPointWrapper_Base;

IMPLEMENT_FORWARD_XINTERFACE2( DataSeriesPointWrapper, DataSeriesPointWrapper_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataSeriesPointWrapper, DataSeriesPointWrapper_Base, OPropertySet )

} //  namespace wrapper
} //  namespace chart
