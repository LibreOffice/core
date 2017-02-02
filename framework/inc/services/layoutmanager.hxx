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

#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_LAYOUTMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_SERVICES_LAYOUTMANAGER_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <properties.h>
#include <stdtypes.h>
#include <uielement/menubarmanager.hxx>
#include <framework/addonsoptions.hxx>
#include <uielement/uielement.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XLayoutManager2.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/ui/XUIElementFactoryManager.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/XTopWindow2.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#include <com/sun/star/frame/XMenuBarMergingAcceptor.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertycontainer.hxx>
#include <tools/wintypes.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/timer.hxx>

class MenuBar;
namespace framework
{

    class ToolbarLayoutManager;
    class GlobalSettings;
    namespace detail
    {
        class InfoHelperBuilder;
    }
    typedef ::cppu::WeakImplHelper <   css::lang::XServiceInfo
                                    ,   css::frame::XLayoutManager2
                                    ,   css::awt::XWindowListener
                                    >   LayoutManager_Base;
    typedef ::comphelper::OPropertyContainer    LayoutManager_PBase;
    class LayoutManager : public  LayoutManager_Base                    ,
                          private cppu::BaseMutex,
                          public  ::cppu::OBroadcastHelper              ,
                          public  LayoutManager_PBase
    {
        public:
            LayoutManager( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~LayoutManager() override;

            /** declaration of XInterface, XTypeProvider, XServiceInfo */
            FWK_DECLARE_XINTERFACE
            FWK_DECLARE_XTYPEPROVIDER
            virtual OUString SAL_CALL getImplementationName() override
            {
                return OUString("com.sun.star.comp.framework.LayoutManager");
            }

            virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
            {
                return cppu::supportsService(this, ServiceName);
            }

            virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
            {
                css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.LayoutManager" };
                return aSeq;
            }

            // XLayoutManager
            virtual void SAL_CALL attachFrame( const css::uno::Reference< css::frame::XFrame >& Frame ) override;
            virtual void SAL_CALL reset() override;
            virtual css::awt::Rectangle SAL_CALL getCurrentDockingArea(  ) override;
            virtual css::uno::Reference< css::ui::XDockingAreaAcceptor > SAL_CALL getDockingAreaAcceptor() override;
            virtual void SAL_CALL setDockingAreaAcceptor( const css::uno::Reference< css::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor ) final override;
            virtual void SAL_CALL createElement( const OUString& aName ) override;
            virtual void SAL_CALL destroyElement( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL requestElement( const OUString& ResourceURL ) override;
            virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL getElement( const OUString& aName ) override;
            virtual css::uno::Sequence< css::uno::Reference< css::ui::XUIElement > > SAL_CALL getElements(  ) override;
            virtual sal_Bool SAL_CALL showElement( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL hideElement( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL dockWindow( const OUString& aName, css::ui::DockingArea DockingArea, const css::awt::Point& Pos ) override;
            virtual sal_Bool SAL_CALL dockAllWindows( ::sal_Int16 nElementType ) override;
            virtual sal_Bool SAL_CALL floatWindow( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL lockWindow( const OUString& ResourceURL ) override;
            virtual sal_Bool SAL_CALL unlockWindow( const OUString& ResourceURL ) override;
            virtual void SAL_CALL setElementSize( const OUString& aName, const css::awt::Size& aSize ) override;
            virtual void SAL_CALL setElementPos( const OUString& aName, const css::awt::Point& aPos ) override;
            virtual void SAL_CALL setElementPosSize( const OUString& aName, const css::awt::Point& aPos, const css::awt::Size& aSize ) override;
            virtual sal_Bool SAL_CALL isElementVisible( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL isElementFloating( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL isElementDocked( const OUString& aName ) override;
            virtual sal_Bool SAL_CALL isElementLocked( const OUString& ResourceURL ) override;
            virtual css::awt::Size SAL_CALL getElementSize( const OUString& aName ) override;
            virtual css::awt::Point SAL_CALL getElementPos( const OUString& aName ) override;
            virtual void SAL_CALL lock(  ) override;
            virtual void SAL_CALL unlock(  ) override;
            virtual void SAL_CALL doLayout(  ) override;
            virtual void SAL_CALL setVisible( sal_Bool bVisible ) override;
            virtual sal_Bool SAL_CALL isVisible() override;

            // XMenuBarMergingAcceptor

            virtual sal_Bool SAL_CALL setMergedMenuBar( const css::uno::Reference< css::container::XIndexAccess >& xMergedMenuBar ) override;
            virtual void SAL_CALL removeMergedMenuBar(  ) override;

            //  XWindowListener
            virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) override;
            virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) override;
            virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) override;
            virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) override;

            //   XFrameActionListener
            virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) override;

            //  XEventListener
            using cppu::OPropertySetHelper::disposing;
            virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

            //  XUIConfigurationListener
            virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& Event ) override;
            virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& Event ) override;
            virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& Event ) override;

