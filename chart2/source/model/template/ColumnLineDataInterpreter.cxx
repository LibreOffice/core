/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
