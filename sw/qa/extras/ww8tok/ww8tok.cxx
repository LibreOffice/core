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

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <vcl/svapp.hxx>

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;
using namespace com::sun::star;

class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp();
    virtual void tearDown();
    void testN757910();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testN757910);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load a WW8 file and make the document available via mxComponent.
    void load(const OUString& rURL);
    uno::Reference<lang::XComponent> mxComponent;
};

void Test::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/ww8tok/data/") + rFilename);
}

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(getMultiServiceFactory()->createInstance("com.sun.star.frame.Desktop"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(mxDesktop.is());
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void Test::testN757910()
{
    load("n757910.doc");

    // The internal margin was larger than 0.28cm
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftBorderDistance") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(280), nValue);

    // The border width was zero
    table::BorderLine2 aBorder;
    xPropertySet->getPropertyValue("LeftBorder") >>= aBorder;
    CPPUNIT_ASSERT(aBorder.LineWidth > 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
