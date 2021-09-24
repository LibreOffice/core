/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string_view>
#include <config_features.h>

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <o3tl/cppunittraitshelper.hxx>
#include <unotools/fltrcfg.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <tools/datetimeutils.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/propertysequence.hxx>
#include <osl/time.h>
#include <comphelper/processfactory.hxx>
#include <vcl/TypeSerializer.hxx>
#include <comphelper/scopeguard.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlimport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testImageHyperlink)
{
    load(mpTestDocumentPath, "image-hyperlink.docx");
    OUString URL = getProperty<OUString>(getShape(1), "HyperLinkURL");
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.libreoffice.org/"), URL);
}

CPPUNIT_TEST_FIXTURE(Test, testMathMalformedXml)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "math-malformed_xml.docx";
    mxComponent = mxDesktop->loadComponentFromURL(aURL, "_default", 0, {});
    CPPUNIT_ASSERT(!mxComponent.is());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103931)
{
    load(mpTestDocumentPath, "tdf103931.docx");
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was 2, the last (empty) section of the document was lost on import.
    // (import test only: Columns/Sections do not round-trip well)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xTextSections->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testN751017)
{
    load(mpTestDocumentPath, "n751017.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xMasters(xTextFieldsSupplier->getTextFieldMasters());
    // Make sure we have a variable named foo.
    CPPUNIT_ASSERT(xMasters->hasByName("com.sun.star.text.FieldMaster.SetExpression.foo"));

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    bool bFoundSet(false), bFoundGet(false);
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        sal_Int16 nValue = 0;
        OUString aValue;
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            bFoundSet = true;
            uno::Reference<text::XDependentTextField> xDependentTextField(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xMasterProps(xDependentTextField->getTextFieldMaster());

            // First step: did we set foo to "bar"?
            xMasterProps->getPropertyValue("Name") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("foo"), aValue);
            xPropertySet->getPropertyValue("SubType") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(text::SetVariableType::STRING, nValue);
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("bar"), aValue);
        }
        else if (xServiceInfo->supportsService("com.sun.star.text.TextField.GetExpression"))
        {
            // Second step: check the value of foo.
            bFoundGet = true;
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("foo"), aValue);
            xPropertySet->getPropertyValue("SubType") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(text::SetVariableType::STRING, nValue);
            xPropertySet->getPropertyValue("CurrentPresentation") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("bar"), aValue);
        }
    }
    CPPUNIT_ASSERT(bFoundSet);
    CPPUNIT_ASSERT(bFoundGet);
}

CPPUNIT_TEST_FIXTURE(Test, testN757890)
{
    load(mpTestDocumentPath, "n757890.docx");
    // The w:pStyle token affected the text outside the textbox.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xPara->getPropertyValue("ParaStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), aValue);

    // This wasn't centered
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nValue;
    xFrame->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, nValue);
}

