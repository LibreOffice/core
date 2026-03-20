/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editdata.hxx>
#include <svl/intitem.hxx>
#include <sot/exchange.hxx>
#include <sfx2/app.hxx>

#include <editdoc.hxx>
#include <eeobj.hxx>

using namespace com::sun::star;

namespace
{
/// Covers editeng/source/editeng/ markdown fixes.
class MarkdownTest : public test::BootstrapFixture
{
public:
    MarkdownTest() {}

    void setUp() override
    {
        test::BootstrapFixture::setUp();
        mpItemPool = new EditEngineItemPool();
        SfxApplication::GetOrCreate();
    }

    void tearDown() override
    {
        mpItemPool.clear();
        test::BootstrapFixture::tearDown();
    }

protected:
    rtl::Reference<EditEngineItemPool> mpItemPool;

    std::string exportAsMarkdown(EditEngine& rEngine)
    {
        sal_Int32 nParas = rEngine.GetParagraphCount();
        sal_Int32 nLastLen = rEngine.GetText(nParas - 1).getLength();
        uno::Reference<datatransfer::XTransferable> xData
            = rEngine.CreateTransferable(ESelection(0, 0, nParas - 1, nLastLen));
        auto pData = dynamic_cast<EditDataObject*>(xData.get());
        SvMemoryStream& rStream = pData->GetMarkdownStream();
        return std::string(static_cast<const char*>(rStream.GetData()),
                           static_cast<size_t>(rStream.GetSize()));
    }

