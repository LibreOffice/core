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
#include <rtl/ustrbuf.hxx>

namespace
{
class TxtImportTest : public SwModelTestBase
{
public:
    TxtImportTest()
        : SwModelTestBase(u"/sw/qa/extras/txtimport/data/"_ustr, u"Text"_ustr)
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
        ErrCodeMsg nError = aWriter.Write(rAsciiWriter);
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, nError.GetCode());

        const char* pData = static_cast<const char*>(aMemoryStream.GetData());
        OString aResult(pData, aMemoryStream.GetSize());

        CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
    }
};

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf112191)
{
    createSwDoc("bullets.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    // just the 5th paragraph - no bullet
    uno::Reference<text::XTextRange> xPara(getParagraph(5));
    SwUnoInternalPaM aPaM(*pDoc);
    bool bSuccess = sw::XTextRangeToSwPaM(aPaM, xPara);
    CPPUNIT_ASSERT(bSuccess);

    assertExportedRange("First bullet"_ostr, aPaM);

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
    createSwDoc("UTF8WITHOUTBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"漢a'"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf8withbom)
{
    createSwDoc("UTF8WITHBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"漢a'"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16lewithoutbom)
{
    createSwDoc("UTF16LEWITHOUTBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"漢a'"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16lewithbom)
{
    createSwDoc("UTF16LEWITHBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"漢a'"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf92161_gb18030)
{
    createSwDoc("GB18030.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"盖闻天地之数，有十二万九千六百岁为一元。"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16bewithoutbom)
{
    createSwDoc("UTF16BEWITHOUTBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"漢a'"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf60145_utf16bewithbom)
{
    createSwDoc("UTF16BEWITHBOM.txt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(u"漢a'"_ustr, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf115088)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert(u"1"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"1"_ustr);

    pWrtShell->SelAll();
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    pWrtShell->Insert(u"test"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:PasteUnformatted"_ustr, {});
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    OUString aActual = xTextDocument->getText()->getString().copy(0, 2);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1\n
    // - Actual  : 1t
    CPPUNIT_ASSERT_EQUAL(u"1\n"_ustr, aActual.replaceAll("\r", "\n"));
}

CPPUNIT_TEST_FIXTURE(TxtImportTest, testTdf70423)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    constexpr sal_Int32 size = 30000; // It should be multiple of 10
    constexpr sal_Int32 parts = size / 10;

    rtl::OUStringBuffer s(size);

    for (size_t i = 0; i < parts; i++)
    {
        s.append("0123456789");
    }

    OUString aResStr = s.makeStringAndClear();
    pWrtShell->Insert(aResStr);

    saveAndReload(u"Text"_ustr); //Reloading the file again

    // Without the fix, this test would have failed with:
    // - Expected: 1
    // - Actual: 3
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    OUString aPara = xPara->getString();

    // Without the fix, this test would have failed with:
    // - Expected: 30000
    // - Actual: 10000
    CPPUNIT_ASSERT_EQUAL(size, aPara.getLength());

    //Matching the paragraph text and created string
    CPPUNIT_ASSERT_EQUAL(aResStr, aPara);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
