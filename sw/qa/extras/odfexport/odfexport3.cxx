/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/color.hxx>
#include <memory>
#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <sortedobjs.hxx>
#include <flyfrm.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/odfexport/data/"_ustr, u"writer8"_ustr)
    {
    }
};

DECLARE_ODFEXPORT_TEST(testTdf111891_frameVertStyle, "tdf111891_frameVertStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        drawing::TextVerticalAdjust_BOTTOM,
        getProperty<drawing::TextVerticalAdjust>(xFrame, u"TextVerticalAdjust"_ustr));
}

DECLARE_ODFEXPORT_TEST(testShapeRelsize, "shape-relsize.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // These were all 0, as style:rel-width/height was ignored on import for shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(40), getProperty<sal_Int16>(xShape, u"RelativeWidth"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, u"RelativeHeight"_ustr));

    // Relation was "page" for both width and height, should be "paragraph" for width.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, u"RelativeWidthRelation"_ustr));
    // And make sure that height stays "page".
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, u"RelativeHeightRelation"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTextboxRoundedCorners)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<drawing::XShape> xShape = getShape(1);
        comphelper::SequenceAsHashMap aCustomShapeGeometry(
            getProperty<uno::Sequence<beans::PropertyValue>>(xShape, u"CustomShapeGeometry"_ustr));

        // Test that the shape is a rounded rectangle.
        CPPUNIT_ASSERT_EQUAL(u"round-rectangle"_ustr,
                             aCustomShapeGeometry[u"Type"_ustr].get<OUString>());

        // The shape text should start with a table, with "a" in its A1 cell.
        uno::Reference<text::XText> xText
            = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1, xText), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, xCell->getString());
    };

    createSwDoc("textbox-rounded-corners.odt");
    verify();
    saveAndReload(mpFilter);
    verify();

    // Table inside a textbox should be in the extension namespace.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // This failed, as draw:custom-shape had a table:table child.
    assertXPath(pXmlDoc, "//draw:custom-shape/loext:table", "name", u"Table1");
}

