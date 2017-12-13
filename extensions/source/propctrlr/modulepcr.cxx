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

#include "modulepcr.hxx"

#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

namespace pcr
{
    struct CreateModuleClass
    {
        PcrModule* operator()()
        {
            static PcrModule* pModule = new PcrModule;
            return pModule;
            /*  yes, in theory, this is a resource leak, since the PcrModule
                will never be cleaned up. However, using a non-heap instance of PcrModule
                would not work: It would be cleaned up when the module is unloaded.
                This might happen (and is likely to happen) *after* the tools-library
                has been unloaded. However, the module's dtor is where we would delete
                our resource manager (in case not all our clients de-registered) - which
                would call into the already-unloaded tools-library. */
        }
    };

    PcrModule::PcrModule()
    {
    }

    PcrModule& PcrModule::getInstance()
    {
        return *rtl_Instance< PcrModule, CreateModuleClass, ::osl::MutexGuard, ::osl::GetGlobalMutex >::
            create( CreateModuleClass(), ::osl::GetGlobalMutex() );
    }

    OUString PcrRes(const char* pId)
    {
        return Translate::get(pId, Translate::Create("pcr"));
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
