#include "functions_test.hxx"

class TextFunctionsTest : public FunctionsTest
{
public:
    TextFunctionsTest();

    void testTextFormulasFODS();

    CPPUNIT_TEST_SUITE(TextFunctionsTest);
    CPPUNIT_TEST(testTextFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void TextFunctionsTest::testTextFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/text/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

TextFunctionsTest::TextFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(TextFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
