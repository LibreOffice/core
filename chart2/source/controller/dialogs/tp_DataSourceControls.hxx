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
#ifndef CHART2_TP_DATASOURCECONTROLS_HXX
#define CHART2_TP_DATASOURCECONTROLS_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

// header for class SvTabListBox
#include <svtools/svtabbx.hxx>
// header for class SvTreeListBox
#include <svtools/treelistbox.hxx>

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
