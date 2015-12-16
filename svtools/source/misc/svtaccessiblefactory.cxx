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

#include "svtaccessiblefactory.hxx"

#include <boost/noncopyable.hpp>
#include <osl/module.h>

namespace svt
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::accessibility;

    namespace
    {
#ifndef DISABLE_DYNLOADING
        static oslModule                                s_hAccessibleImplementationModule = nullptr;
#endif
#if HAVE_FEATURE_DESKTOP
        static GetSvtAccessibilityComponentFactory      s_pAccessibleFactoryFunc = nullptr;
#endif
        static ::rtl::Reference< IAccessibleFactory >   s_pFactory;


        //= AccessibleDummyFactory

        class AccessibleDummyFactory:
            public IAccessibleFactory, private boost::noncopyable
        {
        public:
            AccessibleDummyFactory();

        protected:
            virtual ~AccessibleDummyFactory();

        public:
            // IAccessibleFactory
            virtual IAccessibleTabListBox*
                createAccessibleTabListBox(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*rxParent*/,
                    SvHeaderTabListBox& /*rBox*/
                ) const override
            {
                return nullptr;
            }

            virtual IAccessibleBrowseBox*
                createAccessibleBrowseBox(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/
                ) const override
            {
                return nullptr;
            }

            virtual table::IAccessibleTableControl*
                createAccessibleTableControl(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_rxParent*/,
                    table::IAccessibleTable& /*_rTable*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleIconChoiceCtrl(
                    SvtIconChoiceCtrl& /*_rIconCtrl*/,
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_xParent*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleTabBar(
                    TabBar& /*_rTabBar*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                createAccessibleTextWindowContext(
                    VCLXWindow* /*pVclXWindow*/, TextEngine& /*rEngine*/, TextView& /*rView*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleTreeListBox(
                    SvTreeListBox& /*_rListBox*/,
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_xParent*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleBrowseBoxHeaderBar(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*rxParent*/,
                    IAccessibleTableProvider& /*_rOwningTable*/,
                    AccessibleBrowseBoxObjType /*_eObjType*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleBrowseBoxTableCell(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    const css::uno::Reference< css::awt::XWindow >& /*_xFocusWindow*/,
                    sal_Int32 /*_nRowId*/,
                    sal_uInt16 /*_nColId*/,
                    sal_Int32 /*_nOffset*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleBrowseBoxHeaderCell(
                    sal_Int32 /*_nColumnRowId*/,
                    const css::uno::Reference< css::accessibility::XAccessible >& /*rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    const css::uno::Reference< css::awt::XWindow >& /*_xFocusWindow*/,
                    AccessibleBrowseBoxObjType  /*_eObjType*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createAccessibleCheckBoxCell(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    const css::uno::Reference< css::awt::XWindow >& /*_xFocusWindow*/,
                    sal_Int32 /*_nRowPos*/,
                    sal_uInt16 /*_nColPos*/,
                    const TriState& /*_eState*/,
                    bool /*_bIsTriState*/
                ) const override
            {
                return nullptr;
            }

            virtual css::uno::Reference< css::accessibility::XAccessible >
                createEditBrowseBoxTableCellAccess(
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_rxParent*/,
                    const css::uno::Reference< css::accessibility::XAccessible >& /*_rxControlAccessible*/,
                    const css::uno::Reference< css::awt::XWindow >& /*_rxFocusWindow*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    sal_Int32 /*_nRowPos*/,
                    sal_uInt16 /*_nColPos*/
                ) const override
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

    }


    //= AccessibleFactoryAccess


    AccessibleFactoryAccess::AccessibleFactoryAccess()
        :m_bInitialized( false )
    {
    }

#if HAVE_FEATURE_DESKTOP
#ifndef DISABLE_DYNLOADING
    extern "C" { static void SAL_CALL thisModule() {} }
#else
    extern "C" void* getSvtAccessibilityComponentFactory();
#endif
#endif // HAVE_FEATURE_DESKTOP

    void AccessibleFactoryAccess::ensureInitialized()
    {
        if ( m_bInitialized )
            return;

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#if HAVE_FEATURE_DESKTOP
        // load the library implementing the factory
        if ( !s_pFactory.get() )
        {
#ifndef DISABLE_DYNLOADING
            const OUString sModuleName( SVLIBRARY( "acc" ));
            s_hAccessibleImplementationModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
            if ( s_hAccessibleImplementationModule != nullptr )
            {
                const OUString sFactoryCreationFunc( "getSvtAccessibilityComponentFactory" );
                s_pAccessibleFactoryFunc = reinterpret_cast<GetSvtAccessibilityComponentFactory>(
                    osl_getFunctionSymbol( s_hAccessibleImplementationModule, sFactoryCreationFunc.pData ));

            }
            OSL_ENSURE( s_pAccessibleFactoryFunc, "ac_registerClient: could not load the library, or not retrieve the needed symbol!" );
#else
            s_pAccessibleFactoryFunc = getSvtAccessibilityComponentFactory;
#endif // DISABLE_DYNLOADING

            // get a factory instance
            if ( s_pAccessibleFactoryFunc )
            {
                IAccessibleFactory* pFactory = static_cast< IAccessibleFactory* >( (*s_pAccessibleFactoryFunc)() );
                if ( pFactory )
                {
                    s_pFactory = pFactory;
                    pFactory->release();
                }
            }
        }
#endif // HAVE_FEATURE_DESKTOP

        if ( !s_pFactory.get() )
            // the attempt to load the lib, or to create the factory, failed
            // -> fall back to a dummy factory
            s_pFactory = new AccessibleDummyFactory;

        m_bInitialized = true;
    }

    IAccessibleFactory& AccessibleFactoryAccess::getFactory()
    {
        ensureInitialized();
        DBG_ASSERT( s_pFactory.is(), "AccessibleFactoryAccess::getFactory: at least a dummy factory should have been created!" );
        return *s_pFactory;
    }


}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
