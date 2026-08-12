/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <uielement/menubarmanager.hxx>
#include <uielement/uielement.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XLayoutManager2.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XUIElementFactoryManager.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/XTopWindow2.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <vcl/timer.hxx>

namespace framework
{
    class MenuBarWrapper;
    class ProgressBarWrapper;
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
    class LayoutManager final : public  LayoutManager_Base                    ,
                          private cppu::BaseMutex,
                          public  ::cppu::OBroadcastHelper              ,
                          public  LayoutManager_PBase
    {
        public:
            LayoutManager( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~LayoutManager() override;

            /** declaration of XInterface, XTypeProvider, XServiceInfo */
            DECLARE_XINTERFACE()
            DECLARE_XTYPEPROVIDER()
            virtual OUString getImplementationName() override
            {
                return u"com.sun.star.comp.framework.LayoutManager"_ustr;
            }

            virtual bool supportsService(OUString const & ServiceName) override
            {
                return cppu::supportsService(this, ServiceName);
            }

            virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override
            {
                cpo::uno::Sequence< OUString > aSeq { u"com.sun.star.frame.LayoutManager"_ustr };
                return aSeq;
            }

            // XLayoutManager
            virtual void attachFrame( const css::uno::Reference< css::frame::XFrame >& Frame ) override;
            virtual void reset() override;
            virtual css::awt::Rectangle getCurrentDockingArea(  ) override;
            virtual css::uno::Reference< css::ui::XDockingAreaAcceptor > getDockingAreaAcceptor() override;
            virtual void setDockingAreaAcceptor( const css::uno::Reference< css::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor ) final override;
            virtual void createElement( const OUString& aName ) override;
            virtual void destroyElement( const OUString& aName ) override;
            virtual bool requestElement( const OUString& ResourceURL ) override;
            virtual css::uno::Reference< css::ui::XUIElement > getElement( const OUString& aName ) override;
            virtual cpo::uno::Sequence< css::uno::Reference< css::ui::XUIElement > > getElements(  ) override;
            virtual bool showElement( const OUString& aName ) override;
            virtual bool hideElement( const OUString& aName ) override;
            virtual bool dockWindow( const OUString& aName, css::ui::DockingArea DockingArea, const css::awt::Point& Pos ) override;
            virtual bool dockAllWindows( ::sal_Int16 nElementType ) override;
            virtual bool floatWindow( const OUString& aName ) override;
            virtual bool lockWindow( const OUString& ResourceURL ) override;
            virtual bool unlockWindow( const OUString& ResourceURL ) override;
            virtual void setElementSize( const OUString& aName, const css::awt::Size& aSize ) override;
            virtual void setElementPos( const OUString& aName, const css::awt::Point& aPos ) override;
            virtual void setElementPosSize( const OUString& aName, const css::awt::Point& aPos, const css::awt::Size& aSize ) override;
            virtual bool isElementVisible( const OUString& aName ) override;
            virtual bool isElementFloating( const OUString& aName ) override;
            virtual bool isElementDocked( const OUString& aName ) override;
            virtual bool isElementLocked( const OUString& ResourceURL ) override;
            virtual css::awt::Size getElementSize( const OUString& aName ) override;
            virtual css::awt::Point getElementPos( const OUString& aName ) override;
            virtual void lock(  ) override;
            virtual void unlock(  ) override;
            virtual void doLayout(  ) override;
            virtual void setVisible( bool bVisible ) override;
            virtual bool isVisible() override;

            // XMenuBarMergingAcceptor

            virtual bool setMergedMenuBar( const css::uno::Reference< css::container::XIndexAccess >& xMergedMenuBar ) override;
            virtual void removeMergedMenuBar(  ) override;

            //  XWindowListener
            virtual void windowResized( const css::awt::WindowEvent& aEvent ) override;
            virtual void windowMoved( const css::awt::WindowEvent& aEvent ) override;
            virtual void windowShown( const css::lang::EventObject& aEvent ) override;
            virtual void windowHidden( const css::lang::EventObject& aEvent ) override;

            //   XFrameActionListener
            virtual void frameAction( const css::frame::FrameActionEvent& aEvent ) override;

            //  XEventListener
            using cppu::OPropertySetHelper::disposing;
            virtual void disposing( const css::lang::EventObject& aEvent ) override;

            //  XUIConfigurationListener
            virtual void elementInserted( const css::ui::ConfigurationEvent& Event ) override;
            virtual void elementRemoved( const css::ui::ConfigurationEvent& Event ) override;
            virtual void elementReplaced( const css::ui::ConfigurationEvent& Event ) override;

            //  XLayoutManagerEventBroadcaster
            virtual void addLayoutManagerEventListener( const css::uno::Reference< css::frame::XLayoutManagerListener >& aLayoutManagerListener ) override;
            virtual void removeLayoutManagerEventListener( const css::uno::Reference< css::frame::XLayoutManagerListener >& aLayoutManagerListener ) override;

            DECL_LINK( MenuBarClose, void *, void);
            DECL_LINK( WindowEventListener, VclWindowEvent&, void );

            //  called from ToolbarLayoutManager
            void requestLayout();

            /// Reading of settings - shared with ToolbarLayoutManager.
            static bool readWindowStateData( const OUString& rName, UIElement& rElementData,
                    const css::uno::Reference< css::container::XNameAccess > &rPersistentWindowState,
                    std::unique_ptr<GlobalSettings> &rGlobalSettings, bool &bInGlobalSettings,
                    const css::uno::Reference< css::uno::XComponentContext > &rComponentContext );

        private:
            DECL_LINK(AsyncLayoutHdl, Timer *, void);

            //  menu bar

            void implts_createMenuBar( const OUString& rMenuBarName );
            void impl_clearUpMenuBar();
            void implts_reset( bool bAttach );
            void implts_updateMenuBarClose();
            bool implts_resetMenuBar();
            void implts_createMSCompatibleMenuBar(const OUString& rName);

            //  locking

            void implts_lock();
            bool implts_unlock();

            //  query

            css::uno::Reference< css::ui::XUIElement > implts_findElement( std::u16string_view aName );

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

            void implts_setDockingAreaWindowSizes();
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
            void    implts_notifyListeners(short nEvent, const cpo::uno::Any& rInfoParam);

            //  OPropertySetHelper

            virtual void                                                setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                                const cpo::uno::Any&  aValue          ) override;
            virtual ::cppu::IPropertyArrayHelper&                       getInfoHelper() override;
            virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;

            css::uno::Reference< css::uno::XComponentContext >             m_xContext; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >              m_xURLTransformer;
            css::uno::Reference< css::frame::XFrame >                      m_xFrame;
            css::uno::Reference< css::ui::XUIConfigurationManager >        m_xModuleCfgMgr;
            css::uno::Reference< css::ui::XUIConfigurationManager >        m_xDocCfgMgr;
            css::uno::Reference< css::awt::XWindow >                       m_xContainerWindow;
            css::uno::Reference< css::awt::XTopWindow2 >                   m_xContainerTopWindow;
            sal_Int32                                                      m_nLockCount;
            bool                                                           m_bInplaceMenuSet;
            bool                                                           m_bMenuVisible;
            bool                                                           m_bVisible;
            bool                                                           m_bParentWindowVisible;
            bool                                                           m_bMustDoLayout;
            bool                                                           m_bAutomaticToolbars;
            bool                                                           m_bHideCurrentUI;
            bool                                                           m_bGlobalSettings;
            bool                                                           m_bPreserveContentSize;
            bool                                                           m_bMenuBarCloseButton;
            css::awt::Rectangle                                            m_aDockingArea;
            css::uno::Reference< css::ui::XDockingAreaAcceptor >           m_xDockingAreaAcceptor;
            rtl::Reference< MenuBarManager >                               m_xInplaceMenuBar;
            rtl::Reference< MenuBarWrapper >                               m_xMenuBar;
            UIElement                                                      m_aStatusBarElement;
            UIElement                                                      m_aProgressBarElement;
            rtl::Reference< ProgressBarWrapper >                           m_xProgressBarBackup;
            css::uno::Reference< css::frame::XModuleManager2 >             m_xModuleManager;
            css::uno::Reference< css::ui::XUIElementFactoryManager >       m_xUIElementFactoryManager;
            css::uno::Reference< css::container::XNameAccess >             m_xPersistentWindowState;
            css::uno::Reference< css::container::XNameAccess >             m_xPersistentWindowStateSupplier;
            std::unique_ptr<GlobalSettings>                                m_pGlobalSettings;
            OUString                                                       m_aModuleIdentifier;
            Timer                                                          m_aAsyncLayoutTimer;
            comphelper::OMultiTypeInterfaceContainerHelper2                m_aListenerContainer; // container for ALL Listener
            rtl::Reference< ToolbarLayoutManager >                         m_xToolbarManager;
            bool                                                           m_bInSetCurrentUIVisibility;

        friend class detail::InfoHelperBuilder;
    };

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
