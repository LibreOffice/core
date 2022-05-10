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
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/postitem.hxx>

#include <oox/drawingml/drawingmltypes.hxx>

#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdmetitm.hxx>
#include <unotools/mediadescriptor.hxx>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <svx/svdotable.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <rtl/uri.hxx>
#include <vcl/filter/PngImageReader.hxx>

using namespace css;

class SdOOXMLExportTest1 : public SdModelTestBaseXML
{
public:
    void testTdf147919();
    void testTdf130165();
    void testTdf124781();
    void testTdf144914();
    void testTdf124232();
    void testTdf143624();
    void testTdf142648();
    void testTdf47365();
    void testTdf125071();
    void testTdf54037();
    void testFdo90607();
    void testTdf127237();
    void testBnc887230();
    void testBnc870233_1();
    void testBnc870233_2();
    void testN828390_4();
    void testN828390_5();
    void testFdo71961();
    void testLostPlaceholders();
    void testN828390();
    void testBnc880763();
    void testBnc862510_5();
    void testBnc822347_EmptyBullet();
    void testFdo83751();
    void testFdo79731();
    void testTableCellFillProperties();
    void testBulletStartNumber();
    void testLineStyle();
    void testCellLeftAndRightMargin();
    void testRightToLeftParaghraph();
    void testTextboxWithHyperlink();
    void testMergedCells();
    void testTableCellBorder();
    void testBulletColor();
    void testBulletCharAndFont();
    void testBulletMarginAndIndentation();
    void testParaMarginAndindentation();
    void testTdf111884();
    void testTdf112633();
    void testTdf128952();
    void testTdf127090();
    void testCustomXml();
    void testTdf94238();
    void testPictureTransparency();
    void testTdf125554();
    void testRoundtripOwnLineStyles();
    void testRoundtripPrstDash();
    void testDashOnHairline();
    void testNarrationNonMediaShape();
    void testCustomshapeBitmapfillSrcrect();
    void testTdf100348FontworkBitmapFill();
    void testTdf100348FontworkGradientGlow();
    void testTdf128345FullTransparentGradient();
    void testTdf128345GradientLinear();
    void testTdf128345GradientRadial();
    void testTdf128345GradientAxial();
    void testTdf134969TransparencyOnColorGradient();
    void testTdf136911();
    void testArcTo();
    void testNarrationMimeType();
    void testTdf140865Wordart3D();
    void testTdf124457();
    void testPlaceholderFillAndOutlineExport();
    void testTdf143126();
    void testTdf143129();
    void testTdf118045();
    void testTdf137675();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest1);

    CPPUNIT_TEST(testTdf147919);
    CPPUNIT_TEST(testTdf130165);
    CPPUNIT_TEST(testTdf124781);
    CPPUNIT_TEST(testTdf144914);
    CPPUNIT_TEST(testTdf124232);
    CPPUNIT_TEST(testTdf143624);
    CPPUNIT_TEST(testTdf142648);
    CPPUNIT_TEST(testTdf47365);
    CPPUNIT_TEST(testTdf125071);
    CPPUNIT_TEST(testTdf54037);
    CPPUNIT_TEST(testFdo90607);
    CPPUNIT_TEST(testTdf127237);
    CPPUNIT_TEST(testBnc887230);
    CPPUNIT_TEST(testBnc870233_1);
    CPPUNIT_TEST(testBnc870233_2);
    CPPUNIT_TEST(testN828390_4);
    CPPUNIT_TEST(testN828390_5);
    CPPUNIT_TEST(testFdo71961);
    CPPUNIT_TEST(testLostPlaceholders);
    CPPUNIT_TEST(testN828390);
    CPPUNIT_TEST(testBnc880763);
    CPPUNIT_TEST(testBnc862510_5);
    CPPUNIT_TEST(testBnc822347_EmptyBullet);
    CPPUNIT_TEST(testFdo83751);
    CPPUNIT_TEST(testFdo79731);
    CPPUNIT_TEST(testTableCellFillProperties);
    CPPUNIT_TEST(testBulletStartNumber);
    CPPUNIT_TEST(testLineStyle);
    CPPUNIT_TEST(testCellLeftAndRightMargin);
    CPPUNIT_TEST(testRightToLeftParaghraph);
    CPPUNIT_TEST(testTextboxWithHyperlink);
    CPPUNIT_TEST(testMergedCells);
    CPPUNIT_TEST(testTableCellBorder);
    CPPUNIT_TEST(testBulletColor);
    CPPUNIT_TEST(testBulletCharAndFont);
    CPPUNIT_TEST(testBulletMarginAndIndentation);
    CPPUNIT_TEST(testParaMarginAndindentation);
    CPPUNIT_TEST(testTdf111884);
    CPPUNIT_TEST(testTdf112633);
    CPPUNIT_TEST(testTdf128952);
    CPPUNIT_TEST(testTdf127090);
    CPPUNIT_TEST(testCustomXml);
    CPPUNIT_TEST(testTdf94238);
    CPPUNIT_TEST(testTdf125554);
    CPPUNIT_TEST(testPictureTransparency);
    CPPUNIT_TEST(testRoundtripOwnLineStyles);
    CPPUNIT_TEST(testRoundtripPrstDash);
    CPPUNIT_TEST(testDashOnHairline);
    CPPUNIT_TEST(testNarrationNonMediaShape);
    CPPUNIT_TEST(testCustomshapeBitmapfillSrcrect);
    CPPUNIT_TEST(testTdf100348FontworkBitmapFill);
    CPPUNIT_TEST(testTdf100348FontworkGradientGlow);
    CPPUNIT_TEST(testTdf128345FullTransparentGradient);
    CPPUNIT_TEST(testTdf128345GradientLinear);
    CPPUNIT_TEST(testTdf128345GradientRadial);
    CPPUNIT_TEST(testTdf128345GradientAxial);
    CPPUNIT_TEST(testTdf134969TransparencyOnColorGradient);
    CPPUNIT_TEST(testTdf136911);
    CPPUNIT_TEST(testArcTo);
    CPPUNIT_TEST(testNarrationMimeType);
    CPPUNIT_TEST(testTdf140865Wordart3D);
    CPPUNIT_TEST(testTdf124457);
    CPPUNIT_TEST(testPlaceholderFillAndOutlineExport);
    CPPUNIT_TEST(testTdf143126);
    CPPUNIT_TEST(testTdf143129);
    CPPUNIT_TEST(testTdf118045);
    CPPUNIT_TEST(testTdf137675);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    }
};

