/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <officecfg/Office/Common.hxx>
#include "sdmodeltestbase.hxx"
#include <Outliner.hxx>
#include <comphelper/propertysequence.hxx>
#include <svl/stritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/bulletitem.hxx>

#include <oox/drawingml/drawingmltypes.hxx>

#include <svl/style.hxx>

#include <svx/svdoutl.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <svx/xflclit.hxx>
#include <animations/animationnodehelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <rtl/ustring.hxx>

#include <vcl/opengl/OpenGLWrapper.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>


#include <svx/svdotable.hxx>
#include <config_features.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace css;
using namespace css::animations;

class SdExportTest : public SdModelTestBaseXML
{
public:
    void testN821567();
    void testMediaEmbedding();
    void testFdo84043();
    void testTdf97630();
    void testSwappedOutImageExport();
    void testOOoXMLAnimations();
    void testUnknownAttributes();
    void testTdf80020();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();
    void testTdf62176();
    void testTransparentBackground();
    void testEmbeddedPdf();
    void testEmbeddedText();
    void testAuthorField();
    void testTdf100926();
    void testPageWithTransparentBackground();
    void testTextRotation();

    CPPUNIT_TEST_SUITE(SdExportTest);

    CPPUNIT_TEST(testN821567);
    CPPUNIT_TEST(testMediaEmbedding);
    CPPUNIT_TEST(testFdo84043);
    CPPUNIT_TEST(testTdf97630);
    CPPUNIT_TEST(testSwappedOutImageExport);
    CPPUNIT_TEST(testOOoXMLAnimations);
    CPPUNIT_TEST(testUnknownAttributes);
    CPPUNIT_TEST(testTdf80020);
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
    CPPUNIT_TEST(testTdf62176);
    CPPUNIT_TEST(testTransparentBackground);
    CPPUNIT_TEST(testEmbeddedPdf);
    CPPUNIT_TEST(testEmbeddedText);
    CPPUNIT_TEST(testAuthorField);
    CPPUNIT_TEST(testTdf100926);
    CPPUNIT_TEST(testPageWithTransparentBackground);
    CPPUNIT_TEST(testTextRotation);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        struct { char const * pPrefix; char const * pURI; } namespaces[] =
        {
            // ODF
            { "anim", "urn:oasis:names:tc:opendocument:xmlns:animation:1.0" },
            { "draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" },
            { "fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" },
            { "number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" },
            { "office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" },
            { "presentation", "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0" },
            { "style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0" },
            { "svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" },
            { "table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0" },
            { "text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0" },
            { "xlink", "http://www.w3c.org/1999/xlink" },
            { "loext", "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0" },
            // user-defined
            { "foo", "http://example.com/" },
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(namespaces); ++i)
        {
            xmlXPathRegisterNs(pXmlXPathCtx,
                reinterpret_cast<xmlChar const *>(namespaces[i].pPrefix),
                reinterpret_cast<xmlChar const *>(namespaces[i].pURI));
        }
    }

};

void SdExportTest::testN821567()
{
    OUString bgImage;
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n821567.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef.get(), ODP );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "not exactly one page", static_cast<sal_Int32>(1), xDoc->getDrawPages()->getCount() );
    uno::Reference< drawing::XDrawPage > xPage( getPage( 0, xDocShRef ) );

    uno::Reference< beans::XPropertySet > xPropSet( xPage, uno::UNO_QUERY );
    uno::Any aAny = xPropSet->getPropertyValue( "Background" );
    if(aAny.hasValue())
    {
        uno::Reference< beans::XPropertySet > aXBackgroundPropSet;
        aAny >>= aXBackgroundPropSet;
        aAny = aXBackgroundPropSet->getPropertyValue( "FillBitmapName" );
        aAny >>= bgImage;
    }
    CPPUNIT_ASSERT_MESSAGE("Slide Background is not exported properly", !bgImage.isEmpty());

    xDocShRef->DoClose();
}

