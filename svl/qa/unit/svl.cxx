/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// TODO ...
//    officecfg: can we move this into our skeleton ?
//          Solve the Setup.xcu problem pleasantly [ custom version ? ]
//    deliver.pl
//          don't call regcomp if we don't have it.
//              In an ideal world
//              a) scp2 goes away and logic moved into the deliver d.lst
//              b) install set gets built incrementally as the build progresses
//              c) the new .xml component registration stuff then removes
//                 the need for manually calling regcomp and knowing what
//                 services we need, and in what .so they are implemented

#include "precompiled_svl.hxx"

#ifdef WNT
# include <prewin.h>
# include <postwin.h>
# undef ERROR
#endif

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/config.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/oslfile2streamwrap.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <i18npool/lang.h>

#include "svl/zforlist.hxx"
#include "svl/zformat.hxx"

#include <iostream>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::cout;
using ::std::endl;

namespace {

::std::ostream& operator<< (::std::ostream& os, const OUString& str)
{
    return os << ::rtl::OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}


class Test : public CppUnit::TestFixture {
public:
    Test();
    ~Test();

    virtual void setUp();
    virtual void tearDown();

    void testNumberFormat();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testNumberFormat);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_xContext;
};

Test::Test()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);
}

void Test::setUp()
{
}

void Test::tearDown()
{
}

Test::~Test()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

void Test::testNumberFormat()
{
    SvNumberFormatter aFormatter(LANGUAGE_ENGLISH_US);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
