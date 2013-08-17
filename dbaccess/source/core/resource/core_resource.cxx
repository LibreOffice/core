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

#include "core_resource.hxx"

#include <tools/resmgr.hxx>

// ---- needed as long as we have no contexts for components ---
#include <vcl/svapp.hxx>
#include <rtl/instance.hxx>
#include <svl/solar.hrc>

namespace dbaccess
{

    // ResourceManager
    namespace
    {
        // access safety
        struct theResourceManagerMutex : public rtl::Static< osl::Mutex, theResourceManagerMutex > {};
    }

    sal_Int32       ResourceManager::s_nClients = 0;
    ResMgr*         ResourceManager::m_pImpl = NULL;

    void ResourceManager::ensureImplExists()
    {
        if (m_pImpl)
            return;

        m_pImpl = ResMgr::CreateResMgr("dba", Application::GetSettings().GetUILanguageTag());
    }

    OUString ResourceManager::loadString(sal_uInt16 _nResId)
    {
        OUString sReturn;

        ensureImplExists();
        if (m_pImpl)
            sReturn = OUString(ResId(_nResId,*m_pImpl));

        return sReturn;
    }

    OUString ResourceManager::loadString( sal_uInt16 _nResId, const sal_Char* _pPlaceholderAscii, const OUString& _rReplace )
    {
        OUString sString( loadString( _nResId ) );
        return sString.replaceFirst( OUString::createFromAscii(_pPlaceholderAscii), _rReplace );
    }

    OUString ResourceManager::loadString( sal_uInt16 _nResId, const sal_Char* _pPlaceholderAscii1, const OUString& _rReplace1,
        const sal_Char* _pPlaceholderAscii2, const OUString& _rReplace2 )
    {
        OUString sString( loadString( _nResId ) );
        sString = sString.replaceFirst( OUString::createFromAscii(_pPlaceholderAscii1), _rReplace1 );
        sString = sString.replaceFirst( OUString::createFromAscii(_pPlaceholderAscii2), _rReplace2 );
        return sString;
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
