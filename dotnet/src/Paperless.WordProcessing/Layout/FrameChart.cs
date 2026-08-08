using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Paints a chart held by a floating frame: a laid-out <see cref="ChartPlot"/> straight into the sink.
/// </summary>
/// <remarks>
/// <para>
/// The Writer counterpart to <c>Paperless.Presentations.Layout.SlideChart</c> and
/// <c>Paperless.Spreadsheets.Layout.SheetChart</c>, and the three share everything above the last step:
/// <see cref="ChartLayout.Place"/> composes the same <see cref="ChartDrawing"/> for all of them. A slide
/// turns it into <c>PlacedShape</c> values for a backend to walk; a sheet and a page have no such list —
/// <see cref="PageDrawing"/> paints directly — so this emits the fills, strokes and glyph runs itself.
/// Everything that decides where a mark goes is in <c>Paperless.Core.Charts</c> and is written once.
/// </para>
/// <para>
/// <strong>Paint order is the reference's.</strong> The chart's own background, the plot area's wall,
/// then the axes and their ticks, then the marks over them, then the text. Painting the marks before the
/// axes hides the axis line behind the first bar, which is a one-shape difference and exactly what a
/// fill-by-fill comparison against LibreOffice's PDF catches.
/// </para>
/// <para>
/// <strong>The chart is composed in the frame, not at its own size and stretched.</strong> That is the
/// one place this differs from a sheet, and it is what the two files state rather than a choice: an ODT's
/// <c>draw:object</c> declares no size of its own beside the <c>draw:frame</c>'s, and a DOCX's
/// <c>c:chart</c> relationship carries no extent — the <c>wp:extent</c> is the whole of it. A sheet's
/// <c>draw:object</c> does declare one (<c>svg:width="12cm"</c>), which is why <c>SheetChart</c> has a
/// stretch to fold in and this has none.
/// </para>
/// </remarks>
internal static class FrameChart
{
    /// <summary>Paints one chart into the rectangle its frame was placed at.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="plot">The chart.</param>
    /// <param name="box">Where the frame landed on the page.</param>
    /// <param name="family">The family the labels are set in, or null for Liberation Sans.</param>
    public static void Draw(IDrawingSink sink, ChartPlot plot, DocRect box, string? family)
    {
        ArgumentNullException.ThrowIfNull(sink);
        ArgumentNullException.ThrowIfNull(plot);

        if (box.Width <= Length.Zero || box.Height <= Length.Zero) return;

        ChartFace face = ChartFace.For(family);
        ChartDrawing drawing = ChartLayout.Place(plot, box, face);
        if (drawing.PlotArea.Width <= Length.Zero || drawing.PlotArea.Height <= Length.Zero) return;

        foreach (ChartBox filled in drawing.Boxes)
        {
            if (filled.Bounds.Width <= Length.Zero || filled.Bounds.Height <= Length.Zero) continue;

            GraphicsPath path = GraphicsPath.Rectangle(filled.Bounds);
            if (filled.Fill is { } fill) sink.FillPath(path, Paint.Solid(fill));
            if (filled.Line is { } line) sink.StrokePath(path, Pen(line, filled.LineWidth));
        }

        foreach (ChartLine line in drawing.Lines)
        {
            sink.StrokePath(
                new GraphicsPath().MoveTo(line.From).LineTo(line.To), Pen(line.Colour, line.Width));
        }

        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Path.Commands.Count == 0) continue;

