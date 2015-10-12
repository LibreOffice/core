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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_EXCELCHARTCONVERTER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_EXCELCHARTCONVERTER_HXX

#include <oox/drawingml/chart/chartconverter.hxx>
#include "workbookhelper.hxx"

namespace oox {
namespace xls {

class ExcelChartConverter : public ::oox::drawingml::chart::ChartConverter, public WorkbookHelper
{
public:
    explicit            ExcelChartConverter( const WorkbookHelper& rHelper );
    virtual             ~ExcelChartConverter();

    /** Creates an external data provider that is able to use spreadsheet data. */
    virtual void        createDataProvider(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc ) override;

    /** Creates a data sequence from the passed formula. */
    virtual css::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequence(
            const css::uno::Reference<css::chart2::data::XDataProvider>& rxDataProvider,
            const oox::drawingml::chart::DataSequenceModel& rDataSeq, const OUString& rRole ) override;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
