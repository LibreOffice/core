/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: historyoptions_const.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:17:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#ifndef INCLUDE_CONFIGITEMS_HISTORYOPTIONS_CONST_HXX
#define INCLUDE_CONFIGITEMS_HISTORYOPTIONS_CONST_HXX

#include <rtl/ustring.hxx>

namespace
{
    static const ::rtl::OUString s_sCommonHistory     = ::rtl::OUString::createFromAscii("org.openoffice.Office.Common/History");
    static const ::rtl::OUString s_sHistories         = ::rtl::OUString::createFromAscii("org.openoffice.Office.Histories/Histories");
    static const ::rtl::OUString s_sPickListSize      = ::rtl::OUString::createFromAscii("PickListSize");
    static const ::rtl::OUString s_sURLHistorySize    = ::rtl::OUString::createFromAscii("Size");
    static const ::rtl::OUString s_sHelpBookmarksSize = ::rtl::OUString::createFromAscii("HelpBookmarkSize");
    static const ::rtl::OUString s_sPickList          = ::rtl::OUString::createFromAscii("PickList");
    static const ::rtl::OUString s_sURLHistory        = ::rtl::OUString::createFromAscii("URLHistory");
    static const ::rtl::OUString s_sHelpBookmarks     = ::rtl::OUString::createFromAscii("HelpBookmarks");
    static const ::rtl::OUString s_sItemList          = ::rtl::OUString::createFromAscii("ItemList");
    static const ::rtl::OUString s_sOrderList         = ::rtl::OUString::createFromAscii("OrderList");
    static const ::rtl::OUString s_sHistoryItemRef    = ::rtl::OUString::createFromAscii("HistoryItemRef");
    static const ::rtl::OUString s_sURL               = ::rtl::OUString::createFromAscii("URL");
    static const ::rtl::OUString s_sFilter            = ::rtl::OUString::createFromAscii("Filter");
    static const ::rtl::OUString s_sTitle             = ::rtl::OUString::createFromAscii("Title");
    static const ::rtl::OUString s_sPassword          = ::rtl::OUString::createFromAscii("Password");
}

#endif //  INCLUDE_CONFIGITEMS_HISTORYOPTIONS_CONST_HXX
