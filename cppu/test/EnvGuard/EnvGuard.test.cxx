/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvGuard.test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:41:19 $
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

#include "sal/main.h"

#include "cppu/EnvGuards.hxx"
#include "uno/environment.hxx"

#include <iostream>


using namespace com::sun::star;

static rtl::OUString    s_message;


static void s_test__entered(void)
{
    s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__entered"));

    rtl::OUString ref(RTL_CONSTASCII_USTRINGPARAM("uno:unsafe"));

    rtl::OUString current_EnvDcp;
    {
        cppu::EnvGuard envGuard(uno::Environment(ref, NULL));

        current_EnvDcp = uno::Environment::getCurrent(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))).getTypeName();
    }

    if (current_EnvDcp == ref)
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t        got: \""));
        s_message += current_EnvDcp;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t   expected: \""));
        s_message += ref;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

static void s_test__left(void)
{
    s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__left"));

    rtl::OUString ref(RTL_CONSTASCII_USTRINGPARAM("uno"));

    rtl::OUString current_EnvDcp;
    {
        cppu::EnvGuard envGuard(uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:unsafe")), NULL));
    }
    current_EnvDcp = uno::Environment::getCurrent(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))).getTypeName();


    if (current_EnvDcp == ref)
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t        got: \""));
        s_message += current_EnvDcp;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t   expected: \""));
        s_message += ref;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

static void s_test__clear(void)
{
    s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__clear"));

    rtl::OUString ref(RTL_CONSTASCII_USTRINGPARAM("uno"));

    rtl::OUString current_EnvDcp;
    {
        cppu::EnvGuard envGuard(uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:unsafe")), NULL));

        envGuard.clear();
        current_EnvDcp = uno::Environment::getCurrent(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))).getTypeName();
    }


    if (current_EnvDcp == ref)
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t        got: \""));
        s_message += current_EnvDcp;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t   expected: \""));
        s_message += ref;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    s_test__entered();
    s_test__left();
    s_test__clear();

    int ret;
    if (s_message.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILURE"))) == -1)
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS PASSED\n"));
        ret = 0;
    }
    else
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS _NOT_ PASSED\n"));
        ret = -1;
    }

    std::cerr
        << argv[0]
        << std::endl
        << rtl::OUStringToOString(s_message, RTL_TEXTENCODING_ASCII_US).getStr()
        << std::endl;

    return ret;
}
