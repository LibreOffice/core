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
    ScDocument *m_pDoc;
    ScDocShellRef m_xDocShRef;
};

void ScAddressTest::testAddressParsing()
{
    ScAddress aAddr;
    ScRefFlags nRes = aAddr.Parse("1", m_pDoc, formula::FormulaGrammar::CONV_OOO);
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

    m_pDoc = &m_xDocShRef->GetDocument();
}

void ScAddressTest::tearDown()
{
    m_xDocShRef.Clear();
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
    ScDocument *m_pDoc;
    ScDocShellRef m_xDocShRef;
};

void ScRangeTest::testRangeParsing()
{
    ScRange aRange;
    ScRefFlags nRes = aRange.Parse(":1", m_pDoc, formula::FormulaGrammar::CONV_OOO);
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

    m_pDoc = &m_xDocShRef->GetDocument();
}

void ScRangeTest::tearDown()
{
    m_xDocShRef.Clear();
    BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAddressTest);
CPPUNIT_TEST_SUITE_REGISTRATION(ScRangeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
