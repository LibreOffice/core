/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <shellio.hxx>
#include <unotextrange.hxx>
#include <iodetect.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

class TxtImportTest : public SwModelTestBase
{
public:
    TxtImportTest() :
        SwModelTestBase("/sw/qa/extras/txtexport/data/", "Text")
    {}

    // Export & assert part of the document (defined by SwPaM).
    void assertExportedRange(const OString& aExpected, SwPaM& rPaM)
    {
        WriterRef rAsciiWriter;
        SwReaderWriter::GetWriter(FILTER_TEXT, OUString(), rAsciiWriter);
        CPPUNIT_ASSERT(rAsciiWriter.is());

        // no start char
        rAsciiWriter->m_bUCS2_WithStartChar = false;

        SvMemoryStream aMemoryStream;

        SwWriter aWriter(aMemoryStream, rPaM);
        ErrCode nError = aWriter.Write(rAsciiWriter);
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, nError);

        const char* pData = static_cast<const char*>(aMemoryStream.GetData());
        OString aResult(pData, aMemoryStream.GetSize());

        CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
    }
};

class TxtExportTest : public SwModelTestBase
{
public:
    TxtExportTest() :
        SwModelTestBase("/sw/qa/extras/txtexport/data/", "Text")
    {}

protected:
    OString readExportedFile()
    {
        SvMemoryStream aMemoryStream;
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        aStream.ReadStream(aMemoryStream);
        const char* pData = static_cast<const char*>(aMemoryStream.GetData());

        int offset = 0;
        if (aMemoryStream.GetSize() > 2 && pData[0] == '\xEF' && pData[1] == '\xBB' && pData[2] == '\xBF')
            offset = 3;

        return OString(pData + offset, aMemoryStream.GetSize() - offset);
    }
};

#define DECLARE_TXTIMPORT_TEST(TestName, filename) DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, TxtImportTest)
#define DECLARE_TXTEXPORT_TEST(TestName, filename) DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, TxtExportTest)

DECLARE_TXTEXPORT_TEST(testBullets, "bullets.odt")
{
    OString aData = readExportedFile();

    OUString aString = OStringToOUString(
        "1 Heading 1" SAL_NEWLINE_STRING
        "1.A Heading 2" SAL_NEWLINE_STRING
        "Paragraph" SAL_NEWLINE_STRING
        "" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 First bullet" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 Second bullet" SAL_NEWLINE_STRING
        "        \xe2\x97\xa6 Sub-second bullet" SAL_NEWLINE_STRING
        "      Third bullet, but deleted" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 Fourth bullet" SAL_NEWLINE_STRING
        "" SAL_NEWLINE_STRING
        "Numbering" SAL_NEWLINE_STRING
        "" SAL_NEWLINE_STRING
        "    1. First" SAL_NEWLINE_STRING
        "    2. Second" SAL_NEWLINE_STRING
        "        1. Second-first" SAL_NEWLINE_STRING
        "       Third, but deleted" SAL_NEWLINE_STRING
        "    3. Actual third" SAL_NEWLINE_STRING
        "" SAL_NEWLINE_STRING
        "Paragraph after numbering" SAL_NEWLINE_STRING
        "Next paragraph" SAL_NEWLINE_STRING
        "Final paragraph" SAL_NEWLINE_STRING, RTL_TEXTENCODING_UTF8);

    // To get the stuff back in the system's encoding
    OString aExpected(OUStringToOString(aString, osl_getThreadTextEncoding()));

    CPPUNIT_ASSERT_EQUAL(aExpected, aData);
}

DECLARE_TXTIMPORT_TEST(testTdf112191, "bullets.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    // just the 5th paragraph - no bullet
    uno::Reference<text::XTextRange> xPara(getParagraph(5));
    SwUnoInternalPaM aPaM(*pDoc);
    bool bSuccess = sw::XTextRangeToSwPaM(aPaM, xPara);
    CPPUNIT_ASSERT(bSuccess);

    assertExportedRange("First bullet", aPaM);

    // but when we extend to the next paragraph - now there are bullets
    xPara = getParagraph(6);
    SwUnoInternalPaM aPaM2(*pDoc);
    bSuccess = sw::XTextRangeToSwPaM(aPaM2, xPara);
    CPPUNIT_ASSERT(bSuccess);

    OUString aString = OStringToOUString(
        "    \xe2\x80\xa2 First bullet" SAL_NEWLINE_STRING
        "    \xe2\x80\xa2 Second bullet", RTL_TEXTENCODING_UTF8);

    SwPaM aPaM3(*aPaM2.GetMark(), *aPaM.GetPoint());
    assertExportedRange(OUStringToOString(aString, osl_getThreadTextEncoding()), aPaM3);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
