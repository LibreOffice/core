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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARMANAGER_HXX

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <stdtypes.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <vcl/accel.hxx>
#include <vcl/timer.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <framework/addonsoptions.hxx>

namespace framework
{

struct PopupControllerEntry
{
    css::uno::WeakReference< css::frame::XDispatchProvider > m_xDispatchProvider;
};

typedef std::unordered_map< OUString, PopupControllerEntry, OUStringHash, std::equal_to< OUString > > PopupControllerCache;

class BmkMenu;
class AddonMenu;
class AddonPopupMenu;
class MenuBarManager : public css::frame::XStatusListener                ,
                       public css::frame::XFrameActionListener           ,
                       public css::ui::XUIConfigurationListener          ,
                       public css::lang::XComponent                      ,
                       public css::awt::XSystemDependentMenuPeer         ,
                       public ::cppu::OWeakObject
{
    protected:
        MenuBarManager(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::frame::XFrame >& rFrame,
            const css::uno::Reference< css::util::XURLTransformer >& _xURLTransformer,
            Menu*           pAddonMenu,
            bool            bDelete,
            bool            bDeleteChildren,
            bool popup);

    public:
        MenuBarManager(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::frame::XFrame >& rFrame,
            const css::uno::Reference< css::util::XURLTransformer >& _xURLTransformer,
            const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
            const OUString& aModuleIdentifier,
            Menu* pMenu,
            bool bDelete,
            bool bDeleteChildren,
            bool bHasMenuBar = true );

        virtual ~MenuBarManager();

        // XInterface
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw( css::uno::RuntimeException, std::exception ) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& Action ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

        // XSystemDependentMenuPeer
        virtual css::uno::Any SAL_CALL getMenuHandle( const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw (css::uno::RuntimeException, std::exception) override;

        DECL_LINK_TYPED( Select, Menu *, bool );

        Menu*   GetMenuBar() const { return m_pVCLMenu; }

        // Configuration methods
        static void FillMenuWithConfiguration( sal_uInt16& nId, Menu* pMenu,
                                               const OUString& rModuleIdentifier,
                                               const css::uno::Reference< css::container::XIndexAccess >& rItemContainer,
                                               const css::uno::Reference< css::util::XURLTransformer >& rTransformer );
        static void FillMenu( sal_uInt16& nId,
                              Menu* pMenu,
                              const OUString& rModuleIdentifier,
                              const css::uno::Reference< css::container::XIndexAccess >& rItemContainer,
                              const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider );

        void FillMenuManager( Menu* pMenu,
                              const css::uno::Reference< css::frame::XFrame >& rFrame,
                              const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
                              const OUString& rModuleIdentifier,
                              bool bDelete,
                              bool bDeleteChildren );
        void SetItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rItemContainer );
        void GetPopupController( PopupControllerCache& rPopupController );

    protected:
        DECL_LINK_TYPED(Highlight, Menu *, bool);
        DECL_LINK_TYPED( Activate, Menu *, bool );
        DECL_LINK_TYPED( Deactivate, Menu *, bool );
        DECL_LINK_TYPED( AsyncSettingsHdl, Timer *, void );

        void RemoveListener();
        void RequestImages();
        void RetrieveImageManagers();
        static bool MustBeHidden( PopupMenu* pPopupMenu, const css::uno::Reference< css::util::XURLTransformer >& rTransformer );
        OUString RetrieveLabelFromCommand(const OUString& rCmdURL);

    private:

        void Destroy();

        struct MenuItemHandler
        {
            MenuItemHandler( sal_uInt16             aItemId,
                             css::uno::Reference< css::frame::XStatusListener >& xManager,
                             css::uno::Reference< css::frame::XDispatch >& rDispatch ) :
                             nItemId( aItemId ),
                             xSubMenuManager( xManager ),
                             xMenuItemDispatch( rDispatch ) {}

            sal_uInt16                                                        nItemId;
            OUString                                                          aTargetFrame;
            OUString                                                          aMenuItemURL;
            css::uno::Reference< css::frame::XStatusListener >                xSubMenuManager;
            css::uno::Reference< css::frame::XDispatch >                      xMenuItemDispatch;
            css::uno::Reference< css::frame::XPopupMenuController >           xPopupMenuController;
            css::uno::Reference< css::awt::XPopupMenu >                       xPopupMenu;
            vcl::KeyCode                                                      aKeyCode;
        };

        void             RetrieveShortcuts( std::vector< MenuItemHandler* >& aMenuShortCuts );
        void             CheckAndAddMenuExtension( Menu* pMenu );
        static void      impl_RetrieveShortcutsFromConfiguration( const css::uno::Reference< css::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                                  const css::uno::Sequence< OUString >& rCommands,
                                                                  std::vector< MenuItemHandler* >& aMenuShortCuts );
        static void      MergeAddonMenus( Menu* pMenuBar, const MergeMenuInstructionContainer&, const OUString& aModuleIdentifier );

        MenuItemHandler* GetMenuItemHandler( sal_uInt16 nItemId );
        bool         CreatePopupMenuController( MenuItemHandler* pMenuItemHandler );
        void             AddMenu(MenuBarManager* pSubMenuManager,const OUString& _sItemCommand,sal_uInt16 _nItemId);
        sal_uInt16           FillItemCommand(OUString& _rItemCommand, Menu* _pMenu,sal_uInt16 _nIndex) const;
        void             Init(const css::uno::Reference< css::frame::XFrame >& rFrame,Menu* pAddonMenu,bool bDelete,bool bDeleteChildren,bool _bHandlePopUp);
        void             SetHdl();

        bool                                                         m_bDisposed : 1,
                                                                     m_bInitialized : 1,
                                                                     m_bDeleteMenu : 1,
                                                                     m_bDeleteChildren : 1,
                                                                     m_bActive : 1,
                                                                     m_bIsBookmarkMenu : 1,
                                                                     m_bShowMenuImages : 1;
        bool                                                         m_bRetrieveImages : 1,
                                                                     m_bAcceleratorCfg : 1;
        bool                                                         m_bModuleIdentified;
        bool                                                         m_bHasMenuBar;
        OUString                                                     m_aMenuItemCommand;
        OUString                                                     m_aModuleIdentifier;
        Menu*                                                        m_pVCLMenu;
        css::uno::Reference< css::frame::XFrame >                    m_xFrame;
        css::uno::Reference< css::container::XNameAccess >           m_xUICommandLabels;
        css::uno::Reference< css::frame::XUIControllerFactory >      m_xPopupMenuControllerFactory;
        ::std::vector< MenuItemHandler* >                            m_aMenuItemHandlerVector;
        osl::Mutex                                                   m_mutex;
        ::cppu::OMultiTypeInterfaceContainerHelper                   m_aListenerContainer;   /// container for ALL Listener
        css::uno::Reference< css::frame::XDispatchProvider >         m_xDispatchProvider;
        css::uno::Reference< css::ui::XImageManager >                m_xDocImageManager;
        css::uno::Reference< css::ui::XImageManager >                m_xModuleImageManager;
        css::uno::Reference< css::ui::XAcceleratorConfiguration >    m_xDocAcceleratorManager;
        css::uno::Reference< css::ui::XAcceleratorConfiguration >    m_xModuleAcceleratorManager;
        css::uno::Reference< css::ui::XAcceleratorConfiguration >    m_xGlobalAcceleratorManager;
        css::uno::Reference< css::uno::XComponentContext >           m_xContext;
        css::uno::Reference< css::util::XURLTransformer >            m_xURLTransformer;
        css::uno::Reference< css::container::XIndexAccess >          m_xDeferedItemContainer;
        OUString                                                     m_sIconTheme;
        Timer                                                        m_aAsyncSettingsTimer;
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
