/*************************************************************************
 *
 *  $RCSfile: _serviceregistration_model.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:32:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#ifndef _CHARTMODEL_HXX
#include "ChartModel.hxx"
#endif
#ifndef CHART_FILEDATAPROVIDER_HXX
#include "FileDataProvider.hxx"
#endif
#ifndef CHART_FILEDATASOURCE_HXX
#include "FileDataSource.hxx"
#endif

#include "Diagram.hxx"
#include "Legend.hxx"
#include "Axis.hxx"
#include "Grid.hxx"
#include "Title.hxx"
#include "FormattedString.hxx"
#include "PageBackground.hxx"

#include "DataSeriesTree.hxx"
#include "ChartTypeGroup.hxx"
#include "ContinuousScaleGroup.hxx"
#include "DiscreteScaleGroup.hxx"

#include "ChartTypeManager.hxx"
// #include "BoundedCoordinateSystem.hxx"
#include "Scale.hxx"

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
          ::chart::FileDataProvider::create
        , ::chart::FileDataProvider::getImplementationName_Static
        , ::chart::FileDataProvider::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::FileDataSource::create
        , ::chart::FileDataSource::getImplementationName_Static
        , ::chart::FileDataSource::getSupportedServiceNames_Static
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
          ::chart::Grid::create
        , ::chart::Grid::getImplementationName_Static
        , ::chart::Grid::getSupportedServiceNames_Static
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
          ::chart::DataSeriesTree::create
        , ::chart::DataSeriesTree::getImplementationName_Static
        , ::chart::DataSeriesTree::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ChartTypeGroup::create
        , ::chart::ChartTypeGroup::getImplementationName_Static
        , ::chart::ChartTypeGroup::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ContinuousScaleGroup::create
        , ::chart::ContinuousScaleGroup::getImplementationName_Static
        , ::chart::ContinuousScaleGroup::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::DiscreteScaleGroup::create
        , ::chart::DiscreteScaleGroup::getImplementationName_Static
        , ::chart::DiscreteScaleGroup::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ContinuousStackableScaleGroup::create
        , ::chart::ContinuousStackableScaleGroup::getImplementationName_Static
        , ::chart::ContinuousStackableScaleGroup::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::DiscreteStackableScaleGroup::create
        , ::chart::DiscreteStackableScaleGroup::getImplementationName_Static
        , ::chart::DiscreteStackableScaleGroup::getSupportedServiceNames_Static
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
//     ,{
//           ::chart::BoundedCoordinateSystem::create
//         , ::chart::BoundedCoordinateSystem::getImplementationName_Static
//         , ::chart::BoundedCoordinateSystem::getSupportedServiceNames_Static
//         , ::cppu::createSingleComponentFactory
//         , 0
//         , 0
//     }
    ,{
          ::chart::Scale::create
        , ::chart::Scale::getImplementationName_Static
        , ::chart::Scale::getSupportedServiceNames_Static
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
   ,{ 0, 0, 0, 0, 0, 0 }
};

// component exports
extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
        pServiceManager, pRegistryKey, g_entries_chart2_model );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_model );
}
}
//=========================================================================
