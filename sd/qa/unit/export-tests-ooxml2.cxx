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
#include "Outliner.hxx"
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

#include <rsc/rscsfx.hxx>

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
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
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
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <svx/svdotable.hxx>
#include <config_features.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace css;
using namespace css::animations;

class SdOOXMLExportTest2 : public SdModelTestBaseXML
{
public:
    void testTdf93883();
    void testTdf91378();
//This test gives errors due to ATL
#if HAVE_FEATURE_ATL || !defined(_WIN32)
    void testBnc822341();
#endif
    void testMathObject();
    void testMathObjectPPT2010();
    void testTdf80224();
    void testExportTransitionsPPTX();
    void testPresetShapesExport();
    void testTdf92527();
    void testDatetimeFieldNumberFormat();
    void testDatetimeFieldNumberFormatPPTX();
    void testSlideNumberField();
    void testSlideNumberFieldPPTX();
    void testSlideCountField();
    void testSlideNameField();
    void testExtFileField();
    void testAuthorField();
    void testTdf99224();
    void testTdf92076();
    void testTdf59046();
    void testTdf105739();
    void testTdf111863();
    void testTdf111518();
    void testTdf106867();
    void testTdf112280();
    void testTdf112552();
    void testTdf112557();
    void testTdf112088();
    void testTdf112333();
    void testTdf112089();
    void testTdf112334();
    void testTdf112647();
    void testTdf112086();
    void testTdf104788();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest2);

    CPPUNIT_TEST(testTdf93883);
    CPPUNIT_TEST(testTdf91378);
//This test gives errors due to ATL
#if HAVE_FEATURE_ATL || !defined(_WIN32)
    CPPUNIT_TEST(testBnc822341);
#endif
    CPPUNIT_TEST(testMathObject);
    CPPUNIT_TEST(testMathObjectPPT2010);
    CPPUNIT_TEST(testTdf80224);
    CPPUNIT_TEST(testExportTransitionsPPTX);
    CPPUNIT_TEST(testPresetShapesExport);
    CPPUNIT_TEST(testTdf92527);
    CPPUNIT_TEST(testDatetimeFieldNumberFormat);
    CPPUNIT_TEST(testDatetimeFieldNumberFormatPPTX);
    CPPUNIT_TEST(testSlideNumberField);
    CPPUNIT_TEST(testSlideNumberFieldPPTX);
    CPPUNIT_TEST(testSlideCountField);
    CPPUNIT_TEST(testSlideNameField);
    CPPUNIT_TEST(testExtFileField);
    CPPUNIT_TEST(testAuthorField);
    CPPUNIT_TEST(testTdf99224);
    CPPUNIT_TEST(testTdf92076);
    CPPUNIT_TEST(testTdf59046);
    CPPUNIT_TEST(testTdf105739);
    CPPUNIT_TEST(testTdf111863);
    CPPUNIT_TEST(testTdf111518);
    CPPUNIT_TEST(testTdf106867);
    CPPUNIT_TEST(testTdf112280);
    CPPUNIT_TEST(testTdf112552);
    CPPUNIT_TEST(testTdf112557);
    CPPUNIT_TEST(testTdf112088);
    CPPUNIT_TEST(testTdf112333);
    CPPUNIT_TEST(testTdf112089);
    CPPUNIT_TEST(testTdf112334);
    CPPUNIT_TEST(testTdf112647);
    CPPUNIT_TEST(testTdf112086);
    CPPUNIT_TEST(testTdf104788);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        struct { char const * pPrefix; char const * pURI; } namespaces[] =
        {
            // OOXML
            { "ContentType", "http://schemas.openxmlformats.org/package/2006/content-types" },
            { "rels", "http://schemas.openxmlformats.org/package/2006/relationships" },
            { "mc", "http://schemas.openxmlformats.org/markup-compatibility/2006" },
            { "v", "urn:schemas-microsoft-com:vml" },
            { "a", "http://schemas.openxmlformats.org/drawingml/2006/main" },
            { "c", "http://schemas.openxmlformats.org/drawingml/2006/chart" },
            { "m", "http://schemas.openxmlformats.org/officeDocument/2006/math" },
            { "pic", "http://schemas.openxmlformats.org/drawingml/2006/picture" },
            { "wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" },
            { "p", "http://schemas.openxmlformats.org/presentationml/2006/main" },
            { "p14", "http://schemas.microsoft.com/office/powerpoint/2010/main" },
            { "w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main" },
            { "a14", "http://schemas.microsoft.com/office/drawing/2010/main" },
            { "wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" },
            { "wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" },
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(namespaces); ++i)
        {
            xmlXPathRegisterNs(pXmlXPathCtx,
                reinterpret_cast<xmlChar const *>(namespaces[i].pPrefix),
                reinterpret_cast<xmlChar const *>(namespaces[i].pURI));
        }
    }

};

