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
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdotable.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <sdpage.hxx>
#include <cfloat>
#include <cstdlib>

#include <rtl/character.hxx>

using namespace css;
using namespace css::animations;

namespace {

bool checkBeginWithNumber(const OUString& rStr)
{
    sal_Unicode aChar = (rStr.getLength() > 1) ? rStr[0] : '\0';
    return aChar == '.' || aChar == '-' || rtl::isAsciiDigit(aChar);
}

}

#define CPPUNIT_ASSERT_MOTIONPATH(expect, actual) \
      assertMotionPath(expect, actual, CPPUNIT_SOURCELINE())

static void assertMotionPath(const OUString &rStr1, const OUString &rStr2, const CppUnit::SourceLine &rSourceLine)
{
    sal_Int32 nIdx1 = 0;
    sal_Int32 nIdx2 = 0;

    OString sMessage = "Motion path values mismatch.\nExpect: " +
            OUStringToOString(rStr1, RTL_TEXTENCODING_UTF8) +
            "\nActual: " + OUStringToOString(rStr2, RTL_TEXTENCODING_UTF8);

    while(nIdx1 != -1 && nIdx2 != -1)
    {
        OUString aToken1 = rStr1.getToken(0, ' ', nIdx1);
        OUString aToken2 = rStr2.getToken(0, ' ', nIdx2);

        if (checkBeginWithNumber(aToken1) && checkBeginWithNumber(aToken2))
            assertDoubleEquals(aToken1.toDouble(), aToken2.toDouble(), DBL_EPSILON, rSourceLine, sMessage.getStr());
        else
            assertEquals(aToken1, aToken2, rSourceLine, sMessage.getStr());
    }
    assertEquals(sal_Int32(-1), nIdx1, rSourceLine, sMessage.getStr());
    assertEquals(sal_Int32(-1), nIdx2, rSourceLine, sMessage.getStr());
}

class SdOOXMLExportTest2 : public SdModelTestBaseXML
{
public:
    void testTdf131905();
    void testTdf93883();
    void testTdf91378();
    void testBnc822341();
    void testMathObject();
    void testMathObjectPPT2010();
    void testTdf119015();
    void testTdf123090();
    void testTdf126324();
    void testTdf119187();
    void testTdf132472();
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
    void testTdf133502();
    void testTdf105739();
    void testPageBitmapWithTransparency();
    void testPptmContentType();
    void testTdf111798();
    void testPptmVBAStream();
    void testTdf111863();
    void testTdf111518();
    void testTdf100387();
    void testClosingShapesAndLineCaps();
    void testRotateFlip();
    void testTdf106867();
    void testTdf112280();
    void testTdf112088();
    void testTdf112333();
    void testTdf112552();
    void testTdf112557();
    void testTdf128049();
    void testTdf106026();
    void testTdf112334();
    void testTdf112089();
    void testTdf112086();
    void testTdf112647();
    void testGroupRotation();
    void testTdf104788();
    void testSmartartRotation2();
    void testTdf91999_rotateShape();
    void testTdf114845_rotateShape();
    void testGroupsPosition();
    void testGroupsRotatedPosition();
    void testAccentColor();
    void testThemeColors();
    void testTdf114848();
    void testTdf68759();
    void testTdf127901();
    void testTdf48735();
    void testTdf90626();
    void testTdf107608();
    void testTdf111786();
    void testFontScale();
    void testShapeAutofitPPTX();
    void testLegacyShapeAutofitPPTX();
    void testTdf115394();
    void testTdf115394Zero();
    void testTdf115005();
    int testTdf115005_FallBack_Images(bool bAddReplacementImages);
    void testTdf115005_FallBack_Images_On();
    void testTdf115005_FallBack_Images_Off();
    void testTdf118806();
    void testTdf130058();
    void testTdf111789();
    void testTdf100348_convert_Fontwork2TextWarp();
    void testTdf1225573_FontWorkScaleX();
    void testTdf99497_keepAppearanceOfCircleKind();
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
    void testTdf116350TextEffects();
    void testTdf128096();
    void testTdf120573();
    void testTdf118825();
    void testTdf119118();
    void testTdf99213();
    void testPotxExport();
    void testTdf44223();
    void testSmartArtPreserve();
    void testTdf125346();
    void testTdf125346_2();
    void testTdf125360();
    void testTdf125360_1();
    void testTdf125360_2();
    void testTdf125551();
    void testTdf136830();
    void testTdf126234();
    void testTdf126741();
    void testTdf127372();
    void testTdf127379();
    void testTdf98603();
    void testTdf79082();
    void testTdf128213();
    void testTdf129372();
    void testShapeGlowEffect();
    void testTdf119087();
    void testTdf131554();
    void testTdf132282();
    void testTdf132201EffectOrder();
    void testShapeSoftEdgeEffect();
    void testShapeShadowBlurEffect();
    void testTdf119223();
    void testTdf128213ShapeRot();
    void testTdf125560_textDeflate();
    void testTdf125560_textInflateTop();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest2);

    CPPUNIT_TEST(testTdf131905);
    CPPUNIT_TEST(testTdf93883);
    CPPUNIT_TEST(testTdf91378);
    CPPUNIT_TEST(testBnc822341);
    CPPUNIT_TEST(testMathObject);
    CPPUNIT_TEST(testMathObjectPPT2010);
    CPPUNIT_TEST(testTdf119015);
    CPPUNIT_TEST(testTdf123090);
    CPPUNIT_TEST(testTdf126324);
    CPPUNIT_TEST(testTdf119187);
    CPPUNIT_TEST(testTdf132472);
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
    CPPUNIT_TEST(testTdf133502);
    CPPUNIT_TEST(testTdf105739);
    CPPUNIT_TEST(testPageBitmapWithTransparency);
    CPPUNIT_TEST(testPptmContentType);
    CPPUNIT_TEST(testTdf111798);
    CPPUNIT_TEST(testPptmVBAStream);
    CPPUNIT_TEST(testTdf111863);
    CPPUNIT_TEST(testTdf111518);
    CPPUNIT_TEST(testTdf100387);
    CPPUNIT_TEST(testClosingShapesAndLineCaps);
    CPPUNIT_TEST(testRotateFlip);
    CPPUNIT_TEST(testTdf106867);
    CPPUNIT_TEST(testTdf112280);
    CPPUNIT_TEST(testTdf112088);
    CPPUNIT_TEST(testTdf112333);
    CPPUNIT_TEST(testTdf112552);
    CPPUNIT_TEST(testTdf112557);
    CPPUNIT_TEST(testTdf128049);
    CPPUNIT_TEST(testTdf106026);
    CPPUNIT_TEST(testTdf112334);
    CPPUNIT_TEST(testTdf112089);
    CPPUNIT_TEST(testTdf112086);
    CPPUNIT_TEST(testTdf112647);
    CPPUNIT_TEST(testGroupRotation);
    CPPUNIT_TEST(testTdf104788);
    CPPUNIT_TEST(testSmartartRotation2);
    CPPUNIT_TEST(testTdf91999_rotateShape);
    CPPUNIT_TEST(testTdf114845_rotateShape);
    CPPUNIT_TEST(testGroupsPosition);
    CPPUNIT_TEST(testGroupsRotatedPosition);
    CPPUNIT_TEST(testAccentColor);
    CPPUNIT_TEST(testThemeColors);
    CPPUNIT_TEST(testTdf114848);
    CPPUNIT_TEST(testTdf68759);
    CPPUNIT_TEST(testTdf127901);
    CPPUNIT_TEST(testTdf48735);
    CPPUNIT_TEST(testTdf90626);
    CPPUNIT_TEST(testTdf107608);
    CPPUNIT_TEST(testTdf111786);
    CPPUNIT_TEST(testFontScale);
    CPPUNIT_TEST(testShapeAutofitPPTX);
    CPPUNIT_TEST(testLegacyShapeAutofitPPTX);
    CPPUNIT_TEST(testTdf115394);
    CPPUNIT_TEST(testTdf115394Zero);
    CPPUNIT_TEST(testTdf115005);
    CPPUNIT_TEST(testTdf115005_FallBack_Images_On);
    CPPUNIT_TEST(testTdf115005_FallBack_Images_Off);
    CPPUNIT_TEST(testTdf118806);
    CPPUNIT_TEST(testTdf130058);
    CPPUNIT_TEST(testTdf111789);
    CPPUNIT_TEST(testTdf100348_convert_Fontwork2TextWarp);
    CPPUNIT_TEST(testTdf1225573_FontWorkScaleX);
    CPPUNIT_TEST(testTdf99497_keepAppearanceOfCircleKind);
    CPPUNIT_TEST(testTdf104792);
    CPPUNIT_TEST(testTdf90627);
    CPPUNIT_TEST(testTdf104786);
    CPPUNIT_TEST(testTdf118783);
    CPPUNIT_TEST(testTdf104789);
    CPPUNIT_TEST(testOpenDocumentAsReadOnly);
    CPPUNIT_TEST(testTdf118835);
    CPPUNIT_TEST(testTdf118768);
    CPPUNIT_TEST(testTdf118836);
    CPPUNIT_TEST(testTdf116350TextEffects);
    CPPUNIT_TEST(testTdf128096);
    CPPUNIT_TEST(testTdf120573);
    CPPUNIT_TEST(testTdf118825);
    CPPUNIT_TEST(testTdf119118);
    CPPUNIT_TEST(testTdf99213);
    CPPUNIT_TEST(testPotxExport);
    CPPUNIT_TEST(testTdf44223);
    CPPUNIT_TEST(testSmartArtPreserve);
    CPPUNIT_TEST(testTdf125346);
    CPPUNIT_TEST(testTdf125346_2);
    CPPUNIT_TEST(testTdf125360);
    CPPUNIT_TEST(testTdf125360_1);
    CPPUNIT_TEST(testTdf125360_2);
    CPPUNIT_TEST(testTdf125551);
    CPPUNIT_TEST(testTdf136830);
    CPPUNIT_TEST(testTdf126234);
    CPPUNIT_TEST(testTdf126741);
    CPPUNIT_TEST(testTdf127372);
    CPPUNIT_TEST(testTdf127379);
    CPPUNIT_TEST(testTdf98603);
    CPPUNIT_TEST(testTdf79082);
    CPPUNIT_TEST(testTdf128213);
    CPPUNIT_TEST(testTdf129372);
    CPPUNIT_TEST(testShapeGlowEffect);
    CPPUNIT_TEST(testTdf119087);
    CPPUNIT_TEST(testTdf131554);
    CPPUNIT_TEST(testTdf132282);
    CPPUNIT_TEST(testTdf132201EffectOrder);
    CPPUNIT_TEST(testShapeSoftEdgeEffect);
    CPPUNIT_TEST(testShapeShadowBlurEffect);
    CPPUNIT_TEST(testTdf119223);
    CPPUNIT_TEST(testTdf128213ShapeRot);
    CPPUNIT_TEST(testTdf125560_textDeflate);
    CPPUNIT_TEST(testTdf125560_textInflateTop);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        static const struct { char const * pPrefix; char const * pURI; } namespaces[] =
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
            { "r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships" },
            { "w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main" },
            { "a14", "http://schemas.microsoft.com/office/drawing/2010/main" },
            { "wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" },
            { "wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" },
            { "dgm", "http://schemas.openxmlformats.org/drawingml/2006/diagram" },
            // ODF
            { "office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" },
            { "draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" },
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(namespaces); ++i)
        {
            xmlXPathRegisterNs(pXmlXPathCtx,
                reinterpret_cast<xmlChar const *>(namespaces[i].pPrefix),
                reinterpret_cast<xmlChar const *>(namespaces[i].pURI));
        }
    }

};

