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
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <o3tl/cppunittraitshelper.hxx>
#include <tools/UnitConversion.hxx>
#include <rtl/ustring.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/configuration.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/rtfimport/data/"_ustr, u"Rich Text Format"_ustr)
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
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX,
                         getProperty<sal_Int16>(xPropertySet, u"SizeType"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(300)),
                         getProperty<sal_Int32>(xPropertySet, u"Height"_ustr));

    int nShapes = getShapes();
    CPPUNIT_ASSERT_EQUAL(3, nShapes);

    bool bFrameFound = false, bDrawFound = false;
    for (int i = 0; i < nShapes; ++i)
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(i + 1), uno::UNO_QUERY);
        if (xServiceInfo->supportsService(u"com.sun.star.text.TextFrame"_ustr))
        {
            // Both frames should be anchored to the first paragraph.
            bFrameFound = true;
            uno::Reference<text::XTextContent> xTextContent(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            uno::Reference<text::XText> xText = xRange->getText();
            CPPUNIT_ASSERT_EQUAL(u"plain"_ustr, xText->getString());

            if (i == 0)
                // Additionally, the first frame should have double border at the bottom.
                CPPUNIT_ASSERT_EQUAL(
                    table::BorderLineStyle::DOUBLE,
                    getProperty<table::BorderLine2>(xPropertySet, u"BottomBorder"_ustr).LineStyle);
        }
        else if (xServiceInfo->supportsService(u"com.sun.star.drawing.LineShape"_ustr))
        {
            // The older "drawing objects" syntax should be recognized.
            bDrawFound = true;
            xPropertySet.set(xServiceInfo, uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA,
                                 getProperty<sal_Int16>(xPropertySet, u"HoriOrientRelation"_ustr));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                                 getProperty<sal_Int16>(xPropertySet, u"VertOrientRelation"_ustr));
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
        getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
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
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(u"1.1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(u"1.1.1.1"_ustr,
                             getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }

    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0),
                             getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115715)
{
    createSwDoc("tdf115715.rtf");
    // This was 0, second paragraph was shifted to the right, it had the same
    // horizontal position as the 3rd paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1270),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaFirstLineIndent"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81943)
{
    createSwDoc("tdf81943.rtf");
    // The shape wasn't in background.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), u"Opaque"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115155)
{
    createSwDoc("tdf115155.rtf");
    auto xLevels = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2),
                                                                        u"NumberingRules"_ustr);
    // 1st level
    comphelper::SequenceAsHashMap aMap(xLevels->getByIndex(0));
    // This was 1270: the left margin in the numbering rule was too small.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2540), aMap[u"IndentAt"_ustr].get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108951)
{
    createSwDoc("tdf108951.rtf");
    // This test is import-only, as we assert the list ID, which is OK to
    // re-order on export, but then this test would not pass anymore.

    // \ls2 was imported as WWNum1.
    CPPUNIT_ASSERT_EQUAL(u"WWNum2"_ustr,
                         getProperty<OUString>(getParagraph(1), u"NumberingStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47036)
{
    createSwDoc("fdo47036.rtf");

    int nShapes = getShapes();
    CPPUNIT_ASSERT_EQUAL(3, nShapes);
    int nAtCharacter = 0;
    for (int i = 0; i < nShapes; ++i)
    {
        if (getProperty<text::TextContentAnchorType>(getShape(i + 1), u"AnchorType"_ustr)
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
        getProperty<lang::Locale>(getRun(getParagraph(1), 1, u"This "_ustr), u"CharLocale"_ustr));
    // the \lang inside the group was applied to CJK not Western
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    // further testing indicates that Word is doing really weird stuff
    // \loch \hich \dbch is reset by opening a group
    locale = getProperty<lang::Locale>(getRun(getParagraph(2), 1, u"CharGroup"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ru"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"RU"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(2), 2, u"AfterChar"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(3), 2, u"AfterBookmark"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 1, u"CharGroup"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ru"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"RU"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 1, u"CharGroup"_ustr),
                                       u"CharLocaleComplex"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"DZ"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 2, u"AfterChar"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 2, u"AfterChar"_ustr),
                                       u"CharLocaleComplex"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"DZ"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(5), 2, u"AfterBookmark"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(5), 2, u"AfterBookmark"_ustr),
                                       u"CharLocaleComplex"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"DZ"_ustr, locale.Country);
    // \ltrch \rtlch works differently - it is sticky across groups
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 1, u"CharGroup"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 1, u"CharGroup"_ustr),
                                       u"CharLocaleComplex"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"DZ"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 2, u"AfterChar"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 2, u"AfterChar"_ustr),
                                       u"CharLocaleComplex"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"EG"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(7), 2, u"AfterBookmark"_ustr),
                                       u"CharLocale"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(7), 2, u"AfterBookmark"_ustr),
                                       u"CharLocaleComplex"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, locale.Language);
    CPPUNIT_ASSERT_EQUAL(u"EG"_ustr, locale.Country);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49692)
{
    createSwDoc("fdo49692.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
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
                         getProperty<sal_Int32>(getParagraph(1), u"ParaFirstLineIndent"_ustr));

    // but direct one not
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-100)),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaFirstLineIndent"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf59699)
{
    createSwDoc("tdf59699.rtf");
    // This resulted in a lang.IndexOutOfBoundsException: the referenced graphic data wasn't imported.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xImage, u"Graphic"_ustr);
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    int nWidth = getXPath(pXmlDoc,
                          "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/"
                          "child::*[@type='PortionType::TabLeft']",
                          "width")
                     .toInt32();
    CPPUNIT_ASSERT_LESS(300, nWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122430)
{
    createSwDoc("tdf122430.rtf");
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 18, Actual  : 12', i.e. the font was smaller than expected.
    CPPUNIT_ASSERT_EQUAL(18.0f, getProperty<float>(getRun(getParagraph(1), 2), u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49892)
{
    createSwDoc("fdo49892.rtf");
    int nShapes = getShapes();
    CPPUNIT_ASSERT_EQUAL(5, nShapes);
    for (int i = 0; i < nShapes; ++i)
    {
        OUString aDescription = getProperty<OUString>(getShape(i + 1), u"Description"_ustr);
        if (aDescription == "red")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                                 getProperty<sal_Int32>(getShape(i + 1), u"ZOrder"_ustr));
        else if (aDescription == "green")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                                 getProperty<sal_Int32>(getShape(i + 1), u"ZOrder"_ustr));
        else if (aDescription == "blue")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                                 getProperty<sal_Int32>(getShape(i + 1), u"ZOrder"_ustr));
        else if (aDescription == "rect")
        {
            CPPUNIT_ASSERT_EQUAL(
                text::RelOrientation::PAGE_FRAME,
                getProperty<sal_Int16>(getShape(i + 1), u"HoriOrientRelation"_ustr));
            CPPUNIT_ASSERT_EQUAL(
                text::RelOrientation::PAGE_FRAME,
                getProperty<sal_Int16>(getShape(i + 1), u"VertOrientRelation"_ustr));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52052)
{
    createSwDoc("fdo52052.rtf");
    // Make sure the textframe containing the text "third" appears on the 3rd page.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPathContent(pXmlDoc, "/root/page[3]/body/txt/anchored/fly/txt/text()", u"third");
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
    uno::Sequence<beans::PropertyValue> aProps = getProperty<uno::Sequence<beans::PropertyValue>>(
        getShape(1), u"CustomShapeGeometry"_ustr);
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
                         getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo79319)
{
    createSwDoc("fdo79319.rtf");
    // the thin horizontal rule was imported as a big fat rectangle
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xShape, u"RelativeWidth"_ustr));
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
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr)); // was FRAME
}

