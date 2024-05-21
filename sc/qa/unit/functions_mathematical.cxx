#include "functions_test.hxx"

class MathematicalFunctionsTest : public FunctionsTest
{
public:
    MathematicalFunctionsTest();

    void testMathematicalFormulasFODS();

    CPPUNIT_TEST_SUITE(MathematicalFunctionsTest);
    CPPUNIT_TEST(testMathematicalFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void MathematicalFunctionsTest::testMathematicalFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/mathematical/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

MathematicalFunctionsTest::MathematicalFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(MathematicalFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
