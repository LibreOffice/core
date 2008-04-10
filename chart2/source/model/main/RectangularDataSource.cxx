/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RectangularDataSource.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "RectangularDataSource.hxx"
#include "CachedDataSequence.hxx"
#include "StandardNaNHelper.hxx"
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT;

// necessary for MS compiler
using ::comphelper::OPropertyContainer;
using ::chart::impl::RectangularDataSource_Base;

// ________________________________________

// Property-Ids

namespace
{
enum
{
    PROP_SOURCE_RANGE,
    PROP_DATA_ROW_SOURCE
};
}  // anonymous namespace


// ________________________________________

namespace chart
{

#define STANDARD_PROPERTY_ATTRIBUTES ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT

RectangularDataSource::RectangularDataSource( uno::Reference< uno::XComponentContext > const & xContext )
        : OPropertyContainer( GetBroadcastHelper() ),
          RectangularDataSource_Base( GetMutex() ),
          m_aDataRowSource( ::com::sun::star::chart::ChartDataRowSource_COLUMNS )
{
    registerProperty( C2U( "SourceRange" ),
                      PROP_SOURCE_RANGE,                   // Handle for fast access
                      0,                                   // PropertyAttributes
                      & m_aSourceRange,                    // pointer to corresponding member
                      ::getCppuType( & m_aSourceRange )    // Type of property
        );

    registerProperty( C2U( "DataRowSource" ),
                      PROP_DATA_ROW_SOURCE,                // Handle for fast access
                      MAYBEDEFAULT,                        // PropertyAttributes
                      & m_aDataRowSource,                  // pointer to corresponding member
                      ::getCppuType( & m_aDataRowSource )  // Type of property
        );
}

RectangularDataSource::~RectangularDataSource()
{}

// base for XServiceInfo
Sequence< OUString > RectangularDataSource::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.RectangularDataSource" );
    aServices[ 1 ] = C2U( "com.sun.star.chart2.data.DataFilter" );
    aServices[ 2 ] = C2U( "com.sun.star.chart2.data.DataSource" );
    aServices[ 3 ] = C2U( "com.sun.star.chart2.data.DataSink" );
    return aServices;
}

// ____ XDataSink ____
void SAL_CALL RectangularDataSource::setData( const uno::Sequence< Reference< chart2::data::XDataSequence > >& aData )
    throw (RuntimeException)
{
    m_aDataSequences = aData;
}

// ____ XDataSource ____
uno::Sequence< Reference< chart2::data::XDataSequence > > SAL_CALL RectangularDataSource::getDataSequences()
    throw (RuntimeException)
{
    return m_aDataSequences;
}

double SAL_CALL RectangularDataSource::getNotANumber()
    throw (uno::RuntimeException)
{
    return StandardNaNHelper::getNotANumber();
}

sal_Bool SAL_CALL RectangularDataSource::isNotANumber( double fNumber )
    throw (uno::RuntimeException)
{
    return StandardNaNHelper::isNotANumber( fNumber );
}

// ____ XIndexAccess ____
 sal_Int32 SAL_CALL RectangularDataSource::getCount()
        throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_aDataSequences.getLength();
    // \--
}

 Any SAL_CALL RectangularDataSource::getByIndex( sal_Int32 Index )
        throw (lang::IndexOutOfBoundsException,
               lang::WrappedTargetException,
               RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return uno::makeAny( GetDataByIndex( Index ) );
    // \--
}

// ____ XElementAccess (base of XIndexAccess) ____
 uno::Type SAL_CALL RectangularDataSource::getElementType()
        throw (RuntimeException)
{
    return ::getCppuType((const Reference< chart2::data::XDataSequence >*)0);
}

sal_Bool SAL_CALL RectangularDataSource::hasElements()
        throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return static_cast< sal_Bool >( m_aDataSequences.getLength() > 0 );
    // \--
}

