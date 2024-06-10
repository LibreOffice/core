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
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
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

namespace
{
class HtmlImportTest : public SwModelTestBase
{
    public:
        HtmlImportTest() : SwModelTestBase(u"sw/qa/extras/htmlimport/data/"_ustr, u"HTML (StarWriter)"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testPictureImport)
{
    createSwWebDoc("picture.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // The document contains two pictures stored as a link.
    SwEditShell* const pEditShell(pTextDoc->GetDocShell()->GetDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rLinkManager.GetLinks().size());
    rLinkManager.Remove(0,2);
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // TODO: Get the data into clipboard in html format and paste

    // But when pasting we don't want images to be linked.
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testInlinedImage)
{
    createSwWebDoc("inlined_image.html");
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
    CPPUNIT_ASSERT_EQUAL(u"Image1"_ustr, xNamed->getName());

    uno::Reference<graphic::XGraphic> xGraphic = getProperty< uno::Reference<graphic::XGraphic> >(xShape, u"Graphic"_ustr);
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
    createSwWebDoc("PageAndParagraphFilled.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // The document contains embedded pictures inlined for PageBackground and
    // ParagraphBackground, check for their existence after import
    SwEditShell* const pEditShell(pTextDoc->GetDocShell()->GetDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);

    // images are not linked, check for zero links
    const sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // get the pageStyle where the PageBackgroundFill is defined. Caution: for
    // HTML mode this is *not* called 'Default Style', but 'HTML'. Name is empty
    // due to being loaded embedded. BitmapMode is repeat.
    uno::Reference<beans::XPropertySet> xPageProperties1(getStyles(u"PageStyles"_ustr)->getByName(u"HTML"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xPageProperties1, u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPageProperties1, u"FillBitmapName"_ustr));
    CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xPageProperties1, u"FillBitmapMode"_ustr));

    // we should have one paragraph
    const int nParagraphs = getParagraphs();
    CPPUNIT_ASSERT_EQUAL(1, nParagraphs);

    if(nParagraphs)
    {
        // get the paragraph
        uno::Reference<text::XTextRange> xPara = getParagraph(1);
        uno::Reference< beans::XPropertySet > xParagraphProperties( xPara, uno::UNO_QUERY);

        // check for Bitmap FillStyle, name empty, repeat
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xParagraphProperties, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xParagraphProperties, u"FillBitmapName"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xParagraphProperties, u"FillBitmapMode"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testListStyleType)
{
    createSwWebDoc("list-style.html");
    // check unnumbered list style - should be type circle here
    uno::Reference< beans::XPropertySet > xParagraphProperties(getParagraph(4),
                                                               uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xParagraphProperties->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bBulletFound=false;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "BulletChar")
        {
            // should be 'o'.
            CPPUNIT_ASSERT_EQUAL(u"\uE009"_ustr, rProp.Value.get<OUString>());
            bBulletFound = true;
            break;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("no BulletChar property found for para 4", bBulletFound);

    // check numbered list style - should be type lower-alpha here
    xParagraphProperties.set(getParagraph(14),
                             uno::UNO_QUERY);
    xLevels.set(xParagraphProperties->getPropertyValue(u"NumberingRules"_ustr),
                uno::UNO_QUERY);
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const& rProp : aProps)
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
    createSwWebDoc("meta-ISO8601-dates.html");
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
    createSwWebDoc("image-width-auto.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwEditShell* const pEditShell(pTextDoc->GetDocShell()->GetDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    SwTextAttr const*const pAttr(pEditShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pAttr);
    SwFrameFormat const*const pFmt(pAttr->GetFlyCnt().GetFrameFormat());
    SwFormatFrameSize const& rSize(pFmt->GetFormatAttr(RES_FRM_SIZE));
    CPPUNIT_ASSERT_EQUAL(Size(1835, 560), rSize.GetSize());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageLazyRead)
{
    createSwWebDoc("image-lazy-read.html");
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), u"Graphic"_ustr);
    Graphic aGraphic(xGraphic);
    // This failed, import loaded the graphic, it wasn't lazy-read.
    CPPUNIT_ASSERT(!aGraphic.isAvailable());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testChangedby)
{
    createSwWebDoc("meta-changedby.html");
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
    CPPUNIT_ASSERT_EQUAL(u"Blah"_ustr, xDocProps->getModifiedBy());

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // ...but there is no comment 'HTML: <meta name="changedby" content="Blah">'
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTableBorder1px)
{
    createSwWebDoc("table_border_1px.html");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    table::BorderLine2 aBorder;

    uno::Reference<text::XTextRange> xCellA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell top border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell left border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellB1(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell top border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell left border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellA2(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellA2, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell top border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA2, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA2, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell left border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA2,u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellB2(xTable->getCellByName(u"B2"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell top border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell left border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testOutlineLevel)
{
    createSwWebDoc("outline-level.html");
    // This was 0, HTML imported into Writer lost the outline numbering for
    // Heading 1 styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(1), u"OutlineLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfBr)
{
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("reqif-br.xhtml");
    // <reqif-xhtml:br/> was not recognized as a line break from a ReqIf file.
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("aaa\nbbb"));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf80194_subscript)
{
    createSwWebDoc("tdf80194_subscript.html");
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), u"CharEscapement"_ustr), 0);
    // Most recently, the default subscript was 33%, which is much too large for a subscript.
    // The original 8% (derived from a mathematical calculation) is much better in general,
    // and for HTML was a better match when testing with firefox.
    // DFLT_ESC_AUTO_SUB was tested, but HTML specs are pretty loose, and generally
    // it exceeds the font ascent - so the formula-based-escapement is not appropriate.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -8.f, getProperty<float>(getRun(xPara, 2, u"p"_ustr), u"CharEscapement"_ustr), 1);

    xPara.set(getParagraph(2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), u"CharEscapement"_ustr), 0);
    uno::Reference<text::XTextRange> xRun (getRun(xPara, 2, u"L"_ustr));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 33.f, getProperty<float>(xRun, u"CharEscapement"_ustr), 1);
    // HTML (although unspecified) tends to use a fairly large font. Definitely more than DFLT_ESC_PROP.
    CPPUNIT_ASSERT( 70 < getProperty<sal_Int8>(xRun, u"CharEscapementHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfTable)
{
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("reqif-table.xhtml");
    // to see this: soffice --infilter="HTML (StarWriter):xhtmlns=reqif-xhtml" sw/qa/extras/htmlimport/data/reqif-table.xhtml
    // Load a table with xhtmlns=reqif-xhtml filter param.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr);
    // This was 0, tables had no borders, even if the default autoformat has
    // borders and the markup allows no custom borders.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr);
    // This was 0. Single column tables had no right border.  tdf#115576
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageSize)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    createSwWebDoc("image-size.html");
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
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    createSwWebDoc("tdf142781.html");
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
    createSwWebDoc("tdf122789.html");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFormats.size());
    // This failed, the image had an absolute size, not a relative one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(70), rFormats[0]->GetAttrSet().GetFrameSize().GetWidthPercent());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf118579)
{
    createSwWebDoc("tdf118579.html");
    //Without the fix in place, the file fails to load
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfPageStyle)
{
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("reqif-page-style.xhtml");
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: Standard, Actual  : HTML'.
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(1), u"PageStyleName"_ustr));
}

/// HTML import to the sw doc model tests.
class SwHtmlOptionsImportTest : public SwModelTestBase
{
    public:
        SwHtmlOptionsImportTest() : SwModelTestBase(u"/sw/qa/extras/htmlimport/data/"_ustr, u"HTML (StarWriter)"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testAllowedRTFOLEMimeTypes)
{
    uno::Sequence<OUString> aTypes = { u"test/rtf"_ustr };
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"AllowedRTFOLEMimeTypes"_ustr, aTypes),
    };
    OUString aURL = createFileURL(u"allowed-rtf-ole-mime-types.xhtml");
    mxComponent = loadFromDesktop(aURL, u"com.sun.star.text.TextDocument"_ustr, aLoadProperties);
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
    createSwWebDoc("hidden-textframe.html");

