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

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL rptui_component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
