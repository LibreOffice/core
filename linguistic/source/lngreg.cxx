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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"


#include <cppuhelper/factory.hxx>   // helper for factories
#include <rtl/string.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::lang;

using namespace com::sun::star::registry;

extern void * SAL_CALL LngSvcMgr_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void * /*pRegistryKey*/
);

extern void * SAL_CALL DicList_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

void * SAL_CALL LinguProps_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

extern void * SAL_CALL ConvDicList_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

extern void * SAL_CALL GrammarCheckingIterator_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);


// definition of the two functions that are used to provide the services

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet =
        LngSvcMgr_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet = LinguProps_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  DicList_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  ConvDicList_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  GrammarCheckingIterator_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );
    return pRet;
}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