    void importMarkdown(EditEngine& rEngine, std::string_view aMd)
    {
        SvMemoryStream aStream(const_cast<char*>(aMd.data()), aMd.size(), StreamMode::READ);
        rEngine.Read(aStream, u""_ustr, EETextFormat::Markdown);
    }
};

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportPlainText)
{
    // Given a document with plain text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello world"_ustr);

    // When exporting as markdown via CreateTransferable:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the markdown stream should contain the plain text:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportBold)
{
    // Given a document with bold text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello bold world"_ustr);
    // Apply bold to "bold"
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 6, 0, 10));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the markdown should wrap "bold" in **:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello **bold** world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportItalic)
{
    // Given a document with italic text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello italic world"_ustr);
    // Apply italic to "italic"
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 6, 0, 12));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the markdown should wrap "italic" in *:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello *italic* world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportStrikethrough)
{
    // Given a document with strikethrough text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello struck world"_ustr);
    // Apply strikethrough to "struck"
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxCrossedOutItem(STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 6, 0, 12));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the markdown should wrap "struck" in ~~:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello ~~struck~~ world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportBoldItalic)
{
    // Given a document with bold+italic text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello both world"_ustr);
    // Apply bold+italic to "both"
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aSet.Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 6, 0, 10));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the markdown should wrap "both" in ***:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello ***both*** world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportMultiParagraph)
{
    // Given a document with two paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"First paragraph\nSecond paragraph"_ustr);

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then paragraphs should be separated by double newlines:
    CPPUNIT_ASSERT_EQUAL(std::string("First paragraph\n\nSecond paragraph"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportSpecialChars)
{
    // Given a document with markdown special characters:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Use *asterisks* and [brackets]"_ustr);

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then special chars should be escaped:
    CPPUNIT_ASSERT_EQUAL(std::string("Use \\*asterisks\\* and \\[brackets\\]"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportLink)
{
    // Given a document with a URL field:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Click  please"_ustr);
    // Insert a URL field at position 6
    SvxURLField aURLField(u"https://example.com"_ustr, u"here"_ustr, SvxURLFormat::Repr);
    SvxFieldItem aField(aURLField, EE_FEATURE_FIELD);
    aEditEngine.QuickInsertField(aField, ESelection(0, 6, 0, 6));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the markdown should contain [text](url):
    CPPUNIT_ASSERT(aMd.find("[here](https://example.com)") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportBold)
{
    // Given markdown with bold text:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Hello **bold** world");

    // Then the text should have bold formatting on "bold":
    CPPUNIT_ASSERT_EQUAL(u"Hello bold world"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 6, 10, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
    // Text before should not be bold
    SfxItemSet aPreAttribs = aEditEngine.GetAttribs(0, 0, 5, GetAttribsFlags::CHARATTRIBS);
    auto& rPreWeight = aPreAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT(rPreWeight.GetWeight() != WEIGHT_BOLD);
    // Text after should not be bold
    SfxItemSet aPostAttribs = aEditEngine.GetAttribs(0, 11, 16, GetAttribsFlags::CHARATTRIBS);
    auto& rPostWeight = aPostAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT(rPostWeight.GetWeight() != WEIGHT_BOLD);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportBoldInListItem)
{
    // Given markdown with bold text inside a list item:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "- **Bold**: Normal text");

    // Then "Bold" (indices 0-4) should be bold:
    CPPUNIT_ASSERT_EQUAL(u"Bold: Normal text"_ustr, aEditEngine.GetText(0));
    SfxItemSet aBoldAttribs = aEditEngine.GetAttribs(0, 0, 4, GetAttribsFlags::CHARATTRIBS);
    auto& rBoldWeight = aBoldAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rBoldWeight.GetWeight());
    // Text after bold (": Normal text") should NOT be bold:
    SfxItemSet aAfterAttribs = aEditEngine.GetAttribs(0, 4, 17, GetAttribsFlags::CHARATTRIBS);
    auto& rAfterWeight = aAfterAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT(rAfterWeight.GetWeight() != WEIGHT_BOLD);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportItalic)
{
    // Given markdown with italic text:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Hello *italic* world");

    // Then the text should have italic formatting on "italic":
    CPPUNIT_ASSERT_EQUAL(u"Hello italic world"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 6, 12, GetAttribsFlags::CHARATTRIBS);
    auto& rPosture = aAttribs.Get(EE_CHAR_ITALIC);
    CPPUNIT_ASSERT(rPosture.GetPosture() == ITALIC_NORMAL
                   || rPosture.GetPosture() == ITALIC_OBLIQUE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportStrikethrough)
{
    // Given markdown with strikethrough text:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Hello ~~struck~~ world");

    // Then the text should have strikethrough formatting on "struck":
    CPPUNIT_ASSERT_EQUAL(u"Hello struck world"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 6, 12, GetAttribsFlags::CHARATTRIBS);
    auto& rStrikeout = aAttribs.Get(EE_CHAR_STRIKEOUT);
    CPPUNIT_ASSERT(rStrikeout.GetStrikeout() != STRIKEOUT_NONE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportMultiParagraph)
{
    // Given markdown with multiple paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "First paragraph\n\nSecond paragraph");

    // Then there should be two paragraphs:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"First paragraph"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Second paragraph"_ustr, aEditEngine.GetText(1));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportLooseOrderedList)
{
    // Given a loose ordered list (blank lines between items):
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "1. First\n\n2. Second\n\n3. Third");

    // Then there should be exactly three paragraphs (one per list item):
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, aEditEngine.GetText(1));
    CPPUNIT_ASSERT_EQUAL(u"Third"_ustr, aEditEngine.GetText(2));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtrip)
{
    // Given a document with bold and italic text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Normal and formatted text"_ustr);
    // Apply bold to "formatted"
    SfxItemSet aBoldSet(aEditEngine.GetEmptyItemSet());
    aBoldSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aEditEngine.QuickSetAttribs(aBoldSet, ESelection(0, 11, 0, 20));

    // Export as markdown:
    std::string aMdExport = exportAsMarkdown(aEditEngine);
    CPPUNIT_ASSERT_EQUAL(std::string("Normal and **formatted** text"), aMdExport);

    // Import back into a fresh EditEngine:
    EditEngine aEditEngine2(mpItemPool.get());
    importMarkdown(aEditEngine2, aMdExport);

    // Then the text and formatting should be preserved:
    CPPUNIT_ASSERT_EQUAL(u"Normal and formatted text"_ustr, aEditEngine2.GetText(0));
    SfxItemSet aAttribs = aEditEngine2.GetAttribs(0, 11, 20, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownTransferableSupport)
{
    // Given a document with text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"test"_ustr);

    // When creating a transferable:
    uno::Reference<datatransfer::XTransferable> xData
        = aEditEngine.CreateTransferable(ESelection(0, 0, 0, 4));

    // Then the MARKDOWN format should be supported:
    auto pData = dynamic_cast<EditDataObject*>(xData.get());
    CPPUNIT_ASSERT(pData != nullptr);
    SvMemoryStream& rStream = pData->GetMarkdownStream();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_uInt64>(0), rStream.GetSize());

    // And the transferable should report MARKDOWN as a supported flavor:
    css::datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::MARKDOWN, aFlavor);
    CPPUNIT_ASSERT(xData->isDataFlavorSupported(aFlavor));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportBoldItalic)
{
    // Given markdown with bold+italic text:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Hello ***both*** world");

    // Then "both" should have bold and italic formatting:
    CPPUNIT_ASSERT_EQUAL(u"Hello both world"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 6, 10, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
    auto& rPosture = aAttribs.Get(EE_CHAR_ITALIC);
    CPPUNIT_ASSERT(rPosture.GetPosture() == ITALIC_NORMAL
                   || rPosture.GetPosture() == ITALIC_OBLIQUE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHeadingH1)
{
    // Given markdown with a level-1 heading:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "# Heading One");

    // Then the text should be bold with 180% scaled font height:
    CPPUNIT_ASSERT_EQUAL(u"Heading One"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 0, 11, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
    sal_uInt32 nDefaultHeight = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight();
    auto& rHeight = aAttribs.Get(EE_CHAR_FONTHEIGHT);
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(180) / sal_uInt32(100), rHeight.GetHeight());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHeadingH2)
{
    // Given markdown with a level-2 heading:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "## Heading Two");

    // Then the text should be bold with 150% scaled font height:
    CPPUNIT_ASSERT_EQUAL(u"Heading Two"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 0, 11, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
    sal_uInt32 nDefaultHeight = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight();
    auto& rHeight = aAttribs.Get(EE_CHAR_FONTHEIGHT);
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(150) / sal_uInt32(100), rHeight.GetHeight());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHeadingH3)
{
    // Given markdown with a level-3 heading:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "### Heading Three");

    // Then the text should be bold with 130% scaled font height:
    CPPUNIT_ASSERT_EQUAL(u"Heading Three"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 0, 13, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
    sal_uInt32 nDefaultHeight = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight();
    auto& rHeight = aAttribs.Get(EE_CHAR_FONTHEIGHT);
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(130) / sal_uInt32(100), rHeight.GetHeight());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHeadingH5NoScale)
{
    // Given markdown with a level-5 heading:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "##### Heading Five");

    // Then the text should be bold but font height should be default (no scaling for H5+):
    CPPUNIT_ASSERT_EQUAL(u"Heading Five"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 0, 12, GetAttribsFlags::CHARATTRIBS);
    auto& rWeight = aAttribs.Get(EE_CHAR_WEIGHT);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeight.GetWeight());
    sal_uInt32 nDefaultHeight = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight();
    auto& rHeight = aAttribs.Get(EE_CHAR_FONTHEIGHT);
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight, rHeight.GetHeight());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportInlineCode)
{
    // Given markdown with inline code:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Use `printf()` to print");

    // Then "printf()" should have monospace font:
    CPPUNIT_ASSERT_EQUAL(u"Use printf() to print"_ustr, aEditEngine.GetText(0));
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 4, 12, GetAttribsFlags::CHARATTRIBS);
    auto& rFont = aAttribs.Get(EE_CHAR_FONTINFO);
    CPPUNIT_ASSERT(rFont.GetFamily() == FAMILY_MODERN || rFont.GetPitch() == PITCH_FIXED);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportCodeBlock)
{
    // Given markdown with a fenced code block:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "```\nint x = 5;\n```");

    // Then the code text should have monospace font:
    OUString aText = aEditEngine.GetText(0);
    CPPUNIT_ASSERT(aText.indexOf(u"int x = 5;") >= 0);
    SfxItemSet aAttribs
        = aEditEngine.GetAttribs(0, 0, aText.getLength(), GetAttribsFlags::CHARATTRIBS);
    auto& rFont = aAttribs.Get(EE_CHAR_FONTINFO);
    CPPUNIT_ASSERT(rFont.GetFamily() == FAMILY_MODERN || rFont.GetPitch() == PITCH_FIXED);

    // And the code text should have a gray background:
    auto& rBkgColor = aAttribs.Get(EE_CHAR_BKGCOLOR);
    CPPUNIT_ASSERT_EQUAL(Color(225, 225, 225), rBkgColor.GetValue());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportCodeBlockMultiLine)
{
    // Given markdown with a multi-line fenced code block (should not crash):
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "```\nline1\nline2\nline3\n```");

    // Then all code paragraphs should exist and have monospace font:
    CPPUNIT_ASSERT_EQUAL(u"line1"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT(aEditEngine.GetParagraphCount() >= 3);
    SfxItemSet aAttribs = aEditEngine.GetAttribs(0, 0, 5, GetAttribsFlags::CHARATTRIBS);
    auto& rFont = aAttribs.Get(EE_CHAR_FONTINFO);
    CPPUNIT_ASSERT(rFont.GetFamily() == FAMILY_MODERN || rFont.GetPitch() == PITCH_FIXED);

    // And the code text should have a gray background:
    auto& rBkgColor = aAttribs.Get(EE_CHAR_BKGCOLOR);
    CPPUNIT_ASSERT_EQUAL(Color(225, 225, 225), rBkgColor.GetValue());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportBlockquote)
{
    // Given markdown with a blockquote:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "> Quoted text");

    // Then the paragraph should have a left indent of 720 twips:
    CPPUNIT_ASSERT_EQUAL(u"Quoted text"_ustr, aEditEngine.GetText(0));
    SfxItemSet aParaAttribs = aEditEngine.GetParaAttribs(0);
    auto& rLR = aParaAttribs.Get(EE_PARA_LRSPACE);
    CPPUNIT_ASSERT_EQUAL(720.0, rLR.GetTextLeft().m_dValue);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportNestedBlockquote)
{
    // Given markdown with a nested blockquote:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, ">> Deeply quoted");

    // Then the paragraph should have a left indent of 1440 twips (2 * 720):
    CPPUNIT_ASSERT_EQUAL(u"Deeply quoted"_ustr, aEditEngine.GetText(0));
    SfxItemSet aParaAttribs = aEditEngine.GetParaAttribs(0);
    auto& rLR = aParaAttribs.Get(EE_PARA_LRSPACE);
    CPPUNIT_ASSERT_EQUAL(1440.0, rLR.GetTextLeft().m_dValue);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportUnorderedList)
{
    // Given markdown with an unordered list:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "- First\n- Second");

    // Then there should be two list item paragraphs with bullet numbering:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, aEditEngine.GetText(1));

    // Check outline level
    SfxItemSet aParaAttribs0 = aEditEngine.GetParaAttribs(0);
    auto& rLevel0 = aParaAttribs0.Get(EE_PARA_OUTLLEVEL);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rLevel0.GetValue());

    // Check numbering type is bullet
    auto& rNumBullet0 = aParaAttribs0.Get(EE_PARA_NUMBULLET);
    const SvxNumRule& rRule0 = rNumBullet0.GetNumRule();
    const SvxNumberFormat* pFmt0 = rRule0.Get(0);
    CPPUNIT_ASSERT(pFmt0 != nullptr);
    CPPUNIT_ASSERT_EQUAL(SVX_NUM_CHAR_SPECIAL, pFmt0->GetNumberingType());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportOrderedList)
{
    // Given markdown with an ordered list:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "1. Alpha\n2. Beta");

    // Then there should be two list item paragraphs with arabic numbering:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Alpha"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Beta"_ustr, aEditEngine.GetText(1));

    // Check outline level
    SfxItemSet aParaAttribs0 = aEditEngine.GetParaAttribs(0);
    auto& rLevel0 = aParaAttribs0.Get(EE_PARA_OUTLLEVEL);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rLevel0.GetValue());

    // Check numbering type is arabic
    auto& rNumBullet0 = aParaAttribs0.Get(EE_PARA_NUMBULLET);
    const SvxNumRule& rRule0 = rNumBullet0.GetNumRule();
    const SvxNumberFormat* pFmt0 = rRule0.Get(0);
    CPPUNIT_ASSERT(pFmt0 != nullptr);
    CPPUNIT_ASSERT_EQUAL(SVX_NUM_ARABIC, pFmt0->GetNumberingType());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportLink)
{
    // Given markdown with a link:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Click [here](https://example.com) now");

    // Then a URL field should be present with the correct URL and representation:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aEditEngine.GetParagraphCount());
    // Export back to markdown to verify the link survives:
    std::string aMdOut = exportAsMarkdown(aEditEngine);
    CPPUNIT_ASSERT(aMdOut.find("[here](https://example.com)") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHorizontalRule)
{
    // Given markdown with a horizontal rule:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Before\n\n---\n\nAfter");

    // Then the middle paragraph should contain horizontal bar characters (U+2015):
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Before"_ustr, aEditEngine.GetText(0));
    OUString aMiddle = aEditEngine.GetText(1);
    CPPUNIT_ASSERT(aMiddle.indexOf(u'\u2015') >= 0);
    CPPUNIT_ASSERT_EQUAL(u"After"_ustr, aEditEngine.GetText(2));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHtmlEntity)
{
    // Given markdown with HTML entities:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Tom &amp; Jerry");

    // Then entities should be decoded:
    CPPUNIT_ASSERT_EQUAL(u"Tom & Jerry"_ustr, aEditEngine.GetText(0));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportSoftBreak)
{
    // Given markdown with a soft break (single newline within paragraph):
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Line one\nLine two");

    // Then the soft break should become a space in a single paragraph:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Line one Line two"_ustr, aEditEngine.GetText(0));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportNestedFormatting)
{
    // Given markdown with nested bold and italic:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "**bold *bolditalic* bold**");

    // Then "bold " should be bold-only:
    CPPUNIT_ASSERT_EQUAL(u"bold bolditalic bold"_ustr, aEditEngine.GetText(0));
    SfxItemSet aBoldAttribs = aEditEngine.GetAttribs(0, 0, 5, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aBoldAttribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, aBoldAttribs.Get(EE_CHAR_ITALIC).GetPosture());

    // And "bolditalic" should be bold+italic:
    SfxItemSet aBothAttribs = aEditEngine.GetAttribs(0, 5, 15, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aBothAttribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT(aBothAttribs.Get(EE_CHAR_ITALIC).GetPosture() == ITALIC_NORMAL
                   || aBothAttribs.Get(EE_CHAR_ITALIC).GetPosture() == ITALIC_OBLIQUE);

    // And " bold" should be bold-only again:
    SfxItemSet aEndAttribs = aEditEngine.GetAttribs(0, 15, 20, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aEndAttribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, aEndAttribs.Get(EE_CHAR_ITALIC).GetPosture());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportMultipleHeadings)
{
    // Given markdown with multiple heading levels:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "# Title\n\n## Subtitle\n\nBody");

    // Then there should be three paragraphs with appropriate formatting:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Subtitle"_ustr, aEditEngine.GetText(1));
    CPPUNIT_ASSERT_EQUAL(u"Body"_ustr, aEditEngine.GetText(2));

    sal_uInt32 nDefaultHeight = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight();

    // H1: bold + 180% height
    SfxItemSet aH1Attribs = aEditEngine.GetAttribs(0, 0, 5, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aH1Attribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(180) / sal_uInt32(100),
                         aH1Attribs.Get(EE_CHAR_FONTHEIGHT).GetHeight());

    // H2: bold + 150% height
    SfxItemSet aH2Attribs = aEditEngine.GetAttribs(1, 0, 8, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aH2Attribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(150) / sal_uInt32(100),
                         aH2Attribs.Get(EE_CHAR_FONTHEIGHT).GetHeight());

    // Body: not bold, default height
    SfxItemSet aBodyAttribs = aEditEngine.GetAttribs(2, 0, 4, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT(aBodyAttribs.Get(EE_CHAR_WEIGHT).GetWeight() != WEIGHT_BOLD);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportInlineCode)
{
    // Given a document with monospace text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Use printf() to print"_ustr);
    // Apply monospace font to "printf()"
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxFontItem(FAMILY_MODERN, u"Courier New"_ustr, u""_ustr, PITCH_FIXED,
                         RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 4, 0, 12));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the monospace text should be wrapped in backticks:
    CPPUNIT_ASSERT_EQUAL(std::string("Use `printf()` to print"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportInlineCodeNoEscaping)
{
    // Given a document with monospace text containing markdown special chars:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Use *special* chars"_ustr);
    // Apply monospace font to "*special*"
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxFontItem(FAMILY_MODERN, u"Courier New"_ustr, u""_ustr, PITCH_FIXED,
                         RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 4, 0, 13));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then special chars inside backticks should NOT be escaped:
    CPPUNIT_ASSERT_EQUAL(std::string("Use `*special*` chars"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportOrderedListEscaping)
{
    // Given a document with text that looks like an ordered list but isn't:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"1. Not a list"_ustr);

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the period after the digit should be escaped:
    CPPUNIT_ASSERT_EQUAL(std::string("1\\. Not a list"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportStartOfLineEscaping)
{
    // Given a document with text starting with a markdown heading marker:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"# Not a heading"_ustr);

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the # at start of line should be escaped:
    CPPUNIT_ASSERT_EQUAL(std::string("\\# Not a heading"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportBackslashEscaping)
{
    // Given a document with backslashes:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"path\\to\\file"_ustr);

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then backslashes should be escaped:
    CPPUNIT_ASSERT_EQUAL(std::string("path\\\\to\\\\file"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripItalic)
{
    // Given a document with italic text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Normal and styled text"_ustr);
    SfxItemSet aItalicSet(aEditEngine.GetEmptyItemSet());
    aItalicSet.Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC));
    aEditEngine.QuickSetAttribs(aItalicSet, ESelection(0, 11, 0, 17));

    // When exporting and reimporting as markdown:
    std::string aMdExport = exportAsMarkdown(aEditEngine);
    CPPUNIT_ASSERT_EQUAL(std::string("Normal and *styled* text"), aMdExport);

    EditEngine aEditEngine2(mpItemPool.get());
    importMarkdown(aEditEngine2, aMdExport);

    // Then the text and italic formatting should be preserved:
    CPPUNIT_ASSERT_EQUAL(u"Normal and styled text"_ustr, aEditEngine2.GetText(0));
    SfxItemSet aAttribs = aEditEngine2.GetAttribs(0, 11, 17, GetAttribsFlags::CHARATTRIBS);
    auto& rPosture = aAttribs.Get(EE_CHAR_ITALIC);
    CPPUNIT_ASSERT(rPosture.GetPosture() == ITALIC_NORMAL
                   || rPosture.GetPosture() == ITALIC_OBLIQUE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripStrikethrough)
{
    // Given a document with strikethrough text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Normal and struck text"_ustr);
    SfxItemSet aStrikeSet(aEditEngine.GetEmptyItemSet());
    aStrikeSet.Put(SvxCrossedOutItem(STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT));
    aEditEngine.QuickSetAttribs(aStrikeSet, ESelection(0, 11, 0, 17));

    // When exporting and reimporting as markdown:
    std::string aMdExport = exportAsMarkdown(aEditEngine);
    CPPUNIT_ASSERT_EQUAL(std::string("Normal and ~~struck~~ text"), aMdExport);

    EditEngine aEditEngine2(mpItemPool.get());
    importMarkdown(aEditEngine2, aMdExport);

    // Then the text and strikethrough formatting should be preserved:
    CPPUNIT_ASSERT_EQUAL(u"Normal and struck text"_ustr, aEditEngine2.GetText(0));
    SfxItemSet aAttribs = aEditEngine2.GetAttribs(0, 11, 17, GetAttribsFlags::CHARATTRIBS);
    auto& rStrikeout = aAttribs.Get(EE_CHAR_STRIKEOUT);
    CPPUNIT_ASSERT(rStrikeout.GetStrikeout() != STRIKEOUT_NONE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripMultiParagraph)
{
    // Given a document with two paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"First paragraph\nSecond paragraph"_ustr);

    // When exporting and reimporting as markdown:
    std::string aMdExport = exportAsMarkdown(aEditEngine);

    EditEngine aEditEngine2(mpItemPool.get());
    importMarkdown(aEditEngine2, aMdExport);

    // Then both paragraphs should be preserved:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEditEngine2.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"First paragraph"_ustr, aEditEngine2.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Second paragraph"_ustr, aEditEngine2.GetText(1));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripBoldItalic)
{
    // Given a document with bold+italic text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Normal and styled text"_ustr);
    SfxItemSet aBothSet(aEditEngine.GetEmptyItemSet());
    aBothSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aBothSet.Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC));
    aEditEngine.QuickSetAttribs(aBothSet, ESelection(0, 11, 0, 17));

    // When exporting and reimporting as markdown:
    std::string aMdExport = exportAsMarkdown(aEditEngine);
    CPPUNIT_ASSERT_EQUAL(std::string("Normal and ***styled*** text"), aMdExport);

    EditEngine aEditEngine2(mpItemPool.get());
    importMarkdown(aEditEngine2, aMdExport);

    // Then both bold and italic should be preserved:
    CPPUNIT_ASSERT_EQUAL(u"Normal and styled text"_ustr, aEditEngine2.GetText(0));
    SfxItemSet aAttribs = aEditEngine2.GetAttribs(0, 11, 17, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aAttribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT(aAttribs.Get(EE_CHAR_ITALIC).GetPosture() == ITALIC_NORMAL
                   || aAttribs.Get(EE_CHAR_ITALIC).GetPosture() == ITALIC_OBLIQUE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripLink)
{
    // Given a document with a URL field:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Click  please"_ustr);
    SvxURLField aURLField(u"https://example.com"_ustr, u"here"_ustr, SvxURLFormat::Repr);
    SvxFieldItem aField(aURLField, EE_FEATURE_FIELD);
    aEditEngine.QuickInsertField(aField, ESelection(0, 6, 0, 6));

    // When exporting and reimporting as markdown:
    std::string aMdExport = exportAsMarkdown(aEditEngine);
    CPPUNIT_ASSERT(aMdExport.find("[here](https://example.com)") != std::string::npos);

    EditEngine aEditEngine2(mpItemPool.get());
    importMarkdown(aEditEngine2, aMdExport);

    // Then re-export should still contain the link:
    std::string aMdRoundtrip = exportAsMarkdown(aEditEngine2);
    CPPUNIT_ASSERT(aMdRoundtrip.find("[here](https://example.com)") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportEmptyDocument)
{
    // Given an empty markdown string:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "");

    // Then there should be one empty paragraph (no crash):
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aEditEngine.GetText(0));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportOnlyWhitespace)
{
    // Given markdown with only whitespace:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "   \n\n   ");

    // Then the import should complete without crashing:
    CPPUNIT_ASSERT(aEditEngine.GetParagraphCount() >= 1);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportUnorderedList)
{
    // Given a document with two unordered list paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"First\nSecond"_ustr);

    // Set list attributes on both paragraphs
    for (sal_Int32 nPara = 0; nPara < 2; nPara++)
    {
        SfxItemSet aParaAttribs(aEditEngine.GetParaAttribs(nPara));
        aParaAttribs.Put(SfxInt16Item(EE_PARA_OUTLLEVEL, sal_Int16(0)));
        SvxNumRule aRule(SvxNumRuleFlags::BULLET_REL_SIZE, 10, false);
        SvxNumberFormat aFmt(SVX_NUM_CHAR_SPECIAL);
        aFmt.SetBulletChar(0x2022);
        aFmt.SetFirstLineIndent(0);
        aFmt.SetAbsLSpace(720);
        aRule.SetLevel(0, aFmt);
        aParaAttribs.Put(SvxNumBulletItem(std::move(aRule), EE_PARA_NUMBULLET));
        aEditEngine.SetParaAttribs(nPara, aParaAttribs);
    }

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then unordered list markers should be present:
    CPPUNIT_ASSERT_EQUAL(std::string("- First\n- Second"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportOrderedList)
{
    // Given a document with two ordered list paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Alpha\nBeta"_ustr);

    // Set list attributes on both paragraphs
    for (sal_Int32 nPara = 0; nPara < 2; nPara++)
    {
        SfxItemSet aParaAttribs(aEditEngine.GetParaAttribs(nPara));
        aParaAttribs.Put(SfxInt16Item(EE_PARA_OUTLLEVEL, sal_Int16(0)));
        SvxNumRule aRule(SvxNumRuleFlags::BULLET_REL_SIZE, 10, false);
        SvxNumberFormat aFmt(SVX_NUM_CHAR_SPECIAL);
        aFmt.SetNumberingType(SVX_NUM_ARABIC);
        aFmt.SetLabelFollowedBy(SvxNumberFormat::LabelFollowedBy::LISTTAB);
        aFmt.SetFirstLineIndent(0);
        aFmt.SetAbsLSpace(720);
        aRule.SetLevel(0, aFmt);
        aParaAttribs.Put(SvxNumBulletItem(std::move(aRule), EE_PARA_NUMBULLET));
        aEditEngine.SetParaAttribs(nPara, aParaAttribs);
    }

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then ordered list markers should be present:
    CPPUNIT_ASSERT_EQUAL(std::string("1. Alpha\n1. Beta"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportNestedList)
{
    // Given markdown with a nested unordered list:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "- Outer\n  - Inner");

    // Then there should be two paragraphs at different nesting levels:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Outer"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Inner"_ustr, aEditEngine.GetText(1));

    // Para 0: outline level 0, bullet type
    SfxItemSet aParaAttribs0 = aEditEngine.GetParaAttribs(0);
    auto& rLevel0 = aParaAttribs0.Get(EE_PARA_OUTLLEVEL);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rLevel0.GetValue());
    auto& rNumBullet0 = aParaAttribs0.Get(EE_PARA_NUMBULLET);
    const SvxNumberFormat* pFmt0 = rNumBullet0.GetNumRule().Get(0);
    CPPUNIT_ASSERT(pFmt0 != nullptr);
    CPPUNIT_ASSERT_EQUAL(SVX_NUM_CHAR_SPECIAL, pFmt0->GetNumberingType());

    // Para 1: outline level 1, bullet type at level 1
    SfxItemSet aParaAttribs1 = aEditEngine.GetParaAttribs(1);
    auto& rLevel1 = aParaAttribs1.Get(EE_PARA_OUTLLEVEL);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), rLevel1.GetValue());
    auto& rNumBullet1 = aParaAttribs1.Get(EE_PARA_NUMBULLET);
    const SvxNumberFormat* pFmt1 = rNumBullet1.GetNumRule().Get(1);
    CPPUNIT_ASSERT(pFmt1 != nullptr);
    CPPUNIT_ASSERT_EQUAL(SVX_NUM_CHAR_SPECIAL, pFmt1->GetNumberingType());
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHardBreak)
{
    // Given markdown with a hard break (two trailing spaces + newline):
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "Line one  \nLine two");

    // Then the hard break should create a paragraph break (unlike soft break):
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Line one"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Line two"_ustr, aEditEngine.GetText(1));
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownImportHeadingWithFormatting)
{
    // Given a markdown heading with italic text inside:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "# Hello *world*");

    // Then there should be one paragraph with heading formatting:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, aEditEngine.GetText(0));

    sal_uInt32 nDefaultHeight = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight();

    // "Hello " (0-6): bold + 180% height, NOT italic
    SfxItemSet aHelloAttribs = aEditEngine.GetAttribs(0, 0, 6, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aHelloAttribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(180) / sal_uInt32(100),
                         aHelloAttribs.Get(EE_CHAR_FONTHEIGHT).GetHeight());
    CPPUNIT_ASSERT_EQUAL(ITALIC_NONE, aHelloAttribs.Get(EE_CHAR_ITALIC).GetPosture());

    // "world" (6-11): bold + 180% height + italic
    SfxItemSet aWorldAttribs = aEditEngine.GetAttribs(0, 6, 11, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aWorldAttribs.Get(EE_CHAR_WEIGHT).GetWeight());
    CPPUNIT_ASSERT_EQUAL(nDefaultHeight * sal_uInt32(180) / sal_uInt32(100),
                         aWorldAttribs.Get(EE_CHAR_FONTHEIGHT).GetHeight());
    CPPUNIT_ASSERT(aWorldAttribs.Get(EE_CHAR_ITALIC).GetPosture() == ITALIC_NORMAL
                   || aWorldAttribs.Get(EE_CHAR_ITALIC).GetPosture() == ITALIC_OBLIQUE);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportBoldStrikethrough)
{
    // Given a document with bold+strikethrough text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello both world"_ustr);
    SfxItemSet aFmtSet(aEditEngine.GetEmptyItemSet());
    aFmtSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aFmtSet.Put(SvxCrossedOutItem(STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT));
    aEditEngine.QuickSetAttribs(aFmtSet, ESelection(0, 6, 0, 10));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then bold+strikethrough should nest as ~~**text**~~:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello ~~**both**~~ world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportAllFormatting)
{
    // Given a document with bold+italic+strikethrough text:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Hello all world"_ustr);
    SfxItemSet aFmtSet(aEditEngine.GetEmptyItemSet());
    aFmtSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aFmtSet.Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC));
    aFmtSet.Put(SvxCrossedOutItem(STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT));
    aEditEngine.QuickSetAttribs(aFmtSet, ESelection(0, 6, 0, 9));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then all three formats should nest as ~~***text***~~:
    CPPUNIT_ASSERT_EQUAL(std::string("Hello ~~***all***~~ world"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportCodeBlock)
{
    // Given a document with a fully monospace paragraph:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"int x = 5;"_ustr);
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxFontItem(FAMILY_MODERN, u"Courier New"_ustr, u""_ustr, PITCH_FIXED,
                         RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 0, 0, 10));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the text should be in a fenced code block:
    CPPUNIT_ASSERT_EQUAL(std::string("```\nint x = 5;\n```"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportCodeBlockMultiLine)
{
    // Given a document with three fully monospace paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"line1\nline2\nline3"_ustr);
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxFontItem(FAMILY_MODERN, u"Courier New"_ustr, u""_ustr, PITCH_FIXED,
                         RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO));
    aEditEngine.QuickSetAttribs(aSet, ESelection(0, 0, 2, 5));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the text should be in a single fenced code block:
    CPPUNIT_ASSERT_EQUAL(std::string("```\nline1\nline2\nline3\n```"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripCodeBlock)
{
    // Given markdown with a fenced code block:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "```\nline1\nline2\n```");

    // When re-exporting as markdown (skip trailing empty paragraphs from import):
    sal_Int32 nParas = aEditEngine.GetParagraphCount();
    sal_Int32 nLastPara = nParas - 1;
    while (nLastPara > 0 && aEditEngine.GetText(nLastPara).isEmpty())
        nLastPara--;
    sal_Int32 nLastLen = aEditEngine.GetText(nLastPara).getLength();
    uno::Reference<datatransfer::XTransferable> xData
        = aEditEngine.CreateTransferable(ESelection(0, 0, nLastPara, nLastLen));

    // Then the export should be a proper fenced code block:
    auto pData = dynamic_cast<EditDataObject*>(xData.get());
    SvMemoryStream& rStream = pData->GetMarkdownStream();
    std::string aMdOut(static_cast<const char*>(rStream.GetData()),
                       static_cast<size_t>(rStream.GetSize()));
    CPPUNIT_ASSERT_EQUAL(std::string("```\nline1\nline2\n```"), aMdOut);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownExportMixedCodeAndText)
{
    // Given a document with normal, code, and normal paragraphs:
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetText(u"Before\ncode line\nAfter"_ustr);
    SfxItemSet aSet(aEditEngine.GetEmptyItemSet());
    aSet.Put(SvxFontItem(FAMILY_MODERN, u"Courier New"_ustr, u""_ustr, PITCH_FIXED,
                         RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO));
    aEditEngine.QuickSetAttribs(aSet, ESelection(1, 0, 1, 9));

    // When exporting as markdown:
    std::string aMd = exportAsMarkdown(aEditEngine);

    // Then the code paragraph should be fenced:
    CPPUNIT_ASSERT_EQUAL(std::string("Before\n\n```\ncode line\n```\n\nAfter"), aMd);
}

CPPUNIT_TEST_FIXTURE(MarkdownTest, testMarkdownRoundtripMixedContent)
{
    // Given markdown with heading, list, and paragraph:
    EditEngine aEditEngine(mpItemPool.get());
    importMarkdown(aEditEngine, "# Title\n\n- **Bold item**\n- Normal item\n\nA paragraph.");

    // Then structure should be: heading, 2 list items, paragraph
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aEditEngine.GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(u"Title"_ustr, aEditEngine.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Bold item"_ustr, aEditEngine.GetText(1));
    CPPUNIT_ASSERT_EQUAL(u"Normal item"_ustr, aEditEngine.GetText(2));
    CPPUNIT_ASSERT_EQUAL(u"A paragraph."_ustr, aEditEngine.GetText(3));

    // Heading should be bold
    SfxItemSet aH1Attribs = aEditEngine.GetAttribs(0, 0, 5, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aH1Attribs.Get(EE_CHAR_WEIGHT).GetWeight());

    // List items should have bullet attributes
    SfxItemSet aListAttribs1 = aEditEngine.GetParaAttribs(1);
    auto& rLevel1 = aListAttribs1.Get(EE_PARA_OUTLLEVEL);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rLevel1.GetValue());

    // "Bold item" should have bold formatting on "Bold item"
    SfxItemSet aBoldAttribs = aEditEngine.GetAttribs(1, 0, 9, GetAttribsFlags::CHARATTRIBS);
    CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, aBoldAttribs.Get(EE_CHAR_WEIGHT).GetWeight());

    // When re-exporting as markdown:
    std::string aMdOut = exportAsMarkdown(aEditEngine);

    // Then the exported markdown should contain the key elements.
    // Note: headings are stored as bold+size attributes, so they export as
    // bold text (not # syntax), since heading-level metadata is not preserved.
    CPPUNIT_ASSERT(aMdOut.find("**Title**") != std::string::npos);
    CPPUNIT_ASSERT(aMdOut.find("- **Bold item**") != std::string::npos);
    CPPUNIT_ASSERT(aMdOut.find("- Normal item") != std::string::npos);
    CPPUNIT_ASSERT(aMdOut.find("A paragraph.") != std::string::npos);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
