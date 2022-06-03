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

namespace {

bool checkBeginWithNumber(const OUString& rStr)
{
    sal_Unicode aChar = (rStr.getLength() > 1) ? rStr[0] : '\0';
    return aChar == '.' || aChar == '-' || rtl::isAsciiDigit(aChar);
}

}

#define CPPUNIT_ASSERT_MOTIONPATH(expect, actual) \
      assertMotionPath(expect, actual, CPPUNIT_SOURCELINE())

static void assertMotionPath(std::u16string_view rStr1, std::u16string_view rStr2, const CppUnit::SourceLine &rSourceLine)
{
    sal_Int32 nIdx1 = 0;
    sal_Int32 nIdx2 = 0;

    OString sMessage = "Motion path values mismatch.\nExpect: " +
            OUStringToOString(rStr1, RTL_TEXTENCODING_UTF8) +
            "\nActual: " + OUStringToOString(rStr2, RTL_TEXTENCODING_UTF8);

    while(nIdx1 != -1 && nIdx2 != -1)
    {
        OUString aToken1( o3tl::getToken(rStr1, 0, ' ', nIdx1) );
        OUString aToken2( o3tl::getToken(rStr2, 0, ' ', nIdx2) );

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
    void testTdf149126();
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
    void testTdf111785();
    void testTdf118825();
    void testTextColumns_tdf140852();
    void testTextColumns_3columns();
    void testTdf59323_slideFooters();
    void testTdf53970();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest2);

    CPPUNIT_TEST(testTdf149126);
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
    CPPUNIT_TEST(testTdf111785);
    CPPUNIT_TEST(testTdf118825);
    CPPUNIT_TEST(testTextColumns_tdf140852);
    CPPUNIT_TEST(testTextColumns_3columns);
    CPPUNIT_TEST(testTdf59323_slideFooters);
    CPPUNIT_TEST(testTdf53970);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    }
};

void SdOOXMLExportTest2::testTdf149126()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf149126.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom", "prst",
                "triangle");

    xDocShRef->DoClose();
}

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
        CPPUNIT_ASSERT_EQUAL( SdrObjKind::OLE2, pObj->GetObjIdentifier() );
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
        CPPUNIT_ASSERT_EQUAL( SdrObjKind::OLE2, pObj->GetObjIdentifier() );
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
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
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
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
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
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
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

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x6562ac), nCharColor);
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
        "adj","val 42592",
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
        "adj1","val 12975429",
        "adj2","val 19424571",
        "adj3","val 3770",
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
        for ( size_t j = 1 ; i < SAL_N_ELEMENTS( sShapeTypeAndValues ) && o3tl::starts_with(sShapeTypeAndValues[i], "adj") ; ++j ) {
            OString sXPath= sPattern.replaceFirst( sT, sType).replaceFirst( sN, OString::number(j) );
            assertXPath(pXmlDocCT, sXPath, sPropertyName , OUString::createFromAscii(sShapeTypeAndValues[ i++ ]) );
            assertXPath(pXmlDocCT, sXPath, sPropertyFmla , OUString::createFromAscii(sShapeTypeAndValues[ i++ ]) );
        }
    }
}

