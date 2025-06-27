/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_fonts.h>

#include "helper/debughelper.hxx"

#include "helper/qahelper.hxx"

#include <userdat.hxx>
#include <docpool.hxx>
#include <cellvalue.hxx>
#include <formulacell.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <stlpool.hxx>
#include <editutil.hxx>
#include <scopetools.hxx>
#include <postit.hxx>
#include <validat.hxx>

#include <svx/svdpage.hxx>
#include <tabprotection.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/section.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <unotools/saveopt.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest : public ScModelTestBase
{
protected:
    void testExcelCellBorders(const OUString& sFormatType);

public:
    ScExportTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(ScExportTest, testExport)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();

    pDoc->SetValue(0, 0, 0, 1.0);

    saveAndReload(u"calc8"_ustr);

    pDoc = getScDoc();
    double aVal = pDoc->GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL(1.0, aVal);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testDefaultFontHeight)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();
    ScDocumentPool* pPool = pDoc->GetPool();
    pPool->SetUserDefaultItem(SvxFontHeightItem(400, 100, ATTR_FONT_HEIGHT));
    pPool->SetUserDefaultItem(SvxFontHeightItem(400, 100, ATTR_CJK_FONT_HEIGHT));
    pPool->SetUserDefaultItem(SvxFontHeightItem(400, 100, ATTR_CTL_FONT_HEIGHT));

    saveAndReload(u"calc8"_ustr);

    pDoc = getScDoc();
    pPool = pDoc->GetPool();
    const SvxFontHeightItem& rItem = pPool->GetUserOrPoolDefaultItem(ATTR_FONT_HEIGHT);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(400), rItem.GetHeight());
    const SvxFontHeightItem& rCJKItem = pPool->GetUserOrPoolDefaultItem(ATTR_CJK_FONT_HEIGHT);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(400), rCJKItem.GetHeight());
    const SvxFontHeightItem& rCTLItem = pPool->GetUserOrPoolDefaultItem(ATTR_CTL_FONT_HEIGHT);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(400), rCTLItem.GetHeight());
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf139167)
{
    createScDoc("xlsx/tdf139167.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellStyles", "count", u"6");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:fill/x:patternFill/x:bgColor", "rgb",
                u"FFFFFF00");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testFontColorWithMultipleAttributesDefined)
{
    // Related: TDF #113271
    // Test font color where "rgb" and "theme" attribute is defined and
    // is imported and exported correctly. Theme should have priority,
    // so LO is fine to ignore "rgb" at export.

    createScDoc("xlsx/tdf113271.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:fonts", "count", u"6");

    // Expect "theme" attribute to be set correctly
    assertXPath(pDoc, "/x:styleSheet/x:fonts/x:font[1]/x:color", "theme", u"1");
    // We don't export "rgb" attribute
    assertXPathNoAttribute(pDoc, "/x:styleSheet/x:fonts/x:font[1]/x:color", "rgb");
    // Just making sure the checked font is the correct one
    assertXPath(pDoc, "/x:styleSheet/x:fonts/x:font[1]/x:name", "val", u"Calibri");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf139394)
{
    createScDoc("xlsx/tdf139394.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
        "x14:cfRule/xm:f",
        u"LEFT(A1,LEN(\"+\"))=\"+\"");
    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
        "x14:cfRule/xm:f",
        u"RIGHT(A2,LEN(\"-\"))=\"-\"");
    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[3]/"
        "x14:cfRule/xm:f",
        u"LEFT(A3,LEN($B$3))=$B$3");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testExtCondFormatXLSX)
{
    createScDoc("xlsx/tdf139021.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
        "x14:cfRule",
        "type", u"containsText");
    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
        "x14:cfRule/xm:f[1]",
        u"NOT(ISERROR(SEARCH($B$1,A1)))");
    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
        "x14:cfRule/xm:f[2]",
        u"$B$1");
    assertXPath(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
        "x14:cfRule",
        "type", u"notContainsText");
    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
        "x14:cfRule/xm:f[1]",
        u"ISERROR(SEARCH($B$2,A2))");
    assertXPathContent(
        pDoc,
        "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
        "x14:cfRule/xm:f[2]",
        u"$B$2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf90104)
{
    createScDoc("xlsx/tdf90104.xlsx");

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc,
                       "/x:worksheet/x:dataValidations/x:dataValidation/mc:AlternateContent"
                       "/mc:Choice/x12ac:list",
                       u"1,\"2,3\",4,\"5,6\"");
    assertXPathContent(pDoc,
                       "/x:worksheet/x:dataValidations/x:dataValidation/mc:AlternateContent"
                       "/mc:Fallback/x:formula1",
                       u"\"1,2,3,4,5,6\"");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf111876)
{
    // Document with relative path hyperlink

    createScDoc("xlsx/tdf111876.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/_rels/sheet1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pDoc);
    OUString sTarget = getXPath(pDoc, "/rels:Relationships/rels:Relationship", "Target");

    // Document is saved to the temporary directory, relative path should be different than original one
    CPPUNIT_ASSERT(sTarget != "../xls/bug-fixes.xls");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testPasswordExport)
{
    std::vector<OUString> aFilterNames{ u"calc8"_ustr, u"MS Excel 97"_ustr,
                                        u"Calc Office Open XML"_ustr };

    for (size_t i = 0; i < aFilterNames.size(); ++i)
    {
        createScDoc();

        ScDocument* pDoc = getScDoc();

        pDoc->SetValue(0, 0, 0, 1.0);

        saveAndReload(aFilterNames[i], /*pPassword*/ "test");

        pDoc = getScDoc();
        double aVal = pDoc->GetValue(0, 0, 0);
        ASSERT_DOUBLES_EQUAL(1.0, aVal);
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf134332)
{
    createScDoc("ods/tdf134332.ods");

    ScDocument* pDoc = getScDoc();

    ASSERT_DOUBLES_EQUAL(190.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    ASSERT_DOUBLES_EQUAL(238.0, pDoc->GetValue(ScAddress(0, 10144, 0)));

    saveAndReload(u"calc8"_ustr, /*pPassword*/ "test");

    // Without the fixes in place, it would have failed here
    pDoc = getScDoc();
    ASSERT_DOUBLES_EQUAL(190.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    ASSERT_DOUBLES_EQUAL(238.0, pDoc->GetValue(ScAddress(0, 10144, 0)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf99856_dataValidationTest)
{
    createScDoc("ods/tdf99856_dataValidationTest.ods");

    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    const ScValidationData* pData = pDoc->GetValidationEntry(2);
    CPPUNIT_ASSERT(pData);

    // Excel can't open corrupt file if the list is longer than 255 characters
    std::vector<ScTypedStrData> aList;
    pData->FillSelectionList(aList, ScAddress(0, 1, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(18), aList.size());
    CPPUNIT_ASSERT_EQUAL(u"18 Missis"_ustr, aList[17].GetString());
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testProtectionKeyODS_UTF16LErtlSHA1)
{
    static OUString constexpr password(
        u"1012345678901234567890123456789012345678901234567890"_ustr);

    createScDoc("fods/protection-key1.fods");

    ScDocument* pDoc = getScDoc();
    ScDocProtection* const pDocProt(pDoc->GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    const ScTableProtection* const pTabProt(pDoc->GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    save(u"calc8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc,
                "//office:spreadsheet[@table:structure-protected='true' and "
                "@table:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=' and "
                "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc,
                "//table:table[@table:protected='true' and "
                "@table:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=' and "
                "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testProtectionKeyODS_UTF8SHA1)
{
    static OUString constexpr password(
        u"1012345678901234567890123456789012345678901234567890"_ustr);

    createScDoc("fods/protection-key2.fods");

    ScDocument* pDoc = getScDoc();
    ScDocProtection* const pDocProt(pDoc->GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    const ScTableProtection* const pTabProt(pDoc->GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    save(u"calc8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc,
                "//office:spreadsheet[@table:structure-protected='true' and "
                "@table:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=' and "
                "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc,
                "//table:table[@table:protected='true' and "
                "@table:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=' and "
                "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testProtectionKeyODS_UTF8SHA256ODF12)
{
    static OUString constexpr password(
        u"1012345678901234567890123456789012345678901234567890"_ustr);

    createScDoc("fods/protection-key3.fods");

    ScDocument* pDoc = getScDoc();
    ScDocProtection* const pDocProt(pDoc->GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    const ScTableProtection* const pTabProt(pDoc->GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    save(u"calc8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "//office:spreadsheet[@table:structure-protected='true' and "
        "@table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and "
        "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
    assertXPath(
        pXmlDoc,
        "//table:table[@table:protected='true' and "
        "@table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and "
        "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testProtectionKeyODS_UTF8SHA256W3C)
{
    static OUString constexpr password(
        u"1012345678901234567890123456789012345678901234567890"_ustr);

    createScDoc("fods/protection-key4.fods");

    ScDocument* pDoc = getScDoc();
    ScDocProtection* const pDocProt(pDoc->GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    const ScTableProtection* const pTabProt(pDoc->GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    save(u"calc8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "//office:spreadsheet[@table:structure-protected='true' and "
        "@table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and "
        "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
    assertXPath(
        pXmlDoc,
        "//table:table[@table:protected='true' and "
        "@table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and "
        "@table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testProtectionKeyODS_XL_SHA1)
{
    static OUString constexpr password(
        u"1012345678901234567890123456789012345678901234567890"_ustr);

    createScDoc("fods/protection-key5.fods");

    ScDocument* pDoc = getScDoc();
    ScDocProtection* const pDocProt(pDoc->GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    const ScTableProtection* const pTabProt(pDoc->GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    save(u"calc8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "//office:spreadsheet[@table:structure-protected='true' and "
        "@table:protection-key='OX3WkEe79fv1PE+FUmfOLdwVoqI=' and "
        "@table:protection-key-digest-algorithm='http://docs.oasis-open.org/office/ns/table/"
        "legacy-hash-excel' and "
        "@loext:protection-key-digest-algorithm-2='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(
        pXmlDoc,
        "//table:table[@table:protected='true' and "
        "@table:protection-key='OX3WkEe79fv1PE+FUmfOLdwVoqI=' and "
        "@table:protection-key-digest-algorithm='http://docs.oasis-open.org/office/ns/table/"
        "legacy-hash-excel' and "
        "@loext:protection-key-digest-algorithm-2='http://www.w3.org/2000/09/xmldsig#sha1']");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testFormatExportODS)
{
    createScDoc("ods/formats.ods");

    saveAndReload(u"calc8"_ustr);

    ScDocument* pDoc = getScDoc();

    testFormats(pDoc, u"calc8");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCommentExportXLSX)
{
    //tdf#104729 FILESAVE OpenOffice do not save author of the comment during export to .xlsx
    createScDoc("ods/comment.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pComments = parseExport(u"xl/comments1.xml"_ustr);
    CPPUNIT_ASSERT(pComments);

    assertXPathContent(pComments, "/x:comments/x:authors/x:author[1]", u"BAKO");
    assertXPath(pComments, "/x:comments/x:authors/x:author", 1);

    assertXPathContent(pComments, "/x:comments/x:commentList/x:comment/x:text/x:r/x:t",
                       u"Komentarz");

    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);
    CPPUNIT_ASSERT(pVmlDrawing);

    //assertXPath(pVmlDrawing, "/xml/v:shapetype", "coordsize", "21600,21600");
    assertXPath(pVmlDrawing, "/xml/v:shapetype", "spt", u"202");
    assertXPath(pVmlDrawing, "/xml/v:shapetype/v:stroke", "joinstyle", u"miter");
    const OUString sShapeTypeId = "#" + getXPath(pVmlDrawing, "/xml/v:shapetype", "id");

    assertXPath(pVmlDrawing, "/xml/v:shape", "type", sShapeTypeId);
    assertXPath(pVmlDrawing, "/xml/v:shape/v:shadow", "color", u"black");
    assertXPath(pVmlDrawing, "/xml/v:shape/v:shadow", "obscured", u"t");

    //tdf#117274 fix MSO interoperability with the secret VML shape type id
    assertXPath(pVmlDrawing, "/xml/v:shapetype", "id", u"_x0000_t202");
    assertXPath(pVmlDrawing, "/xml/v:shape", "type", u"#_x0000_t202");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCommentExportXLSX_2_XLSX)
{
    //tdf#117287 FILESAVE XLSX: Comments always disappear after opening the exported XLSX file with Excel
    createScDoc("xlsx/tdf117287_comment.xlsx");

    ScDocument* pDoc = getScDoc();
    ScAddress aPosC9(2, 8, 0);
    ScPostIt* pNote = pDoc->GetNote(aPosC9);

    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT(!pNote->IsCaptionShown());

    pNote->ShowCaption(aPosC9, true);

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pComments = parseExport(u"xl/comments1.xml"_ustr);
    CPPUNIT_ASSERT(pComments);

    assertXPathContent(pComments, "/x:comments/x:commentList/x:comment/x:text/x:r/x:t",
                       u"visible comment");

    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPath(pVmlDrawing, "/xml/v:shape/x:ClientData/x:Visible", 0);
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(ScExportTest, testCustomColumnWidthExportXLSX)
{
    //tdf#100946 FILESAVE Excel on macOS ignored column widths in XLSX last saved by LO
    createScDoc("ods/custom_column_width.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // tdf#124741: check that we export default width, otherwise the skipped columns would have
    // wrong width. Previously defaultColWidth attribute was missing
    double nDefWidth
        = getXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "defaultColWidth").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.53515625, nDefWidth, 0.01);

    // First column, has everything default (width in Calc: 1280), skipped

    // Second column, has custom width (width in Calc: 1225)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "outlineLevel", u"0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "customWidth", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "min", u"2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "max", u"2");

    // Third column, has everything default (width in Calc: 1280), skipped

    // Fourth column has custom width. Columns from 4 to 7 are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "outlineLevel", u"0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "customWidth", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "min", u"4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "max", u"4");

    // 5th column has custom width. Columns from 4 to 7 are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "outlineLevel", u"0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "customWidth", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "min", u"5");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "max", u"5");

    // 6th and 7th columns have default width and they are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "outlineLevel", u"0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "customWidth", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "min", u"6");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "max", u"7");

    // 8th column has everything default - skipped

    // 9th column has custom width
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "outlineLevel", u"0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "customWidth", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "min", u"9");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "max", u"9");

    // We expected that exactly 5 unique Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 5);

    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "outlineLevel", u"0");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "customFormat", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "customHeight", u"false");
}
#endif

CPPUNIT_TEST_FIXTURE(ScExportTest, testXfDefaultValuesXLSX)
{
    //tdf#70565 FORMATTING: User Defined Custom Formatting is not applied during importing XLSX documents
    createScDoc("xlsx/xf_default_values.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // cellStyleXfs don't need xfId, so we need to make sure it is not saved
    assertXPathNoAttribute(pSheet, "/x:styleSheet/x:cellStyleXfs/x:xf[1]", "xfId");

    // Because numFmtId fontId fillId borderId xfId are not existing during import
    // it should be created during export, with values set to "0"
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[1]", "xfId", u"0");
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[2]", "xfId", u"0");
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[3]", "xfId", u"0");
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[4]", "xfId", u"0");

    // We expected that exactly 15 cellXfs:xf Nodes will be produced
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf", 14);
}

static auto verifySpreadsheet13(char const* const pTestName, ScDocument& rDoc) -> void
{
    // OFFICE-2173 table:tab-color
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, Color(0xff3838), rDoc.GetTabBgColor(0));
    // OFFICE-3857 table:scale-to-X/table:scale-to-Y
    OUString styleName = rDoc.GetPageStyle(0);
    ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find(styleName, SfxStyleFamily::Page);
    CPPUNIT_ASSERT_MESSAGE(pTestName, pStyleSheet);

    SfxItemSet const& rSet = pStyleSheet->GetItemSet();
    ScPageScaleToItem const& rItem(rSet.Get(ATTR_PAGE_SCALETO));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, sal_uInt16(2), rItem.GetWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, sal_uInt16(3), rItem.GetHeight());
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testODF13)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // import
    createScDoc("ods/spreadsheet13e.ods");
    ScDocument* pDoc = getScDoc();

    // check model
    verifySpreadsheet13("import", *pDoc);

    {
        // export ODF 1.3
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);

        // FIXME: Error: unexpected attribute "loext:scale-to-X"
        skipValidation();

        saveAndReload(u"calc8"_ustr);
        pDoc = getScDoc();

        // check XML
        xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
        assertXPath(pContentXml, "/office:document-content/office:automatic-styles/style:style/"
                                 "style:table-properties[@table:tab-color='#ff3838']");
        xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/"
                                "style:page-layout-properties[@style:scale-to-X='2']");
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/"
                                "style:page-layout-properties[@style:scale-to-Y='3']");

        // check model
        verifySpreadsheet13("1.3 reload", *pDoc);
    }
    {
        // export ODF 1.2 Extended
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_012_EXTENDED);

        saveAndReload(u"calc8"_ustr);
        pDoc = getScDoc();

        // check XML
        xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
        assertXPath(pContentXml, "/office:document-content/office:automatic-styles/style:style/"
                                 "style:table-properties[@tableooo:tab-color='#ff3838']");
        xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/"
                                "style:page-layout-properties[@loext:scale-to-X='2']");
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/"
                                "style:page-layout-properties[@loext:scale-to-Y='3']");

        // check model
        verifySpreadsheet13("1.2 Extended reload", *pDoc);
    }
    {
        // export ODF 1.2
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_012);

        save(u"calc8"_ustr);

        // check XML
        xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
        assertXPathNoAttribute(
            pContentXml,
            "/office:document-content/office:automatic-styles/style:style/style:table-properties",
            "tab-color");
        xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
        assertXPathNoAttribute(pStylesXml,
                               "/office:document-styles/office:automatic-styles/"
                               "style:page-layout[1]/style:page-layout-properties",
                               "scale-to-X");
        assertXPathNoAttribute(pStylesXml,
                               "/office:document-styles/office:automatic-styles/"
                               "style:page-layout[1]/style:page-layout-properties",
                               "scale-to-Y");

        // don't reload - no point
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testColumnWidthResaveXLSX)
{
    // tdf#91475 FILESAVE: Column width is not preserved in XLSX / after round trip.
    // Test if after resave .xlsx file, columns width is identical with previous one
    createScDoc("xlsx/different-column-width-excel2010.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // In original Excel document the width is "24"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "width", u"24");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "customWidth", u"true");

    // In original Excel document the width is "12"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "width", u"12");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "customWidth", u"true");

    // In original Excel document the width is "6"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "width", u"6");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "customWidth", u"true");

    // In original Excel document the width is "1"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "width", u"1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "customWidth", u"true");

    // In original Excel document the width is "250"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "width", u"250");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "customWidth", u"true");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 5);
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(ScExportTest, testColumnWidthExportFromODStoXLSX)
{
    // tdf#91475 FILESAVE: Column width is not preserved in XLSX / after round trip.
    // Test if after export .ods to .xlsx format, displayed columns width
    // is identical with previous (.ods) one

    createScDoc("ods/different-column-width.ods");

    ScDocument* pDoc = getScDoc();

    // Col 1, Tab 0 (Column width 2.00 in)
    sal_uInt16 nExpectedColumn0Width
        = pDoc->GetColWidth(static_cast<SCCOL>(0), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2880), nExpectedColumn0Width);

    // Col 2, Tab 0 (Column width 1.00 in)
    sal_uInt16 nExpectedColumn1Width
        = pDoc->GetColWidth(static_cast<SCCOL>(1), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1440), nExpectedColumn1Width);

    // Col 3, Tab 0 (Column width 0.50 in)
    sal_uInt16 nExpectedColumn2Width
        = pDoc->GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(720), nExpectedColumn2Width);

    // Col 4, Tab 0 (Column width 0.25 in)
    sal_uInt16 nExpectedColumn3Width
        = pDoc->GetColWidth(static_cast<SCCOL>(3), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(360), nExpectedColumn3Width);

    // Col 5, Tab 0 (Column width 13.57 in)
    sal_uInt16 nExpectedColumn4Width
        = pDoc->GetColWidth(static_cast<SCCOL>(4), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(19539), nExpectedColumn4Width);

    // Export to .xlsx and compare column width with the .ods
    // We expect that column width from .ods will be exactly the same as imported from .xlsx

    saveAndReload(u"Calc Office Open XML"_ustr);

    pDoc = getScDoc();

    // Col 1, Tab 0
    sal_uInt16 nCalcWidth;
    nCalcWidth = pDoc->GetColWidth(static_cast<SCCOL>(0), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn0Width, nCalcWidth);

    // Col 2, Tab 0
    nCalcWidth = pDoc->GetColWidth(static_cast<SCCOL>(1), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn1Width, nCalcWidth);

    // Col 3, Tab 0
    nCalcWidth = pDoc->GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn2Width, nCalcWidth);

    // Col 4, Tab 0
    nCalcWidth = pDoc->GetColWidth(static_cast<SCCOL>(3), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn3Width, nCalcWidth);

    // Col 5, Tab 0
    nCalcWidth = pDoc->GetColWidth(static_cast<SCCOL>(4), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn4Width, nCalcWidth);
}
#endif

CPPUNIT_TEST_FIXTURE(ScExportTest, testOutlineExportXLSX)
{
    //tdf#100347 FILESAVE FILEOPEN after exporting to .xlsx format grouping are lost
    //tdf#51524  FILESAVE .xlsx and.xls looses width information for hidden/collapsed grouped columns
    createScDoc("ods/outline.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Maximum Outline Row is 4 for this document
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "outlineLevelRow", u"4");
    // Maximum Outline Column is 4 for this document
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "outlineLevelCol", u"4");

    // First XML node, creates two columns (from min=1 to max=2)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "outlineLevel", u"1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "min", u"1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "max", u"2");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "outlineLevel", u"2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "min", u"3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "max", u"3");

    // Column 4 has custom width and it is hidden. We need to make sure that it is created
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "outlineLevel", u"2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "min", u"4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "max", u"4");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "min", u"5");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "max", u"6");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "min", u"7");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "max", u"7");

    // Column 8 has custom width and it is hidden. We need to make sure that it is created
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "min", u"8");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "max", u"8");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "min", u"9");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "max", u"19");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "collapsed", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "min", u"20");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "max", u"20");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "min", u"21");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "max", u"21");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "outlineLevel", u"2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "min", u"22");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "max", u"23");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "outlineLevel", u"1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "collapsed", u"true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "min", u"24");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "max", u"24");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "outlineLevel", u"1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "min", u"25");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "max", u"26");

    // We expected that exactly 12 unique Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 12);

    // First row is empty and default so it is not written into XML file
    // so we need to save 29 rows, as it provides information about outLineLevel
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "r", u"2");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "outlineLevel", u"1");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "r", u"3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "outlineLevel", u"2");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "r", u"4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "outlineLevel", u"2");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "r", u"5");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "r", u"6");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "r", u"7");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "r", u"8");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "r", u"9");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "collapsed", u"false");
    // Next rows are the same as the previous one but it needs to be preserved,
    // as they contain information about outlineLevel
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "r", u"21");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "outlineLevel", u"4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "collapsed", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "r", u"22");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "collapsed", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "r", u"23");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "hidden", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "outlineLevel", u"3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "collapsed", u"false");

    // We expected that exactly 29 Row Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 29);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testAllRowsHiddenXLSX)
{
    createScDoc("xlsx/tdf105840_allRowsHidden.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "zeroHeight", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testHiddenEmptyRowsXLSX)
{
    //tdf#98106 FILESAVE: Hidden and empty rows became visible when export to .XLSX
    createScDoc("ods/hidden-empty-rows.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "zeroHeight", u"false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "hidden", u"true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "hidden", u"false");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testHiddenEmptyColsODS)
{
    //tdf#98106 FILESAVE: Hidden and empty rows became visible when export to .XLSX
    createScDoc("ods/tdf128895_emptyHiddenCols.ods");

    save(u"calc8"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);
    assertXPath(pSheet, "//table:table/table:table-column[2]");
    assertXPath(pSheet, "//table:table/table:table-column[2]", "number-columns-repeated", u"1017");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testLandscapeOrientationXLSX)
{
    //tdf#48767 - Landscape page orientation is not loaded from .xlsx format with MS Excel, after export with Libre Office
    createScDoc("ods/hidden-empty-rows.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // the usePrinterDefaults cannot be saved to allow opening sheets in Landscape mode via MS Excel
    assertXPathNoAttribute(pSheet, "/x:worksheet/x:pageSetup", "usePrinterDefaults");
    assertXPath(pSheet, "/x:worksheet/x:pageSetup", "orientation", u"landscape");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testMiscRowHeightExport)
{
    static const TestParam::RowData DfltRowData[] = {
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1746, 0, false },
        // check last couple of row in document to ensure
        // they are 5.29mm ( effective default row xlsx height )
        { 1048573, 1048575, 0, 529, 0, false },
    };

    static const TestParam::RowData EmptyRepeatRowData[] = {
        // rows 0-4, 5-10, 17-20 are all set at various
        // heights, there is no content in the rows, there
        // was a bug where only the first row ( of repeated rows )
        // was set after export
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1767, 0, false },
    };

    TestParam aTestValues[] = {
        // Checks that some distributed ( non-empty ) heights remain set after export (roundtrip)
        // additionally there is effectively a default row height ( 5.29 mm ). So we test the
        // unset rows at the end of the document to ensure the effective xlsx default height
        // is set there too.
        { u"xlsx/miscrowheights.xlsx", u"Calc Office Open XML"_ustr, SAL_N_ELEMENTS(DfltRowData),
          DfltRowData },
        // Checks that some distributed ( non-empty ) heights remain set after export (to xls)
        { u"xlsx/miscrowheights.xlsx", u"MS Excel 97"_ustr, SAL_N_ELEMENTS(DfltRowData),
          DfltRowData },
        // Checks that repeated rows ( of various heights ) remain set after export ( to xlsx )
        { u"ods/miscemptyrepeatedrowheights.ods", u"Calc Office Open XML"_ustr,
          SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
        // Checks that repeated rows ( of various heights ) remain set after export ( to xls )
        { u"ods/miscemptyrepeatedrowheights.ods", u"MS Excel 97"_ustr,
          SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
    };
    miscRowHeightsTest(aTestValues, SAL_N_ELEMENTS(aTestValues));
}

namespace
{
void setAttribute(ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd,
                  sal_uInt16 nType, Color nColor = COL_BLACK)
{
    ESelection aSel(nPara, nStart, nPara, nEnd);

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    switch (nType)
    {
        case EE_CHAR_WEIGHT:
        {
            SvxWeightItem aWeight(WEIGHT_BOLD, nType);
            aItemSet.Put(aWeight);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_ITALIC:
        {
            SvxPostureItem aItalic(ITALIC_NORMAL, nType);
            aItemSet.Put(aItalic);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            SvxCrossedOutItem aCrossOut(STRIKEOUT_SINGLE, nType);
            aItemSet.Put(aCrossOut);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            SvxOverlineItem aItem(LINESTYLE_DOUBLE, nType);
            aItemSet.Put(aItem);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            SvxUnderlineItem aItem(LINESTYLE_DOUBLE, nType);
            aItemSet.Put(aItem);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_COLOR:
        {
            SvxColorItem aItem(nColor, nType);
            aItemSet.Put(aItem);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        default:;
    }
}

void setFont(ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd,
             const OUString& rFontName)
{
    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    SvxFontItem aItem(FAMILY_MODERN, rFontName, u""_ustr, PITCH_VARIABLE, RTL_TEXTENCODING_UTF8,
                      EE_CHAR_FONTINFO);
    aItemSet.Put(aItem);
    rEE.QuickSetAttribs(aItemSet, ESelection(nPara, nStart, nPara, nEnd));
}

void setEscapement(ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd,
                   short nEsc, sal_uInt8 nRelSize)
{
    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    SvxEscapementItem aItem(nEsc, nRelSize, EE_CHAR_ESCAPEMENT);
    aItemSet.Put(aItem);
    rEE.QuickSetAttribs(aItemSet, ESelection(nPara, nStart, nPara, nEnd));
}
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testNamedRangeBugfdo62729)
{
#if !defined(MACOSX) // FIXME: infinite loop on jenkins' mac
    createScDoc("ods/fdo62729.ods");
    ScDocument* pDoc = getScDoc();

    ScRangeName* pNames = pDoc->GetRangeName();
    //should be just a single named range
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNames->size());
    pDoc->DeleteTab(0);
    //should be still a single named range
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNames->size());
    saveAndReload(u"calc8"_ustr);

    pDoc = getScDoc();

    pNames = pDoc->GetRangeName();
    //after reload should still have a named range
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNames->size());
#endif
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testBuiltinRangesXLSX)
{
    createScDoc("xlsx/built-in_ranges.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in names are still there
    assertXPathContent(pDoc,
                       "/x:workbook/x:definedNames/"
                       "x:definedName[@name='_xlnm._FilterDatabase'][@localSheetId='0']",
                       u"'Sheet1 Test'!$A$1:$A$5");
    assertXPathContent(pDoc,
                       "/x:workbook/x:definedNames/"
                       "x:definedName[@name='_xlnm._FilterDatabase'][@localSheetId='1']",
                       u"'Sheet2 Test'!$K$10:$K$14");
    assertXPathContent(
        pDoc,
        "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area'][@localSheetId='0']",
        u"'Sheet1 Test'!$A$1:$A$5");
    assertXPathContent(
        pDoc,
        "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area'][@localSheetId='1']",
        u"'Sheet2 Test'!$K$10:$M$18");

    //...and that no extra ones are added (see tdf#112571)
    assertXPath(pDoc,
                "/x:workbook/x:definedNames/"
                "x:definedName[@name='_xlnm._FilterDatabase_0'][@localSheetId='0']",
                0);
    assertXPath(pDoc,
                "/x:workbook/x:definedNames/"
                "x:definedName[@name='_xlnm._FilterDatabase_0'][@localSheetId='1']",
                0);
    assertXPath(
        pDoc,
        "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area_0'][@localSheetId='0']",
        0);
    assertXPath(
        pDoc,
        "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area_0'][@localSheetId='1']",
        0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testRichTextExportODS)
{
    struct
    {
        static bool isBold(const editeng::Section& rAttr)
        {
            return std::any_of(
                rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [](const SfxPoolItem* p) {
                    return p->Which() == EE_CHAR_WEIGHT
                           && static_cast<const SvxWeightItem*>(p)->GetWeight() == WEIGHT_BOLD;
                });
        }

        static bool isItalic(const editeng::Section& rAttr)
        {
            return std::any_of(
                rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [](const SfxPoolItem* p) {
                    return p->Which() == EE_CHAR_ITALIC
                           && static_cast<const SvxPostureItem*>(p)->GetPosture() == ITALIC_NORMAL;
                });
        }

        static bool isStrikeOut(const editeng::Section& rAttr)
        {
            return std::any_of(
                rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [](const SfxPoolItem* p) {
                    return p->Which() == EE_CHAR_STRIKEOUT
                           && static_cast<const SvxCrossedOutItem*>(p)->GetStrikeout()
                                  == STRIKEOUT_SINGLE;
                });
        }

        static bool isOverline(const editeng::Section& rAttr, FontLineStyle eStyle)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(),
                               [&eStyle](const SfxPoolItem* p) {
                                   return p->Which() == EE_CHAR_OVERLINE
                                          && static_cast<const SvxOverlineItem*>(p)->GetLineStyle()
                                                 == eStyle;
                               });
        }

        static bool isUnderline(const editeng::Section& rAttr, FontLineStyle eStyle)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(),
                               [&eStyle](const SfxPoolItem* p) {
                                   return p->Which() == EE_CHAR_UNDERLINE
                                          && static_cast<const SvxUnderlineItem*>(p)->GetLineStyle()
                                                 == eStyle;
                               });
        }

        static bool isFont(const editeng::Section& rAttr, const OUString& rFontName)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(),
                               [&rFontName](const SfxPoolItem* p) {
                                   return p->Which() == EE_CHAR_FONTINFO
                                          && static_cast<const SvxFontItem*>(p)->GetFamilyName()
                                                 == rFontName;
                               });
        }

        static bool isEscapement(const editeng::Section& rAttr, short nEsc, sal_uInt8 nRelSize)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(),
                               [&nEsc, &nRelSize](const SfxPoolItem* p) {
                                   if (p->Which() != EE_CHAR_ESCAPEMENT)
                                       return false;
                                   const SvxEscapementItem* pItem
                                       = static_cast<const SvxEscapementItem*>(p);
                                   return ((pItem->GetEsc() == nEsc)
                                           && (pItem->GetProportionalHeight() == nRelSize));
                               });
        }

        static bool isColor(const editeng::Section& rAttr, Color nColor)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(),
                               [&nColor](const SfxPoolItem* p) {
                                   return p->Which() == EE_CHAR_COLOR
                                          && static_cast<const SvxColorItem*>(p)->GetValue()
                                                 == nColor;
                               });
        }

        bool checkB2(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Bold and Italic")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // Check the first bold section.
            const editeng::Section* pAttr = aSecAttrs.data();
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 0 || pAttr->mnEnd != 4)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isBold(*pAttr))
                return false;

            // The middle section should be unformatted.
            pAttr = &aSecAttrs[1];
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 4 || pAttr->mnEnd != 9)
                return false;

            if (!pAttr->maAttributes.empty())
                return false;

            // The last section should be italic.
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 9 || pAttr->mnEnd != 15)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isItalic(*pAttr))
                return false;

            return true;
        }

        bool checkB4(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 3)
                return false;

            if (pText->GetText(0) != "One")
                return false;

            if (pText->GetText(1) != "Two")
                return false;

            if (pText->GetText(2) != "Three")
                return false;

            return true;
        }

        bool checkB5(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 6)
                return false;

            if (!pText->GetText(0).isEmpty())
                return false;

            if (pText->GetText(1) != "Two")
                return false;

            if (pText->GetText(2) != "Three")
                return false;

            if (!pText->GetText(3).isEmpty())
                return false;

            if (pText->GetText(4) != "Five")
                return false;

            if (!pText->GetText(5).isEmpty())
                return false;

            return true;
        }

        bool checkB6(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Strike Me")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 2)
                return false;

            // Check the first strike-out section.
            const editeng::Section* pAttr = aSecAttrs.data();
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 0 || pAttr->mnEnd != 6)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isStrikeOut(*pAttr))
                return false;

            // The last section should be unformatted.
            pAttr = &aSecAttrs[1];
            return pAttr->mnParagraph == 0 && pAttr->mnStart == 6 && pAttr->mnEnd == 9;
        }

        bool checkB7(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Font1 and Font2")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // First section should have "Courier" font applied.
            const editeng::Section* pAttr = aSecAttrs.data();
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 0 || pAttr->mnEnd != 5)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isFont(*pAttr, u"Courier"_ustr))
                return false;

            // Last section should have "Luxi Mono" applied.
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 10 || pAttr->mnEnd != 15)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isFont(*pAttr, u"Luxi Mono"_ustr))
                return false;

            return true;
        }

        bool checkB8(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Over and Under")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // First section should have overline applied.
            const editeng::Section* pAttr = aSecAttrs.data();
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 0 || pAttr->mnEnd != 4)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isOverline(*pAttr, LINESTYLE_DOUBLE))
                return false;

            // Last section should have underline applied.
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 9 || pAttr->mnEnd != 14)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isUnderline(*pAttr, LINESTYLE_DOUBLE))
                return false;

            return true;
        }

        bool checkB9(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Sub and Super")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // superscript
            const editeng::Section* pAttr = aSecAttrs.data();
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 0 || pAttr->mnEnd != 3)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isEscapement(*pAttr, 32, 64))
                return false;

            // subscript
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 8 || pAttr->mnEnd != 13)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isEscapement(*pAttr, -32, 66))
                return false;

            return true;
        }

        bool checkB10(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "BLUE AUTO")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 2)
                return false;

            // auto color
            const editeng::Section* pAttr = &aSecAttrs[1];
            if (pAttr->mnParagraph != 0 || pAttr->mnStart != 5 || pAttr->mnEnd != 9)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isColor(*pAttr, COL_AUTO))
                return false;

            return true;
        }

    } aCheckFunc;

    // Start with an empty document, put one edit text cell, and make sure it
    // survives the save and reload.
    createScDoc();
    const EditTextObject* pEditText;
    {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_MESSAGE("This document should at least have one sheet.",
                               pDoc->GetTableCount() > 0);

        // Insert an edit text cell.
        ScFieldEditEngine* pEE = &pDoc->GetEditEngine();
        pEE->SetTextCurrentDefaults(u"Bold and Italic"_ustr);
        // Set the 'Bold' part bold.
        setAttribute(*pEE, 0, 0, 4, EE_CHAR_WEIGHT);
        // Set the 'Italic' part italic.
        setAttribute(*pEE, 0, 9, 15, EE_CHAR_ITALIC);

        // Set this edit text to cell B2.
        pDoc->SetEditText(ScAddress(1, 1, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B2 value.", aCheckFunc.checkB2(pEditText));
    }

    // Now, save and reload this document.
    saveAndReload(u"calc8"_ustr);
    {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_MESSAGE("Reloaded document should at least have one sheet.",
                               pDoc->GetTableCount() > 0);
        ScFieldEditEngine* pEE = &pDoc->GetEditEngine();
        pEditText = pDoc->GetEditText(ScAddress(1, 1, 0));

        // Make sure the content of B2 is still intact.
        CPPUNIT_ASSERT_MESSAGE("Incorrect B2 value.", aCheckFunc.checkB2(pEditText));

        // Insert a multi-line content to B4.
        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"One\nTwo\nThree"_ustr);
        pDoc->SetEditText(ScAddress(1, 3, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 3, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B4 value.", aCheckFunc.checkB4(pEditText));
    }

    // Reload the doc again, and check the content of B2 and B4.
    saveAndReload(u"calc8"_ustr);
    {
        ScDocument* pDoc = getScDoc();
        ScFieldEditEngine* pEE = &pDoc->GetEditEngine();

        pEditText = pDoc->GetEditText(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT_MESSAGE("B2 should be an edit text.", pEditText);
        pEditText = pDoc->GetEditText(ScAddress(1, 3, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B4 value.", aCheckFunc.checkB4(pEditText));

        // Insert a multi-line content to B5, but this time, set some empty paragraphs.
        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"\nTwo\nThree\n\nFive\n"_ustr);
        pDoc->SetEditText(ScAddress(1, 4, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 4, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B5 value.", aCheckFunc.checkB5(pEditText));

        // Insert a text with strikethrough in B6.
        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"Strike Me"_ustr);
        // Set the 'Strike' part strikethrough.
        setAttribute(*pEE, 0, 0, 6, EE_CHAR_STRIKEOUT);
        pDoc->SetEditText(ScAddress(1, 5, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 5, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B6 value.", aCheckFunc.checkB6(pEditText));

        // Insert a text with different font segments in B7.
        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"Font1 and Font2"_ustr);
        setFont(*pEE, 0, 0, 5, u"Courier"_ustr);
        setFont(*pEE, 0, 10, 15, u"Luxi Mono"_ustr);
        pDoc->SetEditText(ScAddress(1, 6, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 6, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B7 value.", aCheckFunc.checkB7(pEditText));

        // Insert a text with overline and underline in B8.
        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"Over and Under"_ustr);
        setAttribute(*pEE, 0, 0, 4, EE_CHAR_OVERLINE);
        setAttribute(*pEE, 0, 9, 14, EE_CHAR_UNDERLINE);
        pDoc->SetEditText(ScAddress(1, 7, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 7, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B8 value.", aCheckFunc.checkB8(pEditText));

        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"Sub and Super"_ustr);
        setEscapement(*pEE, 0, 0, 3, 32, 64);
        setEscapement(*pEE, 0, 8, 13, -32, 66);
        pDoc->SetEditText(ScAddress(1, 8, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 8, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B9 value.", aCheckFunc.checkB9(pEditText));

        ScPatternAttr aCellFontColor(pDoc->getCellAttributeHelper());
        aCellFontColor.GetItemSet().Put(SvxColorItem(COL_BLUE, ATTR_FONT_COLOR));
        // Set font color of B10 to blue.
        pDoc->ApplyPattern(1, 9, 0, aCellFontColor);
        pEE->Clear();
        pEE->SetTextCurrentDefaults(u"BLUE AUTO"_ustr);
        // Set the color of the string "AUTO" to automatic color.
        setAttribute(*pEE, 0, 5, 9, EE_CHAR_COLOR, COL_AUTO);
        pDoc->SetEditText(ScAddress(1, 9, 0), pEE->CreateTextObject());
        pEditText = pDoc->GetEditText(ScAddress(1, 9, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B10 value.", aCheckFunc.checkB10(pEditText));
    }

    // Reload the doc again, and check the content of B2, B4, B6 and B7.
    saveAndReload(u"calc8"_ustr);
    ScDocument* pDoc = getScDoc();

    pEditText = pDoc->GetEditText(ScAddress(1, 1, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B2 value after save and reload.",
                           aCheckFunc.checkB2(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(1, 3, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B4 value after save and reload.",
                           aCheckFunc.checkB4(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(1, 4, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B5 value after save and reload.",
                           aCheckFunc.checkB5(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(1, 5, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B6 value after save and reload.",
                           aCheckFunc.checkB6(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(1, 6, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B7 value after save and reload.",
                           aCheckFunc.checkB7(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(1, 7, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B8 value after save and reload.",
                           aCheckFunc.checkB8(pEditText));
    pEditText = pDoc->GetEditText(ScAddress(1, 9, 0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B10 value after save and reload.",
                           aCheckFunc.checkB10(pEditText));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testRichTextCellFormatXLSX)
{
    createScDoc("xls/cellformat.xls");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // make sure the only cell in this doc is assigned some formatting record
    OUString aCellFormat = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row/x:c", "s");
    CPPUNIT_ASSERT_MESSAGE("Cell format is missing", !aCellFormat.isEmpty());

    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    OString nFormatIdx = OString::number(aCellFormat.toInt32() + 1);
    const OString aXPath1("/x:styleSheet/x:cellXfs/x:xf[" + nFormatIdx + "]/x:alignment");
    // formatting record is set to wrap text
    assertXPath(pStyles, aXPath1, "wrapText", u"true");

    // see what font it references
    const OString aXPath2("/x:styleSheet/x:cellXfs/x:xf[" + nFormatIdx + "]");
    OUString aFontId = getXPath(pStyles, aXPath2, "fontId");
    OString nFontIdx = OString::number(aFontId.toInt32() + 1);

    // that font should be bold
    const OString aXPath3("/x:styleSheet/x:fonts/x:font[" + nFontIdx + "]/x:b");
    assertXPath(pStyles, aXPath3, "val", u"true");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testFormulaRefSheetNameODS)
{
    createScDoc("ods/formula-quote-in-sheet-name.ods");
    {
        ScDocument* pDoc = getScDoc();

        sc::AutoCalcSwitch aACSwitch(*pDoc, true); // turn on auto calc.
        pDoc->SetString(ScAddress(1, 1, 0), u"='90''s Data'.B2"_ustr);
        CPPUNIT_ASSERT_EQUAL(1.1, pDoc->GetValue(ScAddress(1, 1, 0)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"='90''s Data'.B2"_ustr,
                                     pDoc->GetFormula(1, 1, 0));
    }
    // Now, save and reload this document.
    saveAndReload(u"calc8"_ustr);

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(1.1, pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula", u"='90''s Data'.B2"_ustr,
                                 pDoc->GetFormula(1, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCellValuesExportODS)
{
    // Start with an empty document
    createScDoc();
    {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_MESSAGE("This document should at least have one sheet.",
                               pDoc->GetTableCount() > 0);

        // set a value double
        pDoc->SetValue(ScAddress(0, 0, 0), 2.0); // A1

        // set a formula
        pDoc->SetValue(ScAddress(2, 0, 0), 3.0); // C1
        pDoc->SetValue(ScAddress(3, 0, 0), 3); // D1
        pDoc->SetString(ScAddress(4, 0, 0), u"=10*C1/4"_ustr); // E1
        pDoc->SetValue(ScAddress(5, 0, 0), 3.0); // F1
        pDoc->SetString(ScAddress(7, 0, 0), u"=SUM(C1:F1)"_ustr); //H1

        // set a string
        pDoc->SetString(ScAddress(0, 2, 0), u"a simple line"_ustr); //A3

        // set a digit string
        pDoc->SetString(ScAddress(0, 4, 0), u"'12"_ustr); //A5
        // set a contiguous value
        pDoc->SetValue(ScAddress(0, 5, 0), 12.0); //A6
        // set a contiguous string
        pDoc->SetString(ScAddress(0, 6, 0), u"a string"_ustr); //A7
        // set a contiguous formula
        pDoc->SetString(ScAddress(0, 7, 0), u"=$A$6"_ustr); //A8
    }
    // save and reload
    saveAndReload(u"calc8"_ustr);
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_MESSAGE("Reloaded document should at least have one sheet.",
                           pDoc->GetTableCount() > 0);

    // check value
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(7.5, pDoc->GetValue(4, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(5, 0, 0));

    // check formula
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula =10*C1/4", u"=10*C1/4"_ustr,
                                 pDoc->GetFormula(4, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula =SUM(C1:F1)", u"=SUM(C1:F1)"_ustr,
                                 pDoc->GetFormula(7, 0, 0));
    CPPUNIT_ASSERT_EQUAL(16.5, pDoc->GetValue(7, 0, 0));

    // check string
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aCell.getType());

    // check for an empty cell
    aCell.assign(*pDoc, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, aCell.getType());

    // check a digit string
    aCell.assign(*pDoc, ScAddress(0, 4, 0));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, aCell.getType());

    //check contiguous values
    CPPUNIT_ASSERT_EQUAL(12.0, pDoc->GetValue(0, 5, 0));
    CPPUNIT_ASSERT_EQUAL(u"a string"_ustr, pDoc->GetString(0, 6, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula =$A$6", u"=$A$6"_ustr, pDoc->GetFormula(0, 7, 0));
    CPPUNIT_ASSERT_EQUAL(pDoc->GetValue(0, 5, 0), pDoc->GetValue(0, 7, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCellNoteExportODS)
{
    createScDoc("ods/single-note.ods");
    ScAddress aPos(0, 0, 0); // Start with A1.
    {
        ScDocument* pDoc = getScDoc();

        CPPUNIT_ASSERT_MESSAGE("There should be a note at A1.", pDoc->HasNote(aPos));

        aPos.IncRow(); // Move to A2.
        ScPostIt* pNote = pDoc->GetOrCreateNote(aPos);
        pNote->SetText(aPos, u"Note One"_ustr);
        pNote->SetAuthor(u"Author One"_ustr);
        CPPUNIT_ASSERT_MESSAGE("There should be a note at A2.", pDoc->HasNote(aPos));
    }
    // save and reload
    saveAndReload(u"calc8"_ustr);
    ScDocument* pDoc = getScDoc();

    aPos.SetRow(0); // Move back to A1.
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A1.", pDoc->HasNote(aPos));
    aPos.IncRow(); // Move to A2.
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A2.", pDoc->HasNote(aPos));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCellNoteExportXLS)
{
    // Start with an empty document.s
    createScDoc("ods/notes-on-3-sheets.ods");
    {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("This document should have 3 sheets.", SCTAB(3),
                                     pDoc->GetTableCount());

        // Check note's presence.
        CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 2, 0)));

        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 0, 1)));
        CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(0, 1, 1)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 2, 1)));

        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 0, 2)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 1, 2)));
        CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(0, 2, 2)));
    }
    // save and reload as XLS.
    saveAndReload(u"MS Excel 97"_ustr);
    {
        ScDocument* pDoc = getScDoc();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("This document should have 3 sheets.", SCTAB(3),
                                     pDoc->GetTableCount());

        // Check note's presence again.
        CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 2, 0)));

        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 0, 1)));
        CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(0, 1, 1)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 2, 1)));

        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 0, 2)));
        CPPUNIT_ASSERT(!pDoc->HasNote(ScAddress(0, 1, 2)));
        CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(0, 2, 2)));
    }
}

namespace
{
void checkMatrixRange(ScDocument& rDoc, const ScRange& rRange)
{
    ScRange aMatRange;
    ScAddress aMatOrigin;
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            ScAddress aPos(nCol, nRow, rRange.aStart.Tab());
            bool bIsMatrix = rDoc.GetMatrixFormulaRange(aPos, aMatRange);
            CPPUNIT_ASSERT_MESSAGE("Matrix expected, but not found.", bIsMatrix);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong matrix range.", rRange, aMatRange);
            const ScFormulaCell* pCell = rDoc.GetFormulaCell(aPos);
            CPPUNIT_ASSERT_MESSAGE("This must be a formula cell.", pCell);

            bIsMatrix = pCell->GetMatrixOrigin(rDoc, aMatOrigin);
            CPPUNIT_ASSERT_MESSAGE("Not a part of matrix formula.", bIsMatrix);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong matrix origin.", aMatRange.aStart, aMatOrigin);
        }
    }
}
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testInlineArrayXLS)
{
    createScDoc("xls/inline-array.xls");

    saveAndReload(u"MS Excel 97"_ustr);

    ScDocument* pDoc = getScDoc();

    // B2:C3 contains a matrix.
    checkMatrixRange(*pDoc, ScRange(1, 1, 0, 2, 2, 0));

    // B5:D6 contains a matrix.
    checkMatrixRange(*pDoc, ScRange(1, 4, 0, 3, 5, 0));

    // B8:C10 as well.
    checkMatrixRange(*pDoc, ScRange(1, 7, 0, 2, 9, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testEmbeddedChartODS)
{
    createScDoc("xls/embedded-chart.xls");

    save(u"calc8"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[2]/"
                "table:table-row[7]/table:table-cell[2]/draw:frame/draw:object",
                "notify-on-update-of-ranges",
                u"Chart1.B3:Chart1.B5 Chart1.C2:Chart1.C2 Chart1.C3:Chart1.C5");
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testEmbeddedChartXLS)
{
    createScDoc("xls/embedded-chart.xls");

    saveAndReload(u"MS Excel 97"_ustr);

    ScDocument* pDoc = getScDoc();

    // Make sure the 2nd sheet is named 'Chart1'.
    OUString aName;
    pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(u"Chart1"_ustr, aName);

    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(*pDoc, *pOleObj);
    CPPUNIT_ASSERT_MESSAGE("Label range (B3:B5) not found.",
                           aRanges.Contains(ScRange(1, 2, 1, 1, 4, 1)));
    CPPUNIT_ASSERT_MESSAGE("Data label (C2) not found.",
                           aRanges.Contains(ScRange(ScAddress(2, 1, 1))));
    CPPUNIT_ASSERT_MESSAGE("Data range (C3:C5) not found.",
                           aRanges.Contains(ScRange(2, 2, 1, 2, 4, 1)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCellAnchoredGroupXLS)
{
    createScDoc("xls/cell-anchored-group.xls");

    saveAndReload(u"calc8"_ustr);

    // the document contains a group anchored on the first cell, make sure it's there in the right place
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 1 objects.", static_cast<size_t>(1), nCount);

    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Upper left of bounding rectangle should be nonnegative.",
                           pData->getShapeRect().Left() >= 0 || pData->getShapeRect().Top() >= 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testFormulaReferenceXLS)
{
    createScDoc("xls/formula-reference.xls");

    saveAndReload(u"MS Excel 97"_ustr);

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D2", u"=$A$2+$B$2+$C$2"_ustr,
                                 pDoc->GetFormula(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D3", u"=A3+B3+C3"_ustr,
                                 pDoc->GetFormula(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D6", u"=SUM($A$6:$C$6)"_ustr,
                                 pDoc->GetFormula(3, 5, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D7", u"=SUM(A7:C7)"_ustr,
                                 pDoc->GetFormula(3, 6, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D10", u"=$Two.$A$2+$Two.$B$2+$Two.$C$2"_ustr,
                                 pDoc->GetFormula(3, 9, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D11", u"=$Two.A3+$Two.B3+$Two.C3"_ustr,
                                 pDoc->GetFormula(3, 10, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D14", u"=MIN($Two.$A$2:$C$2)"_ustr,
                                 pDoc->GetFormula(3, 13, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula in D15", u"=MAX($Two.A3:C3)"_ustr,
                                 pDoc->GetFormula(3, 14, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testSheetProtectionXLSX)
{
    createScDoc("xlsx/ProtecteSheet1234Pass.xlsx");

    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    const ScTableProtection* pTabProtect = pDoc->GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtect);
    Sequence<sal_Int8> aHash = pTabProtect->getPasswordHash(PASSHASH_XL);
    // check has
    if (aHash.getLength() >= 2)
    {
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(204), static_cast<sal_uInt8>(aHash[0]));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(61), static_cast<sal_uInt8>(aHash[1]));
    }
    // we could flesh out this check I guess
    CPPUNIT_ASSERT(!pTabProtect->isOptionEnabled(ScTableProtection::OBJECTS));
    CPPUNIT_ASSERT(!pTabProtect->isOptionEnabled(ScTableProtection::SCENARIOS));
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testSheetProtectionXLSB)
{
    createScDoc("xlsb/tdf108017_calcProtection.xlsb");

    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    const ScTableProtection* pTabProtect = pDoc->GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtect);
    CPPUNIT_ASSERT(pTabProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS));
    CPPUNIT_ASSERT(!pTabProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS));
}

namespace
{
const char* toBorderName(SvxBorderLineStyle eStyle)
{
    switch (eStyle)
    {
        case SvxBorderLineStyle::SOLID:
            return "SOLID";
        case SvxBorderLineStyle::DOTTED:
            return "DOTTED";
        case SvxBorderLineStyle::DASHED:
            return "DASHED";
        case SvxBorderLineStyle::DASH_DOT:
            return "DASH_DOT";
        case SvxBorderLineStyle::DASH_DOT_DOT:
            return "DASH_DOT_DOT";
        case SvxBorderLineStyle::DOUBLE_THIN:
            return "DOUBLE_THIN";
        case SvxBorderLineStyle::FINE_DASHED:
            return "FINE_DASHED";
        default:;
    }

    return "";
}
}

void ScExportTest::testExcelCellBorders(const OUString& sFormatType)
{
    static const struct
    {
        SCROW mnRow;
        SvxBorderLineStyle mnStyle;
        tools::Long mnWidth;
    } aChecks[] = {
        { 1, SvxBorderLineStyle::SOLID, 1 }, // hair
        { 3, SvxBorderLineStyle::DOTTED, 15 }, // dotted
        { 5, SvxBorderLineStyle::DASH_DOT_DOT, 15 }, // dash dot dot
        { 7, SvxBorderLineStyle::DASH_DOT, 15 }, // dash dot
        { 9, SvxBorderLineStyle::FINE_DASHED, 15 }, // dashed
        { 11, SvxBorderLineStyle::SOLID, 15 }, // thin
        { 13, SvxBorderLineStyle::DASH_DOT_DOT, 35 }, // medium dash dot dot
        { 17, SvxBorderLineStyle::DASH_DOT, 35 }, // medium dash dot
        { 19, SvxBorderLineStyle::DASHED, 35 }, // medium dashed
        { 21, SvxBorderLineStyle::SOLID, 35 }, // medium
        { 23, SvxBorderLineStyle::SOLID, 50 }, // thick
        { 25, SvxBorderLineStyle::DOUBLE_THIN, -1 }, // double (don't check width)
    };

    {
        ScDocument* pDoc = getScDoc();

        for (auto const[nRow, eStyle, nWidth] : aChecks)
        {
            const editeng::SvxBorderLine* pLine = nullptr;
            pDoc->GetBorderLines(2, nRow, 0, nullptr, &pLine, nullptr, nullptr);
            CPPUNIT_ASSERT(pLine);
            CPPUNIT_ASSERT_EQUAL(toBorderName(eStyle), toBorderName(pLine->GetBorderLineStyle()));
            if (nWidth >= 0)
                CPPUNIT_ASSERT_EQUAL(nWidth, pLine->GetWidth());
        }
    }

    saveAndReload(sFormatType);
    ScDocument* pDoc = getScDoc();
    for (auto const[nRow, eStyle, nWidth] : aChecks)
    {
        const editeng::SvxBorderLine* pLine = nullptr;
        pDoc->GetBorderLines(2, nRow, 0, nullptr, &pLine, nullptr, nullptr);
        CPPUNIT_ASSERT(pLine);
        CPPUNIT_ASSERT_EQUAL(toBorderName(eStyle), toBorderName(pLine->GetBorderLineStyle()));
        if (nWidth >= 0)
            CPPUNIT_ASSERT_EQUAL(nWidth, pLine->GetWidth());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCellBordersXLS)
{
    createScDoc("xls/cell-borders.xls");
    testExcelCellBorders(u"MS Excel 97"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testCellBordersXLSX)
{
    createScDoc("xlsx/cell-borders.xlsx");
    testExcelCellBorders(u"Calc Office Open XML"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest, testTdf155368)
{
    createScDoc("ods/tdf155368.ods");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:WrapText"_ustr, {});

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    assertXPath(pStyles, "/x:styleSheet/x:cellXfs/x:xf[1]/x:alignment", "wrapText", u"false");

    // Without the fix in place, this test would have failed with
    // - Expected: false
    // - Actual  : true
    assertXPath(pStyles, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "wrapText", u"false");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
