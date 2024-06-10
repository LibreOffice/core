/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>

#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <comphelper/propertysequence.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/graphicfilter.hxx>
#include <xmloff/odffields.hxx>

#include <wrtsh.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IMark.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <fmtftn.hxx>
#include <ftnidx.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <frame.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <flyfrms.hxx>
#include <tabfrm.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlimport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf143476LockedCanvas_twoShapes)
{
    // Given a lockedCanvas in a docx document with compatibility to Word version 12 (2007).
    // It contains two shapes. Error was, that the lockedCanvas was not imported as group at all,
    // and only one shape was imported and that one was scaled to lockedCanvas area.
    createSwDoc("tdf143476_lockedCanvas_twoShapes.docx");
    // The group shape corresponds to the lockedCanvas.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup->getCount());
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14200), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1120), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1928), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1593), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143476LockedCanvas_position)
{
    // Given a lockedCanvas in a docx document with compatibility to Word version 12 (2007).
    // Tests fix for regression introduced by 3262fc5ef3bde5b158909d11ccb008161ea95519
    // Error was, that the imported shape had wrong position.
    createSwDoc("tdf143476_lockedCanvas_position.docx");
    // The group shape corresponds to the lockedCanvas.
    uno::Reference<drawing::XShape> xGroupShape(getShape(1), uno::UNO_QUERY);
    // Without fix in place the test failed with position 185|947.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2351), xGroupShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(26), xGroupShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143476LockedCanvas_image_line)
{
    // Given a lockedCanvas in a docx document with compatibility to Word version 12 (2007).
    // It contains an image and a line. Error was, that both were not imported.
    createSwDoc("tdf143476_lockedCanvas_image_line.docx");
    CPPUNIT_ASSERT_MESSAGE("No shapes imported", getShapes() > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143475rotatedWord2007imageInline)
{
    // Given a docx document with compatibility to Word version 12 (2007), which has a shape
    // rotated by 75deg. Similar to testTdf143475rotatedWord2007image but with inline anchored
    // shape, as in bug report.
    createSwDoc("tdf143475_rotatedWord2007imageInline.docx");

    // Word 2007 does not swap width and height for rotated images as done in later versions.
    // This was not considered and lead to wrong distance to text on import and wrong effectExtent
    // on export.
    // Import fails without fix with left: expected 1258 actual -743 ; right expected 1256 actual -743;
    // top: expected 14 actual 2013; bottom: expected 0 actual 1960;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1258),
                                 getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1256),
                                 getProperty<sal_Int32>(getShape(1), u"RightMargin"_ustr), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(14),
                                 getProperty<sal_Int32>(getShape(1), u"TopMargin"_ustr), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(0),
                                 getProperty<sal_Int32>(getShape(1), u"BottomMargin"_ustr), 1);

    // Because LO made the same error on export, which inverts the import error, import-export-cycle
    // does not fail without the patch. Therefore no export test.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143475rotatedWord2007image)
{
    // Given a docx document with compatibility to Word version 12 (2007), which has a shape
    // rotated by 75deg.
    createSwDoc("tdf143475_rotatedWord2007image.docx");

    // Word 2007 does not swap width and height for rotated images as done in later versions.
    // This was not considered and lead to wrong distance to text on import and wrong effectExtent
    // on export.
    // Import fails without fix with left: expected 1252 actual -746 ; right expected 1256 actual -743;
    // top: expected 12 actual 2013; bottom: expected 0 actual 1960;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1252),
                                 getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1256),
                                 getProperty<sal_Int32>(getShape(1), u"RightMargin"_ustr), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(12),
                                 getProperty<sal_Int32>(getShape(1), u"TopMargin"_ustr), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(0),
                                 getProperty<sal_Int32>(getShape(1), u"BottomMargin"_ustr), 1);

    // Because LO made the same error on export, which inverts the import error, import-export-cycle
    // does not fail without the patch. Therefore no export test.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143219ContourWrapRotate)
{
    createSwDoc("tdf143219_ContourWrap_rotate.docx");
    const uno::Reference<drawing::XShape> xShape = getShape(1);
    const uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    sal_Int32 nWrapDistanceLeft = -1;
    sal_Int32 nWrapDistanceRight = -1;
    sal_Int32 nWrapDistanceTop = -1;
    sal_Int32 nWrapDistanceBottom = -1;
    xShapeProps->getPropertyValue(u"LeftMargin"_ustr) >>= nWrapDistanceLeft;
    xShapeProps->getPropertyValue(u"RightMargin"_ustr) >>= nWrapDistanceRight;
    xShapeProps->getPropertyValue(u"TopMargin"_ustr) >>= nWrapDistanceTop;
    xShapeProps->getPropertyValue(u"BottomMargin"_ustr) >>= nWrapDistanceBottom;
    // Word and Writer use different concepts for contour wrap. LO needs wrap margins to
    // approximate Word's rendering.
    // Without the fix in place left and right margin were too large, top and bottom margin too
    // small. The test would have failed
    // ... with expected 182 actual 1005.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("LeftMargin", 182, nWrapDistanceLeft, 1);
    // ... with expected 183 actual 1005
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("RightMargin", 183, nWrapDistanceRight, 1);
    // ... with expected 42 actual 0
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("TopMargin", 42, nWrapDistanceTop, 1);
    // ... with expected 41 actual 0
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("BottomMargin", 41, nWrapDistanceBottom, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108545_embeddedDocxIcon)
{
    createSwDoc("tdf108545_embeddedDocxIcon.docx");
    uno::Reference<document::XEmbeddedObjectSupplier2> xSupplier(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(embed::Aspects::MSOLE_ICON, xSupplier->getAspect());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121203)
{
    createSwDoc("tdf121203.docx");
    // We imported the date field
    uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);

    // Custom sdt date content is imported correctly
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bDate{};
    xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
    CPPUNIT_ASSERT(bDate);

    OUString sDateFormat;
    xContentControlProps->getPropertyValue(u"DateFormat"_ustr) >>= sDateFormat;

    OUString sLang;
    xContentControlProps->getPropertyValue(u"DateLanguage"_ustr) >>= sLang;

    uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl,
                                                                            uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentControlEnum
        = xContentControlEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xTextPortionRange(xContentControlEnum->nextElement(),
                                                       uno::UNO_QUERY);
    OUString sCurrentDate = xTextPortionRange->getString();
    CPPUNIT_ASSERT_EQUAL(u"dd-MMM-yy"_ustr, sDateFormat);
    CPPUNIT_ASSERT_EQUAL(u"en-GB"_ustr, sLang);
    CPPUNIT_ASSERT_EQUAL(u"17-Oct-2018 09:00"_ustr, sCurrentDate);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109053)
{
    createSwDoc("tdf109053.docx");
    // Table was imported into a text frame which led to a one page document
    // Originally the table takes two pages, so Writer should import it accordingly.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121664)
{
    createSwDoc("tdf121664.docx");
    uno::Reference<text::XLineNumberingProperties> xLineNumbering(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xLineNumbering.is());
    // Without the accompanying fix in place, numbering did not restart on the
    // second page.
    CPPUNIT_ASSERT(
        getProperty<bool>(xLineNumbering->getLineNumberingProperties(), u"RestartAtEachPage"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108849)
{
    createSwDoc("tdf108849.docx");
    // sectPr element that is child element of body must be the last child. However, Word accepts it
    // in wrong places, and we should do the same (bug-to-bug compatibility) without creating extra sections.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Misplaced body-level sectPr's create extra sections!", 2,
                                 getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97038)
{
    createSwDoc("tdf97038.docx");
    // Without the accompanying fix in place, this test would have failed, as the importer lost the
    // fLayoutInCell shape property for wrap-though shapes.
    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName(u"Kep2"), u"IsFollowingTextFlow"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf114212)
{
    createSwDoc("tdf114212.docx");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1428
    // - Actual  : 387
    OUString aTop = parseDump("//anchored/fly[1]/infos/bounds"_ostr, "top"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"1428"_ustr, aTop);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109524)
{
    createSwDoc("tdf109524.docx");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // The table should have a small width (just to hold the short text in its single cell).
    // Until it's correctly implemented, we assign it 100% relative width.
    // Previously, the table (without explicitly set width) had huge actual width
    // and extended far outside of page's right border.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(0), u"IsWidthRelative"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<sal_Int16>(xTables->getByIndex(0), u"RelativeWidth"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf120547)
{
    createSwDoc("tdf120547.docx");
    uno::Reference<drawing::XShape> xGroupShape = getShape(1);
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup->getCount());

    awt::Point aPosGroup = xGroupShape->getPosition();
    awt::Size aSizeGroup = xGroupShape->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosGroup.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosGroup.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9091), aSizeGroup.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27940), aSizeGroup.Height);

    // Without the fix in place, this test would have failed at many places
    // as the three shapes in the group would have had an incorrect position,
    // an incorrect width or an incorrect height.

    uno::Reference<drawing::XShape> xShape1(xGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    awt::Point aPosShape1 = xShape1->getPosition();
    awt::Size aSizeShape1 = xShape1->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape1.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape1.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9066), aSizeShape1.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27905), aSizeShape1.Height);

    uno::Reference<drawing::XShape> xShape2(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    awt::Point aPosShape2 = xShape2->getPosition();
    awt::Size aSizeShape2 = xShape2->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape2.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20745), aPosShape2.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9066), aSizeShape2.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7195), aSizeShape2.Height);

    // The second shape is a group of 3 shapes
    uno::Reference<container::XIndexAccess> xGroup2(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup2->getCount());

    uno::Reference<drawing::XShape> xShape3(xGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    awt::Point aPosShape3 = xShape3->getPosition();
    awt::Size aSizeShape3 = xShape3->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape3.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape3.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9091), aSizeShape3.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8073), aSizeShape3.Height);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118693)
{
    createSwDoc("tdf118693.docx");
    uno::Reference<drawing::XShape> xGroupShape = getShape(1);
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup->getCount());

    awt::Point aPosGroup = xGroupShape->getPosition();
    awt::Size aSizeGroup = xGroupShape->getSize();

    // ToDo: width and height are inaccurate for unknown reason.
    // Allow some tolerance
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10162), aPosGroup.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(118), aPosGroup.Y);
    // width 2292840 EMU = 6369, height 1793875 EMU = 4982.98
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(6369), aSizeGroup.Width, 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4983), aSizeGroup.Height, 2);

    // Without the fix in place, this test would have failed at many places
    // as the first shape in the group would have had an incorrect position,
    // an incorrect width or an incorrect height.

    uno::Reference<drawing::XShape> xShape1(xGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    awt::Point aPosShape1 = xShape1->getPosition();
    awt::Size aSizeShape1 = xShape1->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(12861), aPosShape1.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(146), aPosShape1.Y);
    // width 2292840/2293461*1321179 EMU = 3668.94, height 1767845 EMU = 4910.68
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(3671), aSizeShape1.Width, 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4914), aSizeShape1.Height, 2);

    uno::Reference<drawing::XShape> xShape2(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    awt::Point aPosShape2 = xShape2->getPosition();
    awt::Size aSizeShape2 = xShape2->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10162), aPosShape2.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(118), aPosShape2.Y);
    // width 2292840/2293461*1654824 EMU = 4595.48, height 1793875 EMU = 4982.98
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4597), aSizeShape2.Width, 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4983), aSizeShape2.Height, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testGroupShapeFontName)
{
    createSwDoc("groupshape-fontname.docx");
    // Font names inside a group shape were not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY_THROW)->getText();

    CPPUNIT_ASSERT_EQUAL(
        u"Calibri"_ustr,
        getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Calibri"_ustr,
                         getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1),
                                               u"CharFontNameComplex"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        u"Calibri"_ustr,
        getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), u"CharFontNameAsian"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124600)
{
    createSwDoc("tdf124600.docx");
    // uno::Reference<drawing::XShape> xShape = getShape(1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 318
    // i.e. the shape had an unexpected left margin, but not in Word.
    // Regina: LO needs a left margin to get the same rendering as Word, because Word aligns the
    // shape with the outer edge of the border, but LibreOffice aligns with the snap rectangle.
    // Expected: 0 is wrong. ToDo: The current margin is wrong and needs to be fixed. Then activate
    // the test again with the correct margin.
    // CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
    //                     getProperty<sal_Int32>(xShape, "HoriOrientPosition"));

    // Make sure that "Shape 1 text" (anchored in the header) has the same left margin as the body
    // text.
    OUString aShapeTextLeft
        = parseDump("/root/page/header/txt/anchored/fly/infos/bounds"_ostr, "left"_ostr);
    OUString aBodyTextLeft = parseDump("/root/page/body/txt/infos/bounds"_ostr, "left"_ostr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1701
    // - Actual  : 1815
    // i.e. there was a >0 left margin on the text of the shape, resulting in incorrect horizontal
    // position.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aBodyTextLeft.toDouble(), aShapeTextLeft.toDouble(), 1.0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf120548)
{
    createSwDoc("tdf120548.docx");
    // Without the accompanying fix in place, this test would have failed with 'Expected: 00ff0000;
    // Actual: ffffffff', i.e. the numbering portion was black, not red.
    CPPUNIT_ASSERT_EQUAL(
        u"00ff0000"_ustr,
        parseDump("//SwFieldPortion[@type='PortionType::Number']/SwFont"_ostr, "color"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, test120551)
{
    createSwDoc("tdf120551.docx");
    auto nHoriOrientPosition = getProperty<sal_Int32>(getShape(1), u"HoriOrientPosition"_ustr);
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 430, Actual  : -2542'.
    // CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(430), nHoriOrientPosition);
    // File 140335EMU = 389,8Hmm
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(390), nHoriOrientPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf111550)
{
    createSwDoc("tdf111550.docx");
    // The test document has following ill-formed structure:
    //
    //    <w:tbl>
    //        ...
    //        <w:tr>
    //            <w:tc>
    //                <w:p>
    //                    <w:r>
    //                        <w:t>[outer:A2]</w:t>
    //                        <w:br w:type="textWrapping"/>
    //                    </w:r>
    //                    <w:tbl>
    //                        <w:tr>
    //                            <w:tc>
    //                                <w:p>
    //                                    <w:r>
    //                                        <w:t>[inner:A1]</w:t>
    //                                    </w:r>
    //                                </w:p>
    //                            </w:tc>
    //                        </w:tr>
    //                    </w:tbl>
    //                </w:p>
    //            </w:tc>
    //        </w:tr>
    //    </w:tbl>
    //
    // i.e., a <w:tbl> as direct child of <w:p> inside another table.
    // Word accepts that illegal OOXML, and treats it as equal to
    //
    //    <w:tbl>
    //        ...
    //        <w:tr>
    //            <w:tc>
    //                <w:tbl>
    //                    <w:tr>
    //                        <w:tc>
    //                            <w:p>
    //                                <w:r>
    //                                    <w:t>[outer:A2]</w:t>
    //                                    <w:br w:type="textWrapping"/>
    //                                </w:r>
    //                                <w:r>
    //                                    <w:t>[inner:A1]</w:t>
    //                                </w:r>
    //                            </w:p>
    //                        </w:tc>
    //                    </w:tr>
    //                </w:tbl>
    //            </w:tc>
    //        </w:tr>
    //    </w:tbl>
    //
    // i.e., moves all contents of the outer paragraph into the inner table's first paragraph.

    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    uno::Reference<text::XTextContent> outerTable = getParagraphOrTable(1);
    getCell(outerTable, u"A1"_ustr, u"[outer:A1]"_ustr);
    uno::Reference<text::XText> cellA2(getCell(outerTable, u"A2"_ustr), uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> innerTable = getParagraphOrTable(1, cellA2);
    getCell(innerTable, u"A1"_ustr, u"[outer:A2]\n[inner:A1]"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117843)
{
    createSwDoc("tdf117843.docx");
    uno::Reference<container::XNameAccess> xPageStyles = getStyles(u"PageStyles"_ustr);
    uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName(u"Standard"_ustr),
                                             uno::UNO_QUERY);
    uno::Reference<text::XText> xHeaderText
        = getProperty<uno::Reference<text::XText>>(xPageStyle, u"HeaderText"_ustr);
    // This was 4025, increased top paragraph margin was unexpected.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xHeaderText), u"ParaTopMargin"_ustr));
}

// related tdf#124754
CPPUNIT_TEST_FIXTURE(Test, testTdf43017)
{
    createSwDoc("tdf43017.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 2, u"kick the bucket"_ustr);

    // Ensure that hyperlink text color is not blue (0x0000ff), but default (-1)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hyperlink color should be black!", sal_Int32(-1),
                                 getProperty<sal_Int32>(xText, u"CharColor"_ustr));
}

// related tdf#43017
CPPUNIT_TEST_FIXTURE(Test, testTdf124754)
{
    createSwDoc("tdf124754.docx");
    uno::Reference<text::XText> textbox(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs(textbox));

    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, textbox);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 2);

    // Ensure that hyperlink text color is not black
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hyperlink color should be not black!", sal_Int32(353217),
                                 getProperty<sal_Int32>(xText, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTextCopy)
{
    createSwDoc("text-copy.docx");
    // The document has a header on the second page that is copied as part of the import process.
    // The header has a single paragraph: make sure shapes anchored to it are not lost.
    // Note that the single paragraph itself has no text portions.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara;
    while (xParaEnum->hasMoreElements())
    {
        xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    }
    auto aPageStyleName = getProperty<OUString>(xPara, u"PageStyleName"_ustr);
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(aPageStyleName), uno::UNO_QUERY);
    auto xHeaderText = getProperty<uno::Reference<text::XText>>(xPageStyle, u"HeaderText"_ustr);
    uno::Reference<text::XTextRange> xHeaderPara = getParagraphOfText(1, xHeaderText);
    auto aTextPortionType = getProperty<OUString>(getRun(xHeaderPara, 1), u"TextPortionType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Frame
    // - Actual  : Text
    // i.e. the second page's header had no anchored shapes.
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, aTextPortionType);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112443)
{
    createSwDoc("tdf112443.docx");
    // the position of the flying text frame should be off page
    // 30624 below its anchor
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwRootFrame* pRootFrame = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwRect aPageRect = pRootFrame->getFrameArea();
    const SwRect aShapeRect(getShape(1)->getPosition().X, getShape(1)->getPosition().Y,
                            getShape(1)->getSize().Width, getShape(1)->getSize().Height);
    CPPUNIT_ASSERT_MESSAGE("The textframe must be off-page!", !aPageRect.Contains(aShapeRect));

    //OUString aTop = parseDump("//anchored/fly[1]/infos/bounds", "top");
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(30624), aTop.toInt32() );
}

// DOCX: Textbox wrap differs in MSO and LO
// Both should layout text regardless of existing text box
// and as result only one page should be generated.
CPPUNIT_TEST_FIXTURE(Test, testTdf113182)
{
    createSwDoc("tdf113182.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrFrameVml)
{
    createSwDoc("btlr-frame-vml.docx");
    uno::Reference<beans::XPropertySet> xTextFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextFrame.is());

    auto nActual = getProperty<sal_Int16>(xTextFrame, u"WritingMode"_ustr);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 5; Actual:
    // 4', i.e. writing direction was inherited from page, instead of explicit btlr.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, nActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124398)
{
    createSwDoc("tdf124398.docx");
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    // Without the accompanying fix in place, this test would have failed with 'Expected: 2; Actual:
    // 1', i.e. the chart children of the group shape was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup->getCount());

    uno::Reference<drawing::XShapeDescriptor> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.OLE2Shape"_ustr, xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104167)
{
    createSwDoc("tdf104167.docx");
    // Make sure that heading 1 paragraphs start on a new page.
    uno::Any xStyle = getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 1"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 0
    // i.e. the <w:pageBreakBefore/> was lost on import.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE,
                         getProperty<style::BreakType>(xStyle, u"BreakType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf113946)
{
    createSwDoc("tdf113946.docx");
    OUString aTop
        = parseDump("/root/page/body/txt/anchored/SwAnchoredDrawObject/bounds"_ostr, "top"_ostr);
    // tdf#106792 Checked loading of tdf113946.docx. Before the change, the expected
    // value of this test was "1696". Opening the file shows a single short line anchored
    // at the doc start. Only diff is that in 'old' version it is slightly rotated, in 'new'
    // version line is strict horizontal. Checked against MSWord2013, there the line
    // is also not rotated -> the change is to the better, correct the expected result here.
    CPPUNIT_ASSERT_EQUAL(u"1695"_ustr, aTop);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121804)
{
    createSwDoc("tdf121804.docx");
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFirstPara = getParagraphOfText(1, xShape->getText());
    uno::Reference<text::XTextRange> xFirstRun = getRun(xFirstPara, 1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xFirstRun, u"CharEscapement"_ustr));
    // This failed with a NoSuchElementException, super/subscript property was
    // lost on import, so the whole paragraph was a single run.
    uno::Reference<text::XTextRange> xSecondRun = getRun(xFirstPara, 2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14000),
                         getProperty<sal_Int32>(xSecondRun, u"CharEscapement"_ustr));
    uno::Reference<text::XTextRange> xThirdRun = getRun(xFirstPara, 3);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-14000),
                         getProperty<sal_Int32>(xThirdRun, u"CharEscapement"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf114217)
{
    // The floating table was not split between page 1 and page 2.
    createSwDoc("tdf114217.docx");
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    auto pTab1 = dynamic_cast<SwTabFrame*>(pPage1Fly->GetLower());
    CPPUNIT_ASSERT(pTab1);
    CPPUNIT_ASSERT(pTab1->HasFollow());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119200)
{
    createSwDoc("tdf119200.docx");
    auto xPara = getParagraph(1);
    // Check that we import MathType functional symbols as symbols, not functions with missing args
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2208 } {}"_ustr, getFormula(getRun(xPara, 1)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2209 } {}"_ustr, getFormula(getRun(xPara, 2)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2282 } {}"_ustr, getFormula(getRun(xPara, 3)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2283 } {}"_ustr, getFormula(getRun(xPara, 4)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2284 } {}"_ustr, getFormula(getRun(xPara, 5)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2286 } {}"_ustr, getFormula(getRun(xPara, 6)));
    CPPUNIT_ASSERT_EQUAL(u" size 12{ func \u2287 } {}"_ustr, getFormula(getRun(xPara, 7)));
}

// Checking a formula where the closing brackets
// come first, and then the opening ones
CPPUNIT_TEST_FIXTURE(Test, testTdf158023Import)
{
    auto verify = [this]() {
        auto xPara = getParagraph(1);
        CPPUNIT_ASSERT_EQUAL(u"\\) sqrt {\\)2\\(} \\("_ustr, getFormula(getRun(xPara, 1)));
    };
    auto verifyReload = [this]() {
        auto xPara = getParagraph(1);
        CPPUNIT_ASSERT_EQUAL(u"\\) sqrt {\\) 2 \\(} \\("_ustr, getFormula(getRun(xPara, 1)));
    };

    createSwDoc("tdf158023_import.docx");
    verify();

    saveAndReload(u"Office Open XML Text"_ustr);
    verifyReload();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115094)
{
    createSwDoc("tdf115094.docx");
    // anchor of graphic has to be the text in the text frame
    // xray ThisComponent.DrawPage(1).Anchor.Text
    uno::Reference<text::XTextContent> xShape(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText1 = xShape->getAnchor()->getText();

    // xray ThisComponent.TextTables(0).getCellByName("A1")
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText2(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(xText1.get(), xText2.get());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115094v2)
{
    createSwDoc("tdf115094v2.docx");
    // layoutInCell="1" combined with <wp:wrapNone/>

    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName(u"Grafik 18"), u"IsFollowingTextFlow"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName(u"Grafik 19"), u"IsFollowingTextFlow"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122224)
{
    createSwDoc("tdf122224.docx");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY_THROW);
    // This was "** Expression is faulty **", because of the unnecessary DOCX number format string
    CPPUNIT_ASSERT_EQUAL(u"2000"_ustr, xCell->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121440)
{
    createSwDoc("tdf121440.docx");
    // Insert some text in front of footnote
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert(u"test"_ustr);

    // Ensure that inserted text is not superscripted
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Inserted text should be not a superscript!", static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getRun(getParagraph(1), 1), u"CharEscapement"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124670)
{
    createSwDoc("tdf124670.docx");
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    // We need to take xml:space attribute into account, even in w:document element
    uno::Reference<text::XTextRange> paragraph = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(
        u"You won't believe, but that's how it was in markup of original      bugdoc!"_ustr,
        paragraph->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126114)
{
    createSwDoc("tdf126114.docx");
    // The problem was that after the drop-down form field, also the placeholder string
    // was imported as text. Beside the duplication of the field, it also caused a crash.
    // the word is from replacement of the drop-down field in ModelToViewHelper
    CPPUNIT_ASSERT_EQUAL(u"gehuwd\n"_ustr, getBodyText());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127825)
{
    createSwDoc("tdf127825.docx");
    // The document has a shape with Japanese-style text in it. The shape has relative size and also
    // has automatic height.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pLayout);
    SwFrame* pPage = pLayout->GetLower();
    CPPUNIT_ASSERT(pPage);
    SwFrame* pBody = pPage->GetLower();
    CPPUNIT_ASSERT(pBody);
    SwFrame* pText = pBody->GetLower();
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT(pText->GetDrawObjs());
    const SwSortedObjs& rDrawObjs = *pText->GetDrawObjs();
    CPPUNIT_ASSERT(rDrawObjs.size());

    // Without the accompanying fix in place, this overlapped the footer area, not the body area.
    CPPUNIT_ASSERT(rDrawObjs[0]->GetObjRect().Overlaps(pBody->getFrameArea()));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103345)
{
    createSwDoc("numbering-circle.docx");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const& prop : aProps)
    {
        if (prop.Name == "NumberingType")
        {
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::CIRCLE_NUMBER, prop.Value.get<sal_Int16>());
            return;
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125038)
{
    createSwDoc("tdf125038.docx");
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone:...
    // - Actual  : result1result2phone:...
    // i.e. the result if the inner MERGEFIELD fields ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(u"phone: \t1234567890"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125038b)
{
    createSwDoc("tdf125038b.docx");
    // Load a document with an IF field, where the IF field command contains a paragraph break.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphAccess(xTextDocument->getText(),
                                                                   uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphAccess->createEnumeration();
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    uno::Reference<text::XTextRange> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone: 1234
    // - Actual  :
    // i.e. the first paragraph was empty and the second paragraph had the content.
    CPPUNIT_ASSERT_EQUAL(u"phone: 1234"_ustr, xParagraph->getString());
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    xParagraphs->nextElement();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !xParagraphs->hasMoreElements()
    // i.e. the document had 3 paragraphs, while only 2 was expected.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125038c)
{
    createSwDoc("tdf125038c.docx");
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: email: test@test.test
    // - Actual  : email:
    // I.e. the result of the MERGEFIELD field inside an IF field was lost.
    CPPUNIT_ASSERT_EQUAL(u"email: test@test.test"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130214)
{
    createSwDoc("tdf130214.docx");
    // Currently this file imports with errors because of tdf#126435; it must not segfault on load
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129659)
{
    createSwDoc("tdf129659.docx");
    // don't crash on footnote with page break
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129912)
{
    createSwDoc("tdf129912.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Goto*FootnoteAnchor iterates the footnotes in a ring, so we need the amount of footnotes to stop the loop
    sal_Int32 nCount = pWrtShell->GetDoc()->GetFootnoteIdxs().size();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nCount);

    // the expected footnote labels
    // TODO: the 5th label is actually wrong (missing the "PR" after the symbol part), but the "b" is there?!
    static constexpr OUString pLabel5 = u"\uF0D1\uF031\uF032b"_ustr;
    const OUString sFootnoteLabels[]
        = { OUString(u'\xF0A7'), u"1"_ustr, u"2"_ustr, OUString(u'\xF020'), pLabel5 };
    CPPUNIT_ASSERT_EQUAL(sal_Int32(SAL_N_ELEMENTS(sFootnoteLabels)), nCount);

    pWrtShell->GotoPrevFootnoteAnchor();
    nCount--;
    while (nCount >= 0)
    {
        SwFormatFootnote aFootnoteNote;
        CPPUNIT_ASSERT(pWrtShell->GetCurFootnote(&aFootnoteNote));
        OUString sNumStr = aFootnoteNote.GetNumStr();
        if (sNumStr.isEmpty())
            sNumStr = OUString::number(aFootnoteNote.GetNumber());
        CPPUNIT_ASSERT_EQUAL(sFootnoteLabels[nCount], sNumStr);
        pWrtShell->GotoPrevFootnoteAnchor();
        nCount--;
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126426)
{
    createSwDoc("tdf126426.docx");

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroup->getCount());

    // get second shape in group
    uno::Reference<text::XTextRange> xRange(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xRange, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xPara, uno::UNO_QUERY_THROW);

    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    {
        // Text before: was before this bugfix
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"Some text "_ustr, xRun->getString());
    }
    {
        // Link and this content was completely missing before
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"Link"_ustr, xRun->getString());
        auto aURL = getProperty<OUString>(xRun, u"HyperLinkURL"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"http://libreoffice.org/"_ustr, aURL);
    }
    {
        // Need to ensure that text following hyperlink is still default color (-1)
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u" and something more."_ustr, xRun->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xRun, u"CharColor"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119039)
{
    createSwDoc("tdf119039_bad_embedded_compound.docx");
    // Should not crash/hang because of problematic embedded compound
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152200)
{
    createSwDoc("tdf152200-bad_fldChar_end.docx");
    // Should not crash/hang because of wrong placement of ending fldChar
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153791)
{
    createSwDoc("tdf153791-shd_overrides_fontRef.docx");

    // the first shape (a paragraph with no background)
    auto xTextBox(getShape(1));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xTextBox, u"CharColor"_ustr));
    uno::Reference<text::XTextRange> xRange(xTextBox, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, xRange->getString());

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xRange, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xPara, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();

    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, xRun->getString());
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xRun, u"CharBackColor"_ustr));
    // In the absence of paragraph/character background, the whole paragraph is red.
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xRun, u"CharColor"_ustr));

    // the second shape: two paragraphs
    xTextBox.set(getShape(2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xTextBox, u"CharColor"_ustr));
    xRange.set(xTextBox, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum" SAL_NEWLINE_STRING "Lorem ipsum"_ustr, xRange->getString());

    xParaEnumAccess.set(xRange, uno::UNO_QUERY_THROW);
    xParaEnum = xParaEnumAccess->createEnumeration();

    // the first one has paragraph background
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(Color(0xF0F0F0), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    xRunEnumAccess.set(xPara, uno::UNO_QUERY_THROW);
    xRunEnum = xRunEnumAccess->createEnumeration();

    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, xRun->getString());
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xRun, u"CharBackColor"_ustr));
    // With paragraph background, the whole paragraph is auto.
    // Without the fix, this would fail with:
    // - Expected: rgba[ffffff00]
    // - Actual  : rgba[ff0000ff]
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xRun, u"CharColor"_ustr));

    // the second paragraph has two runs, the last one with character background
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    xRunEnumAccess.set(xPara, uno::UNO_QUERY_THROW);
    xRunEnum = xRunEnumAccess->createEnumeration();

    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xRun->getString());
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xRun, u"CharBackColor"_ustr));
    // In the absence of paragraph/character background, the run is red
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xRun, u"CharColor"_ustr));

    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"ipsum"_ustr, xRun->getString());
    CPPUNIT_ASSERT_EQUAL(Color(0xF0F0F0), getProperty<Color>(xRun, u"CharBackColor"_ustr));
    // With character background, the run is auto.
    // Without the fix, this would fail with:
    // - Expected: rgba[ffffff00]
    // - Actual  : rgba[ff0000ff]
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xRun, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf154319)
{
    createSwDoc("tdf154319-ToC_with_s_and_d.docx");

    auto xSupplier(mxComponent.queryThrow<css::text::XDocumentIndexesSupplier>());
    auto xIndexes = xSupplier->getDocumentIndexes();
    auto xTOCIndex(xIndexes->getByIndex(0).queryThrow<css::beans::XPropertySet>());
    css::uno::Reference<css::container::XIndexReplace> xLevelFormats;
    CPPUNIT_ASSERT(xTOCIndex->getPropertyValue(u"LevelFormat"_ustr) >>= xLevelFormats);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), xLevelFormats->getCount());

    const auto checkPropVal = [](const auto& expected, const css::beans::PropertyValues& entry,
                                 const OUString& name, sal_Int32 level) {
        auto it
            = std::find_if(entry.begin(), entry.end(),
                           [&name](const css::beans::PropertyValue& p) { return p.Name == name; });
        OString msg = "Property: " + name.toUtf8() + ", level: " + OString::number(level);
        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), it != entry.end());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), css::uno::Any(expected), it->Value);
    };

    // tdf#154360: check tab stops between the number and the entry text
    // The last (10th) level does not correspond to any MS level (only 9 levels there)
    constexpr sal_Int32 levelTabStops[]
        = { 776, 1552, 2328, 3104, 3881, 4657, 5433, 6209, 6985, -1 };

    //start with level 1, 0 is the header level
    for (sal_Int32 nLevel = 1; nLevel < xLevelFormats->getCount(); ++nLevel)
    {
        css::uno::Sequence<css::beans::PropertyValues> aLevel;
        xLevelFormats->getByIndex(nLevel) >>= aLevel;

        sal_Int32 nTabStop = levelTabStops[nLevel - 1];
        sal_Int32 nExpectedTokens = nTabStop < 0 ? 8 : 9;
        CPPUNIT_ASSERT_EQUAL(nExpectedTokens, aLevel.getLength());
        sal_Int32 nIndex = 0;

        checkPropVal(u"TokenHyperlinkStart"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenEntryNumber"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);

        if (nTabStop >= 0)
        {
            checkPropVal(u"TokenTabStop"_ustr, aLevel[nIndex], u"TokenType"_ustr, nLevel);
            checkPropVal(levelTabStops[nLevel - 1], aLevel[nIndex++], u"TabStopPosition"_ustr,
                         nLevel);
        }

        checkPropVal(u"TokenEntryText"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenTabStop"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenChapterInfo"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenText"_ustr, aLevel[nIndex], u"TokenType"_ustr, nLevel);
        checkPropVal(u"\""_ustr, aLevel[nIndex++], u"Text"_ustr, nLevel);

        checkPropVal(u"TokenPageNumber"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenHyperlinkEnd"_ustr, aLevel[nIndex++], u"TokenType"_ustr, nLevel);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf154695)
{
    createSwDoc("tdf154695-ToC_no_numbers.docx");

    auto xSupplier(mxComponent.queryThrow<css::text::XDocumentIndexesSupplier>());
    auto xIndexes = xSupplier->getDocumentIndexes();
    auto xTOCIndex(xIndexes->getByIndex(0).queryThrow<css::beans::XPropertySet>());
    css::uno::Reference<css::container::XIndexReplace> xLevelFormats;
    CPPUNIT_ASSERT(xTOCIndex->getPropertyValue(u"LevelFormat"_ustr) >>= xLevelFormats);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), xLevelFormats->getCount());

    const auto checkPropVal = [](const auto& expected, const css::beans::PropertyValues& entry,
                                 const OUString& name, sal_Int32 level) {
        auto it
            = std::find_if(entry.begin(), entry.end(),
                           [&name](const css::beans::PropertyValue& p) { return p.Name == name; });
        OString msg = "Property: " + name.toUtf8() + ", level: " + OString::number(level);
        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), it != entry.end());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), css::uno::Any(expected), it->Value);
    };

    //start with level 1, 0 is the header level
    for (sal_Int32 nLevel = 1; nLevel < xLevelFormats->getCount(); ++nLevel)
    {
        css::uno::Sequence<css::beans::PropertyValues> aLevel;
        xLevelFormats->getByIndex(nLevel) >>= aLevel;

        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aLevel.getLength());

        checkPropVal(u"TokenHyperlinkStart"_ustr, aLevel[0], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenEntryNumber"_ustr, aLevel[1], u"TokenType"_ustr, nLevel);

        // There's no tab stop between [#E] and [E]!

        checkPropVal(u"TokenEntryText"_ustr, aLevel[2], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenTabStop"_ustr, aLevel[3], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenPageNumber"_ustr, aLevel[4], u"TokenType"_ustr, nLevel);

        checkPropVal(u"TokenHyperlinkEnd"_ustr, aLevel[5], u"TokenType"_ustr, nLevel);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156078)
{
    // Given a DOCX with compat level 15, and a tab stop outside of paragraph right indent
    createSwDoc("tdf156078_rightTabOutsideParaRightIndent.docx");

    // Export it to a PNG (96 ppi)
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PixelWidth", uno::Any(sal_Int32(816)) },
                                           { "PixelHeight", uno::Any(sal_Int32(1056)) } }));
    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"writer_png_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) } }));
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(maTempFile.IsValid());

    Graphic exported;
    GraphicFilter::LoadGraphic(maTempFile.GetURL(), {}, exported);
    Bitmap bmp = exported.GetBitmapEx().GetBitmap();
    BitmapScopedReadAccess pAccess(bmp);

    // "1" must export to the top right corner; check its pixels
    bool numberPixelsFound = false;
    for (tools::Long y = 90; y < 130; ++y)
        for (tools::Long x = 680; x < 720; ++x)
            if (Color(pAccess->GetPixel(y, x)).IsDark())
                numberPixelsFound = true;

    CPPUNIT_ASSERT(numberPixelsFound);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141969)
{
    // Given a file with a table with a style setting font height, and a text re-defining the height
    createSwDoc("tdf141969-font_in_table_with_style.docx");

    auto xTable = getParagraphOrTable(2);
    uno::Reference<text::XText> xCell(getCell(xTable, u"A1"_ustr), uno::UNO_QUERY_THROW);
    auto xParaOfCell = getParagraphOfText(1, xCell);
    auto xRun = getRun(xParaOfCell, 1);

    CPPUNIT_ASSERT_EQUAL(u"<<link:website>>"_ustr, xRun->getString());
    // Without a fix, this would fail with
    // - Expected: 8
    // - Actual  : 11
    CPPUNIT_ASSERT_EQUAL(8.0f, getProperty<float>(xRun, u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf154370)
{
    // Import a file with paragraph and character styles containing toggle properties applied to the end of
    // the paragraphs. Should result in hard attributes resetting the properties
    createSwDoc("tdf154370.docx");
    {
        auto xPara(getParagraph(2));
        auto xRun = getRun(xPara, 2);

        OUString rangeText = xRun->getString();
        CPPUNIT_ASSERT_EQUAL(u"CharStyle BoldItalicCapsEmbossedStrike"_ustr, rangeText);

        const uno::Reference<beans::XPropertyState> xRangePropState(xRun, uno::UNO_QUERY_THROW);
        beans::PropertyState ePropertyState = xRangePropState->getPropertyState(u"CharWeight"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharWeightComplex"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharWeightAsian"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharPosture"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharPostureAsian"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharCaseMap"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharRelief"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharStrikeout"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
    }
    {
        auto xPara(getParagraph(3));
        auto xRun = getRun(xPara, 2);

        OUString rangeText = xRun->getString();
        CPPUNIT_ASSERT_EQUAL(u"CharStyle SmallcapsImprint"_ustr, rangeText);

        const uno::Reference<beans::XPropertyState> xRangePropState(xRun, uno::UNO_QUERY_THROW);
        beans::PropertyState ePropertyState
            = xRangePropState->getPropertyState(u"CharCaseMap"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharRelief"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
    }
    {
        auto xPara(getParagraph(5));
        auto xRun = getRun(xPara, 2);

        OUString rangeText = xRun->getString();
        CPPUNIT_ASSERT_EQUAL(u"CharStyle Hidden"_ustr, rangeText);

        const uno::Reference<beans::XPropertyState> xRangePropState(xRun, uno::UNO_QUERY_THROW);
        beans::PropertyState ePropertyState = xRangePropState->getPropertyState(u"CharHidden"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
    }
    {
        auto xPara(getParagraph(7));
        auto xRun = getRun(xPara, 2);

        OUString rangeText = xRun->getString();
        CPPUNIT_ASSERT_EQUAL(u"OutlineShadow"_ustr, rangeText);

        const uno::Reference<beans::XPropertyState> xRangePropState(xRun, uno::UNO_QUERY_THROW);
        beans::PropertyState ePropertyState
            = xRangePropState->getPropertyState(u"CharContoured"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);

        ePropertyState = xRangePropState->getPropertyState(u"CharShadowed"_ustr);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
    }
}
// tests should only be added to ooxmlIMPORT *if* they fail round-tripping in ooxmlEXPORT

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
