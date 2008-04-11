/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: menubarmerger.hxx,v $
 * $Revision: 1.3 $
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
    ::rtl::OUString    aTitle;
    ::rtl::OUString    aURL;
    ::rtl::OUString    aTarget;
    ::rtl::OUString    aImageId;
    ::rtl::OUString    aContext;
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
        static bool       IsCorrectContext( const ::rtl::OUString& aContext, const ::rtl::OUString& aModuleIdentifier );

        static void       RetrieveReferencePath( const ::rtl::OUString&,
                                                 std::vector< ::rtl::OUString >& aReferencePath );
        static ReferencePathInfo FindReferencePath( const std::vector< ::rtl::OUString >& aReferencePath, Menu* pMenu );
        static sal_uInt16 FindMenuItem( const ::rtl::OUString& rCmd,
                                        Menu* pMenu );
        static void       GetMenuEntry( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rAddonMenuEntry,
                                        AddonMenuItem& aAddonMenu );
        static void       GetSubMenu( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& rSubMenuEntries,
                                      AddonMenuContainer& rSubMenu );
        static bool       ProcessMergeOperation( Menu*                     pMenu,
                                                 sal_uInt16                nPos,
                                                 sal_uInt16&               rItemId,
                                                 const ::rtl::OUString&    rMergeCommand,
                                                 const ::rtl::OUString&    rMergeCommandParameter,
                                                 const ::rtl::OUString&    rModuleIdentifier,
                                                 const AddonMenuContainer& rAddonMenuItems );
        static bool       ProcessFallbackOperation( const ReferencePathInfo&                aRefPathInfo,
                                                    sal_uInt16&                             rItemId,
                                                    const ::rtl::OUString&                  rMergeCommand,
                                                    const ::rtl::OUString&                  rMergeFallback,
                                                    const ::std::vector< ::rtl::OUString >& rReferencePath,
                                                    const ::rtl::OUString&                  rModuleIdentifier,
                                                    const AddonMenuContainer&               rAddonMenuItems );
        static bool       ProcessFallbackOperation();
        static bool       MergeMenuItems( Menu*                     pMenu,
                                          sal_uInt16                nPos,
                                          sal_uInt16                nModIndex,
                                          sal_uInt16&               rItemId,
                                          const ::rtl::OUString&    rModuleIdentifier,
                                          const AddonMenuContainer& rAddonMenuItems );
        static bool       ReplaceMenuItem( Menu*                     pMenu,
                                           sal_uInt16                nPos,
                                           sal_uInt16&               rItemId,
                                           const ::rtl::OUString&    rModuleIdentifier,
                                           const AddonMenuContainer& rAddonMenuItems );
        static bool       RemoveMenuItems( Menu*                     pMenu,
                                           sal_uInt16                nPos,
                                           const ::rtl::OUString&    rMergeCommandParameter );
        static bool       CreateSubMenu( Menu*                     pSubMenu,
                                         sal_uInt16&               nItemId,
                                         const ::rtl::OUString&    rModuleIdentifier,
                                         const AddonMenuContainer& rAddonSubMenu );

    private:
        MenuBarMerger();
        MenuBarMerger( const MenuBarMerger& );
        MenuBarMerger& operator=( const MenuBarMerger& );
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_MENUBARMERGER_HXX_
