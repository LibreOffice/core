/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <comphelper/propertysequence.hxx>
#include <editeng/flditem.hxx>

#include <svx/svdomedia.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <rtl/character.hxx>
#include <o3tl/string_view.hxx>

using namespace css;
using namespace css::animations;

namespace
{
bool checkBeginWithNumber(std::u16string_view aStr)
{
    sal_Unicode aChar = (aStr.size() > 1) ? aStr[0] : '\0';
    return aChar == '.' || aChar == '-' || rtl::isAsciiDigit(aChar);
}
}

#define CPPUNIT_ASSERT_MOTIONPATH(expect, actual)                                                  \
    assertMotionPath(expect, actual, CPPUNIT_SOURCELINE())

static void assertMotionPath(std::u16string_view rStr1, std::u16string_view rStr2,
                             const CppUnit::SourceLine& rSourceLine)
{
    sal_Int32 nIdx1 = 0;
    sal_Int32 nIdx2 = 0;

    OString sMessage
        = "Motion path values mismatch.\nExpect: " + OUStringToOString(rStr1, RTL_TEXTENCODING_UTF8)
          + "\nActual: " + OUStringToOString(rStr2, RTL_TEXTENCODING_UTF8);

    while (nIdx1 != -1 && nIdx2 != -1)
    {
        OUString aToken1(o3tl::getToken(rStr1, 0, ' ', nIdx1));
        OUString aToken2(o3tl::getToken(rStr2, 0, ' ', nIdx2));

        if (checkBeginWithNumber(aToken1) && checkBeginWithNumber(aToken2))
            assertDoubleEquals(aToken1.toDouble(), aToken2.toDouble(), DBL_EPSILON, rSourceLine,
                               std::string(sMessage));
        else
            assertEquals(aToken1, aToken2, rSourceLine, std::string(sMessage));
    }
    assertEquals(sal_Int32(-1), nIdx1, rSourceLine, std::string(sMessage));
    assertEquals(sal_Int32(-1), nIdx2, rSourceLine, std::string(sMessage));
}

