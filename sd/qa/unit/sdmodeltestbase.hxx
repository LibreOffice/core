/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SD_QA_UNIT_SDMODELTESTBASE_HXX
#define SD_QA_UNIT_SDMODELTESTBASE_HXX

#include <test/bootstrapfixture.hxx>
#include <test/xmldiff.hxx>

#include <unotest/filters-test.hxx>
#include <unotest/macros_test.hxx>

#include "drawdoc.hxx"
#include "../source/ui/inc/DrawDocShell.hxx"

#include <rtl/strbuf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <drawinglayer/XShapeDumper.hxx>

using namespace ::com::sun::star;

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; sal_uLong nFormatType;
};

// These values are taken from "Flags" in filter/source/config/fragments/filters/*
#define ODP_FORMAT_TYPE  ( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_TEMPLATE | SFX_FILTER_OWN | SFX_FILTER_DEFAULT | SFX_FILTER_ENCRYPTION | SFX_FILTER_PREFERED )
#define PPT_FORMAT_TYPE  ( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN )
#define PPTX_FORMAT_TYPE ( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN | SFX_FILTER_STARONEFILTER | SFX_FILTER_PREFERED )

/** List of file formats we support in Impress unit tests.

Taken from filter/source/config/fragments/filters/ too:
pName: The file extension.
pFilterName: <node oor:Name="...">
pTypeName: <prop oor:Name="UIName">...</prop>
nFormatType: <prop oor:name="Flags">...</prop>
*/
FileFormat aFileFormats[] = {
    { "odp",  "impress8", "impress8", ODP_FORMAT_TYPE },
    { "ppt",  "MS PowerPoint 97", "Microsoft PowerPoint 97/2000/XP/2003", PPT_FORMAT_TYPE },
    { "pptx", "Impress MS PowerPoint 2007 XML", "MS PowerPoint 2007 XML", PPTX_FORMAT_TYPE },
    { 0, 0, 0, 0 }
};

/// Base class for filter tests loading or roundtriping a document, and asserting the document model.
class SdModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    SdModelTestBase()
    {
    }

    virtual void setUp()
    {
        test::BootstrapFixture::setUp();

        // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
        // which is a private symbol to us, gets called
        m_xDrawComponent = getMultiServiceFactory()->createInstance(OUString("com.sun.star.comp.Draw.PresentationDocument"));
        CPPUNIT_ASSERT_MESSAGE("no impress component!", m_xDrawComponent.is());
    }

    virtual void tearDown()
    {
        uno::Reference< lang::XComponent >( m_xDrawComponent, uno::UNO_QUERY_THROW )->dispose();
        test::BootstrapFixture::tearDown();
    }

protected:
    /// Load the document.
    ::sd::DrawDocShellRef loadURL( const OUString &rURL )
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
            OUString::createFromAscii( pFmt->pFilterName ),
            OUString(), pFmt->nFormatType, nFormat,
            OUString::createFromAscii( pFmt->pTypeName ),
            0, OUString(), OUString(), /* userdata */
            OUString("private:factory/simpress*") );
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

    /// Dump shapes in xDocShRef, and compare the dump against content of pShapesDumpFileNameBase<number>.xml.
    void compareWithShapesDump( ::sd::DrawDocShellRef xDocShRef, const OUString &rShapesDumpFileNameBase )
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
        for (sal_Int32 i = 0; i < nLength; ++i)
        {
            uno::Reference<drawing::XDrawPage> xDrawPage;
            uno::Any aAny = xDrawPages->getByIndex(i);
            aAny >>= xDrawPage;
            uno::Reference< drawing::XShapes > xShapes(xDrawPage, uno::UNO_QUERY_THROW);
            OUString aString = xShapeDumper.dump(xShapes);

            OStringBuffer aFileNameBuf( OUStringToOString( rShapesDumpFileNameBase, RTL_TEXTENCODING_UTF8 ) );
            aFileNameBuf.append(i);
            aFileNameBuf.append(".xml");

            OString aFileName = aFileNameBuf.makeStringAndClear();

            std::cout << aString << std::endl;
            doXMLDiff(aFileName.getStr(),
                    OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr(),
                    static_cast<int>(aString.getLength()),
                    OUStringToOString(
                        getPathFromSrc("/sd/qa/unit/data/tolerance.xml"),
                        RTL_TEXTENCODING_UTF8).getStr());
        }
        xDocShRef->DoClose();
    }


private:
    uno::Reference<uno::XInterface> m_xDrawComponent;
};

#endif
