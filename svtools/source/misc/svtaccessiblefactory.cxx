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

#include "svtaccessiblefactory.hxx"
#include <osl/module.h>

// #define UNLOAD_ON_LAST_CLIENT_DYING
    // this is not recommended currently. If enabled, the implementation will log
    // the number of active clients, and unload the acc library when the last client
    // goes away.
    // Sounds like a good idea, unfortunately, there's no guarantee that all objects
    // implemented in this library are already dead.
    // Iow, just because an object implementing an XAccessible (implemented in this lib
    // here) died, it's not said that everybody released all references to the
    // XAccessibleContext used by this component, and implemented in the acc lib.
    // So we cannot really unload the lib.
    //
    // Alternatively, if the lib would us own "usage counting", i.e. every component
    // implemented therein would affect a static ref count, the acc lib could care
    // for unloading itself.

//........................................................................
namespace svt
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::accessibility;

    namespace
    {
#ifdef UNLOAD_ON_LAST_CLIENT_DYING
        static oslInterlockedCount                      s_nAccessibleFactoryAccesss = 0;
#endif // UNLOAD_ON_LAST_CLIENT_DYING
#ifndef DISABLE_DYNLOADING
        static oslModule                                s_hAccessibleImplementationModule = NULL;
#endif
        static GetSvtAccessibilityComponentFactory      s_pAccessibleFactoryFunc = NULL;
        static ::rtl::Reference< IAccessibleFactory >   s_pFactory;

        //====================================================================
        //= AccessibleDummyFactory
        //====================================================================
        class AccessibleDummyFactory : public IAccessibleFactory
        {
        public:
            AccessibleDummyFactory();

        protected:
            virtual ~AccessibleDummyFactory();

        private:
            AccessibleDummyFactory( const AccessibleDummyFactory& );            // never implemented
            AccessibleDummyFactory& operator=( const AccessibleDummyFactory& ); // never implemented

            oslInterlockedCount m_refCount;

        public:
            // IReference
            virtual oslInterlockedCount SAL_CALL acquire();
            virtual oslInterlockedCount SAL_CALL release();

            // IAccessibleFactory
            virtual IAccessibleTabListBox*
                createAccessibleTabListBox(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*rxParent*/,
                    SvHeaderTabListBox& /*rBox*/
                ) const
            {
                return NULL;
            }

            virtual IAccessibleBrowseBox*
                createAccessibleBrowseBox(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*_rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/
                ) const
            {
                return NULL;
            }

            virtual table::IAccessibleTableControl*
                createAccessibleTableControl(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*_rxParent*/,
                    table::IAccessibleTable& /*_rTable*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleIconChoiceCtrl(
                    SvtIconChoiceCtrl& /*_rIconCtrl*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*_xParent*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleTabBar(
                    TabBar& /*_rTabBar*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleTextWindowContext(
                    VCLXWindow* /*pVclXWindow*/, TextEngine& /*rEngine*/, TextView& /*rView*/, bool /*bCompoundControlChild*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleTreeListBox(
                    SvTreeListBox& /*_rListBox*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*_xParent*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleBrowseBoxHeaderBar(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*rxParent*/,
                    IAccessibleTableProvider& /*_rOwningTable*/,
                    AccessibleBrowseBoxObjType /*_eObjType*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleBrowseBoxTableCell(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*_rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& /*_xFocusWindow*/,
                    sal_Int32 /*_nRowId*/,
                    sal_uInt16 /*_nColId*/,
                    sal_Int32 /*_nOffset*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleBrowseBoxHeaderCell(
                    sal_Int32 /*_nColumnRowId*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& /*_xFocusWindow*/,
                    AccessibleBrowseBoxObjType  /*_eObjType*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createAccessibleCheckBoxCell(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*_rxParent*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& /*_xFocusWindow*/,
                    sal_Int32 /*_nRowPos*/,
                    sal_uInt16 /*_nColPos*/,
                    const TriState& /*_eState*/,
                    sal_Bool /*_bIsTriState*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                createEditBrowseBoxTableCellAccess(
                    const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& /*_rxParent*/,
                    const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& /*_rxControlAccessible*/,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& /*_rxFocusWindow*/,
                    IAccessibleTableProvider& /*_rBrowseBox*/,
                    sal_Int32 /*_nRowPos*/,
                    sal_uInt16 /*_nColPos*/
                ) const
            {
                return NULL;
            }

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleToolPanelDeck(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*i_rAccessibleParent*/,
                    ::svt::ToolPanelDeck& /*i_rPanelDeck*/
                )
            {
                return NULL;
            }
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                createAccessibleToolPanelTabBar(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& /*i_rAccessibleParent*/,
                    ::svt::IToolPanelDeck& /*i_rPanelDeck*/,
                    ::svt::PanelTabBar& /*i_rTabBar*/
                )
            {
                return NULL;
            }
        };

        //----------------------------------------------------------------
        AccessibleDummyFactory::AccessibleDummyFactory()
        {
        }

        //----------------------------------------------------------------
        AccessibleDummyFactory::~AccessibleDummyFactory()
        {
        }

        //----------------------------------------------------------------
        oslInterlockedCount SAL_CALL AccessibleDummyFactory::acquire()
        {
            return osl_atomic_increment( &m_refCount );
        }

        //----------------------------------------------------------------
        oslInterlockedCount SAL_CALL AccessibleDummyFactory::release()
        {
            if ( 0 == osl_atomic_decrement( &m_refCount ) )
            {
                delete this;
                return 0;
            }
            return m_refCount;
        }
    }

    //====================================================================
    //= AccessibleFactoryAccess
    //====================================================================
    //--------------------------------------------------------------------
    AccessibleFactoryAccess::AccessibleFactoryAccess()
        :m_bInitialized( false )
    {
    }

    //--------------------------------------------------------------------
