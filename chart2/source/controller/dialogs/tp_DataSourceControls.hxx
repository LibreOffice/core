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
#ifndef CHART2_TP_DATASOURCECONTROLS_HXX
#define CHART2_TP_DATASOURCECONTROLS_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

// header for class SvTabListBox
#include <svtools/svtabbx.hxx>
// header for class SvTreeListBox
#include <svtools/svtreebx.hxx>

namespace chart
{

class SeriesEntry : public ::SvLBoxEntry
{
public:
    virtual ~SeriesEntry();

    /// the corresponding data series
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >
        m_xDataSeries;

    /// the chart type that contains the series (via XDataSeriesContainer)
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >
        m_xChartType;
};

class SeriesListBox : public ::SvTreeListBox
{
public:
    explicit SeriesListBox( Window* pParent, const ResId & rResId );
    ~SeriesListBox( );

    virtual SvLBoxEntry* CreateEntry() const;
};

} //  namespace chart

// CHART2_TP_DATASOURCECONTROLS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
