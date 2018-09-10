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
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include "FPServiceInfo.hxx"

#include "VistaFilePicker.hxx"
#include "WinImplHelper.hxx"
#include <stdio.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using ::com::sun::star::ui::dialogs::XFilePicker2;
using ::com::sun::star::ui::dialogs::XFolderPicker2;

static Reference< XInterface > createInstance(
    const Reference< XMultiServiceFactory >& rServiceManager )
{
    return Reference<ui::dialogs::XFilePicker2>(
            new ::fpicker::win32::vista::VistaFilePicker(rServiceManager, false));
}

static Reference< XInterface >
createInstance_fop( const Reference< XMultiServiceFactory >& rServiceManager )
{
    return Reference<ui::dialogs::XFolderPicker2>(
            new ::fpicker::win32::vista::VistaFilePicker(rServiceManager, true));
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* fps_win32_component_getFactory(
    const sal_Char* pImplName, void* pSrvManager, void* )
{
    void* pRet = nullptr;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, FILE_PICKER_IMPL_NAME ) ) )
    {
        Sequence<OUString> aSNS { FILE_PICKER_SERVICE_NAME };

        Reference< XSingleServiceFactory > xFactory ( createSingleFactory(
            static_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createInstance,
            aSNS ) );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, FOLDER_PICKER_IMPL_NAME ) ) )
    {
        Sequence<OUString> aSNS { FOLDER_PICKER_SERVICE_NAME };

        Reference< XSingleServiceFactory > xFactory ( createSingleFactory(
            static_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createInstance_fop,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
