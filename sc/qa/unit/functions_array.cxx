#include "functions_test.hxx"

class ArrayFunctionsTest : public FunctionsTest
{
public:
    ArrayFunctionsTest();

    void testArrayFormulasFODS();

    CPPUNIT_TEST_SUITE(ArrayFunctionsTest);
    CPPUNIT_TEST(testArrayFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void ArrayFunctionsTest::testArrayFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/array/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

ArrayFunctionsTest::ArrayFunctionsTest():
    FunctionsTest("sc/qa/unit/data/functions/array/fods/")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ArrayFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
