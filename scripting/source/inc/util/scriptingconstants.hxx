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
#ifndef _COM_SUN_STAR_SCRIPTING_UTIL_SCRIPTINGCONSTANTS_HXX_
#define _COM_SUN_STAR_SCRIPTING_UTIL_SCRIPTINGCONSTANTS_HXX_

namespace scripting_constants
{

class ScriptingConstantsPool
{
public:
    const ::rtl::OUString DOC_REF;
    const ::rtl::OUString DOC_STORAGE_ID;
    const ::rtl::OUString DOC_URI;
    const ::rtl::OUString RESOLVED_STORAGE_ID;
    const ::rtl::OUString SCRIPT_INFO;
    const ::rtl::OUString SCRIPTSTORAGEMANAGER_SERVICE;
    const sal_Int32 SHARED_STORAGE_ID;
    const sal_Int32 USER_STORAGE_ID;
    const sal_Int32 DOC_STORAGE_ID_NOT_SET;

    static ScriptingConstantsPool& instance()
    {
        static ScriptingConstantsPool *pPool = 0;
        if( ! pPool )
        {
            ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
            if( ! pPool )
            {
                static ScriptingConstantsPool pool;
                pPool = &pool;
            }
        }
        return *pPool;
    }
private:
    ScriptingConstantsPool( const ScriptingConstantsPool & );
    ScriptingConstantsPool& operator = ( const ScriptingConstantsPool & );
    ScriptingConstantsPool()
        : DOC_REF( RTL_CONSTASCII_USTRINGPARAM( "SCRIPTING_DOC_REF" ) ),
        DOC_STORAGE_ID( RTL_CONSTASCII_USTRINGPARAM(
            "SCRIPTING_DOC_STORAGE_ID" ) ),
        DOC_URI( RTL_CONSTASCII_USTRINGPARAM( "SCRIPTING_DOC_URI" ) ),
        RESOLVED_STORAGE_ID( RTL_CONSTASCII_USTRINGPARAM(
            "SCRIPTING_RESOLVED_STORAGE_ID" ) ),
        SCRIPT_INFO( RTL_CONSTASCII_USTRINGPARAM( "SCRIPT_INFO" ) ),
        SCRIPTSTORAGEMANAGER_SERVICE( RTL_CONSTASCII_USTRINGPARAM(
            "/singletons/com.sun.star.script.framework.storage.theScriptStorageManager" ) ),
        SHARED_STORAGE_ID( 0 ), USER_STORAGE_ID( 1 ),
        DOC_STORAGE_ID_NOT_SET( -1 )
    {}
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
