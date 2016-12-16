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
#include "precompiled_dtrans.hxx"

//-----------------------------------------------------------------
//  includes of other projects
//-----------------------------------------------------------------
#include <cppuhelper/factory.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>

#ifndef _COM_SUN_STAR_DATATRANSFER_XDATAFLAVORCONTAINER_HPP_
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#endif
#include "ftransl.hxx"

//-----------------------------------------------------------------
// some defines
//-----------------------------------------------------------------

// the service names
#define SERVICE_NAME  "com.sun.star.datatransfer.DataFormatTranslator"

// the implementation names
#define IMPL_NAME  "com.sun.star.datatransfer.DataFormatTranslator"

// the registry key names
// a key under which this service will be registered, Format: -> "/ImplName/UNO/SERVICES/ServiceName"
//                        <     Implementation-Name    ></UNO/SERVICES/><    Service-Name           >
#define REGKEY_NAME  "/com.sun.star.datatransfer.DataFormatTranslator/UNO/SERVICES/com.sun.star.datatransfer.DataFormatTranslator"

//-----------------------------------------------------------------------------------------------------------
// namespace directives
//-----------------------------------------------------------------------------------------------------------

using namespace ::rtl                       ;
using namespace ::cppu                      ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::registry  ;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;

//-----------------------------------------------------------------
// create a static object to initialize the shell9x library
//-----------------------------------------------------------------

namespace
{

    //-----------------------------------------------------------------------------------------------------------
    // functions to create a new Clipboad instance; is needed by factory helper implementation
    // @param rServiceManager - service manager, useful if the component needs other uno services
    // so we should give it to every UNO-Implementation component
    //-----------------------------------------------------------------------------------------------------------

    Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >& rServiceManager )
    {
        return Reference< XInterface >( static_cast< XDataFormatTranslator* >( new CDataFormatTranslator( rServiceManager ) ) );
    }
}

//-----------------------------------------------------------------------------------------------------------
// the 3 important functions which will be exported
//-----------------------------------------------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------
// component_getImplementationEnvironment
//----------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//----------------------------------------------------------------------
// component_getFactory
// returns a factory to create XFilePicker-Services
//----------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* /*pRegistryKey*/ )
{
    void* pRet = 0;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );

        Reference< XSingleServiceFactory > xFactory ( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createInstance,
            aSNS ) );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

} // extern "C"
