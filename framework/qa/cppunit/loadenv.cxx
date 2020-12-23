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

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers framework/source/loadenv/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    void setUp() override;
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

OUStringLiteral const DATA_DIRECTORY = u"/framework/qa/cppunit/data/";

class DocumentOpener
{
public:
    DECL_STATIC_LINK(DocumentOpener, OpenDocument, void*, void);
};

IMPL_STATIC_LINK(DocumentOpener, OpenDocument, void*, pArg, void)
{
    CPPUNIT_ASSERT(pArg);
    auto pURL = static_cast<OUString*>(pArg);
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getProcessComponentContext();
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xComponentContext);
    xDesktop->loadComponentFromURL(*pURL, "_default", 0, {});
    delete pURL;
}

CPPUNIT_TEST_FIXTURE(Test, testDoubleLoading)
{
    // Try to load the same document twice. This is similar to trying to execute the soffice process
    // twice: in that case the 2nd instance forwards to the 1st instance and then uses the same code
    // path.
    for (int i = 0; i < 2; ++i)
    {
        auto pURL = std::make_unique<OUString>(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                               + "double-loading.odt");
        Application::PostUserEvent(LINK(nullptr, DocumentOpener, OpenDocument), pURL.release());
    }
    Scheduler::ProcessEventsToIdle();

    // Verify that the 2nd load didn't happen, since it's the same document.
    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    // Without the accompanying fix in place, this failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the document was loaded twice, into two separate frames/windows.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xFrames->getCount());

    // Close the document, now that we know we have a single one.
    uno::Reference<frame::XFrame> xFrame(xFrames->getByIndex(0), uno::UNO_QUERY);
    xFrame->getController()->getModel()->dispose();
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
