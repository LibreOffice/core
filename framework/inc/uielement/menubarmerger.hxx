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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARMERGER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARMERGER_HXX

#include <com/sun/star/beans/PropertyValue.hpp>

#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>

#include <string_view>
#include <vector>

namespace framework
{

struct AddonMenuItem;
typedef ::std::vector< AddonMenuItem > AddonMenuContainer;

struct AddonMenuItem
{
    OUString    aTitle;
    OUString    aURL;
    OUString    aContext;
    AddonMenuContainer aSubMenu;
};

enum RPResultInfo
{
    RP_OK,
    RP_POPUPMENU_NOT_FOUND,
    RP_MENUITEM_NOT_FOUND,
    RP_MENUITEM_INSTEAD_OF_POPUPMENU_FOUND
};

struct ReferencePathInfo
{
    VclPtr<Menu>       pPopupMenu;
    sal_uInt16         nPos;
    sal_Int32          nLevel;
    RPResultInfo       eResult;
};

namespace MenuBarMerger
{
        bool       IsCorrectContext(
            const OUString& aContext, std::u16string_view aModuleIdentifier );

        void       RetrieveReferencePath( const OUString&,
                                                 std::vector< OUString >& aReferencePath );
        ReferencePathInfo FindReferencePath( const std::vector< OUString >& aReferencePath, Menu* pMenu );
        sal_uInt16 FindMenuItem( std::u16string_view rCmd,
                                 Menu const * pMenu );
        void       GetMenuEntry( const css::uno::Sequence< css::beans::PropertyValue >& rAddonMenuEntry,
                                        AddonMenuItem& aAddonMenu );
        void       GetSubMenu( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rSubMenuEntries,
                                      AddonMenuContainer& rSubMenu );
        bool       ProcessMergeOperation( Menu*                     pMenu,
                                                 sal_uInt16                nPos,
                                                 sal_uInt16&               rItemId,
                                                 std::u16string_view rMergeCommand,
                                                 const OUString&    rMergeCommandParameter,
                                                 const OUString&    rModuleIdentifier,
                                                 const AddonMenuContainer& rAddonMenuItems );
        bool       ProcessFallbackOperation( const ReferencePathInfo&                aRefPathInfo,
                                                    sal_uInt16&                             rItemId,
                                                    std::u16string_view              rMergeCommand,
                                                    std::u16string_view              rMergeFallback,
                                                    const ::std::vector< OUString >& rReferencePath,
                                                    std::u16string_view              rModuleIdentifier,
                                                    const AddonMenuContainer&               rAddonMenuItems );
        bool       MergeMenuItems( Menu*                     pMenu,
                                          sal_uInt16                nPos,
                                          sal_uInt16                nModIndex,
                                          sal_uInt16&               rItemId,
                                          const OUString&    rModuleIdentifier,
                                          const AddonMenuContainer& rAddonMenuItems );
        bool       ReplaceMenuItem( Menu*                     pMenu,
                                           sal_uInt16                nPos,
                                           sal_uInt16&               rItemId,
                                           const OUString&    rModuleIdentifier,
                                           const AddonMenuContainer& rAddonMenuItems );
        bool       RemoveMenuItems( Menu*                     pMenu,
                                           sal_uInt16                nPos,
                                           const OUString&    rMergeCommandParameter );
        bool       CreateSubMenu( Menu*                     pSubMenu,
                                         sal_uInt16&               nItemId,
                                         const OUString&    rModuleIdentifier,
                                         const AddonMenuContainer& rAddonSubMenu );
}

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARMERGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
