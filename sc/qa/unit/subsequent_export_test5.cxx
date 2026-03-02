/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Tests for JSON filter options in CSV import/export (convert-to API)

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <formulacell.hxx>

#include <svtools/sfxecode.hxx>
#include <svl/intitem.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

class ScCsvJsonFilterTest : public ScModelTestBase
{
public:
    ScCsvJsonFilterTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }

protected:
    OString readExportedFile()
    {
        SvMemoryStream aMemoryStream;
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        aStream.ReadStream(aMemoryStream);
        const char* pData = static_cast<const char*>(aMemoryStream.GetData());
        int offset = 0;
        // Skip BOM if present
        if (aMemoryStream.GetSize() > 2 && pData[0] == '\xEF' && pData[1] == '\xBB'
            && pData[2] == '\xBF')
            offset = 3;
        return OString(pData + offset, aMemoryStream.GetSize() - offset);
    }
};

// --- CSV Export Tests ---

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonFieldSeparator)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);
    pDoc->SetString(ScAddress(2, 0, 0), u"C"_ustr);

    setFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\"\\t\"}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    CPPUNIT_ASSERT_MESSAGE("Tab separator should be used", aData.indexOf("A\tB\tC") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonTextDelimiter)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    // Text containing the separator character (comma) requires quoting
    pDoc->SetString(ScAddress(0, 0, 0), u"hello,world"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"plain"_ustr);

    // Use single quote as text delimiter
    setFilterOptions(u"{\"TextDelimiter\":{\"type\":\"string\",\"value\":\"'\"},"
                     "\"QuoteAllText\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    // Fields should be quoted with single quotes
    CPPUNIT_ASSERT_MESSAGE("Single quote delimiter expected", aData.indexOf("'hello,world'") >= 0);
    CPPUNIT_ASSERT_MESSAGE("QuoteAllText should quote plain text too",
                           aData.indexOf("'plain'") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonSaveAsShown)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetValue(ScAddress(0, 0, 0), 1.23456789);

    // SaveAsShown=false means the full precision value is exported
    setFilterOptions(u"{\"SaveAsShown\":{\"type\":\"boolean\",\"value\":false}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    // Should contain the full precision number
    CPPUNIT_ASSERT_MESSAGE("Full precision expected", aData.indexOf("1.23456789") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonSaveFormulas)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetValue(ScAddress(0, 0, 0), 10.0);
    pDoc->SetValue(ScAddress(1, 0, 0), 20.0);
    pDoc->SetString(ScAddress(2, 0, 0), u"=A1+B1"_ustr);

    setFilterOptions(u"{\"SaveFormulas\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    // The formula should appear in the output, not the computed value 30
    CPPUNIT_ASSERT_MESSAGE("Formula should be exported", aData.indexOf("=A1+B1") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonSaveNumberAsSuch)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetValue(ScAddress(0, 0, 0), 42.0);

    // SaveNumberAsSuch=true (default) saves numbers without quoting
    setFilterOptions(u"{\"SaveNumberAsSuch\":{\"type\":\"boolean\",\"value\":true},"
                     "\"QuoteAllText\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    // Number should NOT be quoted even when QuoteAllText is true
    CPPUNIT_ASSERT_MESSAGE("Number should not be quoted", aData.indexOf("\"42\"") < 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonIncludeBOM)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"test"_ustr);

    setFilterOptions(u"{\"IncludeBOM\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    save(TestFilter::CSV);

    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    char buf[3] = {};
    aStream.ReadBytes(buf, 3);
    // UTF-8 BOM: EF BB BF
    CPPUNIT_ASSERT_EQUAL('\xEF', buf[0]);
    CPPUNIT_ASSERT_EQUAL('\xBB', buf[1]);
    CPPUNIT_ASSERT_EQUAL('\xBF', buf[2]);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonNoBOM)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"test"_ustr);

    setFilterOptions(u"{\"IncludeBOM\":{\"type\":\"boolean\",\"value\":false}}"_ustr);
    save(TestFilter::CSV);

    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    char buf[4] = {};
    aStream.ReadBytes(buf, 4);
    // Should start with "test", not BOM
    CPPUNIT_ASSERT_EQUAL("test"_ostr, OString(buf, 4));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonSheet)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->InsertTab(1, u"Second"_ustr);
    pDoc->SetString(ScAddress(0, 0, 0), u"first_sheet"_ustr);
    pDoc->SetString(ScAddress(0, 0, 1), u"second_sheet"_ustr);

    // Export the second sheet by number (1-based)
    setFilterOptions(u"{\"Sheet\":{\"type\":\"long\",\"value\":2}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    CPPUNIT_ASSERT_MESSAGE("Should contain second sheet data", aData.indexOf("second_sheet") >= 0);
    CPPUNIT_ASSERT_MESSAGE("Should not contain first sheet data", aData.indexOf("first_sheet") < 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonSheetByName)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->InsertTab(1, u"MySheet"_ustr);
    pDoc->SetString(ScAddress(0, 0, 0), u"first_sheet"_ustr);
    pDoc->SetString(ScAddress(0, 0, 1), u"named_sheet"_ustr);

    // Export sheet by name
    setFilterOptions(u"{\"Sheet\":{\"type\":\"string\",\"value\":\"MySheet\"}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    CPPUNIT_ASSERT_MESSAGE("Should contain named sheet data", aData.indexOf("named_sheet") >= 0);
    CPPUNIT_ASSERT_MESSAGE("Should not contain first sheet data", aData.indexOf("first_sheet") < 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonDefaults)
{
    // When JSON is used but no options specified, defaults should apply:
    // UTF-8, comma separator, double-quote delimiter
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"hello"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"world"_ustr);

    // Empty JSON object - all defaults
    setFilterOptions(u"{}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    CPPUNIT_ASSERT_MESSAGE("Comma separator by default", aData.indexOf("hello,world") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonMultipleOptions)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"text"_ustr);
    pDoc->SetValue(ScAddress(1, 0, 0), 42.0);
    pDoc->SetString(ScAddress(2, 0, 0), u"=B1*2"_ustr);

    setFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\";\"},"
                     "\"SaveFormulas\":{\"type\":\"boolean\",\"value\":true},"
                     "\"IncludeBOM\":{\"type\":\"boolean\",\"value\":false}}"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    CPPUNIT_ASSERT_MESSAGE("Semicolon separator expected", aData.indexOf(';') >= 0);
    CPPUNIT_ASSERT_MESSAGE("Formula should be present", aData.indexOf("=B1*2") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvExportJsonLegacyFallback)
{
    // Non-JSON filter options should still work (legacy format)
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);

    // Legacy format: field_sep, text_sep, charset
    setFilterOptions(u"9,34,UTF-8"_ustr);
    save(TestFilter::CSV);
    OString aData = readExportedFile();
    CPPUNIT_ASSERT_MESSAGE("Tab separator (legacy format)", aData.indexOf("A\tB") >= 0);
}

// --- CSV Import Tests ---

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonFieldSeparator)
{
    // Create a document, export with semicolons, then reimport with JSON options
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"Col1"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"Col2"_ustr);
    pDoc->SetString(ScAddress(2, 0, 0), u"Col3"_ustr);
    pDoc->SetValue(ScAddress(0, 1, 0), 10.0);
    pDoc->SetValue(ScAddress(1, 1, 0), 20.0);
    pDoc->SetValue(ScAddress(2, 1, 0), 30.0);

    // Export with semicolon separator
    setFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\";\"}}"_ustr);
    save(TestFilter::CSV);

    // Now reimport with JSON options specifying semicolon
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\";\"}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"Col1"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Col2"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Col3"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(2, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonStartRow)
{
    // Create a CSV with header rows we want to skip
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"Header1"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"Header2"_ustr);
    pDoc->SetString(ScAddress(0, 1, 0), u"SubHeader1"_ustr);
    pDoc->SetString(ScAddress(1, 1, 0), u"SubHeader2"_ustr);
    pDoc->SetString(ScAddress(0, 2, 0), u"Data1"_ustr);
    pDoc->SetValue(ScAddress(1, 2, 0), 100.0);

    setFilterOptions(u"{}"_ustr);
    save(TestFilter::CSV);

    // Reimport starting from row 3 (skip two header rows)
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"StartRow\":{\"type\":\"long\",\"value\":3}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    // First row should be "Data1", not "Header1"
    CPPUNIT_ASSERT_EQUAL(u"Data1"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(100.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonQuotedFieldAsText)
{
    // Write a CSV file with a quoted number
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aStream(aTempFile.GetURL(), StreamMode::WRITE);
        // "12345" is a number wrapped in quotes
        aStream.WriteOString("\"12345\",plain\n");
        aStream.Flush();
    }

    // Import with QuotedFieldAsText=true - quoted numbers should become text
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"QuotedFieldAsText\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    loadFromURL(aTempFile.GetURL());
    ScDocument* pDoc = getScDoc();

    // The quoted number should be imported as text, not number
    CPPUNIT_ASSERT_MESSAGE("Should be text, not number", pDoc->HasStringData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"12345"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonDetectSpecialNumber)
{
    // Create CSV with a date-like string
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"text"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"12/31/2025"_ustr);

    // Export without quoting to get raw text in CSV
    setFilterOptions(u"{}"_ustr);
    save(TestFilter::CSV);

    // Import with DetectSpecialNumber=false - date should be kept as text
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(
        u"{\"DetectSpecialNumber\":{\"type\":\"boolean\",\"value\":false}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"text"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    // With DetectSpecialNumber=false, the date string should stay as text
    CPPUNIT_ASSERT_MESSAGE("Date should be imported as text", pDoc->HasStringData(1, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonEvaluateFormulas)
{
    // Create CSV with a formula
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetValue(ScAddress(0, 0, 0), 10.0);
    pDoc->SetString(ScAddress(1, 0, 0), u"=A1*2"_ustr);

    // Export formulas as text
    setFilterOptions(u"{\"SaveFormulas\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    save(TestFilter::CSV);

    // Import with EvaluateFormulas=false - formula should be text
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"EvaluateFormulas\":{\"type\":\"boolean\",\"value\":false}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    // The formula text should be imported as a string, not evaluated
    CPPUNIT_ASSERT_MESSAGE("Formula should be imported as text", pDoc->HasStringData(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"=A1*2"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonEvaluateFormulasTrue)
{
    // Create CSV with a formula
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetValue(ScAddress(0, 0, 0), 10.0);
    pDoc->SetString(ScAddress(1, 0, 0), u"=A1*2"_ustr);

    // Export formulas
    setFilterOptions(u"{\"SaveFormulas\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    save(TestFilter::CSV);

    // Import with EvaluateFormulas=true - formula should be evaluated
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"EvaluateFormulas\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    // The formula should be evaluated
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonRemoveSpace)
{
    // Write a CSV file with spaces around values manually
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"  hello  "_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"  world  "_ustr);

    setFilterOptions(u"{}"_ustr);
    save(TestFilter::CSV);

    // Import with RemoveSpace=true
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"RemoveSpace\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    // Spaces should be trimmed
    CPPUNIT_ASSERT_EQUAL(u"hello"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"world"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonColumnFormat)
{
    // Write a CSV file with values that could be interpreted as numbers
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aStream(aTempFile.GetURL(), StreamMode::WRITE);
        // "00123" has leading zeros - only preserved with Text format
        aStream.WriteOString("00123,text,456\n");
        aStream.Flush();
    }

    // Import with column formats: col 1 as Text (2), col 2 as Standard (1), col 3 as Text (2)
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(
        u"{\"ColumnFormat\":{\"type\":\"string\",\"value\":\"1/2/2/1/3/2\"}}"_ustr);
    loadFromURL(aTempFile.GetURL());
    ScDocument* pDoc = getScDoc();

    // Column 1 (format=Text): "00123" should be kept as text with leading zeros
    CPPUNIT_ASSERT_MESSAGE("Col1 should be text (ColumnFormat=2)", pDoc->HasStringData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"00123"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));

    // Column 2 (format=Standard): "text" stays text
    CPPUNIT_ASSERT_EQUAL(u"text"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));

    // Column 3 (format=Text): "456" should be text, not number
    CPPUNIT_ASSERT_MESSAGE("Col3 should be text (ColumnFormat=2)", pDoc->HasStringData(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"456"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonColumnFormatSkip)
{
    // Write a CSV file to test Skip column format
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aStream(aTempFile.GetURL(), StreamMode::WRITE);
        aStream.WriteOString("keep1,skip_me,keep2\n");
        aStream.Flush();
    }

    // Import: col 1=Standard, col 2=Skip(9), col 3=Standard
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(
        u"{\"ColumnFormat\":{\"type\":\"string\",\"value\":\"1/1/2/9/3/1\"}}"_ustr);
    loadFromURL(aTempFile.GetURL());
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"keep1"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    // Column 2 was skipped, so "keep2" should be in column B (index 1)
    CPPUNIT_ASSERT_EQUAL(u"keep2"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonMergeDelimiters)
{
    // Create CSV with multiple consecutive separators
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"A"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"B"_ustr);

    // Export with tab separator
    setFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\"\\t\"}}"_ustr);
    save(TestFilter::CSV);

    // Manually we can't easily create double-tabs via export, but we can
    // test that MergeDelimiters option is accepted without error
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\"\\t\"},"
                           "\"MergeDelimiters\":{\"type\":\"boolean\",\"value\":true}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonFixedWidth)
{
    // Write a fixed-width text file manually
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aStream(aTempFile.GetURL(), StreamMode::WRITE);
        // Fixed width: col1=5chars, col2=5chars, col3=rest
        OString aLine("ABCDE12345Hello\n"_ostr);
        aStream.WriteOString(aLine);
        aStream.Flush();
    }

    // Import as fixed-width with column positions at 0, 5, 10
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(
        u"{\"FixedWidth\":{\"type\":\"boolean\",\"value\":true},"
        "\"ColumnFormat\":{\"type\":\"string\",\"value\":\"0/1/5/1/10/1\"}}"_ustr);
    loadFromURL(aTempFile.GetURL());
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"ABCDE"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(12345.0, pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonFixedWidthTextFormat)
{
    // Write a fixed-width text file with numbers that should be kept as text
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aStream(aTempFile.GetURL(), StreamMode::WRITE);
        // Fixed width: col1=5chars (zip code), col2=10chars (phone)
        OString aLine("0712305551234567\n"_ostr);
        aStream.WriteOString(aLine);
        aStream.Flush();
    }

    // Import as fixed-width: col1 as Text(2), col2 as Text(2)
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"FixedWidth\":{\"type\":\"boolean\",\"value\":true},"
                           "\"ColumnFormat\":{\"type\":\"string\",\"value\":\"0/2/5/2\"}}"_ustr);
    loadFromURL(aTempFile.GetURL());
    ScDocument* pDoc = getScDoc();

    // Should preserve leading zeros as text
    CPPUNIT_ASSERT_MESSAGE("Zip code should be text", pDoc->HasStringData(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"07123"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("Phone should be text", pDoc->HasStringData(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"05551234567"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportJsonLegacyFallback)
{
    // Non-JSON import filter options should still work (legacy format)
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"X"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"Y"_ustr);

    // Export with tabs
    setFilterOptions(u"9,34,UTF-8"_ustr);
    save(TestFilter::CSV);

    // Reimport with legacy options (tab separator)
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"9,34,UTF-8"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"X"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Y"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportExportJsonRoundTrip)
{
    // Full round-trip: create doc, export with JSON options, reimport with JSON options
    createScDoc();
    ScDocument* pDoc = getScDoc();
    pDoc->SetString(ScAddress(0, 0, 0), u"Name"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"Value"_ustr);
    pDoc->SetString(ScAddress(2, 0, 0), u"Note"_ustr);
    pDoc->SetString(ScAddress(0, 1, 0), u"Item;One"_ustr); // contains semicolon
    pDoc->SetValue(ScAddress(1, 1, 0), 3.14);
    pDoc->SetString(ScAddress(2, 1, 0), u"a \"quoted\" note"_ustr); // contains quotes

    // Export with semicolon separator
    setFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\";\"},"
                     "\"IncludeBOM\":{\"type\":\"boolean\",\"value\":false}}"_ustr);
    save(TestFilter::CSV);

    // Reimport with matching options
    setImportFilterName(TestFilter::CSV);
    setImportFilterOptions(u"{\"FieldSeparator\":{\"type\":\"string\",\"value\":\";\"}}"_ustr);
    loadFromURL(maTempFile.GetURL());
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Value"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Note"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Item;One"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(3.14, pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"a \"quoted\" note"_ustr, pDoc->GetString(ScAddress(2, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScCsvJsonFilterTest, testCsvImportEmptyFilterOptions)
{
    // Write a comma-separated CSV file
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aStream(aTempFile.GetURL(), StreamMode::WRITE);
        aStream.WriteOString("Name,Value,Note\nAlpha,100,first\nBeta,200,second\n");
        aStream.Flush();
    }

    // Load with explicitly empty FilterOptions (simulates convert-to API without
    // infilterOptions). Before the fix, ReadFromString("") left the default semicolon
    // separator and set bOptInit=true, preventing the comma/UTF-8 fallback defaults.
    std::vector<css::beans::PropertyValue> aFilterOptions;
    css::beans::PropertyValue aFilterName;
    aFilterName.Name = "FilterName";
    aFilterName.Value <<= u"Text - txt - csv (StarCalc)"_ustr;
    aFilterOptions.push_back(aFilterName);
    css::beans::PropertyValue aFilterOpts;
    aFilterOpts.Name = "FilterOptions";
    aFilterOpts.Value <<= OUString();
    aFilterOptions.push_back(aFilterOpts);
    loadWithParams(aTempFile.GetURL(), comphelper::containerToSequence(aFilterOptions));

    ScDocument* pDoc = getScDoc();

    // With the fix, commas are detected as field separators (fallback defaults apply).
    // Without the fix, each line is imported as a single cell.
    CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Value"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Note"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Alpha"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(100.0, pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"first"_ustr, pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Beta"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(200.0, pDoc->GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"second"_ustr, pDoc->GetString(ScAddress(2, 2, 0)));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
