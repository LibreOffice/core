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

#ifndef OOX_XLS_EXCELCHARTCONVERTER_HXX
#define OOX_XLS_EXCELCHARTCONVERTER_HXX

#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

class ExcelChartConverter : public ::oox::drawingml::chart::ChartConverter, public WorkbookHelper
{
public:
    explicit            ExcelChartConverter( const WorkbookHelper& rHelper );
    virtual             ~ExcelChartConverter();

    /** Creates an external data provider that is able to use spreadsheet data. */
    virtual void        createDataProvider(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );

    /** Creates a data sequence from the passed formula. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >& rxDataProvider,
                            const ::oox::drawingml::chart::DataSequenceModel& rDataSeq );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

