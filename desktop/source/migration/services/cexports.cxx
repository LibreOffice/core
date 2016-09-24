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


#include <cppuhelper/implementationentry.hxx>
#include "basicmigration.hxx"
#include "wordbookmigration.hxx"


extern "C"
{

::cppu::ImplementationEntry oo2_entries [] =
{
    {
        migration::BasicMigration_create, migration::BasicMigration_getImplementationName,
        migration::BasicMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        nullptr, 0
    },
    {
        migration::WordbookMigration_create, migration::WordbookMigration_getImplementationName,
        migration::WordbookMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        nullptr, 0
    },
    // Extension migration was disabled by Oracle / OpenOffice.org
#if 0
    {
         migration::ExtensionMigration_create, migration::ExtensionMigration_getImplementationName,
         migration::ExtensionMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
         0, 0
    },
#endif
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};


SAL_DLLPUBLIC_EXPORT void * SAL_CALL migrationoo2_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, oo2_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
