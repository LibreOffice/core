/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "mdrivermanager.hxx"

#include <cppuhelper/factory.hxx>

#include <stdio.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//==========================================================================
//= registration
//==========================================================================
extern "C"
{

//---------------------------------------------------------------------------------------
SAL_DLLPUBLIC_EXPORT void* SAL_CALL sdbc2_component_getFactory(const sal_Char* _pImplName, ::com::sun::star::lang::XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    void* pRet = NULL;

    if (::drivermanager::OSDBCDriverManager::getImplementationName_static().compareToAscii(_pImplName) == 0)
    {
        Reference< XSingleServiceFactory > xFactory(
            ::cppu::createOneInstanceFactory(
                _pServiceManager,
                ::drivermanager::OSDBCDriverManager::getImplementationName_static(),
                ::drivermanager::OSDBCDriverManager::Create,
                ::drivermanager::OSDBCDriverManager::getSupportedServiceNames_static()
            )
        );
        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   // extern "C"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
