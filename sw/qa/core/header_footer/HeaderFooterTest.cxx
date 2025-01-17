/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

namespace
{
class HeaderFooterTest : public SwModelTestBase
{
public:
    void checkParagraph(sal_Int32 nNumber, OUString const& rParagraphString,
                        OUString const& rConnectedPageStyle);
    void checkFirstRestHeaderPageStyles();
    void checkLeftRightHeaderPageStyles();
    void checkFirstLeftRightHeaderPageStyles();
    void checkDoubleFirstLeftRightHeaderPageStyles(OUString const& rCustomPageStyleName);
    void checkShapeInFirstPageHeader();

    HeaderFooterTest()
        : SwModelTestBase("/sw/qa/core/header_footer/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testStashedHeaderFooter)
{
    createSwDoc();
    SwDoc* pSourceDocument = getSwDoc();
    uno::Reference<lang::XComponent> xSourceDocument = mxComponent;
    mxComponent.clear();

    createSwDoc();
    SwDoc* pTargetDocument = getSwDoc();
    uno::Reference<lang::XComponent> xTargetDocument = mxComponent;
    mxComponent.clear();

    // Source
    SwPageDesc* pSourcePageDesc = pSourceDocument->MakePageDesc("SourceStyle");
    pSourcePageDesc->ChgFirstShare(false);
    CPPUNIT_ASSERT(!pSourcePageDesc->IsFirstShared());
    pSourcePageDesc->StashFrameFormat(pSourcePageDesc->GetFirstMaster(), true, false, true);
    pSourceDocument->ChgPageDesc("SourceStyle", *pSourcePageDesc);
    CPPUNIT_ASSERT(pSourcePageDesc->HasStashedFormat(true, false, true));

    // Target
    SwPageDesc* pTargetPageDesc = pTargetDocument->MakePageDesc("TargetStyle");

    // Copy source to target
    pTargetDocument->CopyPageDesc(*pSourcePageDesc, *pTargetPageDesc);

    // Check the stashed frame format is copied
    CPPUNIT_ASSERT(pTargetPageDesc->HasStashedFormat(true, false, true));

    // Check document instance
    auto pSourceStashedFormat = pSourcePageDesc->GetStashedFrameFormat(true, false, true);
    CPPUNIT_ASSERT_EQUAL(true, pSourceStashedFormat->GetDoc() == pSourceDocument);

    auto pTargetStashedFormat = pTargetPageDesc->GetStashedFrameFormat(true, false, true);
    CPPUNIT_ASSERT_EQUAL(true, pTargetStashedFormat->GetDoc() == pTargetDocument);

    xSourceDocument->dispose();
    xTargetDocument->dispose();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testNonFirstHeaderIsDisabled)
{
    // related to tdf#127778
    // Header for the first page is enabled, but for the second page it should be disabled or act like it is disabled
    // so the header vertical size is consistent.

    createSwDoc("tdf127778.docx");

    // TODO

    // Header can only be enabled or disabled, but can't be disabled just for first, left or right page.
    // If a header is enabled but empty, the header still takes space in LO, but not in MSO, where it acts the same as
    // if it is disabled.
}

// Check for correct header/footer with special first page with TOC inside
// Related to tdf#118393
CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testHeaderFooterWithSpecialFirstPage_OOXML)
{
    // Load, save in OOXML format and reload
    createSwDoc("tdf118393.odt");
    saveAndReload("Office Open XML Text");

    CPPUNIT_ASSERT_EQUAL(7, getPages());

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // check first page
        assertXPath(pXmlDoc, "/root/page[1]/header"_ostr);
        assertXPath(pXmlDoc, "/root/page[1]/footer"_ostr);
        assertXPath(pXmlDoc, "/root/page[1]/header/txt/text()"_ostr, 0);
        assertXPath(pXmlDoc, "/root/page[1]/footer/txt/text()"_ostr, 0);
        // check second page in the same way
        assertXPath(pXmlDoc, "/root/page[2]/header"_ostr);
        assertXPath(pXmlDoc, "/root/page[2]/footer"_ostr);
    }
    // All other pages should have header/footer
    OUString sExpected("Seite * von *");
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[2]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[3]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[4]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[4]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[5]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[5]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[6]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[6]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[7]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[7]/footer/txt/text()"_ostr));
}

// Check for correct header/footer with special first page with TOC inside
// Related to tdf#118393
CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testHeaderFooterWithSpecialFirstPage_ODF)
{
    // Load, save in ODF format and reload
    createSwDoc("tdf118393.odt");
    saveAndReload("writer8");

    CPPUNIT_ASSERT_EQUAL(7, getPages());

    // First page has no header/footer
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // check first page
        assertXPath(pXmlDoc, "/root/page[1]/header"_ostr, 0);
        assertXPath(pXmlDoc, "/root/page[1]/footer"_ostr, 0);
        // check second page in the same way
        assertXPath(pXmlDoc, "/root/page[2]/header"_ostr);
        assertXPath(pXmlDoc, "/root/page[2]/footer"_ostr);
    }

    // All other pages should have header/footer
    OUString sExpected("Seite * von *");
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[2]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[3]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[4]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[4]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[5]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[5]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[6]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[6]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[7]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(sExpected, parseDump("/root/page[7]/footer/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFloatingTableInHeader)
{
    // Related to tdf#79639
    createSwDoc("tdf79639.docx");

    // This was 0, floating table in header wasn't converted to a TextFrame.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    saveAndReload("Office Open XML Text");

    // This was 0, floating table in header wasn't converted to a TextFrame.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFdo64238_a)
{
    createSwDoc("fdo64238_a.docx");

    // The problem was that when 'Show Only Odd Footer' was marked in Word and the Even footer *was filled*
    // then LO would still import the Even footer and concatenate it to the odd footer.
    // This case specifically is for :
    // 'Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be blank, but instead was the 'even' footer
    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference<text::XTextRange> xFooterParagraph = getParagraphOfText(1, xFooterText);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xFooterParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    sal_Int32 numOfRuns = 0;
    while (xRunEnum->hasMoreElements())
    {
        xRunEnum->nextElement();
        numOfRuns++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), numOfRuns);
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFdo64238_b)
{
    createSwDoc("fdo64238_b.docx");

    // The problem was that when 'Show Only Odd Footer' was marked in Word and the Even footer *was filled*
    // then LO would still import the Even footer and concatenate it to the odd footer.
    // This case specifically is for :
    // 'Non-Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be just the odd footer, but instead was the 'odd' and 'even' footers concatenated
    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference<text::XTextRange> xFooterParagraph = getParagraphOfText(1, xFooterText);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xFooterParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    sal_Int32 numOfRuns = 0;
    while (xRunEnum->hasMoreElements())
    {
        xRunEnum->nextElement();
        numOfRuns++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), numOfRuns);
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstPageHeadersAndEmptyFooters)
{
    // Test for fdo#66145

    // Test case where headers and footers for first page are set, but footers are empty
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(OUString("This is the FIRST page header."),
                             parseDump("/root/page[1]/header/txt/text()"_ostr));

        CPPUNIT_ASSERT_EQUAL(OUString("This is the header for the REST OF THE FILE."),
                             parseDump("/root/page[2]/header/txt/text()"_ostr));

        CPPUNIT_ASSERT_EQUAL(OUString("This is the header for the REST OF THE FILE."),
                             parseDump("/root/page[3]/header/txt/text()"_ostr));
    };

    createSwDoc("fdo66145.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstHeaderFooterImport)
{
    // Test import and export of a section's headerf/footerf properties.
    // (copied from a ww8export test, with doc converted to docx using Word)
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(6, getPages());

        // The document has 6 pages. Note that we don't test if 4 or just 2 page
        // styles are created, the point is that layout should be correct.
        CPPUNIT_ASSERT_EQUAL(OUString("First page header"),
                             parseDump("/root/page[1]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),
                             parseDump("/root/page[1]/footer/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),
                             parseDump("/root/page[2]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),
                             parseDump("/root/page[2]/footer/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),
                             parseDump("/root/page[3]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),
                             parseDump("/root/page[3]/footer/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("First page header2"),
                             parseDump("/root/page[4]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"),
                             parseDump("/root/page[4]/footer/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"),
                             parseDump("/root/page[5]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"),
                             parseDump("/root/page[5]/footer/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),
                             parseDump("/root/page[6]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),
                             parseDump("/root/page[6]/footer/txt/text()"_ostr));
    };

    createSwDoc("first-header-footer.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstHeaderFooterRoundTrip)
{
    createSwDoc("first-header-footerB.odt");
    saveAndReload("Office Open XML Text");

    CPPUNIT_ASSERT_EQUAL(6, getPages());

    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),
                         parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),
                         parseDump("/root/page[1]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),
                         parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),
                         parseDump("/root/page[2]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),
                         parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),
                         parseDump("/root/page[3]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"),
                         parseDump("/root/page[4]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"),
                         parseDump("/root/page[4]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"),
                         parseDump("/root/page[5]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"),
                         parseDump("/root/page[5]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),
                         parseDump("/root/page[6]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),
                         parseDump("/root/page[6]/footer/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstHeaderFooter_ODF)
{
    createSwDoc("first-header-footer.odt");
    saveAndReload("writer8");

    CPPUNIT_ASSERT_EQUAL(6, getPages());
    // Test import and export of the header-first token.

    // The document has 6 pages, two page styles for the first and second half of pages.
    CPPUNIT_ASSERT_EQUAL(OUString("First header"),
                         parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First footer"),
                         parseDump("/root/page[1]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Left header"),
                         parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Left footer"),
                         parseDump("/root/page[2]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Right header"),
                         parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Right footer"),
                         parseDump("/root/page[3]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First header2"),
                         parseDump("/root/page[4]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First footer2"),
                         parseDump("/root/page[4]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Right header2"),
                         parseDump("/root/page[5]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Right footer2"),
                         parseDump("/root/page[5]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Left header2"),
                         parseDump("/root/page[6]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Left footer2"),
                         parseDump("/root/page[6]/footer/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstHeaderFooter_DOC)
{
    createSwDoc("first-header-footer.doc");
    saveAndReload("MS Word 97");

    // Test import and export of a section's headerf/footerf properties.

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),
                         parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),
                         parseDump("/root/page[1]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),
                         parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),
                         parseDump("/root/page[2]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),
                         parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),
                         parseDump("/root/page[3]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"),
                         parseDump("/root/page[4]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"),
                         parseDump("/root/page[4]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"),
                         parseDump("/root/page[5]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"),
                         parseDump("/root/page[5]/footer/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),
                         parseDump("/root/page[6]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),
                         parseDump("/root/page[6]/footer/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFdo65655)
{
    // related - fdo#65655

    // The problem was that the DOCX had a non-blank odd footer and a blank even footer
    // The 'Different Odd & Even Pages' was turned on
    // However - LO assumed that because the 'even' footer is blank - it should ignore the 'Different Odd & Even Pages' flag
    // So it did not import it and did not export it

    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
        {
            bool bValue = false;
            xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
            CPPUNIT_ASSERT_EQUAL(false, bValue);
        }
        {
            bool bValue = false;
            xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
            CPPUNIT_ASSERT_EQUAL(false, bValue);
        }
    };
    createSwDoc("fdo65655.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testImageInHeader)
{
    // Related bug tdf#57155
    createSwDoc("tdf57155.docx");

    // Without the fix in place, the image in the header of page 2 wouldn't exist
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    saveAndReload("Office Open XML Text");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testImageInFooter)
{
    // Related bug tdf#129582
    createSwDoc("tdf129582.docx");

    // Without the fix in place, the image in the footer of page 2 wouldn't exist
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    saveAndReload("Office Open XML Text");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testTdf112694)
{
    auto verify = [this]() {
        uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
        // Header was on when header for file was for explicit first pages only
        // but <w:titlePg> was missing.
        CPPUNIT_ASSERT(!getProperty<bool>(aPageStyle, "HeaderIsOn"));
    };

    createSwDoc("tdf112694.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testContSectBreakHeaderFooter)
{
    auto verify = [this]() {
        // Load a document with a continuous section break on page 2.
        CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 1"),
                             parseDump("/root/page[1]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 1"),
                             parseDump("/root/page[1]/footer/txt/text()"_ostr));

        // Make sure the header stays like this; if we naively just update the page style name of the
        // first para on page 2, then this would be 'Header, section 2', which is incorrect.
        CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 2"),
                             parseDump("/root/page[2]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 2"),
                             parseDump("/root/page[2]/footer/txt/text()"_ostr));

        // This is inherited from page 2.
        CPPUNIT_ASSERT_EQUAL(OUString("Header, section 2"),
                             parseDump("/root/page[3]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(OUString("Footer, section 3"),
                             parseDump("/root/page[3]/footer/txt/text()"_ostr));

        // Without the export fix in place, the import-export-import test would have failed with:
        // - Expected: Header, section 2
        // - Actual  : First page header, section 2
        // i.e. both the header and the footer on page 3 was wrong.

        // Additional problem: top margin on page 3 was wrong.
        if (isExported())
        {
            xmlDocUniquePtr pXml = parseExport("word/document.xml");
            // Without the accompanying fix in place, this test would have failed with:
            // - Expected: 2200
            // - Actual  : 2574
            // i.e. the top margin on page 3 was too large and now matches the value from the input
            // document.
            assertXPath(pXml, "/w:document/w:body/w:sectPr/w:pgMar"_ostr, "top"_ostr, "2200");
        }
    };

    createSwDoc("cont-sect-break-header-footer.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testTdf145998_firstHeader)
{
    createSwDoc("tdf145998_firstHeader.odt");
    saveAndReload("Office Open XML Text");

    // Sanity check - always good to test when dealing with page styles and breaks.
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    CPPUNIT_ASSERT_EQUAL(OUString("Very first header"), parseDump("/root/page[1]/header/txt"_ostr));

    // Page Style is already used in prior section - this can't be the first-header
    CPPUNIT_ASSERT_EQUAL(OUString("Normal Header"), parseDump("/root/page[2]/header/txt"_ostr));
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testEvenPageOddPageFooter_Import)
{
    // Related tdf#135216

    createSwDoc("tdf135216_evenOddFooter.docx");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    // get LO page style for the first page (even page #2)
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);

    xCursor->jumpToFirstPage(); // Even/Left page #2
    uno::Reference<text::XText> xFooter
        = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft");
    CPPUNIT_ASSERT_EQUAL(OUString("even page"), xFooter->getString());

    xCursor->jumpToNextPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    xFooter.set(getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextFirst"));
    CPPUNIT_ASSERT_EQUAL(OUString("odd page - first footer"), xFooter->getString());

    xCursor->jumpToNextPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    xFooter.set(getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("even page"), xFooter->getString());

    // The contents of paragraph 2 should be the page number (2) located on page 1.
    getParagraph(2, "2");
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testEvenPageOddPageFooter_Roundtrip)
{
    // Related tdf#135216

    // Load, save as OOXML and reload
    createSwDoc("tdf135216_evenOddFooter.odt");
    saveAndReload("Office Open XML Text");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    // get LO page style for the first page (even page #2)
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);

    xCursor->jumpToFirstPage(); // Even/Left page #2
    uno::Reference<text::XText> xFooter
        = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft");
    CPPUNIT_ASSERT_EQUAL(OUString("even page"), xFooter->getString());

    xCursor->jumpToNextPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    xFooter.set(getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextFirst"));
    CPPUNIT_ASSERT_EQUAL(OUString("odd page - first footer"), xFooter->getString());

    xCursor->jumpToNextPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    xFooter.set(getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft"));
    CPPUNIT_ASSERT_EQUAL(OUString("even page"), xFooter->getString());

    // The contents of paragraph 2 should be the page number (2) located on page 1.
    getParagraph(2, "2");
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testTdf69635)
{
    createSwDoc("tdf69635.docx");
    saveAndReload("Office Open XML Text");

    xmlDocUniquePtr pXmlHeader1 = parseExport("word/header1.xml");
    CPPUNIT_ASSERT(pXmlHeader1);

    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlSettings);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: "left"
    // - Actual  : "right"
    assertXPathContent(pXmlHeader1, "/w:hdr/w:p/w:r/w:t"_ostr, "left");

    // Make sure "left" appears as a hidden header
    assertXPath(pXmlSettings, "/w:settings/w:evenAndOddHeaders"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testTdf113849_evenAndOddHeaders)
{
    createSwDoc("tdf113849_evenAndOddHeaders.odt");
    saveAndReload("Office Open XML Text");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header2 text", OUString("L. J. Kendall"),
                                 parseDump("/root/page[2]/header/txt"_ostr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer2 text", OUString("*"),
                                 parseDump("/root/page[2]/footer/txt"_ostr));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header3 text", OUString("Shadow Hunt"),
                                 parseDump("/root/page[3]/header/txt"_ostr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer3 text", OUString("*"),
                                 parseDump("/root/page[3]/footer/txt"_ostr));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Header4 text", OUString("L. J. Kendall"),
                                 parseDump("/root/page[4]/header/txt"_ostr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer4 text", OUString("*"),
                                 parseDump("/root/page[4]/footer/txt"_ostr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer5 text", OUString(""),
                                 parseDump("/root/page[5]/footer/txt"_ostr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footer6 text", OUString(""),
                                 parseDump("/root/page[6]/footer/txt"_ostr));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages", 6, getPages());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstPageFooterEnabled)
{
    // tdf#106572
    // Page style with first page and other pages.
    // Footer shouldn't be enabled on first page, but then on the other pages
    auto verify = [this]() {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // TODO
        // It's currently not possible to disable the header on first page only.
        //assertXPath(pXmlDoc, "/root/page[1]/footer/txt"_ostr, 0);
        assertXPath(pXmlDoc, "/root/page[1]/footer/txt"_ostr);
    };
    createSwDoc("TestFirstFooterDisabled.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testBnc519228OddBreaks)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(5, getPages()); // logical pages - "blank page" 4 usually not displayed

        // Check that all the normal styles are not set as right-only, those should be only those used after odd page breaks.
        auto xStyles = getStyles("PageStyles");
        uno::Reference<beans::XPropertySet> xStyle;
        {
            xStyle.set(xStyles->getByName("Standard"), uno::UNO_QUERY);
            auto aPageLayout = xStyle->getPropertyValue("PageStyleLayout");
            CPPUNIT_ASSERT_EQUAL(uno::Any(style::PageStyleLayout_ALL), aPageLayout);
        }
        {
            xStyle.set(xStyles->getByName("First Page"), uno::UNO_QUERY);
            auto aPageLayout = xStyle->getPropertyValue("PageStyleLayout");
            CPPUNIT_ASSERT_EQUAL(uno::Any(style::PageStyleLayout_ALL), aPageLayout);
        }

        uno::Reference<beans::XPropertySet> page1Style;
        {
            auto xPara = getParagraph(1);
            CPPUNIT_ASSERT_EQUAL(OUString("This is the first page."), xPara->getString());
            OUString page1StyleName = getProperty<OUString>(xPara, "PageDescName");
            page1Style.set(xStyles->getByName(page1StyleName), uno::UNO_QUERY);
            auto aPageLayout = page1Style->getPropertyValue("PageStyleLayout");
            CPPUNIT_ASSERT_EQUAL(uno::Any(style::PageStyleLayout_RIGHT), aPageLayout);

            auto xHeaderText = getProperty<uno::Reference<text::XText>>(page1Style, "HeaderText");
            auto xHeaderPara = getParagraphOfText(1, xHeaderText);
            CPPUNIT_ASSERT_EQUAL(OUString("This is the header for odd pages"),
                                 xHeaderPara->getString());
        }

        // Page2 comes from follow of style for page 1 and should be a normal page. Also check the two page style have the same properties,
        // since page style for page1 was created from page style for page 2.
        {
            auto aFollowStyleName = getProperty<OUString>(page1Style, "FollowStyle");

            uno::Reference<beans::XPropertySet> page2Style;
            page2Style.set(xStyles->getByName(aFollowStyleName), uno::UNO_QUERY);
            auto aPage2Layout = page2Style->getPropertyValue("PageStyleLayout");
            CPPUNIT_ASSERT_EQUAL(uno::Any(style::PageStyleLayout_ALL), aPage2Layout);

            auto xHeaderTextLeft
                = getProperty<uno::Reference<text::XText>>(page2Style, "HeaderTextLeft");
            auto xHeaderLeftPara = getParagraphOfText(1, xHeaderTextLeft);
            CPPUNIT_ASSERT_EQUAL(OUString("This is the even header"), xHeaderLeftPara->getString());

            auto xHeaderTextRight
                = getProperty<uno::Reference<text::XText>>(page2Style, "HeaderTextRight");
            auto xHeaderRightPara = getParagraphOfText(1, xHeaderTextRight);
            CPPUNIT_ASSERT_EQUAL(OUString("This is the header for odd pages"),
                                 xHeaderRightPara->getString());

            CPPUNIT_ASSERT_EQUAL(getProperty<sal_Int32>(page1Style, "TopMargin"),
                                 getProperty<sal_Int32>(page2Style, "TopMargin"));
            //tdf164748
            CPPUNIT_ASSERT_EQUAL(getProperty<bool>(page1Style, u"HeaderDynamicSpacing"_ustr),
                                 getProperty<bool>(page2Style, u"HeaderDynamicSpacing"_ustr));
        }

        // Page 5
        {
            auto xPara = getParagraph(4);
            CPPUNIT_ASSERT_EQUAL(
                OUString("Then an odd break after an odd page, should lead us to page #5."),
                xPara->getString());

            OUString page5StyleName = getProperty<OUString>(xPara, "PageDescName");
            uno::Reference<beans::XPropertySet> page5Style(xStyles->getByName(page5StyleName),
                                                           uno::UNO_QUERY);
            auto aPageLayout = page5Style->getPropertyValue("PageStyleLayout");
            CPPUNIT_ASSERT_EQUAL(uno::Any(style::PageStyleLayout_RIGHT), aPageLayout);

            auto xHeaderText = getProperty<uno::Reference<text::XText>>(page5Style, "HeaderText");
            auto xHeaderTextPara = getParagraphOfText(1, xHeaderText);
            CPPUNIT_ASSERT_EQUAL(OUString("This is the header for odd pages"),
                                 xHeaderTextPara->getString());
        }
    };

    createSwDoc("bnc519228_odd-breaksB.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testBnc875718)
{
    createSwDoc("bnc875718.docx");
    //saveAndReload("Office Open XML Text");

    // The frame in the footer must not accidentally end up in the document body.
    // The easiest way for this to test I've found is checking that
    // xray ThisComponent.TextFrames.GetByIndex( index ).Anchor.Text.ImplementationName
    // is not SwXBodyText but rather SwXHeadFootText
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);

    // The sample bugdoc has 3 footer.xml and has a textframe in each. The first one is hidden
    // and it has no text in its anchored text range: it is anchored to body text.
    // At least one text frame should be connected to the header/footer

    sal_Int32 nCheck = 0;
    for (sal_Int32 i = 0; i < xIndexAccess->getCount(); ++i)
    {
        uno::Reference<text::XTextFrame> frame(xIndexAccess->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> range = frame->getAnchor();
        uno::Reference<lang::XServiceInfo> aText(range->getText(), uno::UNO_QUERY);
        OString aMessage("TextFrame " + OString::number(i) + "XText content is empty");
        if (aText.is())
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMessage.getStr(), OUString("SwXHeadFootText"),
                                         aText->getImplementationName());
            nCheck++;
        }
    }
    CPPUNIT_ASSERT(nCheck > 0);

    // Also check that the footer contents are not in the body text.
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> aText = textDocument->getText();
    CPPUNIT_ASSERT(aText.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), aText->getString());
}

// base class to supply a helper method for testHFLinkToPrev
class TestHFBase : public HeaderFooterTest
{
protected:
    OUString getHFText(const uno::Reference<style::XStyle>& xPageStyle, const OUString& sPropName)
    {
        auto xTextRange = getProperty<uno::Reference<text::XTextRange>>(xPageStyle, sPropName);
        return xTextRange->getString();
    }
};

CPPUNIT_TEST_FIXTURE(TestHFBase, testHFLinkToPrev)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");

        // get a page cursor
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
            xModel->getCurrentController(), uno::UNO_QUERY);
        uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                                  uno::UNO_QUERY);

        // get LO page style for page 1, corresponding to docx section 1 first page
        xCursor->jumpToFirstPage();
        OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName(pageStyleName),
                                                 uno::UNO_QUERY);
        // check page 1 header & footer text
        CPPUNIT_ASSERT_EQUAL(OUString("First page header for all sections"),
                             getHFText(xPageStyle, "HeaderTextFirst"));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer for section 1 only"),
                             getHFText(xPageStyle, "FooterTextFirst"));

        // get LO page style for page 2, corresponding to docx section 1
        xCursor->jumpToPage(2);
        pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        // check header & footer text
        CPPUNIT_ASSERT_EQUAL(OUString("Even page header for section 1 only"),
                             getHFText(xPageStyle, "HeaderTextLeft"));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page footer for all sections"),
                             getHFText(xPageStyle, "FooterTextLeft"));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page header for all sections"),
                             getHFText(xPageStyle, "HeaderText"));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer for section 1 only"),
                             getHFText(xPageStyle, "FooterText"));

        // get LO page style for page 4, corresponding to docx section 2 first page
        xCursor->jumpToPage(4);
        pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        // check header & footer text
        CPPUNIT_ASSERT_EQUAL(OUString("First page header for all sections"),
                             getHFText(xPageStyle, "HeaderTextFirst"));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer for sections 2 and 3 only"),
                             getHFText(xPageStyle, "FooterTextFirst"));

        // get LO page style for page 5, corresponding to docx section 2
        xCursor->jumpToPage(5);
        pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        // check header & footer text
        CPPUNIT_ASSERT_EQUAL(OUString("Even page header for sections 2 and 3 only"),
                             getHFText(xPageStyle, "HeaderTextLeft"));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page footer for all sections"),
                             getHFText(xPageStyle, "FooterTextLeft"));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page header for all sections"),
                             getHFText(xPageStyle, "HeaderText"));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer for sections 2 and 3 only"),
                             getHFText(xPageStyle, "FooterText"));

        // get LO page style for page 7, corresponding to docx section 3 first page
        xCursor->jumpToPage(7);
        pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        // check header & footer text
        CPPUNIT_ASSERT_EQUAL(OUString("First page header for all sections"),
                             getHFText(xPageStyle, "HeaderTextFirst"));
        CPPUNIT_ASSERT_EQUAL(OUString("First page footer for sections 2 and 3 only"),
                             getHFText(xPageStyle, "FooterTextFirst"));

        // get LO page style for page 8, corresponding to docx section 3
        xCursor->jumpToPage(8);
        pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
        xPageStyle.set(xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        // check header & footer text
        CPPUNIT_ASSERT_EQUAL(OUString("Even page header for sections 2 and 3 only"),
                             getHFText(xPageStyle, "HeaderTextLeft"));
        CPPUNIT_ASSERT_EQUAL(OUString("Even page footer for all sections"),
                             getHFText(xPageStyle, "FooterTextLeft"));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page header for all sections"),
                             getHFText(xPageStyle, "HeaderText"));
        CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer for sections 2 and 3 only"),
                             getHFText(xPageStyle, "FooterText"));
    };
    createSwDoc("headerfooter-link-to-prev.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testN750935)
{
    auto verify = [this]() {
        // Some page break types were ignores, resulting in less pages.
        CPPUNIT_ASSERT_EQUAL(5, getPages());

        // The problem was that the header and footer was not shared.
        // xray ThisComponent.StyleFamilies.PageStyles.Default.FooterIsShared
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
        bool bValue = false;
        xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
        CPPUNIT_ASSERT_EQUAL(true, bValue);
        xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
        CPPUNIT_ASSERT_EQUAL(true, bValue);
    };
    createSwDoc("n750935.docx");
    verify();
    saveAndReload("Office Open XML Text");
    verify();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testOnlyLeftPageStyle)
{
    createSwDoc("TestPageStyleOnlyLeft.docx");
    saveAndReload("Office Open XML Text");
    // There should be 2 pages - first page should be blank
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testMsoPosition)
{
    auto verifyFooter = [this]() {
        xmlDocUniquePtr doc = parseExport("word/footer2.xml");
        // We write the frames out in different order than they were read, so check it's the correct
        // textbox first by checking width. These tests may need reordering if that gets fixed.
        OUString style1
            = getXPath(doc, "/w:ftr/w:p/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr,
                       "style"_ostr);
        CPPUNIT_ASSERT(style1.indexOf(";width:531pt;") >= 0);
        CPPUNIT_ASSERT(style1.indexOf(";mso-position-vertical-relative:page") >= 0);
        CPPUNIT_ASSERT(style1.indexOf(";mso-position-horizontal-relative:page") >= 0);
        OUString style2
            = getXPath(doc, "/w:ftr/w:p/w:r[4]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr,
                       "style"_ostr);
        CPPUNIT_ASSERT(style2.indexOf(";width:549pt;") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-vertical-relative:text") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-horizontal:center") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-horizontal-relative:text") >= 0);
        OUString style3
            = getXPath(doc, "/w:ftr/w:p/w:r[5]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr,
                       "style"_ostr);
        CPPUNIT_ASSERT(style3.indexOf(";width:36pt;") >= 0);
        CPPUNIT_ASSERT(style3.indexOf(";mso-position-horizontal-relative:text") >= 0);
        CPPUNIT_ASSERT(style3.indexOf(";mso-position-vertical-relative:text") >= 0);
    };

    auto verifyHeader = [this]() {
        xmlDocUniquePtr doc = parseExport("word/header2.xml");
        OUString style1
            = getXPath(doc, "/w:hdr/w:p/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr,
                       "style"_ostr);
        CPPUNIT_ASSERT(style1.indexOf(";width:335.75pt;") >= 0);
        CPPUNIT_ASSERT(style1.indexOf(";mso-position-horizontal-relative:page") >= 0);
        CPPUNIT_ASSERT(style1.indexOf(";mso-position-vertical-relative:page") >= 0);
        OUString style2
            = getXPath(doc, "/w:hdr/w:p/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr,
                       "style"_ostr);
        CPPUNIT_ASSERT(style2.indexOf(";width:138.15pt;") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-horizontal-relative:page") >= 0);
        CPPUNIT_ASSERT(style2.indexOf(";mso-position-vertical-relative:page") >= 0);
        OUString style3
            = getXPath(doc, "/w:hdr/w:p/w:r[4]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr,
                       "style"_ostr);
        CPPUNIT_ASSERT(style3.indexOf(";width:163.8pt;") >= 0);
        CPPUNIT_ASSERT(style3.indexOf(";mso-position-horizontal-relative:page") >= 0);
        CPPUNIT_ASSERT(style3.indexOf(";mso-position-vertical-relative:page") >= 0);
    };

    createSwDoc("bnc884615-mso-position.docx");
    saveAndReload("Office Open XML Text");
    verifyHeader();
    verifyFooter();
}

void HeaderFooterTest::checkParagraph(sal_Int32 nNumber, OUString const& rParagraphString,
                                      OUString const& rConnectedPageStyle)
{
    uno::Reference<beans::XPropertySet> xPropertySet(getParagraphOrTable(nNumber), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xTextRange(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(rParagraphString, xTextRange->getString());

    OUString pageStyle;
    xPropertySet->getPropertyValue("PageStyleName") >>= pageStyle;
    CPPUNIT_ASSERT_EQUAL(rConnectedPageStyle, pageStyle);
}

void HeaderFooterTest::checkFirstRestHeaderPageStyles()
{
    // Page 1
    checkParagraph(1, "P1", "Standard");

    // Page 2
    checkParagraph(2, "P2", "Standard");

    // Page 3
    checkParagraph(3, "P3", "Standard");

    // Check Default Style
    {
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);

        bool bHeader = getProperty<bool>(xPageStyle, "HeaderIsOn");
        CPPUNIT_ASSERT_EQUAL(true, bHeader);

        bool bHeaderIsShared = getProperty<bool>(xPageStyle, "HeaderIsShared");
        CPPUNIT_ASSERT_EQUAL(true, bHeaderIsShared);

        bool bFirstIsShared = getProperty<bool>(xPageStyle, "FirstIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bFirstIsShared);

        auto xHeaderTextFirst
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextFirst");
        CPPUNIT_ASSERT_EQUAL(OUString("FIRST"), xHeaderTextFirst->getString());

        auto xHeaderText = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText");
        CPPUNIT_ASSERT_EQUAL(OUString("NON-FIRST"), xHeaderText->getString());
    }
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstRestHeaderPageStyles_ODF)
{
    createSwDoc("SimpleFirst.odt");
    checkFirstRestHeaderPageStyles();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstRestHeaderPageStyles_OOXML)
{
    createSwDoc("SimpleFirst.docx");
    checkFirstRestHeaderPageStyles();
}

void HeaderFooterTest::checkLeftRightHeaderPageStyles()
{
    // Page 1
    checkParagraph(1, "P1", "Standard");

    // Page 2
    checkParagraph(2, "P2", "Standard");

    // Page 3
    checkParagraph(3, "P3", "Standard");

    // Page 4
    checkParagraph(4, "P4", "Standard");

    // Check Default Style
    {
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);

        bool bHeader = getProperty<bool>(xPageStyle, "HeaderIsOn");
        CPPUNIT_ASSERT_EQUAL(true, bHeader);

        bool bHeaderIsShared = getProperty<bool>(xPageStyle, "HeaderIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bHeaderIsShared);

        bool bFirstIsShared = getProperty<bool>(xPageStyle, "FirstIsShared");
        CPPUNIT_ASSERT_EQUAL(true, bFirstIsShared);

        auto xHeaderTextLeft
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextLeft");
        CPPUNIT_ASSERT_EQUAL(OUString("LEFT"), xHeaderTextLeft->getString());

        auto xHeaderTextRight
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextRight");
        CPPUNIT_ASSERT_EQUAL(OUString("RIGHT"), xHeaderTextRight->getString());
    }
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testLeftRightHeaderPageStyles_ODF)
{
    createSwDoc("SimpleLeftRight.odt");
    checkLeftRightHeaderPageStyles();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testLeftRightHeaderPageStyles_OOXML)
{
    createSwDoc("SimpleLeftRight.docx");
    checkLeftRightHeaderPageStyles();
}

void HeaderFooterTest::checkFirstLeftRightHeaderPageStyles()
{
    // Page 1
    checkParagraph(1, "Para 1", "Standard");

    // Page 2
    checkParagraph(2, "Para 2", "Standard");

    // Page 3
    checkParagraph(3, "Para 3", "Standard");

    // Page 4
    checkParagraph(4, "Para 4", "Standard");

    // Page 5
    checkParagraph(5, "Para 5", "Standard");

    // Check Default Style
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);

    bool bHeader = getProperty<bool>(xPageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT_EQUAL(true, bHeader);

    bool bFooter = getProperty<bool>(xPageStyle, "FooterIsOn");
    CPPUNIT_ASSERT_EQUAL(true, bFooter);

    bool bHeaderIsShared = getProperty<bool>(xPageStyle, "HeaderIsShared");
    CPPUNIT_ASSERT_EQUAL(false, bHeaderIsShared);

    bool bFooterIsShared = getProperty<bool>(xPageStyle, "FooterIsShared");
    CPPUNIT_ASSERT_EQUAL(false, bFooterIsShared);

    bool bFirstIsShared = getProperty<bool>(xPageStyle, "FirstIsShared");
    CPPUNIT_ASSERT_EQUAL(false, bFirstIsShared);

    auto xHeaderTextFirst = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextFirst");
    CPPUNIT_ASSERT_EQUAL(OUString("FIRST HEADER"), xHeaderTextFirst->getString());

    auto xHeaderTextLeft = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextLeft");
    CPPUNIT_ASSERT_EQUAL(OUString("LEFT HEADER"), xHeaderTextLeft->getString());

    auto xHeaderTextRight = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("RIGHT HEADER"), xHeaderTextRight->getString());

    auto xFooterTextFirst = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextFirst");
    CPPUNIT_ASSERT_EQUAL(OUString("FIRST FOOTER"), xFooterTextFirst->getString());

    auto xFooterTextLeft = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft");
    CPPUNIT_ASSERT_EQUAL(OUString("LEFT FOOTER"), xFooterTextLeft->getString());

    auto xFooterTextRight = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL(OUString("RIGHT FOOTER"), xFooterTextRight->getString());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstLeftRightHeaderPageStyles_ODF)
{
    createSwDoc("SimpleFirstLeftRight.odt");
    checkFirstLeftRightHeaderPageStyles();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstLeftRightHeaderPageStyles_OOXML)
{
    createSwDoc("SimpleFirstLeftRight.docx");
    checkFirstLeftRightHeaderPageStyles();
}

void HeaderFooterTest::checkDoubleFirstLeftRightHeaderPageStyles(
    OUString const& rCustomPageStyleName)
{
    // Page 1
    checkParagraph(1, "Para 1 - Default", "Standard");

    // Page 2
    checkParagraph(2, "Para 2 - Default", "Standard");

    // Page 3
    checkParagraph(3, "Para 3 - Default", "Standard");

    // Page 4
    checkParagraph(4, "Para 4 - Custom", rCustomPageStyleName);

    // Page 5
    checkParagraph(5, "Para 5 - Custom", rCustomPageStyleName);

    // Page 6
    checkParagraph(6, "Para 6 - Custom", rCustomPageStyleName);

    // Check Default Style
    {
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);

        bool bHeader = getProperty<bool>(xPageStyle, "HeaderIsOn");
        CPPUNIT_ASSERT_EQUAL(true, bHeader);

        bool bFooter = getProperty<bool>(xPageStyle, "FooterIsOn");
        CPPUNIT_ASSERT_EQUAL(true, bFooter);

        bool bHeaderIsShared = getProperty<bool>(xPageStyle, "HeaderIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bHeaderIsShared);

        bool bFooterIsShared = getProperty<bool>(xPageStyle, "FooterIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bFooterIsShared);

        bool bFirstIsShared = getProperty<bool>(xPageStyle, "FirstIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bFirstIsShared);

        auto xHeaderTextFirst
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextFirst");
        CPPUNIT_ASSERT_EQUAL(OUString("FIRST HEADER"), xHeaderTextFirst->getString());

        auto xHeaderTextLeft
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextLeft");
        CPPUNIT_ASSERT_EQUAL(OUString("LEFT HEADER"), xHeaderTextLeft->getString());

        auto xHeaderTextRight = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText");
        CPPUNIT_ASSERT_EQUAL(OUString("RIGHT HEADER"), xHeaderTextRight->getString());

        auto xFooterTextFirst
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextFirst");
        CPPUNIT_ASSERT_EQUAL(OUString("FIRST FOOTER"), xFooterTextFirst->getString());

        auto xFooterTextLeft
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft");
        CPPUNIT_ASSERT_EQUAL(OUString("LEFT FOOTER"), xFooterTextLeft->getString());

        auto xFooterTextRight = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterText");
        CPPUNIT_ASSERT_EQUAL(OUString("RIGHT FOOTER"), xFooterTextRight->getString());
    }

    {
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles("PageStyles")->getByName(rCustomPageStyleName), uno::UNO_QUERY);

        bool bHeader = getProperty<bool>(xPageStyle, "HeaderIsOn");
        CPPUNIT_ASSERT_EQUAL(true, bHeader);

        bool bFooter = getProperty<bool>(xPageStyle, "FooterIsOn");
        CPPUNIT_ASSERT_EQUAL(true, bFooter);

        bool bHeaderIsShared = getProperty<bool>(xPageStyle, "HeaderIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bHeaderIsShared);

        bool bFooterIsShared = getProperty<bool>(xPageStyle, "FooterIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bFooterIsShared);

        bool bFirstIsShared = getProperty<bool>(xPageStyle, "FirstIsShared");
        CPPUNIT_ASSERT_EQUAL(false, bFirstIsShared);

        auto xHeaderTextFirst
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextFirst");
        CPPUNIT_ASSERT_EQUAL(OUString("FIRST HEADER - CUSTOM"), xHeaderTextFirst->getString());

        auto xHeaderTextLeft
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextLeft");
        CPPUNIT_ASSERT_EQUAL(OUString("LEFT HEADER - CUSTOM"), xHeaderTextLeft->getString());

        auto xHeaderTextRight = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText");
        CPPUNIT_ASSERT_EQUAL(OUString("RIGHT HEADER - CUSTOM"), xHeaderTextRight->getString());

        auto xFooterTextFirst
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextFirst");
        CPPUNIT_ASSERT_EQUAL(OUString("FIRST FOOTER - CUSTOM"), xFooterTextFirst->getString());

        auto xFooterTextLeft
            = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterTextLeft");
        CPPUNIT_ASSERT_EQUAL(OUString("LEFT FOOTER - CUSTOM"), xFooterTextLeft->getString());

        auto xFooterTextRight = getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterText");
        CPPUNIT_ASSERT_EQUAL(OUString("RIGHT FOOTER - CUSTOM"), xFooterTextRight->getString());
    }
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testDoubleFirstLeftRightHeaderPageStyles_ODF)
{
    createSwDoc("DoubleFirstLeftRight.odt");
    checkDoubleFirstLeftRightHeaderPageStyles("Custom Page Style");
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testDoubleFirstLeftRightHeaderPageStyles_OOXML)
{
    createSwDoc("DoubleFirstLeftRight.docx");
    checkDoubleFirstLeftRightHeaderPageStyles("Converted1");
}

void HeaderFooterTest::checkShapeInFirstPageHeader()
{
    // Check Default Style
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);

    bool bHeader = getProperty<bool>(xPageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT_EQUAL(true, bHeader);

    bool bFooter = getProperty<bool>(xPageStyle, "FooterIsOn");
    CPPUNIT_ASSERT_EQUAL(false, bFooter);

    bool bHeaderIsShared = getProperty<bool>(xPageStyle, "HeaderIsShared");
    CPPUNIT_ASSERT_EQUAL(true, bHeaderIsShared);

    bool bFirstIsShared = getProperty<bool>(xPageStyle, "FirstIsShared");
    CPPUNIT_ASSERT_EQUAL(false, bFirstIsShared);

    // Check shape is anchored to paragraph belonging to XText from "HeaderTextFirst" property
    auto xHeaderTextFirst = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderTextFirst");
    CPPUNIT_ASSERT(xHeaderTextFirst.is());

    uno::Reference<text::XTextContent> xShapeText(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeText.is());
    uno::Reference<text::XTextRange> xAnchor = xShapeText->getAnchor();
    CPPUNIT_ASSERT(xAnchor.is());

    CPPUNIT_ASSERT_EQUAL(xHeaderTextFirst, xAnchor->getText());
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstPageHeaderShape_ODF)
{
    createSwDoc("FirstPageHeaderShape.odt");
    checkShapeInFirstPageHeader();
}

CPPUNIT_TEST_FIXTURE(HeaderFooterTest, testFirstPageHeaderShape_OOXML)
{
    createSwDoc("FirstPageHeaderShape.docx");
    checkShapeInFirstPageHeader();
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
