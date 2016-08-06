/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <grfatr.hxx>
#include <pagedesc.hxx>
#include <ndgrf.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>

#include <cmdid.h>
#include <envimg.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97") {}

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }

    /**
     * Validation handling
     */
    bool mustValidate(const char* filename) const override
    {
        const std::vector<const char*> aBlacklist =
        {
            // the following doc exports currently don't pass binary validation
            "tdf56321_flipImage_both.doc",
            "cjklist30.doc",
            "cjklist31.doc",
            "cjklist34.doc",
            "cjklist35.doc",
            "fdo77454.doc",
            "new-page-styles.doc",
            "tdf36117_verticalAdjustment.doc",
            "bnc636128.doc",
            "tdf92281.doc",
            "fdo59530.doc",
            "fdo56513.doc",
            "tscp.doc",
            "zoom.doc",
            "comments-nested.doc",
            "commented-table.doc",
            "zoomtype.doc",
            "n325936.doc",
            "first-header-footer.doc"
        };

        // Don't bother with non-.doc files; weed out blacklisted .doc files
        return (OString(filename).endsWith(".doc") && std::find(aBlacklist.begin(), aBlacklist.end(), filename) == aBlacklist.end());
    }
protected:
    bool CjkNumberedListTestHelper(sal_Int16 &nValue)
    {
        bool isNumber = false;
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        uno::Reference< beans::XPropertySet > properties( xPara, uno::UNO_QUERY);
        properties->getPropertyValue("NumberingIsNumber") >>= isNumber;
        if (!isNumber)
            return false;
        uno::Reference<container::XIndexAccess> xLevels( properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        uno::Sequence< beans::PropertyValue > aPropertyValue;
        xLevels->getByIndex(0) >>= aPropertyValue;
        for( int j = 0 ; j< aPropertyValue.getLength() ; ++j)
        {
            beans::PropertyValue aProp= aPropertyValue[j];
            if (aProp.Name == "NumberingType")
            {
                nValue = aProp.Value.get<sal_Int16>();
                return true;
            }
        }
        return false;
    }

    virtual void postLoad(const char* pFilename) override
    {
        if (OString(pFilename) == "tdf94386.odt")
        {
            SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
            CPPUNIT_ASSERT(pTextDoc);
            SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

            // emulate the behavior from tdf#94386 - insert an envelope to the
            // document
            SfxItemSet aSet(pWrtShell->GetView().GetCurShell()->GetPool(), FN_ENVELOP, FN_ENVELOP);
            aSet.Put(SwEnvItem());
            SfxRequest aRequest(FN_ENVELOP, SfxCallMode::SYNCHRON, aSet);
            SW_MOD()->ExecOther(aRequest);
        }
    }
};

DECLARE_WW8EXPORT_TEST(testN325936, "n325936.doc")
{
    /*
     * The problem was that the transparent background of the drawing in the
     * header was exported as non-transparent.
     *
     * xray ThisComponent.DrawPage(0).BackColorTransparency
     */

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    sal_Int32 nValue = getProperty< sal_Int32 >(getShape(1), "BackColorTransparency");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);
}

DECLARE_WW8EXPORT_TEST(testTscp, "tscp.doc")
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, "urn:bails");
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent, uno::UNO_QUERY);
    uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    // This failed, no graphs had the urn:bails type.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aGraphNames.getLength());
    uno::Reference<rdf::XURI> xGraphName = aGraphNames[0];
    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);

    // No RDF statement on the first paragraph.
    uno::Reference<rdf::XResource> xParagraph(getParagraph(1), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));

    // 3 RDF statements on the second paragraph.
    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    std::map<OUString, OUString> aExpectedStatements =
    {
        {"urn:bails:ExportControl:BusinessAuthorization:Identifier", "urn:example:tscp:1"},
        {"urn:bails:ExportControl:BusinessAuthorizationCategory:Identifier", "urn:example:tscp:1:confidential"},
        {"urn:bails:ExportControl:Authorization:StartValidity", "2015-11-27"}
    };
    std::map<OUString, OUString> aActualStatements;
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    while (xStatements->hasMoreElements())
    {
        rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
        aActualStatements[aStatement.Predicate->getNamespace() + aStatement.Predicate->getLocalName()] = aStatement.Object->getStringValue();
    }
    CPPUNIT_ASSERT(aExpectedStatements == aActualStatements);

    // No RDF statement on the third paragraph.
    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));
}

