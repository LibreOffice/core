#include "functions_test.hxx"

class DateTimeFunctionsTest : public FunctionsTest
{
public:
    DateTimeFunctionsTest();

    void testDateTimeFormulasFODS();

    CPPUNIT_TEST_SUITE(DateTimeFunctionsTest);
    CPPUNIT_TEST(testDateTimeFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void DateTimeFunctionsTest::testDateTimeFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/date_time/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

DateTimeFunctionsTest::DateTimeFunctionsTest():
    FunctionsTest("sc/qa/unit/data/functions/date_time/fods/")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(DateTimeFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
