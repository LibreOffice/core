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
    static const ::rtl::OUString s_sData(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.UserProfile/Data"));
    static const ::rtl::OUString s_so(RTL_CONSTASCII_USTRINGPARAM("o")); // USER_OPT_COMPANY
    static const ::rtl::OUString s_sgivenname(RTL_CONSTASCII_USTRINGPARAM("givenname")); // USER_OPT_FIRSTNAME
    static const ::rtl::OUString s_ssn(RTL_CONSTASCII_USTRINGPARAM("sn")); // USER_OPT_LASTNAME
    static const ::rtl::OUString s_sinitials(RTL_CONSTASCII_USTRINGPARAM("initials")); // USER_OPT_ID
    static const ::rtl::OUString s_sstreet(RTL_CONSTASCII_USTRINGPARAM("street")); // USER_OPT_STREET
    static const ::rtl::OUString s_sl(RTL_CONSTASCII_USTRINGPARAM("l")); // USER_OPT_CITY
    static const ::rtl::OUString s_sst(RTL_CONSTASCII_USTRINGPARAM("st")); // USER_OPT_STATE
    static const ::rtl::OUString s_spostalcode(RTL_CONSTASCII_USTRINGPARAM("postalcode")); // USER_OPT_ZIP
    static const ::rtl::OUString s_sc(RTL_CONSTASCII_USTRINGPARAM("c")); // USER_OPT_COUNTRY
    static const ::rtl::OUString s_stitle(RTL_CONSTASCII_USTRINGPARAM("title")); // USER_OPT_TITLE
    static const ::rtl::OUString s_sposition(RTL_CONSTASCII_USTRINGPARAM("position")); // USER_OPT_POSITION
    static const ::rtl::OUString s_shomephone(RTL_CONSTASCII_USTRINGPARAM("homephone")); // USER_OPT_TELEPHONEHOME
    static const ::rtl::OUString s_stelephonenumber(RTL_CONSTASCII_USTRINGPARAM("telephonenumber")); // USER_OPT_TELEPHONEWORK
    static const ::rtl::OUString s_sfacsimiletelephonenumber(RTL_CONSTASCII_USTRINGPARAM("facsimiletelephonenumber")); // USER_OPT_FAX
    static const ::rtl::OUString s_smail(RTL_CONSTASCII_USTRINGPARAM("mail")); // USER_OPT_EMAIL
    static const ::rtl::OUString s_scustomernumber(RTL_CONSTASCII_USTRINGPARAM("customernumber")); // USER_OPT_CUSTOMERNUMBER
    static const ::rtl::OUString s_sfathersname(RTL_CONSTASCII_USTRINGPARAM("fathersname")); // USER_OPT_FATHERSNAME
    static const ::rtl::OUString s_sapartment(RTL_CONSTASCII_USTRINGPARAM("apartment")); // USER_OPT_APARTMENT
}

#endif //  INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
