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
#include <com/sun/star/style/ParagraphAdjust.hpp>

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

DECLARE_OOXMLEXPORT_TEST(testTdf121867, "tdf121867.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwEditShell* pEditShell = pTextDoc->GetDocShell()->GetEditShell();
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 3; Actual  : 0', i.e. page width zoom was lost on export.
    CPPUNIT_ASSERT_EQUAL(SvxZoomType::PAGEWIDTH, pEditShell->GetViewOptions()->GetZoomType());
}

DECLARE_OOXMLEXPORT_TEST(testParaAdjustDistribute, "para-adjust-distribute.docx")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was
    // left, not block.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(1), "ParaLastLineAdjust")));

    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(2), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(2), "ParaLastLineAdjust")));
}

DECLARE_OOXMLEXPORT_TEST(testInputListExport, "tdf122186_input_list.odt")
{
    // We need to make sure we don't export the text itself next to the input list field
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r", 5);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116371, "tdf116371.odt")
{
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, "RotateAngle"), 10);
    auto frameRect = getProperty<awt::Rectangle>(xShape, "FrameRect");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24070), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24188), frameRect.Width);
}

DECLARE_OOXMLEXPORT_TEST(testFrameSizeExport, "floating-tables-anchor.docx")
{
    // Make sure the table width is 4000
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tblPr/w:tblW", "w", "4000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf119201, "tdf119201.docx")
{
    // Visibility of shapes wasn't imported/exported, for now base printable property on that, too
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_MESSAGE("First shape should be visible.", getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("First shape should be printable.", getProperty<bool>(xShape, "Printable"));
    xShape = getShape(2);
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be visible.", !getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be printable.", !getProperty<bool>(xShape, "Printable"));
    xShape = getShape(3);
    CPPUNIT_ASSERT_MESSAGE("Third shape should be visible.", getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Third shape should be printable.", getProperty<bool>(xShape, "Printable"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
