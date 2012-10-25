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
