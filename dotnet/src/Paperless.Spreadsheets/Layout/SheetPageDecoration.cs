using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>One column as a page places it.</summary>
/// <param name="Column">Its zero-based index in the sheet.</param>
/// <param name="X">Its left edge on the page.</param>
/// <param name="Width">How wide it is at the print scale.</param>
internal readonly record struct PlacedColumn(int Column, Length X, Length Width)
{
    /// <summary>Its right edge.</summary>
    public Length Right => X + Width;
}

/// <summary>One row as a page places it.</summary>
/// <param name="Row">Its zero-based index in the sheet.</param>
/// <param name="Y">Its top edge on the page.</param>
/// <param name="Height">How tall it is at the print scale.</param>
internal readonly record struct PlacedRow(int Row, Length Y, Length Height)
{
    /// <summary>Its bottom edge.</summary>
    public Length Bottom => Y + Height;
}

/// <summary>
/// Everything a printed sheet draws that is not the text of a cell.
/// </summary>
/// <remarks>
/// <para>
/// The order is <c>ScPrintFunc</c>'s and it is not arbitrary
/// (<c>sc/source/ui/view/printfun.cxx:1679-1695</c> and <c>:2344-2404</c>): backgrounds, then
/// borders, then the cells' text, then the grid, then the row and column headings, and last the
/// single rectangle round the lot. Backgrounds are opaque, so anything drawn before them
/// vanishes; the grid is drawn <em>over</em> the text because a hairline under a glyph would
/// otherwise disappear under an opaque fill.
/// </para>
/// <para>
/// Split out from the page's own drawing rather than folded into it because none of it is cell
/// text: this walks the same placed columns and rows, but what it needs from a cell is its fill
/// and its four edges, which live beside the content tree rather than in it.
/// </para>
/// </remarks>
internal sealed class SheetPageDecoration(SheetLayout sheet, SheetPagePlacement placement)
{
    /// <summary>One centimetre, the width of the printed row headings.</summary>
    /// <remarks><c>PRINT_HEADER_WIDTH</c>, <c>sc/source/ui/inc/printfun.hxx:45</c>.</remarks>
    public static Length HeadingWidth { get; } = Length.FromTwips(567);

    /// <summary>12.8 points, the height of the printed column headings.</summary>
    public static Length HeadingHeight { get; } = Length.FromTwips(256);

    /// <summary>
    /// The colour the printed grid and the headings are drawn in: black.
    /// </summary>
    /// <remarks>
    /// Black, not the pale grey the grid has on screen. <c>ScPrintFunc::PrintPage</c> starts from
    /// <c>Color aGridColor(COL_BLACK)</c> and only replaces it when the printout is asked to use
    /// the screen's style colours, which a PDF export is not
    /// (<c>printfun.cxx:1662</c> and <c>:2340</c>). Measured: every grid line in LibreOffice's
    /// PDF of <c>sheet-decor-ods.ods</c> is written under <c>0 0 0 RG</c>.
    /// </remarks>
    private static readonly Colour FurnitureColour = Colour.Black;

    /// <summary>
    /// The pen the grid, the headings and the outer frame are drawn with.
    /// </summary>
    /// <remarks>
    /// A tenth of a point, which is what LibreOffice's PDF export writes for a hairline — the
    /// grid is drawn with <c>DrawLine</c> and no width at all, and the export's initial
    /// <c>0.1 w</c> is what it comes out as.
    /// </remarks>
    private static readonly Length HairlineWidth = Length.FromPoints(0.1);

    private readonly double _scale = Math.Max(1, placement.ZoomPercentage) / 100.0;

    /// <summary>Paints the cells' fills.</summary>
    /// <remarks>
    /// Every fill before any border and any text, rather than each cell's fill before its own
    /// border: a fill is opaque and a border runs through the centre of a shared edge, so half of
    /// every border would be painted over by the neighbour drawn after it.
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawBackgrounds(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        SheetFormatting formatting = sheet.Formatting;
        if (formatting.IsEmpty) return;

        foreach (PlacedRow row in rows)
        {
            foreach (PlacedColumn column in columns)
            {
                if (formatting.At(row.Row, column.Column).Background is not { } colour) continue;

                Fill(new DocRect(column.X, row.Y, column.Width, row.Height), colour, sink);
            }
        }
    }

