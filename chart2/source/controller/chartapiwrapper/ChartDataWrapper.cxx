/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartDataWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:00:11 $
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
#include "ChartDataWrapper.hxx"
#include "macros.hxx"
#include "algohelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XNUMERICALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XNumericalDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <com/sun/star/chart2/XDataSource.hpp>
#endif

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"

#include <algorithm>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

using namespace ::com::sun::star;
using ::osl::MutexGuard;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Title" ));

void lcl_addValueSequence( uno::Reference< chart2::XDataSeriesTreeParent > xParent,
                           ::std::vector< uno::Reference< chart2::XDataSequence > > & rOutSeqVector )
{
    if( xParent.is())
    {
        uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren( xParent->getChildren());
        for( sal_Int32 i = 0; i < aChildren.getLength(); ++i )
        {
            uno::Reference< chart2::XDataSource > aSource( aChildren[ i ], uno::UNO_QUERY );
            if( aSource.is())
            {
                uno::Sequence< uno::Reference< chart2::XDataSequence > > aSequences(
                    aSource->getDataSequences());
                const sal_Int32 nLength = aSequences.getLength();
                for( sal_Int32 j = 0; j < nLength; ++j )
                {
                    uno::Reference< beans::XPropertySet > xProp( aSequences[ j ], uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        ::rtl::OUString aRole;
                        if( ( xProp->getPropertyValue( C2U( "Role" )) >>= aRole ) &&
                            aRole.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "y-values" )) )
                        {
                            rOutSeqVector.push_back( aSequences[ j ] );
                        }
                    }
                }
            }
            else
            {
                uno::Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ i ], uno::UNO_QUERY );
                if( xNewParent.is())
                    lcl_addValueSequence( xNewParent, rOutSeqVector );
            }
        }
    }
}

struct lcl_DataSequenceToDoubleSeq : public ::std::unary_function<
    uno::Sequence< double >,
    uno::Reference< chart2::XDataSequence > >
{
    uno::Sequence< double > operator() ( const uno::Reference< chart2::XDataSequence > & xSeq )
    {
        uno::Reference< chart2::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
        if( xNumSeq.is())
        {
            return xNumSeq->getNumericalData();
        }
        else
        {
            uno::Sequence< uno::Any > aValues = xSeq->getData();
            uno::Sequence< double > aResult( aValues.getLength());
            const sal_Int32 nLength = aValues.getLength();
            for( sal_Int32 i = 0; i < nLength; ++i )
            {
                if( ! ( aValues[ i ] >>= aResult[ i ]) )
                {
                    ::rtl::math::setNan( & aResult[ i ] );
                }
            }
            return aResult;
        }
    }
};

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

ChartDataWrapper::ChartDataWrapper( const uno::Reference<
                                ::com::sun::star::chart2::XChartDocument > & xModel,
                            const uno::Reference< uno::XComponentContext > & xContext,
                            ::osl::Mutex & rMutex ) :
        m_rMutex( rMutex ),
        m_xContext( xContext ),
        m_xChartDoc( xModel ),
        m_aEventListenerContainer( rMutex )
{
}

ChartDataWrapper::~ChartDataWrapper()
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( *this ) );
}

::osl::Mutex & ChartDataWrapper::GetMutex() const
{
    return m_rMutex;
}

// ____ XChartDataArray ____
uno::Sequence< uno::Sequence< double > > SAL_CALL ChartDataWrapper::getData()
    throw (uno::RuntimeException)
{
    ::std::vector< uno::Reference< chart2::XDataSequence > > aSequences;

    // get data from data series
    if( m_xChartDoc.is())
    {
        uno::Reference< chart2::XDiagram > xDia(
            m_xChartDoc->getDiagram() );
        if( xDia.is())
        {
            uno::Reference< chart2::XDataSeriesTreeParent > xRoot( xDia->getTree());
            lcl_addValueSequence( xRoot, aSequences );
        }
    }

    uno::Sequence< uno::Sequence< double > > aDoubleSeq( aSequences.size() );

    ::std::transform( aSequences.begin(), aSequences.end(),
                      aDoubleSeq.getArray(),
                      lcl_DataSequenceToDoubleSeq() );

    return aDoubleSeq;
}

void SAL_CALL ChartDataWrapper::setData(
    const uno::Sequence< uno::Sequence< double > >& aData )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "not implemented" );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChartDataWrapper::getRowDescriptions()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "not implemented" );
    return uno::Sequence< ::rtl::OUString >();
}

void SAL_CALL ChartDataWrapper::setRowDescriptions(
    const uno::Sequence< ::rtl::OUString >& aRowDescriptions )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "not implemented" );
}

uno::Sequence<
    ::rtl::OUString > SAL_CALL ChartDataWrapper::getColumnDescriptions()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "not implemented" );
    return uno::Sequence< ::rtl::OUString >();
}

void SAL_CALL ChartDataWrapper::setColumnDescriptions(
    const uno::Sequence< ::rtl::OUString >& aColumnDescriptions )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "not implemented" );
}


// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL ChartDataWrapper::addChartDataChangeEventListener(
    const uno::Reference<
        ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( aListener );
}

void SAL_CALL ChartDataWrapper::removeChartDataChangeEventListener(
    const uno::Reference<
        ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

double SAL_CALL ChartDataWrapper::getNotANumber()
    throw (uno::RuntimeException)
{
    double fNan;
    ::rtl::math::setNan( & fNan );
    return fNan;
}

sal_Bool SAL_CALL ChartDataWrapper::isNotANumber( double nNumber )
    throw (uno::RuntimeException)
{
    return static_cast< sal_Bool >( ::rtl::math::isNan( nNumber ) );
}

// ____ XEventListener ____
void SAL_CALL ChartDataWrapper::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    m_xChartDoc = NULL;
    // \--
}

//  ::com::sun::star::chart::ChartDataChangeEvent aEvent;
//  aEvent.Type = chart::ChartDataChangeType_ALL;
//  aEvent.StartColumn = 0;
//  aEvent.EndColumn = 0;
//  aEvent.StartRow = 0;
//  aEvent.EndRow = 0;
void ChartDataWrapper::fireChartDataChangeEvent(
    ::com::sun::star::chart::ChartDataChangeEvent& aEvent )
{
    if( ! m_aEventListenerContainer.getLength() )
        return;

    uno::Reference< uno::XInterface > xSrc( static_cast< cppu::OWeakObject* >( this ));
    OSL_ASSERT( xSrc.is());
    if( xSrc.is() )
        aEvent.Source = xSrc;

    ::cppu::OInterfaceIteratorHelper aIter( m_aEventListenerContainer );

    while( aIter.hasMoreElements() )
    {
        uno::Reference<
            ::com::sun::star::chart::XChartDataChangeEventListener > xListener(
                aIter.next(), uno::UNO_QUERY );
        xListener->chartDataChanged( aEvent );
    }
}


// --------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > ChartDataWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartDataArray" );
    aServices[ 1 ] = C2U( "com.sun.star.chart.ChartData" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartDataWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
