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

#include "../swmodeltestbase.hxx"

#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <unotools/tempfile.hxx>

using rtl::OUString;

class Test : public SwModelTestBase
{
public:
    void testN325936();
    void testFdo45724();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testN325936);
    CPPUNIT_TEST(testFdo45724);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void roundtrip(const OUString& rURL);
};

void Test::roundtrip(const OUString& rFilename)
{
    uno::Reference<lang::XComponent> xImported = loadFromDesktop(getURLFromSrc("/sw/qa/extras/ww8export/data/") + rFilename);
    uno::Reference<frame::XStorable> xStorable(xImported, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= OUString("MS Word 97");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);
    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

void Test::testN325936()
{
    /*
     * The problem was that the transparent background of the drawing in the
     * header was exported as non-transparent.
     *
     * xray ThisComponent.DrawPage(0).BackColorTransparency
     */

    roundtrip("n325936.doc");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue = getProperty< sal_Int32 >(xDraws->getByIndex(0), "BackColorTransparency");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);
}

void Test::testFdo45724()
{
    roundtrip("fdo45724.odt");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XControlShape> xControlShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<form::validation::XValidatableFormComponent> xComponent(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<sal_uInt32>(xComponent, "BackgroundColor"));
    CPPUNIT_ASSERT_EQUAL(OUString("xxx"), xComponent->getCurrentValue().get<OUString>());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