class SdOOXMLExportTest2 : public SdModelTestBase
{
public:
    SdOOXMLExportTest2()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testRepeatBitmapMode)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    createSdImpressDoc("odp/repeatBitmapMode.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "tx"_ostr,
                u"1269669"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "ty"_ostr,
                u"186051"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "sx"_ostr,
                u"100000"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "sy"_ostr,
                u"100000"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "algn"_ostr,
                u"tr"_ustr);

    // if the "Scale" setting is checked in the images settings dialog.
    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "tx"_ostr,
                u"0"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "ty"_ostr,
                u"0"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "sx"_ostr,
                u"682760"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "sy"_ostr,
                u"639983"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:tile"_ostr, "algn"_ostr,
                u"ctr"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf153107)
{
    createSdImpressDoc("odp/tdf153107.odp");
    save(u"Impress Office Open XML"_ustr);

    // placeholder
    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:blipFill/a:tile"_ostr,
                "tx"_ostr, u"1879200"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:blipFill/a:tile"_ostr,
                "ty"_ostr, u"83628"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:blipFill/a:tile"_ostr,
                "sx"_ostr, u"264773"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:blipFill/a:tile"_ostr,
                "sy"_ostr, u"91428"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:blipFill/a:tile"_ostr,
                "algn"_ostr, u"ctr"_ustr);

    // custom shape
    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:blipFill/a:tile"_ostr,
                "tx"_ostr, u"198000"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:blipFill/a:tile"_ostr,
                "ty"_ostr, u"324000"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:blipFill/a:tile"_ostr,
                "sx"_ostr, u"69743"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:blipFill/a:tile"_ostr,
                "sy"_ostr, u"78709"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:blipFill/a:tile"_ostr,
                "algn"_ostr, u"tl"_ustr);

    // polygon
    xmlDocUniquePtr pXmlDocContent3 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:blipFill/a:tile"_ostr,
                "tx"_ostr, u"2073600"_ustr);
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:blipFill/a:tile"_ostr,
                "ty"_ostr, u"221760"_ustr);
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:blipFill/a:tile"_ostr,
                "sx"_ostr, u"182602"_ustr);
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:blipFill/a:tile"_ostr,
                "sy"_ostr, u"86580"_ustr);
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:blipFill/a:tile"_ostr,
                "algn"_ostr, u"ctr"_ustr);

    // textbox
    xmlDocUniquePtr pXmlDocContent4 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:blipFill/a:tile"_ostr,
                "tx"_ostr, u"662400"_ustr);
    assertXPath(pXmlDocContent4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:blipFill/a:tile"_ostr,
                "ty"_ostr, u"760320"_ustr);
    assertXPath(pXmlDocContent4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:blipFill/a:tile"_ostr,
                "sx"_ostr, u"202891"_ustr);
    assertXPath(pXmlDocContent4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:blipFill/a:tile"_ostr,
                "sy"_ostr, u"129870"_ustr);
    assertXPath(pXmlDocContent4, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:blipFill/a:tile"_ostr,
                "algn"_ostr, u"tl"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf142291)
{
    createSdImpressDoc("pptx/tdt142291.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr/a:tc[1]/"
                "a:tcPr/a:lnL/a:prstDash"_ostr,
                "val"_ostr, u"sysDashDotDot"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr/a:tc[1]/"
                "a:tcPr/a:lnR/a:prstDash"_ostr,
                "val"_ostr, u"dot"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr/a:tc[1]/"
                "a:tcPr/a:lnT/a:prstDash"_ostr,
                "val"_ostr, u"solid"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr/a:tc[1]/"
                "a:tcPr/a:lnB/a:prstDash"_ostr,
                "val"_ostr, u"dash"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr/a:tc[2]/"
                "a:tcPr/a:lnR/a:prstDash"_ostr,
                "val"_ostr, u"dashDot"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf151492)
{
    createSdImpressDoc("odp/tdf151492.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn"_ostr, "idx"_ostr,
                u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf149697)
{
    createSdImpressDoc("pptx/tdf149697.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:cxnSp[1]/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn"_ostr,
                "idx"_ostr, u"5"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:cxnSp[1]/p:nvCxnSpPr/p:cNvCxnSpPr/a:endCxn"_ostr,
                "idx"_ostr, u"4"_ustr);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn"_ostr,
                "idx"_ostr, u"3"_ustr);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:nvCxnSpPr/p:cNvCxnSpPr/a:endCxn"_ostr,
                "idx"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf149126)
{
    createSdImpressDoc("odp/tdf149126.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr, "prst"_ostr,
                u"triangle"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf131905)
{
    createSdImpressDoc("pptx/tdf131905.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[1]/a:tc/a:tcPr"_ostr,
        "anchor"_ostr, u"t"_ustr);

    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[2]/a:tc/a:tcPr"_ostr,
        "anchor"_ostr, u"ctr"_ustr);

    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[3]/a:tc/a:tcPr"_ostr,
        "anchor"_ostr, u"b"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf93883)
{
    createSdImpressDoc("odp/tdf93883.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xPropSet->getPropertyValue(u"NumberingLevel"_ustr).hasValue());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testBnc822341)
{
    // Check import / export of embedded text document
    createSdImpressDoc("odp/bnc822341.odp");
    save(u"Impress Office Open XML"_ustr);

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/"
                    "vnd.openxmlformats-officedocument.wordprocessingml.document']"_ostr,
                    "PartName"_ostr, u"/ppt/embeddings/oleObject1.docx"_ustr);

        xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
        assertXPath(
            pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']"_ostr,
            "Type"_ostr,
            u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package"_ustr);

        xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
        assertXPath(pXmlDocContent,
                    "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj"_ostr,
                    "progId"_ostr, u"Word.Document.12"_ustr);

        const SdrPage* pPage = GetPage(1);

        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
    }

    save(u"Impress Office Open XML"_ustr);

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/"
                    "vnd.openxmlformats-officedocument.wordprocessingml.document']"_ostr,
                    "PartName"_ostr, u"/ppt/embeddings/oleObject1.docx"_ustr);

        xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
        assertXPath(
            pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']"_ostr,
            "Type"_ostr,
            u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package"_ustr);

        xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
        assertXPath(pXmlDocContent,
                    "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj"_ostr,
                    "progId"_ostr, u"Word.Document.12"_ustr);

        const SdrPage* pPage = GetPage(1);

        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testMathObject)
{
    // Check import / export of math object
    createSdImpressDoc("odp/math.odp");
    save(u"Impress Office Open XML"_ustr);

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
        assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice"_ostr,
                    "Requires"_ostr, u"a14"_ustr);
        assertXPathContent(pXmlDocContent,
                           "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/"
                           "a14:m/m:oMath/m:r[1]/m:t"_ostr,
                           u"a"_ustr);

        const SdrPage* pPage = GetPage(1);
        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
    }

    save(u"Impress Office Open XML"_ustr);

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
        assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice"_ostr,
                    "Requires"_ostr, u"a14"_ustr);
        assertXPathContent(pXmlDocContent,
                           "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/"
                           "a14:m/m:oMath/m:r[1]/m:t"_ostr,
                           u"a"_ustr);

        const SdrPage* pPage = GetPage(1);
        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testMathObjectPPT2010)
{
    // Check import / export of math object
    createSdImpressDoc("pptx/Math.pptx");
    save(u"Impress Office Open XML"_ustr);

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
        assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice"_ostr,
                    "Requires"_ostr, u"a14"_ustr);
        assertXPathContent(pXmlDocContent,
                           "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/"
                           "a14:m/m:oMath/m:sSup/m:e/m:r[1]/m:t"_ostr,
                           u"\U0001D44E"_ustr); // non-BMP char

        const SdrPage* pPage = GetPage(1);
        const SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf119015)
{
    createSdImpressDoc("pptx/tdf119015.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    // The position was previously not properly initialized: (0, 0, 100, 100)
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(6991, 6902), Size(14099, 2000)),
                         pTableObj->GetLogicRect());
    uno::Reference<table::XTable> xTable(pTableObj->getTable());

    // Test that we actually have three cells: this threw css.lang.IndexOutOfBoundsException
    uno::Reference<text::XTextRange> xTextRange(xTable->getCellByPosition(1, 0),
                                                uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"A3"_ustr, xTextRange->getString());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf123090)
{
    createSdImpressDoc("pptx/tdf123090.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XTable> xTable(pTableObj->getTable());

    // Test that we actually have two cells: this threw css.lang.IndexOutOfBoundsException
    uno::Reference<text::XTextRange> xTextRange(xTable->getCellByPosition(1, 0),
                                                uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"aaa"_ustr, xTextRange->getString());

    sal_Int32 nWidth;
    uno::Reference<css::table::XTableColumns> xColumns(xTable->getColumns(), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xRefColumn(xColumns->getByIndex(1), uno::UNO_QUERY_THROW);
    xRefColumn->getPropertyValue(u"Width"_ustr) >>= nWidth;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9136), nWidth);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf126324)
{
    createSdImpressDoc("pptx/tdf126324.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPage.is());
    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_EQUAL(u"17"_ustr, xText->getString());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf119187)
{
    // load document
    createSdImpressDoc("pptx/tdf119187.pptx");

    //Check For Import and Export Both
    for (sal_uInt32 i = 0; i < 2; i++)
    {
        // get shape properties
        const SdrPage* pPage = GetPage(1);
        CPPUNIT_ASSERT(pPage);
        SdrObject* pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);
        const sdr::properties::BaseProperties& rProperties = pObj->GetProperties();

        // check text vertical alignment
        const SdrTextVertAdjustItem& rSdrTextVertAdjustItem
            = rProperties.GetItem(SDRATTR_TEXT_VERTADJUST);
        const SdrTextVertAdjust eTVA = rSdrTextVertAdjustItem.GetValue();
        CPPUNIT_ASSERT_EQUAL(SDRTEXTVERTADJUST_TOP, eTVA);
        saveAndReload(u"Impress Office Open XML"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf132472)
{
    createSdImpressDoc("pptx/tdf132472.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nColor);

    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xCell));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY);
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nColor;

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:0 G:0 B:0 A:0
    // - Actual  : Color: R:255 G:255 B:255 A:0
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf80224)
{
    createSdImpressDoc("odp/tdf80224.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph->getStart(), uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x6562ac), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf91378)
{
    //Check For Import and Export Both
    createSdImpressDoc("pptx/tdf91378.pptx");
    for (sal_uInt32 i = 0; i < 2; i++)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xProps
            = xDocumentPropertiesSupplier->getDocumentProperties();
        uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(),
                                                     uno::UNO_QUERY);
        OUString propValue;
        xUDProps->getPropertyValue(u"Testing"_ustr) >>= propValue;
        CPPUNIT_ASSERT(propValue.isEmpty());
        saveAndReload(u"Impress Office Open XML"_ustr);
    }
}

