/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeries.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:56:27 $
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
#include "DataSeries.hxx"
#include "DataSeriesProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "DataPoint.hxx"
#include "macros.hxx"
#include "algohelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif

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
        ::chart::DataSeriesProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ true );
        ::chart::CharacterProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ true );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
    }

    return aPropSeq;
}
} // anonymous namespace

// ----------------------------------------

namespace chart
{

DataSeries::DataSeries( const ::rtl::OUString & rIdentifier ) :
        ::property::OPropertySet( m_aMutex ),
    m_aIdentifier( rIdentifier )
{}

DataSeries::~DataSeries()
{}

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
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        DataSeriesProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );
        CharacterProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
    {
        if( nHandle == DataSeriesProperties::PROP_DATASERIES_IDENTIFIER )
        {
            return uno::makeAny( m_aIdentifier );
        }
        else
            throw beans::UnknownPropertyException();
    }

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

Reference< beans::XPropertySet >
    SAL_CALL DataSeries::getDataPointByIndex( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > aResult;

    // /--
    MutexGuard aGuard( GetMutex() );

    if( m_aDataSequences.begin() != m_aDataSequences.end())
    {
        if( 0 <= nIndex && nIndex <
            (*(m_aDataSequences.begin()))->getData().getLength() )
        {
            tDataPointAttributeContainer::iterator aIt(
                m_aAttributedDataPoints.find( nIndex ));

            if( aIt == m_aAttributedDataPoints.end())
            {
                // create a new XPropertySet for this data point
                aResult.set( new DataPoint( GetMutex(), this ));
                m_aAttributedDataPoints[ nIndex ] = aResult;
            }
            else
            {
                aResult = (*aIt).second;
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
void SAL_CALL DataSeries::setData( const uno::Sequence< Reference< chart2::XDataSequence > >& aData )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    m_aDataSequences = helper::SequenceToVector( aData );
    // \--
}

// ____ XDataSource ____
Sequence< Reference< chart2::XDataSequence > > SAL_CALL DataSeries::getDataSequences()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return helper::VectorToSequence( m_aDataSequences );
    // \--
}

// ____ XRegressionCurveContainer ____
void SAL_CALL DataSeries::addRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& aRegressionCurve )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    uno::Reference< chart2::XIdentifiable > xIdent( aRegressionCurve, uno::UNO_QUERY );
    if( ! xIdent.is() ||
        m_aRegressionCurves.find( xIdent->getIdentifier() ) != m_aRegressionCurves.end() )
        throw lang::IllegalArgumentException();

    m_aRegressionCurves.insert(
        tRegressionCurveContainerType::value_type( xIdent->getIdentifier(), aRegressionCurve ));
}

void SAL_CALL DataSeries::removeRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& aRegressionCurve )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( ! aRegressionCurve.is())
        throw container::NoSuchElementException();

    uno::Reference< chart2::XIdentifiable > xIdent( aRegressionCurve, uno::UNO_QUERY );
    if( xIdent.is())
    {
        tRegressionCurveContainerType::iterator aIt( m_aRegressionCurves.find( xIdent->getIdentifier() ));

        if( aIt == m_aRegressionCurves.end())
            throw container::NoSuchElementException();

        m_aRegressionCurves.erase( aIt );
    }
    else
        throw container::NoSuchElementException();
}

uno::Sequence< uno::Reference< chart2::XRegressionCurve > > SAL_CALL DataSeries::getRegressionCurves()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< chart2::XRegressionCurve > > aResult( m_aRegressionCurves.size());

    ::std::transform( m_aRegressionCurves.begin(), m_aRegressionCurves.end(),
                      aResult.getArray(),
                      ::std::select2nd< tRegressionCurveContainerType::value_type >() );

    return aResult;
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