// test that import whitespace collapsing is compatible with old docs
DECLARE_ODFEXPORT_TEST(testWhitespace, "whitespace.odt")
{
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XEnumerationAccess> xPara;
    uno::Reference<container::XEnumeration> xPortions;
    uno::Reference<text::XTextRange> xPortion;
    xPara.set(getParagraphOrTable(1), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(2), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    CPPUNIT_ASSERT_EQUAL(u"http://example.com/"_ustr,
                         getProperty<OUString>(xPortion, u"HyperLinkURL"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(3), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Ruby"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), xPortion->getString());
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, getProperty<OUString>(xPortion, u"RubyText"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Ruby"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(4), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"InContentMetadata"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    {
        // what a stupid idea to require recursively enumerating this
        uno::Reference<container::XEnumerationAccess> xMeta(
            getProperty<uno::Reference<text::XTextContent>>(xPortion, u"InContentMetadata"_ustr),
            uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xMetaPortions = xMeta->createEnumeration();
        uno::Reference<text::XTextRange> xMP(xMetaPortions->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xMP, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u" "_ustr, xMP->getString());
        CPPUNIT_ASSERT(!xMetaPortions->hasMoreElements());
    }
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(5), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"TextField"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    {
        // what a stupid idea to require recursively enumerating this
        uno::Reference<container::XEnumerationAccess> xMeta(
            getProperty<uno::Reference<text::XTextContent>>(xPortion, u"TextField"_ustr),
            uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xMetaPortions = xMeta->createEnumeration();
        uno::Reference<text::XTextRange> xMP(xMetaPortions->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xMP, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u" "_ustr, xMP->getString());
        CPPUNIT_ASSERT(!xMetaPortions->hasMoreElements());
    }
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(7), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(8), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(9), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(10), uno::UNO_QUERY);
    uno::Reference<container::XContentEnumerationAccess> xCEA(xPara, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xFrames(
        xCEA->createContentEnumeration(u"com.sun.star.text.TextContent"_ustr));
    xFrames->nextElement(); // one at-paragraph frame
    CPPUNIT_ASSERT(!xFrames->hasMoreElements());
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(11), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Footnote"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(12), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"TextField"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(13), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(15), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(16), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(17), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(18), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(19), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"ReferenceMark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(20), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"ReferenceMark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"ReferenceMark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(21), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"DocumentIndexMark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());

    xPara.set(getParagraphOrTable(22), uno::UNO_QUERY);
    xPortions.set(xPara->createEnumeration());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"X "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"DocumentIndexMark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, xPortion->getString());
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"DocumentIndexMark"_ustr,
                         getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(!getProperty<bool>(xPortion, u"IsCollapsed"_ustr));
    xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xPortion, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" X"_ustr, xPortion->getString());
    CPPUNIT_ASSERT(!xPortions->hasMoreElements());
}

DECLARE_ODFEXPORT_TEST(testTdf136645, "tdf136645.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the fix in place, this would have failed with
    //- Expected: 2640
    //- Actual  : 3000
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(2640),
        getXPath(pXmlDoc, "/root/page/body/section/column[1]/body/infos/bounds", "width")
            .toInt32());

    //- Expected: 6000
    //- Actual  : 6360
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(6000),
        getXPath(pXmlDoc, "/root/page/body/section/column[2]/body/infos/bounds", "width")
            .toInt32());
}

DECLARE_ODFEXPORT_TEST(testBtlrCell, "btlr-cell.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Without the accompanying fix in place, this test would have failed, as
    // the btlr text direction in the A1 cell was lost on ODF import and
    // export.
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName(u"Table1"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR,
                         getProperty<sal_Int16>(xA1, u"WritingMode"_ustr));

    uno::Reference<beans::XPropertySet> xB1(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xB1, u"WritingMode"_ustr);
    CPPUNIT_ASSERT(nActual == text::WritingMode2::LR_TB || nActual == text::WritingMode2::CONTEXT);

    uno::Reference<beans::XPropertySet> xC1(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL,
                         getProperty<sal_Int16>(xC1, u"WritingMode"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrFrame)
{
    auto verify = [this](bool bIsExport = false) {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        // Without the accompanying fix in place, this test would have failed, as
        // the btlr text direction in the text frame was lost on ODF import and
        // export.
        uno::Reference<beans::XPropertySet> xTextFrame(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xTextFrame.is());

        auto nActual = getProperty<sal_Int16>(xTextFrame, u"WritingMode"_ustr);
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, nActual);

        // Without the accompanying fix in place, this test would have failed, as the fly frame had
        // mbVertical==true, but mbVertLRBT==false, even if the writing direction in the doc model was
        // btlr.
        SwDoc* pDoc = getSwDoc();

        SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        CPPUNIT_ASSERT(pLayout);

        SwFrame* pPageFrame = pLayout->GetLower();
        CPPUNIT_ASSERT(pPageFrame);
        CPPUNIT_ASSERT(pPageFrame->IsPageFrame());

        SwFrame* pBodyFrame = pPageFrame->GetLower();
        CPPUNIT_ASSERT(pBodyFrame);
        CPPUNIT_ASSERT(pBodyFrame->IsBodyFrame());

        SwFrame* pBodyTextFrame = pBodyFrame->GetLower();
        CPPUNIT_ASSERT(pBodyTextFrame);
        CPPUNIT_ASSERT(pBodyTextFrame->IsTextFrame());

        CPPUNIT_ASSERT(pBodyTextFrame->GetDrawObjs());
        const SwSortedObjs& rAnchored = *pBodyTextFrame->GetDrawObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rAnchored.size());

        auto* pFlyFrame = dynamic_cast<SwFlyFrame*>(rAnchored[0]);
        CPPUNIT_ASSERT(pFlyFrame);
        CPPUNIT_ASSERT(pFlyFrame->IsVertLRBT());

        if (!bIsExport)
            // Not yet exported, don't modify the doc model for test purposes.
            return;

        // Make sure that btlr -> tbrl transition clears the "BT" flag.
        xTextFrame->setPropertyValue(u"WritingMode"_ustr, uno::Any(text::WritingMode2::TB_LR));
        pFlyFrame = dynamic_cast<SwFlyFrame*>(rAnchored[0]);
        CPPUNIT_ASSERT(pFlyFrame);
        CPPUNIT_ASSERT(!pFlyFrame->IsVertLRBT());
    };

    createSwDoc("btlr-frame.odt");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129520)
{
    loadAndReload("tdf129520.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(u"M"_ustr, getParagraph(1)->getString());

    // Without this fix in place, this test would have failed with
    // - Expected: Ma
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"Ma"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"1815"_ustr, getParagraph(3)->getString());
}

DECLARE_ODFEXPORT_TEST(testFdo86963, "fdo86963.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Export of this document failed with beans::UnknownPropertyException.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

DECLARE_ODFEXPORT_TEST(testTdf135338_firstLeftPageFooter, "tdf135338_firstLeftPageFooter.odt")
{
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    // The first page is a left page only style, but it should still show the first page footer
    // instead of the left footer text "EVEN/LEFT (Left page only)"
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPathContent(pXmlDoc, "/root/page[2]/footer/txt/text()", u"First (Left page only)");
}

DECLARE_ODFEXPORT_TEST(testGerrit13858, "gerrit13858.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Just make sure the output is valid.
}
DECLARE_ODFEXPORT_TEST(testOdtBorderTypes, "border_types.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    static const sal_Int32 lineStyles[] = { 0, 1, 2, 14, 16, 17, 3, 15 };
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(),
                                                                  uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            if (xServiceInfo->supportsService(u"com.sun.star.text.TextTable"_ustr))
            {
                uno::Reference<table::XCellRange> const xCellRange(xServiceInfo,
                                                                   uno::UNO_QUERY_THROW);

                for (sal_Int32 row = 0; row < 15; row += 2)
                {
                    uno::Reference<table::XCell> xCell = xCellRange->getCellByPosition(1, row);
                    uno::Reference<beans::XPropertySet> xPropSet(xCell, uno::UNO_QUERY_THROW);

                    uno::Any aTopBorder = xPropSet->getPropertyValue(u"TopBorder"_ustr);
                    table::BorderLine2 aTopBorderLine;
                    if (aTopBorder >>= aTopBorderLine)
                    {
                        sal_Int32 lineStyle = aTopBorderLine.LineStyle;
                        CPPUNIT_ASSERT_EQUAL(lineStyles[row / 2], lineStyle);
                    }
                } //end of the 'for' loop
            }
        }
    } while (xParaEnum->hasMoreElements());
}

DECLARE_ODFEXPORT_TEST(testMasterPageWithDrawingPage, "sw_hatch.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> xStyles(getStyles(u"PageStyles"_ustr));
    uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH,
                         getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Blue -45 Degrees"_ustr,
                         getProperty<OUString>(xStyle, u"FillHatchName"_ustr));
    CPPUNIT_ASSERT(!getProperty<sal_Bool>(xStyle, u"FillBackground"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xStyle, u"FillTransparence"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testPageStyleBackgroundFullSizeOOo)
{
    loadAndReload("pagestyle_background_ooo33.odt");
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Standard
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-color", u"#99ccff");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
    // Endnote
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"repeat");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Footnote
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"stretch");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
}

