/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: env_subst.test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:47:30 $
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

