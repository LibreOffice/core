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

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/BreakType.hpp>
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

#include <rtl/ustring.hxx>
#include <vcl/settings.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/configuration.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace graphic
{
class XGraphic;
}
}
}
}

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfimport/data/", "Rich Text Format")
    {
    }

protected:
    AllSettings m_aSavedSettings;
};

DECLARE_RTFIMPORT_TEST(testN695479, "n695479.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Negative ABSH should mean fixed size.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xPropertySet, "SizeType"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(300)),
                         getProperty<sal_Int32>(xPropertySet, "Height"));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    bool bFrameFound = false, bDrawFound = false;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xDraws->getByIndex(i), uno::UNO_QUERY);
        if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        {
            // Both frames should be anchored to the first paragraph.
            bFrameFound = true;
            uno::Reference<text::XTextContent> xTextContent(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
            uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
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

DECLARE_RTFIMPORT_TEST(testTdf117246, "tdf117246.rtf")
{
    // This was 2, all but the last \page was lost.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_RTFIMPORT_TEST(testTdf108943, "tdf108943.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo46662, "fdo46662.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum3"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(1) >>= aProps; // 2nd level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "ParentNumbering")
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), rProp.Value.get<sal_Int16>());
        else if (rProp.Name == "Suffix")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<OUString>().getLength());
    }
}

DECLARE_RTFIMPORT_TEST(testTdf115715, "tdf115715.rtf")
{
    // This was 0, second paragraph was shifted to the right, it had the same
    // horizontal position as the 3rd paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1270),
                         getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

DECLARE_RTFIMPORT_TEST(testTdf81943, "tdf81943.rtf")
{
    // The shape wasn't in background.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_RTFIMPORT_TEST(testTdf115155, "tdf115155.rtf")
{
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules");
    // 1st level
    comphelper::SequenceAsHashMap aMap(xLevels->getByIndex(0));
    // This was 1270: the left margin in the numbering rule was too small.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2540), aMap["IndentAt"].get<sal_Int32>());
}

DECLARE_RTFIMPORT_TEST(testTdf108951, "tdf108951.rtf")
{
    // This test is import-only, as we assert the list ID, which is OK to
    // re-order on export, but then this test would not pass anymore.

    // \ls2 was imported as WWNum1.
    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
}

DECLARE_RTFIMPORT_TEST(testFdo47036, "fdo47036.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    int nAtCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        if (getProperty<text::TextContentAnchorType>(xDraws->getByIndex(i), "AnchorType")
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

DECLARE_RTFIMPORT_TEST(testFdo45182, "fdo45182.rtf")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // Encoding in the footnote was wrong.
    OUString aExpected(u"\u017Eivnost\u00ED" SAL_NEWLINE_STRING);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo85812, "fdo85812.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo49692, "fdo49692.rtf")
{
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
            OUString aExpected(u'\x200B');
            CPPUNIT_ASSERT_EQUAL(aExpected, rProp.Value.get<OUString>());
        }
    }
}

DECLARE_RTFIMPORT_TEST(testFdo45190, "fdo45190.rtf")
{
    // inherited \fi should be reset
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));

    // but direct one not
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-100)),
                         getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf59699, "tdf59699.rtf")
{
    // This resulted in a lang.IndexOutOfBoundsException: the referenced graphic data wasn't imported.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xImage, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_RTFIMPORT_TEST(testFdo52066, "fdo52066.rtf")
{
    /*
     * The problem was that the height of the shape was too big.
     *
     * xray ThisComponent.DrawPage(0).Size.Height
     */
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(19)), xShape->getSize().Height);
}

DECLARE_RTFIMPORT_TEST(testTdf112211_2, "tdf112211-2.rtf")
{
    // Spacing between the bullet and the actual text was too large.
    // This is now around 269, large old value was 629.
    int nWidth = parseDump("/root/page/body/txt[2]/Text[@nType='PortionType::TabLeft']", "nWidth")
                     .toInt32();
    CPPUNIT_ASSERT_LESS(300, nWidth);
}

DECLARE_RTFIMPORT_TEST(testTdf122430, "tdf122430.rtf")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 18, Actual  : 12', i.e. the font was smaller than expected.
    CPPUNIT_ASSERT_EQUAL(18.0f, getProperty<float>(getRun(getParagraph(1), 2), "CharHeight"));
}

