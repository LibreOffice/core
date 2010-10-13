/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "ColumnLineDataInterpreter.hxx"
#include "DataSeries.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "CommonConverters.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