void SdOOXMLExportTest2::testTdf93883()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf93883.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT(!xPropSet->getPropertyValue("NumberingLevel").hasValue());
}

//This test gives errors due to ATL
#if HAVE_FEATURE_ATL || !defined(_WIN32)
void SdOOXMLExportTest2::testBnc822341()
{
    // Check import / export of embedded text document
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/bnc822341.odp"), ODP);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile1 );

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocPtr pXmlDocCT = parseExport(tempFile1, "[Content_Types].xml");
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.wordprocessingml.document']",
                    "PartName",
                    "/ppt/embeddings/oleObject1.docx");

        xmlDocPtr pXmlDocRels = parseExport(tempFile1, "ppt/slides/_rels/slide1.xml.rels");
        assertXPath(pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']",
            "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");

        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj",
            "progId",
            "Word.Document.12");

        const SdrPage *pPage = GetPage( 1, xDocShRef.get() );

        const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj(0) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    utl::TempFile tempFile2;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile2 );

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocPtr pXmlDocCT = parseExport(tempFile2, "[Content_Types].xml");
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.wordprocessingml.document']",
                    "PartName",
                    "/ppt/embeddings/oleObject1.docx");

        xmlDocPtr pXmlDocRels = parseExport(tempFile2, "ppt/slides/_rels/slide1.xml.rels");
        assertXPath(pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']",
            "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");

        xmlDocPtr pXmlDocContent = parseExport(tempFile2, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj",
            "progId",
            "Word.Document.12");

        SdDrawDocument *pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( "no page", pPage != nullptr );

        const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj(0) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    xDocShRef->DoClose();
}
#endif

void SdOOXMLExportTest2::testMathObject()
{
    // Check import / export of math object
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/math.odp"), ODP);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile1);

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice",
            "Requires",
            "a14");
        assertXPathContent(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/a14:m/m:oMath/m:r[1]/m:t",
            "a");

        const SdrPage *pPage = GetPage(1, xDocShRef);
        const SdrObject* pObj = dynamic_cast<SdrObject*>(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier());
    }

    utl::TempFile tempFile2;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile2 );

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice",
            "Requires",
            "a14");
        assertXPathContent(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/a14:m/m:oMath/m:r[1]/m:t",
            "a");

        const SdrPage *pPage = GetPage(1, xDocShRef);
        const SdrObject* pObj = dynamic_cast<SdrObject*>(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier());
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testMathObjectPPT2010()
{
    // Check import / export of math object
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/Math.pptx"), PPTX);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile1);

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice",
            "Requires",
            "a14");
        assertXPathContent(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/a14:m/m:oMath/m:sSup/m:e/m:r[1]/m:t",
            OUString::fromUtf8("\xf0\x9d\x91\x8e")); // non-BMP char

        const SdrPage *pPage = GetPage(1, xDocShRef);
        const SdrObject* pObj = dynamic_cast<SdrObject*>(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier());
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf80224()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf80224.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph->getStart(), uno::UNO_QUERY_THROW );

    sal_Int32 nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6644396), nCharColor);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf91378()
{

    //Check For Import and Export Both
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf91378.pptx"), PPTX);
    for( sal_uInt32 i=0;i<2;i++)
    {
      SdDrawDocument *pDoc = xDocShRef->GetDoc();
      CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
      uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
      uno::Reference<document::XDocumentProperties> xProps( xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY );
      uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
      OUString propValue;
      xUDProps->getPropertyValue("Testing") >>= propValue;
      CPPUNIT_ASSERT(propValue.isEmpty());
      xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    }
    xDocShRef->DoClose();
}