            if (shape.Fill is { } fill) sink.FillPath(shape.Path, Paint.Solid(fill));
            if (shape.Line is { } line) sink.StrokePath(shape.Path, Pen(line, shape.LineWidth));
        }

        foreach (ChartLabel label in drawing.Labels) Text(sink, label, face);
    }

    /// <summary>
    /// The pen a chart's line is drawn with.
    /// </summary>
    /// <remarks>
    /// A zero width is passed through rather than replaced, because it is what the file states and what
    /// LibreOffice's export writes: <c>0 w</c> in the PDF, which every reader draws as the thinnest line
    /// the device has. Substituting a visible width makes every gridline and every bar outline heavier
    /// than the reference's.
    /// </remarks>
    private static Stroke Pen(Colour colour, Length width)
        => new(Paint.Solid(colour), width, LineCap.Butt, LineJoin.Miter);

    /// <summary>
    /// Draws one label, shaped and placed by its anchor.
    /// </summary>
    /// <remarks>
    /// <strong>A rotated label is drawn about its own centre.</strong> A glyph run carries an origin and
    /// a list of advances rather than a matrix, so it cannot be turned after the fact; it is laid out at
    /// the origin instead and the turn goes onto the sink's state stack. Composing the rotation about the
    /// page's origin rather than the label's centre puts a value-axis title off the left of the sheet,
    /// which reads as the title having vanished rather than as a placement bug.
    /// </remarks>
    private static void Text(IDrawingSink sink, ChartLabel label, ChartFace face)
    {
        if (label.Text.Length == 0) return;
        if (face.Shape(label.Text, label.Size) is not { } run) return;

        Length line = face.LineHeightAt(label.Size);
        Length ascent = face.AscentAt(label.Size);
        Length width = run.Width;

        if (label.Rotation != 0.0)
        {
            sink.Save();

            sink.Transform(AffineTransform.Concat(
                // Negated: `ChartLabel.Rotation` is anticlockwise, which is how both formats
                // state one and how chart2's own shapes carry it, and the drawing space here has
                // y growing downwards — so a positive angle handed straight to `Rotation` turns
                // the text the other way. Measured: a two-word value axis title comes out reading
                // top-to-bottom against the reference's bottom-to-top, and 45 degree category
                // labels descend to the right against the reference's ascending. The box does not
                // move, being symmetric about the same centre for either sign.
                AffineTransform.Rotation(-label.Rotation),
                AffineTransform.Translation(label.At.X.Emu, label.At.Y.Emu)));

            sink.DrawGlyphRun(
                run.At(new DocPoint(-(width / 2), -(line / 2) + ascent)), Paint.Solid(label.Colour));

            sink.Restore();
            return;
        }

        Length x = label.Anchor switch
        {
            ChartLabelAnchor.RightMiddle => label.At.X - width,
            ChartLabelAnchor.LeftMiddle => label.At.X,
            _ => label.At.X - (width / 2),
        };

        // CentreTop puts the label's top at the point and CentreBottom its bottom; the other three
        // centre it on the point.
        Length top = label.Anchor switch
        {
            ChartLabelAnchor.CentreTop => label.At.Y,
            ChartLabelAnchor.CentreBottom => label.At.Y - line,
            _ => label.At.Y - (line / 2),
        };

        sink.DrawGlyphRun(run.At(new DocPoint(x, top + ascent)), Paint.Solid(label.Colour));
    }
}

/// <summary>
/// One face a chart's labels are measured and shaped in, resolved once per family.
/// </summary>
/// <remarks>
/// <para>
/// A chart's labels are short, few, and all in one face: <c>chart2</c>'s view gives every one of them
/// the chart document's own default, so a page holding three charts asks for at most three faces. They
/// are cached statically because the face is a parsed table directory and reading one off disk is the
/// expensive half.
/// </para>
/// <para>
/// <strong>The line height includes the gap, and a paragraph's does not.</strong> The labels are plain
/// text shapes made by <c>chart2</c> rather than anything Writer laid out, so their height is the face's
/// own ascent plus descent plus leading — 1.1499 em for Liberation Sans — where a body line drops the
/// gap. The difference compounds through the insets that place the plot area rather than showing up in
/// any one label, which is why it is taken from <see cref="LineSpacing"/> here rather than from the
/// paragraph leading beside it.
/// </para>
/// </remarks>
internal sealed class ChartFace : IChartTextMeasurer
{
    /// <summary>
    /// The family a chart falls back to when the reader could not name one.
    /// </summary>
    /// <remarks>
    /// LibreOffice's own default sans, and what its PDF of every ODF chart in the corpus embeds. An
    /// OOXML chart names the theme's minor latin face instead and the reader passes it in, so this is
    /// only reached by a document that states neither.
    /// </remarks>
    private const string DefaultFamily = "Liberation Sans";

    private static readonly Dictionary<string, ChartFace> Cache = new(StringComparer.OrdinalIgnoreCase);
    private static readonly Lock Gate = new();

    private readonly OpenTypeFace? _face;
    private readonly FontReference _reference;
    private readonly LineMetrics? _metrics;

    private ChartFace(OpenTypeFace? face, FontReference? reference, string family)
    {
        _face = face;
        _reference = reference ?? new FontReference { FamilyName = family, FaceKey = string.Empty };
        _metrics = face is null ? null : LineSpacing.Resolve(face);
    }

    /// <summary>The face a family resolves to, resolved once and shared.</summary>
    /// <param name="family">The family, or null for <see cref="DefaultFamily"/>.</param>
    public static ChartFace For(string? family)
    {
        string wanted = string.IsNullOrWhiteSpace(family) ? DefaultFamily : family.Trim();

        lock (Gate)
        {
            if (Cache.TryGetValue(wanted, out ChartFace? cached)) return cached;

            ChartFace resolved = Load(wanted);
            Cache[wanted] = resolved;
            return resolved;
        }
    }

