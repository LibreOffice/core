using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Lays a shape's text body out inside its text rectangle.
/// </summary>
/// <remarks>
/// <para>
/// Line breaking, indents and horizontal alignment all come from <c>Paperless.Text</c>'s
/// <see cref="ParagraphLayouter"/>, which is the same engine the word processor uses and had
/// better stay so. What this adds is the two things a slide does differently: where the baselines
/// sit, and where the block as a whole sits inside the shape.
/// </para>
/// <para>
/// <strong>A slide's line height is a fraction of the font size, not of the font's metrics.</strong>
/// The PPTX importer sets <c>FontIndependentLineSpacing</c> on every text body it reads
/// (<c>oox/source/ppt/pptshapecontext.cxx:186</c>), and EditEngine then computes the line's
/// ascent as the font height outright and its descent as
/// <c>ImplCalculateFontIndependentLineSpacing(height) − ascent</c>
/// (<c>editeng/source/editeng/impedit3.cxx:3138-3141</c>), where that function is
/// <c>fround(height × 12 / 10)</c> (<c>impedit3.cxx:501-505</c>). So the baseline is one em below
/// the top of the line and the next line is 1.2 em further down — whatever face the text is set
/// in.
/// </para>
/// <para>
/// That is worth stating precisely because it is not a small difference. Liberation Sans reports
/// an ascent of 0.905 em, so a reader using the font's metrics puts an 18 pt first baseline
/// 16.30 pt below the text top where LibreOffice puts it at 18.00 — a point and a half, on every
/// line of every shape. Measured on <c>shape-geometry.pptx</c> slide 3: LibreOffice's PDF draws
/// the first text box's only line at 89.972 pt down a page whose shape starts at 71.972, and its
/// middle-anchored box's baseline at 259.172, which is the block-height arithmetic above to
/// within 0.014 pt.
/// </para>
/// </remarks>
public static class SlideTextLayout
{
    /// <summary>
    /// The numerator and denominator of EditEngine's font-independent line height.
    /// </summary>
    /// <remarks>
    /// Kept as the fraction rather than as 1.2 so the rounding matches: LibreOffice rounds the
    /// product to a whole unit of its own layout resolution, and a double multiply followed by a
    /// separate round is the same arithmetic.
    /// </remarks>
    private const double LineHeightFactor = 12.0 / 10.0;

    /// <summary>
    /// Lays a body out and returns its glyph runs, positioned in the given rectangle's space.
    /// </summary>
    /// <param name="body">The text body.</param>
    /// <param name="textRectangle">
    /// The shape's text rectangle, in whatever space the caller wants the runs in — the shape's
    /// own for a rotated shape, the slide's for an upright one. The insets are applied here.
    /// </param>
    /// <param name="fonts">The face cache.</param>
    public static List<PlacedGlyphRun> Place(
        SlideTextBody body, DocRect textRectangle, SlideFonts fonts)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(fonts);

        DocRect area = textRectangle.Deflate(body.Insets);
        List<PlacedGlyphRun> placed = [];
        if (body.Paragraphs.Count == 0) return placed;

        // wrap="none" is expressed as an effectively unbounded width rather than as clipping,
        // which is what keeps an unwrapped label on the single line its author saw.
        Length width = body.Wraps && area.Width > Length.Zero
            ? area.Width
            : Length.FromEmu(int.MaxValue);

        List<Block> blocks = [];
        Length total = Length.Zero;

        foreach (SlideParagraph paragraph in body.Paragraphs)
        {
            Block? block = Measure(paragraph, body, width, fonts);
            if (block is null) continue;

            total += block.Height;
            blocks.Add(block);
        }

        if (blocks.Count == 0) return placed;

        Length top = area.Y + body.Anchor switch
        {
            TextAnchor.Middle => (area.Height - total) / 2,
            TextAnchor.Bottom => area.Height - total,
            _ => Length.Zero,
        };

