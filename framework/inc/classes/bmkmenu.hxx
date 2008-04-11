/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bmkmenu.hxx,v $
 * $Revision: 1.6 $
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
#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#define __FRAMEWORK_CLASSES_BMKMENU_HXX

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/frame/XFrame.hpp>

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
