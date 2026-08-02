using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Which cells outside a page's own column band still put ink on it.
/// </summary>
/// <remarks>
/// <para>
/// Calc's string output starts one column <em>before</em> the block it is printing, so that a
/// long string reaching in from the left is drawn on the page its tail falls on
/// (<c>output2.cxx:1541</c>). Left there, that draws every nearest-left neighbour of every band
/// whether or not any of it reaches the paper. The second half of the rule is <c>bOutside</c>
/// (<c>:2037</c>): a cell whose output area — its own column, widened through the empty cells
/// beside it — does not overlap the block at all is skipped.
/// </para>
/// <para>
/// The two rows of the fixture differ in nothing but the length of one string, so a renderer
/// cannot satisfy both by choosing one side. Measured on
/// <c>ExampleWhiteListData.xlsx</c>, which drew twenty part numbers off the left edge of its
/// last two pages: 838 words against LibreOffice's 821.
/// </para>
/// <para>
/// A merged block anchored in a hidden column is the mirror image and is here for the same
/// reason: nothing that walks the columns a page places can reach its anchor, so Calc reaches
/// it from the first covered cell whose path back is entirely hidden
/// (<c>ScOutputData::GetMergeOrigin</c>, <c>:953</c>).
/// </para>
/// </remarks>
public sealed class SheetOffPageCellTests
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
    public void AStringThatNeverReachesThePageIsNotDrawnOnIt()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-lead-in.fods");
        pages.Count.ShouldBe(3, "the long string widens the printed block past column D");

        // Page 1 holds columns A to C, so both strings are on it in their own right.
        TextOf(pages[0]).ShouldContain("SHORTC2");

        // Page 2 is column D alone. The long string's spill runs through the empty cells beside
        // it and lands here; the short one stops inside column C and must not.
        string second = TextOf(pages[1]);
        second.ShouldContain("DDDD");
        second.ShouldContain("column C");
        second.ShouldNotContain("SHORTC2");

        // Page 3 exists only because of that spill, and holds nothing else.
        TextOf(pages[2]).ShouldContain("beyond");
    }

    [Fact]
    public void AMergeAnchoredInAHiddenColumnIsDrawnFromTheFirstColumnItCovers()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-hidden-merge.fods");
        pages.Count.ShouldBe(1);

        string drawn = TextOf(pages[0]);
        drawn.ShouldContain("Merged heading");

        // Once only: the block's other two covered columns each stop at a visible neighbour.
        drawn.Split("Merged heading").Length.ShouldBe(2);

        // And an ordinary cell in the hidden column is still not drawn, which is the half of
        // the rule that keeps a collapsed column collapsed.
        drawn.ShouldNotContain("Inside");
        drawn.ShouldContain("Cee");
    }
}
