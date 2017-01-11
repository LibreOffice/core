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
    OUString aUrl = "file:///home/karan/stocks.csv";
    OUString aDBName = "TEST";
    sc::ExternalDataMapper aExternalDataMapper (&getDocShell(), aUrl, aDBName, 0, 0, 0, 2, 2, success);
    aExternalDataMapper.StartImport();
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
    BootstrapFixture::tearDown();
}


CPPUNIT_TEST_SUITE_REGISTRATION(ScDataProvidersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
