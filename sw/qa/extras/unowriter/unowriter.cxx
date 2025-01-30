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
#include <com/sun/star/datatransfer/XTransferableSupplier.hpp>
#include <com/sun/star/datatransfer/XTransferableTextSupplier.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
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
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XPageCursor.hpp>

#include <comphelper/propertyvalue.hxx>
#include <tools/UnitConversion.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/compbase.hxx>

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
    auto it = aMap.find(u"TextRange"_ustr);
    if (it != aMap.end())
    {
        auto xTextRange = it->second.get<uno::Reference<text::XTextRange>>();
        if (xTextRange.is())
            m_aString = xTextRange->getString();
        return;
    }

    it = aMap.find(u"TextGraphicObject"_ustr);
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

/// Test to assert UNO API call results of Writer.
class SwUnoWriter : public SwModelTestBase
{
public:
    SwUnoWriter()
        : SwModelTestBase(u"/sw/qa/extras/unowriter/data/"_ustr, u"writer8"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testDefaultCharStyle)
{
    // Create a new document, type a character, set its char style to Emphasis
    // and assert the style was set.
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), u"x"_ustr, false);

    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    xCursor->goLeft(1, true);

    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue(u"CharStyleName"_ustr, uno::Any(u"Emphasis"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xCursorProps, u"CharPosture"_ustr));

    // Now reset the char style and assert that the font slant is back to none.
    // This resulted in a lang.IllegalArgumentException, Standard was not
    // mapped to 'Default Style'.
    xCursorProps->setPropertyValue(u"CharStyleName"_ustr, uno::Any(u"Standard"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xCursorProps, u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertStringExpandsHints)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, u"ab"_ustr, false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
    xProps->setPropertyValue(u"CharPosture"_ustr, uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    xText->insertString(xCursor, u"x"_ustr, false);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertTextPortionNotExpandsHints)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextPortionAppend> const xTextA(xText, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, u"ab"_ustr, false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
    xProps->setPropertyValue(u"CharPosture"_ustr, uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    xTextA->insertTextPortion(u"x"_ustr, uno::Sequence<beans::PropertyValue>(), xCursor);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertTextContentExpandsHints)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XMultiServiceFactory> const xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, u"ab"_ustr, false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
    xProps->setPropertyValue(u"CharPosture"_ustr, uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    uno::Reference<text::XTextContent> const xContent(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, false);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testInsertTextContentWithPropertiesNotExpandsHints)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> const xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XMultiServiceFactory> const xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextContentAppend> const xTextA(xText, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    uno::Reference<beans::XPropertySet> const xProps(xCursor, uno::UNO_QUERY);

    xText->insertString(xCursor, u"ab"_ustr, false);
    xCursor->gotoStart(false);
    xCursor->goRight(1, true);
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
    xProps->setPropertyValue(u"CharPosture"_ustr, uno::Any(awt::FontSlant_ITALIC));
    xCursor->collapseToEnd();
    uno::Reference<text::XTextContent> const xContent(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    xTextA->insertTextContentWithProperties(xContent, uno::Sequence<beans::PropertyValue>(),
                                            xCursor);
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xProps, u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testGraphicDescriptorURL)
{
    createSwDoc();

    // Create a graphic object, but don't insert it yet.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);

    // Set a URL on it.
    xTextGraphic->setPropertyValue(u"GraphicURL"_ustr, uno::Any(createFileURL(u"test.jpg")));
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed, the graphic object had no graphic.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), u"Graphic"_ustr);
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testGraphicDescriptorURLBitmap)
{
    createSwDoc();

    // Load a bitmap into the bitmap table.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance(u"com.sun.star.drawing.BitmapTable"_ustr), uno::UNO_QUERY);
    xBitmaps->insertByName(u"test"_ustr, uno::Any(createFileURL(u"test.jpg")));

    // Create a graphic.
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"GraphicURL"_ustr, xBitmaps->getByName(u"test"_ustr));
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AT_CHARACTER));

    // Insert it.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // This failed: setting GraphicURL to the result of getByName() did not
    // work anymore.
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), u"Graphic"_ustr);
    CPPUNIT_ASSERT(xGraphic.is());
}

