#include "functions_test.hxx"
#include <arraysumfunctor.hxx>

class StatisticalFunctionsTest : public FunctionsTest
{
public:
    StatisticalFunctionsTest();

    void testStatisticalFormulasFODS();
    void testIntrinsicSums();

    CPPUNIT_TEST_SUITE(StatisticalFunctionsTest);
    CPPUNIT_TEST(testStatisticalFormulasFODS);
    CPPUNIT_TEST(testIntrinsicSums);
    CPPUNIT_TEST_SUITE_END();

};

void StatisticalFunctionsTest::testStatisticalFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/statistical/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

StatisticalFunctionsTest::StatisticalFunctionsTest():
    FunctionsTest("sc/qa/unit/data/functions/statistical/fods/")
{
}

void StatisticalFunctionsTest::testIntrinsicSums()
{
    // Checkout SSE2, AVX and AVX512 opperations
        // Needs exactly 9 terms
    double summands[9] = { 0, 1, 2, 3, 4, 10, 20, 2, -1 };
    double* pCurrent = summands;
    size_t i = 0;
    if (sc::op::hasAVX512F)
        CPPUNIT_ASSERT_EQUAL(42.0, sc::op::executeAVX512F(i, 9, pCurrent).get());
    i = 0;
    if (sc::op::hasAVX)
        CPPUNIT_ASSERT_EQUAL(42.0, sc::op::executeAVX(i, 9, pCurrent).get());
    i = 0;
    if (sc::op::hasSSE2)
        CPPUNIT_ASSERT_EQUAL(42.0, sc::op::executeSSE2(i, 9, pCurrent).get());
    i = 0;
    CPPUNIT_ASSERT_EQUAL(42.0, sc::op::executeUnrolled(i, 9, pCurrent).get());
}

CPPUNIT_TEST_SUITE_REGISTRATION(StatisticalFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
