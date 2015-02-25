/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SD_QA_UNIT_SDMODELTESTBASE_HXX
#define INCLUDED_SD_QA_UNIT_SDMODELTESTBASE_HXX

#include <test/bootstrapfixture.hxx>
#include <test/xmldiff.hxx>

#include <unotest/filters-test.hxx>
#include <unotest/macros_test.hxx>

#include "drawdoc.hxx"
#include "../source/ui/inc/DrawDocShell.hxx"
#include "unotools/tempfile.hxx"

#include <rtl/strbuf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svl/itemset.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <drawinglayer/XShapeDumper.hxx>

using namespace ::com::sun::star;

struct FileFormat
{
    const char* pName;
    const char* pFilterName;
    const char* pTypeName;
    const char* pUserData;
    sal_uLong nFormatType;
};

// These values are taken from "Flags" in filter/source/config/fragments/filters/*
// You need to turn value of oor:name="Flags" to SFX_FILTER_*, see
// include/comphelper/documentconstants.hxx for the possible values.
// Note: 3RDPARTYFILTER == SFX_FILTER_STARONEFILTER
#define ODP_FORMAT_TYPE  ( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_TEMPLATE | SFX_FILTER_OWN | SFX_FILTER_DEFAULT | SFX_FILTER_ENCRYPTION | SFX_FILTER_PREFERED )
#define PPT_FORMAT_TYPE  ( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN )
#define PPTX_FORMAT_TYPE ( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN | SFX_FILTER_STARONEFILTER | SFX_FILTER_PREFERED )
#define HTML_FORMAT_TYPE ( SFX_FILTER_EXPORT | SFX_FILTER_ALIEN )
#define PDF_FORMAT_TYPE  ( SFX_FILTER_STARONEFILTER | SFX_FILTER_ALIEN | SFX_FILTER_IMPORT | SFX_FILTER_PREFERED )

/** List of file formats we support in Impress unit tests.

Taken from filter/source/config/fragments/filters/ too:
pName: The file extension.
pFilterName: <node oor:Name="...">
pTypeName: <prop oor:Name="UIName">...</prop>
nFormatType: <prop oor:name="Flags">...</prop>
*/
FileFormat aFileFormats[] =
{
    { "odp",  "impress8", "impress8", "", ODP_FORMAT_TYPE },
    { "ppt",  "MS PowerPoint 97", "Microsoft PowerPoint 97/2000/XP/2003", "sdfilt", PPT_FORMAT_TYPE },
    { "pptx", "Impress Office Open XML", "Office Open XML Presentation", "", PPTX_FORMAT_TYPE },
    { "html", "graphic_HTML", "graphic_HTML", "", HTML_FORMAT_TYPE },
    { "pdf",  "draw_pdf_import", "pdf_Portable_Document_Format", "", PDF_FORMAT_TYPE },
    { 0, 0, 0, 0, 0 }
};

#define ODP  0
#define PPT  1
#define PPTX 2
#define HTML 3
#define PDF  4

/// Base class for filter tests loading or roundtriping a document, and asserting the document model.
class SdModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<uno::XInterface> mxDrawComponent;

public:
    SdModelTestBase()
    {}

    virtual void setUp() SAL_OVERRIDE
    {
        test::BootstrapFixture::setUp();

        // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
        // which is a private symbol to us, gets called
        mxDrawComponent = getMultiServiceFactory()->createInstance("com.sun.star.comp.Draw.PresentationDocument");
        CPPUNIT_ASSERT_MESSAGE("no impress component!", mxDrawComponent.is());
    }

    virtual void tearDown() SAL_OVERRIDE
    {
        uno::Reference<lang::XComponent>(mxDrawComponent, uno::UNO_QUERY_THROW)->dispose();
        test::BootstrapFixture::tearDown();
    }

