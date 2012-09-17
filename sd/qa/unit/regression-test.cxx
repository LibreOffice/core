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
#include <test/xmldiff.hxx>
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

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <drawinglayer/XShapeDumper.hxx>

#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>

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
    void testN759180();
    void testFdo47434();

    CPPUNIT_TEST_SUITE(SdFiltersTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testN759180);
    CPPUNIT_TEST(testFdo47434);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<document::XFilter> m_xFilter;
    uno::Reference<uno::XInterface> m_xDrawComponent;
    void testStuff(::sd::DrawDocShellRef xDocShRef, const rtl::OString& fileNameBase);
};

#define PPTX_FORMAT_TYPE 268959811
#define ODP_FORMAT_TYPE 285212967

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; sal_uLong nFormatType;
};

// cf. sc/qa/unit/filters-test.cxx and filters/...*.xcu to fill out.
FileFormat aFileFormats[] = {
    { "pptx" , "Impress MS PowerPoint 2007 XML", "MS PowerPoint 2007 XML", PPTX_FORMAT_TYPE },
    { "odp" , "impress8", "impress8", ODP_FORMAT_TYPE },
    { 0, 0, 0, 0 }
};

::sd::DrawDocShellRef SdFiltersTest::loadURL( const rtl::OUString &rURL )
{
    FileFormat *pFmt(0);

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
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/simpress*")) );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell();
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
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
    {
        ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/shapes-test.odp"));
        testStuff(xDocShRef, rtl::OUStringToOString(getPathFromSrc("/sd/qa/unit/data/xml/shapes-test_page"), RTL_TEXTENCODING_UTF8));
    }
    /*
    {
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/text-test.odp"));
    testStuff(xDocShRef);
    }*/
}

void SdFiltersTest::testN759180()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/n759180.pptx"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    //sal_uIntPtr nObjs = pPage->GetObjCount();
    //for (sal_uIntPtr i = 0; i < nObjs; i++)
    {
        // Get the object
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT(pTxtObj);
        std::vector<EECharAttrib> rLst;
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxULSpaceItem *pULSpace = dynamic_cast<const SvxULSpaceItem *>(aEdit.GetParaAttribs(0).GetItem(EE_PARA_ULSPACE));
        CPPUNIT_ASSERT(pULSpace);
        CPPUNIT_ASSERT_MESSAGE( "Para bottom spacing is wrong!", pULSpace->GetLower() == 0 );
        aEdit.GetCharAttribs(1, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
            if(pFontHeight)
            {
                // nStart == 9
                // font height = 5 => 5*2540/72
                CPPUNIT_ASSERT_MESSAGE( "Font height is wrong", pFontHeight->GetHeight() == 176 );
                break;
            }
        }
    }
}

void SdFiltersTest::testFdo47434()
{
    // The problem was the arrow that has cy < 180 and flipH = 0 is rendered incorrectly.
    // Its height should be 1, not negative.
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/fdo47434-all.pptx"));
    testStuff(xDocShRef, rtl::OUStringToOString(getPathFromSrc("/sd/qa/unit/data/pptx/xml/fdo47434_page"), RTL_TEXTENCODING_UTF8));
}

void SdFiltersTest::testStuff(::sd::DrawDocShellRef xDocShRef, const rtl::OString& fileNameBase)
{
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    uno::Reference<frame::XModel> xTempModel(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xTempModel.is());
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier (xTempModel, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference< drawing::XDrawPages > xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT(xDrawPages.is());

    XShapeDumper xShapeDumper;
    sal_Int32 nLength = xDrawPages->getCount();
    rtl::OString aFileNameExt(".xml");
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference<drawing::XDrawPage> xDrawPage;
        uno::Any aAny = xDrawPages->getByIndex(i);
        aAny >>= xDrawPage;
        uno::Reference< drawing::XShapes > xShapes(xDrawPage, uno::UNO_QUERY_THROW);
        rtl::OUString aString = xShapeDumper.dump(xShapes);
        rtl::OStringBuffer aFileNameBuf(fileNameBase);
        aFileNameBuf.append(i);
        aFileNameBuf.append(aFileNameExt);

        rtl::OString aFileName = aFileNameBuf.makeStringAndClear();

        std::cout << aString << std::endl;
        doXMLDiff(aFileName.getStr(),
            rtl::OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr(),
            static_cast<int>(aString.getLength()),
            rtl::OUStringToOString(
                getPathFromSrc("/sd/qa/unit/data/tolerance.xml"),
                RTL_TEXTENCODING_UTF8).getStr());
    }
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
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
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
