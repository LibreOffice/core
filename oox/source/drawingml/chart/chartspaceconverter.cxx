/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartspaceconverter.cxx,v $
 *
 * $Revision: 1.2 $
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

#include "oox/drawingml/chart/chartspaceconverter.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include "oox/core/filterbase.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "oox/drawingml/chart/plotareaconverter.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::data::XDataReceiver;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

ChartSpaceConverter::ChartSpaceConverter( const ConverterRoot& rParent, ChartSpaceModel& rModel ) :
    ConverterBase< ChartSpaceModel >( rParent, rModel )
{
}

ChartSpaceConverter::~ChartSpaceConverter()
{
}

void ChartSpaceConverter::convertModelToDocument()
{
    /*  create data provider (virtual function in the ChartConverter class,
        derived converters may create an external data provider) */
    getChartConverter().createDataProvider( getChartDocument() );

    // attach number formatter of container document to data receiver
    try
    {
        Reference< XDataReceiver > xDataRec( getChartDocument(), UNO_QUERY_THROW );
        Reference< XNumberFormatsSupplier > xNumFmtSupp( getFilter().getModel(), UNO_QUERY_THROW );
        xDataRec->attachNumberFormatsSupplier( xNumFmtSupp );
    }
    catch( Exception& )
    {
    }

    // convert plot area (container of all chart type groups)
    PlotAreaConverter aPlotAreaConv( *this, mrModel.mxPlotArea.getOrCreate() );
    OUString aAutoTitle = aPlotAreaConv.convertModelToDocument( mrModel.mxView3D.getOrCreate() );

    // chart title
    if( !mrModel.mbAutoTitleDel ) try
    {
        /*  If the title model is missing, but the chart shows exactly one
            series, the series title is shown as chart title. */
        if( mrModel.mxTitle.is() || (aAutoTitle.getLength() > 0) )
        {
            if( aAutoTitle.getLength() == 0 )
                aAutoTitle = CREATE_OUSTRING( "Chart Title" );
            Reference< XTitled > xTitled( getChartDocument(), UNO_QUERY_THROW );
            TitleConverter aTitleConv( *this, mrModel.mxTitle.getOrCreate() );
            aTitleConv.convertModelToDocument( xTitled, aAutoTitle );
        }
    }
    catch( Exception& )
    {
    }

    // legend
    if( mrModel.mxLegend.is() )
    {
        LegendConverter aLegendConv( *this, *mrModel.mxLegend );
        aLegendConv.convertModelToDocument( getChartDocument()->getFirstDiagram() );
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

