using Paperless.Core.Numbers;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Core.Charts;

/// <summary>How a chart label sits against the point it is placed at.</summary>
public enum ChartLabelAnchor
{
    /// <summary>Centred horizontally, its top at the point — a category label below the axis.</summary>
    CentreTop = 0,

    /// <summary>
    /// Centred horizontally, its <em>bottom</em> at the point — a data label above a bar.
    /// </summary>
    /// <remarks>
    /// The counterpart of <see cref="CentreTop"/> and the anchor every "outside" data label on a
    /// column chart takes. Reusing <see cref="Centre"/> for it instead puts half the text inside
    /// the bar, which reads as a label that has not been offset rather than as a missing anchor.
    /// </remarks>
    CentreBottom,

    /// <summary>Right-aligned, centred vertically on the point — a value label left of the axis.</summary>
    RightMiddle,

    /// <summary>Left-aligned, centred vertically — a legend entry.</summary>
    LeftMiddle,

    /// <summary>Centred both ways — a title.</summary>
    Centre,
}

/// <summary>One piece of text a chart draws, with where and how it goes.</summary>
/// <param name="Text">The characters.</param>
/// <param name="At">The point the anchor refers to.</param>
/// <param name="Anchor">How the text sits against that point.</param>
/// <param name="Size">The em size.</param>
/// <param name="Colour">Its colour.</param>
/// <param name="Rotation">
/// Its rotation in radians, anticlockwise. Only a value-axis title is ever rotated, by a quarter
/// turn — which both formats state, OOXML as <c>a:bodyPr rot="-5400000"</c> and ODF as
/// <c>style:rotation-angle="90"</c>.
/// </param>
/// <param name="Stretch">
/// An extra horizontal scale applied to the glyphs, 1 for text drawn at its natural width.
/// </param>
/// <remarks>
/// <strong><paramref name="Stretch"/> exists because a glyph run carries one em and a
/// non-square stretch has two.</strong> An embedded chart is composed at its own size and scaled
/// into its frame by <c>(sx, sy)</c>; the positions take both, and <paramref name="Size"/> can
/// only take one, so the type came out <c>sx/sy</c> too wide or too narrow — 12% on
/// <c>chart-bar-sheet.ods</c>, whose 12 × 7 cm chart sits in a frame 0.625 as wide and 0.709 as
/// tall. Carrying the residual here and letting each consumer put it into its own transform is
/// what closes that, and it costs a chart that is not stretched nothing at all: the factor is
/// exactly 1 and both consumers take their unstretched path.
/// </remarks>
public readonly record struct ChartLabel(
    string Text,
    DocPoint At,
    ChartLabelAnchor Anchor,
    Length Size,
    Colour Colour,
    double Rotation = 0.0,
    double Stretch = 1.0);

/// <summary>One filled rectangle — a bar, a legend key, the plot area's wall.</summary>
/// <param name="Bounds">Where it goes.</param>
/// <param name="Fill">Its fill, or null when it is outline only.</param>
/// <param name="Line">Its outline colour, or null when it has none.</param>
/// <param name="LineWidth">The outline's width.</param>
public readonly record struct ChartBox(
    DocRect Bounds,
    Colour? Fill,
    Colour? Line = null,
    Length LineWidth = default);

/// <summary>One straight line — an axis, a tick, a gridline.</summary>
/// <param name="From">Its start.</param>
/// <param name="To">Its end.</param>
/// <param name="Colour">Its colour.</param>
/// <param name="Width">Its width; zero is a hairline.</param>
public readonly record struct ChartLine(
    DocPoint From, DocPoint To, Colour Colour, Length Width = default);

/// <summary>
/// One free-form mark — a line chart's polyline, an area's filled region, a pie's wedge.
/// </summary>
/// <remarks>
/// A path rather than a rectangle because those three cannot be expressed as one, and a path
/// rather than three shapes because a renderer treats them identically: fill it, stroke it, or
/// both. <see cref="GraphicsPath"/> lives in <c>Paperless.Core.Graphics</c>, so producing one here
/// costs the layout nothing it did not already depend on.
/// </remarks>
/// <param name="Path">The outline, already closed where it should be.</param>
/// <param name="Fill">Its fill, or null when it is a stroke only — which is a line chart.</param>
/// <param name="Line">Its outline colour, or null for none.</param>
/// <param name="LineWidth">The outline's width; zero is a hairline.</param>
public readonly record struct ChartShape(
    GraphicsPath Path,
    Colour? Fill,
    Colour? Line = null,
    Length LineWidth = default);

/// <summary>
/// A chart laid out: every mark it draws, in paint order, in the frame's coordinates.
/// </summary>
/// <param name="PlotArea">The inner plot rectangle — the axes' extent, labels excluded.</param>
/// <param name="Boxes">The filled and outlined rectangles, back to front.</param>
/// <param name="Lines">The axes, ticks and gridlines.</param>
/// <param name="Labels">The text.</param>
/// <param name="Shapes">
/// The paths — wedges, polylines and areas — drawn after <paramref name="Boxes"/> and before
/// <paramref name="Labels"/>, which is where the reference draws them.
/// </param>
public sealed record ChartDrawing(
    DocRect PlotArea,
    IReadOnlyList<ChartBox> Boxes,
    IReadOnlyList<ChartLine> Lines,
    IReadOnlyList<ChartLabel> Labels,
    IReadOnlyList<ChartShape> Shapes);

/// <summary>
/// Measures a single line of chart text, so that layout can reserve room for it.
/// </summary>
/// <remarks>
/// An interface rather than a font cache because <see cref="ChartLayout"/> sits below every
/// library that owns one, and because the two consumers — a slide and a sheet — resolve faces
/// through caches of their own that may not agree on a substitution.
/// </remarks>
public interface IChartTextMeasurer
{
    /// <summary>The advance width and line height of a single line of text.</summary>
    /// <param name="text">The characters.</param>
    /// <param name="size">The em size.</param>
    DocSize Measure(string text, Length size);
}

/// <summary>
/// Composes a chart: where the plot area goes, and every mark inside and around it.
/// </summary>
/// <remarks>
/// <para>
/// A port of the composition in <c>chart2/source/view/main/ChartView.cxx</c> and the bar
/// geometry in <c>chart2/source/view/charttypes/BarChart.cxx</c>, at the level of "what
/// rectangle does each thing occupy". What it deliberately does <em>not</em> port is
/// LibreOffice's two-pass refinement, in which the axis labels are laid out, measured, and the
/// whole diagram re-laid-out around them until it settles
/// (<c>ChartView::impl_createDiagramAndContent</c>). One pass with measured labels gets the plot
/// rectangle close; the second pass moves it by a point or two.
/// </para>
/// <para>
/// <strong>Which is why the file is consulted first.</strong> ODF states the answer outright, in
/// <c>chart:coordinate-region</c>, so an ODF chart skips the composition entirely and is exact
/// by construction. OOXML states nothing unless the author moved the plot area by hand, so a
/// PPTX or XLSX chart goes through the computation. That asymmetry is the whole reason the two
/// paths exist, and it is the second time on this feature that ODF turned out to carry a baked
/// answer OOXML does not.
/// </para>
/// <para>
/// <strong>The arithmetic inside the plot area is exact and is the part that matters.</strong>
/// Measured against LibreOffice's PDF for <c>chart-bar-deck.pptx</c>, whose plot area is
/// 500.967 pt wide over four categories and two clustered series with
/// <c>c:gapWidth val="100"</c>: a category slot is 125.242 pt, a bar is
/// <c>slot / (series + gap/100)</c> = 41.747 pt against a reference 41.754, and the first bar's
/// left edge is <c>slotLeft + gap/100 × barWidth / 2</c> = 20.874 pt into the slot against a
/// reference 20.863. Both agree to within the hundredth of a millimetre LibreOffice rounds its
/// own coordinates to.
/// </para>
/// </remarks>
public static partial class ChartLayout
{
    /// <summary>The length of a major tick mark, outside the axis.</summary>
    /// <remarks>
    /// <c>AXIS2D_TICKLENGTH = 150</c> hundredths of a millimetre
    /// (<c>chart2/source/view/inc/ViewDefines.hxx:30</c>), commented "value like in old chart"
    /// — so it is a fixed length rather than one derived from the font, and it does not scale
    /// with the chart. Confirmed in the reference PDF: every tick runs exactly 4.252 pt outside
    /// the axis, which is 150 hundredths of a millimetre to three decimal places.
    /// </remarks>
    private static readonly Length TickLength = Length.FromMm100(150);

    /// <summary>The gap between a tick and the label beside it.</summary>
    /// <remarks><c>AXIS2D_TICKLABELSPACING = 100</c> (<c>ViewDefines.hxx:31</c>).</remarks>
    private static readonly Length LabelSpacing = Length.FromMm100(100);

    /// <summary>The extra gap below a main title, beyond the proportional one.</summary>
    /// <remarks>
    /// <c>lcl_createTitle</c> adds a flat 135 hundredths of a millimetre for a main title, on
    /// top of 2% of the chart height (<c>ChartView.cxx:1066-1069</c>).
    /// </remarks>
    private static readonly Length TitleGap = Length.FromMm100(135);

    /// <summary>The gap between the category axis' labels and its title.</summary>
    /// <remarks><c>ChartView.cxx:1070-1073</c>, a flat 420 rather than a proportion.</remarks>
    private static readonly Length CategoryTitleGap = Length.FromMm100(420);

    /// <summary>The gap between the value axis' labels and its title.</summary>
    /// <remarks><c>ChartView.cxx:1074-1077</c>, a flat 450.</remarks>
    private static readonly Length ValueTitleGap = Length.FromMm100(450);

