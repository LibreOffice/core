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
#include <tools/simplerm.hxx>

// ---- needed as long as we have no contexts for components ---
#include <vcl/svapp.hxx>
//---------------------------------------------------
#include <osl/thread.h>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/uri.hxx>

#include <svl/solar.hrc>
#include "ModuleHelper.hxx"
//.........................................................................
namespace reportdesign
{
    using namespace ::com::sun::star;
    //==================================================================
    //= ResourceManager
    //==================================================================
    SimpleResMgr* ResourceManager::m_pImpl = NULL;

    //------------------------------------------------------------------
    ResourceManager::EnsureDelete::~EnsureDelete()
    {
        delete ResourceManager::m_pImpl;
    }

    //------------------------------------------------------------------
    void ResourceManager::ensureImplExists(const uno::Reference< lang::XMultiComponentFactory >& /* _rM */)
    {
        if (!m_pImpl)
        {
            // now that we have an impl class make sure it's deleted on unloading the library
            static ResourceManager::EnsureDelete    s_aDeleteTheImplClass;

            m_pImpl = SimpleResMgr::Create("rpt", Application::GetSettings().GetUILanguageTag());
        }
    }

    //------------------------------------------------------------------
    OUString ResourceManager::loadString(sal_uInt16 _nResId,const uno::Reference< lang::XMultiComponentFactory >& _rM)
    {
        OUString sReturn;

        ensureImplExists(_rM);
        if (m_pImpl)
            sReturn = m_pImpl->ReadString(_nResId);

        return sReturn;
    }

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
