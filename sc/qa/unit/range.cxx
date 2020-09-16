/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <unotools/configmgr.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <global.hxx>
#include <scdll.hxx>

#include <address.hxx>
#include <rangeutl.hxx>
#include <refupdatecontext.hxx>

class ScAddressTest : public test::BootstrapFixture
{
public:

    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScAddressTest);
    CPPUNIT_TEST(testAddressParsing);
    CPPUNIT_TEST_SUITE_END();

    void testAddressParsing();

private:
    ScDocShellRef m_xDocShRef;
};

void ScAddressTest::testAddressParsing()
{
    ScAddress aAddr;
    ScDocument& rDoc = m_xDocShRef->GetDocument();
    ScRefFlags nRes = aAddr.Parse("1", rDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));
}

void ScAddressTest::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShRef = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
}

void ScAddressTest::tearDown()
{
    m_xDocShRef->DoClose();
    m_xDocShRef.clear();
    BootstrapFixture::tearDown();
}

class ScRangeTest : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScRangeTest);
    CPPUNIT_TEST(testRangeParsing);
    CPPUNIT_TEST_SUITE_END();

    void testRangeParsing();

private:
    ScDocShellRef m_xDocShRef;
};

void ScRangeTest::testRangeParsing()
{
    ScRange aRange;
    ScDocument& rDoc = m_xDocShRef->GetDocument();
    ScRefFlags nRes = aRange.Parse(":1", rDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));
}

void ScRangeTest::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShRef = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
}

void ScRangeTest::tearDown()
{
    m_xDocShRef.clear();
    BootstrapFixture::tearDown();
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
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateDeleteTabContext aContext(aDoc, 1, 1);
    ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 0), aAddr);
}

void ScRangeUpdaterTest::testUpdateDeleteTabAfterPos()
{
    ScDocument aDoc;
    ScAddress aAddr(1, 1, 1);
    sc::RefUpdateDeleteTabContext aContext(aDoc, 2, 1);
    ScRangeUpdater::UpdateDeleteTab(aAddr, aContext);
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 1, 1), aAddr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAddressTest);
CPPUNIT_TEST_SUITE_REGISTRATION(ScRangeTest);
CPPUNIT_TEST_SUITE_REGISTRATION(ScRangeUpdaterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
