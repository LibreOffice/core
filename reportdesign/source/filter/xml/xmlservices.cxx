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
#include "precompiled_reportdesign.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implementationentry.hxx>
#include "xmlfilter.hxx"
#include "xmlExport.hxx"
#include "dbloader2.hxx"
#include "xmlExportDocumentHandler.hxx"
#include "xmlImportDocumentHandler.hxx"


/********************************************************************************************/

using namespace ::rptxml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
/********************************************************************************************/
// registry functions
namespace
{

cppu::ImplementationEntry entries[] = {
    { &ORptFilter::create, &ORptFilter::getImplementationName_Static, &ORptFilter::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptTypeDetection::create, &ORptTypeDetection::getImplementationName_Static, &ORptTypeDetection::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ODBFullExportHelper::create, &ODBFullExportHelper::getImplementationName_Static, &ODBFullExportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptExportHelper::create, &ORptExportHelper::getImplementationName_Static, &ORptExportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptExport::create, &ORptExport::getImplementationName_Static, &ORptExport::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptContentExportHelper::create, &ORptContentExportHelper::getImplementationName_Static, &ORptContentExportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptStylesExportHelper::create, &ORptStylesExportHelper::getImplementationName_Static, &ORptStylesExportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptMetaExportHelper::create, &ORptMetaExportHelper::getImplementationName_Static, &ORptMetaExportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },

    { &ORptImportHelper::create, &ORptImportHelper::getImplementationName_Static, &ORptImportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptContentImportHelper::create, &ORptContentImportHelper::getImplementationName_Static, &ORptContentImportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptStylesImportHelper::create, &ORptStylesImportHelper::getImplementationName_Static, &ORptStylesImportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ExportDocumentHandler::create, &ExportDocumentHandler::getImplementationName_Static, &ExportDocumentHandler::getSupportedServiceNames_static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ImportDocumentHandler::create, &ImportDocumentHandler::getImplementationName_Static, &ImportDocumentHandler::getSupportedServiceNames_static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &ORptMetaImportHelper::create, &ORptMetaImportHelper::getImplementationName_Static, &ORptMetaImportHelper::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
}

extern "C" void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */