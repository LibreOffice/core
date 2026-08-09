using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What a merged block draws behind and around itself.
/// </summary>
/// <remarks>
/// <para>
/// The assertions are <c>probes/sheets-r37/merge-decor.fods</c> and <c>merge-grid.fods</c>,
/// rendered by LibreOffice 24.2.7.2 and read with <c>pdf-ops.py dump</c>. Those two sheets carry
/// a 2 × 3 block at B2:C4 whose origin states a red fill and a 2 pt blue box and whose six
/// covered cells state a green fill and a 1 pt magenta box. The reference PDF holds one red
/// rectangle over the whole block, four blue lines round it, and <em>no green and no magenta
/// anywhere</em> — a covered cell's own decoration is never drawn.
/// </para>
/// <para>
/// The block here is the same shape and the same colours, so a reader can put the two side by
/// side. It is built by hand rather than read from the probe because the question is what the
/// decoration path does with a merge, not whether a reader finds one.
/// </para>
/// </remarks>
public sealed class SheetMergedDecorationTests
{
    private static readonly Colour Origin = Colour.FromRgb(0xFF0000);
    private static readonly Colour Covered = Colour.FromRgb(0x00FF00);

    /// <summary>B2:C4 — two columns wide, three rows tall, its origin at (1, 1).</summary>
    private static readonly SheetRange Block = new(1, 1, 2, 3);

    [Fact]
    public void ACoveredCellIsFilledInItsOriginsColourAndNotItsOwn()
    {
        RecordingDrawingSink sink = Draw(Decoration, (page, columns, rows, s)
            => page.DrawBackgrounds(columns, rows, s));

        IReadOnlyList<DrawnFill> fills = sink.Pages[0].FilledPaths;

        // Every cell of the block is filled, and all six take the origin's red. LibreOffice draws
        // one rectangle over the whole block; six abutting rectangles of one colour put the same
        // ink down, and survive the block being split across two pages.
        fills.Count(f => Paints(f, Origin)).ShouldBe(Block.RowCount * Block.ColumnCount);
        fills.ShouldAllBe(f => !Paints(f, Covered));
    }

    [Fact]
    public void AMergedBlocksBorderIsItsOriginsAndItsInteriorEdgesAreNotDrawn()
    {
        RecordingDrawingSink sink = Draw(Decoration, (page, columns, rows, s)
            => page.DrawBorders(columns, rows, s));

        IReadOnlyList<DrawnStroke> strokes = sink.Pages[0].StrokedPaths;

        // The covered cells' 1 pt magenta box is drawn nowhere at all.
        strokes.ShouldAllBe(s => !Paints(s, Covered));

        // What remains of the block is its outline: three left segments, three right, two top and
        // two bottom, one per covered cell edge, all in the origin's colour. Only the edges
        // *interior* to the block are suppressed — a left edge is suppressed by an overlap in the
        // column direction only, which is why a three-row block still emits three of them.
        strokes.Count(s => Paints(s, Origin))
            .ShouldBe((2 * Block.RowCount) + (2 * Block.ColumnCount));

        // The interior vertical between the block's two columns is gone: no stroke sits on it.
        Length interior = ColumnX(2);
        strokes.ShouldAllBe(s => s.Bounds.X != interior || s.Bounds.Height == Length.Zero);
    }

    [Fact]
    public void ThePrintedGridStopsAtAMergedBlockInBothDirections()
    {
        RecordingDrawingSink sink = Draw(
            SheetFormatting.Empty,
            (page, columns, rows, s) => page.DrawGrid(columns, rows, s),
            printsGrid: true);

        IReadOnlyList<DrawnStroke> strokes = sink.Pages[0].StrokedPaths;

        // The vertical at the block's interior column boundary survives only above and below it:
        // two segments where an unmerged sheet draws one full-height line.
        Length interior = ColumnX(2);
        IReadOnlyList<DrawnStroke> verticals =
            [.. strokes.Where(s => s.Bounds.X == interior && s.Bounds.Width == Length.Zero)];

        verticals.Count.ShouldBe(2);
        verticals[0].Bounds.Bottom.ShouldBe(RowY(1));
        verticals[1].Bounds.Y.ShouldBe(RowY(4));

        // And the two horizontals inside the block run to its left and to its right, never
        // through it.
        foreach (int row in (int[])[1, 2])
        {
            IReadOnlyList<DrawnStroke> line =
                [.. strokes.Where(s => s.Bounds.Y == RowY(row + 1) && s.Bounds.Height == Length.Zero)];

            line.Count.ShouldBe(2);
            line[0].Bounds.Right.ShouldBe(ColumnX(1));
            line[1].Bounds.X.ShouldBe(ColumnX(3));
        }
    }

