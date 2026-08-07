using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Lays a DrawingML table out and turns it into the shapes a slide draws.
/// </summary>
/// <remarks>
/// <para>
/// A table on a slide is not a shape with a table inside it: LibreOffice decomposes an
/// <c>SdrTableObj</c> into one filled-and-texted primitive per cell and then, separately, the
/// grid's border lines — cells first, borders afterwards, "to get the correct overlapping"
/// (<c>svx/source/table/viewcontactoftableobj.cxx:202-204</c>). So the output here is a run of
/// <see cref="PlacedShape"/>: one per visible cell carrying its fill and its text, then one per
/// consolidated grid line carrying only a pen. Nothing in the display list needs to know a table
/// happened, which is what keeps <see cref="SlideDrawing"/> unchanged and what lets a second
/// front end — the binary PPT path — reuse every line of this.
/// </para>
/// <para>
/// <strong>One stroke per grid line, not four round each cell.</strong> Measured on a
/// three-row, two-column table whose every cell states a one-point red edge: LibreOffice's PDF
/// writes seven strokes — four full-width horizontals and three full-height verticals — and each
/// <strong>overshoots by half its own pen width at both ends</strong>, so a table spanning
/// 56.693 to 623.622 pt draws its horizontals from 56.268 to 624.047. That is the same rule the
/// word processor's <c>PageDrawing</c> already follows for a Writer table, arrived at through a
/// different vocabulary, and the reason the two must agree: a reader comparing our content stream
/// against LibreOffice's is comparing stroke for stroke.
/// </para>
/// <para>
/// The order the strokes come out in is LibreOffice's too, and it is not "all horizontals then
/// all verticals": the cells are walked row-major and each contributes its top edge, its bottom
/// edge if it is on the last row, its left edge, and its right edge if it is on the last column
/// (<c>svx/source/dialog/framelinkarray.cxx:1487-1520</c>). Collinear neighbours merge into
/// whichever segment appeared first, so a three-row table emits its top rule, then all its
/// verticals, then the three remaining horizontals — which is exactly what the reference PDF
/// contains, in that order.
/// </para>
/// </remarks>
public static class SlideTable
{
    /// <summary>
    /// Lays a table out inside a graphic frame and returns the shapes that draw it.
    /// </summary>
    /// <param name="table">The table's grid and cells.</param>
    /// <param name="size">The frame's extent, in its own coordinates.</param>
    /// <param name="placement">The matrix taking the frame's coordinates onto the slide.</param>
    /// <param name="bodyOf">
    /// Reads a cell's text body, or returns null when it holds none. A delegate rather than a
    /// reader, because the cell's <c>a:txBody</c> is read by whichever family owns the file and
    /// this is the shared half.
    /// </param>
    /// <param name="fonts">The face cache, for measuring what a row has to grow to.</param>
    /// <param name="name">The frame's name, carried onto every shape for diagnostics.</param>
    public static List<PlacedShape> Place(
        DrawingTableBox table,
        DocSize size,
        AffineTransform placement,
        Func<DrawingTableCellBox, SlideTextBody?> bodyOf,
        SlideFonts fonts,
        string? name = null)
    {
        ArgumentNullException.ThrowIfNull(table);
        ArgumentNullException.ThrowIfNull(bodyOf);
        ArgumentNullException.ThrowIfNull(fonts);

        List<PlacedShape> shapes = [];
        if (table.ColumnWidths.Count == 0 || table.RowHeights.Count == 0) return shapes;

        Length[] columnEdges = Edges(table.ColumnWidths, size.Width);
        Cell[] cells = [.. Resolve(table, columnEdges, bodyOf)];
        Length[] rowEdges = Edges(RowHeights(table, cells, fonts), Length.Zero);

        bool upright = IsUpright(placement);

        foreach (Cell cell in cells)
        {
            if (cell.Box.IsCovered) continue;

            DocRect area = Area(cell, columnEdges, rowEdges);
            if (area.Width <= Length.Zero || area.Height <= Length.Zero) continue;

            shapes.Add(new PlacedShape
            {
                Name = name,
                Outline = ShapeTransform.Apply(placement, Rectangle(area)),
                Bounds = ShapeTransform.PlacedBounds(
                    AffineTransform.Concat(
                        AffineTransform.Translation(area.X.Emu, area.Y.Emu), placement),
                    area.Size),
                Fill = cell.Box.Fill,
                Text = Text(cell, area, placement, upright, fonts),
            });
        }

        Grid grid = Grid.Of(cells, columnEdges.Length - 1, rowEdges.Length - 1);

        foreach (Edge edge in grid.Lines())
        {
            shapes.Add(new PlacedShape
            {
                Name = name,
                Outline = ShapeTransform.Apply(
                    placement, Line(edge, grid, columnEdges, rowEdges)),
                Bounds = DocRect.Empty,
                Line = new Stroke(
                    Paint.Solid(edge.Pen.Colour),
                    edge.Pen.Width,
                    LineCap.Butt,
                    LineJoin.Round,
                    DashPattern: SlideDashes.Pattern(edge.Pen.PresetDash, edge.Pen.Width)),
            });
        }

        return shapes;
    }