    /// <summary>The margin round the whole chart, as a fraction of its own size.</summary>
    /// <remarks>
    /// <c>constPageLayoutDistancePercentage = 0.02</c> (<c>ChartView.cxx:918</c>), applied to the
    /// chart's width horizontally and its height vertically — so it is not a square margin on a
    /// chart that is not square.
    /// </remarks>
    private const double PageMargin = 0.02;

    /// <summary>The colour LibreOffice draws an axis, its ticks and its labels in.</summary>
    private static readonly Colour AxisColour = Colour.Black;

    /// <summary>
    /// A chart text shape's horizontal inset, as a fraction of the font height.
    /// </summary>
    /// <remarks>
    /// <c>ShapeFactory::createText</c> sets <c>TextLeftDistance</c> and <c>TextRightDistance</c>
    /// to <c>round(fontHeight × 0.18)</c> and the vertical pair to <c>× 0.30</c>, both in
    /// hundredths of a millimetre, under the comment
    /// "#i109336# Improve auto positioning in chart"
    /// (<c>chart2/source/view/main/ShapeFactory.cxx:2279-2299</c>). Every piece of text a chart
    /// draws goes through that function, so what the layout reserves is the <em>shape's</em>
    /// size and not the text's — 36% of the font height wider and 60% taller. On the corpus
    /// chart's 13 pt title that is 2.75 pt of extra height, all of it above the plot area, and
    /// it was the single largest term missing from the first version of this layout.
    /// </remarks>
    private const double TextShapeInsetX = 0.18;

    /// <summary>A chart text shape's vertical inset, as a fraction of the font height.</summary>
    /// <remarks><c>ShapeFactory.cxx:2285</c>; see <see cref="TextShapeInsetX"/>.</remarks>
    private const double TextShapeInsetY = 0.30;

    /// <summary>
    /// The size of the shape a piece of chart text is drawn in, insets included.
    /// </summary>
    /// <remarks>
    /// What the composition reserves room for. Measuring the text alone under-reserves on every
    /// side at once, which moves the plot area up and left and leaves the labels crowding the
    /// frame's edges.
    /// </remarks>
    private static DocSize Shape(IChartTextMeasurer measurer, string text, Length size)
    {
        DocSize measured = measurer.Measure(text, size);
        return new DocSize(
            measured.Width + size * (TextShapeInsetX * 2),
            measured.Height + size * (TextShapeInsetY * 2));
    }

    /// <summary>Lays a chart out inside a frame.</summary>
    /// <param name="plot">The chart.</param>
    /// <param name="frame">The graphic frame's rectangle, in the caller's coordinates.</param>
    /// <param name="measurer">Measures a line of text, for reserving room.</param>
    public static ChartDrawing Place(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        ArgumentNullException.ThrowIfNull(plot);
        ArgumentNullException.ThrowIfNull(measurer);

        if (frame.Width <= Length.Zero || frame.Height <= Length.Zero)
            return new ChartDrawing(DocRect.Empty, [], [], [], []);

        // A chart with a coordinate space of its own is composed at its own size and the whole
        // picture is then stretched into the frame. See Stretch.
        if (plot.Space is not { } space
            || space.Width <= Length.Zero
            || space.Height <= Length.Zero
            || (space.Width == frame.Width && space.Height == frame.Height))
        {
            return Compose(plot, frame, measurer);
        }

        DocRect own = new(Length.Zero, Length.Zero, space.Width, space.Height);
        return Stretch(Compose(plot, own, measurer), own, frame);
    }

    /// <summary>
    /// Stretches a chart composed at its own size onto the frame that displays it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An embedded chart is rendered at its own size and scaled, not re-laid-out.</strong>
    /// Measured on <c>chart-bar-sheet.ods</c>, whose chart document states <c>svg:width="12cm"</c>
    /// by <c>svg:height="7cm"</c> and whose frame on the sheet is 2.952 in by 1.9547 in — a scale
    /// of 0.625 across and 0.709 down. In LibreOffice's own PDF the chart's 13 pt title measures
    /// 62.1 pt wide against the 99.4 pt the same title measures in the same chart's
    /// <c>.xlsx</c> form, which is 0.625 exactly; its height ratio is 0.708. So the type is
    /// stretched with everything else, by two different factors, and the chart is <em>not</em>
    /// re-composed for the smaller frame.
    /// </para>
    /// <para>
    /// <strong>Which is what decides the tick count, so it is not cosmetic.</strong> Composing
    /// <c>chart-bar-sheet.ods</c> in its frame gives an axis 77 pt long, which has room for six
    /// intervals and lands on <c>0 50 … 200</c>; composing it at its own 12 × 7 cm gives one
    /// 108.8 pt long, room for nine, and <c>0 20 … 180</c> — which is what the reference draws.
    /// </para>
    /// <para>
    /// <strong>The one thing that cannot follow the em, and where it goes instead.</strong> A
    /// glyph run carries one em size, so the type is scaled by the vertical factor alone and the
    /// residual <c>sx/sy</c> — 12% on this chart — is carried on
    /// <see cref="ChartLabel.Stretch"/> for each consumer to fold into its own transform. Dropping
    /// it, which is what this did at first, draws every word of a stretched chart
    /// <c>sx/sy</c> too wide against a reference that is exact.
    /// </para>
    /// </remarks>
    private static ChartDrawing Stretch(ChartDrawing drawing, DocRect from, DocRect frame)
    {
        double sx = (double)frame.Width.Emu / from.Width.Emu;
        double sy = (double)frame.Height.Emu / from.Height.Emu;

        DocPoint At(DocPoint point)
            => new(frame.X + point.X * sx, frame.Y + point.Y * sy);

        DocRect Box(DocRect rectangle)
            => new(
                frame.X + rectangle.X * sx,
                frame.Y + rectangle.Y * sy,
                rectangle.Width * sx,
                rectangle.Height * sy);

        List<ChartBox> boxes = new(drawing.Boxes.Count);
        foreach (ChartBox box in drawing.Boxes)
            boxes.Add(box with { Bounds = Box(box.Bounds), LineWidth = box.LineWidth * sy });

        List<ChartLine> lines = new(drawing.Lines.Count);
        foreach (ChartLine line in drawing.Lines)
            lines.Add(line with { From = At(line.From), To = At(line.To), Width = line.Width * sy });

        // The em follows the vertical factor because that is what a line height is; the residual
        // sx/sy goes onto the label as a horizontal scale for the consumer to apply.
        double residual = sy == 0.0 ? 1.0 : sx / sy;

        List<ChartLabel> labels = new(drawing.Labels.Count);
        foreach (ChartLabel label in drawing.Labels)
        {
            labels.Add(label with
            {
                At = At(label.At),
                Size = label.Size * sy,
                Stretch = label.Stretch * residual,
            });
        }

        List<ChartShape> shapes = new(drawing.Shapes.Count);
        foreach (ChartShape shape in drawing.Shapes)
            shapes.Add(shape with { Path = Stretched(shape.Path), LineWidth = shape.LineWidth * sy });

        return new ChartDrawing(Box(drawing.PlotArea), boxes, lines, labels, shapes);

        GraphicsPath Stretched(GraphicsPath path)
        {
            GraphicsPath moved = new();

            foreach (PathCommand command in path.Commands)
            {
                switch (command.Verb)
                {
                    case PathVerb.MoveTo: moved.MoveTo(At(command.Point)); break;
                    case PathVerb.LineTo: moved.LineTo(At(command.Point)); break;
                    case PathVerb.CubicTo:
                        moved.CubicTo(At(command.Control1), At(command.Control2), At(command.Point));
                        break;
                    default: moved.Close(); break;
                }
            }

            return moved;
        }
    }

    /// <summary>Back to front, which is the order a combination chart's groups are painted in.</summary>
    private static readonly ChartPlotKind[] DrawingOrder =
    [
        ChartPlotKind.Area,
        ChartPlotKind.Bar,
        ChartPlotKind.Stock,
        ChartPlotKind.Line,
        ChartPlotKind.Scatter,
        ChartPlotKind.Radar,
        ChartPlotKind.Bubble,
        ChartPlotKind.Pie,
        ChartPlotKind.OfPie,
    ];

    /// <summary>Composes a chart in the coordinates it is measured in.</summary>
    private static ChartDrawing Compose(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        List<ChartBox> boxes = [];
        List<ChartLine> lines = [];
        List<ChartLabel> labels = [];
        List<ChartShape> shapes = [];

        if (plot.Background is { } background)
            boxes.Add(new ChartBox(frame, background));

        int categories = plot.CategoryCount();
        (double? dataMinimum, double? dataMaximum) = plot.ValueRange(0);

        // A radar chart's radius axis is capped at two intervals whatever it is drawn at:
        // VPolarRadiusAxis::estimateMaximumAutoMainIncrementCount returns a flat 2
        // (chart2/source/view/axes/VPolarRadiusAxis.cxx:87-90) where the cartesian one derives a
        // count from the axis' length. That is the whole of why a radar chart's web has three
        // rings and not eleven, and it is measured: radar-chart-labels.docx peaks at 40 and
        // LibreOffice draws rings at 0, 20 and 40.
        ChartScaleResult scale = ChartScale.Resolve(
            plot.ValueScale,
            dataMinimum,
            dataMaximum,
            maximumIntervals: plot.Kind is ChartPlotKind.Radar
                ? RadarIntervalCount
                : ChartScale.MaximumAutoIntervalCount);

        // A scatter chart's X is a numeric dimension with a scale of its own rather than a run of
        // category slots, so it is resolved here and threaded through everything that maps a point
        // across the plot area. Null for every other type, which is what keeps the category path
        // unchanged.
        ChartScaleResult? domain = DomainScaleOf(plot);

        (double? secondMinimum, double? secondMaximum) = plot.ValueRange(1);
        ChartScaleResult? secondary = plot.HasSecondaryAxis
            ? ChartScale.Resolve(plot.SecondaryValueScale!.Value, secondMinimum, secondMaximum)
            : null;

        DocRect area = PlotAreaOf(plot, frame, scale, secondary, domain, categories, measurer);
        if (area.Width <= Length.Zero || area.Height <= Length.Zero)
            return new ChartDrawing(DocRect.Empty, boxes, lines, labels, shapes);

        bool columns = plot.Direction == ChartBarDirection.Column;

        // LibreOffice's second pass, narrowed to the one thing it changes here. How many ticks an
        // axis may have is decided from how long that axis turned out and how tall — or wide — a
        // label turned out, and neither is known until the labels have been laid out once. So the
        // first pass runs at ten intervals, the count is re-derived from the rectangle it produced,
        // and the scale and the rectangle are computed again if it came out lower.
        if (plot.HasAxes)
        {
            int fitting = IntervalsThatFit(plot, area, columns, scale, measurer);
            if (fitting < ChartScale.MaximumAutoIntervalCount)
            {
                scale = ChartScale.Resolve(
                    plot.ValueScale, dataMinimum, dataMaximum, maximumIntervals: fitting);

                if (secondary is not null)
                {
                    secondary = ChartScale.Resolve(
                        plot.SecondaryValueScale!.Value, secondMinimum, secondMaximum,
                        maximumIntervals: fitting);
                }

                area = PlotAreaOf(plot, frame, scale, secondary, domain, categories, measurer);
                if (area.Width <= Length.Zero || area.Height <= Length.Zero)
                    return new ChartDrawing(DocRect.Empty, boxes, lines, labels, shapes);
            }
        }

        if (plot.PlotBackground is { } wall) boxes.Add(new ChartBox(area, wall));

        if (plot.HasAxes)
        {
            AddValueAxis(plot, area, scale, columns, plot.ValueFormat, false, lines, labels);

            if (secondary is { } second && plot.SecondaryAxisVisible)
            {
                AddValueAxis(
                    plot, area, second, columns, plot.SecondaryValueFormat, true, lines, labels);
            }

            if (domain is { } across) AddDomainAxis(plot, area, across, columns, lines, labels);
            else AddCategoryAxis(plot, area, categories, columns, lines, labels);
        }
        else if (plot.Kind is ChartPlotKind.Radar)
        {
            AddRadarAxis(plot, area, scale, categories, lines, labels);
        }

        // Every plot group is drawn, not only the first, and the order is back to front: areas
        // fill, bars sit on them, lines go over both. A part holding a c:barChart and a
        // c:lineChart over one pair of axes is an ordinary combination chart, and drawing only
        // the first group loses whole series — measured on
        // stacked-non-stacked-mix-y-axis.pptx, whose third chart holds one area series and two
        // bar series and came out with one of the three.
        //
        // Split again by axis index, because a series on the secondary axis is measured against a
        // different scale and drawing it against the primary one puts it at a plausible but wrong
        // height — the same failure the automatic scale itself has.
        foreach (ChartPlotKind kind in DrawingOrder)
        {
            for (int axis = 0; axis <= 1; axis++)
            {
                if (axis == 1 && secondary is null) continue;

                List<ChartSeries> subset = plot.SeriesOf(kind, axis);
                if (subset.Count == 0) continue;

                ChartPlot part = plot with { Series = subset };
                ChartScaleResult against = axis == 1 ? secondary!.Value : scale;

                switch (kind)
                {
                    case ChartPlotKind.Pie:
                        AddWedges(part, area, shapes, labels);
                        break;
                    case ChartPlotKind.Area:
                        AddAreas(part, area, against, categories, columns, shapes, labels);
                        break;
                    case ChartPlotKind.Line:
                    case ChartPlotKind.Scatter:
                        AddLines(part, area, against, domain, categories, columns, shapes, labels);
                        break;
                    case ChartPlotKind.Radar:
                        AddRadar(part, area, against, categories, shapes, labels);
                        break;
                    case ChartPlotKind.Bubble:
                        AddBubbles(part, area, against, domain, shapes, labels);
                        break;
                    case ChartPlotKind.Stock:
                        AddCandles(part, area, against, categories, boxes, lines, labels);
                        break;
                    case ChartPlotKind.OfPie:
                        AddOfPie(part, area, shapes, lines, labels);
                        break;
                    default:
                        AddBars(part, area, against, categories, columns, boxes, labels);
                        break;
                }
            }
        }

        AddTitles(plot, frame, area, measurer, labels);
        AddLegend(plot, frame, area, measurer, boxes, labels);

        return new ChartDrawing(area, boxes, lines, labels, shapes);
    }

    /// <summary>
    /// A scatter chart's X scale, or null when the chart is plotted against categories.
    /// </summary>
    /// <remarks>
    /// <c>c:xVal</c> is a data sequence like any other and gets an axis scale of its own through
    /// the same automatism — except that <c>isExpandWideValuesToZero</c> is false for an X axis
    /// (<c>VSeriesPlotter::isExpandWideValuesToZero</c> tests <c>nDimensionIndex == 1</c>,
    /// <c>VSeriesPlotter.cxx:1742-1746</c>), so a domain running 20 to 120 keeps its minimum near
    /// 20 rather than being pulled down to zero the way a value axis would be.
    /// </remarks>
    private static ChartScaleResult? DomainScaleOf(ChartPlot plot)
    {
        double minimum = double.PositiveInfinity;
        double maximum = double.NegativeInfinity;
        bool any = false;

        foreach (ChartSeries series in plot.Series)
        {
            // A bubble chart is a scatter chart with a third number, so its X is a numeric
            // dimension with a scale of its own in exactly the same way — and its chart part has
            // the same pair of c:valAx and the same trap in reading them.
            if ((series.Kind ?? plot.Kind) is not (ChartPlotKind.Scatter or ChartPlotKind.Bubble))
                continue;

            if (series.XValues is not { } values) continue;

            foreach (double? point in values)
            {
                if (point is not { } value || !double.IsFinite(value)) continue;
                minimum = Math.Min(minimum, value);
                maximum = Math.Max(maximum, value);
                any = true;
            }
        }

        return any
            ? ChartScale.Resolve(plot.DomainScale, minimum, maximum, expandToZero: false)
            : null;
    }

    /// <summary>
    /// How many major intervals the value axis has room for, once its length is known.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VCartesianAxis::estimateMaximumAutoMainIncrementCount</c>
    /// (<c>chart2/source/view/axes/VCartesianAxis.cxx:1559-1618</c>): the axis line's own length
    /// divided by the largest label shape measured so far — its <em>height</em> for a vertical
    /// axis and its <em>width</em> for a horizontal one — and ten whenever nothing has been
    /// measured yet, which is what makes the first pass ten.
    /// </para>
    /// <para>
    /// <strong>The text, not the shape it sits in.</strong> Every other reservation in this file
    /// uses <see cref="Shape"/>, which adds <c>ShapeFactory</c>'s insets; this one must not, and
    /// the corpus pins it down from both sides. <c>chart-bar-sheet.xlsx</c> draws its axis 54.6 pt
    /// long and LibreOffice labels it <c>0 50 … 200</c>: that is four intervals, which
    /// <c>54.6 / 11.5</c> gives and <c>54.6 / 17.5</c> does not — 17.5 gives three, and three
    /// forces the interval to 100. <c>chart-bar-sheet.ods</c> draws its axis 108.8 pt long and is
    /// labelled <c>0 20 … 180</c>: nine intervals, which <c>108.8 / 11.5</c> gives and
    /// <c>108.8 / 17.5</c> does not.
    /// </para>
    /// <para>
    /// <strong>And it is what separates a chart from a smaller copy of the same chart.</strong>
    /// <c>chart-bar-deck.odp</c> and <c>chart-bar-sheet.ods</c> hold the same eight numbers,
    /// peaking at 168, and LibreOffice labels the deck <c>0 20 … 180</c> over an axis 242 pt long
    /// and the sheet's <c>.xlsx</c> form <c>0 50 … 200</c> over one 55 pt long. Reproducing the
    /// deck without this and reusing it draws every bar 10% too tall against ticks that read
    /// perfectly plausibly.
    /// </para>
    /// </remarks>
    private static int IntervalsThatFit(
        ChartPlot plot,
        DocRect area,
        bool columns,
        ChartScaleResult scale,
        IChartTextMeasurer measurer)
    {
        // A stated interval is honoured whatever fits; only the automatic one is re-derived.
        if (plot.ValueScale.MajorUnit is { } stated && stated > 0.0)
            return ChartScale.MaximumAutoIntervalCount;

        Length available;
        Length needed;

        if (columns)
        {
            available = area.Height;
            needed = measurer.Measure("0", plot.LabelSize).Height;
        }
        else
        {
            available = area.Width;
            needed = Length.Zero;

            foreach (double tick in scale.MajorTicks())
            {
                Length width = measurer.Measure(
                    ChartDataLabel.Write(tick, plot.ValueFormat), plot.LabelSize).Width;
                if (width > needed) needed = width;
            }
        }

        if (needed <= Length.Zero) return ChartScale.MaximumAutoIntervalCount;

        return Math.Clamp(
            (int)(available.Emu / needed.Emu),
            ChartScale.MinimumAutoIntervalCount,
            ChartScale.MaximumAutoIntervalCount);
    }

    /// <summary>
    /// The inner plot rectangle: stated by the file when it states one, computed otherwise.
    /// </summary>
    private static DocRect PlotAreaOf(
        ChartPlot plot,
        DocRect frame,
        ChartScaleResult scale,
        ChartScaleResult? secondary,
        ChartScaleResult? domain,
        int categories,
        IChartTextMeasurer measurer)
    {
        // Absolute, in the chart's own coordinates — ODF's chart:coordinate-region, which is
        // already in whatever space Place composed in.
        if (plot.PlotArea is { } stated)
            return new DocRect(frame.X + stated.X, frame.Y + stated.Y, stated.Width, stated.Height);

        // Fractions of the frame — OOXML's c:manualLayout, which states no space of its own.
        if (plot.PlotAreaFraction is { } fraction)
        {
            return new DocRect(
                frame.X + frame.Width * fraction.X,
                frame.Y + frame.Height * fraction.Y,
                frame.Width * fraction.Width,
                frame.Height * fraction.Height);
        }

        // The computed path, which is what every OOXML chart takes. Start from the frame less
        // the proportional page margin, then take away the title, the legend and the axes'
        // labels and titles in that order — ChartView.cxx:920-990 for the margin,
        // lcl_createTitle for the title, lcl_createLegend for the legend.
        Length marginX = frame.Width * PageMargin;
        Length marginY = frame.Height * PageMargin;

        Length left = frame.X + marginX;
        Length top = frame.Y + marginY;
        Length right = frame.Right - marginX;
        Length bottom = frame.Bottom - marginY;

        if (plot.Title is { Length: > 0 } title)
            top += Shape(measurer, title, plot.TitleSize).Height + marginY + TitleGap;

        Length legend = LegendWidth(plot, measurer);
        switch (plot.Legend)
        {
            case ChartLegendPosition.Right: right -= legend + marginX; break;
            case ChartLegendPosition.Left: left += legend + marginX; break;
            case ChartLegendPosition.Top: top += LegendHeight(plot, measurer) + marginY; break;
            case ChartLegendPosition.Bottom: bottom -= LegendHeight(plot, measurer) + marginY; break;
            default: break;
        }

        // A chart with no axes — a pie — reserves nothing for labels it does not draw, and what is
        // left after the title and the legend is the whole diagram.
        if (!plot.HasAxes)
        {
            // Except a radar chart, whose category labels sit *outside* the web on all four sides
            // rather than along one edge. Measured on chart2/qa/extras/data/docx/radar-chart-labels.docx,
            // whose frame is 431.2 x 251.2 pt: LibreOffice draws the outermost web at a radius of
            // 104.8 pt, and reserving a text shape's height above and below — 11.5 pt of text plus
            // ShapeFactory's two 0.30 em insets, so 17.5 — leaves 206.2 pt and a radius of 103.1.
            // Reserving nothing gives 120.6, which is 15% too big and puts the top vertex through
            // its own label.
            if (plot.Kind is ChartPlotKind.Radar && plot.CategoryAxisVisible)
            {
                Length wide = WidestCategoryLabel(plot, categories, measurer);
                Length tall = Shape(measurer, "0", plot.LabelSize).Height;

                left += wide;
                right -= wide;
                top += tall;
                bottom -= tall;
            }

            return right <= left || bottom <= top
                ? DocRect.Empty
                : new DocRect(left, top, right - left, bottom - top);
        }

        bool columns = plot.Direction == ChartBarDirection.Column;

        // The value axis' labels are as wide as the widest of them; the category axis' are one
        // line tall. Both sit a tick length plus a label spacing away from the axis.
        // A deleted axis reserves nothing, which is the whole of what makes the plot area grow
        // into the room its labels would have taken.
        Length valueLabel = plot.ValueAxisVisible
            ? WidestValueLabel(scale, plot.ValueFormat, plot.LabelSize, measurer)
            : Length.Zero;

        Length labelHeight = measurer.Measure("0", plot.LabelSize).Height;

        // A scatter chart's horizontal axis is numeric, so what sits under it is the widest of its
        // own ticks rather than the widest category name — and the last of them overhangs the
        // right edge by half its width exactly as a horizontal value axis' does.
        Length categoryLabel = !plot.CategoryAxisVisible
            ? Length.Zero
            : domain is { } across
                ? WidestValueLabel(across, plot.DomainFormat, plot.LabelSize, measurer)
                : WidestCategoryLabel(plot, categories, measurer);

        Length valueSpace = plot.ValueAxisVisible ? TickLength + LabelSpacing : Length.Zero;
        Length categorySpace = plot.CategoryAxisVisible ? TickLength + LabelSpacing : Length.Zero;
        Length categoryHeight = plot.CategoryAxisVisible ? labelHeight : Length.Zero;
        Length valueHeight = plot.ValueAxisVisible ? labelHeight : Length.Zero;

        if (columns)
        {
            left += valueLabel + valueSpace;
            bottom -= categoryHeight + categorySpace;

            if (plot.ValueAxisTitle is { Length: > 0 } valueTitle)
                left += Shape(measurer, valueTitle, plot.AxisTitleSize).Height + ValueTitleGap;
            if (plot.CategoryAxisTitle is { Length: > 0 } categoryTitle)
                bottom -= Shape(measurer, categoryTitle, plot.AxisTitleSize).Height + CategoryTitleGap;

            // A secondary value axis is drawn on the far side of the plot area and reserves its
            // own labels there, which is the whole of what makes room for it.
            if (secondary is { } second && plot.SecondaryAxisVisible)
            {
                right -= WidestValueLabel(
                             second, plot.SecondaryValueFormat, plot.LabelSize, measurer)
                         + TickLength + LabelSpacing;

                if (plot.SecondaryValueAxisTitle is { Length: > 0 } secondTitle)
                    right -= Shape(measurer, secondTitle, plot.AxisTitleSize).Height + ValueTitleGap;
            }
            else if (domain is not null)
            {
                right -= categoryLabel / 2;
            }

            // The topmost value label is centred on the axis' top, so half of it sticks out
            // above the plot area and would be clipped by the frame without this.
            top += valueHeight / 2;
        }
        else
        {
            left += categoryLabel + categorySpace;
            bottom -= valueHeight + valueSpace;

            if (plot.CategoryAxisTitle is { Length: > 0 } categoryTitle)
                left += Shape(measurer, categoryTitle, plot.AxisTitleSize).Height + ValueTitleGap;
            if (plot.ValueAxisTitle is { Length: > 0 } valueTitle)
                bottom -= Shape(measurer, valueTitle, plot.AxisTitleSize).Height + CategoryTitleGap;

            // The last value label is centred on the axis' right end, so half of it overhangs.
            right -= valueLabel / 2;
        }

        return right <= left || bottom <= top
            ? DocRect.Empty
            : new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>The value axis: its line, its ticks, its gridlines and its labels.</summary>
    /// <remarks>
    /// <strong>A secondary axis is the same axis on the other side of the plot area, and it draws
    /// no gridlines.</strong> Two sets of gridlines over one plot area are two grids at different
    /// spacings, which LibreOffice does not draw either: <c>c:majorGridlines</c> on the secondary
    /// <c>c:valAx</c> is honoured by chart2 only when the primary has none.
    /// </remarks>
    private static void AddValueAxis(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        bool columns,
        NumberFormatCode? format,
        bool secondary,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        // The axis line itself runs the full extent of the plot area on the side the value axis
        // is on: the left edge for columns, the bottom edge for bars — and the far side of each
        // for a secondary axis.
        Length axisX = secondary ? area.Right : area.Left;
        Length axisY = secondary ? area.Top : area.Bottom;
        int outward = secondary ? 1 : -1;

        // A deleted axis keeps its gridlines and loses everything else, so the line, the ticks and
        // the labels are all gated and the grid inside the loop is not.
        bool visible = secondary ? plot.SecondaryAxisVisible : plot.ValueAxisVisible;

        if (visible)
        {
            lines.Add(columns
                ? new ChartLine(
                    new DocPoint(axisX, area.Top), new DocPoint(axisX, area.Bottom), AxisColour)
                : new ChartLine(
                    new DocPoint(area.Left, axisY), new DocPoint(area.Right, axisY), AxisColour));
        }

        foreach (double tick in scale.MajorTicks())
        {
            double along = scale.Fraction(tick);

            if (columns)
            {
                // A fraction of 0 is the axis minimum, which is the *bottom* of a column
                // chart's plot area — hence the subtraction rather than an addition.
                Length y = area.Bottom - area.Height * along;

                if (!secondary && plot.ValueGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(area.Left, y), new DocPoint(area.Right, y), grid));
                }

                if (!visible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(axisX + TickLength * outward, y),
                    new DocPoint(axisX, y),
                    AxisColour));

                labels.Add(new ChartLabel(
                    ChartDataLabel.Write(tick, format),
                    new DocPoint(axisX + (TickLength + LabelSpacing) * outward, y),
                    secondary ? ChartLabelAnchor.LeftMiddle : ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
            }
            else
            {
                Length x = area.Left + area.Width * along;

                if (!secondary && plot.ValueGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Top), new DocPoint(x, area.Bottom), grid));
                }

                if (!visible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(x, axisY),
                    new DocPoint(x, axisY - TickLength * outward),
                    AxisColour));

                labels.Add(new ChartLabel(
                    ChartDataLabel.Write(tick, format),
                    new DocPoint(x, axisY - (TickLength + LabelSpacing) * outward),
                    secondary ? ChartLabelAnchor.CentreBottom : ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour));
            }
        }
    }

    /// <summary>
    /// A scatter chart's X axis: numeric ticks along the category axis' place.
    /// </summary>
    /// <remarks>
    /// The same shape as <see cref="AddValueAxis"/> with the roles of the two dimensions swapped,
    /// which is what a scatter chart is: <c>VSeriesPlotter</c> builds it from two
    /// <c>ExplicitScaleData</c> rather than one scale and a category list, and the only structural
    /// difference from a line chart is that the point's position across the plot area comes from
    /// its own number instead of from its index.
    /// </remarks>
    private static void AddDomainAxis(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult domain,
        bool columns,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (plot.CategoryAxisVisible)
        {
            lines.Add(columns
                ? new ChartLine(
                    new DocPoint(area.Left, area.Bottom),
                    new DocPoint(area.Right, area.Bottom),
                    AxisColour)
                : new ChartLine(
                    new DocPoint(area.Left, area.Top),
                    new DocPoint(area.Left, area.Bottom),
                    AxisColour));
        }

        foreach (double tick in domain.MajorTicks())
        {
            double along = domain.Fraction(tick);
            string text = ChartDataLabel.Write(tick, plot.DomainFormat);

            if (columns)
            {
                Length x = area.Left + area.Width * along;

                if (plot.CategoryGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Top), new DocPoint(x, area.Bottom), grid));
                }

                if (!plot.CategoryAxisVisible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(x, area.Bottom), new DocPoint(x, area.Bottom + TickLength), AxisColour));
                labels.Add(new ChartLabel(
                    text,
                    new DocPoint(x, area.Bottom + TickLength + LabelSpacing),
                    ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour));
            }
            else
            {
                Length y = area.Bottom - area.Height * along;
                if (!plot.CategoryAxisVisible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(area.Left - TickLength, y), new DocPoint(area.Left, y), AxisColour));
                labels.Add(new ChartLabel(
                    text,
                    new DocPoint(area.Left - TickLength - LabelSpacing, y),
                    ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
            }
        }
    }

    /// <summary>
    /// The category axis: its line, one tick per category boundary, and one label per category.
    /// </summary>
    /// <remarks>
    /// <strong>Ticks fall between categories and labels fall in the middle of them.</strong> Four
    /// categories produce five ticks, not four — the reference PDF draws them at the plot area's
    /// left edge, its right edge and the three boundaries between — and the labels sit at the
    /// centres, which is what <c>c:crossBetween val="between"</c> asks for and what every bar
    /// chart uses. Drawing a tick per label instead puts every category label half a slot from
    /// its own bars.
    /// </remarks>
    private static void AddCategoryAxis(
        ChartPlot plot,
        DocRect area,
        int categories,
        bool columns,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (plot.CategoryAxisVisible)
        {
            lines.Add(columns
                ? new ChartLine(
                    new DocPoint(area.Left, area.Bottom),
                    new DocPoint(area.Right, area.Bottom),
                    AxisColour)
                : new ChartLine(
                    new DocPoint(area.Left, area.Top),
                    new DocPoint(area.Left, area.Bottom),
                    AxisColour));
        }

        if (categories <= 0) return;

        // A shifted axis is divided into slots, so n categories give n + 1 boundaries; an
        // unshifted one is marked at n points, the first and last on the plot area's own edges.
        int ticks = plot.ShiftedCategories ? categories : categories - 1;

        for (int at = 0; at <= ticks; at++)
        {
            double along = ticks == 0 ? 0.0 : (double)at / ticks;

            if (columns)
            {
                Length x = area.Left + area.Width * along;

                if (plot.CategoryGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Top), new DocPoint(x, area.Bottom), grid));
                }

                if (plot.CategoryAxisVisible)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Bottom),
                        new DocPoint(x, area.Bottom + TickLength),
                        AxisColour));
                }
            }
            else
            {
                Length y = area.Bottom - area.Height * along;

                if (plot.CategoryGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(area.Left, y), new DocPoint(area.Right, y), grid));
                }

                if (plot.CategoryAxisVisible)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(area.Left - TickLength, y),
                        new DocPoint(area.Left, y),
                        AxisColour));
                }
            }
        }

        if (!plot.CategoryAxisVisible) return;

        for (int at = 0; at < categories; at++)
        {
            if (at >= plot.Categories.Count) continue;
            if (ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                is not { Length: > 0 } text)
            {
                continue;
            }

            double centre = CategoryAt(plot, at, categories);

            labels.Add(columns
                ? new ChartLabel(
                    text,
                    new DocPoint(
                        area.Left + area.Width * centre,
                        area.Bottom + TickLength + LabelSpacing),
                    ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour)
                : new ChartLabel(
                    text,
                    new DocPoint(
                        area.Left - TickLength - LabelSpacing,
                        area.Bottom - area.Height * centre),
                    ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
        }
    }

    /// <summary>
    /// Where a category sits along the axis, 0 at the plot area's start and 1 at its end.
    /// </summary>
    /// <remarks>
    /// The middle of its slot on a shifted axis and the tick itself on an unshifted one; see
    /// <see cref="ChartPlot.ShiftedCategories"/> for which chart type gets which. A single
    /// category on an unshifted axis has nowhere to be but the middle, which is what LibreOffice
    /// draws and what the division would otherwise make a division by zero.
    /// </remarks>
    private static double CategoryAt(ChartPlot plot, int index, int categories)
    {
        if (categories <= 0) return 0.5;
        if (plot.ShiftedCategories) return (index + 0.5) / categories;
        return categories == 1 ? 0.5 : (double)index / (categories - 1);
    }

    /// <summary>
    /// A line or scatter chart: one polyline per series through its points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VSeriesPlotter</c>'s line plotter joins consecutive points and <em>breaks the line at a
    /// gap</em> rather than bridging it — a category a series has no value for ends the current
    /// run and starts a new one (<c>AreaChart::createShapes</c>'s
    /// <c>PolyPolygonShapeInfo</c> handling of <c>bIsVisible</c>). Bridging instead draws a
    /// straight segment across the hole, which is the one thing a reader cannot tell from a real
    /// value.
    /// </para>
    /// <para>
    /// <strong>A scatter chart's points are placed by their own X and not by their index.</strong>
    /// When <paramref name="domain"/> is set the fraction across the plot area is
    /// <c>domain.Fraction(x)</c>, which is the only structural difference between the two types;
    /// spacing them evenly instead is right whenever the X values happen to be evenly spaced and
    /// wrong in proportion to how unevenly they are not.
    /// </para>
    /// </remarks>
    private static void AddLines(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        ChartScaleResult? domain,
        int categories,
        bool columns,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        if (categories <= 0) return;

        foreach (ChartSeries series in plot.Series)
        {
            GraphicsPath path = new();
            bool open = false;
            List<(DocPoint At, int Index, double Value)> points = [];

            int count = domain is not null && series.XValues is { } xs
                ? Math.Max(categories, xs.Count)
                : categories;

            for (int at = 0; at < count; at++)
            {
                if (at >= series.Values.Count
                    || series.Values[at] is not { } value
                    || !double.IsFinite(value)
                    || AcrossAt(plot, series, domain, at, categories) is not { } across)
                {
                    open = false;
                    continue;
                }

                DocPoint point = Point(area, across, scale.Fraction(value), columns);
                points.Add((point, at, value));

                if (open) path.LineTo(point);
                else path.MoveTo(point);

                open = true;
            }

            // Stroked in the series' fill when it states no line of its own, because that is what
            // both formats mean by a line series' colour: OOXML puts it on a:ln and ODF on the
            // series' stroke, and a series that states only a fill is drawn in that fill.
            Colour stroke = series.Line ?? series.Fill ?? Colour.Black;

            if (series.HasLine && path.Commands.Count >= 2)
                shapes.Add(new ChartShape(path, null, stroke, series.LineWidth));

            if (series.Marker != ChartMarker.None)
            {
                Length size = plot.LabelSize * MarkerSize;
                foreach ((DocPoint at, _, _) in points)
                    shapes.Add(Marker(series.Marker, at, size, series.Fill ?? stroke, stroke));
            }

            AddPointLabels(plot, series, points, ChartLabelPlacement.Right, area, labels);
        }
    }

    /// <summary>
    /// Where one point sits across the plot area, 0 at the start and 1 at the end.
    /// </summary>
    /// <remarks>
    /// The scatter case and the category case in one place, because the two differ only here.
    /// Null when a scatter point states no X, which is a genuine gap in the domain sequence and
    /// breaks the polyline exactly as a missing Y does.
    /// </remarks>
    private static double? AcrossAt(
        ChartPlot plot, ChartSeries series, ChartScaleResult? domain, int at, int categories)
    {
        if (domain is not { } across || series.XValues is not { } xs)
            return at < categories ? CategoryAt(plot, at, categories) : null;

        if (at >= xs.Count || xs[at] is not { } x || !double.IsFinite(x)) return null;
        return across.Fraction(x);
    }

    /// <summary>
    /// The data labels of a series drawn at points rather than as rectangles.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A line, a scatter and an area chart all name a single point, so the placement reduces to
    /// which side of it the text goes: <c>RIGHT</c> for a line and a scatter, <c>CENTER</c> for an
    /// area (<c>oox/source/drawingml/chart/typegroupconverter.cxx:98-99</c>). Anything else the
    /// file states is honoured where it maps onto one of the four anchors and falls back to the
    /// type's own default where it does not.
    /// </para>
    /// <para>
    /// The gap is a fifth of the type size, which is <c>LabelPositionHelper</c>'s own offset once
    /// its <c>fXOffset</c>/<c>fYOffset</c> pair is reduced to the non-3-D case.
    /// </para>
    /// </remarks>
    private static void AddPointLabels(
        ChartPlot plot,
        ChartSeries series,
        List<(DocPoint At, int Index, double Value)> points,
        ChartLabelPlacement fallback,
        DocRect area,
        List<ChartLabel> labels)
    {
        if (points.Count == 0) return;

        double total = series.Total();
        Length gap = plot.LabelSize / 5;

        foreach ((DocPoint at, int index, double value) in points)
        {
            if (series.LabelAt(index) is not { Draws: true } label) continue;

            string? text = label.Compose(
                index < plot.Categories.Count ? plot.Categories[index] : null,
                series.Name,
                value,
                total);

            if (text is not { Length: > 0 }) continue;

            (DocPoint where, ChartLabelAnchor anchor) = (label.Placement ?? fallback) switch
            {
                ChartLabelPlacement.Left =>
                    (new DocPoint(at.X - gap, at.Y), ChartLabelAnchor.RightMiddle),
                ChartLabelPlacement.Top or ChartLabelPlacement.Outside =>
                    (new DocPoint(at.X, at.Y - gap), ChartLabelAnchor.CentreBottom),
                ChartLabelPlacement.Bottom =>
                    (new DocPoint(at.X, at.Y + gap), ChartLabelAnchor.CentreTop),
                ChartLabelPlacement.Centre or ChartLabelPlacement.Inside =>
                    (at, ChartLabelAnchor.Centre),
                _ => (new DocPoint(at.X + gap, at.Y), ChartLabelAnchor.LeftMiddle),
            };

            // A label that would fall outside the plot area is pulled back inside it, which is
            // what "bestFit" asks for and what LibreOffice does for every placement anyway
            // (LabelPositionHelper::correctPositionForRotation's clamp against the diagram).
            if (where.X < area.Left) where = new DocPoint(area.Left, where.Y);
            if (where.X > area.Right) where = new DocPoint(area.Right, where.Y);

            labels.Add(new ChartLabel(text, where, anchor, plot.LabelSize, AxisColour));
        }
    }

    /// <summary>A marker's side, as a fraction of the label size.</summary>
    /// <remarks>
    /// <c>VDataSeries::getSymbolProperties</c> defaults a symbol to 250 × 250 hundredths of a
    /// millimetre, which on the 10 pt labels every chart in the corpus uses is 0.71 of the em.
    /// Expressing it against the type rather than absolutely is what makes it survive the
    /// stretch an embedded chart goes through.
    /// </remarks>
    private const double MarkerSize = 0.7;

    /// <summary>One marker, as a path centred on the point.</summary>
    private static ChartShape Marker(
        ChartMarker kind, DocPoint at, Length size, Colour fill, Colour stroke)
    {
        Length half = size / 2;
        GraphicsPath path = new();

        switch (kind)
        {
            case ChartMarker.Circle:
            {
                // Four cubics, the standard 0.5523 approximation; a circle drawn as a square is
                // the one marker shape a reader notices at a glance.
                Length k = half * 0.5522847498307936;
                path.MoveTo(new DocPoint(at.X + half, at.Y));
                path.CubicTo(
                    new DocPoint(at.X + half, at.Y + k), new DocPoint(at.X + k, at.Y + half),
                    new DocPoint(at.X, at.Y + half));
                path.CubicTo(
                    new DocPoint(at.X - k, at.Y + half), new DocPoint(at.X - half, at.Y + k),
                    new DocPoint(at.X - half, at.Y));
                path.CubicTo(
                    new DocPoint(at.X - half, at.Y - k), new DocPoint(at.X - k, at.Y - half),
                    new DocPoint(at.X, at.Y - half));
                path.CubicTo(
                    new DocPoint(at.X + k, at.Y - half), new DocPoint(at.X + half, at.Y - k),
                    new DocPoint(at.X + half, at.Y));
                path.Close();
                return new ChartShape(path, fill, null);
            }

            case ChartMarker.Diamond:
                path.MoveTo(new DocPoint(at.X, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y));
                path.LineTo(new DocPoint(at.X, at.Y + half));
                path.LineTo(new DocPoint(at.X - half, at.Y));
                path.Close();
                return new ChartShape(path, fill, null);

            case ChartMarker.Triangle:
                path.MoveTo(new DocPoint(at.X, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y + half));
                path.LineTo(new DocPoint(at.X - half, at.Y + half));
                path.Close();
                return new ChartShape(path, fill, null);

            case ChartMarker.Cross:
                path.MoveTo(new DocPoint(at.X - half, at.Y));
                path.LineTo(new DocPoint(at.X + half, at.Y));
                path.MoveTo(new DocPoint(at.X, at.Y - half));
                path.LineTo(new DocPoint(at.X, at.Y + half));
                return new ChartShape(path, null, stroke);

            case ChartMarker.Star:
                path.MoveTo(new DocPoint(at.X - half, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y + half));
                path.MoveTo(new DocPoint(at.X + half, at.Y - half));
                path.LineTo(new DocPoint(at.X - half, at.Y + half));
                return new ChartShape(path, null, stroke);

            default:
                path.MoveTo(new DocPoint(at.X - half, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y + half));
                path.LineTo(new DocPoint(at.X - half, at.Y + half));
                path.Close();
                return new ChartShape(path, fill, null);
        }
    }

    /// <summary>
    /// An area chart: one filled region per series, from the baseline up to its points.
    /// </summary>
    /// <remarks>
    /// Stacked areas pile onto a running total per category exactly as stacked bars do, so the
    /// lower edge of a series is the upper edge of the one below it rather than the baseline.
    /// Unstacked areas are drawn in file order and overlap, which is what LibreOffice draws and
    /// what makes a later series hide an earlier one — the reason a real area chart is usually
    /// stacked.
    /// </remarks>
    private static void AddAreas(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        bool columns,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        if (categories <= 0) return;

        double baseline = Math.Clamp(scale.Fraction(0.0), 0.0, 1.0);
        double[] running = new double[categories];
        double[] previous = new double[categories];

        for (int at = 0; at < categories; at++) previous[at] = baseline;

        foreach (ChartSeries series in plot.Series)
        {
            List<DocPoint> upper = [];
            List<DocPoint> lower = [];

            for (int at = 0; at < categories; at++)
            {
                double value = at < series.Values.Count && series.Values[at] is { } stated
                               && double.IsFinite(stated)
                    ? stated
                    : 0.0;

                double top;

                if (plot.IsStacked)
                {
                    running[at] += value;
                    top = scale.Fraction(running[at]);
                }
                else
                {
                    top = scale.Fraction(value);
                }

                double across = CategoryAt(plot, at, categories);
                upper.Add(Point(area, across, top, columns));
                lower.Add(Point(area, across, plot.IsStacked ? previous[at] : baseline, columns));
            }

            if (upper.Count < 2) continue;

            GraphicsPath path = new();
            path.MoveTo(upper[0]);
            for (int at = 1; at < upper.Count; at++) path.LineTo(upper[at]);
            for (int at = lower.Count - 1; at >= 0; at--) path.LineTo(lower[at]);
            path.Close();

            shapes.Add(new ChartShape(path, series.Fill, series.Line, series.LineWidth));

            List<(DocPoint, int, double)> points = [];
            for (int at = 0; at < categories && at < upper.Count; at++)
            {
                if (at >= series.Values.Count) continue;
                if (series.Values[at] is not { } stated || !double.IsFinite(stated)) continue;
                points.Add((upper[at], at, stated));
            }

            AddPointLabels(plot, series, points, ChartLabelPlacement.Centre, area, labels);

            if (plot.IsStacked)
            {
                for (int at = 0; at < categories; at++) previous[at] = scale.Fraction(running[at]);
            }
        }
    }

    /// <summary>
    /// A pie chart: one wedge per category of the first series.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A pie plots one series and colours it per point.</strong> Where a bar chart's
    /// colours belong to the series, a pie's belong to the points — <c>c:dPt</c>, ODF's
    /// <c>chart:data-point</c> — and the legend names the categories rather than the series. Only
    /// the first series is drawn, which is what a pie of several series means to every application
    /// but Excel's "pie of pie".
    /// </para>
    /// <para>
    /// <strong>The first wedge starts at twelve o'clock and they run clockwise.</strong>
    /// <c>PieChart::createShapes</c> begins at <c>fAngleDegree = 90</c> and subtracts, which is
    /// what both formats' default <c>firstSliceAng="0"</c> means. Starting at three o'clock, which
    /// is what a naive polar conversion gives, turns every wedge a quarter turn and is the kind of
    /// error that looks like a data ordering bug.
    /// </para>
    /// <para>
    /// The circle is inscribed in the plot area, its radius the smaller half-extent, which is
    /// <c>PolarPlottingPositionHelper</c>'s unit circle mapped through the diagram's rectangle.
    /// </para>
    /// </remarks>
    private static void AddWedges(
        ChartPlot plot, DocRect area, List<ChartShape> shapes, List<ChartLabel> labels)
    {
        if (plot.Series.Count == 0) return;

        ChartSeries series = plot.Series[0];

        double total = series.Total();
        if (!(total > 0.0)) return;

        DocPoint centre = new(area.X + area.Width / 2, area.Y + area.Height / 2);
        Length radius = area.Width < area.Height ? area.Width / 2 : area.Height / 2;
        if (radius <= Length.Zero) return;

        double start = Math.PI / 2;

        for (int at = 0; at < series.Values.Count; at++)
        {
            if (series.Values[at] is not { } value || !double.IsFinite(value)) continue;

            double sweep = Math.Abs(value) / total * (2 * Math.PI);
            if (sweep <= 0.0) { continue; }

            shapes.Add(new ChartShape(
                Wedge(centre, radius, start, -sweep),
                series.FillAt(at),
                series.Line,
                series.LineWidth));

            // The label sits on the wedge's own bisector. Its distance from the centre is what the
            // placement decides: chart2's AVOID_OVERLAP and CENTER both put it inside, at 0.5 of
            // the radius (PolarLabelPositionHelper::getLabelScreenPositionAndAlignment's
            // fLogicZ / bCenter branch), and OUTSIDE puts it just beyond the rim at 1.1. Putting
            // every pie label at the centre of the circle instead stacks them all on one another,
            // which reads as one label rather than as eight.
            if (series.LabelAt(at) is { Draws: true } label)
            {
                double middle = start - sweep / 2;
                double reach = label.Placement is ChartLabelPlacement.Outside ? 1.1 : 0.5;

                string? text = label.Compose(
                    at < plot.Categories.Count ? plot.Categories[at] : null,
                    series.Name,
                    value,
                    total);

                if (text is { Length: > 0 })
                {
                    labels.Add(new ChartLabel(
                        text,
                        new DocPoint(
                            centre.X + radius * (reach * Math.Cos(middle)),
                            centre.Y - radius * (reach * Math.Sin(middle))),
                        ChartLabelAnchor.Centre,
                        plot.LabelSize,
                        AxisColour));
                }
            }

            start -= sweep;
        }
    }

    /// <summary>
    /// One wedge, as a path: a radius out, an arc, and a radius back.
    /// </summary>
    /// <remarks>
    /// The arc is split into segments of at most a quarter turn and each is the standard cubic
    /// approximation, whose control handles are <c>4/3 × tan(θ/4) × r</c> from the end points along
    /// the tangents. One cubic for a whole half-circle is visibly flat at the sides; a quarter is
    /// accurate to a thousandth of the radius, which is below anything a page can show.
    /// </remarks>
    private static GraphicsPath Wedge(DocPoint centre, Length radius, double start, double sweep)
    {
        GraphicsPath path = new();
        path.MoveTo(centre);
        path.LineTo(On(start));

        int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / segments;
        double handle = 4.0 / 3.0 * Math.Tan(step / 4.0);

        double angle = start;

        for (int at = 0; at < segments; at++)
        {
            DocPoint from = On(angle);
            DocPoint to = On(angle + step);

            // P1 = P0 + k·r·T(θ0) and P2 = P3 − k·r·T(θ1), where T is the tangent d/dθ of the
            // parametrisation — which in a y-down space is (−sin θ, −cos θ) rather than
            // (−sin θ, cos θ). Getting that one sign wrong bends every arc the wrong way and
            // draws a pie as a pinwheel, which is what it looks like and not what it is.
            path.CubicTo(
                new DocPoint(
                    from.X - radius * (handle * Math.Sin(angle)),
                    from.Y - radius * (handle * Math.Cos(angle))),
                new DocPoint(
                    to.X + radius * (handle * Math.Sin(angle + step)),
                    to.Y + radius * (handle * Math.Cos(angle + step))),
                to);

            angle += step;
        }

        path.LineTo(centre);
        path.Close();
        return path;

        // The y term is negated because a document's y axis points down and an angle's does not.
        DocPoint On(double at)
            => new(centre.X + radius * Math.Cos(at), centre.Y - radius * Math.Sin(at));
    }

    /// <summary>
    /// A point in the plot area from its two fractions, whichever way round the axes are.
    /// </summary>
    private static DocPoint Point(DocRect area, double across, double up, bool columns)
        => columns
            ? new DocPoint(area.Left + area.Width * across, area.Bottom - area.Height * up)
            : new DocPoint(area.Left + area.Width * up, area.Bottom - area.Height * across);

    /// <summary>
    /// The bars.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A category slot is the plot area divided by the category count; a bar is that slot divided
    /// by <c>series + gapWidth/100 + (−overlap/100) × (series − 1)</c>, which is
    /// <c>CategoryPositionHelper::getScaledSlotWidth</c>
    /// (<c>chart2/source/view/charttypes/CategoryPositionHelper.cxx:37-45</c>) with
    /// <c>setOuterDistance(gapWidth/100)</c> and <c>setInnerDistance(−overlap/100)</c> from
    /// <c>BarChart.cxx:78-80</c>. So a clustered pair with the default gap of 100 divides its
    /// slot into three, and a stacked pair with an overlap of 100 divides it into two — of which
    /// the two series occupy the same one.
    /// </para>
    /// <para>
    /// The bar's base is the axis' zero, or the axis minimum when the whole scale is above or
    /// below zero. That is what makes a negative value draw downwards from the same line its
    /// positive neighbour draws up from, rather than upwards from the bottom of the plot.
    /// </para>
    /// </remarks>
    private static void AddBars(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        bool columns,
        List<ChartBox> boxes,
        List<ChartLabel> labels)
    {
        if (categories <= 0 || plot.Series.Count == 0) return;

        int series = plot.Series.Count;
        double outer = Math.Clamp(plot.GapWidth / 100.0, 0.0, 6.0);
        double inner = Math.Clamp(-plot.Overlap / 100.0, -1.0, 1.0);

        double denominator = series + outer + inner * (series - 1);
        if (!(denominator > 0.0)) return;

        double slotFraction = 1.0 / (categories * denominator);
        double baseline = Math.Clamp(scale.Fraction(0.0), 0.0, 1.0);

        // A stacked chart's series pile onto a running total per category rather than each
        // starting from the baseline, and positives and negatives pile separately so that a
        // mixed category does not cancel itself out.
        double[] positive = new double[categories];
        double[] negative = new double[categories];

        for (int index = 0; index < series; index++)
        {
            ChartSeries one = plot.Series[index];

            for (int at = 0; at < categories; at++)
            {
                if (at >= one.Values.Count) continue;
                if (one.Values[at] is not { } value || !double.IsFinite(value)) continue;

                double from;
                double to;

                if (plot.IsStacked)
                {
                    ref double running = ref (value >= 0.0 ? ref positive[at] : ref negative[at]);
                    from = scale.Fraction(running);
                    running += value;
                    to = scale.Fraction(running);
                }
                else
                {
                    from = baseline;
                    to = scale.Fraction(value);
                }

                // The slot the bar sits in, as a fraction of the plot area's long side.
                double slotStart = (double)at / categories
                    + (outer / 2.0 + index * (1.0 + inner)) * slotFraction;

                DocRect bounds = columns
                    ? Rectangle(
                        area.Left + area.Width * slotStart,
                        area.Bottom - area.Height * Math.Max(from, to),
                        area.Width * slotFraction,
                        area.Height * Math.Abs(to - from))
                    : Rectangle(
                        area.Left + area.Width * Math.Min(from, to),
                        area.Bottom - area.Height * (slotStart + slotFraction),
                        area.Width * Math.Abs(to - from),
                        area.Height * slotFraction);

                boxes.Add(new ChartBox(bounds, one.Fill, one.Line, one.LineWidth));

                if (one.LabelAt(at) is { Draws: true } label)
                    AddBarLabel(plot, one, label, at, value, bounds, to >= from, columns, labels);
            }
        }
    }

    /// <summary>
    /// One bar's data label, placed against the rectangle it names.
    /// </summary>
    /// <remarks>
    /// <c>OUTSIDE</c> is a bar chart's default (<c>typegroupconverter.cxx:95</c>) and means beyond
    /// the growing end — above a positive column and below a negative one, right of a positive bar
    /// and left of a negative one. <c>INSIDE</c> is the same end from the other side,
    /// <c>NEAR_ORIGIN</c> is the baseline end and <c>CENTER</c> is the middle, which is what a
    /// stacked chart normally uses because an outside label would sit on the series above.
    /// </remarks>
    private static void AddBarLabel(
        ChartPlot plot,
        ChartSeries series,
        ChartDataLabel label,
        int index,
        double value,
        DocRect bounds,
        bool grows,
        bool columns,
        List<ChartLabel> labels)
    {
        string? text = label.Compose(
            index < plot.Categories.Count ? plot.Categories[index] : null,
            series.Name,
            value,
            series.Total());

        if (text is not { Length: > 0 }) return;

        ChartLabelPlacement placement = label.Placement ?? ChartLabelPlacement.Outside;
        Length gap = plot.LabelSize / 5;

        DocPoint at;
        ChartLabelAnchor anchor;

        if (columns)
        {
            Length far = grows ? bounds.Top : bounds.Bottom;
            Length near = grows ? bounds.Bottom : bounds.Top;
            Length middle = bounds.Y + bounds.Height / 2;

            (at, anchor) = placement switch
            {
                ChartLabelPlacement.Centre or ChartLabelPlacement.BestFit =>
                    (new DocPoint(bounds.X + bounds.Width / 2, middle), ChartLabelAnchor.Centre),
                ChartLabelPlacement.Inside =>
                    (new DocPoint(bounds.X + bounds.Width / 2, far + gap),
                     grows ? ChartLabelAnchor.CentreTop : ChartLabelAnchor.CentreBottom),
                ChartLabelPlacement.NearOrigin =>
                    (new DocPoint(bounds.X + bounds.Width / 2, near),
                     grows ? ChartLabelAnchor.CentreBottom : ChartLabelAnchor.CentreTop),
                _ =>
                    (new DocPoint(bounds.X + bounds.Width / 2, far - (grows ? gap : -gap)),
                     grows ? ChartLabelAnchor.CentreBottom : ChartLabelAnchor.CentreTop),
            };
        }
        else
        {
            Length far = grows ? bounds.Right : bounds.Left;
            Length near = grows ? bounds.Left : bounds.Right;
            Length middle = bounds.Y + bounds.Height / 2;

            (at, anchor) = placement switch
            {
                ChartLabelPlacement.Centre or ChartLabelPlacement.BestFit =>
                    (new DocPoint(bounds.X + bounds.Width / 2, middle), ChartLabelAnchor.Centre),
                ChartLabelPlacement.Inside =>
                    (new DocPoint(far - (grows ? gap : -gap), middle),
                     grows ? ChartLabelAnchor.RightMiddle : ChartLabelAnchor.LeftMiddle),
                ChartLabelPlacement.NearOrigin =>
                    (new DocPoint(near, middle),
                     grows ? ChartLabelAnchor.LeftMiddle : ChartLabelAnchor.RightMiddle),
                _ =>
                    (new DocPoint(far + (grows ? gap : -gap), middle),
                     grows ? ChartLabelAnchor.LeftMiddle : ChartLabelAnchor.RightMiddle),
            };
        }

        labels.Add(new ChartLabel(text, at, anchor, plot.LabelSize, AxisColour));
    }

    /// <summary>The chart's title and its two axis titles.</summary>
    /// <remarks>
    /// Each is centred on what it names — the title on the whole frame, the category axis' title
    /// on the plot area's width, the value axis' on its height — which is
    /// <c>changePositionOfAxisTitle</c> (<c>ChartView.cxx:995-1046</c>) once the anchor is
    /// resolved to a rectangle. The value axis' title is rotated a quarter turn anticlockwise,
    /// which both formats state and neither leaves to the renderer.
    /// </remarks>
    private static void AddTitles(
        ChartPlot plot,
        DocRect frame,
        DocRect area,
        IChartTextMeasurer measurer,
        List<ChartLabel> labels)
    {
        if (plot.Title is { Length: > 0 } title)
        {
            Length height = measurer.Measure(title, plot.TitleSize).Height;
            labels.Add(new ChartLabel(
                title,
                new DocPoint(
                    frame.X + frame.Width / 2,
                    frame.Y + frame.Height * PageMargin + height / 2),
                ChartLabelAnchor.Centre,
                plot.TitleSize,
                AxisColour));
        }

        if (plot.CategoryAxisTitle is { Length: > 0 } category)
        {
            Length height = measurer.Measure(category, plot.AxisTitleSize).Height;
            labels.Add(new ChartLabel(
                category,
                new DocPoint(
                    area.X + area.Width / 2,
                    frame.Bottom - frame.Height * PageMargin - height / 2),
                ChartLabelAnchor.Centre,
                plot.AxisTitleSize,
                AxisColour));
        }

        if (plot.ValueAxisTitle is { Length: > 0 } value)
        {
            Length height = measurer.Measure(value, plot.AxisTitleSize).Height;
            labels.Add(new ChartLabel(
                value,
                new DocPoint(
                    frame.X + frame.Width * PageMargin + height / 2,
                    area.Y + area.Height / 2),
                ChartLabelAnchor.Centre,
                plot.AxisTitleSize,
                AxisColour,
                Math.PI / 2));
        }
    }

    /// <summary>
    /// The legend: one key and one name per series.
    /// </summary>
    /// <remarks>
    /// Placed against the plot area's vertical centre for a side legend and its horizontal centre
    /// for a top or bottom one, which is <c>VLegend::createShapes</c>'s
    /// <c>LegendExpansion::HIGH</c> and <c>WIDE</c> reduced to the single column and single row
    /// they produce for a handful of series. A legend with more series than fit in one column is
    /// wrapped by LibreOffice into several; this draws one column and lets it run, which is
    /// wrong only for a chart with more series than the plot area is tall.
    /// </remarks>
    private static void AddLegend(
        ChartPlot plot,
        DocRect frame,
        DocRect area,
        IChartTextMeasurer measurer,
        List<ChartBox> boxes,
        List<ChartLabel> labels)
    {
        if (plot.Legend == ChartLegendPosition.None) return;

        List<(string Name, Colour? Fill, Colour? Line, Length Width)> named = Entries(plot);
        if (named.Count == 0) return;

        Length line = measurer.Measure("0", plot.LabelSize).Height;
        Length key = line * 0.7;
        Length gap = line * 0.4;

        bool vertical = plot.Legend is ChartLegendPosition.Left or ChartLegendPosition.Right;

        Length top = vertical
            ? area.Y + area.Height / 2 - line * named.Count / 2
            : plot.Legend == ChartLegendPosition.Top
                ? frame.Y + frame.Height * PageMargin
                : frame.Bottom - frame.Height * PageMargin - line;

        Length left = plot.Legend switch
        {
            ChartLegendPosition.Right => area.Right + measurer.Measure("0", plot.LabelSize).Width,
            ChartLegendPosition.Left => frame.X + frame.Width * PageMargin,
            _ => area.X + area.Width / 2 - LegendWidth(plot, measurer) / 2,
        };

        foreach ((string name, Colour? fill, Colour? outline, Length width) in named)
        {
            boxes.Add(new ChartBox(
                Rectangle(left, top + (line - key) / 2, key, key), fill, outline, width));

            labels.Add(new ChartLabel(
                name,
                new DocPoint(left + key + gap, top + line / 2),
                ChartLabelAnchor.LeftMiddle,
                plot.LabelSize,
                AxisColour));

            if (vertical) top += line;
            else left += key + gap + measurer.Measure(name, plot.LabelSize).Width + gap * 2;
        }
    }

    /// <summary>
    /// What the legend lists: the series, or a pie's categories.
    /// </summary>
    /// <remarks>
    /// <strong>A pie's legend names its categories.</strong> It plots one series, so listing the
    /// series would give a legend of one entry beside a picture of eight wedges;
    /// <c>VLegend</c> takes its entries from the plotter, and <c>PieChart</c> supplies one per
    /// point with that point's own colour (<c>VSeriesPlotter::createLegendEntries</c>'s
    /// <c>bIsPie</c> branch). Getting this wrong is worth several words of a word count and the
    /// whole legend of a picture.
    /// </remarks>
    /// <remarks>
    /// <strong>And a category with no name of its own is numbered, not skipped.</strong> A chart
    /// part need not state a <c>c:cat</c> at all — <c>barOfPieChart.xlsx</c> and
    /// <c>pieOfPieChart.xlsx</c> state a <c>c:val</c> and nothing else — and LibreOffice then
    /// generates the 1-based index as each category's name, which is
    /// <c>ExplicitCategoriesProvider</c>'s <c>lcl_getGeneratedCategories</c>. Its PDF for
    /// <c>barOfPieChart.xlsx</c> draws a nine-entry legend reading <c>1 2 … 9</c> against a
    /// spreadsheet whose own cells read <c>9 8 … 1</c>; skipping the unnamed categories instead
    /// draws no legend at all, which is nine of that file's words.
    /// </remarks>
    /// <remarks>
    /// The generation is gated on the chart stating <em>no</em> category sequence at all, not on
    /// an individual name being empty. A stated but blank category is a blank label and stays
    /// one — <c>ExplicitCategoriesProvider</c> generates the whole run or none of it — and
    /// numbering the blanks inside a stated sequence would invent labels on every sparse pie in
    /// the corpus.
    /// </remarks>
    private static List<(string Name, Colour? Fill, Colour? Line, Length Width)> Entries(
        ChartPlot plot)
    {
        List<(string, Colour?, Colour?, Length)> entries = [];

        if (plot.Kind is ChartPlotKind.Pie or ChartPlotKind.OfPie)
        {
            ChartSeries? first = plot.Series.Count > 0 ? plot.Series[0] : null;
            int points = Math.Max(plot.Categories.Count, first?.Values.Count ?? 0);

            bool generated = plot.Categories.Count == 0;

            for (int at = 0; at < points; at++)
            {
                string? stated = generated
                    ? (at + 1).ToString(System.Globalization.CultureInfo.InvariantCulture)
                    : at < plot.Categories.Count
                        ? ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                        : null;

                if (stated is not { Length: > 0 }) continue;

                entries.Add((stated, first?.FillAt(at), first?.Line, first?.LineWidth ?? Length.Zero));
            }

            return entries;
        }

        foreach (ChartSeries series in plot.Series)
        {
            if (series.Name is not { Length: > 0 } name) continue;
            entries.Add((name, series.Fill, series.Line, series.LineWidth));
        }

        return entries;
    }

    /// <summary>How wide a vertical legend is, keys and names together.</summary>
    private static Length LegendWidth(ChartPlot plot, IChartTextMeasurer measurer)
    {
        if (plot.Legend == ChartLegendPosition.None) return Length.Zero;

        Length line = measurer.Measure("0", plot.LabelSize).Height;
        Length widest = Length.Zero;
        int count = 0;

        foreach ((string name, _, _, _) in Entries(plot))
        {
            count++;
            Length width = measurer.Measure(name, plot.LabelSize).Width;
            if (width > widest) widest = width;
        }

        if (count == 0) return Length.Zero;

        bool vertical = plot.Legend is ChartLegendPosition.Left or ChartLegendPosition.Right;
        Length one = line * 0.7 + line * 0.4 + widest;
        return vertical ? one : one * count;
    }

    /// <summary>How tall a horizontal legend is.</summary>
    private static Length LegendHeight(ChartPlot plot, IChartTextMeasurer measurer)
        => LegendWidth(plot, measurer) > Length.Zero
            ? measurer.Measure("0", plot.LabelSize).Height
            : Length.Zero;

    /// <summary>The width of the widest value-axis label.</summary>
    private static Length WidestValueLabel(
        ChartScaleResult scale,
        NumberFormatCode? format,
        Length size,
        IChartTextMeasurer measurer)
    {
        Length widest = Length.Zero;
        foreach (double tick in scale.MajorTicks())
        {
            Length width = measurer.Measure(ChartDataLabel.Write(tick, format), size).Width;
            if (width > widest) widest = width;
        }

        return widest;
    }

    /// <summary>The width of the widest category label.</summary>
    private static Length WidestCategoryLabel(
        ChartPlot plot, int categories, IChartTextMeasurer measurer)
    {
        Length widest = Length.Zero;
        for (int at = 0; at < categories && at < plot.Categories.Count; at++)
        {
            if (ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                is not { Length: > 0 } text)
            {
                continue;
            }

            Length width = measurer.Measure(text, plot.LabelSize).Width;
            if (width > widest) widest = width;
        }

        return widest;
    }

    /// <summary>A rectangle from its edges, never negative in either direction.</summary>
    private static DocRect Rectangle(Length x, Length y, Length width, Length height)
        => new(x, y, width < Length.Zero ? Length.Zero : width,
               height < Length.Zero ? Length.Zero : height);
}
