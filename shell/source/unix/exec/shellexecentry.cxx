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

#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include "shellexec.hxx"

//-----------------------------------------------------------------------
// namespace directives
//-----------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using com::sun::star::system::XSystemShellExecute;

//-----------------------------------------------------------------------
// defines
//-----------------------------------------------------------------------

#define SHELLEXEC_SERVICE_NAME  "com.sun.star.system.SystemShellExecute"
#define SHELLEXEC_IMPL_NAME     "com.sun.star.comp.system.SystemShellExecute"

//-----------------------------------------------------------------------

namespace
{
    Reference< XInterface > SAL_CALL createInstance(const Reference< XComponentContext >& xContext)
    {
        return Reference< XInterface >( static_cast< XSystemShellExecute* >( new ShellExec(xContext) ) );
    }
}

extern "C"
{
//----------------------------------------------------------------------
// component_getFactory
//----------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(
    const sal_Char* pImplName,
    SAL_UNUSED_PARAMETER uno_Interface* /*pSrvManager*/,
    SAL_UNUSED_PARAMETER uno_Interface* /*pRegistryKey*/ )
{
    Reference< XSingleComponentFactory > xFactory;

    if (0 == ::rtl_str_compare( pImplName, SHELLEXEC_IMPL_NAME ))
    {
        OUString serviceName( RTL_CONSTASCII_USTRINGPARAM(SHELLEXEC_SERVICE_NAME) );

        xFactory = ::cppu::createSingleComponentFactory(
            createInstance,
            OUString( RTL_CONSTASCII_USTRINGPARAM(SHELLEXEC_IMPL_NAME) ),
            Sequence< OUString >( &serviceName, 1 ) );

    }

    if (xFactory.is())
    xFactory->acquire();

    return xFactory.get();
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
