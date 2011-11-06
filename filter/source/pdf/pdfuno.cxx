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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <stdio.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <pdffilter.hxx>
#include <pdfdialog.hxx>
#include <pdfinteract.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    // -------------------------------------------------------------------------

    SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
    {
        OUString    aImplName( OUString::createFromAscii( pImplName ) );
        void*       pRet = 0;

        if( pServiceManager )
        {
            Reference< XSingleServiceFactory > xFactory;

            if( aImplName.equals( PDFFilter_getImplementationName() ) )
            {
                xFactory = createSingleFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ),
                                                OUString::createFromAscii( pImplName ),
                                                PDFFilter_createInstance, PDFFilter_getSupportedServiceNames() );

            }
            else if( aImplName.equals( PDFDialog_getImplementationName() ) )
            {
                xFactory = createSingleFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ),
                                                OUString::createFromAscii( pImplName ),
                                                PDFDialog_createInstance, PDFDialog_getSupportedServiceNames() );

            }
            else if( aImplName.equals( PDFInteractionHandler_getImplementationName() ) )
            {
                xFactory = createSingleFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ),
                                                OUString::createFromAscii( pImplName ),
                                                PDFInteractionHandler_createInstance, PDFInteractionHandler_getSupportedServiceNames() );

            }

            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }

        return pRet;
    }
}