CPPUNIT_TEST_FIXTURE(Test, testPageStyleBackgroundFullSizeLO64)
{
    loadAndReload("pagestyle_background_lo64.odt");
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Standard
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-color", u"#99ccff");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
    // Endnote
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"repeat");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Footnote
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"stretch");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Landscape
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"no-repeat");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Index
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"gradient");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "gradient-step-count", u"0");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
    // First Page
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"hatch");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-hatch-solid", u"false");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
}

CPPUNIT_TEST_FIXTURE(Test, testPageStyleBackgroundFullSizeLO70)
{
    loadAndReload("pagestyle_background_lo70.odt");
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Standard
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-color", u"#99ccff");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Standard']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
    // Endnote
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"repeat");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Endnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Footnote
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"stretch");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Footnote']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Landscape
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "repeat", u"no-repeat");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Landscape']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-image-ref-point", u"top-left");
    // Index
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"gradient");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "gradient-step-count", u"0");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Index']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
    // First Page
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill", u"hatch");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "fill-hatch-solid", u"false");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='First_20_Page']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "opacity", u"100%");
}

CPPUNIT_TEST_FIXTURE(Test, testFillBitmapUnused)
{
    loadAndReload("fillbitmap3.odt");
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // nav_5f_home and all its references are completely gone
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);

    // paragraph style
    assertXPath(pXmlDoc,
                "/office:document-styles/office:styles/style:style[@style:name='Text_20_body']/"
                "loext:graphic-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:styles/style:style[@style:name='Text_20_body']/"
                "loext:graphic-properties",
                "fill-color", u"#c0c0c0");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:styles/style:style[@style:name='Text_20_body']/"
                "loext:graphic-properties[@draw:fill-image-name]",
                0);

    // page style page-layout
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm1']/style:page-layout-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm1']/style:page-layout-properties",
                "fill-image-name", u"nav_5f_up");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm1']/style:header-style/"
                "style:header-footer-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm1']/style:header-style/"
                "style:header-footer-properties",
                "fill-image-name", u"nav_5f_up");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm1']/style:footer-style/"
                "style:header-footer-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm1']/style:footer-style/"
                "style:header-footer-properties",
                "fill-image-name", u"nav_5f_up");

    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm2']/style:page-layout-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm2']/"
                "style:page-layout-properties[@draw:fill-image-name]",
                0);
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm2']/style:header-style/"
                "style:header-footer-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm2']/style:header-style/"
                "style:header-footer-properties[@draw:fill-image-name]",
                0);
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm2']/style:footer-style/"
                "style:header-footer-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:page-layout[@style:name='Mpm2']/style:footer-style/"
                "style:header-footer-properties[@draw:fill-image-name]",
                0);

    // page style drawing-page
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp1']/"
                "style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp1']/"
                "style:drawing-page-properties",
                "fill-image-name", u"nav_5f_up");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties",
                "fill", u"solid");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties",
                "fill-color", u"#c0c0c0");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties[@draw:fill-image-name]",
                0);

    // the named items
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/draw:fill-image", 1);
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/draw:fill-image", "name",
                u"nav_5f_up");
}

