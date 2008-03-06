/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeries.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:26:30 $
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
#include "DataSeries.hxx"
#include "DataSeriesProperties.hxx"
#include "DataPointProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "DataPoint.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"
#include "ModifyListenerHelper.hxx"
#include "EventListenerHelper.hxx"

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::osl::MutexGuard;

// ----------------------------------------

namespace
{
const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::DataSeriesProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

void lcl_SetParent(
    const uno::Reference< uno::XInterface > & xChildInterface,
    const uno::Reference< uno::XInterface > & xParentInterface )
{
    uno::Reference< container::XChild > xChild( xChildInterface, uno::UNO_QUERY );
    if( xChild.is())
        xChild->setParent( xParentInterface );
}

typedef ::std::map< sal_Int32, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > >
    lcl_tDataPointMap;

void lcl_CloneAttributedDataPoints(
    const lcl_tDataPointMap & rSource, lcl_tDataPointMap & rDestination,
    const uno::Reference< uno::XInterface > & xSeries )
{
    for( lcl_tDataPointMap::const_iterator aIt( rSource.begin());
         aIt != rSource.end(); ++aIt )
    {
        Reference< beans::XPropertySet > xPoint( (*aIt).second );
        if( xPoint.is())
        {
            Reference< util::XCloneable > xCloneable( xPoint, uno::UNO_QUERY );
            if( xCloneable.is())
            {
                xPoint.set( xCloneable->createClone(), uno::UNO_QUERY );
                if( xPoint.is())
                {
                    lcl_SetParent( xPoint, xSeries );
                    rDestination.insert( lcl_tDataPointMap::value_type( (*aIt).first, xPoint ));
                }
            }
        }
    }
}

bool lcl_isInternalData( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    Reference< lang::XServiceInfo > xServiceInfo( xLSeq, uno::UNO_QUERY );
    return ( xServiceInfo.is() && xServiceInfo->getImplementationName().equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("com.sun.star.comp.chart2.LabeledDataSequence")));
}

} // anonymous namespace

// ----------------------------------------

namespace chart
{

DataSeries::DataSeries( const uno::Reference< uno::XComponentContext > & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_xContext( xContext ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
}

DataSeries::DataSeries( const DataSeries & rOther ) :
        MutexContainer(),
        impl::DataSeries_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    if( ! rOther.m_aDataSequences.empty())
    {
        if( lcl_isInternalData( rOther.m_aDataSequences.front()))
            CloneHelper::CloneRefVector< tDataSequenceContainer::value_type >(
                rOther.m_aDataSequences, m_aDataSequences );
        else
            m_aDataSequences = rOther.m_aDataSequences;
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }

    CloneHelper::CloneRefVector< Reference< chart2::XRegressionCurve > >( rOther.m_aRegressionCurves, m_aRegressionCurves );
    ModifyListenerHelper::addListenerToAllElements( m_aRegressionCurves, m_xModifyEventForwarder );

    // add as listener to XPropertySet properties
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );
}

// late initialization to call after copy-constructing
void DataSeries::Init( const DataSeries & rOther )
{
    if( ! rOther.m_aDataSequences.empty())
        EventListenerHelper::addListenerToAllElements( m_aDataSequences, this );

    Reference< uno::XInterface > xThisInterface( static_cast< ::cppu::OWeakObject * >( this ));
    if( ! rOther.m_aAttributedDataPoints.empty())
    {
        lcl_CloneAttributedDataPoints(
            rOther.m_aAttributedDataPoints, m_aAttributedDataPoints, xThisInterface );
        ModifyListenerHelper::addListenerToAllMapElements( m_aAttributedDataPoints, m_xModifyEventForwarder );
    }

    // add as parent to error bars
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        lcl_SetParent( xPropertySet, xThisInterface );

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        lcl_SetParent( xPropertySet, xThisInterface );
}

