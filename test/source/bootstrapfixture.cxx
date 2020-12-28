/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

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
#include <vcl/scheduler.hxx>

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
        uno::Reference<ucb::XUniversalContentBroker> xUcb(pSFactory->createInstance("com.sun.star.ucb.UniversalContentBroker"), uno::UNO_QUERY_THROW);
        uno::Reference<ucb::XContentProvider> xFileProvider(pSFactory->createInstance("com.sun.star.ucb.FileContentProvider"), uno::UNO_QUERY_THROW);
        xUcb->registerContentProvider(xFileProvider, "file", true);
        uno::Reference<ucb::XContentProvider> xTdocProvider(pSFactory->createInstance("com.sun.star.ucb.TransientDocumentsContentProvider"), uno::UNO_QUERY);
        if (xTdocProvider.is())
        {
            xUcb->registerContentProvider(xTdocProvider, "vnd.sun.star.tdoc", true);
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

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
}

test::BootstrapFixture::~BootstrapFixture()
{
}

#if HAVE_EXPORT_VALIDATION
namespace {

OString loadFile(const OUString& rURL)
{
    osl::File aFile(rURL);
    osl::FileBase::RC eStatus = aFile.open(osl_File_OpenFlag_Read);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, eStatus);
    sal_uInt64 nSize;
    aFile.getSize(nSize);
    std::unique_ptr<char[]> aBytes(new char[nSize]);
    sal_uInt64 nBytesRead;
    aFile.read(aBytes.get(), nSize, nBytesRead);
    CPPUNIT_ASSERT_EQUAL(nSize, nBytesRead);
    OString aContent(aBytes.get(), nBytesRead);

    return aContent;
}

}
#endif

void test::BootstrapFixture::validate(const OUString& rPath, test::ValidationFormat eFormat) const
{
#if HAVE_EXPORT_VALIDATION
    OUString var;
    if( eFormat == test::OOXML )
    {
        var = "OFFICEOTRON";
    }
    else if ( eFormat == test::ODF )
    {
        var = "ODFVALIDATOR";
    }
    else if ( eFormat == test::MSBINARY )
    {
#if HAVE_BFFVALIDATOR
        var = "BFFVALIDATOR";
#else
        // Binary Format Validator is disabled
        return;
#endif
    }
    OUString aValidator;
    oslProcessError e = osl_getEnvironment(var.pData, &aValidator.pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OUString("cannot get env var " + var).toUtf8().getStr(),
        osl_Process_E_None, e);
    CPPUNIT_ASSERT_MESSAGE(
        OUString("empty get env var " + var).toUtf8().getStr(),
        !aValidator.isEmpty());

    if (eFormat == test::ODF)
    {
        // invoke without -e so that we know when something new is written
        // in loext namespace that isn't yet in the custom schema
        aValidator += " -M "
            + m_directories.getPathFromSrc(u"/schema/libreoffice/OpenDocument-manifest-schema-v1.3+libreoffice.rng")
            + " -D "
            + m_directories.getPathFromSrc(u"/schema/libreoffice/OpenDocument-dsig-schema-v1.3+libreoffice.rng")
            + " -O "
            + m_directories.getPathFromSrc(u"/schema/libreoffice/OpenDocument-schema-v1.3+libreoffice.rng")
            + " -m "
            + m_directories.getPathFromSrc(u"/schema/mathml2/mathml2.xsd");
    }

    utl::TempFile aOutput;
    aOutput.EnableKillingFile();
    OUString aOutputFile = aOutput.GetFileName();
    OUString aCommand = aValidator + " " + rPath + " > " + aOutputFile;

    SAL_INFO("test", "BootstrapFixture::validate: executing '" << aCommand << "'");
    int returnValue = system(OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8).getStr());

    OString aContentString = loadFile(aOutput.GetURL());
    OUString aContentOUString = OStringToOUString(aContentString, RTL_TEXTENCODING_UTF8);

    if( eFormat == test::OOXML && !aContentOUString.isEmpty() )
    {
        // check for validation errors here
        sal_Int32 nIndex = aContentOUString.lastIndexOf("Grand total of errors in submitted package: ");
        if(nIndex == -1)
        {
            SAL_WARN("test", "no summary line");
        }
        else
        {
            sal_Int32 nStartOfNumber = nIndex + std::strlen("Grand total of errors in submitted package: ");
            OUString aNumber = aContentOUString.copy(nStartOfNumber);
            sal_Int32 nErrors = aNumber.toInt32();
            OString aMsg = "validation error in OOXML export: Errors: " + OString::number(nErrors);
            if(nErrors)
            {
                SAL_WARN("test", aContentOUString);
            }
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(), sal_Int32(0), nErrors);
        }
    }
    else if( eFormat == test::ODF && !aContentOUString.isEmpty() )
    {
        if( aContentOUString.indexOf("Error") != -1 )
        {
            SAL_WARN("test", aContentOUString);
            CPPUNIT_FAIL(aContentString.getStr());
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString(
            "failed to execute: " + OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8) + "\n"
            + OUStringToOString(aContentOUString, RTL_TEXTENCODING_UTF8)).getStr(),
        0, returnValue);
#else
    (void)rPath;
    (void)eFormat;
#endif
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
