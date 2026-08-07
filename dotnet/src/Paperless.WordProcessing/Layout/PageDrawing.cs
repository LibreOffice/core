using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Turns laid-out pages into drawing commands.
/// </summary>
/// <remarks>
/// <para>
/// One line becomes one glyph run, positioned at its baseline. Positioned rather than measured again:
/// layout already committed to these advances when it decided where the lines broke, so re-deriving the
/// positions here would risk output that disagrees with the breaks around it — which is exactly what
/// <see cref="GlyphRun"/>'s own contract says a backend must not do.
/// </para>
/// <para>
/// The glyphs come from shaping the line's own characters, and shaping the line rather than the paragraph
/// is a deliberate small inaccuracy: a kern pair straddling a line break does not apply, which is right,
/// but a contextual form that depended on a following character now sees the line's end instead. For the
/// Latin text this can currently measure, the two are the same.
/// </para>
/// </remarks>
public static class PageDrawing
{
    /// <summary>
    /// Draws a page into a sink: its header, its body and its footer.
    /// </summary>
    /// <remarks>
    /// The header first and the footer last, which is reading order and also the order a backend would
    /// prefer — nothing here overlaps, so the order is a convention rather than a correctness matter, but a
    /// recorded display list reads far better when it matches the page. The footnotes come after the body
    /// they belong to and before the footer, which is where they sit on the sheet, with their separator rule
    /// immediately before them.
    /// <para>
    /// The floating frames come after the body, which is paint order rather than reading order and is the
    /// one place the two differ: a frame with a background is opaque, and the text it displaced has
    /// already been shortened to keep clear of it — so a frame drawn first would be painted over by
    /// whatever ran under it.
    /// </para>
    /// </remarks>
    /// <param name="page">The page to draw.</param>
    /// <param name="blocks">The blocks the page's body lines index into.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public static void Draw(
        LaidOutPage page, IReadOnlyList<PageBlock> blocks, IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(page);
        ArgumentNullException.ThrowIfNull(blocks);
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(page.Size);
        try
        {
            DrawFlow(page.Header, sink);
            DrawBody(page, blocks, sink);
            foreach (PlacedTable table in page.Tables) DrawTable(table, sink);
            DrawSeparator(page.NoteSeparator, sink);
            DrawFlow(page.Notes, sink);
            foreach (PlacedFrame frame in page.Frames) DrawFrame(frame, sink);
            DrawFlow(page.Footer, sink);
        }
        finally
        {
            // Always closed, even if a sink throws part way through: a page left open would make the
            // next one nest inside it, turning one bad page into a broken document.
            sink.EndPage();
        }
    }

    /// <summary>
    /// Draws the body's lines, each relative to the rectangle of the column it landed in.
    /// </summary>
    /// <remarks>
    /// Grouped by column rather than looked up per line, because the rectangle is the same for every line of
    /// a column and computing it per line would divide the body's width by the column count once per line of
    /// the page. Single-column text — which is nearly everything — takes one group and one lookup.
    /// </remarks>
    private static void DrawBody(
        LaidOutPage page, IReadOnlyList<PageBlock> blocks, IDrawingSink sink)
    {
        if (page.ColumnCount <= 1)
        {
            DrawLines(page.BodyArea, page.Lines, blocks, sink);
            return;
        }

        for (int column = 0; column < page.ColumnCount; column++)
        {
            DocRect area = page.ColumnArea(column);
            int at = column;

            DrawLines(area, [.. page.Lines.Where(line => line.Column == at)], blocks, sink);
        }
    }

    /// <summary>
    /// Draws the rule above a page's notes.
    /// </summary>
    /// <remarks>
    /// Filled rather than stroked, which is what LibreOffice's own PDF export does: it writes the separator as
    /// a closed rectangular path and fills it, so its thickness is the rectangle's height rather than a pen
    /// width. Matching that is not pedantry — a stroke is centred on its path, so the same coordinates stroked
    /// would put half the rule's thickness on the wrong side of the line.
    /// </remarks>
    private static void DrawSeparator(DocRect? separator, IDrawingSink sink)
    {
        if (separator is { } rule) Fill(rule, Colour.Black, sink);
    }

    /// <summary>Draws a flow — a header, a footer or a cell — which is lines in their own rectangle.</summary>
    private static void DrawFlow(PlacedFlow? flow, IDrawingSink sink)
    {
        if (flow is null || flow.IsEmpty) return;

        DrawLines(flow.Area, flow.Lines, flow.Blocks, sink);
        foreach (PlacedTable table in flow.Tables) DrawTable(table, sink);
    }

    /// <summary>
    /// Draws a floating frame: its background, its own text, and its border.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Background, then content, then border — the order a table cell is drawn in and for the same reason:
    /// a border runs through the centre of its own line, so half of it overlaps whatever is inside.
    /// </para>
    /// <para>
    /// A picture goes between the two, for the same reason its own text would: it covers the background
    /// and the border is drawn over its edge. The image is handed over exactly as the reader found it —
    /// still the file's own bytes — because the backend is what has a codec, and a PDF backend given a
    /// JPEG passes it through to <c>DCTDecode</c> without ever decoding it. A frame the document called a
    /// picture whose bytes could not be found draws as it always did: its background and its border, and
    /// a hole where the pixels would have gone.
    /// </para>
    /// <para>
    /// <strong>A vector picture is stretched onto the frame by its own view box, not by its ink.</strong>
    /// <c>VectorImage.Draw</c> maps the picture's whole frame onto the destination and clips to it, which
    /// is the mapping LibreOffice uses for a <c>Graphic</c> on an <c>SdrObject</c>'s logic rectangle. This
    /// is the one call a reader gets wrong first: taking the extent of what the picture actually paints
    /// instead makes a logo with margins come out several times too large and clipped, which reads as a
    /// mapping bug in the decoder and is not one.
    /// </para>
    /// <para>
    /// The vector wins over the raster where a frame has both, which happens only for a DrawingML
    /// <c>svgBlip</c>. A decode that comes back empty falls through to the raster, which is what that
    /// fallback is written into the file for.
    /// </para>
    /// <para>
    /// <strong>A chart wins over both, and that ordering is the whole of the ODT case.</strong> ODF
    /// stores a chart as a <c>draw:object</c> followed by a <c>draw:image</c> of it — a picture of the
    /// chart for a reader that cannot embed one — so a frame holding a chart usually holds a replacement
    /// picture too. Every one LibreOffice writes is a <c>VCLMTF</c> StarView metafile, which nothing
    /// here decodes, so the fall-through costs nothing today; drawing the composed chart is still the
    /// right answer whatever the fallback turns out to be, because it is live geometry rather than a
    /// snapshot taken at some other size.
    /// </para>
    /// </remarks>
    private static void DrawFrame(PlacedFrame frame, IDrawingSink sink)
    {
        if (frame.Frame.Fill is { } fill) Fill(frame.Area, fill, sink);

        if (frame.Frame.Chart is { } chart)
            FrameChart.Draw(sink, chart, frame.Area, frame.Frame.ChartFontFamily);
        else if (frame.Frame.Vector is { } vector && !vector.Value.IsEmpty)
            vector.Value.Draw(sink, frame.Area);
        else if (frame.Frame.Image is { } image) sink.DrawImage(image, frame.Area);

        DrawFlow(frame.Content, sink);

        if (frame.Frame.BorderColour is not { } colour) return;
        if (frame.Frame.BorderWidth <= Length.Zero) return;

        Stroke stroke = new(Paint.Solid(colour), frame.Frame.BorderWidth);
        DocRect area = frame.Area;

        // A line shape's outline is its diagonal rather than its rectangle: corner to opposite corner,
        // which is the two-point path `ImportShape` builds for it, with the mirror flags choosing which
        // pair of corners. Drawing the box instead puts three sides on the page that are not in the file.
        if (frame.Frame.IsLine)
        {
            sink.StrokePath(
                new GraphicsPath()
                    .MoveTo(new DocPoint(area.X, frame.Frame.IsLineMirrored ? area.Bottom : area.Y))
                    .LineTo(new DocPoint(area.Right, frame.Frame.IsLineMirrored ? area.Y : area.Bottom)),
                stroke);
            return;
        }

        sink.StrokePath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom))
                .LineTo(new DocPoint(area.X, area.Bottom))
                .Close(),
            stroke);
    }

    /// <summary>
    /// Draws a table, which is its cells' text.
    /// </summary>
    /// <remarks>
    /// Shading behind the text and borders over it, which is paint order rather than preference: a border
    /// runs through the centre of a grid line, so half its width overlaps the cells either side of it.
    /// </remarks>
    private static void DrawTable(PlacedTable table, IDrawingSink sink)
    {
        // Every shade before any text, rather than each cell's shade before its own text: a shade is opaque,
        // and a cell whose content overflows into its neighbour would otherwise have that overflow painted
        // over by the neighbour's fill.
        foreach (PlacedTableCell cell in table.Cells)
        {
            if (cell.Cell.Shading is { } colour) Fill(cell.Area, colour, sink);
        }

        foreach (PlacedTableCell cell in table.Cells) DrawFlow(cell.Content, sink);

        DrawBorders(table, sink);
    }

    /// <summary>
    /// Draws a table's borders, consolidated the way LibreOffice consolidates them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One stroke per <em>grid line</em> rather than four round each cell, which is measured rather than
    /// chosen: LibreOffice writes five horizontals for a four-row table and one vertical per column boundary,
    /// each as a single <c>m … l S</c>. Drawing twelve short segments instead would be right on the page and
    /// incomparable against the reference.
    /// </para>
    /// <para>
    /// Two details of the geometry, both measured. A grid line runs through the <em>centre</em> of the border,
    /// and every stroke <strong>overshoots by half its own width at both ends</strong> — on a table spanning
    /// 56.7 to 538.6 pt the horizontals run 56.45 to 538.85. So the overshoot is what makes two perpendicular
    /// borders meet at a corner rather than leaving a notch.
    /// </para>
    /// <para>
    /// The overshoot is Writer's rule and not Word's, which is what
    /// <see cref="PageTable.JoinsBordersLikeWord"/> switches: a Word table shortens an interior line by the
    /// <em>full</em> width of the outer line it meets, so the outline owns the corner outright. Measured, the
    /// same table read from DOC or DOCX runs its middle horizontals 56.95 to 538.35 where the ODF one runs
    /// them 56.45 to 538.85.
    /// </para>
    /// <para>
    /// Horizontals before verticals, which matters only for which one wins a join and is free to match.
    /// </para>
    /// </remarks>
    private static void DrawBorders(PlacedTable table, IDrawingSink sink)
    {
        // Collected per grid line and merged, so that a run of cells agreeing about an edge becomes one
        // stroke. Keyed on the line's own coordinate rounded to a twip, because two cells' shared edge is
        // computed from two different rectangles and can differ in the last EMU.
        List<Edge> edges = Edges(table);
        if (table.Table.JoinsBordersLikeWord) edges = WithWordJoins(edges);

        foreach (Edge edge in edges)
        {
            Stroke stroke = new(Paint.Solid(edge.Border.Colour), edge.Border.Width);
            Length half = edge.Border.Width / 2;
            Length from = edge.From - half;
            Length to = edge.To + half;

            GraphicsPath path = edge.IsHorizontal
                ? new GraphicsPath()
                    .MoveTo(new DocPoint(from, edge.At))
                    .LineTo(new DocPoint(to, edge.At))
                : new GraphicsPath()
                    .MoveTo(new DocPoint(edge.At, from))
                    .LineTo(new DocPoint(edge.At, to));

            sink.StrokePath(path, stroke);
        }
    }

    /// <summary>One consolidated grid line: where it sits, how far it runs, and its border.</summary>
    /// <param name="IsHorizontal">True when it runs across the page.</param>
    /// <param name="At">Where it sits on the other axis.</param>
    /// <param name="From">Where it starts along its own axis.</param>
    /// <param name="To">Where it ends.</param>
    /// <param name="Border">Its width and colour.</param>
    /// <param name="IsOuter">
    /// True when it is part of the table's outline rather than of its grid, which only Word's join rule
    /// cares about: the outline keeps its full length and the inner lines give way to it.
    /// </param>
    private readonly record struct Edge(
        bool IsHorizontal, Length At, Length From, Length To, TableBorder Border, bool IsOuter = false);

    /// <summary>
    /// The grid lines with Word's joins applied: an inner line gives way to the outline it meets.
    /// </summary>
    /// <remarks>
    /// By the <em>full</em> width of the outer line rather than half of it, which is what makes the two
    /// rules differ by a whole border width at each end rather than by nothing. Ported from
    /// <c>SwTabFramePainter::FindStylesForLine</c>, which adjusts an inner entry's start and end for every
    /// outer entry it meets there, and does it before the half-width overshoot is added.
    /// </remarks>
    private static List<Edge> WithWordJoins(List<Edge> edges)
    {
        // Keyed on the coordinate in twips for the same reason the merge is: two cells' shared edge comes
        // from two rectangles and can differ in the last EMU. The width is the widest outline stroke at
        // that coordinate, since that is the one whose corner has to be cleared.
        Dictionary<(bool, long), Length> outline = [];
        foreach (Edge edge in edges)
        {
            if (!edge.IsOuter) continue;

            (bool, long) key = (edge.IsHorizontal, edge.At.Twips);
            if (!outline.TryGetValue(key, out Length width) || edge.Border.Width > width)
                outline[key] = edge.Border.Width;
        }

        List<Edge> joined = new(edges.Count);
        foreach (Edge edge in edges)
        {
            if (edge.IsOuter)
            {
                joined.Add(edge);
                continue;
            }

            joined.Add(edge with
            {
                From = edge.From + Meeting(edge, edge.From),
                To = edge.To - Meeting(edge, edge.To),
            });
        }

        return joined;

        Length Meeting(Edge edge, Length end)
            => outline.TryGetValue((!edge.IsHorizontal, end.Twips), out Length width)
                ? width
                : Length.Zero;
    }

    /// <summary>
    /// A table's grid lines, merged along each line where consecutive cells agree.
    /// </summary>
    /// <remarks>
    /// Horizontals first and then verticals, each built by grouping the cells' edges on the line they sit on
    /// and joining the runs that touch. A vertical therefore stops where its boundary stops, which is what
    /// LibreOffice does: a table whose last row spans two columns leaves that column's stroke short.
    /// </remarks>
    private static List<Edge> Edges(PlacedTable table)
    {
        List<Edge> loose = [];
        int columns = table.Table.ColumnWidths.Count;

        foreach (PlacedTableCell cell in table.Cells)
        {
            CellBorders borders = cell.Cell.Borders;
            DocRect area = cell.Area;

            // Which of a cell's edges belong to the table's outline, taken from where the cell sits
            // rather than from where its rectangle lands: a row whose cells are short of the grid still
            // has a last cell, and its right edge is still the outline.
            bool first = cell.Row <= table.FirstRow;
            bool last = cell.Row + Math.Max(1, cell.Cell.RowSpan) >= table.RowEnd;

            if (!borders.Top.IsNone)
                loose.Add(new Edge(true, area.Y, area.X, area.Right, borders.Top, first));
            if (!borders.Bottom.IsNone)
                loose.Add(new Edge(true, area.Bottom, area.X, area.Right, borders.Bottom, last));
            if (!borders.Left.IsNone)
                loose.Add(new Edge(false, area.X, area.Y, area.Bottom, borders.Left,
                    cell.Cell.Column == 0));
            if (!borders.Right.IsNone)
                loose.Add(new Edge(false, area.Right, area.Y, area.Bottom, borders.Right,
                    cell.Cell.ColumnEnd >= columns));
        }

        List<Edge> merged = [];

        foreach (bool horizontal in (bool[])[true, false])
        {
            IEnumerable<IGrouping<(long, Length, Colour), Edge>> lines = loose
                .Where(edge => edge.IsHorizontal == horizontal)
                .GroupBy(edge => (edge.At.Twips, edge.Border.Width, edge.Border.Colour));

            foreach (IGrouping<(long, Length, Colour), Edge> line in lines)
            {
                foreach (Edge run in Merge([.. line.OrderBy(edge => edge.From.Emu)]))
                {
                    merged.Add(run);
                }
            }
        }

        return merged;
    }

    /// <summary>Joins the runs along one grid line that touch or overlap.</summary>
    /// <remarks>
    /// Touching counts, and has to: two cells side by side produce two separate edges that meet exactly at
    /// the boundary between them, and a reference that wrote one stroke across both would disagree with two.
    /// </remarks>
    private static List<Edge> Merge(List<Edge> sorted)
    {
        List<Edge> runs = [];

        foreach (Edge edge in sorted)
        {
            if (runs.Count > 0 && edge.From <= runs[^1].To)
            {
                // Outer wins over inner across a merge, which matters for a run that starts as the
                // outline of one row and continues as the grid line of the next.
                runs[^1] = runs[^1] with
                {
                    To = edge.To > runs[^1].To ? edge.To : runs[^1].To,
                    IsOuter = runs[^1].IsOuter || edge.IsOuter,
                };
                continue;
            }

            runs.Add(edge);
        }

        return runs;
    }

    /// <summary>Fills a rectangle, which is what a shade and a rule both are.</summary>
    private static void Fill(DocRect area, Colour colour, IDrawingSink sink)
    {
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return;

        GraphicsPath path = new GraphicsPath()
            .MoveTo(new DocPoint(area.X, area.Y))
            .LineTo(new DocPoint(area.Right, area.Y))
            .LineTo(new DocPoint(area.Right, area.Bottom))
            .LineTo(new DocPoint(area.X, area.Bottom))
            .Close();

        sink.FillPath(path, Paint.Solid(colour));
    }

    /// <summary>
    /// Draws a set of placed lines relative to an area.
    /// </summary>
    /// <remarks>
    /// One path for the body, the header and the footer, because they differ only in which rectangle their
    /// coordinates are relative to. A header drawn by its own code would be the second place tabs and
    /// per-run formatting had to be applied, and the two would drift.
    /// </remarks>
    private static void DrawLines(
        DocRect area,
        IReadOnlyList<PlacedLine> lines,
        IReadOnlyList<PageBlock> blocks,
        IDrawingSink sink)
    {
        DrawParagraphShading(area, lines, blocks, sink);

        foreach (PlacedLine line in lines)
        {
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;
            if (blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;

            List<(DocRect Area, Colour Colour)> highlights = [];
            List<(DocRect Area, Colour Colour)> rules = [];
            List<(GlyphRun Run, Colour Colour)> runs =
                RunsIn(area, line, paragraph, highlights, rules);

            // Every band on the line before any of its glyphs, not band-then-glyphs run by run: two
            // adjacent highlighted runs overlap by a fraction of a point where one's advance ends and the
            // next begins, and painting a band after its neighbour's text has been drawn clips the text.
            foreach ((DocRect band, Colour colour) in highlights) Fill(band, colour, sink);

            foreach ((GlyphRun run, Colour colour) in runs)
            {
                sink.DrawGlyphRun(run, Paint.Solid(colour));
            }

            // After the glyphs, which is the order every other layer here draws a decoration in and the
            // order Writer paints one: a strikethrough belongs over the letters it crosses out, and an
            // underline that a descender interrupts is what a font's own offset already expresses.
            foreach ((DocRect rule, Colour colour) in rules) Fill(rule, colour, sink);
        }
    }

    /// <summary>
    /// Fills the background behind each shaded paragraph, before any of the text is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every shade before any glyph, for the reason a table's cells are drawn that way: a fill is opaque,
    /// and a shaded paragraph drawn after its neighbour would paint over the descenders hanging into it.
    /// </para>
    /// <para>
    /// <strong>The rectangle is the paragraph's print area, not its frame.</strong> Writer paints a text
    /// frame's background over <c>lcl_CalcBorderRect</c>'s rectangle —
    /// <c>getFramePrintArea() + getFrameArea().Pos()</c>, <c>sw/source/core/layout/paintfrm.cxx:1265</c> —
    /// so the fill spans the indents rather than the whole column, and it stops at the first and last
    /// line rather than covering the space before and after the paragraph. Measured on a shaded
    /// paragraph indented 720 twips with 400 twips of spacing either side, LibreOffice fills exactly the
    /// indented line stack and leaves both spacings white.
    /// </para>
    /// <para>
    /// The one exception is the join, and it is why a run of same-coloured headings reads as one bar
    /// rather than as stripes: when the previous frame carries the same background, the rectangle's top
    /// is pulled up to the frame's top — <c>aRect.Top( getFrameArea().Top() )</c>,
    /// <c>paintfrm.cxx:7033</c> — which is where the paragraph before it stopped filling. So the space
    /// between two identically shaded paragraphs is filled and the space between two differently shaded
    /// ones is not, both of which are measurable and neither of which follows from the other.
    /// </para>
    /// </remarks>
    private static void DrawParagraphShading(
        DocRect area,
        IReadOnlyList<PlacedLine> lines,
        IReadOnlyList<PageBlock> blocks,
        IDrawingSink sink)
    {
        // The run being accumulated: one or more consecutive paragraphs that agree about their colour and
        // their edges, and so become a single rectangle. Emitting one per paragraph would be the same
        // coverage and not the same picture — two abutting fills leave a blended seam a rasteriser cannot
        // avoid, which reads as a pale rule across a shaded heading.
        Colour? colour = null;
        DocRect run = default;
        int last = -2;

        int index = -1;
        Length top = Length.Zero;
        Length bottom = Length.Zero;

        void Emit()
        {
            if (colour is { } fill) Fill(run, fill, sink);
            colour = null;
        }

        void Flush()
        {
            if (index < 0 || blocks[index] is not PageParagraph paragraph) return;
            if (paragraph.Shading is not { } fill)
            {
                Emit();
                last = -2;
                return;
            }

            DocRect next = ShadeArea(area, paragraph, top, bottom);

            // Joined when the paragraph immediately before was filled the same way: the rectangle grows
            // downwards over whatever sat between the two, which is the space one's spacing-after and the
            // other's spacing-before left blank.
            if (colour == fill && last == index - 1 && next.X == run.X && next.Width == run.Width)
            {
                run = new DocRect(run.X, run.Y, run.Width, next.Bottom - run.Y);
            }
            else
            {
                Emit();
                colour = fill;
                run = next;
            }

            last = index;
        }

        foreach (PlacedLine line in lines)
        {
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;

            if (line.ParagraphIndex != index)
            {
                Flush();
                index = line.ParagraphIndex;
                top = line.Top;
            }

            bottom = line.Top + line.Box.Height;
        }

        Flush();
        Emit();
    }

    /// <summary>The rectangle a paragraph's shading fills, in the coordinates of the area holding it.</summary>
    /// <remarks>
    /// The indents narrow it from both sides, and which side each one is on depends on the paragraph's
    /// direction — a right-to-left paragraph's start indent is its right edge. The first-line indent
    /// deliberately does not narrow it: it moves one line's text, not the paragraph's print area.
    /// </remarks>
    private static DocRect ShadeArea(
        DocRect area, PageParagraph paragraph, Length top, Length bottom)
    {
        ParagraphFormat format = paragraph.DeclaredFormat;
        Length before = format.IsRightToLeft ? format.EndIndent : format.StartIndent;
        Length after = format.IsRightToLeft ? format.StartIndent : format.EndIndent;

        Length left = area.X + Length.Max(before, Length.Zero);
        Length right = area.Right - Length.Max(after, Length.Zero);

        return new DocRect(left, area.Y + top, right - left, bottom - top);
    }

    /// <summary>
    /// The glyph runs one line draws: one per formatting change, and one per tab.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A paragraph with uniform formatting and no tabs draws one run per line, which is the common case and
    /// the cheap one. Formatting splits it further — a bold phrase crossing a line break becomes two runs,
    /// one on each line, because a glyph run is one font at one size at one position and a line break is a
    /// position — and so do tabs, because the text after a tab starts at a stop rather than where the text
    /// before it ended.
    /// </para>
    /// <para>
    /// Within a stretch the pen advances across the runs rather than restarting per run, so the second run
    /// of a stretch starts where the first ended. Measuring each from zero would stack them all at the
    /// margin.
    /// </para>
    /// </remarks>
    public static List<(GlyphRun Run, Colour Colour)> RunsFor(
        LaidOutPage page, PlacedLine line, PageParagraph paragraph)
    {
        ArgumentNullException.ThrowIfNull(page);
        return RunsIn(page.ColumnArea(line.Column), line, paragraph);
    }

    /// <summary>The glyph runs one line draws, relative to whichever area it belongs to.</summary>
    /// <param name="area">The rectangle the line's coordinates are relative to.</param>
    /// <param name="line">The line.</param>
    /// <param name="paragraph">Its paragraph.</param>
    /// <param name="highlights">
    /// Collects the coloured band behind each highlighted run, or null when the caller wants only the
    /// glyphs. Out of this walk rather than a second one, because the band's left edge and width are the
    /// pen positions the tab stops and the justification decided here — recomputing them elsewhere would
    /// be a second place for that arithmetic to be got right.
    /// </param>
    /// <param name="rules">
    /// Collects the underline and strikethrough rectangles each decorated run asks for, or null when the
    /// caller wants only the glyphs. Out of this walk for the same reason the bands are: a rule spans the
    /// advance the pen just measured, and its offset and thickness come from the face this walk resolved.
    /// </param>
    public static List<(GlyphRun Run, Colour Colour)> RunsIn(
        DocRect area,
        PlacedLine line,
        PageParagraph paragraph,
        List<(DocRect Area, Colour Colour)>? highlights = null,
        List<(DocRect Area, Colour Colour)>? rules = null)
    {
        ArgumentNullException.ThrowIfNull(paragraph);

        List<(GlyphRun, Colour)> runs = [];

        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.VisibleEnd, paragraph.Text.Length);

        Length lineLeft = area.X + line.Box.Left;
        Length baseline = area.Y + line.Baseline;

        // Before the text and before the empty-line exit, because an item with no words still has a
        // number: an empty list paragraph draws its label and nothing else, which is what LibreOffice
        // does and what a list being typed into looks like.
        if (line.StartsParagraph && paragraph.Label is { Text.Length: > 0 } label)
        {
            ShapedText shapedLabel = TextShaper.Default.Shape(label.Face, label.Text, label.Shaping);
            if (shapedLabel.Glyphs.Count > 0)
            {
                runs.Add((
                    Build(
                        shapedLabel,
                        label.Text,
                        label.EmSize,
                        label.Font ?? Reference(label.Face),
                        new DocPoint(lineLeft - paragraph.LabelAdvance, baseline),
                        Length.Zero),
                    label.Colour.A == 0 ? Colour.Black : label.Colour));
            }
        }

        if (end <= start) return runs;

        List<TabbedSegment> stretches = Stretches(paragraph, start, end, line.StartsParagraph);

        for (int index = 0; index < stretches.Count; index++)
        {
            TabbedSegment segment = stretches[index];

            // Before the emptiness test: a tab followed by nothing still draws its leader, which is what
            // a table-of-contents line whose page number sits on the next line looks like.
            if (Leader(paragraph, segment, lineLeft, baseline) is { } filled) runs.Add(filled);

            if (segment.IsEmpty) continue;

            // The justification belongs to the last stretch alone. A tab is a fixed portion whose glue is
            // nought, so the stretch it closes is stretched by nothing and only the last one reaches the
            // right margin's glue — see `ParagraphLayouter.Justification`, which counts the same blanks.
            Length spaceAdd = index == stretches.Count - 1 ? line.Box.SpaceAdd : Length.Zero;

            Length pen = lineLeft + segment.Left;

            // The as-character objects on this stretch, in position order, consumed as the pen reaches
            // them. An object contributes no glyphs — the frame is drawn separately, by `DrawFrame` at
            // the rectangle `FrameLayout` hung it at — so all the text pass owes it is the room it takes.
            int nextObject = 0;
            List<InlineObject> onStretch = paragraph.HasInlineObjects
                ? [.. paragraph.InlineObjects
                    .Where(one => one.Offset >= segment.Start && one.Offset < segment.End)
                    .OrderBy(one => one.Offset)]
                : [];

            foreach (PageRun run in InVisualOrder(paragraph, segment.Start, segment.End))
            {
                while (nextObject < onStretch.Count && onStretch[nextObject].Offset <= run.Start)
                {
                    pen += onStretch[nextObject].Width;
                    nextObject++;
                }

                string text = paragraph.Text[run.Start..run.End];
                ShapedText shaped = TextShaper.Default.Shape(run.Face, text, run.Shaping);
                if (shaped.Glyphs.Count == 0) continue;

                // A raised run draws above the baseline and advances along it unchanged, which is why the
                // rise moves the origin rather than the glyphs: the pen below has to carry on from where an
                // unraised run would have left it.
                GlyphRun glyphRun = Build(
                    shaped,
                    text,
                    run.EmSize,
                    run.Font ?? Reference(run.Face),
                    new DocPoint(pen, baseline - run.Rise),
                    spaceAdd,
                    run.Tracking);

                runs.Add((glyphRun, run.EffectiveColour));

                // The pen carries the justification with it, or the second run on a stretched line would
                // start where the first would have ended unjustified and overlap the words before it.
                Length extent = Extent(glyphRun);

                if (highlights is not null && run.IsHighlighted)
                {
                    highlights.Add((Band(paragraph, run, pen, extent, baseline), run.Highlight));
                }

                if (rules is not null && run.IsDecorated)
                {
                    Rules(run, pen, extent, baseline, rules);
                }

                pen += extent;
            }
        }

        return runs;
    }

    /// <summary>
    /// The coloured band behind one highlighted run.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer paints a character background over the <em>portion's</em> box rather than the line's:
    /// <c>SwTextPaintInfo::CalcRect</c> (<c>sw/source/core/text/inftxt.cxx</c>) takes the rectangle from
    /// the baseline less the portion's ascent, with the portion's own height. So the band follows the
    /// run's face and size, not the tallest thing on the line — which is what stops a highlighted word in
    /// a footnote-sized face from being given a band as tall as the heading beside it, and what stops a
    /// double-spaced paragraph from being highlighted across the whole of its leading.
    /// </para>
    /// <para>
    /// The metrics are resolved through the same <see cref="LineSpacing"/> call and the same device grid
    /// the measurement used, so the band's height is the height layout gave the run rather than a second
    /// opinion about it.
    /// </para>
    /// </remarks>
    private static DocRect Band(
        PageParagraph paragraph, PageRun run, Length pen, Length extent, Length baseline)
    {
        LineMetrics metrics = LineSpacing.Resolve(run.Face, paragraph.Metrics);
        Length size = run.MetricEmSize > Length.Zero ? run.MetricEmSize : run.EmSize;

        Length ascent = metrics.ScaledAscent(size);
        Length height = metrics.ScaledLineHeight(size);
        if (height <= Length.Zero) return default;

        // The rise moves the band with the text: a highlighted superscript is banded where it is drawn.
        return new DocRect(pen, baseline - run.Rise - ascent, extent, height);
    }

    /// <summary>
    /// The rules drawn under and through one decorated run.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A decoration is not shaped. It is a rectangle the output device fills across the run's advance,
    /// which is why it can be added here without any measurement changing: the glyphs had that advance
    /// already. The offset and the thickness are the face's own <c>post</c> and <c>OS/2</c> numbers
    /// through <see cref="LineSpacing.ResolveDecorations(OpenTypeFace, LineMetrics)"/> — the same call
    /// the slide and the cell layers make, including its refusal to believe the three Liberation faces'
    /// <c>post</c> tables, which is LibreOffice's own shipped <c>FontsDontUseUnderlineMetrics</c> and
    /// matters here more than anywhere: those three are what a corpus set in Arial, Times New Roman and
    /// Courier New actually resolves to.
    /// </para>
    /// <para>
    /// No <see cref="MetricGrid"/> is passed, unlike <see cref="Band"/>, and deliberately: a grid
    /// quantises the <em>scaling</em> of a metric onto device pixels and this resolution reads design
    /// units, so a printer-metrics document would get an identical answer from a grid it had to be
    /// threaded here to supply.
    /// </para>
    /// <para>
    /// Per run rather than per line, which is what makes an underlined phrase inside a plain sentence
    /// underline only itself. Two adjacent underlined runs abut, since each spans exactly the advance
    /// the pen charged it — there is no gap to bridge and no overlap to double-darken.
    /// </para>
    /// </remarks>
    private static void Rules(
        PageRun run,
        Length pen,
        Length extent,
        Length baseline,
        List<(DocRect Area, Colour Colour)> rules)
    {
        if (run.EmSize <= Length.Zero || extent <= Length.Zero) return;

        int unitsPerEm = run.Face.UnitsPerEm > 0 ? run.Face.UnitsPerEm : 1000;
        FontVerticalMetrics metrics =
            LineSpacing.ResolveDecorations(run.Face, LineSpacing.Resolve(run.Face));

        Length Scaled(int designUnits) => run.EmSize * ((double)designUnits / unitsPerEm);

        // The rise carries the rules with the text, exactly as it carries the band: a struck-through
        // superscript is struck where it is drawn rather than where it would have sat unraised.
        Length baselineOfRun = baseline - run.Rise;

        if (run.IsUnderlined)
        {
            // The face records the underline's offset as negative below the baseline.
            Length thickness = Scaled(metrics.UnderlineThickness);
            if (thickness > Length.Zero)
            {
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun - Scaled(metrics.UnderlinePosition), extent, thickness),
                    run.EffectiveColour));
            }
        }

        if (run.IsStruckThrough)
        {
            Length thickness = Scaled(metrics.StrikeoutThickness);
            if (thickness > Length.Zero)
            {
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun - Scaled(metrics.StrikeoutPosition), extent, thickness),
                    run.EffectiveColour));
            }
        }
    }

    /// <summary>
    /// How many fill characters one tab may draw, however small the face and however wide the blank.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input, in the same spirit as <see cref="TabRuler.MaxSegments"/>. A page-wide
    /// blank filled at a plausible size holds a few hundred dots; a document declaring a one-EMU face
    /// would ask for billions, and each one costs a glyph.
    /// </remarks>
    private const int MaxLeaderCharacters = 4096;

    /// <summary>
    /// The run of fill characters a tab draws across the blank it advanced over, if it has one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The dot leader of a table of contents, and the port of <c>SwTabPortion::Paint</c>
    /// (<c>sw/source/core/text/txttab.cxx:648-659</c>): the blank's width divided by one fill character's,
    /// truncated, so the fill never runs past the stop it stops at. Underscore takes one extra, because
    /// its glyph spans its whole advance and a truncated run of them shows the rounding as visible gaps —
    /// Writer makes the same exception and for the same reason.
    /// </para>
    /// <para>
    /// Drawn in the face in effect <em>at the tab</em> rather than the one after it, which is what
    /// <c>rInf.GetFont()</c> means at that point in Writer. A contents line whose title is bold and whose
    /// page number is not would otherwise draw bold dots between them.
    /// </para>
    /// <para>
    /// This paints inside space the tab had already reserved, so it moves no line break and no page
    /// break: a paragraph measures exactly as it did before the leader existed.
    /// </para>
    /// </remarks>
    private static (GlyphRun Run, Colour Colour)? Leader(
        PageParagraph paragraph, TabbedSegment segment, Length lineLeft, Length baseline)
    {
        if (!segment.HasLeader) return null;

        PageRun at = RunAt(paragraph, segment.Start - 1);

        Length unit = TextShaper.Default
            .Shape(at.Face, segment.Leader.ToString(), at.Shaping)
            .Width(at.EmSize);
        if (unit <= Length.Zero) return null;

        long count = segment.GapWidth.Emu / unit.Emu;
        if (segment.Leader == '_') count++;
        if (count <= 0) return null;

        string fill = new(segment.Leader, (int)Math.Min(count, MaxLeaderCharacters));
        ShapedText shaped = TextShaper.Default.Shape(at.Face, fill, at.Shaping);
        if (shaped.Glyphs.Count == 0) return null;

        return (
            Build(
                shaped,
                fill,
                at.EmSize,
                at.Font ?? Reference(at.Face),
                new DocPoint(lineLeft + segment.GapLeft, baseline - at.Rise),
                Length.Zero),
            at.EffectiveColour);
    }

    /// <summary>
    /// The formatting run covering a character, or the paragraph's own formatting where none does.
    /// </summary>
    /// <remarks>
    /// Asked for the tab character itself, which sits at the end of the stretch before the one the stop
    /// placed — so a position before the paragraph's first character, or past its last, falls back rather
    /// than failing.
    /// </remarks>
    private static PageRun RunAt(PageParagraph paragraph, int at)
    {
        if (paragraph.HasRuns)
        {
            foreach (PageRun run in paragraph.Runs)
            {
                if (at >= run.Start && at < run.End) return run;
            }
        }

        return new PageRun(
            Math.Max(at, 0),
            0,
            paragraph.Face,
            paragraph.EmSize,
            paragraph.Font,
            paragraph.Colour,
            paragraph.Shaping);
    }

    /// <summary>
    /// The stretches a line is divided into by its tabs, each placed at its stop.
    /// </summary>
    /// <remarks>
    /// One stretch covering the whole line when there is no tab, which is nearly always — and it goes
    /// through the same code path so that a tabbed line and an untabbed one cannot drift apart. The
    /// measurement handed to the ruler is the same one the layout used, so the stops land in the same
    /// places here as they did when the line's width was decided.
    /// </remarks>
    private static List<TabbedSegment> Stretches(
        PageParagraph paragraph, int start, int end, bool isFirstLine)
    {
        if (!TabRuler.HasTab(paragraph.Text, start, end))
        {
            return [new TabbedSegment(start, end, Length.Zero, Length.Zero)];
        }

        return TabRuler.Segments(
            paragraph.Text,
            start,
            end,
            paragraph.Format,
            (from, to) => WidthBetween(paragraph, from, to),
            isFirstLine);
    }

    /// <summary>
    /// The formatting runs covering a stretch, clipped to it, in order.
    /// </summary>
    /// <remarks>
    /// One synthetic run for a uniform paragraph, so the drawing loop does not need two shapes. Ordered by
    /// position rather than trusted to arrive that way: a run list out of order would draw the line's words
    /// in the wrong places, and the readers build it from four different formats.
    /// </remarks>
    private static List<PageRun> RunsIn(PageParagraph paragraph, int start, int end)
    {
        if (!paragraph.HasRuns)
        {
            return AroundObjects(
                paragraph,
                [
                    new PageRun(
                        start,
                        end - start,
                        paragraph.Face,
                        paragraph.EmSize,
                        paragraph.Font,
                        paragraph.Colour,
                        paragraph.Shaping,
                        Tracking: paragraph.Tracking),
                ]);
        }

        List<PageRun> clipped = [];
        foreach (PageRun run in paragraph.Runs.OrderBy(run => run.Start))
        {
            int from = Math.Max(run.Start, start);
            int to = Math.Min(run.End, end);
            if (to <= from) continue;

            clipped.Add(run with { Start = from, Length = to - from });
        }

        return AroundObjects(paragraph, clipped);
    }

    /// <summary>
    /// The runs cut at every as-character object's boundary, so the pen has somewhere to jump.
    /// </summary>
    /// <remarks>
    /// The same cut <see cref="MeasuredParagraph"/> makes before it shapes, and it has to be the same one:
    /// the pen advances by what it draws, so text after a picture starts where the run before the picture
    /// ended plus the picture's width — and a run drawn across the boundary would draw the whole sentence
    /// from one origin and put the words after the picture underneath it.
    /// </remarks>
    private static List<PageRun> AroundObjects(PageParagraph paragraph, List<PageRun> runs)
    {
        if (!paragraph.HasInlineObjects) return runs;

        List<PageRun> cut = [];

        foreach (PageRun run in runs)
        {
            int at = run.Start;

            foreach (InlineObject one in paragraph.InlineObjects.OrderBy(one => one.Offset))
            {
                if (one.Offset <= at || one.Offset >= run.End) continue;

                cut.Add(run with { Start = at, Length = one.Offset - at });
                at = one.Offset;
            }

            if (at < run.End) cut.Add(run with { Start = at, Length = run.End - at });
        }

        return cut;
    }

    /// <summary>
    /// The runs a stretch draws, in the order they are drawn left to right.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Rule L2 over the runs, which is the whole of what makes a mixed-direction line readable: the
    /// runs are stored in logical order and drawn in visual order, so a Hebrew phrase between two
    /// English ones is drawn in the middle and its own words run the other way. Writer arrives at
    /// the same place by a different route — it keeps the portions logical and jumps the pen by the
    /// whole width of a bidi portion whose direction differs from its surroundings
    /// (<c>SwTextPainter::PaintMultiPortion</c>, <c>sw/source/core/text/pormulti.cxx:1630</c>) —
    /// and the result is identical, because both are rule L2.
    /// </para>
    /// <para>
    /// The split is the itemiser's, not a second one: the same sub-runs the paragraph was
    /// <em>measured</em> against, so a line's drawn width is the width its break was decided with.
    /// Each piece is told its script and its direction, which is not decoration — a shaper handed a
    /// Hebrew run without them lays its glyphs out left to right and the word comes out reversed.
    /// </para>
    /// <para>
    /// A paragraph with nothing right-to-left in it never gets here:
    /// <see cref="TextItemiser.MayReorder"/> is checked first, and the runs are returned exactly as
    /// they were. That matters more than it
    /// looks — a run split at a boundary it does not need loses its shaping context and measures
    /// very slightly wide, which is enough to move a line break.
    /// </para>
    /// </remarks>
    private static List<PageRun> InVisualOrder(PageParagraph paragraph, int start, int end)
    {
        List<(PageRun Run, byte Level)> pieces = Pieces(paragraph, start, end);
        if (pieces.Count == 0) return RunsIn(paragraph, start, end);

        TextItemiser.ReorderVisually(pieces, piece => piece.Level);
        return [.. pieces.Select(piece => piece.Run)];
    }

    /// <summary>
    /// A stretch cut into the pieces one direction and one script each, in logical order.
    /// </summary>
    /// <remarks>
    /// Empty when the paragraph cannot reorder, which is how both callers say "use the runs as they
    /// are" without either of them repeating the test.
    /// </remarks>
    private static List<(PageRun Run, byte Level)> Pieces(
        PageParagraph paragraph, int start, int end)
    {
        BidiDirection direction = paragraph.BaseDirection;
        if (!TextItemiser.MayReorder(paragraph.Text, direction)) return [];

        List<TextItem> items = TextItemiser.Itemise(paragraph.Text, direction);
        List<(PageRun, byte)> pieces = [];

        foreach (PageRun run in RunsIn(paragraph, start, end))
        {
            foreach (TextItem item in items)
            {
                int from = Math.Max(run.Start, item.Start);
                int to = Math.Min(run.End, item.End);
                if (to <= from) continue;

                pieces.Add((
                    run with
                    {
                        Start = from,
                        Length = to - from,
                        Shaping = run.Shaping with
                        {
                            Script = item.Script,
                            RightToLeft = item.IsRightToLeft,
                        },
                    },
                    item.Level));
            }
        }

        return pieces;
    }

    /// <summary>
    /// How far along a line a character position sits, measured from where the line's text starts.
    /// </summary>
    /// <remarks>
    /// <para>
    /// What an as-character anchor needs: an inline picture hangs at the position its anchor character
    /// occupies, and that position is a sum of glyph advances rather than anything pagination recorded.
    /// Answered through the same stretches and the same shaping the line will be <em>drawn</em> with, so
    /// that the picture cannot land somewhere the words around it disagree with — which is the failure
    /// re-measuring in a second way would produce, and it would be invisible in every document without a
    /// tab in it.
    /// </para>
    /// <para>
    /// A position on a later stretch of a tabbed line carries that stretch's own start, so a picture after
    /// a tab hangs at the stop rather than where the text before the tab ended.
    /// </para>
    /// </remarks>
    /// <param name="paragraph">The paragraph the line belongs to.</param>
    /// <param name="line">The line, whose own range bounds the answer.</param>
    /// <param name="at">The character position, as an index into the paragraph's text.</param>
    internal static Length OffsetOnLine(PageParagraph paragraph, PlacedLine line, int at)
    {
        ArgumentNullException.ThrowIfNull(paragraph);

        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.End, paragraph.Text.Length);
        int position = Math.Clamp(at, start, end);

        foreach (TabbedSegment segment in Stretches(paragraph, start, end, line.StartsParagraph))
        {
            if (position > segment.End) continue;

            return segment.Left + WidthBetween(paragraph, segment.Start, position);
        }

        return Length.Zero;
    }

    /// <summary>
    /// The width of a range of a paragraph's text, in whichever faces cover it.
    /// </summary>
    /// <remarks>
    /// Shaped here rather than taken from the layout, because what reaches a page is a
    /// <see cref="PageParagraph"/> and its line boxes — not the measured paragraph the layout built. The
    /// two agree because both shape the same text in the same faces with the same options; the cost is one
    /// extra shaping pass per tabbed stretch, and only tabbed stretches ask.
    /// </remarks>
    private static Length WidthBetween(PageParagraph paragraph, int from, int to)
    {
        Length total = Length.Zero;

        // The as-character objects the range crosses, on the same half-open rule the prefix table uses —
        // an object at `from` belongs to this range and one at `to` to the next. So the tab stop a line
        // with a picture before it reaches is the stop the layout measured, and the picture's own left
        // edge, which is asked for as the width up to its own offset, does not include itself.
        foreach (InlineObject one in paragraph.InlineObjects)
        {
            if (one.Offset >= from && one.Offset < to) total += one.Width;
        }

        // The same pieces the stretch will be drawn in, unordered — a width is a sum and does not
        // care which way round they go, but it does care that they were shaped the same way, or a
        // tab in a mixed-direction line would advance to a stop the text does not reach.
        List<(PageRun Run, byte Level)> pieces = Pieces(paragraph, from, to);

        foreach (PageRun run in pieces.Count > 0
                     ? pieces.Select(piece => piece.Run)
                     : RunsIn(paragraph, from, to))
        {
            string text = paragraph.Text[run.Start..run.End];
            total += TextShaper.Default.Shape(run.Face, text, run.Shaping).Width(run.EmSize);

            // One tracking unit per character, which is exactly what the prefix table charges across a
            // range: it puts the gap *before* each character, so a range of n of them carries n. Any other
            // count here would put a tab stop somewhere the layout did not measure.
            if (run.Tracking != Length.Zero) total += run.Tracking * run.Length;
        }

        return total;
    }

    /// <summary>
    /// Builds a glyph run from a shaped stretch of text at an origin.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each glyph's offset is relative to the run's origin, and the pen accumulates across them — which is
    /// what makes a run one draw call rather than one per glyph. The vertical offset is negated because a
    /// shaper's is up-positive and document space is down-positive; getting that backwards puts every
    /// accent below the letter it belongs to.
    /// </para>
    /// <para>
    /// Justification lands here, on the advance of each blank, which is where Writer puts it too: its kern
    /// array adds the space to the blank's own entry (<c>SwFntObj::DrawText</c>) rather than shifting the
    /// words. That keeps a run one draw call and keeps the glyph positions self-consistent, so a backend
    /// that re-measured would still agree with the line's extent.
    /// </para>
    /// </remarks>
    private static GlyphRun Build(
        ShapedText shaped,
        string text,
        Length emSize,
        FontReference font,
        DocPoint origin,
        Length spaceAdd,
        Length tracking = default)
    {
        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);

        Length pen = Length.Zero;
        int remaining = shaped.Glyphs.Count;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, emSize);

            // Tracking is the gap *between* characters, so the run's last glyph does not carry one —
            // which is what the reference draws (`SvxFont::QuickGetTextSize` adds one per advance and
            // then takes the trailing one back off) and what keeps the drawn pen within one tracking
            // unit of the width the measurement charged.
            if (tracking != Length.Zero && --remaining > 0) advance += tracking;

            // A blank on a justified line is wider than the font says. Tested on the character the
            // cluster names rather than on the glyph id, because a glyph id means nothing without the
            // face and the cluster is what the shaper guarantees.
            if (spaceAdd != Length.Zero
                && glyph.Cluster >= 0
                && glyph.Cluster < text.Length
                && text[glyph.Cluster] == ' ')
            {
                advance += spaceAdd;
            }

            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(
                    pen + shaped.Scale(glyph.OffsetX, emSize),
                    -shaped.Scale(glyph.OffsetY, emSize)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = font,
            FontSize = emSize,
            Origin = origin,
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }

    /// <summary>How far a run's pen travels: the sum of its advances, justification included.</summary>
    private static Length Extent(GlyphRun run)
    {
        Length total = Length.Zero;
        foreach (PositionedGlyph glyph in run.Glyphs) total += glyph.Advance;
        return total;
    }

    /// <summary>
    /// A reference for a paragraph whose font was not resolved through a resolver.
    /// </summary>
    /// <remarks>
    /// Hand-built input — a test, or a caller driving the paginator directly — has a face but no
    /// reference. Naming the face's own family is enough for a backend to group runs by font, and it
    /// records no substitution because none was made.
    /// </remarks>
    private static FontReference Reference(Text.Fonts.OpenTypeFace face) => new()
    {
        FamilyName = face.FamilyName ?? string.Empty,
        Weight = face.Weight,
        IsItalic = face.IsItalic,
        FaceKey = face.FamilyName ?? string.Empty,
    };
}
