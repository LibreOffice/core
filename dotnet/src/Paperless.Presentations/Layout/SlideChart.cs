using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.Text.Layout;

namespace Paperless.Presentations.Layout;

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
                Line = Pen(line.Colour, line.Width),
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
    private static Stroke Pen(Colour colour, Length width)
        => new(Paint.Solid(colour), width, LineCap.Butt, LineJoin.Miter);

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
    /// </remarks>
    private static PlacedText? Text(ChartLabel label, AffineTransform placement, SlideFonts fonts)
    {
        if (label.Text.Length == 0) return null;

        DocSize measured = new Measurer(fonts).Measure(label.Text, label.Size);
        if (measured.Width <= Length.Zero) return null;

        // The rectangle the text is laid out in, before rotation. Its width is the measured
        // width plus a hair, because a line broken at exactly its own measured width can wrap.
        DocSize box = new(measured.Width * 1.05 + Length.FromPoints(1), measured.Height);

        DocPoint corner = label.Anchor switch
        {
            ChartLabelAnchor.CentreTop => new DocPoint(label.At.X - box.Width / 2, label.At.Y),
            ChartLabelAnchor.RightMiddle =>
                new DocPoint(label.At.X - box.Width, label.At.Y - box.Height / 2),
            ChartLabelAnchor.LeftMiddle => new DocPoint(label.At.X, label.At.Y - box.Height / 2),
            _ => new DocPoint(label.At.X - box.Width / 2, label.At.Y - box.Height / 2),
        };

        SlideTextBody body = Measurer.Body(label.Text, label.Size, label.Colour);

        AffineTransform transform = placement;
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
                    AffineTransform.Rotation(label.Rotation),
                    AffineTransform.Translation(label.At.X.Emu, label.At.Y.Emu)),
                placement);
        }
        else
        {
            area = new DocRect(corner.X, corner.Y, box.Width, box.Height);
        }

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, fonts);
        return runs.Count == 0 ? null : new PlacedText(runs, transform);
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
    private sealed class Measurer(SlideFonts fonts) : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size)
        {
            ArgumentNullException.ThrowIfNull(text);
            if (text.Length == 0) return new DocSize(Length.Zero, Length.Zero);

            SlideTextBody body = Body(text, size, Colour.Black);
            Length height = SlideTextLayout.Height(body, Length.Zero, fonts);

            // The width is summed from the glyphs the layout produced rather than estimated,
            // because it decides how much room the value axis' labels are given and an
            // underestimate puts the widest of them outside the frame. Laying the line out at
            // the origin and adding up its advances is the same arithmetic the layout used to
            // place them, so the two cannot disagree.
            Length width = Length.Zero;
            foreach (PlacedGlyphRun placed in SlideTextLayout.Place(
                body, new DocRect(Length.Zero, Length.Zero, Length.Zero, height), fonts))
            {
                foreach (PositionedGlyph glyph in placed.Run.Glyphs) width += glyph.Advance;
            }

            return new DocSize(width, height);
        }

        /// <summary>
        /// A one-line, one-run, un-inset, unwrapped body — what every chart label is.
        /// </summary>
        /// <remarks>
        /// <strong>A chart's text is measured by the face's own metrics, not by the em.</strong>
        /// The PPTX importer sets EditEngine's <c>FixedCellHeight</c> on every <em>slide shape's</em>
        /// text body (<c>oox/source/ppt/pptshapecontext.cxx:186</c>), which makes a line 1.2 em
        /// tall whatever face it is in — but a chart's labels are not slide shapes. They are made
        /// by <c>chart2</c>'s own view, which creates plain text shapes and sets no such flag, so
        /// their line height is the face's ascent plus descent plus leading. For Liberation Sans
        /// that is 1.1499 em against 1.2, which is 0.15 pt on a 10 pt label and 0.65 pt on a
        /// 13 pt title — small individually, and the two accumulate into the top and bottom
        /// insets that place the whole plot area.
        /// </remarks>
        internal static SlideTextBody Body(string text, Length size, Colour colour) => new()
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = false,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = false,
            Paragraphs =
            [
                new SlideParagraph(
                    text,
                    [new SlideTextRun(0, text.Length, null, size, 400, false, colour)],
                    TextAlignment.Start),
            ],
        };
    }
}
