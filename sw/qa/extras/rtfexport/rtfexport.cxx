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

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;
using namespace com::sun::star;

class RtfExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp();
    virtual void tearDown();
    void testZoom();
    void testFdo38176();

    CPPUNIT_TEST_SUITE(RtfExportTest);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testZoom);
    CPPUNIT_TEST(testFdo38176);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void roundtrip(const OUString& rURL);
    /// Get the length of the whole document.
    int getLength();
    uno::Reference<lang::XComponent> mxComponent;
};

void RtfExportTest::roundtrip(const OUString& rFilename)
{
    uno::Reference<lang::XComponent> xImported = loadFromDesktop(getURLFromSrc("/sw/qa/extras/rtfexport/data/") + rFilename);
    uno::Reference<frame::XStorable> xStorable(xImported, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= OUString("Rich Text Format");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);
    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

int RtfExportTest::getLength()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    OUStringBuffer aBuf;
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
            aBuf.append(xRange->getString());
        }
    }
    return aBuf.getLength();
}

void RtfExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(getMultiServiceFactory()->createInstance("com.sun.star.frame.Desktop"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(mxDesktop.is());
}

void RtfExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void RtfExportTest::testZoom()
{
    roundtrip("zoom.rtf");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

void RtfExportTest::testFdo38176()
{
    roundtrip("fdo38176.rtf");
    CPPUNIT_ASSERT_EQUAL(9, getLength());
}

CPPUNIT_TEST_SUITE_REGISTRATION(RtfExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
