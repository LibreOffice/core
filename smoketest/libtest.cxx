/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// yuck / FIXME ...
#include "../desktop/inc/liblibreoffice.h"

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp();
    virtual void tearDown();

private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

void Test::setUp()
{
}
void Test::tearDown()
{
}

void Test::test()
{
    rtl::OUString aArgSoffice;
    rtl::Bootstrap::get( rtl::OUString( "arg-soffice" ), aArgSoffice );
    OString aInstall = OUStringToOString( aArgSoffice, RTL_TEXTENCODING_UTF8 );
    fprintf( stderr, "liblibreoffice test: '%s'\n", aInstall.getStr() );
    lo_initialize( aInstall.getStr() );
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
