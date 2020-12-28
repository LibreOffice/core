#include "functions_test.hxx"

class FinancialFunctionsTest : public FunctionsTest
{
public:
    FinancialFunctionsTest();

    void testFinancialFormulasFODS();

    CPPUNIT_TEST_SUITE(FinancialFunctionsTest);
    CPPUNIT_TEST(testFinancialFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void FinancialFunctionsTest::testFinancialFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/financial/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

FinancialFunctionsTest::FinancialFunctionsTest():
    FunctionsTest("sc/qa/unit/data/functions/financial/fods/")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(FinancialFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
