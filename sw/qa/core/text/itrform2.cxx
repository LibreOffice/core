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

#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertyvalue.hxx>
#include <editeng/colritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <formatcontentcontrol.hxx>
#include <textcontentcontrol.hxx>
#include <view.hxx>
#include <cmdid.h>

namespace
{
/// Covers sw/source/core/text/itrform2.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/text/data/")
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
    save("writer_pdf_Export");

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

    SwExportFormFieldsGuard g;
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
    save("writer_pdf_Export");

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
    CPPUNIT_ASSERT_EQUAL(OUString("Yes"), aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlPDFFontColor)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    SwExportFormFieldsGuard g;
    // Given a document with a custom orange font color and a content control:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SfxItemSetFixed<RES_CHRATR_COLOR, RES_CHRATR_COLOR> aSet(pWrtShell->GetAttrPool());
    Color nOrange(0xff6b00);
    SvxColorItem aItem(nOrange, RES_CHRATR_COLOR);
    aSet.Put(aItem);
    pWrtShell->SetAttrSet(aSet);
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);

    // When exporting that document to PDF:
    save(u"writer_pdf_Export"_ustr);

    // Then make sure that the widget in the PDF result has that custom font color:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    pPage->onAfterLoadPage(pPdfDocument.get());
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[ff6b00ff]
    // - Actual  : rgba[000000ff]
    // i.e. the custom color was lost, the font color was black, not orange.
    CPPUNIT_ASSERT_EQUAL(nOrange, pAnnotation->getFontColor(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlPDFDropDownText)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    SwExportFormFieldsGuard g;
    // Given a document with a dropdown: custom default text and 3 items:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"red"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"R"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"green"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"G"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"blue"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"B"_ustr)),
            },
        };
        xContentControlProps->setPropertyValue(u"ListItems"_ustr, uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting that to PDF:
    save(u"writer_pdf_Export"_ustr);

    // Then make sure that the custom default is not lost:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    pPage->onAfterLoadPage(pPdfDocument.get());
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 3
    // i.e. only the 3 colors were exported, the default "test" text was not.
    CPPUNIT_ASSERT_EQUAL(4, pAnnotation->getOptionCount(pPdfDocument.get()));
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlPDFComments)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given a document with both a content control and a comment:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);
    pWrtShell->SttEndDoc(/*bStt=*/false);
    SwDocShell* pDocShell = getSwDocShell();
    SwView* pView = pDocShell->GetView();
    pView->GetViewFrame().GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // When exporting to PDF, exporting notes in master (and not as widgets):
    uno::Sequence<beans::PropertyValue> aFilterData = {
        comphelper::makePropertyValue(u"ExportFormFields"_ustr, true),
        comphelper::makePropertyValue(u"ExportNotes"_ustr, false),
        comphelper::makePropertyValue(u"ExportNotesInMargin"_ustr, true),
    };
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
    });

    // Then make sure the only widget for the content control has a correct position:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    pPage->onAfterLoadPage(pPdfDocument.get());
    CPPUNIT_ASSERT_EQUAL(1, pPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
    basegfx::B2DPoint aAnnotTopLeft = pAnnotation->getRectangle().getMinimum();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: (41.749, 639.401)
    // - Actual  : (59.249,716.951)
    // i.e. the content control rectangle was shifted towards the top right of the page, compared to
    // where it's expected.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(41.749, aAnnotTopLeft.getX(), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(639.401, aAnnotTopLeft.getY(), 0.001);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