void SdOOXMLExportTest2::testTdf131905()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf131905.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[1]/a:tc/a:tcPr",
        "anchor", "t");

    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[2]/a:tc/a:tcPr",
        "anchor", "ctr");

    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[3]/a:tc/a:tcPr",
        "anchor", "b");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf93883()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf93883.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT(!xPropSet->getPropertyValue("NumberingLevel").hasValue());
}

void SdOOXMLExportTest2::testBnc822341()
{
    // Check import / export of embedded text document
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/bnc822341.odp"), ODP);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile1 );

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocUniquePtr pXmlDocCT = parseExport(tempFile1, "[Content_Types].xml");
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.wordprocessingml.document']",
                    "PartName",
                    "/ppt/embeddings/oleObject1.docx");

        xmlDocUniquePtr pXmlDocRels = parseExport(tempFile1, "ppt/slides/_rels/slide1.xml.rels");
        assertXPath(pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']",
            "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");

        xmlDocUniquePtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj",
            "progId",
            "Word.Document.12");

        const SdrPage *pPage = GetPage( 1, xDocShRef.get() );

        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL( OBJ_OLE2, pObj->GetObjIdentifier() );
    }

    utl::TempFile tempFile2;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile2 );

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocUniquePtr pXmlDocCT = parseExport(tempFile2, "[Content_Types].xml");
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.wordprocessingml.document']",
                    "PartName",
                    "/ppt/embeddings/oleObject1.docx");

        xmlDocUniquePtr pXmlDocRels = parseExport(tempFile2, "ppt/slides/_rels/slide1.xml.rels");
        assertXPath(pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']",
            "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");

        xmlDocUniquePtr pXmlDocContent = parseExport(tempFile2, "ppt/slides/slide1.xml");
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
        CPPUNIT_ASSERT_EQUAL( OBJ_OLE2, pObj->GetObjIdentifier() );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testMathObject()
{
    // Check import / export of math object
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/math.odp"), ODP);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile1);

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocUniquePtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
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
        CPPUNIT_ASSERT_EQUAL(OBJ_OLE2, pObj->GetObjIdentifier());
    }

    utl::TempFile tempFile2;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile2 );

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocUniquePtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
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
        CPPUNIT_ASSERT_EQUAL(OBJ_OLE2, pObj->GetObjIdentifier());
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testMathObjectPPT2010()
{
    // Check import / export of math object
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/Math.pptx"), PPTX);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile1);

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocUniquePtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
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
        CPPUNIT_ASSERT_EQUAL(OBJ_OLE2, pObj->GetObjIdentifier());
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf119015()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf119015.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage* pPage = GetPage(1, xDocShRef);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    // The position was previously not properly initialized: (0, 0, 100, 100)
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(6991, 6902), Size(14099, 1999)),
                         pTableObj->GetLogicRect());
    uno::Reference<table::XTable> xTable(pTableObj->getTable());

    // Test that we actually have three cells: this threw css.lang.IndexOutOfBoundsException
    uno::Reference<text::XTextRange> xTextRange(xTable->getCellByPosition(1, 0),
                                                uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("A3"), xTextRange->getString());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf123090()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf123090.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage* pPage = GetPage(1, xDocShRef);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XTable> xTable(pTableObj->getTable());

    // Test that we actually have two cells: this threw css.lang.IndexOutOfBoundsException
    uno::Reference<text::XTextRange> xTextRange(xTable->getCellByPosition(1, 0),
                                                uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), xTextRange->getString());

    sal_Int32 nWidth;
    uno::Reference< css::table::XTableColumns > xColumns( xTable->getColumns(), uno::UNO_SET_THROW);
    uno::Reference< beans::XPropertySet > xRefColumn( xColumns->getByIndex(1), uno::UNO_QUERY_THROW );
    xRefColumn->getPropertyValue( "Width" ) >>= nWidth;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(9136), nWidth);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf126324()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf126324.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference< text::XText > xText = uno::Reference< text::XTextRange>( xShape, uno::UNO_QUERY_THROW )->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("17"), xText->getString());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf119187()
{
    std::vector< sd::DrawDocShellRef > xDocShRef;
    // load document
    xDocShRef.push_back(loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf119187.pptx"), PPTX));
    // load resaved document
    xDocShRef.push_back(saveAndReload( xDocShRef.at(0).get(), PPTX ));

    // check documents
    for (const sd::DrawDocShellRef& xDoc : xDocShRef)
    {
        // get shape properties
        const SdrPage* pPage = GetPage(1, xDoc);
        CPPUNIT_ASSERT(pPage);
        SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);
        const sdr::properties::BaseProperties & rProperties = pObj->GetProperties();

        // check text vertical alignment
        const SdrTextVertAdjustItem& rSdrTextVertAdjustItem = rProperties.GetItem(SDRATTR_TEXT_VERTADJUST);
        const SdrTextVertAdjust eTVA = rSdrTextVertAdjustItem.GetValue();
        CPPUNIT_ASSERT_EQUAL(SDRTEXTVERTADJUST_TOP, eTVA);
    }
}

