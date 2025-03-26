/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/color.hxx>
#include <algorithm>
#include <memory>
#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>

#include <comphelper/storagehelper.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/documentconstants.hxx>
#include <svl/PasswordHelper.hxx>
#include <comphelper/scopeguard.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

#include <docufld.hxx> // for SwHiddenTextField::ParseIfFieldDefinition() method call
#include <ftnidx.hxx>
#include <txtftn.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/odfexport/data/"_ustr, u"writer8"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testMathObjectFlatExport)
{
    comphelper::ScopeGuard g([this]() {
        mpFilter = "writer8";
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Cache::Writer::OLE_Objects::set(20, pBatch);
        return pBatch->commit();
    });
    mpFilter = "OpenDocument Text Flat XML"; // doesn't happen with ODF package
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::Writer::OLE_Objects::set(1, pBatch);
    pBatch->commit();
    loadAndReload("2_MathType3.docx");

    uno::Reference<util::XModifiable> xModifiable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xModifiable->isModified());
    // see above, set the OLE cache to 1 for this test
    // and the problem was that the formulas that were in the cache
    // (the second one) were lost
    OUString formula1(getFormula(getRun(getParagraph(1), 1)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{1+1=2} {}"_ustr, formula1);
    OUString formula2(getFormula(getRun(getParagraph(2), 1)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{2+2=4} {}"_ustr, formula2);
}

DECLARE_ODFEXPORT_TEST(testTdf144319, "tdf144319.odt")
{
    CPPUNIT_ASSERT_EQUAL(7, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    OUString formula1(getFormula(getRun(getParagraph(3), 1)));
    CPPUNIT_ASSERT_EQUAL(u"{ x = frac { { - b +- sqrt { b ^ 2 - 4 a c } } } { { 2 a } } }"_ustr, formula1);
    OUString formula2(getFormula(getRun(getParagraph(4), 1)));
    CPPUNIT_ASSERT_EQUAL(u"{ sum csup n csub { i = 1 } i ^ 3 = left ( frac { { n left ( { n + 1 } right ) } } { 2 } right ) ^ 2 }"_ustr, formula2);
    OUString formula3(getFormula(getRun(getParagraph(5), 1)));
    CPPUNIT_ASSERT_EQUAL(u"{ sum ^ n _ { i = 1 } i ^ 3 = left ( frac { { n left ( { n + 1 } right ) } } { 2 } right ) ^ 2 }"_ustr, formula3);
    OUString formula4(getFormula(getRun(getParagraph(6), 1)));
    CPPUNIT_ASSERT_EQUAL(u"{ sum ^ n _ { i = 1 } i ^ 3 = left ( frac { { n left ( { n + 1 } right ) } } { 2 } right ) ^ 2 }"_ustr, formula4);

    // Without the fix in place, this test would have failed with
    // - the property is of unexpected type or void: Model
    OUString formula5(getFormula(getRun(getParagraph(7), 1)));
    CPPUNIT_ASSERT_EQUAL(u"{ y ^ 2 { nitalic m p } = left ( { x ^ 3 + 7 } right ) { nitalic m p } }"_ustr, formula5);
}

void testTdf43569_CheckIfFieldParse()
{
    {
        OUString paramCondition;
        OUString paramTrue;
        OUString paramFalse;

        SwHiddenTextField::ParseIfFieldDefinition(u"IF A B C", paramCondition, paramTrue, paramFalse);

        CPPUNIT_ASSERT_EQUAL(u"A"_ustr, paramCondition);
        CPPUNIT_ASSERT_EQUAL(u"B"_ustr, paramTrue);
        CPPUNIT_ASSERT_EQUAL(u"C"_ustr, paramFalse);
    }

    {
        OUString paramCondition;
        OUString paramTrue;
        OUString paramFalse;

        SwHiddenTextField::ParseIfFieldDefinition(u"  IF AAA BBB CCC  ", paramCondition, paramTrue, paramFalse);

        CPPUNIT_ASSERT_EQUAL(u"AAA"_ustr, paramCondition);
        CPPUNIT_ASSERT_EQUAL(u"BBB"_ustr, paramTrue);
        CPPUNIT_ASSERT_EQUAL(u"CCC"_ustr, paramFalse);
    }

    {
        OUString paramCondition;
        OUString paramTrue;
        OUString paramFalse;

        SwHiddenTextField::ParseIfFieldDefinition(u"  IF AAA \"BBB\" \"CCC\"  ", paramCondition, paramTrue, paramFalse);

        CPPUNIT_ASSERT_EQUAL(u"AAA"_ustr, paramCondition);
        CPPUNIT_ASSERT_EQUAL(u"BBB"_ustr, paramTrue);
        CPPUNIT_ASSERT_EQUAL(u"CCC"_ustr, paramFalse);
    }

    // true-case and false-case have spaces inside
    {
        OUString paramCondition;
        OUString paramTrue;
        OUString paramFalse;

        SwHiddenTextField::ParseIfFieldDefinition(u"  IF A A A \"B B B\" \"C C C\"  ", paramCondition, paramTrue, paramFalse);

        CPPUNIT_ASSERT_EQUAL(u"A A A"_ustr, paramCondition);
        CPPUNIT_ASSERT_EQUAL(u"B B B"_ustr, paramTrue);
        CPPUNIT_ASSERT_EQUAL(u"C C C"_ustr, paramFalse);
    }

    // true-case and false-case have leading/trailing space
    {
        OUString paramCondition;
        OUString paramTrue;
        OUString paramFalse;

        SwHiddenTextField::ParseIfFieldDefinition(u"IF A1 A2 A3 \"B1 B2 \" \" C1 C2\"  ", paramCondition, paramTrue, paramFalse);

        CPPUNIT_ASSERT_EQUAL(u"A1 A2 A3"_ustr, paramCondition);
        CPPUNIT_ASSERT_EQUAL(u"B1 B2 "_ustr, paramTrue);
        CPPUNIT_ASSERT_EQUAL(u" C1 C2"_ustr, paramFalse);
    }

    // true-case and false-case are empty
    {
        OUString paramCondition;
        OUString paramTrue;
        OUString paramFalse;

        SwHiddenTextField::ParseIfFieldDefinition(u"IF condition \"\" \"\"  ", paramCondition, paramTrue, paramFalse);

        CPPUNIT_ASSERT_EQUAL(u"condition"_ustr, paramCondition);
        CPPUNIT_ASSERT_EQUAL(u""_ustr, paramTrue);
        CPPUNIT_ASSERT_EQUAL(u""_ustr, paramFalse);
    }
}

// Input document contains only one IF-field,
// and it should be imported as com.sun.star.text.TextField.ConditionalText in any case,
// instead of insertion of the pair of two field-marks: <field:fieldmark-start> + <field:fieldmark-end>.
CPPUNIT_TEST_FIXTURE(Test, testTdf43569)
{
    loadAndReload("tdf43569_conditionalfield.doc");
    // check if our parser is valid
    testTdf43569_CheckIfFieldParse();

    // now check field creation during import
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // at least one field should be detected
    CPPUNIT_ASSERT(xFields->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130314)
{
    loadAndReload("tdf130314.docx");
    // Without the fix in place, this test would have hung
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133487)
{
    loadAndReload("MadeByLO7.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // shape in background has lowest index
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p[2]/draw:custom-shape", "z-index", u"0");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name = /office:document-content/office:body/office:text/text:p[2]/draw:custom-shape[@draw:z-index = '0']/attribute::draw:style-name]/style:graphic-properties", "run-through", u"background");
    // shape in foreground, previously index 1
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p[1]/draw:custom-shape", "z-index", u"2");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name = /office:document-content/office:body/office:text/text:p[1]/draw:custom-shape[@draw:z-index = '2']/attribute::draw:style-name]/style:graphic-properties", "run-through", u"foreground");
    // shape in foreground, previously index 0
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p[3]/draw:custom-shape", "z-index", u"1");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name = /office:document-content/office:body/office:text/text:p[3]/draw:custom-shape[@draw:z-index = '1']/attribute::draw:style-name]/style:graphic-properties", "run-through", u"foreground");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141467)
{
    loadAndReload("Formcontrol needs high z-index.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // shape in foreground has lowest index
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p[2]/draw:custom-shape", "z-index", u"0");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name = /office:document-content/office:body/office:text/text:p[2]/draw:custom-shape[@draw:z-index = '0']/attribute::draw:style-name]/style:graphic-properties", "run-through", u"foreground");
    // form control, previously index 0
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p[2]/draw:control", "z-index", u"1");
    // no run-through on form's style
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name = /office:document-content/office:body/office:text/text:p[2]/draw:control[@draw:z-index = '1']/attribute::draw:style-name]/style:graphic-properties/attribute::run-through", 0);
}

DECLARE_ODFEXPORT_TEST(testTdf139126, "tdf139126.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName(u"Table1"_ustr), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xD2(xTable->getCellByName(u"D2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"4.0"_ustr, xD2->getString());

    // Without the fix in place, this test would have failed with
    // - Expected: ** Expression is faulty **
    // - Actual  : 17976931348623200...
    uno::Reference<text::XTextRange> xE2(xTable->getCellByName(u"E2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"** Expression is faulty **"_ustr, xE2->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125877)
{
    loadAndReload("tdf95806.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xSupplier->getTextTables(), uno::UNO_QUERY);

    // This was 0 (lost table during ODT export in footnotes)
    // Note: fix also tdf#95806: painting table layout is correct
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // floating table: there is a frame now
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150149)
{
    loadAndReload("tdf150149.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // This was 0 (lost table header in multi-column section)
    assertXPath(pXmlDoc, "//table:table-header-rows", 1);
    assertXPath(pXmlDoc, "//table:table-header-rows/table:table-row/table:table-cell", 3);
}

DECLARE_ODFEXPORT_TEST(testTdf103567, "tdf103567.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> const xShape(getShape(1));

    // contour wrap polygon
    css::drawing::PointSequenceSequence const pointss(
        getProperty<css::drawing::PointSequenceSequence>(xShape, u"ContourPolyPolygon"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pointss.getLength());
    // for some reason this property exists with 199 points if it wasn't
    // imported, that would be a fail
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pointss[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(   0), pointss[0][0].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2672), pointss[0][0].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(   0), pointss[0][1].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1111), pointss[0][1].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2672), pointss[0][2].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1111), pointss[0][2].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2672), pointss[0][3].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2672), pointss[0][3].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(   0), pointss[0][4].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2672), pointss[0][4].Y);

    // image map, one rectangle
    uno::Reference<container::XIndexContainer> const xImageMap(
        getProperty<uno::Reference<container::XIndexContainer>>(xShape, u"ImageMap"_ustr));

    uno::Reference<beans::XPropertySet> const xEntry(xImageMap->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/"_ustr, getProperty<OUString>(xEntry, u"URL"_ustr));
    awt::Rectangle const rect(getProperty<awt::Rectangle>(xEntry, u"Boundary"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32( 726), rect.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1718), rect.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1347), rect.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32( 408), rect.Height);
}

