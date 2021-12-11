/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <tools/datetime.hxx>
#include <sfx2/linkmgr.hxx>
#include <comphelper/propertyvalue.hxx>

#include <docsh.hxx>
#include <editsh.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <fmtflcnt.hxx>
#include <fmtfsize.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>

class HtmlImportTest : public SwModelTestBase
{
    public:
        HtmlImportTest() : SwModelTestBase("sw/qa/extras/htmlimport/data/", "HTML (StarWriter)") {}
};

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testPictureImport)
{
    load(mpTestDocumentPath, "picture.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // The document contains two pictures stored as a link.
    sfx2::LinkManager& rLinkManager = pTextDoc->GetDocShell()->GetDoc()->GetEditShell()->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rLinkManager.GetLinks().size());
    rLinkManager.Remove(0,2);
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // TODO: Get the data into clipboard in html format and paste

    // But when pasting we don't want images to be linked.
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testInlinedImage)
{
    load(mpTestDocumentPath, "inlined_image.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // The document contains only one embedded picture inlined in img's src attribute.

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwEditShell* pEditShell = pDoc->GetEditShell();
    CPPUNIT_ASSERT(pEditShell);

    // This was 1 before 3914a711060341345f15b83656457f90095f32d6
    const sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    uno::Reference<drawing::XShape> xShape = getShape(1);
    uno::Reference<container::XNamed> const xNamed(xShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Image1"), xNamed->getName());

    uno::Reference<graphic::XGraphic> xGraphic = getProperty< uno::Reference<graphic::XGraphic> >(xShape, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
    CPPUNIT_ASSERT(xGraphic->getType() != graphic::GraphicType::EMPTY);

    for (SwNodeOffset n(0); ; n++)
    {
        SwNode* pNode = pDoc->GetNodes()[ n ];
        if (SwGrfNode *pGrfNode = pNode->GetGrfNode())
        {
            // FIXME? For some reason without the fix in 72703173066a2db5c977d422ace
            // I was getting GraphicType::NONE from SwEditShell::GetGraphicType() when
            // running LibreOffice but cannot reproduce that in a unit test here. :-(
            // So, this does not really test anything.
            CPPUNIT_ASSERT(pGrfNode->GetGrfObj().GetType() != GraphicType::NONE);
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testInlinedImagesPageAndParagraph)
{
    load(mpTestDocumentPath, "PageAndParagraphFilled.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // The document contains embedded pictures inlined for PageBackground and
    // ParagraphBackground, check for their existence after import
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwEditShell* pEditShell = pDoc->GetEditShell();
    CPPUNIT_ASSERT(pEditShell);

    // images are not linked, check for zero links
    const sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // get the pageStyle where the PageBackgroundFill is defined. Caution: for
    // HTML mode this is *not* called 'Default Style', but 'HTML'. Name is empty
    // due to being loaded embedded. BitmapMode is repeat.
    uno::Reference<beans::XPropertySet> xPageProperties1(getStyles("PageStyles")->getByName("HTML"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xPageProperties1, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPageProperties1, "FillBitmapName"));
    CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xPageProperties1, "FillBitmapMode"));

    // we should have one paragraph
    const int nParagraphs = getParagraphs();
    CPPUNIT_ASSERT_EQUAL(1, nParagraphs);

    if(nParagraphs)
    {
        // get the paragraph
        uno::Reference<text::XTextRange> xPara = getParagraph(1);
        uno::Reference< beans::XPropertySet > xParagraphProperties( xPara, uno::UNO_QUERY);

        // check for Bitmap FillStyle, name empty, repeat
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xParagraphProperties, "FillStyle"));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xParagraphProperties, "FillBitmapName"));
        CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xParagraphProperties, "FillBitmapMode"));
    }
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testListStyleType)
{
    load(mpTestDocumentPath, "list-style.html");
    // check unnumbered list style - should be type circle here
    uno::Reference< beans::XPropertySet > xParagraphProperties(getParagraph(4),
                                                               uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xParagraphProperties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bBulletFound=false;
    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "BulletChar")
        {
            // should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString(u"\uE009"), rProp.Value.get<OUString>());
            bBulletFound = true;
            break;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("no BulletChar property found for para 4", bBulletFound);

    // check numbered list style - should be type lower-alpha here
    xParagraphProperties.set(getParagraph(14),
                             uno::UNO_QUERY);
    xLevels.set(xParagraphProperties->getPropertyValue("NumberingRules"),
                uno::UNO_QUERY);
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "NumberingType")
        {
            printf("style is %d\n", rProp.Value.get<sal_Int16>());
            // is lower-alpha in input, translates into chars_lower_letter here
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHARS_LOWER_LETTER,
                                 rProp.Value.get<sal_Int16>());
            return;
        }
    }
    CPPUNIT_FAIL("no NumberingType property found for para 14");
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testMetaIsoDates)
{
    load(mpTestDocumentPath, "meta-ISO8601-dates.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell(pTextDoc->GetDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;

    CPPUNIT_ASSERT(pDocShell);
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(), uno::UNO_QUERY);
    xDocProps.set(xDPS->getDocumentProperties());

    // get the document properties
    CPPUNIT_ASSERT(xDocProps.is());
    DateTime aCreated(xDocProps->getCreationDate()); // in the new format
    DateTime aModified(xDocProps->getModificationDate()); // in the legacy format (what LibreOffice used to write)

    CPPUNIT_ASSERT_EQUAL(DateTime(Date(7, 5, 2017), tools::Time(12, 34, 3, 921000000)), aCreated);
    CPPUNIT_ASSERT_EQUAL(DateTime(Date(8, 5, 2017), tools::Time(12, 47, 0, 386000000)), aModified);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageWidthAuto)
{
    load(mpTestDocumentPath, "image-width-auto.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwTextAttr const*const pAttr(pTextDoc->GetDocShell()->GetDoc()->GetEditShell()->
        GetCursor()->GetNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pAttr);
    SwFrameFormat const*const pFmt(pAttr->GetFlyCnt().GetFrameFormat());
    SwFormatFrameSize const& rSize(pFmt->GetFormatAttr(RES_FRM_SIZE));
    CPPUNIT_ASSERT_EQUAL(Size(1835, 560), rSize.GetSize());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageLazyRead)
{
    load(mpTestDocumentPath, "image-lazy-read.html");
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    Graphic aGraphic(xGraphic);
    // This failed, import loaded the graphic, it wasn't lazy-read.
    CPPUNIT_ASSERT(!aGraphic.isAvailable());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testChangedby)
{
    load(mpTestDocumentPath, "meta-changedby.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell(pTextDoc->GetDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;

    CPPUNIT_ASSERT(pDocShell);
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(), uno::UNO_QUERY);
    xDocProps.set(xDPS->getDocumentProperties());

    // get the document properties
    CPPUNIT_ASSERT(xDocProps.is());

    // the doc's property ModifiedBy is set correctly, ...
    CPPUNIT_ASSERT_EQUAL(OUString("Blah"), xDocProps->getModifiedBy());

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // ...but there is no comment 'HTML: <meta name="changedby" content="Blah">'
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTableBorder1px)
{
    load(mpTestDocumentPath, "table_border_1px.html");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    table::BorderLine2 aBorder;

    uno::Reference<text::XTextRange> xCellA1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellA1, "TopBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell top border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA1, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA1, "LeftBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell left border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA1, "RightBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellB1(xTable->getCellByName("B1"), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellB1, "TopBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell top border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellB1, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB1, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell left border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB1, "RightBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellA2(xTable->getCellByName("A2"), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellA2, "TopBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell top border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA2, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA2, "LeftBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell left border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA2,"RightBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellB2(xTable->getCellByName("B2"), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellB2, "TopBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell top border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell left border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, "RightBorder");
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testOutlineLevel)
{
    load(mpTestDocumentPath, "outline-level.html");
    // This was 0, HTML imported into Writer lost the outline numbering for
    // Heading 1 styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(1), "OutlineLevel"));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfBr)
{
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    load(mpTestDocumentPath, "reqif-br.xhtml");
    // <reqif-xhtml:br/> was not recognized as a line break from a ReqIf file.
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("aaa\nbbb"));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf80194_subscript)
{
    load(mpTestDocumentPath, "tdf80194_subscript.html");
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), "CharEscapement"), 0);
    // Most recently, the default subscript was 33%, which is much too large for a subscript.
    // The original 8% (derived from a mathematical calculation) is much better in general,
    // and for HTML was a better match when testing with firefox.
    // DFLT_ESC_AUTO_SUB was tested, but HTML specs are pretty loose, and generally
    // it exceeds the font ascent - so the formula-based-escapement is not appropriate.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -8.f, getProperty<float>(getRun(xPara, 2, "p"), "CharEscapement"), 1);

    xPara.set(getParagraph(2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), "CharEscapement"), 0);
    uno::Reference<text::XTextRange> xRun (getRun(xPara, 2, "L"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 33.f, getProperty<float>(xRun, "CharEscapement"), 1);
    // HTML (although unspecified) tends to use a fairly large font. Definitely more than DFLT_ESC_PROP.
    CPPUNIT_ASSERT( 70 < getProperty<sal_Int8>(xRun, "CharEscapementHeight"));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfTable)
{
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    load(mpTestDocumentPath, "reqif-table.xhtml");
    // to see this: soffice --infilter="HTML (StarWriter):xhtmlns=reqif-xhtml" sw/qa/extras/htmlimport/data/reqif-table.xhtml
    // Load a table with xhtmlns=reqif-xhtml filter param.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xCell, "TopBorder");
    // This was 0, tables had no borders, even if the default autoformat has
    // borders and the markup allows no custom borders.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, "RightBorder");
    // This was 0. Single column tables had no right border.  tdf#115576
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageSize)
{
    load(mpTestDocumentPath, "image-size.html");
    awt::Size aSize = getShape(1)->getSize();
    OutputDevice* pDevice = Application::GetDefaultDevice();
    Size aPixelSize(200, 400);
    Size aExpected = pDevice->PixelToLogic(aPixelSize, MapMode(MapUnit::Map100thMM));

    // This was 1997, i.e. a hardcoded default, we did not look at the image
    // header when the HTML markup declared no size.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aExpected.getWidth()), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aExpected.getHeight()), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf142781)
{
    load(mpTestDocumentPath, "tdf142781.html");
    OutputDevice* pDevice = Application::GetDefaultDevice();
    Size aPixelSize(672, 480);
    Size aExpected = pDevice->PixelToLogic(aPixelSize, MapMode(MapUnit::Map100thMM));
    awt::Size aSize = getShape(1)->getSize();
    // Without the fix in place, this test would have failed with
    // - Expected: 12700
    // - Actual: 25400
   // CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aExpected.getHeight()), aSize.Height);
    aSize = getShape(2)->getSize();
    // Without the fix in place, this test would have failed with
    // - Expected: 17780
    // - Actual: 35560
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aExpected.getWidth()), aSize.Width);
}


CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf122789)
{
    load(mpTestDocumentPath, "tdf122789.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFormats.size());
    // This failed, the image had an absolute size, not a relative one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(70), rFormats[0]->GetAttrSet().GetFrameSize().GetWidthPercent());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf118579)
{
    load(mpTestDocumentPath, "tdf118579.html");
    //Without the fix in place, the file fails to load
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfPageStyle)
{
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    load(mpTestDocumentPath, "reqif-page-style.xhtml");
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: Standard, Actual  : HTML'.
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "PageStyleName"));
}

/// HTML import to the sw doc model tests.
class SwHtmlOptionsImportTest : public SwModelTestBase
{
};

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/htmlimport/data/";

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testAllowedRTFOLEMimeTypes)
{
    uno::Sequence<OUString> aTypes = { OUString("test/rtf") };
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("AllowedRTFOLEMimeTypes", aTypes),
    };
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "allowed-rtf-ole-mime-types.xhtml";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties);
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
    uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT(xObject.is());
    uno::Reference<embed::XInplaceObject> xEmbeddedObject(
        xObject->getExtendedControlOverEmbeddedObject(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed, because the returned
    // embedded object was a dummy one, which does not support in-place editing.
    CPPUNIT_ASSERT(xEmbeddedObject.is());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testHiddenTextframe)
{
    // Load HTML content into Writer, similar to HTML paste.
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
    };
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "hidden-textframe.html";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties);

    // Check the content of the draw page.
    uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xSupplier->getDrawPage();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. an unexpected text frame was created, covering the actual content.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testOleImg)
{
    // Given an XHTML with an <object> (containing GIF) and an inner <object> (containing PNG, to be
    // ignored):
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ole-img.xhtml";

    // When loading the document:
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties);

    // Then make sure the result is a single Writer image:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the image was not imported as a Writer image (but as an OLE object).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testOleImgSvg)
{
    // Given an XHTML with an <object> (containing SVG) and an inner <object> (containing PNG, to be
    // ignored):
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ole-img-svg.xhtml";

    // When loading the document:
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties);

    // Then make sure the result is a single Writer image:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the image was not imported as a Writer image (but as an OLE object).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testUTF16_nonBMP)
{
    load(mpTestDocumentPath, "emojis16BE.html");
    // tdf#146173: non-BMP characters' surrogates didn't combine correctly
    CPPUNIT_ASSERT_EQUAL(OUString(u"a text with emojis: 🌾 ☀👨🏼‍🌾🏃🏼‍♂️🤙🏽🔍"),
                         getParagraph(1)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