CPPUNIT_TEST_FIXTURE(Test, testN751077)
{
    load(mpTestDocumentPath, "n751077.docx");
/*
xray ThisComponent.DrawPage(1).getByIndex(0).String
xray ThisComponent.DrawPage(1).getByIndex(0).Anchor.PageStyleName
*/
    uno::Reference<drawing::XShapes> xShapes(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xShapes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TEXT1\n"), xShape->getString());
    // we want to test the textbox is on the first page (it was put onto another page without the fix),
    // use a small trick and instead of checking the page layout, check the page style
    uno::Reference<text::XTextContent> xTextContent(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), getProperty<OUString>(xTextContent->getAnchor(), "PageStyleName"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129237)
{
    load(mpTestDocumentPath, "tdf129237.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title (fixed)"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("title new"), xEnumerationAccess1->getPresentation(false).trim());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title (fixed)"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("MoM is supreme"), xEnumerationAccess2->getPresentation(false).trim());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title (fixed)"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("MY PATNA IS BEST IN THE WORLD"), xEnumerationAccess3->getPresentation(false).trim());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title (fixed)"), xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("Title New"), xEnumerationAccess4->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134572)
{
    load(mpTestDocumentPath, "tdf134572.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    // Without the fix in place, this test would have failed with
    // - Expected: RK - Risk / EH&S
    // - Actual  : [Responsible Office]
    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("RK - Risk / EH&S"), xEnumerationAccess1->getPresentation(false).trim());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    // - Expected: Choose an item.
    // - Actual  : A.M.
    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Choose an item."), xEnumerationAccess2->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128076)
{
    load(mpTestDocumentPath, "tdf128076.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("User Field adres = Test"), xEnumerationAccess->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("Test"), xEnumerationAccess->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testfdo90720)
{
    load(mpTestDocumentPath, "testfdo90720.docx");
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    uno::Reference<text::XTextFrame> textbox(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> properties(textbox, uno::UNO_QUERY);
    sal_Int32 fill_transperence;
    properties->getPropertyValue( "FillTransparence" ) >>= fill_transperence;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(100), fill_transperence );
}

CPPUNIT_TEST_FIXTURE(Test, testN760764)
{
    load(mpTestDocumentPath, "n760764.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());

    // Access the second run, which is a textfield
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    float fValue;
    xRun->getPropertyValue("CharHeight") >>= fValue;
    // This used to be 11, as character properties were ignored.
    CPPUNIT_ASSERT_EQUAL(8.f, fValue);
}

CPPUNIT_TEST_FIXTURE(Test, testN764745)
{
    load(mpTestDocumentPath, "n764745-alignment.docx");
/*
shape = ThisComponent.DrawPage.getByIndex(0)
xray shape.AnchorType
xray shape.AnchorPosition.X
xray ThisComponent.StyleFamilies.PageStyles.Default.Width
*/
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    // The paragraph is right-aligned and the picture does not explicitly specify position,
    // so check it's anchored as character and in the right side of the document.
    text::TextContentAnchorType anchorType;
    xPropertySet->getPropertyValue("AnchorType") >>= anchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, anchorType);
    awt::Point pos;
    xPropertySet->getPropertyValue("AnchorPosition") >>= pos;
    uno::Reference<style::XStyleFamiliesSupplier> styleFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> styleFamilies = styleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> pageStyles;
    styleFamilies->getByName("PageStyles") >>= pageStyles;
    uno::Reference<uno::XInterface> defaultStyle;
    pageStyles->getByName("Standard") >>= defaultStyle;
    uno::Reference<beans::XPropertySet> styleProperties( defaultStyle, uno::UNO_QUERY );
    sal_Int32 width = 0;
    styleProperties->getPropertyValue( "Width" ) >>= width;
    CPPUNIT_ASSERT( pos.X > width / 2 );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115719b)
{
    load(mpTestDocumentPath, "tdf115719b.docx");
    // This was 0, 4th (last) paragraph had no increased spacing.
    CPPUNIT_ASSERT(getProperty<sal_Int32>(getParagraph(4), "ParaTopMargin") > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testN766477)
{
    load(mpTestDocumentPath, "n766477.docx");
    /*
     * The problem was that the checkbox was not checked.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.Bookmark.Parameters.ElementNames(0) 'Checkbox_Checked
     */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XFormField> xFormField(xRun->getPropertyValue("Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xParameters(xFormField->getParameters());
    uno::Sequence<OUString> aElementNames(xParameters->getElementNames());
    CPPUNIT_ASSERT_EQUAL(OUString("Checkbox_Checked"), aElementNames[0]);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130804)
{
    load(mpTestDocumentPath, "tdf130804.docx");
    OUString flyHeight = parseDump("/root/page/body/txt[1]/infos/bounds", "height");
    OUString txtHeight = parseDump("/root/page/body/txt[1]/anchored/fly/infos/bounds", "height");

    //Without the fix in place, txtHeight would have been flyHeight + 55
    CPPUNIT_ASSERT_EQUAL(flyHeight, txtHeight);

    // Also check the bookmark portion is ignored in the next paragraph
    OUString aTop = parseDump("/root/page/body/txt[2]/infos/prtBounds", "top");
    CPPUNIT_ASSERT_EQUAL(OUString("240"), aTop);
}

CPPUNIT_TEST_FIXTURE(Test, testN758883)
{
    load(mpTestDocumentPath, "n758883.docx");
    /*
     * The problem was that direct formatting of the paragraph was not applied
     * to the numbering. This is easier to test using a layout dump.
     */
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[1]", "nHeight", "220");

    // check the bookmark portions are of the expected height
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[2]", "nType", "PortionType::Bookmark");
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[2]", "nHeight", "253");
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[3]", "nType", "PortionType::Bookmark");
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[3]", "nHeight", "253");

    /*
     * Next problem was that the page margin contained the width of the page border as well.
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.LeftMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(847), nValue);

    // No assert for the 3rd problem: see the comment in the test doc.

    /*
     * 4th problem: Wrap type of the textwrape was not 'through'.
     *
     * xray ThisComponent.DrawPage(0).Surround ' was 2, should be 1
     */
    xPropertySet.set(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, eValue);

    /*
     * 5th problem: anchor type of the second textbox was wrong.
     *
     * xray ThisComponent.DrawPage(1).AnchorType ' was 1, should be 4
     */
    xPropertySet.set(getShape(2), uno::UNO_QUERY);
    text::TextContentAnchorType eAnchorType;
    xPropertySet->getPropertyValue("AnchorType") >>= eAnchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eAnchorType);

    // 6th problem: xray ThisComponent.DrawPage(2).AnchorType ' was 2, should be 4
    xPropertySet.set(getShape(3), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("AnchorType") >>= eAnchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eAnchorType);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf74367_MarginsZeroed)
{
    load(mpTestDocumentPath, "tdf74367_MarginsZeroed.docx");
    // Do not import page borders with 'None' style, or else it will change the page margins.
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("TopMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501), nValue);
    xPropertySet->getPropertyValue("RightMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501), nValue);
    xPropertySet->getPropertyValue("BottomMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501), nValue);
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501), nValue);
}

CPPUNIT_TEST_FIXTURE(Test, testBnc773061)
{
    load(mpTestDocumentPath, "bnc773061.docx");
    uno::Reference< text::XTextRange > paragraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > normal = getRun( paragraph, 1, "Normal " );
    uno::Reference< text::XTextRange > raised = getRun( paragraph, 2, "Raised" );
    uno::Reference< text::XTextRange > lowered = getRun( paragraph, 4, "Lowered" );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), getProperty< sal_Int32 >( normal, "CharEscapement" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 50 ), getProperty< sal_Int32 >( raised, "CharEscapement" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( -25 ), getProperty< sal_Int32 >( lowered, "CharEscapement" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 100 ), getProperty< sal_Int32 >( normal, "CharEscapementHeight" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 100 ), getProperty< sal_Int32 >( raised, "CharEscapementHeight" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 100 ), getProperty< sal_Int32 >( lowered, "CharEscapementHeight" ));
}

CPPUNIT_TEST_FIXTURE(Test, testN775899)
{
    load(mpTestDocumentPath, "n775899.docx");
    /*
     * The problem was that a floating table wasn't imported as a frame, then it contained fake paragraphs.
     *
     * ThisComponent.TextFrames.Count ' was 0
     * oParas = ThisComponent.TextFrames(0).Text.createEnumeration
     * oPara = oParas.nextElement
     * oPara.supportsService("com.sun.star.text.TextTable") 'was a fake paragraph
     * oParas.hasMoreElements 'was true
     */
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextFrame> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xFrame->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<lang::XServiceInfo> xServiceInfo(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_True, xServiceInfo->supportsService("com.sun.star.text.TextTable"));

    CPPUNIT_ASSERT_EQUAL(sal_False, xParaEnum->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testN777345)
{
    load(mpTestDocumentPath, "n777345.docx");
    // The problem was that v:imagedata inside v:rect was ignored.
    uno::Reference<document::XEmbeddedObjectSupplier2> xSupplier(getShape(1), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = xSupplier->getReplacementGraphic();
    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL( Size( 17, 16 ), aBitmap.GetSizePixel());
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, aBitmap.GetPixelColor( 0, 0 ));
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, aBitmap.GetPixelColor( 16, 15 ));
    CPPUNIT_ASSERT_EQUAL( Color( 153, 0, 0 ), aBitmap.GetPixelColor( 16, 0 ));
    CPPUNIT_ASSERT_EQUAL( Color( 153, 0, 0 ), aBitmap.GetPixelColor( 0, 15 ));
}

CPPUNIT_TEST_FIXTURE(Test, testN778140)
{
    load(mpTestDocumentPath, "n778140.docx");
    /*
     * The problem was that the paragraph top/bottom margins were incorrect due
     * to unhandled w:doNotUseHTMLParagraphAutoSpacing.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testInk)
{
    load(mpTestDocumentPath, "ink.docx");
    /*
     * The problem was that ~nothing was imported, except an empty CustomShape.
     *
     * xray ThisComponent.DrawPage(0).supportsService("com.sun.star.drawing.OpenBezierShape")
     */
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.OpenBezierShape"));
}

CPPUNIT_TEST_FIXTURE(Test, testN779627)
{
    load(mpTestDocumentPath, "n779627.docx");
    /*
     * The problem was that the table left position was based on the tableCellMar left value
     * even for nested tables, while it shouldn't.
     */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Any aValue = xTableProperties->getPropertyValue("LeftMargin");
    sal_Int32 nLeftMargin;
    aValue >>= nLeftMargin;
    // only border width considered.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Left margin shouldn't take tableCellMar into account in nested tables",
            sal_Int32(9), nLeftMargin);

    /*
     * Another problem tested with this document is the loading of the shapes
     * anchored to a hidden header or footer
     */
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testN779627b)
{
    load(mpTestDocumentPath, "n779627b.docx");
    /*
     * Another problem tested with the original n779627.docx document (before removing its unnecessary
     * shape loading) is that the roundrect is centered vertically and horizontally.
     */
    uno::Reference<beans::XPropertySet> xShapeProperties( getShape(1), uno::UNO_QUERY );
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
    // If this goes wrong, probably the index of the shape is changed and the test should be adjusted.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.RectangleShape"), xShapeDescriptor->getShapeType());
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to page", text::RelOrientation::PAGE_FRAME, nValue);
    xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered vertically", text::VertOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("VertOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered vertically relatively to page", text::RelOrientation::PAGE_FRAME, nValue);
}

CPPUNIT_TEST_FIXTURE(Test, testN782061)
{
    load(mpTestDocumentPath, "n782061.docx");
    /*
     * The problem was that the character escapement in the second run was -58.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-9), getProperty<sal_Int32>(getRun(getParagraph(1), 2), "CharEscapement"));
}

CPPUNIT_TEST_FIXTURE(Test, testN773061)
{
    load(mpTestDocumentPath, "n773061.docx");
// xray ThisComponent.TextFrames(0).LeftBorderDistance
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), getProperty< sal_Int32 >( xFrame, "LeftBorderDistance" ) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), getProperty< sal_Int32 >( xFrame, "TopBorderDistance" ) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), getProperty< sal_Int32 >( xFrame, "RightBorderDistance" ) );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), getProperty< sal_Int32 >( xFrame, "BottomBorderDistance" ) );
}

CPPUNIT_TEST_FIXTURE(Test, testN780645)
{
    load(mpTestDocumentPath, "n780645.docx");
    // The problem was that when the number of cells didn't match the grid, we
    // didn't take care of direct cell widths.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2135), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position); // was 1999
}

CPPUNIT_TEST_FIXTURE(Test, testWordArtResizing)
{
    load(mpTestDocumentPath, "WordArt.docx");
    /* The Word-Arts and watermarks were getting resized automatically, It was as if they were
       getting glued to the fallback geometry(the sdrObj) and were getting bound to the font size.
       The test-case ensures the original height and width of the word-art is not changed while importing*/
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10105), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4755), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshapeLine)
{
    load(mpTestDocumentPath, "groupshape-line.docx");
    /*
     * Another fallout from n#792778, this time first the lines inside a
     * groupshape wasn't imported, then the fix broke the size/position of
     * non-groupshape lines. Test both here.
     *
     * xray ThisComponent.DrawPage.Count ' 2 shapes
     * xray ThisComponent.DrawPage(0).Position 'x: 2656, y: 339
     * xray ThisComponent.DrawPage(0).Size ' width: 3270, height: 1392
     * xray ThisComponent.DrawPage(1).getByIndex(0).Position 'x: 1272, y: 2286
     * xray ThisComponent.DrawPage(1).getByIndex(0).Size 'width: 10160, height: 0
     */
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2656), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(339), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3270), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1392), xShape->getSize().Height);

    uno::Reference<drawing::XShapes> xGroupShape(getShape(2), uno::UNO_QUERY);
    xShape.set(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1272), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2286), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10160), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshapeChildRotation)
{
    load(mpTestDocumentPath, "groupshape-child-rotation.docx");
    // The problem was that (due to incorrect handling of rotation inside
    // groupshapes), the first child wasn't in the top left corner of an inline
    // groupshape.
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-5741), xShape->getPosition().Y);

