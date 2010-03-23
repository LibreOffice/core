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

#include "oox/drawingml/chart/datasourceconverter.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::chart2::data::XDataSequence;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

DataSequenceConverter::DataSequenceConverter( const ConverterRoot& rParent, DataSequenceModel& rModel ) :
    ConverterBase< DataSequenceModel >( rParent, rModel )
{
}

DataSequenceConverter::~DataSequenceConverter()
{
}

Reference< XDataSequence > DataSequenceConverter::createDataSequence( const OUString& rRole )
{
    // create data sequence from data source model (virtual call at chart converter)
    Reference< XDataSequence > xDataSeq = getChartConverter().createDataSequence( getChartDocument()->getDataProvider(), mrModel );

    // set sequence role
    PropertySet aSeqProp( xDataSeq );
    aSeqProp.setProperty( PROP_Role, rRole );

    return xDataSeq;
}

// ============================================================================

DataSourceConverter::DataSourceConverter( const ConverterRoot& rParent, DataSourceModel& rModel ) :
    ConverterBase< DataSourceModel >( rParent, rModel )
{
}

DataSourceConverter::~DataSourceConverter()
{
}

Reference< XDataSequence > DataSourceConverter::createDataSequence( const OUString& rRole )
{
    Reference< XDataSequence > xDataSeq;
    if( mrModel.mxDataSeq.is() )
    {
        DataSequenceConverter aDataSeqConv( *this, *mrModel.mxDataSeq );
        xDataSeq = aDataSeqConv.createDataSequence( rRole );
    }
    return xDataSeq;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