    [Fact]
    public void AnUnmergedSheetStillDrawsOneRulePerColumnAndRow()
    {
        // The run-coalescing above must reduce to the single full-length line the unmerged case
        // has always drawn — otherwise every sheet in the corpus changes.
        RecordingDrawingSink sink = Draw(
            SheetFormatting.Empty,
            (page, columns, rows, s) => page.DrawGrid(columns, rows, s),
            printsGrid: true,
            merges: []);

        sink.Pages[0].StrokedPaths.Count.ShouldBe(Columns + Rows);
    }

    [Fact]
    public void AnIndexedMergeAnswersItsOriginAndWhichOfItsEdgesAreInterior()
    {
        SheetMerges merges = SheetMerges.Build([Block]);

        merges.IsEmpty.ShouldBeFalse();
        merges.OriginOf(3, 2).ShouldBe((1, 1));
        merges.OriginOf(0, 0).ShouldBe((0, 0));
        merges.Covering(4, 1).ShouldBeNull();

        // The origin's own top and left are the block's, so neither is interior; its bottom and
        // right are, because the block continues past them.
        merges.IsOverlappedTop(1, 1).ShouldBeFalse();
        merges.IsOverlappedLeft(1, 1).ShouldBeFalse();
        merges.IsOverlappedBottom(1, 1).ShouldBeTrue();
        merges.IsOverlappedRight(1, 1).ShouldBeTrue();

        // The bottom-right cell of the block is the mirror of that.
        merges.IsOverlappedTop(3, 2).ShouldBeTrue();
        merges.IsOverlappedLeft(3, 2).ShouldBeTrue();
        merges.IsOverlappedBottom(3, 2).ShouldBeFalse();
        merges.IsOverlappedRight(3, 2).ShouldBeFalse();
    }

    [Fact]
    public void ABlockTallerThanOneBandIsStillFoundAtBothEnds()
    {
        // The index buckets by a band of 64 rows, so a block that spans several bands has to be
        // registered in each of them. A single bucket would answer the first row and lose the
        // rest, which on a real sheet reads as the merge simply not existing lower down.
        SheetMerges merges = SheetMerges.Build([new SheetRange(0, 10, 0, 300)]);

        merges.OriginOf(10, 0).ShouldBe((10, 0));
        merges.OriginOf(150, 0).ShouldBe((10, 0));
        merges.OriginOf(300, 0).ShouldBe((10, 0));
        merges.Covering(301, 0).ShouldBeNull();
    }

    private const int Columns = 4;
    private const int Rows = 5;

    private static Length ColumnWidth => Length.FromTwips(1000);

    private static Length RowHeight => Length.FromTwips(300);

    private static Length ColumnX(int column) => ColumnWidth * column;

    private static Length RowY(int row) => RowHeight * row;

    private static bool Paints(DrawnFill fill, Colour colour)
        => fill.Paint is SolidPaint solid && solid.Colour == colour;

    private static bool Paints(DrawnStroke stroke, Colour colour)
        => stroke.Stroke.Paint is SolidPaint solid && solid.Colour == colour;

    private static SheetCellBorders Box(SheetBorder border)
        => new(border, border, border, border);

    /// <summary>The probe's formatting: the origin's decoration, and a different one under it.</summary>
    private static SheetFormatting Decoration
    {
        get
        {
            SheetFormatting formatting = new();

            int origin = formatting.Intern(new SheetCellDecoration(
                Origin, Box(SheetBorder.Line(Length.FromTwips(40), Origin))));
            int covered = formatting.Intern(new SheetCellDecoration(
                Covered, Box(SheetBorder.Line(Length.FromTwips(20), Covered))));

            for (int row = Block.FirstRow; row <= Block.LastRow; row++)
            {
                for (int column = Block.FirstColumn; column <= Block.LastColumn; column++)
                {
                    formatting.SetCell(
                        row,
                        column,
                        row == Block.FirstRow && column == Block.FirstColumn ? origin : covered);
                }
            }

            return formatting;
        }
    }

    private static RecordingDrawingSink Draw(
        SheetFormatting formatting,
        Action<SheetPageDecoration, IReadOnlyList<PlacedColumn>, IReadOnlyList<PlacedRow>, IDrawingSink> draw,
        bool printsGrid = false,
        IReadOnlyList<SheetRange>? merges = null)
    {
        SheetLayout sheet = new()
        {
            Name = "S1",
            Setup = new SheetPrintSetup { PrintsGrid = printsGrid },
            Formatting = formatting,
            StatedMerges = merges ?? [Block],
        };

        SheetPagePlacement placement = new(
            new SheetRange(0, 0, Columns - 1, Rows - 1), null, null, 100, 0, 0, 0);

        List<PlacedColumn> columns = [];
        for (int column = 0; column < Columns; column++)
            columns.Add(new PlacedColumn(column, ColumnX(column), ColumnWidth));

        List<PlacedRow> rows = [];
        for (int row = 0; row < Rows; row++)
            rows.Add(new PlacedRow(row, RowY(row), RowHeight));

        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(ColumnX(Columns), RowY(Rows)));
        draw(new SheetPageDecoration(sheet, placement), columns, rows, sink);
        sink.EndPage();

        return sink;
    }
}
