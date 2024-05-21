#include "functions_test.hxx"

class StatisticalFunctionsTest : public FunctionsTest
{
public:
    StatisticalFunctionsTest();

    void testStatisticalFormulasFODS();

    CPPUNIT_TEST_SUITE(StatisticalFunctionsTest);
    CPPUNIT_TEST(testStatisticalFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void StatisticalFunctionsTest::testStatisticalFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/statistical/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

StatisticalFunctionsTest::StatisticalFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(StatisticalFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
