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

#include <config_feature_desktop.h>
#include <config_wasm_strip.h>

#include <sal/config.h>

#include <toolkit/helper/accessiblefactory.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <tools/svlibrary.h>

#include <helper/accessibilityclient.hxx>
#include <com/sun/star/accessibility/GetStandardAccessibleFactoryService.hpp>

namespace toolkit
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::accessibility;

    namespace
    {
        ::rtl::Reference< IAccessibleFactory >   s_pFactory;
    }


    //= AccessibilityClient


    AccessibilityClient::AccessibilityClient()
        :m_bInitialized( false )
    {
    }

    void AccessibilityClient::ensureInitialized()
    {
        if ( m_bInitialized )
            return;

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
#if HAVE_FEATURE_DESKTOP
        if (!s_pFactory)
        {
            auto xService = css::accessibility::GetStandardAccessibleFactoryService::create(comphelper::getProcessComponentContext());
            assert(xService);
            // get a factory instance
            IAccessibleFactory* pFactory = reinterpret_cast<IAccessibleFactory*>(xService->getSomething({}));
            assert(pFactory);
            s_pFactory = pFactory;
            pFactory->release();
        }
#endif // HAVE_FEATURE_DESKTOP
#endif // ENABLE_WASM_STRIP_ACCESSIBILITY

        m_bInitialized = true;
    }

    IAccessibleFactory& AccessibilityClient::getFactory()
    {
        ensureInitialized();
        OSL_ENSURE( s_pFactory.is(), "AccessibilityClient::getFactory: at least a dummy factory should have been created!" );
        return *s_pFactory;
    }


}   // namespace toolkit

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
