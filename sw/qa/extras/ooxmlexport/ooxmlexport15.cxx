/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf133334_followPgStyle, "tdf133334_followPgStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf133370_columnBreak, "tdf133370_columnBreak.odt")
{
    // Since non-DOCX formats ignores column breaks in non-column situtations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}


DECLARE_OOXMLEXPORT_TEST(testTdf134063, "tdf134063.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pDump = parseLayoutDump();

    // There are three tabs with default width
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[1]", "nWidth").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[2]", "nWidth").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(720), getXPath(pDump, "//page[1]/body/txt[1]/Text[3]", "nWidth").toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testAtPageShapeRelOrientation, "rotated_shape.fodt")
{
    // invalid combination of at-page anchor and horizontal-rel="paragraph"
    // caused relativeFrom="column" instead of relativeFrom="page"

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionH/wp:posOffset", "-480060");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionH", "relativeFrom", "page");
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset", "8147685");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV", "relativeFrom", "page");

    // same for sw
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionH/wp:posOffset", "720090");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionH", "relativeFrom", "page");
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset", "1080135");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor"
        "/wp:positionV", "relativeFrom", "page");

    // now test text rotation -> VML writing direction
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape/v:textbox", "style", "mso-layout-flow-alt:bottom-to-top");
    // text wrap -> VML
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape/w10:wrap", "type", "none");
    // vertical alignment -> VML
    OUString const style = getXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Fallback/w:pict/v:shape", "style");
    CPPUNIT_ASSERT(style.indexOf("v-text-anchor:middle") != -1);
}

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginHasFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginHasFooter.docx")
{
    // TODO: fix export too
    if (mbExported)
        return;
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
<<<<<<< HEAD   (cb168f sw: generate unique style name for table box and line frames)
    const sal_Int32 nAnchoredHeight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1147), nAnchoredHeight);
=======
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height", "1147");
}

DECLARE_OOXMLIMPORT_TEST(TestTdf112342, "tdf112342.docx")
{
    //Get the last para
    uno::Reference<text::XTextRange> xPara = getParagraph(3);
    auto xCur = xPara->getText()->createTextCursor();
    //Go to the end of it
    xCur->gotoEnd(false);
    //And let's remove the last 2 chars (the last para with its char).
    xCur->goLeft(2, true);
    xCur->setString("");

    //If the second paragraph on the second page, this will be passed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page break does not match", 2, getPages());
}

DECLARE_OOXMLIMPORT_TEST(TestTdf132483, "tdf132483.docx")
{
    uno::Reference<beans::XPropertySet> xOLEProps(getShape(1), uno::UNO_QUERY_THROW);
    sal_Int16 nVRelPos = -1;
    sal_Int16 nHRelPos = -1;
    xOLEProps->getPropertyValue("VertOrientRelation") >>= nVRelPos;
    xOLEProps->getPropertyValue("HoriOrientRelation") >>= nHRelPos;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The OLE is shifted vertically",
        text::RelOrientation::PAGE_FRAME , nVRelPos);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The OLE is shifted horizontally",
        text::RelOrientation::PAGE_FRAME , nHRelPos);
>>>>>>> CHANGE (e520a4 tdf#112342 DOCX import: fix page break before image-only par)
}

DECLARE_OOXMLEXPORT_TEST(testRelativeAnchorHeightFromBottomMarginNoFooter,
                         "tdf133070_testRelativeAnchorHeightFromBottomMarginNoFooter.docx")
{
    // TODO: fix export too
    if (mbExported)
        return;
    // tdf#133070 The height was set relative to page print area bottom,
    // but this was handled relative to page height.
    // Note: page print area bottom = margin + footer height.
    // In this case the footer does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nAnchoredHeight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1147), nAnchoredHeight);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
