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
    typedef ::cppu::WeakImplHelper <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::frame::XLayoutManager2
                                    ,   ::com::sun::star::awt::XWindowListener
                                    >   LayoutManager_Base;
    typedef ::comphelper::OPropertyContainer    LayoutManager_PBase;
    class LayoutManager : public  LayoutManager_Base                    ,
                          private cppu::BaseMutex,
                          public  ::cppu::OBroadcastHelper              ,
                          public  LayoutManager_PBase
    {
        public:
            enum Hint
            {
                HINT_NOT_SPECIFIED,
                HINT_TOOLBARSPACE_HAS_CHANGED,
                HINT_COUNT
            };
            enum { DOCKINGAREAS_COUNT = 4 };

            LayoutManager( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );
            virtual ~LayoutManager();

            /** declaration of XInterface, XTypeProvider, XServiceInfo */
            FWK_DECLARE_XINTERFACE
            FWK_DECLARE_XTYPEPROVIDER
            virtual OUString SAL_CALL getImplementationName()
                throw (css::uno::RuntimeException, std::exception) override
            {
                return OUString("com.sun.star.comp.framework.LayoutManager");
            }

            virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
                throw (css::uno::RuntimeException, std::exception) override
            {
                return cppu::supportsService(this, ServiceName);
            }

            virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
                throw (css::uno::RuntimeException, std::exception) override
            {
                css::uno::Sequence< OUString > aSeq(1);
                aSeq[0] = "com.sun.star.frame.LayoutManager";
                return aSeq;
            }

            // XLayoutManager
            virtual void SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL reset() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::awt::Rectangle SAL_CALL getCurrentDockingArea(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor > SAL_CALL getDockingAreaAcceptor() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setDockingAreaAcceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL createElement( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL destroyElement( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL requestElement( const OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL getElement( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > > SAL_CALL getElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL showElement( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hideElement( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL dockWindow( const OUString& aName, ::com::sun::star::ui::DockingArea DockingArea, const ::com::sun::star::awt::Point& Pos ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL dockAllWindows( ::sal_Int16 nElementType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL floatWindow( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL lockWindow( const OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL unlockWindow( const OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setElementSize( const OUString& aName, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setElementPos( const OUString& aName, const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setElementPosSize( const OUString& aName, const ::com::sun::star::awt::Point& aPos, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isElementVisible( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isElementFloating( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isElementDocked( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isElementLocked( const OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::awt::Size SAL_CALL getElementSize( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::awt::Point SAL_CALL getElementPos( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL lock(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL unlock(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL doLayout(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isVisible() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XMenuBarMergingAcceptor

            virtual sal_Bool SAL_CALL setMergedMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMergedMenuBar )
                                                       throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeMergedMenuBar(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            //  XWindowListener
            virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception ) override;
            virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception ) override;
            virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception ) override;
            virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception ) override;

            //   XFrameActionListener
            virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

            //  XEventListener
            using cppu::OPropertySetHelper::disposing;
            virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception ) override;

            //  XUIConfigurationListener
            virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            //  XLayoutManagerEventBroadcaster
            virtual void SAL_CALL addLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& aLayoutManagerListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& aLayoutManagerListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            DECL_LINK_TYPED( MenuBarClose, void *, void);
            DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );

            //  called from ToolbarLayoutManager
            void requestLayout( Hint eHint = HINT_NOT_SPECIFIED );

            /// Reading of settings - shared with ToolbarLayoutManager.
            static bool readWindowStateData( const OUString& rName, UIElement& rElementData,
                    const css::uno::Reference< css::container::XNameAccess > &rPersistentWindowState,
                    GlobalSettings* &rGlobalSettings, bool &bInGlobalSettings,
                    const css::uno::Reference< css::uno::XComponentContext > &rComponentContext );

        protected:
            DECL_LINK_TYPED(AsyncLayoutHdl, Timer *, void);

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

            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_findElement( const OUString& aName );

            bool implts_readWindowStateData( const OUString& rName, UIElement& rElementData );
            void implts_writeWindowStateData( const OUString& rName, const UIElement& rElementData );
            void implts_destroyElements();
            void implts_toggleFloatingUIElementsVisibility( bool bActive );
            void implts_reparentChildWindows();
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_createDockingWindow( const OUString& aElementName );

            bool implts_isEmbeddedLayoutManager() const;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_createElement( const OUString& aName );

            // layouting methods
            bool implts_resizeContainerWindow( const ::com::sun::star::awt::Size& rContainerSize, const ::com::sun::star::awt::Point& rComponentPos );
            ::Size  implts_getContainerWindowOutputSize();

            void implts_setDockingAreaWindowSizes( const css::awt::Rectangle& rBorderSpace );
            ::com::sun::star::awt::Rectangle implts_calcDockingAreaSizes();
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

            void    implts_setInplaceMenuBar(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMergedMenuBar )
                            throw (::com::sun::star::uno::RuntimeException, std::exception);
            void    implts_resetInplaceMenuBar()
                            throw (::com::sun::star::uno::RuntimeException);

            void    implts_setVisibleState( bool bShow );
            void    implts_updateUIElementsVisibleState( bool bShow );
            void    implts_setCurrentUIVisibility( bool bShow );
            void    implts_notifyListeners(short nEvent, const css::uno::Any& rInfoParam);

            //  OPropertySetHelper

            virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                                com::sun::star::uno::Any&        aOldValue       ,
                                                                                                                sal_Int32                        nHandle         ,
                                                                                                                const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException ) override;
            virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                                const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception, std::exception                 ) override;
            using cppu::OPropertySetHelper::getFastPropertyValue;
            virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                    sal_Int32                    nHandle         ) const override;
            virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper() override;
            virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            css::uno::Reference< css::uno::XComponentContext >                          m_xContext; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >                           m_xURLTransformer;
            css::uno::Reference< css::frame::XFrame >                                   m_xFrame;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >        m_xModuleCfgMgr;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >        m_xDocCfgMgr;
            css::uno::WeakReference< css::frame::XModel >                               m_xModel;
            css::uno::Reference< css::awt::XWindow >                                    m_xContainerWindow;
            css::uno::Reference< css::awt::XTopWindow2 >                                m_xContainerTopWindow;
            sal_Int32                                                                   m_nLockCount;
            bool                                                                        m_bActive;
            bool                                                                        m_bInplaceMenuSet;
            bool                                                                        m_bMenuVisible;
            bool                                                                        m_bComponentAttached;
            bool                                                                        m_bDoLayout;
            bool                                                                        m_bVisible;
            bool                                                                        m_bParentWindowVisible;
            bool                                                                        m_bMustDoLayout;
            bool                                                                        m_bAutomaticToolbars;
            bool                                                                        m_bStoreWindowState;
            bool                                                                        m_bHideCurrentUI;
            bool                                                                        m_bGlobalSettings;
            bool                                                                        m_bPreserveContentSize;
            bool                                                                        m_bMenuBarCloseButton;
            css::awt::Rectangle                                                         m_aDockingArea;
            css::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor >           m_xDockingAreaAcceptor;
            css::uno::Reference< ::com::sun::star::lang::XComponent >                   m_xInplaceMenuBar;
            MenuBarManager*                                                             m_pInplaceMenuBar;
            css::uno::Reference< ::com::sun::star::ui::XUIElement >                     m_xMenuBar;
            UIElement                                                                   m_aStatusBarElement;
            UIElement                                                                   m_aProgressBarElement;
            com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >          m_xProgressBarBackup;
            css::uno::Reference< ::com::sun::star::frame::XModuleManager2 >             m_xModuleManager;
            css::uno::Reference< ::com::sun::star::ui::XUIElementFactoryManager >       m_xUIElementFactoryManager;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowState;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowStateSupplier;
            GlobalSettings*                                                             m_pGlobalSettings;
            OUString                                                                    m_aModuleIdentifier;
            Timer                                                                       m_aAsyncLayoutTimer;
            ::cppu::OMultiTypeInterfaceContainerHelper                                  m_aListenerContainer; // container for ALL Listener
            ToolbarLayoutManager*                                                       m_pToolbarManager;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >       m_xToolbarManager;

        friend class detail::InfoHelperBuilder;
    };

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_LAYOUTMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