DECLARE_ODFEXPORT_TEST(testCellUserDefineAttr, "userdefattr-tablecell.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<table::XCell> const xCellA1(xTable->getCellByName(u"A1"_ustr),
                                               uno::UNO_SET_THROW);
    uno::Reference<table::XCell> const xCellB1(xTable->getCellByName(u"B1"_ustr),
                                               uno::UNO_SET_THROW);
    uno::Reference<table::XCell> const xCellC1(xTable->getCellByName(u"C1"_ustr),
                                               uno::UNO_SET_THROW);
    getUserDefineAttribute(uno::Any(xCellA1), u"proName"_ustr, u"v1"_ustr);
    getUserDefineAttribute(uno::Any(xCellB1), u"proName"_ustr, u"v2"_ustr);
    getUserDefineAttribute(uno::Any(xCellC1), u"proName"_ustr, u"v3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedPdf)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<drawing::XShape> xShape = getShape(1);
        // This failed, pdf+png replacement graphics pair didn't survive an ODT roundtrip.
        auto xReplacementGraphic
            = getProperty<uno::Reference<graphic::XGraphic>>(xShape, u"ReplacementGraphic"_ustr);
        CPPUNIT_ASSERT(xReplacementGraphic.is());

        auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xShape, u"Graphic"_ustr);
        CPPUNIT_ASSERT(xGraphic.is());
        // This was image/x-vclgraphic, not exposing the info that the image is a PDF one.
        CPPUNIT_ASSERT_EQUAL(u"application/pdf"_ustr,
                             getProperty<OUString>(xGraphic, u"MimeType"_ustr));
    };

    createSwDoc("embedded-pdf.odt");
    verify();
    saveAndReload(mpFilter);
    verify();

    uno::Sequence<uno::Any> aArgs{ uno::Any(maTempFile.GetURL()) };
    uno::Reference<container::XNameAccess> xNameAccess(
        m_xSFactory->createInstanceWithArguments(u"com.sun.star.packages.zip.ZipFileAccess"_ustr,
                                                 aArgs),
        uno::UNO_QUERY);
    bool bHasBitmap = false;
    const uno::Sequence<OUString> aNames = xNameAccess->getElementNames();
    for (const auto& rElementName : aNames)
    {
        if (rElementName.startsWith("Pictures") && rElementName.endsWith("png"))
        {
            bHasBitmap = true;
            break;
        }
    }
    // This failed, replacement was an svm file.
    CPPUNIT_ASSERT(bHasBitmap);
}