#if HAVE_MORE_FONTS
    xShape.set(xGroupShape->getByIndex(4), uno::UNO_QUERY);
    // This was true, a VML textbox without <v:textbox style="mso-fit-shape-to-text:t"> had
    // auto-grow on.
    CPPUNIT_ASSERT(!getProperty<bool>(xShape, "TextAutoGrowHeight"));
    // Paragraph Style Normal should provide the font name - which slightly affects the shape's height (was 686)
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font", OUString("Times New Roman"), getProperty<OUString>(getRun(xText, 1), "CharFontName"));
#endif

    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroupShape->getByIndex(5), uno::UNO_QUERY);
    // This was com.sun.star.drawing.RectangleShape, all shape text in a single line.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.TextShape"), xShapeDescriptor->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testTableWidth)
{
    load(mpTestDocumentPath, "table_width.docx");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Relative width wasn't recognized during import.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(0), "IsWidthRelative"));

    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xFrames->getByIndex(0), "FrameWidthPercent"));
}

CPPUNIT_TEST_FIXTURE(Test, testN820788)
{
    load(mpTestDocumentPath, "n820788.docx");
    // The problem was that AutoSize was not enabled for the text frame.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    // This was text::SizeType::FIX.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::MIN, getProperty<sal_Int16>(xFrame, "SizeType"));
}

