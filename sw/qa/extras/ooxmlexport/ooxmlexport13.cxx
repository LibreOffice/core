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
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF2, "tdf121374_sectionHF2.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
}

DECLARE_OOXMLEXPORT_TEST(testendingSectionProps, "endingSectionProps.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT_EQUAL( OUString("General header"), xHeaderText->getString());

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of paragraphs", 2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section is RightToLeft", text::WritingMode2::RL_TB, getProperty<sal_Int16>(xSect, "WritingMode"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(2540), getProperty<sal_Int32>(xSect, "SectionLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testDateControl, "empty-date-control.odt")
{
    // Check that we did not lost the date control
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDraws->getCount());
    uno::Reference<drawing::XControlShape> xControl(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControl->getControl().is());

    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // We need to export date format and a dummy character (" ") for empty date control
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:dateFormat", "val", "dd/MM/yyyy");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t", u" ");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