bool checkTransitionOnPage(uno::Reference<drawing::XDrawPagesSupplier> const & xDoc, sal_Int32 nSlideNumber,
                           sal_Int16 nExpectedTransitionType, sal_Int16 nExpectedTransitionSubType,
                           bool bExpectedDirection = true)
{
    sal_Int32 nSlideIndex = nSlideNumber - 1;

    CPPUNIT_ASSERT_MESSAGE("Slide/Page index out of range", nSlideIndex < xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nSlideIndex), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    sal_Int16 nTransitionType = 0;
    xPropSet->getPropertyValue("TransitionType") >>= nTransitionType;

    if (nExpectedTransitionType != nTransitionType)
    {
        std::cerr << "Transition type: " << nTransitionType << " " << nExpectedTransitionType << std::endl;
        return false;
    }

    sal_Int16 nTransitionSubtype = 0;
    xPropSet->getPropertyValue("TransitionSubtype") >>= nTransitionSubtype;
    if (nExpectedTransitionSubType != nTransitionSubtype)
    {
        std::cerr << "Transition Subtype: " << nTransitionSubtype << " " << nExpectedTransitionSubType << std::endl;
        return false;
    }

    bool bDirection = false;
    xPropSet->getPropertyValue("TransitionDirection") >>= bDirection;

    if (bExpectedDirection != bDirection)
    {
        std::cerr << "Transition Direction: " << (bExpectedDirection ? "normal" : "reversed")
                  << " "                      << (bDirection ? "normal" : "reversed") << std::endl;
        return false;
    }

    return true;
}