void SdOOXMLExportTest2::testTdf132472()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf132472.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nColor);

    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xCell));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY);
    xPropSet->getPropertyValue("CharColor") >>= nColor;

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:0 G:0 B:0 A:0
    // - Actual  : Color: R:255 G:255 B:255 A:0
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf80224()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf80224.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph->getStart(), uno::UNO_QUERY_THROW );

    sal_Int32 nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x6562ac), nCharColor);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf91378()
{

    //Check For Import and Export Both
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf91378.pptx"), PPTX);
    for( sal_uInt32 i=0;i<2;i++)
    {
      SdDrawDocument *pDoc = xDocShRef->GetDoc();
      CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
      uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
      uno::Reference<document::XDocumentProperties> xProps = xDocumentPropertiesSupplier->getDocumentProperties();
      uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
      OUString propValue;
      xUDProps->getPropertyValue("Testing") >>= propValue;
      CPPUNIT_ASSERT(propValue.isEmpty());
      xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    }
    xDocShRef->DoClose();
}

static bool checkTransitionOnPage(uno::Reference<drawing::XDrawPagesSupplier> const & xDoc, sal_Int32 nSlideNumber,
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/AllTransitions.odp"), ODP);
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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/preset-shapes-export.odp"), ODP);
    const char *sShapeTypeAndValues[] =
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

    xmlDocUniquePtr pXmlDocCT = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPattern( "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom[@prst='_T_']/a:avLst/a:gd[_N_]" );
    const OString sT( "_T_" );
    const OString sN( "_N_" );
    const OString sPropertyName("name");
    const OString sPropertyFmla("fmla");

    size_t i = 0;
    while(i < SAL_N_ELEMENTS( sShapeTypeAndValues )) {
        OString sType( sShapeTypeAndValues[ i++ ] );
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/empty.fodp"), FODG );
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
    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get< uno::Sequence<beans::PropertyValue> >();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (beans::PropertyValue const & rProp : std::as_const(aPathProps))
    {
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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/numfmt.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 6; ++i)
    {
        matchNumberFormat( i, getTextFieldFromPage(0, 0, 0, i, xDocShRef) );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testDatetimeFieldNumberFormatPPTX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/numfmt.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 6; ++i)
    {
        matchNumberFormat( i, getTextFieldFromPage(0, 0, 0, i, xDocShRef) );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideNumberField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/slidenum_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideNumberFieldPPTX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/slidenum_field.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideCountField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/slidecount_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSlideNameField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/slidename_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testExtFileField()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/extfile_field.odp"), ODP);

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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/author_field.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< text::XTextField > xField = getTextFieldFromPage(0, 0, 0, 0, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf99224()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf99224.odp"), ODP);
    xShell = saveAndReload(xShell.get(), PPTX);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    // This was 0: the image with text was lost on export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xPage->getCount());
    xShell->DoClose();
}

void SdOOXMLExportTest2::testTdf92076()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf92076.odp"), ODP);
    xShell = saveAndReload(xShell.get(), PPTX);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());
    xShell->DoClose();
}

void SdOOXMLExportTest2::testTdf59046()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf59046.odp"), ODP);
    utl::TempFile tempFile;
    xShell = saveAndReload(xShell.get(), PPTX, &tempFile);
    xShell->DoClose();
    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", 1);
}

void SdOOXMLExportTest2::testTdf133502()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf133502.odp"), ODP);
    utl::TempFile tempFile;
    xShell = saveAndReload(xShell.get(), PPTX, &tempFile);
    xShell->DoClose();
    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/comments/comment1.xml");

    assertXPathContent(pXmlDocRels, "/p:cmLst/p:cm/p:text", "Test for creator-initials");

    // Without the fix in place, the comment position would have been 0,0
    assertXPath(pXmlDocRels, "/p:cmLst/p:cm/p:pos", "x", "2032");
    assertXPath(pXmlDocRels, "/p:cmLst/p:cm/p:pos", "y", "1029");
}

void SdOOXMLExportTest2::testTdf105739()
{
    // Gradient was lost during saving to ODP
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf105739.pptx"), PPTX);
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
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), aFillGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00b050), aFillGradient.EndColor);
    }

    xShell->DoClose();
}

void SdOOXMLExportTest2::testPageBitmapWithTransparency()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/page_transparent_bitmap.pptx"), PPTX );

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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptm/macro.pptm"), PPTM);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTM, &tempFile);

    // Assert that the content type is the one of PPTM
    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
                "ContentType",
                "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf111798()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf111798.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptm/macro.pptm"), PPTM);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTM, &tempFile);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());
    // This failed: VBA stream was not roundtripped
    CPPUNIT_ASSERT(xNameAccess->hasByName("ppt/vbaProject.bin"));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf111863()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111863.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check that transition attribute didn't change from 'out' to 'in'
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animEffect",
        "transition", "out");
}

