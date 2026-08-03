using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A merged block whose anchor is on the page above still draws on the page its tail falls on.
/// </summary>
/// <remarks>
/// <para>
/// The other axis of <see cref="SheetStraddlingMergeTests"/>, and the same rule.
/// <c>ScOutputData::GetMergeOrigin</c> (<c>sc/source/ui/view/output2.cxx:953</c>) walks left while
/// the position is <c>bHOverlapped</c> and then <em>up</em> while it is <c>bVOverlapped</c>
/// (<c>:1008</c>); which covered cell may walk is <c>bDoMerge</c>, and for a vertically overlapped
/// cell that is <c>bIsTop = (nY == mnVisY1)</c> (<c>:959</c>) — the block's first <em>visible</em>
/// row on the page. <c>GetOutputArea</c> then subtracts the heights of the rows between
/// (<c>:1237-1254</c>), which puts the anchor above the top of the paper and leaves the tail on it.
/// </para>
/// <para>
/// The fixture states both halves, because neither alone constrains a renderer. A1 is a merge forty
/// rows tall whose wrapped paragraph is long enough to reach past the break, so the second page must
/// show its tail; B24 is an ordinary unmerged cell one row tall whose text is far too long for it,
/// and Calc cuts that at its own row rather than carrying it down. A renderer that reached back from
/// every row would draw both, and one that never reached back would draw neither.
/// </para>
/// <para>
/// Checked against LibreOffice 24.2.7.2's own PDF for the fixture: page 2 carries
/// <c>BEYONDTHEBREAK</c> once and carries no part of the unmerged cell, whose first page shows only
/// its opening line. Measured on <c>RegChangeReport.xlsx</c>, whose descriptions are merges up to
/// thirty-five rows tall: pages 2, 3, 4 and 6 of twelve held 5, 294, 249 and 288 extractable words
/// against the reference's 506, 434, 344 and 360.
/// </para>
/// </remarks>
public sealed class SheetDownwardMergeTests
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
    public void AMergeStraddlingARowBreakIsDrawnOnBothPages()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-merge-down-break.fods");
        pages.Count.ShouldBe(2, "forty-five one-centimetre rows take two pages of 25.7 cm");

        TextOf(pages[0]).ShouldContain("Straddling description");

        // Drawn from the anchor's true position, which is above the second page, so the tail is
        // what lands on the paper.
        TextOf(pages[1]).ShouldContain("BEYONDTHEBREAK");
    }

    /// <summary>
    /// The block is drawn once on the page below, not once for every row it covers there.
    /// </summary>
    /// <remarks>
    /// <c>bIsTop</c> is what confines it: only the band's first visible row may reach back, so the
    /// fifteen further covered rows on the second page contribute nothing. Without that test the
    /// same paragraph would be emitted sixteen times over.
    /// </remarks>
    [Fact]
    public void AStraddlingMergeIsDrawnOncePerPage()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-merge-down-break.fods");

        pages[1].Runs.Count(run => run.Text.Contains("BEYONDTHEBREAK", StringComparison.Ordinal))
            .ShouldBe(1);
    }

    /// <summary>
    /// The negative half: an unmerged cell above the band is not dragged onto it.
    /// </summary>
    /// <remarks>
    /// Its output area is its own row, so nothing on the page below may reach back for it — and the
    /// reach-back for a merge must not bypass that.
    /// </remarks>
    [Fact]
    public void AnUnmergedCellAboveTheBandIsNotDrawnOnIt()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-merge-down-break.fods");

        TextOf(pages[0]).ShouldContain("Unmerged one-row cell");
        TextOf(pages[1]).ShouldNotContain("Unmerged one-row cell");
    }
}