#ifndef DISABLE_DYNLOADING
    extern "C" { static void SAL_CALL thisModule() {} }
#else
    extern "C" void* getSvtAccessibilityComponentFactory();
#endif

    void AccessibleFactoryAccess::ensureInitialized()
    {
        if ( m_bInitialized )
            return;

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#ifdef UNLOAD_ON_LAST_CLIENT_DYING
        if ( 1 == osl_atomic_increment( &s_nAccessibleFactoryAccesss ) )
        {   // the first client
#endif // UNLOAD_ON_LAST_CLIENT_DYING
            // load the library implementing the factory
            if ( !s_pFactory.get() )
            {
#ifndef DISABLE_DYNLOADING
                const OUString sModuleName( SVLIBRARY( "acc" ));
                s_hAccessibleImplementationModule = osl_loadModuleRelative( &thisModule, sModuleName.pData, 0 );
                if ( s_hAccessibleImplementationModule != NULL )
                {
                    const OUString sFactoryCreationFunc( "getSvtAccessibilityComponentFactory" );
                    s_pAccessibleFactoryFunc = (GetSvtAccessibilityComponentFactory)
                        osl_getFunctionSymbol( s_hAccessibleImplementationModule, sFactoryCreationFunc.pData );

                }
                OSL_ENSURE( s_pAccessibleFactoryFunc, "ac_registerClient: could not load the library, or not retrieve the needed symbol!" );
#else
                s_pAccessibleFactoryFunc = getSvtAccessibilityComponentFactory;
#endif
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

            if ( !s_pFactory.get() )
                // the attempt to load the lib, or to create the factory, failed
                // -> fall back to a dummy factory
                s_pFactory = new AccessibleDummyFactory;
#ifdef UNLOAD_ON_LAST_CLIENT_DYING
        }
#endif

        m_bInitialized = true;
    }

    //--------------------------------------------------------------------
    AccessibleFactoryAccess::~AccessibleFactoryAccess()
    {
        if ( m_bInitialized )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

#ifdef UNLOAD_ON_LAST_CLIENT_DYING
            if( 0 == osl_atomic_decrement( &s_nAccessibleFactoryAccesss ) )
            {
                s_pFactory = NULL;
                s_pAccessibleFactoryFunc = NULL;
                if ( s_hAccessibleImplementationModule )
                {
                    osl_unloadModule( s_hAccessibleImplementationModule );
                    s_hAccessibleImplementationModule = NULL;
                }
            }
#endif // UNLOAD_ON_LAST_CLIENT_DYING
        }
    }

    //--------------------------------------------------------------------
    IAccessibleFactory& AccessibleFactoryAccess::getFactory()
    {
        ensureInitialized();
        DBG_ASSERT( s_pFactory.is(), "AccessibleFactoryAccess::getFactory: at least a dummy factory should have been created!" );
        return *s_pFactory;
    }

//........................................................................
}   // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
