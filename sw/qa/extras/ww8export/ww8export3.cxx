/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <IDocumentSettingAccess.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }
};

DECLARE_WW8EXPORT_TEST(testTdf37778_readonlySection, "tdf37778_readonlySection.doc")
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT( xStorable->isReadonly() );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // The problem was that section protection was being enabled in addition to being read-only.
    // This created an explicit section with protection. There should be just the default, non-explicit section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of Sections", sal_Int32(0), xSections->getCount());

    // tdf#127862: page fill color (in this case white) was lost
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf122429_header, "tdf122429_header.doc")
{
    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> pageStyle(pageStyles->getByName("Default Style"), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(headerIsOn);
}

DECLARE_WW8EXPORT_TEST(testTdf122460_header, "tdf122460_header.odt")
{
    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> pageStyle(pageStyles->getByName("Default Style"), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(headerIsOn);
}

DECLARE_WW8EXPORT_TEST(testFdo53985, "fdo53985.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTables->getCount()); // Only 4 tables were imported.

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compatibility: Protect form", true, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ) );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xSections->getCount()); // The first paragraph wasn't counted as a section.

    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", true, getProperty<bool>(xSect, "IsProtected"));
    xSect.set(xSections->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section4 is protected", false, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_WW8EXPORT_TEST(testTdf79435_legacyInputFields, "tdf79435_legacyInputFields.docx")
{
    //using .docx input file to verify cross-format compatibility.
    uno::Reference<text::XFormField> xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(5), 3), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters(xFormField->getParameters());

    OUString sTmp;
    // Too often the string reader can fail during import - fix that first to prevent round-tripping garbage.
    // (for example BR-1010B.doc from tdf#48097)
    //xParameters->getByName("EntryMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    //xParameters->getByName("Help") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("F1Help"), sTmp);
    //xParameters->getByName("ExitMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    xParameters->getByName("Description") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("StatusHelp"), sTmp);
    //xParameters->getByName("Content") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("Camelcase"), sTmp);
    //xParameters->getByName("Format") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("TITLE CASE"), sTmp);

    sal_uInt16 nMaxLength = 0;
    xParameters->getByName("MaxLength") >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Max Length", sal_uInt16(10), nMaxLength);

    // too bad this is based on character runs - just found try trial and error.
    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(6), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("calculated"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(7), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("currentDate"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(7), 6), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("currentTime"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(8), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("number"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(8), 6), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("date"), sTmp);
}

DECLARE_WW8EXPORT_TEST(testTdf120225_textControlCrossRef, "tdf120225_textControlCrossRef.doc")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStart"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    uno::Reference<container::XNamed> xBookmark(getProperty< uno::Reference<beans::XPropertySet> >(xPropertySet, "Bookmark"), uno::UNO_QUERY_THROW);

    // Critical test: does TextField's bookmark name match cross-reference?
    const OUString& sTextFieldName( xBookmark->getName() );
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sTextFieldName, getProperty<OUString>(xPropertySet, "SourceName"));

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    // TextFields should not be turned into real bookmarks.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xBookmarksByIdx->getCount());

    // The actual name isn't critical, but if it fails, it is worth asking why.
    CPPUNIT_ASSERT_EQUAL(OUString("Text1"), sTextFieldName);
}

DECLARE_WW8EXPORT_TEST(testTdf127316_autoEscapement, "tdf127316_autoEscapement.odt")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), "CharEscapement"), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-33.f, getProperty<float>(getRun(xPara, 2), "CharEscapement"), 3);
}

DECLARE_WW8EXPORT_TEST(testTdf121111_fillStyleNone, "tdf121111_fillStyleNone.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Numbering - First level"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(184,204,228), Color(getProperty<sal_uInt32>(xStyle, "ParaBackColor")));//R:184 G:204 B:228
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xStyle, "FillStyle"));

    uno::Reference<text::XTextRange> xText(getParagraph(12));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xText, "ParaBackColor")));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xText, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf112618_textbox_no_bg, "tdf112618_textbox_no_bg.doc")
{
    sal_uInt16 nTransparence = getProperty<sal_Int16>(getShape(2), "FillTransparence");
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(100), nTransparence);
    CPPUNIT_ASSERT_EQUAL(nTransparence, getProperty<sal_uInt16>(getShape(2), "BackColorTransparency"));
}

DECLARE_WW8EXPORT_TEST(testTdf101826_xattrTextBoxFill, "tdf101826_xattrTextBoxFill.doc")
{
    //Basic 1 Color Fill: gradient from yellow(FFFF00) to brown(767600) currently saves as mid-color
    CPPUNIT_ASSERT_MESSAGE("background color", Color(0xFF, 0xFF, 0x00) != getProperty<Color>(getShape(1), "BackColor"));
    //Basic 2 Color Fill: gradient from yellow(FFFF00) to green(00B050) currently saves as mid-color
    CPPUNIT_ASSERT_MESSAGE("background color", Color(0xFF, 0xFF, 0x00) != getProperty<Color>(getShape(4), "BackColor"));
    //Basic Picture Fill: Tux image
    CPPUNIT_ASSERT_EQUAL_MESSAGE("background image", drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(getShape(5), "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf123433_fillStyleStop, "tdf123433_fillStyleStop.doc")
{
    uno::Reference<text::XTextRange> xText(getParagraph(12));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xText, "ParaBackColor")));
}

