#include "functions_test.hxx"

class AddinFunctionsTest : public FunctionsTest
{
public:
    AddinFunctionsTest();

    void testAddinFormulasFODS();

    CPPUNIT_TEST_SUITE(AddinFunctionsTest);
    CPPUNIT_TEST(testAddinFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void AddinFunctionsTest::testAddinFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/addin/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

AddinFunctionsTest::AddinFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(AddinFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
