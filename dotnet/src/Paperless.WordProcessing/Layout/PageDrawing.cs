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
    /// recorded display list reads far better when it matches the page.
    /// <para>
    /// Floating frames are still missing, being the one kind of page content pagination does not place.
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

    /// <summary>Draws a flow — a header, a footer or a cell — which is lines in their own rectangle.</summary>
    private static void DrawFlow(PlacedFlow? flow, IDrawingSink sink)
    {
        if (flow is null || flow.IsEmpty) return;

        DrawLines(flow.Area, flow.Lines, flow.Blocks, sink);
        foreach (PlacedTable table in flow.Tables) DrawTable(table, sink);
    }

    /// <summary>
    /// Draws a table, which is its cells' text.
    /// </summary>
    /// <remarks>
    /// Text only for now: borders and cell shading are read by the extraction pass but not yet carried into
    /// layout, so a table currently draws as its words in the right places and nothing round them. That is
    /// the half a text comparison can verify, and the half every other feature depends on.
    /// </remarks>
    private static void DrawTable(PlacedTable table, IDrawingSink sink)
    {
        foreach (PlacedTableCell cell in table.Cells) DrawFlow(cell.Content, sink);
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

                GlyphRun glyphRun = Build(
                    shaped,
                    text,
                    run.EmSize,
                    run.Font ?? Reference(run.Face),
                    new DocPoint(pen, baseline),
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