DECLARE_RTFIMPORT_TEST(testFdo49892, "fdo49892.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        OUString aDescription = getProperty<OUString>(xDraws->getByIndex(i), "Description");
        if (aDescription == "red")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                                 getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "green")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                                 getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "blue")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                                 getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "rect")
        {
            CPPUNIT_ASSERT_EQUAL(
                text::RelOrientation::PAGE_FRAME,
                getProperty<sal_Int16>(xDraws->getByIndex(i), "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(
                text::RelOrientation::PAGE_FRAME,
                getProperty<sal_Int16>(xDraws->getByIndex(i), "VertOrientRelation"));
        }
    }
}

DECLARE_RTFIMPORT_TEST(testFdo52052, "fdo52052.rtf")
{
    // Make sure the textframe containing the text "third" appears on the 3rd page.
    CPPUNIT_ASSERT_EQUAL(OUString("third"),
                         parseDump("/root/page[3]/body/txt/anchored/fly/txt/text()"));
}

DECLARE_RTFIMPORT_TEST(testInk, "ink.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo79319, "fdo79319.rtf")
{
    // the thin horizontal rule was imported as a big fat rectangle
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xShape, "RelativeWidth"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(15238), xShape->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(53), xShape->getSize().Height, 10);
#if 0
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xShape, "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xShape, "HoriOrient"));
#endif
}

DECLARE_RTFIMPORT_TEST(testFdo48442, "fdo48442.rtf")
{
    // The problem was that \pvmrg is the default in RTF, but not in Writer.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, "VertOrientRelation")); // was FRAME
}

DECLARE_RTFIMPORT_TEST(testFdo55525, "fdo55525.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Negative left margin was ~missing, -191
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1877), getProperty<sal_Int32>(xTable, "LeftMargin"));
    // Cell width of A1 was 3332 (e.g. not set, 30% percent of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(896), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                             xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                                             .Position);
}

DECLARE_RTFIMPORT_TEST(testFdo57708, "fdo57708.rtf")
{
    // There were two issues: the doc was of 2 pages and the picture was missing.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // Two objects: a picture and a textframe.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo45183, "fdo45183.rtf")
{
    // Was text::WrapTextMode_PARALLEL, i.e. shpfblwtxt didn't send the shape below text.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(1), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Was 247, resulting in a table having width almost zero and height of 10+ pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16237), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_RTFIMPORT_TEST(testFdo59953, "fdo59953.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A1 was 4998 (e.g. not set / not wide enough, ~50% of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7649), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                                              .Position);
}

DECLARE_RTFIMPORT_TEST(testTdf84684, "tdf84684.rtf")
{
    // The ZOrder of the two children of the group shape were swapped.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xChild1(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was Pie 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Rectangle 1"), xChild1->getName());
    uno::Reference<container::XNamed> xChild2(xGroup->getByIndex(1), uno::UNO_QUERY);
    // This was Rectangle 1.
    CPPUNIT_ASSERT_EQUAL(OUString("Pie 2"), xChild2->getName());
}

DECLARE_RTFIMPORT_TEST(testFdo81944, "fdo81944.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testSbkodd, "sbkodd.rtf")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xCursor->getPage());
    xCursor->jumpToNextPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), xCursor->getPage());
}

DECLARE_RTFIMPORT_TEST(testSbkeven, "sbkeven.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testPoshPosv, "posh-posv.rtf")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                         getProperty<sal_Int16>(getShape(1), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER,
                         getProperty<sal_Int16>(getShape(1), "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "FrameIsAutomaticHeight"));
}

DECLARE_RTFIMPORT_TEST(testPoshLeftRight, "posh-leftright.rtf")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(getShape(1), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT,
                         getProperty<sal_Int16>(getShape(2), "HoriOrient"));
}

