#include "functions_test.hxx"
#include <arraysumfunctor.hxx>
#include <ctime>
#include <cmath>

class StatisticalFunctionsTest : public FunctionsTest
{
public:
    StatisticalFunctionsTest();

    void testStatisticalFormulasFODS();
    void testIntrinsicSums();
    void testIntrinsicSumsPerformance();

    CPPUNIT_TEST_SUITE(StatisticalFunctionsTest);
    CPPUNIT_TEST(testStatisticalFormulasFODS);
    CPPUNIT_TEST(testIntrinsicSums);
    CPPUNIT_TEST(testIntrinsicSumsPerformance);
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

void StatisticalFunctionsTest::testIntrinsicSumsPerformance()
{
    // Checkout SSE2, AVX and AVX512 opperations
    // Needs exactly 9 terms
    constexpr size_t nSize = 1000000;
    double* pSummands = new double[nSize];
    for(size_t i = 0; i < nSize; ++i)
        pSummands[i] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);

    size_t i;
    clock_t nStart;
    clock_t nEnd;
    KahanSum aSum[4];
    clock_t aClocks[4] = {0,0,0,0};

    // Without any boost
    nStart = clock();
    i = 0;
    aSum[3] = sc::op::executeUnrolled(i, nSize, pSummands);
    i = 0;
    aSum[3] += sc::op::executeUnrolled(i, nSize, pSummands);
    i = 0;
    aSum[3] += sc::op::executeUnrolled(i, nSize, pSummands);
    nEnd = clock();
    aClocks[3] = nEnd - nStart;
    printf("\nClocks for sum with NONE: %f\n",static_cast<double>(nEnd-nStart) / CLOCKS_PER_SEC);

    if (sc::op::hasAVX512F)
    {
        nStart = clock();
        i = 0;
        aSum[0] = sc::op::executeAVX512F(i, nSize, pSummands);
        i = 0;
        aSum[0] += sc::op::executeAVX512F(i, nSize, pSummands);
        i = 0;
        aSum[0] += sc::op::executeAVX512F(i, nSize, pSummands);
        nEnd = clock();
        aClocks[0] = nEnd - nStart;
        printf("\nClocks for sum with AVX512: %f\n",static_cast<double>(nEnd-nStart) / CLOCKS_PER_SEC);
        CPPUNIT_ASSERT_EQUAL(aSum[0].get(), aSum[3].get());
    }

    if (sc::op::hasAVX)
    {
        nStart = clock();
        i = 0;
        aSum[1] = sc::op::executeAVX(i, nSize, pSummands);
        i = 0;
        aSum[1] += sc::op::executeAVX(i, nSize, pSummands);
        i = 0;
        aSum[1] += sc::op::executeAVX(i, nSize, pSummands);
        nEnd = clock();
        aClocks[1] = nEnd - nStart;
        printf("\nClocks for sum with AVX: %f\n",static_cast<double>(nEnd-nStart) / CLOCKS_PER_SEC);
        CPPUNIT_ASSERT_EQUAL(aSum[1].get(), aSum[3].get());
    }

    // Original code
    if (sc::op::hasSSE2)
    {
        nStart = clock();
        i = 0;
        aSum[2] = sc::op::executeSSE2(i, nSize, pSummands);
        i = 0;
        aSum[2] += sc::op::executeSSE2(i, nSize, pSummands);
        i = 0;
        aSum[2] += sc::op::executeSSE2(i, nSize, pSummands);
        nEnd = clock();
        aClocks[2] = nEnd - nStart;
        printf("\nClocks for sum with SSE2: %f\n",static_cast<double>(nEnd-nStart) / CLOCKS_PER_SEC);
        CPPUNIT_ASSERT_EQUAL(aSum[2].get(), aSum[3].get());
    }

    CPPUNIT_ASSERT( aClocks[0] <= aClocks[1] );
    CPPUNIT_ASSERT( aClocks[1] <= aClocks[2] );
    CPPUNIT_ASSERT( aClocks[2] <= aClocks[3] );
}


CPPUNIT_TEST_SUITE_REGISTRATION(StatisticalFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