void SdOOXMLExportTest2::testTdf111518()
{
    sd::DrawDocShellRef xShell = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf111518.pptx"), PPTX);
    utl::TempFile tempFile;
    tempFile.EnableKillingFile(false);
    xShell = saveAndReload(xShell.get(), PPTX, &tempFile);
    xShell->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sActual = getXPath(pXmlDocRels,
            "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animMotion",
            "path");
    CPPUNIT_ASSERT_MOTIONPATH("M -3.54167E-6 -4.81481E-6 L 0.39037 -0.00069 E", sActual);
}

void SdOOXMLExportTest2::testTdf100387()
{

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf100387.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

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

// tdf#126746 Add support for Line Caps import and export
void SdOOXMLExportTest2::testClosingShapesAndLineCaps()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/closed-shapes.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo/a:pt", 1);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", 1);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[2]/a:pt", 1);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:close", 1);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:ln", "cap", "rnd");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:ln/a:miter", 1);

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:close", 0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:ln", "cap", "rnd");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:ln/a:miter", 1);

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:custGeom/a:pathLst/a:path/a:close", 0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:ln", "cap", "rnd");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:ln/a:miter", 1);

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:custGeom/a:pathLst/a:path/a:close", 0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:ln", "cap", "sq");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:ln/a:round", 1);

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:custGeom/a:pathLst/a:path/a:close", 0);
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:ln", "cap"); // by default it is "flat" cap style
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:ln/a:bevel", 1);

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:spPr/a:custGeom/a:pathLst/a:path/a:close", 0);
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:ln", "cap"); // by default it is "flat" cap style
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:spPr/a:ln/a:round", 1);
}

void SdOOXMLExportTest2::testRotateFlip()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/rotate_flip.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

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
        assertXPath(pXmlDocContent, sSpPr + "/a:custGeom/a:pathLst/a:path/a:close", 1);
    }
}

void SdOOXMLExportTest2::testTdf106867()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf106867.pptx"), PPTX);
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
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/p:extLst/p:ext/p14:media");

    // target the shape with the video in the command
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:cmd/p:cBhvr/p:tgtEl/p:spTgt",
            "spid", "42");
}

void SdOOXMLExportTest2::testTdf112280()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112280.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check the animRot value
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animRot",
            "by", "21600000");
}

void SdOOXMLExportTest2::testTdf112088()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112088.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // check gradient stops
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:gradFill/a:gsLst", 2);
}

void SdOOXMLExportTest2::testTdf112333()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112333.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf112552.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", "w", "21600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", "h", "21600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "x", "21600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "y", "0");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf112557()
{
    // Subtitle shape should be skipped by export.
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf112557.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slideMasters/slideMaster1.xml");
    assertXPath(pXmlDocContent, "/p:sldMaster/p:cSld/p:spTree/p:sp", 2); // title and object
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf128049()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf128049.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom", 0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:prstGeom", "prst", "noSmoking");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:prstGeom/a:avLst/a:gd", "name", "adj");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:prstGeom/a:avLst/a:gd", "fmla", "val 12500");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf106026()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf106026.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlMasterContent = parseExport(tempFile, "ppt/slideMasters/slideMaster1.xml");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:pPr/a:spcBef/a:spcPts", "val", "1417");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:pPr/a:spcBef/a:spcPts", "val", "1134");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[3]/a:pPr/a:spcBef/a:spcPts", "val", "850");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[4]/a:pPr/a:spcBef/a:spcPts", "val", "567");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[5]/a:pPr/a:spcBef/a:spcPts", "val", "283");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[6]/a:pPr/a:spcBef/a:spcPts", "val", "283");
    assertXPath(pXmlMasterContent, "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[7]/a:pPr/a:spcBef/a:spcPts", "val", "283");

    xmlDocUniquePtr pXmlSlideContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlSlideContent,
                       "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:spcAft/a:spcPts", "val", "11339");
    assertXPath(pXmlSlideContent,
                       "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:spcAft/a:spcPts", "val", "11339");
    assertXPath(pXmlSlideContent,
                       "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:spcAft/a:spcPts", "val", "11339");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf112334()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112334.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animClr[1]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("style.color"), sAttributeName);
}

void SdOOXMLExportTest2::testTdf112089()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112089.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    OUString sID = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr", "id");
    OUString sTarget = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt", "spid");
    CPPUNIT_ASSERT_EQUAL(sID, sTarget);
}

void SdOOXMLExportTest2::testTdf112086()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112086.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

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
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf112647.odp"), ODP);
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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/group_rotation.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:grpSpPr/a:xfrm", "rot");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:xfrm", "rot", "20400000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[2]/p:spPr/a:xfrm", "rot", "20400000");
}

void SdOOXMLExportTest2::testTdf104788()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104788.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide6.xml");

    OUString sVal = getXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]", "to");
    CPPUNIT_ASSERT_EQUAL(-1.0, sVal.toDouble());

    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:anim[2]/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("xshear"), sAttributeName);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSmartartRotation2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/smartart-rotation2.pptx"), PPTX);

    // clear SmartArt data to check how group shapes with double-rotated children are exported, not smartart
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xShape->setPropertyValue("InteropGrabBag", uno::makeAny(aInteropGrabBag));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathContent(pXmlDocContent,
                       "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:txBody/a:p/a:r/a:t", "Text");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:txBody/a:bodyPr", "rot", "10800000");
    double dX = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:spPr/a:xfrm/a:off", "x").toDouble();
    double dY = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:spPr/a:xfrm/a:off", "y").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 2276280.0, dX, dX * .001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 3158280.0, dY, dY * .001);
}

void SdOOXMLExportTest2::testTdf91999_rotateShape()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf91999_rotateShape.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:nvSpPr/p:cNvPr", "name", "CustomShape 2");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:xfrm", "rot", "10800000");
    double dX = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:xfrm/a:off", "x").toDouble();
    double dY = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:xfrm/a:off", "y").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 2960640.0, dX, dX * .001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1449000.0, dY, dY * .001);
}

void SdOOXMLExportTest2::testTdf114845_rotateShape()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf114845_rotateShape.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:nvSpPr/p:cNvPr", "name", "Straight Arrow Connector 9");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:xfrm", "flipV", "1");
    double dX = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:xfrm/a:off", "x").toDouble();
    double dY = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:xfrm/a:off", "y").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 4059000.0, dX, dX * .001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 3287520.0, dY, dY * .001);
}

void SdOOXMLExportTest2::testGroupsPosition()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/group.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[1]/p:spPr/a:xfrm/a:off", "x", "5004000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[1]/p:spPr/a:xfrm/a:off", "y", "3310560");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[3]/p:spPr/a:xfrm/a:off", "x", "7760160");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[3]/p:spPr/a:xfrm/a:off", "y", "3310560");
}