DataSeries::~DataSeries()
{
    try
    {
        ModifyListenerHelper::removeListenerFromAllMapElements( m_aAttributedDataPoints, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListenerFromAllElements( m_aRegressionCurves, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListenerFromAllElements( m_aDataSequences, m_xModifyEventForwarder );

        // remove listener from XPropertySet properties
        Reference< beans::XPropertySet > xPropertySet;
        uno::Any aValue;

        getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );

        getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
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
uno::Reference< util::XCloneable > SAL_CALL DataSeries::createClone()
    throw (uno::RuntimeException)
{
    DataSeries * pNewSeries( new DataSeries( *this ));
    // hold a reference to the clone
    uno::Reference< util::XCloneable > xResult( pNewSeries );
    // do initialization that uses uno references to the clone
    pNewSeries->Init( *this );

    return xResult;
}

Sequence< OUString > DataSeries::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.DataSeries" );
    aServices[ 1 ] = C2U( "com.sun.star.chart2.DataPointProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any DataSeries::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        DataSeriesProperties::AddDefaultsToMap( aStaticDefaults );
        CharacterProperties::AddDefaultsToMap( aStaticDefaults );

        float fDefaultCharHeight = 6.0;
        ::chart::PropertyHelper::setPropertyValue( aStaticDefaults, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( aStaticDefaults, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( aStaticDefaults, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        throw beans::UnknownPropertyException();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL DataSeries::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    DataSeries::getPropertySetInfo()
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

void SAL_CALL DataSeries::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // special handling for get.  set is not possible for this property
    if( nHandle == DataSeriesProperties::PROP_DATASERIES_ATTRIBUTED_DATA_POINTS )
    {
        // ToDo: only add those property sets that are really modified
        uno::Sequence< sal_Int32 > aSeq( m_aAttributedDataPoints.size());
        sal_Int32 * pIndexArray = aSeq.getArray();
        sal_Int32 i = 0;

        for( tDataPointAttributeContainer::const_iterator aIt( m_aAttributedDataPoints.begin());
             aIt != m_aAttributedDataPoints.end(); ++aIt )
        {
            pIndexArray[ i ] = (*aIt).first;
            ++i;
        }

        rValue <<= aSeq;
    }
    else
        OPropertySet::getFastPropertyValue( rValue, nHandle );
}

void SAL_CALL DataSeries::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    if(    nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y
        || nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X )
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

Reference< beans::XPropertySet >
    SAL_CALL DataSeries::getDataPointByIndex( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > aResult;

    // /--
    MutexGuard aGuard( GetMutex() );

    if( ! m_aDataSequences.empty() )
    {
        ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aValuesSeries(
            DataSeriesHelper::getAllDataSequencesByRole(
                ContainerHelper::ContainerToSequence( m_aDataSequences ),
                C2U("values"), true ));
        if( ! aValuesSeries.empty())
        {
            Reference< chart2::data::XDataSequence > xSeq( aValuesSeries.front()->getValues());
            if( 0 <= nIndex && nIndex < xSeq->getData().getLength())
            {
                tDataPointAttributeContainer::iterator aIt(
                    m_aAttributedDataPoints.find( nIndex ));

                if( aIt == m_aAttributedDataPoints.end())
                {
                    // create a new XPropertySet for this data point
                    aResult.set( new DataPoint( this ));
                    ModifyListenerHelper::addListener( aResult, m_xModifyEventForwarder );
                    m_aAttributedDataPoints[ nIndex ] = aResult;
                }
                else
                {
                    aResult = (*aIt).second;
                }
            }
        }
        else
        {
            throw lang::IndexOutOfBoundsException();
        }
    }
    else
    {
        throw lang::IndexOutOfBoundsException();
    }

    return aResult;
    // \--
}

// ____ XDataSink ____
void SAL_CALL DataSeries::setData( const uno::Sequence< Reference< chart2::data::XLabeledDataSequence > >& aData )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    ModifyListenerHelper::removeListenerFromAllElements( m_aDataSequences, m_xModifyEventForwarder );
    EventListenerHelper::removeListenerFromAllElements( m_aDataSequences, this );
    m_aDataSequences = ContainerHelper::SequenceToVector( aData );
    EventListenerHelper::addListenerToAllElements( m_aDataSequences, this );
    ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    fireModifyEvent();
    // \--
}

// ____ XDataSource ____
Sequence< Reference< chart2::data::XLabeledDataSequence > > SAL_CALL DataSeries::getDataSequences()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return ContainerHelper::ContainerToSequence( m_aDataSequences );
    // \--
}


// ____ XRegressionCurveContainer ____
void SAL_CALL DataSeries::addRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ::std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), xRegressionCurve )
        != m_aRegressionCurves.end())
        throw lang::IllegalArgumentException();

    m_aRegressionCurves.push_back( xRegressionCurve );
    ModifyListenerHelper::addListener( xRegressionCurve, m_xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL DataSeries::removeRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( ! xRegressionCurve.is())
        throw container::NoSuchElementException();

    tRegressionCurveContainerType::iterator aIt(
            ::std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), xRegressionCurve ) );

    if( aIt == m_aRegressionCurves.end())
        throw container::NoSuchElementException(
            C2U( "The given regression curve is no element of this series" ),
            static_cast< uno::XWeak * >( this ));

    ModifyListenerHelper::removeListener( xRegressionCurve, m_xModifyEventForwarder );
    m_aRegressionCurves.erase( aIt );
    fireModifyEvent();
}

uno::Sequence< uno::Reference< chart2::XRegressionCurve > > SAL_CALL DataSeries::getRegressionCurves()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aRegressionCurves );
}

void SAL_CALL DataSeries::setRegressionCurves(
    const Sequence< Reference< chart2::XRegressionCurve > >& aRegressionCurves )
    throw (uno::RuntimeException)
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aRegressionCurves, m_xModifyEventForwarder );
    m_aRegressionCurves.clear();
    for( sal_Int32 i=0; i<aRegressionCurves.getLength(); ++i )
        addRegressionCurve( aRegressionCurves[i] );
}

// ____ XModifyBroadcaster ____
void SAL_CALL DataSeries::addModifyListener( const Reference< util::XModifyListener >& aListener )
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

void SAL_CALL DataSeries::removeModifyListener( const Reference< util::XModifyListener >& aListener )
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
void SAL_CALL DataSeries::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataSeries::disposing( const lang::EventObject& rEventObject )
    throw (uno::RuntimeException)
{
    // forget disposed data sequences
    tDataSequenceContainer::iterator aIt(
        ::std::find( m_aDataSequences.begin(), m_aDataSequences.end(), rEventObject.Source ));
    if( aIt != m_aDataSequences.end())
        m_aDataSequences.erase( aIt );
}

// ____ OPropertySet ____
void DataSeries::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void DataSeries::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}


// ================================================================================

using impl::DataSeries_Base;
using ::property::OPropertySet;

IMPLEMENT_FORWARD_XINTERFACE2( DataSeries, DataSeries_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataSeries, DataSeries_Base, OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DataSeries,
                             C2U( "com.sun.star.comp.chart.DataSeries" ));

}  // namespace chart
