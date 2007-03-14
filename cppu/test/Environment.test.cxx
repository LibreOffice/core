/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Environment.test.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-14 08:31:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2000 by Sun Microsystems, Inc.
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

// Test for uno/environment.hxx respectively com::sun::star::uno::Environment

#include "uno/environment.hxx"
#include "uno/lbnames.h"
#include "sal/main.h"

#include <iostream>


using namespace com::sun::star;

static rtl::OUString    s_comment;

static void s_test__constructor_oustring_context(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__constructor_outstring_context\n"));

    uno::Environment environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO)), NULL);

    if (!environment.get())
            s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't get an Environment - FAILURE\n"));
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    s_test__constructor_oustring_context();


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
