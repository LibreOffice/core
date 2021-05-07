/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/storagehelper.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/tempfile.hxx>
#include <osl/thread.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/outdev.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers embeddedobj/source/msole/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
};
}

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void Test::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
}

class OdtExportThread : public osl::Thread
{
    uno::Reference<lang::XComponent> mxComponent;
    OUString maURL;

public:
    OdtExportThread(const uno::Reference<lang::XComponent>& xComponent, const OUString& rURL);
    virtual void SAL_CALL run() override;
};

OdtExportThread::OdtExportThread(const uno::Reference<lang::XComponent>& xComponent,
                                 const OUString& rURL)
    : mxComponent(xComponent)
    , maURL(rURL)
{
}

void OdtExportThread::run()
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("writer8")),
    };
    xStorable->storeToURL(maURL, aStoreProperties);
}

CPPUNIT_TEST_FIXTURE(Test, testSaveOnThread)
{
    // Given an embedded object which hosts mspaint data:
    if (Application::GetDefaultDevice()->GetDPIX() != 96)
    {
        return;
    }

    DBG_TESTSOLARMUTEX();
    OUString aURL = m_directories.getURLFromSrc(u"embeddedobj/qa/cppunit/data/reqif-ole2.xhtml");
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    getComponent().set(loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties));

    // When saving that document on a thread:
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OdtExportThread aThread(getComponent(), aTempFile.GetURL());
    aThread.create();
    {
        SolarMutexReleaser r;
        while (aThread.isRunning())
        {
            SolarMutexGuard g;
            Application::Reschedule(/*bHandleAllCurrentEvents=*/true);
        }
    }

    // Then make sure its visible area's width is correct.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("content.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // 16 pixels, assuming 96 DPI.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0.1665in
    // - Actual  : 1.9685in
    // i.e. we wrote a hardcoded 5cm width, not the real one.
    assertXPath(pXmlDoc, "//style:graphic-properties", "visible-area-width", "0.1665in");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