CPPUNIT_TEST_FIXTURE(Test, testUserFieldDecl)
{
    loadAndReload("user-field-decl.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 2;
    // Actual: 1', i.e. the in-table field had no declaration (in the header), while the
    // outside-table one had the declaration.
    assertXPath(pXmlDoc, "//style:header/text:user-field-decls/text:user-field-decl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testUserFieldDeclFly)
{
    loadAndReload("user-field-decl-fly.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 2;
    // Actual: 1', i.e. the in-textframe field had no declaration (in the header), while the
    // outside-textframe one had the declaration.
    assertXPath(pXmlDoc, "//style:header/text:user-field-decls/text:user-field-decl", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testFramebackgrounds)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(16, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
       //Counting the Number of Frames and checking with the expected count
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), xIndexAccess->getCount());
        uno::Reference<drawing::XShape> xTextFrame;
        awt::Gradient aGradientxTextFrame;
        //Frame 1
        xTextFrame = getShape(1);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 2
        xTextFrame = getShape(2);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x006600), getProperty<Color>(xTextFrame, u"FillColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 3
        xTextFrame = getShape(3);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x006600), getProperty<Color>(xTextFrame, u"FillColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(45), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 4
        xTextFrame = getShape(4);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x579D1C), getProperty<Color>(xTextFrame, u"FillColor"_ustr));
        aGradientxTextFrame = getProperty<awt::Gradient>(xTextFrame, u"FillTransparenceGradient"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::awt::GradientStyle_LINEAR, aGradientxTextFrame.Style);
        //Frame 5
        xTextFrame = getShape(5);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Subtle Tango Green"_ustr, getProperty<OUString>(xTextFrame, u"FillGradientName"_ustr));
        //Frame 6
        xTextFrame = getShape(6);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Subtle Tango Green"_ustr, getProperty<OUString>(xTextFrame, u"FillGradientName"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(45), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 7
        xTextFrame = getShape(7);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Subtle Tango Green"_ustr, getProperty<OUString>(xTextFrame, u"FillGradientName"_ustr));
        aGradientxTextFrame = getProperty<awt::Gradient>(xTextFrame, u"FillTransparenceGradient"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::awt::GradientStyle_LINEAR, aGradientxTextFrame.Style);
        //Frame 8
        xTextFrame = getShape(8);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Black 0 Degrees"_ustr, getProperty<OUString>(xTextFrame, u"FillHatchName"_ustr));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextFrame, u"FillBackground"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 9
        xTextFrame = getShape(9);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Black 0 Degrees"_ustr, getProperty<OUString>(xTextFrame, u"FillHatchName"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextFrame, u"FillBackground"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 10
        xTextFrame = getShape(10);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Black 0 Degrees"_ustr, getProperty<OUString>(xTextFrame, u"FillHatchName"_ustr));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextFrame, u"FillBackground"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(45), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 11
        xTextFrame = getShape(11);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Black 0 Degrees"_ustr, getProperty<OUString>(xTextFrame, u"FillHatchName"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextFrame, u"FillBackground"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(45), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        //Frame 12
        xTextFrame = getShape(12);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Black 0 Degrees"_ustr, getProperty<OUString>(xTextFrame, u"FillHatchName"_ustr));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextFrame, u"FillBackground"_ustr));
        aGradientxTextFrame = getProperty<awt::Gradient>(xTextFrame, u"FillTransparenceGradient"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::awt::GradientStyle_LINEAR, aGradientxTextFrame.Style);
        //Frame 13
        xTextFrame = getShape(13);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Black 0 Degrees"_ustr, getProperty<OUString>(xTextFrame, u"FillHatchName"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextFrame, u"FillBackground"_ustr));
        aGradientxTextFrame = getProperty<awt::Gradient>(xTextFrame, u"FillTransparenceGradient"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::awt::GradientStyle_LINEAR, aGradientxTextFrame.Style);
        //Frame 14
        xTextFrame = getShape(14);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Sky"_ustr, getProperty<OUString>(xTextFrame, u"FillBitmapName"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapPositionOffsetX"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapPositionOffsetY"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapOffsetX"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapOffsetY"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextFrame, u"FillBitmapTile"_ustr));
        //Frame 15
        xTextFrame = getShape(15);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Sky"_ustr, getProperty<OUString>(xTextFrame, u"FillBitmapName"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(45), getProperty<sal_Int32>(xTextFrame, u"FillTransparence"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapPositionOffsetX"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapPositionOffsetY"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapOffsetX"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapOffsetY"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextFrame, u"FillBitmapTile"_ustr));
        //Frame 16
        xTextFrame = getShape(16);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xTextFrame, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Sky"_ustr, getProperty<OUString>(xTextFrame, u"FillBitmapName"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapPositionOffsetX"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapPositionOffsetY"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapOffsetX"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTextFrame, u"FillBitmapOffsetY"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextFrame, u"FillBitmapTile"_ustr));
        aGradientxTextFrame = getProperty<awt::Gradient>(xTextFrame, u"FillTransparenceGradient"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::awt::GradientStyle_LINEAR, aGradientxTextFrame.Style);
    };

    createSwDoc("framebackgrounds.odt");
    verify();
    saveAndReload(mpFilter);
    verify();

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // check that there are 3 background-image elements
    assertXPath(pXmlDoc, "//style:style[@style:parent-style-name='Frame' and @style:family='graphic']/style:graphic-properties[@draw:fill='bitmap']/style:background-image[@style:repeat='stretch']", 3);
    // tdf#90640: check that one of them is 55% opaque
    assertXPath(pXmlDoc, "//style:style[@style:parent-style-name='Frame' and @style:family='graphic']/style:graphic-properties[@draw:fill='bitmap' and @fo:background-color='transparent' and @draw:opacity='55%']/style:background-image[@style:repeat='stretch' and @draw:opacity='55%']", 1);
    // tdf#90640: check that one of them is 43% opaque
    // (emulated - hopefully not with rounding errors)
    assertXPath(pXmlDoc, "//style:style[@style:parent-style-name='Frame' and @style:family='graphic']/style:graphic-properties[@draw:fill='bitmap' and @fo:background-color='transparent' and @draw:opacity-name='Transparency_20_1']/style:background-image[@style:repeat='stretch' and @draw:opacity='43%']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSHA1Correct)
{   // tdf#114939 this has both an affected password as well as content.xml
    const char* const sPass = "1012345678901234567890123456789012345678901234567890";
    createSwDoc("sha1_correct.odt", sPass);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    getParagraph(1, u"012"_ustr);

    saveAndReload(mpFilter, sPass);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    getParagraph(1, u"012"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testSHA1Wrong)
{   // tdf#114939 this has both an affected password as well as content.xml
    const char* const sPass = "1012345678901234567890123456789012345678901234567890";
    createSwDoc("sha1_wrong.odt", sPass);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    getParagraph(1, u"012"_ustr);

    saveAndReload(mpFilter, sPass);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    getParagraph(1, u"012"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testOOoxmlEmbedded)
{
    loadAndReload("oooxml_embedded.sxw");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xAccess(xTEOSupplier->getEmbeddedObjects());
    uno::Sequence<OUString> aSeq(xAccess->getElementNames());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aSeq.getLength());
    uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier1(xAccess->getByName(u"Object1"_ustr), uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xObj1(xEOSupplier1->getEmbeddedObject());
    uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier2(xAccess->getByName(u"Object2"_ustr), uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xObj2(xEOSupplier2->getEmbeddedObject());
    uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier3(xAccess->getByName(u"Object3"_ustr), uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xObj3(xEOSupplier3->getEmbeddedObject());
    uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier4(xAccess->getByName(u"Object4"_ustr), uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xObj4(xEOSupplier4->getEmbeddedObject());
    //checking first object
    uno::Reference<document::XStorageBasedDocument> xSBDoc1(xObj1, uno::UNO_QUERY);
    uno::Reference<embed::XStorage> xStorage1(xSBDoc1->getDocumentStorage());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(SOFFICE_FILEFORMAT_8), comphelper::OStorageHelper::GetXStorageFormat(xStorage1));
    //checking second object
    uno::Reference<document::XStorageBasedDocument> xSBDoc2(xObj2, uno::UNO_QUERY);
    uno::Reference<embed::XStorage> xStorage2(xSBDoc2->getDocumentStorage());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(SOFFICE_FILEFORMAT_8), comphelper::OStorageHelper::GetXStorageFormat(xStorage2));
    //checking third object
    uno::Reference<document::XStorageBasedDocument> xSBDoc3(xObj3, uno::UNO_QUERY);
    uno::Reference<embed::XStorage> xStorage3(xSBDoc3->getDocumentStorage());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(SOFFICE_FILEFORMAT_8), comphelper::OStorageHelper::GetXStorageFormat(xStorage3));
    //checking fourth object
    uno::Reference<document::XStorageBasedDocument> xSBDoc4(xObj4, uno::UNO_QUERY);
    uno::Reference<embed::XStorage> xStorage4(xSBDoc4->getDocumentStorage());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(SOFFICE_FILEFORMAT_8), comphelper::OStorageHelper::GetXStorageFormat(xStorage4));
}

DECLARE_ODFEXPORT_TEST(testTdf152710, "tdf152710.odt")
{
    // Without this fix in place, this test would have crash at import time
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_ODFEXPORT_TEST(testredlineTextFrame, "redlineTextFrame.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //Note this is for a crash test
    //Counting the Number of Frames and checking with the expected count
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_ODFEXPORT_TEST(testTdf107292, "tdf107292.odt")
{
    // tracked deletions at the same position were loaded in reverse order
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without this fix in place, this test would have failed with
    // - Expected: Lorem ipsum dolor sit...
    // - Actual  :  dolor ipsumLorem sit...
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum dolor sit..."_ustr, getParagraph(1)->getString());
}

DECLARE_ODFEXPORT_TEST(testTdf140437, "tdf140437.odt")
{
    // Without the fix in place, the document would have failed to load
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // at least one field should be detected
    CPPUNIT_ASSERT(xFields->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131621)
{
    loadAndReload("tdf131621.ott");
    CPPUNIT_ASSERT_EQUAL(12, getShapes());
    //Crash test, Check number of pages
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135144)
{
    loadAndReload("tdf135144.docx");
    //Crashes at import time after roundtrip
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

DECLARE_ODFEXPORT_TEST(testTdf130950, "tdf130950.odt")
{
    //Crashes at import time
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

DECLARE_ODFEXPORT_TEST(testFdo38244, "fdo38244.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // See ooxmlexport's testFdo38244().

    // Test comment range feature.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));

    // Test properties
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"__Fieldmark__4_1833023242"_ustr, getProperty<OUString>(xPropertySet, u"Name"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"M"_ustr, getProperty<OUString>(xPropertySet, u"Initials"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testSenderInitials)
{
    loadAndReload("sender-initials.fodt");
    // Test sender-initial properties (both annotation metadata and text field)
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // first 3 are annotations, last 2 are text fields
    for (unsigned i = 0; i < 3; ++i)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"I"_ustr, getProperty<OUString>(xPropertySet, u"Initials"_ustr));
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, u"IsFixed"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"I"_ustr, getProperty<OUString>(xPropertySet, u"Content"_ustr));
    }
}

DECLARE_ODFEXPORT_TEST(testResolvedComment, "resolved-comment.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, u"Resolved"_ustr));
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPropertySet, u"Resolved"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92379)
{
    auto verify = [this]() {
        // frame style fo:background-color was not imported
        uno::Reference<container::XNameAccess> xStyles(getStyles(u"FrameStyles"_ustr));
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(u"encarts"_ustr),
                uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0xffcc99), getProperty<Color>(xStyle, u"BackColorRGB"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xStyle, u"BackColorTransparency"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0xffcc99), getProperty<Color>(xStyle, u"FillColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xStyle, u"FillTransparence"_ustr));

        uno::Reference<beans::XPropertySet> xFrameStyle2(xStyles->getByName(u"Untitled1"_ustr),
                uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(xFrameStyle2, u"BackColorRGB"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xFrameStyle2, u"BackTransparent"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xFrameStyle2, u"BackColorTransparency"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xFrameStyle2, u"FillStyle"_ustr));
    // unfortunately this is actually the pool default value, which would be hard to fix - but it isn't a problem because style is NONE
    //    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(xFrameStyle2, "FillColor"));
    //    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xFrameStyle2, "FillTransparence"));

        // paragraph style fo:background-color was wrongly inherited despite being
        // overridden in derived style
        uno::Reference<container::XNameAccess> xParaStyles(getStyles(u"ParagraphStyles"_ustr));
        uno::Reference<beans::XPropertySet> xStyle1(xParaStyles->getByName(
                u"Titre Avis expert"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0x661900), getProperty<Color>(xStyle1, u"ParaBackColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStyle1, u"ParaBackTransparent"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xStyle1, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x661900), getProperty<Color>(xStyle1, u"FillColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xStyle1, u"FillTransparence"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(xStyle1, u"CharColor"_ustr));

        uno::Reference<beans::XPropertySet> xStyle2(xParaStyles->getByName(
                u"Avis expert questions"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xStyle2, u"ParaBackColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle2, u"ParaBackTransparent"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xStyle2, u"FillStyle"_ustr));
    // unfortunately this is actually the pool default value, which would be hard to fix - but it isn't a problem because style is NONE
    //    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(xStyle2, "FillColor"));
    //    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xStyle2, "FillTransparence"));
        CPPUNIT_ASSERT_EQUAL(Color(0x661900), getProperty<Color>(xStyle2, u"CharColor"_ustr));

        uno::Reference<beans::XPropertySet> xStyle31(xParaStyles->getByName(
                u"avis expert questions non cadres"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0x801900), getProperty<Color>(xStyle31, u"ParaBackColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStyle31, u"ParaBackTransparent"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xStyle31, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x801900), getProperty<Color>(xStyle31, u"FillColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xStyle31, u"FillTransparence"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x661900), getProperty<Color>(xStyle31, u"CharColor"_ustr));

        uno::Reference<beans::XPropertySet> xStyle32(xParaStyles->getByName(
                u"Avis expert rXponses"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xStyle32, u"ParaBackColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle32, u"ParaBackTransparent"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xStyle32, u"FillStyle"_ustr));
    // unfortunately this is actually the pool default value, which would be hard to fix - but it isn't a problem because style is NONE
    //    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(xStyle32, "FillColor"));
    //    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xStyle32, "FillTransparence"));
        CPPUNIT_ASSERT_EQUAL(Color(0x461900), getProperty<Color>(xStyle32, u"CharColor"_ustr));
    };

    createSwDoc("tdf92379.fodt");
    verify();
    saveAndReload(mpFilter);
    verify();

    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // check that fo:background-color attribute is exported properly
    assertXPath(pXmlDoc, "//style:style[@style:family='graphic' and @style:name='encarts']/style:graphic-properties[@fo:background-color='#ffcc99']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='graphic' and @style:name='Untitled1']/style:graphic-properties[@fo:background-color='transparent']", 1);

    // check that fo:background-color attribute is exported properly
    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph' and @style:display-name='Titre Avis expert']/style:paragraph-properties[@fo:background-color='#661900']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph' and @style:display-name='Avis expert questions']/style:paragraph-properties[@fo:background-color='transparent']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph' and @style:display-name='avis expert questions non cadres']/style:paragraph-properties[@fo:background-color='#801900']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph' and @style:display-name='Avis expert rXponses']/style:paragraph-properties[@fo:background-color='transparent']", 1);
}

DECLARE_ODFEXPORT_TEST(testFdo79358, "fdo79358.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // the boolean properties of the index were not exported properly
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTOCProps(xTOCIndex, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTOCProps, u"CreateFromOutline"_ustr));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTOCProps, u"CreateFromMarks"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTOCProps, u"CreateFromLevelParagraphStyles"_ustr));
    // check that the source styles are preserved too while at it
    uno::Reference<container::XIndexReplace> xLevels(
        getProperty< uno::Reference<container::XIndexReplace> >(xTOCProps,
            u"LevelParagraphStyles"_ustr));
    uno::Sequence<OUString> seq { u"Heading"_ustr };
    CPPUNIT_ASSERT_EQUAL(uno::Any(seq), xLevels->getByIndex(1));
    CPPUNIT_ASSERT_EQUAL(uno::Any(uno::Sequence<OUString>()), xLevels->getByIndex(2));
}

DECLARE_ODFEXPORT_TEST(testTextframeGradient, "textframe-gradient.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

    // MCGR: Use the completely imported gradient to check for correctness
    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(0.0, aColorStops[0].getStopOffset());
    CPPUNIT_ASSERT_EQUAL(Color(0xc0504d), Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(Color(0xd99594), Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);

    xFrame.set(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

    // MCGR: Use the completely imported gradient to check for correctness
    aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(0.0, aColorStops[0].getStopOffset());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(COL_GRAY7, Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
}

CPPUNIT_TEST_FIXTURE(Test, testDuplicateCrossRefHeadingBookmark)
{
    loadAndReload("CrossRefHeadingBookmark.fodt");
    // the file contains invalid duplicate heading cross reference bookmarks
    // but we have to round trip them, tdf#94804

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarks =
        xBookmarksSupplier->getBookmarks();
    uno::Reference<text::XTextContent> xBookmark1(
        xBookmarks->getByName(u"__RefHeading__8284_1826734303"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmark1.is());
    CPPUNIT_ASSERT_THROW(xBookmarks->getByName(u"__RefHeading__1673_25705824"_ustr), container::NoSuchElementException);

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Any aField1 = xFields->nextElement();
    uno::Reference<text::XTextField> xField1(aField1, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, xField1->getPresentation(false));
    uno::Any aField2 = xFields->nextElement();
    uno::Reference<text::XTextField> xField2(aField2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, xField2->getPresentation(false));
}

DECLARE_ODFEXPORT_TEST(testFdo60769, "fdo60769.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test multi-paragraph comment range feature.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType =  getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr);
        // First paragraph: no field end, no anchor
        CPPUNIT_ASSERT(aType == "Text" || aType == "Annotation");
    }

    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType =  getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr);
        // Second paragraph: no field start
        CPPUNIT_ASSERT(aType == "Text" || aType == "AnnotationEnd");
    }
}

DECLARE_ODFEXPORT_TEST(testTdf115815, "tdf115815.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test comment range feature on tracked deletion.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    bool bAnnotationStart = false;
    bool bBeforeAnnotation = true;
    OUString sTextBeforeAnnotation;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType = getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr);
        // there is no AnnotationEnd with preceding AnnotationStart,
        // i.e. annotation with lost range
        CPPUNIT_ASSERT(aType != "AnnotationEnd" || !bAnnotationStart);

        bAnnotationStart = (aType == "Annotation");

        // collect paragraph text before the first annotation
        if (bBeforeAnnotation)
        {
            if (bAnnotationStart)
                bBeforeAnnotation = false;
            else if (aType == "Text")
            {
                uno::Reference<text::XTextRange> xRun(xPropertySet, uno::UNO_QUERY);
                sTextBeforeAnnotation += xRun->getString();
            }
        }
    }

    // This was "Lorem ipsum" (collapsed annotation range)
    CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, sTextBeforeAnnotation);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58949)
{
    comphelper::ScopeGuard g([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::MathTypeToMath::set(true, pBatch);
        pBatch->commit();
    });

    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::Microsoft::Import::MathTypeToMath::set(false, pBatch);
    pBatch->commit();
    loadAndReload("fdo58949.docx");

    /*
     * The problem was that the exporter didn't insert "Obj102" to the
     * resulting zip file. No idea how to check for "broken" (missing OLE data
     * and replacement image) OLE objects using UNO, so we'll check the zip file directly.
     */

    save(u"writer8"_ustr);

    uno::Sequence<uno::Any> aArgs{ uno::Any(maTempFile.GetURL()) };
    uno::Reference<container::XNameAccess> xNameAccess(m_xSFactory->createInstanceWithArguments(u"com.sun.star.packages.zip.ZipFileAccess"_ustr, aArgs), uno::UNO_QUERY);
    const css::uno::Sequence<OUString> aNames(xNameAccess->getElementNames());
    // The exported document must have three objects named ObjNNN. The names are assigned in
    // OLEHandler::copyOLEOStream using a static counter, and actual numbers depend on previous
    // tests; so just count the matching names here.
    int nMatches = 0;
    for (const OUString& sName : aNames)
    {
        OUString sRest;
        if (sName.startsWith("Obj", &sRest))
        {
            // all following characters must be decimal digits; minimal value is 100
            bool bMatch = sRest.getLength() >= 3
                          && std::all_of(sRest.getStr(), sRest.getStr() + sRest.getLength(),
                                 [](sal_Unicode ch) { return ch >= '0' && ch <= '9'; });
            if (bMatch)
                ++nMatches;
        }
    }
    CPPUNIT_ASSERT_EQUAL(3, nMatches);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134987)
{
    loadAndReload("tdf134987.docx");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTEOSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xAccess(xTEOSupplier->getEmbeddedObjects());
    uno::Sequence<OUString> aSeq(xAccess->getElementNames());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aSeq.getLength());

    OUString aMediaType;
    // checking first object (formula)
    {
        uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier(xAccess->getByName(u"Object1"_ustr), uno::UNO_QUERY);
        uno::Reference<lang::XComponent> xObj(xEOSupplier->getEmbeddedObject());
        CPPUNIT_ASSERT(xObj.is());

        uno::Reference<document::XStorageBasedDocument> xSBDoc(xObj, uno::UNO_QUERY);
        uno::Reference<embed::XStorage> xStorage(xSBDoc->getDocumentStorage());
        CPPUNIT_ASSERT(xStorage.is());

        uno::Reference< beans::XPropertySet > xStorProps(xStorage, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xStorProps->getPropertyValue(u"MediaType"_ustr) >>= aMediaType);
        CPPUNIT_ASSERT(aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII));
    }
    // checking second object (chart)
    {
        uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier(xAccess->getByName(u"Diagram 1"_ustr), uno::UNO_QUERY);
        uno::Reference<lang::XComponent> xObj(xEOSupplier->getEmbeddedObject());
        CPPUNIT_ASSERT(xObj.is());

        uno::Reference<document::XStorageBasedDocument> xSBDoc(xObj, uno::UNO_QUERY);
        uno::Reference<embed::XStorage> xStorage(xSBDoc->getDocumentStorage());
        CPPUNIT_ASSERT(xStorage.is());

        uno::Reference< beans::XPropertySet > xStorProps(xStorage, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xStorProps->getPropertyValue(u"MediaType"_ustr) >>= aMediaType);
        CPPUNIT_ASSERT(aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII));
    }
    // checking third object (chart)
    {
        uno::Reference<document::XEmbeddedObjectSupplier> xEOSupplier(xAccess->getByName(u"Diagram 2"_ustr), uno::UNO_QUERY);
        uno::Reference<lang::XComponent> xObj(xEOSupplier->getEmbeddedObject());
        CPPUNIT_ASSERT(xObj.is());

        uno::Reference<document::XStorageBasedDocument> xSBDoc(xObj, uno::UNO_QUERY);
        uno::Reference<embed::XStorage> xStorage(xSBDoc->getDocumentStorage());
        CPPUNIT_ASSERT(xStorage.is());

        uno::Reference< beans::XPropertySet > xStorProps(xStorage, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xStorProps->getPropertyValue(u"MediaType"_ustr) >>= aMediaType);
        CPPUNIT_ASSERT(aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII));
    }
}

DECLARE_ODFEXPORT_TEST(testStylePageNumber, "ooo321_stylepagenumber.odt")
{
    CPPUNIT_ASSERT_EQUAL(5, getPages());
    uno::Reference<text::XTextContent> xTable1(getParagraphOrTable(1));
// actually no break attribute is written in this case
//    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, getProperty<style::BreakType>(xTable1, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(u"Left Page"_ustr, getProperty<OUString>(xTable1, u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xTable1, u"PageNumberOffset"_ustr));

    uno::Reference<text::XTextContent> xPara1(getParagraphOrTable(2));
    CPPUNIT_ASSERT_EQUAL(u"Right Page"_ustr, getProperty<OUString>(xPara1, u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPara1, u"PageNumberOffset"_ustr));

    // i#114163 tdf#77111: OOo < 3.3 bug, it wrote "auto" as "0" for tables
    uno::Reference<beans::XPropertySet> xTable0(getParagraphOrTable(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Left Page"_ustr, getProperty<OUString>(xTable0, u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xTable0->getPropertyValue(u"PageNumberOffset"_ustr));

    uno::Reference<beans::XPropertySet> xPara0(getParagraphOrTable(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Right Page"_ustr, getProperty<OUString>(xPara0, u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xPara0->getPropertyValue(u"PageNumberOffset"_ustr));

    uno::Reference<container::XNameAccess> xParaStyles = getStyles(u"ParagraphStyles"_ustr);
    uno::Reference<beans::XPropertySet> xStyle1(xParaStyles->getByName(u"stylewithbreak1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Right Page"_ustr, getProperty<OUString>(xStyle1, u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xStyle1, u"PageNumberOffset"_ustr));

    uno::Reference<beans::XPropertySet> xStyle0(xParaStyles->getByName(u"stylewithbreak0"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"First Page"_ustr, getProperty<OUString>(xStyle0, u"PageDescName"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xStyle0->getPropertyValue(u"PageNumberOffset"_ustr));
}

DECLARE_ODFEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure paragraph and character attributes don't interfere
    // First paragraph has a paragraph border and a character border included by the paragraph style

    // Paragraph border of first paragraph
    {
        const table::BorderLine2 aFirstParTopBorder(0x6666FF,2,26,26,7,55);
        const sal_Int32 aFirstParTopPadding(150);
        uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);

        // Top border
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,u"TopBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,u"TopBorderDistance"_ustr));

        // Bottom border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,u"BottomBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,u"BottomBorderDistance"_ustr));

        // Left border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,u"LeftBorderDistance"_ustr));

        // Right border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,u"RightBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,u"RightBorderDistance"_ustr));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,u"ParaShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(aShadow.IsTransparent));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation(0), aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aShadow.ShadowWidth);
    }

    // Character border for first paragraph
    {
        const table::BorderLine2 aFirstParCharTopBorder(0xFF3333,0,37,0,2,37);
        const sal_Int32 aFirstParCharTopPadding(450);
        uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);

        // Top border
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharTopBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharTopBorderDistance"_ustr));

        // Bottom border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharBottomBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharBottomBorderDistance"_ustr));

        // Left border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharLeftBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharLeftBorderDistance"_ustr));

        // Right border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharRightBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharRightBorderDistance"_ustr));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(Color(0xFF3333), Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(aShadow.IsTransparent));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation(2), aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(280), aShadow.ShadowWidth);

        // Check autostyle
        {
            uno::Reference< style::XAutoStyleFamily > xAutoStyleFamily(getAutoStyles(u"ParagraphStyles"_ustr));
            uno::Reference < container::XEnumeration > xAutoStylesEnum( xAutoStyleFamily->createEnumeration() );
            CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(xAutoStylesEnum->hasMoreElements()));

            // Top border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharTopBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharTopBorderDistance"_ustr));

            // Bottom border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharBottomBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharBottomBorderDistance"_ustr));

            // Left border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharLeftBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharLeftBorderDistance"_ustr));

            // Right border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,u"CharRightBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,u"CharRightBorderDistance"_ustr));
        }
    }

    // Second paragraph's second text portion has a character style named CharDiffBor
    // This style includes border with different sides
    {

        table::BorderLine2 aBorderArray[4] =
        {
            table::BorderLine2(0xFF3333,0,37,0,14,37),     // Top (fine dashed line)
            table::BorderLine2(0x99FF66,26,26,53,11,106),  // Bottom
            table::BorderLine2(0x6666FF,9,26,9,12,71),     // Left
            table::BorderLine2(0,0,0,0,table::BorderLineStyle::NONE,0) // Right
        };

        sal_Int32 aDistances[4] = { 400 /*Top*/, 300 /*Bottom*/, 250 /*Left*/, 0 /*Right*/ };

        // Get second text portion of second paragraph
        uno::Reference < beans::XPropertySet > xSet( getRun(getParagraph(2),2), uno::UNO_QUERY );

        // Top border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[0], getProperty<table::BorderLine2>(xSet,u"CharTopBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aDistances[0], getProperty<sal_Int32>(xSet,u"CharTopBorderDistance"_ustr));

        // Bottom border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[1], getProperty<table::BorderLine2>(xSet,u"CharBottomBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aDistances[1], getProperty<sal_Int32>(xSet,u"CharBottomBorderDistance"_ustr));

        // Left border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[2], getProperty<table::BorderLine2>(xSet,u"CharLeftBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aDistances[2], getProperty<sal_Int32>(xSet,u"CharLeftBorderDistance"_ustr));

        // Right border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[3], getProperty<table::BorderLine2>(xSet,u"CharRightBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(aDistances[3], getProperty<sal_Int32>(xSet,u"CharRightBorderDistance"_ustr));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(aShadow.IsTransparent));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation(3), aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(79), aShadow.ShadowWidth);

        // Check character style
        {
            uno::Reference< container::XNameAccess > xStyleFamily = getStyles(u"CharacterStyles"_ustr);
            uno::Reference < beans::XPropertySet > xStyleSet(xStyleFamily->getByName(u"CharDiffBor"_ustr), uno::UNO_QUERY);

            // Top border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[0], getProperty<table::BorderLine2>(xStyleSet,u"CharTopBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aDistances[0], getProperty<sal_Int32>(xStyleSet,u"CharTopBorderDistance"_ustr));

            // Bottom border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[1], getProperty<table::BorderLine2>(xStyleSet,u"CharBottomBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aDistances[1], getProperty<sal_Int32>(xStyleSet,u"CharBottomBorderDistance"_ustr));

            // Left border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[2], getProperty<table::BorderLine2>(xStyleSet,u"CharLeftBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aDistances[2], getProperty<sal_Int32>(xStyleSet,u"CharLeftBorderDistance"_ustr));

            // Right border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[3], getProperty<table::BorderLine2>(xStyleSet,u"CharRightBorder"_ustr));
            CPPUNIT_ASSERT_EQUAL(aDistances[3], getProperty<sal_Int32>(xStyleSet,u"CharRightBorderDistance"_ustr));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testProtectionKey)
{
    auto verify = [this]() {
        static OUString constexpr password(u"1012345678901234567890123456789012345678901234567890"_ustr);

        // check 1 invalid OOo legacy password and 3 valid ODF 1.2 passwords
        uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xSect0(xSections->getByIndex(0), uno::UNO_QUERY);
        uno::Sequence<sal_Int8> const key0(getProperty<uno::Sequence<sal_Int8>>(xSect0, u"ProtectionKey"_ustr));
        CPPUNIT_ASSERT(SvPasswordHelper::CompareHashPassword(key0, password));
        uno::Reference<beans::XPropertySet> xSect1(xSections->getByIndex(1), uno::UNO_QUERY);
        uno::Sequence<sal_Int8> const key1(getProperty<uno::Sequence<sal_Int8>>(xSect1, u"ProtectionKey"_ustr));
        CPPUNIT_ASSERT(SvPasswordHelper::CompareHashPassword(key1, password));
        uno::Reference<beans::XPropertySet> xSect2(xSections->getByIndex(2), uno::UNO_QUERY);
        uno::Sequence<sal_Int8> const key2(getProperty<uno::Sequence<sal_Int8>>(xSect2, u"ProtectionKey"_ustr));
        CPPUNIT_ASSERT(SvPasswordHelper::CompareHashPassword(key2, password));
        uno::Reference<beans::XPropertySet> xSect3(xSections->getByIndex(3), uno::UNO_QUERY);
        uno::Sequence<sal_Int8> const key3(getProperty<uno::Sequence<sal_Int8>>(xSect3, u"ProtectionKey"_ustr));
        CPPUNIT_ASSERT(SvPasswordHelper::CompareHashPassword(key3, password));
    };

    createSwDoc("protection-key.fodt");
    verify();
    saveAndReload(mpFilter);
    verify();

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//text:section[@text:name='Section0' and @text:protected='true' and @text:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=']");
    assertXPath(pXmlDoc, "//text:section[@text:name='Section1' and @text:protected='true' and @text:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=']");
    assertXPath(pXmlDoc, "//text:section[@text:name='Section2' and @text:protected='true' and @text:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256' and @text:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=']");
    assertXPath(pXmlDoc, "//text:section[@text:name='Section3' and @text:protected='true' and @text:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256' and @text:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=']");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128188)
{
    loadAndReload("footnote-collect-at-end-of-section.fodt");
    SwDoc* pDoc = getSwDoc();
    SwFootnoteIdxs const& rFootnotes(pDoc->GetFootnoteIdxs());
    // Section1
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnotes[0]->GetFootnote().GetNumber());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnotes[0]->GetFootnote().GetNumberRLHidden());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnotes[1]->GetFootnote().GetNumber());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnotes[1]->GetFootnote().GetNumberRLHidden());
    // Section2
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnotes[2]->GetFootnote().GetNumber());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnotes[2]->GetFootnote().GetNumberRLHidden());
    // deleted
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnotes[3]->GetFootnote().GetNumber());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnotes[3]->GetFootnote().GetNumberRLHidden());
    // deleted
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), rFootnotes[4]->GetFootnote().GetNumber());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rFootnotes[4]->GetFootnote().GetNumberRLHidden());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), rFootnotes[5]->GetFootnote().GetNumber());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rFootnotes[5]->GetFootnote().GetNumberRLHidden());
}

