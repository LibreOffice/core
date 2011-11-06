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
#include "precompiled_fpicker.hxx"

//----------------------------------------------
//  includes of other projects
//----------------------------------------------
#include <cppuhelper/factory.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>
#include "FilePicker.hxx"
#include "FPServiceInfo.hxx"

#pragma warning (disable:4917)
#include "VistaFilePicker.hxx"
#include "..\misc\WinImplHelper.hxx"
#include <stdio.h>

//-----------------------------------------------
// namespace directives
//-----------------------------------------------

using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::registry  ;
using namespace ::cppu                      ;
using ::com::sun::star::ui::dialogs::XFilePicker;
using ::com::sun::star::ui::dialogs::XFilePicker2;

//------------------------------------------------
//
//------------------------------------------------

static Reference< XInterface > SAL_CALL createInstance(
    const Reference< XMultiServiceFactory >& rServiceManager )
{
    Reference< XInterface > xDlg;
    bool                    bVistaOrNewer = IsWindowsVistaOrNewer();

    if (bVistaOrNewer)
    {
        OSL_TRACE("use special (vista) system file picker ...");
        xDlg.set(
            static_cast< XFilePicker2* >(
                new ::fpicker::win32::vista::VistaFilePicker( rServiceManager ) ) );
    }
    else
    {
        OSL_TRACE("use normal system file picker ...");
        xDlg.set(
            static_cast< XFilePicker2* >(
                new CFilePicker( rServiceManager ) ) );
    }

    return xDlg;
}

//------------------------------------------------
// the three uno functions that will be exported
//------------------------------------------------

extern "C"
{

//------------------------------------------------
// component_getImplementationEnvironment
//------------------------------------------------

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//------------------------------------------------
//
//------------------------------------------------

void* SAL_CALL component_getFactory(
    const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* )
{
    void* pRet = 0;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, FILE_PICKER_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString::createFromAscii( FILE_PICKER_SERVICE_NAME );

        Reference< XSingleServiceFactory > xFactory ( createSingleFactory(
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
