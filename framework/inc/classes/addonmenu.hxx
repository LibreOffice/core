/*************************************************************************
 *
 *  $RCSfile: addonmenu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:30 $
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

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/menu.hxx>
#include <vcl/image.hxx>

class String;
class ImageList;

#define ADDONMENU_ITEMID_START    20000

namespace framework
{

class AddonMenu_Impl;
class AddonMenu : public PopupMenu
{
    public:
                        AddonMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                        ~AddonMenu();

        void            Initialize(); // Synchrones Laden der Eintraege

        static sal_Bool HasElements();

    protected:
        USHORT          CreateMenuId();

    private:
                        AddonMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, AddonMenu* pRoot );
                        PopupMenu* BuildSubMenu( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aAddonSubMenuDefinition );

        AddonMenu_Impl* _pImp;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& m_xFrame;
};

class AddonPopupMenu : public PopupMenu
{
    public:
                                ~AddonPopupMenu();

        const rtl::OUString&    GetContext() const { return m_aContext; }
        const rtl::OUString&    GetCommandURL() const { return m_aCommandURL; }
        const rtl::OUString&    GetTitle() const { return m_aTitle; }
        USHORT                  GetId() const { return m_nId; }

        // Get the top-level popup menus for addons. The menubar is used as a container. Every popup menu is from type AddonPopupMenu!
        static void             GetAddonPopupMenus( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                                    MenuBar* pAddonPopupMenus );

        // Merge the addon popup menus into the given menu bar at the provided pos.
        static void             MergeAddonPopupMenus( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                                      ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel,
                                                      USHORT                nMergeAtPos,
                                                      MenuBar*              pMergeMenuBar );

    protected:
        void                    Initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rAddonPopupMenuDefinition );
        USHORT                  CreateMenuId();
        sal_Bool                IsCorrectContext( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel ) const;

    private:
                                AddonPopupMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );

        PopupMenu*              BuildSubMenu( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aAddonSubMenuDefinition );

        USHORT                                                                  m_nId;
        rtl::OUString                                                           m_aCommandURL;
        rtl::OUString                                                           m_aTitle;
        rtl::OUString                                                           m_aContext;
        AddonMenu_Impl*                                                         _pImp;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&    m_xFrame;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
