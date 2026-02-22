/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_validation.h>

#include <test/bootstrapfixture.hxx>
#include <vcl/errinf.hxx>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>

#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <tools/link.hxx>
#include <vcl/graphicfilter.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <unotools/tempfile.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/virdev.hxx>
#include <o3tl/string_view.hxx>

#include <memory>
#include <cstring>

#include "setupvcl.hxx"

using namespace ::com::sun::star;

static void aBasicErrorFunc( const OUString &rErr, const OUString &rAction )
{
    OString aErr = "Unexpected dialog: " +
        OUStringToOString( rAction, RTL_TEXTENCODING_ASCII_US ) +
        " Error: " +
        OUStringToOString( rErr, RTL_TEXTENCODING_ASCII_US );
    CPPUNIT_ASSERT_MESSAGE( aErr.getStr(), false);
}

// NB. this constructor is called before any tests are run, once for each
// test function in a rather non-intuitive way. This is why all the 'real'
// heavy lifting is deferred until setUp. setUp and tearDown are interleaved
// between the tests as you might expect.
test::BootstrapFixture::BootstrapFixture( bool bAssertOnDialog, bool bNeedUCB )
    : m_bNeedUCB( bNeedUCB )
    , m_bAssertOnDialog( bAssertOnDialog )
{
}

extern "C"
{

static void test_init_impl(bool bAssertOnDialog, bool bNeedUCB,
        lang::XMultiServiceFactory * pSFactory)
{
    if (bAssertOnDialog)
        ErrorRegistry::RegisterDisplay( aBasicErrorFunc );

    // Make GraphicConverter work, normally done in desktop::Desktop::Main()
    Application::SetFilterHdl(
            LINK(nullptr, test::BootstrapFixture, ImplInitFilterHdl));

    if (bNeedUCB)
    {
        // initialise unconfigured UCB:
        uno::Reference<ucb::XUniversalContentBroker> xUcb(pSFactory->createInstance(u"com.sun.star.ucb.UniversalContentBroker"_ustr), uno::UNO_QUERY_THROW);
        uno::Reference<ucb::XContentProvider> xFileProvider(pSFactory->createInstance(u"com.sun.star.ucb.FileContentProvider"_ustr), uno::UNO_QUERY_THROW);
        xUcb->registerContentProvider(xFileProvider, u"file"_ustr, true);
        uno::Reference<ucb::XContentProvider> xTdocProvider(pSFactory->createInstance(u"com.sun.star.ucb.TransientDocumentsContentProvider"_ustr), uno::UNO_QUERY);
        if (xTdocProvider.is())
        {
            xUcb->registerContentProvider(xTdocProvider, u"vnd.sun.star.tdoc"_ustr, true);
        }
    }
}

// this is called from pyuno
SAL_DLLPUBLIC_EXPORT void test_init(lang::XMultiServiceFactory *pFactory)
{
    try
    {
        ::comphelper::setProcessServiceFactory(pFactory);
        test::setUpVcl(true); // hard-code python tests to headless
        test_init_impl(false, true, pFactory);
    }
    catch (...) { abort(); }
}

// this is called from pyuno
SAL_DLLPUBLIC_EXPORT void test_deinit()
{
    DeInitVCL();
}

} // extern "C"

void test::BootstrapFixture::setUp()
{
    test::BootstrapFixtureBase::setUp();

    test_init_impl(m_bAssertOnDialog, m_bNeedUCB, m_xSFactory.get());

#if OSL_DEBUG_LEVEL > 0
    Scheduler::ProcessEventsToIdle();
#endif
}

test::BootstrapFixture::~BootstrapFixture()
{
}

IMPL_STATIC_LINK(
        test::BootstrapFixture, ImplInitFilterHdl, ConvertData&, rData, bool)
{
    return GraphicFilter::GetGraphicFilter().GetFilterCallback().Call( rData );
}

bool test::BootstrapFixture::IsDefaultDPI()
{
    return (Application::GetDefaultDevice()->GetDPIX() == 96
            && Application::GetDefaultDevice()->GetDPIY() == 96);
}

std::pair<double, double> test::BootstrapFixture::getDPIScaling()
{
    return { Application::GetDefaultDevice()->GetDPIX() / 96.0,
             Application::GetDefaultDevice()->GetDPIY() / 96.0 };
}

sal_uInt16 test::BootstrapFixture::getDefaultDeviceBitCount()
{
    ScopedVclPtr<VirtualDevice> device
        = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    return device->GetBitCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