        foreach (Block block in blocks)
        {
            top += block.SpaceBefore;
            foreach (PlacedLine line in block.Lines)
            {
                Emit(placed, block, line, area.X, top);
                top += line.Height;
            }
            top += block.SpaceAfter;
        }

        return placed;
    }

    /// <summary>
    /// Breaks one paragraph into lines and gives each the height EditEngine would.
    /// </summary>
    /// <remarks>
    /// The break positions and the horizontal placement come from the shared layouter; only the
    /// vertical is recomputed. Doing it the other way — teaching the layouter this rule — would
    /// put a presentation-specific metric into the engine three families share.
    /// </remarks>
    private static Block? Measure(
        SlideParagraph paragraph, SlideTextBody body, Length width, SlideFonts fonts)
    {
        List<FormattedRun> runs = [];
        List<Colour> colours = [];
        OpenTypeFace? first = null;

        foreach (SlideTextRun run in paragraph.Runs)
        {
            (OpenTypeFace? face, FontReference? reference) =
                fonts.Resolve(run.Typeface, run.Weight, run.IsItalic);
            if (face is null) continue;

            first ??= face;
            Length size = Scaled(run.Size, body.FontScale);

            runs.Add(new FormattedRun(run.Start, run.Length, face, size));
            colours.Add(run.Colour);
        }

        if (first is null) return null;

        ParagraphFormat format = new()
        {
            Alignment = paragraph.Alignment,
            StartIndent = paragraph.StartIndent,
            FirstLineIndent = paragraph.FirstLineIndent,
            LineSpacing = paragraph.LineSpacing,
        };

        MeasuredParagraph measured = MeasuredParagraph.Measure(paragraph.Text, runs);
        ParagraphLayouter layouter = new(first);
        LaidOutParagraph laid = layouter.Layout(
            measured, format, width, paragraph.Language);

        List<PlacedLine> lines = [];
        foreach (LineBox box in laid.Lines)
        {
            if (!body.FontIndependentLineSpacing)
            {
                // The face's own metrics, which the shared layouter has already applied — so a
                // natively authored ODP lays its text out exactly as a word processor would.
                lines.Add(new PlacedLine(
                    box, box.Baseline, Reduced(box.Height, body.LineSpaceReduction)));
                continue;
            }

            Length em = LargestSize(runs, box.Line.Start, box.Line.VisibleEnd);

            // The rule itself: one em of ascent, 1.2 em of box, then whatever the paragraph's own
            // spacing does to it. A paragraph stating 150% gets 1.5 x 1.2 em, which is what
            // EditEngine's proportional spacing applies to the height it just computed.
            Length natural = Length.FromEmu((long)Math.Round(em.Emu * LineHeightFactor));
            Length height = Reduced(paragraph.LineSpacing.Apply(natural), body.LineSpaceReduction);

            lines.Add(new PlacedLine(box, em, height));
        }

        Length total = Length.Zero;
        foreach (PlacedLine line in lines) total += line.Height;

        return new Block(
            paragraph, measured, colours, lines, total + paragraph.SpaceBefore + paragraph.SpaceAfter);
    }

    /// <summary>The largest em size among the runs a line touches.</summary>
    /// <remarks>
    /// The line's own runs rather than the paragraph's, because a 32 pt word in an 18 pt paragraph
    /// makes <em>its</em> line taller and leaves the others alone — which is the same rule the
    /// shared layouter applies to font metrics, restated for a metric that is not the font's.
    /// </remarks>
    private static Length LargestSize(List<FormattedRun> runs, int start, int end)
    {
        Length largest = Length.Zero;

        foreach (FormattedRun run in runs)
        {
            bool touches = run.Start < end && start < run.End;
            bool contains = start == end && run.Covers(start);
            if (touches || contains) largest = Length.Max(largest, run.EmSize);
        }

        if (largest > Length.Zero) return largest;

        // An empty paragraph still occupies a line, and it is as tall as the text that would go
        // on it: the first run's size, which is what the paragraph mark carries.
        return runs.Count > 0 ? runs[0].EmSize : Length.FromPoints(18);
    }

    private static Length Scaled(Length size, double scale)
        => scale is > 0 and not 1.0
            ? Length.FromEmu((long)Math.Round(size.Emu * scale))
            : size;

    private static Length Reduced(Length height, double reduction)
        => reduction is > 0 and < 1
            ? Length.FromEmu((long)Math.Round(height.Emu * (1 - reduction)))
            : height;

    /// <summary>Emits one line's glyph runs, one per formatting change along it.</summary>
    private static void Emit(
        List<PlacedGlyphRun> placed, Block block, PlacedLine line, Length areaLeft, Length top)
    {
        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.VisibleEnd, block.Measured.Text.Length);
        if (end <= start) return;

        Length pen = areaLeft + line.Box.Left;
        Length baseline = top + line.Ascent;

        foreach (FormattedRun run in block.Measured.RunsBetween(start, end))
        {
            string text = block.Measured.Text[run.Start..run.End];
            ShapedText shaped = TextShaper.Default.Shape(run.Face, text, run.Shaping);
            if (shaped.Glyphs.Count == 0) continue;

            GlyphRun glyphs = Build(
                shaped, text, run.EmSize, Reference(run.Face), new DocPoint(pen, baseline),
                line.Box.SpaceAdd);

            placed.Add(new PlacedGlyphRun(glyphs, block.ColourAt(run.Start)));

            // The pen carries across the runs of a line, so the second run starts where the first
            // ended rather than back at the margin.
            foreach (PositionedGlyph glyph in glyphs.Glyphs) pen += glyph.Advance;
        }
    }

    /// <summary>Builds a glyph run from a shaped stretch of text at an origin.</summary>
    /// <remarks>
    /// Each glyph's offset is relative to the run's origin and the pen accumulates across them,
    /// which is what makes a run one draw call. The vertical offset is negated because a shaper's
    /// is up-positive and document space is down-positive.
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

    private static FontReference Reference(OpenTypeFace face) => new()
    {
        FamilyName = face.FamilyName ?? string.Empty,
        Weight = face.Weight,
        IsItalic = face.IsItalic,
        FaceKey = face.FamilyName ?? string.Empty,
    };

    /// <summary>One paragraph, measured and broken.</summary>
    private sealed record Block(
        SlideParagraph Paragraph,
        MeasuredParagraph Measured,
        IReadOnlyList<Colour> Colours,
        IReadOnlyList<PlacedLine> Lines,
        Length Height)
    {
        public Length SpaceBefore => Paragraph.SpaceBefore;

        public Length SpaceAfter => Paragraph.SpaceAfter;

        /// <summary>The colour covering a character, or black when no run does.</summary>
        /// <remarks>
        /// Looked up by position rather than carried on the measured run, because
        /// <see cref="MeasuredParagraph"/> keeps only what changes a measurement — a colour does
        /// not move a line break, so it travels with whatever draws the text.
        /// </remarks>
        public Colour ColourAt(int index)
        {
            for (int i = 0; i < Paragraph.Runs.Count && i < Colours.Count; i++)
            {
                if (index >= Paragraph.Runs[i].Start && index < Paragraph.Runs[i].End)
                    return Colours[i];
            }

            return Colours.Count > 0 ? Colours[0] : Colour.Black;
        }
    }

    /// <summary>One line, with the height EditEngine's rule gives it.</summary>
    /// <param name="Box">The shared layouter's box: which characters, and where across the area.</param>
    /// <param name="Ascent">
    /// How far the baseline sits below the line's top, which under the font-independent rule is
    /// the em size itself.
    /// </param>
    /// <param name="Height">The distance to the next line's top.</param>
    private readonly record struct PlacedLine(LineBox Box, Length Ascent, Length Height);
}
