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

#include <cppuhelper/implementationentry.hxx>
#include "ChartModel.hxx"

#include "Diagram.hxx"
#include "Legend.hxx"
#include "Axis.hxx"
#include "GridProperties.hxx"
#include "Title.hxx"
#include "FormattedString.hxx"
#include "PageBackground.hxx"
#include "DataSeries.hxx"
#include "PolarCoordinateSystem.hxx"
#include "CartesianCoordinateSystem.hxx"

#include "ChartTypeManager.hxx"
#include "XMLFilter.hxx"

#include "_serviceregistration_charttypes.hxx"

static struct ::cppu::ImplementationEntry g_entries_chart2_model[] =
{
    {
          ::chart::ChartModel::create
        , ::chart::ChartModel::getImplementationName_Static
        , ::chart::ChartModel::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::Diagram::create
        , ::chart::Diagram::getImplementationName_Static
        , ::chart::Diagram::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::Legend::create
        , ::chart::Legend::getImplementationName_Static
        , ::chart::Legend::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::Axis::create
        , ::chart::Axis::getImplementationName_Static
        , ::chart::Axis::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::GridProperties::create
        , ::chart::GridProperties::getImplementationName_Static
        , ::chart::GridProperties::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::Title::create
        , ::chart::Title::getImplementationName_Static
        , ::chart::Title::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }

    ,{
          ::chart::FormattedString::create
        , ::chart::FormattedString::getImplementationName_Static
        , ::chart::FormattedString::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }

    ,{
          ::chart::ChartTypeManager::create
        , ::chart::ChartTypeManager::getImplementationName_Static
        , ::chart::ChartTypeManager::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::PageBackground::create
        , ::chart::PageBackground::getImplementationName_Static
        , ::chart::PageBackground::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::DataSeries::create
        , ::chart::DataSeries::getImplementationName_Static
        , ::chart::DataSeries::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::XMLFilter::create
        , ::chart::XMLFilter::getImplementationName_Static
        , ::chart::XMLFilter::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::XMLReportFilterHelper::create
        , ::chart::XMLReportFilterHelper::getImplementationName_Static
        , ::chart::XMLFilter::getSupportedServiceNames_Static // we support the same, because we are derived from
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::PolarCoordinateSystem2d::create
        , ::chart::PolarCoordinateSystem2d::getImplementationName_Static
        , ::chart::PolarCoordinateSystem2d::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::PolarCoordinateSystem3d::create
        , ::chart::PolarCoordinateSystem3d::getImplementationName_Static
        , ::chart::PolarCoordinateSystem3d::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::CartesianCoordinateSystem2d::create
        , ::chart::CartesianCoordinateSystem2d::getImplementationName_Static
        , ::chart::CartesianCoordinateSystem2d::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
     ,{
          ::chart::CartesianCoordinateSystem3d::create
        , ::chart::CartesianCoordinateSystem3d::getImplementationName_Static
        , ::chart::CartesianCoordinateSystem3d::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
   ,{ 0, 0, 0, 0, 0, 0 }
};

// component exports
extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL chartview_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
SAL_DLLPUBLIC_EXPORT void * SAL_CALL charttools_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
SAL_DLLPUBLIC_EXPORT void * SAL_CALL chartcore_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pResult = ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, g_entries_chart2_model );

    if( ! pResult )
        pResult = ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey,
            ChartTypeEntriesForServiceRegistration::getImplementationEntries() );
    if (!pResult)
    {
        pResult = charttools_component_getFactory(
            pImplName, pServiceManager, pRegistryKey);
    }

    if (!pResult)
    {
        pResult = chartview_component_getFactory(
            pImplName, pServiceManager, pRegistryKey);
    }

    return pResult;
}
}
//=========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