namespace {

template< typename ItemValue, typename ItemType >
void checkFontAttributes( const SdrTextObj* pObj, ItemValue nVal)
{
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
    {
        const ItemType* pAttrib = dynamic_cast<const ItemType *>((*it).pAttr);
        if (pAttrib)
        {
            CPPUNIT_ASSERT_EQUAL( nVal, static_cast<ItemValue>(pAttrib->GetValue()));
        }
    }
}

}

void SdExportTest::testTransparentBackground()
{
   ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/transparent_background.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), ODP );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    const SdrTextObj *pObj1 = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
    checkFontAttributes<Color, SvxBackgroundColorItem>( pObj1, Color(COL_TRANSPARENT) );

    const SdrTextObj *pObj2 = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
    checkFontAttributes<Color, SvxBackgroundColorItem>( pObj2, Color(COL_YELLOW));

    xDocShRef->DoClose();
}

void SdExportTest::testMediaEmbedding()
{
    if (!OpenGLWrapper::isVCLOpenGLEnabled())
        return;

    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/media_embedding.odp"), ODP);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Second object is a sound
    SdrMediaObj *pMediaObj = dynamic_cast<SdrMediaObj*>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "missing media object", pMediaObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "vnd.sun.star.Package:Media/button-1.wav" ), pMediaObj->getMediaProperties().getURL());
    CPPUNIT_ASSERT_EQUAL( OUString( "application/vnd.sun.star.media" ), pMediaObj->getMediaProperties().getMimeType());

    xDocShRef->DoClose();
}

void SdExportTest::testFdo84043()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/fdo84043.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), ODP );

    // the bug was duplicate attributes, causing crash in a build with asserts
    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject const* pShape = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    xDocShRef->DoClose();
}

void SdExportTest::testTdf97630()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/fit-to-size.fodp"), FODP);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDP(xDPS->getDrawPages()->getByIndex(0), uno::UNO_QUERY);

        drawing::TextFitToSizeType tmp;
        // text shapes
        uno::Reference<beans::XPropertySet> xShape0(xDP->getByIndex(0), uno::UNO_QUERY);
        xShape0->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape1(xDP->getByIndex(1), uno::UNO_QUERY);
        xShape1->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape2(xDP->getByIndex(2), uno::UNO_QUERY);
        xShape2->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
        uno::Reference<beans::XPropertySet> xShape3(xDP->getByIndex(3), uno::UNO_QUERY);
        xShape3->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, tmp);

        // fontworks
        uno::Reference<beans::XPropertySet> xShape4(xDP->getByIndex(4), uno::UNO_QUERY);
        xShape4->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape5(xDP->getByIndex(5), uno::UNO_QUERY);
        xShape5->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);

    }

    utl::TempFile aTempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), ODP, &aTempFile);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDP(xDPS->getDrawPages()->getByIndex(0), uno::UNO_QUERY);

        drawing::TextFitToSizeType tmp;
        // text shapes
        uno::Reference<beans::XPropertySet> xShape0(xDP->getByIndex(0), uno::UNO_QUERY);
        xShape0->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape1(xDP->getByIndex(1), uno::UNO_QUERY);
        xShape1->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape2(xDP->getByIndex(2), uno::UNO_QUERY);
        xShape2->getPropertyValue("TextFitToSize") >>= tmp;
#if 1
// TODO see TODO in sdpropls.cxx
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
#else
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
#endif
        uno::Reference<beans::XPropertySet> xShape3(xDP->getByIndex(3), uno::UNO_QUERY);
        xShape3->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, tmp);

        // fontworks
        uno::Reference<beans::XPropertySet> xShape4(xDP->getByIndex(4), uno::UNO_QUERY);
        xShape4->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape5(xDP->getByIndex(5), uno::UNO_QUERY);
        xShape5->getPropertyValue("TextFitToSize") >>= tmp;
