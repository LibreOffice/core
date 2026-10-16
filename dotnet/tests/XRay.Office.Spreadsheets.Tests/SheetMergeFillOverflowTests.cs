using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A merged cell's background where the merge runs off the right of the printed column block.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScOutputData::DrawBackground</c> extends one run across <c>ATTR_MERGE</c>'s column count and
/// stops the accumulation at <c>nCol &gt; mnX2 + 2</c> while it is adding the width of column
/// <c>nCol - 1</c> — so a merged fill reaches <strong>one</strong> column past the block's last and
/// no further, whatever the merge's own width is. The line numbers are
/// <c>sc/source/ui/view/output.cxx</c>:1148-1172 <em>in this tree</em>, which declares
/// 27.2.0.0.alpha0+ and is not the reference binary's source; the figures below are the 26.2.4.2
/// arm and stand on their own.
/// </para>
/// <para>
/// Every expectation is read off LibreOffice 26.2.4.2's own PDF of
/// <c>sheet-merge-fill-overflow.fods</c>, whose header carries the full table. The discriminator
/// that file was built for is that its 4-column merge and its 3-column merge are painted at
/// <strong>the same</strong> 5 cm on page 1: a rule that painted the whole merge and let the paper
/// clip it would draw 11 cm and 8 cm there, and both fit inside the 17 cm of printable width.
/// </para>
/// <para>
/// Its page 2 is the control for the other direction. The origin is off the block to the left
/// there, so the reference extends nothing — a covered cell carries <c>ATTR_MERGE_FLAG</c> and no
/// column count — and each covered column contributes its own width, which is what painting per
/// covered cell already did.
/// </para>
/// </remarks>
public sealed class SheetMergeFillOverflowTests
{
    /// <summary>Every filled rectangle of one page, as left and right edges in points.</summary>
    private static IReadOnlyList<(double Left, double Right)> Fills(int page)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-merge-fill-overflow.fods"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        PlacedDrawingSink sink = new();
        ((SheetPage)pages[page - 1]).Draw(sink);

        return [.. sink.Fills
            .Select(fill => (Math.Round(fill.Bounds.X.Points, 2),
                             Math.Round(fill.Bounds.Right.Points, 2)))
            .OrderBy(pair => pair.Item2)
            .ThenBy(pair => pair.Item1)];
    }

    [Fact]
    public void AMergeRunningPastTheBlockIsPaintedOneColumnPastItAndNoFurther()
    {
        IReadOnlyList<(double Left, double Right)> fills = Fills(1);

        // The block is column A alone, 2 cm. The unmerged control keeps it.
        fills.ShouldContain(pair => pair.Left == 56.69 && pair.Right == 113.39);

        // Both merges — A:D at 11 cm and A:C at 8 cm — are painted A + B, which is 5 cm.
        // 26.2.4.2 draws 56.64..198.43 for each of them, and again for the two rows of the
        // three-column block at A4:C5: a merge spanning rows is extended on every row it covers
        // and not only on its origin's, which is why that is four rectangles here and three
        // coalesced ones in the reference.
        fills.Count(pair => pair.Left == 56.69 && pair.Right == 198.43).ShouldBe(4);

        // And nothing is painted wider than that: a rule taking the whole merge would reach
        // 311.81 for the four-column one.
        fills.ShouldAllBe(pair => pair.Right <= 198.43);
    }

    [Fact]
    public void AMergeWhoseOriginIsOffTheBlockExtendsNothing()
    {
        // Page 2's block is B to E. The A:D merge covers B, C and D of it and the A:C merge
        // covers B and C, so the reference paints 9 cm and 6 cm — the covered columns' own
        // widths, with no overflow into E.
        IReadOnlyList<(double Left, double Right)> fills = Fills(2);

        fills.Max(pair => pair.Right).ShouldBe(311.81);
        fills.ShouldAllBe(pair => pair.Left >= 56.69);
    }
}
