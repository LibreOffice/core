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
#include "precompiled_sdext.hxx"

#include "PresenterComponent.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>

#include "PresenterExtensionIdentifier.hxx"
#include "PresenterProtocolHandler.hxx"
#include "PresenterScreen.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cppu;
using namespace osl;
using ::rtl::OUString;



namespace sdext { namespace presenter {

static OUString gsBasePath;

::rtl::OUString PresenterComponent::GetBasePath (
    const Reference<XComponentContext>& rxComponentContext)
{
    return GetBasePath(rxComponentContext, gsExtensionIdentifier);
}




::rtl::OUString PresenterComponent::GetBasePath (
    const Reference<XComponentContext>& rxComponentContext,
    const OUString& rsExtensionIdentifier)
{
    if (gsBasePath.getLength() == 0)
    {
        // Determine the base path of the bitmaps.
        Reference<deployment::XPackageInformationProvider> xInformationProvider (
            rxComponentContext->getValueByName(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/com.sun.star.deployment.PackageInformationProvider"))),
            UNO_QUERY);
        if (xInformationProvider.is())
        {
            try
            {
                gsBasePath = xInformationProvider->getPackageLocation(rsExtensionIdentifier)
                    + OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
            }
            catch(deployment::DeploymentException&)
            {
            }
        }
    }

    return gsBasePath;
}



rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static struct ImplementationEntry gServiceEntries[] =
{
    {
        PresenterProtocolHandler::Create,
        PresenterProtocolHandler::getImplementationName_static,
        PresenterProtocolHandler::getSupportedServiceNames_static,
        createSingleComponentFactory, &g_moduleCount.modCnt, 0
    },
    {
        PresenterScreenJob::Create,
        PresenterScreenJob::getImplementationName_static,
        PresenterScreenJob::getSupportedServiceNames_static,
        createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};




extern "C"
{
    sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
    {
        return g_moduleCount.canUnload( &g_moduleCount , pTime );
    }




    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }




    sal_Bool SAL_CALL component_writeInfo(
        void * pServiceManager, void * pRegistryKey )
    {
        return component_writeInfoHelper(pServiceManager, pRegistryKey, gServiceEntries);
    }




    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , gServiceEntries);
    }

}

} } // end of namespace sdext::presenter
