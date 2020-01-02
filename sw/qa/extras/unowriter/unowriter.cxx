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
#include <comphelper/propertyvalue.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <ndtxt.hxx>
#include <IDocumentMarkAccess.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <AnnotationWin.hxx>
#include <view.hxx>

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

DECLARE_UNOAPI_TEST(testDefaultCharStyle)
{
    // Create a new document, type a character, set its char style to Emphasis
    // and assert the style was set.
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    xBodyText->insertString(xBodyText->getStart(), "x", false);

    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    xCursor->goLeft(1, true);

    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue("CharStyleName", uno::makeAny(OUString("Emphasis")));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));

    // Now reset the char style and assert that the font slant is back to none.
    // This resulted in a lang.IllegalArgumentException, Standard was not
    // mapped to 'Default Style'.
    xCursorProps->setPropertyValue("CharStyleName", uno::makeAny(OUString("Standard")));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));
}

DECLARE_UNOAPI_TEST(testGraphicDesciptorURL)
{
    loadURL("private:factory/swriter", nullptr);

    // Create a graphic object, but don't insert it yet.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);

    // Set an URL on it.
    OUString aGraphicURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "test.jpg";
    xTextGraphic->setPropertyValue("GraphicURL", uno::makeAny(aGraphicURL));
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed, the graphic object had no graphic.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_UNOAPI_TEST(testGraphicDesciptorURLBitmap)
{
    loadURL("private:factory/swriter", nullptr);

    // Load a bitmap into the bitmap table.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);
    OUString aGraphicURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "test.jpg";
    xBitmaps->insertByName("test", uno::makeAny(aGraphicURL));

    // Create a graphic.
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("GraphicURL", xBitmaps->getByName("test"));
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed: setting GraphicURL to the result of getByName() did not
    // work anymore.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

static bool ensureAutoTextExistsByTitle(const Reference<XAutoTextGroup>& autoTextGroup,
                                        const OUString& autoTextName)
{
    uno::Sequence<OUString> aTitles(autoTextGroup->getTitles());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

static bool ensureAutoTextExistsByName(const Reference<XAutoTextGroup>& autoTextGroup,
                                       const OUString& autoTextName)
{
    uno::Sequence<OUString> aTitles(autoTextGroup->getElementNames());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

DECLARE_UNOAPI_TEST_FILE(testXAutoTextGroup, "xautotextgroup.odt")
{
    Reference<XAutoTextContainer> xAutoTextContainer
        = AutoTextContainer::create(comphelper::getProcessComponentContext());

    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    const OUString sGroupName = "TestGroup*1";
    const OUString sTextName = "TEST";
    const OUString sTextNameNew = "TESTRENAMED";
    const OUString sTextTitle = "Test Auto Text";
    const OUString sTextTitleNew = "Test Auto Text Renamed";

    // Create new temporary group
    Reference<XAutoTextGroup> xAutoTextGroup(xAutoTextContainer->insertNewByName(sGroupName),
                                             uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("AutoTextGroup was not found!", xAutoTextGroup.is());

    // Insert new element and ensure it exists
    Reference<XAutoTextEntry> xAutoTextEntry
        = xAutoTextGroup->insertNewByName(sTextName, sTextTitle, xTextRange);
    CPPUNIT_ASSERT_MESSAGE("AutoText was not inserted!", xAutoTextEntry.is());
    CPPUNIT_ASSERT_MESSAGE("Can't find newly created AutoText by title!",
                           ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitle));
    CPPUNIT_ASSERT_MESSAGE("Can't find newly created AutoText by name!",
                           ensureAutoTextExistsByName(xAutoTextGroup, sTextName));

    // Insert once again the same should throw an exception
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on insertion of same AutoText",
                                 xAutoTextGroup->insertNewByName(sTextName, sTextTitle, xTextRange),
                                 container::ElementExistException);

    // Rename it & ensure everything is ok
    xAutoTextGroup->renameByName(sTextName, sTextNameNew, sTextTitleNew);
    CPPUNIT_ASSERT_MESSAGE("Can't find renamed AutoText by title!",
                           ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitleNew));
    CPPUNIT_ASSERT_MESSAGE("Can't find renamed AutoText by name!",
                           ensureAutoTextExistsByName(xAutoTextGroup, sTextNameNew));
    // Not found by old names
    CPPUNIT_ASSERT_MESSAGE("Found AutoText by old title!",
                           !ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitle));
    CPPUNIT_ASSERT_MESSAGE("Found AutoText by old name!",
                           !ensureAutoTextExistsByName(xAutoTextGroup, sTextName));

    // Rename not existing should throw an exception
    CPPUNIT_ASSERT_THROW_MESSAGE(
        "We expect an exception on renaming not-exising AutoText",
        xAutoTextGroup->renameByName(sTextName, sTextNameNew, sTextTitleNew),
        container::ElementExistException);

    // Remove it and ensure it does not exist
    xAutoTextGroup->removeByName(sTextNameNew);
    CPPUNIT_ASSERT_MESSAGE("AutoText was not removed!",
                           !ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitleNew));
    CPPUNIT_ASSERT_MESSAGE("AutoText was not removed!",
                           !ensureAutoTextExistsByName(xAutoTextGroup, sTextNameNew));

    // Remove non-existing element should throw an exception
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on removing not-existing AutoText",
                                 xAutoTextGroup->removeByName(sTextName),
                                 container::NoSuchElementException);

    // Remove our temporary group
    xAutoTextContainer->removeByName(sGroupName);
}