void SdOOXMLExportTest2::testTdf92527()
{
    // We draw a diamond in an empty document.
    // If custom shape has name and preset information in OOXML, should be export as preset shape.
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
            {"Type", uno::Any(OUString("diamond"))},
        }));
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);
    xPropertySet1->setPropertyValue("CustomShapeGeometry", uno::Any(aShapeGeometry));

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

    // 4 coordinate pairs
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aCoordinates.getLength());
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
        case 0:     // 13/02/96 (StdSmall)
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(SvxDateFormat::StdSmall), nNumFmt);
                    break;
        case 1:     // 13/02/1996
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(SvxDateFormat::B), nNumFmt);
                    break;
        case 2:     // Tuesday, June 29, 2021 (StdBig)
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(SvxDateFormat::StdBig), nNumFmt);
                    break;
        case 3:     // 13:49:38 (Standard)
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(SvxTimeFormat::Standard), nNumFmt);
                    break;
        case 4:     // 13:49
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(SvxTimeFormat::HH24_MM), nNumFmt);
                    break;
        case 5:     // 01:49 PM
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(SvxTimeFormat::HH12_MM), nNumFmt);
                    break;
        case 6:     // 01:49:38 PM
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Time fields don't match", sal_Int32(SvxTimeFormat::HH12_MM_SS), nNumFmt);
                    break;
        case 7:     // June 29, 2021
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(SvxDateFormat::D), nNumFmt);
                    break;
        case 8:     // Jun 29, 2021
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number formats of Date fields don't match", sal_Int32(SvxDateFormat::C), nNumFmt);
                    break;
    }
}

}

void SdOOXMLExportTest2::testDatetimeFieldNumberFormat()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/numfmt.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 8; ++i)
    {
        matchNumberFormat( i, getTextFieldFromPage(0, 0, 0, i, xDocShRef) );
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testDatetimeFieldNumberFormatPPTX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/numfmt.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef.get(), PPTX );

    for(sal_uInt16 i = 0; i <= 8; ++i)
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

    const OUString data[][11] =
    {
        {
            "2700000", "2458080", "2414880", "1439640", "1440000",
            "gd[1]", "adj1", "val 50000",
            "gd[2]", "adj2", "val 25000"
        },
        {
            "2700000", "6778080", "2414880", "1439640", "1440000",
            "gd[1]", "adj1", "val 50000",
            "gd[2]", "adj2", "val 25006"
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
            const OString sGd = sSpPr + "/a:prstGeom/a:avLst/a:" + data[nShapeIndex][nDataIndex++].toUtf8() + "";
            assertXPath(pXmlDoc, sGd, "name", data[nShapeIndex][nDataIndex++]);
            assertXPath(pXmlDoc, sGd, "fmla", data[nShapeIndex][nDataIndex++]);
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
    CPPUNIT_ASSERT_MOTIONPATH(u"M -3.54167E-6 -4.81481E-6 L 0.39037 -0.00069 E", sActual);
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
    // NOTE: Resetting the GrabBag data is a *very* indirect way to reset the SmartArt functionality.
    //       Since this worked before and there is not (yet?) a better way to do it using UNO API, I added
    //       code to support this for now
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    xShape->setPropertyValue("InteropGrabBag", uno::Any(aInteropGrabBag));

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
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:nvCxnSpPr/p:cNvPr", "name", "Straight Arrow Connector 9");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:xfrm", "flipV", "1");
    double dX = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:xfrm/a:off", "x").toDouble();
    double dY = getXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:cxnSp[2]/p:spPr/a:xfrm/a:off", "y").toDouble();
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

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Motyw pakietu Office
    // - Actual  : Office Theme
    // i.e. the theme and color scheme name was lost on export.
    assertXPath(pXmlDocTheme1, "/a:theme", "name", "Motyw pakietu Office");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme", "name", "Pakiet Office");
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

void SdOOXMLExportTest2::testTdf111785()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf111785.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");

    // Without the fix in place, this test would have failed with
    // - Expected: ed1c24
    // - Actual  : ffffff
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:pattFill/a:bgClr/a:srgbClr", "val", "ed1c24");
}

