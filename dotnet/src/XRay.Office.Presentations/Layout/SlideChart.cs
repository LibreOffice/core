using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;

namespace XRay.Office.Presentations.Layout;

/// <summary>
/// Draws a chart onto a slide: the shapes a laid-out <see cref="ChartPlot"/> becomes.
/// </summary>
/// <remarks>
/// <para>
/// The same shape as <see cref="SlideTable"/> and for the same reason: a chart is not one shape
/// with a chart inside it but a run of ordinary <see cref="PlacedShape"/> — a fill per bar, a
/// stroke per axis and tick, a glyph run per label. Nothing in the display list needs to know a
/// chart happened, which keeps <see cref="SlideDrawing"/> unchanged and lets a second front end
/// reuse every line of this.
/// </para>
/// <para>
/// <strong>Paint order is the reference's.</strong> Background, then the plot area's wall, then
/// the axes and their ticks, then the bars over them, then the text. Measured in LibreOffice's
/// own PDF for <c>chart-bar-deck.pptx</c>: the wall rectangle is painted, then five category
/// ticks, then ten value ticks, then the two axis lines, then eight bars each immediately
/// followed by its own outline, then every label. Painting the bars before the axes would hide
/// the axis line behind the first bar, which is a one-shape difference that shows up as a
/// missing stroke in a fill-by-fill comparison.
/// </para>
/// </remarks>
public static class SlideChart
{
    /// <summary>
    /// Lays a chart out inside a graphic frame and returns the shapes that draw it.
    /// </summary>
    /// <param name="plot">The chart.</param>
    /// <param name="size">The frame's extent, in its own coordinates.</param>
    /// <param name="placement">The matrix taking the frame's coordinates onto the slide.</param>
    /// <param name="fonts">The face cache, for measuring and shaping the labels.</param>
    /// <param name="name">The frame's name, carried onto every shape for diagnostics.</param>
    public static List<PlacedShape> Place(
        ChartPlot plot,
        DocSize size,
        AffineTransform placement,
        SlideFonts fonts,
        string? name = null)
    {
        ArgumentNullException.ThrowIfNull(plot);
        ArgumentNullException.ThrowIfNull(fonts);

        List<PlacedShape> shapes = [];
        if (size.Width <= Length.Zero || size.Height <= Length.Zero) return shapes;

        DocRect frame = new(Length.Zero, Length.Zero, size.Width, size.Height);
        ChartDrawing drawing = ChartLayout.Place(plot, frame, new Measurer(fonts));

        if (drawing.PlotArea.Width <= Length.Zero) return shapes;

        foreach (ChartBox box in drawing.Boxes)
        {
            if (box.Bounds.Width <= Length.Zero || box.Bounds.Height <= Length.Zero) continue;

            shapes.Add(new PlacedShape
            {
                Name = name,
                Outline = ShapeTransform.Apply(placement, GraphicsPath.Rectangle(box.Bounds)),
                Bounds = ShapeTransform.PlacedBounds(
                    AffineTransform.Concat(
                        AffineTransform.Translation(box.Bounds.X.Emu, box.Bounds.Y.Emu), placement),
                    box.Bounds.Size),
                Fill = box.Fill is { } fill ? Paint.Solid(fill) : null,
                Line = box.Line is { } line ? Pen(line, box.LineWidth) : null,
            });
        }

        foreach (ChartLine line in drawing.Lines)
        {
            GraphicsPath path = new GraphicsPath().MoveTo(line.From).LineTo(line.To);

            shapes.Add(new PlacedShape
            {
                Name = name,
                Outline = ShapeTransform.Apply(placement, path),
                Bounds = DocRect.Empty,
                Line = Pen(line.Colour, line.Width, line.DashPattern, line.Cap),
            });
        }

        // The free-form marks — a pie's wedges, a line's polyline, an area's region — after the
        // axes and before the text, which is where the reference draws them.
        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Path.Commands.Count == 0) continue;