namespace {

template< typename ItemValue, typename ItemType >
void checkFontAttributes( const SdrTextObj* pObj, ItemValue nVal, sal_uInt32 nId)
{
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
    {
        const ItemType* pAttrib = dynamic_cast<const ItemType *>((*it).pAttr);
        if (pAttrib && pAttrib->Which() == nId)
        {
            CPPUNIT_ASSERT_EQUAL( nVal, static_cast<ItemValue>(pAttrib->GetValue()));
        }
    }
}

}

void SdOOXMLExportTest1::testTdf147919()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf147919.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[1]/p:spPr/a:prstGeom", "prst",
                "bentConnector2");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:prstGeom/a:avLst/a:gd", "name", "adj1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:prstGeom", "prst",
                "bentConnector3");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:xfrm", "rot", "16200000");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:prstGeom", "prst",
                "bentConnector4");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:xfrm", "flipV", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:xfrm", "rot", "10800000");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom/a:avLst/a:gd[3]", "name", "adj3");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom", "prst",
                "bentConnector5");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[5]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[5]/p:spPr/a:xfrm", "rot", "16200000");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[5]/p:spPr/a:prstGeom", "prst",
                "curvedConnector2");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:xfrm", "rot", "16200000");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:prstGeom/a:avLst/a:gd", "name", "adj1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:prstGeom", "prst",
                "curvedConnector3");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:xfrm", "flipV", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:xfrm", "rot", "10800000");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:prstGeom", "prst",
                "curvedConnector4");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:xfrm", "flipV", "1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:xfrm", "rot", "16200000");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom/a:avLst/a:gd[1]", "name", "adj1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom/a:avLst/a:gd[2]", "name", "adj2");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom/a:avLst/a:gd[3]", "name", "adj3");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom", "prst",
                "curvedConnector5");
}

void SdOOXMLExportTest1::testTdf130165()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf146223.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "/p:sld", "showMasterSp", "0");
}

void SdOOXMLExportTest1::testTdf124781()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf124781.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:r[2]/a:rPr", "baseline",
                "33000");

    xmlDocUniquePtr pXmlDoc2 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc2, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:r[2]/a:rPr", "baseline",
                "-8000");
}

void SdOOXMLExportTest1::testTdf144914()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf144616.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:nvSpPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=firstslide");

    xmlDocUniquePtr pXmlDoc2 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:nvSpPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=lastslide");

    xmlDocUniquePtr pXmlDoc3 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:nvSpPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=nextslide");

    xmlDocUniquePtr pXmlDoc4 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:nvSpPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=previousslide");

    xmlDocUniquePtr pXmlDoc5 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc5, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:nvSpPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinksldjump");
    xmlDocUniquePtr pRelsDoc5 = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pRelsDoc5, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "slide2.xml");

    xmlDocUniquePtr pXmlDoc6 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc6, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:nvSpPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=endshow");

    xmlDocUniquePtr pXmlDoc7 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc7, "/p:sld/p:cSld/p:spTree/p:sp[7]/p:nvSpPr/p:cNvPr/a:hlinkClick", "id",
                "rId2");
    xmlDocUniquePtr pRelsDoc7 = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pRelsDoc7, "/rels:Relationships/rels:Relationship[@Id='rId2']", "Target",
                "http://www.example.com/");
}

void SdOOXMLExportTest1::testTdf124232()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf141704.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=lastslide");

    xmlDocUniquePtr pXmlDoc2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDoc2, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=nextslide");

    xmlDocUniquePtr pXmlDoc3 = parseExport(tempFile, "ppt/slides/slide3.xml");
    assertXPath(pXmlDoc3, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick", "action",
                "ppaction://hlinkshowjump?jump=previousslide");

    xmlDocUniquePtr pXmlDoc4 = parseExport(tempFile, "ppt/slides/slide4.xml");
    assertXPath(pXmlDoc4, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick",
        "id", "rId1");
    xmlDocUniquePtr pRelsDoc4 = parseExport(tempFile, "ppt/slides/_rels/slide4.xml.rels");
    assertXPath(pRelsDoc4, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "http://www.example.com/");

    xmlDocUniquePtr pXmlDoc5 = parseExport(tempFile, "ppt/slides/slide5.xml");
    assertXPath(pXmlDoc5, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick",
        "action", "ppaction://hlinksldjump");
    xmlDocUniquePtr pRelsDoc5 = parseExport(tempFile, "ppt/slides/_rels/slide5.xml.rels");
    assertXPath(pRelsDoc5, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
        "slide6.xml");

    xmlDocUniquePtr pXmlDoc6 = parseExport(tempFile, "ppt/slides/slide6.xml");
    assertXPath(pXmlDoc6, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick",
        "action", "ppaction://hlinkshowjump?jump=endshow");

    xmlDocUniquePtr pXmlDoc7 = parseExport(tempFile, "ppt/slides/slide7.xml");
    assertXPath(pXmlDoc7, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick",
        "action", "ppaction://hlinkshowjump?jump=firstslide");
}