void SdOOXMLExportTest2::testExportTransitionsPPTX()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/AllTransitions.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);

    // WIPE TRANSITIONS
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 01, TransitionType::BARWIPE, TransitionSubType::TOPTOBOTTOM, false));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 02, TransitionType::BARWIPE, TransitionSubType::LEFTTORIGHT));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 03, TransitionType::BARWIPE, TransitionSubType::LEFTTORIGHT, false));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 04, TransitionType::BARWIPE, TransitionSubType::TOPTOBOTTOM));

    // CUT THROUGH BLACK
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 31, TransitionType::BARWIPE, TransitionSubType::FADEOVERCOLOR));

    // COMB
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 41, TransitionType::PUSHWIPE, TransitionSubType::COMBHORIZONTAL));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 42, TransitionType::PUSHWIPE, TransitionSubType::COMBVERTICAL));

    // OUTSIDE TURNING CUBE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 57, TransitionType::MISCSHAPEWIPE, TransitionSubType::CORNERSOUT));
    // INSIDE TURNING CUBE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 60, TransitionType::MISCSHAPEWIPE, TransitionSubType::CORNERSIN));

    // FALL
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 61, TransitionType::MISCSHAPEWIPE, TransitionSubType::LEFTTORIGHT));

    // VORTEX
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 70, TransitionType::MISCSHAPEWIPE, TransitionSubType::VERTICAL));

    // RIPPLE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 71, TransitionType::MISCSHAPEWIPE, TransitionSubType::HORIZONTAL));

    // GLITTER
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 72, TransitionType::MISCSHAPEWIPE, TransitionSubType::DIAMOND));

    // HONEYCOMB
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 73, TransitionType::MISCSHAPEWIPE, TransitionSubType::HEART));

    // NEWSFLASH
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 74, TransitionType::ZOOM, TransitionSubType::ROTATEIN));

    // OVAL VERTICAL - cannot be exported to PPTX so fallback to circle
    //CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 76, TransitionType::ELLIPSEWIPE, TransitionSubType::VERTICAL));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 76, TransitionType::ELLIPSEWIPE, TransitionSubType::CIRCLE));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testPresetShapesExport()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/preset-shapes-export.odp"), ODP);
    const sal_Char *sShapeTypeAndValues[] =
    {
        "wedgeEllipseCallout",
        "adj1","val 45310",
        "adj2","val 97194",
        "wedgeRoundRectCallout",
        "adj1","val 46694",
        "adj2","val 129726",
        "adj3","val 16667",
        "wedgeRectCallout",
        "adj1","val 40037",
        "adj2","val 111694",
        "smileyFace",
        "adj","val -9282",
        "can",
        "adj","val 50000",
        "frame",
        "adj1","val 10490",
        "donut",
        "adj","val 9601",
        "bevel",
        "adj","val 42587",
        "foldedCorner",
        "adj","val 10750",
        "verticalScroll",
        "adj","val 25000",
        "horizontalScroll",
        "adj","val 25000",
        "cube",
        "adj","val 85129",
        "bracketPair",
        "adj","val 50000",
        "sun",
        "adj","val 12500",
        "bracePair",
        "adj","val 25000",
        "cloudCallout",
        "adj1","val 77611",
        "adj2","val -47819",
        "borderCallout1",
        "adj1","val 18750",
        "adj2","val -8333",
        "adj3","val 170013",
        "adj4","val 143972",
        "borderCallout2",
        "adj1","val 18750",
        "adj2","val -8333",
        "adj3","val 113768",
        "adj4","val -81930",
        "adj5","val -22375",
        "adj6","val -134550",
        "blockArc",
        "adj1","val 13020000",
        "adj2","val 19380000",
        "adj3","val 3773",
    };

    utl::TempFile tempFile;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile );

    xmlDocPtr pXmlDocCT = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPattern( "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom[@prst='_T_']/a:avLst/a:gd[_N_]" );
    const OString sT( "_T_" );
    const OString sN( "_N_" );
    const OString sPropertyName("name");
    const OString sPropertyFmla("fmla");

    size_t i = 0;
    while(i < SAL_N_ELEMENTS( sShapeTypeAndValues )) {
        OString sType = OString( sShapeTypeAndValues[ i++ ] );
        for ( size_t j = 1 ; i < SAL_N_ELEMENTS( sShapeTypeAndValues ) && OString(sShapeTypeAndValues[i]).startsWith("adj") ; ++j ) {
            OString sXPath= sPattern.replaceFirst( sT, sType).replaceFirst( sN, OString::number(j) );
            assertXPath(pXmlDocCT, sXPath, sPropertyName , OUString::createFromAscii(sShapeTypeAndValues[ i++ ]) );
            assertXPath(pXmlDocCT, sXPath, sPropertyFmla , OUString::createFromAscii(sShapeTypeAndValues[ i++ ]) );
        }
    }
}

void SdOOXMLExportTest2::testTdf92527()
{
    // We draw a diamond in an empty document. A newly created diamond shape does not have
    // CustomShapeGeometry - Path - Segments property, and previously DrawingML exporter
    // did not export custom shapes which did not have CustomShapeGeometry - Path - Segments property.
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/empty.fodp"), FODG );
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape1(xFactory->createInstance("com.sun.star.drawing.CustomShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc1(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage1(xDoc1->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    xPage1->add(xShape1);
    xShape1->setSize(awt::Size(10000, 10000));
    xShape1->setPosition(awt::Point(1000, 1000));
    uno::Sequence<beans::PropertyValue> aShapeGeometry(comphelper::InitPropertySequence(
        {
            {"Type", uno::makeAny(OUString("diamond"))},
        }));
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);
    xPropertySet1->setPropertyValue("CustomShapeGeometry", uno::makeAny(aShapeGeometry));

    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc2(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage2(xDoc2->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xPage2->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropertySet2( xShape2, uno::UNO_QUERY_THROW );
    uno::Sequence<beans::PropertyValue> aProps;
    xPropertySet2->getPropertyValue("CustomShapeGeometry") >>= aProps;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get< uno::Sequence<beans::PropertyValue> >();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            aCoordinates = rProp.Value.get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    }

    // 5 coordinate pairs, 1 MoveTo, 4 LineTo
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aCoordinates.getLength());
    xDocShRef->DoClose();
}

namespace {

void matchNumberFormat( int nPage, uno::Reference< text::XTextField > const & xField)
{
    uno::Reference< beans::XPropertySet > xPropSet( xField, uno::UNO_QUERY_THROW );
    sal_Int32 nNumFmt;
    xPropSet->getPropertyValue("NumberFormat") >>= nNumFmt;
    switch( nPage )
    {
        case 0:     // 13/02/96
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(2), nNumFmt);
                    break;
        case 1:     // 13/02/1996
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(5), nNumFmt);
                    break;
        case 2:     // 13 February 1996
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(3), nNumFmt);
                    break;
        case 3:     // 13:49:38
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(2), nNumFmt);
                    break;
        case 4:     // 13:49
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(3), nNumFmt);
                    break;
        case 5:     // 01:49 PM
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(6), nNumFmt);
                    break;
        case 6:     // 01:49:38 PM
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(7), nNumFmt);
    }
}

}