protected:
    /// Load the document.
    sd::DrawDocShellRef loadURL( const OUString &rURL, sal_Int32 nFormat, SfxAllItemSet *pParams = 0 )
    {
        FileFormat *pFmt = getFormat(nFormat);
        CPPUNIT_ASSERT_MESSAGE( "missing filter info", pFmt->pName != NULL );

        sal_uInt32 nOptions = 0;
        if (pFmt->nFormatType)
            nOptions = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
        SfxFilter* aFilter = new SfxFilter(
            OUString::createFromAscii( pFmt->pFilterName ),
            OUString(), pFmt->nFormatType, nOptions,
            OUString::createFromAscii( pFmt->pTypeName ),
            0, OUString(),
            OUString::createFromAscii( pFmt->pUserData ),
            OUString("private:factory/simpress*") );
        aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

        ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell();
        SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ, aFilter, pParams);
        if ( !xDocShRef->DoLoad(pSrcMed) || !xDocShRef.Is() )
        {
            if (xDocShRef.Is())
                xDocShRef->DoClose();
            CPPUNIT_ASSERT_MESSAGE( OUStringToOString( "failed to load " + rURL, RTL_TEXTENCODING_UTF8 ).getStr(), false );
        }
        CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

        return xDocShRef;
    }

    FileFormat* getFormat(sal_Int32 nExportType)
    {
        FileFormat* pFormat = &aFileFormats[0];
        if (((sal_uInt32) nExportType) < SAL_N_ELEMENTS(aFileFormats))
            pFormat = &aFileFormats[nExportType];
        return pFormat;
    }

    void exportTo(sd::DrawDocShell* pShell, FileFormat* pFormat, utl::TempFile& rTempFile)
    {
        SfxMedium aStoreMedium(rTempFile.GetURL(), STREAM_STD_WRITE);
        sal_uInt32 nExportFormat = 0;
        if (pFormat->nFormatType == ODP_FORMAT_TYPE)
            nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
        SfxFilter* pExportFilter = new SfxFilter(
                                        OUString::createFromAscii(pFormat->pFilterName),
                                        OUString(), pFormat->nFormatType, nExportFormat,
                                        OUString::createFromAscii(pFormat->pTypeName),
                                        0, OUString(),
                                        OUString::createFromAscii(pFormat->pUserData),
                                        OUString("private:factory/simpress*") );
        pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
        aStoreMedium.SetFilter(pExportFilter);
        pShell->ConvertTo(aStoreMedium);
        pShell->DoClose();
    }

    void save(sd::DrawDocShell* pShell, FileFormat* pFormat, utl::TempFile& rTempFile)
    {
        SfxMedium aStoreMedium(rTempFile.GetURL(), STREAM_STD_WRITE);
        sal_uInt32 nExportFormat = 0;
        if (pFormat->nFormatType == ODP_FORMAT_TYPE)
            nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
        SfxFilter* pExportFilter = new SfxFilter(
                                        OUString::createFromAscii(pFormat->pFilterName),
                                        OUString(), pFormat->nFormatType, nExportFormat,
                                        OUString::createFromAscii(pFormat->pTypeName),
                                        0, OUString(),
                                        OUString::createFromAscii(pFormat->pUserData),
                                        OUString("private:factory/simpress*") );
        pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
        aStoreMedium.SetFilter(pExportFilter);
        pShell->DoSaveAs(aStoreMedium);
        pShell->DoClose();
    }

    sd::DrawDocShellRef saveAndReload(sd::DrawDocShell *pShell, sal_Int32 nExportType)
    {
        FileFormat* pFormat = getFormat(nExportType);
        utl::TempFile aTempFile;
        save(pShell, pFormat, aTempFile);
        if(nExportType == ODP)
        {
            // BootstrapFixture::validate(aTempFile.GetFileName(), test::ODF);
        }
        else if(nExportType == PPTX)
        {
            BootstrapFixture::validate(aTempFile.GetFileName(), test::OOXML);
        }
        aTempFile.EnableKillingFile();
        return loadURL(aTempFile.GetURL(), nExportType);
    }

    /** Dump shapes in xDocShRef, and compare the dump against content of pShapesDumpFileNameBase<number>.xml.

        @param bCreate Instead of comparing to the reference file(s), create it/them.
    */
    void compareWithShapesDump( ::sd::DrawDocShellRef xDocShRef, const OUString &rShapesDumpFileNameBase, bool bCreate = false )
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

            if ( bCreate )
            {
                std::ofstream aStream( aFileName.getStr(), std::ofstream::out );
                aStream << aString;
                aStream.close();
            }
            else
            {
                doXMLDiff(aFileName.getStr(),
                        OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr(),
                        static_cast<int>(aString.getLength()),
                        OUStringToOString(
                            getPathFromSrc("/sd/qa/unit/data/tolerance.xml"),
                            RTL_TEXTENCODING_UTF8).getStr());
            }
        }
        xDocShRef->DoClose();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