void SdOOXMLExportTest1::testTdf143624()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf143624.pptx"), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/presProps.xml");

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr", "useTimings", "0");

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr", "showNarration", "1");
}

void SdOOXMLExportTest1::testTdf142648()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf142648.pptx"), PPTX );
    utl::TempFile tempFile;

    uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage;
    xDrawPages->getByIndex(0) >>= xDrawPage;
    uno::Reference<container::XNamed> xNamed(xDrawPage, uno::UNO_QUERY_THROW);
    xNamed->setName("#Slide 1");

    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pRelsDoc = parseExport(tempFile, "ppt/slides/_rels/slide2.xml.rels");

    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "slide1.xml");
}

void SdOOXMLExportTest1::testTdf47365()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/loopNoPause.pptx"), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/presProps.xml");

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr", "loop", "1");

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr", "showNarration", "1");
}

void SdOOXMLExportTest1::testTdf125071()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf125071.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/presentation.xml");

    assertXPath(pXmlDoc, "/p:presentation/p:custShowLst/p:custShow[1]", "name", "Custom1");
    assertXPath(pXmlDoc, "/p:presentation/p:custShowLst/p:custShow[2]", "name", "Custom2");
}

void SdOOXMLExportTest1::testTdf54037()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf54037.pptx"), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    xmlDocUniquePtr pRelsDoc = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:r/a:rPr/a:hlinkClick",
                "action", "ppaction://hlinksldjump");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:r/a:rPr/a:hlinkClick",
                "action", "ppaction://hlinkshowjump?jump=nextslide");

    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "slide2.xml");
}

