/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlservices.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:06:52 $
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
#include "precompiled_reportdesign.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef RPT_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef RPT_XMLEXPORT_HXX
#include "xmlExport.hxx"
#endif

#ifndef RPTXML_DBLOADER2_HXX
#include "dbloader2.hxx"
#endif
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

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}
