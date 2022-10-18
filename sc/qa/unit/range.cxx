/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include "helper/qahelper.hxx"
#include <unotools/configmgr.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <global.hxx>
#include <scdll.hxx>

#include <address.hxx>
#include <rangeutl.hxx>
#include <refupdatecontext.hxx>

class ScRangeTest : public ScSimpleBootstrapFixture
{
public:
    CPPUNIT_TEST_SUITE(ScRangeTest);
    CPPUNIT_TEST(testOverlap);
    CPPUNIT_TEST(testRangeParsing);
    CPPUNIT_TEST(testAddressParsing);
    CPPUNIT_TEST(testTdf147451);
    CPPUNIT_TEST_SUITE_END();

    void testOverlap();
    void testRangeParsing();
    void testAddressParsing();
    void testTdf147451();
};

void ScRangeTest::testOverlap()
{
    ScRange aRange1( ScAddress( 0, 0, 0 ), ScAddress( 1, 1, 1 ));
    CPPUNIT_ASSERT(aRange1.Contains( ScAddress( 0, 0, 0 )));
    CPPUNIT_ASSERT(aRange1.Contains( ScAddress( 1, 1, 1 )));
    CPPUNIT_ASSERT(!aRange1.Contains( ScAddress( 2, 1, 1 )));
    CPPUNIT_ASSERT(!aRange1.Contains( ScAddress( 1, 2, 1 )));
    CPPUNIT_ASSERT(!aRange1.Contains( ScAddress( 1, 1, 2 )));

    ScRange aRange2( ScAddress( 0, 0, 0 ), ScAddress( 10, 10, 10 ));
    ScRange aRange3( ScAddress( 5, 5, 5 ), ScAddress( 15, 15, 15 ));
    CPPUNIT_ASSERT(!aRange2.Contains( aRange3 ));
    CPPUNIT_ASSERT(!aRange3.Contains( aRange2 ));
    CPPUNIT_ASSERT(aRange2.Intersects( aRange3 ));
    CPPUNIT_ASSERT(aRange3.Intersects( aRange2 ));
    CPPUNIT_ASSERT(!aRange3.Intersects( aRange1 ));
    CPPUNIT_ASSERT(!aRange1.Intersects( aRange3 ));
}

void ScRangeTest::testRangeParsing()
{
    ScRange aRange;
    ScRefFlags nRes = aRange.Parse(":1", *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));
}

void ScRangeTest::testAddressParsing()
{
    ScAddress aAddr;
    ScRefFlags nRes = aAddr.Parse("1", *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));
}

void ScRangeTest::testTdf147451()
{
    ScAddress aAddr;
    // "Sheet1" is technically a valid address like "XF1", but it should overflow.
    ScRefFlags nRes = aAddr.Parse("Sheet1", *m_pDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));
}

class ScRangeUpdaterTest : public CppUnit::TestFixture
{
public:

    virtual void setUp() override
    {
        utl::ConfigManager::EnableFuzzing();
        ScDLL::Init();
        ScGlobal::Init();
    }
    void testUpdateInsertTabBeforePos();
    void testUpdateInsertTabAtPos();
    void testUpdateInsertTabAfterPos();
    void testUpdateDeleteTabBeforePos();
    void testUpdateDeleteTabAtPos();
    void testUpdateDeleteTabAfterPos();

    CPPUNIT_TEST_SUITE(ScRangeUpdaterTest);
    CPPUNIT_TEST(testUpdateInsertTabBeforePos);
    CPPUNIT_TEST(testUpdateInsertTabAtPos);
    CPPUNIT_TEST(testUpdateInsertTabAfterPos);
    CPPUNIT_TEST(testUpdateDeleteTabBeforePos);
    CPPUNIT_TEST(testUpdateDeleteTabAtPos);
    CPPUNIT_TEST(testUpdateDeleteTabAfterPos);
    CPPUNIT_TEST_SUITE_END();
};

void ScRangeUpdaterTest::testUpdateInsertTabBeforePos()
{
    ScDocument aDoc;
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateInsertTabContext aContext(aDoc, 0, 1);
    ScRangeUpdater::UpdateInsertTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 2), aAddr);
}

void ScRangeUpdaterTest::testUpdateInsertTabAtPos()
{
    ScDocument aDoc;
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateInsertTabContext aContext(aDoc, 1, 1);
    ScRangeUpdater::UpdateInsertTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 2), aAddr);
}

void ScRangeUpdaterTest::testUpdateInsertTabAfterPos()
{
    ScDocument aDoc;
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateInsertTabContext aContext(aDoc, 2, 1);
    ScRangeUpdater::UpdateInsertTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 1), aAddr);
}

void ScRangeUpdaterTest::testUpdateDeleteTabBeforePos()
{
    ScDocument aDoc;
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateDeleteTabContext aContext(aDoc, 0, 1);
    ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 0), aAddr);
}

void ScRangeUpdaterTest::testUpdateDeleteTabAtPos()
{
    ScDocument aDoc;

    // Position within deleted range is moved to the front.
    {
        ScAddress aAddr(1, 1, 1);
        sc::RefUpdateDeleteTabContext aContext(aDoc, 1, 1);
        ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
        CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 0), aAddr);
    }
    {
        ScAddress aAddr(1, 1, 2);
        sc::RefUpdateDeleteTabContext aContext(aDoc, 1, 2);
        ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
        CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 0), aAddr);
    }

    // Would-be negative results are clamped to 0.
    {
        ScAddress aAddr(1, 1, 0);
        sc::RefUpdateDeleteTabContext aContext(aDoc, 0, 1);
        ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
        CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 0), aAddr);
    }
    {
        ScAddress aAddr(1, 1, 1);
        sc::RefUpdateDeleteTabContext aContext(aDoc, 0, 2);
        ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
        CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 0), aAddr);
    }
}

void ScRangeUpdaterTest::testUpdateDeleteTabAfterPos()
{
    ScDocument aDoc;
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateDeleteTabContext aContext(aDoc, 2, 1);
    ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 1), aAddr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScRangeTest);
CPPUNIT_TEST_SUITE_REGISTRATION(ScRangeUpdaterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
