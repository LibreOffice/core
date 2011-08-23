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

#include "oox/drawingml/chart/chartconverter.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "oox/drawingml/chart/chartspaceconverter.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::chart2::data::XDataProvider;
using ::com::sun::star::chart2::data::XDataSequence;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

ChartConverter::ChartConverter()
{
}

ChartConverter::~ChartConverter()
{
}

void ChartConverter::convertFromModel( XmlFilterBase& rFilter,
        ChartSpaceModel& rChartModel, const Reference< XChartDocument >& rxChartDoc,
        const Reference< XShapes >& rxExternalPage, const Point& rChartPos, const Size& rChartSize )
{
    OSL_ENSURE( rxChartDoc.is(), "ChartConverter::convertFromModel - missing chart document" );
    if( rxChartDoc.is() )
    {
        ConverterRoot aConvBase( rFilter, *this, rChartModel, rxChartDoc, rChartSize );
        ChartSpaceConverter aSpaceConv( aConvBase, rChartModel );
        aSpaceConv.convertFromModel( rxExternalPage, rChartPos );
    }
}

void ChartConverter::createDataProvider( const Reference< XChartDocument >& rxChartDoc )
{
    try
    {
        if( !rxChartDoc->hasInternalDataProvider() )
            rxChartDoc->createInternalDataProvider( sal_False );
    }
    catch( Exception& )
    {
    }
}

Reference< XDataSequence > ChartConverter::createDataSequence( const Reference< XDataProvider >&, const DataSequenceModel& )
{
    return 0;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

