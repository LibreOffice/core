/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>

#include <swmodeltestbase.hxx>

class Test : public SwModelTestBase
{
public:
    void testN325936();
    void testFdo45724();
    void testFdo46020();
    void testFirstHeaderFooter();
    void testZoom();
    void test56513();
    void testNewPageStylesTable();

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
        {"n325936.doc", &Test::testN325936},
        {"fdo45724.odt", &Test::testFdo45724},
        {"fdo46020.odt", &Test::testFdo46020},
        {"first-header-footer.doc", &Test::testFirstHeaderFooter},
        {"zoom.doc", &Test::testZoom},
        {"fdo56513.doc", &Test::test56513},
        {"new-page-styles.doc", &Test::testNewPageStylesTable},
    };
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        load("/sw/qa/extras/ww8export/data/", rEntry.pName);
        // If the testcase is stored in some other format, it's pointless to test.
        if (OString(rEntry.pName).endsWith(".doc"))
            (this->*rEntry.pMethod)();
        reload("MS Word 97");
        (this->*rEntry.pMethod)();
        finish();
    }
}

void Test::testN325936()
{
    /*
     * The problem was that the transparent background of the drawing in the
     * header was exported as non-transparent.
     *
     * xray ThisComponent.DrawPage(0).BackColorTransparency
     */

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue = getProperty< sal_Int32 >(xDraws->getByIndex(0), "BackColorTransparency");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);
}

void Test::testFdo45724()
{
    // The text and background color of the control shape was not correct.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XControlShape> xControlShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    // This should not happen (the shape was there before the fix as well), but let's avoid a segfault in some random cases...
    if (xControlShape.is())
    {
        uno::Reference<form::validation::XValidatableFormComponent> xComponent(xControlShape->getControl(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<sal_uInt32>(xComponent, "BackgroundColor"));
        CPPUNIT_ASSERT_EQUAL(OUString("xxx"), xComponent->getCurrentValue().get<OUString>());
    }
}

void Test::testFdo46020()
{
    // The footnote in that document wasn't exported, check that it is actually exported
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

void Test::testFirstHeaderFooter()
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

void Test::testZoom()
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

void Test::test56513()
{
    CPPUNIT_ASSERT_EQUAL(OUString("This is the header of the first section"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header of the second section"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page header of the second section"),  parseDump("/root/page[3]/header/txt/text()"));
}

void Test::testNewPageStylesTable()
{
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals and Results (Page 1)*"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Assessment (Page 2)****"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals: Next Year (Page 3)*******"),  parseDump("/root/page[3]/header/txt/text()"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
