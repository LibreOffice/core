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
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pResult = ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, g_entries_chart2_model );

    if( ! pResult )
        pResult = ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey,
            ChartTypeEntriesForServiceRegistration::getImplementationEntries() );

    return pResult;
}
}
//=========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
