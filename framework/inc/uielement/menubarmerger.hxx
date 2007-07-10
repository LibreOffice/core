/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubarmerger.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-10 15:07:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