CPPUNIT_TEST_FIXTURE(Test, testN820504)
{
    load(mpTestDocumentPath, "n820504.docx");
    uno::Reference<style::XStyleFamiliesSupplier> xFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xFamiliesAccess = xFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStylesAccess(xFamiliesAccess->getByName("ParagraphStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStylesAccess->getByName("Default Paragraph Style"), uno::UNO_QUERY);
    // The problem was that the CharColor was set to AUTO (-1) even if we have some default char color set
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4040635), getProperty<sal_Int32>(xStyle, "CharColor"));

    // Also, the groupshape was anchored at-page instead of at-character
    // (that's incorrect as Word only supports at-character and as-character).
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo43641)
{
    load(mpTestDocumentPath, "fdo43641.docx");
    uno::Reference<container::XIndexAccess> xGroupLockedCanvas(getShape(1), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xGroupShape(xGroupLockedCanvas->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xLine(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    // This was 2200, not 2579 in mm100, i.e. the size of the line shape was incorrect.
    // File cx=928694EMU = 2579.7Hmm, round up 2580Hmm. Currently off by 1.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2581), xLine->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshapeSdt)
{
    load(mpTestDocumentPath, "groupshape-sdt.docx");
    // All problems here are due to the groupshape: we have a drawinglayer rectangle, not a writer textframe.
    uno::Reference<drawing::XShapes> xOuterGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xInnerGroupShape(xOuterGroupShape->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    // Border distances were not implemented, this was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1905), getProperty<sal_Int32>(xShape, "TextUpperDistance"));
    // Sdt field result wasn't imported, this was "".
    CPPUNIT_ASSERT_EQUAL(OUString("placeholder text"), xShape->getString());
    // w:spacing was ignored in oox, this was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xShape->getText()), 1), "CharKerning"));
}

static void lcl_countTextFrames(const css::uno::Reference< lang::XComponent >& xComponent,
   sal_Int32 nExpected )
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( nExpected, xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testBnc779620)
{
    load(mpTestDocumentPath, "bnc779620.docx");
    // The problem was that the floating table was imported as a non-floating one.
    lcl_countTextFrames( mxComponent, 1 );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105127)
{
    load(mpTestDocumentPath, "tdf105127.docx");
    auto aPolyPolygon = getProperty<drawing::PolyPolygonBezierCoords>(getShape(1), "PolyPolygonBezier");
    // tdf#106792 These values were wrong all the time due to a missing
    // conversion in SvxShapePolyPolygon::getPropertyValueImpl. There was no
    // ForceMetricTo100th_mm -> the old results were in twips due to the
    // object residing in Writer. The UNO API by definition is in 100thmm,
    // thus I will correct the value here.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5719), aPolyPolygon.Coordinates[0][0].Y); // was: 3257
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105143)
{
    load(mpTestDocumentPath, "tdf105143.docx");
    OUString aTop = parseDump("/root/page/body/txt/anchored/SwAnchoredDrawObject/bounds", "top");
    // This was 6272, i.e. the shape was moved up (incorrect position) to be
    // inside the page rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("6731"), aTop);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105975)
{
    load(mpTestDocumentPath, "105975.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xMasters(xTextFieldsSupplier->getTextFieldMasters());
    // Make sure we have a variable named TEST_VAR.
    CPPUNIT_ASSERT(xMasters->hasByName("com.sun.star.text.FieldMaster.SetExpression.TEST_VAR"));
}

CPPUNIT_TEST_FIXTURE(Test, testfdo76583)
{
    load(mpTestDocumentPath, "fdo76583.docx");
    // The problem was that the floating table was imported as a non-floating one.
    // floating tables are imported as text frames, therefore the document should
    // exactly 1 text frame.
    lcl_countTextFrames( mxComponent, 1 );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105975formula)
{
    load(mpTestDocumentPath, "tdf105975.docx");
    // Make sure the field contains a formula with 10 + 15
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("10+15"), xEnumerationAccess->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("25"), xEnumerationAccess->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133647)
{
    load(mpTestDocumentPath, "tdf133647.docx");
    /* Tests that argument lists, cell references, and cell ranges are translated correctly
     * when importing table formulae from MS Word */
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SUM(1|2|3)"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("6"), xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("sum(<A1>|<B1>)"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xEnumerationAccess2->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("(SUM(<C1>|5)*(2+7))*(3+SUM(1|<B1>))"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("432"), xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1+(SUM(1|2))"), xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("4"), xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess5(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("3*(2+SUM(<A1:C1>)+7)"), xEnumerationAccess5->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("45"), xEnumerationAccess5->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess6(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("(1+2)*SUM(<C1>|<D1>)"), xEnumerationAccess6->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("21"), xEnumerationAccess6->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess7(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SUM(<A1>|5|<B1:C1>|6)"), xEnumerationAccess7->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("17"), xEnumerationAccess7->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess8(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SUM(<C1:D1>)"), xEnumerationAccess8->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("7"), xEnumerationAccess8->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess9(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("SUM(<A1>|<B1>)"), xEnumerationAccess9->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xEnumerationAccess9->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123386)
{
    load(mpTestDocumentPath, "tdf123386.docx");
    /* Tests that argument lists, cell references, and cell ranges are translated correctly
     * when importing table formulae from MS Word */
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("<A1> L 2"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess1->getPresentation(false).trim());

    /* Ensures non-cell references passed to DEFINED() are preserved.
     * Doesn't test the display string because LO doesn't support DEFINED(). */
    uno::Reference<text::XTextField> xEnumerationAccess10(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((1) AND (DEFINED(ABC1)))"), xEnumerationAccess10->getPresentation(true).trim());

    uno::Reference<text::XTextField> xEnumerationAccess9(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("NOT(TRUE)"), xEnumerationAccess9->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("0"), xEnumerationAccess9->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess8(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((TRUE) OR (FALSE))"), xEnumerationAccess8->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess8->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess7(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((<A1> EQ 1) OR (<B1> EQ 2))"), xEnumerationAccess7->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess7->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess6(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("(((<A1> L 1)) AND ((<B1> NEQ 2)))"), xEnumerationAccess6->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("0"), xEnumerationAccess6->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess5(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((<A1> EQ 1) AND (<B1> EQ 2))"), xEnumerationAccess5->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess5->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess4(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("<D1> NEQ 3"), xEnumerationAccess4->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess4->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("<C1> EQ 3"), xEnumerationAccess3->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess3->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("<B1> G 1"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("1"), xEnumerationAccess2->getPresentation(false).trim());

}

CPPUNIT_TEST_FIXTURE(Test, testTdf133647_unicode)
{
    load(mpTestDocumentPath, "tdf133647_unicode.docx");
    /* Tests that non-ASCII characters in formulas are preserved when importing from MS Word */
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    xFields->nextElement();
    xFields->nextElement();
    xFields->nextElement();

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(u"defined(預期結果)"), xEnumerationAccess1->getPresentation(true).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(u"defined(نتيجةمتوقعة)"), xEnumerationAccess2->getPresentation(true).trim());

    uno::Reference<text::XTextField> xEnumerationAccess3(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(u"defined(ExpectedResult)"), xEnumerationAccess3->getPresentation(true).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123389)
{
    load(mpTestDocumentPath, "tdf123389.docx");
    /* Tests that argument lists, cell references, and cell ranges are translated correctly
     * when importing table formulae from MS Word */
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((2.345) ROUND (1))"), xEnumerationAccess1->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("2.3"), xEnumerationAccess1->getPresentation(false).trim());

    uno::Reference<text::XTextField> xEnumerationAccess2(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("((<A1>) ROUND (2))"), xEnumerationAccess2->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("2.35"), xEnumerationAccess2->getPresentation(false).trim());
}


CPPUNIT_TEST_FIXTURE(Test, testTdf107784)
{
    load(mpTestDocumentPath, "tdf107784.docx");
    // Make sure the field displays the citation's title and not the identifier
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    if( !xFields->hasMoreElements() ) {
        CPPUNIT_ASSERT(false);
        return;
    }

    uno::Reference<text::XTextField> xEnumerationAccess(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Bibliography entry"), xEnumerationAccess->getPresentation(true).trim());
    CPPUNIT_ASSERT_EQUAL(OUString("(Smith, 1950)"), xEnumerationAccess->getPresentation(false).trim());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115883)
{
    load(mpTestDocumentPath, "tdf115883.docx");
    // Import failed due to an unhandled exception when getting the Surround
    // property of a not yet inserted frame.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf75573)
{
    load(mpTestDocumentPath, "tdf75573_page1frame.docx");
    // the problem was that the frame was discarded
    // when an unrelated, unused, odd-header was flagged as discardable
    lcl_countTextFrames( mxComponent, 1 );

    // the frame should be on page 1
    CPPUNIT_ASSERT_EQUAL( OUString("lorem ipsum"), parseDump("/root/page[1]/body/section/txt/anchored/fly/txt[1]/text()") );

    // the "Proprietary" style should set the vertical and horizontal anchors to the page
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xPropertySet, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xPropertySet, "HoriOrientRelation"));

    // the frame should be located near the bottom[23186]/center[2955] of the page
    CPPUNIT_ASSERT(sal_Int32(20000) < getProperty<sal_Int32>(xPropertySet, "VertOrientPosition"));
    CPPUNIT_ASSERT(sal_Int32(2500) < getProperty<sal_Int32>(xPropertySet, "HoriOrientPosition"));

    css::uno::Reference<css::lang::XMultiServiceFactory> m_xTextFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xSettings(m_xTextFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    uno::Any aProtect = xSettings->getPropertyValue("ProtectForm");
    bool bProt = true;
    aProtect >>= bProt;
    CPPUNIT_ASSERT(!bProt);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf75573_lostTable)
{
    load(mpTestDocumentPath, "tdf75573_lostTable.docx");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of tables", sal_Int32(1), xTables->getCount() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of frames/shapes", 0, getShapes() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of pages", 3, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109316_dropCaps)
{
    load(mpTestDocumentPath, "tdf109316_dropCaps.docx");
    uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);
    css::style::DropCapFormat aDropCap = getProperty<css::style::DropCapFormat>(xSet,"DropCapFormat");
    CPPUNIT_ASSERT_EQUAL( sal_Int8(2), aDropCap.Lines );
    CPPUNIT_ASSERT_EQUAL( sal_Int8(1), aDropCap.Count );
    CPPUNIT_ASSERT_EQUAL( sal_Int16(1270), aDropCap.Distance );

    xSet.set(getParagraph(2), uno::UNO_QUERY);
    aDropCap = getProperty<css::style::DropCapFormat>(xSet,"DropCapFormat");
    CPPUNIT_ASSERT_EQUAL( sal_Int8(3), aDropCap.Lines );
    CPPUNIT_ASSERT_EQUAL( sal_Int8(1), aDropCap.Count );
    CPPUNIT_ASSERT_EQUAL( sal_Int16(508), aDropCap.Distance );

    xSet.set(getParagraph(3), uno::UNO_QUERY);
    aDropCap = getProperty<css::style::DropCapFormat>(xSet,"DropCapFormat");
    CPPUNIT_ASSERT_EQUAL( sal_Int8(4), aDropCap.Lines );
    CPPUNIT_ASSERT_EQUAL( sal_Int8(7), aDropCap.Count );
    CPPUNIT_ASSERT_EQUAL( sal_Int16(0), aDropCap.Distance );
}

CPPUNIT_TEST_FIXTURE(Test, lineWpsOnly)
{
    load(mpTestDocumentPath, "line-wps-only.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was -7223 as it was set after the CustomShapeGeometry property.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(210), xShape->getPosition().X);
}

CPPUNIT_TEST_FIXTURE(Test, lineRotation)
{
    load(mpTestDocumentPath, "line-rotation.docx");
    uno::Reference<drawing::XShape> xShape = getShape(3);
    // This was 5096: the line was shifted towards the bottom, so the end of
    // the 3 different lines wasn't at the same point.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4808), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(Test, textboxWpsOnly)
{
    load(mpTestDocumentPath, "textbox-wps-only.docx");
    uno::Reference<text::XTextRange> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello world!"), xFrame->getString());
    // Position wasn't horizontally centered.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xFrame, "HoriOrient"));

    // Position was the default (hori center, vert top) for the textbox.
    xFrame.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2173), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
#ifdef MACOSX
    // FIXME: The assert below fails wildly on a Retina display
    NSScreen* nsScreen = [ NSScreen mainScreen ];
    CGFloat scaleFactor = [ nsScreen backingScaleFactor ];   // for instance on the 5K Retina iMac,
                                                             // [NSScreen mainScreen].frame.size is 2560x1440,
                                                             // while real display size is 5120x2880
    if ( nsScreen.frame.size.width * scaleFactor > 4000 )
        return;
#endif
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2805), getProperty<sal_Int32>(xFrame, "VertOrientPosition"));
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshapeRelsize)
{
    load(mpTestDocumentPath, "groupshape-relsize.docx");
    // This was 43760, i.e. the height of the groupshape was larger than the page height, which is obviously incorrect.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(9142730), getShape(1)->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(Test, testOleAnchor)
{
    load(mpTestDocumentPath, "ole-anchor.docx");
    // This was AS_CHARACTER, even if the VML style explicitly contains "position:absolute".
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
    // This was DYNAMIC, even if the default is THROUGH and there is no w10:wrap element in the bugdoc.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(getShape(1), "Surround"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf48658_transparentOLEheader)
{
    load(mpTestDocumentPath, "tdf48658_transparentOLEheader.docx");
    // The problem was that the shape in the header was hidden in the background.
    // The round-tripped document was always fine (even before the fix) but the shape numbers change, so import-only test.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "Opaque"));
}

CPPUNIT_TEST_FIXTURE(Test, testDMLGroupShapeParaAdjust)
{
    load(mpTestDocumentPath, "dml-groupshape-paraadjust.docx");
    // Paragraph adjustment inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY_THROW)->getText();
    // 2nd line is adjusted to the right
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getRun(getParagraphOfText(2, xText), 1), "ParaAdjust"));
    // 3rd line has no adjustment
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT), getProperty<sal_Int16>(getRun(getParagraphOfText(3, xText), 1), "ParaAdjust"));
    // 4th line is adjusted to center
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getRun(getParagraphOfText(4, xText), 1), "ParaAdjust"));
    // 5th line has no adjustment
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT), getProperty<sal_Int16>(getRun(getParagraphOfText(5, xText), 1), "ParaAdjust"));
    // 6th line is justified
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_BLOCK), getProperty<sal_Int16>(getRun(getParagraphOfText(6, xText), 1), "ParaAdjust"));
    // 7th line has no adjustment
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT), getProperty<sal_Int16>(getRun(getParagraphOfText(7, xText), 1), "ParaAdjust"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99135)
{
    load(mpTestDocumentPath, "tdf99135.docx");
    // This was 0, crop was ignored on VML import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1825), getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop").Bottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf85523)
{
    load(mpTestDocumentPath, "tdf85523.docx");
    auto xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 7), "TextField");
    auto xText = getProperty< uno::Reference<text::XText> >(xTextField, "TextRange");
    // This was "commentX": an unexpected extra char was added at the comment end.
    getParagraphOfText(1, xText, "comment");
}

CPPUNIT_TEST_FIXTURE(Test, testStrictLockedcanvas)
{
    load(mpTestDocumentPath, "strict-lockedcanvas.docx");
    // locked canvas shape was missing.
    getShape(1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo75722vml)
{
    load(mpTestDocumentPath, "fdo75722-vml.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    awt::Point aPos = xShape->getPosition();
    awt::Size aSize = xShape->getSize();
    sal_Int64 nRot = getProperty<sal_Int64>(xShape, "RotateAngle");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3720), aPos.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-392), aPos.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5457), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3447), aSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3100), nRot);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo75722dml)
{
    load(mpTestDocumentPath, "fdo75722-dml.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    awt::Point aPos = xShape->getPosition();
    awt::Size aSize = xShape->getSize();
    sal_Int64 nRot = getProperty<sal_Int64>(xShape, "RotateAngle");

    // a slight difference regarding vml file is tolerated due to rounding errors
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3720), aPos.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-397), aPos.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5457), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3447), aSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3128), nRot);
}

