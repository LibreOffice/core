/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmenu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 16:53:20 $
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
#include <map>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SMARTTAGMGR_HXX
#include "SmartTagMgr.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif

#ifndef _VIEW_HXX
#include <view.hxx>
#endif

using rtl::OUString;
using rtl::OString;
using com::sun::star::uno::Reference;
using com::sun::star::text::XTextRange;

class SwWrtShell;
class SvStringsDtor;

/** Class: SwSmartTagPopup

   This class contains the implementation of the smarttag popup
   menu that is opened if a user clicks on an underlined word.

   The menu is built in the constructor and the actions for each
   menu entry are invoked in the excute-method.
*/
class SwSmartTagPopup : public PopupMenu
{
  SwView*  pSwView;
  sal_Int32    nMaxVerbCount;
  std::vector <ActionReference> aActionRefs;
  Reference<XTextRange> xTextRange;

public:
  SwSmartTagPopup( SwView* _pSwView, std::vector <ActionReference> _aActionRefs, Reference<XTextRange> _xTextRange );
  sal_uInt16  Execute( Window* pWin, const Rectangle& rPopupPos );
};

#endif

