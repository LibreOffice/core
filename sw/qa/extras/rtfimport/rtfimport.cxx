/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <swmodeltestbase.hxx>

#include <config_fonts.h>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>

#include <o3tl/cppunittraitshelper.hxx>
#include <tools/UnitConversion.hxx>
#include <rtl/ustring.hxx>
#include <vcl/settings.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/configuration.hxx>

#include <editeng/charhiddenitem.hxx>

#include <ndindex.hxx>
#include <ndtxt.hxx>
#include <fmtautofmt.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfimport/data/", "Rich Text Format")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testN695479)
{
    createSwDoc("n695479.rtf");
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Negative ABSH should mean fixed size.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xPropertySet, "SizeType"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(300)),
                         getProperty<sal_Int32>(xPropertySet, "Height"));

    int nShapes = getShapes();
    CPPUNIT_ASSERT_EQUAL(3, nShapes);

    bool bFrameFound = false, bDrawFound = false;
    for (int i = 0; i < nShapes; ++i)
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(i + 1), uno::UNO_QUERY);
        if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        {
            // Both frames should be anchored to the first paragraph.
            bFrameFound = true;
            uno::Reference<text::XTextContent> xTextContent(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            uno::Reference<text::XText> xText = xRange->getText();
            CPPUNIT_ASSERT_EQUAL(OUString("plain"), xText->getString());

            if (i == 0)
                // Additionally, the first frame should have double border at the bottom.
                CPPUNIT_ASSERT_EQUAL(
                    table::BorderLineStyle::DOUBLE,
                    getProperty<table::BorderLine2>(xPropertySet, "BottomBorder").LineStyle);
        }
        else if (xServiceInfo->supportsService("com.sun.star.drawing.LineShape"))
        {
            // The older "drawing objects" syntax should be recognized.
            bDrawFound = true;
            xPropertySet.set(xServiceInfo, uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA,
                                 getProperty<sal_Int16>(xPropertySet, "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                                 getProperty<sal_Int16>(xPropertySet, "VertOrientRelation"));
        }
    }
    CPPUNIT_ASSERT(bFrameFound);
    CPPUNIT_ASSERT(bDrawFound);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117246)
{
    createSwDoc("tdf117246.rtf");
    // This was 2, all but the last \page was lost.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108943)
{
    createSwDoc("tdf108943.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    sal_Int32 nListtabStopPosition = 0;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "ListtabStopPosition")
            nListtabStopPosition = rProp.Value.get<sal_Int32>();
    }
    // This was 0, \tx was handled in paragraphs only (and not in list definitions).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nListtabStopPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo46662)
{
    createSwDoc("fdo46662.rtf");

    OUString listStyle;

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                             getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                             getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2),
                             getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3),
                             getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                             getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115715)
{
    createSwDoc("tdf115715.rtf");
    // This was 0, second paragraph was shifted to the right, it had the same
    // horizontal position as the 3rd paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1270),
                         getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81943)
{
    createSwDoc("tdf81943.rtf");
    // The shape wasn't in background.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "Opaque"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115155)
{
    createSwDoc("tdf115155.rtf");
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules");
    // 1st level
    comphelper::SequenceAsHashMap aMap(xLevels->getByIndex(0));
    // This was 1270: the left margin in the numbering rule was too small.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2540), aMap["IndentAt"].get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108951)
{
    createSwDoc("tdf108951.rtf");
    // This test is import-only, as we assert the list ID, which is OK to
    // re-order on export, but then this test would not pass anymore.

    // \ls2 was imported as WWNum1.
    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47036)
{
    createSwDoc("fdo47036.rtf");

    int nShapes = getShapes();
    CPPUNIT_ASSERT_EQUAL(3, nShapes);
    int nAtCharacter = 0;
    for (int i = 0; i < nShapes; ++i)
    {
        if (getProperty<text::TextContentAnchorType>(getShape(i + 1), "AnchorType")
            == text::TextContentAnchorType_AT_CHARACTER)
            nAtCharacter++;
    }
    // The image at the document start was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAtCharacter);

    // There should be 2 textboxes, not 4
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45182)
{
    createSwDoc("fdo45182.rtf");
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // Encoding in the footnote was wrong.
    CPPUNIT_ASSERT_EQUAL(u"\u017Eivnost\u00ED" SAL_NEWLINE_STRING ""_ustr, xTextRange->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo85812)
{
    createSwDoc("fdo85812.rtf");
    lang::Locale locale(
        getProperty<lang::Locale>(getRun(getParagraph(1), 1, "This "), "CharLocale"));
    // the \lang inside the group was applied to CJK not Western
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    // further testing indicates that Word is doing really weird stuff
    // \loch \hich \dbch is reset by opening a group
    locale = getProperty<lang::Locale>(getRun(getParagraph(2), 1, "CharGroup"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("ru"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("RU"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(2), 2, "AfterChar"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(3), 2, "AfterBookmark"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 1, "CharGroup"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("ru"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("RU"), locale.Country);
    locale
        = getProperty<lang::Locale>(getRun(getParagraph(4), 1, "CharGroup"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 2, "AfterChar"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale
        = getProperty<lang::Locale>(getRun(getParagraph(4), 2, "AfterChar"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(5), 2, "AfterBookmark"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(5), 2, "AfterBookmark"),
                                       "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    // \ltrch \rtlch works differently - it is sticky across groups
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 1, "CharGroup"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale
        = getProperty<lang::Locale>(getRun(getParagraph(6), 1, "CharGroup"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 2, "AfterChar"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale
        = getProperty<lang::Locale>(getRun(getParagraph(6), 2, "AfterChar"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("EG"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(7), 2, "AfterBookmark"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(7), 2, "AfterBookmark"),
                                       "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("EG"), locale.Country);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49692)
{
    createSwDoc("fdo49692.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
        {
            CPPUNIT_ASSERT(rProp.Value.get<OUString>().isEmpty());
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45190)
{
    createSwDoc("fdo45190.rtf");
    // inherited \fi should be reset
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));

    // but direct one not
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-100)),
                         getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf59699)
{
    createSwDoc("tdf59699.rtf");
    // This resulted in a lang.IndexOutOfBoundsException: the referenced graphic data wasn't imported.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xImage, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52066)
{
    createSwDoc("fdo52066.rtf");
    /*
     * The problem was that the height of the shape was too big.
     *
     * xray ThisComponent.DrawPage(0).Size.Height
     */
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(19)), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112211_2)
{
    createSwDoc("tdf112211-2.rtf");
    // Spacing between the bullet and the actual text was too large.
    // This is now around 269, large old value was 629.
    int nWidth = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout/"
                           "child::*[@type='PortionType::TabLeft']"_ostr,
                           "width"_ostr)
                     .toInt32();
    CPPUNIT_ASSERT_LESS(300, nWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122430)
{
    createSwDoc("tdf122430.rtf");
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 18, Actual  : 12', i.e. the font was smaller than expected.
    CPPUNIT_ASSERT_EQUAL(18.0f, getProperty<float>(getRun(getParagraph(1), 2), "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49892)
{
    createSwDoc("fdo49892.rtf");
    int nShapes = getShapes();
    CPPUNIT_ASSERT_EQUAL(5, nShapes);
    for (int i = 0; i < nShapes; ++i)
    {
        OUString aDescription = getProperty<OUString>(getShape(i + 1), "Description");
        if (aDescription == "red")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getShape(i + 1), "ZOrder"));
        else if (aDescription == "green")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(getShape(i + 1), "ZOrder"));
        else if (aDescription == "blue")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(getShape(i + 1), "ZOrder"));
        else if (aDescription == "rect")
        {
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                                 getProperty<sal_Int16>(getShape(i + 1), "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                                 getProperty<sal_Int16>(getShape(i + 1), "VertOrientRelation"));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52052)
{
    createSwDoc("fdo52052.rtf");
    // Make sure the textframe containing the text "third" appears on the 3rd page.
    CPPUNIT_ASSERT_EQUAL(OUString("third"),
                         parseDump("/root/page[3]/body/txt/anchored/fly/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testInk)
{
    createSwDoc("ink.rtf");
    /*
     * The problem was that the second segment had wrong command count and wrap type.
     *
     * oShape = ThisComponent.DrawPage(0)
     * oPathPropVec = oShape.CustomShapeGeometry(1).Value
     * oSegments = oPathPropVec(1).Value
     * msgbox oSegments(1).Count ' was 0x2000 | 10, should be 10
     * msgbox oShape.Surround ' was 2, should be 1
     */
    uno::Sequence<beans::PropertyValue> aProps
        = getProperty<uno::Sequence<beans::PropertyValue>>(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Segments")
            rProp.Value >>= aSegments;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), aSegments[1].Count);
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(1), "Surround"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo79319)
{
    createSwDoc("fdo79319.rtf");
    // the thin horizontal rule was imported as a big fat rectangle
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xShape, "RelativeWidth"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(16508), xShape->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(53), xShape->getSize().Height, 10);
#if 0
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xShape, "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xShape, "HoriOrient"));
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48442)
{
    createSwDoc("fdo48442.rtf");
    // The problem was that \pvmrg is the default in RTF, but not in Writer.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, "VertOrientRelation")); // was FRAME
}

CPPUNIT_TEST_FIXTURE(Test, testFdo55525)
{
    createSwDoc("fdo55525.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Negative left margin was ~missing, -191
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1877), getProperty<sal_Int32>(xTable, "LeftMargin"));
    // Cell width of A1 was 3332 (e.g. not set, 30% percent of total width)
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(896), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                             xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                                             .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo57708)
{
    createSwDoc("fdo57708.rtf");
    // There were two issues: the doc was of 2 pages and the picture was missing.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Two objects: a picture and a textframe.
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45183)
{
    createSwDoc("fdo45183.rtf");
    // Was text::WrapTextMode_PARALLEL, i.e. shpfblwtxt didn't send the shape below text.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(1), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Was 247, resulting in a table having width almost zero and height of 10+ pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16237), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo59953)
{
    createSwDoc("fdo59953.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A1 was 4998 (e.g. not set / not wide enough, ~50% of total width)
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7649), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                                              .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf84684)
{
    createSwDoc("tdf84684.rtf");
    // The ZOrder of the two children of the group shape were swapped.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xChild1(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was Pie 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Rectangle 1"), xChild1->getName());
    uno::Reference<container::XNamed> xChild2(xGroup->getByIndex(1), uno::UNO_QUERY);
    // This was Rectangle 1.
    CPPUNIT_ASSERT_EQUAL(OUString("Pie 2"), xChild2->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo81944)
{
    createSwDoc("fdo81944.rtf");
    // font properties in style were not imported
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Segoe UI"), getProperty<OUString>(xStyle, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(9.0f, getProperty<float>(xStyle, "CharHeight"));
    // not sure if this should be set on Asian or Complex or both?
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"),
                         getProperty<OUString>(xStyle, "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(11.0f, getProperty<float>(xStyle, "CharHeightComplex"));
}

CPPUNIT_TEST_FIXTURE(Test, testSbkodd)
{
    createSwDoc("sbkodd.rtf");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xCursor->getPage());
    xCursor->jumpToNextPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), xCursor->getPage());
}

CPPUNIT_TEST_FIXTURE(Test, testSbkeven)
{
    createSwDoc("sbkeven.rtf");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xCursor->getPage());
    xCursor->jumpToNextPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), xCursor->getPage());
    xCursor->jumpToNextPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(4), xCursor->getPage());
}

CPPUNIT_TEST_FIXTURE(Test, testPoshPosv)
{
    createSwDoc("posh-posv.rtf");
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                         getProperty<sal_Int16>(getShape(1), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER,
                         getProperty<sal_Int16>(getShape(1), "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "FrameIsAutomaticHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testPoshLeftRight)
{
    createSwDoc("posh-leftright.rtf");
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(getShape(1), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT,
                         getProperty<sal_Int16>(getShape(2), "HoriOrient"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96326)
{
    createSwDoc("tdf96326.rtf");
    // Make sure this is not checked.
    auto xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(1), 2), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters = xFormField->getParameters();
    // This was true, ffres=25 was interpreted as checked.
    CPPUNIT_ASSERT_EQUAL(false, bool(xParameters->hasElements()));

    // And this is checked.
    xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(2), 2), "Bookmark");
    xParameters = xFormField->getParameters();
    CPPUNIT_ASSERT_EQUAL(true, xParameters->getByName("Checkbox_Checked").get<bool>());
}

CPPUNIT_TEST_FIXTURE(Test, testN823655)
{
    createSwDoc("n823655.rtf");
    uno::Sequence<beans::PropertyValue> aProps
        = getProperty<uno::Sequence<beans::PropertyValue>>(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            aCoordinates
                = rProp.Value.get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>();
    }
    // The first coordinate pair of this freeform shape was 286,0 instead of 0,286.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(286), aCoordinates[0].Second.Value.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testN823675)
{
    createSwDoc("n823675.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    awt::FontDescriptor aFont;

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletFont")
            aFont = rProp.Value.get<awt::FontDescriptor>();
    }
    // This was empty, i.e. no font name was set for the bullet numbering.
    CPPUNIT_ASSERT_EQUAL(OUString("Symbol"), aFont.Name);
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshape)
{
    createSwDoc("groupshape.rtf");
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShapes> xGroupshape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshape_notext)
{
    createSwDoc("groupshape-notext.rtf");
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShapes> xGroupshape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo81033)
{
    createSwDoc("fdo81033.rtf");
    // Number of tabstops in the paragraph should be 2, was 3.
    uno::Sequence<style::TabStop> tabs(
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), tabs.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5808), tabs[0].Position);
    CPPUNIT_ASSERT_EQUAL(style::TabAlign_LEFT, tabs[0].Alignment);
    CPPUNIT_ASSERT_EQUAL(u' ', tabs[0].FillChar);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16002), tabs[1].Position);
    CPPUNIT_ASSERT_EQUAL(style::TabAlign_LEFT, tabs[1].Alignment);
    CPPUNIT_ASSERT_EQUAL(u'_', tabs[1].FillChar);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116269)
{
    createSwDoc("tdf116269.rtf");
    // This was 2540, implicit 0 left margin was ignored on import (inherited
    // value from list definition is repeated if it's not 0).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116265)
{
    createSwDoc("tdf116265.rtf");
    // This was -635, \fi as direct formatting has to be ignored due to
    // matching \fi in list definition (and with invalid level numbers).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));

    const auto& pLayout = parseLayoutDump();
    // Ensure that there is a tabstop in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo66565)
{
    createSwDoc("fdo66565.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A2 was 554, should be 453/14846*10000
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(304), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                             xTableRows->getByIndex(1), "TableColumnSeparators")[0]
                                             .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo67365)
{
    createSwDoc("fdo67365.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xRows = xTable->getRows();
    // The table only had 3 rows.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xRows->getCount());
    // This was 4999, i.e. the two cells of the row had equal widths instead of a larger and a smaller cell.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5290), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xRows->getByIndex(2), "TableColumnSeparators")[0]
                                              .Position);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    // Paragraph was aligned to center, should be left.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(
                             getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47440)
{
    createSwDoc("fdo47440.rtf");
    // Vertical and horizontal orientation of the picture wasn't imported (was text::RelOrientation::FRAME).
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo53556)
{
    createSwDoc("fdo53556.rtf");
    // This was drawing::FillStyle_SOLID, which resulted in being non-transparent, hiding text which would be visible.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(getShape(3), "FillStyle"));

    // This was a com.sun.star.drawing.CustomShape, which resulted in lack of word wrapping in the bugdoc.
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshapeRotation)
{
    createSwDoc("groupshape-rotation.rtf");
    // Rotation on groupshapes wasn't handled correctly, RotateAngle was 4500.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(315 * 100), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115153)
{
    createSwDoc("tdf115153.rtf");
    auto const xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE, getProperty<sal_Int16>(xShape, "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2533), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
    // VertOrient was wrong
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::NONE, getProperty<sal_Int16>(xShape, "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2131), getProperty<sal_Int32>(xShape, "VertOrientPosition"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo68291)
{
    createSwDoc("fdo68291.odt");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfimport/data/fdo68291-paste.rtf", "com.sun.star.comp.Writer.RtfFilter", xEnd);

    // This was "Standard", causing an unwanted page break on next paste.
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(getParagraph(1), uno::UNO_QUERY_THROW)
                             ->getPropertyValue("PageDescName"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105511)
{
    struct DefaultLocale : public comphelper::ConfigurationProperty<DefaultLocale, OUString>
    {
        static OUString path() { return "/org.openoffice.Office.Linguistic/General/DefaultLocale"; }
        ~DefaultLocale() = delete;
    };
    auto batch = comphelper::ConfigurationChanges::create();
    DefaultLocale::set("ru-RU", batch);
    batch->commit();
    createSwDoc("tdf105511.rtf");
    getParagraph(1, u"\u0418\u043C\u044F"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testContSectionPageBreak)
{
    createSwDoc("cont-section-pagebreak.rtf");
    uno::Reference<text::XTextRange> xParaSecond = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString("SECOND"), xParaSecond->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaSecond, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaSecond, uno::UNO_QUERY_THROW)
                             ->getPropertyValue("PageDescName"));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    // (could be either 1 or 2; in Word it's a 2-line paragraph with the 1st
    // line containing only the page break being ~0 height)
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaNext->getString());
    //If PageDescName is not empty, a page break / switch to page style is defined
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaNext, "PageDescName"));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("THIRD"), xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaThird, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaThird, uno::UNO_QUERY_THROW)
                             ->getPropertyValue("PageDescName"));
    // there is an empty paragraph after THIRD
    uno::Reference<text::XTextRange> xParaLast = getParagraph(5);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaLast->getString());
    try
    {
        getParagraph(6);
    }
    catch (container::NoSuchElementException const&)
    {
        // does not exist - expected
    }

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSectionPageBreak)
{
    createSwDoc("section-pagebreak.rtf");
    uno::Reference<text::XTextRange> xParaSecond = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString("SECOND"), xParaSecond->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaSecond, "BreakType"));
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaSecond, "PageDescName"));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    // (could be either 1 or 2; in Word it's a 2-line paragraph with the 1st
    // line containing only the page break being ~0 height)
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaNext->getString());
    //If PageDescName is not empty, a page break / switch to page style is defined
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE,
                         getProperty<style::BreakType>(xParaNext, "BreakType"));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("THIRD"), xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaThird, "BreakType"));
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaThird, "PageDescName"));
    // there is an empty paragraph after THIRD
    uno::Reference<text::XTextRange> xParaLast = getParagraph(5);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaLast->getString());
    try
    {
        getParagraph(6);
    }
    catch (container::NoSuchElementException const&)
    {
        // does not exist - expected
    }

    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testBackground)
{
    createSwDoc("background.rtf");
    // The first shape wasn't in the foreground.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "Opaque"));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), "Opaque"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo74823)
{
    createSwDoc("fdo74823.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of C2 was too large / column separator being 3749 too small (e.g. not set, around 3/7 of total width)
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5391), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xTableRows->getByIndex(1), "TableColumnSeparators")[2]
                                              .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo74599)
{
    createSwDoc("fdo74599.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Heading 3"), uno::UNO_QUERY);
    // Writer default styles weren't disabled, so the color was gray, not default (black).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         getProperty<sal_Int32>(xPropertySet, "CharColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testColumnBreak)
{
    createSwDoc("column-break.rtf");
    // Column break at the very start of the document was ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

CPPUNIT_TEST_FIXTURE(Test, testUnbalancedColumnsCompat)
{
    createSwDoc("unbalanced-columns-compat.rtf");
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was false, we ignored the relevant compat setting to make this non-last section unbalanced.
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

CPPUNIT_TEST_FIXTURE(Test, testOleInline)
{
    createSwDoc("ole-inline.rtf");
    // Problem was that inline shape had at-page anchor.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128611)
{
    createSwDoc("tdf128611.rtf");
    auto aPolyPolySequence
        = getProperty<uno::Sequence<uno::Sequence<awt::Point>>>(getShape(1), "PolyPolygon");
    CPPUNIT_ASSERT(aPolyPolySequence.hasElements());
    const uno::Sequence<awt::Point>& rPolygon = aPolyPolySequence[0];
    CPPUNIT_ASSERT_GREATER(static_cast<sal_uInt32>(1), rPolygon.size());
    sal_Int32 nY1 = rPolygon[0].Y;
    sal_Int32 nY2 = rPolygon[1].Y;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 6242
    // - Actual  : 3438
    // i.e. the vertical flip was missing, and the y1 > y2 assert failed, because the line pointed
    // from top left to bottom right, not bottom left to top right.
    CPPUNIT_ASSERT_GREATER(nY2, nY1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo80742)
{
    createSwDoc("fdo80742.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Heading 2"), uno::UNO_QUERY);
    // This was 0, outline level was body text.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xPropertySet, "OutlineLevel"));
}

CPPUNIT_TEST_FIXTURE(Test, testBehindDoc)
{
    createSwDoc("behind-doc.rtf");
    // The problem was that "behind doc" didn't result in the shape being in the background, only in being wrapped as "through".
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(xShape, "Surround"));
    // This was true.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xShape, "Opaque"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82114)
{
    createSwDoc("fdo82114.rtf");
    uno::Reference<text::XText> xHeaderTextPage1 = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "HeaderTextFirst");
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 1"), xHeaderTextPage1->getString());

    uno::Reference<text::XText> xHeaderTextPage2 = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Converted1"), "HeaderTextFirst");
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 2"), xHeaderTextPage2->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo44984)
{
    createSwDoc("fdo44984.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was Text, i.e. the checkbox field portion was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStartEnd"),
                         getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1),
                                               "TextPortionType"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131386)
{
    createSwDoc("hidden-para-separator.rtf");
    SwDoc const* const pDoc = getSwDoc();
    SwNodeIndex ix(pDoc->GetNodes().GetEndOfContent(), -1);
    CPPUNIT_ASSERT(ix.GetNode()
                       .GetTextNode()
                       ->GetAttr(RES_PARATR_LIST_AUTOFMT)
                       .GetStyleHandle()
                       ->Get(RES_CHRATR_HIDDEN)
                       .GetValue());
    --ix;
    --ix;
    CPPUNIT_ASSERT(ix.GetNode()
                       .GetTextNode()
                       ->GetAttr(RES_PARATR_LIST_AUTOFMT)
                       .GetStyleHandle()
                       ->Get(RES_CHRATR_HIDDEN)
                       .GetValue());
    --ix;
    CPPUNIT_ASSERT(ix.GetNode()
                       .GetTextNode()
                       ->GetAttr(RES_PARATR_LIST_AUTOFMT)
                       .GetStyleHandle()
                       ->Get(RES_CHRATR_HIDDEN)
                       .GetValue());
    --ix;
    CPPUNIT_ASSERT(ix.GetNode()
                       .GetTextNode()
                       ->GetAttr(RES_PARATR_LIST_AUTOFMT)
                       .GetStyleHandle()
                       ->Get(RES_CHRATR_HIDDEN)
                       .GetValue());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82071)
{
    createSwDoc("fdo82071.rtf");
    // The problem was that in TOC, chapter names were underlined, but they should not be.
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1);
    // Make sure we test the right text portion.
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 1"), xRun->getString());
    // This was awt::FontUnderline::SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::NONE, getProperty<sal_Int16>(xRun, "CharUnderline"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo85179)
{
    createSwDoc("fdo85179.rtf");
    // This was 0, border around the picture was ignored on import.
    // 360: EMU -> MM100
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(50800 / 360),
                         getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testPasteFirstParaDirectFormat)
{
    // Create a new document.
    createSwDoc();
    {
        // Set some direct formatting on the first paragraph, but leave paragraph adjust at its
        // default (left).
        uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
        xParagraph->setPropertyValue("PageNumberOffset", uno::Any(static_cast<sal_Int16>(0)));
    }

    // Paste from RTF.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<document::XDocumentInsertable> xCursor(
        xText->createTextCursorByRange(xText->getStart()), uno::UNO_QUERY);
    xCursor->insertDocumentFromURL(createFileURL(u"paste-first-para-direct-format.rtf"), {});

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3 (center)
    // - Actual  : 0 (left)
    // i.e. the inserted document's first paragraph's paragraph formatting was lost.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xParagraph, "ParaAdjust"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82512)
{
    createSwDoc("fdo82512.rtf");
    // This was style::BreakType_NONE, column break was before the 3rd paragraph, not before the 2nd one.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

CPPUNIT_TEST_FIXTURE(Test, testUnbalancedColumns)
{
    createSwDoc("unbalanced-columns.rtf");
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126173)
{
    createSwDoc("tdf126173.rtf");
    // Without the accompanying fix in place, this test would have failed, as the TextFrame was lost
    // on import.
    CPPUNIT_ASSERT(getShape(1).is());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo84685)
{
    createSwDoc("fdo84685.rtf");
    // index mark was not imported
    uno::Reference<text::XDocumentIndexMark> xMark(
        getProperty<uno::Reference<text::XDocumentIndexMark>>(getRun(getParagraph(1), 1),
                                                              "DocumentIndexMark"));
    CPPUNIT_ASSERT(xMark.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Key the 1st"), getProperty<OUString>(xMark, "PrimaryKey"));
    // let's test toc entry too
    uno::Reference<text::XDocumentIndexMark> xTOCMark(
        getProperty<uno::Reference<text::XDocumentIndexMark>>(getRun(getParagraph(2), 1),
                                                              "DocumentIndexMark"));
    CPPUNIT_ASSERT(xTOCMark.is());
    uno::Reference<lang::XServiceInfo> xTOCSI(xTOCMark, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTOCSI->supportsService("com.sun.star.text.ContentIndexMark"));
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 ).
CPPUNIT_TEST_FIXTURE(Test, testCp950listleveltext1)
{
    createSwDoc("cp950listleveltext1.rtf");
    // suffix with Chinese only ( most common case generated by MSO2010 TC)
    // This is a dot that is generally used as suffix of Chinese list number
    static constexpr OUStringLiteral aExpectedNumbering = u"\u4e00\u3001";

    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedNumbering),
                         getProperty<OUString>(xPara, "ListLabelString"));
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 ).
CPPUNIT_TEST_FIXTURE(Test, testCp950listleveltext2)
{
    createSwDoc("cp950listleveltext2.rtf");
    // Prefix and suffix with Chinese only ( tweaked from default in MSO2010 TC)
    static constexpr OUStringLiteral aExpectedNumbering = u"\u524d\u7f6e\u7532\u3001\u5f8c\u7f6e";

    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedNumbering),
                         getProperty<OUString>(xPara, "ListLabelString"));
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 )
CPPUNIT_TEST_FIXTURE(Test, testCp950listleveltext3)
{
    createSwDoc("cp950listleveltext3.rtf");
    // Numbering is a mix Chinese and English ( tweaked from default in MSO2010 TC)
    static constexpr OUStringLiteral aExpectedNumbering = u"\u524da\u7f6eb\u7532\u3001"
                                                          "c\u5f8cd\u7f6ee";

    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedNumbering),
                         getProperty<OUString>(xPara, "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testChtOutlineNumberingRtf)
{
    createSwDoc("chtoutline.rtf");

    static constexpr OUStringLiteral aExpectedNumbering = u"\u7b2c 1 \u7ae0";

    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedNumbering),
                         getProperty<OUString>(xPara, "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf90046)
{
    createSwDoc("tdf90046.rtf");
    // this was crashing on importing the footnote
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Ma"), xTextRange->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49893)
{
    createSwDoc("fdo49893.rtf");
    // Image from shape was not loaded, invalid size of image after load
    uno::Reference<drawing::XShape> xShape = getShape(2);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(432)), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(1296)), xShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49893_3)
{
    createSwDoc("fdo49893-3.rtf");
    // No artifacts (black lines in left top corner) as shape #3 are expected
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // Correct wrapping for shape
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(2), "Surround"));
}

