/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AxisWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:59:42 $
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
#include "AxisWrapper.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "algohelper.hxx"
#include "MeterHelper.hxx"
#include "Scaling.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "UserDefinedProperties.hxx"

#include <algorithm>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::property::OPropertySet;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Axis" ));

enum
{
    PROP_AXIS_MAX,
    PROP_AXIS_MIN,
    PROP_AXIS_ORIGIN,
    PROP_AXIS_STEPMAIN,
    PROP_AXIS_STEPHELP,
    PROP_AXIS_AUTO_MAX,
    PROP_AXIS_AUTO_MIN,
    PROP_AXIS_AUTO_ORIGIN,
    PROP_AXIS_AUTO_STEPMAIN,
    PROP_AXIS_AUTO_STEPHELP,
    PROP_AXIS_LOGARITHMIC,
    PROP_AXIS_DISPLAY_LABELS,
    PROP_AXIS_TEXT_ROTATION,
    PROP_AXIS_MARKS,
    PROP_AXIS_HELPMARKS,
    PROP_AXIS_OVERLAP,
    PROP_AXIS_GAP_WIDTH,
    PROP_AXIS_ARRANGE_ORDER,
    PROP_AXIS_TEXTBREAK,
    PROP_AXIS_CAN_OVERLAP,
    PROP_AXIS_NUMBERFORMAT
};

typedef ::std::map< sal_Int32, ::rtl::OUString > lcl_PropertyMapType;
typedef ::comphelper::MakeMap< sal_Int32, ::rtl::OUString > lcl_MakePropertyMapType;

lcl_PropertyMapType & lcl_GetPropertyMap()
{
    static lcl_PropertyMapType aMap(
        lcl_MakePropertyMapType
        ( PROP_AXIS_DISPLAY_LABELS,           C2U( "DisplayLabels" ))
        ( PROP_AXIS_TEXT_ROTATION,            C2U( "TextRotation" ))
        // preliminary
        ( PROP_AXIS_MARKS,                    C2U( "MajorTickmarks" ))
        // preliminary
        ( PROP_AXIS_HELPMARKS,                C2U( "MinorTickmarks" ))
        ( PROP_AXIS_ARRANGE_ORDER,            C2U( "ArrangeOrder" ))
        ( PROP_AXIS_TEXTBREAK,                C2U( "TextBreak" ))
        ( PROP_AXIS_CAN_OVERLAP,              C2U( "TextOverlap" ))
        // has to be converted
        ( PROP_AXIS_NUMBERFORMAT,             C2U( "NumberFormat" ))
        );

    return aMap;
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Max" ),
                  PROP_AXIS_MAX,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Min" ),
                  PROP_AXIS_MIN,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "Origin" ),
                  PROP_AXIS_ORIGIN,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "StepMain" ),
                  PROP_AXIS_STEPMAIN,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "StepHelp" ),
                  PROP_AXIS_STEPHELP,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "AutoMax" ),
                  PROP_AXIS_AUTO_MAX,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoMin" ),
                  PROP_AXIS_AUTO_MIN,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoOrigin" ),
                  PROP_AXIS_AUTO_ORIGIN,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoStepMain" ),
                  PROP_AXIS_AUTO_STEPMAIN,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AutoStepHelp" ),
                  PROP_AXIS_AUTO_STEPHELP,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Logarithmic" ),
                  PROP_AXIS_LOGARITHMIC,
                  ::getBooleanCppuType(),
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
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Marks" ),
                  PROP_AXIS_MARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "HelpMarks" ),
                  PROP_AXIS_HELPMARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Overlap" ),
                  PROP_AXIS_OVERLAP,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Gapwidth" ),
                  PROP_AXIS_GAP_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ArrangeOrder" ),
                  PROP_AXIS_ARRANGE_ORDER,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisArrangeOrderType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextBreak" ),
                  PROP_AXIS_TEXTBREAK,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextCanOverlap" ),
                  PROP_AXIS_CAN_OVERLAP,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_AXIS_NUMBERFORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{}

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

