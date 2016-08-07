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


#include "precompiled_rptui.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implementationentry.hxx>
#include "ReportController.hxx"
#include "toolboxcontroller.hxx"
#include "statusbarcontroller.hxx"
#include "DefaultInspection.hxx"
#include "ReportComponentHandler.hxx"
#include "GeometryHandler.hxx"
#include "DataProviderHandler.hxx"

/********************************************************************************************/

using namespace ::rptui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
namespace
{

cppu::ImplementationEntry entries[] = {
    { &OReportController::create, &OReportController::getImplementationName_Static, &OReportController::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OToolboxController::create, &OToolboxController::getImplementationName_Static, &OToolboxController::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OStatusbarController::create, &OStatusbarController::getImplementationName_Static, &OStatusbarController::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &DefaultComponentInspectorModel::create, &DefaultComponentInspectorModel::getImplementationName_Static, &DefaultComponentInspectorModel::getSupportedServiceNames_static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ReportComponentHandler::create, &ReportComponentHandler::getImplementationName_Static, &ReportComponentHandler::getSupportedServiceNames_static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &GeometryHandler::create, &GeometryHandler::getImplementationName_Static, &GeometryHandler::getSupportedServiceNames_static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &DataProviderHandler::create, &DataProviderHandler::getImplementationName_Static, &DataProviderHandler::getSupportedServiceNames_static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
