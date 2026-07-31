using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Works out how tall a table's rows are, and where each of its cells goes.
/// </summary>
/// <remarks>
/// <para>
/// Two passes, because the answer is circular until it is not. A cell's height depends on its width, which
/// the grid states outright; a row's height is its tallest cell's; and a cell's <em>rectangle</em> depends
/// on the row heights. So the first pass lays every cell's text out at its own width and keeps the result,
/// and the second turns the heights into rectangles. Nothing is measured twice.
/// </para>
/// <para>
/// A cell spanning rows contributes to the last row it covers rather than to the first. That is what keeps
/// a tall merged cell from making its <em>first</em> row tall — the two-row merge in a real table is
/// usually beside two short ordinary cells, and charging its height to row one would leave row two empty
/// and the table the wrong shape. LibreOffice reaches the same answer by growing the merge's rows to fit;
/// charging the last row is the same result for the common case and cheaper than an iteration.
/// </para>
/// </remarks>
public static class TableLayouter
{
    /// <summary>
    /// Lays a table out at a stated origin, reporting the cells and the row heights.
    /// </summary>
    /// <param name="table">The table.</param>
    /// <param name="origin">
    /// Where the table's top-left corner goes, in page coordinates. Its left indent is applied here rather
    /// than by the caller, so a caller only has to know where the body area starts.
    /// </param>
    /// <param name="nesting">
    /// How many tables enclose this one, so that a file claiming absurd nesting stops rather than recursing.
    /// </param>
    /// <returns>
    /// The cells with page-coordinate rectangles, and each row's height in order — the caller needs the
    /// heights to decide where the table ends and which rows fit on the page.
    /// </returns>
    public static (List<PlacedTableCell> Cells, List<Length> RowHeights) LayOut(
        PageTable table, DocPoint origin, int nesting = 0)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<Length> lefts = ColumnLefts(table);
        int rows = Math.Min(table.Rows.Count, PageTable.MaxRows);

        // Pass one: every cell's text, laid out at its own width, with the row it charges its height to.
        List<Measured> measured = [];
        List<Length> heights = [.. Enumerable.Repeat(Length.Zero, rows)];

        for (int row = 0; row < rows; row++)
        {
            foreach (PageTableCell cell in table.Rows[row].Cells)
            {
                Length width = WidthOf(cell, lefts, table);
                if (width <= Length.Zero) continue;

                Length inner = width - cell.Padding.Horizontal;
                PlacedFlow? content = inner > Length.Zero
                    ? FlowLayouter.LayOut(
                        cell.Blocks,
                        new DocRect(Length.Zero, Length.Zero, inner, Length.Zero),
                        Length.Zero,
                        nesting)
                    : null;

                Length text = content is null ? Length.Zero : FlowLayouter.Extent(content);

                int last = Math.Min(row + Math.Max(1, cell.RowSpan), rows) - 1;
                measured.Add(new Measured(row, last, cell, width, content, text));

                // A merged cell charges only its last row, and only for what one row's worth of it needs.
                // Charging the whole height there would make that row as tall as the merge.
                if (last == row)
                {
                    heights[row] = Length.Max(heights[row], text + cell.Padding.Vertical);
                }
            }

            // The declared height, which is a floor unless the row says it is exact — in which case it is the
            // height, and content taller than it is clipped rather than growing the row. Applied per row
            // before the merge shortfall below, so that a merge spanning an exact row cannot stretch it.
            heights[row] = table.Rows[row].HasExactHeight
                ? Length.Max(Length.Zero, table.Rows[row].MinHeight)
                : Length.Max(heights[row], table.Rows[row].MinHeight);
        }

        // A merged cell may still need more room than the rows it covers add up to, so the last row it
        // covers takes the difference. Done after every row has its own floor, since the sum is what
        // decides whether there is a shortfall at all.
        foreach (Measured cell in measured)
        {
            if (cell.LastRow == cell.Row) continue;

            Length available = Length.Zero;
            for (int row = cell.Row; row <= cell.LastRow; row++) available += heights[row];

            // An exact row does not grow, so a merge ending in one has nowhere to put its shortfall and its
            // content overflows. Skipping the row rather than growing it is the whole point of "exact".
            if (table.Rows[cell.LastRow].HasExactHeight) continue;

            Length needed = cell.TextHeight + cell.Cell.Padding.Vertical;
            if (needed > available) heights[cell.LastRow] += needed - available;
        }

        // Pass two: the heights are settled, so every cell has a rectangle.
        List<Length> tops = [];
        Length top = Length.Zero;
        for (int row = 0; row < rows; row++)
        {
            tops.Add(top);
            top += heights[row];
        }

        List<PlacedTableCell> placed = new(measured.Count);
        foreach (Measured cell in measured)
        {
            Length height = tops[cell.LastRow] + heights[cell.LastRow] - tops[cell.Row];

            DocRect area = new(
                origin.X + table.LeftIndent + lefts[cell.Cell.Column],
                origin.Y + tops[cell.Row],
                cell.Width,
                height);

            placed.Add(new PlacedTableCell
            {
                Cell = cell.Cell,
                Area = area,
                Content = Positioned(cell, area),
                Row = cell.Row,
            });
        }

