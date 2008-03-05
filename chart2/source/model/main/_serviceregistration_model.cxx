/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _serviceregistration_model.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:15:40 $
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
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#ifndef _CHARTMODEL_HXX
#include "ChartModel.hxx"
#endif

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
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return (::cppu::component_writeInfoHelper(
                pServiceManager, pRegistryKey, g_entries_chart2_model ) &&
            ::cppu::component_writeInfoHelper(
                pServiceManager, pRegistryKey,
                ChartTypeEntriesForServiceRegistration::getImplementationEntries() ));
}
//==================================================================================================
void * SAL_CALL component_getFactory(
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