    // Check the content of the draw page.
    uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xSupplier->getDrawPage();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. an unexpected text frame was created, covering the actual content.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleImg)
{
    // Given an XHTML with an <object> (containing GIF) and an inner <object> (containing PNG, to be
    // ignored):
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("ole-img.xhtml");

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

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleImgSvg)
{
    // Given an XHTML with an <object> (containing SVG) and an inner <object> (containing PNG, to be
    // ignored):
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("ole-img-svg.xhtml");

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
    createSwWebDoc("emojis16BE.html");
    // tdf#146173: non-BMP characters' surrogates didn't combine correctly
    CPPUNIT_ASSERT_EQUAL(u"a text with emojis: 🌾 ☀👨🏼‍🌾🏃🏼‍♂️🤙🏽🔍"_ustr,
                         getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf154273)
{
    createSwWebDoc("tdf154273.html");

    // Without the fix in place, this test would have failed with
    // - Expected: 'test'
    // - Actual  : &apos;test&apos;
    CPPUNIT_ASSERT_EQUAL(u"'test' "_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleData)
{
    // Given an XHTML with an <object> (containing non-image, non-OLE2 data) and an inner <object>
    // (containing PNG):
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("ole-data.xhtml");

    // Then make sure the result is a single clickable Writer image:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the image was not imported as a Writer image (but as an OLE object).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
    uno::Reference<css::drawing::XShape> xShape = getShape(1);
    // And then the image was not clickable: this was empty.
    CPPUNIT_ASSERT(getProperty<OUString>(xShape, u"HyperLinkURL"_ustr).endsWith("/data.ole"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleData2)
{
    // Given an XHTML with 2 objects: the first has a link, the second does not have:
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    createSwDoc("ole-data2.xhtml");

    // Then make sure that the second image doesn't have a link set:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xObjects->getCount());
    uno::Reference<css::drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(getProperty<OUString>(xShape, u"HyperLinkURL"_ustr).endsWith("/data.ole"));
    xShape = getShape(2);
    // Without the accompanying fix in place, this test would have failed, the link from the 1st
    // image leaked to the 2nd image.
    CPPUNIT_ASSERT(getProperty<OUString>(xShape, u"HyperLinkURL"_ustr).isEmpty());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testRGBAColor)
{
    createSwWebDoc("green-highlight.html");
    const uno::Reference<text::XTextRange> xPara = getParagraph(1);
    const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    const Color nBackColor(0xaed89a);

    // Without the accompanying fix in place, this test would have failed, the background
    // color was not imported at all, when it was in hex RGBA format in HTML.
    CPPUNIT_ASSERT_EQUAL(nBackColor, getProperty<Color>(xRun, u"CharBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf154581)
{
    createSwWebDoc("tdf154581.html");

    // Without the fix in place, this test would have failed with
    // - Expected: 150
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE), getProperty<sal_Int16>(getRun(getParagraph(2), 1), u"CharUnderline"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(getRun(getParagraph(3), 1), u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf153341)
{
    createSwWebDoc("tdf153341.html");

    const uno::Reference<text::XTextRange> xPara1 = getParagraph(1);
    const uno::Reference<beans::XPropertySet> xRun1(getRun(xPara1,1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x00, 0xFF, 0x00, 0x00), getProperty<Color>(xRun1, u"CharColor"_ustr));

    const uno::Reference<text::XTextRange> xPara2 = getParagraph(2);
    const uno::Reference<beans::XPropertySet> xRun2(getRun(xPara2,1), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: rgba[ff00007f]
    // - Actual  : rgba[ff0000ff]
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x80, 0xFF, 0x00, 0x00), getProperty<Color>(xRun2, u"CharColor"_ustr));

    const uno::Reference<text::XTextRange> xPara3 = getParagraph(3);
    const uno::Reference<beans::XPropertySet> xRun3(getRun(xPara3,1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xB3, 0xFF, 0x00, 0x00), getProperty<Color>(xRun3, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf155011)
{
    createSwWebDoc("tdf155011.html");
    // Must not crash / fail asserts
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