CPPUNIT_TEST_FIXTURE(Test, testFdo55525)
{
    createSwDoc("fdo55525.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Negative left margin was ~missing, -191
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1877), getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));
    // Cell width of A1 was 3332 (e.g. not set, 30% percent of total width)
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(896),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)[0]
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
                         getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Was 247, resulting in a table having width almost zero and height of 10+ pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16237),
                         getProperty<sal_Int32>(xTables->getByIndex(0), u"Width"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7649),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)[0]
                             .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf84684)
{
    createSwDoc("tdf84684.rtf");
    // The ZOrder of the two children of the group shape were swapped.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xChild1(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was Pie 2.
    CPPUNIT_ASSERT_EQUAL(u"Rectangle 1"_ustr, xChild1->getName());
    uno::Reference<container::XNamed> xChild2(xGroup->getByIndex(1), uno::UNO_QUERY);
    // This was Rectangle 1.
    CPPUNIT_ASSERT_EQUAL(u"Pie 2"_ustr, xChild2->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo81944)
{
    createSwDoc("fdo81944.rtf");
    // font properties in style were not imported
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Segoe UI"_ustr, getProperty<OUString>(xStyle, u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(9.0f, getProperty<float>(xStyle, u"CharHeight"_ustr));
    // not sure if this should be set on Asian or Complex or both?
    CPPUNIT_ASSERT_EQUAL(u"Times New Roman"_ustr,
                         getProperty<OUString>(xStyle, u"CharFontNameComplex"_ustr));
    CPPUNIT_ASSERT_EQUAL(11.0f, getProperty<float>(xStyle, u"CharHeightComplex"_ustr));
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
                         getProperty<sal_Int16>(getShape(1), u"HoriOrient"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER,
                         getProperty<sal_Int16>(getShape(1), u"VertOrient"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"FrameIsAutomaticHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testPoshLeftRight)
{
    createSwDoc("posh-leftright.rtf");
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrient"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT,
                         getProperty<sal_Int16>(getShape(2), u"HoriOrient"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96326)
{
    createSwDoc("tdf96326.rtf");
    // Make sure this is not checked.
    auto xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(1), 2),
                                                                    u"Bookmark"_ustr);
    uno::Reference<container::XNameContainer> xParameters = xFormField->getParameters();
    // This was true, ffres=25 was interpreted as checked.
    CPPUNIT_ASSERT_EQUAL(false, bool(xParameters->hasElements()));

    // And this is checked.
    xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(2), 2),
                                                               u"Bookmark"_ustr);
    xParameters = xFormField->getParameters();
    CPPUNIT_ASSERT_EQUAL(true, xParameters->getByName(u"Checkbox_Checked"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(Test, testN823655)
{
    createSwDoc("n823655.rtf");
    uno::Sequence<beans::PropertyValue> aProps = getProperty<uno::Sequence<beans::PropertyValue>>(
        getShape(1), u"CustomShapeGeometry"_ustr);
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
        getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(u"Symbol"_ustr, aFont.Name);
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
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), u"ParaTabStops"_ustr));
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
                         getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116265)
{
    createSwDoc("tdf116265.rtf");
    // This was -635, \fi as direct formatting has to be ignored due to
    // matching \fi in list definition (and with invalid level numbers).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaFirstLineIndent"_ustr));

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Ensure that there is a tabstop in the pseudo-numbering (numbering::NONE followed by tabstop)
    assertXPath(pLayout, "//SwFixPortion", 1);
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(304),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), u"TableColumnSeparators"_ustr)[0]
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5290),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xRows->getByIndex(2), u"TableColumnSeparators"_ustr)[0]
                             .Position);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    // Paragraph was aligned to center, should be left.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(
                             getParagraphOfText(1, xCell->getText()), u"ParaAdjust"_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47440)
{
    createSwDoc("fdo47440.rtf");
    // Vertical and horizontal orientation of the picture wasn't imported (was text::RelOrientation::FRAME).
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo53556)
{
    createSwDoc("fdo53556.rtf");
    // This was drawing::FillStyle_SOLID, which resulted in being non-transparent, hiding text which would be visible.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(getShape(3), u"FillStyle"_ustr));

    // This was a com.sun.star.drawing.CustomShape, which resulted in lack of word wrapping in the bugdoc.
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"FrameShape"_ustr, xShapeDescriptor->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testGroupshapeRotation)
{
    createSwDoc("groupshape-rotation.rtf");
    // Rotation on groupshapes wasn't handled correctly, RotateAngle was 4500.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(315 * 100),
                         getProperty<sal_Int32>(getShape(1), u"RotateAngle"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115153)
{
    createSwDoc("tdf115153.rtf");
    auto const xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE,
                         getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2533),
                         getProperty<sal_Int32>(xShape, u"HoriOrientPosition"_ustr));
    // VertOrient was wrong
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::NONE,
                         getProperty<sal_Int16>(xShape, u"VertOrient"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2131),
                         getProperty<sal_Int32>(xShape, u"VertOrientPosition"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo68291)
{
    createSwDoc("fdo68291.odt");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfimport/data/fdo68291-paste.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);

    // This was "Standard", causing an unwanted page break on next paste.
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(getParagraph(1), uno::UNO_QUERY_THROW)
                             ->getPropertyValue(u"PageDescName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105511)
{
    struct DefaultLocale : public comphelper::ConfigurationProperty<DefaultLocale, OUString>
    {
        static OUString path()
        {
            return u"/org.openoffice.Office.Linguistic/General/DefaultLocale"_ustr;
        }
        ~DefaultLocale() = delete;
    };
    auto batch = comphelper::ConfigurationChanges::create();
    DefaultLocale::set(u"ru-RU"_ustr, batch);
    batch->commit();
    createSwDoc("tdf105511.rtf");
    getParagraph(1, u"\u0418\u043C\u044F"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testContSectionPageBreak)
{
    createSwDoc("cont-section-pagebreak.rtf");
    uno::Reference<text::XTextRange> xParaSecond = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(u"SECOND"_ustr, xParaSecond->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaSecond, u"BreakType"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaSecond, uno::UNO_QUERY_THROW)
                             ->getPropertyValue(u"PageDescName"_ustr));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    // (could be either 1 or 2; in Word it's a 2-line paragraph with the 1st
    // line containing only the page break being ~0 height)
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaNext->getString());
    //If PageDescName is not empty, a page break / switch to page style is defined
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaNext, u"PageDescName"_ustr));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(u"THIRD"_ustr, xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaThird, u"BreakType"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaThird, uno::UNO_QUERY_THROW)
                             ->getPropertyValue(u"PageDescName"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(u"SECOND"_ustr, xParaSecond->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaSecond, u"BreakType"_ustr));
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaSecond, u"PageDescName"_ustr));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    // (could be either 1 or 2; in Word it's a 2-line paragraph with the 1st
    // line containing only the page break being ~0 height)
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(), xParaNext->getString());
    //If PageDescName is not empty, a page break / switch to page style is defined
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE,
                         getProperty<style::BreakType>(xParaNext, u"BreakType"_ustr));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(u"THIRD"_ustr, xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaThird, u"BreakType"_ustr));
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaThird, u"PageDescName"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"Opaque"_ustr));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), u"Opaque"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5391),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), u"TableColumnSeparators"_ustr)[2]
                             .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo74599)
{
    createSwDoc("fdo74599.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 3"_ustr), uno::UNO_QUERY);
    // Writer default styles weren't disabled, so the color was gray, not default (black).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         getProperty<sal_Int32>(xPropertySet, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testColumnBreak)
{
    createSwDoc("column-break.rtf");
    // Column break at the very start of the document was ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(getParagraph(2), u"BreakType"_ustr));
}

// tests should only be added to rtfIMPORT *if* they fail round-tripping in rtfEXPORT
} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
