/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DiagramWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:01:18 $
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
#include "DiagramWrapper.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "TitleWrapper.hxx"
#include "DataSeriesPointWrapper.hxx"
#include "AxisWrapper.hxx"
#include "MeterHelper.hxx"
#include "DataSeriesTreeHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPEGROUP_HPP_
#include <com/sun/star/chart2/XChartTypeGroup.hpp>
#endif

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
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Diagram" ));

enum
{
    PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS,
    PROP_DIAGRAM_PERCENT_STACKED,
    PROP_DIAGRAM_STACKED,
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "AttributedDataPoints" ),
                  PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS,
                  ::getCppuType( reinterpret_cast< const uno::Sequence< uno::Sequence< sal_Int32 > > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // see com.sun.star.chart.StackableDiagram
    rOutProperties.push_back(
        Property( C2U( "Percent" ),
                  PROP_DIAGRAM_PERCENT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Stacked" ),
                  PROP_DIAGRAM_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DIAGRAM_PERCENT_STACKED ));
    rOutMap[ PROP_DIAGRAM_PERCENT_STACKED ] =
        uno::makeAny( sal_False );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DIAGRAM_STACKED ));
    rOutMap[ PROP_DIAGRAM_STACKED ] =
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

void lcl_getDataSeriesByIndex(
    uno::Reference< chart2::XDataSeriesTreeParent > xParent,
    uno::Reference< chart2::XDataSeries > & rOutSeries,
    sal_Int32 nIndex,
    sal_Int32 nCurrentIndex = 0 )
{
    if( xParent.is())
    {
        uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren( xParent->getChildren());
        for( sal_Int32 i = 0; i < aChildren.getLength(); ++i )
        {
            uno::Reference< chart2::XDataSeries > aDataSeries( aChildren[ i ], uno::UNO_QUERY );
            if( aDataSeries.is())
            {
                if( nCurrentIndex == nIndex )
                {
                    rOutSeries = aDataSeries;
                    return;
                }

                ++nCurrentIndex;
            }
            else
            {
                uno::Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ i ], uno::UNO_QUERY );
                if( xNewParent.is())
                    lcl_getDataSeriesByIndex( xNewParent, rOutSeries, nIndex, nCurrentIndex );
            }
        }
    }
}

void lcl_addDataSeries(
    uno::Reference< chart2::XDataSeriesTreeParent > xParent,
    ::std::vector< uno::Reference< chart2::XDataSeries > > & rOutSeriesVec )
{
    if( xParent.is())
    {
        uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren( xParent->getChildren());
        for( sal_Int32 i = 0; i < aChildren.getLength(); ++i )
        {
            uno::Reference< chart2::XDataSeries > aDataSeries( aChildren[ i ], uno::UNO_QUERY );
            if( aDataSeries.is())
            {
                rOutSeriesVec.push_back( aDataSeries );
            }
            else
            {
                uno::Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ i ], uno::UNO_QUERY );
                if( xNewParent.is())
                    lcl_addDataSeries( xNewParent, rOutSeriesVec );
            }
        }
    }
}
} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

DiagramWrapper::DiagramWrapper(
    const uno::Reference< chart2::XDiagram > & xDia,
    const uno::Reference< uno::XComponentContext > & xContext,
    ::osl::Mutex & _rMutex ) :
        OPropertySet( _rMutex ),
        m_rMutex( _rMutex ),
        m_xContext( xContext ),
        m_aEventListenerContainer( _rMutex ),
        m_xDiagram( xDia )
{}

DiagramWrapper::~DiagramWrapper()
{}

::osl::Mutex & DiagramWrapper::GetMutex() const
{
    return m_rMutex;
}

