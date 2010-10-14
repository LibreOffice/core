/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
