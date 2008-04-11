/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EnvDcp.test.cxx,v $
 * $Revision: 1.3 $
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
