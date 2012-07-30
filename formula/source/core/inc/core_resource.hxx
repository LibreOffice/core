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

#ifndef _FORMULA_CORE_RESOURCE_HXX_
#define _FORMULA_CORE_RESOURCE_HXX_

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

class ResMgr;
//.........................................................................
namespace formula
{
    //==================================================================
    //= ResourceManager
    //= handling resources within the FORMULA-Core library
    //==================================================================
    class ResourceManager
    {
        friend class OModuleClient;
        static sal_Int32    s_nClients;     /// number of registered clients
        static ResMgr*  m_pImpl;

    private:
        // no instantiation allowed
        ResourceManager() { }
        ~ResourceManager() { }

    protected:
        static void ensureImplExists();
        /// register a client for the module
        static void registerClient();
        /// revoke a client for the module
        static void revokeClient();

    public:

        static ResMgr*  getResManager();
    };

    //=========================================================================
    //= OModuleClient
    //=========================================================================
    /** base class for objects which uses any global module-specific resources
    */
    class OModuleClient
    {
    public:
        OModuleClient()     { ResourceManager::registerClient(); }
        ~OModuleClient()    { ResourceManager::revokeClient(); }
    };


//.........................................................................
} // formula
//.........................................................................

#endif // _FORMULA_CORE_RESOURCE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