void SdOOXMLExportTest2::testGroupsRotatedPosition()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/group-rot.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "x", "2857320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off", "y", "4026960");
}

void SdOOXMLExportTest2::testAccentColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/accent-color.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:style/a:fillRef/a:schemeClr", "val", "accent6");
    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp/p:style/a:fillRef/a:schemeClr", "val", "accent6");
    xmlDocUniquePtr pXmlDocTheme1 = parseExport(tempFile, "ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:accent6/a:srgbClr", "val", "70ad47");
    xmlDocUniquePtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:accent6/a:srgbClr", "val", "deb340");
}

void SdOOXMLExportTest2::testThemeColors()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf84205.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val", "44546a");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:accent3/a:srgbClr", "val", "a5a5a5");
}

void SdOOXMLExportTest2::testTdf114848()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf114848.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocTheme1 = parseExport(tempFile, "ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val", "1f497d");
    xmlDocUniquePtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val", "1f497d");
}

void SdOOXMLExportTest2::testTdf68759()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf68759.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
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

void SdOOXMLExportTest2::testTdf127901()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf127901.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "bright", "70000");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "contrast", "-70000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:grayscl", 1);

    xmlDocUniquePtr pXmlDocContent3 = parseExport(tempFile, "ppt/slides/slide3.xml");
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:biLevel", "thresh", "50000");

}

void SdOOXMLExportTest2::testTdf48735()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf48735.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "b", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "l", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "r", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "t", "18842");
}

void SdOOXMLExportTest2::testTdf90626()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf90626.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent =  parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buSzPct", "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buSzPct", "val", "150142");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buSzPct", "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr/a:buSzPct", "val", "150142");
}

void SdOOXMLExportTest2::testTdf107608()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf107608.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_SET_THROW );

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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111786.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_SET_THROW );

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/font-scale.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    // Rounding errors possible, approximate value
    OUString sScale = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:normAutofit", "fontScale");
    if (sScale != "73000" && sScale != "72000" && sScale != "74000")
        CPPUNIT_ASSERT_EQUAL(OUString("73000"), sScale);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testShapeAutofitPPTX()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/testShapeAutofit.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // TextAutoGrowHeight --> "Resize shape to fit text" --> true
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:spAutoFit", 1);
    // TextAutoGrowHeight --> "Resize shape to fit text" --> false
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

void SdOOXMLExportTest2::testLegacyShapeAutofitPPTX()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/testLegacyShapeAutofit.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // Text in a legacy rectangle
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:noAutofit", 1);
    // Text in (closed) Polygon
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
    // Text in a legacy ellipse
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

void SdOOXMLExportTest2::testTdf115394()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf115394.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf115394-zero.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRefOriginal = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf115005.odp"), ODP);
    utl::TempFile tempFile;
    sd::DrawDocShellRef xDocShRefResaved = saveAndReload(xDocShRefOriginal.get(), ODP, &tempFile);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());

    // check that the document contains original vector images
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSVMFiles = 0;
    for (OUString const & s : names)
    {
        if(s.endsWith(".svm"))
            nSVMFiles++;
    }
    CPPUNIT_ASSERT_EQUAL(3, nSVMFiles);
}

int SdOOXMLExportTest2::testTdf115005_FallBack_Images(bool bAddReplacementImages)
{
    sd::DrawDocShellRef xDocShRefOriginal = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf115005_no_fallback_images.odp"), ODP);

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
    for (OUString const & n :names)
    {
        if(n.endsWith(".svm"))
            nSVMFiles++;
        if(n.endsWith(".png"))
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
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118806.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animMotion", "origin", "layout");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf130058()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf130058.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    double fShadowDist = 0.0;
    xShape->getPropertyValue("ShadowXDistance") >>= fShadowDist;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(0), fShadowDist);
    xShape->getPropertyValue("ShadowYDistance") >>= fShadowDist;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(141), fShadowDist);
    sal_Int32 nColor = 0;
    xShape->getPropertyValue("ShadowColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x000000), nColor);
    sal_Int32 nTransparency = 0;
    xShape->getPropertyValue("ShadowTransparence") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(57), nTransparency);
    double fShadowSizeX = 0.0;
    xShape->getPropertyValue("ShadowSizeX") >>= fShadowSizeX;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000), fShadowSizeX);
    double fShadowSizeY = 0.0;
    xShape->getPropertyValue("ShadowSizeY") >>= fShadowSizeY;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000), fShadowSizeY);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf111789()
{
    // Shadow properties were not exported for text shapes.
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111789.pptx"), PPTX);
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
        double fShadowSizeX = 0.0;
        xShape->getPropertyValue("ShadowSizeX") >>= fShadowSizeX;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(100000), fShadowSizeX);
        double fShadowSizeY = 0.0;
        xShape->getPropertyValue("ShadowSizeY") >>= fShadowSizeY;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(100000), fShadowSizeY);
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
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104792-smart-art-animation.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/p:par[1]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:set/p:cBhvr/p:tgtEl/p:spTgt", 1);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf90627()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf90627.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    // Don't export empty endCondLst without cond.
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:endCondLst[not(*)]", 0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf104786()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104786.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide2.xml");
    // Don't export empty 'to'
    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst/p:set[2]/p:to", 0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf118783()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118783.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sAttributeName = getXPathContent(pXmlDocContent, "//p:animRot/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("r"), sAttributeName);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf104789()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104789.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sAttributeName = getXPathContent(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("style.opacity"), sAttributeName);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testOpenDocumentAsReadOnly()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/open-as-read-only.pptx"), PPTX);
    CPPUNIT_ASSERT(xDocShRef->IsSecurityOptOpenReadOnly());
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    CPPUNIT_ASSERT(xDocShRef->IsSecurityOptOpenReadOnly());
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf118835()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118835.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118768-brake.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118836.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "x", "250000");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "y", "250000");
    xDocShRef->DoClose();
}

static double getAdjustmentValue( const uno::Reference<beans::XPropertySet>& xSet )
{
    auto aGeomPropSeq = xSet->getPropertyValue( "CustomShapeGeometry" )
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(
            aGeomPropSeq );

    auto aIterator = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        []( const beans::PropertyValue& rValue ) { return rValue.Name == "AdjustmentValues"; } );

    if (aIterator != aGeomPropVec.end())
    {
        uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustment;
        double fResult = 0.0;
        aIterator->Value >>= aAdjustment;
        aAdjustment[0].Value >>= fResult;
        return fResult;
    }

    return -1.0;
}

static bool getScaleXValue(const uno::Reference<beans::XPropertySet>& xSet)
{
    bool bScaleX = false;

    auto aGeomPropSeq = xSet->getPropertyValue("CustomShapeGeometry")
        .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(
            aGeomPropSeq);

    auto aIterator = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [](const beans::PropertyValue& rValue) { return rValue.Name == "TextPath"; });

    if (aIterator != aGeomPropVec.end())
    {
        uno::Sequence<beans::PropertyValue> aTextPathProperties;
        aIterator->Value >>= aTextPathProperties;
        auto aIterator2 = std::find_if(
            aTextPathProperties.begin(), aTextPathProperties.end(),
            [](const beans::PropertyValue& rValue) { return rValue.Name == "ScaleX"; });

        if (aIterator2 != aTextPathProperties.end())
        {
            aIterator2->Value >>= bScaleX;
        }
    }

    return bScaleX;
}