Reference< chart2::data::XDataSequence > RectangularDataSource::GetDataByIndex( sal_Int32 Index ) const
    throw (lang::IndexOutOfBoundsException)
{
    if( 0 > Index || Index >= m_aDataSequences.getLength() )
        throw lang::IndexOutOfBoundsException();

    return m_aDataSequences.getConstArray() [ Index ];
}

void SAL_CALL RectangularDataSource::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle,
    const Any& rValue )
    throw (uno::Exception)
{
    // at this point it is clear that rValue contains a different value than the
    // property currently still has, and that it will be overwritten by it in
    // the final base class call

    switch( nHandle )
    {
         case PROP_DATA_ROW_SOURCE:
        {
            ::com::sun::star::chart::ChartDataRowSource aRowSource;
            if( rValue >>= aRowSource )
            {
                if( aRowSource != m_aDataRowSource )
                {
                    SwapData();
                }
            }
        }
        break;
    }

    OPropertyContainer::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

// ================================================================================

using impl::RectangularDataSource_Base;

IMPLEMENT_FORWARD_XINTERFACE2( RectangularDataSource, RectangularDataSource_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( RectangularDataSource, RectangularDataSource_Base, OPropertyContainer )

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL RectangularDataSource::getPropertySetInfo()
    throw(RuntimeException)
{
    return Reference< beans::XPropertySetInfo >( createPropertySetInfo( getInfoHelper() ) );
}

// ____ ::comphelper::OPropertySetHelper ____
// __________________________________________
::cppu::IPropertyArrayHelper& RectangularDataSource::getInfoHelper()
{
    return *getArrayHelper();
}

// ____ ::comphelper::OPropertyArrayHelper ____
// ____________________________________________
::cppu::IPropertyArrayHelper* RectangularDataSource::createArrayHelper() const
{
    Sequence< Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( RectangularDataSource,
                             C2U( "com.sun.star.comp.chart.RectangularDataSource" ))

void RectangularDataSource::SwapData()
{
    try
    {
        // assume that there are always text labels
        Reference< chart2::data::XTextualDataSequence > xCategories( GetDataByIndex( 0 ), uno::UNO_QUERY );

        // -1: the first sequence contains labels
        sal_Int32 nNewSequenceSize = m_aDataSequences.getLength() - 1;
        if( nNewSequenceSize > 0 )
        {
            // +1: one series more for labels
            sal_Int32 nNewNumOfSequences = GetDataByIndex( 0 )->getData().getLength() + 1;
            Sequence< Reference< chart2::data::XDataSequence > > aResult( nNewNumOfSequences );
            ::std::vector< OUString > aCategoryVector( nNewSequenceSize );
            sal_Int32 nIdx = 0;

            // fill new categories consisting of former labels
            for( nIdx = 0; nIdx < nNewSequenceSize; ++nIdx )
            {
                aCategoryVector.push_back( GetDataByIndex( nIdx )->getLabel() );
            }
            aResult[ 0 ] = Reference< chart2::data::XDataSequence >(
                static_cast< chart2::data::XTextualDataSequence * >(
                    new ::chart::CachedDataSequence( aCategoryVector )));

            // fill new data sequences.  The label is set by the former categories
            for( sal_Int32 nSeqIdx = 1; nSeqIdx < nNewNumOfSequences; ++ nSeqIdx )
            {
                ::std::vector< double > aDataVector( nNewSequenceSize );
                for( nIdx = 0; nIdx < nNewSequenceSize; ++nIdx )
                {
                    aDataVector.push_back(
                        Reference< chart2::data::XNumericalDataSequence >(
                            GetDataByIndex( nIdx ),
                            uno::UNO_QUERY )->getNumericalData().getConstArray()[ nSeqIdx - 1 ] );
                }
                aResult[ nSeqIdx ] = Reference< chart2::data::XDataSequence >(
                    static_cast< chart2::data::XNumericalDataSequence * >(
                        new ::chart::CachedDataSequence( aDataVector )));
            }

            setData( aResult );
        }
    }
    catch( lang::IndexOutOfBoundsException aEx )
    {
    }
}

} // namespace chart