            //  XLayoutManagerEventBroadcaster
            virtual void SAL_CALL addLayoutManagerEventListener( const css::uno::Reference< css::frame::XLayoutManagerListener >& aLayoutManagerListener ) override;
            virtual void SAL_CALL removeLayoutManagerEventListener( const css::uno::Reference< css::frame::XLayoutManagerListener >& aLayoutManagerListener ) override;

            DECL_LINK( MenuBarClose, void *, void);
            DECL_LINK( WindowEventListener, VclWindowEvent&, void );

            //  called from ToolbarLayoutManager
            void requestLayout();

            /// Reading of settings - shared with ToolbarLayoutManager.
            static bool readWindowStateData( const OUString& rName, UIElement& rElementData,
                    const css::uno::Reference< css::container::XNameAccess > &rPersistentWindowState,
                    GlobalSettings* &rGlobalSettings, bool &bInGlobalSettings,
                    const css::uno::Reference< css::uno::XComponentContext > &rComponentContext );

        protected:
            DECL_LINK(AsyncLayoutHdl, Timer *, void);

        private:

            //  helper

            //  menu bar

            void impl_clearUpMenuBar();
            void implts_reset( bool bAttach );
            void implts_updateMenuBarClose();
            bool implts_resetMenuBar();

            //  locking

            void implts_lock();
            bool implts_unlock();

            //  query

            css::uno::Reference< css::ui::XUIElement > implts_findElement( const OUString& aName );

            bool implts_readWindowStateData( const OUString& rName, UIElement& rElementData );
            void implts_writeWindowStateData( const OUString& rName, const UIElement& rElementData );
            void implts_destroyElements();
            void implts_toggleFloatingUIElementsVisibility( bool bActive );
            void implts_reparentChildWindows();
            css::uno::Reference< css::ui::XUIElement > implts_createDockingWindow( const OUString& aElementName );

            bool implts_isEmbeddedLayoutManager() const;
            css::uno::Reference< css::ui::XUIElement > implts_createElement( const OUString& aName );

            // layouting methods
            bool implts_resizeContainerWindow( const css::awt::Size& rContainerSize, const css::awt::Point& rComponentPos );
            ::Size  implts_getContainerWindowOutputSize();

            void implts_setDockingAreaWindowSizes( const css::awt::Rectangle& rBorderSpace );
            css::awt::Rectangle implts_calcDockingAreaSizes();
            bool implts_doLayout( bool bForceRequestBorderSpace, bool bOuterResize );
            void implts_doLayout_notify( bool bOuterResize );