DECLARE_UNOAPI_TEST(testXURI)
{
    Reference<XComponentContext> xContext(::comphelper::getProcessComponentContext());

    // createKnown()
    Reference<rdf::XURI> xURIcreateKnown(rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX),
                                         UNO_SET_THROW);
    CPPUNIT_ASSERT(xURIcreateKnown.is());
    CPPUNIT_ASSERT_EQUAL(OUString("http://docs.oasis-open.org/ns/office/1.2/meta/odf#"),
                         xURIcreateKnown->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("prefix"), xURIcreateKnown->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://docs.oasis-open.org/ns/office/1.2/meta/odf#prefix"),
                         xURIcreateKnown->getStringValue());

    // createKnown() with invalid constant
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid constant",
                                 rdf::URI::createKnown(xContext, 12345),
                                 lang::IllegalArgumentException);

    // create()
    Reference<rdf::XURI> xURIcreate(rdf::URI::create(xContext, "http://example.com/url#somedata"),
                                    UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#"), xURIcreate->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreate->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#somedata"), xURIcreate->getStringValue());

    // create() without local name splitted with "/"
    Reference<rdf::XURI> xURIcreate2(rdf::URI::create(xContext, "http://example.com/url"),
                                     UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/"), xURIcreate2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("url"), xURIcreate2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url"), xURIcreate2->getStringValue());

    // create() without prefix
    Reference<rdf::XURI> xURIcreate3(rdf::URI::create(xContext, "#somedata"), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("#"), xURIcreate3->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreate3->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("#somedata"), xURIcreate3->getStringValue());

    // create() with invalid URI
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::create(xContext, "some junk and not URI"),
                                 lang::IllegalArgumentException);

    // createNS()
    Reference<rdf::XURI> xURIcreateNS(
        rdf::URI::createNS(xContext, "http://example.com/url#", "somedata"), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#"), xURIcreateNS->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreateNS->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#somedata"),
                         xURIcreateNS->getStringValue());

    // TODO: What's going on here? Is such usecase valid?
    Reference<rdf::XURI> xURIcreateNS2(
        rdf::URI::createNS(xContext, "http://example.com/url", "somedata"), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/"), xURIcreateNS2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("urlsomedata"), xURIcreateNS2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/urlsomedata"),
                         xURIcreateNS2->getStringValue());

    // createNS() some invalid cases
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::createNS(xContext, "bla", "bla"),
                                 lang::IllegalArgumentException);

    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::createNS(xContext, OUString(), OUString()),
                                 lang::IllegalArgumentException);
}

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

DECLARE_UNOAPI_TEST(testImageCommentAtChar)
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTransparentText)
{
    // Test the CharTransparence text portion property.

    // Create a new document.
    loadURL("private:factory/swriter", nullptr);

    // Set a custom transparency.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    sal_Int16 nExpected = 42;
    xParagraph->setPropertyValue("CharTransparence", uno::makeAny(nExpected));

    // Get the transparency & verify.
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xParagraph, "CharTransparence"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
