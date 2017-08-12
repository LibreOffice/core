/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#include "helper/qahelper.hxx"
#include "document.hxx"
#include <stringutil.hxx>
#include "address.hxx"
#include "dataprovider.hxx"
#include <vcl/scheduler.hxx>

#include <memory>

class ScDataProvidersTest : public ScBootstrapFixture
{
public:

    ScDataProvidersTest();

    ScDocShell& getDocShell();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCSVImport();

    CPPUNIT_TEST_SUITE(ScDataProvidersTest);
    CPPUNIT_TEST(testCSVImport);
    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef m_xDocShell;
    ScDocument *m_pDoc;
};

void ScDataProvidersTest::testCSVImport()
{
    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    bool bInserted = m_pDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    createFileURL("test1.", "csv", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.csv", m_pDoc);
    aDataSource.setDBData(pDBData);


    m_pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = m_pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(m_pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), m_pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), m_pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test3"), m_pDoc->GetString(2, 1, 0));
}

ScDataProvidersTest::ScDataProvidersTest() :
    ScBootstrapFixture( "/sc/qa/unit/data/dataprovider" ),
    m_pDoc(nullptr)
{
}

void ScDataProvidersTest::setUp()
{
    ScBootstrapFixture::setUp();

    ScDLL::Init();
    m_xDocShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    m_xDocShell->SetIsInUcalc();
    m_xDocShell->DoInitUnitTest();
    m_pDoc = &m_xDocShell->GetDocument();
    m_pDoc->InsertTab(0, "Tab");
}

void ScDataProvidersTest::tearDown()
{
    m_xDocShell->DoClose();
    m_xDocShell.clear();
    ScBootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataProvidersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
