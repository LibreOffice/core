#include "functions_test.hxx"

class LogicalFunctionsTest : public FunctionsTest
{
public:
    LogicalFunctionsTest();

    void testLogicalFormulasFODS();

    CPPUNIT_TEST_SUITE(LogicalFunctionsTest);
    CPPUNIT_TEST(testLogicalFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void LogicalFunctionsTest::testLogicalFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/logical/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

LogicalFunctionsTest::LogicalFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(LogicalFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