// ____ XDiagram ____
::rtl::OUString SAL_CALL DiagramWrapper::getDiagramType()
    throw (uno::RuntimeException)
{
    ::rtl::OUString aTypeName;

    if( ! m_xDiagram.is())
        return aTypeName;

    // find first element in tree (must be a ChartTypeGroup) and take its type
    uno::Reference< chart2::XDataSeriesTreeParent > xParent( m_xDiagram->getTree());
    if( xParent.is())
    {
        uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren( xParent->getChildren());
        if( aChildren.getLength() > 0 )
        {
            uno::Reference< chart2::XChartTypeGroup > xGroup( aChildren[ 0 ], uno::UNO_QUERY );
            if( xGroup.is())
            {
                uno::Reference< chart2::XChartType > xType( xGroup->getChartType() );
                if( xType.is())
                {
                    aTypeName = xType->getChartType();
                }
            }
        }
    }

    // ToDo: Necessary?  New chart could use old strings, API users
    // (e.g. xmloff) have to deal with new types?

    // ToDo: If necessary, use a map

    // convert new names to old ones
    if( aTypeName.getLength() > 0 )
    {
        if( aTypeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.BarChart" )))
            aTypeName = C2U( "com.sun.star.chart.BarDiagram" );
        if( aTypeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.AreaChart" )))
            aTypeName = C2U( "com.sun.star.chart.AreaDiagram" );
        if( aTypeName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.PieChart" )))
            aTypeName = C2U( "com.sun.star.chart.PieDiagram" );
    }

    return aTypeName;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataRowProperties( sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    uno::Reference< chart2::XDataSeries > xSeries;

    if( nRow >= 0 )
        lcl_getDataSeriesByIndex( m_xDiagram->getTree(), xSeries, nRow );

    if( ! xSeries.is() )
        throw lang::IndexOutOfBoundsException(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    return new DataSeriesPointWrapper(
        DataSeriesPointWrapper::DATA_SERIES,
        uno::Reference< beans::XPropertySet >( xSeries, uno::UNO_QUERY ),
        m_xContext, GetMutex() );
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    uno::Reference< chart2::XDataSeries > xSeries;

    if( nRow >= 0 )
        lcl_getDataSeriesByIndex( m_xDiagram->getTree(), xSeries, nRow );

    if( ! xSeries.is() )
        throw lang::IndexOutOfBoundsException(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    // may throw an IllegalArgumentException
    uno::Reference< beans::XPropertySet > xPointProp(
        xSeries->getDataPointByIndex( nCol ));

    return new DataSeriesPointWrapper(
        DataSeriesPointWrapper::DATA_POINT, xPointProp, m_xContext, GetMutex() );
}

// ____ XShape (base of XDiagram) ____
awt::Point SAL_CALL DiagramWrapper::getPosition()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

void SAL_CALL DiagramWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

awt::Size SAL_CALL DiagramWrapper::getSize()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

void SAL_CALL DiagramWrapper::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

// ____ XShapeDescriptor (base of XShape) ____
::rtl::OUString SAL_CALL DiagramWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.Diagram" );
}

// ____ XAxisZSupplier ____
uno::Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getZAxisTitle()
    throw (uno::RuntimeException)
{
    if( ! m_xZAxisTitle.is())
    {
        try
        {
            uno::Reference< chart2::XAxisContainer > xAxisCnt( m_xDiagram, uno::UNO_QUERY_THROW );
            uno::Reference< chart2::XTitled > xAxisTitled(
                xAxisCnt->getAxisByIdentifier( MeterHelper::makeAxisIdentifier( 2, 0 ) ),
                uno::UNO_QUERY_THROW );
            m_xZAxisTitle = new TitleWrapper( xAxisTitled->getTitle(), m_xContext, GetMutex() );
            uno::Reference< lang::XComponent > xComp( m_xZAxisTitle, uno::UNO_QUERY );
            if( xComp.is())
                xComp->addEventListener( this );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xZAxisTitle;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getZMainGrid()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getZHelpGrid()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getZAxis()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}


// ____ XTwoAxisXSupplier ____
uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getSecondaryXAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xSecondXAxis.is())
    {
        m_xSecondXAxis = new AxisWrapper( AxisWrapper::SECOND_X_AXIS, m_xDiagram, m_xContext, GetMutex() );
        uno::Reference< lang::XComponent > xComp( m_xSecondXAxis, uno::UNO_QUERY );
        if( xComp.is())
            xComp->addEventListener( this );
    }

    return m_xSecondXAxis;
}


// ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
uno::Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getXAxisTitle()
    throw (uno::RuntimeException)
{
    if( ! m_xXAxisTitle.is())
    {
        try
        {
            uno::Reference< chart2::XAxisContainer > xAxisCnt( m_xDiagram, uno::UNO_QUERY_THROW );
            uno::Reference< chart2::XTitled > xAxisTitled(
                xAxisCnt->getAxisByIdentifier( MeterHelper::makeAxisIdentifier( 0, 0 ) ),
                uno::UNO_QUERY_THROW );
            m_xXAxisTitle = new TitleWrapper( xAxisTitled->getTitle(), m_xContext, GetMutex() );
            uno::Reference< lang::XComponent > xComp( m_xXAxisTitle, uno::UNO_QUERY );
            if( xComp.is())
                xComp->addEventListener( this );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xXAxisTitle;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getXAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xXAxis.is())
    {
        m_xXAxis = new AxisWrapper( AxisWrapper::X_AXIS, m_xDiagram, m_xContext, GetMutex() );
        uno::Reference< lang::XComponent > xComp( m_xXAxis, uno::UNO_QUERY );
        if( xComp.is())
            xComp->addEventListener( this );
    }

    return m_xXAxis;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getXMainGrid()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getXHelpGrid()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}


// ____ XTwoAxisYSupplier ____
uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getSecondaryYAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xSecondYAxis.is())
    {
        m_xSecondYAxis = new AxisWrapper( AxisWrapper::SECOND_Y_AXIS, m_xDiagram, m_xContext, GetMutex() );
        uno::Reference< lang::XComponent > xComp( m_xSecondYAxis, uno::UNO_QUERY );
        if( xComp.is())
            xComp->addEventListener( this );
    }

    return m_xSecondYAxis;
}


// ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
uno::Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getYAxisTitle()
    throw (uno::RuntimeException)
{
    if( ! m_xYAxisTitle.is())
    {
        try
        {
            uno::Reference< chart2::XAxisContainer > xAxisCnt( m_xDiagram, uno::UNO_QUERY_THROW );
            uno::Reference< chart2::XTitled > xAxisTitled(
                xAxisCnt->getAxisByIdentifier( MeterHelper::makeAxisIdentifier( 1, 0 ) ),
                uno::UNO_QUERY_THROW );
            m_xYAxisTitle = new TitleWrapper( xAxisTitled->getTitle(), m_xContext, GetMutex() );
            uno::Reference< lang::XComponent > xComp( m_xYAxisTitle, uno::UNO_QUERY );
            if( xComp.is())
                xComp->addEventListener( this );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xYAxisTitle;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getYAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xYAxis.is())
    {
        m_xYAxis = new AxisWrapper( AxisWrapper::Y_AXIS, m_xDiagram, m_xContext, GetMutex() );
        uno::Reference< lang::XComponent > xComp( m_xYAxis, uno::UNO_QUERY );
        if( xComp.is())
            xComp->addEventListener( this );
    }

    return m_xYAxis;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getYHelpGrid()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getYMainGrid()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}


// ____ XStatisticDisplay ____
uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getUpBar()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDownBar()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getMinMaxLine()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not implemented yet!" );
    throw uno::RuntimeException();
}

// ____ X3DDisplay ____
uno::Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getWall()
    throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > aResult;

    if( m_xDiagram.is())
        aResult.set( m_xDiagram->getWall() );

    return aResult;
}

uno::Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getFloor()
    throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > aResult;

    if( m_xDiagram.is())
        aResult.set( m_xDiagram->getFloor() );

    throw uno::RuntimeException();
}

// ____ XComponent ____
void SAL_CALL DiagramWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( *this ) );

    // /--
    MutexGuard aGuard( GetMutex());
    m_xDiagram = NULL;
    m_xXAxisTitle = NULL;
    m_xYAxisTitle = NULL;
    m_xZAxisTitle = NULL;
    m_xXAxis = NULL;
    m_xYAxis = NULL;
    m_xZAxis = NULL;
    m_xSecondXAxis = NULL;
    m_xSecondYAxis = NULL;
    // \--
}