CPPUNIT_TEST_FIXTURE(Test, testUnbalancedColumnsCompat)
{
    load(mpTestDocumentPath, "unbalanced-columns-compat.docx");
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, we ignored the relevant compat setting to make this non-last section unbalanced.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableSectionColumns)
{
    load(mpTestDocumentPath, "floating-table-section-columns.docx");
    OUString tableWidth = parseDump("/root/page[1]/body/section/column[2]/body/txt/anchored/fly/tab/infos/bounds", "width");
    // table width was restricted by a column
    CPPUNIT_ASSERT( tableWidth.toInt32() > 10000 );
}

static OString dateTimeToString( const util::DateTime& dt )
{
    return DateTimeToOString( DateTime( Date( dt.Day, dt.Month, dt.Year ), tools::Time( dt.Hours, dt.Minutes, dt.Seconds )));
}

CPPUNIT_TEST_FIXTURE(Test, testBnc821804)
{
    load(mpTestDocumentPath, "bnc821804.docx");
    CPPUNIT_ASSERT_EQUAL( OUString( "TITLE" ), getRun( getParagraph( 1 ), 1 )->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(1), 1), "RedlineType"));
    // Redline information (SwXRedlinePortion) are separate "runs" apparently.
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(1), 2), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown1"),getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:46:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(1), 2), "RedlineDateTime")));
    // So only the 3rd run is actual text (and the two runs have been merged into one, not sure why, but that shouldn't be a problem).
    CPPUNIT_ASSERT_EQUAL(OUString(" (1st run of an insert) (2nd run of an insert)"), getRun(getParagraph(1),3)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(1), 3), "RedlineType"));
    // And the end SwXRedlinePortion of the redline.
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(1), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown1"),getProperty<OUString>(getRun(getParagraph(1), 4), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:46:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(1), 4), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(1), 4), "IsStart"));

    CPPUNIT_ASSERT_EQUAL(OUString("Normal text"), getRun(getParagraph(2),1)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(2), 1), "RedlineType"));

    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(3), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown2"),getProperty<OUString>(getRun(getParagraph(3), 1), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:47:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(3), 1), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(OUString("Deleted"), getRun(getParagraph(3),2)->getString());

    // This is both inserted and formatted, so there are two SwXRedlinePortion "runs". Given that the redlines overlap and Writer core
    // doesn't officially expect that (even though it copes, the redline info will be split depending on how Writer deal with it).
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(4), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(4), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(4), 1), "IsStart"));
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(4), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(4), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(4), 2), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("Inserted and formatted"), getRun(getParagraph(4),3)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(4), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(4), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(4), 4), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and this is only formatted"), getRun(getParagraph(4),5)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(4), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(4), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(4), 6), "IsStart"));

    CPPUNIT_ASSERT_EQUAL(OUString("Normal text"), getRun(getParagraph(5),1)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(5), 1), "RedlineType"));

    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(6), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"),getProperty<OUString>(getRun(getParagraph(6), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(6), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown5"),getProperty<OUString>(getRun(getParagraph(6), 1), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T10:02:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(6), 1), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(OUString("Formatted run"), getRun(getParagraph(6),2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(6), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"),getProperty<OUString>(getRun(getParagraph(6), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(6), 3), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and normal text here "), getRun(getParagraph(6),4)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(6), 4), "RedlineType"));
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(6), 5), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(6), 5), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(6), 5), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown6"),getProperty<OUString>(getRun(getParagraph(6), 5), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:48:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(6), 5), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(OUString("and inserted again"), getRun(getParagraph(6),6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(6), 7), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(6), 7), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(6), 7), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and normal text again "), getRun(getParagraph(6),8)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(6), 8), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"),getProperty<OUString>(getRun(getParagraph(6), 9), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(6), 9), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown7"),getProperty<OUString>(getRun(getParagraph(6), 9), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("and formatted"), getRun(getParagraph(6),10)->getString());
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(6), 11), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and normal."), getRun(getParagraph(6),12)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(6), 12), "RedlineType"));

    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(7), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(7), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown8"),getProperty<OUString>(getRun(getParagraph(7), 1), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("One insert."), getRun(getParagraph(7),2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(7), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(7), 3), "IsStart"));
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(7), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(7), 4), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown9"),getProperty<OUString>(getRun(getParagraph(7), 4), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("Second insert."), getRun(getParagraph(7),5)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(7), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(7), 6), "IsStart"));

    // Overlapping again.
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(8), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(8), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(8), 1), "IsStart"));
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(8), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(8), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(8), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("Deleted and formatted"), getRun(getParagraph(8),3)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(8), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(8), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(8), 4), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and this is only formatted"), getRun(getParagraph(8),5)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(8), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(8), 6), "IsStart"));

    CPPUNIT_ASSERT_EQUAL(OUString("Normal text"), getRun(getParagraph(9),1)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(9), 1), "RedlineType"));

    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(10), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(10), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(10), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is only formatted."), getRun(getParagraph(10),2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(10), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(10), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(10), 3), "IsStart"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo87488)
{
    // The shape on the right (index 0, CustomShape within a
    // GroupShape) is rotated 90 degrees clockwise and contains text
    // rotated 90 degrees anticlockwise.  Must be read with SmartArt
    // enabled in preTest above, otherwise it gets converted to a
    // StarView MetaFile.
    SvtFilterOptions::Get().SetSmartArt2Shape(true);
    comphelper::ScopeGuard g([] { SvtFilterOptions::Get().SetSmartArt2Shape(false); });
    load(mpTestDocumentPath, "fdo87488.docx");
    uno::Reference<container::XIndexAccess> group(getShape(1), uno::UNO_QUERY);
    {
        uno::Reference<text::XTextRange> text(group->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("text2"), text->getString());
    }
    {
        uno::Reference<beans::XPropertySet> props(group->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(props->getPropertyValue("RotateAngle"),
                             uno::makeAny<sal_Int32>(270 * 100));
        comphelper::SequenceAsHashMap geom(props->getPropertyValue("CustomShapeGeometry"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(90), geom["TextPreRotateAngle"].get<sal_Int32>());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf85232)
{
    load(mpTestDocumentPath, "tdf85232.docx");
    uno::Reference<drawing::XShapes> xShapes(getShapeByName(u"Group 219"), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xShapes->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor = xShape;
    // Make sure we're not testing the ellipse child.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.LineShape"), xShapeDescriptor->getShapeType());

    // This was 2900: horizontal position of the line was incorrect, the 3 children were not connected visually.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2265), xShape->getPosition().X);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95755)
{
    load(mpTestDocumentPath, "tdf95755.docx");
    /*
    * The problem was that the width of a second table with single cell was discarded
    * and resulted in too wide table
    */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Any aValue = xTableProperties->getPropertyValue("Width");
    sal_Int32 nWidth;
    aValue >>= nWidth;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10659), nWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf60351)
{
    load(mpTestDocumentPath, "tdf60351.docx");
    // Get the first image in the document and check its contour polygon.
    // It should contain 6 points. Check their coordinates.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    // test for TODO: if paragraph's background becomes bottommost [better yet: wraps around shape], then remove this hack
    CPPUNIT_ASSERT_EQUAL_MESSAGE("HACK ALERT: Shape is in foreground", true, getProperty<bool>(xPropertySet, "Opaque"));

    css::drawing::PointSequenceSequence aPolyPolygon;
    xPropertySet->getPropertyValue("ContourPolyPolygon") >>= aPolyPolygon;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPolyPolygon.getLength());
    const css::drawing::PointSequence& aPolygon = aPolyPolygon[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6),   aPolygon.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[0].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[0].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[1].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[1].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[2].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[2].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(158), aPolygon[3].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(298), aPolygon[3].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[4].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[4].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[5].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[5].Y);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95970)
{
    load(mpTestDocumentPath, "tdf95970.docx");
    // First shape: the rotation should be -12.94 deg, it should be mirrored.
    // Proper color order of image on test doc (left->right):
    // top row: green->red
    // bottom row: yellow->blue
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY_THROW);
    sal_Int32 aRotate = 0;
    xPropertySet->getPropertyValue("RotateAngle") >>= aRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(34706), aRotate);
    bool bIsMirrored = false;
    xPropertySet->getPropertyValue("IsMirrored") >>= bIsMirrored;
    CPPUNIT_ASSERT(bIsMirrored);
    drawing::HomogenMatrix3 aTransform;
    xPropertySet->getPropertyValue("Transformation") >>= aTransform;
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line1.Column1,  4767.0507250872988));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line1.Column2, -1269.0985325236848));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line1.Column3,  696.73611111111109));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line2.Column1,  1095.3035265135941));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line2.Column2,  5523.4525711162969));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line2.Column3,  672.04166666666663));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line3.Column1,  0.0));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line3.Column2,  0.0));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line3.Column3,  1.0));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96674)
{
    load(mpTestDocumentPath, "tdf96674.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());
    awt::Size aActualSize(xShape->getSize());
    // Width was 3493: the vertical line was horizontal.
    CPPUNIT_ASSERT(aActualSize.Width < aActualSize.Height);
    CPPUNIT_ASSERT(aActualSize.Height > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122717)
{
    load(mpTestDocumentPath, "tdf122717.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());
    awt::Size aActualSize(xShape->getSize());
    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 8160
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aActualSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8160), aActualSize.Height);

}

CPPUNIT_TEST_FIXTURE(Test, testTdf98882)
{
    load(mpTestDocumentPath, "tdf98882.docx");
    sal_Int32 nFlyHeight = parseDump("//anchored/fly/infos/bounds", "height").toInt32();
    sal_Int32 nContentHeight = parseDump("//notxt/infos/bounds", "height").toInt32();
    // The content height was 600, not 360, so the frame and the content height did not match.
    CPPUNIT_ASSERT_EQUAL(nFlyHeight, nContentHeight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100830)
{
    load(mpTestDocumentPath, "tdf100830.docx");
    // FillTransparence wasn't imported, this was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(30), getProperty<sal_Int16>(getShape(1), "FillTransparence"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103664)
{
    load(mpTestDocumentPath, "tdf103664.docx");
    // Wingdings symbols were displayed as rectangles
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(u'\xf020', xPara->getString()[0] );
    CPPUNIT_ASSERT_EQUAL(u'\xf0fc', xPara->getString()[1] );
    CPPUNIT_ASSERT_EQUAL(u'\xf0dc', xPara->getString()[2] );
    CPPUNIT_ASSERT_EQUAL(u'\xf081', xPara->getString()[3] );

    uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameAsian"));
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameComplex"));

    // Make sure these special characters are imported as symbols
    CPPUNIT_ASSERT_EQUAL(awt::CharSet::SYMBOL, getProperty<sal_Int16>(xRun, "CharFontCharSet"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf82824)
{
    load(mpTestDocumentPath, "tdf82824.docx");
    // This was text::TextContentAnchorType_AS_CHARACTER, <wp:anchor> wasn't handled on import for the chart.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96218)
{
    load(mpTestDocumentPath, "tdf96218.docx");
    // Image had a bad position because layoutInCell attribute was not ignored
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "IsFollowingTextFlow"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101626)
{
    load(mpTestDocumentPath, "tdf101626.docx");
    // Transform soft-hyphen to hard-hyphen as list bulletChar to avoid missing symbols in export
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "BulletChar")
        {
            // the bulletChar has to be 0x2d!
            CPPUNIT_ASSERT_EQUAL(OUString("\x2d"), rProp.Value.get<OUString>());
            return;
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test,  testTdf106606)
{
    load(mpTestDocumentPath, "tdf106606.docx" );
    auto FindGraphicBitmapPropertyInNumStyle = [&]( OUString rStyleName )
    {
        uno::Reference<beans::XPropertySet>     xPropertySet( getStyles( "NumberingStyles" )->getByName( rStyleName ), uno::UNO_QUERY );
        uno::Reference<container::XIndexAccess> xLevels( xPropertySet->getPropertyValue( "NumberingRules" ), uno::UNO_QUERY );
        uno::Sequence<beans::PropertyValue>     aProps;
        xLevels->getByIndex( 0 ) >>= aProps; // 1st level

        for ( beans::PropertyValue const & rProp : std::as_const(aProps))
        {
            // If the image was prematurely removed from cache when processed for previous numbering list, then the sequence hasn't the property.
            if ( rProp.Name == "GraphicBitmap" )
                return true;
        }
        return false;
    };

    // The document has two numbering lists with a picture
    CPPUNIT_ASSERT( FindGraphicBitmapPropertyInNumStyle("WWNum1") );
    CPPUNIT_ASSERT( FindGraphicBitmapPropertyInNumStyle("WWNum2") );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101627)
{
    load(mpTestDocumentPath, "tdf101627.docx");
    // Do not shrink the textbox in the footer
    uno::Reference<text::XTextRange> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame->getString().startsWith( "1" ) );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(466), getProperty<sal_Int32>(xFrame, "Height"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133448)
{
    load(mpTestDocumentPath, "tdf133448.docx");
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    Graphic aGraphic(xGraphic);
    uno::Reference<beans::XPropertySet> xGraphicDescriptor(xGraphic, uno::UNO_QUERY_THROW);
    awt::Size aSizePixel;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("SizePixel") >>= aSizePixel);

    //Without the fix in place, the graphic's size is 0x0
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), aSizePixel.Width);
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), aSizePixel.Height);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100072)
{
    load(mpTestDocumentPath, "tdf100072.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);

    // Ensure that shape has non-zero height
    CPPUNIT_ASSERT(xShape->getSize().Height > 0);

    // Ensure that shape left corner is within page (positive)
    CPPUNIT_ASSERT(xShape->getPosition().X > 0);

    // Save the first shape to a metafile.
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter = drawing::GraphicExportFilter::create(comphelper::getProcessComponentContext());
    uno::Reference<lang::XComponent> xSourceDoc(xShape, uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
            { "OutputStream", uno::Any(xOutputStream) },
            { "FilterName", uno::Any(OUString("SVM")) }
        }));
    xGraphicExporter->filter(aDescriptor);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Read it back and dump it as an XML file.
    Graphic aGraphic;
    TypeSerializer aSerializer(aStream);
    aSerializer.readGraphic(aGraphic);
    const GDIMetaFile& rMetaFile = aGraphic.GetGDIMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, rMetaFile);

    // Get first polyline rightside x coordinate
    sal_Int32 nFirstEnd = getXPath(pXmlDoc, "(//polyline)[1]/point[2]", "x").toInt32();

    // Get last stroke x coordinate
    sal_Int32 nSecondEnd = getXPath(pXmlDoc, "(//polyline)[last()]/point[2]", "x").toInt32();

    // Assert that the difference is less than half point.
    CPPUNIT_ASSERT_MESSAGE("Shape line width does not match", abs(nFirstEnd - nSecondEnd) < 10);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf76446)
{
    load(mpTestDocumentPath, "tdf76446.docx");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    sal_Int64 nRot = getProperty<sal_Int64>(xShape, "RotateAngle");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3128), nRot);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108350)
{
    load(mpTestDocumentPath, "tdf108350.docx");
    // For OOXML without explicit font information, font needs to be Calibri 11 pt
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    uno::Reference<beans::XPropertySet> xRun(getRun(xPara, 1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Calibri"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(double(11), getProperty<double>(xRun, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108408)
{
    load(mpTestDocumentPath, "tdf108408.docx");
    // Font size must consider units specifications; previously ignored and only used
    // integer part as half-pt size, i.e. 10 pt (20 half-pt) instead of 20 pt
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    uno::Reference<beans::XPropertySet> xRun(getRun(xPara, 1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(double(20), getProperty<double>(xRun, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108806)
{
    load(mpTestDocumentPath, "tdf108806.docx");
    // tdf#108806:The CRLF in the text contents of XML must be converted to single spaces.
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    uno::Reference< text::XTextRange > paragraph = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(
        OUString("First part of a line (before CRLF). Second part of the same line (after CRLF)."),
        paragraph->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf87533_bidi)
{
    load(mpTestDocumentPath, "tdf87533_bidi.docx");
    // "w:bidi" (specified inside Default paragraph properties) should not be ignored
    static const OUStringLiteral writingMode = u"WritingMode"; //getPropertyName(PROP_WRITING_MODE);

    // check: "Default Style" master-style has RTL
    {
        const uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default Page Style"), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(text::WritingMode2::RL_TB), getProperty<sal_Int32>(xPropertySet, writingMode));
    }

    // check: "Standard" master-style has RTL
    {
        const uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(text::WritingMode2::RL_TB), getProperty<sal_Int32>(xPropertySet, writingMode));
    }

    // check: style of the first paragraph has RTL
    // it has missing usage of the <w:bidi> => this property should be taken from style
    {
        const uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(text::WritingMode2::RL_TB), getProperty<sal_Int32>(xPara, writingMode));
    }

    // check: style of the first paragraph has LTR
    // it has <w:bidi w:val="false"/>
    {
        const uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(text::WritingMode2::LR_TB), getProperty<sal_Int32>(xPara, writingMode));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testVmlAdjustments)
{
    load(mpTestDocumentPath, "vml-adjustments.docx");
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues =
        aGeometry["AdjustmentValues"].get<uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue>>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aAdjustmentValues.getLength());
    drawing::EnhancedCustomShapeAdjustmentValue aAdjustmentValue = *std::cbegin(aAdjustmentValues);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17639), aAdjustmentValue.Value.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108714)
{
    load(mpTestDocumentPath, "tdf108714.docx");
    CPPUNIT_ASSERT_EQUAL(6, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page break is absent - we lost bug-to-bug compatibility with Word", 4, getPages());

    // The second (empty) paragraph must be at first page, despite the <w:br> element was before it.
    // That's because Word treats such break as first element in first run of following paragraph:
    //
    //    <w:br w:type="page"/>
    //    <w:p>
    //        <w:r>
    //            <w:t/>
    //        </w:r>
    //    </w:p>
    //
    // is equal to
    //
    //    <w:p>
    //        <w:r>
    //            <w:br w:type="page"/>
    //        </w:r>
    //    </w:p>
    //
    // which emits page break after that empty paragraph.

    uno::Reference< text::XTextRange > paragraph = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Paragraph 1"), paragraph->getString());
    style::BreakType breakType = getProperty<style::BreakType>(paragraph, "BreakType");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, breakType);

    paragraph = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString(), paragraph->getString());
    breakType = getProperty<style::BreakType>(paragraph, "BreakType");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, breakType);

    paragraph = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString("Paragraph 3"), paragraph->getString());
    breakType = getProperty<style::BreakType>(paragraph, "BreakType");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, breakType);

    paragraph = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("Paragraph 4"), paragraph->getString());
    breakType = getProperty<style::BreakType>(paragraph, "BreakType");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, breakType);

    // A table with immediately following break
    // Line breaks in block and paragraph levels must be taken into account
    // Several successive out-of-place w:br's must produce required amount of breaks
    uno::Reference<text::XTextContent> table = getParagraphOrTable(5);
    getCell(table, "A1", "\n\n\n\nParagraph 5 in table");
    breakType = getProperty<style::BreakType>(table, "BreakType");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, breakType);

    paragraph = getParagraph(6);
    CPPUNIT_ASSERT_EQUAL(OUString("Paragraph 6"), paragraph->getString());
    breakType = getProperty<style::BreakType>(paragraph, "BreakType");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, breakType);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136952_pgBreak3)
{
    load(mpTestDocumentPath, "tdf136952_pgBreak3.docx");
    // The original 6 page ODT was designed to visually exaggerate the problems
    // of emulating LO's followed-by-page-style into MSWord's sections.
    // While much has been improved, there are extra pages present, which still need fixing.
    xmlDocUniquePtr pDump = parseLayoutDump();

    //Do not lose the page::breakAfter. This SHOULD be on page 4, but sadly it is not.
    //The key part of this test is that the page starts with "Lorem ipsum"
    //Prior to this, there was no page break, and so it was in the middle of a page.
    CPPUNIT_ASSERT(getXPath(pDump, "//page[6]/body/txt[1]/Text[1]", "Portion").startsWith("Lorem ipsum"));
}


