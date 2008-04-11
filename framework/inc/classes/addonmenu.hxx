/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: addonmenu.hxx,v $
 * $Revision: 1.8 $
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
#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#define __FRAMEWORK_CLASSES_ADDONMENU_HXX_

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/menu.hxx>

#define ADDONMENU_ITEMID_START       2000
#define ADDONMENU_ITEMID_END         3000

namespace framework
{

class AddonMenu : public PopupMenu
{
    public:
                        AddonMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                        ~AddonMenu();

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
};

class AddonMenuManager;
class AddonPopupMenu : public PopupMenu
{
    public:
                                ~AddonPopupMenu();

        // Check if command URL string has the unique prefix to identify addon popup menus
        static sal_Bool         IsCommandURLPrefix( const rtl::OUString& aCmdURL );

        void                    SetCommandURL( const rtl::OUString& aCmdURL ) { m_aCommandURL = aCmdURL; }
        const rtl::OUString&    GetCommandURL() const { return m_aCommandURL; }

    protected:
        void                    Initialize( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rAddonPopupMenuDefinition );

    private:
                                AddonPopupMenu( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame );

        rtl::OUString                                                     m_aCommandURL;
        ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame > m_xFrame;

    friend class AddonMenuManager;
};

class AddonMenuManager
{
    public:
        enum MenuType
        {
            ADDON_MENU,
            ADDON_POPUPMENU
        };

        static sal_Bool   HasAddonMenuElements();
        static sal_Bool   HasAddonHelpMenuElements();

        static sal_Bool   IsAddonMenuId( USHORT nId ) { return (( nId >= ADDONMENU_ITEMID_START ) && ( nId < ADDONMENU_ITEMID_END )); }

        // Check if the context string matches the provided xModel context
        static sal_Bool   IsCorrectContext( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel, const rtl::OUString& aContext );

        // Factory method to create different Add-On menu types
        static PopupMenu* CreatePopupMenuType( MenuType eMenuType, const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame );

        // Create the Add-Ons menu
        static AddonMenu* CreateAddonMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

        // Merge the Add-Ons help menu items into the given menu bar at a defined pos
        static void       MergeAddonHelpMenu( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                              MenuBar* pMergeMenuBar );

        // Merge the addon popup menus into the given menu bar at the provided pos.
        static void       MergeAddonPopupMenus( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                                const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel,
                                                USHORT   nMergeAtPos,
                                                MenuBar* pMergeMenuBar );

        // Returns the next position to insert a menu item/sub menu
        static USHORT     GetNextPos( USHORT nPos );

        // Build up the menu item and sub menu into the provided pCurrentMenu. The sub menus should be of type nSubMenuType.
        static void       BuildMenu( PopupMenu*  pCurrentMenu,
                                     MenuType    nSubMenuType,
                                     USHORT      nInsPos,
                                     USHORT&     nUniqueMenuId,
                                     com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > > aAddonMenuDefinition,
                                     const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                     const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel );

        // Retrieve the menu entry property values from a sequence
        static void       GetMenuEntry( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rAddonMenuEntry,
                                        ::rtl::OUString& rTitle,
                                        ::rtl::OUString& rURL,
                                        ::rtl::OUString& rTarget,
                                        ::rtl::OUString& rImageId,
                                        ::rtl::OUString& rContext,
                                        com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > >& rAddonSubMenu );
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