void SAL_CALL DiagramWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL DiagramWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ____ XEventListener ____
void SAL_CALL DiagramWrapper::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
}


// sal_Bool SAL_CALL DiagramWrapper::convertFastPropertyValue
//     ( uno::Any & rConvertedValue,
//       uno::Any & rOldValue,
//       sal_Int32 nHandle,
//       const uno::Any& rValue )
//     throw (lang::IllegalArgumentException)
// {
// }

void SAL_CALL DiagramWrapper::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // /--
    MutexGuard aGuard( GetMutex());
    uno::Reference< beans::XPropertySet > xDiaProp( m_xDiagram, uno::UNO_QUERY );

    switch( nHandle )
    {
        case PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS:
            if( m_xDiagram.is() &&
                xDiaProp.is())
            {
                ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesVec;
                lcl_addDataSeries( m_xDiagram->getTree(), aSeriesVec );
                uno::Sequence< uno::Sequence< sal_Int32 > > aResult( aSeriesVec.size() );

                ::std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt =
                      aSeriesVec.begin();
                sal_Int32 i = 0;
                for( ; aIt != aSeriesVec.end(); ++aIt, ++i )
                {
                    uno::Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        uno::Any aVal(
                            xProp->getPropertyValue( C2U( "AttributedDataPoints" )));
                        uno::Sequence< sal_Int32 > aSeq;
                        if( aVal >>= aSeq )
                            aResult[ i ] = aSeq;
                    }
                }
                rValue <<= aResult;
            }
            break;

        case PROP_DIAGRAM_PERCENT_STACKED:
        {
            sal_Bool bIsStacked = (
                helper::DataSeriesTreeHelper::getStackMode(
                    m_xDiagram->getTree()) == chart2::StackMode_STACKED_PERCENT);
            rValue <<= bIsStacked;
        }
        break;

        case PROP_DIAGRAM_STACKED:
        {
            sal_Bool bIsStacked = (
                helper::DataSeriesTreeHelper::getStackMode(
                    m_xDiagram->getTree()) == chart2::StackMode_STACKED);
            rValue <<= bIsStacked;
        }
        break;

        default:
            OPropertySet::getFastPropertyValue( rValue, nHandle );
            break;
    }
    // \--
}