void SdOOXMLExportTest1::testTdf127237()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf127237.pptx"), PPTX );
    xDocShRef = saveAndReload(xDocShRef.get(), ODP);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    Color nFillColor;
    uno::Reference< beans::XPropertySet > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), nFillColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc887230()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc887230.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
    // Without the fix in place, this test would have failed with
    //- Expected: 255
    //- Actual  : 13421823
    checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x0000ff), EE_CHAR_COLOR );

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc870233_1()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc870233_1.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // The problem was all shapes had the same font (the last parsed font attributes overwrote all previous ones)

    // First shape has red, bold font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0xff0000), EE_CHAR_COLOR );
        checkFontAttributes<FontWeight, SvxWeightItem>( pObj, WEIGHT_BOLD, EE_CHAR_WEIGHT );
    }

    // Second shape has blue, italic font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x0000ff), EE_CHAR_COLOR);
        checkFontAttributes<FontItalic, SvxPostureItem>( pObj, ITALIC_NORMAL, EE_CHAR_ITALIC);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc870233_2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc870233_2.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // The problem was in some SmartArts font color was wrong

    // First smart art has blue font color (direct formatting)
    {
        const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>(pObjGroup->GetSubList()->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>(pObj, Color(0x0000ff), EE_CHAR_COLOR);
    }

    // Second smart art has "dk2" font color (style)
    {
        const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(1));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>(pObjGroup->GetSubList()->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x1f497d), EE_CHAR_COLOR );
    }

    // Third smart art has white font color (style)
    {
        const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(2));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>(pObjGroup->GetSubList()->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>(pObj, Color(0xffffff), EE_CHAR_COLOR);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testN828390_4()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/n828390_4.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
            if( pFontHeight && (*it).nStart == 18 )
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font height is wrong", static_cast<sal_uInt32>(1129), pFontHeight->GetHeight() );
            const SvxFontItem *pFont = dynamic_cast<const SvxFontItem *>((*it).pAttr);
            if( pFont )
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font is wrong", OUString("Arial"), pFont->GetFamilyName() );
                bPassed = true;
            }
            const SvxWeightItem *pWeight = dynamic_cast<const SvxWeightItem *>((*it).pAttr);
            if( pWeight && (*it).nStart == 18 )
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font Weight is wrong", WEIGHT_BOLD, pWeight->GetWeight() );
        }
    }
    CPPUNIT_ASSERT(bPassed);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testN828390_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/n828390_5.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem& rNumFmt = aEdit.GetParaAttribs(3).Get(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's relative size is wrong!", sal_uInt16(75), rNumFmt.GetNumRule().GetLevel(1).GetBulletRelSize() ); // != 25
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testLostPlaceholders()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/LostPlaceholder.odp"), ODP);
    CPPUNIT_ASSERT(xDocShRef.is());

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    CPPUNIT_ASSERT(xDocShRef.is());

    auto pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    auto pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT(pPage);
    auto pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT(pObj);
    uno::Reference<drawing::XShape> xShp (pObj->getUnoShape(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ShapeType!", OUString(u"com.sun.star.presentation.OutlinerShape"), xShp->getShapeType());
    uno::Reference<beans::XPropertySet> xShpProps(xShp, uno::UNO_QUERY);
    // Without the fix in place there will be the following error:
    // Expected: com.sun.star.presentation.OutlinerShape
    // Actual: com.sun.star.drawing.CustomShape

    CPPUNIT_ASSERT_EQUAL_MESSAGE("It must be a placeholder!", true, xShpProps->getPropertyValue("IsPresentationObject").get<bool>());
    // Without the fix in place this will the following:
    // Expected: true
    // Actual: false

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testPlaceholderFillAndOutlineExport()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/LostPlaceholderFill.odp"), ODP);

    uno::Any aFillStyle;
    uno::Any aFillColor;
    uno::Any aLineStyle;
    uno::Any aLineColor;

    for (int i = 1; i <= 2; i++)
    {
        CPPUNIT_ASSERT(xDocShRef.is());

        auto pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT(pDoc);
        auto pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT(pPage);
        auto pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        uno::Reference<drawing::XShape> xShp(pObj->getUnoShape(), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShp);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ShapeType!", OUString(u"com.sun.star.presentation.OutlinerShape"), xShp->getShapeType());
        uno::Reference<beans::XPropertySet> xShpProps(xShp, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("It must be a placeholder!", true, xShpProps->getPropertyValue("IsPresentationObject").get<bool>());

        if (i == 1)
        {
            aFillStyle = xShpProps->getPropertyValue("FillStyle");
            aFillColor = xShpProps->getPropertyValue("FillColor");

            aLineStyle = xShpProps->getPropertyValue("LineStyle");
            aLineColor = xShpProps->getPropertyValue("LineColor");

            xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
            continue;
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder fillstyle has not been exported!", aFillStyle, xShpProps->getPropertyValue("FillStyle"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder fillcolor has not been exported!", aFillColor, xShpProps->getPropertyValue("FillColor"));

            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder linestyle has not been exported!", aLineStyle, xShpProps->getPropertyValue("LineStyle"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder linecolor has not been exported!", aLineColor, xShpProps->getPropertyValue("LineColor"));
            break;
        }
    }
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testFdo71961()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo71961.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Export to .pptx changes all text frames to custom shape objects, which obey TextWordWrap property
    // (which is false for text frames otherwise and is ignored). Check that frames that should wrap still do.
    auto  pTxtObj = pPage->GetObj( 1 );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text to be always wrapped" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape non-wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( false, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testN828390()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n828390.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        // Get the object
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        bPassed = std::any_of(rLst.rbegin(), rLst.rend(),
            [](const EECharAttrib& rCharAttr) {
                const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>(rCharAttr.pAttr);
                return pFontEscapement && (pFontEscapement->GetEsc() == -25);
            });
    }
    CPPUNIT_ASSERT_MESSAGE("Subscript not exported properly", bPassed);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc880763()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc880763.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Check z-order of the two shapes, use background color to identify them
    // First object in the background has blue background color
    const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject *pObj = pObjGroup->GetSubList()->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( Color(0x0000ff),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue());

    // Second object at the front has green background color
    pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( Color(0x00ff00),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBnc862510_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc862510_5.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Same as testBnc870237, but here we check the horizontal spacing
    const SdrObjGroup *pObjGroup = dynamic_cast<SdrObjGroup *>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject* pObj = pObjGroup->GetSubList()->GetObj(2);
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(7510), pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());

    xDocShRef->DoClose();
}

// In numbering a bullet could be defined as empty (no character).
// When exporting to OOXML make sure that the bullet is ignored and
// not written into the file.
void SdOOXMLExportTest1::testBnc822347_EmptyBullet()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/bnc822347_EmptyBullet.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX);

    SdDrawDocument* pDoc = xDocShRef->GetDoc();
    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
    const SdrPage* pPage = pDoc->GetPage(1);
    SdrObject* pObject = pPage->GetObj(0);
    SdrTextObj* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
    CPPUNIT_ASSERT(pTextObject);

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();

    OUString sText = aEdit.GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("M3 Feature Test"), sText);

    pOutliner->SetText(*pOutlinerParagraphObject);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pOutliner->GetParagraphCount());

    const sal_Int16 nDepth = pOutliner->GetDepth(0);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(-1), nDepth); // depth >= 0 means that the paragraph has bullets enabled

    xDocShRef->DoClose();
}

