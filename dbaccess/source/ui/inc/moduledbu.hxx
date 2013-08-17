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

#ifndef _DBAUI_MODULE_DBU_HXX_
#define _DBAUI_MODULE_DBU_HXX_

#include <osl/mutex.hxx>
#include <tools/resid.hxx>

class ResMgr;

namespace dbaui
{

// OModule
class OModuleImpl;
class OModule
{
    friend class OModuleClient;

private:
    OModule();
        // not implemented. OModule is a static class

protected:
    static ::osl::Mutex s_aMutex;       /// access safety
    static sal_Int32    s_nClients;     /// number of registered clients
    static OModuleImpl* s_pImpl;        /// impl class. lives as long as at least one client for the module is registered

public:
    /// get the vcl res manager of the module
    static ResMgr*  getResManager();

protected:
    /// register a client for the module
    static void registerClient();
    /// revoke a client for the module
    static void revokeClient();

private:
    /** ensure that the impl class exists
        @precond m_aMutex is guarded when this method gets called
    */
    static void ensureImpl();
};

// OModuleClient
/** base class for objects which uses any global module-specific resources
*/
class OModuleClient
{
public:
    OModuleClient()     { OModule::registerClient(); }
    ~OModuleClient()    { OModule::revokeClient(); }
};

// ModuleRes
/** specialized ResId, using the resource manager provided by the global module
*/
class ModuleRes : public ::ResId
{
public:
    ModuleRes(sal_uInt16 _nId) : ResId(_nId, *OModule::getResManager()) { }
};

}   // namespace dbaui

#endif // _DBAUI_MODULE_DBU_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
