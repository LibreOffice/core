using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>nextPage</c> section break keeps the opening paragraph's space-before, whatever the file's
/// <c>compatibilityMode</c> says — and collapses it against the space-after of the section mark that
/// the importer discarded.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="PageBreakTopSpacingTests"/> covers the other three ways a page can start and is the
/// contrast this exists against: at <c>compatibilityMode</c> 15 a <c>w:br w:type="page"</c> and a
/// <c>w:pageBreakBefore</c> both drop the space, because <c>SwFrame::IsCollapseUpper</c>
/// (<c>sw/source/core/layout/calcmove.cxx</c>:1120) takes it back on every page but the first. A
/// section break is the carve-out that function states in its own comment — it declines "after
/// applying a new page style (but do it after page breaks)" — and
/// <c>SectionPropertyMap::CloseSectionGroup</c> gives every page-starting section a page style, so
/// the paragraph opening one carries a <c>RES_PAGEDESC</c> and keeps its space.
/// </para>
/// <para>
/// Measured on the installed 26.2.4.2 over sixteen authored synthetics — the three break kinds
/// crossed with <c>compatibilityMode</c> 15 and 12, with and without <c>w:titlePg</c>, and with and
/// without a portrait-to-landscape change across the break, all carrying 20 pt of space-before over
/// 10 pt of space-after. The two explicit breaks track the mode, 91.4 pt at 12 and 81.4 pt at 15. A
/// <c>nextPage</c> break puts the line at 91.4 pt in <b>all eight</b>: neither the mode nor the
/// geometry moves it. The sweep is <c>dotnet/probes/words-furniture-01/section-break-sweep.py</c>.
/// </para>
/// <para>
/// This supersedes the note on <c>PaginationOptions.CollapsesUpperAtPageTop</c> that read "a plain
/// <c>nextPage</c> section break does not set one — measured above, and it collapses like any other
/// break". That measurement was taken against 24.2.7.2 and does not survive the reference change.
/// </para>
/// <para>
/// The corpus case is <c>words/done-015/docx/airbus-pdf-information-package_v1-4.docx</c>, whose
/// landscape section opens with a 12 pt space-before heading after a mark declaring 6 pt. The 6 pt
/// this restores is what stops its glossary table fitting one extra row on page four, which is what
/// carried its last row — and the repeated heading row above it — off page nine. It went from
/// 1269 words against the reference's 1299 to 1299 exactly.
/// </para>
/// </remarks>
public sealed class SectionBreakTopSpacingTests
{
    /// <summary>
    /// The break keeps the space at <c>compatibilityMode</c> 15, where a page break would not.
    /// </summary>
    /// <remarks>
    /// 20 pt of space-before over the 10 pt of space-after the previous paragraph states, collapsed
    /// to the larger of the two: 10 pt of gap is left to add above the line.
    /// </remarks>
    [Fact]
    public void ASectionBreakKeepsTheSpaceBeforeAtTheTopOfItsPage()
    {
        SecondPageTop("section-break-top-spacing.docx").ShouldBe(Length.FromPoints(10));
    }

    /// <summary>
    /// What it collapses against is the discarded section mark's own space-after, not the last
    /// paragraph that survived.
    /// </summary>
    /// <remarks>
    /// The mark states 12 pt where <c>Normal</c> states none, so the 20 pt of space-before comes out
    /// as 8 pt. Reading the surviving paragraph instead gives the whole 20. Writer arrives here by
    /// moving the value: <c>DomainMapper_Impl::handleSectPrBeforeRemoval</c> stashes the mark's
    /// bottom margin before dropping it and <c>SectionPropertyMap::EmulateSectPrBelowSpacing</c>
    /// writes it onto the paragraph before the break.
    /// </remarks>
    [Fact]
    public void ItCollapsesAgainstTheDiscardedSectionMarksOwnSpaceAfter()
    {
        SecondPageTop("section-mark-below-spacing.docx").ShouldBe(Length.FromPoints(8));
    }

    /// <summary>How far below the body's top edge the second page's first line sits.</summary>
    private static Length SecondPageTop(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBe(2, $"{name} states one section break");

        return pages.Pages[1].Lines[0].Top;
    }
}
