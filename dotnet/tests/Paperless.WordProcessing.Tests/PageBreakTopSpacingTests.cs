using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Whether the paragraph that broke the page keeps its own space-before at the top of it, which
/// the file's <c>compatibilityMode</c> decides.
/// </summary>
/// <remarks>
/// <para>
/// Two rules stacked, and reading either one alone gives the wrong answer.
/// <c>SwFlowFrame::HasParaSpaceAtPages</c> (<c>flowfrm.cxx</c>:1415) grants the space to the first
/// page and to any paragraph carrying an explicit break; <c>SwFrame::IsCollapseUpper</c>
/// (<c>calcmove.cxx</c>:1120) then takes it back on every page but the first, and its gate is
/// <c>TAB_OVER_SPACING &amp;&amp; !TAB_OVER_MARGIN</c> — <c>compatibilityMode</c> 15 or more, since
/// <c>SettingsTable.cxx</c>:685 sets <c>TabOverMargin</c> at 14 and below.
/// </para>
/// <para>
/// The two documents here differ in exactly one attribute, the <c>compatSetting</c>'s value, and
/// LibreOffice 24.2.7.2 renders their second pages 20 pt apart: the first word sits at 72.35 pt in
/// <c>page-break-top-spacing.docx</c> and at 92.35 in <c>page-break-top-spacing-compat14.docx</c>,
/// against a 72 pt top margin and 20 pt of space-before.
/// </para>
/// <para>
/// Found on <c>words/batch-013/docx/bulletin.docx</c>, whose <c>Introduction</c> heading carries a
/// leading <c>w:br w:type="page"</c> and 12 pt of space-before. The 12 pt cost that page its last
/// line, which then cost the next page its last line, and the document came out fifteen pages
/// against fourteen.
/// </para>
/// </remarks>
public sealed class PageBreakTopSpacingTests
{
    /// <summary>Word 2013 and after: the space is collapsed away.</summary>
    [Fact]
    public void FromWord2013TheBrokenPageStartsAtTheMargin()
    {
        SecondPageTop("page-break-top-spacing.docx").ShouldBe(Length.Zero);
    }

    /// <summary>Word 2010 and before: it is kept, which is the older rule.</summary>
    [Fact]
    public void BeforeWord2013TheBrokenPageKeepsTheSpace()
    {
        SecondPageTop("page-break-top-spacing-compat14.docx").ShouldBe(Length.FromPoints(20));
    }

    /// <summary>How far below the body's top edge the second page's first line sits.</summary>
    private static Length SecondPageTop(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBe(2, $"{name} states a page break");

        return pages.Pages[1].Lines[0].Top;
    }
}
