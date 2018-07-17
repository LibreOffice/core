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

#include <sdpage.hxx>

using namespace css;
using namespace css::animations;

class SdOOXMLExportTest2 : public SdModelTestBaseXML
{
public:
    void testTdf93883();
    void testTdf91378();
    void testBnc822341();
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
    void testPageBitmapWithTransparency();
    void testPptmContentType();
    void testTdf111798();
    void testPptmVBAStream();
    void testTdf111863();
    void testTdf111518();
    void testTdf100387();
    void testRotateFlip();
    void testTdf106867();
    void testTdf112280();
    void testTdf112088();
    void testTdf112333();
    void testTdf112552();
    void testTdf112557();
    void testTdf112334();
    void testTdf112089();
    void testTdf112086();
    void testTdf112647();
    void testGroupRotation();
    void testTdf104788();
    void testSmartartRotation2();
    void testTdf114845_rotateShape();
    void testGroupsPosition();
    void testGroupsRotatedPosition();
    void testAccentColor();
    void testThemeColors();
    void testTdf114848();
    void testTdf68759();
    void testTdf90626();
    void testTdf107608();
    void testTdf111786();
    void testFontScale();
    void testTdf115394();
    void testTdf115394Zero();
    void testTdf115005();
    int testTdf115005_FallBack_Images(bool bAddReplacementImages);
    void testTdf115005_FallBack_Images_On();
    void testTdf115005_FallBack_Images_Off();
    void testTdf118806();
    void testTdf111789();
    /// SmartArt animated elements
    void testTdf104792();
    void testTdf90627();
    void testTdf104786();
    void testTdf118783();
    void testTdf104789();
    void testOpenDocumentAsReadOnly();
    void testTdf118835();
    void testTdf118768();
    void testTdf118836();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest2);

    CPPUNIT_TEST(testTdf93883);
    CPPUNIT_TEST(testTdf91378);
    CPPUNIT_TEST(testBnc822341);
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
    CPPUNIT_TEST(testPageBitmapWithTransparency);
    CPPUNIT_TEST(testPptmContentType);
    CPPUNIT_TEST(testTdf111798);
    CPPUNIT_TEST(testPptmVBAStream);
    CPPUNIT_TEST(testTdf111863);
    CPPUNIT_TEST(testTdf111518);
    CPPUNIT_TEST(testTdf100387);
    CPPUNIT_TEST(testRotateFlip);
    CPPUNIT_TEST(testTdf106867);
    CPPUNIT_TEST(testTdf112280);
    CPPUNIT_TEST(testTdf112088);
    CPPUNIT_TEST(testTdf112333);
    CPPUNIT_TEST(testTdf112552);
    CPPUNIT_TEST(testTdf112557);
    CPPUNIT_TEST(testTdf112334);
    CPPUNIT_TEST(testTdf112089);
    CPPUNIT_TEST(testTdf112086);
    CPPUNIT_TEST(testTdf112647);
    CPPUNIT_TEST(testGroupRotation);
    CPPUNIT_TEST(testTdf104788);
    CPPUNIT_TEST(testSmartartRotation2);
    CPPUNIT_TEST(testTdf114845_rotateShape);
    CPPUNIT_TEST(testGroupsPosition);
    CPPUNIT_TEST(testGroupsRotatedPosition);
    CPPUNIT_TEST(testAccentColor);
    CPPUNIT_TEST(testThemeColors);
    CPPUNIT_TEST(testTdf114848);
    CPPUNIT_TEST(testTdf68759);
    CPPUNIT_TEST(testTdf90626);
    CPPUNIT_TEST(testTdf107608);
    CPPUNIT_TEST(testTdf111786);
    CPPUNIT_TEST(testFontScale);
    CPPUNIT_TEST(testTdf115394);
    CPPUNIT_TEST(testTdf115394Zero);
    CPPUNIT_TEST(testTdf115005);
    CPPUNIT_TEST(testTdf115005_FallBack_Images_On);
    CPPUNIT_TEST(testTdf115005_FallBack_Images_Off);
    CPPUNIT_TEST(testTdf118806);
    CPPUNIT_TEST(testTdf111789);
    CPPUNIT_TEST(testTdf104792);
    CPPUNIT_TEST(testTdf90627);
    CPPUNIT_TEST(testTdf104786);
    CPPUNIT_TEST(testTdf118783);
    CPPUNIT_TEST(testTdf104789);
    CPPUNIT_TEST(testOpenDocumentAsReadOnly);
    CPPUNIT_TEST(testTdf118835);
    CPPUNIT_TEST(testTdf118768);
    CPPUNIT_TEST(testTdf118836);

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

        const SdrObject* pObj = pPage->GetObj(0);
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

        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    xDocShRef->DoClose();
}

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
        const SdrObject* pObj = pPage->GetObj(0);
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
        const SdrObject* pObj = pPage->GetObj(0);
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
            u"\U0001D44E"); // non-BMP char

        const SdrPage *pPage = GetPage(1, xDocShRef);
        const SdrObject* pObj = pPage->GetObj(0);
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
        CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(aFillStyle));

        // Test gradient properties
        com::sun::star::awt::Gradient aFillGradient;
        aXBackgroundPropSet->getPropertyValue("FillGradient") >>= aFillGradient;
        CPPUNIT_ASSERT_EQUAL(int(awt::GradientStyle_LINEAR), static_cast<int>(aFillGradient.Style));
        CPPUNIT_ASSERT_EQUAL(util::Color(0xff0000), aFillGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(util::Color(0x00b050), aFillGradient.EndColor);
    }

    xShell->DoClose();
}