void SdOOXMLExportTest2::testTdf118825()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118825-motionpath.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");


    CPPUNIT_ASSERT_MOTIONPATH(u"M 0.0449285714285714 0.00368253968253968 C 0.0575714285714285 -0.00095238095238096 0.0704264795523803 -0.00370117418637049 0.0831071428571428 -0.00819047619047622 C 0.0953550597998766 -0.0125265741339082 0.107821870086751 -0.010397536991717 0.120321428571429 -0.0115555555555556 C 0.133179018681433 -0.0127467438724762 0.151318627483861 -0.0158700272533852 0.1585 0.00539682539682542 C 0.16478291361998 0.0240029898688431 0.15828642886492 0.0483806254341085 0.161392857142857 0.0698412698412698 C 0.165179286017685 0.0959996731216037 0.17453898927982 0.119735912694626 0.187142857142857 0.132634920634921 C 0.199788991845377 0.145577185161529 0.215607110490848 0.142889773028431 0.230107142857143 0.142857142857143 C 0.243821417584191 0.142826280916829 0.257716514999779 0.142685979556724 0.271142857142857 0.137777777777778 C 0.286895094567923 0.132019309914514 0.302318190711873 0.122962218306185 0.317928571428571 0.11568253968254 C 0.333496771884548 0.108422531222479 0.348787823719556 0.0990570571890929 0.363714285714286 0.0885079365079364 C 0.374930683062651 0.080580865157908 0.385357142857143 0.0693333333333332 0.396178571428571 0.0596825396825396 L 0.404785714285714 0.0410158730158729 L 0.401892857142857 0.0342222222222221 E", getXPath(pXmlDocContent, "(//p:animMotion)[1]", "path"));
    CPPUNIT_ASSERT_MOTIONPATH(u"M 0.025 0.0571428571428571 L 0.0821428571428571 0.184126984126984 L -0.175 0.234920634920635 L -0.246428571428571 -0.0190476190476191 L -0.0821428571428573 -0.133333333333333 E", getXPath(pXmlDocContent, "(//p:animMotion)[2]", "path"));
    CPPUNIT_ASSERT_MOTIONPATH(u"M -0.0107142857142857 0.00634920634920635 C -0.110714285714286 0.501587301587301 -0.153571428571429 -0.00634920634920635 -0.246428571428572 0.184126984126984 C -0.339285714285715 0.374603174603175 -0.296428571428572 0.514285714285714 -0.267857142857143 0.603174603174603 C -0.239285714285715 0.692063492063492 0.0607142857142858 0.590476190476191 0.0607142857142858 0.590476190476191 E", getXPath(pXmlDocContent, "(//p:animMotion)[3]", "path"));
    CPPUNIT_ASSERT_MOTIONPATH(u"M 0.0535714285714286 -0.0444444444444444 L 0.132142857142857 -0.0444444444444444 L 0.132142857142857 -0.146031746031746 L 0.0964285714285715 -0.146031746031746 E", getXPath(pXmlDocContent, "(//p:animMotion)[4]", "path"));
    xDocShRef->DoClose();
}


void SdOOXMLExportTest2::testTextColumns_tdf140852()
{
    // The document defines two columns in slideLayout12.xml, but explicitly redefines
    // in slide1.xml. Here we check that the redefinition in the slide takes precedence.

    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf140852.pptx"), PPTX);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier = getDoc(xDocShRef);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        CPPUNIT_ASSERT_EQUAL(OUString("Training will be treated as part of sharing the sweet when "
                                      "it comes to serving ice cream"),
                             xParagraph->getString());
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue("AutomaticDistance"));
    }

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier = getDoc(xDocShRef);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        CPPUNIT_ASSERT_EQUAL(OUString("Training will be treated as part of sharing the sweet when "
                                      "it comes to serving ice cream"),
                             xParagraph->getString());
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue("AutomaticDistance"));
    }

    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr", "numCol", "1");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr", "spcCol", "360000");

    tempFile.EnableKillingFile();
}

