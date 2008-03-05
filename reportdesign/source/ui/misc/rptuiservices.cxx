/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rptuiservices.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:12:15 $
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

#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#include "toolboxcontroller.hxx"
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
