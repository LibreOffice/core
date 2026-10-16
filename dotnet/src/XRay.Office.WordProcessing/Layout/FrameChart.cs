using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Shaping;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// Paints a chart held by a floating frame: a laid-out <see cref="ChartPlot"/> straight into the sink.
/// </summary>
/// <remarks>
/// <para>
/// The Writer counterpart to <c>XRay.Office.Presentations.Layout.SlideChart</c> and
/// <c>XRay.Office.Spreadsheets.Layout.SheetChart</c>, and the three share everything above the last step:
/// <see cref="ChartLayout.Place"/> composes the same <see cref="ChartDrawing"/> for all of them. A slide
/// turns it into <c>PlacedShape</c> values for a backend to walk; a sheet and a page have no such list —
/// <see cref="PageDrawing"/> paints directly — so this emits the fills, strokes and glyph runs itself.
/// Everything that decides where a mark goes is in <c>XRay.Office.Core.Charts</c> and is written once.
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
                new GraphicsPath().MoveTo(line.From).LineTo(line.To),
                Pen(line.Colour, line.Width, line.DashPattern, line.Cap));
        }

        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Path.Commands.Count == 0) continue;

            if (shape.Fill is { } fill) sink.FillPath(shape.Path, Paint.Solid(fill));
            if (shape.Line is { } line)
                sink.StrokePath(shape.Path, Pen(line, shape.LineWidth, shape.DashPattern, shape.Cap));
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
    private static Stroke Pen(
        Colour colour, Length width, IReadOnlyList<Length>? dash = null,
        LineCap cap = LineCap.Butt)
        => new(Paint.Solid(colour), width, cap, LineJoin.Miter, DashPattern: dash);

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

        // A label that shows a percentage without a value is written on two lines — Office's own
        // separator, `seriesconverter.cxx:168-172`, which `ChartDataLabel.Separator` already
        // defaults to "\n". Shaping the whole string as one run drew the newline as a
        // zero-width nothing and ran the two halves together, so `Leaf 11` and `15%` came out as
        // the single token `Leaf 1115%` — 8 of the 16 labels of
        // `027_Unit_Circle_Chart_Graphical_Chart`, and the whole of its remaining word gap once
        // its categories were being read at every level.
        if (label.Text.AsSpan().IndexOfAny('\n', '\r') >= 0)
        {
            Lines(sink, label, face);
            return;
        }

        bool bold = label.IsBold == true;

        if (face.Shape(label.Text, label.Size, bold) is not { } run) return;

        Length line = face.LineHeightAt(label.Size, bold);
        Length ascent = face.AscentAt(label.Size, bold);
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

    /// <summary>
    /// A label holding line breaks, drawn as a stack of lines about the same anchor point.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The block is as tall as its lines and each line keeps the whole label's horizontal
    /// alignment, which is what <c>chart2</c>'s text shape does with a multi-paragraph label:
    /// a centred label centres every line on the anchor, and a left- or right-anchored one
    /// stacks them flush to that edge. The block's height replaces one line's in the vertical
    /// anchoring, so a <c>CentreBottom</c> two-line label still ends at the point it is given.
    /// </para>
    /// <para>
    /// Rotation is deliberately not handled here and falls back to the single-run path above:
    /// no rotated label in this corpus carries a break, and stacking under a rotation needs the
    /// lines offset along the rotated normal rather than down the page.
    /// </para>
    /// </remarks>
    private static void Lines(IDrawingSink sink, ChartLabel label, ChartFace face)
    {
        string[] parts = label.Text.Split(['\n', '\r'], StringSplitOptions.RemoveEmptyEntries);
        List<ChartRun> runs = [];
        foreach (string part in parts)
        {
            if (face.Shape(part, label.Size, label.IsBold == true) is { } shaped) runs.Add(shaped);
        }

        if (runs.Count == 0) return;

        Length line = face.LineHeightAt(label.Size, label.IsBold == true);
        Length ascent = face.AscentAt(label.Size, label.IsBold == true);
        Length block = line * runs.Count;

        Length top = label.Anchor switch
        {
            ChartLabelAnchor.CentreTop => label.At.Y,
            ChartLabelAnchor.CentreBottom => label.At.Y - block,
            _ => label.At.Y - (block / 2),
        };

        for (int at = 0; at < runs.Count; at++)
        {
            Length width = runs[at].Width;
            Length x = label.Anchor switch
            {
                ChartLabelAnchor.RightMiddle => label.At.X - width,
                ChartLabelAnchor.LeftMiddle => label.At.X,
                _ => label.At.X - (width / 2),
            };

            sink.DrawGlyphRun(
                runs[at].At(new DocPoint(x, top + (line * at) + ascent)), Paint.Solid(label.Colour));
        }
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
/// <strong>The height and the ascent come through <c>chart2</c>'s own 96 dpi device, and not
/// through Writer's.</strong> The labels are plain text shapes made by <c>chart2</c> rather than
/// anything Writer laid out, so they are neither measured against Writer's 8640 dpi reference
/// device nor scaled exactly from the face — they go through the <c>VirtualDevice</c> that
/// <c>DrawModelWrapper</c> creates from <c>Application::GetDefaultDevice()</c>
/// (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), which asks for no
/// <c>RefDevMode</c> and is therefore 96 dpi (<c>SvpSalGraphics::GetResolution</c>,
/// <c>vcl/headless/svpgdi.cxx</c>:44). See <see cref="MetricGrid.Chart"/>.
/// </para>
/// <para>
/// A 96 dpi pixel is <strong>0.75 pt</strong>, so the em itself is rounded to a whole number of
/// them before a single metric is read and the line height is a <em>sawtooth</em> in the size
/// rather than a fixed fraction of it: Liberation Sans stacks at 11.254 pt at 10 pt where its
/// design metrics give 11.499, and at 12.756 at 11 pt where they give 12.649. The external
/// leading goes with the grid — <c>IsAddExtLeading()</c> is false in EditEngine and a chart's
/// label is an EditEngine text — so this is no longer ascent plus descent plus leading.
/// </para>
/// <para>
/// <strong>This is the vertical half of the rule <see cref="Shape"/> applies horizontally</strong>,
/// and the two are taken together deliberately: a chart label is drawn at
/// <c>blockCentre − blockHeight/2 + ascent</c>, so a height that moves without its ascent moves
/// every label. <c>SheetBandText.ChartLineHeightAt</c> is the same rule for a workbook's charts,
/// landed in round 60; this is the words track catching up. Measured on both reference binaries in
/// <c>probes/chart-vertical/</c> — three faces × twelve sizes × two binaries × a deck and a Writer
/// document, <strong>144 of 144</strong> baseline-to-baseline distances within 0.019 pt of it,
/// where scaling the face's metrics exactly is out by up to 1.208 pt.
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

    /// <summary>The bold twin of <see cref="_face"/>, or null where the family has none.</summary>
    /// <remarks>
    /// Resolved beside the regular face rather than on demand, because a chart asks for both on
    /// the same page and the resolver's own cache is not shared with this one. A family whose
    /// bold is the regular face — which is what the resolver answers for a roman-only family —
    /// leaves this null and every label is drawn in the one face, exactly as before.
    /// </remarks>
    private readonly ChartFace? _bold;

    private ChartFace(OpenTypeFace? face, FontReference? reference, string family,
                      ChartFace? bold = null)
    {
        _face = face;
        _reference = reference ?? new FontReference { FamilyName = family, FaceKey = string.Empty };
        _metrics = face is null ? null : LineSpacing.Resolve(face, MetricGrid.Chart);
        _bold = bold;
    }

    /// <summary>This face, or its bold twin when there is one and the caller wants it.</summary>
    private ChartFace Weighted(bool bold) => bold && _bold is not null ? _bold : this;

    /// <summary>The face a family resolves to, resolved once and shared.</summary>
    /// <param name="family">The family, or null for <see cref="DefaultFamily"/>.</param>
    public static ChartFace For(string? family)
    {
        string wanted = string.IsNullOrWhiteSpace(family) ? DefaultFamily : family.Trim();

        lock (Gate)
        {
            if (Cache.TryGetValue(wanted, out ChartFace? cached)) return cached;

            ChartFace regular = Load(wanted, weight: RegularWeight);
            ChartFace bold = Load(wanted, weight: BoldWeight);

            // A family with no bold of its own resolves to the same file for both weights, and
            // pairing a face with itself would embed a second identical subset and change every
            // such document's bytes for nothing.
            ChartFace resolved = regular.Same(bold)
                ? regular
                : Load(wanted, weight: RegularWeight, bold: bold);

            Cache[wanted] = resolved;
            return resolved;
        }
    }

    /// <summary>The distance from a line's top to its baseline, at a size.</summary>
    /// <remarks>
    /// On <see cref="MetricGrid.Chart"/>, like <see cref="LineHeightAt"/> — the two have to move
    /// together or the labels that agree today stop agreeing, because a label is drawn at
    /// <c>blockCentre − blockHeight/2 + ascent</c> and the two errors used to cancel on a
    /// single-line label. See the remark on this class.
    /// </remarks>
    /// <param name="size">The em size.</param>
    /// <param name="bold">Whether the label is drawn in the bold face.</param>
    public Length AscentAt(Length size, bool bold = false)
    {
        ChartFace face = Weighted(bold);
        return face._metrics is { } metrics ? metrics.ScaledAscent(size) : size * 0.9;
    }

    /// <summary>How tall one line of a chart's text is, at a size.</summary>
    /// <remarks>
    /// Ascent plus descent on <see cref="MetricGrid.Chart"/>, taken as the taller of the two
    /// roundings EditEngine compares and with no external leading. See the remark on this class
    /// for the measurement.
    /// </remarks>
    /// <param name="size">The em size.</param>
    /// <param name="bold">Whether the label is drawn in the bold face.</param>
    public Length LineHeightAt(Length size, bool bold = false)
    {
        ChartFace face = Weighted(bold);
        return face._metrics is { } metrics ? metrics.ScaledLineHeight(size) : size * 1.15;
    }

    /// <summary>
    /// Measures one line; <paramref name="family"/> is ignored because this instance is already
    /// bound to one.
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
    /// <strong><paramref name="bold"/> is honoured, and for four rounds it was not.</strong> This
    /// class resolved one face and shaped every label through it, so a chart's bold title, bold
    /// axis labels and bold data labels were all drawn light on the words track while the model
    /// carried the right value — the slides and sheets painters having acted on it all along.
    /// <c>021_Unit_Circle_Chart_3D_Pie_Chart</c> is the witness: its series-level
    /// <c>c:dLbls/c:txPr</c> states <c>sz="1400" b="1"</c>, 26.2.4.2 draws Carlito-Bold at 14.01
    /// and this tree drew Carlito-Regular at 14.00.
    /// </para>
    /// </remarks>
    public DocSize Measure(string text, Length size, string? family, bool bold)
    {
        ArgumentNullException.ThrowIfNull(text);

        Length height = LineHeightAt(size, bold);
        return text.Length == 0
            ? new DocSize(Length.Zero, height)
            : new DocSize(Shape(text, size, bold)?.Width ?? Length.Zero, height);
    }

    /// <summary>Shapes one line, or null when there is no face to shape it with.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The advances go through <see cref="MetricGrid.Chart"/>, exactly as
    /// <c>SheetBandText.ChartShape</c>'s do.</strong> A chart's text is not laid out by Writer: it
    /// is built by <c>chart2</c>'s own view on the <c>VirtualDevice</c> that
    /// <c>DrawModelWrapper</c> creates from <c>Application::GetDefaultDevice()</c>
    /// (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), and that device is 96 dpi
    /// (<c>SvpSalGraphics::GetResolution</c>, <c>vcl/headless/svpgdi.cxx</c>:44). A font is
    /// instantiated at a whole number of device pixels, so at 10 pt the device sets 13 for 13.333
    /// and every advance comes back 2.5% narrow, while at 11 pt it sets 15 for 14.667 and they
    /// come back 2.3% wide.
    /// </para>
    /// <para>
    /// This is the words counterpart of the change round 62 made for a workbook's charts, and it
    /// was left behind then: the same rule, in the third of the three places that shape a chart's
    /// text. See <c>probes/chart-text-metafile/results.md</c>, where it is measured on both
    /// reference binaries at twelve sizes with a slide text box beside the chart as the control.
    /// </para>
    /// </remarks>
    /// <param name="text">The line.</param>
    /// <param name="size">The em size.</param>
    /// <param name="bold">Whether to shape through the bold twin.</param>
    public ChartRun? Shape(string text, Length size, bool bold = false)
    {
        ChartFace weighted = Weighted(bold);
        if (text.Length == 0 || weighted._face is not { } face) return null;

        ShapedText shaped = TextShaper.Default.Shape(face, text);
        double scale = MetricGrid.Chart.PixelEmScale(size);

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size) * scale;
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, size), -shaped.Scale(glyph.OffsetY, size)),
                advance));
            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new ChartRun(glyphs, clusters, weighted._reference, size, text, pen);
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
    private static ChartFace Load(string family, int weight, ChartFace? bold = null)
    {
        try
        {
            SystemFontResolver resolver = SystemFontResolver.Build();
            FontReference reference = resolver.Resolve(new FontRequest(family, weight));
            return new ChartFace(resolver.LoadOpenType(reference), reference, family, bold);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // No readable face is not a reason to fail a layout: the plot area, the bars and everything
            // drawn as a path are already decided, and only the lettering is missing.
            return new ChartFace(null, null, family, bold);
        }
    }

    /// <summary>Whether this face came out of the same file as another.</summary>
    /// <remarks>
    /// The resolver's <c>FaceKey</c> is the file's path, which is what the PDF writer embeds by,
    /// so two references with one key are one face however they were asked for. A roman-only
    /// family answers the same key for both weights, and pairing it with itself would embed a
    /// second identical subset and change every such document's bytes for nothing.
    /// </remarks>
    private bool Same(ChartFace other)
        => string.Equals(_reference.FaceKey, other._reference.FaceKey, StringComparison.Ordinal);

    /// <summary>What a request asks for when it wants the ordinary weight.</summary>
    private const int RegularWeight = 400;

    /// <summary>And the bold one — <c>OS/2</c>'s own usWeightClass for bold.</summary>
    private const int BoldWeight = 700;
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
