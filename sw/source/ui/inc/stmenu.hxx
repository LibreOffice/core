/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmenu.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:23:29 $
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
 *    Initial Contributer was Fabalabs Software GmbH, Jakob Lechner
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

// SMARTTAGS

#ifndef _STMENU_HXX
#define _STMENU_HXX

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#include <vector>

#ifndef _COM_SUN_STAR_SMARTTAGS_XSMARTTAGACTION_HPP_
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#endif

#ifndef _COM_SUN_STAR_SMARTTAGS_XSTRINGKEYMAP_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif

class SwView;

/** Class: SwSmartTagPopup

   This class contains the implementation of the smarttag popup
   menu that is opened if a user clicks on an underlined word.

   The menu is built in the constructor and the actions for each
   menu entry are invoked in the excute-method.
*/

class SwSmartTagPopup : public PopupMenu
{
    SwView*  mpSwView;
    com::sun::star::uno::Reference< com::sun::star::text::XTextRange > mxTextRange;

    struct InvokeAction
    {
        com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > mxAction;
        com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > mxSmartTagProperties;
        sal_uInt32 mnActionID;
        InvokeAction( com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > xAction,
                      com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xSmartTagProperties,
                      sal_uInt32 nActionID ) : mxAction( xAction ), mxSmartTagProperties( xSmartTagProperties ), mnActionID( nActionID ) {}
    };

    std::vector< InvokeAction > maInvokeActions;

    using PopupMenu::Execute;

public:
    SwSmartTagPopup( SwView* _pSwView,
                     ::com::sun::star::uno::Sequence< rtl::OUString >& rSmartTagTypes,
                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                     ::com::sun::star::uno::Reference< com::sun::star::text::XTextRange > xTextRange );

    sal_uInt16  Execute( const Rectangle& rPopupPos, Window* pWin );
};

#endif