void SdOOXMLExportTest2::testTdf116350TextEffects()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc( u"sd/qa/unit/data/pptx/tdf116350-texteffects.pptx" ), PPTX );

    // Default angle for ArchUp
    uno::Reference<beans::XPropertySet> xShape0( getShapeFromPage( 0, 0, xDocShRef ) );
    double fAdjust = getAdjustmentValue( xShape0 );
    CPPUNIT_ASSERT_EQUAL( 180.0, fAdjust );

    bool bScaleX = getScaleXValue( xShape0 );
    CPPUNIT_ASSERT_EQUAL( true, bScaleX );

    // Default angle for ArchDown
    uno::Reference<beans::XPropertySet> xShape14( getShapeFromPage( 14, 0, xDocShRef ) );
    fAdjust = getAdjustmentValue( xShape14 );
    CPPUNIT_ASSERT_EQUAL( 0.0, fAdjust );

    bScaleX = getScaleXValue( xShape14 );
    CPPUNIT_ASSERT_EQUAL( true, bScaleX );

    // Angle directly set
    uno::Reference<beans::XPropertySet> xShape1( getShapeFromPage( 1, 0, xDocShRef ) );
    fAdjust = getAdjustmentValue( xShape1 );
    CPPUNIT_ASSERT_EQUAL( 213.25, fAdjust );

    bScaleX = getScaleXValue( xShape1 );
    CPPUNIT_ASSERT_EQUAL( true, bScaleX );

    // Export
    utl::TempFile tempFile;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile );

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", "textArchUp");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", "textCircle");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:spPr/a:solidFill/a:srgbClr", 0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf128096()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128096.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val", "ffff00");

    // Check that underlined content is also highlighted
    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val", "ffff00");
}
void SdOOXMLExportTest2::testTdf120573()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc( u"sd/qa/unit/data/pptx/tdf120573.pptx" ), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile );

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:audioFile", 1);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile", 0);

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../media/media1.wav'])[1]",
        "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");

    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.wav']",
                "ContentType",
                "audio/x-wav");
}

void SdOOXMLExportTest2::testTdf118825()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118825-motionpath.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");


    CPPUNIT_ASSERT_MOTIONPATH("M 0.0449285714285714 0.00368253968253968 C 0.0575714285714285 -0.00095238095238096 0.0704264795523803 -0.00370117418637049 0.0831071428571428 -0.00819047619047622 C 0.0953550597998766 -0.0125265741339082 0.107821870086751 -0.010397536991717 0.120321428571429 -0.0115555555555556 C 0.133179018681433 -0.0127467438724762 0.151318627483861 -0.0158700272533852 0.1585 0.00539682539682542 C 0.16478291361998 0.0240029898688431 0.15828642886492 0.0483806254341085 0.161392857142857 0.0698412698412698 C 0.165179286017685 0.0959996731216037 0.17453898927982 0.119735912694626 0.187142857142857 0.132634920634921 C 0.199788991845377 0.145577185161529 0.215607110490848 0.142889773028431 0.230107142857143 0.142857142857143 C 0.243821417584191 0.142826280916829 0.257716514999779 0.142685979556724 0.271142857142857 0.137777777777778 C 0.286895094567923 0.132019309914514 0.302318190711873 0.122962218306185 0.317928571428571 0.11568253968254 C 0.333496771884547 0.108422531222479 0.348787823719556 0.0990570571890929 0.363714285714286 0.0885079365079364 C 0.374930683062651 0.080580865157908 0.385357142857143 0.0693333333333332 0.396178571428571 0.0596825396825396 L 0.404785714285714 0.0410158730158729 L 0.401892857142857 0.0342222222222221 E", getXPath(pXmlDocContent, "(//p:animMotion)[1]", "path"));
    CPPUNIT_ASSERT_MOTIONPATH("M 0.025 0.0571428571428571 L 0.0821428571428571 0.184126984126984 L -0.175 0.234920634920635 L -0.246428571428571 -0.0190476190476191 L -0.0821428571428573 -0.133333333333333 E", getXPath(pXmlDocContent, "(//p:animMotion)[2]", "path"));
    CPPUNIT_ASSERT_MOTIONPATH("M -0.0107142857142857 0.00634920634920635 C -0.110714285714286 0.501587301587301 -0.153571428571429 -0.00634920634920635 -0.246428571428572 0.184126984126984 C -0.339285714285715 0.374603174603175 -0.296428571428572 0.514285714285714 -0.267857142857143 0.603174603174603 C -0.239285714285715 0.692063492063492 0.0607142857142858 0.590476190476191 0.0607142857142858 0.590476190476191 E", getXPath(pXmlDocContent, "(//p:animMotion)[3]", "path"));
    CPPUNIT_ASSERT_MOTIONPATH("M 0.0535714285714286 -0.0444444444444444 L 0.132142857142857 -0.0444444444444444 L 0.132142857142857 -0.146031746031746 L 0.0964285714285715 -0.146031746031746 E", getXPath(pXmlDocContent, "(//p:animMotion)[4]", "path"));
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf119118()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc( u"sd/qa/unit/data/pptx/tdf119118.pptx" ), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:iterate", "type", "lt");
    assertXPath(pXmlDocContent, "//p:tmAbs", "val", "200");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf99213()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc( u"sd/qa/unit/data/odp/tdf99213-target-missing.odp" ), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    // Number of nodes with p:attrNameLst was 3, including one that missed tgtEl
    assertXPath(pXmlDocContent, "//p:attrNameLst", 2);
    // Timenode that miss its target element should be filtered.
    assertXPath(pXmlDocContent, "//p:attrNameLst/preceding-sibling::p:tgtEl", 2);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testPotxExport()
{
    // Create new document
    sd::DrawDocShellRef xDocShRef
        = new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Draw);
    uno::Reference<frame::XLoadable> xLoadable(xDocShRef->GetModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xLoadable.is());
    xLoadable->initNew();

    // Export as a POTM template
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), POTX, &tempFile);
    xDocShRef->DoClose();

    // Load and check content type
    xmlDocUniquePtr pContentTypes = parseExport(tempFile, "[Content_Types].xml");
    CPPUNIT_ASSERT(pContentTypes);
    assertXPath(pContentTypes, "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
        "ContentType", "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml");
}

