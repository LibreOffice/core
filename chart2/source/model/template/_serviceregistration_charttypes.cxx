/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _serviceregistration_charttypes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:53:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include "PieChartType.hxx"
#include "ScatterChartType.hxx"

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
   ,{ 0, 0, 0, 0, 0, 0 }
};

} // anonymous namespace

// static
struct ::cppu::ImplementationEntry *
    ChartTypeEntriesForServiceRegistration::getImplementationEntries()
{
    return g_entries_chart2_charttypes;
}
