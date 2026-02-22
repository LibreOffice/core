/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/util/XCloseable.hpp>

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

#ifdef _WIN32
#include <systools/win32/comtools.hxx>
#endif

using namespace ::com::sun::star;

namespace
{
/// Covers embeddedobj/source/msole/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/embeddedobj/qa/cppunit/data/")
    {
    }
};

bool IsPaintClassNotRegistered()
{
#ifdef _WIN32
    sal::systools::CoInitializeGuard g(0);
    // Check if MS Paint's {0003000A-0000-0000-C000-000000000046} is registered
    CLSID clsidPaint{ 0x0003000A, 0000, 0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
    LPOLESTR pProgId = nullptr;
    if (ProgIDFromCLSID(clsidPaint, &pProgId) == REGDB_E_CLASSNOTREG)
        return true;
    CoTaskMemFree(pProgId);
#endif
    return false;
}
}

namespace
{
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
}

CPPUNIT_TEST_FIXTURE(Test, testSaveOnThread)
{
    // Given an embedded object which hosts mspaint data:
    if (Application::GetDefaultDevice()->GetDPIX() != 96)
    {
        return;
    }

    if (IsPaintClassNotRegistered())
        return;

    DBG_TESTSOLARMUTEX();
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    loadFromFile(u"reqif-ole2.xhtml", aLoadProperties);

    // When saving that document on a thread:
    OdtExportThread aThread(mxComponent, maTempFile.GetURL());
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
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // 16 pixels, assuming 96 DPI.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0.1665in
    // - Actual  : 1.9685in
    // i.e. we wrote a hardcoded 5cm width, not the real one.
    assertXPath(pXmlDoc, "//style:graphic-properties", "visible-area-width", u"0.1665in");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