AxisWrapper::AxisWrapper(
    eAxisType eType,
    const uno::Reference<
        ::com::sun::star::chart2::XDiagram > & xDia,
    const uno::Reference< uno::XComponentContext > & xContext,
    ::osl::Mutex & _rMutex ) :
        OPropertySet( _rMutex ),
        m_rMutex( _rMutex ),
        m_xContext( xContext ),
        m_eType( eType ),
        m_aEventListenerContainer( _rMutex ),
        m_xDiagram( xDia )
{
    try
    {
        uno::Reference< chart2::XAxisContainer > xAxisCnt( m_xDiagram, uno::UNO_QUERY_THROW );
        ::rtl::OUString aId;

        switch( eType )
        {
            case X_AXIS:
                aId = MeterHelper::makeAxisIdentifier( 0, 0 );  break;
            case Y_AXIS:
                aId = MeterHelper::makeAxisIdentifier( 1, 0 );  break;
            case Z_AXIS:
                aId = MeterHelper::makeAxisIdentifier( 2, 0 );  break;
            case SECOND_X_AXIS:
                aId = MeterHelper::makeAxisIdentifier( 0, 1 );  break;
            case SECOND_Y_AXIS:
                aId = MeterHelper::makeAxisIdentifier( 1, 1 );  break;
        }

        if( aId.getLength() > 0 )
            m_xAxis = xAxisCnt->getAxisByIdentifier( aId );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    m_xAxisProperties.set( m_xAxis, uno::UNO_QUERY );
    m_xAxisFastProperties.set( m_xAxis, uno::UNO_QUERY );
}

AxisWrapper::~AxisWrapper()
{}

::osl::Mutex & AxisWrapper::GetMutex() const
{
    return m_rMutex;
}

// ____ XComponent ____
void SAL_CALL AxisWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( *this ) );

    // /--
    MutexGuard aGuard( GetMutex());
    m_xDiagram = NULL;
    m_xAxis = NULL;
    m_xAxisProperties = NULL;
    m_xAxisFastProperties = NULL;
    // \--
}

void SAL_CALL AxisWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL AxisWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

sal_Bool SAL_CALL AxisWrapper::convertFastPropertyValue
    ( uno::Any & rConvertedValue,
      uno::Any & rOldValue,
      sal_Int32 nHandle,
      const uno::Any& rValue )
    throw (lang::IllegalArgumentException)
{
    switch( nHandle )
    {
        case PROP_AXIS_TEXT_ROTATION:
        {
            // /--
            MutexGuard aGuard( GetMutex());
            if( m_xAxisProperties.is())
                getFastPropertyValue( rOldValue, nHandle );

            sal_Int32 nVal;
            if( rValue >>= nVal )
            {
                double fDoubleDegrees = ( static_cast< double >( nVal ) / 100.0 );
                rConvertedValue <<= fDoubleDegrees;
                return sal_True;
            }
            break;
            // \--
        }

        default:
            return OPropertySet::convertFastPropertyValue(
                rConvertedValue, rOldValue, nHandle, rValue );
    }

    return sal_False;
}

void SAL_CALL AxisWrapper::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    // /--
    MutexGuard aGuard( GetMutex());

    // try same handle for FastPropertySet.  Caution!  Works for global
    // properties like FillProperties, LineProperties and CharacterProperties
    if( nHandle > FAST_PROPERTY_ID_START )
    {
        if( m_xAxisFastProperties.is() )
            m_xAxisFastProperties->setFastPropertyValue( nHandle, rValue );
    }
    else
    {
        switch( nHandle )
        {
            case PROP_AXIS_MAX:
            case PROP_AXIS_MIN:
            case PROP_AXIS_ORIGIN:
            case PROP_AXIS_STEPMAIN:
            case PROP_AXIS_STEPHELP:
            case PROP_AXIS_AUTO_MAX:
            case PROP_AXIS_AUTO_MIN:
            case PROP_AXIS_AUTO_ORIGIN:
            case PROP_AXIS_AUTO_STEPMAIN:
            case PROP_AXIS_AUTO_STEPHELP:
            case PROP_AXIS_LOGARITHMIC:
                if( m_xAxis.is())
                    setFastMeterPropertyValue_NoBroadcast( nHandle, rValue );
                break;

            case PROP_AXIS_DISPLAY_LABELS:
            case PROP_AXIS_TEXT_ROTATION:
            case PROP_AXIS_MARKS:
            case PROP_AXIS_HELPMARKS:
            case PROP_AXIS_OVERLAP:
            case PROP_AXIS_GAP_WIDTH:
            case PROP_AXIS_ARRANGE_ORDER:
            case PROP_AXIS_TEXTBREAK:
            case PROP_AXIS_CAN_OVERLAP:
            case PROP_AXIS_NUMBERFORMAT:
                if( m_xAxisProperties.is())
                {
                    lcl_PropertyMapType & rMap( lcl_GetPropertyMap());
                    lcl_PropertyMapType::const_iterator aIt( rMap.find( nHandle ));

                    if( aIt != rMap.end())
                    {
                        // found in map
                        m_xAxisProperties->setPropertyValue( (*aIt).second, rValue );
                    }
                }
                break;
        }
    }
    // \--
}