//Bullets not having  any text following them are not getting exported to pptx correctly.
void SdOOXMLExportTest1::testFdo90607()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo90607.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(1) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    OutlinerParaObject* pOutlinerParagraphObject = pTxtObj->GetOutlinerParaObject();
    const sal_Int16 nDepth = pOutlinerParagraphObject->GetDepth(0);
    CPPUNIT_ASSERT_MESSAGE("not equal", nDepth != -1);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testFdo83751()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/fdo83751.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
    uno::Reference<document::XDocumentProperties> xProps = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
    OUString propValue;
    xUDProps->getPropertyValue("Testing") >>= propValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Document"), propValue);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testFdo79731()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo79731.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTableCellFillProperties()
{
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    // Load the original file
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/Table_with_Cell_Fill.odp"), ODP);

    // Export the document and import again for a check
    uno::Reference< lang::XComponent > xComponent = xDocShRef->GetModel();
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OStringToOUString(std::string_view(aFileFormats[PPTX].pFilterName), RTL_TEXTENCODING_UTF8);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    xComponent.set(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    xDocShRef = loadURL(aTempFile.GetURL(), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;

    // Test Solid fill color
    Color nColor;
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x66ffff), nColor);

    // Test Picture fill type for cell
    drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_BITMAP), static_cast<int>(aFillStyle));

    // Test Gradient fill type for cell
    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(aFillStyle));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletStartNumber()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n90255.pptx"), PPTX );
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's start number is wrong!", sal_Int16(3), sal_Int16(pNumFmt->GetNumRule().GetLevel(0).GetStart()) );
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testLineStyle()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/lineStyle.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject const* pShape = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pShape->GetMergedItem(XATTR_LINESTYLE));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong style", int(drawing::LineStyle_SOLID), static_cast<int>(rStyleItem.GetValue()));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testRightToLeftParaghraph()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/rightToLeftParagraph.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue( "WritingMode" ) >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB, nWritingMode);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTextboxWithHyperlink()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/hyperlinktest.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // first chunk of text
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("http://www.xkcd.com/"), aURL);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf136911()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf136911.ppt"), PPT);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get second paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // first chunk of text
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("http://google.com"), aURL);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bulletColor.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's color is wrong!", Color(0xff0000),pNumFmt->GetNumRule().GetLevel(0).GetBulletColor());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletCharAndFont()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/bulletCharAndFont.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    uno::Reference<container::XIndexAccess> xLevels(xPropSet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    OUString    sBulletChar(u'\xf06c');
    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "BulletChar")
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "BulletChar incorrect.", sBulletChar ,rProp.Value.get<OUString>());
        if (rProp.Name == "BulletFont")
        {
            awt::FontDescriptor aFontDescriptor;
            rProp.Value >>= aFontDescriptor;
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "BulletFont incorrect.", OUString("Wingdings"),aFontDescriptor.Name);
        }
    }
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testBulletMarginAndIndentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bulletMarginAndIndent.pptx"), PPTX );
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's left margin is wrong!", sal_Int32(1000),pNumFmt->GetNumRule().GetLevel(0).GetAbsLSpace() ); // left margin is 0.79 cm
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's indentation is wrong!", sal_Int32(-1000),pNumFmt->GetNumRule().GetLevel(0). GetFirstLineOffset());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testParaMarginAndindentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/paraMarginAndIndentation.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int32 nParaLeftMargin = 0;
    xPropSet->getPropertyValue( "ParaLeftMargin" ) >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1000), sal_uInt32(nParaLeftMargin));

    sal_Int32 nParaFirstLineIndent = 0;
    xPropSet->getPropertyValue( "ParaFirstLineIndent" ) >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1270), nParaFirstLineIndent);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testCellLeftAndRightMargin()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/n90223.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    sal_Int32 nLeftMargin, nRightMargin;

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    uno::Reference< css::table::XTable > xTable (pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< css::table::XMergeableCell > xCell( xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xCellPropSet(xCell, uno::UNO_QUERY_THROW);

    uno::Any aLeftMargin = xCellPropSet->getPropertyValue("TextLeftDistance");
    CPPUNIT_ASSERT(aLeftMargin >>= nLeftMargin);

    uno::Any aRightMargin = xCellPropSet->getPropertyValue("TextRightDistance");
    CPPUNIT_ASSERT(aRightMargin >>= nRightMargin);

    // Convert values to EMU
    nLeftMargin  =  oox::drawingml::convertHmmToEmu( nLeftMargin );
    nRightMargin =  oox::drawingml::convertHmmToEmu( nRightMargin );

    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nRightMargin);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testMergedCells()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/cellspan.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));

    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XTable > xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< text::XTextRange > xText1(xTable->getCellByPosition(3, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL( OUString("0,3"), xText1->getString() );

    uno::Reference< text::XTextRange > xText2(xTable->getCellByPosition(3, 2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL( OUString("2,3"), xText2->getString() );
}

void SdOOXMLExportTest1::testTableCellBorder()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/n90190.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    table::BorderLine2 aBorderLine;

    uno::Reference< table::XTable > xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< css::table::XMergeableCell > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCellPropSet (xCell, uno::UNO_QUERY_THROW);

    xCellPropSet->getPropertyValue("LeftBorder") >>= aBorderLine;
// While importing the table cell border line width, it converts EMU->Hmm then divided result by 2.
// To get original value of LineWidth need to multiple by 2.
    sal_Int32 nLeftBorder = aBorderLine.LineWidth * 2;
    nLeftBorder = oox::drawingml::convertHmmToEmu( nLeftBorder );
    CPPUNIT_ASSERT(nLeftBorder);
    CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), Color(ColorTransparency, aBorderLine.Color));

    xCellPropSet->getPropertyValue("RightBorder") >>= aBorderLine;
    sal_Int32 nRightBorder = aBorderLine.LineWidth * 2;
    nRightBorder = oox::drawingml::convertHmmToEmu( nRightBorder );
    CPPUNIT_ASSERT(nRightBorder);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(ColorTransparency, aBorderLine.Color));

    xCellPropSet->getPropertyValue("TopBorder") >>= aBorderLine;
    sal_Int32 nTopBorder = aBorderLine.LineWidth * 2;
    nTopBorder = oox::drawingml::convertHmmToEmu( nTopBorder );
    CPPUNIT_ASSERT(nTopBorder);
    CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), Color(ColorTransparency, aBorderLine.Color));


    xCellPropSet->getPropertyValue("BottomBorder") >>= aBorderLine;
    sal_Int32 nBottomBorder = aBorderLine.LineWidth * 2;
    nBottomBorder = oox::drawingml::convertHmmToEmu( nBottomBorder );
    CPPUNIT_ASSERT(nBottomBorder);
    CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), Color(ColorTransparency, aBorderLine.Color));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf111884()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111884.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    const SdrPage *pPage = GetPage(1, xDocShRef);
    SdrObject const* pShape = pPage->GetObj(2);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    // must be a group shape
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Group, pShape->GetObjIdentifier());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf112633()
{
    // Load document and export it to a temporary file
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf112633.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    xmlDocUniquePtr pRelsDoc = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");

    // Check image with artistic effect exists in the slide
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/"
            "a14:imgLayer/a14:imgEffect/a14:artisticPencilGrayscale",
            "pencilSize", "80");

    // Check there is a relation with the .wdp file that contains the backed up image
    OUString sEmbedId1 = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:extLst/"
            "a:ext/a14:imgProps/a14:imgLayer", "embed");
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    assertXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target", "../media/hdphoto1.wdp");

    // Check the .wdp file exists
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getComponentContext(m_xSFactory), tempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("ppt/media/hdphoto1.wdp")));
}