    /// <summary>
    /// Draws the cells' borders, one stroke per cell edge, with each shared edge settled.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Per <em>cell edge</em> and not per grid line, which is where a sheet parts company with a
    /// Writer table and is worth stating because the two look alike: <c>Array::CreateB2DPrimitiveRange</c>
    /// emits one primitive for each cell's top and left edge, and the bottom and right only for
    /// the last row and column (<c>svx/source/dialog/framelinkarray.cxx:1490-1537</c>). Two
    /// vertically adjacent cells agreeing about a border therefore produce two strokes, not one.
    /// Writer's <c>PageDrawing</c> merges runs because <c>SwTabFramePainter</c> does; Calc does
    /// not, and measuring LibreOffice's own PDF of <c>sheet-decor-ods.ods</c> confirms it —
    /// B4's box arrives as four separate <c>m … l S</c> pairs rather than one closed path.
    /// </para>
    /// <para>
    /// Which border a shared edge gets is <see cref="SheetCellBorders.Resolve"/>: the heavier of
    /// the two cells' facing edges. Both neighbours are consulted even at the page's own edges,
    /// because Calc's border array is built one column and one row wider than the page on every
    /// side (<c>ScDocument::FillInfo</c> loops <c>nCol1-1</c> to <c>nCol2+1</c>,
    /// <c>sc/source/core/data/fillinfo.cxx:1019</c>) and sets no clipping range when printing.
    /// </para>
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawBorders(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        SheetFormatting formatting = sheet.Formatting;
        if (formatting.IsEmpty || columns.Count == 0 || rows.Count == 0) return;

        Edges edges = Edges.Build(formatting, columns, rows);

        foreach (Edge edge in edges.All) Stroke(edge, edges, sink);
    }

    /// <summary>
    /// Draws the faint rules between cells, when the sheet prints them.
    /// </summary>
    /// <remarks>
    /// At the <em>far</em> edge of each column and row rather than the near one, which is what
    /// <c>ScOutputData::DrawGrid</c> does: it advances the pen by the column's width and then
    /// draws (<c>sc/source/ui/view/output.cxx:420-424</c>), so there is no line down the left of
    /// the first column and none along the top of the first row. Measured on
    /// <c>sheet-decor-ods.ods</c>, whose three columns start at 85.039 pt: the verticals are at
    /// 148.904, 212.882 and 276.86 and there is none at 85.039. The block's own left and top
    /// edges come from the outer frame instead, which is why the two are drawn together.
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawGrid(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        if (!sheet.Setup.PrintsGrid || columns.Count == 0 || rows.Count == 0) return;

        Length left = columns[0].X;
        Length right = columns[^1].Right;
        Length top = rows[0].Y;
        Length bottom = rows[^1].Bottom;

        foreach (PlacedColumn column in columns) Rule(column.Right, top, column.Right, bottom, sink);
        foreach (PlacedRow row in rows) Rule(left, row.Bottom, right, row.Bottom, sink);
    }

