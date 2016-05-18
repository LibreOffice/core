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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_CORE_RESOURCE_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_CORE_RESOURCE_HXX

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

class ResMgr;

namespace dbaccess
{

#define DBA_RES( id )                       ResourceManager::loadString( id )
#define DBA_RES_PARAM( id, ascii, replace ) ResourceManager::loadString( id, ascii, replace )

#define DBACORE_RESSTRING( id ) DBA_RES( id )
        // (compatibility)

    // ResourceManager
    // handling resources within the DBA-Core library
    class ResourceManager
    {
        friend class OModuleClient;
        static sal_Int32    s_nClients;     /// number of registered clients
        static ResMgr*  m_pImpl;

    private:
        // no instantiation allowed
        ResourceManager() = delete;
        ~ResourceManager() { }

    protected:
        static void ensureImplExists();
        /// register a client for the module
        static void registerClient();
        /// revoke a client for the module
        static void revokeClient();

    public:
        /** loads the string with the specified resource id
        */
        static OUString  loadString(sal_uInt16 _nResId);

        /** loads a string from the resource file, substituting a placeholder with a given string

            @param  _nResId
                the resource ID of the string to load
            @param  _pPlaceholderAscii
                the ASCII representation of the placeholder string
            @param  _rReplace
                the string which should substitute the placeholder
        */
        static OUString  loadString(
                sal_uInt16              _nResId,
                const sal_Char*         _pPlaceholderAscii,
                const OUString&  _rReplace
        );

        /** loads a string from the resource file, substituting two placeholders with given strings

            @param  _nResId
                the resource ID of the string to load
            @param  _pPlaceholderAscii1
                the ASCII representation of the first placeholder string
            @param  _rReplace1
                the string which should substitute the first placeholder
            @param  _pPlaceholderAscii2
                the ASCII representation of the second placeholder string
            @param  _rReplace2
                the string which should substitute the second placeholder
        */
        static OUString  loadString(
                sal_uInt16              _nResId,
                const sal_Char*         _pPlaceholderAscii1,
                const OUString&  _rReplace1,
                const sal_Char*         _pPlaceholderAscii2,
                const OUString&  _rReplace2
        );
    };

    // OModuleClient
    /** base class for objects which uses any global module-specific resources
    */
    class OModuleClient
    {
    public:
        OModuleClient()     { ResourceManager::registerClient(); }
        ~OModuleClient()    { ResourceManager::revokeClient(); }
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_CORE_RESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
