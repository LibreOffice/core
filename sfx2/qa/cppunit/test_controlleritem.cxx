/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>

namespace {

class ControllerItemTest
    : public ::CppUnit::TestFixture
{
public:
    virtual void setUp() override {}
    virtual void tearDown() override {}

    void test();

    CPPUNIT_TEST_SUITE(ControllerItemTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
};

static bool bDeleted = false;

class FooController : public SfxControllerItem {
public:
    FooController() : SfxControllerItem() {}
    virtual ~FooController() { bDeleted = true; }
};

void ControllerItemTest::test()
{
    FooController *pController(new FooController());

    // TESTME: binding, un-binding, re-binding, IsBound, SetId etc.

    pController->dispose();
    delete pController;
    CPPUNIT_ASSERT( bDeleted );
}

CPPUNIT_TEST_SUITE_REGISTRATION(ControllerItemTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
