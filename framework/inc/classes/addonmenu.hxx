/*************************************************************************
 *
 *  $RCSfile: addonmenu.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:11:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#define __FRAMEWORK_CLASSES_ADDONMENU_HXX_

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/menu.hxx>

#define ADDONMENU_ITEMID_START    1500
#define ADDONMENU_ITEMID_END      2000

namespace framework
{

class AddonMenu : public PopupMenu
{
    public:
                        AddonMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                        ~AddonMenu();

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& m_xFrame;
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
                                AddonPopupMenu( com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame );

        rtl::OUString                                                       m_aCommandURL;
        ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame >&  m_xFrame;

    friend AddonMenuManager;
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
        static sal_Bool   IsCorrectContext( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel, const rtl::OUString& aContext );

        // Factory method to create different Add-On menu types
        static PopupMenu* CreatePopupMenuType( MenuType eMenuType, com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame );

        // Create the Add-Ons menu
        static AddonMenu* CreateAddonMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

        // Merge the Add-Ons help menu items into the given menu bar at a defined pos
        static void       MergeAddonHelpMenu( com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                              MenuBar* pMergeMenuBar );

        // Merge the addon popup menus into the given menu bar at the provided pos.
        static void       MergeAddonPopupMenus( com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                                com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel,
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
                                     com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                     com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rModel );

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
