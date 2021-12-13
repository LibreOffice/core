#include "functions_test.hxx"

class ArrayFunctionsTest : public FunctionsTest
{
public:
    ArrayFunctionsTest();

    void testArrayFormulasFODS();
    void testDubiousArrayFormulasFODS();

    CPPUNIT_TEST_SUITE(ArrayFunctionsTest);
    CPPUNIT_TEST(testArrayFormulasFODS);
    CPPUNIT_TEST(testDubiousArrayFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void ArrayFunctionsTest::testArrayFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/array/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

void ArrayFunctionsTest::testDubiousArrayFormulasFODS()
{
    //TODO: sc/qa/unit/data/functions/array/dubious/fods/linest.fods produces widely different
    // values when built with -ffp-contract enabled (-ffp-contract=on default on Clang 14,
    // -ffp-contract=fast default when building with optimizations on GCC) on at least aarch64:
#if !((defined __clang__ || defined __GNUC__) && defined __aarch64__)
    OUString aDirectoryURL
        = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/array/dubious/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
#endif
}

ArrayFunctionsTest::ArrayFunctionsTest():
    FunctionsTest("sc/qa/unit/data/functions/array/fods/")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ArrayFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