DECLARE_ODFEXPORT_TEST(testFdo43807, "fdo43807.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Drop Caps"_ustr,getProperty<OUString>(xSet,u"DropCapCharStyleName"_ustr));

    xSet.set(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"User Defined Drop Caps"_ustr,getProperty<OUString>(xSet,u"DropCapCharStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103091)
{
    loadAndReload("tdf103091.fodt");
    // check that all conditional paragraph style conditions are imported
    uno::Reference<container::XNameAccess> xParaStyles(getStyles(u"ParagraphStyles"_ustr));
    uno::Reference<beans::XPropertySet> xStyle1(xParaStyles->getByName(
            u"Text body"_ustr), uno::UNO_QUERY);
    auto conditions(getProperty<uno::Sequence<beans::NamedValue>>(xStyle1, u"ParaStyleConditions"_ustr));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(28), conditions.getLength());
    CPPUNIT_ASSERT_EQUAL(u"TableHeader"_ustr, conditions[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Addressee"_ustr), conditions[0].Value);
    CPPUNIT_ASSERT_EQUAL(u"Table"_ustr, conditions[1].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Bibliography 1"_ustr), conditions[1].Value);
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, conditions[2].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Bibliography Heading"_ustr), conditions[2].Value);
    CPPUNIT_ASSERT_EQUAL(u"Section"_ustr, conditions[3].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Caption"_ustr), conditions[3].Value);
    CPPUNIT_ASSERT_EQUAL(u"Footnote"_ustr, conditions[4].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Salutation"_ustr), conditions[4].Value);
    CPPUNIT_ASSERT_EQUAL(u"Endnote"_ustr, conditions[5].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 1"_ustr), conditions[5].Value);
    CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, conditions[6].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 2"_ustr), conditions[6].Value);
    CPPUNIT_ASSERT_EQUAL(u"Footer"_ustr, conditions[7].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 3"_ustr), conditions[7].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel1"_ustr, conditions[8].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 4"_ustr), conditions[8].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel2"_ustr, conditions[9].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 5"_ustr), conditions[9].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel3"_ustr, conditions[10].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 6"_ustr), conditions[10].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel4"_ustr, conditions[11].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 7"_ustr), conditions[11].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel5"_ustr, conditions[12].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 8"_ustr), conditions[12].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel6"_ustr, conditions[13].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 9"_ustr), conditions[13].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel7"_ustr, conditions[14].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents 10"_ustr), conditions[14].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel8"_ustr, conditions[15].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Contents Heading"_ustr), conditions[15].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel9"_ustr, conditions[16].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Standard"_ustr), conditions[16].Value);
    CPPUNIT_ASSERT_EQUAL(u"OutlineLevel10"_ustr, conditions[17].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Drawing"_ustr), conditions[17].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel1"_ustr, conditions[18].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Endnote"_ustr), conditions[18].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel2"_ustr, conditions[19].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"First line indent"_ustr), conditions[19].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel3"_ustr, conditions[20].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Footer"_ustr), conditions[20].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel4"_ustr, conditions[21].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Footer left"_ustr), conditions[21].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel5"_ustr, conditions[22].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Footer right"_ustr), conditions[22].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel6"_ustr, conditions[23].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Footnote"_ustr), conditions[23].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel7"_ustr, conditions[24].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Frame contents"_ustr), conditions[24].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel8"_ustr, conditions[25].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Hanging indent"_ustr), conditions[25].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel9"_ustr, conditions[26].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Header"_ustr), conditions[26].Value);
    CPPUNIT_ASSERT_EQUAL(u"NumberingLevel10"_ustr, conditions[27].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Header left"_ustr), conditions[27].Value);
}

DECLARE_ODFEXPORT_TEST(testTextframeTransparentShadow, "textframe-transparent-shadow.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xPicture = getShape(1);
    // ODF stores opacity of 75%, that means 25% transparency.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<sal_Int32>(xPicture, u"ShadowTransparence"_ustr));
}

DECLARE_ODFEXPORT_TEST(testRelhPage, "relh-page.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xTextFrame = getShape(1);
    // This was text::RelOrientation::FRAME (the default), RelativeHeightRelation was not handled in xmloff.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xTextFrame, u"RelativeHeightRelation"_ustr));
    // Make sure rel-height-rel doesn't affect width.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xTextFrame, u"RelativeWidthRelation"_ustr));

    // This was 2601, 20% height was relative from margin, not page.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3168), getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height").toInt32());
}

DECLARE_ODFEXPORT_TEST(testRelhPageTdf80282, "relh-page-tdf80282.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xTextFrame = getShape(1);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/fly/infos/bounds", "height", u"8391");
    assertXPath(pXmlDoc, "//anchored/fly/infos/bounds", "width", u"5953");
}