DECLARE_ODFEXPORT_TEST(testTableStyles1, "table_styles_1.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Table styles basic graphic test.
    // Doesn't cover all attributes.
    uno::Reference<style::XStyleFamiliesSupplier> XFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xFamilies(XFamiliesSupplier->getStyleFamilies());
    uno::Reference<container::XNameAccess> xCellFamily(xFamilies->getByName(u"CellStyles"_ustr),
                                                       uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell1Style;
    xCellFamily->getByName(u"Test style.1"_ustr) >>= xCell1Style;

    sal_Int32 nInt32 = 0xF0F0F0;
    table::BorderLine2 oBorder;

    CPPUNIT_ASSERT_EQUAL(Color(0xCC0000), getProperty<Color>(xCell1Style, u"BackColor"_ustr));
    xCell1Style->getPropertyValue(u"WritingMode"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nInt32);
    xCell1Style->getPropertyValue(u"VertOrient"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nInt32);
    xCell1Style->getPropertyValue(u"BorderDistance"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), nInt32);
    xCell1Style->getPropertyValue(u"LeftBorderDistance"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), nInt32);
    xCell1Style->getPropertyValue(u"RightBorderDistance"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), nInt32);
    xCell1Style->getPropertyValue(u"TopBorderDistance"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), nInt32);
    xCell1Style->getPropertyValue(u"BottomBorderDistance"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), nInt32);
    xCell1Style->getPropertyValue(u"RightBorder"_ustr) >>= oBorder;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), oBorder.Color);
    xCell1Style->getPropertyValue(u"LeftBorder"_ustr) >>= oBorder;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, oBorder.Color));
    xCell1Style->getPropertyValue(u"TopBorder"_ustr) >>= oBorder;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, oBorder.Color));
    xCell1Style->getPropertyValue(u"BottomBorder"_ustr) >>= oBorder;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, oBorder.Color));
}

