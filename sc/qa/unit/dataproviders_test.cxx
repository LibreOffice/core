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
#include <vcl/scheduler.hxx>
#include <orcusxml.hxx>
#include <sortparam.hxx>
#include <datatransformation.hxx>

#include <memory>

using namespace ::com::sun::star;

class ScDataProvidersTest : public ScModelTestBase
{
public:
    ScDataProvidersTest();

    void testCSVImport();
    void testDataLargerThanDB();
    void testHTMLImport();
    void testXMLImport();
    void testBaseImport();
    void testTdf169541_TwoDataMapping();
    void testTdf169610_SortTransform();

    CPPUNIT_TEST_SUITE(ScDataProvidersTest);
    CPPUNIT_TEST(testCSVImport);
    CPPUNIT_TEST(testDataLargerThanDB);
    CPPUNIT_TEST(testHTMLImport);
    CPPUNIT_TEST(testXMLImport);
    CPPUNIT_TEST(testBaseImport);
    CPPUNIT_TEST(testTdf169541_TwoDataMapping);
    CPPUNIT_TEST(testTdf169610_SortTransform);
    CPPUNIT_TEST_SUITE_END();
};

void ScDataProvidersTest::testCSVImport()
{
    createScDoc();

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 0, 0, 10, 10);
    ScDocument* pDoc = getScDoc();
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL = createFileURL(u"csv/test1.csv");
    sc::ExternalDataSource aDataSource(aFileURL, u"org.libreoffice.calc.csv"_ustr, pDoc);
    aDataSource.setDBData(pDBData->GetName());

    pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"test2"_ustr, pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"test3"_ustr, pDoc->GetString(2, 1, 0));
}

void ScDataProvidersTest::testDataLargerThanDB()
{
    createScDoc();

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 0, 0, 1, 1);
    ScDocument* pDoc = getScDoc();
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL = createFileURL(u"csv/test1.csv");
    sc::ExternalDataSource aDataSource(aFileURL, u"org.libreoffice.calc.csv"_ustr, pDoc);
    aDataSource.setDBData(pDBData->GetName());

    pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"test2"_ustr, pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(2, 1, 0));
}

void ScDataProvidersTest::testHTMLImport()
{
    createScDoc();

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 0, 0, 10, 10);
    ScDocument* pDoc = getScDoc();
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    OUString aFileURL = createFileURL(u"html/test1.html");
    sc::ExternalDataSource aDataSource(aFileURL, u"org.libreoffice.calc.html"_ustr, pDoc);
    aDataSource.setID(u"//table"_ustr);
    aDataSource.setDBData(pDBData->GetName());

    pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(pDoc, true);
    Scheduler::ProcessEventsToIdle();

    std::vector<OUString> aCarManufacturers
        = { u"Audi"_ustr, u"GM"_ustr, u"Nissan"_ustr, u"Ferrari"_ustr, u"Peugeot"_ustr };
    std::vector<OUString> aCities
        = { u"Berlin"_ustr, u"San Francisco"_ustr, u"Tokyo"_ustr, u"Rome"_ustr, u"Paris"_ustr };
    std::vector<double> aFirstCol = { 1, 10, -100, -0.11111, 1 };
    std::vector<double> aSecondCol = {
        2, 2.1, 40179, 2, 2,
    }; // 40179 is equal to 2010-1-1

    CPPUNIT_ASSERT_EQUAL(u"Col1"_ustr, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Col2"_ustr, pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Col3"_ustr, pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Col4"_ustr, pDoc->GetString(3, 0, 0));

    for (SCROW nRow = 0; nRow <= 4; ++nRow)
    {
        ASSERT_DOUBLES_EQUAL(aFirstCol[nRow], pDoc->GetValue(0, nRow + 1, 0));
        ASSERT_DOUBLES_EQUAL(aSecondCol[nRow], pDoc->GetValue(1, nRow + 1, 0));
        CPPUNIT_ASSERT_EQUAL(aCarManufacturers[nRow], pDoc->GetString(2, nRow + 1, 0));
        CPPUNIT_ASSERT_EQUAL(aCities[nRow], pDoc->GetString(3, nRow + 1, 0));
    }
}

void ScDataProvidersTest::testXMLImport()
{
    createScDoc();

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 0, 0, 10, 10);
    ScDocument* pDoc = getScDoc();
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    ScOrcusImportXMLParam aParam;

    ScOrcusImportXMLParam::RangeLink aRangeLink;
    aRangeLink.maPos = ScAddress(0, 0, 0);
    aRangeLink.maFieldPaths.push_back("/bookstore/book/title"_ostr);
    aRangeLink.maFieldPaths.push_back("/bookstore/book/author"_ostr);
    aRangeLink.maRowGroups.push_back("/bookstore/book"_ostr);
    aParam.maRangeLinks.push_back(aRangeLink);

    OUString aFileURL = createFileURL(u"xml/test1.xml");
    sc::ExternalDataSource aDataSource(aFileURL, u"org.libreoffice.calc.xml"_ustr, pDoc);
    aDataSource.setDBData(u"testDB"_ustr);
    aDataSource.setXMLImportParam(aParam);

    pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"title"_ustr, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"author"_ustr, pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"test2"_ustr, pDoc->GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(0, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"test3"_ustr, pDoc->GetString(1, 3, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(0, 4, 0));
    CPPUNIT_ASSERT_EQUAL(u"test4"_ustr, pDoc->GetString(1, 4, 0));
}

