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
#ifndef _CHART2_CONTROLLER_CHARTMODELHELPER_HXX
#define _CHART2_CONTROLLER_CHARTMODELHELPER_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeHighlighter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "charttoolsdllapi.hxx"

#include <vector>

namespace chart
{

/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS ChartModelHelper
{
public:
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XRangeHighlighter > createRangeHighlighter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >& xSelectionSupplier );

    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > createInternalDataProvider(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc, bool bConnectToModel );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >
        findDiagram( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >
        findDiagram( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >
        getFirstCoordinateSystem( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );

    SAL_DLLPRIVATE static ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > > getDataSeries(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );
    static ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > > getDataSeries(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

    static ::com::sun::star::uno::Reference<
    ::com::sun::star::chart2::XChartType >
        getChartTypeOfSeries(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xModel
            , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xGivenDataSeries );

    static ::com::sun::star::awt::Size getDefaultPageSize();

    static ::com::sun::star::awt::Size getPageSize(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xModel );

    static void setPageSize( const ::com::sun::star::awt::Size& rSize
                           , const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XModel >& xModel );

    static void triggerRangeHighlighting( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XModel >& xModel );

    static bool isIncludeHiddenCells( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XModel >& xChartModel );

    static bool setIncludeHiddenCells( bool bIncludeHiddenCells, const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XModel >& xChartModel );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
