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
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/GraphicNativeTransform.hxx>
#include <sfx2/linkmgr.hxx>
#include <comphelper/propertyvalue.hxx>

#include <docsh.hxx>
#include <editsh.hxx>
#include <ndgrf.hxx>

class HtmlImportTest : public SwModelTestBase
{
    public:
        HtmlImportTest() : SwModelTestBase("sw/qa/extras/htmlimport/data/", "HTML (StarWriter)") {}
    private:
        std::unique_ptr<Resetter> preTest(const char* /*filename*/) override
        {
            if (getTestName().indexOf("ReqIf") != -1)
            {
                setImportFilterOptions("xhtmlns=reqif-xhtml");
                // Bypass type detection, this is an XHTML fragment only.
                setImportFilterName("HTML (StarWriter)");
            }

            return nullptr;
        }
};

#define DECLARE_HTMLIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, nullptr, HtmlImportTest)

DECLARE_HTMLIMPORT_TEST(testPictureImport, "picture.html")
{
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

DECLARE_HTMLIMPORT_TEST(testInlinedImage, "inlined_image.html")
{
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

    uno::Reference<graphic::XGraphic> xGraphic =
        getProperty< uno::Reference<graphic::XGraphic> >(xShape, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
    CPPUNIT_ASSERT(xGraphic->getType() != graphic::GraphicType::EMPTY);

    OUString sGraphicURL = getProperty< OUString >(xShape, "GraphicURL");
    // Before it was "data:image/png;base64,<data>"
    CPPUNIT_ASSERT(sGraphicURL.startsWith("vnd.sun.star.GraphicObject:"));

    for (int n = 0; ; n++)
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

DECLARE_HTMLIMPORT_TEST(testInlinedImagesPageAndParagraph, "PageAndParagraphFilled.html")
{
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

DECLARE_HTMLIMPORT_TEST(testListStyleType, "list-style.html")
{
    // check unnumbered list style - should be type circle here
    uno::Reference< beans::XPropertySet > xParagraphProperties(getParagraph(4),
                                                               uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xParagraphProperties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bBulletFound=false;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletChar")
        {
            // should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString("\xEE\x80\x89", 3, RTL_TEXTENCODING_UTF8), rProp.Value.get<OUString>());
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

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

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

DECLARE_HTMLIMPORT_TEST(testMetaIsoDates, "meta-ISO8601-dates.html")
{
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

DECLARE_HTMLIMPORT_TEST(testChangedby, "meta-changedby.html")
{
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

DECLARE_HTMLIMPORT_TEST(testOutlineLevel, "outline-level.html")
{
    // This was 0, HTML imported into Writer lost the outline numbering for
    // Heading 1 styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(1), "OutlineLevel"));
}

DECLARE_HTMLIMPORT_TEST(testReqIfBr, "reqif-br.xhtml")
{
    // <reqif-xhtml:br/> was not recognized as a line break from a ReqIf file.
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("aaa\nbbb"));
}

DECLARE_HTMLIMPORT_TEST(testReqIfTable, "reqif-table.xhtml")
{
    // Load a table with xhtmlns=reqif-xhtml filter param.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xCell, "TopBorder");
    // This was 0, tables had no borders, even if the default autoformat has
    // borders and the markup allows no custom borders.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(2), aBorder.LineWidth);

    // This was 0x000080 (~blue), table format didn't match the default UI one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_HTMLIMPORT_TEST(testReqIfPageStyle, "reqif-page-style.xhtml")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: Standard, Actual  : HTML'.
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "PageStyleName"));
}

/// HTML import to the sw doc model tests.
class SwHtmlOptionsImportTest : public SwModelTestBase
{
};

char const DATA_DIRECTORY[] = "/sw/qa/extras/htmlimport/data/";

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