void SdOOXMLExportTest2::testPageBitmapWithTransparency()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/page_transparent_bitmap.pptx"), PPTX );

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide background transparency is wrong", sal_Int32(49), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testPptmContentType()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptm/macro.pptm"), PPTM);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTM, &tempFile);

    // Assert that the content type is the one of PPTM
    xmlDocPtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
                "ContentType",
                "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf111798()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf111798.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocPtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    const OUString data[][29] =
    {
        {
            "2700000", "2458080", "2414880", "1439640", "1440000",
            "moveTo",  "0",    "3000",
            "lnTo[1]", "3000", "3000",
            "lnTo[2]", "3000", "4000",
            "lnTo[3]", "4000", "2000",
            "lnTo[4]", "3000", "0",
            "lnTo[5]", "3000", "1000",
            "lnTo[6]", "0",    "1000",
            "lnTo[7]", "0",    "3000"
        },
        {
            "2700000", "6778080", "2414880", "1439640", "1440000",
            "moveTo",  "3000", "0",
            "lnTo[1]", "3000", "3000",
            "lnTo[2]", "4000", "3000",
            "lnTo[3]", "2000", "4000",
            "lnTo[4]", "0", "3000",
            "lnTo[5]", "1000", "3000",
            "lnTo[6]", "1000", "0",
            "lnTo[7]", "3000", "0"
        }
    };

    for (size_t nShapeIndex = 0; nShapeIndex < SAL_N_ELEMENTS(data); nShapeIndex++)
    {
        size_t nDataIndex = 0;

        const OString sSpPr = "/p:sld/p:cSld/p:spTree/p:sp[" + OString::number(nShapeIndex + 1) + "]/p:spPr";
        const OString sXfrm = sSpPr + "/a:xfrm";
        assertXPath(pXmlDoc, sXfrm, "rot", data[nShapeIndex][nDataIndex++]);
        const OString sOff = sXfrm + "/a:off";
        assertXPath(pXmlDoc, sOff, "x", data[nShapeIndex][nDataIndex++]);
        assertXPath(pXmlDoc, sOff, "y", data[nShapeIndex][nDataIndex++]);
        const OString sExt = sXfrm + "/a:ext";
        assertXPath(pXmlDoc, sExt, "cx", data[nShapeIndex][nDataIndex++]);
        assertXPath(pXmlDoc, sExt, "cy", data[nShapeIndex][nDataIndex++]);

        while (nDataIndex < SAL_N_ELEMENTS(data[nShapeIndex]))
        {
            const OString sPt = sSpPr + "/a:custGeom/a:pathLst/a:path/a:" + data[nShapeIndex][nDataIndex++].toUtf8() + "/a:pt";
            assertXPath(pXmlDoc, sPt, "x", data[nShapeIndex][nDataIndex++]);
            assertXPath(pXmlDoc, sPt, "y", data[nShapeIndex][nDataIndex++]);
        }
    }
}

void SdOOXMLExportTest2::testPptmVBAStream()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptm/macro.pptm"), PPTM);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTM, &tempFile);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());
    // This failed: VBA stream was not roundtripped
    CPPUNIT_ASSERT(xNameAccess->hasByName("ppt/vbaProject.bin"));

    xDocShRef->DoClose();
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