CPPUNIT_TEST_FIXTURE(Test, testImageLazyRead)
{
    load(mpTestDocumentPath, "image-lazy-read.docx");
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    Graphic aGraphic(xGraphic);
    // This failed, import loaded the graphic, it wasn't lazy-read.
    CPPUNIT_ASSERT(!aGraphic.isAvailable());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108995)
{
    load(mpTestDocumentPath, "xml_space.docx");
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    // We need to take xml:space attribute into account
    uno::Reference< text::XTextRange > paragraph = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("\tA\t\tline  with\txml:space=\"preserve\" \n"
                                  "A  line  without xml:space"),
                         paragraph->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testGroupShapeTextHighlight)
{
    load(mpTestDocumentPath, "tdf131841_HighlightColorGroupedShape.docx");
    // tdf#131841 Highlight color of text in grouped shapes was not imported.

    // These are the possible highlight colors in MSO Word. Check that we import them properly.
    const std::vector<sal_uInt32> xColors {
        0xFFFF00UL, // yellow
        0x00FF00UL, // green
        0x00FFFFUL, // cyan
        0xFF00FFUL, // magenta
        0x0000FFUL, // blue
        0xFF0000UL, // red
        0x000080UL, // dark blue
        0x008080UL, // dark cyan
        0x008000UL, // dark green
        0x800080UL, // dark magenta
        0x800000UL, // dark red
        0x808000UL, // dark yellow
        0x808080UL, // dark grey
        0xC0C0C0UL, // light grey
        0x000000UL  // black
    };

    // The grouped shape, consists of 15 rectangles.
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);

    // Iterate through all of the rectangles and check the colors of the texts.
    // They should correspond to the list above.
    for (size_t idx = 0; idx < xColors.size(); ++idx)
    {
        uno::Reference<text::XTextRange> xTextRange(xGroupShape->getByIndex(idx), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> firstParagraph = getParagraphOfText(1, xTextRange->getText());
        uno::Reference<text::XTextRange> firstRun = getRun(firstParagraph, 1);
        uno::Reference<beans::XPropertySet> props(firstRun, uno::UNO_QUERY_THROW);

        CPPUNIT_ASSERT_EQUAL(xColors[idx], props->getPropertyValue("CharHighlight").get<sal_uInt32>());
    }
}

// tests should only be added to ooxmlIMPORT *if* they fail round-tripping in ooxmlEXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