void SdOOXMLExportTest2::testDatetimeFieldNumberFormat()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/numfmt.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 6; ++i)
    {
        matchNumberFormat( i, getTextFieldFromPage(0, 0, 0, i, xDocShRef) );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testDatetimeFieldNumberFormatPPTX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/numfmt.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 6; ++i)
    {
        matchNumberFormat( i, getTextFieldFromPage(0, 0, 0, i, xDocShRef) );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideNumberField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/slidenum_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideNumberFieldPPTX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/slidenum_field.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideCountField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/slidecount_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideNameField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/slidename_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testExtFileField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/extfile_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 3; ++i)
    {
        uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, i, 0, xDocShRef);
        CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

        uno::Reference< beans::XPropertySet > xPropSet( xField, uno::UNO_QUERY_THROW );
        sal_Int32 nNumFmt;
        xPropSet->getPropertyValue("FileFormat") >>= nNumFmt;
        switch( i )
        {
            case 0:     // Path/File name
                        CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(0), nNumFmt);
                        break;
            case 1:     // Path
                        CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(1), nNumFmt);
                        break;
            case 2:     // File name without extension
                        CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(2), nNumFmt);
                        break;
            case 3:     // File name with extension
                        CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(3), nNumFmt);
        }
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testAuthorField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/author_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf99224()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf99224.odp"), ODP);
    xShell = saveAndReload(xShell.get(), PPTX);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    // This was 0: the image with text was lost on export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xPage->getCount());
    xShell->DoClose();
}

void SdOOXMLExportTest2::testTdf92076()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf92076.odp"), ODP);
    xShell = saveAndReload(xShell.get(), PPTX);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());
    xShell->DoClose();
}

void SdOOXMLExportTest2::testTdf59046()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf59046.odp"), ODP);
    utl::TempFile tempFile;
    xShell = saveAndReload(xShell.get(), PPTX, &tempFile);
    xShell->DoClose();
    xmlDocPtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", 1);
}

void SdOOXMLExportTest2::testTdf105739()
{
    // Gradient was lost during saving to ODP
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf105739.pptx"), PPTX);
    utl::TempFile tempFile;
    xShell = saveAndReload(xShell.get(), ODP, &tempFile);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    uno::Any aAny = xPropSet->getPropertyValue("Background");
    CPPUNIT_ASSERT(aAny.hasValue());
    if (aAny.hasValue())
    {
        uno::Reference< beans::XPropertySet > aXBackgroundPropSet;
        aAny >>= aXBackgroundPropSet;

        // Test fill type
        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        aXBackgroundPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, aFillStyle);

        // Test gradient properties
        com::sun::star::awt::Gradient aFillGradient;
        aXBackgroundPropSet->getPropertyValue("FillGradient") >>= aFillGradient;
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aFillGradient.Style);
        CPPUNIT_ASSERT_EQUAL(util::Color(0xff0000), aFillGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(util::Color(0x00b050), aFillGradient.EndColor);
    }

    xShell->DoClose();
}