DECLARE_ODFEXPORT_TEST(testTableStyles2, "table_styles_2.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Table styles paragraph and char tests
    // Doesn't cover all attributes.
    // Problem: underline for table autoformat doesn't work.
    uno::Reference<style::XStyleFamiliesSupplier> XFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xFamilies(XFamiliesSupplier->getStyleFamilies());
    uno::Reference<container::XNameAccess> xTableFamily(xFamilies->getByName(u"TableStyles"_ustr),
                                                        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTableStyle(xTableFamily->getByName(u"Test style2"_ustr),
                                                       uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell1Style;

    float fFloat = 0.;
    bool bBool = true;
    sal_Int16 nInt16 = 0xF0;
    sal_Int32 nInt32 = 0xF0F0F0;
    OUString sString;
    awt::FontSlant eCharPosture;

    // cell 1
    xTableStyle->getByName(u"first-row-start-column"_ustr) >>= xCell1Style;
    xCell1Style->getPropertyValue(u"ParaAdjust"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nInt32);
    CPPUNIT_ASSERT_EQUAL(Color(0xFF6600), getProperty<Color>(xCell1Style, u"CharColor"_ustr));
    xCell1Style->getPropertyValue(u"CharContoured"_ustr) >>= bBool;
    CPPUNIT_ASSERT_EQUAL(false, bBool);
    xCell1Style->getPropertyValue(u"CharShadowed"_ustr) >>= bBool;
    CPPUNIT_ASSERT_EQUAL(true, bBool);
    xCell1Style->getPropertyValue(u"CharStrikeout"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nInt32);
    xCell1Style->getPropertyValue(u"CharUnderline"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nInt32);
    // underline color is not working for table autoformats
    // xCell1Style->getPropertyValue("CharUnderlineHasColor") >>= bBool;
    // CPPUNIT_ASSERT_EQUAL(bool(false), bBool);
    // xCell1Style->getPropertyValue("CharUnderlineColor") >>= nInt64;
    // CPPUNIT_ASSERT_EQUAL(sal_Int64(-1), nInt64);
    // standard font
    xCell1Style->getPropertyValue(u"CharHeight"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(18.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeight"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(100.), fFloat);
    xCell1Style->getPropertyValue(u"CharPosture"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontName"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Courier"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleName"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(OUString(), sString);
    xCell1Style->getPropertyValue(u"CharFontFamily"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitch"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nInt16);
    // cjk font
    xCell1Style->getPropertyValue(u"CharHeightAsian"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(18.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeightAsian"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(100.), fFloat);
    xCell1Style->getPropertyValue(u"CharPostureAsian"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontNameAsian"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Courier"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleNameAsian"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Regularna"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontFamilyAsian"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitchAsian"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nInt16);
    // ctl font
    xCell1Style->getPropertyValue(u"CharHeightComplex"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(18.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeightComplex"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(100.), fFloat);
    xCell1Style->getPropertyValue(u"CharPostureComplex"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontNameComplex"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Courier"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleNameComplex"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Regularna"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontFamilyComplex"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitchComplex"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nInt16);

    // cell 2
    xTableStyle->getByName(u"first-row"_ustr) >>= xCell1Style;
    xCell1Style->getPropertyValue(u"ParaAdjust"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nInt32);
    CPPUNIT_ASSERT_EQUAL(Color(0x9900FF), getProperty<Color>(xCell1Style, u"CharColor"_ustr));
    xCell1Style->getPropertyValue(u"CharContoured"_ustr) >>= bBool;
    CPPUNIT_ASSERT_EQUAL(true, bBool);
    xCell1Style->getPropertyValue(u"CharShadowed"_ustr) >>= bBool;
    CPPUNIT_ASSERT_EQUAL(false, bBool);
    xCell1Style->getPropertyValue(u"CharStrikeout"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nInt32);
    xCell1Style->getPropertyValue(u"CharUnderline"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), nInt32);
    // underline color test place
    // standard font
    xCell1Style->getPropertyValue(u"CharHeight"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(12.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeight"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(150.), fFloat);
    xCell1Style->getPropertyValue(u"CharPosture"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontName"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Liberation Serif"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleName"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(OUString(), sString);
    xCell1Style->getPropertyValue(u"CharFontFamily"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitch"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    // cjk font
    xCell1Style->getPropertyValue(u"CharHeightAsian"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(12.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeightAsian"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(150.), fFloat);
    xCell1Style->getPropertyValue(u"CharPostureAsian"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontNameAsian"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Liberation Serif"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleNameAsian"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Pogrubiona"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontFamilyAsian"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitchAsian"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    // ctl font
    xCell1Style->getPropertyValue(u"CharHeightComplex"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(12.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeightComplex"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(150.), fFloat);
    xCell1Style->getPropertyValue(u"CharPostureComplex"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontNameComplex"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Liberation Serif"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleNameComplex"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Pogrubiona"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontFamilyComplex"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitchComplex"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);

    // cell 3
    xTableStyle->getByName(u"first-row-even-column"_ustr) >>= xCell1Style;
    xCell1Style->getPropertyValue(u"ParaAdjust"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nInt32);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xCell1Style, u"CharColor"_ustr));
    xCell1Style->getPropertyValue(u"CharContoured"_ustr) >>= bBool;
    CPPUNIT_ASSERT_EQUAL(true, bBool);
    xCell1Style->getPropertyValue(u"CharShadowed"_ustr) >>= bBool;
    CPPUNIT_ASSERT_EQUAL(true, bBool);
    xCell1Style->getPropertyValue(u"CharStrikeout"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nInt32);
    xCell1Style->getPropertyValue(u"CharUnderline"_ustr) >>= nInt32;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), nInt32);
    // underline color test place
    // standard font
    xCell1Style->getPropertyValue(u"CharHeight"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(12.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeight"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(100.), fFloat);
    xCell1Style->getPropertyValue(u"CharPosture"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontName"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Open Sans"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleName"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(OUString(), sString);
    xCell1Style->getPropertyValue(u"CharFontFamily"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitch"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    // cjk font
    xCell1Style->getPropertyValue(u"CharHeightAsian"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(12.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeightAsian"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(100.), fFloat);
    xCell1Style->getPropertyValue(u"CharPostureAsian"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontNameAsian"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Open Sans"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleNameAsian"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Kursywa"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontFamilyAsian"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitchAsian"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
    // ctl font
    xCell1Style->getPropertyValue(u"CharHeightComplex"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(12.), fFloat);
    xCell1Style->getPropertyValue(u"CharWeightComplex"_ustr) >>= fFloat;
    CPPUNIT_ASSERT_EQUAL(float(100.), fFloat);
    xCell1Style->getPropertyValue(u"CharPostureComplex"_ustr) >>= eCharPosture;
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, eCharPosture);
    xCell1Style->getPropertyValue(u"CharFontNameComplex"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Open Sans"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontStyleNameComplex"_ustr) >>= sString;
    CPPUNIT_ASSERT_EQUAL(u"Kursywa"_ustr, sString);
    xCell1Style->getPropertyValue(u"CharFontFamilyComplex"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nInt16);
    xCell1Style->getPropertyValue(u"CharFontPitchComplex"_ustr) >>= nInt16;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nInt16);
}

CPPUNIT_TEST_FIXTURE(Test, testTableStyles3)
{
    loadAndReload("table_styles_3.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This test checks if default valued attributes aren't exported.
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);

    // <style:paragraph-properties>
    // For this element the only exported attributes are: "border-left", "border-bottom"
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "background-color");
    // border-left place
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "border-right");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "border-top");
    // border-bottom place
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "padding");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "padding-left");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "padding-right");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "padding-top");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "padding-bottom");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test "
                           "style3.2']/style:table-cell-properties",
                           "writing-mode");

    // <style:paragraph-properties> should be absent, because it has only "text-align" attribute, which shouldn't be exported.
    // Assume that style:paragraph-properties and style:text-properties exists.
    assertXPathChildren(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style3.2']",
        2);

    // <style:text-properties>
    // For this element the only exported attributes are: "use-window-font-color place", "font-size-asian", "font-name-asian", "font-family-asian", "font-name-complex", "font-family-complex"
    // use-window-font-color place
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "text-shadow");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "text-outline");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "text-line-through-style");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "text-line-through-type");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "text-underline-style");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "text-underline-color");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-size");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-weight");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-style");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-name");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-family");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-style-name");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-family-generic");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-pitch");
    // font-size-asian place
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-weight-asian");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-style-asian");
    // font-name-asian place
    // font-family-asian place
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-style-name-asian");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-family-generic-asian");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-pitch-asian");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-size-complex");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-weight-complex");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-style-complex");
    // font-name-complex place
    // font-family-complex place
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-style-name-complex");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-family-generic-complex");
    assertXPathNoAttribute(pXmlDoc,
                           "/office:document-styles/office:styles/"
                           "style:style[@style:display-name='Test style3.2']/style:text-properties",
                           "font-pitch-complex");
}

