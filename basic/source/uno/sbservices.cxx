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

#include "sbmodule.hxx"


namespace basic
{



    extern void createRegistryInfo_SfxDialogLibraryContainer();
    extern void createRegistryInfo_SfxScriptLibraryContainer();

    static void initializeModule()
    {
        static bool bInitialized( false );
        if ( !bInitialized )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !bInitialized )
            {
                createRegistryInfo_SfxDialogLibraryContainer();
                createRegistryInfo_SfxScriptLibraryContainer();
            }
        }
    }


} // namespace basic


extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL sb_component_getFactory(
    const sal_Char* pImplementationName, void*, void* )
{
    ::basic::initializeModule();
    return ::basic::BasicModule::getInstance().getComponentFactory( pImplementationName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
