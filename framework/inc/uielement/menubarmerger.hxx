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

#ifndef __FRAMEWORK_UIELEMENT_MENUBARMERGER_HXX_
#define __FRAMEWORK_UIELEMENT_MENUBARMERGER_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>

#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>

namespace framework
{

struct AddonMenuItem;
typedef ::std::vector< AddonMenuItem > AddonMenuContainer;

struct AddonMenuItem
{
    OUString    aTitle;
    OUString    aURL;
    OUString    aTarget;
    OUString    aImageId;
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
    Menu*              pPopupMenu;
    sal_uInt16         nPos;
    sal_Int32          nLevel;
    RPResultInfo       eResult;
};

class MenuBarMerger
{
    public:
        static bool       IsCorrectContext( const OUString& aContext, const OUString& aModuleIdentifier );

        static void       RetrieveReferencePath( const OUString&,
                                                 std::vector< OUString >& aReferencePath );
        static ReferencePathInfo FindReferencePath( const std::vector< OUString >& aReferencePath, Menu* pMenu );
        static sal_uInt16 FindMenuItem( const OUString& rCmd,
                                        Menu* pMenu );
        static void       GetMenuEntry( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rAddonMenuEntry,
                                        AddonMenuItem& aAddonMenu );
        static void       GetSubMenu( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& rSubMenuEntries,
                                      AddonMenuContainer& rSubMenu );
        static bool       ProcessMergeOperation( Menu*                     pMenu,
                                                 sal_uInt16                nPos,
                                                 sal_uInt16&               rItemId,
                                                 const OUString&    rMergeCommand,
                                                 const OUString&    rMergeCommandParameter,
                                                 const OUString&    rModuleIdentifier,
                                                 const AddonMenuContainer& rAddonMenuItems );
        static bool       ProcessFallbackOperation( const ReferencePathInfo&                aRefPathInfo,
                                                    sal_uInt16&                             rItemId,
                                                    const OUString&                  rMergeCommand,
                                                    const OUString&                  rMergeFallback,
                                                    const ::std::vector< OUString >& rReferencePath,
                                                    const OUString&                  rModuleIdentifier,
                                                    const AddonMenuContainer&               rAddonMenuItems );
        static bool       ProcessFallbackOperation();
        static bool       MergeMenuItems( Menu*                     pMenu,
                                          sal_uInt16                nPos,
                                          sal_uInt16                nModIndex,
                                          sal_uInt16&               rItemId,
                                          const OUString&    rModuleIdentifier,
                                          const AddonMenuContainer& rAddonMenuItems );
        static bool       ReplaceMenuItem( Menu*                     pMenu,
                                           sal_uInt16                nPos,
                                           sal_uInt16&               rItemId,
                                           const OUString&    rModuleIdentifier,
                                           const AddonMenuContainer& rAddonMenuItems );
        static bool       RemoveMenuItems( Menu*                     pMenu,
                                           sal_uInt16                nPos,
                                           const OUString&    rMergeCommandParameter );
        static bool       CreateSubMenu( Menu*                     pSubMenu,
                                         sal_uInt16&               nItemId,
                                         const OUString&    rModuleIdentifier,
                                         const AddonMenuContainer& rAddonSubMenu );

    private:
        MenuBarMerger();
        MenuBarMerger( const MenuBarMerger& );
        MenuBarMerger& operator=( const MenuBarMerger& );
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_MENUBARMERGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