void SdOOXMLExportTest2::testTdf44223()
{
    utl::TempFile tempFile;
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf44223.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    std::unique_ptr<SvStream> const pStream1(parseExportStream(tempFile, "ppt/media/audio1.wav"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(11140), pStream1->remainingSize());

    std::unique_ptr<SvStream> const pStream2(parseExportStream(tempFile, "ppt/media/audio2.wav"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(28074), pStream2->remainingSize());

    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio1.wav']",
                "ContentType",
                "audio/x-wav");

    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio2.wav']",
                "ContentType",
                "audio/x-wav");

    xmlDocUniquePtr pDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");

    // Start condition: 0s after timenode id 5 begins.
    assertXPath(pDoc1 , "//p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "evt", "begin");
    assertXPath(pDoc1 , "//p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "delay", "0");
    assertXPath(pDoc1 , "//p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond/p:tn", "val", "5");

    xmlDocUniquePtr pDoc2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pDoc2 , "//p:transition/p:sndAc/p:stSnd/p:snd[@r:embed]", 2);

    xmlDocUniquePtr pRels1 = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Target", "../media/audio1.wav");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testSmartArtPreserve()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(
            m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/smartart-preserve.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/dgm:relIds");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/p:extLst/p:ext",
                "uri", "{D42A27DB-BD31-4B8C-83A1-F6EECF244321}");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/p:extLst/p:ext/p14:modId");

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../diagrams/layout1.xml'])[1]", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramLayout");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../diagrams/data1.xml'])[1]", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../diagrams/colors1.xml'])[1]", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramColors");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../diagrams/quickStyle1.xml'])[1]", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramQuickStyle");

    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/layout1.xml']",
                "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/data1.xml']",
                "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/colors1.xml']",
                "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/quickStyle1.xml']",
                "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf125346()
{
    // There are two themes in the test document, make sure we use the right theme
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125346.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_SET_THROW );

    drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
    xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

    sal_Int32 nFillColor;
    xPropSet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x90C226), nFillColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf125346_2()
{
    // There are two themes in the test document, make sure we use the right theme
    // Test more slides with different themes
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125346_2.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    {
        uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_SET_THROW );

        drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        sal_Int32 nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x90C226), nFillColor);
    }

    {
        uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 1, xDocShRef ) );
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_SET_THROW );

        drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        sal_Int32 nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x052F61), nFillColor);
    }

    {
        uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 2, xDocShRef ) );
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_SET_THROW );

        drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        sal_Int32 nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x90C226), nFillColor);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf125360()
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by shape style
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125360.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    xShape->setPropertyValue("FillTransparence", uno::makeAny(static_cast<sal_Int32>(23)));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xShape.set( getShapeFromPage( 0, 0, xDocShRef ) );

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf125360_1()
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125360_1.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    xShape->setPropertyValue("FillTransparence", uno::makeAny(static_cast<sal_Int32>(23)));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xShape.set( getShapeFromPage( 0, 0, xDocShRef ) );

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf125360_2()
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme with a transparency
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125360_2.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(82), nTransparence);

    xShape->setPropertyValue("FillTransparence", uno::makeAny(static_cast<sal_Int32>(23)));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xShape.set( getShapeFromPage( 0, 0, xDocShRef ) );

    nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf125551()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125551.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeBg(xGroupShape->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1024), xShapeBg->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(576), xShapeBg->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10815), xShapeBg->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8587), xShapeBg->getSize().Height);
}

void SdOOXMLExportTest2::testTdf136830()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf136830.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // Without the fix in place, the X position of the shapes would have been 0
    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape1(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14134), xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-5321), xShape1->getPosition().Y);

    uno::Reference<drawing::XShape> xShape2(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8085), xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8085), xShape2->getPosition().Y);

    uno::Reference<drawing::XShape> xShape3(xGroupShape->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8283), xShape3->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4620), xShape3->getPosition().Y);
}

void SdOOXMLExportTest2::testTdf100348_convert_Fontwork2TextWarp()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf100348_Fontwork2TextWarp.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // Resulting pptx has to contain the TextWarp shape
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp");
    assertXPath(pXmlDocContent, sPathStart + "[@prst='textWave1']");
    const OString sPathAdj(sPathStart + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj1' and  @fmla='val 18750']");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj2' and  @fmla='val -7500']");

    // Reloading has to get the Fontwork shape back
    // TextPath makes a custom shape to a Fontwork shape, so must exist
    uno::Reference<beans::XPropertySet> xShapeWavePropSet(getShapeFromPage(0, 0, xDocShRef));
    auto aGeomPropSeq = xShapeWavePropSet->getPropertyValue("CustomShapeGeometry")
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);
    OUString sName = "TextPath";
    auto aIterator = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [sName](const beans::PropertyValue& rValue) { return rValue.Name == sName; });
    CPPUNIT_ASSERT_MESSAGE("No TextPath", aIterator != aGeomPropVec.end());

    // Type has to be same as in original document on roundtrip.
    sName = "Type";
    auto aIterator2 = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [sName](const beans::PropertyValue& rValue) { return rValue.Name == sName; });
    CPPUNIT_ASSERT_MESSAGE("No Type", aIterator2 != aGeomPropVec.end());
    OUString sOwnName;
    aIterator2->Value >>= sOwnName;
    CPPUNIT_ASSERT_EQUAL(OUString("fontwork-wave"), sOwnName);

    // Adjustmentvalues need to be the same.
    sName = "AdjustmentValues";
    auto aIterator3 = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [sName](const beans::PropertyValue& rValue) { return rValue.Name == sName; });
    CPPUNIT_ASSERT_MESSAGE("No AdjustmentValues", aIterator3 != aGeomPropVec.end());
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjValueSeq;
    aIterator3->Value >>= aAdjValueSeq;
    double fAdj1;
    aAdjValueSeq[0].Value >>= fAdj1;
    double fAdj2;
    aAdjValueSeq[1].Value >>= fAdj2;
    CPPUNIT_ASSERT_EQUAL(4050.0, fAdj1); // odp values, not pptx values
    CPPUNIT_ASSERT_EQUAL(9180.0, fAdj2);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf1225573_FontWorkScaleX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf125573_FontWorkScaleX.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // Error was, that attribute 'fromWordArt' was ignored
    // ensure, resulting pptx has fromWordArt="1" on textArchDown shape
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(
        pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr[@fromWordArt='1']");

    // Error was, that text in legacy shapes of category "Follow Path" was not scaled to the path.
    uno::Reference<beans::XPropertySet> xShapeArchProps(getShapeFromPage(0, 0, xDocShRef));
    awt::Rectangle aBoundRectArch;
    xShapeArchProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectArch;
    // difference should be zero, but allow some range for stroke thickness
    CPPUNIT_ASSERT_LESS(sal_Int32(50), std::abs(aBoundRectArch.Width - 13081));

    // Error was, that text in shapes of category "Warp" was not scaled to the path.
    uno::Reference<beans::XPropertySet> xShapeWaveProps(getShapeFromPage(0, 1, xDocShRef));
    awt::Rectangle aBoundRectWave;
    xShapeWaveProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectWave;
    // difference should be zero, but allow some range for stroke thickness
    CPPUNIT_ASSERT_LESS(sal_Int32(50), std::abs(aBoundRectWave.Width - 11514));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf126234()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf126234.pptx"), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // check relative size of the bullet, 400% is a legitimate value for MS Office document
    // Without a fix, it will fail to set the size correctly
    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(400), pNumFmt->GetNumRule()->GetLevel(0).GetBulletRelSize());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf126741()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf126741.pptx"), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // dash dot dot line style import fix
    // The original fixed values are replaced with the percent values, because
    // with fix for tdf#127166 the MS Office preset styles are correctly detected.
    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject *const pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);

    const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
        pObj->GetMergedItem(XATTR_LINESTYLE));
    const XLineDashItem& rDashItem = dynamic_cast<const XLineDashItem&>(
        pObj->GetMergedItem(XATTR_LINEDASH));

    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rDashItem.GetDashValue().GetDots());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(800), rDashItem.GetDashValue().GetDotLen());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rDashItem.GetDashValue().GetDashes());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(100), rDashItem.GetDashValue().GetDashLen());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(300), rDashItem.GetDashValue().GetDistance());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf99497_keepAppearanceOfCircleKind()
{
    // Error was, that all CircleKind were exported to 'ellipse'.
    // Resulting pptx has to contain the customshapes of the corresponding kind
    // slide 1 ARC -> arc, slide 2 CUT -> chord, slide 3 SECTION -> pie
    // Adjustment values need to exist and their values need to correspond to the
    // original angles. Shape 'arc' needs to be unfilled.
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf99497_CircleKind.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // slide 1 45° -> adj1 = 20493903, 270° -> adj2 = 5400000, <a:noFill/> exists
    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart1("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent1, sPathStart1 + "[@prst='arc']");
    const OString sPathAdj1(sPathStart1 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj1' and  @fmla='val 20493903']");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj2' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill");

    // slide 2 270° -> adj1 = 5400000, 180° -> adj2 = 10800000
    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    const OString sPathStart2("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent2, sPathStart2 + "[@prst='chord']");
    const OString sPathAdj2(sPathStart2 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj1' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj2' and  @fmla='val 10800000']");

    // slide 3 120° -> adj1 = 12600000, 30° -> adj2 = 20946396
    xmlDocUniquePtr pXmlDocContent3 = parseExport(tempFile, "ppt/slides/slide3.xml");
    const OString sPathStart3("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent3, sPathStart3 + "[@prst='pie']");
    const OString sPathAdj3(sPathStart3 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj1' and  @fmla='val 12600000']");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj2' and  @fmla='val 20946396']");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf127372()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf127372.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    awt::Gradient aTransparenceGradient;
    xShape->getPropertyValue("FillTransparenceGradient") >>= aTransparenceGradient;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aTransparenceGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aTransparenceGradient.EndColor);
}

void SdOOXMLExportTest2::testTdf127379()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf127379.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
    xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDoc->getDrawPages()->getCount() );

    uno::Reference< drawing::XDrawPage > xPage( getPage( 0, xDocShRef ) );
    uno::Reference< beans::XPropertySet > xPropSet( xPage, uno::UNO_QUERY );

    uno::Any aAny = xPropSet->getPropertyValue( "Background" );
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());
    uno::Reference< beans::XPropertySet > aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    aXBackgroundPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, aFillStyle);

    awt::Gradient aGradient;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue("FillGradient") >>= aGradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x2A6099), aGradient.EndColor);
}

