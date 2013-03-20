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

#ifndef _XMLOFF_SCH_XML_SERIESHELPER_HXX
#define _XMLOFF_SCH_XML_SERIESHELPER_HXX

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>
#include <map>
#include <iterator>

class SchXMLSeriesHelper
{
public:
    static ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >
            getDataSeriesFromDiagram(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );
    static ::std::map< ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >, sal_Int32 >
            getDataSeriesIndexMapFromDiagram(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool isCandleStickSeries(
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDataSeries >& xSeries
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel  );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > createOldAPISeriesPropertySet(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > createOldAPIDataPointPropertySet(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries
                    , sal_Int32 nPointIndex
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel );
};

// _XMLOFF_SCH_XML_SERIESHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
