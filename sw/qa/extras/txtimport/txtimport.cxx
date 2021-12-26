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
#include <wrtsh.hxx>

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

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf112191)
{
    load(mpTestDocumentPath, "bullets.odt");
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

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf8withoutbom)
{
    load(mpTestDocumentPath, "UTF8WITHOUTBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf8withbom)
{
    load(mpTestDocumentPath, "UTF8WITHBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16lewithoutbom)
{
    load(mpTestDocumentPath, "UTF16LEWITHOUTBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16lewithbom)
{
    load(mpTestDocumentPath, "UTF16LEWITHBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf92161_gb18030)
{
    load(mpTestDocumentPath, "GB18030.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"盖闻天地之数，有十二万九千六百岁为一元。"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf146429_EUC_KR)
{
    //ICU returned 100% confidence at time of writing
    load(mpTestDocumentPath, "EUC-KR.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"행복한 크리스마스가 되길 빌어 요"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf146429_EUC_KR_2_char)
{
    //confidence below threshold so this should fail detection
    load(mpTestDocumentPath, "EUC-KR_2_char.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"�ູ"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16bewithoutbom)
{
    load(mpTestDocumentPath, "UTF16BEWITHOUTBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16bewithbom)
{
    load(mpTestDocumentPath, "UTF16BEWITHBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(OUString(u"漢a'"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf115088)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("1");
    pWrtShell->SplitNode();
    pWrtShell->Insert("1");

    pWrtShell->SelAll();
    dispatchCommand(mxComponent, ".uno:Cut", {});
    pWrtShell->Insert("test");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:PasteUnformatted", {});
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    OUString aActual = xTextDocument->getText()->getString().copy(0, 2);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1\n
    // - Actual  : 1t
    CPPUNIT_ASSERT_EQUAL(OUString("1\n"), aActual.replaceAll("\r", "\n"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