DECLARE_RTFIMPORT_TEST(testTdf96326, "tdf96326.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testN823655, "n823655.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testN823675, "n823675.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testGroupshape, "groupshape.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
    uno::Reference<drawing::XShapes> xGroupshape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

DECLARE_RTFIMPORT_TEST(testGroupshape_notext, "groupshape-notext.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
    uno::Reference<drawing::XShapes> xGroupshape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo81033, "fdo81033.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testTdf116269, "tdf116269.rtf")
{
    // This was 2540, implicit 0 left margin was ignored on import (inherited
    // value from list definition is repeated if it's not 0).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testTdf116265, "tdf116265.rtf")
{
    // This was -635, \fi as direct formatting has to be ignored due to
    // matching \fi in list definition (and with invalid level numbers).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

DECLARE_RTFIMPORT_TEST(testFdo66565, "fdo66565.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A2 was 554, should be 453/14846*10000
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(304), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                             xTableRows->getByIndex(1), "TableColumnSeparators")[0]
                                             .Position);
}

DECLARE_RTFIMPORT_TEST(testFdo67365, "fdo67365.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo47440, "fdo47440.rtf")
{
    // Vertical and horizontal orientation of the picture wasn't imported (was text::RelOrientation::FRAME).
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
}

DECLARE_RTFIMPORT_TEST(testFdo53556, "fdo53556.rtf")
{
    // This was drawing::FillStyle_SOLID, which resulted in being non-transparent, hiding text which would be visible.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(getShape(3), "FillStyle"));

    // This was a com.sun.star.drawing.CustomShape, which resulted in lack of word wrapping in the bugdoc.
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
}

DECLARE_RTFIMPORT_TEST(testGroupshapeRotation, "groupshape-rotation.rtf")
{
    // Rotation on groupshapes wasn't handled correctly, RotateAngle was 4500.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(315 * 100), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_RTFIMPORT_TEST(testTdf115153, "tdf115153.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo68291, "fdo68291.odt")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfimport/data/fdo68291-paste.rtf", xEnd);

    // This was "Standard", causing an unwanted page break on next paste.
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(getParagraph(1), uno::UNO_QUERY)
                             ->getPropertyValue("PageDescName"));
}

class testTdf105511 : public Test
{
protected:
    virtual OUString getTestName() override { return OUString("testTdf105511"); }

public:
    CPPUNIT_TEST_SUITE(testTdf105511);
    CPPUNIT_TEST(Import);
    CPPUNIT_TEST_SUITE_END();

    void Import()
    {
        struct DefaultLocale : public comphelper::ConfigurationProperty<DefaultLocale, OUString>
        {
            static OUString path()
            {
                return OUString("/org.openoffice.Office.Linguistic/General/DefaultLocale");
            }
            ~DefaultLocale() = delete;
        };
        auto batch = comphelper::ConfigurationChanges::create();
        DefaultLocale::set("ru-RU", batch);
        batch->commit();
        executeImportTest("tdf105511.rtf", nullptr);
    }
    virtual void verify() override
    {
        OUString aExpected(u"\u0418\u043C\u044F");
        getParagraph(1, aExpected);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(testTdf105511);

DECLARE_RTFIMPORT_TEST(testContSectionPageBreak, "cont-section-pagebreak.rtf")
{
    uno::Reference<text::XTextRange> xParaSecond = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString("SECOND"), xParaSecond->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaSecond, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaSecond, uno::UNO_QUERY)
                             ->getPropertyValue("PageDescName"));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaNext->getString());
    //If PageDescName is not empty, a page break / switch to page style is defined
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaNext, "PageDescName"));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("THIRD"), xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaThird, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(), uno::Reference<beans::XPropertySet>(xParaThird, uno::UNO_QUERY)
                                         ->getPropertyValue("PageDescName"));

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testBackground, "background.rtf")
{
    // The first shape wasn't in the foreground.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "Opaque"));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), "Opaque"));
}

DECLARE_RTFIMPORT_TEST(testFdo74823, "fdo74823.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of C2 was too large / column separator being 3749 too small (e.g. not set, around 3/7 of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5391), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xTableRows->getByIndex(1), "TableColumnSeparators")[2]
                                              .Position);
}

DECLARE_RTFIMPORT_TEST(testFdo74599, "fdo74599.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Heading 3"), uno::UNO_QUERY);
    // Writer default styles weren't disabled, so the color was gray, not default (black).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         getProperty<sal_Int32>(xPropertySet, "CharColor"));
}

