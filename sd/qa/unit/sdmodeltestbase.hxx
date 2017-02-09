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
#include <test/xmltesttools.hxx>

#include <unotest/filters-test.hxx>
#include <unotest/macros_test.hxx>

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/color.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/strbuf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svl/itemset.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <drawinglayer/XShapeDumper.hxx>
#include <com/sun/star/text/XTextField.hpp>

using namespace ::com::sun::star;

struct FileFormat
{
    const char* pName;
    const char* pFilterName;
    const char* pTypeName;
    const char* pUserData;
    SfxFilterFlags nFormatType;
};

// These values are taken from "Flags" in filter/source/config/fragments/filters/*
// You need to turn value of oor:name="Flags" to SfxFilterFlags::*, see
// include/comphelper/documentconstants.hxx for the possible values.
// Note: 3RDPARTYFILTER == SfxFilterFlags::STARONEFILTER
#define ODP_FORMAT_TYPE  ( SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::TEMPLATE | SfxFilterFlags::OWN | SfxFilterFlags::DEFAULT | SfxFilterFlags::ENCRYPTION | SfxFilterFlags::PREFERED )
#define PPT_FORMAT_TYPE  ( SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN )
#define PPTX_FORMAT_TYPE ( SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER | SfxFilterFlags::PREFERED )
#define HTML_FORMAT_TYPE ( SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN )
#define PDF_FORMAT_TYPE  ( SfxFilterFlags::STARONEFILTER | SfxFilterFlags::ALIEN | SfxFilterFlags::IMPORT | SfxFilterFlags::PREFERED )
#define FODG_FORMAT_TYPE  (SfxFilterFlags::STARONEFILTER | SfxFilterFlags::OWN | SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT)
#define FODP_FORMAT_TYPE  (SfxFilterFlags::STARONEFILTER | SfxFilterFlags::OWN | SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT)
#define SXI_FORMAT_TYPE  (SfxFilterFlags::IMPORT | SfxFilterFlags::TEMPLATE | SfxFilterFlags::OWN | SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED | SfxFilterFlags::ENCRYPTION)

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
    { "fodg",  "OpenDocument Drawing Flat XML", "Flat XML ODF Drawing", "", FODG_FORMAT_TYPE },
    { "fodp",  "OpenDocument Presentation Flat XML", "Flat XML ODF Presentation", "", FODP_FORMAT_TYPE },
    { "sxi",  "StarOffice XML (Impress)", "OpenOffice.org 1.0 Presentation", "", SXI_FORMAT_TYPE },
    { "odg",  "draw8", "draw8", "", ODP_FORMAT_TYPE },
    { nullptr, nullptr, nullptr, nullptr, SfxFilterFlags::NONE }
};

#define ODP  0
#define PPT  1
#define PPTX 2
#define HTML 3
#define PDF  4
#define FODG 5
#define FODP 6
#define SXI 7
#define ODG  8

/// Base class for filter tests loading or roundtriping a document, and asserting the document model.
class SdModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<uno::XInterface> mxDrawComponent;

public:
    SdModelTestBase()
    {}

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
        // which is a private symbol to us, gets called
        mxDrawComponent = getMultiServiceFactory()->createInstance("com.sun.star.comp.Draw.PresentationDocument");
        CPPUNIT_ASSERT_MESSAGE("no impress component!", mxDrawComponent.is());
    }

    virtual void tearDown() override
    {
        uno::Reference<lang::XComponent>(mxDrawComponent, uno::UNO_QUERY_THROW)->dispose();
        test::BootstrapFixture::tearDown();
    }