void SdOOXMLExportTest2::testTdf111863()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf111863.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check that transition attribute didn't change from 'out' to 'in'
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animEffect",
        "transition", "out");
}

void SdOOXMLExportTest2::testTdf111518()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf111518.pptx"), PPTX);
    utl::TempFile tempFile;
    tempFile.EnableKillingFile(false);
    xShell = saveAndReload(xShell.get(), PPTX, &tempFile);
    xShell->DoClose();

    xmlDocPtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels,
            "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animMotion",
            "path",
            "M -3.54167E-6 -4.81481E-6 L 0.39037 -0.00069");
}

void SdOOXMLExportTest2::testTdf106867()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf106867.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    const SdrPage *pPage = GetPage(1, xDocShRef.get());

    // first check that we have the media object
    const SdrMediaObj* pMediaObj = dynamic_cast<SdrMediaObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT_MESSAGE("no media object", pMediaObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(OUString("vnd.sun.star.Package:ppt/media/media1.avi"), pMediaObj->getURL());

    xDocShRef->DoClose();

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());
    // check that the document contains the video stream
    CPPUNIT_ASSERT(xNameAccess->hasByName("ppt/media/media1.avi"));

    // both the ooxml and the extended markup
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/p:extLst/p:ext/p14:media");

    // target the shape with the video in the command
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:cmd/p:cBhvr/p:tgtEl/p:spTgt",
            "spid", "42");
}

void SdOOXMLExportTest2::testTdf112280()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf112280.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check the animRot value
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animRot",
            "by", "21600000");
}

void SdOOXMLExportTest2::testTdf112552()
{
    // Background fill was not displayed, but it was because of the wrong geometry
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf112552.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", "w", "21600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", "h", "21600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "x", "21600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "y", "0");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf112557()
{
    // Subtitle shape should be skipped by export.
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf112557.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slideMasters/slideMaster1.xml");
    assertXPath(pXmlDocContent, "/p:sldMaster/p:cSld/p:spTree/p:sp", 2); // title and object
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf112088()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf112088.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check gradient stops
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:gradFill/a:gsLst", 2);
}

void SdOOXMLExportTest2::testTdf112333()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf112333.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    OUString sTo = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set[1]/p:to/p:strVal", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("solid"), sTo);

    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set[1]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("fill.type"), sAttributeName);

    sTo = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set[2]/p:to/p:strVal", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("true"), sTo);

    sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set[2]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("fill.on"), sAttributeName);

    sTo = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animClr/p:to/a:srgbClr", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("0563c1"), sTo);

    sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animClr/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("fillcolor"), sAttributeName);
}

void SdOOXMLExportTest2::testTdf112089()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf112089.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    OUString sID = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr", "id");
    OUString sTarget = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt", "spid");
    CPPUNIT_ASSERT_EQUAL(sID, sTarget);
}

void SdOOXMLExportTest2::testTdf112334()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf112334.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animClr[1]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("style.color"), sAttributeName);
}

void SdOOXMLExportTest2::testTdf112647()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf112647.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference<text::XTextRange> xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    css::style::LineSpacing aLineSpacing;
    xPropSet->getPropertyValue("ParaLineSpacing") >>= aLineSpacing;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(css::style::LineSpacingMode::FIX), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2117), aLineSpacing.Height);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf112086()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf112086.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    OUString sVal = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]/p:tavLst/p:tav/p:val/p:fltVal", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("0"), sVal);

    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[1]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("ppt_w"), sAttributeName);

    sVal = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]/p:tavLst/p:tav/p:val/p:fltVal", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("0"), sVal);

    sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("ppt_h"), sAttributeName);
}

void SdOOXMLExportTest2::testTdf104788()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104788.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide6.xml");

    OUString sVal = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]", "to");
    CPPUNIT_ASSERT_EQUAL(OUString("-1.0"), sVal);

    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("xshear"), sAttributeName);
    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