void SdOOXMLExportTest1::testTdf128952()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf128952.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:off", "x", "360");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:off", "y", "-360");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:ext", "cx", "1919880");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:ext", "cy", "1462680");
}

void SdOOXMLExportTest1::testTdf127090()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf127090.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr", "rot", "-5400000");
}

void SdOOXMLExportTest1::testCustomXml()
{
    // Load document and export it to a temporary file
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/customxml.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "customXml/item1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    xmlDocUniquePtr pRelsDoc = parseExport(tempFile, "customXml/_rels/item1.xml.rels");
    CPPUNIT_ASSERT(pRelsDoc);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship", 1);
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target", "itemProps1.xml");

    std::unique_ptr<SvStream> pStream = parseExportStream(tempFile, "ddp/ddpfile.xen");
    CPPUNIT_ASSERT(pStream);
}

void SdOOXMLExportTest1::testTdf94238()
{
    // Load document and export it to a temporary file.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf94238.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xShape.is());

    awt::Gradient aGradient;
    CPPUNIT_ASSERT(xShape->getPropertyValue("FillGradient") >>= aGradient);

    // Without the accompanying fix in place, this test would have failed with
    // the following details:
    // - aGradient.Style was awt::GradientStyle_ELLIPTICAL
    // - aGradient.YOffset was 70
    // - aGradient.Border was 0
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(100), aGradient.YOffset);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(39), aGradient.Border);

    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 0, Actual  : 10592673', i.e. the start color of the gradient
    // was incorrect.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x0), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x8B8B8B), aGradient.EndColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testPictureTransparency()
{
    // Load document and export it to a temporary file.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/image_transparency.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xGraphicShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xGraphicShape.is());

    sal_Int16 nTransparency = 0;
    CPPUNIT_ASSERT(xGraphicShape->getPropertyValue("Transparency") >>= nTransparency);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(51), nTransparency);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf125554()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125554.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    uno::Reference<beans::XPropertySet> xShape = getShapeFromPage(0, 0, xDocShRef);
    uno::Any aFillTransparenceGradientName
        = xShape->getPropertyValue("FillTransparenceGradientName");
    CPPUNIT_ASSERT(aFillTransparenceGradientName.has<OUString>());
    // Without the accompanying fix in place, this test would have failed, i.e. the transparency of
    // the shape has no gradient, so it looked like a solid fill instead of a gradient fill.
    CPPUNIT_ASSERT(!aFillTransparenceGradientName.get<OUString>().isEmpty());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testRoundtripOwnLineStyles()
{
    // Load odp document and read the LineDash values.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/LineStylesOwn.odp"), ODP);
    uno::Reference<drawing::XDrawPagesSupplier> xDocodp(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocodp.is());
    uno::Reference<drawing::XDrawPage> xPageodp(xDocodp->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageodp.is());
    drawing::LineDash aLineDashodp[10];
    for (sal_uInt16 i= 0; i < 10; i++)
    {
        uno::Reference<beans::XPropertySet> xShapeodp(getShape(i, xPageodp));
        CPPUNIT_ASSERT(xShapeodp.is());
        xShapeodp->getPropertyValue("LineDash") >>= aLineDashodp[i];
    }

    // Save to pptx, reload and compare the LineDash values
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    uno::Reference<drawing::XDrawPagesSupplier> xDocpptx(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocpptx.is());
    uno::Reference<drawing::XDrawPage> xPagepptx(xDocpptx->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPagepptx.is());

    for (sal_uInt16 i = 0; i < 10; i++)
    {
        drawing::LineDash aLineDashpptx;
        uno::Reference<beans::XPropertySet> xShapepptx(getShape(i, xPagepptx));
        CPPUNIT_ASSERT(xShapepptx.is());
        xShapepptx->getPropertyValue("LineDash") >>= aLineDashpptx;
        bool bIsSameLineDash = (aLineDashodp[i].Style == aLineDashpptx.Style
                                && aLineDashodp[i].Dots == aLineDashpptx.Dots
                                && aLineDashodp[i].DotLen == aLineDashpptx.DotLen
                                && aLineDashodp[i].Dashes == aLineDashpptx.Dashes
                                && aLineDashodp[i].DashLen == aLineDashpptx.DashLen
                                && aLineDashodp[i].Distance == aLineDashpptx.Distance);
        CPPUNIT_ASSERT_MESSAGE("LineDash differ", bIsSameLineDash);
    }
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testRoundtripPrstDash()
{
    // load and save document, compare prstDash values in saved document with original.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/presetDashDot.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    const OUString sOriginal[] = {
        "dash",
        "dashDot",
        "dot",
        "lgDash",
        "lgDashDot",
        "lgDashDotDot",
        "sysDash",
        "sysDashDot",
        "sysDashDotDot",
        "sysDot"
    };
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sStart = "/p:sld/p:cSld/p:spTree/p:sp[";
    const OString sEnd = "]/p:spPr/a:ln/a:prstDash";
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        OString sXmlPath = sStart + OString::number(i+1) + sEnd;
        OUString sResaved = getXPath(pXmlDoc, sXmlPath, "val");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong prstDash", sOriginal[i], sResaved);
    }

    // tdf#126746: Make sure that dash-dot pattern starts with the longer dash, as defined in OOXML
    // Make sure Style is drawing::DashStyle_RECTRELATIVE
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        drawing::LineDash aLineDash;
        uno::Reference<beans::XPropertySet> xShape(getShape(i, xPage));
        CPPUNIT_ASSERT(xShape.is());
        xShape->getPropertyValue("LineDash") >>= aLineDash;
        CPPUNIT_ASSERT_MESSAGE("First dash is short", aLineDash.DotLen >= aLineDash.DashLen);
        bool bIsRectRelative = aLineDash.Style == drawing::DashStyle_RECTRELATIVE;
        CPPUNIT_ASSERT_MESSAGE("not RECTRELATIVE", bIsRectRelative);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testDashOnHairline()
{
    // load and save document, make sure the custDash has 11 child elements.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf127267DashOnHairline.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:ln/a:custDash/a:ds", 11);
}