    /// <summary>A cell resolved against the grid: its box, and the text body it holds.</summary>
    private readonly record struct Cell(DrawingTableCellBox Box, SlideTextBody? Body);

    private static IEnumerable<Cell> Resolve(
        DrawingTableBox table,
        Length[] columnEdges,
        Func<DrawingTableCellBox, SlideTextBody?> bodyOf)
    {
        foreach (DrawingTableCellBox box in table.Cells)
        {
            yield return new Cell(box, box.IsCovered ? null : bodyOf(box));
        }
    }

    /// <summary>
    /// The cumulative edges of a track list, with the last stretched to a stated total.
    /// </summary>
    /// <remarks>
    /// The columns are stretched to the frame's own width and the rows are not, which is the
    /// asymmetry LibreOffice has: <c>LayoutTableWidth</c> distributes any difference across the
    /// columns while <c>LayoutTableHeight</c> only ever grows a row past its stated height
    /// (<c>svx/source/table/tablelayouter.cxx</c>). A table taller than its frame overflows it,
    /// which is what a viewer shows.
    /// </remarks>
    private static Length[] Edges(IReadOnlyList<Length> tracks, Length total)
    {
        Length[] edges = new Length[tracks.Count + 1];
        Length at = Length.Zero;

        for (int i = 0; i < tracks.Count; i++)
        {
            edges[i] = at;
            at += tracks[i];
        }

        edges[tracks.Count] = total > Length.Zero && tracks.Count > 0 ? total : at;
        return edges;
    }

    /// <summary>
    /// Each row's height: the greater of what the file states and what its cells' text needs.
    /// </summary>
    /// <remarks>
    /// <c>a:tr/@h</c> is a minimum, not an answer — <c>TableLayouter::LayoutTableHeight</c> takes
    /// <c>max(stated, minimum)</c> per row (<c>tablelayouter.cxx:1026-1029</c>), where the minimum
    /// is the tallest single-row cell's text plus its top and bottom margins. A cell that spans
    /// rows is deferred to its <em>last</em> row and only grows that one, so a tall merged cell
    /// pushes the bottom row down rather than spreading over the rows it covers
    /// (<c>tablelayouter.cxx:1054-1076</c>).
    /// </remarks>
    private static List<Length> RowHeights(DrawingTableBox table, Cell[] cells, SlideFonts fonts)
    {
        List<Length> heights = [.. table.RowHeights];
        List<(int LastRow, int First, Length Minimum)> spanning = [];

        foreach (Cell cell in cells)
        {
            if (cell.Box.IsCovered || cell.Body is null) continue;

            Length minimum = Minimum(cell, fonts);
            int last = cell.Box.Row + cell.Box.RowSpan - 1;
            if (last >= heights.Count) continue;

            if (cell.Box.RowSpan > 1)
            {
                spanning.Add((last, cell.Box.Row, minimum));
                continue;
            }

            if (minimum > heights[cell.Box.Row]) heights[cell.Box.Row] = minimum;
        }

        foreach ((int last, int first, Length minimum) in spanning)
        {
            Length remaining = minimum;
            for (int row = first; row < last; row++) remaining -= heights[row];

            if (remaining > heights[last]) heights[last] = remaining;
        }

        return heights;

        Length Minimum(Cell cell, SlideFonts faces)
        {
            Length width = Width(cell, table);
            Margins margins = cell.Box.Margins;
            Length inner = width - margins.Left - margins.Right;

            return SlideTextLayout.Height(cell.Body!, inner, faces)
                   + margins.Top + margins.Bottom;
        }

        Length Width(Cell cell, DrawingTableBox box)
        {
            Length width = Length.Zero;
            for (int i = 0; i < cell.Box.ColumnSpan; i++)
            {
                int column = cell.Box.Column + i;
                if (column < box.ColumnWidths.Count) width += box.ColumnWidths[column];
            }

            return width;
        }
    }