DECLARE_WW8EXPORT_TEST(testTdf127862_pageFillStyle, "tdf127862_pageFillStyle.odt")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf94009_zeroPgMargin, "tdf94009_zeroPgMargin.odt")
{
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(defaultStyle, "TopMargin"));
}

DECLARE_WW8EXPORT_TEST(testTdf120711_joinedParagraphWithChangeTracking, "tdf120711.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(5);
    // last paragraph is not a list item
    CPPUNIT_ASSERT(style::NumberingType::CHAR_SPECIAL != numFormat);
}

DECLARE_WW8EXPORT_TEST(testBtlrCell, "btlr-cell.doc")
{
    // Without the accompanying fix in place, this test would have failed, as
    // the btlr text direction in the A1 cell was lost on DOC import and
    // export.
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xA1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, getProperty<sal_Int16>(xA1, "WritingMode"));

    uno::Reference<beans::XPropertySet> xB1(xTable->getCellByName("B1"), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xB1, "WritingMode");
    CPPUNIT_ASSERT(nActual == text::WritingMode2::LR_TB || nActual == text::WritingMode2::CONTEXT);

    uno::Reference<beans::XPropertySet> xC1(xTable->getCellByName("C1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, getProperty<sal_Int16>(xC1, "WritingMode"));
}

DECLARE_WW8EXPORT_TEST(testTdf118375export, "tdf118375_240degClockwise.doc")
{
    // The input document has one custom shape, which is rotated 240deg. Check
    // that it has the same position as in Word.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    sal_Int32 nPosX, nPosY;
    xShapeProps->getPropertyValue("HoriOrientPosition") >>= nPosX;
    xShapeProps->getPropertyValue("VertOrientPosition") >>= nPosY;
    // Allow some tolerance because rounding errors through integer arithmetic
    // in rotation.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5200.0, static_cast<double>(nPosX), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1152.0, static_cast<double>(nPosY), 1.0);
}

DECLARE_WW8EXPORT_TEST(testImageCommentAtChar, "image-comment-at-char.doc")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment start before the image was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));
}

DECLARE_WW8EXPORT_TEST(testTdf126708emf, "tdf126708_containsemf.odt")
{
    auto xShape = getShape(1);
    // First check the size of the EMF graphic contained in the shape.
    auto xGraphic = getProperty< uno::Reference<graphic::XGraphic> >(
        xShape, "Graphic");
    auto xSize = getProperty<awt::Size>(xGraphic, "Size100thMM");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8501), xSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18939), xSize.Width);

    // Now check that the shape itself has a decent size.
    // This size varies slightly when round tripping through doc format.
    xSize = getProperty<awt::Size>(xShape, "Size");
    CPPUNIT_ASSERT(abs(xSize.Height - 7629) <= 6);
    CPPUNIT_ASSERT(abs(xSize.Width - 17000) <= 6);
}

DECLARE_WW8EXPORT_TEST(testBtlrFrame, "btlr-frame.odt")
{
    if (!mbExported)
    {
        return;
    }

    // Without the accompanying fix in place, this test would have failed with a
    // beans.UnknownPropertyException, as the writing direction was lost, so the default direction
    // resulted in a conversion to a Writer text frame.
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9000), getProperty<sal_Int32>(xFrame, "RotateAngle"));
}

DECLARE_WW8EXPORT_TEST(testPresetDash, "tdf127166_prstDash_Word97.doc")
{
    // Error was, that the 'sys' preset dash styles were neither imported not
    // exported, the mixed styles had wrong dash-dot order, they were imported
    // with absolute values.
    const drawing::LineDash dashParams[] =
    {
        {drawing::DashStyle_RECTRELATIVE, 1, 400, 0, 0, 300}, // dash
        {drawing::DashStyle_RECTRELATIVE, 1, 400, 1, 100, 300}, // dashDot
        {drawing::DashStyle_RECTRELATIVE, 1, 100, 0, 0, 300}, // dot
        {drawing::DashStyle_RECTRELATIVE, 1, 800, 0, 0, 300}, // lgDash
        {drawing::DashStyle_RECTRELATIVE, 1, 800, 1, 100, 300}, // lgDashDot
        {drawing::DashStyle_RECTRELATIVE, 1, 800, 2, 100, 300}, // lgDashDotDot
        {drawing::DashStyle_RECTRELATIVE, 1, 300, 0, 0, 100}, // sysDash
        {drawing::DashStyle_RECTRELATIVE, 1, 300, 1, 100, 100}, // sysDashDot
        {drawing::DashStyle_RECTRELATIVE, 1, 300, 2, 100, 100}, // sysDashDotDot
        {drawing::DashStyle_RECTRELATIVE, 1, 100, 0, 0, 100} // sysDot
    };
    drawing::LineDash aPresetLineDash;
    drawing::LineDash aShapeLineDash;
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        aPresetLineDash = dashParams[i];
        uno::Reference<drawing::XShape> xShape = getShape(i+1);
        aShapeLineDash = getProperty<drawing::LineDash>(xShape, "LineDash");
        bool bIsEqual = aPresetLineDash.Style == aShapeLineDash.Style
                        && aPresetLineDash.Dots == aShapeLineDash.Dots
                        && aPresetLineDash.DotLen == aShapeLineDash.DotLen
                        && aPresetLineDash.Dashes == aShapeLineDash.Dashes
                        && aPresetLineDash.DashLen == aShapeLineDash.DashLen
                        && aPresetLineDash.Distance == aShapeLineDash.Distance;
        CPPUNIT_ASSERT_MESSAGE("LineDash differ", bIsEqual);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
