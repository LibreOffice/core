/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>

class Test : public SwModelTestBase
{
public:
    void testFdo38244();
    void testFirstHeaderFooter();
    void testTextframeGradient();
    void testFdo60769();
    void testFdo58949();
    void testCharacterBorder();
    void testFdo43807();
    void testTextframeTransparentShadow();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(run);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void run();
};

void Test::run()
{
    MethodEntry<Test> aMethods[] = {
        {"fdo38244.odt", &Test::testFdo38244},
        {"first-header-footer.odt", &Test::testFirstHeaderFooter},
        {"textframe-gradient.odt", &Test::testTextframeGradient},
        {"fdo60769.odt", &Test::testFdo60769},
        {"fdo58949.docx", &Test::testFdo58949},
        {"charborder.odt", &Test::testCharacterBorder },
        {"fdo43807.odt", &Test::testFdo43807 },
        {"textframe-transparent-shadow.odt", &Test::testTextframeTransparentShadow},
    };
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        load("/sw/qa/extras/odfexport/data/", rEntry.pName);
        // If the testcase is stored in some other format, it's pointless to test.
        if (OString(rEntry.pName).endsWith(".odt"))
            (this->*rEntry.pMethod)();
        reload("writer8");
        (this->*rEntry.pMethod)();
        finish();
    }
}

void Test::testFdo38244()
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
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStart"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));

    // Test properties
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("__Fieldmark__4_1833023242"), getProperty<OUString>(xPropertySet, "Name"));
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));
}

void Test::testFirstHeaderFooter()
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

void Test::testTextframeGradient()
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

void Test::testFdo60769()
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
        CPPUNIT_ASSERT(aType == "Text" || aType == "TextFieldStart");
    }

    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType =  getProperty<OUString>(xPropertySet, "TextPortionType");
        // Second paragraph: no field start
        CPPUNIT_ASSERT(aType == "Text" || aType == "TextFieldEnd");
    }
}

void Test::testFdo58949()
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

