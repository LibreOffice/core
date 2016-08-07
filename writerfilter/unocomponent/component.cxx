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



#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
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


SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL
component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    fprintf(stderr, "Loading service: %s: ", implName);

    void * pResult = ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );

    fprintf(stderr, "%p\n", pResult);

    return pResult;
}

}
