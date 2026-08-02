using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A merged block whose anchor is on the previous page still draws on the page its tail falls on.
/// </summary>
/// <remarks>
/// <para>
/// Two things put a merge's origin out of a page's reach, and Calc treats them as one rule. A
/// hidden column is not placed at all, so a merge anchored inside one is never walked to; and a
/// merge that straddles a column break has its origin on the previous page, so the page carrying
/// its tail holds nothing but covered cells. <c>ScOutputData::GetMergeOrigin</c>
/// (<c>sc/source/ui/view/output2.cxx:953</c>) reaches both from the covered end, and which covered
/// cell may reach is decided by one flag: <c>bDoMerge</c>, which for a horizontally overlapped cell
/// is <c>bIsLeft = (nX == mnVisX1)</c> — the block's first <em>visible</em> column. Set, the walk
/// runs back to the origin through anything; clear, it gives up the moment it steps onto a column
/// that is not hidden, because a nearer cell will draw the block instead.
/// </para>
/// <para>
/// The fixture states both halves, because neither alone constrains a renderer. Row 1 is a merge
/// across all six columns whose text is long enough to reach past the break, so the second page
/// must show its tail; row 3 is an ordinary unmerged string in column A, which no page but the
/// first may show. A renderer that walked left from every band would draw both and one that never
/// walked left would draw neither.
/// </para>
/// <para>
/// Checked against LibreOffice 24.2.7.2's own PDF for the fixture, word by word: its second page
/// carries the heading from "going" to "BEYONDTHEBREAK" and does not carry "Unmerged". Measured on
/// <c>P1636e.xls</c>, whose title and eight footnotes are each merged across all six of its columns
/// while the break falls after the third: 304 words against 344, and 345 now.
/// </para>
/// </remarks>
public sealed class SheetStraddlingMergeTests
{
    private static IReadOnlyList<DrawnPage> Draw(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink.Pages;
    }

    private static string TextOf(DrawnPage page)
        => string.Join(" ", page.Runs.Select(run => run.Text));

    [Fact]
    public void AMergeStraddlingABreakIsDrawnOnBothPages()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-merge-across-break.fods");
        pages.Count.ShouldBe(2, "six five-centimetre columns take two pages side by side");

        TextOf(pages[0]).ShouldContain("Straddling heading");

        // Drawn from the origin's true position, which is off the left of the second page, so the
        // tail is what lands on the paper. The whole run is emitted and the page clips it, which
        // is what the reference does too.
        TextOf(pages[1]).ShouldContain("BEYONDTHEBREAK");
    }

    /// <summary>
    /// The negative half: an unmerged cell left of the band is not dragged onto it.
    /// </summary>
    /// <remarks>
    /// Its output area is its own column, which ends 425 pt before the second page begins, so
    /// <c>bOutside</c> skips it — and the reach-back for a merge must not bypass that.
    /// </remarks>
    [Fact]
    public void AnUnmergedCellLeftOfTheBandIsNotDrawnOnIt()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-merge-across-break.fods");

        TextOf(pages[0]).ShouldContain("Unmerged");
        TextOf(pages[1]).ShouldNotContain("Unmerged");
    }

    /// <summary>
    /// The hidden-column case still works, which is what the same walk used to do alone.
    /// </summary>
    [Fact]
    public void AMergeAnchoredInAHiddenColumnIsStillDrawn()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-hidden-merge.fods");
        TextOf(pages[0]).ShouldContain("Merged heading");
    }
}
