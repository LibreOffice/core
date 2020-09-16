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
    TxtImportTest()
        : SwModelTestBase("/sw/qa/extras/txtimport/data/", "Text")
    {
    }

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

#define DECLARE_TXTIMPORT_TEST(TestName, filename)                                                 \
    DECLARE_SW_IMPORT_TEST(TestName, filename, nullptr, TxtImportTest)

DECLARE_TXTIMPORT_TEST(testTdf112191, "bullets.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
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

    OUString aString = OStringToOUString("    \xe2\x80\xa2 First bullet" SAL_NEWLINE_STRING
                                         "    \xe2\x80\xa2 Second bullet",
                                         RTL_TEXTENCODING_UTF8);

    SwPaM aPaM3(*aPaM2.GetMark(), *aPaM.GetPoint());
    assertExportedRange(OUStringToOString(aString, osl_getThreadTextEncoding()), aPaM3);
}

DECLARE_TXTIMPORT_TEST(testTdf60145_utf8withoutbom, "UTF8WITHOUTBOM.txt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

DECLARE_TXTIMPORT_TEST(testTdf60145_utf8withbom, "UTF8WITHBOM.txt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

DECLARE_TXTIMPORT_TEST(testTdf60145_utf16lewithoutbom, "UTF16LEWITHOUTBOM.txt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

DECLARE_TXTIMPORT_TEST(testTdf60145_utf16lewithbom, "UTF16LEWITHBOM.txt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

DECLARE_TXTIMPORT_TEST(testTdf60145_utf16bewithoutbom, "UTF16BEWITHOUTBOM.txt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

DECLARE_TXTIMPORT_TEST(testTdf60145_utf16bewithbom, "UTF16BEWITHBOM.txt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