void SAL_CALL DiagramWrapper::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    sal_Bool bValue;
    switch( nHandle )
    {
        case PROP_DIAGRAM_PERCENT_STACKED:
            if( rValue >>= bValue )
            {
                helper::DataSeriesTreeHelper::setStackMode(
                    m_xDiagram->getTree(),
                    bValue
                    ? chart2::StackMode_STACKED_PERCENT
                    : chart2::StackMode_NONE );
            }
            break;

        case PROP_DIAGRAM_STACKED:
            if( rValue >>= bValue )
            {
                helper::DataSeriesTreeHelper::setStackMode(
                    m_xDiagram->getTree(),
                    bValue
                    ? chart2::StackMode_STACKED
                    : chart2::StackMode_NONE );
            }
            break;

        default:
            break;
    }
}

// ____ XDiagramProvider ____
uno::Reference< chart2::XDiagram > SAL_CALL DiagramWrapper::getDiagram()
    throw (uno::RuntimeException)
{
    return m_xDiagram;
}

void SAL_CALL DiagramWrapper::setDiagram(
    const uno::Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    m_xDiagram = xDiagram;

    // invalidate all children using the old diagram
    m_xXAxisTitle = NULL;
    m_xYAxisTitle = NULL;
    m_xZAxisTitle = NULL;
    m_xXAxis = NULL;
    m_xYAxis = NULL;
    m_xZAxis = NULL;
    m_xSecondXAxis = NULL;
    m_xSecondYAxis = NULL;
    // \--
}

// --------------------------------------------------------------------------------

// ____ OPropertySet ____
uno::Any DiagramWrapper::GetDefaultValue( sal_Int32 nHandle ) const
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
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL DiagramWrapper::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    DiagramWrapper::getPropertySetInfo()
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

uno::Sequence< ::rtl::OUString > DiagramWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.Diagram" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.chart.StackableDiagram" );
//     aServices[ 4 ] = C2U( "com.sun.star.beans.PropertySet" );
//     aServices[ 5 ] = C2U( "com.sun.star.drawing.FillProperties" );
//     aServices[ 6 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DiagramWrapper, lcl_aServiceName );

// needed by MSC compiler
using impl::DiagramWrapper_Base;

IMPLEMENT_FORWARD_XINTERFACE2( DiagramWrapper, DiagramWrapper_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( DiagramWrapper, DiagramWrapper_Base, OPropertySet )

} //  namespace wrapper
} //  namespace chart