void SdOOXMLExportTest2::testTextColumns_3columns()
{
    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/3columns.pptx"), PPTX);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier = getDoc(xDocShRef);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(300)),
                             xColProps->getPropertyValue("AutomaticDistance"));
        // Scale value may be unstable; just test that the text is actually scaled
        sal_Int16 nScale;
        CPPUNIT_ASSERT(xProps->getPropertyValue("TextFitToSizeScale") >>= nScale);
        CPPUNIT_ASSERT_GREATER(sal_Int16(0), nScale);
        CPPUNIT_ASSERT_LESS(sal_Int16(100), nScale);
    }

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier = getDoc(xDocShRef);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(300)),
                             xColProps->getPropertyValue("AutomaticDistance"));
        // Scale value may be unstable; just test that the text is actually scaled
        sal_Int16 nScale;
        CPPUNIT_ASSERT(xProps->getPropertyValue("TextFitToSizeScale") >>= nScale);
        CPPUNIT_ASSERT_GREATER(sal_Int16(0), nScale);
        CPPUNIT_ASSERT_LESS(sal_Int16(100), nScale);
    }

    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr", "numCol", "3");
    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr", "spcCol", "108000");

    tempFile.EnableKillingFile();
}

void SdOOXMLExportTest2::testTdf59323_slideFooters()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf59323.pptx"), PPTX);

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDoc->getDrawPages()->getCount());

    for (int nPageIndex = 0; nPageIndex < 3; nPageIndex++)
    {
        uno::Reference<drawing::XDrawPage> xPage(getPage(0, xDocShRef));
        uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, xPropSet->getPropertyValue("IsFooterVisible").get<bool>());
        CPPUNIT_ASSERT_EQUAL(true, xPropSet->getPropertyValue("IsDateTimeVisible").get<bool>());
        CPPUNIT_ASSERT_EQUAL(true, xPropSet->getPropertyValue("IsPageNumberVisible").get<bool>());
    }

    // Test placeholder indexes
    xmlDocUniquePtr pXmlDocMaster = parseExport(tempFile, "ppt/slideMasters/slideMaster1.xml");
    assertXPath(pXmlDocMaster, "//p:ph [@type='dt']", "idx", "1");
    assertXPath(pXmlDocMaster, "//p:ph [@type='ftr']", "idx", "2");
    assertXPath(pXmlDocMaster, "//p:ph [@type='sldNum']", "idx", "3");

    xmlDocUniquePtr pXmlDocSlide1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocSlide1, "//p:ph [@type='dt']", "idx", "1");
    assertXPath(pXmlDocSlide1, "//p:ph [@type='ftr']", "idx", "2");
    assertXPath(pXmlDocSlide1, "//p:ph [@type='sldNum']", "idx", "3");

    // Test if datetime fields have text in them
    // This is needed for backwards compatibility
    assertXPath(pXmlDocSlide1, "//a:fld [@type='datetime1']/a:t");

    // tdf#143316: Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlDocSlide1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:fld/a:rPr");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest2::testTdf53970()
{
    // Embedded media file
    {
        ::sd::DrawDocShellRef xDocShRef
            = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf53970.odp"), ODP);
        xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

        // Without fix in place, the media shape was lost on export.
        CPPUNIT_ASSERT(getPage(0, xDocShRef)->hasElements());

        xDocShRef->DoClose();
    }

    // Linked media file
    {
        ::sd::DrawDocShellRef xDocShRef = loadURL(
            m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf53970_linked.odp"), ODP);
        utl::TempFile tempFile;
        xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

        xmlDocUniquePtr pXmlRels = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
        CPPUNIT_ASSERT(pXmlRels);
        assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@TargetMode='External']", 2);

        uno::Reference<beans::XPropertySet> xShape(getShape(0, getPage(0, xDocShRef)));
        CPPUNIT_ASSERT(xShape.is());
        OUString sVideoURL;

        // Without fix in place, the media shape was imported as an image after export
        // and this test would have failed with exception of type com.sun.star.beans.UnknownPropertyException
        CPPUNIT_ASSERT_MESSAGE("MediaURL property is not set",
                               xShape->getPropertyValue("MediaURL") >>= sVideoURL);
        CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sVideoURL.isEmpty());

        xDocShRef->DoClose();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
