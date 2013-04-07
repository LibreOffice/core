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

#include <cppuhelper/factory.hxx>
#include <clipboardmanager.hxx>
#include <generic_clipboard.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::uno;
using namespace cppu;


extern "C"
{

//==================================================================================================

SAL_DLLPUBLIC_EXPORT void * SAL_CALL dtrans_component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * /*pRegistryKey*/
)
{
    void * pRet = 0;

    if (pServiceManager)
    {
        Reference< XSingleServiceFactory > xFactory;

        if (rtl_str_compare( pImplName, CLIPBOARDMANAGER_IMPLEMENTATION_NAME ) == 0)
        {
            xFactory = createOneInstanceFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                ClipboardManager_createInstance,
                ClipboardManager_getSupportedServiceNames() );
        }
        else if (rtl_str_compare( pImplName, GENERIC_CLIPBOARD_IMPLEMENTATION_NAME ) == 0)
        {
            xFactory = createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                GenericClipboard_createInstance,
                GenericClipboard_getSupportedServiceNames() );
        }

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
