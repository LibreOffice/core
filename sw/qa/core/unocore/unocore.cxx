/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/errinf.hxx>

#include <wrtsh.hxx>
#include <unotextrange.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/unocore/data/";
}

/// Covers sw/source/core/unocore/ fixes.
class SwCoreUnocoreTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf119081)
{
    // Load a doc with a nested table in it.
    load(DATA_DIRECTORY, "tdf119081.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Enter outer A1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/3);
    // Enter inner A1.
    pWrtShell->Right(CRSR_SKIP_CELLS, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false,
                     /*bVisual=*/true);
    // Enter outer B1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);

    SwDoc* pDoc = pDocShell->GetDoc();
    SwPaM& rCursor = pWrtShell->GetCurrentShellCursor();
    uno::Reference<text::XTextRange> xInsertPosition
        = SwXTextRange::CreateXTextRange(*pDoc, *rCursor.GetPoint(), nullptr);
    uno::Reference<text::XTextAppend> xTextAppend(xInsertPosition->getText(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.uno.RuntimeException
    xTextAppend->insertTextPortion("x", {}, xInsertPosition);

    // Verify that the string is indeed inserted.
    pWrtShell->Left(CRSR_SKIP_CELLS, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false,
                    /*bVisual=*/true);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pWrtShell->GetCurrentShellCursor().GetText());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, flyAtParaAnchor)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<lang::XMultiServiceFactory> const xMSF(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextDocument> const xTD(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextFrame> const xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xFrameProps(xTextFrame, uno::UNO_QUERY_THROW);
    xFrameProps->setPropertyValue("AnchorType",
                                  uno::makeAny(text::TextContentAnchorType_AT_PARAGRAPH));
    auto const xText = xTD->getText();
    auto const xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xTextFrame, false);
    auto const xAnchor = xTextFrame->getAnchor();
    uno::Reference<text::XTextContent> const xFieldmark(
        xMSF->createInstance("com.sun.star.text.Fieldmark"), uno::UNO_QUERY_THROW);
    // this crashed because the anchor didn't have SwIndex
    xText->insertTextContent(xAnchor, xFieldmark, false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testRtlGutter)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Unknown property: RtlGutter
    auto bRtlGutter = getProperty<bool>(xPageStyle, "RtlGutter");
    CPPUNIT_ASSERT(!bRtlGutter);
    xPageStyle->setPropertyValue("RtlGutter", uno::makeAny(true));
    bRtlGutter = getProperty<bool>(xPageStyle, "RtlGutter");
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testBiblioLocalCopy)
{
    // Given an empty document:
    createSwDoc();

    // When setting the LocalURL of a biblio field:
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("ARJ00")),
        comphelper::makePropertyValue("Author", OUString("Me")),
        comphelper::makePropertyValue("Title", OUString("mytitle")),
        comphelper::makePropertyValue("Year", OUString("2020")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf")),
        comphelper::makePropertyValue("LocalURL", OUString("file:///home/me/test.pdf")),
    };
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Then make sure we get that LocalURL back:
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue("Fields"));
    // Without the accompanying fix in place, this test would have failed, there was no LocalURL key
    // in the map.
    CPPUNIT_ASSERT(aMap.find("LocalURL") != aMap.end());
    auto aActual = aMap["LocalURL"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("file:///home/me/test.pdf"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testLinkedStyles)
{
    // Given an empty document:
    createSwDoc();

    // When defining a linked style for a para style:
    uno::Reference<container::XNameAccess> xParaStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xParaStyle(xParaStyles->getByName("Caption"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xParaStyle, "LinkStyle"));
    xParaStyle->setPropertyValue("LinkStyle", uno::makeAny(OUString("Emphasis")));
    // Then make sure we get the linked char style back:
    CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xParaStyle, "LinkStyle"));

    // When defining a linked style for a char style:
    uno::Reference<container::XNameAccess> xCharStyles = getStyles("CharacterStyles");
    uno::Reference<beans::XPropertySet> xCharStyle(xCharStyles->getByName("Emphasis"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xCharStyle, "LinkStyle"));
    xCharStyle->setPropertyValue("LinkStyle", uno::makeAny(OUString("Caption")));
    // Then make sure we get the linked para style back:
    CPPUNIT_ASSERT_EQUAL(OUString("Caption"), getProperty<OUString>(xCharStyle, "LinkStyle"));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testViewCursorTextFrame)
{
    // Given a document with a graphic and holding a reference to that graphic frame:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aInsertArgs = { comphelper::makePropertyValue(
        "FileName", m_directories.getURLFromSrc(DATA_DIRECTORY) + "graphic.png") };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aInsertArgs);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xViewCursor(xTextViewCursorSupplier->getViewCursor(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame;
    xViewCursor->getPropertyValue("TextFrame") >>= xFrame;

    // When saving to ODT, then make sure the store doesn't fail:
    uno::Reference<frame::XStorable> xStorable(xModel, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreArgs
        = { comphelper::makePropertyValue("FilterName", OUString("writer8")) };
    // Without the accompanying fix in place, this test would have failed with:
    // uno.RuntimeException: "SwXParagraph: disposed or invalid ..."
    xStorable->storeToURL(maTempFile.GetURL(), aStoreArgs);
}

/// Fails the test if an error popup would be presented.
static void BasicDisplayErrorHandler(const OUString& /*rErr*/, const OUString& /*rAction*/)
{
    CPPUNIT_ASSERT(false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testBrokenEmbeddedObject)
{
    // Given a document with a broken embedded object (the XML markup is not well-formed):
    load(DATA_DIRECTORY, "broken-embedded-object.odt");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xObject(xObjects->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xEmbeddedObject;
    // Get the property first, which initializes Draw, which would overwrite our error handler.
    xObject->getPropertyValue("EmbeddedObject") >>= xEmbeddedObject;
    ErrorRegistry::RegisterDisplay(&BasicDisplayErrorHandler);

    // When trying to load that embedded object:
    xObject->getPropertyValue("EmbeddedObject") >>= xEmbeddedObject;

    // Then make sure we get a non-empty reference and an error popup it not shown:
    CPPUNIT_ASSERT(xEmbeddedObject.is());
    // Without the accompanying fix in place, we got this reference, but first an error popup was
    // shown to the user.
    CPPUNIT_ASSERT(
        xEmbeddedObject->supportsService("com.sun.star.comp.embed.OCommonEmbeddedObject"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
