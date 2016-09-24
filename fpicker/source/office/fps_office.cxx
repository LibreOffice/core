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


#include "sal/types.h"

#include "cppuhelper/implementationentry.hxx"

#include "OfficeFilePicker.hxx"
#include "OfficeFolderPicker.hxx"

static const cppu::ImplementationEntry g_entries[] =
{
    {
        SvtRemoteFilePicker::impl_createInstance,
        SvtRemoteFilePicker::impl_getStaticImplementationName,
        SvtRemoteFilePicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        SvtFilePicker::impl_createInstance,
        SvtFilePicker::impl_getStaticImplementationName,
        SvtFilePicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        SvtFolderPicker::impl_createInstance,
        SvtFolderPicker::impl_getStaticImplementationName,
        SvtFolderPicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL fps_office_component_getFactory(
    const sal_Char * pImplementationName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper (
        pImplementationName, pServiceManager, pRegistryKey, g_entries);
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
