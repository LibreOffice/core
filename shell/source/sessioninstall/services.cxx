/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SyncDbusSessionHelper.hxx>
#include <comphelper/servicedecl.hxx>
#include <uno/environment.h>

namespace sdecl = ::comphelper::service_decl;

sdecl::class_< ::shell::sessioninstall::SyncDbusSessionHelper> SyncDbusSessionHelperServiceImpl;

const sdecl::ServiceDecl SyncDbusSessionHelperServiceDecl(
    SyncDbusSessionHelperServiceImpl,
    "org.libreoffice.comp.shell.sessioninstall.SyncDbusSessionHelper",
    "org.freedesktop.PackageKit.SyncDbusSessionHelper");

extern "C"
SAL_DLLPUBLIC_EXPORT void* SAL_CALL losessioninstall_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&SyncDbusSessionHelperServiceDecl} );
}

extern "C"
SAL_DLLPUBLIC_EXPORT void* SAL_CALL sessioninstall_component_getFactory( sal_Char const* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return losessioninstall_component_getFactory(pImplName, pServiceManager, pRegistryKey);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
