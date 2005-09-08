/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bmkmenu.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:02:37 $
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
#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#define __FRAMEWORK_CLASSES_BMKMENU_HXX

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/menu.hxx>
#include <vcl/image.hxx>

class String;
class ImageList;

#define BMKMENU_ITEMID_START    20000

namespace framework
{

class BmkMenu_Impl;
class BmkMenu : public PopupMenu
{
    public:
                        enum BmkMenuType
                        {
                            BMK_NEWMENU,
                            BMK_WIZARDMENU
                        };

                        BmkMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                 BmkMenuType nType );
                        ~BmkMenu();

    void                Initialize(); // Synchrones Laden der Eintraege

    protected:
        BmkMenu::BmkMenuType m_nType;
        USHORT          CreateMenuId();

    private:
                        BmkMenu( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                 BmkMenuType, BmkMenu* pRoot );

        BmkMenu_Impl*   _pImp;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& m_xFrame;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
