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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

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
static struct ::cppu::ImplementationEntry g_entries_chart2_charttypes[] =
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

struct ::cppu::ImplementationEntry *
    ChartTypeEntriesForServiceRegistration::getImplementationEntries()
{
    return g_entries_chart2_charttypes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
