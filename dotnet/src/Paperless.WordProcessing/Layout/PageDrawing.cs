using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
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
    /// An image frame draws nothing but its background and border. The raster is a separate matter and the
    /// wrap, which is what moves text, never depended on it — so a picture reserves exactly the right room
    /// and leaves a hole where its pixels will go.
    /// </para>
    /// </remarks>
    private static void DrawFrame(PlacedFrame frame, IDrawingSink sink)
    {
        if (frame.Frame.Fill is { } fill) Fill(frame.Area, fill, sink);

        DrawFlow(frame.Content, sink);

        if (frame.Frame.BorderColour is not { } colour) return;
        if (frame.Frame.BorderWidth <= Length.Zero) return;

        Stroke stroke = new(Paint.Solid(colour), frame.Frame.BorderWidth);
        DocRect area = frame.Area;

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
        foreach (PlacedLine line in lines)
        {
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;
            if (blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;

            foreach ((GlyphRun run, Colour colour) in RunsIn(area, line, paragraph))
            {
                sink.DrawGlyphRun(run, Paint.Solid(colour));
            }
        }
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
    public static List<(GlyphRun Run, Colour Colour)> RunsIn(
        DocRect area, PlacedLine line, PageParagraph paragraph)
    {
        ArgumentNullException.ThrowIfNull(paragraph);

        List<(GlyphRun, Colour)> runs = [];

        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.VisibleEnd, paragraph.Text.Length);
        if (end <= start) return runs;

        Length lineLeft = area.X + line.Box.Left;
        Length baseline = area.Y + line.Baseline;

        foreach (TabbedSegment segment in Stretches(paragraph, start, end))
        {
            if (segment.IsEmpty) continue;

            Length pen = lineLeft + segment.Left;

            foreach (PageRun run in RunsIn(paragraph, segment.Start, segment.End))
            {
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
                    line.Box.SpaceAdd);

                runs.Add((glyphRun, run.EffectiveColour));

                // The pen carries the justification with it, or the second run on a stretched line would
                // start where the first would have ended unjustified and overlap the words before it.
                pen += Extent(glyphRun);
            }
        }

        return runs;
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
    private static List<TabbedSegment> Stretches(PageParagraph paragraph, int start, int end)
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
            (from, to) => WidthBetween(paragraph, from, to));
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
            return
            [
                new PageRun(
                    start,
                    end - start,
                    paragraph.Face,
                    paragraph.EmSize,
                    paragraph.Font,
                    paragraph.Colour,
                    paragraph.Shaping),
            ];
        }

        List<PageRun> clipped = [];
        foreach (PageRun run in paragraph.Runs.OrderBy(run => run.Start))
        {
            int from = Math.Max(run.Start, start);
            int to = Math.Min(run.End, end);
            if (to <= from) continue;

            clipped.Add(run with { Start = from, Length = to - from });
        }

        return clipped;
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

        foreach (PageRun run in RunsIn(paragraph, from, to))
        {
            string text = paragraph.Text[run.Start..run.End];
            total += TextShaper.Default.Shape(run.Face, text, run.Shaping).Width(run.EmSize);
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
        Length spaceAdd)
    {
        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);

        Length pen = Length.Zero;
        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, emSize);

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
