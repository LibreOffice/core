/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *          Markus Mohrhard <markus.mohrhard@googlemail.com>
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Michael Meeks <michael.meeks@suse.com>
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include <svx/svdtext.hxx>
#include <svx/svdotext.hxx>

#include "drawdoc.hxx"
#include "../source/ui/inc/DrawDocShell.hxx"

#include <osl/process.h>
#include <osl/thread.h>

#include <string>
#include <iostream>
#include <rtl/oustringostreaminserter.hxx>

namespace {

bool compareFiles( const rtl::OUString& aFileNameOne, const rtl::OUString& aFileNameTwo)
{
    rtl::OString aOFileNameOne = rtl::OUStringToOString(aFileNameOne, RTL_TEXTENCODING_UTF8);
    std::ifstream aFileOne(aOFileNameOne.getStr());
    rtl::OString aOFileNameTwo = rtl::OUStringToOString(aFileNameTwo, RTL_TEXTENCODING_UTF8);
    std::ifstream aFileTwo(aOFileNameTwo.getStr());

    CPPUNIT_ASSERT_MESSAGE("files not open", aFileOne.is_open() && aFileTwo.is_open());

    sal_Int32 nLine = 1;
    while(!aFileOne.eof() && !aFileTwo.eof())
    {
        std::string aLineFileOne;
        std::string aLineFileTwo;

        std::getline(aFileOne, aLineFileOne);
        std::getline(aFileTwo, aLineFileTwo);

        if( aLineFileOne != aLineFileTwo)
        {
            rtl::OStringBuffer aErrorMessage("Mismatch between reference file and exported file in line ");
            aErrorMessage.append(nLine).append(".\nExpected: ");
            aErrorMessage.append(aLineFileOne.c_str()).append("\nFound   : ").append(aLineFileTwo.c_str());
            CPPUNIT_ASSERT_MESSAGE(aErrorMessage.getStr(), false);
        }
        nLine++;
    }
    return true;
}

}

/* Implementation of Filters test */

using namespace ::com::sun::star;

class SdFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    SdFiltersTest();

    ::sd::DrawDocShellRef loadURL( const rtl::OUString &rURL );
    virtual bool load( const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData);

    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(SdFiltersTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<document::XFilter> m_xFilter;
    uno::Reference<uno::XInterface> m_xDrawComponent;
};

#define PPTX_FORMAT_TYPE 268959811

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; sal_uLong nFormatType;
};

// cf. sc/qa/unit/filters-test.cxx and filters/...*.xcu to fill out.
FileFormat aFileFormats[] = {
    { "pptx" , "Impress MS PowerPoint 2007 XML", "MS PowerPoint 2007 XML", PPTX_FORMAT_TYPE },
    { 0, 0, 0, 0 }
};

::sd::DrawDocShellRef SdFiltersTest::loadURL( const rtl::OUString &rURL )
{
    FileFormat *pFmt;

    for (size_t i = 0; i < SAL_N_ELEMENTS (aFileFormats); i++)
    {
        pFmt = aFileFormats + i;
        if (pFmt->pName &&  rURL.endsWithIgnoreAsciiCaseAsciiL (pFmt->pName, strlen (pFmt->pName)))
            break;
    }
    CPPUNIT_ASSERT_MESSAGE( "missing filter info", pFmt->pName != NULL );

    sal_uInt32 nFormat = 0;
    if (pFmt->nFormatType)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* aFilter = new SfxFilter(
        rtl::OUString::createFromAscii( pFmt->pFilterName ),
        rtl::OUString(), pFmt->nFormatType, nFormat,
        rtl::OUString::createFromAscii( pFmt->pTypeName ),
        0, rtl::OUString(), rtl::OUString(), /* userdata */
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/sdraw*")) );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell();
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ, true);
    pSrcMed->SetFilter(aFilter);
    if ( !xDocShRef->DoLoad(pSrcMed) )
    {
        if (xDocShRef.Is())
            xDocShRef->DoClose();
        CPPUNIT_ASSERT_MESSAGE( "failed to load", false );
    }

    return xDocShRef;
}

void SdFiltersTest::test()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/a.pptx"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    uno::Reference< frame::XModel > xModel = xDocShRef->GetModel();
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference< frame::XStorable > xStorable( xModel, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT( xStorable.is());

    uno::Sequence< beans::PropertyValue > aArgs(1);
    beans::PropertyValue aValue;
    uno::Any aAny;

    aAny <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("impress_svg_Export"));

    aValue.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
    aValue.Value = aAny;
    aValue.State = beans::PropertyState_DIRECT_VALUE;

    aArgs[0] = aValue;

    rtl::OUString aNewSvgURL = m_aSolverRootURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/unittest/sd/test2.svg"));

    xStorable->storeToURL( aNewSvgURL, aArgs );

    compareFiles( getPathFromSrc("/sd/qa/unit/data/svg/test.svg"), getPathFromSolver("/unittest/sd/test2.svg") );

    xDocShRef->DoClose();
}

bool SdFiltersTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData)
{
    SfxFilter aFilter(
        rFilter,
        rtl::OUString(), 0, 0, rtl::OUString(), 0, rtl::OUString(),
        rUserData, rtl::OUString() );

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell();
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ, true);
    pSrcMed->SetFilter(&aFilter);
    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    xDocShRef->DoClose();
    return bLoaded;
}

SdFiltersTest::SdFiltersTest()
{
}

void SdFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xDrawComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.PresentationDocument")));
    CPPUNIT_ASSERT_MESSAGE("no impress component!", m_xDrawComponent.is());
}

void SdFiltersTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xDrawComponent, uno::UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
