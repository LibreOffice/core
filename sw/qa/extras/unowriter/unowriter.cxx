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
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/text/XTextPortionAppend.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XPasteListener.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XPageCursor.hpp>

#include <comphelper/propertyvalue.hxx>
#include <tools/UnitConversion.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <AnnotationWin.hxx>
#include <flyfrm.hxx>
#include <fmtanchr.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/unowriter/data/";

/// Listener implementation for testPasteListener.
class PasteListener : public cppu::WeakImplHelper<text::XPasteListener>
{
    OUString m_aString;
    uno::Reference<text::XTextContent> m_xTextGraphicObject;

public:
    void SAL_CALL notifyPasteEvent(const uno::Sequence<beans::PropertyValue>& rEvent) override;

    OUString& GetString();
    uno::Reference<text::XTextContent>& GetTextGraphicObject();
};

void PasteListener::notifyPasteEvent(const uno::Sequence<beans::PropertyValue>& rEvent)
{
    comphelper::SequenceAsHashMap aMap(rEvent);
    auto it = aMap.find("TextRange");
    if (it != aMap.end())
    {
        auto xTextRange = it->second.get<uno::Reference<text::XTextRange>>();
        if (xTextRange.is())
            m_aString = xTextRange->getString();
        return;
    }

    it = aMap.find("TextGraphicObject");
    if (it != aMap.end())
    {
        auto xTextGraphicObject = it->second.get<uno::Reference<text::XTextContent>>();
        if (xTextGraphicObject.is())
            m_xTextGraphicObject = xTextGraphicObject;
    }
}

OUString& PasteListener::GetString() { return m_aString; }