void Test::testCharacterBorder()
{
    // Make sure paragraph and character attributes don't interfere
    // First paragraph has a paragraph border and a character border included by the paragraph style

    // Paragraph border of first paragraph
    {
        const table::BorderLine2 aFirstParTopBorder(6711039,0,26,26,7,53);
        const sal_Int32 aFirstParTopPadding(150);
        uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);

        // Top border
        assertEqualBorder(
            aFirstParTopBorder, aFirstParTopPadding,
            getProperty<table::BorderLine2>(xSet,"TopBorder"),
            getProperty<sal_Int32>(xSet,"TopBorderDistance"));

        // Bottom border (same as top border)
        assertEqualBorder(
            aFirstParTopBorder, aFirstParTopPadding,
            getProperty<table::BorderLine2>(xSet,"BottomBorder"),
            getProperty<sal_Int32>(xSet,"BottomBorderDistance"));

        // Left border (same as top border)
        assertEqualBorder(
            aFirstParTopBorder, aFirstParTopPadding,
            getProperty<table::BorderLine2>(xSet,"LeftBorder"),
            getProperty<sal_Int32>(xSet,"LeftBorderDistance"));

        // Right border (same as top border)
        assertEqualBorder(
            aFirstParTopBorder, aFirstParTopPadding,
            getProperty<table::BorderLine2>(xSet,"RightBorder"),
            getProperty<sal_Int32>(xSet,"RightBorderDistance"));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,"ParaShadowFormat");
        CPPUNIT_ASSERT_EQUAL(aShadow.Color, sal_Int32(0));
        CPPUNIT_ASSERT_EQUAL((bool)aShadow.IsTransparent, false);
        CPPUNIT_ASSERT_EQUAL(aShadow.Location, table::ShadowLocation(0));
        CPPUNIT_ASSERT_EQUAL(aShadow.ShadowWidth, sal_Int16(0));
    }

    // Character border for first paragraph
    {
        const table::BorderLine2 aFirstParCharTopBorder(16724787,0,37,0,2,37);
        const sal_Int32 aFirstParCharTopPadding(450);
        uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);

        // Top border
        assertEqualBorder(
            aFirstParCharTopBorder, aFirstParCharTopPadding,
            getProperty<table::BorderLine2>(xSet,"CharTopBorder"),
            getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

        // Bottom border (same as top border)
        assertEqualBorder(
            aFirstParCharTopBorder, aFirstParCharTopPadding,
            getProperty<table::BorderLine2>(xSet,"CharBottomBorder"),
            getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

        // Left border (same as top border)
        assertEqualBorder(
            aFirstParCharTopBorder, aFirstParCharTopPadding,
            getProperty<table::BorderLine2>(xSet,"CharLeftBorder"),
            getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

        // Right border (same as top border)
        assertEqualBorder(
            aFirstParCharTopBorder, aFirstParCharTopPadding,
            getProperty<table::BorderLine2>(xSet,"CharRightBorder"),
            getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,"CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(aShadow.Color, sal_Int32(16724787));
        CPPUNIT_ASSERT_EQUAL((bool)aShadow.IsTransparent, false);
        CPPUNIT_ASSERT_EQUAL(aShadow.Location, table::ShadowLocation(2));
        CPPUNIT_ASSERT_EQUAL(aShadow.ShadowWidth, sal_Int16(280));

        // Check autostyle
        {
            uno::Reference< style::XAutoStyleFamily > xAutoStyleFamily(getAutoStyles("ParagraphStyles"));
            uno::Reference < container::XEnumeration > xAutoStylesEnum( xAutoStyleFamily->createEnumeration() );
            CPPUNIT_ASSERT_EQUAL((bool)xAutoStylesEnum->hasMoreElements(), true);

            // First paragraph autostyle
            uno::Reference < beans::XPropertySet > xPSet( xAutoStylesEnum->nextElement(), uno::UNO_QUERY );

            // Top border
            assertEqualBorder(
                aFirstParCharTopBorder, aFirstParCharTopPadding,
                getProperty<table::BorderLine2>(xSet,"CharTopBorder"),
                getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

            // Bottom border
            assertEqualBorder(
                aFirstParCharTopBorder, aFirstParCharTopPadding,
                getProperty<table::BorderLine2>(xSet,"CharBottomBorder"),
                getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

            // Left border
            assertEqualBorder(
                aFirstParCharTopBorder, aFirstParCharTopPadding,
                getProperty<table::BorderLine2>(xSet,"CharLeftBorder"),
                getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

            // Right border
            assertEqualBorder(
                aFirstParCharTopBorder, aFirstParCharTopPadding,
                getProperty<table::BorderLine2>(xSet,"CharRightBorder"),
                getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));
        }
    }

    // Second paragraph's second text portion has a character style named CharDiffBor
    // This style includes border with different sides
    {

        table::BorderLine2 aBorderArray[4] =
        {
            table::BorderLine2(16724787,0,37,0,14,37),     // Top (fine dashed line)
            table::BorderLine2(10092390,26,26,53,11,106),  // Bottom
            table::BorderLine2(6711039,9,26,9,12,71),      // Left
            table::BorderLine2(0,0,0,0,0,0)                // Right
        };

        sal_Int32 aDistances[4] = { 400 /*Top*/, 300 /*Bottom*/, 250 /*Left*/, 0 /*Right*/ };

        // Get second text portion of second paragraph
        uno::Reference < beans::XPropertySet > xSet( getRun(getParagraph(2),2), uno::UNO_QUERY );

        // Top border
        assertEqualBorder(
            aBorderArray[0], aDistances[0],
            getProperty<table::BorderLine2>(xSet,"CharTopBorder"),
            getProperty<sal_Int32>(xSet,"CharTopBorderDistance"));

        // Bottom border
        assertEqualBorder(
            aBorderArray[1], aDistances[1],
            getProperty<table::BorderLine2>(xSet,"CharBottomBorder"),
            getProperty<sal_Int32>(xSet,"CharBottomBorderDistance"));

        // Left border
        assertEqualBorder(
            aBorderArray[2], aDistances[2],
            getProperty<table::BorderLine2>(xSet,"CharLeftBorder"),
            getProperty<sal_Int32>(xSet,"CharLeftBorderDistance"));

        // Right border
        assertEqualBorder(
            aBorderArray[3], aDistances[3],
            getProperty<table::BorderLine2>(xSet,"CharRightBorder"),
            getProperty<sal_Int32>(xSet,"CharRightBorderDistance"));

        // Shadow
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xSet,"CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(aShadow.Color, sal_Int32(0));
        CPPUNIT_ASSERT_EQUAL((bool)aShadow.IsTransparent, false);
        CPPUNIT_ASSERT_EQUAL(aShadow.Location, table::ShadowLocation(3));
        CPPUNIT_ASSERT_EQUAL(aShadow.ShadowWidth, sal_Int16(79));

        // Check character style
        {
            uno::Reference< container::XNameAccess > xStyleFamily(getStyles("CharacterStyles"), uno::UNO_QUERY);
            uno::Reference < beans::XPropertySet > xStyleSet(xStyleFamily->getByName("CharDiffBor"), uno::UNO_QUERY);

            // Top border
            assertEqualBorder(
                aBorderArray[0], aDistances[0],
                getProperty<table::BorderLine2>(xStyleSet,"CharTopBorder"),
                getProperty<sal_Int32>(xStyleSet,"CharTopBorderDistance"));

            // Bottom border
            assertEqualBorder(
                aBorderArray[1], aDistances[1],
                getProperty<table::BorderLine2>(xStyleSet,"CharBottomBorder"),
                getProperty<sal_Int32>(xStyleSet,"CharBottomBorderDistance"));

            // Left border
            assertEqualBorder(
                aBorderArray[2], aDistances[2],
                getProperty<table::BorderLine2>(xStyleSet,"CharLeftBorder"),
                getProperty<sal_Int32>(xStyleSet,"CharLeftBorderDistance"));

            // Right border
            assertEqualBorder(
                aBorderArray[3], aDistances[3],
                getProperty<table::BorderLine2>(xStyleSet,"CharRightBorder"),
                getProperty<sal_Int32>(xStyleSet,"CharRightBorderDistance"));
        }
    }
}

void Test::testFdo43807()
{
    uno::Reference<beans::XPropertySet> xSet(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Drop Caps"),getProperty<OUString>(xSet,"DropCapCharStyleName"));

    xSet = uno::Reference<beans::XPropertySet>(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("User Defined Drop Caps"),getProperty<OUString>(xSet,"DropCapCharStyleName"));
}

void Test::testTextframeTransparentShadow()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> xPicture(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // ODF stores opacity of 75%, that means 25% transparency.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<sal_Int32>(xPicture, "ShadowTransparence"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
