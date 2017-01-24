/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include <rtl/ustring.hxx>

#include "dataproviders_test.hxx"
#include "address.hxx"

struct TestImpl
{
    ScDocShellRef m_xDocShell;
};

ScDataProvidersTest::ScDataProvidersTest() :
    ScBootstrapFixture( "/sc/qa/unit/data" ),
    m_pImpl(new TestImpl),
    m_pDoc(nullptr)
{
}

ScDataProvidersTest::~ScDataProvidersTest()
{
}

ScDocShell& ScDataProvidersTest::getDocShell()
{
    return *m_pImpl->m_xDocShell;
}

void ScDataProvidersTest::testCSVImport()
{
    m_pDoc->InsertTab(0, "foo");
    bool success;
    OUString aCSVFile("dataprovider.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    OUString aDBName = "TEST";
    sc::ExternalDataMapper aExternalDataMapper (&getDocShell(), aCSVPath, aDBName, 0, 0, 0, 5, 5, success);
    aExternalDataMapper.StartImport();
    Scheduler::ProcessEventsToIdle();
    aExternalDataMapper.StopImport();
    CPPUNIT_ASSERT_EQUAL (-2012.0, m_pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL (-1.0, m_pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL (0.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL (1.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL (2012.0, m_pDoc->GetValue(4, 0, 0));
    CPPUNIT_ASSERT_EQUAL (-3.14, m_pDoc->GetValue(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL (-0.99, m_pDoc->GetValue(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL (0.01, m_pDoc->GetValue(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL (3.14, m_pDoc->GetValue(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL (OUString("H"), m_pDoc->GetString(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL (OUString("Hello, Calc!"), m_pDoc->GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL (0.0, m_pDoc->GetValue(2, 2, 0));
    CPPUNIT_ASSERT_EQUAL (0.0, m_pDoc->GetValue(0, 3, 0));
}

void ScDataProvidersTest::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();
    m_pImpl->m_xDocShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    m_pImpl->m_xDocShell->SetIsInUcalc();
    m_pImpl->m_xDocShell->DoInitUnitTest();
    m_pDoc = &m_pImpl->m_xDocShell->GetDocument();
}

void ScDataProvidersTest::tearDown()
{
    m_pImpl->m_xDocShell->DoClose();
    m_pImpl->m_xDocShell.Clear();
    ScBootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataProvidersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