            shapes.Add(new PlacedShape
            {
                Name = name,
                Outline = ShapeTransform.Apply(placement, shape.Path),
                Bounds = DocRect.Empty,
                Fill = shape.Fill is { } fill ? Paint.Solid(fill) : null,
                Line = shape.Line is { } line
                    ? Pen(line, shape.LineWidth, shape.DashPattern, shape.Cap)
                    : null,
            });
        }

        foreach (ChartLabel label in drawing.Labels)
        {
            if (Text(label, placement, fonts) is { } text)
                shapes.Add(new PlacedShape
                {
                    Name = name,
                    Outline = new GraphicsPath(),
                    Bounds = DocRect.Empty,
                    Text = text,
                });
        }

        return shapes;
    }

    /// <summary>
    /// The pen a chart line is drawn with.
    /// </summary>
    /// <remarks>
    /// A zero width is kept rather than replaced with a default, because it is what the file
    /// says and what the reference draws: LibreOffice's export writes <c>0 w</c> and every PDF
    /// reader renders that as the thinnest line the device has. Substituting a visible width
    /// here would make every bar outline and every gridline heavier than the reference's.
    /// </remarks>
    private static Stroke Pen(
        Colour colour, Length width, IReadOnlyList<Length>? dash = null,
        LineCap cap = LineCap.Butt)
        => new(Paint.Solid(colour), width, cap, LineJoin.Miter, DashPattern: dash);

    /// <summary>Which edge of its block a broken label's lines line up on.</summary>
    /// <remarks>
    /// The anchor's own horizontal half: a value axis' labels hang from their right edge, a
    /// legend entry from its left, and everything else is centred.
    /// </remarks>
    private static TextAlignment Adjust(ChartLabelAnchor anchor)
        => anchor switch
        {
            ChartLabelAnchor.RightMiddle => TextAlignment.End,
            ChartLabelAnchor.LeftMiddle => TextAlignment.Start,
            _ => TextAlignment.Centre,
        };

    /// <summary>
    /// Lays one chart label out and returns its glyph runs, placed on the slide.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Run through <see cref="SlideTextLayout"/> rather than shaped here, so that a chart's text
    /// is measured by the same engine as a shape's and cannot drift from it. The body is given
    /// zero insets — DrawingML's 0.1-inch default insets are a text box's, and a chart label has
    /// none — and a rectangle sized to what the text measures, positioned by the anchor.
    /// </para>
    /// <para>
    /// <strong>A rotated label is placed by its own centre.</strong> The value axis' title is the
    /// only rotated text a chart draws, and a quarter turn about the label's centre is what both
    /// formats mean by it. Composing the rotation about the centre rather than about the frame's
    /// origin is the difference between a title beside the axis and one off the left of the
    /// slide.
    /// </para>
    /// <para>
    /// <strong>A label the arrangement broke is as wide as its widest line, and until this said
    /// so it was as wide as all of them joined.</strong> A wrapped label came here as one
    /// paragraph holding a newline, and the anchor was worked out from a width that added the two
    /// lines together — so a right-aligned one landed a whole line to the left of the axis it
    /// hangs from. <c>SheetChart</c> and <c>FrameChart</c> have split on the newline and anchored
    /// line by line since they were written; this is the third of the three catching up. It is
    /// done here by giving the body one paragraph per line and letting the paragraph alignment
    /// carry the anchor, which is what <c>changeTextAdjustment</c> does for the reference's own
    /// label shapes (<c>chart2/source/view/main/LabelPositionHelper.cxx</c>).
    /// </para>
    /// </remarks>
    private static PlacedText? Text(ChartLabel label, AffineTransform placement, SlideFonts fonts)
    {
        if (label.Text.Length == 0) return null;

        Measurer measurer = new(fonts);
        string[] lines = label.Text.Split('\n');

        DocSize measured = measurer.Measure(label.Text, label.Size, label.Family, label.IsBold == true);
        if (measured.Width <= Length.Zero) return null;

        // A non-square stretch leaves a residual horizontal factor the em cannot carry. The text
        // is laid out at 1/stretch of where it goes and the factor is put into the transform, so
        // that the glyphs land exactly where they would have and are that much wider — which is
        // origin-independent, unlike scaling about the frame's own corner.
        double stretch = double.IsFinite(label.Stretch) && label.Stretch > 0.0 ? label.Stretch : 1.0;

        // The rectangle the text is laid out in, before rotation. Its width is the measured
        // width plus a hair, because a line broken at exactly its own measured width can wrap.
        DocSize box = new(measured.Width * 1.05 + Length.FromPoints(1), measured.Height);
        Length effective = measured.Width * stretch;

        DocPoint corner = label.Anchor switch
        {
            ChartLabelAnchor.CentreTop => new DocPoint(label.At.X - effective / 2, label.At.Y),
            ChartLabelAnchor.CentreBottom =>
                new DocPoint(label.At.X - effective / 2, label.At.Y - box.Height),
            ChartLabelAnchor.RightMiddle =>
                new DocPoint(label.At.X - effective, label.At.Y - box.Height / 2),
            ChartLabelAnchor.LeftMiddle => new DocPoint(label.At.X, label.At.Y - box.Height / 2),
            _ => new DocPoint(label.At.X - effective / 2, label.At.Y - box.Height / 2),
        };

        SlideTextBody body = lines.Length > 1
            ? Measurer.Lines(
                  lines, label.Size, label.Colour, label.Family, label.IsBold == true,
                  Adjust(label.Anchor))
            : Measurer.Body(label.Text, label.Size, label.Colour, label.Family, label.IsBold == true);

        AffineTransform transform = stretch == 1.0
            ? placement
            : AffineTransform.Concat(AffineTransform.Scale(stretch, 1.0), placement);

        DocRect area;

        if (label.Rotation != 0.0)
        {
            // Lay the text out at the origin and put the rotation into the transform, because a
            // glyph run carries an origin and advances rather than a matrix and cannot be
            // rotated after the fact.
            area = new DocRect(
                -box.Width / 2, -box.Height / 2, box.Width, box.Height);

            transform = AffineTransform.Concat(
                AffineTransform.Concat(
                    // Negated: `ChartLabel.Rotation` is anticlockwise, which is how both formats
                    // state one and how chart2's own shapes carry it, and the drawing space here has
                    // y growing downwards — so a positive angle handed straight to `Rotation` turns
                    // the text the other way. Measured: a two-word value axis title comes out reading
                    // top-to-bottom against the reference's bottom-to-top, and 45 degree category
                    // labels descend to the right against the reference's ascending. The box does not
                    // move, being symmetric about the same centre for either sign.
                    AffineTransform.Rotation(-label.Rotation),
                    AffineTransform.Translation(label.At.X.Emu, label.At.Y.Emu)),
                placement);
        }
        else
        {
            area = new DocRect(
                corner.X / stretch, corner.Y,
                lines.Length > 1 ? measured.Width : box.Width,
                box.Height);
        }

        List<PlacedGlyphRun> runs = OnChartDevice(
            SlideTextLayout.Place(body, area, fonts), label.Size);
        return runs.Count == 0 ? null : new PlacedText(runs, transform);
    }

    /// <summary>
    /// Puts a chart label's advances onto <c>chart2</c>'s own device before anything reads them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart's text is not laid out by Impress.</strong> It is built by
    /// <c>chart2</c>'s view as plain text shapes on the <c>VirtualDevice</c> that
    /// <c>DrawModelWrapper</c> creates from <c>Application::GetDefaultDevice()</c> with
    /// <c>MapUnit::Map100thMM</c> (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), and
    /// that device is <strong>96 dpi</strong> (<c>SvpSalGraphics::GetResolution</c>,
    /// <c>vcl/headless/svpgdi.cxx</c>:44). An <c>OutputDevice</c> instantiates a font at a whole
    /// number of device pixels, so at 10 pt it sets <strong>13</strong> for 13.333 and every
    /// advance the chart measures is 2.5% narrow, while at 11 pt it sets 15 for 14.667 and they
    /// are 2.3% wide. <see cref="MetricGrid.PixelEmScale"/> is that ratio.
    /// </para>
    /// <para>
    /// <strong>The glyphs keep the size the file states; only the pen moves.</strong> The
    /// reference draws a 10 pt label at 10.005 pt with the narrower advances rather than at
    /// 9.75 pt, so the em is quantised for the advance and not for the glyph — which is why this
    /// scales the run's positions and leaves <see cref="GlyphRun.FontSize"/> alone.
    /// </para>
    /// <para>
    /// <strong>Measuring and drawing take it together or neither may.</strong> The width a value
    /// axis' labels measure is what reserves the plot area and what right-aligns them against it,
    /// so a label measured wide and drawn narrow is aligned on a width it does not have — which
    /// is precisely what 24.2.7.2 does and 26.2.4.2 stopped doing. Both paths here go through
    /// this one call for that reason.
    /// </para>
    /// <para>
    /// <c>SheetBandText.ChartShape</c> is the same rule for a workbook's charts, landed one round
    /// earlier; <c>FrameChart.ChartFace.Shape</c> is the third. What is deliberately left out on
    /// all three is the reference's further rounding of each glyph's advance to a whole hundredth
    /// of a millimetre, worth at most 0.014 pt a glyph — see
    /// <see cref="MetricGrid.PixelEmScale"/>. Decorations in
    /// <see cref="PlacedGlyphRun.Rules"/> are in slide coordinates rather than run-relative ones
    /// and are left alone; a chart label carries none.
    /// </para>
    /// </remarks>
    /// <param name="runs">The runs the ordinary slide text layout produced.</param>
    /// <param name="size">The em size the label states.</param>
    private static List<PlacedGlyphRun> OnChartDevice(List<PlacedGlyphRun> runs, Length size)
    {
        double scale = MetricGrid.Chart.PixelEmScale(size);
        if (runs.Count == 0 || scale == 1.0) return runs;

        for (int i = 0; i < runs.Count; i++)
        {
            PlacedGlyphRun placed = runs[i];
            IReadOnlyList<PositionedGlyph> glyphs = placed.Run.Glyphs;
            List<PositionedGlyph> scaled = new(glyphs.Count);

            foreach (PositionedGlyph glyph in glyphs)
            {
                scaled.Add(glyph with
                {
                    Offset = new DocPoint(glyph.Offset.X * scale, glyph.Offset.Y),
                    Advance = glyph.Advance * scale,
                });
            }

            runs[i] = placed with { Run = placed.Run with { Glyphs = scaled } };
        }

        return runs;
    }

    /// <summary>
    /// Measures a line of chart text with the deck's own face cache.
    /// </summary>
    /// <remarks>
    /// Wraps <see cref="SlideTextLayout"/>'s own measurement so that the width a label is
    /// reserved and the width it is drawn at come from one place. A chart's labels are short and
    /// there are few of them, so measuring each twice — once to reserve room, once to place it —
    /// costs nothing worth caching.
    /// </remarks>
    /// <summary>The face a chart's own text falls back to when the file names none.</summary>
    /// <remarks>
    /// <para>
    /// A chart is not a slide shape and does not inherit the slide's typeface: with no theme to
    /// consult, chart2 gives its text <c>DefaultFontType::LATIN_SPREADSHEET</c>, which resolves to
    /// Liberation Sans on this machine. Leaving the run's face null substitutes the generic serif
    /// instead — which is not merely a different-looking label, because the axis labels'
    /// <em>width</em> is what reserves the plot area. Measured on <c>chart-bar-deck.odp</c>: the
    /// plot's left edge is 1.29 pt short in the serif and 0.44 pt long in Liberation Sans, so the
    /// wrong face was 1.73 pt of an error that three separate attempts hunted for in the label
    /// geometry.
    /// </para>
    /// <para>
    /// <strong>It was a constant for four rounds and that was wrong on every deck whose theme is
    /// not Arial.</strong> The evidence for the constant was <c>pdffonts</c> on LibreOffice's own
    /// PDF of <c>chart-bar-deck.pptx</c> reporting Liberation Sans — and that deck's chart states
    /// <c>&lt;a:latin typeface="Arial"/&gt;</c> eleven times, which fontconfig substitutes with
    /// Liberation Sans. The measurement was right and what it was evidence for was not. Two
    /// corpus decks separate the readings: <c>Demick_JetBlue.pptx</c>'s theme minor face is
    /// Constantia and the reference draws its chart in <em>DejaVu Serif</em>;
    /// <c>bitesize-writing-a-report.pptx</c>'s is Calibri and the reference draws its chart in
    /// Carlito. Neither is Liberation Sans and the first is not even a sans.
    /// </para>
    /// </remarks>
    private const string ChartFace = "Liberation Sans";

    private sealed class Measurer(SlideFonts fonts) : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
        {
            ArgumentNullException.ThrowIfNull(text);
            if (text.Length == 0) return new DocSize(Length.Zero, Length.Zero);

            // A label the arrangement broke is as wide as its widest line and as deep as the sum
            // of them — the same shape `ChartAxisLabels.Shape` reserves room for. Measuring the
            // joined run instead adds the lines together, which is a width no label ever has.
            if (text.AsSpan().IndexOf('\n') >= 0)
            {
                Length widest = Length.Zero;
                Length deep = Length.Zero;

                foreach (string line in text.Split('\n'))
                {
                    DocSize one = Measure(line, size, family, bold);
                    if (one.Width > widest) widest = one.Width;
                    deep += one.Height;
                }

                return new DocSize(widest, deep);
            }

            SlideTextBody body = Body(text, size, Colour.Black, family, bold);
            Length height = SlideTextLayout.Height(body, Length.Zero, fonts);

            // The width is summed from the glyphs the layout produced rather than estimated,
            // because it decides how much room the value axis' labels are given and an
            // underestimate puts the widest of them outside the frame. Laying the line out at
            // the origin and adding up its advances is the same arithmetic the layout used to
            // place them — through the same `OnChartDevice` call — so the two cannot disagree.
            Length width = Advances(body, height, size);

            // …and that is exactly why the trailing blanks have to be added back. A line's
            // trailing blanks are not part of its width — they hang past its right edge rather
            // than pushing a word onto the next line — so `SlideTextLayout` stops the drawn run
            // at `TextLine.VisibleEnd` and the sum above never sees them. That is right for a
            // wrapped body and wrong for a chart label, which is an EditEngine text shape
            // autogrown around the paragraph whole.
            //
            // An accounting number format puts one there on every value-axis tick: the positive
            // and zero subformats of `_("$"* #,##0.00_);…;_("$"* "-"??_)` both end in `_)`, a
            // blank the width of a closing parenthesis. Measured on `Demick_JetBlue.pptx` page 5
            // against 26.2.4.2 by deleting that one token from the format and changing nothing
            // else (`probes/chartblank-r191/`): the reference's plot area starts at x = 165.71
            // as authored and at 162.65 without it, with the gap from the widest label's last
            // glyph to the plot edge going 5.91 pt to 2.85 — and this tree drew 161.99 and 2.66,
            // which is the without-it answer. Deleting the zero row's two `?` placeholders
            // instead moves nothing, because that row is not the widest.
            //
            // The blanks are measured where they are not trailing, by putting a digit after them
            // and taking the digit back off; a space kerns against nothing in any of the faces a
            // chart resolves to, so the pair the sentinel introduces costs nothing.
            int visible = text.Length;
            while (visible > 0 && text[visible - 1] == ' ') visible--;

            if (visible < text.Length)
            {
                width += Advances(
                             Body(text[visible..] + Sentinel, size, Colour.Black, family, bold),
                             height, size)
                         - Advances(
                             Body(Sentinel, size, Colour.Black, family, bold), height, size);
            }

            return new DocSize(width, height);
        }

        /// <summary>A character that is neither blank nor dropped, for measuring blanks with.</summary>
        private const string Sentinel = "0";

        /// <summary>The sum of the advances the layout gave one body's glyphs.</summary>
        /// <param name="body">The one-line body.</param>
        /// <param name="height">The height to lay it out in.</param>
        /// <param name="size">The em size, for the chart device's own scale.</param>
        private Length Advances(SlideTextBody body, Length height, Length size)
        {
            Length width = Length.Zero;

            foreach (PlacedGlyphRun placed in OnChartDevice(
                SlideTextLayout.Place(
                    body, new DocRect(Length.Zero, Length.Zero, Length.Zero, height), fonts),
                size))
            {
                foreach (PositionedGlyph glyph in placed.Run.Glyphs) width += glyph.Advance;
            }

            return width;
        }

        /// <summary>
        /// A one-line, one-run, un-inset, unwrapped body — what every chart label is.
        /// </summary>
        /// <remarks>
        /// <para>
        /// <strong>A chart's text is measured by the face's own metrics, not by the em.</strong>
        /// The PPTX importer sets EditEngine's <c>FixedCellHeight</c> on every <em>slide shape's</em>
        /// text body (<c>oox/source/ppt/pptshapecontext.cxx:186</c>), which makes a line 1.2 em
        /// tall whatever face it is in — but a chart's labels are not slide shapes. They are made
        /// by <c>chart2</c>'s own view, which creates plain text shapes and sets no such flag, so
        /// their line height is the face's ascent and descent. For Liberation Sans that is
        /// 1.1499 em against 1.2 before any device — small individually, and it accumulates into
        /// the top and bottom insets that place the whole plot area.
        /// </para>
        /// <para>
        /// <strong>…and those metrics come through <c>chart2</c>'s own 96 dpi device, which is the
        /// vertical half of the same rule <see cref="OnChartDevice"/> applies horizontally.</strong>
        /// <see cref="SlideTextBody.Device"/> is what carries it, so the height
        /// <c>SlideTextLayout.Height</c> reserves and the baselines <c>SlideTextLayout.Place</c>
        /// draws on are quantised by one device and cannot drift apart. A 96 dpi pixel is 0.75 pt,
        /// so at 10 pt the device sets the em at <strong>13</strong> pixels and Liberation Sans
        /// stacks at 11.254 pt where its design metrics give 11.499 — and at 11 pt, where the em
        /// rounds <em>up</em> to 15, at 12.756 against 12.649. The external leading goes with the
        /// grid, which is right for the same reason it is everywhere else: <c>IsAddExtLeading()</c>
        /// is false in EditEngine and a chart label is an EditEngine text.
        /// </para>
        /// <para>
        /// Measured on both reference binaries in <c>probes/chart-vertical/</c>: three faces ×
        /// twelve sizes × two binaries × a deck and a Writer document, <strong>144 of 144</strong>
        /// baseline-to-baseline distances within 0.019 pt of this rule, where scaling the face's
        /// metrics exactly is out by up to 1.208 pt.
        /// </para>
        /// </remarks>
        internal static SlideTextBody Body(
            string text, Length size, Colour colour, string? family, bool bold = false)
            => new()
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = false,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = false,
            Device = MetricGrid.Chart,
            Paragraphs =
            [
                new SlideParagraph(
                    text,
                    [
                        new SlideTextRun(
                            0,
                            text.Length,
                            string.IsNullOrWhiteSpace(family) ? ChartFace : family.Trim(),
                            size,
                            bold ? 700 : 400,
                            false,
                            colour),
                    ],
                    TextAlignment.Start),
            ],
        };

        /// <summary>One paragraph per line, aligned on the edge the anchor names.</summary>
        /// <remarks>
        /// What a label the arrangement broke is drawn from. Each line is its own paragraph
        /// because the body does not wrap — <see cref="Body"/> sets <c>Wraps = false</c>, which
        /// is right for a chart label and means a newline inside one paragraph would be the only
        /// thing breaking it — and because the alignment is what carries the anchor across all of
        /// them at once.
        /// </remarks>
        internal static SlideTextBody Lines(
            string[] lines,
            Length size,
            Colour colour,
            string? family,
            bool bold,
            TextAlignment adjust)
        {
            SlideTextBody body =
                Body(string.Concat(lines), size, colour, family, bold);

            List<SlideParagraph> paragraphs = new(lines.Length);

            foreach (string line in lines)
            {
                paragraphs.Add(new SlideParagraph(
                    line,
                    [
                        new SlideTextRun(
                            0,
                            line.Length,
                            string.IsNullOrWhiteSpace(family) ? ChartFace : family.Trim(),
                            size,
                            bold ? 700 : 400,
                            false,
                            colour),
                    ],
                    adjust));
            }

            return body with { Paragraphs = paragraphs };
        }
    }
}
