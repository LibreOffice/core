/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartController.hxx"
#include "ChartFrameloader.hxx"
#include "dlg_CreationWizard_UNO.hxx"
#include "dlg_ChartType_UNO.hxx"
#include "ChartDocumentWrapper.hxx"
#include "AccessibleChartView.hxx"
#include "ElementSelector.hxx"
#include "ShapeToolbarController.hxx"
#include <cppuhelper/implementationentry.hxx>

static struct ::cppu::ImplementationEntry g_entries_chart2_controller[] =
{
    {
          ::chart::ChartController::create
        , ::chart::ChartController::getImplementationName_Static
        , ::chart::ChartController::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ChartFrameLoader::create
        , ::chart::ChartFrameLoader::getImplementationName_Static
        , ::chart::ChartFrameLoader::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::CreationWizardUnoDlg::create
        , ::chart::CreationWizardUnoDlg::getImplementationName_Static
        , ::chart::CreationWizardUnoDlg::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ChartTypeUnoDlg::Create
        , ::chart::ChartTypeUnoDlg::getImplementationName_Static
        , ::chart::ChartTypeUnoDlg::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::wrapper::ChartDocumentWrapper::create
        , ::chart::wrapper::ChartDocumentWrapper::getImplementationName_Static
        , ::chart::wrapper::ChartDocumentWrapper::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ElementSelectorToolbarController::create
        , ::chart::ElementSelectorToolbarController::getImplementationName_Static
        , ::chart::ElementSelectorToolbarController::getSupportedServiceNames_Static
        , ::cppu::createSingleComponentFactory
        , 0
        , 0
    }
    ,{
          ::chart::ShapeToolbarController::create
        , ::chart::ShapeToolbarController::getImplementationName_Static
        , ::chart::ShapeToolbarController::getSupportedServiceNames_Static
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
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries_chart2_controller );
}
}
//=========================================================================
