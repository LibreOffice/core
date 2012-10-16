/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
#define __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <properties.h>
#include <stdtypes.h>
#include <uielement/menubarmanager.hxx>
#include <uiconfiguration/windowstateconfiguration.hxx>
#include <framework/addonsoptions.hxx>
#include <uielement/uielement.hxx>
#include <helper/ilayoutnotifications.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/XTopWindow2.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#include <com/sun/star/frame/XMenuBarMergingAcceptor.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase7.hxx>
#include <cppuhelper/interfacecontainer.hxx>
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
    typedef ::cppu::WeakImplHelper7 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::frame::XLayoutManager
                                    ,   ::com::sun::star::awt::XWindowListener
                                    ,   ::com::sun::star::frame::XFrameActionListener
                                    ,   ::com::sun::star::ui::XUIConfigurationListener
                                    ,   ::com::sun::star::frame::XMenuBarMergingAcceptor
                                    ,   ::com::sun::star::frame::XLayoutManagerEventBroadcaster
                                    >   LayoutManager_Base;
    typedef ::comphelper::OPropertyContainer    LayoutManager_PBase;
    class LayoutManager : public  LayoutManager_Base                    ,
                          // base classes
                          // Order is neccessary for right initialization!
                          private ThreadHelpBase                        ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                          public  ::cppu::OBroadcastHelper              ,
                          public  ILayoutNotifications                  ,
                          public  LayoutManager_PBase
    {
        public:
            enum { DOCKINGAREAS_COUNT = 4 };

            LayoutManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMGR );
            virtual ~LayoutManager();

            /** declaration of XInterface, XTypeProvider, XServiceInfo */
            FWK_DECLARE_XINTERFACE
            FWK_DECLARE_XTYPEPROVIDER
            DECLARE_XSERVICEINFO

            //---------------------------------------------------------------------------------------------------------
            // XLayoutManager
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL reset() throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Rectangle SAL_CALL getCurrentDockingArea(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor > SAL_CALL getDockingAreaAcceptor() throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setDockingAreaAcceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL createElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL destroyElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL requestElement( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL getElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > > SAL_CALL getElements(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL showElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hideElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL dockWindow( const ::rtl::OUString& aName, ::com::sun::star::ui::DockingArea DockingArea, const ::com::sun::star::awt::Point& Pos ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL dockAllWindows( ::sal_Int16 nElementType ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL floatWindow( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL lockWindow( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL unlockWindow( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementSize( const ::rtl::OUString& aName, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementPos( const ::rtl::OUString& aName, const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementPosSize( const ::rtl::OUString& aName, const ::com::sun::star::awt::Point& aPos, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementVisible( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementFloating( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementDocked( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL isElementLocked( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Size SAL_CALL getElementSize( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Point SAL_CALL getElementPos( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL lock(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL unlock(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL doLayout(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isVisible() throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            // XMenuBarMergingAcceptor
            //---------------------------------------------------------------------------------------------------------
            virtual sal_Bool SAL_CALL setMergedMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMergedMenuBar )
                                                       throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeMergedMenuBar(  ) throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            //  XWindowListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException );
            virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException );
            virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );
            virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //   XFrameActionListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) throw ( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //  XEventListener
            //---------------------------------------------------------------------------------------------------------
            using cppu::OPropertySetHelper::disposing;
            virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //  XUIConfigurationListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            //  XLayoutManagerEventBroadcaster
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL addLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& aLayoutManagerListener ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& aLayoutManagerListener ) throw (::com::sun::star::uno::RuntimeException);

            DECL_LINK(MenuBarClose, void *);
            DECL_LINK( WindowEventListener, VclSimpleEvent* );

            //---------------------------------------------------------------------------------------------------------
            //  ILayoutNotifications
            //---------------------------------------------------------------------------------------------------------
            virtual void requestLayout( Hint eHint );

        protected:
            DECL_LINK(AsyncLayoutHdl, void *);

        private:
            //---------------------------------------------------------------------------------------------------------
            //  helper
            //---------------------------------------------------------------------------------------------------------

            //---------------------------------------------------------------------------------------------------------
            //  menu bar
            //---------------------------------------------------------------------------------------------------------
            void impl_clearUpMenuBar();
            void implts_reset( sal_Bool bAttach );
            void implts_updateMenuBarClose();
            sal_Bool implts_resetMenuBar();

            //---------------------------------------------------------------------------------------------------------
            //  locking
            //---------------------------------------------------------------------------------------------------------
            void implts_lock();
            sal_Bool implts_unlock();

            //---------------------------------------------------------------------------------------------------------
            //  query
            //---------------------------------------------------------------------------------------------------------
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_findElement( const rtl::OUString& aName );

            void implts_writeNewStateData( const rtl::OUString aName, const ::com::sun::star::uno::Reference< com::sun::star::awt::XWindow >& xWindow );
            sal_Bool implts_readWindowStateData( const rtl::OUString& rName, UIElement& rElementData );
            void implts_writeWindowStateData( const rtl::OUString& rName, const UIElement& rElementData );
            void implts_setElementData( UIElement& rUIElement, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindow >& rDockWindow );
            void implts_sortUIElements();
            void implts_destroyElements();
            void implts_toggleFloatingUIElementsVisibility( sal_Bool bActive );
            void implts_reparentChildWindows();
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_createDockingWindow( const ::rtl::OUString& aElementName );

            sal_Bool implts_isEmbeddedLayoutManager() const;
            sal_Int16 implts_getCurrentSymbolsSize();
            sal_Int16 implts_getCurrentSymbolsStyle();
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_createElement( const rtl::OUString& aName );

            // layouting methods
            sal_Bool implts_resizeContainerWindow( const ::com::sun::star::awt::Size& rContainerSize, const ::com::sun::star::awt::Point& rComponentPos );
            ::Size  implts_getTopBottomDockingAreaSizes();
            ::Size  implts_getContainerWindowOutputSize();

            void implts_setDockingAreaWindowSizes( const css::awt::Rectangle& rBorderSpace );
            ::com::sun::star::awt::Rectangle implts_calcDockingAreaSizes();
            sal_Bool implts_doLayout( sal_Bool bForceRequestBorderSpace, sal_Bool bOuterResize );
            void implts_doLayout_notify( sal_Bool bOuterResize );

            // internal methods to control status/progress bar
            ::Size      implts_getStatusBarSize();
            void        implts_destroyStatusBar();
            void        implts_createStatusBar( const rtl::OUString& rStatusBarName );
            void        implts_createProgressBar();
            void        implts_destroyProgressBar();
            void        implts_setStatusBarPosSize( const ::Point& rPos, const ::Size& rSize );
            sal_Bool    implts_showStatusBar( sal_Bool bStoreState=sal_False );
            sal_Bool    implts_hideStatusBar( sal_Bool bStoreState=sal_False );
            void        implts_readStatusBarState( const rtl::OUString& rStatusBarName );
            sal_Bool    implts_showProgressBar();
            sal_Bool    implts_hideProgressBar();
            void        implts_backupProgressBarWrapper();
            void        implts_setOffset( const sal_Int32 nBottomOffset );

            void    implts_setInplaceMenuBar(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMergedMenuBar )
                            throw (::com::sun::star::uno::RuntimeException);
            void    implts_resetInplaceMenuBar()
                            throw (::com::sun::star::uno::RuntimeException);

            void    implts_setVisibleState( sal_Bool bShow );
            void    implts_updateUIElementsVisibleState( sal_Bool bShow );
            void    implts_setCurrentUIVisibility( sal_Bool bShow );
            void    implts_notifyListeners( short nEvent, ::com::sun::star::uno::Any aInfoParam );

            DECL_LINK( OptionsChanged, void* );
            DECL_LINK( SettingsChanged, void* );

            //---------------------------------------------------------------------------------------------------------
            //  OPropertySetHelper
            //---------------------------------------------------------------------------------------------------------
            virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                                com::sun::star::uno::Any&        aOldValue       ,
                                                                                                                sal_Int32                        nHandle         ,
                                                                                                                const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException );
            virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                                const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception                 );
            using cppu::OPropertySetHelper::getFastPropertyValue;
            virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                    sal_Int32                    nHandle         ) const;
            virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper();
            virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);

            css::uno::Reference< css::lang::XMultiServiceFactory >                      m_xSMGR; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >                           m_xURLTransformer;
            css::uno::Reference< css::container::XIndexAccess >                         m_xDisplayAccess;
            css::uno::Reference< css::frame::XFrame >                                   m_xFrame;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >        m_xModuleCfgMgr;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >        m_xDocCfgMgr;
            css::uno::WeakReference< css::frame::XModel >                               m_xModel;
            css::uno::Reference< css::awt::XWindow >                                    m_xContainerWindow;
            css::uno::Reference< css::awt::XTopWindow2 >                                m_xContainerTopWindow;
            sal_Int32                                                                   m_nLockCount;
            bool                                                                        m_bActive;
            bool                                                                        m_bInplaceMenuSet;
            bool                                                                        m_bDockingInProgress;
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
            bool                                                                        m_bMenuBarCloser;
            css::awt::Rectangle                                                         m_aDockingArea;
            css::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor >           m_xDockingAreaAcceptor;
            css::uno::Reference< ::com::sun::star::lang::XComponent >                   m_xInplaceMenuBar;
            MenuBarManager*                                                             m_pInplaceMenuBar;
            css::uno::Reference< ::com::sun::star::ui::XUIElement >                     m_xMenuBar;
            UIElement                                                                   m_aStatusBarElement;
            UIElement                                                                   m_aProgressBarElement;
            com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >          m_xProgressBarBackup;
            css::uno::Reference< ::com::sun::star::frame::XModuleManager2 >             m_xModuleManager;
            css::uno::Reference< ::com::sun::star::ui::XUIElementFactory >              m_xUIElementFactoryManager;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowState;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowStateSupplier;
            GlobalSettings*                                                             m_pGlobalSettings;
            rtl::OUString                                                               m_aModuleIdentifier;
            rtl::OUString                                                               m_aStatusBarAlias;
            rtl::OUString                                                               m_aProgressBarAlias;
            rtl::OUString                                                               m_aPropDocked;
            rtl::OUString                                                               m_aPropVisible;
            rtl::OUString                                                               m_aPropDockingArea;
            rtl::OUString                                                               m_aPropDockPos;
            rtl::OUString                                                               m_aPropPos;
            rtl::OUString                                                               m_aPropSize;
            rtl::OUString                                                               m_aPropUIName;
            rtl::OUString                                                               m_aPropStyle;
            rtl::OUString                                                               m_aPropLocked;
            rtl::OUString                                                               m_aCustomizeCmd;
            sal_Int16                                                                   m_eSymbolsSize;
            sal_Int16                                                                   m_eSymbolsStyle;
        Timer                                                                       m_aAsyncLayoutTimer;
            ::cppu::OMultiTypeInterfaceContainerHelper                                  m_aListenerContainer; // container for ALL Listener
            ToolbarLayoutManager*                                                       m_pToolbarManager;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >       m_xToolbarManager;

        friend class detail::InfoHelperBuilder;
    };

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
