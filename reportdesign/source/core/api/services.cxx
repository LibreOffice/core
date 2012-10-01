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
#include "sal/types.h"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/implementationentry.hxx>
#include "ReportDefinition.hxx"
#include "FormattedField.hxx"
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
SAL_DLLPUBLIC_EXPORT void * SAL_CALL rpt_component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
