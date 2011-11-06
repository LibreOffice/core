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
#include "SalGtkFilePicker.hxx"
#include "SalGtkFolderPicker.hxx"
#include <vcl/svapp.hxx>
#include "FPServiceInfo.hxx"

#include <glib-object.h>

extern      const guint gtk_major_version;
extern      const guint gtk_minor_version;

//-----------------------------------------------
// namespace directives
//-----------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using ::com::sun::star::ui::dialogs::XFilePicker;
using ::com::sun::star::ui::dialogs::XFilePicker2;
using ::com::sun::star::ui::dialogs::XFolderPicker;

//------------------------------------------------
//
//------------------------------------------------

static Reference< XInterface > SAL_CALL createFileInstance(
    const Reference< XMultiServiceFactory >& rServiceManager )
{
    return Reference< XInterface >(
        static_cast< XFilePicker2* >(
            new SalGtkFilePicker( rServiceManager ) ) );
}

static Reference< XInterface > SAL_CALL createFolderInstance(
    const Reference< XMultiServiceFactory >& rServiceManager )
{
    return Reference< XInterface >(
        static_cast< XFolderPicker* >(
            new SalGtkFolderPicker( rServiceManager ) ) );
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
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//------------------------------------------------
//
//------------------------------------------------

void* SAL_CALL component_getFactory(
    const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* /*pRegistryKey*/ )
{
    void* pRet = 0;

    if( pSrvManager )
    {
            if (
                 /* crude gtkplug check */ !g_type_from_name( "GdkDisplay" ) ||
                 /* old version */ !( gtk_major_version >= 2 && gtk_minor_version >= 4 )
               )
            {
                    return 0;
            }

            Reference< XSingleServiceFactory > xFactory;

            if (0 == rtl_str_compare(pImplName, FILE_PICKER_IMPL_NAME))
            {
                Sequence< OUString > aSNS( 1 );
                aSNS.getArray( )[0] =
                    OUString::createFromAscii(FILE_PICKER_SERVICE_NAME);

                xFactory = createSingleFactory(
                    reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
                    OUString::createFromAscii( pImplName ),
                    createFileInstance,
                    aSNS );
            }
            else if (0 == rtl_str_compare(pImplName, FOLDER_PICKER_IMPL_NAME))
            {
                Sequence< OUString > aSNS( 1 );
                aSNS.getArray( )[0] =
                    OUString::createFromAscii(FOLDER_PICKER_SERVICE_NAME);

                xFactory = createSingleFactory(
                    reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
                    OUString::createFromAscii( pImplName ),
                    createFolderInstance,
                    aSNS );
            }

            if ( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
    }

    return pRet;
}

} // extern "C"

