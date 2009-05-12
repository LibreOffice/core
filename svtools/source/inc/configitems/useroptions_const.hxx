/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: useroptions_const.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:17:50 $
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

#ifndef INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX
#define INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX

#include <rtl/ustring.hxx>

namespace
{
    static const ::rtl::OUString s_sData                     = ::rtl::OUString::createFromAscii("org.openoffice.UserProfile/Data"); 
    static const ::rtl::OUString s_so                        = ::rtl::OUString::createFromAscii("o"); // USER_OPT_COMPANY
    static const ::rtl::OUString s_sgivenname                = ::rtl::OUString::createFromAscii("givenname"); // USER_OPT_FIRSTNAME
    static const ::rtl::OUString s_ssn                       = ::rtl::OUString::createFromAscii("sn"); // USER_OPT_LASTNAME
    static const ::rtl::OUString s_sinitials                 = ::rtl::OUString::createFromAscii("initials"); // USER_OPT_ID
    static const ::rtl::OUString s_sstreet                   = ::rtl::OUString::createFromAscii("street"); // USER_OPT_STREET
    static const ::rtl::OUString s_sl                        = ::rtl::OUString::createFromAscii("l"); // USER_OPT_CITY
    static const ::rtl::OUString s_sst                       = ::rtl::OUString::createFromAscii("st"); // USER_OPT_STATE
    static const ::rtl::OUString s_spostalcode               = ::rtl::OUString::createFromAscii("postalcode"); // USER_OPT_ZIP
    static const ::rtl::OUString s_sc                        = ::rtl::OUString::createFromAscii("c"); // USER_OPT_COUNTRY
    static const ::rtl::OUString s_stitle                    = ::rtl::OUString::createFromAscii("title"); // USER_OPT_TITLE
    static const ::rtl::OUString s_sposition                 = ::rtl::OUString::createFromAscii("position"); // USER_OPT_POSITION
    static const ::rtl::OUString s_shomephone                = ::rtl::OUString::createFromAscii("homephone"); // USER_OPT_TELEPHONEHOME
    static const ::rtl::OUString s_stelephonenumber          = ::rtl::OUString::createFromAscii("telephonenumber"); // USER_OPT_TELEPHONEWORK
    static const ::rtl::OUString s_sfacsimiletelephonenumber = ::rtl::OUString::createFromAscii("facsimiletelephonenumber"); // USER_OPT_FAX
    static const ::rtl::OUString s_smail                     = ::rtl::OUString::createFromAscii("mail"); // USER_OPT_EMAIL
    static const ::rtl::OUString s_scustomernumber           = ::rtl::OUString::createFromAscii("customernumber"); // USER_OPT_CUSTOMERNUMBER
    static const ::rtl::OUString s_sfathersname              = ::rtl::OUString::createFromAscii("fathersname"); // USER_OPT_FATHERSNAME
    static const ::rtl::OUString s_sapartment                = ::rtl::OUString::createFromAscii("apartment"); // USER_OPT_APARTMENT
}

#endif //  INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX
