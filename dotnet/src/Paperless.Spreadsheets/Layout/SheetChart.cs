using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Paints a chart anchored on a sheet: a laid-out <see cref="ChartPlot"/> straight into the sink.
/// </summary>
/// <remarks>
/// <para>
/// The spreadsheet counterpart to <c>Paperless.Presentations.Layout.SlideChart</c>, and the two
/// share everything above the last step. <see cref="ChartLayout"/> gives back the same
/// <see cref="ChartDrawing"/> for both families; a slide turns it into <c>PlacedShape</c> values
/// for a backend to walk, and a sheet has no such list — <c>SheetPageGraphics</c> paints directly
/// — so this emits the fills, strokes and glyph runs itself. Everything that decides where a mark
/// goes is in <c>Paperless.Core.Charts</c> and is written once.
/// </para>
/// <para>
/// <strong>Paint order is the reference's.</strong> The chart's own background, the plot area's
/// wall, then the axes and their ticks, then the bars over them, then the text. Painting the bars
/// before the axes hides the axis line behind the first bar, which is a one-shape difference and
/// exactly what a fill-by-fill comparison against LibreOffice's PDF catches.
/// </para>
/// <para>
/// <strong>The print zoom scales the type, not just the rectangle.</strong> The box this is given
/// has already been through <c>SheetPageGraphics</c>'s scale; the font sizes have not, and a chart
/// laid out at 100% type inside a 50% rectangle reserves twice the room its labels need and
/// squeezes the plot area to nothing. So the sizes are scaled here rather than a transform being
/// pushed onto the sink, which keeps every glyph run in page coordinates and readable out of the
/// content stream.
/// </para>
/// </remarks>
internal static class SheetChart
{
    /// <summary>Paints one chart into the rectangle its anchor gave it.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="plot">The chart.</param>
    /// <param name="box">Where the frame lands on the page, already scaled.</param>
    /// <param name="scale">The print zoom, applied to the type.</param>
    public static void Draw(IDrawingSink sink, ChartPlot plot, DocRect box, double scale)
    {
        ArgumentNullException.ThrowIfNull(sink);
        ArgumentNullException.ThrowIfNull(plot);

        if (box.Width <= Length.Zero || box.Height <= Length.Zero) return;

        ChartDrawing drawing = ChartLayout.Place(Sized(plot, scale), box, Measurer.Instance);
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

        // The free-form marks — wedges, polylines, areas — after the axes and before the text.
        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Path.Commands.Count == 0) continue;

            if (shape.Fill is { } fill) sink.FillPath(shape.Path, Paint.Solid(fill));
            if (shape.Line is { } line) sink.StrokePath(shape.Path, Pen(line, shape.LineWidth));
        }

        foreach (ChartLabel label in drawing.Labels) Text(sink, label);
    }

    /// <summary>
    /// The chart with every stated type size taken through the print zoom.
    /// </summary>
    /// <remarks>
    /// Returned unchanged at 100%, which is every sheet in the corpus, so the common case allocates
    /// nothing.
    /// </remarks>
    private static ChartPlot Sized(ChartPlot plot, double scale)
        => scale == 1.0 || !double.IsFinite(scale) || scale <= 0.0
            ? plot
            : plot with
            {
                TitleSize = plot.TitleSize * scale,
                AxisTitleSize = plot.AxisTitleSize * scale,
                LabelSize = plot.LabelSize * scale,
            };

    /// <summary>
    /// The pen a chart's line is drawn with.
    /// </summary>
    /// <remarks>
    /// A zero width is passed through rather than replaced, because it is what the file states and
    /// what LibreOffice's export writes: <c>0 w</c> in the PDF, which every reader draws as the
    /// thinnest line the device has. Substituting a visible width makes every gridline and every
    /// bar outline heavier than the reference's.
    /// </remarks>
    private static Stroke Pen(Colour colour, Length width)
        => new(Paint.Solid(colour), width, LineCap.Butt, LineJoin.Miter);

    /// <summary>
    /// Draws one label, shaped and placed by its anchor.
    /// </summary>
    /// <remarks>
    /// <strong>A rotated label is drawn about its own centre.</strong> The value axis' title is the
    /// only rotated text a chart holds, a quarter turn, and a glyph run carries an origin and a
    /// list of advances rather than a matrix — so it cannot be turned after the fact. It is laid
    /// out at the origin instead and the turn goes onto the sink's state stack. Composing the
    /// rotation about the page's origin rather than the label's centre puts the title off the left
    /// of the sheet, which reads as the title having vanished rather than as a placement bug.
    /// </remarks>
    private static void Text(IDrawingSink sink, ChartLabel label)
    {
        if (label.Text.Length == 0) return;
        if (SheetBandText.Shape(label.Text, label.Size) is not { } run) return;

        Length line = SheetBandText.ChartLineHeightAt(label.Size);
        Length ascent = SheetBandText.AscentAt(label.Size);

        if (label.Rotation != 0.0)
        {
            sink.Save();

            sink.Transform(AffineTransform.Concat(
                AffineTransform.Rotation(label.Rotation),
                AffineTransform.Translation(label.At.X.Emu, label.At.Y.Emu)));

            sink.DrawGlyphRun(
                run.At(new DocPoint(-(run.Width / 2), -(line / 2) + ascent)),
                Paint.Solid(label.Colour));

            sink.Restore();
            return;
        }

        Length x = label.Anchor switch
        {
            ChartLabelAnchor.RightMiddle => label.At.X - run.Width,
            ChartLabelAnchor.LeftMiddle => label.At.X,
            _ => label.At.X - (run.Width / 2),
        };

        // CentreTop puts the label's *top* at the point; the other three centre it on the point.
        Length top = label.Anchor == ChartLabelAnchor.CentreTop
            ? label.At.Y
            : label.At.Y - (line / 2);

        sink.DrawGlyphRun(run.At(new DocPoint(x, top + ascent)), Paint.Solid(label.Colour));
    }

    /// <summary>
    /// Measures a line of chart text in the face a chart's labels are set in.
    /// </summary>
    /// <remarks>
    /// Stateless and shared, because a chart's labels are short, few, and all in one face:
    /// LibreOffice gives a chart's text the same default the rest of the document has, which for
    /// every sheet in the corpus resolves to Liberation Sans. A workbook whose chart states a face
    /// of its own is measured in the default and drawn in it too, so the two agree and the error is
    /// a substitution rather than a misplacement.
    /// </remarks>
    private sealed class Measurer : IChartTextMeasurer
    {
        public static Measurer Instance { get; } = new();

        public DocSize Measure(string text, Length size)
        {
            ArgumentNullException.ThrowIfNull(text);

            Length height = SheetBandText.ChartLineHeightAt(size);
            return text.Length == 0
                ? new DocSize(Length.Zero, height)
                : new DocSize(SheetBandText.Shape(text, size)?.Width ?? Length.Zero, height);
        }
    }
}