    private static DocRect Area(Cell cell, Length[] columnEdges, Length[] rowEdges)
    {
        int left = Math.Clamp(cell.Box.Column, 0, columnEdges.Length - 1);
        int right = Math.Clamp(cell.Box.Column + cell.Box.ColumnSpan, 0, columnEdges.Length - 1);
        int top = Math.Clamp(cell.Box.Row, 0, rowEdges.Length - 1);
        int bottom = Math.Clamp(cell.Box.Row + cell.Box.RowSpan, 0, rowEdges.Length - 1);

        return new DocRect(
            columnEdges[left],
            rowEdges[top],
            columnEdges[right] - columnEdges[left],
            rowEdges[bottom] - rowEdges[top]);
    }

    private static PlacedText? Text(
        Cell cell, DocRect area, AffineTransform placement, bool upright, SlideFonts fonts)
    {
        if (cell.Body is not { } body) return null;

        DocRect rectangle = upright
            ? new DocRect(ShapeTransform.Apply(placement, area.Origin), area.Size)
            : area;

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, rectangle, fonts);
        return runs.Count == 0
            ? null
            : new PlacedText(runs, upright ? AffineTransform.Identity : placement);
    }

    /// <summary>True when a placement is a pure translation, so text needs no matrix.</summary>
    private static bool IsUpright(AffineTransform transform)
        => transform.A == 1 && transform.B == 0 && transform.C == 0 && transform.D == 1;

    /// <summary>
    /// One consolidated grid line, in grid indices: which line, and which run of it is drawn.
    /// </summary>
    /// <param name="IsHorizontal">True when it runs across the table.</param>
    /// <param name="At">Its own grid index: the row edge it sits on, or the column edge.</param>
    /// <param name="From">The first grid index it spans along its own axis.</param>
    /// <param name="To">One past the last.</param>
    /// <param name="Pen">The pen it is drawn with.</param>
    private readonly record struct Edge(
        bool IsHorizontal, int At, int From, int To, DrawingTableEdge Pen);

    /// <summary>
    /// The pen at every grid position, and the runs of them a table draws.
    /// </summary>
    /// <remarks>
    /// Every grid position takes <em>one</em> pen, chosen between the two cells that meet there by
    /// <c>TableLayouter::HasPriority</c> (<c>svx/source/table/tablelayouter.cxx:944-978</c>): the
    /// wider wins, a tie goes to whichever was written later — which under a row-major walk is the
    /// cell below or to the right — and a cell that states no edge never displaces one that does.
    /// All three are measured on <c>slide-table-grid.pptx</c>, whose middle row states a green
    /// left edge against its neighbour's grey right edge of the same width (green wins) and an
    /// orange three-point right edge against a grey one-point left edge (orange wins), while its
    /// top row states a red right edge against a grey left edge of the same width (grey wins,
    /// because it is the later cell).
    /// </remarks>
    private sealed class Grid
    {
        private readonly DrawingTableEdge?[,] _horizontal;
        private readonly DrawingTableEdge?[,] _vertical;
        private readonly bool[,] _coveredAbove;
        private readonly bool[,] _coveredLeft;
        private readonly int _columns;
        private readonly int _rows;

        private Grid(int columns, int rows)
        {
            _columns = columns;
            _rows = rows;
            _horizontal = new DrawingTableEdge?[columns, rows + 1];
            _vertical = new DrawingTableEdge?[columns + 1, rows];
            _coveredAbove = new bool[columns, rows];
            _coveredLeft = new bool[columns, rows];
        }

        public static Grid Of(Cell[] cells, int columns, int rows)
        {
            Grid grid = new(columns, rows);

            foreach (Cell cell in cells)
            {
                if (cell.Box.IsCovered) continue;

                int lastRow = Math.Min(cell.Box.Row + cell.Box.RowSpan, rows);
                int lastColumn = Math.Min(cell.Box.Column + cell.Box.ColumnSpan, columns);

                for (int row = cell.Box.Row; row < lastRow; row++)
                {
                    Set(grid._vertical, cell.Box.Column, row, cell.Box.Left);
                    Set(grid._vertical, lastColumn, row, cell.Box.Right);

                    for (int column = cell.Box.Column; column < lastColumn; column++)
                    {
                        if (row > cell.Box.Row) grid._coveredAbove[column, row] = true;
                        if (column > cell.Box.Column) grid._coveredLeft[column, row] = true;
                    }
                }

                for (int column = cell.Box.Column; column < lastColumn; column++)
                {
                    Set(grid._horizontal, column, cell.Box.Row, cell.Box.Top);
                    Set(grid._horizontal, column, lastRow, cell.Box.Bottom);
                }
            }

            return grid;
        }

        /// <summary>
        /// The grid lines to draw, merged, in the order LibreOffice creates them.
        /// </summary>
        /// <remarks>
        /// The cells are walked row-major and each contributes its top edge, its bottom edge if
        /// it is on the last row, its left edge, and its right edge if it is on the last column
        /// (<c>svx/source/dialog/framelinkarray.cxx:1487-1530</c>). Collinear neighbours that
        /// agree merge into whichever appeared first, so a three-row table emits its top rule,
        /// then the verticals crossing its first row, then the next horizontal, and so on — which
        /// is the order the reference PDF's twelve strokes come out in, stroke for stroke.
        /// </remarks>
        public List<Edge> Lines()
        {
            List<Edge> merged = [];

            for (int row = 0; row < _rows; row++)
            {
                for (int column = 0; column < _columns; column++)
                {
                    if ((!_coveredAbove[column, row] || row == 0)
                        && _horizontal[column, row] is { } top)
                    {
                        Add(new Edge(true, row, column, column + 1, top));
                    }

                    if (row == _rows - 1 && _horizontal[column, _rows] is { } bottom)
                    {
                        Add(new Edge(true, _rows, column, column + 1, bottom));
                    }

                    if ((!_coveredLeft[column, row] || column == 0)
                        && _vertical[column, row] is { } left)
                    {
                        Add(new Edge(false, column, row, row + 1, left));
                    }

                    if (column == _columns - 1 && _vertical[_columns, row] is { } right)
                    {
                        Add(new Edge(false, _columns, row, row + 1, right));
                    }
                }
            }

            return merged;

            void Add(Edge edge)
            {
                for (int i = 0; i < merged.Count; i++)
                {
                    Edge run = merged[i];
                    if (run.IsHorizontal != edge.IsHorizontal || run.At != edge.At) continue;
                    if (run.Pen != edge.Pen) continue;
                    if (edge.From > run.To || edge.To < run.From) continue;

                    merged[i] = run with
                    {
                        From = Math.Min(run.From, edge.From),
                        To = Math.Max(run.To, edge.To),
                    };
                    return;
                }

                merged.Add(edge);
            }
        }

        /// <summary>
        /// How far past its own end a line runs, to meet the line crossing it there.
        /// </summary>
        /// <remarks>
        /// <para>
        /// <strong>Half the width of the line it meets, not half its own.</strong> Measured on
        /// <c>slide-table-grid.pptx</c>: its one-point grey horizontals run from 71.121 to 612.879
        /// on a table spanning 72 to 612 pt, which is 0.879 at each end — half of the
        /// <em>two-point</em> outer verticals they meet, where half of their own width would be
        /// 0.425. Its three-point orange vertical, conversely, is extended by 0.425 at each end,
        /// which is half of the one-point horizontals crossing it.
        /// </para>
        /// <para>
        /// The general rule is <c>getExtends</c>
        /// (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:310-395</c>), which
        /// intersects this border's two offset edges with every style meeting the junction and
        /// takes the <em>nearest</em> crossing — hence the minimum here when the two perpendicular
        /// lines at a junction differ. For single lines meeting at a right angle that reduces to
        /// half the perpendicular width, which is every junction a table has; the double-line and
        /// diagonal cases it also serves are Calc's, and are not implemented.
        /// </para>
        /// </remarks>
        public Length Extension(Edge edge, bool atStart)
        {
            int along = atStart ? edge.From : edge.To;
            Length? nearest = null;

            foreach (DrawingTableEdge? crossing in Crossing(edge, along))
            {
                if (crossing is not { } pen) continue;
                nearest = nearest is { } best ? Length.Min(best, pen.Width) : pen.Width;
            }

            return nearest is { } width ? Length.FromEmu(width.Emu / 2) : Length.Zero;
        }

        private IEnumerable<DrawingTableEdge?> Crossing(Edge edge, int along)
        {
            if (edge.IsHorizontal)
            {
                yield return At(_vertical, along, edge.At - 1);
                yield return At(_vertical, along, edge.At);
            }
            else
            {
                yield return At(_horizontal, edge.At - 1, along);
                yield return At(_horizontal, edge.At, along);
            }
        }

        private static DrawingTableEdge? At(DrawingTableEdge?[,] map, int x, int y)
            => x < 0 || y < 0 || x >= map.GetLength(0) || y >= map.GetLength(1) ? null : map[x, y];

        private static void Set(DrawingTableEdge?[,] map, int x, int y, DrawingTableEdge? pen)
        {
            if (pen is not { } candidate) return;
            if (x < 0 || y < 0 || x >= map.GetLength(0) || y >= map.GetLength(1)) return;

            if (map[x, y] is { } existing && existing.Width > candidate.Width) return;

            map[x, y] = candidate;
        }
    }

    /// <summary>
    /// One grid line as a two-point path, run out at each end to meet what crosses it.
    /// </summary>
    private static GraphicsPath Line(
        Edge edge, Grid grid, Length[] columnEdges, Length[] rowEdges)
    {
        Length[] along = edge.IsHorizontal ? columnEdges : rowEdges;
        Length at = (edge.IsHorizontal ? rowEdges : columnEdges)[edge.At];

        Length from = along[edge.From] - grid.Extension(edge, atStart: true);
        Length to = along[edge.To] + grid.Extension(edge, atStart: false);

        return edge.IsHorizontal
            ? new GraphicsPath()
                .MoveTo(new DocPoint(from, at))
                .LineTo(new DocPoint(to, at))
            : new GraphicsPath()
                .MoveTo(new DocPoint(at, from))
                .LineTo(new DocPoint(at, to));
    }

    private static GraphicsPath Rectangle(DocRect area)
        => new GraphicsPath()
            .MoveTo(new DocPoint(area.X, area.Y))
            .LineTo(new DocPoint(area.Right, area.Y))
            .LineTo(new DocPoint(area.Right, area.Bottom))
            .LineTo(new DocPoint(area.X, area.Bottom))
            .Close();
}
