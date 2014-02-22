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


#include "core_resource.hxx"

#include <tools/resmgr.hxx>


#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>
#include <svl/solar.hrc>


namespace formula
{

    
    
    
    namespace
    {
        
        struct theResourceManagerMutex : public rtl::Static< osl::Mutex, theResourceManagerMutex > {};
    }
    sal_Int32       ResourceManager::s_nClients = 0;
    ResMgr*         ResourceManager::m_pImpl = NULL;

    
    void ResourceManager::ensureImplExists()
    {
        if (m_pImpl)
            return;

        m_pImpl = ResMgr::CreateResMgr("for", Application::GetSettings().GetUILanguageTag());
    }

    
    void ResourceManager::registerClient()
    {
        ::osl::MutexGuard aGuard(theResourceManagerMutex::get());
        ++s_nClients;
    }

    
    void ResourceManager::revokeClient()
    {
        ::osl::MutexGuard aGuard(theResourceManagerMutex::get());
        if (!--s_nClients && m_pImpl)
        {
            delete m_pImpl;
            m_pImpl = NULL;
        }
    }
    ResMgr* ResourceManager::getResManager()
    {
        ensureImplExists();
        return m_pImpl;
    }


} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
