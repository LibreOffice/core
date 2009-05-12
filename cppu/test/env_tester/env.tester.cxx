/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: env.tester.cxx,v $
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

#include <iostream>

#include "sal/main.h"
#include "rtl/ustring.hxx"
#include "rtl/string.hxx"
#include "rtl/alloc.h"
#include "uno/environment.hxx"
#include "uno/lbnames.h"

using namespace com::sun::star;


static rtl::OUString s_replaceCPP(rtl::OUString const & str)
{
    rtl::OUString cpp(RTL_CONSTASCII_USTRINGPARAM("CPP"));

    rtl::OUString result;

    sal_Int32 index_old = 0;
    sal_Int32 index     = str.indexOf(cpp, index_old);
    while (index != -1)
    {
        result += str.copy(index_old, index - index_old);
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)));

        index_old = index + 3;
        index = str.indexOf(cpp, index_old);
    }
    result += str.copy(index_old);

    return result;
}


rtl::OUString register_test(rtl::OUString const & envDcp);
rtl::OUString purpenv_test(rtl::OUString const & envDcp);



SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int ret = 0;
    rtl::OUString message;

    if (argc == 2)
    {
        message = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

        rtl::OUString envDcp(argv[1], rtl_str_getLength(argv[1]), RTL_TEXTENCODING_ASCII_US);
        envDcp = s_replaceCPP(envDcp);


        message += register_test(envDcp);
        message += purpenv_test(envDcp);

        if (message.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILURE"))) == -1)
        {
            message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS PASSED\n"));
            ret = 0;
        }
        else
        {
            message += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS _NOT_ PASSED\n"));
            ret = -1;
        }
    }
    else
        message = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<env. descriptor>"));

    std::cerr
        << argv[0]
        << " "
        << rtl::OUStringToOString(message, RTL_TEXTENCODING_ASCII_US).getStr()
        << std::endl;

    return ret;
}
