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



#ifndef _DBAUI_MODULE_DBU_HXX_
#define _DBAUI_MODULE_DBU_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif

class ResMgr;

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OModule
//=========================================================================
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

//=========================================================================
//= OModuleClient
//=========================================================================
/** base class for objects which uses any global module-specific ressources
*/
class OModuleClient
{
public:
    OModuleClient()     { OModule::registerClient(); }
    ~OModuleClient()    { OModule::revokeClient(); }
};

//=========================================================================
//= ModuleRes
//=========================================================================
/** specialized ResId, using the ressource manager provided by the global module
*/
class ModuleRes : public ::ResId
{
public:
    ModuleRes(sal_uInt16 _nId) : ResId(_nId, *OModule::getResManager()) { }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_MODULE_DBU_HXX_