#if 1
// TODO see TODO in sdpropls.cxx
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
#else
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
#endif
    }

    xmlDocPtr pXmlDoc = parseExport(aTempFile, "content.xml");
    // text shapes
    assertXPath(pXmlDoc, "//style:style[@style:family='presentation']/style:graphic-properties[@draw:fit-to-size='false' and @style:shrink-to-fit='false']", 1);
#if 1
// TODO see TODO in sdpropls.cxx
    assertXPath(pXmlDoc, "//style:style[@style:family='presentation']/style:graphic-properties[@draw:fit-to-size='true' and @style:shrink-to-fit='false']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='presentation']/style:graphic-properties[@draw:fit-to-size='all' and @style:shrink-to-fit='false']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='presentation']/style:graphic-properties[@draw:fit-to-size='shrink-to-fit' and @style:shrink-to-fit='true']", 1);
#else
    assertXPath(pXmlDoc, "//style:style[@style:family='presentation']/style:graphic-properties[@draw:fit-to-size='true' and @style:shrink-to-fit='false']", 2);
    assertXPath(pXmlDoc, "//style:style[@style:family='presentation']/style:graphic-properties[@draw:fit-to-size='false' and @style:shrink-to-fit='true']", 1);
#endif
    // fontworks
    assertXPath(pXmlDoc, "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fit-to-size='false' and @style:shrink-to-fit='false']", 1);
#if 1
// TODO see TODO in sdpropls.cxx
    assertXPath(pXmlDoc, "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fit-to-size='all' and @style:shrink-to-fit='false']", 1);
#else
    assertXPath(pXmlDoc, "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fit-to-size='true' and @style:shrink-to-fit='false']", 1);
#endif

    xDocShRef->DoClose();
}

void SdExportTest::testSwappedOutImageExport()
{
    // Problem was with the swapped out images, which were not swapped in during export.
    const sal_Int32 vFormats[] = {
        ODP,
        PPT,
        PPTX,
    };

    // Set cache size to a very small value to make sure one of the images is swapped out
    std::shared_ptr< comphelper::ConfigurationChanges > xBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), xBatch);
    xBatch->commit();

    for( size_t nExportFormat = 0; nExportFormat < SAL_N_ELEMENTS(vFormats); ++nExportFormat )
    {
        // Load the original file with one image
        ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/document_with_two_images.odp"), ODP);
        const OString sFailedMessage = OString("Failed on filter: ") + OString(aFileFormats[vFormats[nExportFormat]].pFilterName);

        // Export the document and import again for a check
        uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[vFormats[nExportFormat]].pFilterName), RTL_TEXTENCODING_UTF8);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xComponent.set(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        xDocShRef = loadURL(aTempFile.GetURL(), nExportFormat);

        // Check whether graphic exported well after it was swapped out
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDrawPagesSupplier->getDrawPages()->getCount());
        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
        xDocShRef->DoClose();
    }
}

void SdExportTest::testOOoXMLAnimations()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/sxi/ooo41061-1.sxi"), SXI);

    uno::Reference<lang::XComponent> xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(getFormat(ODP)->pFilterName), RTL_TEXTENCODING_UTF8);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    xDocShRef->DoClose();

    // the problem was that legacy OOoXML animations were lost if store
    // immediately follows load because they were "converted" async by a timer
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "content.xml");
    assertXPath(pXmlDoc, "//anim:par[@presentation:node-type='timing-root']", 26);
    // currently getting 52 of these without the fix (depends on timing)
    assertXPath(pXmlDoc, "//anim:par", 223);
}

void SdExportTest::testUnknownAttributes()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/unknown-attribute.fodp"), FODP);

    uno::Reference<lang::XComponent> xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(getFormat(ODP)->pFilterName), RTL_TEXTENCODING_UTF8);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    xDocShRef->DoClose();

    xmlDocPtr pXmlDoc = parseExport(aTempFile, "content.xml");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/style:graphic-properties[@foo:non-existent-att='bar']");
