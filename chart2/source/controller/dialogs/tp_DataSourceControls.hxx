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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCECONTROLS_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCECONTROLS_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

#include <svtools/svtabbx.hxx>
#include <svtools/treelistbox.hxx>
#include "svtools/treelistentry.hxx"

namespace chart
{

class SeriesEntry : public ::SvTreeListEntry
{
public:
    virtual ~SeriesEntry();

    /// the corresponding data series
    css::uno::Reference< css::chart2::XDataSeries > m_xDataSeries;

    /// the chart type that contains the series (via XDataSeriesContainer)
    css::uno::Reference< css::chart2::XChartType > m_xChartType;
};

class SeriesListBox : public ::SvTreeListBox
{
public:
    explicit SeriesListBox(vcl::Window* pParent, WinBits nStyle);

    virtual SvTreeListEntry* CreateEntry() const override;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_DATASOURCECONTROLS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