DECLARE_RTFIMPORT_TEST(testColumnBreak, "column-break.rtf")
{
    // Column break at the very start of the document was ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_RTFIMPORT_TEST(testUnbalancedColumnsCompat, "unbalanced-columns-compat.rtf")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was false, we ignored the relevant compat setting to make this non-last section unbalanced.
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

DECLARE_RTFIMPORT_TEST(testOleInline, "ole-inline.rtf")
{
    // Problem was that inline shape had at-page anchor.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFIMPORT_TEST(testFdo80742, "fdo80742.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Heading 2"), uno::UNO_QUERY);
    // This was 0, outline level was body text.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xPropertySet, "OutlineLevel"));
}

DECLARE_RTFIMPORT_TEST(testBehindDoc, "behind-doc.rtf")
{
    // The problem was that "behind doc" didn't result in the shape being in the background, only in being wrapped as "through".
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(xShape, "Surround"));
    // This was true.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xShape, "Opaque"));
}

DECLARE_RTFIMPORT_TEST(testFdo82114, "fdo82114.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Converted1"), "HeaderText");
    OUString aActual = xHeaderText->getString();
    OUString const aExpected("First page header, section 2");
    // This was 'Right page header, section 1'.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFIMPORT_TEST(testFdo44984, "fdo44984.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo82071, "fdo82071.rtf")
{
    // The problem was that in TOC, chapter names were underlined, but they should not be.
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(2), 1);
    // Make sure we test the right text portion.
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 1"), xRun->getString());
    // This was awt::FontUnderline::SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::NONE, getProperty<sal_Int16>(xRun, "CharUnderline"));
}

