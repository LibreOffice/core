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

#include "_serviceregistration_charttypes.hxx"

#include "AreaChartType.hxx"
#include "BarChartType.hxx"
#include "CandleStickChartType.hxx"
#include "ColumnChartType.hxx"
#include "LineChartType.hxx"
#include "NetChartType.hxx"
#include "FilledNetChartType.hxx"
#include "PieChartType.hxx"
#include "ScatterChartType.hxx"
#include "BubbleChartType.hxx"

namespace
{
static const struct ::cppu::ImplementationEntry g_entries_chart2_charttypes[] =
{
    {
          ::chart::AreaChartType::create
        , ::chart::AreaChartType::getImplementationName_Static
        , ::chart::AreaChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::BarChartType::create
        , ::chart::BarChartType::getImplementationName_Static
        , ::chart::BarChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::CandleStickChartType::create
        , ::chart::CandleStickChartType::getImplementationName_Static
        , ::chart::CandleStickChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ColumnChartType::create
        , ::chart::ColumnChartType::getImplementationName_Static
        , ::chart::ColumnChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::LineChartType::create
        , ::chart::LineChartType::getImplementationName_Static
        , ::chart::LineChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::NetChartType::create
        , ::chart::NetChartType::getImplementationName_Static
        , ::chart::NetChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::FilledNetChartType::create
        , ::chart::FilledNetChartType::getImplementationName_Static
        , ::chart::FilledNetChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::PieChartType::create
        , ::chart::PieChartType::getImplementationName_Static
        , ::chart::PieChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ScatterChartType::create
        , ::chart::ScatterChartType::getImplementationName_Static
        , ::chart::ScatterChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
        ,{
          ::chart::BubbleChartType::create
        , ::chart::BubbleChartType::getImplementationName_Static
        , ::chart::BubbleChartType::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{ 0, 0, 0, 0, 0, 0 }
};

} // anonymous namespace

const struct ::cppu::ImplementationEntry *
    ChartTypeEntriesForServiceRegistration::getImplementationEntries()
{
    return g_entries_chart2_charttypes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
