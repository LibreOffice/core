/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:20:23 $
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

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef REPORTDESIGN_API_REPORTDEFINITION_HXX
#include "ReportDefinition.hxx"
#endif
#ifndef RPT_FORMATTEDFIELD_HXX
#include "FormattedField.hxx"
#endif
#include "FixedText.hxx"
#include "ImageControl.hxx"
#include "FormatCondition.hxx"
#include "ReportEngineJFree.hxx"
#include "Function.hxx"
#include "Shape.hxx"
#include "FixedLine.hxx"

/********************************************************************************************/

using namespace ::reportdesign;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
namespace
{

cppu::ImplementationEntry entries[] = {
    { &OReportDefinition::create, &OReportDefinition::getImplementationName_Static, &OReportDefinition::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OFormattedField::create, &OFormattedField::getImplementationName_Static, &OFormattedField::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OFixedText::create, &OFixedText::getImplementationName_Static, &OFixedText::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OImageControl::create, &OImageControl::getImplementationName_Static, &OImageControl::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OFormatCondition::create, &OFormatCondition::getImplementationName_Static, &OFormatCondition::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OReportEngineJFree::create, &OReportEngineJFree::getImplementationName_Static, &OReportEngineJFree::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OFunction::create, &OFunction::getImplementationName_Static, &OFunction::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OShape::create, &OShape::getImplementationName_Static, &OShape::getSupportedServiceNames_Static,
        &cppu::createSingleComponentFactory, 0, 0 },
    { &OFixedLine::create, &OFixedLine::getImplementationName_Static, &OFixedLine::getSupportedServiceNames_Static,
    &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
}

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}
} // extern "C"
