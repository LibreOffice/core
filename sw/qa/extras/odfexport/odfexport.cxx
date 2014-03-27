/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/odfexport/data/", "writer8") {}

    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE {
        // Only test import of .odt document
        return OString(filename).endsWith(".odt");
    }
};

#define DECLARE_ODFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

DECLARE_ODFEXPORT_TEST(testFdo38244, "fdo38244.odt")
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

    // Test properties
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("__Fieldmark__4_1833023242"), getProperty<OUString>(xPropertySet, "Name"));
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));
}

DECLARE_ODFEXPORT_TEST(testFirstHeaderFooter, "first-header-footer.odt")
{
    // Test import and export of the header-first token.

    // The document has 6 pages, two page styles for the first and second half of pages.
    CPPUNIT_ASSERT_EQUAL(OUString("First header"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First footer"),  parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Left header"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Left footer"),   parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Right header"),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Right footer"),  parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First header2"), parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First footer2"), parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Right header2"), parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Right footer2"), parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Left header2"),  parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Left footer2"),  parseDump("/root/page[6]/footer/txt/text()"));
}

DECLARE_ODFEXPORT_TEST(testTextframeGradient, "textframe-gradient.odt")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient aGradient = getProperty<awt::Gradient>(xFrame, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xC0504D), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD99594), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);

    xFrame.set(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    aGradient = getProperty<awt::Gradient>(xFrame, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x666666), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
}

DECLARE_ODFEXPORT_TEST(testFdo60769, "fdo60769.odt")
{
    // Test multi-paragraph comment range feature.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType =  getProperty<OUString>(xPropertySet, "TextPortionType");
        // First paragraph: no field end, no anchor
        CPPUNIT_ASSERT(aType == "Text" || aType == "Annotation");
    }

    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType =  getProperty<OUString>(xPropertySet, "TextPortionType");
        // Second paragraph: no field start
        CPPUNIT_ASSERT(aType == "Text" || aType == "AnnotationEnd");
    }
}

DECLARE_ODFEXPORT_TEST(testFdo58949, "fdo58949.docx")
{
    /*
     * The problem was that the exporter didn't insert "Obj102" to the
     * resulting zip file. No idea how to check for "broken" (missing OLE data
     * and replacement image) OLE objects using UNO, so we'll check the zip file directly.
     */

    utl::TempFile aTempFile;
    save("writer8", aTempFile);

    uno::Sequence<uno::Any> aArgs(1);
    aArgs[0] <<= OUString(aTempFile.GetURL());
    uno::Reference<container::XNameAccess> xNameAccess(m_xSFactory->createInstanceWithArguments("com.sun.star.packages.zip.ZipFileAccess", aArgs), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("Obj102")));
}

