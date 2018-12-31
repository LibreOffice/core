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

#include <config_features.h>

#include <sal/config.h>

#include <toolkit/helper/accessiblefactory.hxx>
#include <osl/module.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <tools/svlibrary.h>

#include <helper/accessibilityclient.hxx>

namespace toolkit
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::accessibility;

    namespace
    {
#ifndef DISABLE_DYNLOADING
        static oslModule                                s_hAccessibleImplementationModule = nullptr;
#endif
#if HAVE_FEATURE_DESKTOP
        static GetStandardAccComponentFactory           s_pAccessibleFactoryFunc = nullptr;
#endif
        static ::rtl::Reference< IAccessibleFactory >   s_pFactory;
    }


    //= AccessibleDummyFactory

    class AccessibleDummyFactory:
        public IAccessibleFactory
    {
    public:
        AccessibleDummyFactory();
        AccessibleDummyFactory(const AccessibleDummyFactory&) = delete;
        AccessibleDummyFactory& operator=(const AccessibleDummyFactory&) = delete;

    protected:
        virtual ~AccessibleDummyFactory() override;

    public:
        // IAccessibleFactory
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXButton* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXCheckBox* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXRadioButton* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXListBox* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXFixedHyperlink* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXFixedText* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXScrollBar* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXEdit* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXComboBox* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXToolBox* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXHeaderBar* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleContext( VCLXWindow* /*_pXWindow*/ ) override
        {
            return nullptr;
        }
        css::uno::Reference< css::accessibility::XAccessible >
                createAccessible( Menu* /*_pMenu*/, bool /*_bIsMenuBar*/ ) override
        {
            return nullptr;
        }
    };


    AccessibleDummyFactory::AccessibleDummyFactory()
    {
    }


    AccessibleDummyFactory::~AccessibleDummyFactory()
    {
    }


    //= AccessibilityClient


    AccessibilityClient::AccessibilityClient()
        :m_bInitialized( false )
    {
    }

#if HAVE_FEATURE_DESKTOP
#ifndef DISABLE_DYNLOADING
    extern "C" { static void thisModule() {} }
#else
    extern "C" void *getStandardAccessibleFactory();
#endif
#endif // HAVE_FEATURE_DESKTOP

    void AccessibilityClient::ensureInitialized()
    {
        if ( m_bInitialized )
            return;

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#if HAVE_FEATURE_DESKTOP
        // load the library implementing the factory
        if (!s_pFactory)
        {
#ifndef DISABLE_DYNLOADING
            const OUString sModuleName( SVLIBRARY( "acc" ) );
            s_hAccessibleImplementationModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
            if ( s_hAccessibleImplementationModule != nullptr )
            {
                const OUString sFactoryCreationFunc =
                    OUString("getStandardAccessibleFactory");
                s_pAccessibleFactoryFunc = reinterpret_cast<GetStandardAccComponentFactory>(
                    osl_getFunctionSymbol( s_hAccessibleImplementationModule, sFactoryCreationFunc.pData ));

            }
            OSL_ENSURE( s_pAccessibleFactoryFunc, "AccessibilityClient::ensureInitialized: could not load the library, or not retrieve the needed symbol!" );
#else
            s_pAccessibleFactoryFunc = getStandardAccessibleFactory;
#endif // DISABLE_DYNLOADING

            // get a factory instance
            if ( s_pAccessibleFactoryFunc )
            {
                IAccessibleFactory* pFactory = static_cast< IAccessibleFactory* >( (*s_pAccessibleFactoryFunc)() );
                OSL_ENSURE( pFactory, "AccessibilityClient::ensureInitialized: no factory provided by the A11Y lib!" );
                if ( pFactory )
                {
                    s_pFactory = pFactory;
                    pFactory->release();
                }
            }
        }
#endif // HAVE_FEATURE_DESKTOP

        if (!s_pFactory)
            // the attempt to load the lib, or to create the factory, failed
            // -> fall back to a dummy factory
            s_pFactory = new AccessibleDummyFactory;

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