void SdOOXMLExportTest2::testTdf98603()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf98603.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference< beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    css::lang::Locale aLocale;
    xPropSet->getPropertyValue("CharLocaleComplex") >>= aLocale;
    CPPUNIT_ASSERT_EQUAL(OUString("he"), aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("IL"), aLocale.Country);
}

void SdOOXMLExportTest2::testTdf128213()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf128213.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile );

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm", "rot");
}

void SdOOXMLExportTest2::testTdf79082()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf79082.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX, &tempFile );

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]",
        "pos",
        "360000");
    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]",
        "algn",
        "l");

    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]",
        "pos",
        "756000");
    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]",
        "algn",
        "l");

    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]",
        "pos",
        "1440000");
    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]",
        "algn",
        "ctr");

    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]",
        "pos",
        "1800000");
    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]",
        "algn",
        "r");

    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]",
        "pos",
        "3240000");
    assertXPath(pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]",
        "algn",
        "dec");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf129372()
{
    //Without the fix in place, it would crash at import time
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf129372.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef.get() );

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OBJ_OLE2, pObj->GetObjIdentifier() );
}

void SdOOXMLExportTest2::testShapeGlowEffect()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-glow-effect.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("GlowEffectRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 139700 EMU = 388.0556 mm/100
    Color nColor;
    xShape->getPropertyValue("GlowEffectColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue("GlowEffectTransparency") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nTransparency);
}

void SdOOXMLExportTest2::testTdf119087()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf119087.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    // This would fail both on export validation, and reloading the saved pptx file.
}

void SdOOXMLExportTest2::testTdf131554()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf131554.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5622), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(13251), xShape->getPosition().Y);
}

void SdOOXMLExportTest2::testTdf132282()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf132282.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    // Without the fix in place, the position would be 0,0, height = 1 and width = 1
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1736), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(763), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(30523), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2604), xShape->getSize().Height);
}

void SdOOXMLExportTest2::testTdf132201EffectOrder()
{
    auto xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/effectOrder.pptx"),
                             PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst", 2);
    // The relative order of effects is important: glow must be before shadow
    CPPUNIT_ASSERT_EQUAL(0, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "glow"));
    CPPUNIT_ASSERT_EQUAL(1, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "outerShdw"));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testShapeSoftEdgeEffect()
{
    auto xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-soft-edges.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    auto xShapeProps(getShapeFromPage(0, 0, xDocShRef));
    sal_Int32 nRadius = -1;
    xShapeProps->getPropertyValue("SoftEdgeRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRadius); // 18 pt
}

void SdOOXMLExportTest2::testShapeShadowBlurEffect()
{
    auto xDocShRef
            = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-blur-effect.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("ShadowBlur") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 11 pt
}

void SdOOXMLExportTest2::testTdf119223()
{
    auto xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf119223.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomeCustomShape']");

    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomePicture']");

    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomeFormula']");

    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomeLine']");

    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomeTextbox']");

    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomeTable']");

    assertXPath(pXmlDocRels,
                "//p:cNvPr[@name='SomeGroup']");
}

void SdOOXMLExportTest2::testTdf128213ShapeRot()
{
    auto xDocShRef
            = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf128213-shaperot.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot", "rev", "5400000");
}

void SdOOXMLExportTest2::testTdf125560_textDeflate()
{
    auto xDocShRef
        = loadURL( m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/ShapePlusImage.pptx"), PPTX );
    utl::TempFile tempFile;

    // This problem did not affect the pptx export, only the ODP so assert that
    xDocShRef = saveAndReload(xDocShRef.get(), ODP, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "content.xml");
    assertXPath(pXmlDocRels, "/office:document-content/office:body/office:presentation/draw:page/draw:custom-shape/draw:enhanced-geometry", "type", "mso-spt161");
}

void SdOOXMLExportTest2::testTdf125560_textInflateTop()
{
    auto xDocShRef
        = loadURL( m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/ShapeTextInflateTop.pptx"), PPTX );
    utl::TempFile tempFile;

    // This problem did not affect the pptx export, only the ODP so assert that
    xDocShRef = saveAndReload(xDocShRef.get(), ODP, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "content.xml");
    assertXPath(pXmlDocRels, "/office:document-content/office:body/office:presentation/draw:page/draw:custom-shape/draw:enhanced-geometry", "type", "mso-spt164");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