protected:
    /// Load the document.
    sd::DrawDocShellRef loadURL( const OUString &rURL, sal_Int32 nFormat, SfxAllItemSet *pParams = nullptr )
    {
        FileFormat *pFmt = getFormat(nFormat);
        CPPUNIT_ASSERT_MESSAGE( "missing filter info", pFmt->pName != nullptr );

        SotClipboardFormatId nOptions = SotClipboardFormatId::NONE;
        if (pFmt->nFormatType != SfxFilterFlags::NONE)
            nOptions = SotClipboardFormatId::STARCALC_8;
        SfxFilter* pFilter = new SfxFilter(
            OUString::createFromAscii( pFmt->pFilterName ),
            OUString(), pFmt->nFormatType, nOptions,
            OUString::createFromAscii( pFmt->pTypeName ),
            0, OUString(),
            OUString::createFromAscii( pFmt->pUserData ),
            OUString("private:factory/simpress*") );
        pFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
        std::shared_ptr<const SfxFilter> pFilt(pFilter);

        ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false);
        SfxMedium* pSrcMed = new SfxMedium(rURL, StreamMode::STD_READ, pFilt, pParams);
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
        SfxMedium aStoreMedium(rTempFile.GetURL(), StreamMode::STD_WRITE);
        SotClipboardFormatId nExportFormat = SotClipboardFormatId::NONE;
        if (pFormat->nFormatType == ODP_FORMAT_TYPE)
            nExportFormat = SotClipboardFormatId::STARCALC_8;
        std::shared_ptr<const SfxFilter> pExportFilter(new SfxFilter(
                                        OUString::createFromAscii(pFormat->pFilterName),
                                        OUString(), pFormat->nFormatType, nExportFormat,
                                        OUString::createFromAscii(pFormat->pTypeName),
                                        0, OUString(),
                                        OUString::createFromAscii(pFormat->pUserData),
                                        OUString("private:factory/simpress*") ));
        const_cast<SfxFilter*>(pExportFilter.get())->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
        aStoreMedium.SetFilter(pExportFilter);
        pShell->ConvertTo(aStoreMedium);
        pShell->DoClose();
    }

    void save(sd::DrawDocShell* pShell, FileFormat* pFormat, utl::TempFile& rTempFile)
    {
        SfxMedium aStoreMedium(rTempFile.GetURL(), StreamMode::STD_WRITE);
        SotClipboardFormatId nExportFormat = SotClipboardFormatId::NONE;
        if (pFormat->nFormatType == ODP_FORMAT_TYPE)
            nExportFormat = SotClipboardFormatId::STARCHART_8;
        std::shared_ptr<const SfxFilter> pExportFilter(new SfxFilter(
                                        OUString::createFromAscii(pFormat->pFilterName),
                                        OUString(), pFormat->nFormatType, nExportFormat,
                                        OUString::createFromAscii(pFormat->pTypeName),
                                        0, OUString(),
                                        OUString::createFromAscii(pFormat->pUserData),
                                        OUString("private:factory/simpress*") ));
        const_cast<SfxFilter*>(pExportFilter.get())->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
        aStoreMedium.SetFilter(pExportFilter);
        pShell->DoSaveAs(aStoreMedium);
        pShell->DoClose();
    }

    sd::DrawDocShellRef saveAndReload(sd::DrawDocShell *pShell, sal_Int32 nExportType,
            utl::TempFile * pTempFile = nullptr)
    {
        FileFormat* pFormat = getFormat(nExportType);
        std::unique_ptr<utl::TempFile> pNewTempFile;
        if (!pTempFile)
        {
            pNewTempFile.reset(new utl::TempFile);
            pTempFile = pNewTempFile.get();
        }
        save(pShell, pFormat, *pTempFile);
        if (nExportType == ODP || nExportType == ODG)
        {
            // BootstrapFixture::validate(pTempFile->GetFileName(), test::ODF);
        }
        else if(nExportType == PPTX)
        {
            BootstrapFixture::validate(pTempFile->GetFileName(), test::OOXML);
        }
        else if(nExportType == PPT)
        {
            BootstrapFixture::validate(pTempFile->GetFileName(), test::MSBINARY);
        }
        pTempFile->EnableKillingFile();
        return loadURL(pTempFile->GetURL(), nExportType);
    }

    /** Dump shapes in xDocShRef, and compare the dump against content of pShapesDumpFileNameBase<number>.xml.

        @param bCreate Instead of comparing to the reference file(s), create it/them.
    */
    void compareWithShapesDump( ::sd::DrawDocShellRef xDocShRef, const OUString &rShapesDumpFileNameBase, bool bCreate )
    {
        CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
        CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

        uno::Reference<frame::XModel> xTempModel(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xTempModel.is());
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier (xTempModel, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xDrawPagesSupplier.is());
        uno::Reference< drawing::XDrawPages > xDrawPages = xDrawPagesSupplier->getDrawPages();
        CPPUNIT_ASSERT(xDrawPages.is());

        XShapeDumper aShapeDumper;
        sal_Int32 nLength = xDrawPages->getCount();
        for (sal_Int32 i = 0; i < nLength; ++i)
        {
            uno::Reference<drawing::XDrawPage> xDrawPage;
            uno::Any aAny = xDrawPages->getByIndex(i);
            aAny >>= xDrawPage;
            uno::Reference< drawing::XShapes > xShapes(xDrawPage, uno::UNO_QUERY_THROW);
            OUString aString = XShapeDumper::dump(xShapes);

            OStringBuffer aFileNameBuf( OUStringToOString( rShapesDumpFileNameBase, RTL_TEXTENCODING_UTF8 ) );
            aFileNameBuf.append(i);
            aFileNameBuf.append(".xml");

            OString aFileName = aFileNameBuf.makeStringAndClear();

            if ( bCreate )
            {
                std::ofstream aStream( aFileName.getStr(), std::ofstream::out | std::ofstream::binary );
                aStream << aString;
                aStream.close();
            }
            else
            {
                doXMLDiff(aFileName.getStr(),
                        OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr(),
                        static_cast<int>(aString.getLength()),
                        OUStringToOString(
                            m_directories.getPathFromSrc("/sd/qa/unit/data/tolerance.xml"),
                            RTL_TEXTENCODING_UTF8).getStr());
            }
        }
        xDocShRef->DoClose();
    }

    uno::Reference< drawing::XDrawPagesSupplier > getDoc( sd::DrawDocShellRef xDocShRef )
    {
        uno::Reference< drawing::XDrawPagesSupplier > xDoc (
            xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
        CPPUNIT_ASSERT_MESSAGE( "no document", xDoc.is() );
        return xDoc;
    }

    uno::Reference< drawing::XDrawPage > getPage( int nPage,  sd::DrawDocShellRef xDocShRef )
    {
        uno::Reference< drawing::XDrawPagesSupplier > xDoc( getDoc( xDocShRef ) );
        uno::Reference< drawing::XDrawPage > xPage( xDoc->getDrawPages()->getByIndex( nPage ), uno::UNO_QUERY_THROW );
        CPPUNIT_ASSERT_MESSAGE( "no page", xPage.is() );
        return xPage;
    }

    // very confusing ... UNO index-based access to pages is 0-based. This one is 1-based
    const SdrPage* GetPage( int nPage, sd::DrawDocShellRef xDocShRef )
    {
        SdDrawDocument* pDoc =  xDocShRef->GetDoc() ;
        CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

        const SdrPage* pPage = pDoc->GetPage( nPage );
        CPPUNIT_ASSERT_MESSAGE( "no page", pPage != nullptr );
        return pPage;
    }

    uno::Reference< beans::XPropertySet > getShape( int nShape, uno::Reference< drawing::XDrawPage > const & xPage )
    {
        uno::Reference< beans::XPropertySet > xShape( xPage->getByIndex( nShape ), uno::UNO_QUERY );
        CPPUNIT_ASSERT_MESSAGE( "Failed to load shape", xShape.is() );
        return xShape;
    }

    // Nth shape on Mth page
    uno::Reference< beans::XPropertySet > getShapeFromPage( int nShape, int nPage, sd::DrawDocShellRef xDocShRef )
    {
        uno::Reference< drawing::XDrawPage > xPage ( getPage( nPage, xDocShRef ) );
        uno::Reference< beans::XPropertySet > xShape( getShape( nShape, xPage ) );
        CPPUNIT_ASSERT_MESSAGE( "Failed to load shape", xShape.is() );

        return xShape;
    }

    // Nth paragraph of text in given text shape
    uno::Reference< text::XTextRange > getParagraphFromShape( int nPara, uno::Reference< beans::XPropertySet > const & xShape )
    {
        uno::Reference< text::XText > xText = uno::Reference< text::XTextRange>( xShape, uno::UNO_QUERY )->getText();
        CPPUNIT_ASSERT_MESSAGE( "Not a text shape", xText.is() );

        uno::Reference< container::XEnumerationAccess > paraEnumAccess( xText, uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > paraEnum( paraEnumAccess->createEnumeration() );

        for ( int i = 0; i < nPara; ++i )
            paraEnum->nextElement();

        uno::Reference< text::XTextRange > xParagraph( paraEnum->nextElement(), uno::UNO_QUERY_THROW );

        return xParagraph;
    }

    uno::Reference< text::XTextRange > getRunFromParagraph( int nRun, uno::Reference< text::XTextRange > const & xParagraph )
    {
        uno::Reference< container::XEnumerationAccess > runEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference< container::XEnumeration > runEnum = runEnumAccess->createEnumeration();

        for ( int i = 0; i < nRun; ++i )
            runEnum->nextElement();

        uno::Reference< text::XTextRange > xRun( runEnum->nextElement(), uno::UNO_QUERY);

        return xRun;
    }

    uno::Reference<text::XTextField> getTextFieldFromPage(int nRun, int nPara, int nShape, int nPage, sd::DrawDocShellRef xDocShRef)
    {
        // get TextShape 1 from the first page
        uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( nShape, nPage, xDocShRef ) );

        // Get first paragraph
        uno::Reference<text::XTextRange> xParagraph( getParagraphFromShape( nPara, xShape ) );

        // first chunk of text
        uno::Reference<text::XTextRange> xRun( getRunFromParagraph( nRun, xParagraph ) );

        uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

        uno::Reference<text::XTextField> xField;
        xPropSet->getPropertyValue("TextField") >>= xField;
        return xField;
    }

};

