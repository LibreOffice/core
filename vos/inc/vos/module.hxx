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




#ifndef _VOS_MODULE_HXX_
#define _VOS_MODULE_HXX_

#   include <vos/object.hxx>
#   include <rtl/ustring.hxx>
#   include <osl/module.h>

namespace vos
{

class OModule : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OModule, vos));

public:

    /// default c'tor
    OModule();

    /// this c'tor is a combination of the default c'tor and load()
    OModule(const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);
    virtual ~OModule();

    /// loads the specified module
    sal_Bool SAL_CALL load(const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);

    /// unloads the currently loaded module
    void SAL_CALL unload();

    /// returns sal_True, if a module is loaded, sal_False otherwise
    sal_Bool SAL_CALL isLoaded();

#if 0
    // not implemented yet
    // returns the name of the currently loaded module or an empty string if none.
    ::rtl::OUString SAL_CALL getModuleName();
#endif

    /// returns a pointer to the specified Symbol if found, NULL otherwise
    void* SAL_CALL getSymbol(const ::rtl::OUString& strSymbolName);

protected:

    oslModule m_Module;
};

}

#endif


