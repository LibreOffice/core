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

#ifndef _DBA_CORE_RESOURCE_HXX_
#define _DBA_CORE_RESOURCE_HXX_

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <sal/macros.h>

class ResMgr;

namespace dbaccess
{

#define DBA_RES( id )                       ResourceManager::loadString( id )
#define DBA_RES_PARAM( id, ascii, replace ) ResourceManager::loadString( id, ascii, replace )

#define DBACORE_RESSTRING( id ) DBA_RES( id )
        // (compatibility)

    //==================================================================
    //= ResourceManager
    //= handling ressources within the DBA-Core library
    //==================================================================
    class ResourceManager
    {
        friend class OModuleClient;
        static ::osl::Mutex s_aMutex;       /// access safety
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
        /** loads the string with the specified resource id
        */
        static ::rtl::OUString  loadString(sal_uInt16 _nResId);

        /** loads a string from the resource file, substituting a placeholder with a given string

            @param  _nResId
                the resource ID of the string to load
            @param  _pPlaceholderAscii
                the ASCII representation of the placeholder string
            @param  _rReplace
                the string which should substitute the placeholder
        */
        static ::rtl::OUString  loadString(
                sal_uInt16              _nResId,
                const sal_Char*         _pPlaceholderAscii,
                const ::rtl::OUString&  _rReplace
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
        static ::rtl::OUString  loadString(
                sal_uInt16              _nResId,
                const sal_Char*         _pPlaceholderAscii1,
                const ::rtl::OUString&  _rReplace1,
                const sal_Char*         _pPlaceholderAscii2,
                const ::rtl::OUString&  _rReplace2
        );
    };

    //=========================================================================
    //= OModuleClient
    //=========================================================================
    /** base class for objects which uses any global module-specific ressources
    */
    class OModuleClient
    {
    public:
        OModuleClient()     { ResourceManager::registerClient(); }
        ~OModuleClient()    { ResourceManager::revokeClient(); }
    };
}

#endif // _DBA_CORE_RESOURCE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