// TODO: if the namespace is *known*, the attribute is not preserved, but that seems to be a pre-existing problem, or maybe it's even intentional?
//    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/style:graphic-properties[@svg:non-existent-att='blah']");
    // this was on style:graphic-properties on the import, but the export moves it to root node which is OK
    assertXPathNSDef(pXmlDoc, "/office:document-content", "foo", "http://example.com/");
}

void SdExportTest::testTdf80020()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf80020.odp"), ODP);
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"), uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("Test Style"), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("text"), xStyle->getParentStyle());
        xDocShRef = saveAndReload( xDocShRef.get(), ODP );
    }
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("Test Style"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("text"), xStyle->getParentStyle());

    xDocShRef->DoClose();
}

void SdExportTest::testLinkedGraphicRT()
{
    // Problem was with linked images
    const sal_Int32 vFormats[] = {
        ODP,
        PPT,
//      PPTX, -> this fails now, need a fix
    };

    for( size_t nExportFormat = 0; nExportFormat < SAL_N_ELEMENTS(vFormats); ++nExportFormat )
    {
        // Load the original file with one image
        ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/document_with_linked_graphic.odp"), ODP);
        const OString sFailedMessage = OString("Failed on filter: ") + OString(aFileFormats[vFormats[nExportFormat]].pFilterName);

        // Export the document and import again for a check
        uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[vFormats[nExportFormat]].pFilterName), RTL_TEXTENCODING_UTF8);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xComponent.set(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        xDocShRef = loadURL(aTempFile.GetURL(), nExportFormat);

        // Check whether graphic imported well after export
        SdDrawDocument *pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDoc != nullptr );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pPage != nullptr );
        SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject != nullptr );
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject->IsLinkedGraphic() );

        const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), !rGraphicObj.IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());

        xDocShRef->DoClose();
    }
}

void SdExportTest::testImageWithSpecialID()
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.
    const sal_Int32 vFormats[] = {
        ODP,
        PPT,
        PPTX,
    };

    // Trigger swap out mechanism to test swapped state factor too.
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    for( size_t nExportFormat = 0; nExportFormat < SAL_N_ELEMENTS(vFormats); ++nExportFormat )
    {
        // Load the original file
        ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/images_with_special_IDs.odp"), ODP);
        const OString sFailedMessage = OString("Failed on filter: ") + OString(aFileFormats[vFormats[nExportFormat]].pFilterName);

        // Export the document and import again for a check
        uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[vFormats[nExportFormat]].pFilterName), RTL_TEXTENCODING_UTF8);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xComponent.set(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        xDocShRef = loadURL(aTempFile.GetURL(), nExportFormat);

        // Check whether graphic was exported well
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDrawPagesSupplier->getDrawPages()->getCount() );
        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
        xDocShRef->DoClose();
    }
}

void SdExportTest::testTdf62176()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/Tdf62176.odp"), ODP);
    uno::Reference<drawing::XDrawPage> xPage( getPage( 0, xDocShRef ) );

    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape( getShape( 0, xPage ) );
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin = 0;
    xShape->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent = 0;
    xShape->getPropertyValue("ParaFirstLineIndent") >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph( getParagraphFromShape( 0, xShape ) );
    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), xParagraph->getString());

    //Saving and Reloading the file
    xDocShRef = saveAndReload( xDocShRef.get(), ODP );
    uno::Reference<drawing::XDrawPage> xPage2( getPage(0, xDocShRef ) );
    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage2->getCount());

    uno::Reference<beans::XPropertySet> xShape2( getShape( 0, xPage2 ) );
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin2 = 0;
    xShape2->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin2);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent2 = 0;
    xShape2->getPropertyValue("ParaFirstLineIndent") >>= nParaFirstLineIndent2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent2);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph2( getParagraphFromShape( 0, xShape2 ) );
    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), xParagraph2->getString());

    xDocShRef->DoClose();
}

