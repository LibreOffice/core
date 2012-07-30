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
#include "ModuleHelper.hxx"
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/thread.h>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/instance.hxx>
#include <rtl/uri.hxx>
#include <tools/debug.hxx>
#include <svl/solar.hrc>

#define EXPAND_PROTOCOL     "vnd.sun.star.expand:"
#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(theOModuleMutex::get()); \
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
    ResMgr* m_pResources;

public:
    /// ctor
    OModuleImpl();
    ~OModuleImpl();

    /// get the manager for the resources of the module
    ResMgr* getResManager();
};

DBG_NAME( rpt_OModuleImpl )
//-------------------------------------------------------------------------
OModuleImpl::OModuleImpl()
    :m_pResources(NULL)
{
    DBG_CTOR( rpt_OModuleImpl,NULL);

}

//-------------------------------------------------------------------------
OModuleImpl::~OModuleImpl()
{
    if (m_pResources)
        delete m_pResources;

    DBG_DTOR( rpt_OModuleImpl,NULL);
}

//-------------------------------------------------------------------------
ResMgr* OModuleImpl::getResManager()
{
    // note that this method is not threadsafe, which counts for the whole class !

    if (!m_pResources)
    {
        // create a manager with a fixed prefix
        m_pResources = ResMgr::CreateResMgr("forui");
    }
    return m_pResources;
}

//=========================================================================
//= OModule
//=========================================================================
namespace
{
    // access safety
    struct theOModuleMutex : public rtl::Static< osl::Mutex, theOModuleMutex > {};
}
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
    ::osl::MutexGuard aGuard(theOModuleMutex::get());
    ++s_nClients;
}

//-------------------------------------------------------------------------
void OModule::revokeClient()
{
    ::osl::MutexGuard aGuard(theOModuleMutex::get());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
