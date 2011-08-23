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


    void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_writeInfoHelper( xMgr, xRegistry, s_component_entries );
}


void * SAL_CALL component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    fprintf(stderr, "Loading service: %s: ", implName);

    void * pResult = ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );
    
    fprintf(stderr, "%p\n", pResult);

    return pResult;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