DECLARE_RTFIMPORT_TEST(testFdo85179, "fdo85179.rtf")
{
    // This was 0, border around the picture was ignored on import.
    // 360: EMU -> MM100
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(50800 / 360),
                         getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_RTFIMPORT_TEST(testFdo82512, "fdo82512.rtf")
{
    // This was style::BreakType_NONE, column break was before the 3rd paragraph, not before the 2nd one.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_RTFIMPORT_TEST(testUnbalancedColumns, "unbalanced-columns.rtf")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

DECLARE_RTFIMPORT_TEST(testFdo84685, "fdo84685.rtf")
{
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
DECLARE_RTFIMPORT_TEST(testCp950listleveltext1, "cp950listleveltext1.rtf")
{
    // suffix with Chinese only ( most common case generated by MSO2010 TC)
    const sal_Unicode aExpectedSuffix[1]
        = { 0x3001 }; // This is a dot that is generally used as suffix of Chinese list number
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum3"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix, SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 ).
DECLARE_RTFIMPORT_TEST(testCp950listleveltext2, "cp950listleveltext2.rtf")
{
    // Prefix and suffix with Chinese only ( tweaked from default in MSO2010 TC)
    const sal_Unicode aExpectedPrefix[2] = { 0x524d, 0x7f6e };
    const sal_Unicode aExpectedSuffix[3] = { 0x3001, 0x5f8c, 0x7f6e };

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix, aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix, SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix, SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 )
DECLARE_RTFIMPORT_TEST(testCp950listleveltext3, "cp950listleveltext3.rtf")
{
    // Prefix and suffix that mix Chinese and English ( tweaked from default in MSO2010 TC)
    const sal_Unicode aExpectedPrefix[4] = { 0x524d, 0x0061, 0x7f6e, 0x0062 };
    const sal_Unicode aExpectedSuffix[6] = { 0x3001, 0x0063, 0x5f8c, 0x0064, 0x7f6e, 0x0065 };

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix, aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix, SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix, SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_RTFIMPORT_TEST(testChtOutlineNumberingRtf, "chtoutline.rtf")
{
    const sal_Unicode aExpectedPrefix[2] = { 0x7b2c, 0x0020 };
    const sal_Unicode aExpectedSuffix[2] = { 0x0020, 0x7ae0 };
    uno::Reference<text::XChapterNumberingSupplier> xChapterNumberingSupplier(mxComponent,
                                                                              uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xChapterNumberingSupplier->getChapterNumberingRules());
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix, aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix, SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix, SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_RTFIMPORT_TEST(testTdf90046, "tdf90046.rtf")
{
    // this was crashing on importing the footnote
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Ma"), xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo49893, "fdo49893.rtf")
{
    // Image from shape was not loaded, invalid size of image after load
    uno::Reference<drawing::XShape> xShape(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(432)), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(1296)), xShape->getSize().Width);
}

DECLARE_RTFIMPORT_TEST(testFdo49893_3, "fdo49893-3.rtf")
{
    // No artifacts (black lines in left top corner) as shape #3 are expected
    try
    {
        uno::Reference<drawing::XShape> xShape2(getShape(3), uno::UNO_QUERY);
        CPPUNIT_FAIL("exception expected: no shape #3 in document");
    }
    catch (lang::IndexOutOfBoundsException const&)
    {
        /* expected */
    }

    // Correct wrapping for shape
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(2), "Surround"));
}

DECLARE_RTFIMPORT_TEST(testWrapDistance, "wrap-distance.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testTdf90260Par, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfimport/data/tdf90260-par.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

DECLARE_RTFIMPORT_TEST(testTdf119599, "tdf119599.rtf")
{
    uno::Reference<beans::XPropertyState> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE, changing the font size in
    // the style had no effect on the rendering result.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, xRun->getPropertyState("CharHeight"));
}

DECLARE_RTFIMPORT_TEST(testTdf90315, "tdf90315.rtf")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSection(xTextSections->getByIndex(0), uno::UNO_QUERY);
    // This was 0, but default should be 720 twips.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(1270),
        getProperty<sal_Int32>(xTextSection->getPropertyValue("TextColumns"), "AutomaticDistance"));
}

DECLARE_RTFIMPORT_TEST(testTdf59454, "tdf59454.rtf")
{
    // This was 1, section break was ignored right before a table.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testTdf96308Tabpos, "tdf96308-tabpos.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testLndscpsxn, "lndscpsxn.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testTdf99498, "tdf99498.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    // Table width was a tiny sub one char wide 145twips, it should now be a table wide
    // enough to see all the text in the first column without breaking into multiple lines
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7056), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_RTFIMPORT_TEST(testClassificatonPaste, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();

    // Not classified source, not classified destination: OK.
    paste("rtfimport/data/classification-no.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(OUString("classification-no"), getParagraph(2)->getString());

    // Classified source, not classified destination: nothing should happen.
    OUString aOld = xText->getString();
    paste("rtfimport/data/classification-yes.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(aOld, xText->getString());
}

DECLARE_RTFIMPORT_TEST(testTdf90097, "tdf90097.rtf")
{
    // Get the second child of the group shape.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Sequence<uno::Sequence<awt::Point>> aPolyPolySequence;
    xShape->getPropertyValue("PolyPolygon") >>= aPolyPolySequence;
    uno::Sequence<awt::Point>& rPolygon = aPolyPolySequence[0];

    // Vertical flip for the line shape was ignored, so Y coordinates were swapped.
    CPPUNIT_ASSERT(rPolygon[0].Y > rPolygon[1].Y);
}

DECLARE_RTFIMPORT_TEST(testTdf91684, "tdf91684.rtf")
{
    // Scaling of the group shape children were incorrect, this was 3203.
    // (Unit was assumed to be twips, but it was relative coordinates.)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1337), getShape(1)->getSize().Height);
}

DECLARE_RTFIMPORT_TEST(testFlip, "flip.rtf")
{
    comphelper::SequenceAsHashMap aMap = getProperty<uno::Sequence<beans::PropertyValue>>(
        getShapeByName("h-and-v"), "CustomShapeGeometry");
    // This resulted in a uno::RuntimeException, as MirroredX wasn't set at all, so could not extract void to boolean.
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredX"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredY"].get<bool>());

    aMap = getProperty<uno::Sequence<beans::PropertyValue>>(getShapeByName("h-only"),
                                                            "CustomShapeGeometry");
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredX"].get<bool>());
    CPPUNIT_ASSERT(!aMap["MirroredY"].hasValue());

    aMap = getProperty<uno::Sequence<beans::PropertyValue>>(getShapeByName("v-only"),
                                                            "CustomShapeGeometry");
    CPPUNIT_ASSERT(!aMap["MirroredX"].hasValue());
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredY"].get<bool>());

    aMap = getProperty<uno::Sequence<beans::PropertyValue>>(getShapeByName("neither-h-nor-v"),
                                                            "CustomShapeGeometry");
    CPPUNIT_ASSERT(!aMap["MirroredX"].hasValue());
    CPPUNIT_ASSERT(!aMap["MirroredY"].hasValue());
}

DECLARE_RTFIMPORT_TEST(testTdf78506, "tdf78506.rtf")
{
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
            CPPUNIT_ASSERT_EQUAL(CHAR_ZWSP, rProp.Value.get<OUString>().toChar());
    }
}

DECLARE_RTFIMPORT_TEST(testTdf117403, "tdf117403.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testImportHeaderFooter, "tdf108055.rtf")
{
    // The RTF import sometimes added Header and Footer multiple Times
    // as well as added the Header to the document body.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // Check if any Header or Footer text snuck into the TextBody
    uno::Reference<text::XTextRange> paragraph = getParagraph(1);
    uno::Reference<text::XTextRange> text(paragraph, uno::UNO_QUERY);
    OUString value = text->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), value);

    paragraph = getParagraph(4);
    uno::Reference<text::XTextRange> text2(paragraph, uno::UNO_QUERY);
    value = text2->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Second Page"), value);

    paragraph = getParagraph(7);
    uno::Reference<text::XTextRange> text3(paragraph, uno::UNO_QUERY);
    value = text3->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Third Page"), value);

    //Check if Headers/Footers only contain what they should in this document
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("First Page"), "HeaderText");
    OUString aActual = xHeaderText->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("First Page Header"), aActual);

    uno::Reference<text::XText> xHeaderTextLeft = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Style"), "HeaderTextLeft");
    aActual = xHeaderTextLeft->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Header even"), aActual);

    uno::Reference<text::XText> xHeaderTextRight = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Style"), "HeaderTextRight");
    aActual = xHeaderTextRight->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Header uneven"), aActual);

    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("First Page"), "FooterText");
    aActual = xFooterText->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("First Page Footer"), aActual);

    uno::Reference<text::XText> xFooterTextLeft = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Style"), "FooterTextLeft");
    aActual = xFooterTextLeft->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Footer even"), aActual);

    uno::Reference<text::XText> xFooterTextRight = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Style"), "FooterTextRight");
    aActual = xFooterTextRight->getString();
    CPPUNIT_ASSERT_EQUAL(OUString("Footer uneven"), aActual);
}