void SAL_CALL AxisWrapper::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // /--
    MutexGuard aGuard( GetMutex());
    if( ! m_xAxis.is())
        return;

    // try same handle for FastPropertySet.  Caution!  Works for global
    // properties like FillProperties, LineProperties and CharacterProperties
    if( nHandle > FAST_PROPERTY_ID_START )
    {
        if( m_xAxisFastProperties.is() )
            rValue = m_xAxisFastProperties->getFastPropertyValue( nHandle );
    }
    else
    {
        switch( nHandle )
        {
            case PROP_AXIS_MAX:
            case PROP_AXIS_MIN:
            case PROP_AXIS_ORIGIN:
            case PROP_AXIS_STEPMAIN:
            case PROP_AXIS_STEPHELP:
            case PROP_AXIS_AUTO_MAX:
            case PROP_AXIS_AUTO_MIN:
            case PROP_AXIS_AUTO_ORIGIN:
            case PROP_AXIS_AUTO_STEPMAIN:
            case PROP_AXIS_AUTO_STEPHELP:
            case PROP_AXIS_LOGARITHMIC:
                getFastMeterPropertyValue( rValue, nHandle );
                break;

            case PROP_AXIS_DISPLAY_LABELS:
            case PROP_AXIS_MARKS:
            case PROP_AXIS_HELPMARKS:
            case PROP_AXIS_OVERLAP:
            case PROP_AXIS_GAP_WIDTH:
            case PROP_AXIS_ARRANGE_ORDER:
            case PROP_AXIS_TEXTBREAK:
            case PROP_AXIS_CAN_OVERLAP:
            case PROP_AXIS_NUMBERFORMAT:
                if( m_xAxisProperties.is())
                {
                    lcl_PropertyMapType & rMap( lcl_GetPropertyMap());
                    lcl_PropertyMapType::const_iterator aIt( rMap.find( nHandle ));

                    if( aIt != rMap.end())
                    {
                        // found in map
                        rValue = m_xAxisProperties->getPropertyValue( (*aIt).second );
                    }
                }
                break;

            case PROP_AXIS_TEXT_ROTATION:
                if( m_xAxisProperties.is())
                {
                    double fDoubleDegrees = 0.0;
                    uno::Any aAny( m_xAxisProperties->getPropertyValue( C2U("TextRotation") ));
                    if( aAny >>= fDoubleDegrees )
                    {
                        sal_Int32 nDeg = static_cast< sal_Int32 >(
                            ::rtl::math::round( fDoubleDegrees * 100.0 ));
                        rValue <<= nDeg;
                    }
                }
                break;
        }
    }
    // \--
}