bool ensureAutoTextExistsByTitle(const uno::Reference<text::XAutoTextGroup>& autoTextGroup,
                                 std::u16string_view autoTextName)
{
    const uno::Sequence<OUString> aTitles(autoTextGroup->getTitles());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

bool ensureAutoTextExistsByName(const uno::Reference<text::XAutoTextGroup>& autoTextGroup,
                                std::u16string_view autoTextName)
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
    createSwDoc("xautotextgroup.odt");
    uno::Reference<text::XAutoTextContainer> xAutoTextContainer
        = text::AutoTextContainer::create(comphelper::getProcessComponentContext());

    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    static constexpr OUString sGroupName = u"TestGroup*1"_ustr;
    static constexpr OUString sTextName = u"TEST"_ustr;
    static constexpr OUString sTextNameNew = u"TESTRENAMED"_ustr;
    static constexpr OUString sTextTitle = u"Test Auto Text"_ustr;
    static constexpr OUString sTextTitleNew = u"Test Auto Text Renamed"_ustr;

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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSectionAnchorCopyTableAtStart)
{
    // this contains a section that starts with a table
    createSwDoc("tdf134250.fodt");

    uno::Reference<text::XTextTablesSupplier> const xTextTablesSupplier(mxComponent,
                                                                        uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xTables(xTextTablesSupplier->getTextTables(),
                                                          uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    uno::Reference<text::XTextSectionsSupplier> const xTextSectionsSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xSections(
        xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    uno::Reference<text::XTextContent> const xSection(xSections->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> const xAnchor(xSection->getAnchor());
    CPPUNIT_ASSERT_EQUAL(u"foo" SAL_NEWLINE_STRING "bar"_ustr, xAnchor->getString());

    // copy the content of the section to a clipboard document
    uno::Reference<datatransfer::XTransferableSupplier> const xTS(
        uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY_THROW)->getCurrentController(),
        uno::UNO_QUERY);
    uno::Reference<datatransfer::XTransferableTextSupplier> const xTTS(xTS, uno::UNO_QUERY);
    uno::Reference<datatransfer::XTransferable> const xTransferable(
        xTTS->getTransferableForTextRange(xAnchor));

    // check this doesn't throw
    CPPUNIT_ASSERT(xAnchor->getText().is());
    CPPUNIT_ASSERT(xAnchor->getStart().is());
    CPPUNIT_ASSERT(xAnchor->getEnd().is());

    // replace section content
    xAnchor->setString(u"quux"_ustr);

    // table in section was deleted, but not section itself
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"\""
                         "quux" /*SAL_NEWLINE_STRING*/ "\""_ustr,
                         OUString("\"" + xAnchor->getString() + "\""));

    // now paste it
    uno::Reference<text::XTextViewCursorSupplier> const xTVCS(xTS, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursor> const xCursor(xTVCS->getViewCursor());
    xCursor->gotoEnd(false);
    xTS->insertTransferable(xTransferable);

    // table in section was pasted, but not section itself
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    xCursor->gotoStart(true);
    CPPUNIT_ASSERT_EQUAL(u"quux" SAL_NEWLINE_STRING "foo" SAL_NEWLINE_STRING "bar"_ustr,
                         xCursor->getString());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSectionAnchorCopyTableAtEnd)
{
    // this contains a section that ends with a table (plus another section)
    createSwDoc("tdf134252.fodt");

    uno::Reference<text::XTextTablesSupplier> const xTextTablesSupplier(mxComponent,
                                                                        uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xTables(xTextTablesSupplier->getTextTables(),
                                                          uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    uno::Reference<text::XTextSectionsSupplier> const xTextSectionsSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xSections(
        xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());

    uno::Reference<text::XTextContent> const xSection(xSections->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> const xAnchor(xSection->getAnchor());
    CPPUNIT_ASSERT_EQUAL(u"bar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING ""_ustr,
                         xAnchor->getString());

    // copy the content of the section to a clipboard document
    uno::Reference<datatransfer::XTransferableSupplier> const xTS(
        uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY_THROW)->getCurrentController(),
        uno::UNO_QUERY);
    uno::Reference<datatransfer::XTransferableTextSupplier> const xTTS(xTS, uno::UNO_QUERY);
    uno::Reference<datatransfer::XTransferable> const xTransferable(
        xTTS->getTransferableForTextRange(xAnchor));

    // check this doesn't throw
    CPPUNIT_ASSERT(xAnchor->getText().is());
    CPPUNIT_ASSERT(xAnchor->getStart().is());
    CPPUNIT_ASSERT(xAnchor->getEnd().is());

    // replace section content
    xAnchor->setString(u"quux"_ustr);

    // table in section was deleted, but not section itself
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"\""
                         "quux" /*SAL_NEWLINE_STRING*/ "\""_ustr,
                         OUString("\"" + xAnchor->getString() + "\""));

    // now paste it
    uno::Reference<text::XTextViewCursorSupplier> const xTVCS(xTS, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursor> const xCursor(xTVCS->getViewCursor());
    xCursor->gotoEnd(false);
    xTS->insertTransferable(xTransferable);

    // table in section was pasted, but not section itself
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
    // note: this selects the 2nd section because it calls StartOfSection()
    // not SttEndDoc() like it should?
    xCursor->gotoStart(true);
    CPPUNIT_ASSERT_EQUAL(u"foobar" SAL_NEWLINE_STRING "baz" SAL_NEWLINE_STRING ""_ustr,
                         xCursor->getString());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSectionAnchorCopyTable)
{
    // this contains a section that ends with a table (plus another section)
    createSwDoc("tdf134252_onlytable_protected.fodt");

    uno::Reference<text::XTextTablesSupplier> const xTextTablesSupplier(mxComponent,
                                                                        uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xTables(xTextTablesSupplier->getTextTables(),
                                                          uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    uno::Reference<text::XTextSectionsSupplier> const xTextSectionsSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xSections(
        xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    uno::Reference<text::XTextContent> const xSection(xSections->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> const xAnchor(xSection->getAnchor());
    CPPUNIT_ASSERT_EQUAL(u"baz" SAL_NEWLINE_STRING ""_ustr, xAnchor->getString());

    // copy the content of the section to a clipboard document
    uno::Reference<datatransfer::XTransferableSupplier> const xTS(
        uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY_THROW)->getCurrentController(),
        uno::UNO_QUERY);
    uno::Reference<datatransfer::XTransferableTextSupplier> const xTTS(xTS, uno::UNO_QUERY);
    uno::Reference<datatransfer::XTransferable> const xTransferable(
        xTTS->getTransferableForTextRange(xAnchor));

    // check this doesn't throw
    CPPUNIT_ASSERT(xAnchor->getText().is());
    CPPUNIT_ASSERT(xAnchor->getStart().is());
    CPPUNIT_ASSERT(xAnchor->getEnd().is());

    // replace section content
    xAnchor->setString(u"quux"_ustr);

    // table in section was deleted, but not section itself
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    CPPUNIT_ASSERT_EQUAL(u"\""
                         "quux" /*SAL_NEWLINE_STRING*/ "\""_ustr,
                         OUString("\"" + xAnchor->getString() + "\""));

    // now paste it
    uno::Reference<text::XTextViewCursorSupplier> const xTVCS(xTS, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursor> const xCursor(xTVCS->getViewCursor());
    xCursor->gotoEnd(false);
    xTS->insertTransferable(xTransferable);

    // table in section was pasted, but not section itself
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
    xCursor->gotoStart(true);
    CPPUNIT_ASSERT_EQUAL(u"quux" SAL_NEWLINE_STRING "foo" SAL_NEWLINE_STRING
                         "baz" SAL_NEWLINE_STRING ""_ustr,
                         xCursor->getString());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSectionAnchorProperties)
{
    createSwDoc("section-table.fodt");

    uno::Reference<text::XTextSectionsSupplier> const xTextSectionsSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> const xSections(
        xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    uno::Reference<text::XTextContent> const xSection(xSections->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> const xAnchor(xSection->getAnchor());
    uno::Reference<beans::XPropertySet> const xAnchorProp(xAnchor, uno::UNO_QUERY);

    // the problem was that the property set didn't work
    auto xSecFromProp = getProperty<uno::Reference<text::XTextContent>>(xAnchorProp, "TextSection");
    CPPUNIT_ASSERT_EQUAL(xSection, xSecFromProp);

    xAnchorProp->setPropertyValue("CharHeight", uno::Any(float(64)));
    CPPUNIT_ASSERT_EQUAL(float(64), getProperty<float>(xAnchorProp, "CharHeight"));
    uno::Reference<beans::XPropertyState> const xAnchorState(xAnchor, uno::UNO_QUERY);
    // TODO: why does this return DEFAULT_VALUE instead of DIRECT_VALUE?
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xAnchorState->getPropertyState("CharHeight"));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xAnchorState->getPropertyStates({ "CharHeight" })[0]);
    CPPUNIT_ASSERT_EQUAL(float(12), xAnchorState->getPropertyDefault("CharHeight").get<float>());
    xAnchorState->setPropertyToDefault("CharHeight");
    CPPUNIT_ASSERT_EQUAL(float(12), getProperty<float>(xAnchorProp, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTextRangeInTable)
{
    createSwDoc("bookmarkintable.fodt");

    uno::Reference<text::XBookmarksSupplier> const xBS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> const xMarks(xBS->getBookmarks());
    uno::Reference<text::XTextContent> const xMark(xMarks->getByName(u"Bookmark 1"_ustr),
                                                   uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> const xAnchor(xMark->getAnchor(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> const xEnum(xAnchor->createEnumeration());
    uno::Reference<lang::XServiceInfo> const xPara(xEnum->nextElement(), uno::UNO_QUERY);
    // not the top-level table!
    CPPUNIT_ASSERT(!xPara->supportsService(u"com.sun.star.text.TextTable"_ustr));
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());
    uno::Reference<container::XEnumerationAccess> const xParaEA(xPara, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> const xPortions(xParaEA->createEnumeration());
    uno::Reference<beans::XPropertySet> const xP1(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(xP1, u"TextPortionType"_ustr));
    uno::Reference<beans::XPropertySet> const xP2(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xP2, u"TextPortionType"_ustr));
    uno::Reference<text::XTextRange> const xP2R(xP2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, xP2R->getString());
    uno::Reference<beans::XPropertySet> const xP3(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(xP3, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testXURI)
{
    uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

    // createKnown()
    uno::Reference<rdf::XURI> xURIcreateKnown(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xURIcreateKnown.is());
    CPPUNIT_ASSERT_EQUAL(u"http://docs.oasis-open.org/ns/office/1.2/meta/odf#"_ustr,
                         xURIcreateKnown->getNamespace());
    CPPUNIT_ASSERT_EQUAL(u"prefix"_ustr, xURIcreateKnown->getLocalName());
    CPPUNIT_ASSERT_EQUAL(u"http://docs.oasis-open.org/ns/office/1.2/meta/odf#prefix"_ustr,
                         xURIcreateKnown->getStringValue());

    // createKnown() with invalid constant
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid constant",
                                 rdf::URI::createKnown(xContext, 12345),
                                 lang::IllegalArgumentException);

    // create()
    uno::Reference<rdf::XURI> xURIcreate(
        rdf::URI::create(xContext, u"http://example.com/url#somedata"_ustr), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/url#"_ustr, xURIcreate->getNamespace());
    CPPUNIT_ASSERT_EQUAL(u"somedata"_ustr, xURIcreate->getLocalName());
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/url#somedata"_ustr, xURIcreate->getStringValue());

    // create() without local name split with "/"
    uno::Reference<rdf::XURI> xURIcreate2(
        rdf::URI::create(xContext, u"http://example.com/url"_ustr), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/"_ustr, xURIcreate2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(u"url"_ustr, xURIcreate2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/url"_ustr, xURIcreate2->getStringValue());

    // create() without prefix
    uno::Reference<rdf::XURI> xURIcreate3(rdf::URI::create(xContext, u"#somedata"_ustr),
                                          uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(u"#"_ustr, xURIcreate3->getNamespace());
    CPPUNIT_ASSERT_EQUAL(u"somedata"_ustr, xURIcreate3->getLocalName());
    CPPUNIT_ASSERT_EQUAL(u"#somedata"_ustr, xURIcreate3->getStringValue());

    // create() with invalid URI
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::create(xContext, u"some junk and not URI"_ustr),
                                 lang::IllegalArgumentException);

    // createNS()
    uno::Reference<rdf::XURI> xURIcreateNS(
        rdf::URI::createNS(xContext, u"http://example.com/url#"_ustr, u"somedata"_ustr),
        uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/url#"_ustr, xURIcreateNS->getNamespace());
    CPPUNIT_ASSERT_EQUAL(u"somedata"_ustr, xURIcreateNS->getLocalName());
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/url#somedata"_ustr, xURIcreateNS->getStringValue());

    // TODO: What's going on here? Is such usecase valid?
    uno::Reference<rdf::XURI> xURIcreateNS2(
        rdf::URI::createNS(xContext, u"http://example.com/url"_ustr, u"somedata"_ustr),
        uno::UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/"_ustr, xURIcreateNS2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(u"urlsomedata"_ustr, xURIcreateNS2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/urlsomedata"_ustr, xURIcreateNS2->getStringValue());

    // createNS() some invalid cases
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::createNS(xContext, u"bla"_ustr, u"bla"_ustr),
                                 lang::IllegalArgumentException);

    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::createNS(xContext, OUString(), OUString()),
                                 lang::IllegalArgumentException);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSetPagePrintSettings)
{
    // Create an empty new document with a single char
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), u"x"_ustr, false);

    uno::Reference<text::XPagePrintable> xPagePrintable(mxComponent, uno::UNO_QUERY);

    // set some stuff, try to get it back
    uno::Sequence<beans::PropertyValue> aProps{
        comphelper::makePropertyValue(u"PageColumns"_ustr, sal_Int16(2)),
        comphelper::makePropertyValue(u"IsLandscape"_ustr, true)
    };

    xPagePrintable->setPagePrintSettings(aProps);
    const comphelper::SequenceAsHashMap aMap(xPagePrintable->getPagePrintSettings());

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), aMap.getValue(u"PageColumns"_ustr).get<short>());
    CPPUNIT_ASSERT_EQUAL(true, aMap.getValue(u"IsLandscape"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testDeleteFlyAtCharAtStart)
{
    createSwDoc();
    SwWrtShell* const pWrtShell(getSwDocShell()->GetWrtShell());
    SwDoc* const pDoc(pWrtShell->GetDoc());

    // insert some text
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());
    rIDCO.InsertString(*pWrtShell->GetCursor(), u"foo bar baz"_ustr);

    // insert fly anchored at start of body text
    pWrtShell->ClearMark();
    pWrtShell->SttEndDoc(true);
    SfxItemSet frameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SfxItemSet grfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>);
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    frameSet.Put(anchor);
    Graphic grf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphic(*pWrtShell->GetCursor(), OUString(), OUString(), &grf,
                                       &frameSet, &grfSet, nullptr));

    // check fly
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<text::XTextContent> const xShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    // anchored at start of body text?
    uno::Reference<text::XText> const xText(xTextDocument->getText());
    uno::Reference<text::XTextRangeCompare> const xTextRC(xText, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0),
                         xTextRC->compareRegionStarts(xText->getStart(), xShape->getAnchor()));

    // delete 1st character
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    xCursor->goRight(1, true);
    xCursor->setString(u""_ustr);

    // there is exactly one fly
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // select entire body text
    xCursor->gotoStart(true);
    xCursor->gotoEnd(true);
    xCursor->setString(u""_ustr);

    // there is no fly
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testSelectionInTableEnum)
{
    createSwDoc("selection-in-table-enum.odt");
    // Select the A1 cell's text.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    CPPUNIT_ASSERT_EQUAL(u"A1"_ustr,
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());

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
    createSwDoc("selection-in-table-enum.odt");
    // Select from "Before" till the table end.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    CPPUNIT_ASSERT_EQUAL(u"Before" SAL_NEWLINE_STRING "A1" SAL_NEWLINE_STRING
                         "B1" SAL_NEWLINE_STRING "C2" SAL_NEWLINE_STRING "A2" SAL_NEWLINE_STRING
                         "B2" SAL_NEWLINE_STRING "C2" SAL_NEWLINE_STRING ""_ustr,
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

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf62603)
{
    // Unit test for tdf#62603
    // Test to see if font style is retained when performing find/replace on strings
    // containing mixed font styles/sizes
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);

    // Set up test by inserting strings with different font style/sizes
    // Inserts 1st string containing quotation marks (") with no font style
    xText->insertString(xCursor, "\"", false);
    xCursor->gotoStart(true); // selects full string
    CPPUNIT_ASSERT_EQUAL(OUString("\""), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));
    xCursor->collapseToEnd();

    // Inserts 2nd string 'test' with italic font style
    xCursorProps->setPropertyValue("CharPosture", uno::Any(awt::FontSlant_ITALIC));
    xText->insertString(xCursor, "test", false);
    xCursor->goLeft(4, true); // selects 2nd string
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xCursor->getString());
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));
    xCursor->collapseToEnd();

    // Insert 3rd string '? ' with 28 pt font height
    xCursorProps->setPropertyValue("CharPosture", uno::Any(awt::FontSlant_NONE)); // no font style
    xCursorProps->setPropertyValue("CharHeight", uno::Any(float(28.0)));
    xText->insertString(xCursor, "? ", false);
    xCursor->goLeft(2, true); // selects 3rd string
    CPPUNIT_ASSERT_EQUAL(float(28.0), getProperty<float>(xCursorProps, "CharHeight"));
    xCursor->collapseToEnd();

    // Insert 4th string 'who' with default 12 pt font height
    xCursorProps->setPropertyValue("CharHeight", uno::Any(float(12.0)));
    xText->insertString(xCursor, "who", false);
    xCursor->goLeft(3, true); // selects 4rd string
    CPPUNIT_ASSERT_EQUAL(float(12.0), getProperty<float>(xCursorProps, "CharHeight"));
    xCursor->collapseToEnd();

    // Asserts that full string is properly inserted as: '"test? who'
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("\"test? who"), getParagraph(1)->getString());

    uno::Reference<util::XReplaceable> xReplace(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XReplaceDescriptor> xReplaceDesc(xReplace->createReplaceDescriptor());

    // Searches for "t and replaces with "gu
    // Note: Search string contains both no font style and italic font style
    xReplaceDesc->setSearchString("\"t");
    xReplaceDesc->setReplaceString("\"gu");
    xReplace->replaceAll(xReplaceDesc);

    // Search/replace adds extra space between ? and w
    // Note: Search string contains both 28 pt and 12 pt font sizes
    xReplaceDesc->setSearchString("? w");
    xReplaceDesc->setReplaceString("?  w");
    xReplace->replaceAll(xReplaceDesc);

    // Asserts that '"test? who' is replaced with '"guest?  who'
    CPPUNIT_ASSERT_EQUAL(OUString("\"guest?  who"), getParagraph(1)->getString());

    // Asserts no font style is on double quote mark (")
    CPPUNIT_ASSERT_EQUAL(
        awt::FontSlant_NONE,
        getProperty<awt::FontSlant>(getRun(getParagraph(1), 1, u"\""_ustr), "CharPosture"));

    // Asserts font style for 'guest' is italic
    // Without the test, 'g' and 'u' in 'guest' will change to no font style
    // - Expected: 2 // ITALIC
    // - Actual: 0 // NONE
    CPPUNIT_ASSERT_EQUAL(
        awt::FontSlant_ITALIC,
        getProperty<awt::FontSlant>(getRun(getParagraph(1), 2, u"guest"_ustr), "CharPosture"));

    // Asserts font size is 28 pt
    CPPUNIT_ASSERT_EQUAL(float(28.0),
                         getProperty<float>(getRun(getParagraph(1), 3, u"? "_ustr), "CharHeight"));

    // Asserts font size is 12 pt
    // Without the test, the space ' ' and 'w' will change to 28 pt font size
    // - Expected: 12
    // - Actual: 28
    CPPUNIT_ASSERT_EQUAL(
        float(12.0), getProperty<float>(getRun(getParagraph(1), 4, u" who"_ustr), "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testRenderablePagePosition)
{
    createSwDoc("renderable-page-position.odt");
    // Make sure that the document has 2 pages.

    uno::Any aSelection(mxComponent);

    uno::Reference<awt::XToolkit> xToolkit = VCLUnoHelper::CreateToolkit();
    uno::Reference<awt::XDevice> xDevice(xToolkit->createScreenCompatibleDevice(32, 32));

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController = xModel->getCurrentController();

    beans::PropertyValues aRenderOptions = {
        comphelper::makePropertyValue(u"IsPrinter"_ustr, true),
        comphelper::makePropertyValue(u"RenderDevice"_ustr, xDevice),
        comphelper::makePropertyValue(u"View"_ustr, xController),
        comphelper::makePropertyValue(u"RenderToGraphic"_ustr, true),
    };

    uno::Reference<view::XRenderable> xRenderable(mxComponent, uno::UNO_QUERY);
    sal_Int32 nPages = xRenderable->getRendererCount(aSelection, aRenderOptions);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), nPages);

    // Make sure that the first page has some offset.
    comphelper::SequenceAsHashMap aRenderer1(
        xRenderable->getRenderer(0, aSelection, aRenderOptions));
    // Without the accompanying fix in place, this test would have failed: i.e.
    // there was no PagePos key in this map.
    awt::Point aPosition1 = aRenderer1[u"PagePos"_ustr].get<awt::Point>();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition1.X);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition1.Y);

    // Make sure that the second page is below the first one.
    comphelper::SequenceAsHashMap aRenderer2(
        xRenderable->getRenderer(1, aSelection, aRenderOptions));
    awt::Point aPosition2 = aRenderer2[u"PagePos"_ustr].get<awt::Point>();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), aPosition2.X);
    CPPUNIT_ASSERT_GREATER(aPosition1.Y, aPosition2.Y);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testPasteListener)
{
    createSwDoc();

    // Insert initial string.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), u"ABCDEF"_ustr, false);

    // Add paste listener.
    uno::Reference<text::XPasteBroadcaster> xBroadcaster(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XPasteListener> xListener(new PasteListener);
    auto pListener = static_cast<PasteListener*>(xListener.get());
    xBroadcaster->addPasteEventListener(xListener);

    // Cut "DE" and then paste it.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    TransferableDataHelper aHelper(pTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    // Without working listener registration in place, this test would have
    // failed with 'Expected: DE; Actual:', i.e. the paste listener was not
    // invoked.
    CPPUNIT_ASSERT_EQUAL(u"DE"_ustr, pListener->GetString());

    // Make sure that paste did not overwrite anything.
    CPPUNIT_ASSERT_EQUAL(u"ABCDEF"_ustr, xBodyText->getString());

    // Paste again, this time overwriting "BC".
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pListener->GetString().clear();
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(u"DE"_ustr, pListener->GetString());

    // Make sure that paste overwrote "BC".
    CPPUNIT_ASSERT_EQUAL(u"ADEDEF"_ustr, xBodyText->getString());

    // Test image paste.
    SwView& rView = pWrtShell->GetView();
    rView.InsertGraphic(createFileURL(u"test.jpg"), OUString(), /*bAsLink=*/false,
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
    createSwDoc("image-comment-at-char.odt");
    SwDoc* pDoc = getSwDoc();

    // Verify that we have an annotation mark (comment with a text range) in the document.
    // Without the accompanying fix in place, this test would have failed, as comments lost their
    // ranges on load when their range only covered the placeholder character of the comment (which
    // is also the anchor position of the image).
    IDocumentMarkAccess* pMarks = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pMarks->getAnnotationMarksCount());

    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment-start portion was after the commented image.
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                         getProperty<OUString>(getRun(xPara, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                         getProperty<OUString>(getRun(xPara, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                         getProperty<OUString>(getRun(xPara, 4), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 5), u"TextPortionType"_ustr));

    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // 5892; Actual: 1738', i.e. the anchor pos was between the "aaa" and "bbb" portions, not at the
    // center of the page (horizontally) where the image is.  On macOS, though, with the fix in
    // place the actual value consistently is even greater with 6283 now instead of 5892, for
    // whatever reason.
    SwView* pView = getSwDocShell()->GetView();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (const auto& pItem : *pPostItMgr)
    {
        const SwRect& rAnchor = pItem->mpPostIt->GetAnchorRect();
        CPPUNIT_ASSERT_GREATEREQUAL(static_cast<tools::Long>(5892), rAnchor.Left());
    }
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testChapterNumberingCharStyle)
{
    createSwDoc();

    uno::Reference<lang::XMultiServiceFactory> xDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(
        xDoc->createInstance(u"com.sun.star.style.CharacterStyle"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xStyleN(xStyle, uno::UNO_QUERY);
    xStyle->setPropertyValue(u"CharColor"_ustr, uno::Any(COL_LIGHTRED));
    uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xStyles(
        xSFS->getStyleFamilies()->getByName(u"CharacterStyles"_ustr), uno::UNO_QUERY);
    xStyles->insertByName(u"red"_ustr, uno::Any(xStyle));

    uno::Reference<text::XChapterNumberingSupplier> xCNS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexReplace> xOutline(xCNS->getChapterNumberingRules());
    {
        comphelper::SequenceAsHashMap hashMap(xOutline->getByIndex(0));
        hashMap[u"CharStyleName"_ustr] <<= u"red"_ustr;
        uno::Sequence<beans::PropertyValue> props;
        hashMap >> props;
        xOutline->replaceByIndex(0, uno::Any(props));
    }
    // now rename the style
    xStyleN->setName(u"reddishred"_ustr);
    {
        comphelper::SequenceAsHashMap hashMap(xOutline->getByIndex(0));

        // tdf#137810 this failed, was old value "red"
        CPPUNIT_ASSERT_EQUAL(u"reddishred"_ustr, hashMap[u"CharStyleName"_ustr].get<OUString>());
    }
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testViewCursorPageStyle)
{
    // Load a document with 2 pages, but a single paragraph.
    createSwDoc("view-cursor-page-style.fodt");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<text::XTextViewCursorSupplier> xController(xModel->getCurrentController(),
                                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<text::XPageCursor> xViewCursor(xController->getViewCursor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xViewCursor.is());

    // Go to the first page, which has an explicit page style.
    xViewCursor->jumpToPage(1);
    OUString aActualPageStyleName = getProperty<OUString>(xViewCursor, u"PageStyleName"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"First Page"_ustr, aActualPageStyleName);

    // Go to the second page, which is still the first paragraph, but the page style is different,
    // as the explicit 'First Page' page style has a next style defined (Standard).
    xViewCursor->jumpToPage(2);
    aActualPageStyleName = getProperty<OUString>(xViewCursor, u"PageStyleName"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Standard
    // - Actual  : First Page
    // i.e. the cursor position was determined only based on the node index, ignoring the content
    // index.
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, aActualPageStyleName);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testXTextCursor_setPropertyValues)
{
    // Create a new document, type a character, pass a set of property/value pairs consisting of one
    // unknown property and CharStyleName, assert that it threw UnknownPropertyException (actually
    // wrapped into WrappedTargetException), and assert the style was set, not discarded.
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText = xTextDocument->getText();
    xBodyText->insertString(xBodyText->getStart(), u"x"_ustr, false);

    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    xCursor->goLeft(1, true);

    uno::Reference<beans::XMultiPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    uno::Sequence<OUString> aPropNames = { u"OneUnknownProperty"_ustr, u"CharStyleName"_ustr };
    uno::Sequence<uno::Any> aPropValues = { uno::Any(), uno::Any(u"Emphasis"_ustr) };
    CPPUNIT_ASSERT_THROW(xCursorProps->setPropertyValues(aPropNames, aPropValues),
                         lang::WrappedTargetException);
    CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                         getProperty<OUString>(xCursorProps, u"CharStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testShapeAllowOverlap)
{
    // Test the AllowOverlap frame/shape property.

    // Create a new document and insert a rectangle.
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(10000, 10000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // The property is on by default, turn it off & verify.
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue(u"AllowOverlap"_ustr, uno::Any(false));
    CPPUNIT_ASSERT(!getProperty<bool>(xShapeProperties, u"AllowOverlap"_ustr));

    // Turn it back to on & verify.
    xShapeProperties->setPropertyValue(u"AllowOverlap"_ustr, uno::Any(true));
    CPPUNIT_ASSERT(getProperty<bool>(xShapeProperties, u"AllowOverlap"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTextConvertToTableLineSpacing)
{
    // Load a document which has a table with a single cell.
    // The cell has both a table style and a paragraph style, with different line spacing
    // heights.
    createSwDoc("table-line-spacing.docx");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
    uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCellText);
    style::LineSpacing aLineSpacing
        = getProperty<style::LineSpacing>(xParagraph, u"ParaLineSpacing"_ustr);
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
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, css::uno::UNO_QUERY_THROW);
    auto xSimpleText = xTextDocument->getText();
    xSimpleText->insertString(xSimpleText->getStart(), u"Abc aBc abC"_ustr, false);

    // Create a search descriptor and find all occurrences of search string
    css::uno::Reference<css::util::XSearchable> xSearchable(mxComponent, css::uno::UNO_QUERY_THROW);
    auto xSearchDescriptor = xSearchable->createSearchDescriptor();
    xSearchDescriptor->setPropertyValue(u"SearchStyles"_ustr, css::uno::Any(false));
    xSearchDescriptor->setPropertyValue(u"SearchCaseSensitive"_ustr, css::uno::Any(false));
    xSearchDescriptor->setPropertyValue(u"SearchBackwards"_ustr, css::uno::Any(true));
    xSearchDescriptor->setPropertyValue(u"SearchRegularExpression"_ustr, css::uno::Any(false));
    xSearchDescriptor->setSearchString(u"abc"_ustr);
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
    CPPUNIT_ASSERT_EQUAL(u"abC"_ustr, xTextRange->getString());
    xTextRange.set(xSelection->getByIndex(1), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"aBc"_ustr, xTextRange->getString());
    xTextRange.set(xSelection->getByIndex(2), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Abc"_ustr, xTextRange->getString());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTransparentText)
{
    // Test the CharTransparence text portion property.

    // Create a new document.
    createSwDoc();

    // Set a custom transparency.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    sal_Int16 nExpected = 42;
    xParagraph->setPropertyValue(u"CharTransparence"_ustr, uno::Any(nExpected));

    // Get the transparency & verify.
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xParagraph, u"CharTransparence"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf129839)
{
    // Create a new document and add a table
    createSwDoc();
    css::uno::Reference<css::text::XTextDocument> xTextDocument(mxComponent,
                                                                css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::lang::XMultiServiceFactory> xFac(xTextDocument,
                                                              css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::text::XTextTable> xTable(
        xFac->createInstance(u"com.sun.star.text.TextTable"_ustr), css::uno::UNO_QUERY_THROW);
    xTable->initialize(4, 4);
    auto xSimpleText = xTextDocument->getText();
    xSimpleText->insertTextContent(xSimpleText->createTextCursor(), xTable, true);
    css::uno::Reference<css::table::XCellRange> xTableCellRange(xTable, css::uno::UNO_QUERY_THROW);
    // Get instance of SwXCellRange
    css::uno::Reference<css::beans::XPropertySet> xCellRange(
        xTableCellRange->getCellRangeByPosition(0, 0, 1, 1), css::uno::UNO_QUERY_THROW);
    // Test retrieval of VertOrient property - this crashed
    css::uno::Any aOrient = xCellRange->getPropertyValue(u"VertOrient"_ustr);
    CPPUNIT_ASSERT_EQUAL(css::uno::Any(css::text::VertOrientation::NONE), aOrient);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf129841)
{
    // Create a new document and add a table
    createSwDoc();
    css::uno::Reference<css::text::XTextDocument> xTextDocument(mxComponent,
                                                                css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::lang::XMultiServiceFactory> xFac(xTextDocument,
                                                              css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::text::XTextTable> xTable(
        xFac->createInstance(u"com.sun.star.text.TextTable"_ustr), css::uno::UNO_QUERY_THROW);
    xTable->initialize(4, 4);
    auto xSimpleText = xTextDocument->getText();
    xSimpleText->insertTextContent(xSimpleText->createTextCursor(), xTable, true);
    // Get SwXTextTableCursor
    css::uno::Reference<css::beans::XPropertySet> xTableCursor(
        xTable->createCursorByCellName(u"A1"_ustr), css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XCellRange> xTableCellRange(xTable, css::uno::UNO_QUERY_THROW);
    // Get SwXCellRange for the same cell
    css::uno::Reference<css::beans::XPropertySet> xCellRange(
        xTableCellRange->getCellRangeByName(u"A1:A1"_ustr), css::uno::UNO_QUERY_THROW);
    static constexpr OUString sBackColor = u"BackColor"_ustr;
    // Apply background color to table cursor, and read background color from cell range
    css::uno::Any aRefColor(COL_LIGHTRED);
    xTableCursor->setPropertyValue(sBackColor, aRefColor);
    css::uno::Any aColor = xCellRange->getPropertyValue(sBackColor);
    // This failed
    CPPUNIT_ASSERT_EQUAL(aRefColor, aColor);
    // Now the other way round
    aRefColor <<= COL_LIGHTGREEN;
    xCellRange->setPropertyValue(sBackColor, aRefColor);
    aColor = xTableCursor->getPropertyValue(sBackColor);
    CPPUNIT_ASSERT_EQUAL(aRefColor, aColor);
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf141525)
{
    // Unit test for tdf#141525:
    // Checks if "Line with Arrow/Circle" is inserted with correct end points
    createSwDoc();

    // Insert "Line with Arrow/Circle" shape with CTRL key
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));
    dispatchCommand(mxComponent, u".uno:LineArrowCircle"_ustr, aArgs);

    // Asserts line shape has been inserted into the doc
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.LineShape"_ustr, getShape(1)->getShapeType());

    // Asserts end of line has a circle
    // Without the test, "Line Starts with Arrow" is inserted
    // i.e. the circle is missing from the line end point
    // - Expected: "Circle"
    // - Actual: ""
    CPPUNIT_ASSERT_EQUAL(u"Circle"_ustr, getProperty<OUString>(getShape(1), u"LineEndName"_ustr));
    // Asserts start of line has an arrow
    CPPUNIT_ASSERT_EQUAL(u"Arrow"_ustr, getProperty<OUString>(getShape(1), u"LineStartName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf160278)
{
    createSwDoc();
    auto xTextDocument(mxComponent.queryThrow<css::text::XTextDocument>());
    auto xText(xTextDocument->getText());
    xText->setString(u"123"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"123"_ustr, xText->getString());
    auto xCursor = xText->createTextCursorByRange(xText->getEnd());
    xCursor->goLeft(1, true);
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xCursor->getString());
    // Insert an SMP character U+1f702 (so it's two UTF-16 code units, 0xd83d 0xdf02):
    xCursor->setString(u"🜂"_ustr);
    // Without the fix, the replacement would expand the cursor one too many characters to the left,
    // and the cursor text would become "2🜂", failing the next test:
    CPPUNIT_ASSERT_EQUAL(u"🜂"_ustr, xCursor->getString());
    xCursor->setString(u"test"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xCursor->getString());
    // This test would fail, too; the text would be "1test":
    CPPUNIT_ASSERT_EQUAL(u"12test"_ustr, xText->getString());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf161035)
{
    // Given a paragraph with a bookmark:
    createSwDoc("tdf161035.fodt");
    auto xModel = mxComponent.queryThrow<frame::XModel>();

    // Create a text view cursor in the paragraph.
    auto xController = xModel->getCurrentController().queryThrow<text::XTextViewCursorSupplier>();
    auto xViewCursor = xController->getViewCursor();
    CPPUNIT_ASSERT(xViewCursor);
    auto xText = xViewCursor->getText();
    CPPUNIT_ASSERT(xText);
    // Create a text cursor from the text view cursor, and move it to the end of the paragraph
    auto xTextCursor = xText->createTextCursorByRange(xViewCursor);
    CPPUNIT_ASSERT(xTextCursor);
    xTextCursor->gotoEnd(false);
    // Get the first paragraph portion from the text cursor
    auto xParaEnum = xTextCursor.queryThrow<container::XEnumerationAccess>()->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum);
    auto xPara = xParaEnum->nextElement().queryThrow<container::XEnumerationAccess>();
    // Try to enumerate text portions. Without the fix, it would fail an assertion in debug builds,
    // and hang in release builds, because the paragraph portion started after the bookmark, and
    // so the bookmark wasn't processed (expectedly):
    auto xRunEnum = xPara->createEnumeration();
    CPPUNIT_ASSERT(!xRunEnum->hasMoreElements()); // Empty enumeration for empty selection
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf162480)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Name", uno::Any(createFileURL(u"textboxInColumn2.fodt")) },
    });

    // Inserting a document with text box attached in a table's second column must not crash
    dispatchCommand(mxComponent, u".uno:InsertDoc"_ustr, aPropertyValues);

    auto xTextBox = getShape(1).queryThrow<css::text::XTextContent>();
    auto xTable = getParagraphOrTable(2).queryThrow<css::text::XTextTable>();
    auto xAnchorRange = xTextBox->getAnchor();
    auto xCellText = xTable->getCellByName("B1").queryThrow<css::text::XText>();
    CPPUNIT_ASSERT_EQUAL(xCellText, xAnchorRange->getText());
}

CPPUNIT_TEST_FIXTURE(SwUnoWriter, testTdf164885)
{
    class LocalDispatch : public comphelper::WeakImplHelper<css::frame::XDispatch>
    {
    public:
        LocalDispatch() = default;

        void SAL_CALL dispatch(const css::util::URL& URL,
                               const css::uno::Sequence<css::beans::PropertyValue>&) override
        {
            sLastCommand = URL.Complete;
        }
        void SAL_CALL addStatusListener(const css::uno::Reference<css::frame::XStatusListener>&,
                                        const css::util::URL&) override
        {
            // empty
        }
        void SAL_CALL removeStatusListener(const css::uno::Reference<css::frame::XStatusListener>&,
                                           const css::util::URL&) override
        {
            // empty
        }

        OUString sLastCommand;
    };

    class LocalInterceptor
        : public comphelper::WeakImplHelper<css::frame::XDispatchProviderInterceptor>
    {
    public:
        LocalInterceptor() = default;

        // XDispatchProvider
        css::uno::Reference<css::frame::XDispatch>
            SAL_CALL queryDispatch(const css::util::URL& URL, const OUString& TargetFrameName,
                                   sal_Int32 SearchFlags) override
        {
            if (URL.Complete == ".uno:Open")
                return pDispatch;
            if (m_slave)
                return m_slave->queryDispatch(URL, TargetFrameName, SearchFlags);
            return {};
        }
        css::uno::Sequence<css::uno::Reference<css::frame::XDispatch>> SAL_CALL
        queryDispatches(const css::uno::Sequence<css::frame::DispatchDescriptor>&) override
        {
            return {};
        }

        // XDispatchProviderInterceptor
        css::uno::Reference<css::frame::XDispatchProvider>
            SAL_CALL getSlaveDispatchProvider() override
        {
            return m_slave;
        }
        void SAL_CALL setSlaveDispatchProvider(
            const css::uno::Reference<css::frame::XDispatchProvider>& val) override
        {
            m_slave = val;
        }
        css::uno::Reference<css::frame::XDispatchProvider>
            SAL_CALL getMasterDispatchProvider() override
        {
            return m_master;
        }
        void SAL_CALL setMasterDispatchProvider(
            const css::uno::Reference<css::frame::XDispatchProvider>& val) override
        {
            m_master = val;
        }

        rtl::Reference<LocalDispatch> pDispatch{ new LocalDispatch };

    private:
        css::uno::Reference<css::frame::XDispatchProvider> m_master;
        css::uno::Reference<css::frame::XDispatchProvider> m_slave;
    };

    // Given a document with a hyperlink
    createSwDoc("hyperlink.fodt");
    auto controller(mxComponent.queryThrow<frame::XModel>()->getCurrentController());
    auto xProvider(controller->getFrame().queryThrow<css::frame::XDispatchProviderInterception>());

    rtl::Reference<LocalInterceptor> interceptor(new LocalInterceptor);
    xProvider->registerDispatchProviderInterceptor(interceptor);

    auto xCursor = controller.queryThrow<text::XTextViewCursorSupplier>()->getViewCursor();
    xCursor->goRight(5, false); // put cursor inside the hyperlink

    // Initiale "open hyperlink"
    dispatchCommand(mxComponent, u".uno:OpenHyperlinkOnCursor"_ustr, {});

    xProvider->releaseDispatchProviderInterceptor(interceptor);

    // Without the fix, this failed with
    // - Expected: .uno:Open
    // - Actual  :
    // because the interception didn't happen
    CPPUNIT_ASSERT_EQUAL(u".uno:Open"_ustr, interceptor->pDispatch->sLastCommand);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