CPPUNIT_TEST_FIXTURE(Test, testTableStyles4)
{
    createSwDoc("table_styles_4.odt");
    // Test if loaded styles overwrite existing styles
    uno::Reference<style::XStyleFamiliesSupplier> XFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xFamilies(XFamiliesSupplier->getStyleFamilies());
    uno::Reference<container::XNameAccess> xTableFamily(xFamilies->getByName(u"TableStyles"_ustr),
                                                        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTableStyle(xTableFamily->getByName(u"Green"_ustr),
                                                       uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell1Style;

    xTableStyle->getByName(u"first-row-start-column"_ustr) >>= xCell1Style;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(xCell1Style, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTableStyles5)
{
    loadAndReload("table_styles_5.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test if cell styles doesn't have a style:parent-style-name attribute.
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);

    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.1']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.2']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.3']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.4']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.5']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.6']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.7']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.8']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.9']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.10']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.11']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.12']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.13']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.14']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.15']",
        "parent-style-name");
    assertXPathNoAttribute(
        pXmlDoc,
        "/office:document-styles/office:styles/style:style[@style:display-name='Test style.16']",
        "parent-style-name");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf145226)
{
    loadAndReload("tdf145226.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPathNoAttribute(
        pXmlDoc, "/office:document-content/office:body/office:text/table:table/table:table-row[1]",
        "style-name");
    assertXPathNoAttribute(
        pXmlDoc, "/office:document-content/office:body/office:text/table:table/table:table-row[2]",
        "style-name");
    assertXPathNoAttribute(
        pXmlDoc, "/office:document-content/office:body/office:text/table:table/table:table-row[3]",
        "style-name");
    assertXPathNoAttribute(
        pXmlDoc, "/office:document-content/office:body/office:text/table:table/table:table-row[4]",
        "style-name");
}

