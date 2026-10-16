using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A merged block that holds nothing still stops a neighbour's long string.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScOutputData::IsAvailable</c> stops at "non-empty <em>or merged or overlapped</em>"
/// (<c>sc/source/ui/view/output2.cxx:1178-1191</c>), so what a merged block contains is beside the
/// point: a title beside one is clipped at the end of its own column whether the block is a
/// sub-heading or blank.
/// </para>
/// <para>
/// The reason this needs a test of its own is where the merge comes from. Every format states its
/// merges once, as a list of ranges, and every reader then puts the span on the block's top-left
/// cell and drops the cells it covers — which recovers the merge only when that top-left cell
/// survives. An empty cell past the last filled one in its row is trailing padding that every one
/// of the four readers drops, so a merge whose whole block is blank was reaching layout as no
/// merge at all. <see cref="SheetLayout.StatedMerges"/> carries the ranges themselves for exactly
/// this case.
/// </para>
/// <para>
/// Measured on <c>Bulletin-37-Appendix-2-immediate-detriment-data-request.xlsx</c>, whose A1 title
/// ran straight through its empty <c>B1:D1</c> merge and onto the next column band: 554 words
/// against LibreOffice's 541, and 541 now. Checked against LibreOffice 24.2.7.2's own PDF for the
/// fixture — its first page carries "Blocked title that would otherw" and neither page carries
/// "BLOCKEDTAIL" — and re-checked against 26.2.4.2, which agrees word for word on both pages.
/// </para>
/// </remarks>
public sealed class SheetEmptyMergeTests
{
    private static IReadOnlyList<DrawnPage> Draw(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink.Pages;
    }

    private static string TextOf(DrawnPage page)
        => string.Join(" ", page.Runs.Select(run => run.Text));

    [Fact]
    public void AnEmptyMergeStopsTheStringBesideIt()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-empty-merge.fods");
        pages.Count.ShouldBe(2, "six five-centimetre columns take two pages side by side");

        // The title is drawn — clipped to its own column by the merge beside it, so its tail is
        // on neither page.
        TextOf(pages[0]).ShouldContain("Blocked title");
        TextOf(pages[1]).ShouldNotContain("BLOCKEDTAIL");
    }

    /// <summary>
    /// The control: the same string with nothing merged beside it is not shortened at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Without it the test above passes for a renderer that blocks every spill, which is a
    /// different and larger defect than the one it is meant to catch. The discriminator is the
    /// <em>tail</em>: a string stopped by the merge loses it, and a string with empty cells beside
    /// it keeps it, because its output area was widened through them and nothing had to be cut.
    /// </para>
    /// <para>
    /// <strong>The tail is on page 1, with the rest of the string, and not on page 2.</strong>
    /// The whole run is painted once, at its anchor, and simply runs off the right-hand edge of
    /// the paper — <c>bOutside</c> and the clip are both about the output area rather than about
    /// the sheet of paper, so an unclipped run is not trimmed at the page edge either. Measured
    /// through 26.2.4.2: page 1 reads "…until its tail reaches SPILLEDTAIL" and page 2 reads
    /// "Delta Echo Foxtrot" and nothing else. This used to assert the tail onto page 2, which was
    /// the untagged rendering — see <see cref="SheetOffPageCellTests"/>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AStringWithNothingBesideItIsNotShortenedAtAll()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-empty-merge.fods");

        string first = TextOf(pages[0]);
        first.ShouldContain("Spilling title");
        first.ShouldContain("SPILLEDTAIL");

        // The page it spills over holds its own columns and no part of the spill.
        string second = TextOf(pages[1]);
        second.ShouldContain("Delta");
        second.ShouldNotContain("SPILLEDTAIL");
        second.ShouldNotContain("Spilling title");
    }
}
