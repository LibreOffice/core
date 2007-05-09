/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvDcp.test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:40:59 $
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

#include "cppu/EnvDcp.hxx"

#include <iostream>


static rtl::OUString    s_message;

static void s_test__getTypeName(void)
{
    s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__getTypeName"));

    rtl::OUString envDcp(RTL_CONSTASCII_USTRINGPARAM("acaQEQWE123:asda:2342"));

    rtl::OUString typeName(cppu::EnvDcp::getTypeName(envDcp));

    rtl::OUString ref(RTL_CONSTASCII_USTRINGPARAM("acaQEQWE123"));

    if (typeName == ref)
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t        got: \""));
        s_message += typeName;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t   expected: \""));
        s_message += ref;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

static void s_test__getPurpose(void)
{
    s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__getPurpose"));

    rtl::OUString envDcp(RTL_CONSTASCII_USTRINGPARAM("acaQEQWE123:asda:2342"));

    rtl::OUString purpose(cppu::EnvDcp::getPurpose(envDcp));

    rtl::OUString ref(RTL_CONSTASCII_USTRINGPARAM(":asda:2342"));

    if (purpose == ref)
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
    {
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t        got: \""));
        s_message += purpose;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t   expected: \""));
        s_message += ref;
        s_message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    s_test__getTypeName();
    s_test__getPurpose();

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
