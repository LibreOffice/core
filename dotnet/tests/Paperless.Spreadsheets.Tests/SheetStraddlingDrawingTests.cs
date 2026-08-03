using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A drawing wider than the page it starts on is printed on every page it reaches.
/// </summary>
/// <remarks>
/// <para>
/// A drawing does not belong to a page. <c>ScOutputData::PrePrintDrawingLayer</c>
/// (<c>sc/source/ui/view/output3.cxx:40-104</c>) sets a map-mode offset of minus the width of the
/// columns and the height of the rows before the page's first, and <c>PrintDrawingLayer</c>
/// (<c>:138</c>) then paints the <em>whole</em> drawing page through it, letting the device discard
/// what falls off the paper. So a picture straddling a break appears on both pages, cut, and a
/// renderer that anchors it to the page holding its top-left cell loses the second half.
/// </para>
/// <para>
/// Measured on <c>Air_Boss_Master_List.xlsx</c>, whose note box is anchored in column E and
/// straddles the column break: LibreOffice prints its left half on page 1 and its right half on
/// page 3, so page 3 carried 15 words against our none — 514 words against 527, and 529 now.
/// Checked against LibreOffice 24.2.7.2's own PDF for the fixture: page 1 ends the box at
/// "STRADDLINGT" and page 2 opens with the rest of it.
/// </para>
/// </remarks>
public sealed class SheetStraddlingDrawingTests
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
    public void AShapeStraddlingABreakIsDrawnOnBothPages()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-drawing-across-break.xlsx");
        pages.Count.ShouldBe(2, "six wide columns take two pages side by side");

        // Anchored in column B, so its head is on the first page and its tail — six inches to the
        // right of the anchor — lands past the break on the second.
        TextOf(pages[0]).ShouldContain("STRADDLINGHEAD");
        TextOf(pages[1]).ShouldContain("STRADDLINGTAIL");
    }

    /// <summary>
    /// The control: the cells still land on the page their own column band is on.
    /// </summary>
    /// <remarks>
    /// A rule that put every drawing on every page would satisfy the test above, and would also
    /// have to have broken the ordinary placement to do it. This pins the placement.
    /// </remarks>
    [Fact]
    public void TheCellsAreStillSplitBetweenTheTwoPages()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-drawing-across-break.xlsx");

        TextOf(pages[0]).ShouldContain("Alpha");
        TextOf(pages[0]).ShouldNotContain("Foxtrot");
        TextOf(pages[1]).ShouldContain("Foxtrot");
        TextOf(pages[1]).ShouldNotContain("Alpha");
    }
}
