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

#ifndef _CPPUHELPTER_FACTORY_
#include <cppuhelper/factory.hxx>
#endif
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <debugservices/doctok/DocTokTestService.hxx>
#include <debugservices/doctok/DocTokAnalyzeService.hxx>
#include <debugservices/ooxml/OOXMLTestService.hxx>
#include <debugservices/ooxml/OOXMLAnalyzeService.hxx>

#include <stdio.h>

using namespace com::sun::star;


extern "C"
{
/* shared lib exports implemented with helpers */
static struct ::cppu::ImplementationEntry s_component_entries [] =
{//uno -l writerfilter.uno.dll -c debugservices.rtftok.ScannerTestService -- a b c
    DOCTOK_SCANNERTESTSERVICE_COMPONENT_ENTRY, /* debugservices.doctok.ScannerTestService */
    DOCTOK_ANALYZESERVICE_COMPONENT_ENTRY, /* debugservices.doctok.AnalyzeService */
    OOXML_SCANNERTESTSERVICE_COMPONENT_ENTRY, /* debugservices.ooxml.ScannerTestService */
    OOXML_ANALYZESERVICE_COMPONENT_ENTRY, /* debugservices.ooxml.AnalyzeService */
    { 0, 0, 0, 0, 0, 0 } // terminate with NULL
};

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    fprintf(stderr, "Loading service: %s: ", implName);

    void * pResult = ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );

    fprintf(stderr, "%p\n", pResult);

    return pResult;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