DECLARE_ODFEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    // Make sure paragraph and character attributes don't interfere
    // First paragraph has a paragraph border and a character border included by the paragraph style

    // Paragraph border of first paragraph
    {
        const table::BorderLine2 aFirstParTopBorder(0x6666FF,0,26,26,7,53);
        const sal_Int32 aFirstParTopPadding(150);
        uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);

        // Top border
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,"TopBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,"TopBorderDistance"));

        // Bottom border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,"BottomBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,"BottomBorderDistance"));

        // Left border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,"LeftBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,"LeftBorderDistance"));

        // Right border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParTopBorder, getProperty<table::BorderLine2>(xSet,"RightBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParTopPadding, getProperty<sal_Int32>(xSet,"RightBorderDistance"));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,"ParaShadowFormat");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aShadow.Color);
        CPPUNIT_ASSERT_EQUAL(false, (bool)aShadow.IsTransparent);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation(0), aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aShadow.ShadowWidth);
    }

    // Character border for first paragraph
    {
        const table::BorderLine2 aFirstParCharTopBorder(0xFF3333,0,37,0,2,37);
        const sal_Int32 aFirstParCharTopPadding(450);
        uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);

        // Top border
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharTopBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

        // Bottom border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharBottomBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

        // Left border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharLeftBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

        // Right border (same as top border)
        CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharRightBorder"));
        CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,"CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF3333), aShadow.Color);
        CPPUNIT_ASSERT_EQUAL(false, (bool)aShadow.IsTransparent);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation(2), aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(280), aShadow.ShadowWidth);

        // Check autostyle
        {
            uno::Reference< style::XAutoStyleFamily > xAutoStyleFamily(getAutoStyles("ParagraphStyles"));
            uno::Reference < container::XEnumeration > xAutoStylesEnum( xAutoStyleFamily->createEnumeration() );
            CPPUNIT_ASSERT_EQUAL(true, (bool)xAutoStylesEnum->hasMoreElements());

            // First paragraph autostyle
            uno::Reference < beans::XPropertySet > xPSet( xAutoStylesEnum->nextElement(), uno::UNO_QUERY );

            // Top border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharTopBorder"));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

            // Bottom border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharBottomBorder"));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

            // Left border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharLeftBorder"));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

            // Right border
            CPPUNIT_ASSERT_BORDER_EQUAL(aFirstParCharTopBorder, getProperty<table::BorderLine2>(xSet,"CharRightBorder"));
            CPPUNIT_ASSERT_EQUAL(aFirstParCharTopPadding, getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));
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
            table::BorderLine2(0,0,0,0,0,0)                // Right
        };

        sal_Int32 aDistances[4] = { 400 /*Top*/, 300 /*Bottom*/, 250 /*Left*/, 0 /*Right*/ };

        // Get second text portion of second paragraph
        uno::Reference < beans::XPropertySet > xSet( getRun(getParagraph(2),2), uno::UNO_QUERY );

        // Top border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[0], getProperty<table::BorderLine2>(xSet,"CharTopBorder"));
        CPPUNIT_ASSERT_EQUAL(aDistances[0], getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

        // Bottom border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[1], getProperty<table::BorderLine2>(xSet,"CharBottomBorder"));
        CPPUNIT_ASSERT_EQUAL(aDistances[1], getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

        // Left border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[2], getProperty<table::BorderLine2>(xSet,"CharLeftBorder"));
        CPPUNIT_ASSERT_EQUAL(aDistances[2], getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

        // Right border
        CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[3], getProperty<table::BorderLine2>(xSet,"CharRightBorder"));
        CPPUNIT_ASSERT_EQUAL(aDistances[3], getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,"CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aShadow.Color);
        CPPUNIT_ASSERT_EQUAL(false, (bool)aShadow.IsTransparent);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation(3), aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(79), aShadow.ShadowWidth);

        // Check character style
        {
            uno::Reference< container::XNameAccess > xStyleFamily(getStyles("CharacterStyles"), uno::UNO_QUERY);
            uno::Reference < beans::XPropertySet > xStyleSet(xStyleFamily->getByName("CharDiffBor"), uno::UNO_QUERY);

            // Top border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[0], getProperty<table::BorderLine2>(xSet,"CharTopBorder"));
            CPPUNIT_ASSERT_EQUAL(aDistances[0], getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

            // Bottom border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[1], getProperty<table::BorderLine2>(xSet,"CharBottomBorder"));
            CPPUNIT_ASSERT_EQUAL(aDistances[1], getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

            // Left border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[2], getProperty<table::BorderLine2>(xSet,"CharLeftBorder"));
            CPPUNIT_ASSERT_EQUAL(aDistances[2], getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

            // Right border
            CPPUNIT_ASSERT_BORDER_EQUAL(aBorderArray[3], getProperty<table::BorderLine2>(xSet,"CharRightBorder"));
            CPPUNIT_ASSERT_EQUAL(aDistances[3], getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));
        }
    }
}

DECLARE_ODFEXPORT_TEST(testFdo43807, "fdo43807.odt")
{
    uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Drop Caps"),getProperty<OUString>(xSet,"DropCapCharStyleName"));

    xSet = uno::Reference<beans::XPropertySet>(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("User Defined Drop Caps"),getProperty<OUString>(xSet,"DropCapCharStyleName"));
}

DECLARE_ODFEXPORT_TEST(testTextframeTransparentShadow, "textframe-transparent-shadow.odt")
{
    uno::Reference<drawing::XShape> xPicture = getShape(1);
    // ODF stores opacity of 75%, that means 25% transparency.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<sal_Int32>(xPicture, "ShadowTransparence"));
}

DECLARE_ODFEXPORT_TEST(testRelhPage, "relh-page.odt")
{
    uno::Reference<drawing::XShape> xTextFrame = getShape(1);
    // This was text::RelOrientation::FRAME (the default), RelativeHeightRelation was not handled in xmloff.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeHeightRelation"));
    // Make sure rel-height-rel doesn't affect width.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeWidthRelation"));

    // This was 2601, 20% height was relative from margin, not page.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3168), parseDump("/root/page/body/txt/anchored/fly/infos/bounds", "height").toInt32());
}

DECLARE_ODFEXPORT_TEST(testRelwPage, "relw-page.odt")
{
    uno::Reference<drawing::XShape> xTextFrame = getShape(1);
    // This was text::RelOrientation::FRAME (the default), RelativeWidthRelation was not handled in xmloff.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeWidthRelation"));
    // Make sure rel-width-rel doesn't affect height.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xTextFrame, "RelativeHeightRelation"));

    // This was 3762, 40% width was relative from margin, not page.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4896), parseDump("/root/page/body/txt/anchored/fly/infos/bounds", "width").toInt32());
}

DECLARE_ODFEXPORT_TEST(testTextFrameVertAdjust, "textframe-vertadjust.odt")
{
    // Test import/export of new frame attribute called TextVerticalAdjust

    // 1st frame's context is adjusted to the top
    uno::Reference<beans::XPropertySet> xFrame(getTextFrameByName("Rectangle 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 2nd frame's context is adjusted to the center
    xFrame.set(getTextFrameByName("Rectangle 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_CENTER, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
    // 3rd frame's context is adjusted to the bottom
    xFrame.set(getTextFrameByName("Rectangle 3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xFrame, "TextVerticalAdjust"));
}

DECLARE_ODFEXPORT_TEST(testShapeRelsize, "shape-relsize.odt")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // These were all 0, as style:rel-width/height was ignored on import for shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(40), getProperty<sal_Int16>(xShape, "RelativeWidth"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), getProperty<sal_Int16>(xShape, "RelativeHeight"));

    // Relation was "page" for both width and height, should be "paragraph" for width.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(xShape, "RelativeWidthRelation"));
    // And make sure that height stays "page".
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "RelativeHeightRelation"));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
