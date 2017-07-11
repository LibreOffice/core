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
#include "helper/qahelper.hxx"
#include "document.hxx"
#include "docsh.hxx"

#include "address.hxx"

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
    ScRefFlags nRes = aAddr.Parse("1", &rDoc, formula::FormulaGrammar::CONV_OOO);
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
    CPPUNIT_TEST(testRangeParsingOOX);
    CPPUNIT_TEST_SUITE_END();

    void testRangeParsing();
    void testRangeParsingOOX();

private:
    ScDocShellRef m_xDocShRef;
};

void ScRangeTest::testRangeParsing()
{
    ScRange aRange;
    ScDocument& rDoc = m_xDocShRef->GetDocument();
    ScRefFlags nRes = aRange.Parse(":1", &rDoc, formula::FormulaGrammar::CONV_OOO);
    CPPUNIT_ASSERT_MESSAGE("Should fail to parse.", !(nRes & ScRefFlags::VALID));
}

void ScRangeTest::testRangeParsingOOX()
{
    ScRange aRange;
    ScDocument& rDoc = m_xDocShRef->GetDocument();
    ScRefFlags nRes = aRange.Parse("A2:BZZ2", &rDoc, formula::FormulaGrammar::CONV_XL_OOX);
    CPPUNIT_ASSERT_MESSAGE("Should truncate parse.", (nRes & ScRefFlags::VALID));
    CPPUNIT_ASSERT_MESSAGE("Range end col must be MAXCOL", aRange.aEnd.Col() == MAXCOL);

    nRes = aRange.Parse("BZZ2:CZZ2", &rDoc, formula::FormulaGrammar::CONV_XL_OOX);
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

CPPUNIT_TEST_SUITE_REGISTRATION(ScAddressTest);
CPPUNIT_TEST_SUITE_REGISTRATION(ScRangeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
