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


#include "macbackend.hxx"
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace uno = com::sun::star::uno;

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL createMacOSXBackend(
    const uno::Reference<uno::XComponentContext>&)
{
    return * MacOSXBackend::createInstance();
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createMacOSXBackend,
        MacOSXBackend::getBackendName,
        MacOSXBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { 0, 0, 0, 0, 0, 0 }
};

//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL macbe1_component_getFactory( const sal_Char *aImplementationName, void *aServiceManager, void *aRegistryKey)
{

    return cppu::component_getFactoryHelper(
        aImplementationName,
        aServiceManager,
        aRegistryKey,
        kImplementations_entries);
}

//------------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
