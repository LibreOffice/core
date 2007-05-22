/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnLineDataInterpreter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:47:57 $
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

#include "ColumnLineDataInterpreter.hxx"
#include "DataSeries.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "CommonConverters.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASINK_HPP_
#include <com/sun/star/chart2/data/XDataSink.hpp>
#endif

// #include <deque>

#include <vector>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

// explicit
ColumnLineDataInterpreter::ColumnLineDataInterpreter(
    sal_Int32 nNumberOfLines,
    const Reference< uno::XComponentContext > & xContext ) :
        DataInterpreter( xContext ),
        m_nNumberOfLines( nNumberOfLines )
{}

ColumnLineDataInterpreter::~ColumnLineDataInterpreter()
{}

// ____ XDataInterpreter ____
InterpretedData SAL_CALL ColumnLineDataInterpreter::interpretDataSource(
    const Reference< data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const Sequence< Reference< XDataSeries > >& aSeriesToReUse )
    throw (uno::RuntimeException)
{
    InterpretedData aResult(  DataInterpreter::interpretDataSource( xSource, aArguments, aSeriesToReUse ));

    // the base class should return one group
    OSL_ASSERT( aResult.Series.getLength() == 1 );
    if( aResult.Series.getLength() == 1 )
    {
        sal_Int32 nNumberOfSeries = aResult.Series[0].getLength();

        // if we have more than one series put the last nNumOfLines ones into a new group
        if( nNumberOfSeries > 1 && m_nNumberOfLines > 0 )
        {
            sal_Int32 nNumOfLines = ::std::min( m_nNumberOfLines, nNumberOfSeries - 1 );
            aResult.Series.realloc(2);

            Sequence< Reference< XDataSeries > > & rColumnDataSeries = aResult.Series[0];
            Sequence< Reference< XDataSeries > > & rLineDataSeries   = aResult.Series[1];
            rLineDataSeries.realloc( nNumOfLines );
            ::std::copy( rColumnDataSeries.getConstArray() + nNumberOfSeries - nNumOfLines,
                         rColumnDataSeries.getConstArray() + nNumberOfSeries,
                         rLineDataSeries.getArray() );
            rColumnDataSeries.realloc( nNumberOfSeries - nNumOfLines );
        }
    }

    return aResult;
}

} // namespace chart
