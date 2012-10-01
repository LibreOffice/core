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

#include "log_module.hxx"

//........................................................................
namespace logging
{
//........................................................................

    //--------------------------------------------------------------------
    extern void createRegistryInfo_LoggerPool();
    extern void createRegistryInfo_FileHandler();
    extern void createRegistryInfo_ConsoleHandler();
    extern void createRegistryInfo_PlainTextFormatter();
    extern void createRegistryInfo_CsvFormatter();

    static void initializeModule()
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        createRegistryInfo_LoggerPool();
        createRegistryInfo_FileHandler();
        createRegistryInfo_ConsoleHandler();
        createRegistryInfo_PlainTextFormatter();
        createRegistryInfo_CsvFormatter();
    }

//........................................................................
} // namespace logging
//........................................................................

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL log_component_getFactory(
    const sal_Char* pImplementationName, SAL_UNUSED_PARAMETER void*,
    SAL_UNUSED_PARAMETER void* )
{
    ::logging::initializeModule();
    return ::logging::LogModule::getInstance().getComponentFactory( pImplementationName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
