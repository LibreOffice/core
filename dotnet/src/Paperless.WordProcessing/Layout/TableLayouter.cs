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
    /// <param name="available">
    /// How wide the area holding the table is — the body's text width, or the enclosing cell's inner width
    /// for a nested one. It changes nothing for a table that declares its grid, which is every table this
    /// engine has ever laid out; it is read only by one that left a column without a width and so has to be
    /// fitted to what it sits in. See <see cref="PageTable.ColumnFit"/>.
    /// </param>
    /// <param name="collapsesSpacing">
    /// Whether the paragraphs inside a cell collapse their spacing against one another rather than adding
    /// it — see <see cref="FlowLayouter.LayOut"/>. It decides a cell's height, and so a row's, so passing
    /// the document's answer matters as much here as it does in the body.
    /// </param>
    /// <returns>
    /// The cells with page-coordinate rectangles, and each row's height in order — the caller needs the
    /// heights to decide where the table ends and which rows fit on the page.
    /// </returns>
    public static (List<PlacedTableCell> Cells, List<Length> RowHeights) LayOut(
        PageTable table,
        DocPoint origin,
        int nesting = 0,
        Length? available = null,
        bool collapsesSpacing = false)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<Length> lefts = ColumnLefts(table.WidthsWithin(available ?? table.Width));
        int rows = Math.Min(table.Rows.Count, PageTable.MaxRows);

        // Pass one: every cell's text, laid out at its own width, with the row it charges its height to.
        List<Measured> measured = [];
        List<Length> heights = [.. Enumerable.Repeat(Length.Zero, rows)];

        for (int row = 0; row < rows; row++)
        {
            foreach (PageTableCell cell in table.Rows[row].Cells)
            {
                Length width = WidthOf(cell, lefts);
                if (width <= Length.Zero) continue;

                Length inner = width - cell.Padding.Horizontal;
                PlacedFlow? content = inner > Length.Zero
                    ? FlowLayouter.LayOut(
                        cell.Blocks,
                        new DocRect(Length.Zero, Length.Zero, inner, Length.Zero),
                        Length.Zero,
                        nesting,
                        collapsesSpacing)
                    : null;

                // The advance rather than the ink: a cell is as tall as its content plus the space after
                // its last paragraph, which is what `AddParaSpacingToTableCells` — on for every Word
                // document — makes Writer do. See `PlacedFlow.Advance`.
                Length text = content is null ? Length.Zero : content.Advance;

                int last = Math.Min(row + Math.Max(1, cell.RowSpan), rows) - 1;
                measured.Add(new Measured(row, last, cell, width, content, text));

                // A merged cell charges only its last row, and only for what one row's worth of it needs.
                // Charging the whole height there would make that row as tall as the merge.
                if (last == row)
                {
                    heights[row] = Length.Max(heights[row], text + cell.Padding.Vertical);
                }
            }

            // A border takes space, and a row owns *half* of each of the two grid lines it sits between — the
            // line runs through the border's centre, so the other half belongs to the neighbour. The two
            // outermost halves, which have no neighbour, are added to the last row once the rectangles are
            // built; see there.
            heights[row] += BorderHeight(table.Rows[row]);

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

            Length covered = Length.Zero;
            for (int row = cell.Row; row <= cell.LastRow; row++) covered += heights[row];

            // An exact row does not grow, so a merge ending in one has nowhere to put its shortfall and its
            // content overflows. Skipping the row rather than growing it is the whole point of "exact".
            if (table.Rows[cell.LastRow].HasExactHeight) continue;

            Length needed = cell.TextHeight + cell.Cell.Padding.Vertical;
            if (needed > covered) heights[cell.LastRow] += needed - covered;
        }

        // Pass two: the heights are settled, so every cell has a rectangle.
        List<Length> tops = [];

        // The first grid line sits half a border *below* the table's top edge, because a grid line runs through
        // the centre of its border and the row heights already include half of it at each end. Measured: a
        // table whose top edge is at 70.2 pt draws its first border at 70.45 with a 0.5 pt border.
        Length top = rows > 0 ? BorderHeight(table.Rows[0]) / 2 : Length.Zero;

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
                origin.X + table.LeftWithin(available ?? table.Width) + lefts[cell.Cell.Column],
                origin.Y + tops[cell.Row],
                cell.Width,
                height);

            placed.Add(new PlacedTableCell
            {
                Cell = cell.Cell,
                Area = area,
                Content = Positioned(
                    cell,
                    area,
                    BorderHeight(table.Rows[cell.Row]) / 2,
                    BorderHeight(table.Rows[cell.LastRow]) / 2),
                Row = cell.Row,
            });
        }

        // The two half grid lines the rectangles do not cover: the one above the first row, which is why
        // `tops` starts half a border down, and the one below the last. Writer charges a cell for its
        // whole border and neighbouring rows share the line between them, so a table of n rows is n+1
        // borders tall rather than n — measured on a three-row fixture at 0, 1 and 2 pt, where each of the
        // three cases came out exactly one border taller than this engine made it. Charged to the last row
        // rather than split between the first and the last, because the paginator reconstructs a
        // continuation page's offset by adding up the heights it has already placed, and an allowance in
        // `heights[0]` that no rectangle carries would move every later row up by half a border.
        if (rows > 0)
        {
            heights[rows - 1] +=
                BorderHeight(table.Rows[0]) / 2 + BorderHeight(table.Rows[rows - 1]) / 2;
        }

        return (placed, heights);
    }

    /// <summary>
    /// One row's cells restricted to the part of the row that goes on a single page.
    /// </summary>
    /// <remarks>
    /// Positioned with the part's own top at nought and the table's left edge at nought, exactly as
    /// <see cref="LayOut"/> leaves a whole table, so the caller moves it onto the page with
    /// <see cref="Offset"/> and nothing here needs to know where the page is.
    /// </remarks>
    /// <param name="Cells">The cells, each holding only the lines that belong to this part.</param>
    /// <param name="Height">How tall the part is, borders included.</param>
    /// <param name="Cut">
    /// The row-relative depth everything above which is now drawn. Handed back to
    /// <see cref="SliceRow"/> for the next page's part, and equal to <see cref="Height"/> only by
    /// coincidence — the part is as tall as its tallest cell needs, and the cut is where the text stopped.
    /// </param>
    /// <param name="IsComplete">True when nothing of the row is left over.</param>
    public readonly record struct RowSlice(
        List<PlacedTableCell> Cells, Length Height, Length Cut, bool IsComplete);

    /// <summary>
    /// Takes the part of a row that fits in a given height, starting below what is already drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// What lets a table row cross a page break, which Writer does through a <em>follow flow line</em>:
    /// <c>SwTabFrame::Split</c> keeps the rows that fit in the master table and hands the first one that
    /// does not to <c>lcl_InsertNewFollowFlowLine</c>, which builds a second frame for the same row on the
    /// next page and moves whatever text did not fit into it
    /// (<c>sw/source/core/layout/tabfrm.cxx</c>). This is that in one function: the lines above the cut
    /// stay, the rest are the next page's problem.
    /// </para>
    /// <para>
    /// The cut is one depth for the whole row rather than a per-cell allowance, which is the point rather
    /// than a simplification — a row has one bottom edge, and choosing per cell would let one cell's text
    /// run past the edge another cell's stopped at. So the candidate cuts are the line bottoms of every
    /// cell together, and the deepest one whose part still fits is taken.
    /// </para>
    /// <para>
    /// Returns null when the row must move whole: a cell holding a nested table (Writer's
    /// <c>bTableLayoutTooComplex</c>), a cell merged across this row, a row of a stated exact height, or a
    /// cut that would leave nothing on either side of it. The last is what stops a split that gains
    /// nothing — a page ending in an empty row followed by the same row again does not terminate.
    /// </para>
    /// </remarks>
    /// <param name="row">The row, for its borders and its height rule.</param>
    /// <param name="cells">Its cells as <see cref="LayOut"/> placed them.</param>
    /// <param name="drawn">How far into the row an earlier page already reached; nought at its first part.</param>
    /// <param name="room">How much height is left on this page.</param>
    public static RowSlice? SliceRow(
        PageTableRow row, IReadOnlyList<PlacedTableCell> cells, Length drawn, Length room)
    {
        ArgumentNullException.ThrowIfNull(row);
        ArgumentNullException.ThrowIfNull(cells);

        if (cells.Count == 0 || row.HasExactHeight) return null;

        Length rowTop = cells[0].Area.Y;
        foreach (PlacedTableCell cell in cells)
        {
            // A cell covering more than this row cannot be cut here: its text belongs to a row further
            // down, and half of its rectangle would be drawn on each of two pages.
            if (Math.Max(1, cell.Cell.RowSpan) > 1) return null;
            if (cell.Content is { Tables.Count: > 0 }) return null;
            rowTop = Length.Min(rowTop, cell.Area.Y);
        }

        Length border = BorderHeight(row);
        Length above = rowTop + drawn;

        // Every line that is not yet drawn, as the depth its bottom sits at. These are the only places the
        // row may be cut, since a cut between two of them would draw half a line.
        List<Length> candidates = [];
        foreach (PlacedTableCell cell in cells)
        {
            if (cell.Content is not { } flow) continue;

            foreach (PlacedLine line in flow.Lines)
            {
                Length bottom = flow.Area.Y + line.Top + line.Box.Height;
                if (bottom > above) candidates.Add(bottom);
            }
        }

        if (candidates.Count == 0) return null;

        candidates.Sort();

        // The deepest cut whose part still fits. The height is not decreasing in the cut, so the first
        // candidate that does not fit ends the search.
        Length? chosen = null;
        Length height = Length.Zero;

        foreach (Length candidate in candidates)
        {
            if (chosen is { } already && already == candidate) continue;

            Length needed = HeightAt(cells, rowTop, above, candidate, border);
            if (needed > room) break;

            chosen = candidate;
            height = needed;
        }

        if (chosen is not { } cut) return null;

        bool complete = candidates[^1] <= cut;

        // A part holding every remaining line is not a split at all; the caller places the whole row.
        if (complete && drawn <= Length.Zero) return null;

        return new RowSlice(Sliced(cells, rowTop, above, cut, height), height, cut - rowTop, complete);
    }

    /// <summary>
    /// How tall the row's part is when it is cut at a stated depth.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same sum <see cref="LayOut"/> makes a whole row from — the tallest cell's text plus its
    /// padding, plus the row's borders — over the lines this part holds. The row's declared floor is
    /// deliberately <em>not</em> applied: it is a floor on the row and the parts add up to the row, so
    /// imposing it on each would make a split row twice as tall as an unsplit one. A row whose floor
    /// exceeds its text never reaches here anyway, since every line then fits in one part and
    /// <see cref="SliceRow"/> declines a split that leaves nothing over.
    /// </para>
    /// <para>
    /// The two ends of the run are measured as <see cref="LayOut"/> measures them rather than as ink, and
    /// that is what makes the two agree. A cell's flow does not begin at its first line — a first
    /// paragraph's space-before sits above it — and it does not end at its last, since
    /// <see cref="PlacedFlow.Advance"/> carries the last paragraph's space-after, which is exactly what
    /// a row's height is built from. So the row's own first part starts at the flow's top, and a part
    /// holding a cell's last line ends at its advance. Measuring both from the ink instead makes the sum
    /// of a row's parts shorter than the row, and then a row whose text fits the page but whose *height*
    /// does not is judged unbreakable by one measure and too tall by the other: it moves whole and leaves
    /// the difference blank. Measured on
    /// <c>f445896eb008d14c1746fc37d412dc22.docx</c>, where 205.8 pt of a page went empty because the row
    /// was 211.8 pt tall and its lines measured 202.5.
    /// </para>
    /// </remarks>
    private static Length HeightAt(
        IReadOnlyList<PlacedTableCell> cells, Length rowTop, Length above, Length cut, Length border)
    {
        // The row's own first part keeps the offset it was laid out with — see `Sliced` — so its cells
        // begin at the top of their flow and not at their first line.
        bool isFirst = above <= rowTop;
        Length text = Length.Zero;

        foreach (PlacedTableCell cell in cells)
        {
            if (cell.Content is not { } flow) continue;

            Length? top = null;
            Length bottom = Length.Zero;

            foreach (PlacedLine line in flow.Lines)
            {
                Length end = flow.Area.Y + line.Top + line.Box.Height;
                if (end <= above || end > cut) continue;

                top ??= isFirst ? flow.Area.Y : flow.Area.Y + line.Top;
                bottom = end;
            }

            if (top is not { } start) continue;

            // Nothing of this cell is left over, so its part is as tall as the cell — the trailing
            // spacing included, which is what `LayOut` charged the row for.
            if (flow.Lines.Count > 0
                && flow.Area.Y + flow.Lines[^1].Top + flow.Lines[^1].Box.Height <= cut)
            {
                bottom = Length.Max(bottom, flow.Area.Y + flow.Advance);
            }

            text = Length.Max(text, bottom - start + cell.Cell.Padding.Vertical);
        }

        return text + border;
    }

    /// <summary>The cells of one part, holding its lines and positioned from the part's own top.</summary>
    private static List<PlacedTableCell> Sliced(
        IReadOnlyList<PlacedTableCell> cells, Length rowTop, Length above, Length cut, Length height)
    {
        List<PlacedTableCell> sliced = new(cells.Count);

        foreach (PlacedTableCell cell in cells)
        {
            List<PlacedLine> kept = [];
            Length? first = null;

            if (cell.Content is { } flow)
            {
                foreach (PlacedLine line in flow.Lines)
                {
                    Length end = flow.Area.Y + line.Top + line.Box.Height;
                    if (end <= above || end > cut) continue;

                    first ??= line.Top;
                    kept.Add(line);
                }
            }

            DocRect area = new(cell.Area.X, Length.Zero, cell.Area.Width, height);
            PlacedFlow? content = null;

            if (cell.Content is { } text && kept.Count > 0)
            {
                // The remaining text starts at the top of the part rather than where it was measured, which
                // is what a follow flow line is: the cell begins again on the next page. The first part
                // keeps the offset it was laid out with, so a short cell beside a long one stays where its
                // vertical alignment put it.
                Length top = above <= rowTop
                    ? text.Area.Y - rowTop
                    : cell.Cell.Padding.Top - first!.Value;

                content = text with
                {
                    Area = new DocRect(text.Area.X, top, text.Area.Width, height),
                    Lines = kept,
                };
            }

            sliced.Add(cell with { Area = area, Content = content });
        }

        return sliced;
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

    /// <summary>
    /// How much of a row's height its borders take: half of its thickest top and half of its thickest bottom.
    /// </summary>
    /// <remarks>
    /// The thickest rather than each cell's own, because the row has one height and one grid line above it: two
    /// cells disagreeing about their top border share the thicker one's line, which is what the drawing does too
    /// when it consolidates. Measured: a row 18.95 pt tall without borders is 19.4 pt with 0.5 pt ones, and half
    /// of each of two borders is 0.5 pt — right to within a twip of rounding.
    /// </remarks>
    private static Length BorderHeight(PageTableRow row)
    {
        Length top = Length.Zero;
        Length bottom = Length.Zero;

        foreach (PageTableCell cell in row.Cells)
        {
            top = Length.Max(top, cell.Borders.Top.Width);
            bottom = Length.Max(bottom, cell.Borders.Bottom.Width);
        }

        return (top + bottom) / 2;
    }

    private static DocRect Shift(DocRect area, Length dx, Length dy)
        => new(area.X + dx, area.Y + dy, area.Width, area.Height);

    /// <summary>
    /// A cell's text moved from the origin into its own rectangle, and aligned inside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The lines themselves do not move: they are positioned relative to the flow's area, so shifting the
    /// area takes them with it. What does change is the vertical alignment, which is only decidable now —
    /// it depends on the row's final height, and the row's height depended on this cell.
    /// </para>
    /// <para>
    /// A border is not only a line, it is a band the text may not enter: Writer insets a cell's content by
    /// the whole border width on top of the padding, so a 1 pt border moves the first line down 1 pt and
    /// everything after the table down 2. The rectangle here starts half a grid line above the content —
    /// the line is drawn through its middle — so what the content owes is the other half at each end.
    /// Measured against LibreOffice on a one-column fixture at 0, 1 and 2 pt borders, where the step was
    /// exactly half the border and did not depend on the number of rows.
    /// </para>
    /// </remarks>
    /// <param name="cell">The measured cell.</param>
    /// <param name="area">Its rectangle.</param>
    /// <param name="bandAbove">Half the grid line above the cell's first row.</param>
    /// <param name="bandBelow">Half the grid line below the cell's last row.</param>
    private static PlacedFlow? Positioned(
        Measured cell, DocRect area, Length bandAbove, Length bandBelow)
    {
        if (cell.Content is null) return null;

        CellPadding padding = cell.Cell.Padding;
        Length height = area.Height - padding.Vertical - bandAbove - bandBelow;
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
            area.Y + bandAbove + padding.Top + offset,
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
    private static List<Length> ColumnLefts(IReadOnlyList<Length> widths)
    {
        int columns = Math.Min(widths.Count, PageTable.MaxColumns);
        List<Length> lefts = new(columns + 1);

        Length at = Length.Zero;
        for (int column = 0; column < columns; column++)
        {
            lefts.Add(at);
            at += widths[column];
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
    private static Length WidthOf(PageTableCell cell, List<Length> lefts)
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
