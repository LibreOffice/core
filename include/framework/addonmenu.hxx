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

#include <rtl/ustring.hxx>
#include <vcl/vclptr.hxx>

#include <framework/fwedllapi.h>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::uno { template <class E> class Sequence; }
namespace com::sun::star::uno { template <typename > class Reference; }

class MenuBar;
class PopupMenu;

#define ADDONMENU_ITEMID_START       2000
#define ADDONMENU_ITEMID_END         3000

namespace framework
{

class FWE_DLLPUBLIC AddonMenuManager
{
    public:
        static bool       HasAddonMenuElements();

        // Check if the context string matches the provided xModel context
        static bool IsCorrectContext(const OUString& rModuleIdentifier, const OUString& rContext);

        // Create the Add-Ons menu
        static VclPtr<PopupMenu> CreateAddonMenu( const css::uno::Reference< css::frame::XFrame >& rFrame );

        // Merge the Add-Ons help menu items into the given menu bar at a defined pos
        static void       MergeAddonHelpMenu( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                              MenuBar const * pMergeMenuBar );

        // Merge the addon popup menus into the given menu bar at the provided pos.
        static void       MergeAddonPopupMenus( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                                sal_uInt16   nMergeAtPos,
                                                MenuBar* pMergeMenuBar );

        // Returns the next position to insert a menu item/sub menu
        static sal_uInt16     GetNextPos( sal_uInt16 nPos );

        // Build up the menu item and sub menu into the provided pCurrentMenu. The sub menus should be of type nSubMenuType.
        static void       BuildMenu( PopupMenu*  pCurrentMenu,
                                     sal_uInt16      nInsPos,
                                     sal_uInt16&     nUniqueMenuId,
                                     const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& aAddonMenuDefinition,
                                     const css::uno::Reference< css::frame::XFrame >& rFrame,
                                     const OUString& rModuleIdentifier );

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
