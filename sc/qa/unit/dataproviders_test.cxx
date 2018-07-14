/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#include "helper/qahelper.hxx"
#include <document.hxx>
#include <stringutil.hxx>
#include <address.hxx>
#include <dataprovider.hxx>
#include <vcl/scheduler.hxx>
#include <orcusxml.hxx>

#include <memory>

class ScDataProvidersTest : public ScBootstrapFixture
{
public:

    ScDataProvidersTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCSVImport();
    void testDataLargerThanDB();
    void testHTMLImport();
    void testXMLImport();
    void testBaseImport();

    CPPUNIT_TEST_SUITE(ScDataProvidersTest);
    CPPUNIT_TEST(testCSVImport);
    CPPUNIT_TEST(testDataLargerThanDB);
    CPPUNIT_TEST(testHTMLImport);
    CPPUNIT_TEST(testXMLImport);
    CPPUNIT_TEST(testBaseImport);
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
    aDataSource.setDBData(pDBData->GetName());


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

void ScDataProvidersTest::testDataLargerThanDB()
{
    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 1, 1);
    bool bInserted = m_pDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    createFileURL("test1.", "csv", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.csv", m_pDoc);
    aDataSource.setDBData(pDBData->GetName());


    m_pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = m_pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(m_pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), m_pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), m_pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(), m_pDoc->GetString(2, 1, 0));
}

void ScDataProvidersTest::testHTMLImport()
{
    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    bool bInserted = m_pDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    createFileURL("test1.", "html", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.html", m_pDoc);
    aDataSource.setID("//table");
    aDataSource.setDBData(pDBData->GetName());


    m_pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = m_pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(m_pDoc, true);
    Scheduler::ProcessEventsToIdle();


    std::vector<OUString> aCarManufacturers = {"Audi", "GM", "Nissan", "Ferrari", "Peugeot"};
    std::vector<OUString> aCities = {"Berlin", "San Francisco", "Tokyo", "Rome", "Paris"};
    std::vector<double> aFirstCol = {1, 10, -100, -0.11111, 1};
    std::vector<double> aSecondCol = {2, 2.1, 40179, 2, 2,}; // 40179 is equal to 2010-1-1

    CPPUNIT_ASSERT_EQUAL(OUString("Col1"), m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Col2"), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Col3"), m_pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Col4"), m_pDoc->GetString(3, 0, 0));

    for (SCROW nRow = 0; nRow <= 4; ++nRow)
    {
        ASSERT_DOUBLES_EQUAL(aFirstCol[nRow], m_pDoc->GetValue(0, nRow + 1, 0));
        ASSERT_DOUBLES_EQUAL(aSecondCol[nRow], m_pDoc->GetValue(1, nRow + 1, 0));
        CPPUNIT_ASSERT_EQUAL(aCarManufacturers[nRow], m_pDoc->GetString(2, nRow + 1, 0));
        CPPUNIT_ASSERT_EQUAL(aCities[nRow], m_pDoc->GetString(3, nRow + 1, 0));
    }
}

void ScDataProvidersTest::testXMLImport()
{
    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    bool bInserted = m_pDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    ScOrcusImportXMLParam aParam;

    ScOrcusImportXMLParam::RangeLink aRangeLink;
    aRangeLink.maPos = ScAddress(0,0,0);
    aRangeLink.maFieldPaths.push_back("/bookstore/book/title");
    aRangeLink.maFieldPaths.push_back("/bookstore/book/author");
    aParam.maRangeLinks.push_back(aRangeLink);

    createFileURL("test1.", "xml", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.xml", m_pDoc);
    aDataSource.setDBData("testDB");
    aDataSource.setXMLImportParam(aParam);


    m_pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = m_pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(m_pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("title"), m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("author"), m_pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), m_pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), m_pDoc->GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(0, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test3"), m_pDoc->GetString(1, 3, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(0, 4, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test4"), m_pDoc->GetString(1, 4, 0));
}

void ScDataProvidersTest::testBaseImport()
{
    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    bool bInserted = m_pDoc->GetDBCollection()->getNamedDBs().insert(pDBData);
    CPPUNIT_ASSERT(bInserted);

    sc::ExternalDataSource aDataSource("~/dummy.file", "org.libreoffice.calc.sql", m_pDoc);
    aDataSource.setDBData("testDB");
    aDataSource.setID("biblio@Bibliography");


    m_pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = m_pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(m_pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("ARJ00"), m_pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("AVV00"), m_pDoc->GetString(1, 1, 0));
}

ScDataProvidersTest::ScDataProvidersTest() :
    ScBootstrapFixture( "sc/qa/unit/data/dataprovider" ),
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
