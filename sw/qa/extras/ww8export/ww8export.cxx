/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97") {}

    bool mustTestImportOf(const char* filename) const
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }
};

#define DECLARE_WW8EXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, Test)

DECLARE_WW8EXPORT_TEST(testN325936, "n325936.doc")
{
    /*
     * The problem was that the transparent background of the drawing in the
     * header was exported as non-transparent.
     *
     * xray ThisComponent.DrawPage(0).BackColorTransparency
     */

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    sal_Int32 nValue = getProperty< sal_Int32 >(getShape(1), "BackColorTransparency");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);
}

DECLARE_WW8EXPORT_TEST(testFdo45724, "fdo45724.odt")
{
    // The text and background color of the control shape was not correct.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<form::validation::XValidatableFormComponent> xComponent(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<sal_uInt32>(xComponent, "BackgroundColor"));
    CPPUNIT_ASSERT_EQUAL(OUString("xxx"), xComponent->getCurrentValue().get<OUString>());
}

DECLARE_WW8EXPORT_TEST(testFdo46020, "fdo46020.odt")
{
    // The footnote in that document wasn't exported, check that it is actually exported
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

DECLARE_WW8EXPORT_TEST(testFirstHeaderFooter, "first-header-footer.doc")
{
    // Test import and export of a section's headerf/footerf properties.

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),  parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),   parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),  parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"), parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"), parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"), parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"), parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),  parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),  parseDump("/root/page[6]/footer/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testZoom, "zoom.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_WW8EXPORT_TEST(test56513, "fdo56513.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("This is the header of the first section"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header of the second section"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page header of the second section"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testNewPageStylesTable, "new-page-styles.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals and Results (Page 1)*"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Assessment (Page 2)****"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals: Next Year (Page 3)*******"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testFdo42144, "fdo42144.odt")
{
    // Footer wasn't disabled -- instead empty footer was exported.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(xStyle, "FooterIsOn")));
}

DECLARE_WW8EXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // WW8 has just one border attribute (sprmCBrc) for text border so all side has
    // the same border
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF3333,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Padding (dptSpace) it is constant 0
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // Shadow (fShadow)
    /* WW8 use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

DECLARE_WW8EXPORT_TEST(testFdo59530, "fdo59530.doc")
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

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));

    // Test commented text range which spans over more text nodes
    // Comment starts in the second paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    // Comment ends in the third paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));
}

DECLARE_WW8EXPORT_TEST(testCommentsNested, "comments-nested.doc")
{
    uno::Reference<beans::XPropertySet> xOuter(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content"));

    uno::Reference<beans::XPropertySet> xInner(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content"));
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