DECLARE_ODFEXPORT_TEST(testRelwPage, "relw-page.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xTextFrame = getShape(1);
    // This was text::RelOrientation::FRAME (the default), RelativeWidthRelation was not handled in xmloff.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xTextFrame, u"RelativeWidthRelation"_ustr));
    // Make sure rel-width-rel doesn't affect height.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xTextFrame, u"RelativeHeightRelation"_ustr));

    // This was 3762, 40% width was relative from margin, not page.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4896), getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width").toInt32());
}

DECLARE_ODFEXPORT_TEST(testTextFrameVertAdjust, "textframe-vertadjust.odt")
{
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test import/export of new frame attribute called TextVerticalAdjust

    // 1st frame's context is adjusted to the top
    uno::Reference<beans::XPropertySet> xFrame(getTextFrameByName(u"Rectangle 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xFrame, u"TextVerticalAdjust"_ustr));
    // 2nd frame's context is adjusted to the center
    xFrame.set(getTextFrameByName(u"Rectangle 2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER, getProperty<drawing::TextVerticalAdjust>(xFrame, u"TextVerticalAdjust"_ustr));
    // 3rd frame's context is adjusted to the bottom
    xFrame.set(getTextFrameByName(u"Rectangle 3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xFrame, u"TextVerticalAdjust"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf69500)
{
    createSwDoc();

    static constexpr OUString sToolBarName = u"private:resource/toolbar/custom_toolbar_1"_ustr;

    auto getUIConfigManager = [this]() {
        css::uno::Reference<css::uno::XComponentContext> xContext
            = comphelper::getProcessComponentContext();
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xModel.is());
        uno::Reference<ui::XUIConfigurationManagerSupplier> xConfigSupplier(xModel, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xConfigSupplier.is());
        uno::Reference<ui::XUIConfigurationManager> xConfigManager
            = xConfigSupplier->getUIConfigurationManager();
        return xConfigManager;
    };

    // Create and persist a custom toolbar to the document
    {
        uno::Reference<ui::XUIConfigurationManager> xConfigManager = getUIConfigManager();

        uno::Reference<container::XIndexContainer> xIndexContainer(xConfigManager->createSettings(),
                                                                   uno::UNO_SET_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xIndexContainer, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xIndexContainer, uno::UNO_QUERY_THROW);

        xProps->setPropertyValue(u"UIName"_ustr, uno::Any(u"Custom Toolbar 1"_ustr));

        xConfigManager->insertSettings(sToolBarName, xIndexAccess);

        uno::Reference<ui::XUIConfigurationPersistence> xPersistence(xConfigManager,
                                                                     uno::UNO_QUERY_THROW);
        xPersistence->store();
    }

    saveAndReload(mpFilter);

    // Without the fix, the toolbar will be gone after save-and-reload
    {
        uno::Reference<ui::XUIConfigurationManager> xConfigManager = getUIConfigManager();

        CPPUNIT_ASSERT(xConfigManager->hasSettings(sToolBarName));
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