void AxisWrapper::setFastMeterPropertyValue_NoBroadcast
    ( sal_Int32 nHandle, const uno::Any& rValue )
{
    OSL_ASSERT( m_xAxis.is());

    bool bSetScale         = false;
    bool bSetIncrement     = false;
    bool bSetOrigin        = false;

    uno::Reference< chart2::XBoundedCoordinateSystem > xCooSys( m_xAxis->getCoordinateSystem());
    sal_Int32 nDim = m_xAxis->getRepresentedDimension();
    uno::Reference< chart2::XIncrement > xInc( m_xAxis->getIncrement());
    uno::Reference< chart2::XScale > xScale;
    uno::Sequence< uno::Any > aOrigin;

    chart2::ScaleData aScaleData;
    chart2::IncrementData aIncData;

    if( xInc.is())
        aIncData = xInc->getIncrementData();

    if( xCooSys.is())
    {
        xScale.set( xCooSys->getScaleByDimension( nDim ));
        if( xScale.is())
            aScaleData = xScale->getScaleData();
        aOrigin = xCooSys->getOrigin();
    }

    sal_Bool bBool;
    switch( nHandle )
    {
        case PROP_AXIS_MAX:
            aScaleData.Maximum = rValue;
            bSetScale = true;
            break;
        case PROP_AXIS_MIN:
            aScaleData.Minimum = rValue;
            bSetScale = true;
            break;
        case PROP_AXIS_STEPMAIN:
            aIncData.Distance = rValue;
            bSetIncrement = true;
            break;
        case PROP_AXIS_STEPHELP:
            if( aIncData.Distance.hasValue())
            {
                // todo: evaluate PostEquidistant
                uno::Sequence< chart2::SubIncrement > aSubIncs( xInc->getSubIncrements());
                if( aSubIncs.getLength() == 0 )
                    aSubIncs.realloc( 1 );

                double fStepMain, fStepHelp;
                if( (rValue >>= fStepHelp) &&
                    (aIncData.Distance >>= fStepMain) &&
                    (fStepHelp != 0.0) )
                {
                    // approximate interval count
                    sal_Int32 nIntervalCount = static_cast< sal_Int32 >
                        (fStepMain / fStepHelp);

                    aSubIncs[ 0 ].IntervalCount <<= nIntervalCount;
                    xInc->setSubIncrements( aSubIncs );
                }
            }
            break;
        case PROP_AXIS_AUTO_MAX:
            if( (rValue >>= bBool ) &&
                bBool )
                aScaleData.Maximum = uno::Any();
            else
                // todo: get former auto-value from view
                aScaleData.Maximum <<= (double)(10.0);
            bSetScale = true;
            break;
        case PROP_AXIS_AUTO_MIN:
            if( (rValue >>= bBool ) &&
                bBool )
                aScaleData.Minimum = uno::Any();
            else
                // todo: get former auto-value from view
                aScaleData.Minimum <<= (double)(0.0);
            bSetScale = true;
            break;
        case PROP_AXIS_AUTO_STEPMAIN:
            if( (rValue >>= bBool ) &&
                bBool )
                aIncData.Distance = uno::Any();
            else
                // todo: get former auto-value from view
                aIncData.Distance <<= (double)(1.0);
            bSetIncrement = true;
            break;
        case PROP_AXIS_AUTO_STEPHELP:
        {
            uno::Sequence< chart2::SubIncrement > aSubIncs( xInc->getSubIncrements());
            if( aSubIncs.getLength() == 0 )
                aSubIncs.realloc( 1 );

            if( (rValue >>= bBool ) &&
                bBool )
                aSubIncs[ 0 ].IntervalCount = uno::Any();
            else
                // todo: get former auto-value from view
                aSubIncs[ 0 ].IntervalCount <<= (sal_Int32)(5);
            xInc->setSubIncrements( aSubIncs );
            break;
        }

        case PROP_AXIS_ORIGIN:
        {
            OSL_ASSERT( nDim < aOrigin.getLength());

            if( nDim < aOrigin.getLength())
            {
                aOrigin[ nDim ] = rValue;
                bSetOrigin = true;
            }
            break;
        }
        case PROP_AXIS_AUTO_ORIGIN:
        {
            OSL_ASSERT( nDim < aOrigin.getLength());

            if( nDim < aOrigin.getLength() &&
                ( rValue >>= bBool ) )
            {
                if( bBool )
                    aOrigin[ nDim ] = uno::Any();
                else
                    // todo: get former auto-value from view
                    aOrigin[ nDim ] <<= (double)(0.0);
            }
            bSetOrigin = true;
            break;
        }
        case PROP_AXIS_LOGARITHMIC:
            if( rValue >>= bBool )
            {
                uno::Reference< lang::XServiceName > xServiceName( aScaleData.Scaling, uno::UNO_QUERY );
                bool bWasLogarithm =
                    ( xServiceName.is() &&
                      (xServiceName->getServiceName()).equals(
                          C2U( "com.sun.star.chart2.LogarithmicScaling" )));

                if( bBool != bWasLogarithm )
                {
                    if( bBool )
                        aScaleData.Scaling = new LogarithmicScaling( 10.0 );
                    else
                        aScaleData.Scaling = new LinearScaling( 1.0, 0.0 );
                    bSetScale = true;
                }
            }
            break;
    }

    if( bSetScale &&
        xScale.is() )
        xScale->setScaleData( aScaleData );
    if( bSetIncrement &&
        xInc.is() )
        xInc->setIncrementData( aIncData );
    if( bSetOrigin &&
        xCooSys.is() )
        xCooSys->setOrigin( aOrigin );
}

