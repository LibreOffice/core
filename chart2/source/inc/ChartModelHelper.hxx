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

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
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

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
