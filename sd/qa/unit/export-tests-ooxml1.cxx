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
#include <tools/color.hxx>
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

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <docmodel/uno/UnoGradientTools.hxx>
#include <svx/svdotable.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <rtl/uri.hxx>
#include <vcl/filter/PngImageReader.hxx>

using namespace css;

class SdOOXMLExportTest1 : public SdModelTestBase
{
public:
    SdOOXMLExportTest1()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

namespace
{
template <typename ItemValue, typename ItemType>
void checkFontAttributes(const SdrTextObj* pObj, ItemValue nVal, sal_uInt32 nId)
{
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for (std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend(); ++it)
    {
        const ItemType* pAttrib = dynamic_cast<const ItemType*>((*it).pAttr);
        if (pAttrib && pAttrib->Which() == nId)
        {
            CPPUNIT_ASSERT_EQUAL(nVal, static_cast<ItemValue>(pAttrib->GetValue()));
        }
    }
}
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf152335)
{
    createSdImpressDoc("pptx/tdf152335.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocTheme = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPathChildren(
        pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:effectStyleLst/a:effectStyle[3]"_ostr,
        1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf149803)
{
    createSdImpressDoc("pptx/tdf149803.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp"_ostr, "useBgFill"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf149311)
{
    createSdImpressDoc("odp/tdf149311.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);

    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"slide2.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf149128)
{
    createSdImpressDoc("odp/tdf149128.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn"_ostr,
                "id"_ostr, u"8"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn"_ostr,
                "idx"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:endCxn"_ostr,
                "id"_ostr, u"9"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:endCxn"_ostr,
                "idx"_ostr, u"2"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf66228)
{
    createSdImpressDoc("odp/tdf66228.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"bentConnector3"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf147919)
{
    createSdImpressDoc("odp/tdf147919.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[1]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"bentConnector2"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:prstGeom/a:avLst/a:gd"_ostr,
                "name"_ostr, u"adj1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"bentConnector3"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"16200000"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:prstGeom/a:avLst/a:gd[1]"_ostr,
                "name"_ostr, u"adj1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:prstGeom/a:avLst/a:gd[2]"_ostr,
                "name"_ostr, u"adj2"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[3]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"bentConnector4"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:xfrm"_ostr, "flipV"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"10800000"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom/a:avLst/a:gd[1]"_ostr,
                "name"_ostr, u"adj1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom/a:avLst/a:gd[2]"_ostr,
                "name"_ostr, u"adj2"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom/a:avLst/a:gd[3]"_ostr,
                "name"_ostr, u"adj3"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[4]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"bentConnector5"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[5]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[5]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"16200000"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[5]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"curvedConnector2"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"16200000"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:prstGeom/a:avLst/a:gd"_ostr,
                "name"_ostr, u"adj1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[6]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"curvedConnector3"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:xfrm"_ostr, "flipV"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"10800000"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:prstGeom/a:avLst/a:gd[1]"_ostr,
                "name"_ostr, u"adj1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:prstGeom/a:avLst/a:gd[2]"_ostr,
                "name"_ostr, u"adj2"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[7]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"curvedConnector4"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:xfrm"_ostr, "flipV"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"16200000"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom/a:avLst/a:gd[1]"_ostr,
                "name"_ostr, u"adj1"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom/a:avLst/a:gd[2]"_ostr,
                "name"_ostr, u"adj2"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom/a:avLst/a:gd[3]"_ostr,
                "name"_ostr, u"adj3"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[8]/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"curvedConnector5"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf130165)
{
    createSdImpressDoc("pptx/tdf146223.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld"_ostr, "showMasterSp"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf124781)
{
    createSdImpressDoc("odp/tdf124781.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:r[2]/a:rPr"_ostr,
                "baseline"_ostr, u"33000"_ustr);

    xmlDocUniquePtr pXmlDoc2 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc2, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:r[2]/a:rPr"_ostr,
                "baseline"_ostr, u"-8000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf144914)
{
    createSdImpressDoc("pptx/tdf144616.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=firstslide"_ustr);

    xmlDocUniquePtr pXmlDoc2 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=lastslide"_ustr);

    xmlDocUniquePtr pXmlDoc3 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=nextslide"_ustr);

    xmlDocUniquePtr pXmlDoc4 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=previousslide"_ustr);

    xmlDocUniquePtr pXmlDoc5 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc5, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinksldjump"_ustr);
    xmlDocUniquePtr pRelsDoc5 = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(pRelsDoc5, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"slide2.xml"_ustr);

    xmlDocUniquePtr pXmlDoc6 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc6, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=endshow"_ustr);

    xmlDocUniquePtr pXmlDoc7 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc7, "/p:sld/p:cSld/p:spTree/p:sp[7]/p:nvSpPr/p:cNvPr/a:hlinkClick"_ostr,
                "id"_ostr, u"rId2"_ustr);
    xmlDocUniquePtr pRelsDoc7 = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(pRelsDoc7, "/rels:Relationships/rels:Relationship[@Id='rId2']"_ostr, "Target"_ostr,
                u"http://www.example.com/"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf124232)
{
    createSdImpressDoc("pptx/tdf141704.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=lastslide"_ustr);

    xmlDocUniquePtr pXmlDoc2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPath(pXmlDoc2, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=nextslide"_ustr);

    xmlDocUniquePtr pXmlDoc3 = parseExport(u"ppt/slides/slide3.xml"_ustr);
    assertXPath(pXmlDoc3, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=previousslide"_ustr);

    xmlDocUniquePtr pXmlDoc4 = parseExport(u"ppt/slides/slide4.xml"_ustr);
    assertXPath(pXmlDoc4, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "id"_ostr, u"rId1"_ustr);
    xmlDocUniquePtr pRelsDoc4 = parseExport(u"ppt/slides/_rels/slide4.xml.rels"_ustr);
    assertXPath(pRelsDoc4, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"http://www.example.com/"_ustr);

    xmlDocUniquePtr pXmlDoc5 = parseExport(u"ppt/slides/slide5.xml"_ustr);
    assertXPath(pXmlDoc5, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinksldjump"_ustr);
    xmlDocUniquePtr pRelsDoc5 = parseExport(u"ppt/slides/_rels/slide5.xml.rels"_ustr);
    assertXPath(pRelsDoc5, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"slide6.xml"_ustr);

    xmlDocUniquePtr pXmlDoc6 = parseExport(u"ppt/slides/slide6.xml"_ustr);
    assertXPath(pXmlDoc6, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=endshow"_ustr);

    xmlDocUniquePtr pXmlDoc7 = parseExport(u"ppt/slides/slide7.xml"_ustr);
    assertXPath(pXmlDoc7, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:cNvPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=firstslide"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf143624)
{
    createSdImpressDoc("pptx/tdf143624.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presProps.xml"_ustr);

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr"_ostr, "useTimings"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr"_ostr, "showNarration"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf142648)
{
    createSdImpressDoc("pptx/tdf142648.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDPS(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage;
    xDrawPages->getByIndex(0) >>= xDrawPage;
    uno::Reference<container::XNamed> xNamed(xDrawPage, uno::UNO_QUERY_THROW);
    xNamed->setName(u"#Slide 1"_ustr);

    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);

    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"slide1.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf47365)
{
    createSdImpressDoc("pptx/loopNoPause.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presProps.xml"_ustr);

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr"_ostr, "loop"_ostr, u"1"_ustr);

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr"_ostr, "showNarration"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf125071)
{
    createSdImpressDoc("pptx/tdf125071.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    assertXPath(pXmlDoc, "/p:presentation/p:custShowLst/p:custShow[1]"_ostr, "name"_ostr,
                u"Custom1"_ustr);
    assertXPath(pXmlDoc, "/p:presentation/p:custShowLst/p:custShow[2]"_ostr, "name"_ostr,
                u"Custom2"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf54037)
{
    createSdImpressDoc("pptx/tdf54037.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:r/a:rPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinksldjump"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:r/a:rPr/a:hlinkClick"_ostr,
                "action"_ostr, u"ppaction://hlinkshowjump?jump=nextslide"_ustr);

    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"slide2.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf127237)
{
    createSdImpressDoc("pptx/tdf127237.pptx");
    saveAndReload(u"impress8"_ustr);

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    Color nFillColor;
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x0070C0), nFillColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBnc887230)
{
    createSdImpressDoc("pptx/bnc887230.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    const SdrTextObj* pObj = DynCastSdrTextObj(pPage->GetObj(0));
    // Without the fix in place, this test would have failed with
    //- Expected: 255
    //- Actual  : 13421823
    checkFontAttributes<Color, SvxColorItem>(pObj, COL_LIGHTBLUE, EE_CHAR_COLOR);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBnc870233_1)
{
    createSdImpressDoc("pptx/bnc870233_1.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    // The problem was all shapes had the same font (the last parsed font attributes overwrote all previous ones)

    // First shape has red, bold font
    {
        const SdrTextObj* pObj = DynCastSdrTextObj(pPage->GetObj(0));
        checkFontAttributes<Color, SvxColorItem>(pObj, COL_LIGHTRED, EE_CHAR_COLOR);
        checkFontAttributes<FontWeight, SvxWeightItem>(pObj, WEIGHT_BOLD, EE_CHAR_WEIGHT);
    }

    // Second shape has blue, italic font
    {
        const SdrTextObj* pObj = DynCastSdrTextObj(pPage->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>(pObj, COL_LIGHTBLUE, EE_CHAR_COLOR);
        checkFontAttributes<FontItalic, SvxPostureItem>(pObj, ITALIC_NORMAL, EE_CHAR_ITALIC);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBnc870233_2)
{
    createSdImpressDoc("pptx/bnc870233_2.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    // The problem was in some SmartArts font color was wrong

    // First smart art has blue font color (direct formatting)
    {
        const SdrObjGroup* pObjGroup = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj* pObj = DynCastSdrTextObj(pObjGroup->GetSubList()->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>(pObj, COL_LIGHTBLUE, EE_CHAR_COLOR);
    }

    // Second smart art has "dk2" font color (style)
    {
        const SdrObjGroup* pObjGroup = dynamic_cast<SdrObjGroup*>(pPage->GetObj(1));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj* pObj = DynCastSdrTextObj(pObjGroup->GetSubList()->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>(pObj, Color(0x1f497d), EE_CHAR_COLOR);
    }

    // Third smart art has white font color (style)
    {
        const SdrObjGroup* pObjGroup = dynamic_cast<SdrObjGroup*>(pPage->GetObj(2));
        CPPUNIT_ASSERT(pObjGroup);
        const SdrTextObj* pObj = DynCastSdrTextObj(pObjGroup->GetSubList()->GetObj(1));
        checkFontAttributes<Color, SvxColorItem>(pObj, COL_WHITE, EE_CHAR_COLOR);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testN828390_4)
{
    bool bPassed = false;
    createSdImpressDoc("n828390_4.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    {
        std::vector<EECharAttrib> rLst;
        SdrObject* pObj = pPage->GetObj(0);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT(pTxtObj);
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        for (std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend();
             ++it)
        {
            const SvxFontHeightItem* pFontHeight
                = dynamic_cast<const SvxFontHeightItem*>((*it).pAttr);
            if (pFontHeight && (*it).nStart == 18)
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Font height is wrong", static_cast<sal_uInt32>(1129),
                                             pFontHeight->GetHeight());
            const SvxFontItem* pFont = dynamic_cast<const SvxFontItem*>((*it).pAttr);
            if (pFont)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Font is wrong", u"Arial"_ustr,
                                             pFont->GetFamilyName());
                bPassed = true;
            }
            const SvxWeightItem* pWeight = dynamic_cast<const SvxWeightItem*>((*it).pAttr);
            if (pWeight && (*it).nStart == 18)
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Font Weight is wrong", WEIGHT_BOLD,
                                             pWeight->GetWeight());
        }
    }
    CPPUNIT_ASSERT(bPassed);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testN828390_5)
{
    createSdImpressDoc("n828390_5.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    {
        SdrObject* pObj = pPage->GetObj(0);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT(pTxtObj);
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem& rNumFmt = aEdit.GetParaAttribs(3).Get(EE_PARA_NUMBULLET);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's relative size is wrong!", sal_uInt16(75),
                                     rNumFmt.GetNumRule().GetLevel(1).GetBulletRelSize()); // != 25
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testLostPlaceholders)
{
    createSdImpressDoc("pptx/LostPlaceholder.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    auto pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT(pPage);
    auto pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT(pObj);
    uno::Reference<drawing::XShape> xShp(pObj->getUnoShape(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong ShapeType!", u"com.sun.star.presentation.OutlinerShape"_ustr, xShp->getShapeType());
    uno::Reference<beans::XPropertySet> xShpProps(xShp, uno::UNO_QUERY);
    // Without the fix in place there will be the following error:
    // Expected: com.sun.star.presentation.OutlinerShape
    // Actual: com.sun.star.drawing.CustomShape

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "It must be a placeholder!", true,
        xShpProps->getPropertyValue(u"IsPresentationObject"_ustr).get<bool>());
    // Without the fix in place this will the following:
    // Expected: true
    // Actual: false
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testPlaceholderFillAndOutlineExport)
{
    createSdImpressDoc("pptx/LostPlaceholderFill.odp");

    uno::Any aFillStyle;
    uno::Any aFillColor;
    uno::Any aLineStyle;
    uno::Any aLineColor;

    for (int i = 1; i <= 2; i++)
    {
        SdXImpressDocument* pXImpressDocument
            = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        CPPUNIT_ASSERT(pDoc);
        auto pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT(pPage);
        auto pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        uno::Reference<drawing::XShape> xShp(pObj->getUnoShape(), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShp);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ShapeType!",
                                     u"com.sun.star.presentation.OutlinerShape"_ustr,
                                     xShp->getShapeType());
        uno::Reference<beans::XPropertySet> xShpProps(xShp, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "It must be a placeholder!", true,
            xShpProps->getPropertyValue(u"IsPresentationObject"_ustr).get<bool>());

        if (i == 1)
        {
            aFillStyle = xShpProps->getPropertyValue(u"FillStyle"_ustr);
            aFillColor = xShpProps->getPropertyValue(u"FillColor"_ustr);

            aLineStyle = xShpProps->getPropertyValue(u"LineStyle"_ustr);
            aLineColor = xShpProps->getPropertyValue(u"LineColor"_ustr);

            saveAndReload(u"Impress Office Open XML"_ustr);
            continue;
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder fillstyle has not been exported!",
                                         aFillStyle,
                                         xShpProps->getPropertyValue(u"FillStyle"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder fillcolor has not been exported!",
                                         aFillColor,
                                         xShpProps->getPropertyValue(u"FillColor"_ustr));

            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder linestyle has not been exported!",
                                         aLineStyle,
                                         xShpProps->getPropertyValue(u"LineStyle"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("The Placeholder linecolor has not been exported!",
                                         aLineColor,
                                         xShpProps->getPropertyValue(u"LineColor"_ustr));
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testFdo71961)
{
    createSdImpressDoc("fdo71961.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);
    const SdrPage* pPage = GetPage(1);

    // Export to .pptx changes all text frames to custom shape objects, which obey TextWordWrap property
    // (which is false for text frames otherwise and is ignored). Check that frames that should wrap still do.
    auto pTxtObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(u"Text to be always wrapped"_ustr,
                         pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL(true, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(2));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(u"Custom shape non-wrapped text"_ustr,
                         pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL(false, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(3));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(u"Custom shape wrapped text"_ustr,
                         pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL(true, pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP).GetValue());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testN828390)
{
    bool bPassed = false;
    createSdImpressDoc("pptx/n828390.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    {
        std::vector<EECharAttrib> rLst;
        // Get the object
        SdrObject* pObj = pPage->GetObj(0);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT(pTxtObj);
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        bPassed = std::any_of(rLst.rbegin(), rLst.rend(), [](const EECharAttrib& rCharAttr) {
            const SvxEscapementItem* pFontEscapement
                = dynamic_cast<const SvxEscapementItem*>(rCharAttr.pAttr);
            return pFontEscapement && (pFontEscapement->GetEsc() == -25);
        });
    }
    CPPUNIT_ASSERT_MESSAGE("Subscript not exported properly", bPassed);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBnc880763)
{
    createSdImpressDoc("pptx/bnc880763.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    // Check z-order of the two shapes, use background color to identify them
    // First object in the background has blue background color
    const SdrObjGroup* pObjGroup = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject* pObj = pObjGroup->GetSubList()->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(
        COL_LIGHTBLUE,
        (static_cast<const XColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue());

    // Second object at the front has green background color
    pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(
        COL_LIGHTGREEN,
        (static_cast<const XColorItem&>(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBnc862510_5)
{
    createSdImpressDoc("pptx/bnc862510_5.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    // Same as testBnc870237, but here we check the horizontal spacing
    const SdrObjGroup* pObjGroup = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject* pObj = pObjGroup->GetSubList()->GetObj(2);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7510), pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());
}

// In numbering a bullet could be defined as empty (no character).
// When exporting to OOXML make sure that the bullet is ignored and
// not written into the file.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBnc822347_EmptyBullet)
{
    createSdImpressDoc("bnc822347_EmptyBullet.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
    const SdrPage* pPage = pDoc->GetPage(1);
    SdrObject* pObject = pPage->GetObj(0);
    SdrTextObj* pTextObject = DynCastSdrTextObj(pObject);
    CPPUNIT_ASSERT(pTextObject);

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();

    OUString sText = aEdit.GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"M3 Feature Test"_ustr, sText);

    pOutliner->SetText(*pOutlinerParagraphObject);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pOutliner->GetParagraphCount());

    const sal_Int16 nDepth = pOutliner->GetDepth(0);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(-1),
                         nDepth); // depth >= 0 means that the paragraph has bullets enabled
}

//Bullets not having  any text following them are not getting exported to pptx correctly.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testFdo90607)
{
    createSdImpressDoc("fdo90607.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(1));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    OutlinerParaObject* pOutlinerParagraphObject = pTxtObj->GetOutlinerParaObject();
    const sal_Int16 nDepth = pOutlinerParagraphObject->GetDepth(0);
    CPPUNIT_ASSERT_MESSAGE("not equal", nDepth != -1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testFdo83751)
{
    createSdImpressDoc("pptx/fdo83751.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps
        = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(),
                                                 uno::UNO_QUERY);
    OUString propValue;
    xUDProps->getPropertyValue(u"Testing"_ustr) >>= propValue;
    CPPUNIT_ASSERT_EQUAL(u"Document"_ustr, propValue);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testFdo79731)
{
    createSdImpressDoc("fdo79731.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTableCellFillProperties)
{
    // Load the original file
    createSdImpressDoc("odp/Table_with_Cell_Fill.odp");

    // Export the document and import again for a check
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;

    // Test Solid fill color
    Color nColor;
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x66ffff), nColor);

    // Test Picture fill type for cell
    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_BITMAP), static_cast<int>(aFillStyle));

    // Test Gradient fill type for cell
    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(aFillStyle));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBulletStartNumber)
{
    createSdImpressDoc("pptx/n90255.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's start number is wrong!", sal_Int16(3),
                                 sal_Int16(pNumFmt->GetNumRule().GetLevel(0).GetStart()));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testLineStyle)
{
    createSdImpressDoc("pptx/lineStyle.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    SdrObject const* pShape = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    const XLineStyleItem& rStyleItem = pShape->GetMergedItem(XATTR_LINESTYLE);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong style", int(drawing::LineStyle_SOLID),
                                 static_cast<int>(rStyleItem.GetValue()));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf134862)
{
    createSdImpressDoc("pptx/tdf134862.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue(u"WritingMode"_ustr) >>= nWritingMode;

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB,
                                 nWritingMode);

    saveAndReload(u"Impress Office Open XML"_ustr);

    xShape.set(getShapeFromPage(0, 0));

    xParagraph.set(getParagraphFromShape(0, xShape));
    xPropSet.set(xParagraph, uno::UNO_QUERY_THROW);

    nWritingMode = 0;
    xPropSet->getPropertyValue(u"WritingMode"_ustr) >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB,
                                 nWritingMode);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testRightToLeftParaghraph)
{
    createSdImpressDoc("pptx/rightToLeftParagraph.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue(u"WritingMode"_ustr) >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB,
                                 nWritingMode);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTextboxWithHyperlink)
{
    createSdImpressDoc("pptx/hyperlinktest.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // first chunk of text
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue(u"TextField"_ustr) >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue(u"URL"_ustr) >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", u"http://www.xkcd.com/"_ustr, aURL);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf136911)
{
    createSdImpressDoc("ppt/tdf136911.ppt");

    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get second paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // first chunk of text
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue(u"TextField"_ustr) >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue(u"URL"_ustr) >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", u"http://google.com"_ustr, aURL);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBulletColor)
{
    createSdImpressDoc("pptx/bulletColor.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's color is wrong!", COL_LIGHTRED,
                                 pNumFmt->GetNumRule().GetLevel(0).GetBulletColor());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBulletCharAndFont)
{
    createSdImpressDoc("odp/bulletCharAndFont.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(
        xPropSet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    OUString sBulletChar(u'\xf06c');
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "BulletChar")
            CPPUNIT_ASSERT_EQUAL_MESSAGE("BulletChar incorrect.", sBulletChar,
                                         rProp.Value.get<OUString>());
        if (rProp.Name == "BulletFont")
        {
            awt::FontDescriptor aFontDescriptor;
            rProp.Value >>= aFontDescriptor;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("BulletFont incorrect.", u"Wingdings"_ustr,
                                         aFontDescriptor.Name);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testBulletMarginAndIndentation)
{
    createSdImpressDoc("pptx/bulletMarginAndIndent.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Bullet's left margin is wrong!", sal_Int32(1000),
        pNumFmt->GetNumRule().GetLevel(0).GetAbsLSpace()); // left margin is 0.79 cm
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's indentation is wrong!", sal_Int32(-1000),
                                 pNumFmt->GetNumRule().GetLevel(0).GetFirstLineOffset());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testParaMarginAndindentation)
{
    createSdImpressDoc("pptx/paraMarginAndIndentation.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    sal_Int32 nParaLeftMargin = 0;
    xPropSet->getPropertyValue(u"ParaLeftMargin"_ustr) >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1000), sal_uInt32(nParaLeftMargin));

    sal_Int32 nParaFirstLineIndent = 0;
    xPropSet->getPropertyValue(u"ParaFirstLineIndent"_ustr) >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1270), nParaFirstLineIndent);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testCellLeftAndRightMargin)
{
    createSdImpressDoc("pptx/n90223.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);
    sal_Int32 nLeftMargin, nRightMargin;

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<css::table::XTable> xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XMergeableCell> xCell(xTable->getCellByPosition(0, 0),
                                                     uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCellPropSet(xCell, uno::UNO_QUERY_THROW);

    uno::Any aLeftMargin = xCellPropSet->getPropertyValue(u"TextLeftDistance"_ustr);
    CPPUNIT_ASSERT(aLeftMargin >>= nLeftMargin);

    uno::Any aRightMargin = xCellPropSet->getPropertyValue(u"TextRightDistance"_ustr);
    CPPUNIT_ASSERT(aRightMargin >>= nRightMargin);

    // Convert values to EMU
    nLeftMargin = oox::drawingml::convertHmmToEmu(nLeftMargin);
    nRightMargin = oox::drawingml::convertHmmToEmu(nRightMargin);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nRightMargin);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testMergedCells)
{
    createSdImpressDoc("odp/cellspan.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));

    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XTable> xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference<text::XTextRange> xText1(xTable->getCellByPosition(3, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"0,3"_ustr, xText1->getString());

    uno::Reference<text::XTextRange> xText2(xTable->getCellByPosition(3, 2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"2,3"_ustr, xText2->getString());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTableCellBorder)
{
    createSdImpressDoc("pptx/n90190.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    table::BorderLine2 aBorderLine;

    uno::Reference<table::XTable> xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XMergeableCell> xCell(xTable->getCellByPosition(0, 0),
                                                     uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCellPropSet(xCell, uno::UNO_QUERY_THROW);

    xCellPropSet->getPropertyValue(u"LeftBorder"_ustr) >>= aBorderLine;
    // While importing the table cell border line width, it converts EMU->Hmm then divided result by 2.
    // To get original value of LineWidth need to multiple by 2.
    sal_Int32 nLeftBorder = aBorderLine.LineWidth * 2;
    nLeftBorder = oox::drawingml::convertHmmToEmu(nLeftBorder);
    CPPUNIT_ASSERT(nLeftBorder);
    CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), Color(ColorTransparency, aBorderLine.Color));

    xCellPropSet->getPropertyValue(u"RightBorder"_ustr) >>= aBorderLine;
    sal_Int32 nRightBorder = aBorderLine.LineWidth * 2;
    nRightBorder = oox::drawingml::convertHmmToEmu(nRightBorder);
    CPPUNIT_ASSERT(nRightBorder);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(ColorTransparency, aBorderLine.Color));

    xCellPropSet->getPropertyValue(u"TopBorder"_ustr) >>= aBorderLine;
    sal_Int32 nTopBorder = aBorderLine.LineWidth * 2;
    nTopBorder = oox::drawingml::convertHmmToEmu(nTopBorder);
    CPPUNIT_ASSERT(nTopBorder);
    CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), Color(ColorTransparency, aBorderLine.Color));

    xCellPropSet->getPropertyValue(u"BottomBorder"_ustr) >>= aBorderLine;
    sal_Int32 nBottomBorder = aBorderLine.LineWidth * 2;
    nBottomBorder = oox::drawingml::convertHmmToEmu(nBottomBorder);
    CPPUNIT_ASSERT(nBottomBorder);
    CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), Color(ColorTransparency, aBorderLine.Color));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf111884)
{
    createSdImpressDoc("pptx/tdf111884.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);
    SdrObject const* pShape = pPage->GetObj(2);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    // must be a group shape
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Group, pShape->GetObjIdentifier());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf112633)
{
    // Load document and export it to a temporary file
    createSdImpressDoc("pptx/tdf112633.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);

    // Check image with artistic effect exists in the slide
    assertXPath(pXmlDoc,
                "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/"
                "a14:imgLayer/a14:imgEffect/a14:artisticPencilGrayscale"_ostr,
                "pencilSize"_ostr, u"80"_ustr);

    // Check there is a relation with the .wdp file that contains the backed up image
    OUString sEmbedId1 = getXPath(pXmlDoc,
                                  "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:extLst/"
                                  "a:ext/a14:imgProps/a14:imgLayer"_ostr,
                                  "embed"_ostr);
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    assertXPath(pRelsDoc, OUStringToOString(sXmlPath, RTL_TEXTENCODING_UTF8), "Target"_ostr,
                u"../media/hdphoto1.wdp"_ustr);

    // Check the .wdp file exists
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName(u"ppt/media/hdphoto1.wdp"_ustr)));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf128952)
{
    createSdImpressDoc("pptx/tdf128952.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:off"_ostr, "x"_ostr,
                u"360"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:off"_ostr, "y"_ostr,
                u"-360"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:ext"_ostr, "cx"_ostr,
                u"1919880"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm/a:ext"_ostr, "cy"_ostr,
                u"1462680"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf127090)
{
    createSdImpressDoc("pptx/tdf127090.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr"_ostr, "rot"_ostr,
                u"-5400000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testCustomXml)
{
    // Load document and export it to a temporary file
    createSdImpressDoc("pptx/customxml.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"customXml/item1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    xmlDocUniquePtr pRelsDoc = parseExport(u"customXml/_rels/item1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pRelsDoc);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship"_ostr, 1);
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                u"itemProps1.xml"_ustr);

    std::unique_ptr<SvStream> pStream
        = parseExportStream(maTempFile.GetURL(), u"ddp/ddpfile.xen"_ustr);
    CPPUNIT_ASSERT(pStream);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf94238)
{
    // Load document and export it to a temporary file.
    createSdImpressDoc("pptx/tdf94238.pptx");
    save(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xShape.is());

    awt::Gradient2 aGradient;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"FillGradient"_ustr) >>= aGradient);

    // Without the accompanying fix in place, this test would have failed with
    // the following details:
    // - aGradient.Style was awt::GradientStyle_ELLIPTICAL
    // - aGradient.YOffset was 70
    // - aGradient.Border was 0
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(100), aGradient.YOffset);
    // MCGR: 39->0 no border needed anymore with ooxml import
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), aGradient.Border);

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 0, Actual  : 10592673', i.e. the start color of the gradient
    // was incorrect.
    CPPUNIT_ASSERT_EQUAL(size_t(3), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 0.39000000000000001));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[2].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x8b8b8b), Color(aColorStops[2].getStopColor()));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testPictureTransparency)
{
    // Load document and export it to a temporary file.
    createSdImpressDoc("odp/image_transparency.odp");
    save(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());

    uno::Reference<beans::XPropertySet> xGraphicShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xGraphicShape.is());

    sal_Int16 nTransparency = 0;
    CPPUNIT_ASSERT(xGraphicShape->getPropertyValue(u"Transparency"_ustr) >>= nTransparency);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(51), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf125554)
{
    createSdImpressDoc("pptx/tdf125554.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<beans::XPropertySet> xShape = getShapeFromPage(0, 0);
    uno::Any aFillTransparenceGradientName
        = xShape->getPropertyValue(u"FillTransparenceGradientName"_ustr);
    CPPUNIT_ASSERT(aFillTransparenceGradientName.has<OUString>());
    // Without the accompanying fix in place, this test would have failed, i.e. the transparency of
    // the shape has no gradient, so it looked like a solid fill instead of a gradient fill.
    CPPUNIT_ASSERT(!aFillTransparenceGradientName.get<OUString>().isEmpty());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testRoundtripOwnLineStyles)
{
    // Load odp document and read the LineDash values.
    createSdImpressDoc("odp/LineStylesOwn.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDocodp(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocodp.is());
    uno::Reference<drawing::XDrawPage> xPageodp(xDocodp->getDrawPages()->getByIndex(0),
                                                uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageodp.is());
    drawing::LineDash aLineDashodp[10];
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        uno::Reference<beans::XPropertySet> xShapeodp(getShape(i, xPageodp));
        CPPUNIT_ASSERT(xShapeodp.is());
        xShapeodp->getPropertyValue(u"LineDash"_ustr) >>= aLineDashodp[i];
    }

    // Save to pptx, reload and compare the LineDash values
    save(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDocpptx(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocpptx.is());
    uno::Reference<drawing::XDrawPage> xPagepptx(xDocpptx->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPagepptx.is());

    for (sal_uInt16 i = 0; i < 10; i++)
    {
        drawing::LineDash aLineDashpptx;
        uno::Reference<beans::XPropertySet> xShapepptx(getShape(i, xPagepptx));
        CPPUNIT_ASSERT(xShapepptx.is());
        xShapepptx->getPropertyValue(u"LineDash"_ustr) >>= aLineDashpptx;
        bool bIsSameLineDash = (aLineDashodp[i].Style == aLineDashpptx.Style
                                && aLineDashodp[i].Dots == aLineDashpptx.Dots
                                && aLineDashodp[i].DotLen == aLineDashpptx.DotLen
                                && aLineDashodp[i].Dashes == aLineDashpptx.Dashes
                                && aLineDashodp[i].DashLen == aLineDashpptx.DashLen
                                && aLineDashodp[i].Distance == aLineDashpptx.Distance);
        CPPUNIT_ASSERT_MESSAGE("LineDash differ", bIsSameLineDash);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testRoundtripPrstDash)
{
    // load and save document, compare prstDash values in saved document with original.
    createSdImpressDoc("pptx/presetDashDot.pptx");
    save(u"Impress Office Open XML"_ustr);

    const OUString sOriginal[]
        = { u"dash"_ustr,          u"dashDot"_ustr,      u"dot"_ustr,     u"lgDash"_ustr,
            u"lgDashDot"_ustr,     u"lgDashDotDot"_ustr, u"sysDash"_ustr, u"sysDashDot"_ustr,
            u"sysDashDotDot"_ustr, u"sysDot"_ustr };
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OStringLiteral sStart = "/p:sld/p:cSld/p:spTree/p:sp[";
    static constexpr OStringLiteral sEnd = "]/p:spPr/a:ln/a:prstDash";
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        OString sXmlPath = sStart + OString::number(i + 1) + sEnd;
        OUString sResaved = getXPath(pXmlDoc, sXmlPath, "val"_ostr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong prstDash", sOriginal[i], sResaved);
    }

    // tdf#126746: Make sure that dash-dot pattern starts with the longer dash, as defined in OOXML
    // Make sure Style is drawing::DashStyle_RECTRELATIVE
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        drawing::LineDash aLineDash;
        uno::Reference<beans::XPropertySet> xShape(getShape(i, xPage));
        CPPUNIT_ASSERT(xShape.is());
        xShape->getPropertyValue(u"LineDash"_ustr) >>= aLineDash;
        CPPUNIT_ASSERT_MESSAGE("First dash is short", aLineDash.DotLen >= aLineDash.DashLen);
        bool bIsRectRelative = aLineDash.Style == drawing::DashStyle_RECTRELATIVE;
        CPPUNIT_ASSERT_MESSAGE("not RECTRELATIVE", bIsRectRelative);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testDashOnHairline)
{
    // load and save document, make sure the custDash has 11 child elements.
    createSdImpressDoc("odp/tdf127267DashOnHairline.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:ln/a:custDash/a:ds"_ostr, 11);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testNarrationNonMediaShape)
{
    createSdImpressDoc("pptx/narration-non-media-shape.pptx");
    // Without the accompanying fix in place, this test would have failed,
    // beans::UnknownPropertyException was thrown.
    saveAndReload(u"Impress Office Open XML"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testCustomshapeBitmapfillSrcrect)
{
    createSdImpressDoc("pptx/customshape-bitmapfill-srcrect.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // tdf#132680
    // We are preventing the side effect of DOCX improvement to PPTX case.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect' number of nodes is incorrect
    // i.e. <a:srcRect> was exported as <a:fillRect> in <a:stretch>, which made part of the image
    // invisible.

    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect"_ostr);

    // tdf#134210
    // Original values of attribute of l and r in xml files: <a:srcRect l="4393" r="4393"/>
    // No core feature for handling this. We add support to import filter. We crop the bitmap
    // physically during import and shouldn't export the l r t b attributes anymore. In the
    // future if we add core feature to LibreOffice, we should change the control value with
    // 4393.

    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect"_ostr,
                           "l"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect"_ostr,
                           "r"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect"_ostr,
                           "t"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:blipFill/a:srcRect"_ostr,
                           "b"_ostr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf100348FontworkBitmapFill)
{
    createSdImpressDoc("odp/tdf100348_FontworkBitmapFill.odp");
    save(u"Impress Office Open XML"_ustr);

    // Make sure the fontwork shape has a blip bitmap fill and a colored outline.
    // Without the patch, fill and outline were black.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr"_ostr);
    assertXPath(pXmlDoc, sPathStart + "/a:blipFill/a:blip", 1);
    assertXPath(pXmlDoc, sPathStart + "/a:ln/a:solidFill/a:srgbClr", "val"_ostr, u"ffbf00"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf100348FontworkGradientGlow)
{
    createSdImpressDoc("odp/tdf100348_FontworkGradientGlow.odp");
    save(u"Impress Office Open XML"_ustr);

    // Make sure the fontwork shape has a gradient fill and a colored glow.
    // Without the patch, fill was black and no glow applied.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr"_ostr);
    assertXPath(pXmlDoc, sPathStart + "/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val"_ostr,
                u"8d281e"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:effectLst/a:glow", "rad"_ostr, u"63360"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:effectLst/a:glow/a:srgbClr", "val"_ostr, u"ff4500"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf128345FullTransparentGradient)
{
    createSdImpressDoc("odp/tdf128345_FullTransparentGradient.odp");
    save(u"Impress Office Open XML"_ustr);

    // Make sure the shape has no fill. Without the patch, fill was solid red.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill"_ostr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf128345GradientLinear)
{
    createSdImpressDoc("odp/tdf128345_GradientLinear.odp");
    save(u"Impress Office Open XML"_ustr);

    // Make sure the shape has a lin fill. Without the patch, fill was solid red.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill"_ostr);
    assertXPath(pXmlDoc, sPathStart + "/a:lin", "ang"_ostr, u"3600000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs", 2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]", "pos"_ostr, u"25000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr", "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val"_ostr,
                u"20000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]", "pos"_ostr, u"100000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr", "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val"_ostr,
                u"80000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf128345GradientRadial)
{
    createSdImpressDoc("odp/tdf128345_GradientRadial.odp");
    save(u"Impress Office Open XML"_ustr);

    // Make sure the shape has transparency. In OOXML alpha means 'opacity' with default
    // 100000 for full opak, so only the full transparency with val 0 should be written.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill"_ostr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs", 2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr", "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", 0);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr", "val"_ostr, u"ffffff"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf128345GradientAxial)
{
    // Without the patch, symmetric linear gradient with full transparence outside and
    // full opak in the middle were imported as full transparent.
    createSdImpressDoc("odp/tdf128345_GradientAxial.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShapePropSet(getShapeFromPage(0, 0));

    awt::Gradient2 aTransparenceGradient;
    xShapePropSet->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aTransparenceGradient;

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aTransparenceGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(3), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 0.5));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[2].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(aColorStops[2].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aTransparenceGradient.Style);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf134969TransparencyOnColorGradient)
{
    createSdImpressDoc("odp/tdf134969_TransparencyOnColorGradient.odp");
    save(u"Impress Office Open XML"_ustr);

    // Make sure the shape has a transparency in gradient stops.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:gradFill"_ostr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs", 2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val"_ostr,
                u"60000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val"_ostr,
                u"60000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testArcTo)
{
    createSdImpressDoc("pptx/arc-validiert.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPath("//a:custGeom/a:pathLst/a:path/a:arcTo"_ostr);
    assertXPath(pXmlDoc, sPath, "wR"_ostr, u"3"_ustr);
    assertXPath(pXmlDoc, sPath, "hR"_ostr, u"3"_ustr);
    assertXPath(pXmlDoc, sPath, "stAng"_ostr, u"1800000"_ustr);
    assertXPath(pXmlDoc, sPath, "swAng"_ostr, u"2700000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testNarrationMimeType)
{
    createSdImpressDoc("pptx/narration.pptx");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: audio/mp4
    // - Actual  : application/vnd.sun.star.media
    // i.e. the mime type of the narration was incorrect.
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.m4a']"_ostr,
                "ContentType"_ostr, u"audio/mp4"_ustr);

    // Check if the bitmap of the media shape is exported correctly.
    xmlDocUniquePtr pSlideDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString aImageId
        = getXPath(pSlideDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip"_ostr, "embed"_ostr);
    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    OUString aImagePath = "/rels:Relationships/rels:Relationship[@Id='" + aImageId + "']";
    // Something like ../media/image2.png.
    OUString aImageStreamName = getXPath(pRelsDoc, aImagePath.toUtf8(), "Target"_ostr);
    OUString aImageAbsName
        = rtl::Uri::convertRelToAbs(u"file:///ppt/slides/"_ustr, aImageStreamName);
    // Something like ppt/media/image2.png.
    OUString aImageRelName;
    CPPUNIT_ASSERT(aImageAbsName.startsWith("file:///", &aImageRelName));
    std::unique_ptr<SvStream> pImageStream = parseExportStream(maTempFile.GetURL(), aImageRelName);
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
    assertXPath(pSlideDoc, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:stretch/a:fillRect"_ostr, 1);

    // Without the accompanying fix in place, this test would have failed with:
    // - ... no attribute 'cmd' exist
    // i.e. '<p:cmd type="call">' was written instead of '<p:cmd type="call" cmd="playFrom(0.0)">'.
    assertXPath(pSlideDoc, "//p:cmd"_ostr, "cmd"_ostr, u"playFrom(0.0)"_ustr);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <p:childTnLst> had no <p:audio> children, the whole audio animation node was lost.
    assertXPath(pSlideDoc, "//p:childTnLst/p:audio/p:cMediaNode"_ostr, 1);

    // Without the accompanying fix in place, this test would have failed with:
    // - ... no attribute 'showWhenStopped' exist
    // i.e. <p:cMediaNode> had the default visibility -> bitmap was visible during slideshow.
    assertXPath(pSlideDoc, "//p:childTnLst/p:audio/p:cMediaNode"_ostr, "showWhenStopped"_ostr,
                u"0"_ustr);

    // Without the accompanying fix in place, this test would have failed with:
    // - ... no attribute 'isNarration' exist
    // i.e. <p:audio> was not a narration -> could not mass-remove narrations on the UI.
    assertXPath(pSlideDoc, "//p:childTnLst/p:audio"_ostr, "isNarration"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf150316)
{
    createSdImpressDoc("odp/tdf150316.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:pPr"_ostr, "indent"_ostr,
                u"-343080"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf140865Wordart3D)
{
    createSdImpressDoc("pptx/tdf140865Wordart3D.pptx");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // without the fix in place a:sp3d was lost on round trip, and so extrusion was lost.
    constexpr OString sPathStart("//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr"_ostr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d", "extrusionH"_ostr, u"342900"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d", "contourW"_ostr, u"12700"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelT", "w"_ostr, u"114300"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelT", "h"_ostr, u"38100"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelT", "prst"_ostr, u"softRound"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:bevelB", "h"_ostr, u"152400"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:extrusionClr/a:srgbClr", "val"_ostr,
                u"990000"_ustr);
    assertXPath(pXmlDoc, sPathStart + "/a:sp3d/a:contourClr/a:srgbClr", "val"_ostr, u"009876"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf124457)
{
    createSdImpressDoc("pptx/tdf124457.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(
        pXmlDoc,
        "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
        "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par[1]/p:cTn"_ostr,
        "repeatCount"_ostr, u"3000"_ustr);

    assertXPath(
        pXmlDoc,
        "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
        "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par[2]/p:cTn"_ostr,
        "repeatCount"_ostr, u"indefinite"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf143126)
{
    createSdImpressDoc("pptx/tdf143126.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presProps.xml"_ustr);

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr"_ostr, "showNarration"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr/p:sldRg"_ostr, "st"_ostr, u"2"_ustr);
    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr/p:sldRg"_ostr, "end"_ostr, u"3"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf143129)
{
    createSdImpressDoc("pptx/tdf143129.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presProps.xml"_ustr);

    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr"_ostr, "showNarration"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/p:presentationPr/p:showPr/p:custShow"_ostr, "id"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf118045)
{
    createSdImpressDoc("odp/tdf118045.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:lnSpc/a:spcPct"_ostr,
                "val"_ostr, u"110000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf137675)
{
    createSdImpressDoc("pptx/tdf137675.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path"_ostr,
                "fill"_ostr, u"none"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest1, testTdf151134)
{
    createSdImpressDoc("pptx/tdf151134.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr"_ostr, "lIns"_ostr,
                u"91440"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