void SdOOXMLExportTest1::testNarrationNonMediaShape()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/narration-non-media-shape.pptx"), PPTX);
    utl::TempFile aTempFile;
    // Without the accompanying fix in place, this test would have failed,
    // beans::UnknownPropertyException was thrown.
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &aTempFile);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testCustomshapeBitmapfillSrcrect()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/customshape-bitmapfill-srcrect.pptx"),
        PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    // tdf#132680
    // We are preventing the side effect of DOCX improvement to PPTX case.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect' number of nodes is incorrect
    // i.e. <a:srcRect> was exported as <a:fillRect> in <a:stretch>, which made part of the image
    // invisible.

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect");

    // tdf#134210
    // Original values of attribute of l and r in xml files: <a:srcRect l="4393" r="4393"/>
    // No core feature for handling this. We add support to import filter. We crop the bitmap
    // physically during import and shouldn't export the l r t b attributes anymore. In the
    // future if we add core feature to LibreOffice, we should change the control value with
    // 4393.

    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect", "l");
    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect", "r");
    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect", "t");
    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect", "b");
}



void SdOOXMLExportTest1::testTdf100348FontworkBitmapFill()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf100348_FontworkBitmapFill.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the fontwork shape has a blip bitmap fill and a colored outline.
    // Without the patch, fill and outline were black.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr");
    assertXPath(pXmlDoc, sPathStart + "/a:blipFill/a:blip", 1);
    assertXPath(pXmlDoc, sPathStart + "/a:ln/a:solidFill/a:srgbClr", "val", "ffbf00");
}

void SdOOXMLExportTest1::testTdf100348FontworkGradientGlow()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf100348_FontworkGradientGlow.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the fontwork shape has a gradient fill and a colored glow.
    // Without the patch, fill was black and no glow applied.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr");
    assertXPath(pXmlDoc, sPathStart + "/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "8d281e");
    assertXPath(pXmlDoc, sPathStart + "/a:effectLst/a:glow", "rad", "63360");
    assertXPath(pXmlDoc, sPathStart + "/a:effectLst/a:glow/a:srgbClr", "val", "ff4500");
}

void SdOOXMLExportTest1::testTdf128345FullTransparentGradient()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_FullTransparentGradient.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has no fill. Without the patch, fill was solid red.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill");
}

void SdOOXMLExportTest1::testTdf128345GradientLinear()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_GradientLinear.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has a lin fill. Without the patch, fill was solid red.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:lin", "ang", "3600000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs",2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]", "pos", "25000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "20000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]", "pos", "100000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "80000");
}

void SdOOXMLExportTest1::testTdf128345GradientRadial()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_GradientRadial.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has transparency. In OOXML alpha means 'opacity' with default
    // 100000 for full opak, so only the full transparency with val 0 should be written.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs",2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", 0);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr", "val", "ffffff");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "0");
}

void SdOOXMLExportTest1::testTdf128345GradientAxial()
{
    // Without the patch, symmetric linear gradient with full transparence outside and
    // full opak in the middle were imported as full transparent.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128345_GradientAxial.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShapePropSet(getShapeFromPage(0, 0, xDocShRef));

    awt::Gradient aTransparenceGradient;
    xShapePropSet->getPropertyValue("FillTransparenceGradient") >>= aTransparenceGradient;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aTransparenceGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(ColorTransparency, aTransparenceGradient.EndColor));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aTransparenceGradient.Style);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf134969TransparencyOnColorGradient()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf134969_TransparencyOnColorGradient.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    // Make sure the shape has a transparency in gradient stops.
    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs",2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "60000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "60000");
}