DECLARE_WW8EXPORT_TEST(testFdo45724, "fdo45724.odt")
{
    // The text and background color of the control shape was not correct.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<form::validation::XValidatableFormComponent> xComponent(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<sal_uInt32>(xComponent, "BackgroundColor"));
    CPPUNIT_ASSERT_EQUAL(OUString("xxx"), xComponent->getCurrentValue().get<OUString>());
}

DECLARE_WW8EXPORT_TEST(testFdo46020, "fdo46020.odt")
{
    // The footnote in that document wasn't exported, check that it is actually exported
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

DECLARE_WW8EXPORT_TEST(testFirstHeaderFooter, "first-header-footer.doc")
{
    // Test import and export of a section's headerf/footerf properties.

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),  parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),   parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),  parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"), parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"), parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"), parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"), parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),  parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),  parseDump("/root/page[6]/footer/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testZoom, "zoom.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_WW8EXPORT_TEST(testZoomType, "zoomtype.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomType") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(view::DocumentZoomType::PAGE_WIDTH), nValue);
}

DECLARE_WW8EXPORT_TEST(test56513, "fdo56513.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("This is the header of the first section"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header of the second section"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page header of the second section"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testNewPageStylesTable, "new-page-styles.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals and Results (Page 1)*"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Assessment (Page 2)****"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals: Next Year (Page 3)*******"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testFdo42144, "fdo42144.odt")
{
    // Footer wasn't disabled -- instead empty footer was exported.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<bool>(xStyle, "FooterIsOn")));
}

DECLARE_WW8EXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // WW8 has just one border attribute (sprmCBrc) for text border so all side has
    // the same border
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF3333,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Padding (dptSpace) it is constant 0
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // Shadow (fShadow)
    /* WW8 use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

DECLARE_WW8EXPORT_TEST(testFdo77454, "fdo77454.doc")
{
    {
        // check negative crops round-trip
        text::GraphicCrop const crop =
            getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
        CPPUNIT_ASSERT_EQUAL(sal_Int32( -439), crop.Left);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-7040), crop.Right);
        CPPUNIT_ASSERT_EQUAL(sal_Int32( -220), crop.Top);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-7040), crop.Bottom);
    }

    {
        // check positive crops round-trip
        text::GraphicCrop const crop =
            getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(  326), crop.Left);
        CPPUNIT_ASSERT_EQUAL(sal_Int32( 1208), crop.Right);
        CPPUNIT_ASSERT(abs(sal_Int32(1635) -  crop.Top) <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32(  95) - crop.Bottom) <= 2);
    }
}

DECLARE_WW8EXPORT_TEST(testFdo59530, "fdo59530.doc")
{
    // See ooxmlexport's testFdo38244().
    // Test comment range feature.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));

    // Test commented text range which spans over more text nodes
    // Comment starts in the second paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    // Comment ends in the third paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));
}

DECLARE_WW8EXPORT_TEST(testCommentsNested, "comments-nested.doc")
{
    uno::Reference<beans::XPropertySet> xOuter(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content"));

    uno::Reference<beans::XPropertySet> xInner(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content"));
}

DECLARE_WW8EXPORT_TEST(testBorderColoursExport, "bordercolours.odt")
{
    // This is very close to testBorderColours in ww8import.cxx, but for export

    // The following 6 colours can only be represented with WW9 (Word 2000)
    // BRC (BoRder Control) structures.  We can tell that they have been
    // exported/imported using a WW8 (Word '97) BRC if they instead come
    // through as one of the 16 colours listed at this link:
    // http://msdn.microsoft.com/en-us/library/dd773060.aspx
    table::BorderLine2 expectedTop(0xFA670C, 0, 53, 0, 1, 53);
    table::BorderLine2 expectedLeft(0xD99594, 0, 79, 0, 0, 79);
    table::BorderLine2 expectedRight(0xB2A1C7, 53, 53, 53, 3, 159);
    table::BorderLine2 expectedBottom(0xB6DDE8, 0, 106, 0, 14, 106);
    table::BorderLine2 expectedDashedRed(0xFA670C, 0, 53, 0, 2, 53);
    table::BorderLine2 expectedDoubleGreen(0xC2D69B, 26, 106, 26, 4, 159);

    // Paragraph border
    uno::Reference<text::XBookmarksSupplier> bookmarksSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> bookmarks(
        bookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> bookmark(
        bookmarks->getByName("ParagraphBorder"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> anchor(bookmark->getAnchor());
    table::BorderLine2 border;
    border = getProperty<table::BorderLine2>(anchor, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(anchor, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(anchor, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Page border
    OUString pageStyleName = getProperty<OUString>(anchor, "PageStyleName");
    uno::Reference<style::XStyle> pageStyle(
        getStyles("PageStyles")->getByName(pageStyleName), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(pageStyle, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(pageStyle, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(pageStyle, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(pageStyle, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Character border
    bookmark.set(bookmarks->getByName("CharBorder"), uno::UNO_QUERY);
    anchor = bookmark->getAnchor();
    border = getProperty<table::BorderLine2>(anchor, "CharTopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "CharLeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "CharRightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "CharBottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);

    // Table border
    uno::Reference<text::XTextTablesSupplier> tablesSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> tables(
        tablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> table(
        tables->getByName("Table1"), uno::UNO_QUERY);
    table::TableBorder2 tableBorder = getProperty<table::TableBorder2>(
        table, "TableBorder2");
    CPPUNIT_ASSERT_EQUAL(expectedTop.Color, tableBorder.TopLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedLeft.Color, tableBorder.LeftLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedRight.Color, tableBorder.RightLine.Color);
#if 0
    // #if'd out because the "fine dashed" border line style for table borders
    // does not seem to save or load correctly in odt format at present
    CPPUNIT_ASSERT_EQUAL(expectedBottom.Color, tableBorder.BottomLine.Color);
#endif

    // Table cells
    uno::Reference<table::XCell> cell(
        table->getCellByName("A2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
#if 0
    // #if'd out because the "fine dashed" border line style for table borders
    // does not seem to save or load correctly in odt format at present
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);
#endif

    cell.set(table->getCellByName("B2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    cell.set(table->getCellByName("C2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDashedRed, border);
    border = getProperty<table::BorderLine2>(cell, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    // Picture border
    // (#if'd out as they are not yet exported with correct colours)
#if 0
    bookmark.set(bookmarks->getByName("PictureBorder"),uno::UNO_QUERY);
    anchor = bookmark->getAnchor();
    border = getProperty<table::BorderLine2>(anchor, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(anchor, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(anchor, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);
#endif
}

DECLARE_WW8EXPORT_TEST(testRedlineExport1, "redline-export-1.odt")
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    //there must be no redline information on the first line before and after reloading
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(false, hasProperty(xRun, "RedlineType"));
    }
}

DECLARE_WW8EXPORT_TEST(testRedlineExport2, "redline-export-2.odt")
{
    //there must be redline information on the first portion of the third paragraph before and after reloading
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(3), 1), "RedlineType"));
}

DECLARE_WW8EXPORT_TEST(testRedlineExport3, "redline-export-3.odt")
{
    //there must be redline information just on the para-break boundary between para one and two
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(1), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 2), "RedlineType"));
}

DECLARE_WW8EXPORT_TEST(testCellBgColor, "cell-bg-color.odt")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xCC0000), getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor"));
}

DECLARE_WW8EXPORT_TEST(testBnc636128, "bnc636128.doc")
{
    // Import / export of FFData.cch was missing.
    uno::Reference<text::XFormField> xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(1), 2), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters = xFormField->getParameters();
    // This resulted in a container.NoSuchElementException.
    CPPUNIT_ASSERT_EQUAL(OUString("5"), xParameters->getByName("MaxLength").get<OUString>());
}


DECLARE_WW8EXPORT_TEST(testWw8Cjklist30, "cjklist30.doc")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist31, "cjklist31.doc")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist34, "cjklist34.doc")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist35, "cjklist35.doc")
{
    sal_Int16   numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testTdf92281, "tdf92281.doc")
{
        uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontName"));
        CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameAsian"));
        CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameComplex"));

        uno::Reference<text::XText> xXText(getParagraph(1)->getText(), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursor( xXText->createTextCursor() , uno::UNO_QUERY );

        xCursor->goRight( 5 , false );
        uno::Reference< beans::XPropertySet > xPropSet(xCursor, uno::UNO_QUERY);
        const sal_Unicode aFontname[14] = { 26032, 32048, 26126, 39636, ';' , 'P' , 'M', 'i', 'n', 'g', 'L', 'i', 'U' , 0 };
        CPPUNIT_ASSERT_EQUAL(OUString("Calibri"), getProperty<OUString>(xPropSet, "CharFontName"));
        CPPUNIT_ASSERT_EQUAL(OUString(aFontname), getProperty<OUString>(xPropSet, "CharFontNameAsian"));
        CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), getProperty<OUString>(xPropSet, "CharFontNameComplex"));
}

DECLARE_WW8EXPORT_TEST(testCommentedTable, "commented-table.doc")
{
    // Document has a non-trivial commented text range, as the range contains a table.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);
    // After first import, there was an off-by-one during import, so this was "efore.\nA1\nB1\nAfte". (Notice the additional "e" prefix.)
    // After export and import, things got worse, this was "\nA1\nB1\nAfte".
    CPPUNIT_ASSERT_EQUAL(OUString("fore." SAL_NEWLINE_STRING "A1" SAL_NEWLINE_STRING "B1" SAL_NEWLINE_STRING "Afte"), xField->getAnchor()->getString());
}

DECLARE_WW8EXPORT_TEST(testTextVerticalAdjustment, "tdf36117_verticalAdjustment.doc")
{
    //Preserve the page vertical alignment setting for .doc
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwPageDesc &Desc = pDoc->GetPageDesc( 0 );
    drawing::TextVerticalAdjust nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_CENTER, nVA );

    Desc = pDoc->GetPageDesc( 1 );
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_TOP, nVA );

    Desc = pDoc->GetPageDesc( 2 );
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BOTTOM, nVA );

    Desc = pTextDoc->GetDocShell()->GetDoc()->GetPageDesc( 3 );
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BLOCK, nVA );
}

DECLARE_WW8EXPORT_TEST(testRES_MIRROR_GRAPH_BOTH, "tdf56321_flipImage_both.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    for (int n = 0; ; n++)
    {
        SwNode* pNode = pDoc->GetNodes()[ n ];
        if (SwGrfNode *pGrfNode = pNode->GetGrfNode())
        {
            CPPUNIT_ASSERT(pGrfNode->GetSwAttrSet().GetMirrorGrf().GetValue() == 3);
            break;
        }
    }
}

DECLARE_WW8EXPORT_TEST(testCommentExport, "comment-export.odt")
{
    struct TextPortionInfo {
        OUString sKind;
        OUString sText;
        int nAnnotationID;
    };

    const TextPortionInfo aTextPortions[] = {
        {OUString("Annotation"), OUString("Comment on [A...A]"), 0},
        {OUString("Text"), OUString("[A xx "), 0},
        {OUString("Annotation"), OUString("Comment on [B...B]"), 1},
        {OUString("Text"), OUString("[B x "), 0},
        {OUString("Annotation"), OUString("Comment on [C..C]"), 2},
        {OUString("Text"), OUString("[C x B]"), 0},
        {OUString("AnnotationEnd"), OUString(""), 1},
        {OUString("Text"), OUString(" x C]"), 0},
        {OUString("AnnotationEnd"), OUString(""), 2},
        {OUString("Text"), OUString(" xx A]"), 0},
        {OUString("AnnotationEnd"), OUString(""), 0},
        {OUString("Text"), OUString(" Comment on a point"), 0},
        {OUString("Annotation"), OUString("Comment on point"), 3},
        {OUString("Text"), OUString("x "), 0},
        {OUString("Annotation"), OUString("Comment on AA...BB"), 4},
        {OUString("Annotation"), OUString("Comment on AAAAAA"), 5},
        {OUString("Text"), OUString("AAAAAA"), 0},
        {OUString("AnnotationEnd"), OUString(""), 5},
        {OUString("Text"), OUString(" BBBBBB"), 0},
        {OUString("AnnotationEnd"), OUString(""), 4}
    };

    OUString sNames[6];

    const int nNumberOfTextPortions = sizeof(aTextPortions) / (sizeof(TextPortionInfo));

    uno::Reference<text::XTextRange> xPara = getParagraph(1);

    for (int i = 0; i < nNumberOfTextPortions; ++i)
    {
        OUString sKind = aTextPortions[i].sKind;
        uno::Reference<text::XTextRange> xRun(getRun(xPara, i + 1), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xRun, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sKind, getProperty<OUString>(xPropertySet, "TextPortionType"));

        if (sKind == "Text")
        {
            // Check if textportion has the correct text
            CPPUNIT_ASSERT_EQUAL(aTextPortions[i].sText, xRun->getString());
        }
        else if (sKind == "Annotation")
        {
            // Check if the comment text is correct and save the name of the comment
            uno::Reference<beans::XPropertySet> xComment(getProperty< uno::Reference<beans::XPropertySet> >(xRun, "TextField"), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(aTextPortions[i].sText, getProperty<OUString>(xComment, "Content"));
            sNames[aTextPortions[i].nAnnotationID] = getProperty<OUString>(xComment, "Name");
        }
        else // if (sKind == OUString("AnnotationEnd"))
        {
            // Check if the correct Annotation ends here (by Name)
            uno::Reference<container::XNamed> xBookmark(getProperty< uno::Reference<beans::XPropertySet> >(xRun, "Bookmark"), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(sNames[aTextPortions[i].nAnnotationID], xBookmark->getName());
        }
    }
}

#if !defined(MACOSX)
DECLARE_WW8EXPORT_TEST(testTableKeep, "tdf91083.odt")
{
    //emulate table "keep with next" -do not split table
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[3]/body/tab[1]/row[2]/cell[1]/txt[1]") );
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[6]/body/tab[1]/row[2]/cell[1]/txt[1]") );
}
#endif

DECLARE_WW8EXPORT_TEST(testMoveRange, "fdo66304-1.odt")
{
    //the save must survive without asserting
}

DECLARE_WW8EXPORT_TEST(testClearFramePams, "tdf46441-2.odt")
{
    //the save must survive without asserting
}

DECLARE_WW8EXPORT_TEST(testTdf94386, "tdf94386.odt")
{
    // check that the first and next page use different page styles
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    xCursor->jumpToFirstPage();
    OUString firstPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xCursor->jumpToLastPage();
    OUString lastPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    CPPUNIT_ASSERT(firstPageStyleName != lastPageStyleName);

    uno::Reference<beans::XPropertySet> xFirstPropertySet(getStyles("PageStyles")->getByName(firstPageStyleName), uno::UNO_QUERY);
    awt::Size fSize;
    xFirstPropertySet->getPropertyValue("Size") >>= fSize;

    uno::Reference<beans::XPropertySet> xNextPropertySet(getStyles("PageStyles")->getByName(lastPageStyleName), uno::UNO_QUERY);
    awt::Size lSize;
    xNextPropertySet->getPropertyValue("Size") >>= lSize;

    CPPUNIT_ASSERT((fSize.Width != lSize.Width) && (fSize.Height != lSize.Height));
}

DECLARE_WW8EXPORT_TEST(testTdf99474, "tdf99474.odt")
{
    // The bullet colour of paragraph #3 should be COL_AUTO
    auto xPara = getParagraph(3);
    uno::Reference<container::XIndexReplace> xNumRules(
        getProperty< uno::Reference<container::XIndexReplace> >(
            xPara, "NumberingRules"),
        uno::UNO_QUERY);
    int numLevel = getProperty<sal_Int32>(xPara, "NumberingLevel");
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    xNumRules->getByIndex(numLevel) >>= aPropertyValues;
    OUString charStyleName;
    for(int j = 0 ; j< aPropertyValues.getLength() ; ++j)
    {
        auto aProp = aPropertyValues[j];
        if (aProp.Name == "CharStyleName") {
            charStyleName = aProp.Value.get<OUString>();
            break;
        }
    }
    CPPUNIT_ASSERT(charStyleName.getLength());
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles("CharacterStyles")->getByName(charStyleName),
        uno::UNO_QUERY);
    ColorData charColor = getProperty<util::Color>(xStyle, "CharColor");
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, charColor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
