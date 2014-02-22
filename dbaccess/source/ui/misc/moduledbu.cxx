/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "moduledbu.hxx"

#include <tools/resmgr.hxx>
#include <svl/solar.hrc>
#include <tools/debug.hxx>

#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()

namespace dbaui
{


/** implementation for <type>OModule</type>. not threadsafe, has to be guarded by it's owner
*/
class OModuleImpl
{
    ResMgr* m_pResources;

public:
    
    OModuleImpl();
    ~OModuleImpl();

    
    ResMgr* getResManager();
};

OModuleImpl::OModuleImpl()
    :m_pResources(NULL)
{

}

OModuleImpl::~OModuleImpl()
{
    if (m_pResources)
        delete m_pResources;

}

ResMgr* OModuleImpl::getResManager()
{
    

    if (!m_pResources)
    {
        
        m_pResources = ResMgr::CreateResMgr("dbu");
    }
    return m_pResources;
}


::osl::Mutex    OModule::s_aMutex;
sal_Int32       OModule::s_nClients = 0;
OModuleImpl*    OModule::s_pImpl = NULL;
ResMgr* OModule::getResManager()
{
    ENTER_MOD_METHOD();
    return s_pImpl->getResManager();
}

void OModule::registerClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ++s_nClients;
}

void OModule::revokeClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    if (!--s_nClients && s_pImpl)
    {
        delete s_pImpl;
        s_pImpl = NULL;
    }
}

void OModule::ensureImpl()
{
    if (s_pImpl)
        return;
    s_pImpl = new OModuleImpl();
}

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
