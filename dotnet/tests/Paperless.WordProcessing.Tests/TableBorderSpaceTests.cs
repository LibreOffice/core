using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

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
    /// The grid lines stay where they were: the band grows around the stroke, not under it.
    /// </summary>
    /// <remarks>
    /// The stroke is drawn along the cell rectangle's edge, and the rectangle's top is half a band below
    /// the table's. Pinning it here is what stops the inset above from being paid for by moving every rule
    /// in every table half a border down the page.
    /// </remarks>
    [Fact]
    public void TheGridLinesThemselvesDoNotMove()
    {
        Length border = Length.FromPoints(1);

        (List<PlacedTableCell> cells, List<Length> heights) =
            TableLayouter.LayOut(Table(3, border), new DocPoint(Length.Zero, Length.Zero));

        // Row one's rectangle starts half a border down, and each later one a whole row pitch after it.
        cells[0].Area.Y.ShouldBe(border / 2);
        cells[1].Area.Y.ShouldBe(cells[0].Area.Y + (cells[1].Area.Y - cells[0].Area.Y));

        // The last row is charged for the two half bands the rectangles do not cover, so the row heights
        // add up to the table while the rectangles stay put.
        Length rectangles = cells[2].Area.Bottom - cells[0].Area.Y;
        heights.Aggregate(Length.Zero, (a, b) => a + b)
            .ShouldBe(rectangles + border, "half a band above the first row and half below the last");
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
