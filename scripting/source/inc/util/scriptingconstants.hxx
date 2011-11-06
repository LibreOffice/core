/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
