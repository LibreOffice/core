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
#include <address.hxx>
#include <dbdata.hxx>
#include <datamapper.hxx>
#include <scdll.hxx>
#include <vcl/scheduler.hxx>
#include <orcusxml.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScDataProvidersTest : public ScBootstrapFixture
{
public:
    ScDataProvidersTest();

    virtual void setUp() override;

    void testCSVImport();
    void testDataLargerThanDB();
    void testHTMLImport();
    void testXMLImport();
    // void testBaseImport();

    CPPUNIT_TEST_SUITE(ScDataProvidersTest);
    CPPUNIT_TEST(testCSVImport);
    CPPUNIT_TEST(testDataLargerThanDB);
    CPPUNIT_TEST(testHTMLImport);
    CPPUNIT_TEST(testXMLImport);
    // CPPUNIT_TEST(testBaseImport);
    CPPUNIT_TEST_SUITE_END();
};

void ScDataProvidersTest::testCSVImport()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    ScDocument& rDoc = xDocSh->GetDocument();
    bool bInserted
        = rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    createFileURL(u"test1.", u"csv", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.csv", &rDoc);
    aDataSource.setDBData(pDBData->GetName());

    rDoc.GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = rDoc.GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(&rDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), rDoc.GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), rDoc.GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test3"), rDoc.GetString(2, 1, 0));

    xDocSh->DoClose();
}

void ScDataProvidersTest::testDataLargerThanDB()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 1, 1);
    ScDocument& rDoc = xDocSh->GetDocument();
    bool bInserted
        = rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    createFileURL(u"test1.", u"csv", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.csv", &rDoc);
    aDataSource.setDBData(pDBData->GetName());

    rDoc.GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = rDoc.GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(&rDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), rDoc.GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), rDoc.GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(2, 1, 0));

    xDocSh->DoClose();
}

void ScDataProvidersTest::testHTMLImport()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    ScDocument& rDoc = xDocSh->GetDocument();
    bool bInserted
        = rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    createFileURL(u"test1.", u"html", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.html", &rDoc);
    aDataSource.setID("//table");
    aDataSource.setDBData(pDBData->GetName());

    rDoc.GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = rDoc.GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(&rDoc, true);
    Scheduler::ProcessEventsToIdle();

    std::vector<OUString> aCarManufacturers = { "Audi", "GM", "Nissan", "Ferrari", "Peugeot" };
    std::vector<OUString> aCities = { "Berlin", "San Francisco", "Tokyo", "Rome", "Paris" };
    std::vector<double> aFirstCol = { 1, 10, -100, -0.11111, 1 };
    std::vector<double> aSecondCol = {
        2, 2.1, 40179, 2, 2,
    }; // 40179 is equal to 2010-1-1

    CPPUNIT_ASSERT_EQUAL(OUString("Col1"), rDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Col2"), rDoc.GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Col3"), rDoc.GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Col4"), rDoc.GetString(3, 0, 0));

    for (SCROW nRow = 0; nRow <= 4; ++nRow)
    {
        ASSERT_DOUBLES_EQUAL(aFirstCol[nRow], rDoc.GetValue(0, nRow + 1, 0));
        ASSERT_DOUBLES_EQUAL(aSecondCol[nRow], rDoc.GetValue(1, nRow + 1, 0));
        CPPUNIT_ASSERT_EQUAL(aCarManufacturers[nRow], rDoc.GetString(2, nRow + 1, 0));
        CPPUNIT_ASSERT_EQUAL(aCities[nRow], rDoc.GetString(3, nRow + 1, 0));
    }

    xDocSh->DoClose();
}

void ScDataProvidersTest::testXMLImport()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    ScDocument& rDoc = xDocSh->GetDocument();
    bool bInserted
        = rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL;
    ScOrcusImportXMLParam aParam;

    ScOrcusImportXMLParam::RangeLink aRangeLink;
    aRangeLink.maPos = ScAddress(0, 0, 0);
    aRangeLink.maFieldPaths.push_back("/bookstore/book/title");
    aRangeLink.maFieldPaths.push_back("/bookstore/book/author");
    aRangeLink.maRowGroups.push_back("/bookstore/book");
    aParam.maRangeLinks.push_back(aRangeLink);

    createFileURL(u"test1.", u"xml", aFileURL);
    sc::ExternalDataSource aDataSource(aFileURL, "org.libreoffice.calc.xml", &rDoc);
    aDataSource.setDBData("testDB");
    aDataSource.setXMLImportParam(aParam);

    rDoc.GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = rDoc.GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(&rDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("title"), rDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("author"), rDoc.GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), rDoc.GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), rDoc.GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(0, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test3"), rDoc.GetString(1, 3, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(0, 4, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("test4"), rDoc.GetString(1, 4, 0));

    xDocSh->DoClose();
}

/*
void ScDataProvidersTest::testBaseImport()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    ScDocument& rDoc = xDocSh->GetDocument();
    bool bInserted = rDoc.GetDBCollection()->getNamedDBs().insert(pDBData);
    CPPUNIT_ASSERT(bInserted);

    sc::ExternalDataSource aDataSource("~/dummy.file", "org.libreoffice.calc.sql", &rDoc);
    aDataSource.setDBData("testDB");
    aDataSource.setID("biblio@Bibliography");


    rDoc.GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = rDoc.GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(&rDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("ARJ00"), rDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("AVV00"), rDoc.GetString(1, 1, 0));

    xDocSh->DoClose();
}
*/

ScDataProvidersTest::ScDataProvidersTest()
    : ScBootstrapFixture("sc/qa/unit/data/dataprovider")
{
}

void ScDataProvidersTest::setUp()
{
    ScBootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataProvidersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