            // internal methods to control status/progress bar
            ::Size      implts_getStatusBarSize();
            void        implts_destroyStatusBar();
            void        implts_createStatusBar( const OUString& rStatusBarName );
            void        implts_createProgressBar();
            void        implts_destroyProgressBar();
            void        implts_setStatusBarPosSize( const ::Point& rPos, const ::Size& rSize );
            bool    implts_showStatusBar( bool bStoreState=false );
            bool    implts_hideStatusBar( bool bStoreState=false );
            void        implts_readStatusBarState( const OUString& rStatusBarName );
            bool    implts_showProgressBar();
            bool    implts_hideProgressBar();
            void        implts_backupProgressBarWrapper();
            void        implts_setOffset( const sal_Int32 nBottomOffset );

            /// @throws css::uno::RuntimeException
            void    implts_setInplaceMenuBar(
                        const css::uno::Reference< css::container::XIndexAccess >& xMergedMenuBar );
            /// @throws css::uno::RuntimeException
            void    implts_resetInplaceMenuBar();

            void    implts_setVisibleState( bool bShow );
            void    implts_updateUIElementsVisibleState( bool bShow );
            void    implts_setCurrentUIVisibility( bool bShow );
            void    implts_notifyListeners(short nEvent, const css::uno::Any& rInfoParam);

            //  OPropertySetHelper

            virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                                const css::uno::Any&  aValue          ) override;
            virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() override;
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;

            css::uno::Reference< css::uno::XComponentContext >             m_xContext; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >              m_xURLTransformer;
            css::uno::Reference< css::frame::XFrame >                      m_xFrame;
            css::uno::Reference< css::ui::XUIConfigurationManager >        m_xModuleCfgMgr;
            css::uno::Reference< css::ui::XUIConfigurationManager >        m_xDocCfgMgr;
            css::uno::WeakReference< css::frame::XModel >                  m_xModel;
            css::uno::Reference< css::awt::XWindow >                       m_xContainerWindow;
            css::uno::Reference< css::awt::XTopWindow2 >                   m_xContainerTopWindow;
            sal_Int32                                                      m_nLockCount;
            bool                                                           m_bActive;
            bool                                                           m_bInplaceMenuSet;
            bool                                                           m_bMenuVisible;
            bool                                                           m_bComponentAttached;
            bool                                                           m_bDoLayout;
            bool                                                           m_bVisible;
            bool                                                           m_bParentWindowVisible;
            bool                                                           m_bMustDoLayout;
            bool                                                           m_bAutomaticToolbars;
            bool                                                           m_bStoreWindowState;
            bool                                                           m_bHideCurrentUI;
            bool                                                           m_bGlobalSettings;
            bool                                                           m_bPreserveContentSize;
            bool                                                           m_bMenuBarCloseButton;
            css::awt::Rectangle                                            m_aDockingArea;
            css::uno::Reference< css::ui::XDockingAreaAcceptor >           m_xDockingAreaAcceptor;
            rtl::Reference< MenuBarManager >                               m_xInplaceMenuBar;
            css::uno::Reference< css::ui::XUIElement >                     m_xMenuBar;
            UIElement                                                      m_aStatusBarElement;
            UIElement                                                      m_aProgressBarElement;
            css::uno::Reference< css::ui::XUIElement >                     m_xProgressBarBackup;
            css::uno::Reference< css::frame::XModuleManager2 >             m_xModuleManager;
            css::uno::Reference< css::ui::XUIElementFactoryManager >       m_xUIElementFactoryManager;
            css::uno::Reference< css::container::XNameAccess >             m_xPersistentWindowState;
            css::uno::Reference< css::container::XNameAccess >             m_xPersistentWindowStateSupplier;
            GlobalSettings*                                                m_pGlobalSettings;
            OUString                                                       m_aModuleIdentifier;
            Timer                                                          m_aAsyncLayoutTimer;
            ::cppu::OMultiTypeInterfaceContainerHelper                     m_aListenerContainer; // container for ALL Listener
            rtl::Reference< ToolbarLayoutManager >                         m_xToolbarManager;

        friend class detail::InfoHelperBuilder;
    };

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_LAYOUTMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
