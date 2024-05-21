#include "functions_test.hxx"

class DatabaseFunctionsTest : public FunctionsTest
{
public:
    DatabaseFunctionsTest();

    void testDatabaseFormulasFODS();

    CPPUNIT_TEST_SUITE(DatabaseFunctionsTest);
    CPPUNIT_TEST(testDatabaseFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void DatabaseFunctionsTest::testDatabaseFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/database/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

DatabaseFunctionsTest::DatabaseFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