uno::Reference<text::XTextContent>& PasteListener::GetTextGraphicObject()
{
    return m_xTextGraphicObject;
}
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testDefaultCharStyle)
{
    // Create a new document, type a character, set its char style to Emphasis
    // and assert the style was set.
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertStringExpandsHints)
{
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, "ab", false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xProps, "CharPosture"));
    xProps->setPropertyValue("CharPosture", uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    xText->insertString(xCursor, "x", false);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(xProps, "CharPosture"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertTextPortionNotExpandsHints)
{
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextPortionAppend> const xTextA(xText, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, "ab", false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xProps, "CharPosture"));
    xProps->setPropertyValue("CharPosture", uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    xTextA->insertTextPortion("x", uno::Sequence<beans::PropertyValue>(), xCursor);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xProps, "CharPosture"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertTextContentExpandsHints)
{
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XMultiServiceFactory> const xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, "ab", false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xProps, "CharPosture"));
    xProps->setPropertyValue("CharPosture", uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    uno::Reference<text::XTextContent> const xContent(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, false);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(xProps, "CharPosture"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertTextContentWithPropertiesNotExpandsHints)
{
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XMultiServiceFactory> const xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextContentAppend> const xTextA(xText, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, "ab", false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xProps, "CharPosture"));
    xProps->setPropertyValue("CharPosture", uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    uno::Reference<text::XTextContent> const xContent(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    xTextA->insertTextContentWithProperties(xContent, uno::Sequence<beans::PropertyValue>(),
                                            xCursor);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xProps, "CharPosture"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testGraphicDescriptorURL)
{
    loadURL("private:factory/swriter", nullptr);

    // Create a graphic object, but don't insert it yet.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);

    // Set a URL on it.
    OUString aGraphicURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "test.jpg";
    xTextGraphic->setPropertyValue("GraphicURL", uno::makeAny(aGraphicURL));
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed, the graphic object had no graphic.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testGraphicDescriptorURLBitmap)
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
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed: setting GraphicURL to the result of getByName() did not
    // work anymore.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

static bool ensureAutoTextExistsByTitle(const uno::Reference<text::XAutoTextGroup>& autoTextGroup,
                                        const OUString& autoTextName)
{
    const uno::Sequence<OUString> aTitles(autoTextGroup->getTitles());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

static bool ensureAutoTextExistsByName(const uno::Reference<text::XAutoTextGroup>& autoTextGroup,
                                       const OUString& autoTextName)
{
    const uno::Sequence<OUString> aTitles(autoTextGroup->getElementNames());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testXAutoTextGroup)
{
    load(mpTestDocumentPath, "xautotextgroup.odt");
    uno::Reference<text::XAutoTextContainer> xAutoTextContainer
        = text::AutoTextContainer::create(comphelper::getProcessComponentContext());

    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    const OUString sGroupName = "TestGroup*1";
    const OUString sTextName = "TEST";
    const OUString sTextNameNew = "TESTRENAMED";
    const OUString sTextTitle = "Test Auto Text";
    const OUString sTextTitleNew = "Test Auto Text Renamed";

    // Create new temporary group
    uno::Reference<text::XAutoTextGroup> xAutoTextGroup
        = xAutoTextContainer->insertNewByName(sGroupName);
    CPPUNIT_ASSERT_MESSAGE("AutoTextGroup was not found!", xAutoTextGroup.is());

    // Insert new element and ensure it exists
    uno::Reference<text::XAutoTextEntry> xAutoTextEntry
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
        "We expect an exception on renaming not-existing AutoText",
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testXURI)
{
    uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

    // createKnown()
    uno::Reference<rdf::XURI> xURIcreateKnown(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX), uno::UNO_SET_THROW);
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
    uno::Reference<rdf::XURI> xURIcreate(
        rdf::URI::create(xContext, "http://example.com/url#somedata"), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#"), xURIcreate->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreate->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#somedata"), xURIcreate->getStringValue());

    // create() without local name split with "/"
    uno::Reference<rdf::XURI> xURIcreate2(rdf::URI::create(xContext, "http://example.com/url"),
                                          uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/"), xURIcreate2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("url"), xURIcreate2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url"), xURIcreate2->getStringValue());

    // create() without prefix
    uno::Reference<rdf::XURI> xURIcreate3(rdf::URI::create(xContext, "#somedata"),
                                          uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("#"), xURIcreate3->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreate3->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("#somedata"), xURIcreate3->getStringValue());

    // create() with invalid URI
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::create(xContext, "some junk and not URI"),
                                 lang::IllegalArgumentException);

    // createNS()
    uno::Reference<rdf::XURI> xURIcreateNS(
        rdf::URI::createNS(xContext, "http://example.com/url#", "somedata"), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#"), xURIcreateNS->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreateNS->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#somedata"),
                         xURIcreateNS->getStringValue());

    // TODO: What's going on here? Is such usecase valid?
    uno::Reference<rdf::XURI> xURIcreateNS2(
        rdf::URI::createNS(xContext, "http://example.com/url", "somedata"), uno::UNO_SET_THROW);
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSetPagePrintSettings)
{
    // Create an empty new document with a single char
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), "x", false);

    uno::Reference<text::XPagePrintable> xPagePrintable(mxComponent, uno::UNO_QUERY);

    // set some stuff, try to get it back
    uno::Sequence<beans::PropertyValue> aProps(2);
    aProps[0].Name = "PageColumns";
    aProps[0].Value <<= sal_Int16(2);
    aProps[1].Name = "IsLandscape";
    aProps[1].Value <<= true;

    xPagePrintable->setPagePrintSettings(aProps);
    const comphelper::SequenceAsHashMap aMap(xPagePrintable->getPagePrintSettings());

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), aMap.getValue("PageColumns").get<short>());
    CPPUNIT_ASSERT_EQUAL(true, aMap.getValue("IsLandscape").get<bool>());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testDeleteFlyAtCharAtStart)
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* const pWrtShell(pTextDoc->GetDocShell()->GetWrtShell());
    SwDoc* const pDoc(pWrtShell->GetDoc());

    // insert some text
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.InsertString(*pWrtShell->GetCursor(), "foo bar baz");

    // insert fly anchored at start of body text
    pWrtShell->ClearMark();
    pWrtShell->SttEndDoc(true);
    SfxItemSet frameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>{});
    SfxItemSet grfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>{});
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    frameSet.Put(anchor);
    GraphicObject grf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pWrtShell->GetCursor(), grf, &frameSet, &grfSet));

    // check fly
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<text::XTextContent> const xShape(getShape(1), uno::UNO_QUERY);
    // anchored at start of body text?
    uno::Reference<text::XText> const xText(pTextDoc->getText());
    uno::Reference<text::XTextRangeCompare> const xTextRC(xText, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0),
                         xTextRC->compareRegionStarts(xText->getStart(), xShape->getAnchor()));

    // delete 1st character
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    xCursor->goRight(1, true);
    xCursor->setString("");

    // there is exactly one fly
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // select entire body text
    xCursor->gotoStart(true);
    xCursor->gotoEnd(true);
    xCursor->setString("");

    // there is no fly
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSelectionInTableEnum)
{
    load(mpTestDocumentPath, "selection-in-table-enum.odt");
    // Select the A1 cell's text.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    CPPUNIT_ASSERT_EQUAL(OUString("A1"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());

    // Access the selection.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelections.is());
    uno::Reference<text::XTextRange> xSelection(xSelections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelection.is());

    // Enumerate paragraphs in the selection.
    uno::Reference<container::XEnumerationAccess> xCursor(
        xSelection->getText()->createTextCursorByRange(xSelection), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCursor.is());
    uno::Reference<container::XEnumeration> xEnum = xCursor->createEnumeration();
    xEnum->nextElement();
    // Without the accompanying fix in place, this test would have failed: i.e.
    // the enumeration contained a second paragraph, even if the cell has only
    // one paragraph.
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSelectionInTableEnumEnd)
{
    load(mpTestDocumentPath, "selection-in-table-enum.odt");
    // Select from "Before" till the table end.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Down(/*bSelect=*/true);

    // Access the selection.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelections.is());
    uno::Reference<text::XTextRange> xSelection(xSelections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelection.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Before" SAL_NEWLINE_STRING "A1" SAL_NEWLINE_STRING
                                  "B1" SAL_NEWLINE_STRING "C2" SAL_NEWLINE_STRING
                                  "A2" SAL_NEWLINE_STRING "B2" SAL_NEWLINE_STRING
                                  "C2" SAL_NEWLINE_STRING),
                         xSelection->getString());

    // Enumerate paragraphs in the selection.
    uno::Reference<container::XEnumerationAccess> xCursor(
        xSelection->getText()->createTextCursorByRange(xSelection), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCursor.is());
    uno::Reference<container::XEnumeration> xEnum = xCursor->createEnumeration();
    // Before.
    xEnum->nextElement();
    // Table.
    xEnum->nextElement();
    // Without the accompanying fix in place, this test would have failed: i.e.
    // the enumeration contained the paragraph after the table, but no part of
    // that paragraph was part of the selection.
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testRenderablePagePosition)
{
    load(mpTestDocumentPath, "renderable-page-position.odt");
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
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_CHAR, eFlyAnchor);

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
        const SwRect& rAnchor = pItem->mpPostIt->GetAnchorRect();
        CPPUNIT_ASSERT_GREATEREQUAL(static_cast<tools::Long>(5892), rAnchor.Left());
    }
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testChapterNumberingCharStyle)
{
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<lang::XMultiServiceFactory> xDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(
        xDoc->createInstance("com.sun.star.style.CharacterStyle"), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xStyleN(xStyle, uno::UNO_QUERY);
    xStyle->setPropertyValue("CharColor", uno::makeAny(sal_Int32(0x00FF0000)));
    uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xStyles(
        xSFS->getStyleFamilies()->getByName("CharacterStyles"), uno::UNO_QUERY);
    xStyles->insertByName("red", uno::makeAny(xStyle));

    uno::Reference<text::XChapterNumberingSupplier> xCNS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexReplace> xOutline(xCNS->getChapterNumberingRules());
    {
        comphelper::SequenceAsHashMap hashMap(xOutline->getByIndex(0));
        hashMap["CharStyleName"] <<= OUString("red");
        uno::Sequence<beans::PropertyValue> props;
        hashMap >> props;
        xOutline->replaceByIndex(0, uno::makeAny(props));
    }
    // now rename the style
    xStyleN->setName("reddishred");
    {
        comphelper::SequenceAsHashMap hashMap(xOutline->getByIndex(0));

        // tdf#137810 this failed, was old value "red"
        CPPUNIT_ASSERT_EQUAL(OUString("reddishred"), hashMap["CharStyleName"].get<OUString>());
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testMultiSelect)
{
    // Create a new document and add a text with several repeated sequences.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, css::uno::UNO_QUERY_THROW);
    auto xSimpleText = xTextDocument->getText();
    xSimpleText->insertString(xSimpleText->getStart(), "Abc aBc abC", false);

    // Create a search descriptor and find all occurencies of search string
    css::uno::Reference<css::util::XSearchable> xSearchable(mxComponent, css::uno::UNO_QUERY_THROW);
    auto xSearchDescriptor = xSearchable->createSearchDescriptor();
    xSearchDescriptor->setPropertyValue("SearchStyles", css::uno::Any(false));
    xSearchDescriptor->setPropertyValue("SearchCaseSensitive", css::uno::Any(false));
    xSearchDescriptor->setPropertyValue("SearchBackwards", css::uno::Any(true));
    xSearchDescriptor->setPropertyValue("SearchRegularExpression", css::uno::Any(false));
    xSearchDescriptor->setSearchString("abc");
    auto xSearchResult = xSearchable->findAll(xSearchDescriptor);

    // Select them all
    auto xController = xTextDocument->getCurrentController();
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(
        xController, css::uno::UNO_QUERY_THROW);
    xSelectionSupplier->select(css::uno::Any(xSearchResult));
    css::uno::Reference<css::container::XIndexAccess> xSelection(xSelectionSupplier->getSelection(),
                                                                 css::uno::UNO_QUERY_THROW);
    // Now check that they all are selected in the reverse order ("SearchBackwards").
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSelection->getCount());
    css::uno::Reference<css::text::XTextRange> xTextRange(xSelection->getByIndex(0),
                                                          css::uno::UNO_QUERY_THROW);
    // For #0, result was empty (cursor was put before the last occurrence without selection)
    CPPUNIT_ASSERT_EQUAL(OUString("abC"), xTextRange->getString());
    xTextRange.set(xSelection->getByIndex(1), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("aBc"), xTextRange->getString());
    xTextRange.set(xSelection->getByIndex(2), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Abc"), xTextRange->getString());
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf129839)
{
    // Create a new document and add a table
    loadURL("private:factory/swriter", nullptr);
    css::uno::Reference<css::text::XTextDocument> xTextDocument(mxComponent,
                                                                css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::lang::XMultiServiceFactory> xFac(xTextDocument,
                                                              css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::text::XTextTable> xTable(
        xFac->createInstance("com.sun.star.text.TextTable"), css::uno::UNO_QUERY_THROW);
    xTable->initialize(4, 4);
    auto xSimpleText = xTextDocument->getText();
    xSimpleText->insertTextContent(xSimpleText->createTextCursor(), xTable, true);
    css::uno::Reference<css::table::XCellRange> xTableCellRange(xTable, css::uno::UNO_QUERY_THROW);
    // Get instance of SwXCellRange
    css::uno::Reference<css::beans::XPropertySet> xCellRange(
        xTableCellRange->getCellRangeByPosition(0, 0, 1, 1), css::uno::UNO_QUERY_THROW);
    // Test retrieval of VertOrient property - this crashed
    css::uno::Any aOrient = xCellRange->getPropertyValue("VertOrient");
    CPPUNIT_ASSERT_EQUAL(css::uno::Any(css::text::VertOrientation::NONE), aOrient);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf129841)
{
    // Create a new document and add a table
    loadURL("private:factory/swriter", nullptr);
    css::uno::Reference<css::text::XTextDocument> xTextDocument(mxComponent,
                                                                css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::lang::XMultiServiceFactory> xFac(xTextDocument,
                                                              css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::text::XTextTable> xTable(
        xFac->createInstance("com.sun.star.text.TextTable"), css::uno::UNO_QUERY_THROW);
    xTable->initialize(4, 4);
    auto xSimpleText = xTextDocument->getText();
    xSimpleText->insertTextContent(xSimpleText->createTextCursor(), xTable, true);
    // Get SwXTextTableCursor
    css::uno::Reference<css::beans::XPropertySet> xTableCursor(xTable->createCursorByCellName("A1"),
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XCellRange> xTableCellRange(xTable, css::uno::UNO_QUERY_THROW);
    // Get SwXCellRange for the same cell
    css::uno::Reference<css::beans::XPropertySet> xCellRange(
        xTableCellRange->getCellRangeByName("A1:A1"), css::uno::UNO_QUERY_THROW);
    const OUString sBackColor = "BackColor";
    // Apply background color to table cursor, and read background color from cell range
    css::uno::Any aRefColor(sal_Int32(0x00FF0000));
    xTableCursor->setPropertyValue(sBackColor, aRefColor);
    css::uno::Any aColor = xCellRange->getPropertyValue(sBackColor);
    // This failed
    CPPUNIT_ASSERT_EQUAL(aRefColor, aColor);
    // Now the other way round
    aRefColor <<= sal_Int32(0x0000FF00);
    xCellRange->setPropertyValue(sBackColor, aRefColor);
    aColor = xTableCursor->getPropertyValue(sBackColor);
    CPPUNIT_ASSERT_EQUAL(aRefColor, aColor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