void SdOOXMLExportTest2::testTdf100387()
{

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf100387.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn", "dur", "indefinite");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]/p:cTn", "fill", "hold");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]/p:cTn/p:childTnLst/p:par/p:cTn", "fill", "hold");

    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]"
                             "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg", "st", "0");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]"
                             "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg", "end", "0");

    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]"
                             "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg", "st", "1");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]"
                             "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg", "end", "1");

    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[3]"
                             "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg", "st", "2");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[3]"
                             "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg", "end", "2");
}

void SdOOXMLExportTest2::testRotateFlip()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/rotate_flip.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    const OUString data[][4] =
    {// flipH flipV     x          y
        { "0", "1", "1170000", "1035720" },
        { "1", "1", "7108560", "1036440" },
        { "1", "0", "4140000", "1036440" }
    };

    const OUString points[][2] = { {"221", "293"},     {"506", "12" },     {"367", "0"  },     {"29",  "406"},     {"431", "347"},    {"145", "645"},
                                   {"99",  "520"},     {"0",   "861"},     {"326", "765"},     {"209", "711"},     {"640", "233"},    {"640", "233"} };

    for (size_t nShapeIndex = 0; nShapeIndex < SAL_N_ELEMENTS(data); nShapeIndex++)
    {
        size_t nDataIndex = 0;

        const OString sSpPr = "/p:sld/p:cSld/p:spTree/p:sp[" + OString::number(nShapeIndex + 1) + "]/p:spPr";
        const OString sXfrm = sSpPr + "/a:xfrm";
        if(data[nShapeIndex][nDataIndex++] == "1")
            assertXPath(pXmlDocContent, sXfrm, "flipH", "1");
        if(data[nShapeIndex][nDataIndex++] == "1")
            assertXPath(pXmlDocContent, sXfrm, "flipV", "1");
        assertXPath(pXmlDocContent, sXfrm, "rot", "20400000");
        const OString sOff = sXfrm + "/a:off";
        assertXPath(pXmlDocContent, sOff, "x", data[nShapeIndex][nDataIndex++]);
        assertXPath(pXmlDocContent, sOff, "y", data[nShapeIndex][nDataIndex++]);
        const OString sExt = sXfrm + "/a:ext";
        assertXPath(pXmlDocContent, sExt, "cx", "1800000");
        assertXPath(pXmlDocContent, sExt, "cy", "3600000");

        for (size_t nPointIndex = 0; nPointIndex < SAL_N_ELEMENTS(points); nPointIndex++)
        {
            const OString sPt = sSpPr + "/a:custGeom/a:pathLst/a:path/a:lnTo[" + OString::number(nPointIndex + 1) + "]/a:pt";
            assertXPath(pXmlDocContent, sPt, "x", points[nPointIndex][0]);
            assertXPath(pXmlDocContent, sPt, "y", points[nPointIndex][1]);
        }
    }
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
    xDocShRef->DoClose();
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

void SdOOXMLExportTest2::testGroupRotation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/group_rotation.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:grpSpPr/a:xfrm", "rot");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:xfrm", "rot", "20400000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[2]/p:spPr/a:xfrm", "rot", "20400000");
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

void SdOOXMLExportTest2::testSmartartRotation2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-rotation2.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:txBody/a:p/a:r/a:t", "Text");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:txBody/a:bodyPr", "rot", "10800000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "x", "2276280");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "y", "3158280");
}

void SdOOXMLExportTest2::testTdf114845_rotateShape()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf114845_rotateShape.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:nvSpPr/p:cNvPr", "name", "CustomShape 5");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:xfrm", "flipV", "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:xfrm/a:off", "x", "4059000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:xfrm/a:off", "y", "3287520");
}

void SdOOXMLExportTest2::testGroupsPosition()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/group.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "x", "6796800");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "y", "4273920");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[1]/p:spPr/a:xfrm/a:off", "x", "4040640");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[1]/p:spPr/a:xfrm/a:off", "y", "4273920");
}

void SdOOXMLExportTest2::testGroupsRotatedPosition()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/group-rot.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "x", "2857320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "y", "4026960");
}

void SdOOXMLExportTest2::testAccentColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/accent-color.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:style/a:fillRef/a:schemeClr", "val", "accent6");
    xmlDocPtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp/p:style/a:fillRef/a:schemeClr", "val", "accent6");
    xmlDocPtr pXmlDocTheme1 = parseExport(tempFile, "ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:accent6/a:srgbClr", "val", "70ad47");
    xmlDocPtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:accent6/a:srgbClr", "val", "deb340");
}

