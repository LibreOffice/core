/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: env_subst.test.cxx,v $
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

#include "uno/environment.hxx"

#include <iostream>


using namespace com::sun::star;

static rtl::OUString s_comment;

static void s_test_substituting(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_substituting\n"));

     putenv(strdup("UNO_ENV_SUBST:uno:unsafe=uno:affine"));

    uno::Environment env(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:unsafe")));
    if (!env.getTypeName().equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:affine"))))
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE - expected \"uno:affine\" instead of \""));
        s_comment += env.getTypeName();
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

static void s_test_not_substituting(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_not_substituting\n"));

    putenv(strdup("UNO_ENV_SUBST:uno:unsafe=uno:affine"));

    uno::Environment env(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:affine")));
    if (!env.getTypeName().equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:affine"))))
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE - expected \"uno:affine\" instead of \""));
        s_comment += env.getTypeName();
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"\n"));
    }
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    s_test_substituting();
    s_test_not_substituting();


    int ret;
    if (s_comment.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILURE"))) == -1)
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS PASSED\n"));
        ret = 0;
    }
    else
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS _NOT_ PASSED\n"));
        ret = -1;
    }

    std::cerr
        << argv[0]
        << std::endl
        << rtl::OUStringToOString(s_comment, RTL_TEXTENCODING_ASCII_US).getStr()
        << std::endl;

    return ret;
}

