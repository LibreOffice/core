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

#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#define _RPTUI_MODULE_HELPER_RPT_HXX_

#include "dllapi.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <tools/resmgr.hxx>

//.........................................................................
namespace rptui
{
//.........................................................................
    //=========================================================================
    //= OModule
    //=========================================================================
    class OModuleImpl;
    class REPORTDESIGN_DLLPUBLIC OModule
    {
        friend class OModuleClient;

    private:
        OModule();
            // not implemented. OModule is a static class

    protected:
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

    //=========================================================================
    //= OModuleClient
    //=========================================================================
    /** base class for objects which uses any global module-specific resources
    */
    class REPORTDESIGN_DLLPUBLIC OModuleClient
    {
    public:
        inline OModuleClient()      { OModule::registerClient(); }
        inline ~OModuleClient() { OModule::revokeClient(); }
    };

    //=========================================================================
    //= ModuleRes
    //=========================================================================
    /** specialized ResId, using the resource manager provided by the global module
    */
    class REPORTDESIGN_DLLPUBLIC ModuleRes : public ::ResId
    {
    public:
        inline ModuleRes(sal_uInt16 _nId) : ResId(_nId, *OModule::getResManager()) { }
    };
//.........................................................................
}   // namespace rptui
//.........................................................................

#endif // _RPTUI_MODULE_HELPER_RPT_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
