/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/text/XTextViewTextRangeSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <vcl/scheduler.hxx>
#include <tools/json_writer.hxx>
#include <comphelper/propertyvalue.hxx>
#include <xmloff/odffields.hxx>

#include <docsh.hxx>
#include <edtwin.hxx>
#include <unotextrange.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <frameformats.hxx>
#include <fmtanchr.hxx>
#include <unotxdoc.hxx>

/// Covers sw/source/uibase/uno/ fixes.
class SwUibaseUnoTest : public SwModelTestBase
{
public:
    SwUibaseUnoTest()
        : SwModelTestBase(u"/sw/qa/uibase/uno/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testLockControllers)
{
    createSwDoc();
    {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
        xModel->lockControllers();
    }
    {
        uno::Reference<util::XCloseable> xCloseable(mxComponent, uno::UNO_QUERY_THROW);
        xCloseable->close(false);
    }
    // Without the accompanying fix in place, this test would have crashed.
    mxComponent.clear();
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCondFieldCachedValue)
{
    createSwDoc("cond-field-cached-value.docx");
    Scheduler::ProcessEventsToIdle();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  :
    // i.e. the conditional field lost its cached content.
    getParagraph(2, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCreateTextRangeByPixelPosition)
{
    // Given a document with 2 characters, and the pixel position of the point between them:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2(u"AZ"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    Point aLogic = pWrtShell->GetCharRect().Center();
    SwView* pView = pDocShell->GetView();
    SwEditWin& rEditWin = pView->GetEditWin();
    Point aPixel = rEditWin.LogicToPixel(aLogic);

    // When converting that pixel position to a document model position (text range):
    uno::Reference<frame::XModel2> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xControllers = xModel->getControllers();
    uno::Reference<text::XTextViewTextRangeSupplier> xController(xControllers->nextElement(),
                                                                 uno::UNO_QUERY);
    awt::Point aPoint(aPixel.getX(), aPixel.getY());
    uno::Reference<text::XTextRange> xTextRange
        = xController->createTextRangeByPixelPosition(aPoint);

    // Then make sure that text range points after the first character:
    auto pTextRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    SwPaM aPaM(pDoc->GetNodes());
    pTextRange->GetPositions(aPaM);
    sal_Int32 nActual = aPaM.GetPoint()->GetContentIndex();
    // Without the needed PixelToLogic() call in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the returned text range pointed before the first character, not between the first and
    // the second character.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCreateTextRangeByPixelPositionGraphic)
{
    // Given a document with an as-char image and the center of that image in pixels:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    xTextGraphic->setPropertyValue(u"Width"_ustr, uno::Any(static_cast<sal_Int32>(10000)));
    xTextGraphic->setPropertyValue(u"Height"_ustr, uno::Any(static_cast<sal_Int32>(10000)));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pText = pBody->GetLower();
    SwSortedObjs& rDrawObjs = *pText->GetDrawObjs();
    SwAnchoredObject* pAnchored = rDrawObjs[0];
    Point aLogic = pAnchored->GetObjRect().Center();
    SwView* pView = pDocShell->GetView();
    SwEditWin& rEditWin = pView->GetEditWin();
    Point aPixel = rEditWin.LogicToPixel(aLogic);

    // When converting that pixel position to a document model position (text range):
    uno::Reference<frame::XModel2> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xControllers = xModel->getControllers();
    uno::Reference<text::XTextViewTextRangeSupplier> xController(xControllers->nextElement(),
                                                                 uno::UNO_QUERY);
    awt::Point aPoint(aPixel.getX(), aPixel.getY());
    // Without the accompanying fix in place, this test would have crashed, because an XTextRange
    // can't point to a graphic node.
    uno::Reference<text::XTextRange> xTextRange
        = xController->createTextRangeByPixelPosition(aPoint);

    // Then make sure that the anchor of the image is returned:
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    const auto pFormat = rFormats[0];
    SwPosition aAnchorPos(*pFormat->GetAnchor().GetContentAnchor());
    auto pTextRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    SwPaM aPaM(pDoc->GetNodes());
    pTextRange->GetPositions(aPaM);
    CPPUNIT_ASSERT_EQUAL(aAnchorPos, *aPaM.GetPoint());
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCreateTextRangeByPixelPositionAtPageGraphic)
{
    // Given a document with an at-page anchored image:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AT_PAGE));
    xTextGraphic->setPropertyValue(u"AnchorPageNo"_ustr, uno::Any(static_cast<sal_Int16>(1)));
    xTextGraphic->setPropertyValue(u"Width"_ustr, uno::Any(static_cast<sal_Int32>(10000)));
    xTextGraphic->setPropertyValue(u"Height"_ustr, uno::Any(static_cast<sal_Int32>(10000)));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwSortedObjs& rDrawObjs = *pPage->GetDrawObjs();
    SwAnchoredObject* pAnchored = rDrawObjs[0];
    Point aLogic = pAnchored->GetObjRect().Center();
    SwView* pView = pDocShell->GetView();
    SwEditWin& rEditWin = pView->GetEditWin();
    Point aPixel = rEditWin.LogicToPixel(aLogic);

    // When asking for the doc model pos of the image's anchor by pixel position:
    uno::Reference<frame::XModel2> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xControllers = xModel->getControllers();
    uno::Reference<text::XTextViewTextRangeSupplier> xController(xControllers->nextElement(),
                                                                 uno::UNO_QUERY);
    awt::Point aPoint(aPixel.getX(), aPixel.getY());
    // Without the accompanying fix in place, this test would have crashed.
    uno::Reference<text::XTextRange> xTextRange
        = xController->createTextRangeByPixelPosition(aPoint);

    // Then make sure that the result is empty, since the image is at-page anchored:
    CPPUNIT_ASSERT(!xTextRange.is());
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetTextFormFields)
{
    // Given a document with 3 fieldmarks: 2 zotero items and a zotero
    // bibliography:
    createSwDoc();
    for (int i = 0; i < 2; ++i)
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_ITEM foo bar"_ustr)),
            comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"result"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
            comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                          uno::Any(u"ADDIN ZOTERO_BIBL foo bar"_ustr)),
            comphelper::makePropertyValue(u"FieldResult"_ustr,
                                          uno::Any(u"<p>aaa</p><p>bbb</p>"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);
    }

    // When getting the zotero items:
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:TextFormFields?type=vnd.oasis.opendocument.field.UNHANDLED&"
                              "commandPrefix=ADDIN%20ZOTERO_ITEM");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we find the 2 items and ignore the bibliography:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (fields)
    // i.e. the returned JSON was just empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTree.get_child("fields").count(""));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetDocumentProperties)
{
    // Given a document with 3 custom properties: 2 Zotero ones and one other:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(),
                                                               uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDP = xDPS->getDocumentProperties();
    uno::Reference<beans::XPropertyContainer> xUDP = xDP->getUserDefinedProperties();
    xUDP->addProperty(u"ZOTERO_PREF_1"_ustr, beans::PropertyAttribute::REMOVABLE,
                      uno::Any(u"foo"_ustr));
    xUDP->addProperty(u"ZOTERO_PREF_2"_ustr, beans::PropertyAttribute::REMOVABLE,
                      uno::Any(u"bar"_ustr));
    xUDP->addProperty(u"OTHER"_ustr, beans::PropertyAttribute::REMOVABLE, uno::Any(u"baz"_ustr));

    // When getting the zotero properties:
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:SetDocumentProperties?namePrefix=ZOTERO_PREF_");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we find the 2 properties and ignore the other one:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (userDefinedProperties)
    // i.e. the returned JSON was just empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2),
                         aTree.get_child("userDefinedProperties").count(""));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetBookmarks)
{
    // Given a document with 3 bookmarks: 2 zotero references and a zotero bibliography:
    createSwDoc();
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"Bookmark"_ustr, uno::Any(u"ZOTERO_BREF_1"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:InsertBookmark"_ustr, aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"Bookmark"_ustr, uno::Any(u"ZOTERO_BREF_2"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:InsertBookmark"_ustr, aArgs);
    }
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"Bookmark"_ustr, uno::Any(u"ZOTERO_BIBL"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:InsertBookmark"_ustr, aArgs);
    }

    // When getting the reference bookmarks:
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:Bookmarks?namePrefix=ZOTERO_BREF_");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we get the 2 references but not the bibliography:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (bookmarks)
    // i.e. the returned JSON was just empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTree.get_child("bookmarks").count(""));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetFields)
{
    // Given a document with a refmark:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aName(u"ZOTERO_ITEM CSL_CITATION {} "_ustr);
    for (int i = 0; i < 5; ++i)
    {
        uno::Sequence<css::beans::PropertyValue> aArgs = {
            comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
            comphelper::makePropertyValue(u"Name"_ustr, uno::Any(aName + OUString::number(i + 1))),
            comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"mycontent"_ustr)),
        };
        dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);
        pWrtShell->SttEndDoc(/*bStt=*/false);
        pWrtShell->SplitNode();
        pWrtShell->SttEndDoc(/*bStt=*/false);
    }

    // When getting the refmarks:
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:Fields?typeName=SetRef&namePrefix=ZOTERO_ITEM%20CSL_CITATION");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we get the 1 refmark:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (setRefs)
    // i.e. the returned JSON was just empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), aTree.get_child("setRefs").count(""));
    auto it = aTree.get_child("setRefs").begin();
    boost::property_tree::ptree aRef = (it++)->second;
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_ITEM CSL_CITATION {} 1"),
                         aRef.get<std::string>("name"));
    aRef = (it++)->second;
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_ITEM CSL_CITATION {} 2"),
                         aRef.get<std::string>("name"));
    aRef = (it++)->second;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ZOTERO_ITEM CSL_CITATION {} 3
    // - Actual  : ZOTERO_ITEM CSL_CITATION {} 4
    // i.e. the output was unsorted.
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_ITEM CSL_CITATION {} 3"),
                         aRef.get<std::string>("name"));
    aRef = (it++)->second;
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_ITEM CSL_CITATION {} 4"),
                         aRef.get<std::string>("name"));
    aRef = (it++)->second;
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_ITEM CSL_CITATION {} 5"),
                         aRef.get<std::string>("name"));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetTextFormField)
{
    // Given a document with a fieldmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FieldType"_ustr, uno::Any(ODF_UNHANDLED)),
        comphelper::makePropertyValue(u"FieldCommand"_ustr,
                                      uno::Any(u"ADDIN ZOTERO_ITEM foo bar"_ustr)),
        comphelper::makePropertyValue(u"FieldResult"_ustr, uno::Any(u"result"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, aArgs);

    // When stepping into the fieldmark with the cursor and getting the command value for
    // uno:TextFormField:
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:TextFormField?type=vnd.oasis.opendocument.field.UNHANDLED&"
                              "commandPrefix=ADDIN%20ZOTERO_ITEM");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we find the inserted fieldmark:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (type)
    // i.e. the returned JSON was just an empty object.
    auto field = aTree.get_child("field");
    CPPUNIT_ASSERT_EQUAL(std::string("vnd.oasis.opendocument.field.UNHANDLED"),
                         field.get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(std::string("ADDIN ZOTERO_ITEM foo bar"),
                         field.get<std::string>("command"));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetSections)
{
    // Given a document with a section:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(
            u"RegionName"_ustr, uno::Any(u"ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RNDRfiit6mXBc"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"<p>aaa</p><p>bbb</p>"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertSection"_ustr, aArgs);

    // When asking for a list of section names:
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:Sections?namePrefix=ZOTERO_BIBL");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Make sure we find our just inserted section:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (sections)
    // i.e. the returned JSON was an empty object.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("sections").count(""));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetBookmark)
{
    // Given a document with a bookmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Bookmark"_ustr, uno::Any(u"ZOTERO_BREF_1"_ustr)),
        comphelper::makePropertyValue(u"BookmarkText"_ustr, uno::Any(u"<p>aaa</p><p>bbb</p>"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertBookmark"_ustr, aArgs);

    // When stepping into the bookmark with the cursor and getting the command value for
    // .uno:Bookmark:
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:Bookmark?namePrefix=ZOTERO_BREF_");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we find the inserted bookmark:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    boost::property_tree::ptree aBookmark = aTree.get_child("bookmark");
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (bookmark)
    // i.e. the returned JSON was an empty object.
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_BREF_1"), aBookmark.get<std::string>("name"));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testGetField)
{
    // Given a document with a refmark:
    createSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr,
                                      uno::Any(u"ZOTERO_ITEM CSL_CITATION {} refmark"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"content"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

    // When in the refmark with the cursor and getting the command value for .uno:Field:
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    tools::JsonWriter aJsonWriter;
    std::string_view aCommand(".uno:Field?typeName=SetRef&namePrefix=ZOTERO_ITEM%20CSL_CITATION");
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->getCommandValues(aJsonWriter, aCommand);

    // Then make sure we find the inserted refmark:
    OString pJSON(aJsonWriter.finishAndGetAsOString());
    std::stringstream aStream((std::string(pJSON)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    boost::property_tree::ptree aBookmark = aTree.get_child("setRef");
    // Without the accompanying fix in place, this test would have failed with:
    // - No such node (setRef)
    // i.e. the returned JSON was an empty object.
    CPPUNIT_ASSERT_EQUAL(std::string("ZOTERO_ITEM CSL_CITATION {} refmark"),
                         aBookmark.get<std::string>("name"));
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testDoNotBreakWrappedTables)
{
    // Given an empty document:
    createSwDoc();

    // When checking the state of the DoNotBreakWrappedTables compat flag:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bDoNotBreakWrappedTables{};
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // i.e. the compat flag was not recognized.
    xSettings->getPropertyValue(u"DoNotBreakWrappedTables"_ustr) >>= bDoNotBreakWrappedTables;
    // Then make sure it's false by default:
    CPPUNIT_ASSERT(!bDoNotBreakWrappedTables);

    // And when setting DoNotBreakWrappedTables=true:
    xSettings->setPropertyValue(u"DoNotBreakWrappedTables"_ustr, uno::Any(true));
    // Then make sure it gets enabled:
    xSettings->getPropertyValue(u"DoNotBreakWrappedTables"_ustr) >>= bDoNotBreakWrappedTables;
    CPPUNIT_ASSERT(bDoNotBreakWrappedTables);
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testAllowTextAfterFloatingTableBreak)
{
    // Given an empty document:
    createSwDoc();

    // When checking the state of the AllowTextAfterFloatingTableBreak compat flag:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bAllowTextAfterFloatingTableBreak{};
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // i.e. the compat flag was not recognized.
    xSettings->getPropertyValue(u"AllowTextAfterFloatingTableBreak"_ustr)
        >>= bAllowTextAfterFloatingTableBreak;
    // Then make sure it's false by default:
    CPPUNIT_ASSERT(!bAllowTextAfterFloatingTableBreak);

    // And when setting AllowTextAfterFloatingTableBreak=true:
    xSettings->setPropertyValue(u"AllowTextAfterFloatingTableBreak"_ustr, uno::Any(true));
    // Then make sure it gets enabled:
    xSettings->getPropertyValue(u"AllowTextAfterFloatingTableBreak"_ustr)
        >>= bAllowTextAfterFloatingTableBreak;
    CPPUNIT_ASSERT(bAllowTextAfterFloatingTableBreak);
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testDoNotMirrorRtlDrawObjs)
{
    // Given an empty document:
    createSwDoc();

    // When checking the state of the DoNotMirrorRtlDrawObjs compat flag:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bDoNotMirrorRtlDrawObjs{};
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // i.e. the compat flag was not recognized.
    xSettings->getPropertyValue(u"DoNotMirrorRtlDrawObjs"_ustr) >>= bDoNotMirrorRtlDrawObjs;
    // Then make sure it's false by default:
    CPPUNIT_ASSERT(!bDoNotMirrorRtlDrawObjs);

    // And when setting DoNotMirrorRtlDrawObjs=true:
    xSettings->setPropertyValue(u"DoNotMirrorRtlDrawObjs"_ustr, uno::Any(true));
    // Then make sure it gets enabled:
    xSettings->getPropertyValue(u"DoNotMirrorRtlDrawObjs"_ustr) >>= bDoNotMirrorRtlDrawObjs;
    CPPUNIT_ASSERT(bDoNotMirrorRtlDrawObjs);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
