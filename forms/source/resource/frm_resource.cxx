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

#include "frm_resource.hxx"
#include <tools/simplerm.hxx>

// needed as long as we have no contexts for components
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svl/solar.hrc>


namespace frm
{


    //= ResourceManager

    SimpleResMgr* ResourceManager::m_pImpl = nullptr;


    ResourceManager::EnsureDelete::~EnsureDelete()
    {
        delete ResourceManager::m_pImpl;
    }


    void ResourceManager::ensureImplExists()
    {
        if (m_pImpl)
            return;

        m_pImpl = SimpleResMgr::Create("frm", Application::GetSettings().GetUILanguageTag());

        if (m_pImpl)
        {
            // no that we have a impl class make sure it's deleted on unloading the library
            static ResourceManager::EnsureDelete    s_aDeleteTheImplClas;
        }
    }


    OUString ResourceManager::loadString(sal_uInt16 _nResId)
    {
        OUString sReturn;

        ensureImplExists();
        if (m_pImpl)
            sReturn = m_pImpl->ReadString(_nResId);

        return sReturn;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
