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
#ifndef INCLUDED_FRAMEWORK_ADDONMENU_HXX
#define INCLUDED_FRAMEWORK_ADDONMENU_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vcl/menu.hxx>
#include <framework/fwedllapi.h>

#define ADDONMENU_ITEMID_START       2000
#define ADDONMENU_ITEMID_END         3000

namespace framework
{

class FWE_DLLPUBLIC AddonMenu : public PopupMenu
{
    public:
        AddonMenu( const css::uno::Reference< css::frame::XFrame >& rFrame );
        virtual ~AddonMenu();

    protected:
        css::uno::Reference< css::frame::XFrame > m_xFrame;
};

class AddonMenuManager;

class FWE_DLLPUBLIC AddonPopupMenu : public AddonMenu
{
    public:
        virtual ~AddonPopupMenu();

        // Check if command URL string has the unique prefix to identify addon popup menus
        static bool        IsCommandURLPrefix( const OUString& aCmdURL );

        void               SetCommandURL( const OUString& aCmdURL ) { m_aCommandURL = aCmdURL; }

    private:
        AddonPopupMenu( const css::uno::Reference< css::frame::XFrame >& rFrame );

        OUString               m_aCommandURL;

    friend class AddonMenuManager;
};

class FWE_DLLPUBLIC AddonMenuManager
{
    public:
        enum MenuType
        {
            ADDON_MENU,
            ADDON_POPUPMENU
        };

        static bool       HasAddonMenuElements();

        static bool       IsAddonMenuId( sal_uInt16 nId ) { return (( nId >= ADDONMENU_ITEMID_START ) && ( nId < ADDONMENU_ITEMID_END )); }

        // Check if the context string matches the provided xModel context
        static bool IsCorrectContext(const OUString& rModuleIdentifier, const OUString& rContext);

        // Factory method to create different Add-On menu types
        static PopupMenu* CreatePopupMenuType( MenuType eMenuType, const css::uno::Reference< css::frame::XFrame >& rFrame );

        // Create the Add-Ons menu
        static AddonMenu* CreateAddonMenu( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                           const css::uno::Reference< css::uno::XComponentContext >& rContext );

        // Merge the Add-Ons help menu items into the given menu bar at a defined pos
        static void       MergeAddonHelpMenu( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                              MenuBar* pMergeMenuBar,
                                              const css::uno::Reference< css::uno::XComponentContext >& rContext );

        // Merge the addon popup menus into the given menu bar at the provided pos.
        static void       MergeAddonPopupMenus( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                                sal_uInt16   nMergeAtPos,
                                                MenuBar* pMergeMenuBar,
                                                const css::uno::Reference< css::uno::XComponentContext >& rContext );

        // Returns the next position to insert a menu item/sub menu
        static sal_uInt16     GetNextPos( sal_uInt16 nPos );

        // Build up the menu item and sub menu into the provided pCurrentMenu. The sub menus should be of type nSubMenuType.
        static void       BuildMenu( PopupMenu*  pCurrentMenu,
                                     MenuType    nSubMenuType,
                                     sal_uInt16      nInsPos,
                                     sal_uInt16&     nUniqueMenuId,
                                     const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& aAddonMenuDefinition,
                                     const css::uno::Reference< css::frame::XFrame >& rFrame,
                                     const ::rtl::OUString& rModuleIdentifier );

        // Retrieve the menu entry property values from a sequence
        static void       GetMenuEntry( const css::uno::Sequence< css::beans::PropertyValue >& rAddonMenuEntry,
                                        OUString& rTitle,
                                        OUString& rURL,
                                        OUString& rTarget,
                                        OUString& rImageId,
                                        OUString& rContext,
                                        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rAddonSubMenu );
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_ADDONMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
