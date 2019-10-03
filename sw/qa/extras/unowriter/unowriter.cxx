/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <comphelper/propertyvalue.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/unowriter/data/";
}

/// Test to assert UNO API call results of Writer.
class SwUnoWriter : public SwModelTestBase
{
public:
    SwUnoWriter()
        : SwModelTestBase(DATA_DIRECTORY, "writer8")
    {
    }
};

/**
 * Macro to declare a new test with preloaded file
 * (similar to DECLARE_SW_ROUNDTRIP_TEST)
 */
#define DECLARE_UNOAPI_TEST_FILE(TestName, filename)                                               \
    class TestName : public SwUnoWriter                                                            \
    {                                                                                              \
    protected:                                                                                     \
        virtual OUString getTestName() override { return OUString(#TestName); }                    \
                                                                                                   \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(loadAndTest);                                                                 \
        CPPUNIT_TEST_SUITE_END();                                                                  \
        void loadAndTest()                                                                         \
        {                                                                                          \
            load(mpTestDocumentPath, filename);                                                    \
            runTest();                                                                             \
        }                                                                                          \
        void runTest();                                                                            \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::runTest()

/**
 * Macro to declare a new test without loading any files
 */
#define DECLARE_UNOAPI_TEST(TestName)                                                              \
    class TestName : public SwUnoWriter                                                            \
    {                                                                                              \
    protected:                                                                                     \
        virtual OUString getTestName() override { return OUString(#TestName); }                    \
                                                                                                   \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(runTest);                                                                     \
        CPPUNIT_TEST_SUITE_END();                                                                  \
        void runTest();                                                                            \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::runTest()

DECLARE_UNOAPI_TEST_FILE(testRenderablePagePosition, "renderable-page-position.odt")
{
    // Make sure that the document has 2 pages.
    uno::Reference<view::XRenderable> xRenderable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Any aSelection = uno::makeAny(mxComponent);

    uno::Reference<awt::XToolkit> xToolkit = VCLUnoHelper::CreateToolkit();
    uno::Reference<awt::XDevice> xDevice(xToolkit->createScreenCompatibleDevice(32, 32));

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController = xModel->getCurrentController();

    beans::PropertyValues aRenderOptions = {
        comphelper::makePropertyValue("IsPrinter", true),
        comphelper::makePropertyValue("RenderDevice", xDevice),
        comphelper::makePropertyValue("View", xController),
        comphelper::makePropertyValue("RenderToGraphic", true),
    };

    sal_Int32 nPages = xRenderable->getRendererCount(aSelection, aRenderOptions);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), nPages);

    // Make sure that the first page has some offset.
    comphelper::SequenceAsHashMap aRenderer1(
        xRenderable->getRenderer(0, aSelection, aRenderOptions));
    // Without the accompanying fix in place, this test would have failed: i.e.
    // there was no PagePos key in this map.
    awt::Point aPosition1 = aRenderer1["PagePos"].get<awt::Point>();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition1.X);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition1.Y);

    // Make sure that the second page is below the first one.
    comphelper::SequenceAsHashMap aRenderer2(
        xRenderable->getRenderer(1, aSelection, aRenderOptions));
    awt::Point aPosition2 = aRenderer2["PagePos"].get<awt::Point>();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition2.X);
    CPPUNIT_ASSERT_GREATER(aPosition1.Y, aPosition2.Y);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testPasteListener)
{
    loadURL("private:factory/swriter", nullptr);

    // Insert initial string.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), "ABCDEF", false);

    // Add paste listener.
    uno::Reference<text::XPasteBroadcaster> xBroadcaster(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XPasteListener> xListener(new PasteListener);
    auto pListener = static_cast<PasteListener*>(xListener.get());
    xBroadcaster->addPasteEventListener(xListener);

    // Cut "DE" and then paste it.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    TransferableDataHelper aHelper(pTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    // Without working listener registration in place, this test would have
    // failed with 'Expected: DE; Actual:', i.e. the paste listener was not
    // invoked.
    CPPUNIT_ASSERT_EQUAL(OUString("DE"), pListener->GetString());

    // Make sure that paste did not overwrite anything.
    CPPUNIT_ASSERT_EQUAL(OUString("ABCDEF"), xBodyText->getString());

    // Paste again, this time overwriting "BC".
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pListener->GetString().clear();
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(OUString("DE"), pListener->GetString());

    // Make sure that paste overwrote "BC".
    CPPUNIT_ASSERT_EQUAL(OUString("ADEDEF"), xBodyText->getString());

    // Test image paste.
    SwView& rView = pWrtShell->GetView();
    OUString aGraphicURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "test.jpg";
    rView.InsertGraphic(aGraphicURL, OUString(), /*bAsLink=*/false,
                        &GraphicFilter::GetGraphicFilter());

    // Test that the pasted image is anchored as-char.
    SwFlyFrame* pFly = pWrtShell->GetSelectedFlyFrame();
    CPPUNIT_ASSERT(pFly);
    SwFrameFormat* pFlyFormat = pFly->GetFormat();
    CPPUNIT_ASSERT(pFlyFormat);
    RndStdIds eFlyAnchor = pFlyFormat->GetAnchor().GetAnchorId();
    // Without the working image listener in place, this test would have
    // failed, eFlyAnchor was FLY_AT_PARA.
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AS_CHAR, eFlyAnchor);

    pTransfer->Cut();
    pListener->GetString().clear();
    SwTransferable::Paste(*pWrtShell, aHelper);
    // Without the working image listener in place, this test would have
    // failed, the listener was not invoked in case of a graphic paste.
    CPPUNIT_ASSERT(pListener->GetTextGraphicObject().is());
    CPPUNIT_ASSERT(pListener->GetString().isEmpty());

    // Deregister paste listener, make sure it's not invoked.
    xBroadcaster->removePasteEventListener(xListener);
    pListener->GetString().clear();
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT(pListener->GetString().isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testImageCommentAtChar)
{
    // Load a document with an at-char image in it (and a comment on the image).
    load(mpTestDocumentPath, "image-comment-at-char.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();

    // Verify that we have an annotation mark (comment with a text range) in the document.
    // Without the accompanying fix in place, this test would have failed, as comments lost their
    // ranges on load when their range only covered the placeholder character of the comment (which
    // is also the anchor position of the image).
    IDocumentMarkAccess* pMarks = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pMarks->getAnnotationMarksCount());

    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment-start portion was after the commented image.
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));

    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // 5892; Actual: 1738', i.e. the anchor pos was between the "aaa" and "bbb" portions, not at the
    // center of the page (horizontally) where the image is.  On macOS, though, with the fix in
    // place the actual value consistently is even greater with 6283 now instead of 5892, for
    // whatever reason.
    SwView* pView = pDoc->GetDocShell()->GetView();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (const auto& pItem : *pPostItMgr)
    {
        const SwRect& rAnchor = pItem->pPostIt->GetAnchorRect();
        CPPUNIT_ASSERT_GREATEREQUAL(static_cast<long>(5892), rAnchor.Left());
    }
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testViewCursorPageStyle)
{
    // Load a document with 2 pages, but a single paragraph.
    load(mpTestDocumentPath, "view-cursor-page-style.fodt");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<text::XTextViewCursorSupplier> xController(xModel->getCurrentController(),
                                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<text::XPageCursor> xViewCursor(xController->getViewCursor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xViewCursor.is());

    // Go to the first page, which has an explicit page style.
    xViewCursor->jumpToPage(1);
    OUString aActualPageStyleName = getProperty<OUString>(xViewCursor, "PageStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aActualPageStyleName);

    // Go to the second page, which is still the first paragraph, but the page style is different,
    // as the explicit 'First Page' page style has a next style defined (Standard).
    xViewCursor->jumpToPage(2);
    aActualPageStyleName = getProperty<OUString>(xViewCursor, "PageStyleName");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Standard
    // - Actual  : First Page
    // i.e. the cursor position was determined only based on the node index, ignoring the content
    // index.
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), aActualPageStyleName);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testXTextCursor_setPropertyValues)
{
    // Create a new document, type a character, pass a set of property/value pairs consisting of one
    // unknown property and CharStyleName, assert that it threw UnknownPropertyException (actually
    // wrapped into WrappedTargetException), and assert the style was set, not discarded.
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), "x", false);

    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    xCursor->goLeft(1, true);

    uno::Reference<beans::XMultiPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    uno::Sequence<OUString> aPropNames = { "OneUnknownProperty", "CharStyleName" };
    uno::Sequence<uno::Any> aPropValues = { uno::Any(), uno::Any(OUString("Emphasis")) };
    CPPUNIT_ASSERT_THROW(xCursorProps->setPropertyValues(aPropNames, aPropValues),
                         lang::WrappedTargetException);
    CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"),
                         getProperty<OUString>(xCursorProps, "CharStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testShapeAllowOverlap)
{
    // Test the AllowOverlap frame/shape property.

    // Create a new document and insert a rectangle.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(10000, 10000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // The property is on by default, turn it off & verify.
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::makeAny(false));
    CPPUNIT_ASSERT(!getProperty<bool>(xShapeProperties, "AllowOverlap"));

    // Turn it back to on & verify.
    xShapeProperties->setPropertyValue("AllowOverlap", uno::makeAny(true));
    CPPUNIT_ASSERT(getProperty<bool>(xShapeProperties, "AllowOverlap"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTextConvertToTableLineSpacing)
{
    // Load a document which has a table with a single cell.
    // The cell has both a table style and a paragraph style, with different line spacing
    // heights.
    load(mpTestDocumentPath, "table-line-spacing.docx");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCellText);
    style::LineSpacing aLineSpacing
        = getProperty<style::LineSpacing>(xParagraph, "ParaLineSpacing");
    // Make sure that we take the line spacing from the paragraph style, not from the table style.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 388
    // - Actual  : 635
    // I.e. the 360 twips line spacing was taken from the table style, not the 220 twips one from
    // the paragraph style.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(convertTwipToMm100(220)), aLineSpacing.Height);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