static bool checkTransitionOnPage(uno::Reference<drawing::XDrawPagesSupplier> const& xDoc,
                                  sal_Int32 nSlideNumber, sal_Int16 nExpectedTransitionType,
                                  sal_Int16 nExpectedTransitionSubType,
                                  bool bExpectedDirection = true)
{
    sal_Int32 nSlideIndex = nSlideNumber - 1;

    CPPUNIT_ASSERT_MESSAGE("Slide/Page index out of range",
                           nSlideIndex < xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nSlideIndex),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    sal_Int16 nTransitionType = 0;
    xPropSet->getPropertyValue(u"TransitionType"_ustr) >>= nTransitionType;

    if (nExpectedTransitionType != nTransitionType)
    {
        std::cerr << "Transition type: " << nTransitionType << " " << nExpectedTransitionType
                  << std::endl;
        return false;
    }

    sal_Int16 nTransitionSubtype = 0;
    xPropSet->getPropertyValue(u"TransitionSubtype"_ustr) >>= nTransitionSubtype;
    if (nExpectedTransitionSubType != nTransitionSubtype)
    {
        std::cerr << "Transition Subtype: " << nTransitionSubtype << " "
                  << nExpectedTransitionSubType << std::endl;
        return false;
    }

    bool bDirection = false;
    xPropSet->getPropertyValue(u"TransitionDirection"_ustr) >>= bDirection;

    if (bExpectedDirection != bDirection)
    {
        std::cerr << "Transition Direction: " << (bExpectedDirection ? "normal" : "reversed") << " "
                  << (bDirection ? "normal" : "reversed") << std::endl;
        return false;
    }

    return true;
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testExportTransitionsPPTX)
{
    createSdImpressDoc("AllTransitions.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    // WIPE TRANSITIONS
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 01, TransitionType::BARWIPE,
                                         TransitionSubType::TOPTOBOTTOM, false));
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 02, TransitionType::BARWIPE, TransitionSubType::LEFTTORIGHT));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 03, TransitionType::BARWIPE,
                                         TransitionSubType::LEFTTORIGHT, false));
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 04, TransitionType::BARWIPE, TransitionSubType::TOPTOBOTTOM));

    // CUT THROUGH BLACK
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 31, TransitionType::BARWIPE, TransitionSubType::FADEOVERCOLOR));

    // COMB
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 41, TransitionType::PUSHWIPE,
                                         TransitionSubType::COMBHORIZONTAL));
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 42, TransitionType::PUSHWIPE, TransitionSubType::COMBVERTICAL));

    // OUTSIDE TURNING CUBE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 57, TransitionType::MISCSHAPEWIPE,
                                         TransitionSubType::CORNERSOUT));
    // INSIDE TURNING CUBE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 60, TransitionType::MISCSHAPEWIPE,
                                         TransitionSubType::CORNERSIN));

    // FALL
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 61, TransitionType::MISCSHAPEWIPE,
                                         TransitionSubType::LEFTTORIGHT));

    // VORTEX
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 70, TransitionType::MISCSHAPEWIPE,
                                         TransitionSubType::VERTICAL));

    // RIPPLE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 71, TransitionType::MISCSHAPEWIPE,
                                         TransitionSubType::HORIZONTAL));

    // GLITTER
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 72, TransitionType::MISCSHAPEWIPE, TransitionSubType::DIAMOND));

    // HONEYCOMB
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 73, TransitionType::MISCSHAPEWIPE, TransitionSubType::HEART));

    // NEWSFLASH
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 74, TransitionType::ZOOM, TransitionSubType::ROTATEIN));

    // OVAL VERTICAL - cannot be exported to PPTX so fallback to circle
    //CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 76, TransitionType::ELLIPSEWIPE, TransitionSubType::VERTICAL));
    CPPUNIT_ASSERT(
        checkTransitionOnPage(xDoc, 76, TransitionType::ELLIPSEWIPE, TransitionSubType::CIRCLE));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testPresetShapesExport)
{
    createSdImpressDoc("odp/preset-shapes-export.odp");
    const char* sShapeTypeAndValues[] = {
        "wedgeEllipseCallout",
        "adj1",
        "val 45310",
        "adj2",
        "val 97194",
        "wedgeRoundRectCallout",
        "adj1",
        "val 46694",
        "adj2",
        "val 129726",
        "adj3",
        "val 16667",
        "wedgeRectCallout",
        "adj1",
        "val 40037",
        "adj2",
        "val 111694",
        "smileyFace",
        "adj",
        "val -9282",
        "can",
        "adj",
        "val 50000",
        "frame",
        "adj1",
        "val 10490",
        "donut",
        "adj",
        "val 9601",
        "bevel",
        "adj",
        "val 42592",
        "foldedCorner",
        "adj",
        "val 10750",
        "verticalScroll",
        "adj",
        "val 25000",
        "horizontalScroll",
        "adj",
        "val 25000",
        "cube",
        "adj",
        "val 85129",
        "bracketPair",
        "adj",
        "val 50000",
        "sun",
        "adj",
        "val 12500",
        "bracePair",
        "adj",
        "val 25000",
        "cloudCallout",
        "adj1",
        "val 77611",
        "adj2",
        "val -47819",
        "borderCallout1",
        "adj1",
        "val 18750",
        "adj2",
        "val -8333",
        "adj3",
        "val 170013",
        "adj4",
        "val 143972",
        "borderCallout2",
        "adj1",
        "val 18750",
        "adj2",
        "val -8333",
        "adj3",
        "val 113768",
        "adj4",
        "val -81930",
        "adj5",
        "val -22375",
        "adj6",
        "val -134550",
        "blockArc",
        "adj1",
        "val 12975429",
        "adj2",
        "val 19424571",
        "adj3",
        "val 3770",
    };

    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocCT = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OStringLiteral sT("_T_");
    static constexpr OStringLiteral sN("_N_");
    static constexpr OStringLiteral sPropertyName("name");
    static constexpr OStringLiteral sPropertyFmla("fmla");

    size_t i = 0;
    while (i < SAL_N_ELEMENTS(sShapeTypeAndValues))
    {
        OString sType(sShapeTypeAndValues[i++]);
        for (size_t j = 1; i < SAL_N_ELEMENTS(sShapeTypeAndValues)
                           && o3tl::starts_with(sShapeTypeAndValues[i], "adj");
             ++j)
        {
            OString sXPath
                = "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom[@prst='_T_']/a:avLst/a:gd[_N_]"_ostr
                      .replaceFirst(sT, sType)
                      .replaceFirst(sN, OString::number(j));
            assertXPath(pXmlDocCT, sXPath, sPropertyName,
                        OUString::createFromAscii(sShapeTypeAndValues[i++]));
            assertXPath(pXmlDocCT, sXPath, sPropertyFmla,
                        OUString::createFromAscii(sShapeTypeAndValues[i++]));
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf92527)
{
    // We draw a diamond in an empty document.
    // If custom shape has name and preset information in OOXML, should be export as preset shape.
    createSdImpressDoc("empty.fodp");
    uno::Reference<drawing::XDrawPage> xPage1(getPage(0));
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape1(
        xFactory->createInstance(u"com.sun.star.drawing.CustomShape"_ustr), uno::UNO_QUERY);
    xPage1->add(xShape1);
    xShape1->setSize(awt::Size(10000, 10000));
    xShape1->setPosition(awt::Point(1000, 1000));
    uno::Sequence<beans::PropertyValue> aShapeGeometry(comphelper::InitPropertySequence({
        { "Type", uno::Any(u"diamond"_ustr) },
    }));
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);
    xPropertySet1->setPropertyValue(u"CustomShapeGeometry"_ustr, uno::Any(aShapeGeometry));

    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc2(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage2(xDoc2->getDrawPages()->getByIndex(0),
                                              uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xPage2->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet2(xShape2, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProps;
    xPropertySet2->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aProps;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "Coordinates")
            aCoordinates
                = rProp.Value.get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>();
    }

    // 4 coordinate pairs
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aCoordinates.getLength());
}

namespace
{
void matchNumberFormat(int nPage, uno::Reference<text::XTextField> const& xField)
{
    uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);
    sal_Int32 nNumFmt;
    xPropSet->getPropertyValue(u"NumberFormat"_ustr) >>= nNumFmt;
    switch (nPage)
    {
        case 0: // 13/02/96 (StdSmall)
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match",
                                         sal_Int32(SvxDateFormat::StdSmall), nNumFmt);
            break;
        case 1: // 13/02/1996
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match",
                                         sal_Int32(SvxDateFormat::B), nNumFmt);
            break;
        case 2: // Tuesday, June 29, 2021 (StdBig)
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match",
                                         sal_Int32(SvxDateFormat::StdBig), nNumFmt);
            break;
        case 3: // 13:49:38 (Standard)
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match",
                                         sal_Int32(SvxTimeFormat::Standard), nNumFmt);
            break;
        case 4: // 13:49
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match",
                                         sal_Int32(SvxTimeFormat::HH24_MM), nNumFmt);
            break;
        case 5: // 01:49 PM
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match",
                                         sal_Int32(SvxTimeFormat::HH12_MM), nNumFmt);
            break;
        case 6: // 01:49:38 PM
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match",
                                         sal_Int32(SvxTimeFormat::HH12_MM_SS), nNumFmt);
            break;
        case 7: // June 29, 2021
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match",
                                         sal_Int32(SvxDateFormat::D), nNumFmt);
            break;
        case 8: // Jun 29, 2021
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match",
                                         sal_Int32(SvxDateFormat::C), nNumFmt);
            break;
    }
}
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testDatetimeFieldNumberFormat)
{
    createSdImpressDoc("odp/numfmt.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    for (sal_uInt16 i = 0; i <= 8; ++i)
    {
        matchNumberFormat(i, getTextFieldFromPage(0, 0, 0, i));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testDatetimeFieldNumberFormatPPTX)
{
    createSdImpressDoc("pptx/numfmt.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);

    for (sal_uInt16 i = 0; i <= 8; ++i)
    {
        matchNumberFormat(i, getTextFieldFromPage(0, 0, 0, i));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testSlideNumberField)
{
    createSdImpressDoc("odp/slidenum_field.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testSlideNumberFieldPPTX)
{
    createSdImpressDoc("pptx/slidenum_field.pptx");

    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testSlideCountField)
{
    createSdImpressDoc("odp/slidecount_field.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testSlideNameField)
{
    createSdImpressDoc("odp/slidename_field.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testExtFileField)
{
    createSdImpressDoc("odp/extfile_field.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    for (sal_uInt16 i = 0; i <= 3; ++i)
    {
        uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, i, 0);
        CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());

        uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);
        sal_Int32 nNumFmt;
        xPropSet->getPropertyValue(u"FileFormat"_ustr) >>= nNumFmt;
        switch (i)
        {
            case 0: // Path/File name
                CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(0), nNumFmt);
                break;
            case 1: // Path
                CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(1), nNumFmt);
                break;
            case 2: // File name without extension
                CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(2), nNumFmt);
                break;
            case 3: // File name with extension
                CPPUNIT_ASSERT_EQUAL_MESSAGE("File formats don't match", sal_Int32(3), nNumFmt);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testAuthorField)
{
    createSdImpressDoc("odp/author_field.odp");

    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf99224)
{
    createSdImpressDoc("odp/tdf99224.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    // This was 0: the image with text was lost on export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf92076)
{
    createSdImpressDoc("odp/tdf92076.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf59046)
{
    createSdImpressDoc("odp/tdf59046.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf133502)
{
    createSdImpressDoc("odp/tdf133502.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/comments/comment1.xml"_ustr);

    assertXPathContent(pXmlDocRels, "/p:cmLst/p:cm/p:text"_ostr, u"Test for creator-initials"_ustr);

    // Without the fix in place, the comment position would have been 0,0
    assertXPath(pXmlDocRels, "/p:cmLst/p:cm/p:pos"_ostr, "x"_ostr, u"2032"_ustr);
    assertXPath(pXmlDocRels, "/p:cmLst/p:cm/p:pos"_ostr, "y"_ostr, u"1029"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf105739)
{
    // Gradient was lost during saving to ODP
    createSdImpressDoc("pptx/tdf105739.pptx");
    save(u"impress8"_ustr);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    uno::Any aAny = xPropSet->getPropertyValue(u"Background"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    if (aAny.hasValue())
    {
        uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
        aAny >>= aXBackgroundPropSet;

        // Test fill type
        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        aXBackgroundPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(aFillStyle));

        // Test gradient properties
        com::sun::star::awt::Gradient2 aFillGradient;
        aXBackgroundPropSet->getPropertyValue(u"FillGradient"_ustr) >>= aFillGradient;

        // MCGR: Use the completely imported gradient to check for correctness
        const basegfx::BColorStops aColorStops
            = model::gradient::getColorStopsFromUno(aFillGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0x00b050), Color(aColorStops[1].getStopColor()));
        CPPUNIT_ASSERT_EQUAL(int(awt::GradientStyle_LINEAR), static_cast<int>(aFillGradient.Style));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testPageBitmapWithTransparency)
{
    createSdImpressDoc("pptx/page_transparent_bitmap.pptx");

    saveAndReload(u"impress8"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly one page", static_cast<sal_Int32>(1),
                                 xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    uno::Any aAny = xPropSet->getPropertyValue(u"Background"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());

    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;
    sal_Int32 nTransparence;
    aAny = aXBackgroundPropSet->getPropertyValue(u"FillTransparence"_ustr);
    aAny >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide background transparency is wrong", sal_Int32(49),
                                 nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testPptmContentType)
{
    createSdImpressDoc("pptm/macro.pptm");
    save(u"Impress MS PowerPoint 2007 XML VBA"_ustr);

    // Assert that the content type is the one of PPTM
    xmlDocUniquePtr pXmlContentType = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']"_ostr,
                "ContentType"_ostr,
                u"application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf111798)
{
    createSdImpressDoc("odp/tdf111798.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString data[][11]
        = { { u"2700000"_ustr, u"2458080"_ustr, u"2414880"_ustr, u"1439640"_ustr, u"1440000"_ustr,
              u"gd[1]"_ustr, u"adj1"_ustr, u"val 50000"_ustr, u"gd[2]"_ustr, u"adj2"_ustr,
              u"val 25000"_ustr },
            { u"2700000"_ustr, u"6778080"_ustr, u"2414880"_ustr, u"1439640"_ustr, u"1440000"_ustr,
              u"gd[1]"_ustr, u"adj1"_ustr, u"val 50000"_ustr, u"gd[2]"_ustr, u"adj2"_ustr,
              u"val 25006"_ustr } };

    for (size_t nShapeIndex = 0; nShapeIndex < SAL_N_ELEMENTS(data); nShapeIndex++)
    {
        size_t nDataIndex = 0;

        const OString sSpPr
            = "/p:sld/p:cSld/p:spTree/p:sp[" + OString::number(nShapeIndex + 1) + "]/p:spPr";
        const OString sXfrm = sSpPr + "/a:xfrm";
        assertXPath(pXmlDoc, sXfrm, "rot"_ostr, data[nShapeIndex][nDataIndex++]);
        const OString sOff = sXfrm + "/a:off";
        assertXPath(pXmlDoc, sOff, "x"_ostr, data[nShapeIndex][nDataIndex++]);
        assertXPath(pXmlDoc, sOff, "y"_ostr, data[nShapeIndex][nDataIndex++]);
        const OString sExt = sXfrm + "/a:ext";
        assertXPath(pXmlDoc, sExt, "cx"_ostr, data[nShapeIndex][nDataIndex++]);
        assertXPath(pXmlDoc, sExt, "cy"_ostr, data[nShapeIndex][nDataIndex++]);

        while (nDataIndex < SAL_N_ELEMENTS(data[nShapeIndex]))
        {
            const OString sGd
                = sSpPr + "/a:prstGeom/a:avLst/a:" + data[nShapeIndex][nDataIndex++].toUtf8() + "";
            assertXPath(pXmlDoc, sGd, "name"_ostr, data[nShapeIndex][nDataIndex++]);
            assertXPath(pXmlDoc, sGd, "fmla"_ostr, data[nShapeIndex][nDataIndex++]);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testPptmVBAStream)
{
    createSdImpressDoc("pptm/macro.pptm");
    save(u"Impress MS PowerPoint 2007 XML VBA"_ustr);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // This failed: VBA stream was not roundtripped
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"ppt/vbaProject.bin"_ustr));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf111863)
{
    createSdImpressDoc("pptx/tdf111863.pptx");
    save(u"Impress Office Open XML"_ustr);

    // check that transition attribute didn't change from 'out' to 'in'
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
        "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animEffect"_ostr,
        "transition"_ostr, u"out"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf111518)
{
    createSdImpressDoc("pptx/tdf111518.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString sActual = getXPath(pXmlDocRels,
                                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                                "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/"
                                "p:par/p:cTn/p:childTnLst/p:animMotion"_ostr,
                                "path"_ostr);
    CPPUNIT_ASSERT_MOTIONPATH(u"M -3.54167E-6 -4.81481E-6 L 0.39037 -0.00069 E", sActual);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf100387)
{
    createSdImpressDoc("odp/tdf100387.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn"_ostr,
                "dur"_ostr, u"indefinite"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]/p:cTn"_ostr,
        "fill"_ostr, u"hold"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                "p:par[1]/p:cTn/p:childTnLst/p:par/p:cTn"_ostr,
                "fill"_ostr, u"hold"_ustr);

    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]"
                "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg"_ostr,
                "st"_ostr, u"0"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[1]"
                "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg"_ostr,
                "end"_ostr, u"0"_ustr);

    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]"
                "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg"_ostr,
                "st"_ostr, u"1"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[2]"
                "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg"_ostr,
                "end"_ostr, u"1"_ustr);

    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[3]"
                "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg"_ostr,
                "st"_ostr, u"2"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par[3]"
                "/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt/p:txEl/p:pRg"_ostr,
                "end"_ostr, u"2"_ustr);
}

// tdf#126746 Add support for Line Caps import and export
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testClosingShapesAndLineCaps)
{
    createSdImpressDoc("odp/closed-shapes.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo/a:pt"_ostr, 1);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt"_ostr, 1);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[2]/a:pt"_ostr, 1);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:close"_ostr,
                1);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:ln"_ostr, "cap"_ostr,
                u"rnd"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:ln/a:miter"_ostr, 1);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:close"_ostr,
                0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:ln"_ostr, "cap"_ostr,
                u"rnd"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:ln/a:miter"_ostr, 1);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:custGeom/a:pathLst/a:path/a:close"_ostr,
                0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:ln"_ostr, "cap"_ostr,
                u"rnd"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:ln/a:miter"_ostr, 1);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:custGeom/a:pathLst/a:path/a:close"_ostr,
                0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:ln"_ostr, "cap"_ostr,
                u"sq"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[4]/p:spPr/a:ln/a:round"_ostr, 1);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:custGeom/a:pathLst/a:path/a:close"_ostr,
                0);
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:ln"_ostr,
                           "cap"_ostr); // by default it is "flat" cap style
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:ln/a:bevel"_ostr, 1);

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[6]/p:spPr/a:custGeom/a:pathLst/a:path/a:close"_ostr,
                0);
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[5]/p:spPr/a:ln"_ostr,
                           "cap"_ostr); // by default it is "flat" cap style
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[6]/p:spPr/a:ln/a:round"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testRotateFlip)
{
    createSdImpressDoc("odp/rotate_flip.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString data[][4] = { // flipH flipV     x          y
                                 { u"0"_ustr, u"1"_ustr, u"1170000"_ustr, u"1035720"_ustr },
                                 { u"1"_ustr, u"1"_ustr, u"7108560"_ustr, u"1036440"_ustr },
                                 { u"1"_ustr, u"0"_ustr, u"4140000"_ustr, u"1036440"_ustr }
    };

    const OUString points[][2] = { { u"221"_ustr, u"293"_ustr }, { u"506"_ustr, u"12"_ustr },
                                   { u"367"_ustr, u"0"_ustr },   { u"29"_ustr, u"406"_ustr },
                                   { u"431"_ustr, u"347"_ustr }, { u"145"_ustr, u"645"_ustr },
                                   { u"99"_ustr, u"520"_ustr },  { u"0"_ustr, u"861"_ustr },
                                   { u"326"_ustr, u"765"_ustr }, { u"209"_ustr, u"711"_ustr },
                                   { u"640"_ustr, u"233"_ustr }, { u"640"_ustr, u"233"_ustr } };

    for (size_t nShapeIndex = 0; nShapeIndex < SAL_N_ELEMENTS(data); nShapeIndex++)
    {
        size_t nDataIndex = 0;

        const OString sSpPr
            = "/p:sld/p:cSld/p:spTree/p:sp[" + OString::number(nShapeIndex + 1) + "]/p:spPr";
        const OString sXfrm = sSpPr + "/a:xfrm";
        if (data[nShapeIndex][nDataIndex++] == "1")
            assertXPath(pXmlDocContent, sXfrm, "flipH"_ostr, u"1"_ustr);
        if (data[nShapeIndex][nDataIndex++] == "1")
            assertXPath(pXmlDocContent, sXfrm, "flipV"_ostr, u"1"_ustr);
        assertXPath(pXmlDocContent, sXfrm, "rot"_ostr, u"20400000"_ustr);
        const OString sOff = sXfrm + "/a:off";
        assertXPath(pXmlDocContent, sOff, "x"_ostr, data[nShapeIndex][nDataIndex++]);
        assertXPath(pXmlDocContent, sOff, "y"_ostr, data[nShapeIndex][nDataIndex++]);
        const OString sExt = sXfrm + "/a:ext";
        assertXPath(pXmlDocContent, sExt, "cx"_ostr, u"1800000"_ustr);
        assertXPath(pXmlDocContent, sExt, "cy"_ostr, u"3600000"_ustr);

        for (size_t nPointIndex = 0; nPointIndex < SAL_N_ELEMENTS(points); nPointIndex++)
        {
            const OString sPt = sSpPr + "/a:custGeom/a:pathLst/a:path/a:lnTo["
                                + OString::number(nPointIndex + 1) + "]/a:pt";
            assertXPath(pXmlDocContent, sPt, "x"_ostr, points[nPointIndex][0]);
            assertXPath(pXmlDocContent, sPt, "y"_ostr, points[nPointIndex][1]);
        }
        assertXPath(pXmlDocContent, sSpPr + "/a:custGeom/a:pathLst/a:path/a:close", 1);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf106867)
{
    createSdImpressDoc("pptx/tdf106867.pptx");
    save(u"Impress Office Open XML"_ustr);

    const SdrPage* pPage = GetPage(1);

    // first check that we have the media object
    const SdrMediaObj* pMediaObj = dynamic_cast<SdrMediaObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT_MESSAGE("no media object", pMediaObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(u"vnd.sun.star.Package:ppt/media/media1.avi"_ustr, pMediaObj->getURL());

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // check that the document contains the video stream
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"ppt/media/media1.avi"_ustr));

    // both the ooxml and the extended markup
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile"_ostr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/p:extLst/p:ext/p14:media"_ostr);

    // target the shape with the video in the command
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
                "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:cmd/"
                "p:cBhvr/p:tgtEl/p:spTgt"_ostr,
                "spid"_ostr, u"67"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112280)
{
    createSdImpressDoc("pptx/tdf112280.pptx");
    save(u"Impress Office Open XML"_ustr);

    // check the animRot value
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
        "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animRot"_ostr,
        "by"_ostr, u"21600000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112088)
{
    createSdImpressDoc("pptx/tdf112088.pptx");
    save(u"Impress Office Open XML"_ustr);

    // check gradient stops
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathChildren(pXmlDocContent,
                        "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:gradFill/a:gsLst"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112333)
{
    createSdImpressDoc("pptx/tdf112333.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    OUString sTo = getXPath(pXmlDocContent,
                            "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                            "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/"
                            "p:cTn/p:childTnLst/p:set[1]/p:to/p:strVal"_ostr,
                            "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"solid"_ustr, sTo);

    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:set[1]/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"fill.type"_ustr, sAttributeName);

    sTo = getXPath(pXmlDocContent,
                   "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                   "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/"
                   "p:set[2]/p:to/p:strVal"_ostr,
                   "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"true"_ustr, sTo);

    sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:set[2]/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"fill.on"_ustr, sAttributeName);

    sTo = getXPath(pXmlDocContent,
                   "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                   "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/"
                   "p:animClr/p:to/a:srgbClr"_ostr,
                   "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"0563c1"_ustr, sTo);

    sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:animClr/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"fillcolor"_ustr, sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112552)
{
    // Background fill was not displayed, but it was because of the wrong geometry
    createSdImpressDoc("odp/tdf112552.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path"_ostr, "w"_ostr,
                u"21600"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path"_ostr, "h"_ostr,
                u"21600"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt"_ostr,
        "x"_ostr, u"21600"_ustr);
    assertXPath(
        pXmlDocContent,
        "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt"_ostr,
        "y"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112557)
{
    // Subtitle shape should be skipped by export.
    createSdImpressDoc("odp/tdf112557.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sldMaster/p:cSld/p:spTree/p:sp"_ostr, 2); // title and object
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf128049)
{
    createSdImpressDoc("odp/tdf128049.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:custGeom"_ostr, 0);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:prstGeom"_ostr,
                "prst"_ostr, u"noSmoking"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:prstGeom/a:avLst/a:gd"_ostr, "name"_ostr,
                u"adj"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:prstGeom/a:avLst/a:gd"_ostr, "fmla"_ostr,
                u"val 12500"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf106026)
{
    createSdImpressDoc("odp/tdf106026.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlMasterContent = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"1417"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"1134"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[3]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"850"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[4]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"567"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[5]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"283"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[6]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"283"_ustr);
    assertXPath(pXmlMasterContent,
                "/p:sldMaster/p:cSld/p:spTree/p:sp/p:txBody/a:p[7]/a:pPr/a:spcBef/a:spcPts"_ostr,
                "val"_ostr, u"283"_ustr);

    xmlDocUniquePtr pXmlSlideContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlSlideContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:spcAft/a:spcPts"_ostr,
                "val"_ostr, u"11339"_ustr);
    assertXPath(pXmlSlideContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:spcAft/a:spcPts"_ostr,
                "val"_ostr, u"11339"_ustr);
    assertXPath(pXmlSlideContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:spcAft/a:spcPts"_ostr,
                "val"_ostr, u"11339"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112334)
{
    createSdImpressDoc("pptx/tdf112334.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:animClr[1]/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"style.color"_ustr, sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112089)
{
    createSdImpressDoc("pptx/tdf112089.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    OUString sID = getXPath(pXmlDocContent,
                            "/p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr"_ostr,
                            "id"_ostr);
    OUString sTarget = getXPath(pXmlDocContent,
                                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                                "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/"
                                "p:par/p:cTn/p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt"_ostr,
                                "spid"_ostr);
    CPPUNIT_ASSERT_EQUAL(sID, sTarget);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112086)
{
    createSdImpressDoc("pptx/tdf112086.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    OUString sVal = getXPath(pXmlDocContent,
                             "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                             "p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/"
                             "p:cTn/p:childTnLst/p:anim[2]/p:tavLst/p:tav/p:val/p:fltVal"_ostr,
                             "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, sVal);

    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:anim[1]/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"ppt_w"_ustr, sAttributeName);

    sVal = getXPath(pXmlDocContent,
                    "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                    "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/"
                    "p:anim[2]/p:tavLst/p:tav/p:val/p:fltVal"_ostr,
                    "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, sVal);

    sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:anim[2]/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"ppt_h"_ustr, sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf112647)
{
    createSdImpressDoc("odp/tdf112647.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    css::style::LineSpacing aLineSpacing;
    xPropSet->getPropertyValue(u"ParaLineSpacing"_ustr) >>= aLineSpacing;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(css::style::LineSpacingMode::FIX), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2117), aLineSpacing.Height);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testGroupRotation)
{
    createSdImpressDoc("odp/group_rotation.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:grpSpPr/a:xfrm"_ostr,
                           "rot"_ostr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:xfrm"_ostr,
                "rot"_ostr, u"20400000"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[2]/p:spPr/a:xfrm"_ostr,
                "rot"_ostr, u"20400000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf104788)
{
    createSdImpressDoc("pptx/tdf104788.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide6.xml"_ustr);

    OUString sVal = getXPath(pXmlDocContent,
                             "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/"
                             "p:childTnLst/p:par[2]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/"
                             "p:par/p:cTn/p:childTnLst/p:anim[2]"_ostr,
                             "to"_ostr);
    CPPUNIT_ASSERT_EQUAL(-1.0, sVal.toDouble());

    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par[2]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:anim[2]/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"xshear"_ustr, sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testSmartartRotation2)
{
    createSdImpressDoc("pptx/smartart-rotation2.pptx");

    // clear SmartArt data to check how group shapes with double-rotated children are exported, not smartart
    // NOTE: Resetting the GrabBag data is a *very* indirect way to reset the SmartArt functionality.
    //       Since this worked before and there is not (yet?) a better way to do it using UNO API, I added
    //       code to support this for now
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xShape->setPropertyValue(u"InteropGrabBag"_ustr, uno::Any(aInteropGrabBag));

    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathContent(pXmlDocContent,
                       "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:txBody/a:p/a:r/a:t"_ostr,
                       u"Text"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:txBody/a:bodyPr"_ostr,
                "rot"_ostr, u"10800000"_ustr);
    double dX
        = getXPath(pXmlDocContent,
                   "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:spPr/a:xfrm/a:off"_ostr, "x"_ostr)
              .toDouble();
    double dY
        = getXPath(pXmlDocContent,
                   "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[4]/p:spPr/a:xfrm/a:off"_ostr, "y"_ostr)
              .toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2276280.0, dX, dX * .001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3158280.0, dY, dY * .001);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf91999_rotateShape)
{
    createSdImpressDoc("pptx/tdf91999_rotateShape.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:nvSpPr/p:cNvPr"_ostr, "name"_ostr,
                u"CustomShape 2"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                u"10800000"_ustr);
    double dX = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:xfrm/a:off"_ostr,
                         "x"_ostr)
                    .toDouble();
    double dY = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:xfrm/a:off"_ostr,
                         "y"_ostr)
                    .toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2960640.0, dX, dX * .001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1449000.0, dY, dY * .001);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf114845_rotateShape)
{
    createSdImpressDoc("pptx/tdf114845_rotateShape.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:nvCxnSpPr/p:cNvPr"_ostr,
                "name"_ostr, u"Straight Arrow Connector 9"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:xfrm"_ostr,
                "flipV"_ostr, u"1"_ustr);
    double dX = getXPath(pXmlDocContent,
                         "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:xfrm/a:off"_ostr, "x"_ostr)
                    .toDouble();
    double dY = getXPath(pXmlDocContent,
                         "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:xfrm/a:off"_ostr, "y"_ostr)
                    .toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4059000.0, dX, dX * .001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3287520.0, dY, dY * .001);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testGroupsPosition)
{
    createSdImpressDoc("pptx/group.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[1]/p:spPr/a:xfrm/a:off"_ostr, "x"_ostr,
                u"5004000"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[1]/p:spPr/a:xfrm/a:off"_ostr, "y"_ostr,
                u"3310560"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[3]/p:spPr/a:xfrm/a:off"_ostr, "x"_ostr,
                u"7760160"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:grpSp[1]/p:sp[3]/p:spPr/a:xfrm/a:off"_ostr, "y"_ostr,
                u"3310560"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testGroupsRotatedPosition)
{
    createSdImpressDoc("pptx/group-rot.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off"_ostr,
                "x"_ostr, u"2857320"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[3]/p:spPr/a:xfrm/a:off"_ostr,
                "y"_ostr, u"4026960"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testAccentColor)
{
    createSdImpressDoc("pptx/accent-color.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:style/a:fillRef/a:schemeClr"_ostr,
                "val"_ostr, u"accent6"_ustr);
    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp/p:style/a:fillRef/a:schemeClr"_ostr,
                "val"_ostr, u"accent6"_ustr);
    xmlDocUniquePtr pXmlDocTheme1 = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:accent6/a:srgbClr"_ostr,
                "val"_ostr, u"70ad47"_ustr);
    xmlDocUniquePtr pXmlDocTheme2 = parseExport(u"ppt/theme/theme12.xml"_ustr);
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:accent6/a:srgbClr"_ostr,
                "val"_ostr, u"deb340"_ustr);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Motyw pakietu Office
    // - Actual  : Office Theme
    // i.e. the theme and color scheme name was lost on export.
    assertXPath(pXmlDocTheme1, "/a:theme"_ostr, "name"_ostr, u"Motyw pakietu Office"_ustr);
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme"_ostr, "name"_ostr,
                u"Pakiet Office"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testThemeColors)
{
    createSdImpressDoc("pptx/tdf84205.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocTheme2 = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr"_ostr,
                "val"_ostr, u"44546a"_ustr);
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:accent3/a:srgbClr"_ostr,
                "val"_ostr, u"a5a5a5"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf111785)
{
    createSdImpressDoc("odp/tdf111785.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: ed1c24
    // - Actual  : ffffff
    assertXPath(pXmlDocRels,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:pattFill/a:bgClr/a:srgbClr"_ostr,
                "val"_ostr, u"ed1c24"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf118825)
{
    createSdImpressDoc("odp/tdf118825-motionpath.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    CPPUNIT_ASSERT_MOTIONPATH(
        u"M 0.0449285714285714 0.00368253968253968 C 0.0575714285714285 -0.00095238095238096 "
        u"0.0704264795523803 -0.00370117418637049 0.0831071428571428 -0.00819047619047622 C "
        u"0.0953550597998766 -0.0125265741339082 0.107821870086751 -0.010397536991717 "
        u"0.120321428571429 -0.0115555555555556 C 0.133179018681433 -0.0127467438724762 "
        u"0.151318627483861 -0.0158700272533852 0.1585 0.00539682539682542 C 0.16478291361998 "
        u"0.0240029898688431 0.15828642886492 0.0483806254341085 0.161392857142857 "
        u"0.0698412698412698 C 0.165179286017685 0.0959996731216037 0.17453898927982 "
        u"0.119735912694626 0.187142857142857 0.132634920634921 C 0.199788991845377 "
        u"0.145577185161529 0.215607110490848 0.142889773028431 0.230107142857143 "
        u"0.142857142857143 C 0.243821417584191 0.142826280916829 0.257716514999779 "
        u"0.142685979556724 0.271142857142857 0.137777777777778 C 0.286895094567923 "
        u"0.132019309914514 0.302318190711873 0.122962218306185 0.317928571428571 0.11568253968254 "
        u"C 0.333496771884548 0.108422531222479 0.348787823719556 0.0990570571890929 "
        u"0.363714285714286 0.0885079365079364 C 0.374930683062651 0.080580865157908 "
        u"0.385357142857143 0.0693333333333332 0.396178571428571 0.0596825396825396 L "
        u"0.404785714285714 0.0410158730158729 L 0.401892857142857 0.0342222222222221 E",
        getXPath(pXmlDocContent, "(//p:animMotion)[1]"_ostr, "path"_ostr));
    CPPUNIT_ASSERT_MOTIONPATH(u"M 0.025 0.0571428571428571 L 0.0821428571428571 0.184126984126984 "
                              u"L -0.175 0.234920634920635 L -0.246428571428571 "
                              u"-0.0190476190476191 L -0.0821428571428573 -0.133333333333333 E",
                              getXPath(pXmlDocContent, "(//p:animMotion)[2]"_ostr, "path"_ostr));
    CPPUNIT_ASSERT_MOTIONPATH(
        u"M -0.0107142857142857 0.00634920634920635 C -0.110714285714286 0.501587301587301 "
        u"-0.153571428571429 -0.00634920634920635 -0.246428571428572 0.184126984126984 C "
        u"-0.339285714285715 0.374603174603175 -0.296428571428572 0.514285714285714 "
        u"-0.267857142857143 0.603174603174603 C -0.239285714285715 0.692063492063492 "
        u"0.0607142857142858 0.590476190476191 0.0607142857142858 0.590476190476191 E",
        getXPath(pXmlDocContent, "(//p:animMotion)[3]"_ostr, "path"_ostr));
    CPPUNIT_ASSERT_MOTIONPATH(u"M 0.0535714285714286 -0.0444444444444444 L 0.132142857142857 "
                              u"-0.0444444444444444 L 0.132142857142857 -0.146031746031746 L "
                              u"0.0964285714285715 -0.146031746031746 E",
                              getXPath(pXmlDocContent, "(//p:animMotion)[4]"_ostr, "path"_ostr));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTextColumns_tdf140852)
{
    // The document defines two columns in slideLayout12.xml, but explicitly redefines
    // in slide1.xml. Here we check that the redefinition in the slide takes precedence.

    createSdImpressDoc("pptx/tdf140852.pptx");
    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        CPPUNIT_ASSERT_EQUAL(u"Training will be treated as part of sharing the sweet when "
                             "it comes to serving ice cream"_ustr,
                             xParagraph->getString());
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue(u"TextColumns"_ustr),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));
    }
    save(u"Impress Office Open XML"_ustr);

    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        CPPUNIT_ASSERT_EQUAL(u"Training will be treated as part of sharing the sweet when "
                             "it comes to serving ice cream"_ustr,
                             xParagraph->getString());
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue(u"TextColumns"_ustr),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));
    }

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr"_ostr, "numCol"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr"_ostr, "spcCol"_ostr,
                u"360000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTextColumns_3columns)
{
    createSdImpressDoc("pptx/3columns.pptx");
    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue(u"TextColumns"_ustr),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(300)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));
        // Scale value may be unstable; just test that the text is actually scaled
        double fFontScale;
        CPPUNIT_ASSERT(xProps->getPropertyValue(u"TextFitToSizeFontScale"_ustr) >>= fFontScale);
        CPPUNIT_ASSERT_GREATER(0.0, fFontScale);
        CPPUNIT_ASSERT_LESS(100.0, fFontScale);
    }

    save(u"Impress Office Open XML"_ustr);

    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue(u"TextColumns"_ustr),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(300)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));
        // Scale value may be unstable; just test that the text is actually scaled
        double fFontScale;
        CPPUNIT_ASSERT(xProps->getPropertyValue(u"TextFitToSizeFontScale"_ustr) >>= fFontScale);
        CPPUNIT_ASSERT_GREATER(0.0, fFontScale);
        CPPUNIT_ASSERT_LESS(100.0, fFontScale);
    }

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr"_ostr, "numCol"_ostr,
                u"3"_ustr);
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr"_ostr, "spcCol"_ostr,
                u"108000"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf59323_slideFooters)
{
    createSdImpressDoc("pptx/tdf59323.pptx");

    save(u"Impress Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDoc->getDrawPages()->getCount());

    for (int nPageIndex = 0; nPageIndex < 3; nPageIndex++)
    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, xPropSet->getPropertyValue(u"IsFooterVisible"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(true,
                             xPropSet->getPropertyValue(u"IsDateTimeVisible"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(true,
                             xPropSet->getPropertyValue(u"IsPageNumberVisible"_ustr).get<bool>());
    }

    // Test placeholder indexes
    xmlDocUniquePtr pXmlDocMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDocMaster, "//p:ph [@type='dt']"_ostr, "idx"_ostr, u"1"_ustr);
    assertXPath(pXmlDocMaster, "//p:ph [@type='ftr']"_ostr, "idx"_ostr, u"2"_ustr);
    assertXPath(pXmlDocMaster, "//p:ph [@type='sldNum']"_ostr, "idx"_ostr, u"3"_ustr);

    xmlDocUniquePtr pXmlDocSlide1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocSlide1, "//p:ph [@type='dt']"_ostr, "idx"_ostr, u"1"_ustr);
    assertXPath(pXmlDocSlide1, "//p:ph [@type='ftr']"_ostr, "idx"_ostr, u"2"_ustr);
    assertXPath(pXmlDocSlide1, "//p:ph [@type='sldNum']"_ostr, "idx"_ostr, u"3"_ustr);

    // Test if datetime fields have text in them
    // This is needed for backwards compatibility
    assertXPath(pXmlDocSlide1, "//a:fld [@type='datetime1']/a:t"_ostr);

    // tdf#143316: Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlDocSlide1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:fld/a:rPr"_ostr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest2, testTdf53970)
{
    // Embedded media file
    {
        createSdImpressDoc("odp/tdf53970.odp");
        saveAndReload(u"Impress Office Open XML"_ustr);

        // Without fix in place, the media shape was lost on export.
        CPPUNIT_ASSERT(getPage(0)->hasElements());
    }

    // Linked media file
    {
        createSdImpressDoc("odp/tdf53970_linked.odp");
        save(u"Impress Office Open XML"_ustr);

        xmlDocUniquePtr pXmlRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
        CPPUNIT_ASSERT(pXmlRels);
        assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@TargetMode='External']"_ostr,
                    2);

        uno::Reference<beans::XPropertySet> xShape(getShape(0, getPage(0)));
        CPPUNIT_ASSERT(xShape.is());
        OUString sVideoURL;

        // Without fix in place, the media shape was imported as an image after export
        // and this test would have failed with exception of type com.sun.star.beans.UnknownPropertyException
        CPPUNIT_ASSERT_MESSAGE("MediaURL property is not set",
                               xShape->getPropertyValue(u"MediaURL"_ustr) >>= sVideoURL);
        CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sVideoURL.isEmpty());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