class SdModelTestBaseXML
    : public SdModelTestBase, public XmlTestTools
{

public:
    xmlDocPtr parseExport(utl::TempFile & rTempFile, OUString const& rStreamName)
    {
        OUString const url(rTempFile.GetURL());
        uno::Reference<packages::zip::XZipFileAccess2> const xZipNames(
            packages::zip::ZipFileAccess::createWithURL(
                comphelper::getComponentContext(m_xSFactory), url));
        uno::Reference<io::XInputStream> const xInputStream(
            xZipNames->getByName(rStreamName), uno::UNO_QUERY);
        std::unique_ptr<SvStream> const pStream(
            utl::UcbStreamHelper::CreateStream(xInputStream, true));
        xmlDocPtr const pXmlDoc = parseXmlStream(pStream.get());
        pXmlDoc->name = reinterpret_cast<char *>(xmlStrdup(
            reinterpret_cast<xmlChar const *>(OUStringToOString(url, RTL_TEXTENCODING_UTF8).getStr())));
        return pXmlDoc;
    }

};

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<Color>
{
    static bool equal( const Color& c1, const Color& c2 )
    {
        return c1 == c2;
    }

    static std::string toString( const Color& c )
    {
        OStringStream ost;
        ost << static_cast<unsigned int>(c.GetColor());
        return ost.str();
    }
};

CPPUNIT_NS_END

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