DECLARE_RTFIMPORT_TEST(testTdf108947, "tdf108947.rtf")
{
    //Check if Headers/Footers contain what they should in this document
    uno::Reference<text::XText> xHeaderTextRight = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Style"), "HeaderTextRight");
    OUString aActual = xHeaderTextRight->getString();
    CPPUNIT_ASSERT_EQUAL(OUString(""), aActual);

    //Document is very fragile, hence we need this guard.
#if HAVE_MORE_FONTS
    OUString aExpected = SAL_NEWLINE_STRING;
    aExpected += SAL_NEWLINE_STRING;
    aExpected += "Header Page 2 ?";
    uno::Reference<text::XText> xHeaderTextLeft = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Default Style"), "HeaderTextLeft");
    aActual = xHeaderTextLeft->getString();
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
#endif
}

DECLARE_RTFIMPORT_TEST(testWatermark, "watermark.rtf")
{
    Size aExpectedSize(14965, 7482);
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Width()), aActualSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Height()), aActualSize.Height);
}

DECLARE_RTFIMPORT_TEST(testTdf104016, "tdf104016.rtf")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE, leading to lack of
    // inheritance from numbering.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xParagraph->getPropertyState("ParaLeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testTdf115242, "tdf115242.rtf")
{
    // This was 0, overridden left margin was lost by too aggressive style
    // deduplication.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2787),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testDefaultValues, "default-values.rtf")
{
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
    CPPUNIT_ASSERT_EQUAL(sal_Int64(20),
                         convertMm100ToTwip(getProperty<sal_Int16>(run, "CharKerning")));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int64(20),
                         convertMm100ToTwip(getProperty<sal_Int16>(run, "CharKerning")));
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

// tests should only be added to rtfIMPORT *if* they fail round-tripping in rtfEXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