    /// <summary>The distance from a line's top to its baseline, at a size.</summary>
    public Length AscentAt(Length size)
        => _metrics is { } metrics ? metrics.ScaledAscent(size) : size * 0.9;

    /// <summary>How tall one line of a chart's text is, at a size.</summary>
    public Length LineHeightAt(Length size)
        => _metrics is { } metrics ? metrics.ScaledLineHeight(size) : size * 1.15;

    /// <summary>
    /// <paramref name="family"/> is ignored because this instance is already bound to one, and
    /// <paramref name="bold"/> because this instance holds no bold face.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The words track resolves the chart's family in its own reader and hands it to
    /// <see cref="ChartFace.For"/>, so a <see cref="ChartFace"/> <em>is</em> a family and the
    /// argument would only be a second, later opportunity to disagree with it. When that reader's
    /// rule and <c>DrawingChartPlot.FamilyOf</c>'s are shown to be the same rule — they are the
    /// same rule today, in two places — this can take the argument and the duplicate can go.
    /// </para>
    /// <para>
    /// <strong><paramref name="bold"/> is the slides track's <see cref="ChartPlot.IsTitleBold"/>
    /// reaching a consumer that cannot yet act on it.</strong> A <see cref="ChartFace"/> resolves
    /// one face and shapes every label through it, so drawing a title bold means resolving a
    /// second face here and threading it through <see cref="Shape"/> — a change that moves every
    /// DOCX whose chart has a title, on a words sweep this round did not run. Taking the argument
    /// and dropping it keeps the words track byte-identical while the model gets the value right.
    /// </para>
    /// </remarks>
    public DocSize Measure(string text, Length size, string? family, bool bold)
    {
        ArgumentNullException.ThrowIfNull(text);

        Length height = LineHeightAt(size);
        return text.Length == 0
            ? new DocSize(Length.Zero, height)
            : new DocSize(Shape(text, size)?.Width ?? Length.Zero, height);
    }

    /// <summary>Shapes one line, or null when there is no face to shape it with.</summary>
    public ChartRun? Shape(string text, Length size)
    {
        if (text.Length == 0 || _face is not { } face) return null;

        ShapedText shaped = TextShaper.Default.Shape(face, text);

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size);
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, size), -shaped.Scale(glyph.OffsetY, size)),
                advance));
            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new ChartRun(glyphs, clusters, _reference, size, text, pen);
    }

    /// <summary>
    /// Resolves one family, keeping the reference the resolver answered with.
    /// </summary>
    /// <remarks>
    /// Both the face and the reference, because the second cannot be rebuilt from the first: an
    /// <see cref="OpenTypeFace"/> is a parsed table directory and does not know which file it came out
    /// of. The resolver's own <c>FaceKey</c> is that file's path, and it is what lets the PDF writer
    /// embed the face — naming the family instead gives the provider a key it cannot open, so the run
    /// references a face the file does not carry and a reader substitutes or draws tofu, with neither
    /// the page count nor the extracted words changing.
    /// </remarks>
    private static ChartFace Load(string family)
    {
        try
        {
            SystemFontResolver resolver = SystemFontResolver.Build();
            FontReference reference = resolver.Resolve(new FontRequest(family));
            return new ChartFace(resolver.LoadOpenType(reference), reference, family);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // No readable face is not a reason to fail a layout: the plot area, the bars and everything
            // drawn as a path are already decided, and only the lettering is missing.
            return new ChartFace(null, null, family);
        }
    }
}

/// <summary>A shaped line of a chart's text, given an origin once it is placed.</summary>
/// <remarks>
/// Shaped without an origin and positioned later, because where it starts depends on its own width: a
/// value-axis label ends at the axis, so its start is only known once it has been measured.
/// </remarks>
internal sealed class ChartRun(
    List<PositionedGlyph> glyphs,
    List<int> clusters,
    FontReference font,
    Length size,
    string text,
    Length width)
{
    /// <summary>How far the pen travels across the whole line.</summary>
    public Length Width { get; } = width;

    /// <summary>The same glyphs, drawn from a point.</summary>
    public GlyphRun At(DocPoint origin) => new()
    {
        Font = font,
        FontSize = size,
        Origin = origin,
        Glyphs = glyphs,
        Text = text,
        ClusterMap = clusters,
    };
}
