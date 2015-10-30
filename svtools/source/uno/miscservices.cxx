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

#include <sal/types.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "fpicker.hxx"

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::lang;

namespace
{
    static const struct ::cppu::ImplementationEntry s_aServiceEntries[] =
    {
        {
            // FilePicker should not use a constructor, it is only a
            // trampoline to a real impl.
            FilePicker_CreateInstance,
            FilePicker_getImplementationName,
            FilePicker_getSupportedServiceNames,
            ::cppu::createSingleComponentFactory, 0, 0
        },
        {
            // FolderPicker should not use a constructor, it is only a
            // trampoline to a real impl.
            FolderPicker_CreateInstance,
            FolderPicker_getImplementationName,
            FolderPicker_getSupportedServiceNames,
            ::cppu::createSingleComponentFactory, 0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL svt_component_getFactory(
    const sal_Char * pImplementationName, void * _pServiceManager, void * pRegistryKey)
{
    void * pResult = 0;
    if (_pServiceManager)
    {
        Reference< XMultiServiceFactory > xSMgr(static_cast< XMultiServiceFactory * >(_pServiceManager));

        pResult = cppu::component_getFactoryHelper(pImplementationName,
                _pServiceManager,
                pRegistryKey,
                s_aServiceEntries);
    }
    return pResult;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
