#include "functions_test.hxx"

class SpreadsheetFunctionsTest : public FunctionsTest
{
public:
    SpreadsheetFunctionsTest();

    void testSpreadsheetFormulasFODS();

    CPPUNIT_TEST_SUITE(SpreadsheetFunctionsTest);
    CPPUNIT_TEST(testSpreadsheetFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void SpreadsheetFunctionsTest::testSpreadsheetFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/spreadsheet/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

SpreadsheetFunctionsTest::SpreadsheetFunctionsTest():
    FunctionsTest("sc/qa/unit/data/functions/spreadsheet/fods/")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(SpreadsheetFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
