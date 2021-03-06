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

#pragma once

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <unordered_map>
#include <vector>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <framework/addonsoptions.hxx>

namespace framework
{

struct PopupControllerEntry
{
    css::uno::WeakReference< css::frame::XDispatchProvider > m_xDispatchProvider;
};

typedef std::unordered_map< OUString, PopupControllerEntry > PopupControllerCache;

class MenuBarManager final :
    protected cppu::BaseMutex,
    public cppu::WeakComponentImplHelper<
        css::frame::XStatusListener,
        css::frame::XFrameActionListener,
        css::ui::XUIConfigurationListener,
        css::awt::XSystemDependentMenuPeer>
{
    public:
        MenuBarManager(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::frame::XFrame >& rFrame,
            const css::uno::Reference< css::util::XURLTransformer >& _xURLTransformer,
            const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
            const OUString& aModuleIdentifier,
            Menu* pMenu,
            bool bDelete,
            bool bHasMenuBar = true );

        virtual ~MenuBarManager() override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& Action ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& Event ) override;

        // XSystemDependentMenuPeer
        virtual css::uno::Any SAL_CALL getMenuHandle( const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) override;

        DECL_LINK( Select, Menu *, bool );

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
                              bool bDelete );
        void SetItemContainer( const css::uno::Reference< css::container::XIndexAccess >& rItemContainer );
        void GetPopupController( PopupControllerCache& rPopupController );

    private:
        DECL_LINK( Activate, Menu *, bool );
        DECL_LINK( Deactivate, Menu *, bool );
        DECL_LINK( AsyncSettingsHdl, Timer *, void );

        void SAL_CALL disposing() override;
        void RemoveListener();
        void RequestImages();
        void RetrieveImageManagers();
        static bool MustBeHidden( PopupMenu* pPopupMenu, const css::uno::Reference< css::util::XURLTransformer >& rTransformer );
        OUString RetrieveLabelFromCommand(const OUString& rCmdURL);

        void Destroy();

        struct MenuItemHandler
        {
            MenuItemHandler( sal_uInt16             aItemId,
                             css::uno::Reference< css::frame::XStatusListener > const & xManager,
                             css::uno::Reference< css::frame::XDispatch > const & rDispatch ) :
                             nItemId( aItemId ),
                             bMadeInvisible ( false ),
                             xSubMenuManager( xManager ),
                             xMenuItemDispatch( rDispatch ) {}

            sal_uInt16                                                        nItemId;
            bool                                                              bMadeInvisible;
            OUString                                                          aTargetFrame;
            OUString                                                          aMenuItemURL;
            OUString                                                          aParsedItemURL;
            css::uno::Reference< css::frame::XStatusListener >                xSubMenuManager;
            css::uno::Reference< css::frame::XDispatch >                      xMenuItemDispatch;
            css::uno::Reference< css::frame::XPopupMenuController >           xPopupMenuController;
            css::uno::Reference< css::awt::XPopupMenu >                       xPopupMenu;
            vcl::KeyCode                                                      aKeyCode;
        };

        void             RetrieveShortcuts( std::vector< std::unique_ptr<MenuItemHandler> >& aMenuShortCuts );
        static void      FillMenuImages( css::uno::Reference< css::frame::XFrame > const & xFrame, Menu* _pMenu, bool bShowMenuImages );
        static void      impl_RetrieveShortcutsFromConfiguration( const css::uno::Reference< css::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                                  const css::uno::Sequence< OUString >& rCommands,
                                                                  std::vector< std::unique_ptr<MenuItemHandler> >& aMenuShortCuts );
        static void      MergeAddonMenus( Menu* pMenuBar, const MergeMenuInstructionContainer&, const OUString& aModuleIdentifier );

        MenuItemHandler* GetMenuItemHandler( sal_uInt16 nItemId );
        bool         CreatePopupMenuController( MenuItemHandler* pMenuItemHandler,
                                                const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
                                                const OUString& rModuleIdentifier );
        void             AddMenu(MenuBarManager* pSubMenuManager,const OUString& _sItemCommand,sal_uInt16 _nItemId);
        sal_uInt16           FillItemCommand(OUString& _rItemCommand, Menu* _pMenu,sal_uInt16 _nIndex) const;
        void             SetHdl();

        bool                                                         m_bDeleteMenu;
        bool                                                         m_bActive;
        bool                                                         m_bShowMenuImages;
        bool                                                         m_bRetrieveImages;
        bool                                                         m_bAcceleratorCfg;
        bool                                                         m_bHasMenuBar;
        OUString                                                     m_aModuleIdentifier;
        VclPtr<Menu>                                                 m_pVCLMenu;
        css::uno::Reference< css::frame::XFrame >                    m_xFrame;
        css::uno::Reference< css::frame::XUIControllerFactory >      m_xPopupMenuControllerFactory;
        ::std::vector< std::unique_ptr<MenuItemHandler> >            m_aMenuItemHandlerVector;
        css::uno::Reference< css::frame::XDispatchProvider >         m_xDispatchProvider;
        css::uno::Reference< css::ui::XImageManager >                m_xDocImageManager;
        css::uno::Reference< css::ui::XImageManager >                m_xModuleImageManager;
        css::uno::Reference< css::ui::XAcceleratorConfiguration >    m_xDocAcceleratorManager;
        css::uno::Reference< css::ui::XAcceleratorConfiguration >    m_xModuleAcceleratorManager;
        css::uno::Reference< css::ui::XAcceleratorConfiguration >    m_xGlobalAcceleratorManager;
        css::uno::Reference< css::uno::XComponentContext >           m_xContext;
        css::uno::Reference< css::util::XURLTransformer >            m_xURLTransformer;
        css::uno::Reference< css::container::XIndexAccess >          m_xDeferredItemContainer;
        OUString                                                     m_sIconTheme;
        Timer                                                        m_aAsyncSettingsTimer;
};

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
