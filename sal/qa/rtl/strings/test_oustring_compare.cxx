/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_oustring_compare.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:59:46 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "cppunit/simpleheader.hxx"
#include "rtl/string.h"
#include "rtl/ustring.hxx"

namespace test { namespace oustring {

class Compare: public CppUnit::TestFixture
{
private:
    void equalsIgnoreAsciiCaseAscii();

    CPPUNIT_TEST_SUITE(Compare);
    CPPUNIT_TEST(equalsIgnoreAsciiCaseAscii);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test::oustring::Compare, "alltest");

void test::oustring::Compare::equalsIgnoreAsciiCaseAscii()
{
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAsciiL(
                       RTL_CONSTASCII_STRINGPARAM("abc")));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii(""));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("")));

    CPPUNIT_ASSERT(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abcd")).
                   equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii("abcd"));
}