CPPUNIT_TEST_FIXTURE(Test, testWrapDistance)
{
    createSwDoc("wrap-distance.rtf");
    // Custom shape, handled directly in RTFSdrImport.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.CustomShape"), xShape->getShapeType());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(xShape, "TopMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(xShape, "BottomMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3),
                         getProperty<sal_Int32>(xShape, "LeftMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         getProperty<sal_Int32>(xShape, "RightMargin") / 1000);

    // Picture, handled in GraphicImport, shared with DOCX.
    xShape = getShape(2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(xShape, "TopMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(xShape, "BottomMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3),
                         getProperty<sal_Int32>(xShape, "LeftMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         getProperty<sal_Int32>(xShape, "RightMargin") / 1000);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf90260Par)
{
    createSwDoc("hello.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfimport/data/tdf90260-par.rtf", "com.sun.star.comp.Writer.RtfFilter", xEnd);
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119599)
{
    createSwDoc("tdf119599.rtf");
    uno::Reference<beans::XPropertyState> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE, changing the font size in
    // the style had no effect on the rendering result.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, xRun->getPropertyState("CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf90315)
{
    createSwDoc("tdf90315.rtf");
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSection(xTextSections->getByIndex(0), uno::UNO_QUERY);
    // This was 0, but default should be 720 twips.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(1270),
        getProperty<sal_Int32>(xTextSection->getPropertyValue("TextColumns"), "AutomaticDistance"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf59454)
{
    createSwDoc("tdf59454.rtf");
    // This was 1, section break was ignored right before a table.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96308Tabpos)
{
    createSwDoc("tdf96308-tabpos.rtf");
    // Get the tab stops of the second para in the B1 cell of the first table in the document.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    auto aTabStops = getProperty<uno::Sequence<style::TabStop>>(xPara, "ParaTabStops");
    // This failed: tab stops were not deleted as direct formatting on the paragraph.
    CPPUNIT_ASSERT(!aTabStops.hasElements());
}

CPPUNIT_TEST_FIXTURE(Test, testLndscpsxn)
{
    createSwDoc("lndscpsxn.rtf");
    // Check landscape flag.
    CPPUNIT_ASSERT_EQUAL(4, getPages());

    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    // check that the first page has landscape flag
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> xStylePage(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the second page has no landscape flag
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the third page has landscape flag
    xCursor->jumpToPage(3);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the last page has no landscape flag
    xCursor->jumpToLastPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStylePage, "IsLandscape"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99498)
{
    createSwDoc("tdf99498.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    // Table width was a tiny sub one char wide 145twips, it should now be a table wide
    // enough to see all the text in the first column without breaking into multiple lines
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7056), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

CPPUNIT_TEST_FIXTURE(Test, testClassificatonPaste)
{
    createSwDoc("hello.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();

    // Not classified source, not classified destination: OK.
    paste(u"rtfimport/data/classification-no.rtf", "com.sun.star.comp.Writer.RtfFilter", xEnd);
    CPPUNIT_ASSERT_EQUAL(OUString("classification-no"), getParagraph(2)->getString());

    // Classified source, not classified destination: nothing should happen.
    OUString aOld = xText->getString();
    paste(u"rtfimport/data/classification-yes.rtf", "com.sun.star.comp.Writer.RtfFilter", xEnd);
    CPPUNIT_ASSERT_EQUAL(aOld, xText->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf90097)
{
    createSwDoc("tdf90097.rtf");
    // Get the second child of the group shape.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Sequence<uno::Sequence<awt::Point>> aPolyPolySequence;
    xShape->getPropertyValue("PolyPolygon") >>= aPolyPolySequence;
    const uno::Sequence<awt::Point>& rPolygon = aPolyPolySequence[0];

    // Vertical flip for the line shape was ignored, so Y coordinates were swapped.
    CPPUNIT_ASSERT(rPolygon[0].Y > rPolygon[1].Y);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf91684)
{
    createSwDoc("tdf91684.rtf");
    // Scaling of the group shape children were incorrect, this was 3203.
    // (Unit was assumed to be twips, but it was relative coordinates.)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1337), getShape(1)->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(Test, testFlip)
{
    createSwDoc("flip.rtf");
    comphelper::SequenceAsHashMap aMap = getProperty<uno::Sequence<beans::PropertyValue>>(
        getShapeByName(u"h-and-v"), "CustomShapeGeometry");
    // This resulted in a uno::RuntimeException, as MirroredX wasn't set at all, so could not extract void to boolean.
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredX"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredY"].get<bool>());

    aMap = getProperty<uno::Sequence<beans::PropertyValue>>(getShapeByName(u"h-only"),
                                                            "CustomShapeGeometry");
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredX"].get<bool>());
    CPPUNIT_ASSERT(!aMap["MirroredY"].hasValue());

    aMap = getProperty<uno::Sequence<beans::PropertyValue>>(getShapeByName(u"v-only"),
                                                            "CustomShapeGeometry");
    CPPUNIT_ASSERT(!aMap["MirroredX"].hasValue());
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredY"].get<bool>());

    aMap = getProperty<uno::Sequence<beans::PropertyValue>>(getShapeByName(u"neither-h-nor-v"),
                                                            "CustomShapeGeometry");
    CPPUNIT_ASSERT(!aMap["MirroredX"].hasValue());
    CPPUNIT_ASSERT(!aMap["MirroredY"].hasValue());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf78506)
{
    createSwDoc("tdf78506.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1sd level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            // This was '0', invalid \levelnumbers wasn't ignored.
            CPPUNIT_ASSERT(rProp.Value.get<OUString>().isEmpty());
    }

    const auto& pLayout = parseLayoutDump();
    // Ensure that there is a tabstop in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117403)
{
    createSwDoc("tdf117403.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 aExpected(static_cast<sal_Int32>(COL_BLACK), 0, 4, 0,
                                 table::BorderLineStyle::SOLID, 4);
    // This failed, border was not imported, OuterLineWidth was 0 instead of 4.
    CPPUNIT_ASSERT_BORDER_EQUAL(aExpected, getProperty<table::BorderLine2>(xCell, "BottomBorder"));
}

CPPUNIT_TEST_FIXTURE(Test, testImportHeaderFooter)
{
    createSwDoc("tdf108055.rtf");
    // The RTF import sometimes added Header and Footer multiple Times
    // as well as added the Header to the document body.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // Check if any Header or Footer text snuck into the TextBody
    uno::Reference<text::XTextRange> paragraph = getParagraph(1);
    OUString value = paragraph->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), value);

    paragraph = getParagraph(3);
    value = paragraph->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Second Page"), value);

    paragraph = getParagraph(5);
    value = paragraph->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Third Page"), value);

    //Check if Headers/Footers only contain what they should in this document
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "HeaderTextFirst");
    OUString aActual = xHeaderText->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("First Page Header"), aActual);

    uno::Reference<text::XText> xHeaderTextLeft = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "HeaderTextLeft");
    aActual = xHeaderTextLeft->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Header even"), aActual);

    uno::Reference<text::XText> xHeaderTextRight = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "HeaderTextRight");
    aActual = xHeaderTextRight->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Header uneven"), aActual);

    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "FooterTextFirst");
    aActual = xFooterText->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("First Page Footer"), aActual);

    uno::Reference<text::XText> xFooterTextLeft = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "FooterTextLeft");
    aActual = xFooterTextLeft->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Footer even"), aActual);

    uno::Reference<text::XText> xFooterTextRight = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "FooterTextRight");
    aActual = xFooterTextRight->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Footer uneven"), aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108947)
{
    createSwDoc("tdf108947.rtf");
    //Check if Headers/Footers contain what they should in this document
    uno::Reference<text::XText> xHeaderTextRight = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "HeaderTextRight");
    OUString aActual = xHeaderTextRight->getString();
    CPPUNIT_ASSERT_EQUAL(OUString(""), aActual);

    //Document is very fragile, hence we need this guard.
#if HAVE_MORE_FONTS
    uno::Reference<text::XText> xHeaderTextLeft = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Page Style"), "HeaderTextLeft");
    aActual = xHeaderTextLeft->getString();
    CPPUNIT_ASSERT_EQUAL(OUString(SAL_NEWLINE_STRING "Header Page 2 ?"), aActual);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testWatermark)
{
    createSwDoc("watermark.rtf");
    Size aExpectedSize(14965, 7482);
    uno::Reference<drawing::XShape> xShape = getShape(1);
    awt::Size aActualSize(xShape->getSize());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Width()), aActualSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Height()), aActualSize.Height);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104016)
{
    createSwDoc("tdf104016.rtf");
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE, leading to lack of
    // inheritance from numbering.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xParagraph->getPropertyState("ParaLeftMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115242)
{
    createSwDoc("tdf115242.rtf");
    // This was 0, overridden left margin was lost by too aggressive style
    // deduplication.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2787),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153196)
{
    createSwDoc("tdf153196.rtf");

    const auto& pLayout = parseLayoutDump();

    CPPUNIT_ASSERT_EQUAL(4, getPages());

    // TODO: Writer creates an empty page 1 here, which Word does not
    assertXPath(pLayout, "/root/page[1]/footer"_ostr, 0);
    assertXPath(pLayout, "/root/page[2]/footer"_ostr, 1);
    // the first page (2) has a page style applied, which has a follow page
    // style; the problem was that the follow page style had a footer.
    assertXPath(pLayout, "/root/page[3]/footer"_ostr, 0);
    assertXPath(pLayout, "/root/page[4]/footer"_ostr, 1);

    // TODO exporting this, wrongly produces "even" footer from stashed one
    // TODO importing that, wrongly creates a footer even without evenAndOddHeaders
}

CPPUNIT_TEST_FIXTURE(Test, testDefaultValues)
{
    createSwDoc("default-values.rtf");
    // tdf#105910: control words without values must be treated as having default values,
    // instead of being silently ignored

    uno::Reference<text::XTextRange> paragraph = getParagraph(1);

    uno::Reference<text::XTextRange> run = getRun(paragraph, 1, "scaleWidth50%");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 2, "scaleWidth100%");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 3, "fontSize25");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(25), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 4, "fontSize12");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 5, "textDown3pt");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-25), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 6, "textUp3pt");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 7, "expand1pt");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(20), o3tl::toTwips(getProperty<sal_Int16>(run, "CharKerning"),
                                                      o3tl::Length::mm100));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 8, "expand0pt");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 9, "expand1pt");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(20), o3tl::toTwips(getProperty<sal_Int16>(run, "CharKerning"),
                                                      o3tl::Length::mm100));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 10, "expand0pt");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 11, "colorBlack");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_BLACK), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 12, "colorRed");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTRED), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 13, "colorGreen");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTGREEN), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 14, "colorBlue");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTBLUE), getProperty<sal_Int32>(run, "CharColor"));

    run = getRun(paragraph, 15, "colorAuto");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(run, "CharScaleWidth"));
    CPPUNIT_ASSERT_EQUAL(double(12), getProperty<double>(run, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(run, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(run, "CharEscapementHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(run, "CharKerning"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), getProperty<sal_Int32>(run, "CharColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleBottomMargin)
{
    createSwDoc("para-style-bottom-margin.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), getProperty<sal_Int32>(xPropertySet, "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP,
                         getProperty<style::LineSpacing>(xPropertySet, "ParaLineSpacing").Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(115),
                         getProperty<style::LineSpacing>(xPropertySet, "ParaLineSpacing").Height);

    // The reason why this is 0 despite the default style containing \sa200
    // is that Word will actually interpret \basedonN
    // as "set style N and for every attribute of that style,
    // set an attribute with default value on the style"
    uno::Reference<beans::XPropertySet> xPropertySet1(
        getStyles("ParagraphStyles")->getByName("Contents 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet1, "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP,
                         getProperty<style::LineSpacing>(xPropertySet1, "ParaLineSpacing").Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<style::LineSpacing>(xPropertySet1, "ParaLineSpacing").Height);
    auto const xPara(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP, // 0 or 3 ???
                         getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
}

CPPUNIT_TEST_FIXTURE(Test, test158044Tdf)
{
    createSwDoc("tdf158044.rtf");

    {
        auto xPara(getParagraph(1));
        auto tabStops = getProperty<uno::Sequence<style::TabStop>>(xPara, "ParaTabStops");

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), tabStops.getLength());
    }

    {
        auto xPara(getParagraph(2));
        auto fillColor = getProperty<Color>(xPara, "FillColor");
        auto fillStyle = getProperty<drawing::FillStyle>(xPara, "FillStyle");

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, fillStyle);
        CPPUNIT_ASSERT_EQUAL(Color(0xffffff), fillColor);
    }

    {
        auto xPara(getParagraph(3));
        auto adjust = getProperty<sal_Int16>(xPara, "ParaAdjust");

        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), adjust);
    }

    {
        auto xPara(getParagraph(4));
        auto tabStops = getProperty<uno::Sequence<style::TabStop>>(xPara, "ParaTabStops");

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), tabStops.getLength());
    }

    {
        auto xPara(getParagraph(5));
        auto fillColor = getProperty<Color>(xPara, "FillColor");
        auto fillStyle = getProperty<drawing::FillStyle>(xPara, "FillStyle");
        auto tabStops = getProperty<uno::Sequence<style::TabStop>>(xPara, "ParaTabStops");

        CPPUNIT_ASSERT_LESS(sal_Int32(2), tabStops.getLength());
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, fillStyle);
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), fillColor);
    }

    {
        auto xPara(getParagraph(6));
        auto fillStyle = getProperty<drawing::FillStyle>(xPara, "FillStyle");
        auto tabStops = getProperty<uno::Sequence<style::TabStop>>(xPara, "ParaTabStops");

        CPPUNIT_ASSERT_LESS(sal_Int32(2), tabStops.getLength());
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, fillStyle);
    }
}
// tests should only be added to rtfIMPORT *if* they fail round-tripping in rtfEXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