        return (placed, heights);
    }

    /// <summary>
    /// The same cells, shifted.
    /// </summary>
    /// <remarks>
    /// What lets a table be laid out once and placed many times: a cell's lines are positioned relative to
    /// its content rectangle, so moving the rectangle takes the text with it and nothing needs measuring
    /// again. Used by the paginator both to move a table onto a page and to draw a repeated heading row
    /// part way down one.
    /// </remarks>
    /// <param name="cells">The cells to shift.</param>
    /// <param name="dx">How far right.</param>
    /// <param name="dy">How far down.</param>
    public static List<PlacedTableCell> Offset(
        IEnumerable<PlacedTableCell> cells, Length dx, Length dy)
    {
        ArgumentNullException.ThrowIfNull(cells);

        List<PlacedTableCell> moved = [];
        foreach (PlacedTableCell cell in cells)
        {
            moved.Add(cell with
            {
                Area = Shift(cell.Area, dx, dy),
                Content = ShiftFlow(cell.Content, dx, dy),
            });
        }

        return moved;
    }

    /// <summary>
    /// Moves a cell's whole content: its rectangle, and any table nested inside it.
    /// </summary>
    /// <remarks>
    /// The lines need no attention — they are positioned relative to the flow's rectangle, so moving the
    /// rectangle takes them along. A nested table is the exception and the reason this exists: its cells
    /// carry page coordinates rather than flow-relative ones, so they have to be moved by the same amount
    /// explicitly. Missing that leaves a nested table wherever the pre-layout pass put it — which is near
    /// the page's top-left corner, since a table is laid out once at the origin and placed later.
    /// </remarks>
    private static PlacedFlow? ShiftFlow(PlacedFlow? flow, Length dx, Length dy)
    {
        if (flow is null) return null;
        if (dx == Length.Zero && dy == Length.Zero) return flow;

        return flow with
        {
            Area = Shift(flow.Area, dx, dy),
            Tables = [.. flow.Tables.Select(table => table with
            {
                Area = Shift(table.Area, dx, dy),
                Cells = Offset(table.Cells, dx, dy),
            })],
        };
    }

    private static DocRect Shift(DocRect area, Length dx, Length dy)
        => new(area.X + dx, area.Y + dy, area.Width, area.Height);

    /// <summary>
    /// A cell's text moved from the origin into its own rectangle, and aligned inside it.
    /// </summary>
    /// <remarks>
    /// The lines themselves do not move: they are positioned relative to the flow's area, so shifting the
    /// area takes them with it. What does change is the vertical alignment, which is only decidable now —
    /// it depends on the row's final height, and the row's height depended on this cell.
    /// </remarks>
    private static PlacedFlow? Positioned(Measured cell, DocRect area)
    {
        if (cell.Content is null) return null;

        CellPadding padding = cell.Cell.Padding;
        Length height = area.Height - padding.Vertical;
        Length spare = height - cell.TextHeight;

        Length offset = spare <= Length.Zero
            ? Length.Zero
            : cell.Cell.VerticalAlignment switch
            {
                CellVerticalAlignment.Middle => spare / 2,
                CellVerticalAlignment.Bottom => spare,
                _ => Length.Zero,
            };

        DocRect placed = new(
            area.X + padding.Left,
            area.Y + padding.Top + offset,
            area.Width - padding.Horizontal,
            height > Length.Zero ? height : Length.Zero);

        // From the origin the flow was laid out at to where the cell actually is.
        PlacedFlow moved = ShiftFlow(
            cell.Content,
            placed.X - cell.Content.Area.X,
            placed.Y - cell.Content.Area.Y)!;

        // The height comes from the cell rather than from the content, since the row may be taller.
        return moved with { Area = placed };
    }

    /// <summary>Where each grid column starts, measured from the table's left edge.</summary>
    /// <remarks>
    /// One entry per column plus a final one past the last, so a cell's width is the difference between
    /// two lookups whatever it spans — including a cell whose span runs off the end of the declared grid,
    /// which real documents contain.
    /// </remarks>
    private static List<Length> ColumnLefts(PageTable table)
    {
        int columns = Math.Min(table.ColumnWidths.Count, PageTable.MaxColumns);
        List<Length> lefts = new(columns + 1);

        Length at = Length.Zero;
        for (int column = 0; column < columns; column++)
        {
            lefts.Add(at);
            at += table.ColumnWidths[column];
        }

        lefts.Add(at);
        return lefts;
    }

    /// <summary>How wide a cell is: the columns it spans, added up.</summary>
    /// <remarks>
    /// Clamped to the declared grid at both ends. A cell starting past the last column has no width and is
    /// dropped; one spanning past it stops at the edge, which is what LibreOffice's own importers do with a
    /// row whose cells overrun the grid rather than widening the table.
    /// </remarks>
    private static Length WidthOf(PageTableCell cell, List<Length> lefts, PageTable table)
    {
        int columns = lefts.Count - 1;
        if (cell.Column < 0 || cell.Column >= columns) return Length.Zero;

        int end = Math.Clamp(cell.ColumnEnd, cell.Column + 1, columns);
        return lefts[end] - lefts[cell.Column];
    }

    /// <summary>
    /// A cell after pass one: its text laid out, and which rows it charges.
    /// </summary>
    /// <param name="Row">The row it starts in.</param>
    /// <param name="LastRow">The last row it covers, which is where its height is charged.</param>
    /// <param name="Cell">The cell itself.</param>
    /// <param name="Width">Its outer width, padding included.</param>
    /// <param name="Content">Its text at the origin, to be moved once the rectangle is known.</param>
    /// <param name="TextHeight">How tall that text is.</param>
    private readonly record struct Measured(
        int Row,
        int LastRow,
        PageTableCell Cell,
        Length Width,
        PlacedFlow? Content,
        Length TextHeight);
}
