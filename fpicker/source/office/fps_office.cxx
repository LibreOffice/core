/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fps_office.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_fpicker.hxx"
#include "sal/types.h"

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include "cppuhelper/implementationentry.hxx"
#endif

#include "OfficeFilePicker.hxx"
#include "OfficeFolderPicker.hxx"

static cppu::ImplementationEntry g_entries[] =
{
    {
        SvtFilePicker::impl_createInstance,
        SvtFilePicker::impl_getStaticImplementationName,
        SvtFilePicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    {
        SvtFolderPicker::impl_createInstance,
        SvtFolderPicker::impl_getStaticImplementationName,
        SvtFolderPicker::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_writeInfoHelper (
        pServiceManager, pRegistryKey, g_entries);
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper (
        pImplementationName, pServiceManager, pRegistryKey, g_entries);
}

} // extern "C"
