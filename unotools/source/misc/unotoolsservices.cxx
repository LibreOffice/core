/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/servicedecl.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <uno/environment.h>

namespace sdecl = ::comphelper::service_decl;

extern sdecl::ServiceDecl const OTempFileServiceDecl;
extern sdecl::ServiceDecl const ServiceDocumenterDecl;


extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL utl_component_getFactory(
    sal_Char const* pImplName, void*, void*)
{
    return sdecl::component_getFactoryHelper( pImplName,
            {&OTempFileServiceDecl, &ServiceDocumenterDecl});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