void AxisWrapper::getFastMeterPropertyValue
    ( uno::Any & rValue,
      sal_Int32 nHandle ) const
{
    OSL_ASSERT( m_xAxis.is());

    uno::Reference< chart2::XBoundedCoordinateSystem > xCooSys( m_xAxis->getCoordinateSystem());
    sal_Int32 nDim = m_xAxis->getRepresentedDimension();
    uno::Reference< chart2::XIncrement > xInc( m_xAxis->getIncrement());
    uno::Sequence< uno::Any > aOrigin;

    chart2::ScaleData aScaleData;
    chart2::IncrementData aIncData;

    if( xInc.is())
        aIncData = xInc->getIncrementData();

    if( xCooSys.is())
    {
        uno::Reference< chart2::XScale > xScale( xCooSys->getScaleByDimension( nDim ));
        if( xScale.is())
            aScaleData = xScale->getScaleData();
        aOrigin = xCooSys->getOrigin();
    }

    switch( nHandle )
    {
        case PROP_AXIS_MAX:
            // todo: If value is auto, we need the explicit value from the view
            rValue = aScaleData.Maximum;
            break;
        case PROP_AXIS_MIN:
            // todo: If value is auto, we need the explicit value from the view
            rValue = aScaleData.Minimum;
            break;
        case PROP_AXIS_STEPMAIN:
            // todo: If value is auto, we need the explicit value from the view
            rValue = aIncData.Distance;
            break;
        case PROP_AXIS_STEPHELP:
            // todo: If value is auto, we need the explicit value from the view
            if( aIncData.Distance.hasValue())
            {
                // todo: evaluate PostEquidistant
                uno::Sequence< chart2::SubIncrement > aSubIncs( xInc->getSubIncrements());
                if( aSubIncs.getLength() > 0 )
                {
                    double fStepMain;
                    sal_Int32 nIntervalCount;
                    if( (aIncData.Distance >>= fStepMain) &&
                        (aSubIncs[ 0 ].IntervalCount >>= nIntervalCount) &&
                        nIntervalCount > 0 )
                    {
                        rValue <<= ( fStepMain / static_cast< double >( nIntervalCount ) );
                    }
                }
            }
            break;
        case PROP_AXIS_AUTO_MAX:
            rValue <<= (sal_Bool)( ! aScaleData.Maximum.hasValue());
            break;
        case PROP_AXIS_AUTO_MIN:
            rValue <<= (sal_Bool)( ! aScaleData.Minimum.hasValue());
            break;
        case PROP_AXIS_AUTO_STEPMAIN:
            rValue <<= (sal_Bool)( ! aIncData.Distance.hasValue());
            break;
        case PROP_AXIS_AUTO_STEPHELP:
        {
            uno::Sequence< chart2::SubIncrement > aSubIncs( xInc->getSubIncrements());
            if( aSubIncs.getLength() > 0 )
                rValue <<= (sal_Bool)( ! aSubIncs[ 0 ].IntervalCount.hasValue() );
            break;
        }

        case PROP_AXIS_ORIGIN:
            OSL_ASSERT( nDim < aOrigin.getLength());
            // todo: If value is auto, we need the explicit value from the view
            rValue = aOrigin[ nDim ];
            break;
        case PROP_AXIS_AUTO_ORIGIN:
            OSL_ASSERT( nDim < aOrigin.getLength());
            rValue <<= (sal_Bool)( ! aOrigin[ nDim ].hasValue());
            break;

        case PROP_AXIS_LOGARITHMIC:
        {
            uno::Reference< lang::XServiceName > xServiceName( aScaleData.Scaling, uno::UNO_QUERY );
            rValue <<= static_cast< sal_Bool >
                ( xServiceName.is() &&
                  (xServiceName->getServiceName()).equals(
                      C2U( "com.sun.star.chart2.LogarithmicScaling" )));
            break;
        }
    }
}

// --------------------------------------------------------------------------------

// ____ OPropertySet ____
uno::Any AxisWrapper::GetDefaultValue( sal_Int32 nHandle ) const
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
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL AxisWrapper::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    AxisWrapper::getPropertySetInfo()
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

uno::Sequence< ::rtl::OUString > AxisWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartAxis" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.style.CharacterProperties" );
//     aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );
//     aServices[ 4 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( AxisWrapper, lcl_aServiceName );

// needed by MSC compiler
using impl::AxisWrapper_Base;

IMPLEMENT_FORWARD_XINTERFACE2( AxisWrapper, AxisWrapper_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AxisWrapper, AxisWrapper_Base, OPropertySet )

} //  namespace wrapper
} //  namespace chart
