/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvStack.tester.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:41:59 $
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

#include "uno/lbnames.h"
#include "uno/environment.hxx"
#include "cppu/EnvDcp.hxx"

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif


#define CPPU_test_EnvStack_tester_IMPL
#include "EnvStack.tester.hxx"


using namespace com::sun::star;


static rtl::OUString s_getCurrentEnvDcp(void)
{
    uno::Environment env(uno::Environment::getCurrent());
    rtl::OUString env_dcp(env.getTypeName());

    return env_dcp;
}

extern "C" { static void s_getCurrentEnvDcp_v(va_list param)
{
    rtl_uString ** ppEnvDcp  = va_arg(param, rtl_uString **);

    rtl::OUString env_dcp(s_getCurrentEnvDcp());

    rtl_uString_assign(ppEnvDcp, env_dcp.pData);
}}

static rtl::OUString s_test__uno_Environment_invoke(rtl::OUString const & src_purpose,
                                                    rtl::OUString const & dst_purpose,
                                                    rtl::OUString const & ref)
{
    rtl::OUString result;
    rtl::OUString target_envDcp;

    rtl::OUString src_envDcp(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO));
    src_envDcp += src_purpose;

    rtl::OUString dst_envDcp(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO));
    dst_envDcp += dst_purpose;

    uno::Environment cur_Env(src_envDcp.pData, NULL);
    uno_Environment_enter(cur_Env.get());
    cur_Env.clear();

    uno::Environment dst_Env(dst_envDcp.pData, NULL);
    uno_Environment_invoke(dst_Env.get(), s_getCurrentEnvDcp_v, &target_envDcp.pData);
    dst_Env.clear();

    uno_Environment_enter(NULL);


    if (cppu::EnvDcp::getPurpose(target_envDcp).equals(dst_purpose)
        && g_commentStack.equals(ref))
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PASSED\n"));

    else
    {
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FAILED -> "));
        result += target_envDcp;
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\texpected: \""));
        result += ref;
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tgot:      \""));
        result += g_commentStack;
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }



    return result;
}

static rtl::OUString s_test__uno_Environment_invoke_v(va_list param)
{
    rtl::OUString const * pSrc_purpose = va_arg(param, rtl::OUString const *);
    rtl::OUString const * pDst_purpose = va_arg(param, rtl::OUString const *);
    rtl::OUString const * pRef         = va_arg(param, rtl::OUString  const *);


    rtl::OUString result;

    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("s_test__uno_Environment_invoke_v"));
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - \""));
    result += *pSrc_purpose;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" -> \""));
    result += *pDst_purpose;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));

    result += s_test__uno_Environment_invoke(*pSrc_purpose, *pDst_purpose, *pRef);

    return result;
}

static rtl::OUString s_do_a_test(rtl::OUString (* pTest)(va_list), ...)
{
    rtl::OUString result;

    va_list param;

    va_start(param, pTest);
    try {
        result += pTest(param);
    }
    catch (uno::Exception & exception)
    {
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FAILED with exception: "));
        result += exception.Message;
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
    }
    va_end(param);

    return result;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int result = 0;
    rtl::OString message;

    if (argc >= 4 && argc <= 5)
    {
        rtl::OUString src_purpose(argv[1], rtl_str_getLength(argv[1]), RTL_TEXTENCODING_ASCII_US);
        rtl::OUString dst_purpose(argv[2], rtl_str_getLength(argv[2]), RTL_TEXTENCODING_ASCII_US);
        rtl::OUString ref        (argv[3], rtl_str_getLength(argv[3]), RTL_TEXTENCODING_ASCII_US);

        if (argc == 5)
        {
            rtl::OUString test_env(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO));
            test_env += rtl::OUString(argv[4], rtl_str_getLength(argv[4]), RTL_TEXTENCODING_ASCII_US);
            g_env = uno::Environment(test_env);
        }


        message += rtl::OUStringToOString(
            s_do_a_test(s_test__uno_Environment_invoke_v, &src_purpose, &dst_purpose, &ref),
            RTL_TEXTENCODING_ASCII_US);


        if (g_env.is())
            g_env.clear();


        if (message.indexOf(rtl::OString("FAILED")) == -1)
            message += rtl::OString("TESTS PASSED\n");

        else
        {
            message += rtl::OString("TESTS _NOT_ PASSED\n");
            result = -1;
        }
    }
    else
    {
        message = "usage: EnvStack.tester <source purpose>"
            " <destination purpose>"
            " <reference string>"
            " [<reference environment>]\n\n";
    }

    std::cout << message.getStr();


    return result;
}

