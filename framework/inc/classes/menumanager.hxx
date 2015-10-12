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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_MENUMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_MENUMANAGER_HXX

#include <vector>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <vcl/accel.hxx>
#include <cppuhelper/implbase.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace framework
{

class BmkMenu;
class AddonMenu;
class AddonPopupMenu;
class MenuManager : public ::cppu::WeakImplHelper< css::frame::XStatusListener >
{
    public:
        MenuManager(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            css::uno::Reference< css::frame::XFrame >& rFrame,
            Menu* pMenu,
            bool bDelete,
            bool bDeleteChildren );

        virtual ~MenuManager();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        DECL_LINK_TYPED( Select, Menu *, bool );

        Menu*   GetMenu() const { return m_pVCLMenu; }

        void    RemoveListener();

        static void UpdateSpecialWindowMenu( Menu* pMenu ,const css::uno::Reference< css::uno::XComponentContext >& xContext);
        static void FillMenuImages(
            css::uno::Reference< css::frame::XFrame >& xFrame,
            Menu* _pMenu,
            bool bShowMenuImages
        );

    protected:
        DECL_LINK_TYPED(Highlight, Menu *, bool);
        DECL_LINK_TYPED( Activate, Menu *, bool );
        DECL_LINK_TYPED( Deactivate, Menu *, bool );

    private:
        void UpdateSpecialFileMenu( Menu* pMenu );
        void ClearMenuDispatch(const css::lang::EventObject& Source = css::lang::EventObject(),bool _bRemoveOnly = true);
        void SetHdl();
        void AddMenu(PopupMenu* _pPopupMenu,const OUString& _sItemCommand,sal_uInt16 _nItemId,bool _bDelete,bool _bDeleteChildren);
        sal_uInt16 FillItemCommand(OUString& _rItemCommand, Menu* _pMenu,sal_uInt16 _nIndex) const;

        struct MenuItemHandler
        {
            MenuItemHandler( sal_uInt16 aItemId, MenuManager* pManager, css::uno::Reference< css::frame::XDispatch >& rDispatch ) :
                nItemId( aItemId ), pSubMenuManager( pManager ), xMenuItemDispatch( rDispatch ) {}

            sal_uInt16                  nItemId;
            OUString         aTargetFrame;
            OUString         aMenuItemURL;
            OUString         aFilter;
            OUString         aPassword;
            OUString         aTitle;
            MenuManager*            pSubMenuManager;
            css::uno::Reference< css::frame::XDispatch >  xMenuItemDispatch;
        };

        void             CreatePicklistArguments(
                            css::uno::Sequence< css::beans::PropertyValue >& aArgsList,
                            const MenuItemHandler* );

        MenuItemHandler* GetMenuItemHandler( sal_uInt16 nItemId );

        bool                            m_bInitialized;
        bool                            m_bDeleteMenu;
        bool                            m_bDeleteChildren;
        bool                            m_bActive;
        bool                            m_bIsBookmarkMenu;
        bool                            m_bShowMenuImages;
        OUString                     m_aMenuItemCommand;
        Menu*                               m_pVCLMenu;
        css::uno::Reference< css::frame::XFrame >                 m_xFrame;
        ::std::vector< MenuItemHandler* >   m_aMenuItemHandlerVector;

        css::uno::Reference< css::uno::XComponentContext >      m_xContext;
        css::uno::Reference< css::util::XURLTransformer >       m_xURLTransformer;
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
