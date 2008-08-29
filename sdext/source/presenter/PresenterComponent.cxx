/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterComponent.cxx,v $
 *
 * $Revision: 1.5 $
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
