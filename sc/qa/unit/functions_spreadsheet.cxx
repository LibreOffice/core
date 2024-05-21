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
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

SpreadsheetFunctionsTest::SpreadsheetFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(SpreadsheetFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
