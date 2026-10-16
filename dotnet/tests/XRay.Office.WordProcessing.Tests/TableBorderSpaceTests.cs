using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// How much height a cell's border takes, which is not the same as where its line is drawn.
/// </summary>
/// <remarks>
/// <para>
/// A border in Writer is a band the text may not enter, not merely a stroke: <c>SwBorderAttrs::CalcTop</c>
/// asks <c>SvxBoxItem::CalcLineSpace</c>, which adds the line's whole width to the padding
/// (<c>editeng/source/items/frmitems.cxx</c>:3717–3746). Neighbouring rows share the band between them, so
/// a table of <em>n</em> rows is <em>n+1</em> borders taller than the same table with none — and the first
/// row's text starts a whole border below the table's top edge while the line itself is drawn through the
/// middle of that band.
/// </para>
/// <para>
/// Both halves are asserted because getting one right hides the other. This engine already drew every grid
/// line where LibreOffice draws it and still made each cell's text sit half a border too high, which no
/// comparison of the strokes could see; on
/// <c>words/batch-010/docx/195584360.docx</c> it was 1 pt of lost height per table, about fourteen tables
/// to the page.
/// </para>
/// <para>
/// The numbers come from a one-column fixture rendered by LibreOffice 24.2.7.2 at borders of 0, 1 and 2 pt
/// and at one and three rows. With a 1 pt border the three-row table's text sat at 84.99, 97.54 and 110.09
/// against 83.99, 95.54 and 107.09 with none, and the paragraph after it at 122.64 against 118.64: one
/// border of inset at the top and four borders of extra height over three rows.
/// </para>
/// </remarks>
public sealed class TableBorderSpaceTests
{
    /// <summary>
    /// A table of <em>n</em> bordered rows is <em>n+1</em> border widths taller than an unbordered one.
    /// </summary>
    [Theory]
    [InlineData(1)]
    [InlineData(3)]
    [InlineData(7)]
    public void ATableIsOneMoreBorderTallThanItHasRows(int rows)
    {
        Length border = Length.FromPoints(1);

        Length bare = TotalHeight(Table(rows, Length.Zero));
        Length ruled = TotalHeight(Table(rows, border));

        (ruled - bare).ShouldBe(border * (rows + 1),
            "each grid line costs its whole width and two rows share the line between them");
    }

    /// <summary>
    /// The first row's text starts a whole border below the table's top edge.
    /// </summary>
    /// <remarks>
    /// Half of that is the rectangle, which begins where the line's band does; the other half is the inset
    /// inside it. Asserted from the origin so that neither half can be wrong without the sum showing it.
    /// </remarks>
    [Theory]
    [InlineData(20)]
    [InlineData(40)]
    public void TheFirstRowsTextStartsAWholeBorderDown(int twips)
    {
        Length border = Length.FromTwips(twips);

        (List<PlacedTableCell> ruled, _) =
            TableLayouter.LayOut(Table(3, border), new DocPoint(Length.Zero, Length.Zero));
        (List<PlacedTableCell> bare, _) =
            TableLayouter.LayOut(Table(3, Length.Zero), new DocPoint(Length.Zero, Length.Zero));

        Top(ruled, 0).ShouldBe(Top(bare, 0) + border);

        // And every row after it, since the pitch gains exactly one shared band.
        Top(ruled, 1).ShouldBe(Top(bare, 1) + border * 2);
        Top(ruled, 2).ShouldBe(Top(bare, 2) + border * 3);
    }

    /// <summary>
    /// A grid line sits ON the row boundary and its band hangs downwards into the row below it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This asserted the opposite until round 149, and the opposite is what O83, O84 and O85 all
    /// were.</strong> The old model put the grid line half a band below the boundary and drew the band
    /// centred on it, which is <em>algebraically identical</em> — the ink lands in the same place — for as
    /// long as every horizontal rule in the table has one width and the table does not split. It comes
    /// apart at a boundary whose columns differ, at a boundary whose two facing statements differ, and at
    /// a page cut, which is why one rule seated as three defects.
    /// </para>
    /// <para>
    /// [src] <c>SwTabFramePainter::Insert</c> (<c>paintfrm.cxx</c>:3061-3064) sets <c>RefMode::Begin</c> on
    /// a cell's horizontal borders — <em>"drawn below the reference points"</em> — and <c>Centered</c> on
    /// its vertical ones. [bin] <c>probes/tablerow-r146/results.md</c> §3.1: three columns stating 0.5,
    /// 3.0 and 1.5 pt across one boundary draw <c>95.001..95.501</c>, <c>95.001..98.001</c> and
    /// <c>95.001..96.501</c> — one shared top edge, three different bottoms — and the table's own outer
    /// top rule is a band <c>82.401..83.401</c> on a frame whose top is 82.401, so there is no separate
    /// rule for an outer line.
    /// </para>
    /// </remarks>
    [Fact]
    public void AGridLineSitsOnTheBoundaryAndItsBandHangsBelow()
    {
        Length border = Length.FromPoints(1);

        (List<PlacedTableCell> cells, List<Length> heights) =
            TableLayouter.LayOut(Table(3, border), new DocPoint(Length.Zero, Length.Zero));

        // The table's own top edge IS the first boundary, so the first rectangle starts at nought and the
        // band it pays for is inside it rather than above it.
        cells[0].Area.Y.ShouldBe(Length.Zero);

        // Each later rectangle begins exactly one row pitch on, and that pitch carries one whole band.
        Length pitch = cells[1].Area.Y - cells[0].Area.Y;
        cells[2].Area.Y.ShouldBe(cells[0].Area.Y + (pitch * 2));

        // The one band no rectangle covers is the outer bottom rule, which hangs below the last row
        // because nothing beneath it can pay for it.
        Length rectangles = cells[2].Area.Bottom - cells[0].Area.Y;
        heights.Aggregate(Length.Zero, (a, b) => a + b)
            .ShouldBe(rectangles + border, "the outer bottom band hangs below the last row");
    }

    /// <summary>Where a row's text sits, measured from the table's origin.</summary>
    private static Length Top(List<PlacedTableCell> cells, int row)
        => cells.First(cell => cell.Row == row).Content!.Area.Y;

    private static Length TotalHeight(PageTable table)
        => TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero))
            .RowHeights.Aggregate(Length.Zero, (a, b) => a + b);

    private static PageTable Table(int rows, Length border) => new()
    {
        ColumnWidths = [Length.FromTwips(4000)],
        Rows =
        [
            .. Enumerable.Range(0, rows).Select(row => new PageTableRow
            {
                Cells =
                [
                    new PageTableCell
                    {
                        Padding = new CellPadding(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
                        Borders = CellBorders.Uniform(new TableBorder(border, Colour.Black)),
                        Blocks = [Paragraph($"row {row}")],
                    },
                ],
            }),
        ],
    };

    private static PageParagraph Paragraph(string text) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = ParagraphFormat.Default,
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
