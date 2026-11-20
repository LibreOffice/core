/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <memory>
#include <string>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/kit.hxx>
#include <comphelper/scopeguard.hxx>
#include <cppunit/TestAssert.h>
#include <test/unoapi_test.hxx>
#include <vcl/scheduler.hxx>

#include <lib/init.hxx>

using namespace com::sun::star;
using namespace ::cpo;
using namespace desktop;

/// Covers desktop/source/lib/kitclipboard.cxx fixes.
class KitClipboardTest : public UnoApiTest
{
public:
    KitClipboardTest() : UnoApiTest(u"/desktop/qa/data/"_ustr) {}

    virtual void setUp() override
    {
        comphelper::COKit::setActive(true);
        UnoApiTest::setUp();
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            cpo::uno::Reference<util::XCloseable> xCloseable(mxComponent, cpo::uno::UNO_QUERY_THROW);
            xCloseable->close(false);
            mxComponent.clear();
        }
        test::BootstrapFixture::tearDown();
        comphelper::COKit::setActive(false);
    }

protected:
    /// Loads a blank Impress document with a plain graphic shape selected.
    std::unique_ptr<COKitDocumentImpl> loadImpressWithSelectedShape();
};

std::unique_ptr<COKitDocumentImpl>
KitClipboardTest::loadImpressWithSelectedShape()
{
    // Reset the doc id, so the order of test cases in the suite doesn't matter.
    const int nDocId = 0;
    comphelper::COKit::setDocId(ViewShellDocId(nDocId));
    mxComponent = loadFromDesktop(createFileURL(u"blank_presentation.odp"),
                                  u"com.sun.star.presentation.PresentationDocument"_ustr);
    auto pDocument = std::make_unique<COKitDocumentImpl>(mxComponent, nDocId);
    pDocument->initializeForRendering("{}");
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(
        xSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr),
        uno::UNO_QUERY);
    xPage->add(xShape);
    xShape->setSize(awt::Size(5000, 3000));
    xShape->setPosition(awt::Point(1000, 1000));
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelection(
        xModel->getCurrentController(), uno::UNO_QUERY);
    xSelection->select(cpo::uno::Any(xShape));
    Scheduler::ProcessEventsToIdle();
    return pDocument;
}

namespace
{
// Records the MIME types the engine asks the platform to advertise, so a test
// can see what a copy offered to the platform clipboard.
std::vector<std::string> g_aShapeAdvertise;

void shapeAdvertiseRecord(const char** pMimeTypes)
{
    g_aShapeAdvertise.clear();
    for (size_t i = 0; pMimeTypes && pMimeTypes[i]; ++i)
        g_aShapeAdvertise.emplace_back(pMimeTypes[i]);
}

bool shapeAdvertiseOwns() { return false; }
std::vector<std::string> shapeAdvertiseMimeTypes() { return {}; }
bool shapeAdvertiseData(const char*, std::vector<char>*) { return false; }

const COKitClipboardProvider aShapeAdvertiseProvider = { &shapeAdvertiseRecord,
                                                         &shapeAdvertiseOwns,
                                                         &shapeAdvertiseMimeTypes,
                                                         &shapeAdvertiseData };
}

CPPUNIT_TEST_FIXTURE(KitClipboardTest, testClipboardShapeAdvertisesHtml)
{
    // The provider goes in before the document loads: the engine calls
    // advertiseToPlatform on every clipboard change, and the record tells the
    // test what the kit offered to the platform for a shape copy.
    COKitImpl aOffice;
    aOffice.installClipboardProvider(&aShapeAdvertiseProvider);
    comphelper::ScopeGuard aRestore(
        [&aOffice] { aOffice.installClipboardProvider(nullptr); });

    // Given an Impress document with a plain graphic shape selected:
    auto pDocument = loadImpressWithSelectedShape();

    // When copying it to the clipboard:
    g_aShapeAdvertise.clear();
    pDocument->postUnoCommand(".uno:Copy", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    // Then make sure text/html is advertised as a derived flavor from image/png,
    // so an HTML-aware paste target gets the shape as an inline image:
    const bool bHasPng = std::find(g_aShapeAdvertise.begin(), g_aShapeAdvertise.end(),
                                   "image/png")
                         != g_aShapeAdvertise.end();
    const bool bHasHtml = std::find(g_aShapeAdvertise.begin(), g_aShapeAdvertise.end(),
                                    "text/html")
                          != g_aShapeAdvertise.end();
    CPPUNIT_ASSERT(bHasPng);
    // Without the accompanying fix in place, this test would have failed, no text/html
    // was advertised for a shape selection.
    CPPUNIT_ASSERT(bHasHtml);
}

CPPUNIT_TEST_FIXTURE(KitClipboardTest, testClipboardShapeHtmlHasCoolXml)
{
    // Given an Impress document with a plain graphic shape selected:
    auto pDocument = loadImpressWithSelectedShape();

    // When copying it to the clipboard:
    pDocument->postUnoCommand(".uno:Copy", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    // Then make sure the HTML <img> of the shape also contains ODF:
    const char* pMimeTypes[] = { "text/html", nullptr };
    const std::vector<COKitClipboardItem> aItems = pDocument->getClipboard(pMimeTypes);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aItems.size());
    CPPUNIT_ASSERT_EQUAL(std::string("text/html"), aItems[0].aMimeType);
    const OString aHtml(aItems[0].aData.data(), aItems[0].aData.size());
    CPPUNIT_ASSERT(!aHtml.isEmpty());
    // Without the accompanying fix in place, this test would have failed, the
    // ODF package of the shape was dropped when the HTML <img> was built.
    CPPUNIT_ASSERT(aHtml.indexOf(" data-coolxml=\"") != -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
