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
#include <config_options.h>

#include <vcl/svtaccessiblefactory.hxx>
#include <vcl/accessiblefactory.hxx>
#include <vcl/accessibletable.hxx>
#include <vcl/accessibletableprovider.hxx>

#include <tools/svlibrary.h>
#include <tools/debug.hxx>

#include <osl/module.h>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>

namespace vcl
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::accessibility;

    namespace
    {
#ifndef DISABLE_DYNLOADING
        oslModule                                s_hAccessibleImplementationModule = nullptr;
#endif
#if HAVE_FEATURE_DESKTOP
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        GetSvtAccessibilityComponentFactory      s_pAccessibleFactoryFunc = nullptr;
#endif
#endif
        ::rtl::Reference< IAccessibleFactory >   s_pFactory;
    }


    //= AccessibleFactoryAccess


    AccessibleFactoryAccess::AccessibleFactoryAccess()
        :m_bInitialized( false )
    {
    }

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
#if HAVE_FEATURE_DESKTOP
#ifndef DISABLE_DYNLOADING
    extern "C" { static void thisModule() {} }
#else
    extern "C" void* getSvtAccessibilityComponentFactory();
#endif
#endif // HAVE_FEATURE_DESKTOP
#endif // ENABLE_WASM_STRIP_ACCESSIBILITY

    void AccessibleFactoryAccess::ensureInitialized()
    {
        if ( m_bInitialized )
            return;

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
#if HAVE_FEATURE_DESKTOP
        // load the library implementing the factory
        if (!s_pFactory)
        {
#ifndef DISABLE_DYNLOADING
#if ENABLE_MERGELIBS_MORE
            s_hAccessibleImplementationModule = osl_loadModuleRelative( &thisModule, u"" SVLIBRARY( "merged" ) ""_ustr.pData, 0 );
#else
            s_hAccessibleImplementationModule = osl_loadModuleRelative( &thisModule, u"" SVLIBRARY( "acc" ) ""_ustr.pData, 0 );
#endif
            assert(s_hAccessibleImplementationModule);
            s_pAccessibleFactoryFunc = reinterpret_cast<GetSvtAccessibilityComponentFactory>(
                osl_getFunctionSymbol( s_hAccessibleImplementationModule, u"getSvtAccessibilityComponentFactory"_ustr.pData ));

            assert( s_pAccessibleFactoryFunc && "ac_registerClient: could not load the library, or not retrieve the needed symbol!" );
#else
            s_pAccessibleFactoryFunc = getSvtAccessibilityComponentFactory;
#endif // DISABLE_DYNLOADING

            // get a factory instance
            IAccessibleFactory* pFactory = static_cast< IAccessibleFactory* >( (*s_pAccessibleFactoryFunc)() );
            assert(pFactory);
            s_pFactory = pFactory;
            pFactory->release();
        }
#endif // HAVE_FEATURE_DESKTOP
#endif // ENABLE_WASM_STRIP_ACCESSIBILITY

        assert(s_pFactory);

        m_bInitialized = true;
    }

    IAccessibleFactory& AccessibleFactoryAccess::getFactory()
    {
        ensureInitialized();
        DBG_ASSERT( s_pFactory.is(), "AccessibleFactoryAccess::getFactory: at least a dummy factory should have been created!" );
        return *s_pFactory;
    }

}   // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
