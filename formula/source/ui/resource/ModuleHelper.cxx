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


#include "ModuleHelper.hxx"
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/thread.h>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/uri.hxx>
#include <tools/debug.hxx>
#ifndef _SOLAR_HRC
#include <svl/solar.hrc>
#endif

#define EXPAND_PROTOCOL     "vnd.sun.star.expand:"
#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()

//.........................................................................
namespace formula
{
//.........................................................................
    using namespace ::com::sun::star;
//=========================================================================
//= OModuleImpl
//=========================================================================
/** implementation for <type>OModule</type>. not threadsafe, has to be guarded by it's owner
*/
class OModuleImpl
{
    ResMgr* m_pRessources;

public:
    /// ctor
    OModuleImpl();
    ~OModuleImpl();

    /// get the manager for the ressources of the module
    ResMgr* getResManager();
};

DBG_NAME( rpt_OModuleImpl )
//-------------------------------------------------------------------------
OModuleImpl::OModuleImpl()
    :m_pRessources(NULL)
{
    DBG_CTOR( rpt_OModuleImpl,NULL);

}

//-------------------------------------------------------------------------
OModuleImpl::~OModuleImpl()
{
    if (m_pRessources)
        delete m_pRessources;

    DBG_DTOR( rpt_OModuleImpl,NULL);
}

//-------------------------------------------------------------------------
ResMgr* OModuleImpl::getResManager()
{
    // note that this method is not threadsafe, which counts for the whole class !

    if (!m_pRessources)
    {
        // create a manager with a fixed prefix
        m_pRessources = ResMgr::CreateResMgr( "forui");
    }
    return m_pRessources;
}

//=========================================================================
//= OModule
//=========================================================================
::osl::Mutex    OModule::s_aMutex;
sal_Int32       OModule::s_nClients = 0;
OModuleImpl*    OModule::s_pImpl = NULL;
//-------------------------------------------------------------------------
ResMgr* OModule::getResManager()
{
    ENTER_MOD_METHOD();
    return s_pImpl->getResManager();
}

//-------------------------------------------------------------------------
void OModule::registerClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ++s_nClients;
}

//-------------------------------------------------------------------------
void OModule::revokeClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    if (!--s_nClients && s_pImpl)
    {
        delete s_pImpl;
        s_pImpl = NULL;
    }
}

//-------------------------------------------------------------------------
void OModule::ensureImpl()
{
    if (s_pImpl)
        return;
    s_pImpl = new OModuleImpl();
}

//.........................................................................
}   // namespace formula
//.........................................................................

