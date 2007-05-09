/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: env.tester.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:48:13 $
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
