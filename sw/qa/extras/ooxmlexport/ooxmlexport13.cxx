/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>

#include <editsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF, "tdf121374_sectionHF.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL( OUString("footer"), xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 6, getParagraphs() );
    //CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF2, "tdf121374_sectionHF2.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121867, "tdf121867.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwEditShell* pEditShell = pTextDoc->GetDocShell()->GetEditShell();
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 3; Actual  : 0', i.e. page width zoom was lost on export.
    CPPUNIT_ASSERT_EQUAL(SvxZoomType::PAGEWIDTH, pEditShell->GetViewOptions()->GetZoomType());
}

DECLARE_OOXMLEXPORT_TEST(testFrameSizeExport, "floating-tables-anchor.docx")
{
    // Make sure the table width is 4000
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tblPr/w:tblW", "w", "4000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf124594, "tdf124594.docx")
{
    xmlDocPtr pDump = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed, as the portion text was
    // only "Er horte leise Schritte hinter", which means the 1st line of the 2nd paragraph was
    // split into two by a Special portion, i.e. the top margin of the shape was too large.
    assertXPath(pDump, "/root/page/body/txt[2]/Text[1]", "Portion",
                "Er horte leise Schritte hinter sich. Das bedeutete nichts Gutes. Wer wurde ihm ");
}

DECLARE_OOXMLEXPORT_TEST(testTextInput, "textinput.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Ensure we have a formfield
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText", " FORMTEXT ");
    // and it's content is not gone
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:t", "SomeText");
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
