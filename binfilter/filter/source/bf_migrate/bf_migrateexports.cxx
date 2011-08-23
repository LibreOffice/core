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
#include <stdio.h>

#include <osl/thread.h>


#include <legacysmgr/legacy_binfilters_smgr.hxx>

#include <bf_migratefilter.hxx>

#include <bf_sfx2/objuno.hxx>

namespace binfilter {

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(const sal_Char** ppEnvTypeName, uno_Environment** ppEnv)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(void* pServiceManager, void* pRegistryKey)
{
    if(pRegistryKey)
    {
        try
        {
            sal_Bool bLegacySmgrWriteInfoDidWork(legacysmgr_component_writeInfo
                ( reinterpret_cast<XMultiServiceFactory*>( pServiceManager), reinterpret_cast<XRegistryKey*> (pRegistryKey) ));
            OSL_ENSURE(bLegacySmgrWriteInfoDidWork, "### LegacyServiceManager writeInfo failed!" );

            Reference< XRegistryKey > xNewKey;
            xNewKey = reinterpret_cast< XRegistryKey * >(pRegistryKey)->createKey(bf_MigrateFilter_getImplementationName());
            xNewKey = xNewKey->createKey(OUString::createFromAscii("/UNO/SERVICES"));

            Sequence< OUString > rSNL = bf_MigrateFilter_getSupportedServiceNames();

            sal_Int32 nPos;
            for(nPos=rSNL.getLength(); nPos--; )
                xNewKey->createKey(rSNL[nPos]);

            // standalone document info
            xNewKey = reinterpret_cast< XRegistryKey * >(pRegistryKey)->createKey(binfilter::SfxStandaloneDocumentInfoObject::impl_getStaticImplementationName());
            xNewKey = xNewKey->createKey(OUString::createFromAscii("/UNO/SERVICES"));

            rSNL = binfilter::SfxStandaloneDocumentInfoObject::impl_getStaticSupportedServiceNames();
            for(nPos=rSNL.getLength(); nPos--; )
                xNewKey->createKey(rSNL[nPos]);

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE(sal_False, "### InvalidRegistryException!");
        }
    }

    return sal_False;
}

//==================================================================================================
void* SAL_CALL component_getFactory(const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey)
{
    void* pRet = 0;

    OUString implName = OUString::createFromAscii(pImplName);

    if(pServiceManager && implName.equals(bf_MigrateFilter_getImplementationName()))
    {
        Reference< XSingleServiceFactory > xFactory(createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >(pServiceManager),
            OUString::createFromAscii(pImplName),
            bf_MigrateFilter_createInstance, bf_MigrateFilter_getSupportedServiceNames()));

        if(xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }

        // init LegacyServiceFactory
        legacysmgr_component_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory *>(pServiceManager),
            reinterpret_cast<XRegistryKey*> (pRegistryKey) );
    }
    else if(pServiceManager && implName.equals(SfxStandaloneDocumentInfoObject::impl_getStaticImplementationName()))
    {
        Reference< XSingleServiceFactory > xFactory(createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >(pServiceManager),
            OUString::createFromAscii(pImplName),
            bf_BinaryDocInfo_createInstance, binfilter::SfxStandaloneDocumentInfoObject::impl_getStaticSupportedServiceNames()));

        if(xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }

        // init LegacyServiceFactory
        legacysmgr_component_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory *>(pServiceManager),
            reinterpret_cast<XRegistryKey*> (pRegistryKey) );
    }

    return pRet;
}
}

// eof
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
