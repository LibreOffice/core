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
#include "precompiled_desktop.hxx"

#include "cppuhelper/implementationentry.hxx"
#include "basicmigration.hxx"
#include "wordbookmigration.hxx"


extern "C"
{

::cppu::ImplementationEntry entries [] =
{
    {
        migration::BasicMigration_create, migration::BasicMigration_getImplementationName,
        migration::BasicMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        migration::WordbookMigration_create, migration::WordbookMigration_getImplementationName,
        migration::WordbookMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    // Extension migration was disabled by Oracle / OpenOffice.org
#if 0
    {
         migration::ExtensionMigration_create, migration::ExtensionMigration_getImplementationName,
         migration::ExtensionMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
         0, 0
    },
#endif
    { 0, 0, 0, 0, 0, 0 }
};


void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