    /// <summary>
    /// Draws the row and column headings, and the frame round the printed block.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The headings are a boxed cell each, holding the column's letter or the row's number
    /// centred both ways (<c>ScPrintFunc::PrintColHdr</c> and <c>PrintRowHdr</c>,
    /// <c>printfun.cxx:1417</c> and <c>:1466</c>). The strip is one centimetre wide and
    /// 12.8 points tall, taken off the printable area before any cell is placed.
    /// </para>
    /// <para>
    /// The frame is drawn whenever <em>either</em> the grid or the headings print, not only with
    /// the headings — <c>if (bDoPrint &amp;&amp; (aTableParam.bGrid || aTableParam.bHeaders))</c>
    /// (<c>printfun.cxx:2384</c>) — and it encloses the headings as well as the cells. Measured
    /// on <c>sheet-decor-ods.ods</c>: one rectangle from 56.58 pt to 276.889 pt across, which is
    /// the row headings' left edge to the last column's right edge.
    /// </para>
    /// </remarks>
    /// <param name="origin">Where the block starts, headings included.</param>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawHeadings(
        DocPoint origin,
        IReadOnlyList<PlacedColumn> columns,
        IReadOnlyList<PlacedRow> rows,
        IDrawingSink sink)
    {
        SheetPrintSetup setup = sheet.Setup;
        if (!setup.PrintsHeadings && !setup.PrintsGrid) return;
        if (columns.Count == 0 || rows.Count == 0) return;

        Length headingWidth = setup.PrintsHeadings ? HeadingWidth * _scale : Length.Zero;
        Length headingHeight = setup.PrintsHeadings ? HeadingHeight * _scale : Length.Zero;

        if (setup.PrintsHeadings)
        {
            Length top = origin.Y;
            foreach (PlacedColumn column in columns)
            {
                Box(new DocRect(column.X, top, column.Width, headingHeight),
                    ColumnName(column.Column), sink);
            }

            Length left = origin.X;
            foreach (PlacedRow row in rows)
            {
                Box(new DocRect(left, row.Y, headingWidth, row.Height),
                    (row.Row + 1).ToString(System.Globalization.CultureInfo.CurrentCulture), sink);
            }
        }

        DocRect frame = new(
            origin.X,
            origin.Y,
            columns[^1].Right - origin.X,
            rows[^1].Bottom - origin.Y);

        Outline(frame, sink);
    }

    /// <summary>
    /// Draws the header and the footer: three pieces of text in one band each.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The band runs from the page's left margin plus the band's own left margin to the mirror of
    /// that on the right (<c>ScPrintFunc::PrintHF</c>, <c>printfun.cxx:1800-1802</c>), its text
    /// occupies the top <c>height - gap</c> of it, and each of the three parts is drawn into that
    /// same rectangle — left-aligned, centred and right-aligned. They overlap when they are long,
    /// which is what LibreOffice shows and why this is three runs rather than one line with tab
    /// stops.
    /// </para>
    /// <para>
    /// Vertically the text is centred in what is left: <c>aDraw.Y += (paperHeight - textHeight)
    /// / 2</c>. Measured on <c>sheet-decor-ods.ods</c>, whose header band gives its text
    /// 14.099 pt and whose single line measures 11.1 pt, that puts the baseline 10.55 pt below
    /// the top margin — 1.5 pt of centring plus a 9.05 pt ascent.
    /// </para>
    /// </remarks>
    /// <param name="context">What the fields resolve to on this page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawHeaderAndFooter(SheetHeaderContext context, IDrawingSink sink)
    {
        SheetPrintSetup setup = sheet.Setup;
        DocSize page = setup.PageSize;

        if (setup.Header is { IsEmpty: false } header && setup.HeaderHeight > Length.Zero)
        {
            DrawBand(
                header,
                context,
                setup.LeftMargin + setup.HeaderLeftMargin,
                page.Width - setup.RightMargin - setup.HeaderRightMargin,
                setup.TopMargin,
                setup.HeaderHeight - setup.HeaderGap,
                setup.HeaderIsDynamic,
                false,
                sink);
        }

        if (setup.Footer is { IsEmpty: false } footer && setup.FooterHeight > Length.Zero)
        {
            // The footer's gap sits at the *top* of its band, between the last row and the text,
            // so the text starts that far below the band's top rather than at it.
            DrawBand(
                footer,
                context,
                setup.LeftMargin + setup.FooterLeftMargin,
                page.Width - setup.RightMargin - setup.FooterRightMargin,
                page.Height - setup.BottomMargin - setup.FooterHeight + setup.FooterGap,
                setup.FooterHeight - setup.FooterGap,
                setup.FooterIsDynamic,
                true,
                sink);
        }
    }

    private void DrawBand(
        SheetHeaderFooter band,
        SheetHeaderContext context,
        Length left,
        Length right,
        Length top,
        Length height,
        bool dynamic,
        bool fromBottom,
        IDrawingSink sink)
    {
        if (right <= left || height <= Length.Zero) return;

        // The band is drawn at the page's own zoom. `ScPrintFunc::PrintHF` switches the device to
        // `aTwipMode`, which carries the zoom as its scale fraction
        // (`InitModes`, sc/source/ui/view/printfun.cxx:2645), so a header on a sheet printed at
        // 33% is drawn at a third of its stated size along with everything else.
        double zoom = Math.Max(1, placement.ZoomPercentage) / 100.0;

        // The three parts share one band and are each centred in it, which is why the band's own
        // height is the tallest of the three rather than each part's own: `PrintHF` gives the
        // EditEngine one `aPaperSize` and computes `nDif` per area against it
        // (sc/source/ui/view/printfun.cxx:1876-1912), and `UpdateHFHeight` has set that height to
        // the greatest of the three (`:820-834`). Measured on `sheet-outline-collapse.xlsx`,
        // whose footer holds two 8 pt lines on the left and one on the right: LibreOffice puts
        // the left part's last line hard against the footer margin and the right part's single
        // line 3.35 pt above it, which is half the difference between the two.
        Length bandText = Length.Zero;
        foreach (SheetHeaderPart part in (SheetHeaderPart[])[band.Left, band.Centre, band.Right])
            bandText = Length.Max(bandText, TextHeight(part, context, zoom));

        if (bandText <= Length.Zero) return;

        Length drawn = dynamic ? bandText : height;
        Length bandTop = dynamic && fromBottom ? top + height - bandText : top;

        Place(band.Left, _ => left);
        Place(band.Centre, width => left + ((right - left - width) / 2));
        Place(band.Right, width => right - width);

        void Place(SheetHeaderPart part, Func<Length, Length> position)
        {
            if (part.IsEmpty) return;

            IReadOnlyList<IReadOnlyList<SheetHeaderPiece>> lines = part.Lines(context);
            if (lines.Count == 0) return;

            Length text = Length.Zero;
            foreach (IReadOnlyList<SheetHeaderPiece> line in lines) text += LineHeight(line, zoom);

            Length spare = drawn - text;
            Length pen = bandTop + (spare > Length.Zero ? spare / 2 : Length.Zero);

            foreach (IReadOnlyList<SheetHeaderPiece> line in lines)
            {
                Length lineHeight = LineHeight(line, zoom);
                if (line.Count == 0)
                {
                    pen += lineHeight;
                    continue;
                }

                Length width = Length.Zero;
                List<(BandRun Run, Length Size)> runs = [];
                foreach (SheetHeaderPiece piece in line)
                {
                    Length size = SizeOf(piece, zoom);
                    if (SheetBandText.Shape(piece.Text, size) is not { } run) continue;
                    runs.Add((run, size));
                    width += run.Width;
                }

                if (runs.Count > 0)
                {
                    Length ascent = Length.Zero;
                    foreach ((_, Length size) in runs)
                        ascent = Length.Max(ascent, SheetBandText.AscentAt(size));

                    Length x = position(width);
                    foreach ((BandRun run, _) in runs)
                    {
                        sink.DrawGlyphRun(
                            run.At(new DocPoint(x, pen + ascent)), Paint.Solid(Colour.Black));
                        x += run.Width;
                    }
                }

                pen += lineHeight;
            }
        }
    }

    /// <summary>How tall one part of a band is: the sum of its lines.</summary>
    private static Length TextHeight(
        SheetHeaderPart part, SheetHeaderContext context, double zoom)
    {
        if (part.IsEmpty) return Length.Zero;

        Length height = Length.Zero;
        foreach (IReadOnlyList<SheetHeaderPiece> line in part.Lines(context))
            height += LineHeight(line, zoom);

        return height;
    }

    /// <summary>The em size one piece of a band is drawn at, the page's zoom applied.</summary>
    private static Length SizeOf(SheetHeaderPiece piece, double zoom)
        => (piece.Size ?? SheetBandText.DefaultSize) * zoom;

    /// <summary>How tall one line of a band is: the tallest of the pieces on it.</summary>
    /// <remarks>
    /// An empty line — a bare break, which a footer written as <c>&amp;RPage &amp;P\n\nrest</c>
    /// contains — still takes a line, at the sheet's default height.
    /// </remarks>
    private static Length LineHeight(IReadOnlyList<SheetHeaderPiece> line, double zoom)
    {
        Length height = Length.Zero;
        foreach (SheetHeaderPiece piece in line)
            height = Length.Max(height, SheetBandText.LineHeightAt(SizeOf(piece, zoom)));

        return height > Length.Zero
            ? height
            : SheetBandText.LineHeightAt(SheetBandText.DefaultSize * zoom);
    }

    /// <summary>One stroke of a border, with its ends extended to meet what it crosses.</summary>
    /// <remarks>
    /// <para>
    /// The extension is half the width of the perpendicular border it meets, and nothing at all
    /// where it meets none — which is the whole of what <c>getExtends</c> computes for the simple
    /// case (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:310</c>): the line is cut
    /// against the far edge of the other line's band, and that edge is half its width past the
    /// crossing point.
    /// </para>
    /// <para>
    /// Measured on <c>sheet-decor-ods.ods</c>: the red 2.5 pt vertical in row 2 meets no
    /// horizontal at either end and runs 12.784 pt against a row 12.813 pt tall, while the blue
    /// vertical in row 3 ends on B4's one-point box and overshoots it by 0.509 pt. A rule that
    /// always overshot by half its <em>own</em> width — which is what a Writer table does —
    /// would put the red one 2.5 pt too long.
    /// </para>
    /// </remarks>
    private static void Stroke(Edge edge, Edges edges, IDrawingSink sink)
    {
        SheetBorder border = edge.Border;
        Length start = edge.From - edges.ExtensionAt(edge, edge.From);
        Length end = edge.To + edges.ExtensionAt(edge, edge.To);

        // A double rule is two lines about the centre, the gap between them untouched.
        if (border.IsDouble)
        {
            Length half = (border.Primary + border.Distance + border.Secondary) / 2;
            Line(edge, start, end, half - (border.Primary / 2), border.Primary, sink);
            Line(edge, start, end, (border.Secondary / 2) - half, border.Secondary, sink);
            return;
        }

        Line(edge, start, end, Length.Zero, border.Primary, sink);
    }

    private static void Line(
        Edge edge, Length start, Length end, Length offset, Length width, IDrawingSink sink)
    {
        if (width <= Length.Zero) return;

        Length at = edge.At + offset;

        // Butt caps and round joins, which is what LibreOffice's own export writes for a border:
        // "q 2.49983 w 0 J 1 j". A square cap would add half a width at each end and undo the
        // extension arithmetic above.
        Stroke pen = new(
            Paint.Solid(edge.Border.Colour), width, LineCap.Butt, LineJoin.Round,
            DashPattern: Dashes(edge.Border, width));

        GraphicsPath path = edge.IsHorizontal
            ? new GraphicsPath().MoveTo(new DocPoint(start, at)).LineTo(new DocPoint(end, at))
            : new GraphicsPath().MoveTo(new DocPoint(at, start)).LineTo(new DocPoint(at, end));

        sink.StrokePath(path, pen);
    }

    /// <summary>
    /// The dash pattern a border pattern draws with, or null for a solid line.
    /// </summary>
    /// <remarks>
    /// Proportional to the line's width, which is how <c>SvxBorderLine</c> states them: the
    /// patterns are defined as multiples of the width rather than in absolute lengths, so a thick
    /// dashed border has long dashes and a hairline one has short ones.
    /// </remarks>
    private static IReadOnlyList<Length>? Dashes(SheetBorder border, Length width)
    {
        Length unit = width > Length.Zero ? width : Length.FromTwips(1);

        return border.Pattern switch
        {
            SheetBorderPattern.Dotted => [unit, unit],
            SheetBorderPattern.Dashed => [unit * 4, unit * 2],
            SheetBorderPattern.FineDashed => [unit * 3, unit * 3],
            SheetBorderPattern.DashDot => [unit * 4, unit * 2, unit, unit * 2],
            SheetBorderPattern.DashDotDot => [unit * 4, unit * 2, unit, unit * 2, unit, unit * 2],
            _ => null,
        };
    }

    private static void Rule(Length x1, Length y1, Length x2, Length y2, IDrawingSink sink)
        => sink.StrokePath(
            new GraphicsPath().MoveTo(new DocPoint(x1, y1)).LineTo(new DocPoint(x2, y2)),
            new Stroke(Paint.Solid(FurnitureColour), HairlineWidth));

    /// <summary>A heading cell: its box, and its label centred inside it.</summary>
    private static void Box(DocRect area, string label, IDrawingSink sink)
    {
        Outline(area, sink);

        Length size = SheetBandText.DefaultSize;
        if (SheetBandText.Shape(label, size) is not { } run) return;

        Length x = area.X + ((area.Width - run.Width) / 2);
        Length y = area.Y + ((area.Height - SheetBandText.LineHeightAt(size)) / 2)
                   + SheetBandText.AscentAt(size);

        sink.DrawGlyphRun(run.At(new DocPoint(x, y)), Paint.Solid(FurnitureColour));
    }

    /// <summary>
    /// A column's heading: A, B … Z, AA, AB and so on.
    /// </summary>
    /// <remarks>
    /// Bijective base 26, which is the part that catches people out: there is no zero digit, so
    /// column 26 is AA rather than BA and the remainder has to be taken before the division.
    /// <c>ScColToAlpha</c> (<c>sc/source/core/tool/address.cxx</c>) writes the same loop.
    /// </remarks>
    private static string ColumnName(int column)
    {
        if (column < 0) return string.Empty;

        Span<char> letters = stackalloc char[8];
        int at = letters.Length;
        int value = column;

        do
        {
            letters[--at] = (char)('A' + (value % 26));
            value = (value / 26) - 1;
        }
        while (value >= 0 && at > 0);

        return new string(letters[at..]);
    }

    private static void Outline(DocRect area, IDrawingSink sink)
    {
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return;

        sink.StrokePath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom))
                .LineTo(new DocPoint(area.X, area.Bottom))
                .Close(),
            new Stroke(Paint.Solid(FurnitureColour), HairlineWidth));
    }

    private static void Fill(DocRect area, Colour colour, IDrawingSink sink)
    {
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return;

        sink.FillPath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom))
                .LineTo(new DocPoint(area.X, area.Bottom))
                .Close(),
            Paint.Solid(colour));
    }

    /// <summary>One drawn border: where it sits, how far it runs, and what it is.</summary>
    /// <param name="IsHorizontal">True when it runs across the page.</param>
    /// <param name="At">Where it sits on the other axis.</param>
    /// <param name="From">Where it starts along its own axis.</param>
    /// <param name="To">Where it ends.</param>
    /// <param name="Border">The resolved border.</param>
    private readonly record struct Edge(
        bool IsHorizontal, Length At, Length From, Length To, SheetBorder Border);

    /// <summary>
    /// A page's borders, with the crossings indexed so an end can be extended.
    /// </summary>
    /// <remarks>
    /// Indexed on the coordinate in twips, because a corner is reached from two rectangles — a
    /// row's bottom and the next row's top — and the two can differ in the last EMU after the
    /// print scale has been applied.
    /// </remarks>
    private sealed class Edges
    {
        private readonly List<Edge> _edges = [];
        private readonly Dictionary<(bool Horizontal, long At, long Along), Length> _widths = [];

        public IReadOnlyList<Edge> All => _edges;

        public static Edges Build(
            SheetFormatting formatting,
            IReadOnlyList<PlacedColumn> columns,
            IReadOnlyList<PlacedRow> rows)
        {
            Edges edges = new();

            for (int r = 0; r < rows.Count; r++)
            {
                PlacedRow row = rows[r];

                for (int c = 0; c < columns.Count; c++)
                {
                    PlacedColumn column = columns[c];
                    SheetCellBorders own = formatting.At(row.Row, column.Column).Borders;

                    edges.Add(true, row.Y, column.X, column.Right, SheetCellBorders.Resolve(
                        own.Top, formatting.At(row.Row - 1, column.Column).Borders.Bottom));

                    edges.Add(false, column.X, row.Y, row.Bottom, SheetCellBorders.Resolve(
                        own.Left, formatting.At(row.Row, column.Column - 1).Borders.Right));

                    // The far edges only where nothing follows to cover them, because every
                    // other one is already the next cell's near edge — which is what keeps two
                    // neighbours agreeing about a border from drawing it twice. "Nothing
                    // follows" is not the same as "last on the page": a repeated header band is
                    // placed above a block it is not adjacent to, and a hidden row leaves the
                    // same gap, so the test is whether the next placed row really is this row's
                    // successor in the sheet. Calc reaches the same answer by drawing each band
                    // through its own ScOutputData (ScPrintFunc::PrintPage, printfun.cxx:2300).
                    if (r == rows.Count - 1 || rows[r + 1].Row != row.Row + 1)
                    {
                        edges.Add(true, row.Bottom, column.X, column.Right, SheetCellBorders.Resolve(
                            own.Bottom, formatting.At(row.Row + 1, column.Column).Borders.Top));
                    }

                    if (c == columns.Count - 1 || columns[c + 1].Column != column.Column + 1)
                    {
                        edges.Add(false, column.Right, row.Y, row.Bottom, SheetCellBorders.Resolve(
                            own.Right, formatting.At(row.Row, column.Column + 1).Borders.Left));
                    }
                }
            }

            return edges;
        }

        /// <summary>How far an end reaches past its own line to meet what crosses it.</summary>
        public Length ExtensionAt(Edge edge, Length end)
            => _widths.TryGetValue((!edge.IsHorizontal, end.Twips, edge.At.Twips), out Length width)
                ? width / 2
                : Length.Zero;

        private void Add(bool horizontal, Length at, Length from, Length to, SheetBorder border)
        {
            if (border.IsNone) return;

            _edges.Add(new Edge(horizontal, at, from, to, border));

            // Recorded at both ends so that either end of a crossing line can find it. The
            // narrowest wins a tie, which is what getExtends does when several lines meet at one
            // point: it takes the minimum of the cut sets.
            Note(horizontal, at, from, border.Width);
            Note(horizontal, at, to, border.Width);
        }

        private void Note(bool horizontal, Length at, Length along, Length width)
        {
            (bool, long, long) key = (horizontal, at.Twips, along.Twips);
            if (!_widths.TryGetValue(key, out Length existing) || width < existing)
                _widths[key] = width;
        }
    }
}
