#include "functions_test.hxx"

class InformationFunctionsTest : public FunctionsTest
{
public:
    InformationFunctionsTest();

    void testInformationFormulasFODS();

    CPPUNIT_TEST_SUITE(InformationFunctionsTest);
    CPPUNIT_TEST(testInformationFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void InformationFunctionsTest::testInformationFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/information/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

InformationFunctionsTest::InformationFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(InformationFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
