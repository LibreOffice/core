/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <formatcontentcontrol.hxx>
#include <textcontentcontrol.hxx>

namespace
{
/// Covers sw/source/core/text/itrform2.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableWrapEmptyParagraph)
{
    // Given a document with 2 pages, a floating table on both pages:
    createSwDoc("floattable-wrap-empty-para.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure that each page has exactly 1 floating table:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both tables were on page 1, leading to an overlap.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPageObjs2 = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs2.size());
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableLegacyWrapEmptyParagraph)
{
    // Given a document with 2 pages, a floating table on both pages (from DOC, so the table is
    // shifted towards the left page edge slightly):
    createSwDoc("floattable-wrap-empty-para-legacy.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure that each page has exactly 1 floating table:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both tables were on page 1, leading to an overlap.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPageObjs2 = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs2.size());
}

CPPUNIT_TEST_FIXTURE(Test, testApplyTextAttrToEmptyLineAtEndOfParagraph)
{
    createSwDoc("A011-charheight.rtf");

    calcLayout();

    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());

    SwContentFrame* pLastPara = pPage->FindLastBodyContent();
    // wrong was 449 (11.5pt)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(368), pLastPara->getFrameArea().Height());
    SwContentFrame* pFirstPara = pPage->FindFirstBodyContent();
    // wrong was 817 (11.5pt)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(736), pFirstPara->getFrameArea().Height());
}

CPPUNIT_TEST_FIXTURE(Test, testFlyMinimalWrap)
{
    // Given a document with a first page that has a shape and a table in it (not floating table),
    // some empty paragraphs wrapping around the shape:
    createSwDoc("fly-minimal-wrap.docx");

    // When calculating the layout:
    calcLayout();

    // Then make sure the wrap happens, so the 2nd page only has 2 paragraphs:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPageObjs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(!pPage2->GetSortedObjs());
    SwLayoutFrame* pBody2 = pPage2->FindBodyCont();
    SwFrame* pPage2Para1 = pBody2->GetLower();
    CPPUNIT_ASSERT(pPage2Para1);
    SwFrame* pPage2Para2 = pPage2Para1->GetNext();
    CPPUNIT_ASSERT(pPage2Para2);
    // Without the accompanying fix in place, this test would have failed, the second page had 19
    // text frames in the body frame, not 2.
    CPPUNIT_ASSERT(!pPage2Para2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlHeaderPDFExport)
{
    // Given a document with a content control in the header:
    createSwDoc("content-control-header.docx");

    // When exporting to PDF:
    save(u"writer_pdf_Export"_ustr);

    // Then make sure all the expected text is there on page 2:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    if (!pPdfDocument)
    {
        return;
    }
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage2 = pPdfDocument->openPage(1);
    int nTextCount = 0;
    for (int i = 0; i < pPage2->getObjectCount(); ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pObject = pPage2->getObject(i);
        if (pObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            ++nTextCount;
        }
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 2
    // i.e. not all of header, heading and body text was there on page 2, content was lost.
    CPPUNIT_ASSERT_EQUAL(3, nTextCount);
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyAnchorLeftMargin)
{
    // Given a document with a floating table, anchor para is followed by another para with a left
    // margin:
    createSwDoc("floattable-anchor-left-margin.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the left margin of this last paragraph is not lost:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    SwContentFrame* pLastPara = pPage->FindLastBodyContent();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6480
    // - Actual  : 0
    // i.e. the left margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(6480), pLastPara->getFramePrintArea().Left());
}

CPPUNIT_TEST_FIXTURE(Test, testCheckedCheckboxContentControlPDF)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a file with a checked checkbox content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);
    // Toggle it, so we get a checked one:
    SwTextContentControl* pTextContentControl = pWrtShell->CursorInsideContentControl();
    const SwFormatContentControl& rFormatContentControl = pTextContentControl->GetContentControl();
    pWrtShell->GotoContentControl(rFormatContentControl);

    // When exporting to PDF:
    save(u"writer_pdf_Export"_ustr);

    // Then make sure that a checked checkbox form widget is emitted:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Widget, pAnnotation->getSubType());
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::CheckBox,
                         pAnnotation->getFormFieldType(pPdfDocument.get()));
    OUString aActual = pAnnotation->getFormFieldValue(pPdfDocument.get());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Yes
    // - Actual  : Off
    // i.e. the /AP -> /N key of the checkbox widget annotation object didn't have a sub-key that
    // would match /V, leading to not showing the checked state.
    CPPUNIT_ASSERT_EQUAL(u"Yes"_ustr, aActual);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