void SdOOXMLExportTest1::testArcTo()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/arc-validiert.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPath("//a:custGeom/a:pathLst/a:path/a:arcTo");
    assertXPath(pXmlDoc, sPath, "wR", "3");
    assertXPath(pXmlDoc, sPath, "hR", "3");
    assertXPath(pXmlDoc, sPath, "stAng", "1800000");
    assertXPath(pXmlDoc, sPath, "swAng", "2700000");
}

void SdOOXMLExportTest1::testNarrationMimeType()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/narration.pptx"), PPTX);
    utl::TempFile aTempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &aTempFile);
    xmlDocUniquePtr pXmlDoc = parseExport(aTempFile, "[Content_Types].xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: audio/mp4
    // - Actual  : application/vnd.sun.star.media
    // i.e. the mime type of the narration was incorrect.
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.m4a']",
                "ContentType", "audio/mp4");

    // Check if the bitmap of the media shape is exported correctly.
    xmlDocUniquePtr pSlideDoc = parseExport(aTempFile, "ppt/slides/slide1.xml");
    OUString aImageId = getXPath(pSlideDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip", "embed");
    xmlDocUniquePtr pRelsDoc = parseExport(aTempFile, "ppt/slides/_rels/slide1.xml.rels");
    OUString aImagePath = "/rels:Relationships/rels:Relationship[@Id='" + aImageId + "']";
    // Something like ../media/image2.png.
    OUString aImageStreamName = getXPath(pRelsDoc, aImagePath.toUtf8(), "Target");
    OUString aImageAbsName = rtl::Uri::convertRelToAbs("file:///ppt/slides/", aImageStreamName);
    // Something like ppt/media/image2.png.
    OUString aImageRelName;
    CPPUNIT_ASSERT(aImageAbsName.startsWith("file:///", &aImageRelName));
    std::unique_ptr<SvStream> pImageStream = parseExportStream(aTempFile, aImageRelName);
    vcl::PngImageReader aReader(*pImageStream);
    BitmapEx aBitmapEx = aReader.read();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 256
    // - Actual  : 120
    // i.e. the bitmap of the narration was lost, some default placeholder was exported instead.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(256), aBitmapEx.GetSizePixel().Height());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. p:blipFill was missing its a:stretch child element, so the shape was invisible.
    assertXPath(pSlideDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:stretch/a:fillRect", 1);

    // Without the accompanying fix in place, this test would have failed with:
    // - ... no attribute 'cmd' exist
    // i.e. '<p:cmd type="call">' was written instead of '<p:cmd type="call" cmd="playFrom(0.0)">'.
    assertXPath(pSlideDoc, "//p:cmd", "cmd", "playFrom(0.0)");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <p:childTnLst> had no <p:audio> children, the whole audio animation node was lost.
    assertXPath(pSlideDoc, "//p:childTnLst/p:audio/p:cMediaNode", 1);

    // Without the accompanying fix in place, this test would have failed with:
    // - ... no attribute 'showWhenStopped' exist
    // i.e. <p:cMediaNode> had the default visibility -> bitmap was visible during slideshow.
    assertXPath(pSlideDoc, "//p:childTnLst/p:audio/p:cMediaNode", "showWhenStopped", "0");

    // Without the accompanying fix in place, this test would have failed with:
    // - ... no attribute 'isNarration' exist
    // i.e. <p:audio> was not a narration -> could not mass-remove narrations on the UI.
    assertXPath(pSlideDoc, "//p:childTnLst/p:audio", "isNarration", "1");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf140865Wordart3D()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf140865Wordart3D.pptx"), PPTX);
    utl::TempFile aTempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &aTempFile);
    xmlDocUniquePtr pXmlDoc = parseExport(aTempFile, "ppt/slides/slide1.xml");

    // without the fix in place a:sp3d was lost on round trip, and so extrusion was lost.
    constexpr OStringLiteral sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d", "extrusionH", "342900");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d", "contourW", "12700");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelT", "w", "114300");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelT", "h", "38100");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelT", "prst", "softRound");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelB", "h", "152400");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:extrusionClr/a:srgbClr", "val", "990000");
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:contourClr/a:srgbClr", "val", "009876");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest1::testTdf124457()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf124457.pptx"), PPTX );
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par[1]/p:cTn",
                "repeatCount", "3000");

    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par[2]/p:cTn",
                "repeatCount", "indefinite");
}

void SdOOXMLExportTest1::testTdf143126()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf143126.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/presProps.xml");

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr", "showNarration", "1");
    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr/p:sldRg", "st", "2" );
    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr/p:sldRg", "end", "3" );
}

void SdOOXMLExportTest1::testTdf143129()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf143129.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/presProps.xml");

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr", "showNarration", "1");
    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr/p:custShow", "id", "0" );
}

void SdOOXMLExportTest1::testTdf118045()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf118045.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:lnSpc/a:spcPct", "val",
                "110000");
}

void SdOOXMLExportTest1::testTdf137675()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf137675.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path", "fill", "none");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest1);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