void SdExportTest::testEmbeddedPdf()
{
#if HAVE_FEATURE_PDFIUM
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/embedded-pdf.odp"), ODP);
    xShell = saveAndReload( xShell.get(), ODP );
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    OUString aReplacementGraphicURL;
    xShape->getPropertyValue("ReplacementGraphicURL") >>= aReplacementGraphicURL;
    CPPUNIT_ASSERT(!aReplacementGraphicURL.isEmpty());
    xShell->DoClose();
#endif
}

void SdExportTest::testEmbeddedText()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/objectwithtext.fodg"), FODG);
    xShell = saveAndReload( xShell.get(), ODG );

    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xText.is());

    uno::Reference<container::XEnumerationAccess> xEA(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEA->hasElements());
    uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration());
    uno::Reference<text::XTextContent> xTC;
    xEnum->nextElement() >>= xTC;
    CPPUNIT_ASSERT(xTC.is());

    uno::Reference<container::XEnumerationAccess> xParaEA(xTC, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEA->createEnumeration());
    uno::Reference<beans::XPropertySet> xPortion(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortion.is());
    uno::Reference<text::XTextRange> xRange(xPortion, uno::UNO_QUERY);
    OUString type;
    xPortion->getPropertyValue("TextPortionType") >>= type;
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), type);
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), xRange->getString()); //tdf#112547

    xShell->DoClose();
}

void SdExportTest::testAuthorField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/author_fixed.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), ODP );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    uno::Reference< beans::XPropertySet > xPropSet( xField, uno::UNO_QUERY_THROW );
    bool bFixed = false;
    xPropSet->getPropertyValue("IsFixed") >>= bFixed;
    CPPUNIT_ASSERT_MESSAGE("Author field is not fixed", bFixed);

    xDocShRef->DoClose();
}

void SdExportTest::testTdf100926()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf100926_ODP.pptx"), PPTX);

    xDocShRef = saveAndReload(xDocShRef.get(), ODP);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    sal_Int32 nRotation = 0;
    uno::Reference< beans::XPropertySet > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000), nRotation);

    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9000), nRotation);

    xCell.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRotation);

    xDocShRef->DoClose();
}

void SdExportTest::testPageWithTransparentBackground()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/odp/page_transparent_background.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef.get(), ODP );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "There should be exactly one page", static_cast<sal_Int32>(1), xDoc->getDrawPages()->getCount() );

    uno::Reference< drawing::XDrawPage > xPage( getPage( 0, xDocShRef ) );

    uno::Reference< beans::XPropertySet > xPropSet( xPage, uno::UNO_QUERY );
    uno::Any aAny = xPropSet->getPropertyValue( "Background" );
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());

    uno::Reference< beans::XPropertySet > aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;
    sal_Int32 nTransparence;
    aAny = aXBackgroundPropSet->getPropertyValue( "FillTransparence" );
    aAny >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide background transparency is wrong", sal_Int32(42), nTransparence);

    xDocShRef->DoClose();
}

void SdExportTest::testTextRotation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/shape-text-rotate.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), ODP, &tempFile);

    uno::Reference<drawing::XDrawPage> xPage(getPage(0, xDocShRef));
    uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));

    CPPUNIT_ASSERT(xPropSet.is());

    auto aGeomPropSeq = xPropSet->getPropertyValue("CustomShapeGeometry").get<uno::Sequence<beans::PropertyValue>>();
    comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);

    auto it = aCustomShapeGeometry.find("TextRotateAngle");
    CPPUNIT_ASSERT(it != aCustomShapeGeometry.end());

    CPPUNIT_ASSERT_EQUAL((double)(-90), aCustomShapeGeometry["TextRotateAngle"].get<double>());

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