DECLARE_ODFEXPORT_TEST(testTdf101710, "tdf101710.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test that number format of cell styles can be imported and exported.
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"CellStyles"_ustr)->getByName(u"Test Style.11"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(10104), getProperty<sal_uInt32>(xStyle, u"NumberFormat"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129568)
{
    loadAndReload("tdf129568.fodt");
    // Test that export doesn't fail, and that style is imported and in use.
    uno::Reference<style::XStyle> xStyle(
        getStyles(u"CellStyles"_ustr)->getByName(u"Default Style.1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStyle->isInUse());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getProperty<Color>(xStyle, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129568ui)
{
    loadAndReload("tdf129568-ui.fodt");
    // Same as above, but styles referenced by UI name.
    uno::Reference<style::XStyle> xStyle(
        getStyles(u"CellStyles"_ustr)->getByName(u"Default Style.1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStyle->isInUse());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getProperty<Color>(xStyle, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf164712)
{
    loadAndReload("tdf164712.odt");
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    auto xCursor(xText->createTextCursorByRange(getParagraph(1)));
    CPPUNIT_ASSERT(xCursor.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xCursor, uno::UNO_QUERY);
    Color nColor;
    // without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(
        !css::uno::fromAny(xPropertySet->getPropertyValue(u"CharBackColor"_ustr), &nColor));
}

DECLARE_ODFEXPORT_TEST(testTdf132642_keepWithNextTable, "tdf132642_keepWithNextTable.odt")
{
    // Since the row is very big, it should split over two pages.
    // Since up to this point we haven't tried to make it match MS formats, it should start on page 1.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row splits over 2 pages", 2, getPages());
}

DECLARE_ODFEXPORT_TEST(testArabicZeroNumberingRTF, "arabic-zero-numbering.rtf")
{
    auto xNumberingRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.docx")
{
    auto xNumberingRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero3Numbering, "arabic-zero3-numbering.docx")
{
    auto xNumberingRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO3.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero4Numbering, "arabic-zero4-numbering.docx")
{
    auto xNumberingRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 66
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO4.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero5Numbering, "arabic-zero5-numbering.docx")
{
    auto xNumberingRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 67
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO5.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

CPPUNIT_TEST_FIXTURE(Test, testImageMimetype)
{
    loadAndReload("image-mimetype.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Test that the loext:mimetype attribute is written for exported images, tdf#109202
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Original image (svg)
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p/draw:frame/"
                         "draw:image[@draw:mime-type='image/svg+xml']");
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