void SdOOXMLExportTest2::testThemeColors()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf84205.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val", "44546a");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:accent3/a:srgbClr", "val", "a5a5a5");
}

void SdOOXMLExportTest2::testTdf114848()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf114848.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocTheme1 = parseExport(tempFile, "ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val", "1f497d");
    xmlDocPtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val", "1f497d");
}

void SdOOXMLExportTest2::testTdf68759()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf68759.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off", "x", "1687320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off", "y", "1615320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm", "rot", "9600000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off", "x", "3847320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off", "y", "1614600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off", "x", "5934960");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off", "y", "1615320");

}

void SdOOXMLExportTest2::testTdf90626()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf90626.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocPtr pXmlDocContent =  parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buSzPct", "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buSzPct", "val", "150568");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buSzPct", "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr/a:buSzPct", "val", "150568");
}

void SdOOXMLExportTest2::testTdf107608()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf107608.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY_THROW );

    drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
    xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, aFillStyle);

    bool bBackgroundFill = false;
    xPropSet->getPropertyValue("FillBackground") >>= bBackgroundFill;
    CPPUNIT_ASSERT(bBackgroundFill);

    sal_Int32 nBackgroundColor;
    xPropSet->getPropertyValue("FillColor") >>= nBackgroundColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x92D050), nBackgroundColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf111786()
{
    // Export line transparency with the color
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf111786.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY_THROW );

    sal_uInt32 nLineColor;
    xPropSet->getPropertyValue("LineColor") >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0x3465A4), nLineColor);

    sal_Int16 nTransparency;
    xPropSet->getPropertyValue("LineTransparence") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(33), nTransparency);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testFontScale()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/font-scale.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    // Rounding errors possible, approximate value
    OUString sScale = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:normAutofit", "fontScale");
    if (sScale != "73000" && sScale != "72000" && sScale != "74000")
        CPPUNIT_ASSERT_EQUAL(OUString("73000"), sScale);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf115394()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf115394.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    double fTransitionDuration;

    // Slow in MS formats
    SdPage* pPage1 = xDocShRef->GetDoc()->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);

    // Medium in MS formats
    SdPage* pPage2 = xDocShRef->GetDoc()->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Fast in MS formats
    SdPage* pPage3 = xDocShRef->GetDoc()->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Custom values
    SdPage* pPage4 = xDocShRef->GetDoc()->GetSdPage(3, PageKind::Standard);
    fTransitionDuration = pPage4->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.25, fTransitionDuration);

    SdPage* pPage5 = xDocShRef->GetDoc()->GetSdPage(4, PageKind::Standard);
    fTransitionDuration = pPage5->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(4.25, fTransitionDuration);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf115394Zero()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf115394-zero.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    double fTransitionDuration;

    SdPage* pPage = xDocShRef->GetDoc()->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.01, fTransitionDuration);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf115005()
{
    sd::DrawDocShellRef xDocShRefOriginal = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf115005.odp"), ODP);
    utl::TempFile tempFile;
    sd::DrawDocShellRef xDocShRefResaved = saveAndReload(xDocShRefOriginal.get(), ODP, &tempFile);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());

    // check that the document contains original vector images
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSVMFiles = 0;
    for (int i=0; i<names.getLength(); i++)
    {
        if(names[i].endsWith(".svm"))
            nSVMFiles++;
    }
    CPPUNIT_ASSERT_EQUAL(3, nSVMFiles);
}

int SdOOXMLExportTest2::testTdf115005_FallBack_Images(bool bAddReplacementImages)
{
    sd::DrawDocShellRef xDocShRefOriginal = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf115005_no_fallback_images.odp"), ODP);

    // check if fallback images were not created if AddReplacementImages=true/false
    // set AddReplacementImages
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
        if ( !officecfg::Office::Common::Save::Graphic::AddReplacementImages::isReadOnly() )
            officecfg::Office::Common::Save::Graphic::AddReplacementImages::set(bAddReplacementImages, batch);
        batch->commit();
    }

    // save the file with already set options
    utl::TempFile tempFile;
    sd::DrawDocShellRef xDocShRefResaved = saveAndReload(xDocShRefOriginal.get(), ODP, &tempFile);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());

    // check that the document contains original vector images
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSVMFiles = 0;
    int nPNGFiles = 0;
    for (int i=0; i<names.getLength(); i++)
    {
        if(names[i].endsWith(".svm"))
            nSVMFiles++;
        if(names[i].endsWith(".png"))
            nPNGFiles++;
    }

    // check results
    CPPUNIT_ASSERT_EQUAL(1, nSVMFiles);
    return nPNGFiles;
}

