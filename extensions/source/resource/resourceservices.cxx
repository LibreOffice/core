/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <ResourceIndexAccess.hxx>
#include <oooresourceloader.hxx>
#include <comphelper/servicedecl.hxx>
#include <uno/environment.h>

namespace sdecl = ::comphelper::service_decl;

sdecl::class_< ::extensions::resource::ResourceIndexAccess, sdecl::with_args<true> > ResourceIndexAccessServiceImpl;
sdecl::class_< ::extensions::resource::OpenOfficeResourceLoader> OpenOfficeResourceLoaderServiceImpl;

const sdecl::ServiceDecl ResourceIndexAccessDecl(
    ResourceIndexAccessServiceImpl,
    "org.libreoffice.extensions.resource.ResourceIndexAccess",
    "org.libreoffice.resource.ResourceIndexAccess");

//TODO: this is a singleton, not a service:
const sdecl::ServiceDecl OpenOfficeResourceLoaderDecl(
    OpenOfficeResourceLoaderServiceImpl,
    "com.sun.star.comp.resource.OpenOfficeResourceLoader",
    "com.sun.star.resource.OfficeResourceLoader");

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL res_component_getFactory(
    sal_Char const* pImplName, void*, void*)
{
    return sdecl::component_getFactoryHelper( pImplName,
            {&ResourceIndexAccessDecl, &OpenOfficeResourceLoaderDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