void ScDataProvidersTest::testBaseImport()
{
    createScDoc();

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 10, 10);
    ScDocument* pDoc = getScDoc();
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    sc::ExternalDataSource aDataSource("~/dummy.file", "org.libreoffice.calc.sql", pDoc);
    aDataSource.setDBData("testDB");
    aDataSource.setID("biblio@Bibliography");

    pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    rDataSources[0].refresh(pDoc, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("ARJ00"), pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("AVV00"), pDoc->GetString(0, 1, 0));
}

void ScDataProvidersTest::testTdf169541_TwoDataMapping()
{
    // tdf#169541 Crash in: Data Provider: import 2x + save = crash on open file
    // Without fix, loading had crashed after initial creating and saving
    createScDoc("tdf169541_two_data_mapping.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);
    // Because this test is only about the crash, the paths in the test document are
    // not adapted to the test environment and thus no further tests about the content here.
}

void ScDataProvidersTest::testTdf169610_SortTransform()
{
    //Create Document
    createScDoc();

    // Create database range. Size arbitrary, but larger as data, here range C4:K16.
    // rName, nTab, nCol1, nRow1, nCol2, nRow2
    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 2, 3, 10, 15);
    pDBData->SetHeader(true);
    ScDocument* pDoc = getScDoc();
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    // Create data mapping directly without dialog
    OUString aFileURL = createFileURL(u"csv/tdf169610_SortTransform.csv");
    sc::ExternalDataSource aDataSource(aFileURL, u"org.libreoffice.calc.csv"_ustr, pDoc);
    aDataSource.setDBData(pDBData->GetName());

    // range address is relative in SortTransform of Data Provider
    ScSortParam aSortParam;
    // ctor includes Clear(). That sets alreay 0 for nCol1, nRow1 and nSourceTab
    aSortParam.nCol2 = 8;
    aSortParam.nRow2 = 12;
    aSortParam.bHasHeader = true;
    aSortParam.maKeyState[0].bDoSort = true;
    aSortParam.maKeyState[0].nField = 1;
    aSortParam.maKeyState[0].bAscending = true;
    aDataSource.AddDataTransformation(std::make_shared<sc::SortTransformation>(aSortParam));

    pDoc->GetExternalDataMapper().insertDataSource(aDataSource);
    auto& rDataSources = pDoc->GetExternalDataMapper().getDataSources();
    CPPUNIT_ASSERT(!rDataSources.empty());

    // actually import the data. The data will cover the range C4:F12.
    rDataSources[0].refresh(pDoc, true);
    Scheduler::ProcessEventsToIdle();

    // Examine some data
    // Header Row with ID, Name, Region, Sales
    CPPUNIT_ASSERT_EQUAL(u"ID"_ustr, pDoc->GetString(2, 3, 0)); // col, row, tab
    CPPUNIT_ASSERT_EQUAL(u"Sales"_ustr, pDoc->GetString(5, 3, 0));
    // First data row
    CPPUNIT_ASSERT_EQUAL(u"Ali"_ustr, pDoc->GetString(3, 4, 0));
    CPPUNIT_ASSERT_EQUAL(9.0, pDoc->GetValue(5, 4, 0));
    // Last data row
    CPPUNIT_ASSERT_EQUAL(u"Tom"_ustr, pDoc->GetString(3, 11, 0));
    CPPUNIT_ASSERT_EQUAL(14.0, pDoc->GetValue(5, 11, 0));

    // Save document and examine markup. Without fix the sort transformation
    // was not written to file
    skipValidation(); // ToDo: tdf#169669 no schema yet for calcext:data-mappings
    saveAndReload(TestFilter::ODS);
    pDoc = getScDoc();

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    static constexpr OString sDataMappingPath = "//calcext:data-mapping"_ostr;
    assertXPath(pXmlDoc, sDataMappingPath, 1);
    const OString sTransformationPath = sDataMappingPath
                                        + "/calcext:data-transformations"
                                          "/calcext:column-sort-transformation";
    assertXPath(pXmlDoc, sTransformationPath, 1);
    assertXPath(pXmlDoc, sTransformationPath + "/table:sort", 1);
    const OString sSortByPath = sTransformationPath + "/table:sort/table:sort-by";
    assertXPath(pXmlDoc, sSortByPath, 1);
    assertXPath(pXmlDoc, sSortByPath + "[@table:field-number='1']");

    // Examine data again in reloaded document.
    // Without fix the data were unsorted after reload.
    CPPUNIT_ASSERT_EQUAL(u"Ali"_ustr, pDoc->GetString(3, 4, 0));
    CPPUNIT_ASSERT_EQUAL(9.0, pDoc->GetValue(5, 4, 0));
    CPPUNIT_ASSERT_EQUAL(u"Tom"_ustr, pDoc->GetString(3, 11, 0));
    CPPUNIT_ASSERT_EQUAL(14.0, pDoc->GetValue(5, 11, 0));
}

ScDataProvidersTest::ScDataProvidersTest()
    : ScModelTestBase(u"sc/qa/unit/data/dataprovider"_ustr)
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataProvidersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