void SdOOXMLExportTest2::testTdf115005_FallBack_Images_On()
{
    const int nPNGFiles = testTdf115005_FallBack_Images(true);
    CPPUNIT_ASSERT_EQUAL(1, nPNGFiles);
}

void SdOOXMLExportTest2::testTdf115005_FallBack_Images_Off()
{
    const int nPNGFiles = testTdf115005_FallBack_Images(false);
    CPPUNIT_ASSERT_EQUAL(0, nPNGFiles);
}

void SdOOXMLExportTest2::testTdf118806()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf118806.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animMotion", "origin", "layout");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf111789()
{
    // Shadow properties were not exported for text shapes.
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf111789.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // First text shape has some shadow
    {
        uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
        bool bHasShadow = false;
        xShape->getPropertyValue("Shadow") >>= bHasShadow;
        CPPUNIT_ASSERT(bHasShadow);
        double fShadowDist = 0.0;
        xShape->getPropertyValue("ShadowXDistance") >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(273), fShadowDist);
        xShape->getPropertyValue("ShadowYDistance") >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(273), fShadowDist);
        sal_Int32 nColor = 0;
        xShape->getPropertyValue("ShadowColor") >>= nColor;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xFF0000), nColor);
        sal_Int32 nTransparency = 0;
        xShape->getPropertyValue("ShadowTransparence") >>= nTransparency;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7), nTransparency);
    }

    // Second text shape has no shadow
    {
        uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 1, 0, xDocShRef ) );
        bool bHasShadow = false;
        xShape->getPropertyValue("Shadow") >>= bHasShadow;
        CPPUNIT_ASSERT(!bHasShadow);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf104792()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104792-smart-art-animation.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/p:par[1]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:set/p:cBhvr/p:tgtEl/p:spTgt", 1);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf90627()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf90627.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    // Don't export empty conditions
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:endCondLst", 0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf104786()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104786.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide2.xml");
    // Don't export empty 'to'
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst/p:set[2]/p:to", 0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf118783()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf118783.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check that transition attribute didn't change from 'out' to 'in'
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sAttributeName = getXPathContent(pXmlDocContent, "//p:animRot/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("r"), sAttributeName);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf104789()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104789.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("style.opacity"), sAttributeName);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testOpenDocumentAsReadOnly()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/open-as-read-only.pptx"), PPTX);
    CPPUNIT_ASSERT(xDocShRef->IsSecurityOptOpenReadOnly());
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    CPPUNIT_ASSERT(xDocShRef->IsSecurityOptOpenReadOnly());
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf118835()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf118835.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "(//p:animClr)[1]", "clrSpc", "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[1]//p:attrName", "style.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[1]//p:to/a:srgbClr", "val", "ed1c24");

    assertXPath(pXmlDocContent, "(//p:animClr)[2]", "clrSpc", "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[2]//p:attrName", "stroke.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[2]//p:to/a:srgbClr", "val", "333399");

    assertXPath(pXmlDocContent, "(//p:animClr)[3]", "clrSpc", "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[3]//p:attrName", "fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[3]//p:to/a:srgbClr", "val", "fcd3c1");

    assertXPath(pXmlDocContent, "(//p:animClr)[5]", "clrSpc", "hsl");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[5]//p:attrName", "fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "h", "10800000");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "s", "0");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "l", "0");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf118768()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf118768-brake.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // check that transition attribute didn't change from 'out' to 'in'
    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:anim[1]", "from", "(-#ppt_w/2)");
    assertXPath(pXmlDocContent, "//p:anim[1]", "to", "(#ppt_x)");
    assertXPath(pXmlDocContent, "//p:anim[2]", "from", "0");

    assertXPath(pXmlDocContent, "//p:anim[2]", "to", "-1");
    assertXPath(pXmlDocContent, "//p:anim[2]/p:cBhvr/p:cTn", "autoRev", "1");

    assertXPath(pXmlDocContent, "//p:anim[3]", "by", "(#ppt_h/3+#ppt_w*0.1)");
    assertXPath(pXmlDocContent, "//p:anim[3]/p:cBhvr/p:cTn", "autoRev", "1");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf118836()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/odp/tdf118836.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocPtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "x", "250000");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "y", "250000");
    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
