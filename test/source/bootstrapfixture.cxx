/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <test/bootstrapfixture.hxx>
#include <tools/errinf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>

#include <vcl/svapp.hxx>
#include <tools/resmgr.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>

#include <boost/scoped_array.hpp>
#include <cstring>

using namespace ::com::sun::star;

static void aBasicErrorFunc( const OUString &rErr, const OUString &rAction )
{
    OStringBuffer aErr( "Unexpected dialog: " );
    aErr.append( OUStringToOString( rAction, RTL_TEXTENCODING_ASCII_US ) );
    aErr.append( " Error: " );
    aErr.append( OUStringToOString( rErr, RTL_TEXTENCODING_ASCII_US ) );
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

void test_init_impl(bool bAssertOnDialog, bool bNeedUCB,
        lang::XMultiServiceFactory * pSFactory)
{
    // force locale (and resource files loaded) to en-US
    OUString aLangISO( "en-US" );
    ResMgr::SetDefaultLocale( LanguageTag( aLangISO) );

    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetLocaleConfigString( aLangISO );
    aLocalOptions.SetUILocaleConfigString( aLangISO );

    InitVCL();
    if (Application::IsHeadlessModeRequested())
        Application::EnableHeadlessMode(true);

    if (bAssertOnDialog)
        ErrorHandler::RegisterDisplay( aBasicErrorFunc );

    // Make GraphicConverter work, normally done in desktop::Desktop::Main()
    Application::SetFilterHdl(
            STATIC_LINK(0, test::BootstrapFixture, ImplInitFilterHdl));

    if (bNeedUCB)
    {
        // initialise unconfigured UCB:
        uno::Reference<ucb::XUniversalContentBroker> xUcb(pSFactory->createInstance("com.sun.star.ucb.UniversalContentBroker"), uno::UNO_QUERY_THROW);
        uno::Reference<ucb::XContentProvider> xFileProvider(pSFactory->createInstance("com.sun.star.ucb.FileContentProvider"), uno::UNO_QUERY_THROW);
        xUcb->registerContentProvider(xFileProvider, "file", sal_True);
        uno::Reference<ucb::XContentProvider> xTdocProvider(pSFactory->createInstance("com.sun.star.ucb.TransientDocumentsContentProvider"), uno::UNO_QUERY);
        if (xTdocProvider.is())
        {
            xUcb->registerContentProvider(xTdocProvider, "vnd.sun.star.tdoc", sal_True);
        }
    }
}

// this is called from pyuno
SAL_DLLPUBLIC_EXPORT void test_init(lang::XMultiServiceFactory *pFactory)
{
    try
    {
        ::comphelper::setProcessServiceFactory(pFactory);
        test_init_impl(false, true, pFactory);
    }
    catch (...) { abort(); }
}

} // extern "C"

void test::BootstrapFixture::setUp()
{
    test::BootstrapFixtureBase::setUp();

    test_init_impl(m_bAssertOnDialog, m_bNeedUCB, m_xSFactory.get());
}

void test::BootstrapFixture::tearDown()
{
    test::BootstrapFixtureBase::tearDown();
}

test::BootstrapFixture::~BootstrapFixture()
{
}

namespace {

OString loadFile(const OUString& rURL)
{
    osl::File aFile(rURL);
    osl::FileBase::RC eStatus = aFile.open(osl_File_OpenFlag_Read);
    CPPUNIT_ASSERT_EQUAL(eStatus, osl::FileBase::E_None);
    sal_uInt64 nSize;
    aFile.getSize(nSize);
    boost::scoped_array<char> aBytes(new char[nSize]);
    sal_uInt64 nBytesRead;
    aFile.read(aBytes.get(), nSize, nBytesRead);
    CPPUNIT_ASSERT_EQUAL(nSize, nBytesRead);
    OString aContent(aBytes.get());

    return aContent;
}

}

void test::BootstrapFixture::validate(const OUString& rPath, test::ValidationFormat eFormat )
{
    (void)rPath;
    (void)eFormat;

#if HAVE_EXPORT_VALIDATION
    OUString aValidator;
    if( eFormat == test::OOXML )
    {
        aValidator = "officeotron ";
    }
    else
    {
        aValidator = "odfvalidator ";
    }

    utl::TempFile aOutput;
    aOutput.EnableKillingFile();
    OUString aOutputFile = aOutput.GetFileName();
    OUString aCommand = aValidator + rPath + " > " + aOutputFile;

    system(OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8).getStr());

    OString aContentString = loadFile(aOutput.GetURL());
    OUString aContentOUString = OStringToOUString(aContentString, RTL_TEXTENCODING_UTF8);

    if( eFormat == test::OOXML && !aContentOUString.isEmpty() )
    {
        // check for validation errors here
        sal_Int32 nIndex = aContentOUString.lastIndexOf("Grand total of errors in submitted package: ");
        if(nIndex == -1)
        {
            SAL_WARN("test", "no summery line");
        }
        else
        {
            sal_Int32 nStartOfNumber = nIndex + std::strlen("Grand total of errors in submitted package: ");
            OUString aNumber = aContentOUString.copy(nStartOfNumber);
            sal_Int32 nErrors = aNumber.toInt32();
            OString aMsg("validation error in OOXML export: Errors: ");
            aMsg = aMsg + OString::number(nErrors);
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
            CPPUNIT_FAIL("validation errors during export");
        }
    }
#endif
}

IMPL_STATIC_LINK_NOINSTANCE(
        test::BootstrapFixture, ImplInitFilterHdl, ConvertData*, pData)
{
    return GraphicFilter::GetGraphicFilter().GetFilterCallback().Call( pData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
